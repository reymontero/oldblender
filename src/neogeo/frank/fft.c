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
#include <stdio.h>
#include <math.h>
#include <local/iff.h>
#include <fft.h>

/*

zipfork "cc -g fft.c -DX_H -D_XLIB_H_ -o fft util.o -limbuf -limage -lgl -lX11 -lcomplib.sgimath -lm >/dev/console"                                             
zipfork "fft >/dev/console"                                                                              
*/


#define SIZE 720
#define SCALE 1.0
#define BOX 20
#define END 360

long cbars[] =
{
	0xffffff, 
	0x00c0c0, 
	0xc0c000, 
	0x00c000, 
	0xc000c0, 
	0x0000c0, 
	0xc00000, 
	0x000000
};


/*
#define complex zomplex
#define cfft1d zfft1d
#define cfft1di zfft1di
*/

main_image(int argc,  char ** argv)
{
	complex * arr, *p;
	complex * wspace;
	long i, col, x, y, box = BOX, offs = 1, arrsize;
	long mx = 0, my = 0, device;
	long start = 0, end = END;
	long ofsx, ofsy, filter, spatial = TRUE, changed;
	ulong * rect;
	short val;
	float v[2], t, w;
	struct ImBuf *sbuf, *dbuf;
	int zoom = 1, rept = 2;
	
	if (argc == 1) exit (0);
	
	sbuf = loadiffname(argv[1], IB_rect);
	if (sbuf == 0) exit (1);
	
	dbuf = dupImBuf(sbuf);
	
	arrsize = sbuf->x * sbuf->y;
	arr = mallocstructN(complex, arrsize, "FFT-array");
	wspace = mallocstructN(complex, (sbuf->x + 15) * (sbuf->y + 15), "FFT-array");
	
	prefsize(rept * zoom * sbuf->x, rept * zoom * sbuf->y);
	
	winopen("FFT");
	RGBmode();
	doublebuffer();
	gconfig();
	rectzoom(zoom, zoom);
	
	lrectwrite(0, 0, dbuf->x - 1, dbuf->y - 1, dbuf->rect);
	swapbuffers();
	
	qdevice(KEYBD);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(LEFTMOUSE);

	cfft2di(sbuf->x, sbuf->y, wspace);

	start = 0;

	while (1) {
		/* array maken */

		/* vullen */
		
		rect = sbuf->rect;
		p = arr;
		
		for (i = 0; i < arrsize; i++) {
			p->re = (*rect & 0xff) / 255.0;
			p->im = 0.0;
			p++;
			rect++;
		}
		
		printf("forward\n");
		cfft2d(-1, sbuf->x, sbuf->y, arr, sbuf->x, wspace);
			
		/* filteren */
		
		printf("using %d %d\n", mx, my);
		
		p = arr;
		for (y = 0 ; y < sbuf->y; y++) {
			for (x = 0 ; x < sbuf->x; x++) {
				if (y != 0 || x != 0) {
					if (y < my || x < mx) p->re = p->im = 0.0;
				}
				p++;
			}
		}

		printf("reverse\n");
		cfft2d(1, sbuf->x, sbuf->y, arr, sbuf->x, wspace);

		rect = dbuf->rect;
		p = arr;
		
		for (i = 0; i < arrsize; i++) {
			col = ((255.0 * p->re) / arrsize) + 0.5;
			if (col < 0) col = 0;
			if (col > 255) col = 255;
			
			*rect = col * 0x010101;
			p++;
			rect++;
		}
		
		printf("\n");
		for (x = 0 ; x < rept * zoom; x += zoom) {
			for (y = 0 ; y < rept * zoom; y += zoom) {
				lrectwrite((x * dbuf->x), y * dbuf->y, (x * dbuf->x) + dbuf->x - 1, (y * dbuf->y) + dbuf->y - 1, dbuf->rect);				
			}
		}
		swapbuffers();

		changed = FALSE;

		do{
			switch (device = qread(&val)){
			case KEYBD:
				changed = TRUE;
				switch (val){
					/* output */
				case 'c':
					make_cbars();
					break;
				case 'f':
					spatial = FALSE;
					break;
				case 's':
					spatial = TRUE;
					break;
					/* componenten */
				case 'u':
					offs = 2;
					break;
				case 'v':
					offs = 0;
					break;
				case 'y':
					offs = 1;
					break;
					/* filters */
				case 'b':
					filter = 0;
					break;
				case 't':
					filter = 1;
					break;
				case 'g':
					filter = 2;
					break;
				}
				break;
			case MOUSEX:
			case MOUSEY:
				getorigin(&ofsx, &ofsy);
				if (device == MOUSEX) mx = (val - ofsx) / (rept * zoom);
				else my = (val - ofsy) / (rept * zoom);
				if (getbutton(LEFTMOUSE)){
					changed = TRUE;
				} else if (getbutton(MIDDLEMOUSE)){
					changed = TRUE;
					getorigin(&ofsx, &ofsy);
					end = val - ofsx - 5;
					if (end < start) end = start;
					if (end > SIZE / 2 + 1) end = SIZE / 2 + 1;
				}
				break;
			case LEFTMOUSE:
				changed = TRUE;
			}
		}while(qtest() || changed == FALSE);
	}
}

main_cbars()
{
	complex arr[SIZE + 1], *p;
	complex wspace[(SIZE + 15)];
	long i, a, col, x, box = BOX, offs = 1;
	long start = 0, end = END;
	long ofsx, ofsy, filter, spatial = TRUE, changed;
	short val;
	ulong rect[SIZE];
	float v[2], t, w;

	prefsize(SCALE * SIZE + 10, SCALE * SIZE);
	winopen("FFT");
	RGBmode();
	doublebuffer();
	gconfig();
	qdevice(KEYBD);
	qdevice(MOUSEX);
	qdevice(LEFTMOUSE);

	cfft1di(SIZE, wspace);

	start = 0;

	for (i = 0; i < 8; i++){
		col = cbars[i];
		col = colcspace(col,rgbbeta);
		x = (col >> (offs * 8)) & 0xff;
		printf("%x\n", x);
	}

	while (1) {
		/* array maken */

		p = arr;
		for (i = 0; i < 8; i++){
			col = cbars[i];
			col = colcspace(col,rgbbeta);
			t = ((col >> (offs * 8)) & 0xff) / 255.0 - 0.5;
			for (x = 0; x < SIZE / 8; x++){
				p->re = t;
				p->im = 0.0;
				p ++;
			}
		}

		cfft1d(-1, SIZE, arr, 1, wspace);
	
		/* box */

		for (x = end; x < SIZE - end; x++){
			arr[x].re = 0.0;
			arr[x].im = 0.0;
		}

		box = end - start;

		if (filter == 1) {
			/* tent */

			for (i = start; i < end; i++){
				t = 1.0 - ((float) (i - start) / (float) box);
				arr[i].re *= t;
				arr[i].im *= t;
				arr[SIZE - 1 - i].re *= t;
				arr[SIZE - 1 - i].im *= t;
			}
		} else if (filter == 2){
			/* Gauss */
			w = 1.0 / (1.0  - 1.0 / exp(1.5*1.5*1.5*1.5));

			for (i = start; i < end; i++){
				t = (2.25 * (i - start)) / box;
				t = 1.0 / exp (t * t) - 1.0 / exp(1.5 * 1.5 * 1.5 * 1.5);
				t *= w;
				arr[i].re *= t;
				arr[i].im *= t;
				arr[SIZE - 1 - i].re *= t;
				arr[SIZE - 1 - i].im *= t;
			}
		}

		if (spatial) cfft1d(1, SIZE, arr, 1, wspace);
		else{
			for (i =0; i < SIZE; i++){
				arr[i].re *= 10.0;
				arr[i].im *= 5.0;
			}
		}

		/* drawing */
		cpack(0);
		clear();

		/* box */
		cpack(0xffff0000);
		bgnline();
		v[0] = end + 5;
		v[1] = 10000;
		v2f(v);
		v[1] = -10000;
		v2f(v);
		endline();

		cpack(0xff0000ff);
		bgnline();
		v[0] = start + 5;
		v[1] = 10000;
		v2f(v);
		v[1] = -10000;
		v2f(v);
		endline();

		cpack(-1);
		bgnline();
		for (i = 0; i < SIZE; i++){
			v[0] = SCALE * i + 5;
			v[1] = ((SCALE * arr[i].re) + (SIZE * SCALE)) * 0.5;
			v2f(v);
		}
		endline();

		for (i = 0; i < SIZE; i++){
			t = arr[i].re / (float) SIZE + 0.5;
			/*printf("%.2f\n", t);*/
			if (t > 1.0) t = 1.0;
			else if (t < 0.0) t = 0.0;
			a = 255.0 * t;
			rect[i] = (a << 16) + (a << 8) + a;
		}

		rectzoom(1.0, 40.0);
		lrectwrite(0, 0, SIZE - 1, 0, rect);
		swapbuffers();

		changed = FALSE;

		do{
			switch (qread(&val)){
			case KEYBD:
				changed = TRUE;
				switch (val){
					/* output */
				case 'c':
					make_cbars();
					break;
				case 'f':
					spatial = FALSE;
					break;
				case 's':
					spatial = TRUE;
					break;
					/* componenten */
				case 'u':
					offs = 2;
					break;
				case 'v':
					offs = 0;
					break;
				case 'y':
					offs = 1;
					break;
					/* filters */
				case 'b':
					filter = 0;
					break;
				case 't':
					filter = 1;
					break;
				case 'g':
					filter = 2;
					break;
				}
				break;
			case MOUSEX:
				if (getbutton(LEFTMOUSE)){
					changed = TRUE;
					getorigin(&ofsx, &ofsy);
					start = val - ofsx - 5;
					if (start > end) start = end;
					if (start < 0) start = 0;
				} else if (getbutton(MIDDLEMOUSE)){
					changed = TRUE;
					getorigin(&ofsx, &ofsy);
					end = val - ofsx - 5;
					if (end < start) end = start;
					if (end > SIZE / 2 + 1) end = SIZE / 2 + 1;
				}
				break;
			}
		}while(qtest() || changed == FALSE);
	}
}


make_cbars()
{
	complex arr[SIZE + 1], *p;
	complex wspace[(SIZE + 15)];
	long i, a, col, x, box = BOX, offs = 1;
	long start = 0, end = END;
	long filter = 1;
	ulong *rect;
	float t, w, v[2];
	struct ImBuf * ibuf;

	cfft1di(SIZE, wspace);

	ibuf = allocImBuf(720, 576, 24, 1, 0);
	rect = ibuf->rect;
	rectzoom(1.0, 20.0);

	for (offs = 0; offs < 4; offs++){

		if (offs == 0) end = 576 - 450;
		if (offs == 1) end = 576 - 340;
		if (offs == 2) end = 576 - 450;

		p = arr;
		for (i = 0; i < 8; i++){
			col = cbars[i];
			col = colcspace(col,rgbbeta);
			t = ((col >> (offs * 8)) & 0xff) / 255.0 - 0.5;
			for (x = 0; x < SIZE / 8; x++){
				p->re = t;
				p->im = 0.0;
				p ++;
			}
		}

		cfft1d(-1, SIZE, arr, 1, wspace);

		/* box */

		for (x = end; x < SIZE - end; x++){
			arr[x].re = 0.0;
			arr[x].im = 0.0;
		}

		box = end - start;

		if (filter == 1) {
			/* tent */

			for (i = start; i < end; i++){
				t = 1.0 - ((float) i / (float) box);
				arr[i].re *= t;
				arr[i].im *= t;
				arr[SIZE - 1 - i].re *= t;
				arr[SIZE - 1 - i].im *= t;
			}
		} else if (filter == 2){
			/* Gauss */
			w = 1.0 / (1.0  - 1.0 / exp(1.5*1.5*1.5*1.5));

			for (i = start; i < end; i++){
				t = (2.25 * (i - start)) / box;
				t = 1.0 / exp (t * t) - 1.0 / exp(1.5 * 1.5 * 1.5 * 1.5);
				t *= w;
				arr[i].re *= t;
				arr[i].im *= t;
				arr[SIZE - 1 - i].re *= t;
				arr[SIZE - 1 - i].im *= t;
			}
		}

		cfft1d(1, SIZE, arr, 1, wspace);

		/* drawing */
		cpack(0); 
		clear();

		cpack(0xffff0000);
		bgnline();
		v[0] = end + 5;
		v[1] = 10000;
		v2f(v);
		v[1] = -10000;
		v2f(v);
		endline();

		cpack(-1);
		bgnline();
		for (i = 0; i < SIZE; i++){
			v[0] = SCALE * i + 5;
			v[1] = ((SCALE * arr[i].re) + (SIZE * SCALE)) * 0.5;
			v2f(v);
		}
		endline();

		for (i = 0; i < SIZE; i++){
			t = arr[i].re / (float) SIZE + 0.5;
			if (t > 1.0) t = 1.0;
			else if (t < 0.0) t = 0.0;
			a = 255.0 * t;
			*(((char *) rect) + 4 * i + (3 - offs)) = a;
		}

		lrectwrite(0, 0, SIZE - 1, 0, rect);
		swapbuffers();
	}

	printf("\n");

	for (i = 0; i< SIZE; i++){
		printf("0x%6x,\n", rect[i] & 0xffffff);
	}
}

main(int argc, char ** argv)
{
	if (argc == 1) main_cbars();
	else main_image(argc, argv);
}

