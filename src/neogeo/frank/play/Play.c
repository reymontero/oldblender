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

/* Form definition file generated with fdesign. */


#include "forms.h"
#include <stdlib.h>
#include "Play.h"

FD_Play *create_form_Play(void)
{
  FL_OBJECT *obj;
  FD_Play *fdui = (FD_Play *) fl_calloc(1, sizeof(*fdui));

  fdui->Play = fl_bgn_form(FL_NO_BOX, 1010, 720);
  obj = fl_add_box(FL_UP_BOX,0,0,1010,720,"");
  fdui->video_canvas = obj = fl_add_canvas(FL_NORMAL_CANVAS,30,20,720,575,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,canvas_callback,0);
  fdui->MGoto_in = obj = fl_add_button(FL_TOUCH_BUTTON,350,640,30,30,"|<");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_pos,1);
  fdui->MGoto_out = obj = fl_add_button(FL_TOUCH_BUTTON,430,640,30,30,">|");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_pos,3);
  fdui->MLoop = obj = fl_add_button(FL_TOUCH_BUTTON,390,640,30,30,"|<>|");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_pos,2);
  fdui->MPos = obj = fl_add_slider(FL_HOR_FILL_SLIDER,30,610,720,25,"");
    fl_set_object_color(obj,FL_COL1,FL_SLATEBLUE);
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_RIGHT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_pos,0);
  fdui->MSpeed_input = obj = fl_add_input(FL_FLOAT_INPUT,270,640,55,30,"Speed");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,4);
  fdui->MFrame = obj = fl_add_input(FL_FLOAT_INPUT,270,680,55,30,"Frame");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_pos,4);

  fdui->movie_speed = fl_bgn_group();
  fdui->MSpeed_slider = obj = fl_add_slider(FL_HOR_SLIDER,30,680,190,30,"");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,3);
  fdui->MFReverse = obj = fl_add_button(FL_RADIO_BUTTON,30,640,30,30,"<<");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,-2);
  fdui->MForward = obj = fl_add_button(FL_RADIO_BUTTON,150,640,30,30,">");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,1);
  fdui->MFForward = obj = fl_add_button(FL_RADIO_BUTTON,190,640,30,30,">>");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,2);
  fdui->MReverse = obj = fl_add_button(FL_RADIO_BUTTON,70,640,30,30,"<");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,-1);
  fdui->MStop = obj = fl_add_button(FL_RADIO_BUTTON,110,640,30,30,"||");
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,movie_speed,0);
  fl_end_group();


  fdui->video_speed = fl_bgn_group();
  fdui->VSpeed_slider = obj = fl_add_slider(FL_HOR_SLIDER,540,560,190,20,"");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,999);
  fdui->VFReverse = obj = fl_add_button(FL_RADIO_BUTTON,540,520,30,30,"<<");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,-5);
  fdui->VForward = obj = fl_add_button(FL_RADIO_BUTTON,660,520,30,30,">");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,1);
  fdui->VFForward = obj = fl_add_button(FL_RADIO_BUTTON,700,520,30,30,">>");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,5);
  fdui->VReverse = obj = fl_add_button(FL_RADIO_BUTTON,580,520,30,30,"<");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,-1);
  fdui->VStop = obj = fl_add_button(FL_RADIO_BUTTON,620,520,30,30,"||");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_speed,0);
  fl_end_group();

  fdui->TimeCode = obj = fl_add_input(FL_NORMAL_INPUT,540,390,90,30,"TimeCode");
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,timecode,0);
  fdui->VSet_In = obj = fl_add_input(FL_NORMAL_INPUT,540,480,90,30,"In");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,set_in_out,'i');
  fdui->VSet_Out = obj = fl_add_input(FL_NORMAL_INPUT,640,480,90,30,"Out");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_RIGHT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,set_in_out,'o');
  fdui->VGoto_in = obj = fl_add_button(FL_NORMAL_BUTTON,540,430,30,30,"|<");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_pos,0);
  fdui->VGoto_out = obj = fl_add_button(FL_NORMAL_BUTTON,700,430,30,30,">|");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_pos,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,430,90,30,"Initialise");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,video_init,1);
  fdui->MShots = obj = fl_add_browser(FL_HOLD_BROWSER,780,20,210,290,"");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);

  fdui->controls = fl_bgn_group();
  fdui->Video_Controls = obj = fl_add_checkbutton(FL_RADIO_BUTTON,730,680,25,25,"Video");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,add_controls,'v');
  fdui->Edit_Controls = obj = fl_add_checkbutton(FL_RADIO_BUTTON,730,660,25,25,"Edit");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,add_controls,'e');
  fdui->No_Controls = obj = fl_add_checkbutton(FL_RADIO_BUTTON,730,640,25,25,"None");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,add_controls,'n');
  fl_end_group();

  fdui->Image_frame = obj = fl_add_text(FL_NORMAL_TEXT,880,320,110,20,"1");
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->Sound_frame = obj = fl_add_text(FL_NORMAL_TEXT,880,370,110,20,"1");
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->Sound_movie = obj = fl_add_input(FL_NORMAL_INPUT,780,390,210,30,"Sound Movie");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,nop,0);
  fdui->Image_movie = obj = fl_add_input(FL_NORMAL_INPUT,780,340,210,30,"Image Movie");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,nop,0);
  fdui->Auto_reload = obj = fl_add_checkbutton(FL_PUSH_BUTTON,775,680,25,25,"Auto Reload");
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Buffer_Size = obj = fl_add_input(FL_INT_INPUT,780,650,45,30,"Size");
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,buffer_size,0);
  fdui->Buffer = obj = fl_add_slider(FL_HOR_FILL_SLIDER,830,650,160,30,"Buffer");
    fl_set_object_color(obj,FL_COL1,FL_SLATEBLUE);
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,nop,0);
  fdui->Render = obj = fl_add_button(FL_NORMAL_BUTTON,940,440,50,30,"Render");
    fl_set_button_shortcut(obj,"Rr&12",1);
    fl_set_object_callback(obj,render,0);
  fdui->StopRender = obj = fl_add_button(FL_NORMAL_BUTTON,940,480,50,30,"Stop");
    fl_set_button_shortcut(obj,"Ss^C",1);
    fl_set_object_callback(obj,stop_render,0);
  fl_end_form();

  fdui->Play->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


