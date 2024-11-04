/*                         S E T U P . C
 * BRL-CAD
 *
 * Copyright (c) 1985-2024 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file mged/setup.c
 *
 * routines to initialize mged
 *
 */

#include "common.h"

/* system headers */
#include <stdlib.h>
#include <tcl.h>
#include <string.h>

/* common headers */
#include "vmath.h"
#include "bu/app.h"
#include "bn.h"
#include "bv/util.h"
#include "tclcad.h"
#include "ged.h"

/* local headers */
#include "./mged.h"
#include "./cmd.h"

/* catch auto-formatting errors in this file.  be careful as there are
 * mged_display() vars that use comma too.
 */
#define COMMA ','


extern Tk_Window tkwin; /* in cmd.c */

extern void init_qray(void);
extern void mged_global_variable_setup(Tcl_Interp *interpreter);

const char cmd3525[] = {'3', '5', COMMA, '2', '5', '\0'};
const char cmd4545[] = {'4', '5', COMMA, '4', '5', '\0'};

// We need to trigger MGED operations when opening and closing
// database files.  However, some commands like garbage_collect
// also need to do these operations, and they have no awareness
// of the extra steps MGED takes with f_opendb/f_closedb.  To
// allow both MGED and GED to do what they need, we define
// default callbacks in GEDP with MGED functions and data that
// will do the necessary work if the opendb/closedb functions
// are called at lower levels.
struct mged_opendb_ctx mged_global_db_ctx;

static struct cmdtab mged_cmdtab[] = {
    {MGED_CMD_MAGIC,"%", f_comm, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,cmd3525, f_bv_35_25, GED_FUNC_PTR_NULL,NULL}, /* 35,25 */
    {MGED_CMD_MAGIC,"3ptarb", cmd_ged_more_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,cmd4545, f_bv_45_45, GED_FUNC_PTR_NULL,NULL}, /* 45,45 */
    {MGED_CMD_MAGIC,"B", cmd_blast, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"accept", f_be_accept, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"adc", f_adc, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"adjust", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ae", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ae2dir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"aip", f_aip, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"analyze", cmd_ged_info_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"annotate", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"arb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"arced", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"area", f_area, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"arot", cmd_arot, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"art", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"attach", f_attach, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"attr", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"autoview", cmd_autoview, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"bb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bev", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bo", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bomb", f_bomb, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"bot", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_condense", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_decimate", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_dump", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_exterior", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_face_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_face_sort", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_flip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_merge", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_smooth", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_split", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_sync", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bot_vertex_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"bottom", f_bv_bottom, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"brep", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"c", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"cat", cmd_ged_info_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"cc", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"center", cmd_center, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"check", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"clone", cmd_ged_edit_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"closedb", f_closedb, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"cmd_win", cmd_cmd_win, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"coil", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"comb_color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"constraint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"copyeval", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"copymat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"cp", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"cpi", f_copy_inv, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"d", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"db", cmd_stub, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"db_glob", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dbconcat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dbfind", cmd_ged_info_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dbip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dbversion", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"debug", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"debugbu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"debugdir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"debuglib", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"debugnmg", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"decompose", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"delay", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dir2ae", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dump", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dm", f_dm, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"draw", cmd_draw, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"dsp", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"dup", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"E", cmd_E, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"e", cmd_draw, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"eac", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"echo", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"edcodes", f_edcodes, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"edit", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"edcolor", f_edcolor, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"edcomb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"edgedir", f_edgedir, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"edmater", f_edmater, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"env", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"erase", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ev", cmd_ev, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"eqn", f_eqn, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"exit", f_quit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"expand", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"extrude", f_extrude, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"eye_pt", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"exists", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"facedef", f_facedef, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"facetize", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"facetize_old", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"form", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"fracture", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"front", f_bv_front, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"g", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"gdiff", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"garbage_collect", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get_type", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get_autoview", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get_comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get_dbip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"get_dm_list", f_get_dm_list, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"get_more_default", cmd_get_more_default, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"get_sed", f_get_sedit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"get_sed_menus", f_get_sedit_menus, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"get_solid_keypoint", f_get_solid_keypoint, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"graph", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"gqa", cmd_ged_gqa, ged_exec,NULL},
    {MGED_CMD_MAGIC,"grid2model_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"grid2view_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"has_embedded_fb", cmd_has_embedded_fb, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"heal", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"hide", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"hist", cmd_hist, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"history", f_history, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"i", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"idents", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ill", f_ill, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"in", cmd_ged_in, ged_exec,NULL},
    {MGED_CMD_MAGIC,"inside", cmd_ged_inside, ged_exec,NULL},
    {MGED_CMD_MAGIC,"item", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"joint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"joint2", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"journal", f_journal, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"keep", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"keypoint", f_keypoint, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"kill", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"killall", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"killrefs", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"killtree", cmd_ged_erase_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"knob", f_knob, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"l", cmd_ged_info_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"labelvert", f_labelvert, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"labelface", f_labelface, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"lc", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"left", f_bv_left, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"lint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"listeval", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"loadtk", cmd_tk, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"loadview", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"lod", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"lookat", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ls", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"lt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"M", f_mouse, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"m2v_point", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"make", f_make, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"make_name", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"make_pnts", cmd_ged_more_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"match", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"material", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"matpick", f_matpick, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mat_ae", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mat_mul", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mat4x3pnt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mat_scale_about_pnt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mged_update", f_update, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mged_wait", f_wait, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mirface", f_mirface, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mirror", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mmenu_get", cmd_mmenu_get, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mmenu_set", cmd_nop, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"model2grid_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"model2view", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"model2view_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mrot", cmd_mrot, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"mv", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"mvall", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"nirt", f_nirt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"nmg_collapse", cmd_nmg_collapse, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"nmg_fix_normals", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"nmg_simplify", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"nmg", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"npush", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"o_rotate", f_be_o_rotate, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"o_scale", f_be_o_scale, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oed", cmd_oed, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oed_apply", f_oedit_apply, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oed_reset", f_oedit_reset, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oill", f_be_o_illuminate, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"opendb", f_opendb, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"orientation", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"orot", f_rot_obj, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oscale", f_sc_obj, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"output_hook", cmd_output_hook, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"overlay", cmd_overlay, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"ox", f_be_o_x, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oxscale", f_be_o_xscale, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oxy", f_be_o_xy, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oy", f_be_o_y, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"oyscale", f_be_o_yscale, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"ozscale", f_be_o_zscale, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"p", f_param, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"pathlist", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"paths", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"permute", f_permute, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"plot", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"png", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"pnts", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"prcolor", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"prefix", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"press", f_press, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"preview", cmd_ged_dm_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"process", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"postscript", f_postscript, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"ps", cmd_ps, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"pull", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"push", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"put", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"put_comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"put_sed", f_put_sedit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"putmat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"q", f_quit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"qorot", f_qorot, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"qray", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"query_ray", f_nirt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"quit", f_quit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"qvrot", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"r", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"rate", f_bv_rate_toggle, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rcodes", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"rear", f_bv_rear, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"red", f_red, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"refresh", f_refresh, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"regdebug", f_regdebug, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"regdef", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"regions", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"reject", f_be_reject, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"release", f_release, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"reset", f_bv_reset, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"restore", f_bv_vrestore, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rfarb", f_rfarb, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"right", f_bv_right, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rm", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"rmater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"rmats", f_rmats, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rot", cmd_rot, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rotobj", f_rot_obj, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rrt", cmd_rrt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rset", f_rset, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rt", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rt_gettrees", cmd_rt_gettrees, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rtabort", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"rtarea", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rtcheck", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rtedge", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"rtweight", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"save", f_bv_vsave, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"savekey", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"saveview", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"sca", cmd_sca, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"screengrab", cmd_ged_dm_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"search", cmd_search, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sed", f_sed, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sed_apply", f_sedit_apply, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sed_reset", f_sedit_reset, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sedit", f_be_s_edit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"select", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"set_more_default", cmd_set_more_default, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"setview", cmd_setview, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"shaded_mode", cmd_shaded_mode, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"shader", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"share", f_share, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"shells", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"showmats", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"sill", f_be_s_illuminate, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"size", cmd_size, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"simulate", cmd_ged_simulate_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"solid_report", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"solids", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"solids_on_ray", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"srot", f_be_s_rotate, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sscale", f_be_s_scale, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"stat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"status", f_status, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"stuff_str", cmd_stuff_str, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"summary", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"sv", f_slewview, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"svb", f_svbase, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sxy", f_be_s_trans, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"sync", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"t", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"ted", f_tedit, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"tie", f_tie, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"tire", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"title", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"tol", cmd_tol, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"top", f_bv_top, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"tops", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"tra", cmd_tra, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"track", f_amtrack, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"tracker", f_tracker, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"translate", f_tr_obj, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"tree", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"unhide", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"units", cmd_units, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"v2m_point", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"vars", f_set, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"vdraw", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"view", cmd_ged_view_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"view_ring", f_view_ring, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"view2grid_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"view2model", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"view2model_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"view2model_vec", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"viewdir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"viewsize", cmd_size, GED_FUNC_PTR_NULL,NULL}, /* alias "size" for saveview scripts */
    {MGED_CMD_MAGIC,"vnirt", f_vnirt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"voxelize", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"vquery_ray", f_vnirt, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"vrot", cmd_vrot, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"wcodes", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"whatid", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"which_shader", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"whichair", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"whichid", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"who", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"winset", f_winset, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"wmater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"x", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"xpush", cmd_ged_plain_wrapper, ged_exec,NULL},
    {MGED_CMD_MAGIC,"Z", cmd_zap, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"zoom", cmd_zoom, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"zoomin", f_bv_zoomin, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,"zoomout", f_bv_zoomout, GED_FUNC_PTR_NULL,NULL},
    {MGED_CMD_MAGIC,NULL, NULL, GED_FUNC_PTR_NULL, NULL}
};


/**
 * Register all MGED commands.
 */
static void
cmd_setup(struct mged_state *s)
{
    struct cmdtab *ctp;
    struct bu_vls temp = BU_VLS_INIT_ZERO;

    for (ctp = mged_cmdtab; ctp->name != NULL; ctp++) {
	ctp->s = s;
	bu_vls_strcpy(&temp, "_mged_");
	bu_vls_strcat(&temp, ctp->name);

	(void)Tcl_CreateCommand(INTERP, ctp->name, ctp->tcl_func,
				(ClientData)ctp, (Tcl_CmdDeleteProc *)NULL);
	(void)Tcl_CreateCommand(INTERP, bu_vls_addr(&temp), ctp->tcl_func,
				(ClientData)ctp, (Tcl_CmdDeleteProc *)NULL);
    }

    /* Init mged's Tcl interface to libwdb */
    Wdb_Init(INTERP);

    tkwin = NULL;

    bu_vls_free(&temp);
}


/*
 * Initialize mged, configure the path, set up the tcl interpreter.
 */
void
mged_setup(Tcl_Interp **interpreter)
{
    struct bu_vls str = BU_VLS_INIT_ZERO;
    struct bu_vls tlog = BU_VLS_INIT_ZERO;
    const char *name = bu_dir(NULL, 0, BU_DIR_BIN, bu_getprogname(), BU_DIR_EXT, NULL);

    /* locate our run-time binary (must be called before Tcl_CreateInterp()) */
    if (name) {
	Tcl_FindExecutable(name);
    } else {
	Tcl_FindExecutable("mged");
    }

    if (!interpreter ) {
      bu_log("mged_setup Error - interpreter is NULL!\n");
      return;
    }

    if (*interpreter != NULL)
	Tcl_DeleteInterp(*interpreter);

    /* Create the interpreter */
    *interpreter = Tcl_CreateInterp();

    /* Do basic Tcl initialization - note that Tk
     * is not initialized at this point. */
    if (tclcad_init(*interpreter, 0, &tlog) == TCL_ERROR) {
	bu_log("tclcad_init error:\n%s\n", bu_vls_addr(&tlog));
    }
    bu_vls_free(&tlog);

    BU_GET(MGED_STATE, struct mged_state);
    struct mged_state *s = MGED_STATE;
    s->interp = *interpreter;
    mged_global_db_ctx.s = s;

    BU_GET(s->GEDP, struct ged);
    GED_INIT(s->GEDP, NULL);
    s->GEDP->ged_output_handler = mged_output_handler;
    s->GEDP->ged_refresh_clientdata = (void *)s;
    s->GEDP->ged_refresh_handler = mged_refresh_handler;
    s->GEDP->vlist_ctx = (void *)s;
    s->GEDP->ged_create_vlist_scene_obj_callback = createDListSolid;
    s->GEDP->ged_create_vlist_display_list_callback = createDListAll;
    s->GEDP->ged_destroy_vlist_callback = freeDListsAll;
    s->GEDP->ged_create_io_handler = &tclcad_create_io_handler;
    s->GEDP->ged_delete_io_handler = &tclcad_delete_io_handler;
    s->GEDP->ged_pre_opendb_callback = &mged_pre_opendb_clbk;
    s->GEDP->ged_post_opendb_callback = &mged_post_opendb_clbk;
    s->GEDP->ged_pre_closedb_callback = &mged_pre_closedb_clbk;
    s->GEDP->ged_post_closedb_callback = &mged_post_closedb_clbk;
    s->GEDP->ged_db_callback_udata = &mged_global_db_ctx;
    s->GEDP->cmd_interp = (void *)interpreter;
    s->GEDP->search_ctx = (void *)s;
    s->GEDP->ged_search_eval = &mged_db_search_callback;
    struct tclcad_io_data *t_iod = tclcad_create_io_data();
    t_iod->io_mode = TCL_READABLE;
    t_iod->interp = *interpreter;
    s->GEDP->ged_io_data = t_iod;

    /* Set up the default state of the standard open/close db container */
    mged_global_db_ctx.argc = 0;
    mged_global_db_ctx.argv = NULL;
    mged_global_db_ctx.force_create = 0;
    mged_global_db_ctx.no_create = 0;
    mged_global_db_ctx.created_new_db = 0;
    mged_global_db_ctx.ret = 0;
    mged_global_db_ctx.ged_ret = 0;
    mged_global_db_ctx.interpreter = *interpreter;
    mged_global_db_ctx.old_dbip = NULL;
    mged_global_db_ctx.post_open_cnt = 0;

    BU_ALLOC(view_state->vs_gvp, struct bview);
    bv_init(view_state->vs_gvp, NULL);
    BU_GET(view_state->vs_gvp->callbacks, struct bu_ptbl);
    bu_ptbl_init(view_state->vs_gvp->callbacks, 8, "bv callbacks");

    view_state->vs_gvp->gv_callback = mged_view_callback;
    view_state->vs_gvp->gv_clientData = (void *)view_state;
    MAT_DELTAS_GET_NEG(view_state->vs_orig_pos, view_state->vs_gvp->gv_center);

    view_state->vs_gvp->vset = &s->GEDP->ged_views;

    bv_set_add_view(&s->GEDP->ged_views, view_state->vs_gvp);
    bu_ptbl_ins(&s->GEDP->ged_free_views, (long *)view_state->vs_gvp);
    s->GEDP->ged_gvp = view_state->vs_gvp;

    /* register commands */
    cmd_setup(s);

    history_setup();
    mged_global_variable_setup(*interpreter);
    mged_variable_setup(s, *interpreter);
    s->GEDP->cmd_interp = (void *)*interpreter;

    /* Tcl needs to write nulls onto subscripted variable names */
    bu_vls_printf(&str, "%s(state)", MGED_DISPLAY_VAR);
    Tcl_SetVar(*interpreter, bu_vls_addr(&str), state_str[STATE], TCL_GLOBAL_ONLY);

    /* Set defaults for view status variables */
    bu_vls_trunc(&str, 0);
    bu_vls_printf(&str, "set mged_display(.topid_0.ur,ang) {ang=(0.00 0.00 0.00)};\
set mged_display(.topid_0.ur,aet) {az=35.00  el=25.00  tw=0.00};\
set mged_display(.topid_0.ur,size) sz=1000.000;\
set mged_display(.topid_0.ur,center) {cent=(0.000 0.000 0.000)};\
set mged_display(units) mm");
    Tcl_Eval(*interpreter, bu_vls_addr(&str));

    Tcl_ResetResult(*interpreter);

    bu_vls_free(&str);
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
