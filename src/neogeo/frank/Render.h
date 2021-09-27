/**
 * $Id:$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 1997 by Ton Roosendaal, Frank van Beek and Joeri Kassenaar.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#ifndef FD_Render_h_

#define FD_Render_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void Status_cb(FL_OBJECT *, long);
extern void File_browse(FL_OBJECT *, long);
extern void Sys_browse(FL_OBJECT *, long);
extern void Priority(FL_OBJECT *, long);
extern void Add_file(FL_OBJECT *, long);
extern void Load(FL_OBJECT *, long);
extern void Rem_file(FL_OBJECT *, long);
extern void Do_nothing(FL_OBJECT *, long);
extern void Accept(FL_OBJECT *, long);
extern void Retry_file(FL_OBJECT *, long);
extern void Beep_file(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *Render;
	void *vdata;
	long ldata;
	FL_OBJECT *Done_black;
	FL_OBJECT *Done_border;
	FL_OBJECT *Wait;
	FL_OBJECT *Animations;
	FL_OBJECT *Systems;
	FL_OBJECT *Priority;
	FL_OBJECT *Pri_Max;
	FL_OBJECT *Pri_High;
	FL_OBJECT *Pri_Def;
	FL_OBJECT *Pri_Low;
	FL_OBJECT *Pri_Min;
	FL_OBJECT *Return;
	FL_OBJECT *Add;
	FL_OBJECT *Load;
	FL_OBJECT *Load_64;
	FL_OBJECT *Load_48;
	FL_OBJECT *Load_32;
	FL_OBJECT *Load_16;
	FL_OBJECT *Remove;
	FL_OBJECT *Memory;
	FL_OBJECT *Swap;
	FL_OBJECT *Status_grp;
	FL_OBJECT *Busy;
	FL_OBJECT *Finish;
	FL_OBJECT *Accept;
	FL_OBJECT *Accept_64;
	FL_OBJECT *Accept_32;
	FL_OBJECT *Accept_16;
	FL_OBJECT *Accept_48;
	FL_OBJECT *CPU;
	FL_OBJECT *Offline;
	FL_OBJECT *Done_green;
	FL_OBJECT *Done_red;
	FL_OBJECT *Done_yellow;
	FL_OBJECT *Retry;
	FL_OBJECT *Beep;
} FD_Render;

extern FD_Render * create_form_Render(void);

#endif /* FD_Render_h_ */

