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
#include "Render.h"

FD_Render *create_form_Render(void)
{
  FL_OBJECT *obj;
  FD_Render *fdui = (FD_Render *) fl_calloc(1, sizeof(*fdui));

  fdui->Render = fl_bgn_form(FL_NO_BOX, 460, 280);
  obj = fl_add_box(FL_UP_BOX,0,0,460,280,"");
  fdui->Done_black = obj = fl_add_box(FL_BORDER_BOX,324,114,122,12,"Done");
    fl_set_object_color(obj,FL_BLACK,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Done_border = obj = fl_add_frame(FL_DOWN_FRAME,324,114,122,12,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Wait = obj = fl_add_checkbutton(FL_PUSH_BUTTON,190,200,25,25,"Wait");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Status_cb,0);
  fdui->Animations = obj = fl_add_browser(FL_MULTI_BROWSER,10,30,160,100,"Animations");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_East);
    fl_set_object_callback(obj,File_browse,0);
  fdui->Systems = obj = fl_add_browser(FL_MULTI_BROWSER,10,160,160,100,"Systems");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_West, FL_SouthEast);
    fl_set_object_callback(obj,Sys_browse,0);

  fdui->Priority = fl_bgn_group();
  obj = fl_add_text(FL_NORMAL_TEXT,190,10,70,20,"Priority");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Pri_Max = obj = fl_add_checkbutton(FL_RADIO_BUTTON,190,30,25,25,"Max");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Priority,40);
  fdui->Pri_High = obj = fl_add_checkbutton(FL_RADIO_BUTTON,190,50,25,25,"High");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Priority,30);
  fdui->Pri_Def = obj = fl_add_checkbutton(FL_RADIO_BUTTON,190,70,25,25,"Normal");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Priority,20);
  fdui->Pri_Low = obj = fl_add_checkbutton(FL_RADIO_BUTTON,190,90,25,25,"Low");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Priority,10);
  fdui->Pri_Min = obj = fl_add_checkbutton(FL_RADIO_BUTTON,190,110,25,25,"Stopped");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Priority,0);
  fl_end_group();

  fdui->Return = obj = fl_add_checkbutton(FL_TOUCH_BUTTON,190,240,25,25,"Return");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Status_cb,2);
  fdui->Add = obj = fl_add_button(FL_NORMAL_BUTTON,320,30,60,30,"Add");
    fl_set_button_shortcut(obj,"Aa",1);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Add_file,0);

  fdui->Load = fl_bgn_group();
  obj = fl_add_text(FL_NORMAL_TEXT,260,10,70,20,"Load");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Load_64 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,30,25,25,"64");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Load,64);
  fdui->Load_48 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,50,25,25,"48");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Load,48);
  fdui->Load_32 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,70,25,25,"32");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Load,32);
  fdui->Load_16 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,90,25,25,"16");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Load,16);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,320,10,70,20,"Files");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Remove = obj = fl_add_button(FL_NORMAL_BUTTON,390,30,60,30,"Remove");
    fl_set_button_shortcut(obj,"Rr",1);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Rem_file,0);
  fdui->Memory = obj = fl_add_slider(FL_HOR_FILL_SLIDER,320,200,130,20,"Memory");
    fl_set_object_color(obj,FL_BLACK,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Do_nothing,0);
  fdui->Swap = obj = fl_add_slider(FL_HOR_FILL_SLIDER,320,240,130,20,"Swap");
    fl_set_object_color(obj,FL_BLACK,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Do_nothing,0);

  fdui->Status_grp = fl_bgn_group();
  obj = fl_add_text(FL_NORMAL_TEXT,190,140,70,20,"Status");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Busy = obj = fl_add_checkbutton(FL_PUSH_BUTTON,190,180,25,25,"Busy");
    fl_set_object_color(obj,FL_COL1,FL_GREEN);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Do_nothing,0);
  fdui->Finish = obj = fl_add_checkbutton(FL_PUSH_BUTTON,190,220,25,25,"Finish");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Status_cb,1);
  fl_end_group();


  fdui->Accept = fl_bgn_group();
  obj = fl_add_text(FL_NORMAL_TEXT,260,140,60,20,"Accept");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Accept_64 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,160,25,25,"64");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Accept,64);
  fdui->Accept_32 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,200,25,25,"32");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Accept,32);
  fdui->Accept_16 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,220,25,25,"16");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Accept,16);
  fdui->Accept_48 = obj = fl_add_checkbutton(FL_RADIO_BUTTON,260,180,25,25,"48");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Accept,48);
  fl_end_group();

  fdui->CPU = obj = fl_add_slider(FL_HOR_FILL_SLIDER,320,161,130,20,"CPU");
    fl_set_object_color(obj,FL_BLACK,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Do_nothing,0);
  fdui->Offline = obj = fl_add_checkbutton(FL_PUSH_BUTTON,190,160,25,25,"Offline");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Do_nothing,0);
  fdui->Done_green = obj = fl_add_box(FL_BORDER_BOX,324,114,100,12,"");
    fl_set_object_color(obj,FL_GREEN,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Done_red = obj = fl_add_box(FL_BORDER_BOX,324,114,92,12,"");
    fl_set_object_color(obj,FL_RED,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Done_yellow = obj = fl_add_box(FL_BORDER_BOX,324,114,86,12,"");
    fl_set_object_color(obj,FL_YELLOW,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Retry = obj = fl_add_button(FL_NORMAL_BUTTON,390,64,60,30,"Retry");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Retry_file,0);
  fdui->Beep = obj = fl_add_checkbutton(FL_PUSH_BUTTON,260,109,24,26,"Beep");
    fl_set_object_gravity(obj, FL_East, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,Beep_file,0);
  fl_end_form();

  fdui->Render->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


