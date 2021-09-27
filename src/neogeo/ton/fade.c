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

#include <gl/gl.h>

#include <gl/device.h>
#include <math.h>
#include <stdio.h>
 

void gamma_set(int clamp)
{
	int a, steps= 1;
	short ramp[3][256];

	a= 256;
	while(a--) {
		ramp[0][a]= a;

		if (a>clamp) ramp[1][a]= (a-clamp);
		else ramp[1][a]=0;
		
		ramp[2][a]= a;
	}
	gammaramp(ramp[0], ramp[1], ramp[2]);
	
}

void main(int argc, char **argv)
{
	short event, val;
	int win;

	foreground();
	noport();
	win= winopen("presenter");
	
	gamma_set(atoi(argv[1]));
	
	gexit();
}

