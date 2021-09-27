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

#ifndef FD_Chroma_h_

#define FD_Chroma_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void set_filter(FL_OBJECT *, long);
extern void set_result(FL_OBJECT *, long);
extern void set_crop(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *Chroma;
	void *vdata;
	long ldata;
	FL_OBJECT *Active;
	FL_OBJECT *Orig_out;
	FL_OBJECT *Value_out;
	FL_OBJECT *Alpha_out;
	FL_OBJECT *Result_out;
	FL_OBJECT *Index;
	FL_OBJECT *AlphaSetting;
	FL_OBJECT *Alpha0;
	FL_OBJECT *Alpha1;
	FL_OBJECT *Cutoff;
	FL_OBJECT *Soft;
	FL_OBJECT *Color1;
	FL_OBJECT *Filttype;
	FL_OBJECT *Fminus;
	FL_OBJECT *Fnormal;
	FL_OBJECT *Fplus;
	FL_OBJECT *Render;
	FL_OBJECT *Stop;
	FL_OBJECT *Hue;
	FL_OBJECT *Sat;
	FL_OBJECT *SoftHue;
	FL_OBJECT *SoftSat;
	FL_OBJECT *HueOff;
	FL_OBJECT *SatOff;
	FL_OBJECT *MovieFrame;
	FL_OBJECT *Load;
	FL_OBJECT *Filtcount;
	FL_OBJECT *SubUV;
	FL_OBJECT *Output;
	FL_OBJECT *Image;
	FL_OBJECT *Black;
	FL_OBJECT *White;
	FL_OBJECT *Alpha;
	FL_OBJECT *Crop;
	FL_OBJECT *SubKey;
} FD_Chroma;

extern FD_Chroma * create_form_Chroma(void);

#endif /* FD_Chroma_h_ */

