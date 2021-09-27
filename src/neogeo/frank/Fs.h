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

#ifndef FD_Fs_h_

#define FD_Fs_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void a(FL_OBJECT *, long);
extern void b(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *Fs;
	void *vdata;
	long ldata;
} FD_Fs;

extern FD_Fs * create_form_Fs(void);

#endif /* FD_Fs_h_ */

