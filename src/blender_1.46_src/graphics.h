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

/* graphics.h    dec 93 */


#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <gl/mygl.h>		/* in verband met typedef Object */
#include <gl/device.h>
#include <local/objfnt.h>
#include <fmclient.h>

#include <local/Button.h>
#include "screen.h"


#define LINE2S(v1, v2)	{bgnline(); v2s((short *)(v1)); v2s((short *)(v2)); endline();}
#define LINE3S(v1, v2)	{bgnline(); v3s((short *)(v1)); v3s((short *)(v2)); endline();}
#define LINE2I(v1, v2)	{bgnline(); v2i((int *)(v1)); v2i((int *)(v2)); endline();}
#define LINE3I(v1, v2)	{bgnline(); v3i((int *)(v1)); v3i((int *)(v2)); endline();}
#define LINE2F(v1, v2)	{bgnline(); v2f((float *)(v1)); v2f((float *)(v2)); endline();}
#define LINE3F(v1, v2)	{bgnline(); v3f((float *)(v1)); v3f((float *)(v2)); endline();}

#define IRIS 1
#define ELAN 2
#define ENTRY 3
#undef VIDEO
#define VIDEO 4
#define M_O2 5

#define drawmode mydrawmode

extern short getcursorN();	/* usiblender.c */

#endif /* GRAPHICS_H */

