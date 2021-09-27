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

#include <sys/types.h>

#include <stdlib.h>			/* voorkomt dat je bij malloc type moet aangeven */
#include <gl/gl.h>
#include <gl/device.h>
#include <local/iff.h>
#include <local/util.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <ctype.h>
#include <sys/mman.h> /* mapped memory */


/*

zipfork "cc -O3 -D_X_H_ -D_XLIB_H_ stretch.c util.o -float -limbuf -limage -lgl -lX11 -lm -o stretch >/dev/console"                                                
zipfork "stretch /data/pics/NLlijnen.iff >/dev/console"                                                

*/

/* hoe snel kun je een plaatje lrectwriten
 * willekeurige zoom (x, y) 
 * 
 */


float ibleft, ibbottom, ibright, ibtop;

void mapat(float left, float bottom, float right, float top)
{
	ibleft = left;
	ibbottom = bottom;
	ibright = right;
	ibtop = top;
}

float inter(float v1, float v2, float v3, float va, float vc)
{
	float vb;
		
	if (v1 == v3) return (va);
	vb = va - ((va - vc) * (v1 - v2) / (v1 - v3));

	return (vb);
}

void test_nw(float left, float bottom, float right, float top, struct ImBuf * ibuf)
{
	ulong *rect, *newrect;
	static ulong _newrect[4096];
	long x,y;
	long ofsx, ofsy, stepx, stepy, lasty, sizex, sizey;
	Matrix mat;
	float cox1, coy1, cox2, coy2;
	short newx, newy;

	if (ibuf == 0) return;
	if (ibuf->rect == 0) return;

	/* clip coordinaten in image-space (0 -> 1) */
	
	if (left == right || top == bottom) return;
	
	if (left < ibleft) left = ibleft;
	if (bottom < ibbottom) bottom = ibbottom;
	if (right > ibright) right = ibright;
	if (top > ibtop) top = ibtop;
	
	getmatrix(mat);

	for (y = 0; y < 4; y++){
		for (x = 0; x < 4; x++){
			printf("%.4f ", mat[y][x]);
		}
		printf("\n");
	}

	/* projectie */
	cox1 = (mat[0][0] * left) + mat[3][0];
	coy1 = (mat[1][1] * bottom) + mat[3][1];
	cox2 = (mat[0][0] * right) + mat[3][0];
	coy2 = (mat[1][1] * top) + mat[3][1];
	
	printf("%f %f %f %f\n", cox1, coy1, cox2, coy2);

	/* clippen met viewvolume */
	
	if (cox1 < -1.0) {
		left = inter(cox1, -1.0, cox2, left, right);
		cox1 = -1.0;
	}

	if (coy1 < -1.0) {
		bottom = inter(coy1, -1.0, coy2, bottom, top);
		coy1 = -1.0;
	}
	
	if (cox2 > 1.0) {
		right = inter(cox1, 1.0, cox2, left, right);
		cox2 = 1.0;
	}

	if (coy2 > 1.0) {
		top = inter(coy1, 1.0, coy2, bottom, top);
		coy2 = 1.0;
	}
	
	/* omzetten naar window coordinaten */
	
	getsize(&sizex,  &sizey);
	getorigin(&ofsx, &ofsy);
	
	cox1 = ((sizex + 1.0) * cox1 + sizex) / 2.0;
	cox2 = ((sizex + 1.0) * cox2 + sizex) / 2.0;

	coy1 = ((sizey + 1.0) * coy1 + sizey) / 2.0;
	coy2 = ((sizey + 1.0) * coy2 + sizey) / 2.0;
		
	sizex = getgdesc(GD_XPMAX);
	sizey = getgdesc(GD_YPMAX);
	
	/* clippen met scherm */
	
	if (cox1 + ofsx < 0) {
		left = inter(cox1, -ofsx, cox2, left, right);
		cox1 = -ofsx;		
	}

	if (coy1 + ofsy < 0) {
		bottom = inter(coy1, -ofsy, coy2, bottom, top);
		coy1 = -ofsy;
	}

	if (cox2 + ofsx > sizex) {
		right = inter(cox1, sizex - ofsx, cox2, left, right);
		cox2 = sizex - ofsx;		
	}

	if (coy2 + ofsy > sizey) {
		top = inter(coy1, sizey - ofsy, coy2, bottom, top);
		coy2 = sizey - ofsy;
	}

	printf("%f %f %f %f\n", cox1, coy1, cox2, coy2);
	
	/* coordinaten in plaatje berekenen */
	
	left = ibuf->x * (left - ibleft) / (ibright - ibleft);
	right = ibuf->x * (right - ibleft) / (ibright - ibleft);
	
	bottom = ibuf->y * (bottom - ibbottom) / (ibtop - ibbottom);
	top = ibuf->y * (top - ibbottom) / (ibtop - ibbottom);
	
	printf("%f %f %f %f\n\n", left, bottom, right, top);
	
	
	stepx = (65536.0 * (right - left) / (cox2 - cox1)) + 0.5;
	stepy = (65536.0 * (top - bottom) / (coy2 - coy1)) + 0.5;
	
	printf("%x %x\n", stepx, stepy);
	ofsy = 65536 * (bottom + 0.5);
	lasty = -0xffffff;
	newy = coy2;
	newx = cox2 - cox1;
	
	for (y = coy1; y < newy ; y++){
		newrect = _newrect;
		rect = ibuf->rect;
		if ((ofsy >> 16) >= ibuf->y) break;
		
		if ((ofsy >> 16) != (lasty >> 16)) {
			lasty = ofsy;
			rect += (ofsy >> 16) * ibuf->x;
			
			ofsx = 65536 * (left + 0.5);
			for (x = newx ; x>0 ; x--){
				*newrect++ = rect[ofsx >> 16];
				ofsx += stepx;
			}
		}
		ofsy += stepy;
		lrectwrite(cox1, y, cox1 + newx - 1, y, _newrect);
	}
}


main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf *ibuf;
	long sizex,sizey;
	long ofsx,ofsy;
	long maxx,maxy;
	short event,val;
	short redraw = TRUE;

	if (argc ==1) ibuf=loadiffname("/usr/data/pics/rt",LI_rect);
	else ibuf=loadiffname(argv[1],LI_rect);
	if(ibuf==0) exit(0);
	
	sizex = ibuf->x;
	sizey = ibuf->y;	
	
	maxx = getgdesc(GD_XPMAX);
	maxy = getgdesc(GD_YPMAX);
	
	if (ibuf->x <= maxx && ibuf->y <= maxy) prefsize(ibuf->x,ibuf->y);
	winopen("Stretch");
	RGBmode();
	gconfig();
	cpack(0);clear();
	maxsize(2 * maxx -1,2 * maxy-1);
	winconstraints();

	qdevice(ESCKEY);
	qdevice(WKEY);
	qdevice(IKEY);
	qdevice(OKEY);
	
	getsize(&sizex,&sizey);
	
	mapat(0.0, 0.0, sizex - 1, sizey -1 );
	
	while(((event = qread(&val)) != ESCKEY) ||  redraw != FALSE){
		getorigin(&ofsx,&ofsy);
		getsize(&sizex,&sizey);
		if (val){
			if (event == WKEY){
				if (argc > 2){
					saveiff(ibuf,argv[2],SI_rect);
				}
			}else if (event == IKEY){
				translate(ibuf->x / 2, ibuf->y / 2, 0);
				scale(2.0, 2.0, 1.0);
				translate(-ibuf->x / 2, -ibuf->y / 2, 0);
				redraw = TRUE;
			}else if (event == OKEY){
				translate(ibuf->x / 2, ibuf->y / 2, 0);
				scale(0.5, 0.5, 1.0);
				translate(-ibuf->x / 2, -ibuf->y / 2, 0);
				redraw = TRUE;
			}
		}
		if (event == REDRAW || redraw != FALSE) {
			viewport(0,sizex-1,0,sizey-1);

			clear();
						
			test_nw(0.0, 0.0, ibuf->x - 1, ibuf->y - 1, ibuf);
		}
		redraw = FALSE;
	}
}

