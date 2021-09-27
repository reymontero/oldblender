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

#ifndef GL_UTIL_H

#define GL_UTIL_H

#include <sys/types.h>
#include <gl/gl.h>

#define LSHIFT	(1<<0)
#define RSHIFT	(1<<1)
#define SHIFT	(LSHIFT | RSHIFT)
#define LALT	(1<<2)
#define RALT	(1<<3)
#define ALT	(LALT | RALT)
#define LCTRL	(1<<4)
#define RCTRL	(1<<5)
#define CTRL	(LCTRL | RCTRL)
#define LMOUSE	(1<<16)
#define MMOUSE	(1<<17)
#define RMOUSE	(1<<18)
#define MOUSE	(LMOUSE | MMOUSE | RMOUSE)

extern short mousexN, mouseyN;
extern long qualN, winN;
extern short *mousexNp, *mouseyNp;
extern long *qualNp, *winNp;
extern long (*queread)();
extern Boolean (*getbut)(Device);
extern short findcolor(short r, short g, short b);
extern void percentdone(float percent);
extern short okee(char * str);
#endif /* GL_UTIL_H */

