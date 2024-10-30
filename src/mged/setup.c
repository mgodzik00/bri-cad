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
    {"%", f_comm, GED_FUNC_PTR_NULL,NULL},
    {cmd3525, f_bv_35_25, GED_FUNC_PTR_NULL,NULL}, /* 35,25 */
    {"3ptarb", cmd_ged_more_wrapper, ged_exec,NULL},
    {cmd4545, f_bv_45_45, GED_FUNC_PTR_NULL,NULL}, /* 45,45 */
    {"B", cmd_blast, GED_FUNC_PTR_NULL,NULL},
    {"accept", f_be_accept, GED_FUNC_PTR_NULL,NULL},
    {"adc", f_adc, GED_FUNC_PTR_NULL,NULL},
    {"adjust", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"ae", cmd_ged_view_wrapper, ged_exec,NULL},
    {"ae2dir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"aip", f_aip, GED_FUNC_PTR_NULL,NULL},
    {"analyze", cmd_ged_info_wrapper, ged_exec,NULL},
    {"annotate", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"arb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"arced", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"area", f_area, GED_FUNC_PTR_NULL,NULL},
    {"arot", cmd_arot, GED_FUNC_PTR_NULL,NULL},
    {"art", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"attach", f_attach, GED_FUNC_PTR_NULL,NULL},
    {"attr", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"autoview", cmd_autoview, GED_FUNC_PTR_NULL,NULL},
    {"bb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bev", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bo", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bomb", f_bomb, GED_FUNC_PTR_NULL,NULL},
    {"bot", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_condense", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_decimate", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_dump", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_exterior", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_face_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_face_sort", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_flip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_merge", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_smooth", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_split", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_sync", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bot_vertex_fuse", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"bottom", f_bv_bottom, GED_FUNC_PTR_NULL,NULL},
    {"brep", cmd_ged_view_wrapper, ged_exec,NULL},
    {"c", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"cat", cmd_ged_info_wrapper, ged_exec,NULL},
    {"cc", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"center", cmd_center, GED_FUNC_PTR_NULL,NULL},
    {"check", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"clone", cmd_ged_edit_wrapper, ged_exec,NULL},
    {"closedb", f_closedb, GED_FUNC_PTR_NULL,NULL},
    {"cmd_win", cmd_cmd_win, GED_FUNC_PTR_NULL,NULL},
    {"coil", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"comb_color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"constraint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"copyeval", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"copymat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"cp", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"cpi", f_copy_inv, GED_FUNC_PTR_NULL,NULL},
    {"d", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"db", cmd_stub, GED_FUNC_PTR_NULL,NULL},
    {"db_glob", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dbconcat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dbfind", cmd_ged_info_wrapper, ged_exec,NULL},
    {"dbip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dbversion", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"debug", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"debugbu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"debugdir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"debuglib", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"debugnmg", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"decompose", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"delay", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dir2ae", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dump", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dm", f_dm, GED_FUNC_PTR_NULL,NULL},
    {"draw", cmd_draw, GED_FUNC_PTR_NULL,NULL},
    {"dsp", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"dup", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"E", cmd_E, GED_FUNC_PTR_NULL,NULL},
    {"e", cmd_draw, GED_FUNC_PTR_NULL,NULL},
    {"eac", cmd_ged_view_wrapper, ged_exec,NULL},
    {"echo", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"edcodes", f_edcodes, GED_FUNC_PTR_NULL,NULL},
    {"edit", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"color", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"edcolor", f_edcolor, GED_FUNC_PTR_NULL,NULL},
    {"edcomb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"edgedir", f_edgedir, GED_FUNC_PTR_NULL,NULL},
    {"edmater", f_edmater, GED_FUNC_PTR_NULL,NULL},
    {"env", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"erase", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"ev", cmd_ev, GED_FUNC_PTR_NULL,NULL},
    {"eqn", f_eqn, GED_FUNC_PTR_NULL,NULL},
    {"exit", f_quit, GED_FUNC_PTR_NULL,NULL},
    {"expand", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"extrude", f_extrude, GED_FUNC_PTR_NULL,NULL},
    {"eye_pt", cmd_ged_view_wrapper, ged_exec,NULL},
    {"exists", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"facedef", f_facedef, GED_FUNC_PTR_NULL,NULL},
    {"facetize", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"facetize_old", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"form", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"fracture", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"front", f_bv_front, GED_FUNC_PTR_NULL,NULL},
    {"g", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"gdiff", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"garbage_collect", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get_type", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get_autoview", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get_comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get_dbip", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"get_dm_list", f_get_dm_list, GED_FUNC_PTR_NULL,NULL},
    {"get_more_default", cmd_get_more_default, GED_FUNC_PTR_NULL,NULL},
    {"get_sed", f_get_sedit, GED_FUNC_PTR_NULL,NULL},
    {"get_sed_menus", f_get_sedit_menus, GED_FUNC_PTR_NULL,NULL},
    {"get_solid_keypoint", f_get_solid_keypoint, GED_FUNC_PTR_NULL,NULL},
    {"graph", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"gqa", cmd_ged_gqa, ged_exec,NULL},
    {"grid2model_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"grid2view_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"has_embedded_fb", cmd_has_embedded_fb, GED_FUNC_PTR_NULL,NULL},
    {"heal", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"hide", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"hist", cmd_hist, GED_FUNC_PTR_NULL,NULL},
    {"history", f_history, GED_FUNC_PTR_NULL,NULL},
    {"i", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"idents", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"ill", f_ill, GED_FUNC_PTR_NULL,NULL},
    {"in", cmd_ged_in, ged_exec,NULL},
    {"inside", cmd_ged_inside, ged_exec,NULL},
    {"item", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"joint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"joint2", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"journal", f_journal, GED_FUNC_PTR_NULL,NULL},
    {"keep", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"keypoint", f_keypoint, GED_FUNC_PTR_NULL,NULL},
    {"kill", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"killall", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"killrefs", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"killtree", cmd_ged_erase_wrapper, ged_exec,NULL},
    {"knob", f_knob, GED_FUNC_PTR_NULL,NULL},
    {"l", cmd_ged_info_wrapper, ged_exec,NULL},
    {"labelvert", f_labelvert, GED_FUNC_PTR_NULL,NULL},
    {"labelface", f_labelface, GED_FUNC_PTR_NULL,NULL},
    {"lc", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"left", f_bv_left, GED_FUNC_PTR_NULL,NULL},
    {"lint", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"listeval", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"loadtk", cmd_tk, GED_FUNC_PTR_NULL,NULL},
    {"loadview", cmd_ged_view_wrapper, ged_exec,NULL},
    {"lod", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"lookat", cmd_ged_view_wrapper, ged_exec,NULL},
    {"ls", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"lt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"M", f_mouse, GED_FUNC_PTR_NULL,NULL},
    {"m2v_point", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"make", f_make, GED_FUNC_PTR_NULL,NULL},
    {"make_name", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"make_pnts", cmd_ged_more_wrapper, ged_exec,NULL},
    {"match", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"material", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"matpick", f_matpick, GED_FUNC_PTR_NULL,NULL},
    {"mat_ae", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mat_mul", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mat4x3pnt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mat_scale_about_pnt", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mged_update", f_update, GED_FUNC_PTR_NULL,NULL},
    {"mged_wait", f_wait, GED_FUNC_PTR_NULL,NULL},
    {"mirface", f_mirface, GED_FUNC_PTR_NULL,NULL},
    {"mirror", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mmenu_get", cmd_mmenu_get, GED_FUNC_PTR_NULL,NULL},
    {"mmenu_set", cmd_nop, GED_FUNC_PTR_NULL,NULL},
    {"model2grid_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"model2view", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"model2view_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mrot", cmd_mrot, GED_FUNC_PTR_NULL,NULL},
    {"mv", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"mvall", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"nirt", f_nirt, GED_FUNC_PTR_NULL,NULL},
    {"nmg_collapse", cmd_nmg_collapse, GED_FUNC_PTR_NULL,NULL},
    {"nmg_fix_normals", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"nmg_simplify", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"nmg", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"npush", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"o_rotate", f_be_o_rotate, GED_FUNC_PTR_NULL,NULL},
    {"o_scale", f_be_o_scale, GED_FUNC_PTR_NULL,NULL},
    {"oed", cmd_oed, GED_FUNC_PTR_NULL,NULL},
    {"oed_apply", f_oedit_apply, GED_FUNC_PTR_NULL,NULL},
    {"oed_reset", f_oedit_reset, GED_FUNC_PTR_NULL,NULL},
    {"oill", f_be_o_illuminate, GED_FUNC_PTR_NULL,NULL},
    {"opendb", f_opendb, GED_FUNC_PTR_NULL,NULL},
    {"orientation", cmd_ged_view_wrapper, ged_exec,NULL},
    {"orot", f_rot_obj, GED_FUNC_PTR_NULL,NULL},
    {"oscale", f_sc_obj, GED_FUNC_PTR_NULL,NULL},
    {"output_hook", cmd_output_hook, GED_FUNC_PTR_NULL,NULL},
    {"overlay", cmd_overlay, GED_FUNC_PTR_NULL,NULL},
    {"ox", f_be_o_x, GED_FUNC_PTR_NULL,NULL},
    {"oxscale", f_be_o_xscale, GED_FUNC_PTR_NULL,NULL},
    {"oxy", f_be_o_xy, GED_FUNC_PTR_NULL,NULL},
    {"oy", f_be_o_y, GED_FUNC_PTR_NULL,NULL},
    {"oyscale", f_be_o_yscale, GED_FUNC_PTR_NULL,NULL},
    {"ozscale", f_be_o_zscale, GED_FUNC_PTR_NULL,NULL},
    {"p", f_param, GED_FUNC_PTR_NULL,NULL},
    {"pathlist", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"paths", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"permute", f_permute, GED_FUNC_PTR_NULL,NULL},
    {"plot", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"png", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"pnts", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"prcolor", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"prefix", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"press", f_press, GED_FUNC_PTR_NULL,NULL},
    {"preview", cmd_ged_dm_wrapper, ged_exec,NULL},
    {"process", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"postscript", f_postscript, GED_FUNC_PTR_NULL,NULL},
    {"ps", cmd_ps, GED_FUNC_PTR_NULL,NULL},
    {"pull", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"push", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"put", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"put_comb", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"put_sed", f_put_sedit, GED_FUNC_PTR_NULL,NULL},
    {"putmat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"q", f_quit, GED_FUNC_PTR_NULL,NULL},
    {"qorot", f_qorot, GED_FUNC_PTR_NULL,NULL},
    {"qray", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"query_ray", f_nirt, GED_FUNC_PTR_NULL,NULL},
    {"quit", f_quit, GED_FUNC_PTR_NULL,NULL},
    {"qvrot", cmd_ged_view_wrapper, ged_exec,NULL},
    {"r", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"rate", f_bv_rate_toggle, GED_FUNC_PTR_NULL,NULL},
    {"rcodes", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"rear", f_bv_rear, GED_FUNC_PTR_NULL,NULL},
    {"red", f_red, GED_FUNC_PTR_NULL,NULL},
    {"refresh", f_refresh, GED_FUNC_PTR_NULL,NULL},
    {"regdebug", f_regdebug, GED_FUNC_PTR_NULL,NULL},
    {"regdef", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"regions", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"reject", f_be_reject, GED_FUNC_PTR_NULL,NULL},
    {"release", f_release, GED_FUNC_PTR_NULL,NULL},
    {"reset", f_bv_reset, GED_FUNC_PTR_NULL,NULL},
    {"restore", f_bv_vrestore, GED_FUNC_PTR_NULL,NULL},
    {"rfarb", f_rfarb, GED_FUNC_PTR_NULL,NULL},
    {"right", f_bv_right, GED_FUNC_PTR_NULL,NULL},
    {"rm", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"rmater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"rmats", f_rmats, GED_FUNC_PTR_NULL,NULL},
    {"rot", cmd_rot, GED_FUNC_PTR_NULL,NULL},
    {"rotobj", f_rot_obj, GED_FUNC_PTR_NULL,NULL},
    {"rrt", cmd_rrt, GED_FUNC_PTR_NULL,NULL},
    {"rset", f_rset, GED_FUNC_PTR_NULL,NULL},
    {"rt", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"rt_gettrees", cmd_rt_gettrees, GED_FUNC_PTR_NULL,NULL},
    {"rtabort", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"rtarea", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"rtcheck", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"rtedge", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"rtweight", cmd_rt, GED_FUNC_PTR_NULL,NULL},
    {"save", f_bv_vsave, GED_FUNC_PTR_NULL,NULL},
    {"savekey", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"saveview", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"sca", cmd_sca, GED_FUNC_PTR_NULL,NULL},
    {"screengrab", cmd_ged_dm_wrapper, ged_exec,NULL},
    {"search", cmd_search, GED_FUNC_PTR_NULL,NULL},
    {"sed", f_sed, GED_FUNC_PTR_NULL,NULL},
    {"sed_apply", f_sedit_apply, GED_FUNC_PTR_NULL,NULL},
    {"sed_reset", f_sedit_reset, GED_FUNC_PTR_NULL,NULL},
    {"sedit", f_be_s_edit, GED_FUNC_PTR_NULL,NULL},
    {"select", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"set_more_default", cmd_set_more_default, GED_FUNC_PTR_NULL,NULL},
    {"setview", cmd_setview, GED_FUNC_PTR_NULL,NULL},
    {"shaded_mode", cmd_shaded_mode, GED_FUNC_PTR_NULL,NULL},
    {"shader", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"share", f_share, GED_FUNC_PTR_NULL,NULL},
    {"shells", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"showmats", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"sill", f_be_s_illuminate, GED_FUNC_PTR_NULL,NULL},
    {"size", cmd_size, GED_FUNC_PTR_NULL,NULL},
    {"simulate", cmd_ged_simulate_wrapper, ged_exec,NULL},
    {"solid_report", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"solids", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"solids_on_ray", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"srot", f_be_s_rotate, GED_FUNC_PTR_NULL,NULL},
    {"sscale", f_be_s_scale, GED_FUNC_PTR_NULL,NULL},
    {"stat", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"status", f_status, GED_FUNC_PTR_NULL,NULL},
    {"stuff_str", cmd_stuff_str, GED_FUNC_PTR_NULL,NULL},
    {"summary", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"sv", f_slewview, GED_FUNC_PTR_NULL,NULL},
    {"svb", f_svbase, GED_FUNC_PTR_NULL,NULL},
    {"sxy", f_be_s_trans, GED_FUNC_PTR_NULL,NULL},
    {"sync", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"t", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"ted", f_tedit, GED_FUNC_PTR_NULL,NULL},
    {"tie", f_tie, GED_FUNC_PTR_NULL,NULL},
    {"tire", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"title", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"tol", cmd_tol, GED_FUNC_PTR_NULL,NULL},
    {"top", f_bv_top, GED_FUNC_PTR_NULL,NULL},
    {"tops", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"tra", cmd_tra, GED_FUNC_PTR_NULL,NULL},
    {"track", f_amtrack, GED_FUNC_PTR_NULL,NULL},
    {"tracker", f_tracker, GED_FUNC_PTR_NULL,NULL},
    {"translate", f_tr_obj, GED_FUNC_PTR_NULL,NULL},
    {"tree", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"unhide", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"units", cmd_units, GED_FUNC_PTR_NULL,NULL},
    {"v2m_point", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"vars", f_set, GED_FUNC_PTR_NULL,NULL},
    {"vdraw", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"view", cmd_ged_view_wrapper, ged_exec,NULL},
    {"view_ring", f_view_ring, GED_FUNC_PTR_NULL,NULL},
    {"view2grid_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"view2model", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"view2model_lu", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"view2model_vec", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"viewdir", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"viewsize", cmd_size, GED_FUNC_PTR_NULL,NULL}, /* alias "size" for saveview scripts */
    {"vnirt", f_vnirt, GED_FUNC_PTR_NULL,NULL},
    {"voxelize", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"vquery_ray", f_vnirt, GED_FUNC_PTR_NULL,NULL},
    {"vrot", cmd_vrot, GED_FUNC_PTR_NULL,NULL},
    {"wcodes", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"whatid", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"which_shader", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"whichair", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"whichid", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"who", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"winset", f_winset, GED_FUNC_PTR_NULL,NULL},
    {"wmater", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"x", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"xpush", cmd_ged_plain_wrapper, ged_exec,NULL},
    {"Z", cmd_zap, GED_FUNC_PTR_NULL,NULL},
    {"zoom", cmd_zoom, GED_FUNC_PTR_NULL,NULL},
    {"zoomin", f_bv_zoomin, GED_FUNC_PTR_NULL,NULL},
    {"zoomout", f_bv_zoomout, GED_FUNC_PTR_NULL,NULL},
    {NULL, NULL, GED_FUNC_PTR_NULL, NULL}
};


/**
 * Register all MGED commands.
 */
static void
cmd_setup(void)
{
    struct cmdtab *ctp;
    struct bu_vls temp = BU_VLS_INIT_ZERO;

    for (ctp = mged_cmdtab; ctp->name != NULL; ctp++) {
	ctp->mged_state = MGED_STATE;
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

    BU_GET(GEDP, struct ged);
    GED_INIT(GEDP, NULL);
    GEDP->ged_output_handler = mged_output_handler;
    GEDP->ged_refresh_handler = mged_refresh_handler;
    GEDP->ged_create_vlist_scene_obj_callback = createDListSolid;
    GEDP->ged_create_vlist_display_list_callback = createDListAll;
    GEDP->ged_destroy_vlist_callback = freeDListsAll;
    GEDP->ged_create_io_handler = &tclcad_create_io_handler;
    GEDP->ged_delete_io_handler = &tclcad_delete_io_handler;
    GEDP->ged_pre_opendb_callback = &mged_pre_opendb_clbk;
    GEDP->ged_post_opendb_callback = &mged_post_opendb_clbk;
    GEDP->ged_pre_closedb_callback = &mged_pre_closedb_clbk;
    GEDP->ged_post_closedb_callback = &mged_post_closedb_clbk;
    GEDP->ged_db_callback_udata = &mged_global_db_ctx;
    GEDP->ged_interp = (void *)interpreter;
    GEDP->ged_interp_eval = &mged_db_search_callback;
    struct tclcad_io_data *t_iod = tclcad_create_io_data();
    t_iod->io_mode = TCL_READABLE;
    t_iod->interp = *interpreter;
    GEDP->ged_io_data = t_iod;

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

    view_state->vs_gvp->vset = &GEDP->ged_views;

    bv_set_add_view(&GEDP->ged_views, view_state->vs_gvp);
    bu_ptbl_ins(&GEDP->ged_free_views, (long *)view_state->vs_gvp);
    GEDP->ged_gvp = view_state->vs_gvp;

    /* register commands */
    cmd_setup();

    history_setup();
    mged_global_variable_setup(*interpreter);
    mged_variable_setup(*interpreter);
    GEDP->ged_interp = (void *)*interpreter;
    GEDP->ged_interp_eval = &mged_db_search_callback;

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
