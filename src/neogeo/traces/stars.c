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

/* 	stars.c	 */


#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <string.h>
#include "iff.h"
#include <malloc.h>
#include "storage.h"
#include <sys/times.h>
#include <fcntl.h>
#include <sys/param.h>
#include "/usr/people/include/Trace.h"
#include <stdlib.h>			/* rand */

extern unsigned char hash[512];

/* - er moet een 'vast' aantal sterren gegenereerd worden tussen near en far.
 * - alle sterren moeten bij voorkeur op de far liggen en uitsluitend in
 *   helderheid / kleur verschillen.
 * - 
 */

void make_stars(long wire)
{
	float vec[4], fx, fy, fz;
	long x, y, z, sx, sy, sz, ex, ey, ez, done = 0;
	float minx, maxx, miny, maxy, minz, maxz;
	double dblrand, hlfrand;
	ushort seed[3];
	float mat[4][4];
	struct HaloRen *har, *inithalo();
	extern struct HaloRen *inithalo();
	
	float stargrid = 1000 * wrld.rt5[0];		/* om de hoeveel een ster ? */

	float maxrand = 2.0;						/* hoeveel mag een ster verschuiven (uitgedrukt in grid eenheden) */
	int maxjit = 2 * wrld.rt5[1];			/* hoeveel mag een kleur veranderen */
	float force = 100.0 * wrld.rt5[2];			/* hoe sterk zijn de sterren */

	float far = 1000.0 * view0.far;
	float near = view0.near;
	
	if (wrld.rt5[0] <= 10) return;
	if (wire == 0) R.f |= 1;

	if(wire) stargrid*= 3.0;	/* tekent er minder */

	minx = miny = minz = HUGE;
	maxx = maxy = maxz = -HUGE;
	
	if (wire) Mat4Invert(mat, G.viewmat);
	else Mat4Invert(mat, G.persmat);
	
	/* BOUNDINGBOX BEREKENING
	 * bbox loopt van z = near | far,
	 * x = -z | +z,
	 * y = -z | +z
	 */
	 
	for (z = 0; z <= 1; z++) {
		if (z) fz = - far;
		else fz = - near;
		for (y = -1; y <= 1; y += 2){
			for (x = -1; x <= 1; x += 2){
				vec[0] = x * fz;
				vec[1] = y * fz;
				vec[2] = fz;
				vec[3] = 1.0;
				/*printf("%.2f %.2f %.2f       ", vec[0], vec[1], vec[2]);*/
				Mat4MulVecfl(mat, vec);
				/*printf("%.2f %.2f %.2f\n", vec[0], vec[1], vec[2]);*/
				if (vec[0] < minx) minx = vec[0];
				if (vec[0] > maxx) maxx = vec[0];
				if (vec[1] < miny) miny = vec[1];
				if (vec[1] > maxy) maxy = vec[1];
				if (vec[2] < minz) minz = vec[2];
				if (vec[2] > maxz) maxz = vec[2];
			}
		}
	}
	
/*	printf("x: %.2f %.2f %.0f\n", minx, maxx, (maxx - minx) / stargrid);
	printf("y: %.2f %.2f %.0f\n", miny, maxy, (maxy - miny) / stargrid);
	printf("z: %.2f %.2f %.0f\n", minz, maxz, (maxz - minz) / stargrid);
*/	
	/* omzetten naar grid coordinaten */
	
	sx = (minx / stargrid) - maxrand;
	sy = (miny / stargrid) - maxrand;
	sz = (minz / stargrid) - maxrand;
	
	ex = (maxx / stargrid) + maxrand;
	ey = (maxy / stargrid) + maxrand;
	ez = (maxz / stargrid) + maxrand;	

	dblrand = maxrand * stargrid;
	hlfrand = 2.0 * dblrand / RAND_MAX;
	/*hlfrand = 2.0 * dblrand / 1.0;*/

	if (wire) {
		cpack(-1);
		bgnpoint();
	}
		
	for (x = sx, fx = sx * stargrid; x <= ex; x++, fx += stargrid) {
		for (y = sy, fy = sy * stargrid; y <= ey ; y++, fy += stargrid){
			for (z = sz, fz = sz * stargrid; z <= ez; z++, fz += stargrid){
				srand((hash[z & 0xff] << 24) + (hash[y & 0xff] << 16) + (hash[x & 0xff] << 8));
				vec[0] = fx + (hlfrand * rand()) - dblrand;
				vec[1] = fy + (hlfrand * rand()) - dblrand;
				vec[2] = fz + (hlfrand * rand()) - dblrand;
				vec[3] = 1.0;

				if (wire) {
					v3f(vec);
					done++;
				} else {
					Mat4MulVecfl(G.persmat, vec);
					
					if (vec[2] <= -near) {
						minx = (far + vec[2]) / (far - near);
						if (minx < 0.0) minx = 0.0;
						if (minx > 1.0) minx = 1.0;
	
						/* halo's naar achteren brengen == bah */
/*						vec[0] /= vec[2];
						vec[1] /= vec[2];
						vec[2] -= (minx * 0.5 * (far - near));

						vec[0] *= vec[2];
						vec[1] *= vec[2];
*/						
						har = inithalo(vec, force);
						if (har) {
							har->alfa = 255.0 * minx;
							har->r = har->g = har->b = 255;
							if (maxjit) {
								har->r += ((maxjit * rand()) / RAND_MAX) - maxjit;
								har->g += ((maxjit * rand()) / RAND_MAX) - maxjit;
								har->b += ((maxjit * rand()) / RAND_MAX) - maxjit;
							}
							har->type = 31;
							done++;
						}
					}
				}
			}
			if(G.afbreek) break;
		}
		if(G.afbreek) break;
	}
	
	if (wire) endpoint();
	/*printf("done: %d\n", done);*/
}

