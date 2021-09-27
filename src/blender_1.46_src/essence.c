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







#include "/usr/people/frank/source/iff.h"
#include "/usr/people/include/util.h"
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <sys/syssgi.h>
#include <fcntl.h>

/* cc -O3 /usr/people/blend/noise.o essence.c -lgl_s -lfm_s -lm -o essence */

extern float hnoise(float, float, float, float);

float Tin, Tr, Tg, Tb;
float noisesize= 0.5;

int essence(float *texvec)
{
	float fac;
	int rt1, rt2, rt3, temp=0;
	
	fac= 255.0*hnoise(noisesize, texvec[0], texvec[1], texvec[2]);
	
	rt1= ffloor(1024.0*texvec[0]);
	rt2= ffloor(1024.0*texvec[1]);
	rt3= ffloor(1024.0*texvec[2]);
	
	rt1 &= 255;
	rt2 &= 255;
	rt3 &= 255;
		
	if(rt1 > fac) temp++;
	if(rt2 > fac) temp++;
	if(rt3 > fac) temp++;

	if(temp>=1) Tin= 1.0;
	else Tin= 0.0;
	
	return 0;
}


main(argc,argv)
int argc;
char **argv;
{
	float fwsize, texvec[3], dxz, dyz, dz;
	long sco[2];
	int r, g, b, rgb, num, x, y, wsize = 300;
	
	fwsize= 0.5*wsize;
	
	prefsize(wsize, wsize);
	winopen("Essence");
	RGBmode();
	gconfig();

	cpack(0);
	clear();
	
	dxz= 0.5/fwsize;
	dyz= 0.3/fwsize;
	
	for(y=0; y<wsize; y++) {
		texvec[1]= y/fwsize-1.0;
		dz= dyz*(y-fwsize);
		sco[1]= y;
		
		bgnpoint();
		
		for(x=0; x<wsize; x++) {
			texvec[0]= x/fwsize-1.0;
			texvec[2]= dz;
			
			rgb= essence(texvec);
			
			if(rgb) {
				r= 255.0*Tr;
				if(r<0) r=0; else if(r>255) r= 255;
				g= 255.0*Tg;
				if(g<0) g=0; else if(g>255) g= 255;
				b= 255.0*Tb;
				if(b<0) b=0; else if(b>255) b= 255;
				
				cpack(r+ 256*g + 65536*b);
			}
			else {
				r= 255.0*Tin;
				if(r<0) r=0; else if(r>255) r= 255;
				
				cpack(r+ 256*r + 65536*r);
				
			}
			sco[0]= x;
			v2i(sco);
			
			dz+= dxz;
		}
		
		endpoint();
	}
	
	while(getbutton(LEFTMOUSE)==0) sginap(1);
}

