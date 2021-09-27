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
#include "Chroma.h"

FD_Chroma *create_form_Chroma(void)
{
  FL_OBJECT *obj;
  FD_Chroma *fdui = (FD_Chroma *) fl_calloc(1, sizeof(*fdui));

  fdui->Chroma = fl_bgn_form(FL_NO_BOX, 1260, 980);
  obj = fl_add_box(FL_UP_BOX,0,0,1260,980,"");
  fdui->Active = obj = fl_add_button(FL_PUSH_BUTTON,130,940,50,30,"Active");
  fdui->Orig_out = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,20,15,600,430,"");
  fdui->Value_out = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,640,15,600,430,"");
  fdui->Alpha_out = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,20,455,600,430,"");
  fdui->Result_out = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,640,455,600,430,"");
  fdui->Index = obj = fl_add_valslider(FL_HOR_SLIDER,20,910,160,20,"Index");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 10);
    fl_set_slider_value(obj, 1);

  fdui->AlphaSetting = fl_bgn_group();
  fdui->Alpha0 = obj = fl_add_button(FL_RADIO_BUTTON,20,940,40,30,"0");
  fdui->Alpha1 = obj = fl_add_button(FL_RADIO_BUTTON,70,940,40,30,"1");
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->Cutoff = obj = fl_add_slider(FL_HOR_SLIDER,440,800,160,20,"Cutoff");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 255);
    fl_set_slider_value(obj, 127);
  fdui->Soft = obj = fl_add_slider(FL_HOR_SLIDER,440,840,160,20,"Soft");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 1);
    fl_set_slider_bounds(obj, 0.5, 128.0);
  fdui->Color1 = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,190,910,30,60,"");

  fdui->Filttype = fl_bgn_group();
  fdui->Fminus = obj = fl_add_button(FL_RADIO_BUTTON,715,950,30,20,"-");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_button(obj, 1);
  fdui->Fnormal = obj = fl_add_button(FL_RADIO_BUTTON,760,950,29,20,".");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
  fdui->Fplus = obj = fl_add_button(FL_RADIO_BUTTON,805,950,30,20,"+");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,710,890,90,20,"Filter Count");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,710,930,90,20,"Filter Type");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  fdui->Render = obj = fl_add_button(FL_NORMAL_BUTTON,855,940,50,30,"Render");
    fl_set_button_shortcut(obj,"Rr&12",1);
  fdui->Stop = obj = fl_add_button(FL_NORMAL_BUTTON,915,940,50,30,"Stop");
    fl_set_button_shortcut(obj,"Ss^C",1);
  obj = fl_add_text(FL_NORMAL_TEXT,575,930,80,20,"Crop");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  fdui->Hue = obj = fl_add_slider(FL_HOR_SLIDER,290,910,160,20,"Hue  (Blue)");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 128, 0);
    fl_set_slider_value(obj, 64);
  fdui->Sat = obj = fl_add_slider(FL_HOR_SLIDER,290,950,160,20,"Saturation  (Red)");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 256, 0);
    fl_set_slider_value(obj, 128);
  fdui->SoftHue = obj = fl_add_slider(FL_HOR_SLIDER,460,910,70,20,"Soft");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 30);
    fl_set_slider_value(obj, 15);
  fdui->SoftSat = obj = fl_add_slider(FL_HOR_SLIDER,460,950,70,20,"");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 30);
    fl_set_slider_value(obj, 15);
  fdui->HueOff = obj = fl_add_button(FL_PUSH_BUTTON,230,950,50,20,"Single");
  fdui->SatOff = obj = fl_add_button(FL_PUSH_BUTTON,230,910,50,20,"Single");
  fdui->MovieFrame = obj = fl_add_slider(FL_HOR_SLIDER,855,910,170,20,"Movie Frame");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_value(obj, 0.00);
  fdui->Load = obj = fl_add_button(FL_NORMAL_BUTTON,975,940,50,30,"New");
    fl_set_button_shortcut(obj,"Nn&1",1);

  fdui->Filtcount = fl_bgn_group();
  obj = fl_add_button(FL_RADIO_BUTTON,755,910,20,20,"2");
    fl_set_object_callback(obj,set_filter,2);
  obj = fl_add_button(FL_RADIO_BUTTON,735,910,20,20,"1");
    fl_set_object_callback(obj,set_filter,1);
  obj = fl_add_button(FL_RADIO_BUTTON,775,910,20,20,"3");
    fl_set_object_callback(obj,set_filter,3);
  obj = fl_add_button(FL_RADIO_BUTTON,794,910,21,20,"4");
    fl_set_object_callback(obj,set_filter,4);
  obj = fl_add_button(FL_RADIO_BUTTON,815,910,20,20,"5");
    fl_set_object_callback(obj,set_filter,5);
  obj = fl_add_button(FL_RADIO_BUTTON,715,910,20,20,"0");
    fl_set_object_callback(obj,set_filter,0);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->SubUV = obj = fl_add_button(FL_PUSH_BUTTON,1205,910,35,20,"Lum");
  obj = fl_add_text(FL_NORMAL_TEXT,1160,890,80,20,"Sub color");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,1045,920,80,20,"Background");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);

  fdui->Output = fl_bgn_group();
  fdui->Image = obj = fl_add_button(FL_RADIO_BUTTON,1100,940,40,30,"Image");
    fl_set_button_shortcut(obj,"Ii",1);
    fl_set_object_callback(obj,set_result,1);
    fl_set_button(obj, 1);
  fdui->Black = obj = fl_add_button(FL_RADIO_BUTTON,1050,940,40,30,"Black");
    fl_set_button_shortcut(obj,"Bb",1);
    fl_set_object_callback(obj,set_result,0);
  fdui->White = obj = fl_add_button(FL_RADIO_BUTTON,1150,940,40,30,"White");
    fl_set_button_shortcut(obj,"Ww",1);
    fl_set_object_callback(obj,set_result,2);
  fdui->Alpha = obj = fl_add_button(FL_RADIO_BUTTON,1200,940,40,30,"Alpha");
    fl_set_button_shortcut(obj,"Aa",1);
    fl_set_object_callback(obj,set_result,3);
  fl_end_group();


  fdui->Crop = fl_bgn_group();
  obj = fl_add_button(FL_RADIO_BUTTON,600,950,20,20,"1");
    fl_set_object_callback(obj,set_crop,1);
    fl_set_button(obj, 1);
  obj = fl_add_button(FL_RADIO_BUTTON,580,950,20,20,"0");
    fl_set_object_callback(obj,set_crop,0);
  obj = fl_add_button(FL_RADIO_BUTTON,620,950,20,20,"2");
    fl_set_object_callback(obj,set_crop,2);
  fl_end_group();

  fdui->SubKey = obj = fl_add_slider(FL_HOR_SLIDER,580,910,100,20,"Sub Key Color");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_slider_precision(obj, 1);
  fl_end_form();

  fdui->Chroma->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


