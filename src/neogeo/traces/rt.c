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



#include <stdio.h>
#include <math.h>

/*
cc rt.c -lm -o rt
*/

#define MIN2(x,y)		( (x)<(y) ? (x) : (y) )
#define MIN3(x,y,z)		MIN2( MIN2((x),(y)) , (z) )

float Normalise(n)
float *n;
{
	register float d;

	d= n[0]*n[0]+n[1]*n[1]+n[2]*n[2];
	if(d>0.0) {
		d= fsqrt(d);

		n[0]/=d; 
		n[1]/=d; 
		n[2]/=d;
	} else {
		n[0]=n[1]=n[2]= 0.0;
	}
	return d;
}


main()
{
	float lv[3], f, nr, fx, fy, fz;
	long c, a, b, a1[100],  a2[100];
	
	lv[0]= 0.000000;
	lv[1]= -0.995884;
	lv[2]= -1.090632;
	
	for(a=0; a<1000; a++) {
		for(b=0; b<1000; b++) {
			lv[0]= 0.000000;
			lv[1]= -0.995884;
			lv[2]= -1.090632;
			Normalise(lv);
		}	
	}
	
}

