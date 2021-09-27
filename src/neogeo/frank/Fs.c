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
#include "Fs.h"

FD_Fs *create_form_Fs(void)
{
  FL_OBJECT *obj;
  FD_Fs *fdui = (FD_Fs *) fl_calloc(1, sizeof(*fdui));

  fdui->Fs = fl_bgn_form(FL_NO_BOX, 470, 460);
  obj = fl_add_box(FL_UP_BOX,0,0,470,460,"");
  obj = fl_add_browser(FL_MULTI_BROWSER,10,90,440,350,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,a,0);
  obj = fl_add_input(FL_NORMAL_INPUT,70,10,380,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,b,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,10,10,50,30,"Up");
    fl_set_button_shortcut(obj,"^U",1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,b,0);
  obj = fl_add_button(FL_MENU_BUTTON,10,50,50,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,b,0);
  obj = fl_add_input(FL_HIDDEN_INPUT,70,40,380,10,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,b,0);
  obj = fl_add_input(FL_NORMAL_INPUT,70,50,380,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,b,0);
  obj = fl_add_input(FL_HIDDEN_INPUT,70,80,380,10,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,b,0);
  fl_end_form();

  return fdui;
}
/*---------------------------------------*/


