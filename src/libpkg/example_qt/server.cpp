/*                       S E R V E R . C
 * BRL-CAD
 *
 * Copyright (c) 2006-2021 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file libpkg/example/server.c
 *
 * Basic pkg server.
 *
 */

#include "common.h"

/* system headers */
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "bio.h"

/* interface headers */
#include "bu/log.h"
#include "bu/str.h"
#include "bu/malloc.h"
#include "bu/getopt.h"
#include "bu/vls.h"
#include "bu/snooze.h"
#include "pkg.h"

#define QT_SERVER
#include "ncp.h"

PKGServer::PKGServer()
    : QTcpServer()
{
    client = NULL;
}

PKGServer::~PKGServer()
{
    bu_vls_free(&buffer);
}

/*
 * callback when a HELO message packet is received.
 *
 * We should not encounter this packet specifically since we listened
 * for it before beginning processing of packets as part of a simple
 * handshake setup.
 */
void
server_helo(struct pkg_conn *UNUSED(connection), char *buf)
{
    bu_log("Unexpected HELO encountered\n");
    free(buf);
}

/* callback when a DATA message packet is received */
void
server_data(struct pkg_conn *UNUSED(connection), char *buf)
{
    bu_log("Received message from client: %s\n", buf);
    free(buf);
}


/* callback when a CIAO message packet is received */
void
server_ciao(struct pkg_conn *UNUSED(connection), char *buf)
{
    bu_log("CIAO encountered: %s\n", buf);
    free(buf);
}

int
main()
{
    PKGServer *tcps = new PKGServer;
    /** our server callbacks for each message type */
    struct pkg_switch callbacks[] = {
	{MSG_HELO, server_helo, "HELO", NULL},
	{MSG_DATA, server_data, "DATA", NULL},
	{MSG_CIAO, server_ciao, "CIAO", NULL},
	{0, 0, (char *)0, (void*)0}
    };

    /* ignore broken pipes, on platforms where we have SIGPIPE */
#ifdef SIGPIPE
    (void)signal(SIGPIPE, SIG_IGN);
#endif

    /* start up the server on the given port */
    char portname[MAX_DIGITS + 1] = {0};
    snprintf(portname, MAX_DIGITS, "%d", tcps->port);
    tcps->netfd = pkg_permserver(portname, "tcp", 0, 0);
    tcps->setSocketDescriptor(tcps->netfd);
    if (tcps->netfd < 0) {
	bu_bomb("Unable to start the server");
    } else {
	bu_log("netfd: %d\n", tcps->netfd);
    }

    /* listen for a good client indefinitely.  this is a simple
     * handshake that waits for a HELO message from the client.  if it
     * doesn't get one, the server continues to wait.
     */
    bu_log("Listening on port %d\n", tcps->port);
    do {
	tcps->client = pkg_getclient(tcps->netfd, callbacks, NULL, 0);
	if (tcps->client == PKC_NULL) {
	    bu_log("Connection seems to be busy, waiting...\n");
	    bu_snooze(BU_SEC2USEC(2));
	    continue;
	} else if (tcps->client == PKC_ERROR) {
	    bu_log("Fatal error accepting client connection.\n");
	    pkg_close(tcps->client);
	    tcps->client = PKC_NULL;
	    continue;
	}

	/* got a connection, process it */
	tcps->msgbuffer = pkg_bwaitfor (MSG_HELO, tcps->client);
	if (tcps->msgbuffer == NULL) {
	    bu_log("Failed to process the client connection, still waiting\n");
	    pkg_close(tcps->client);
	    tcps->client = PKC_NULL;
	} else {
	    bu_log("msgbuffer: %s\n", tcps->msgbuffer);
	    /* validate magic header that client should have sent */
	    if (!BU_STR_EQUAL(tcps->msgbuffer, MAGIC_ID)) {
		bu_log("Bizarre corruption, received a HELO without at matching MAGIC ID!\n");
		pkg_close(tcps->client);
		tcps->client = PKC_NULL;
	    }
	}
    } while (tcps->client == PKC_NULL);

    /* send the first message to the server */
    bu_vls_sprintf(&tcps->buffer, "This is a message from the server.");
    tcps->bytes = pkg_send(MSG_DATA, bu_vls_addr(&tcps->buffer), (size_t)bu_vls_strlen(&tcps->buffer)+1, tcps->client);
    if (tcps->bytes < 0) goto failure;

    /* send another message to the server */
    bu_vls_sprintf(&tcps->buffer, "Yet another message from the server.");
    tcps->bytes = pkg_send(MSG_DATA, bu_vls_addr(&tcps->buffer), (size_t)bu_vls_strlen(&tcps->buffer)+1, tcps->client);
    if (tcps->bytes < 0) goto failure;

    /* Tell the client we're done */
    tcps->bytes = pkg_send(MSG_CIAO, "DONE", 5, tcps->client);
    if (tcps->bytes < 0) {
	bu_log("Connection to client seems faulty.\n");
    }

    /* Wait to hear from the client */
    do {
	(void)pkg_process(tcps->client);
	(void)pkg_suckin(tcps->client);
	(void)pkg_process(tcps->client);
    } while (tcps->client->pkc_type != MSG_CIAO);


    /* Confirm the client is done */
    (void)pkg_bwaitfor(MSG_CIAO , tcps->client);

    /* shut down the server, one-time use */
    pkg_close(tcps->client);
    delete tcps;
    return 0;
failure:
    pkg_close(tcps->client);
    bu_log("Unable to successfully send message");
    delete tcps;
    return 0;
}

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */