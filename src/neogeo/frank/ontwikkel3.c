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

volatile long k;

/*

bindkeyo -r f1,'cc -O3 ontwikkel.c util.o -limbuf -limage -lgl_s -lm -o ontwikkel \n'
bindkeyo -r f2,'/usr/people/frank/source/script \n'
bindkeyo -r f2,'ontwikkel \n'
bindkeyo -r f3,'/usr/gfx/setmon -n 60HZ \n'
bindkeyo -r f1,'cc -O3 ontwikkel.c util.o -limbuf -limage -lgl -lX11 -o ontwikkel \n'

zipfork "cc -O3 ontwikkel.c util.o -limbuf -limage -lgl_s -lm -o ontwikkel"                                         
zipfork "ontwikkel >/dev/console"                                                                         
zipfork "cc -g ontwikkel.c util.o -limbuf -limage -lgl -lX11 -o ontwikkel"                                         
zipfork "ontwikkel >/dev/console"                                                                         

*/

/*
	tijden 250 maal:

	19.8		720		576		rgb		memcpy

	16.6		720		576		rgb		rectzoom(2.0,2.0) origineel half zo groot
	15.7		720		576		rgb		single/double (idem met video in pal-mode)
	20.8		720		576		cmap	single/double
	119.1	720		576		rgb		scanline

	4.1		720/2	576/2	rgb		single/double
	29.2		720/2	576/2	rgb		scanline(horz)
	58.0		720/2	576/2	rgb		scanline(vert)


*/

main()
{
    struct ImBuf *ibuf;
    long i, ofs, depth;
    short val;
    
    ibuf = loadiffname("/data/eurocard/crop/postbank.crop", 0);
    if (ibuf == 0) exit(0);
    
    prefsize(ibuf->x, ibuf->y);
    winopen("test");
    RGBmode();
    gconfig();
    qdevice(ESCKEY);
    
    cpack(0);
    clear();
    pixmode(PM_C0, 0);
    pixmode(PM_C1, -1);
    /*pixmode(PM_EXPAND, 1);*/
    pixmode(PM_SIZE, 1);
    pixmode(PM_TTOB, 1);
    
    while(qread(&val) != ESCKEY || val==0) {
	RGBwritemask(-1, -1, -1);
	cpack(0);
	clear();
	ofs = 0;
	depth = ibuf->depth / 3;
/*	for (i = 8 - depth; i < 8; i++){
	    RGBwritemask(1 << i, 0, 0);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs]);
	    ofs++;
	}
	for (i = 8 - depth; i < 8; i++){
	    RGBwritemask(0, 1 << i, 0);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs]);
	    ofs++;
	}
	for (i = 8 - depth; i < 8; i++){
	    RGBwritemask(0, 0, 1 << i);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs]);
	    ofs++;
	}
*/
	ofs = depth - 1;
	for (i = 7; i >= 8 - depth; i--){
	    RGBwritemask(1 << i, 0, 0);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs]);
	    RGBwritemask(0, 1 << i, 0);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs + depth]);
	    RGBwritemask(0, 0, 1 << i);
	    lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->planes[ofs + 2*depth]);
	    ofs--;
	}
    }
}

main_qeqe()
{
    ulong rect[100 * 100];
    long i;
    short val;
    struct ImBuf *ibuf;
    
    prefsize(100, 100);
    winopen("test");
    RGBmode();
    gconfig();
    qdevice(ESCKEY);
    
    cpack(0);
    clear();
    pixmode(PM_C0, 0);
    pixmode(PM_C1, -1);
    /*pixmode(PM_EXPAND, 1);*/
    pixmode(PM_SIZE, 1);
    
    for (i = (100 * 100) - 2; i>= 0; i-=2){
	rect[i] = -1;
	rect[i + 1] = 0;
    }
    
    while(qread(&val) != ESCKEY || val==0) lrectwrite(0, 0, 99, 99, rect);
}


main_full()
{
	long i;
	struct ImBuf *ibuf1, *ibuf2;
	struct tms voidbuf;
	long sx,sy, y;
	clock_t stijd,etijd,mtijd;
	long *rect,*rect1;
	float zoomx = 1.0, zoomy = 2.0;


	sx=720 / zoomx;sy=576/zoomy;
	
	sy /= 4;
	foreground();

	ibuf1 = allocImBuf(sx, sy, 24, 1, 0);
	ibuf2 = allocImBuf(sx, sy, 24, 1, 0);
		 
	for (i = (ibuf2->x * ibuf2->y) -1; i>=0 ; i--) ibuf2->rect[i] = 0xffffff;
	
	prefsize(zoomx * sx,zoomy * sy);
	winopen("test");
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	finish();
	rectzoom(zoomx, zoomy);
	
	stijd = times(&voidbuf);
	sginap(10);
	stijd = times(&voidbuf);
	
	for(i=240;i>0;i--){
		lrectwrite(0,0,ibuf1->x-1,ibuf1->y-1,ibuf1->rect);
		gsync();
		lrectwrite(0,0,ibuf2->x-1,ibuf2->y-1,ibuf2->rect);
		gsync();
	}

	etijd = times(&voidbuf);
	printf("%f\n",(etijd-stijd)/100.0);
	gexit();
}

main_stencil()
{
	long i;
	struct ImBuf *ibuf;
	struct tms voidbuf;
	long sx,sy, y;
	clock_t stijd,etijd,mtijd;
	long *rect,*rect1;

	sx=720;sy=576;
	foreground();

	if (ibuf = loadiffname("/data/GB/0001", LI_rect)){
	    sx = ibuf->x;
	    sy = ibuf->y;
	}else{
	    ibuf = allocImBuf(sx, sy, 24, 1, 0);
	}
	
	/*
	 * INDOGO
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
	 */
	 
	 
	prefsize(sx,sy);
	winopen("test");
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	finish();

	rectzoom(1.0, 2.0);
/*	stensize(4);
	gconfig();
	stencil(TRUE, 1, SF_ALWAYS, 1, ST_KEEP, ST_KEEP, ST_KEEP);
*/	stijd = times(&voidbuf);
	sginap(10);
	stijd = times(&voidbuf);
	for(i=250;i>0;i--){
		lrectwrite(0,0,ibuf->x-1,(ibuf->y / 2) -1,ibuf->rect);
	}

	etijd = times(&voidbuf);
	printf("%f\n",(etijd-stijd)/100.0);
	gexit();
}

void delay(cnt)
long cnt;
{
    ulong array[780];
    
    for (;cnt>0;cnt--)
	lrectwrite(0, 576, sizeof(array) - 1, 576, array);
}


main_benchmark()
{
	long i, j, event;
	struct ImBuf *ibuf, *ibufwit;
	struct tms voidbuf;
	long sx,sy, y;
	clock_t stijd,etijd,mtijd;
	long *rect,*rect1, del = 0;
	short val;
	
	sx=1000;sy=740;
	sx=780;sy=576;
	foreground();

	if (ibuf = loadiffname("rt/data/GB/0001", LI_rect)){
	    sx = ibuf->x;
	    sy = ibuf->y;
	}else{
	    ibuf = allocImBuf(sx, sy, 24, 1, 0);
	    ibufwit = allocImBuf(sx, sy, 24, 1, 0);
	    for(i = (ibuf->x * ibuf->y)-1 ; i>=0; i--){
		ibuf->rect[i] = 0;
		ibufwit->rect[i] = 0xffffff;
	    }
	}
	
	/*
	 * INDOGO
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
	 
	/* In PAL stel je met swapinterval hele frames in, geen fields!
	 * Omdat daarmee de tijdsduur tussen swaps verlengt wordt tot
	 * 1/25 seconde,  heb je ook geen problemen met gemiste 
	 * vertikale interrupts. Hoera !
	 */
	 
	noborder();
	prefposition(0, sx-1, 0, sy-1);
	winopen("test");
	/*doublebuffer();*/
	RGBmode();
	gconfig();
	clear();
	swapbuffers();
	clear();
	finish();
	
	/*pixmode(PM_SIZE, 16);*/
	pixmode(PM_TTOB, 1);
        schedctl(NDPRI, 0, NDPHIMAX);

	rectzoom(2.0, 2.0);
	stijd = times(&voidbuf);
	sginap(10);
	gsync();
	stijd = times(&voidbuf);
	qdevice(ESCKEY);
	qdevice(MINUSKEY);
	qdevice(EQUALKEY);
	qdevice(SPACEKEY);
	while((event=qread(&val)) != ESCKEY){
	    if (val==0) continue;
	    switch(event){
	    case EQUALKEY:
		del++;
		break;
	    case MINUSKEY:
		if (del > 0) del--;
		break;
	    case SPACEKEY:
		stijd = times(&voidbuf);
		for(i=250;i>0;i--){
		    lrectwrite(0,0,(ibuf->x-1)/2,(ibuf->y-1) / 2,ibufwit->rect);
/*
		    gsync();
		    delay(del);
		    lrectwrite(0,0,(ibuf->x-1),ibuf->y-1,ibufwit->rect);
		    gsync();
		    delay(del);
*/		    /*swapbuffers();*/
		}
		etijd = times(&voidbuf);
		printf("del:%d %f\n",del, (etijd-stijd)/100.0);
		break;
	    }
	}
	gexit();
}


mainrectwrite()
{
	short i;
	struct ImBuf *ibuf;
	struct tms voidbuf;
	long tijd;
	long sx,sy;
	clock_t stijd,etijd,mtijd;
	long *rect;

	sx=720;sy=576;

	prefsize(sx,sy);
	winopen(0);
	RGBmode();
/*	doublebuffer();
*/	gconfig();
	color(0);
	clear();
	color(1);

	ibuf=allocImBuf(sx,sy,32,1,0);

	tijd=time(0);

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



