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

#ifndef FD_Play_h_

#define FD_Play_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void canvas_callback(FL_OBJECT *, long);
extern void movie_pos(FL_OBJECT *, long);
extern void movie_speed(FL_OBJECT *, long);
extern void video_speed(FL_OBJECT *, long);
extern void timecode(FL_OBJECT *, long);
extern void set_in_out(FL_OBJECT *, long);
extern void video_pos(FL_OBJECT *, long);
extern void video_init(FL_OBJECT *, long);
extern void add_controls(FL_OBJECT *, long);
extern void nop(FL_OBJECT *, long);
extern void buffer_size(FL_OBJECT *, long);
extern void render(FL_OBJECT *, long);
extern void stop_render(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *Play;
	void *vdata;
	long ldata;
	FL_OBJECT *video_canvas;
	FL_OBJECT *MGoto_in;
	FL_OBJECT *MGoto_out;
	FL_OBJECT *MLoop;
	FL_OBJECT *MPos;
	FL_OBJECT *MSpeed_input;
	FL_OBJECT *MFrame;
	FL_OBJECT *movie_speed;
	FL_OBJECT *MSpeed_slider;
	FL_OBJECT *MFReverse;
	FL_OBJECT *MForward;
	FL_OBJECT *MFForward;
	FL_OBJECT *MReverse;
	FL_OBJECT *MStop;
	FL_OBJECT *video_speed;
	FL_OBJECT *VSpeed_slider;
	FL_OBJECT *VFReverse;
	FL_OBJECT *VForward;
	FL_OBJECT *VFForward;
	FL_OBJECT *VReverse;
	FL_OBJECT *VStop;
	FL_OBJECT *TimeCode;
	FL_OBJECT *VSet_In;
	FL_OBJECT *VSet_Out;
	FL_OBJECT *VGoto_in;
	FL_OBJECT *VGoto_out;
	FL_OBJECT *MShots;
	FL_OBJECT *controls;
	FL_OBJECT *Video_Controls;
	FL_OBJECT *Edit_Controls;
	FL_OBJECT *No_Controls;
	FL_OBJECT *Image_frame;
	FL_OBJECT *Sound_frame;
	FL_OBJECT *Sound_movie;
	FL_OBJECT *Image_movie;
	FL_OBJECT *Auto_reload;
	FL_OBJECT *Buffer_Size;
	FL_OBJECT *Buffer;
	FL_OBJECT *Render;
	FL_OBJECT *StopRender;
} FD_Play;

extern FD_Play * create_form_Play(void);

#endif /* FD_Play_h_ */

