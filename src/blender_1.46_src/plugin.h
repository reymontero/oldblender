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
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/* plugin.h    dec 95

 * 
 * 
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include "/usr/people/include/Button.h"

#include <dlfcn.h>
#include <local/iff.h>

typedef struct VarStruct {
	int type;
	char name[16];
	float def, min, max;
} VarStruct;


/*extern void plugin_seq_doit(void *, float facf0, float facf1, int x, int y, ImBuf *ibuf1, ImBuf *ibuf2, ImBuf *out, ImBuf *use);
*/

#endif /* PLUGIN_H */

