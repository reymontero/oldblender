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

#include <gl/gl.h>
#include <gl/device.h>
#include "iff.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/schedctl.h>

/*

bindkeyo -r f1,'cc -O2 ontwikkel.c util.o -limbuf -limage -lgl -lm -o ontwikkel \n'
bindkeyo -r f2,'ontwikkel \n'
bindkeyo -r f1,'cc -O2 ontwikkel.c -lgl_s imbuf.o -o ontwikkel \n'

*/

/*
	tijden 250 maal:

	19.8		720		576		rgb		memcpy

	16.6		720		576		rgb		rectzoom(2.0,2.0) origineel half zo groot
	15.7		720		576		rgb		single/double (idem met video in pal-mode)
	20.8		720		576		cmap	single/double
	119.1		720		576		rgb		scanline

	4.1			720/2	576/2	rgb		single/double
	29.2		720/2	576/2	rgb		scanline(horz)
	58.0		720/2	576/2	rgb		scanline(vert)


*/


main()
{
	short i;
	struct ImBuf *ibuf;
	struct tms voidbuf;
	long sx,sy;
	clock_t stijd,etijd,mtijd;
	long *rect,*rect1;

	sx=720;
	sy=576;
	foreground();

	if (ibuf = loadiffname("/data/GB/0001", IB_rect)){
		sx = ibuf->x;
		sy = ibuf->y;
	} else{
		ibuf = allocImBuf(sx, sy, 24, 1, 0);
	}

	/*
	 * INDIGO
	 * 
	 * x	y   mode    tijd    notes
	 * 
	 * 720	576 cmode   6.00    met PM_STRIDE = 720
	 * 		    5.02    PM_STRIDE = 0
	 *		    4.31    idem met PM_TTOB
	 *		    35.0    scanline
	 *		    6.13    in stroken van 16
	 * 720 576 RGB	    8.60    PM_TTOB
	 *
	 * 720	576 cmode   4.41    geen swapbuffers
	 *		    8.31    swapinterval(1)
	 *		    12.48   swapinterval(2)
	 *		    16.60   swapinterval(3)
	 *
	 * 780 576 RGB	    8.94
	 *		    6.13    rectzoom(2.0, 2.0);
	 */


	prefsize(sx,sy);
	winopen("test");
	/*doublebuffer();*/
	RGBmode();
	gconfig();
/*	frontbuffer(1);
	backbuffer(0);
	color(0);
	clear();
	finish();
*/
	/*pixmode(PM_SIZE, 16);*/
	/*pixmode(PM_STRIDE, sx);*/
	pixmode(PM_TTOB, 1);

	stijd = times(&voidbuf);
	sginap(10);
	stijd = times(&voidbuf);
	for(i=250;i>0;i--){
		lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
		swapbuffers();
	}
	etijd = times(&voidbuf);
	printf("%f\n",(etijd-stijd)/100.0);
	gexit();
}


main_kwenie()
{
	short i;
	struct ImBuf *ibuf;
	struct tms voidbuf;
	long tijd;
	long sx,sy;
	clock_t stijd,etijd,mtijd;
	long *rect;

	sx=720;
	sy=576;

	prefsize(sx,sy);
	winopen(0);
	RGBmode();
	gconfig;
	color(0);
	clear();
	color(1);

	ibuf=allocImBuf(sx,sy,32,1,0);

	tijd=time(0);

	pixmode(PM_TTOB, 1);
	/*pixmode(PM_SIZE, 32);*/

	sginap(10);
	for(i=10;i>0;i--){
		lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
	}

	sginap(10);
	tijd=time(0);

	stijd = times(&voidbuf);
	for(i=250;i>0;i--){
		lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
	}
	etijd = times(&voidbuf);
	printf("%f\n",(etijd-stijd)/100.0);
	gexit();
}


main_diskspeed(int argc, char ** argv)
{
	clock_t stijd,etijd,mtijd;
	char * buf;
	int bufsize = 512, min = 0, max = 0, avg = 0, done = 0, total = 0, count = 0;
	struct tms voidbuf;
	
	if (argc > 1) bufsize = atoi(argv[1]);

	buf = malloc(bufsize);
	
	stijd = times(&voidbuf);
	min = 2000000000;
	printf("%d\n", fread(buf, 1, bufsize, stdin));
	
	while(fread(buf, 1, bufsize, stdin) == bufsize) {
		done += bufsize;
		etijd = times(&voidbuf);
		if (etijd - stijd > 100) {
			avg = (done * 100.0) / (etijd - stijd);
			if (avg < min) min = avg;
			if (avg > max) max = avg;
			total += avg;
			count++;
			printf("min: %8d, cur: %8d, max %8d, avg:%8d\n", min, avg, max, total / count);
			done = 0;
			stijd = etijd;
		}
	}
}

