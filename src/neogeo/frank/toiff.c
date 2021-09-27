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

#include <local/iff.h>

#include <stdio.h>
#include <gl/image.h>
#include "imbuf/imbuf.h"
#include <sys/schedctl.h>


#define OBJECTBLOK "imconv"

void rota(double * px, double * py, double angle)
{
	double si, co;
	double x, y, nx, ny;
	
	angle = angle * (2.0 * M_PI / 360.0);
	
	si = sin(angle);
	co = cos(angle);
	
	x = *px;
	y = *py;
	
	nx = (co * x) - (si * y);
	ny = (si * x) + (co * y);
	
	*px = nx;
	*py = ny;
}


void add12more(ulong * cmap, long y, double fu, double fv)
{
	long i, j, u, v, col;
	
	for (i = 0; i < 12; i ++) {
		u = fu + 128.5;
		v = fv + 128.5;
		
		if (u & 0xffffff00) {
			if (u < 0) u = 0;
			else u = 255;
		}

		if (v & 0xffffff00) {
			if (v < 0) v = 0;
			else v = 255;
		}
		
		col = (u << 16) + (y << 8) + v;
		col = colcspace(col, yuvrgb);
		
		for (j = 1; j > 0; j--) {
			col = colcspace(col, rgbyuv);
			
			u = (col >> 16) & 0xff;
			v = col & 0xff;
			
			if (u < 128 - 96) u = 128 - 96;
			else if (u > 128 + 96) u = 128 + 96;

			if (v < 128 - 96) v = 128 - 96;
			else if (v > 128 + 96) v = 128 + 96;
			
			col = (u << 16) + (y << 8) + v;
			col = colcspace(col, yuvrgb);
		}
		
		cmap[i] = col;
		
		rota(&fu, &fv, 30.0);
	}
}

void add6more(ulong * cmap, long y, double fu, double fv)
{
	long i, j, u, v, col;
	
	for (i = 0; i < 6; i ++) {
		u = fu + 128.5;
		v = fv + 128.5;
		
		if (u & 0xffffff00) {
			if (u < 0) u = 0;
			else u = 255;
		}

		if (v & 0xffffff00) {
			if (v < 0) v = 0;
			else v = 255;
		}
		
		col = (u << 16) + (y << 8) + v;
		col = colcspace(col, yuvrgb);
		
		for (j = 3; j > 0; j--) {
			col = colcspace(col, rgbyuv);
			
			u = (col >> 16) & 0xff;
			v = col & 0xff;
			
			if (u < 128 - 96) u = 128 - 96;
			else if (u > 128 + 96) u = 128 + 96;

			if (v < 128 - 96) v = 128 - 96;
			else if (v > 128 + 96) v = 128 + 96;
			
			col = (u << 16) + (y << 8) + v;
			col = colcspace(col, yuvrgb);
		}
		
		cmap[i] = col;
		
		rota(&fu, &fv, 60.0);
	}
}

void add3more(ulong * cmap, long y, double fu, double fv)
{
	long i, j, u, v, col;
	
	for (i = 0; i < 3; i ++) {
		u = fu + 128.5;
		v = fv + 128.5;
		
		if (u & 0xffffff00) {
			if (u < 0) u = 0;
			else u = 255;
		}

		if (v & 0xffffff00) {
			if (v < 0) v = 0;
			else v = 255;
		}
		
		col = (u << 16) + (y << 8) + v;
		col = colcspace(col, yuvrgb);
		
		for (j = 3; j > 0; j--) {
			col = colcspace(col, rgbyuv);
			
			u = (col >> 16) & 0xff;
			v = col & 0xff;
			
			if (u < 128 - 96) u = 128 - 96;
			else if (u > 128 + 96) u = 128 + 96;

			if (v < 128 - 96) v = 128 - 96;
			else if (v > 128 + 96) v = 128 + 96;
			
			col = (u << 16) + (y << 8) + v;
			col = colcspace(col, yuvrgb);
		}
		
		cmap[i] = col;
		
		rota(&fu, &fv, 120.0);
	}
}


void fillyuvcmap(struct ImBuf * ibuf)
{
	long y, i, col;
	ulong * cmap;
	long ur, vr;
	double fu, fv;
	
	/* genereert een yuv colormap
	 * 
	 * 13 y stappen met telkens 19 u & v stappen
	 * 
	 * de u en v bevinden zich als 3 ringen van 6 om de centrale as
	 * van de grijswaarde
	 * 
	 * primaire 8 kleuren (zwart, wit, rood, ...) komen in de eerste 8 te staan.
	 * zwart & wit kunnen geen 19 u & v stappen hebben.
	 */
	
	cmap = ibuf->cmap;
	
	col = 0xff; /* rood */
	col = colcspace(col, rgbyuv);
	ur = (col & 0xff0000) >> 16;
	vr = col & 0xff;
		
	ur -= 128;
	vr -= 128;
		
	/* zwart & wit & primair*/
	
	*cmap++ = 0;
	*cmap++ = 0xffffff;

	for (i = 1; i <= 14; i++) {
		*cmap++ = i * 0x111111;
	}
	
	*cmap++ = 0x0000ff;
	*cmap++ = 0x00ffff;
	*cmap++ = 0x00ff00;
	*cmap++ = 0xffff00;
	*cmap++ = 0xff0000;
	*cmap++ = 0xff00ff;

	for (i = 0; i < 13; i++) {
		y = (((i + 1) * 255) / 14.0) + 0.5;
		
		fu = 1.0 * ur / 4.0;
		fv = 1.0 * vr / 4.0;
		
		add6more(cmap, y, fu, fv);
		cmap += 6;
				
		fu = 2.0 * ur / 4.0;
		fv = 2.0 * vr / 4.0;
		rota(&fu, &fv, 30.0);
		
		add6more(cmap, y, fu, fv);
		cmap += 6;
				
		fu = 3.0 * ur / 4.0;
		fv = 3.0 * vr / 4.0;
		
		add6more(cmap, y, fu, fv);
		cmap += 6;
	}	
}

void fillyuvcmap_new(struct ImBuf * ibuf)
{
	long y, i, col;
	ulong * cmap;
	long ur, vr;
	double fu, fv;
	
	/* genereert een yuv colormap
	 * 
	 * 13 y stappen met telkens 19 u & v stappen
	 * 
	 * de u en v bevinden zich als 3 ringen van 6 om de centrale as
	 * van de grijswaarde
	 * 
	 * primaire 8 kleuren (zwart, wit, rood, ...) komen in de eerste 8 te staan.
	 * zwart & wit kunnen geen 19 u & v stappen hebben.
	 */
	
	cmap = ibuf->cmap;
	
	col = 0xff; /* rood */
	col = colcspace(col, rgbyuv);
	ur = (col & 0xff0000) >> 16;
	vr = col & 0xff;
		
	ur -= 128;
	vr -= 128;
		
	/* zwart & wit & primair*/
	
	*cmap++ = 0;
	*cmap++ = 0xffffff;

	for (i = 1; i <= 14; i++) {
		*cmap++ = i * 0x111111;
	}
	
	*cmap++ = 0x0000ff;
	*cmap++ = 0x00ffff;
	*cmap++ = 0x00ff00;
	*cmap++ = 0xffff00;
	*cmap++ = 0xff0000;
	*cmap++ = 0xff00ff;

	for (i = 0; i < 13; i++) {
		y = (((i + 1) * 255) / 14.0) + 0.5;
		
		fu = 0.5 * ur / 4.0;
		fv = 0.5 * vr / 4.0;
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
				
		fu = 1.0 * ur / 4.0;
		fv = 1.0 * vr / 4.0;
		rota(&fu, &fv, 60.0);
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
		
		fu = 1.5 * ur / 4.0;
		fv = 1.5 * vr / 4.0;
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
				
		fu = 2.0 * ur / 4.0;
		fv = 2.0 * vr / 4.0;
		rota(&fu, &fv, 60.0);
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
		
		fu = 2.5 * ur / 4.0;
		fv = 2.5 * vr / 4.0;
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
				
		fu = 3.0 * ur / 4.0;
		fv = 3.0 * vr / 4.0;
		rota(&fu, &fv, 60.0);
		
		add3more(cmap, y, fu, fv);
		cmap += 3;
	}
}

void fill666cmap_oud(struct ImBuf * ibuf)
{
	long i, r, g, b;
	ulong * cmap;
	
	cmap = ibuf->cmap;
	
	for (r = 0; r < 256; r += 51) {
		for (g = 0; g < 256; g += 51) {
			for (b = 0; b < 256; b += 51) {
				*cmap++ = (b << 16) + (g << 8) + r;
			}
		}
	}
	
	for (i = 0; i < 256; i += 17) {
		if (i % 51) {
			*cmap++ = (i << 16) + (i << 8) + i;
		}
	}
	
	for (i = 25; i < 256; i += 51) {
		*cmap++ = (0 << 16) + (0 << 8) + i;
		*cmap++ = (0 << 16) + (i << 8) + 0;
		*cmap++ = (0 << 16) + (i << 8) + i;
		*cmap++ = (i << 16) + (0 << 8) + 0;
		*cmap++ = (i << 16) + (0 << 8) + i;
		*cmap++ = (i << 16) + (i << 8) + 0;
	}
}

void wincmap(ulong * cmap)
{
	*cmap++ = 0x000000;
	*cmap++ = 0x0000bf;
	*cmap++ = 0x00bf00;
	*cmap++ = 0x00bfbf;
	*cmap++ = 0xbf0000;
	*cmap++ = 0xbf00bf;
	*cmap++ = 0xbfbf00;
	*cmap++ = 0xbfbfbf;
	*cmap++ = 0x808080;
	*cmap++ = 0x0000ff;
	*cmap++ = 0x00ff00;
	*cmap++ = 0x00ffff;
	*cmap++ = 0xff0000;
	*cmap++ = 0xff00ff;
	*cmap++ = 0xffff00;
	*cmap++ = 0xffffff;
}

void fill666cmap(struct ImBuf * ibuf)
{
	long i, r, g, b;
	ulong * cmap;
	
	cmap = ibuf->cmap;
	ibuf->mincol = 16;
	ibuf->maxcol = 216 + 16;
	
	wincmap(cmap);
	cmap[0] = 0xffffff;
	cmap[15] = 0x0;
	
	cmap += 16;
	
	for (r = 0; r < 256; r += 51) {
		for (g = 0; g < 256; g += 51) {
			for (b = 0; b < 256; b += 51) {
				*cmap++ = (b << 16) + (g << 8) + r;
			}
		}
	}
	cmap[-1] = 0xfefefe;
	cmap += 8;
	wincmap(cmap);
}


void dity(struct ImBuf * ibuf, short ofs, double res)
{
	/*uchar _dit[] = {6, 4, 2, 3, 1, 8, 0, 7, 5}, *dit, *rect;*/
	uchar _dit[] = {0, 8, 4, 5, 1, 6, 7, 3, 2}, *dit, *rect;
	long y, x, u = 0, v = 0;
	double div;
	short c;
	
	for (x = 0; x < sizeof(_dit); x++) _dit[x] *= res;
	
	rect= (uchar *)ibuf->rect;
	rect +=ofs;
	
	div = res * 9.0;
	
	for(y = ibuf->y; y > 0; y--){
		dit = _dit + 3 * u;		
		v = 0;
		for (x = ibuf->x; x>0; x--){
			c = rect[0] + dit[v];
			c = (c / div);
			
			rect[0] = c * div; /* for now */
			rect += 4;
			
			v++;
			if (v == 3) v = 0;
		}
		u++;
		if (u == 3) u = 0;
	}

}

void dit_max_ic(struct ImBuf * ibuf, short ofs, short max)
{
	short x, y, pix, and, add1, add2;
	uchar *rect;
	double div;
	uchar dit[] = {2, 10, 14, 6}; /* gemiddeld 8 */

	rect = (uchar *)ibuf->rect;
	rect += ofs;

	div = 255.0 / max;
	
	for(y=ibuf->y;y>0;y--){
		if(y & 1){
			add1 = dit[0];
			add2 = dit[1];
		}
		else{
			add1 = dit[2];
			add2 = dit[3];
		}
		
		for(x = ibuf->x; x > 0; x--){
			pix = *rect;
			if (x & 1) pix += add1;
			else pix += add2;

			*rect = pix / div;
			*rect *= div;
			rect += 4;
		}
	}
}


void tan_add_dit4_(struct ImBuf * ibuf, short ofs)
{
	short x, y;
	uchar * rect;
	short * add;
	static short dit[] = 	{
		0, 14, 1, 15, 10, 4, 11, 5, 2, 12, 3, 13, 8, 6, 9, 7 };

	rect= (uchar *)ibuf->rect;
	rect +=ofs;

	for(y=ibuf->y;y>0;y--){
		add = &dit[4 * (y & 3)];
		for (x = ibuf->x; x > 0; x--){
			*rect += add[x & 3];
			rect += 4;
		}
	}
}

void tan_add_dit2_(struct ImBuf * ibuf, short ofs)
{
	short x, y, pix, add1, add2, dit[] = {0, 4, 6, 2}, dit_[] = {-3, 1, 3, -1};
	uchar *rect;

	rect = (uchar *) ibuf->rect;
	rect += ofs;

	for (y = ibuf->y; y > 0; y--){

		if (y & 1){
			add1 = dit[0];
			add2 = dit[1];
		} else {
			add1 = dit[2];
			add2 = dit[3];
		}
		
		for (x = ibuf->x; x > 0; x--){
			pix = *rect;
			if (x & 1) pix += add1;
			else pix += add2;
			
			if (pix & 0x100) {
				if (pix < 0) pix = 0;
				else pix = 255;
			}

			*rect = pix;
			rect += 4;
		}
	}
}


void atan_space_(ImBuf * ibuf)
{
	long x, y;
	double u, v, t, l, lastl = -1.0;
	uchar * rect;
	
	rect = (uchar *) ibuf->rect;
	for (y = ibuf->y; y > 0 ; y--) {
		for (x = ibuf->x; x > 0 ; x--) {
			u = rect[1] - 128;
			v = rect[3] - 128;
			t = 128.0 * ((atan2(0.75 * u, v) / M_PI) + 1.0);
						
			l = 256.0 * sqrt(((u * u) + (v * v)) / 32768.0);
			
/*			if (l != lastl) {
				lastl = l;
				printf("%.2f\n", l);
			}
*/			
			/* l loopt van 0 -> 193 en moet uiteindelijk een bereik
			 * van (7 - 1) * 16  = 96 krijgen. (komt dat even mooi uit)
			 */
			
			l = (l / 2.0) + 80.0;
			
			if (t > 255.0) t = 255.0;
			if (l > 255.0) l = 255.0;			
						
			rect[1] = t;
			rect[3] = l;
			rect += 4;
		}
	}
}


void tan_space_(ImBuf * ibuf)
{
	long x, y;
	double u, v, t, l;
	uchar * rect;
	
	rect = (uchar *) ibuf->rect;
	for (y = ibuf->y; y > 0 ; y--) {
		for (x = ibuf->x; x > 0 ; x--) {
			t = M_PI * ((rect[1] / 128.0) - 1.0);

			l = 2.0 * (rect[3] - 80.0) / 256.0;
			l = 181.0 * l;
			
			u = (l * sin(t)) + 128.0;
			v = (l * cos(t)) + 128.0;

			if (u > 255.0) u = 255.0;
			if (v > 255.0) v = 255.0;			
			if (u < 0.0) u = 0.0;
			if (v < 0.0) v = 0.0;			
			
			rect[1] = u;
			rect[3] = v;
			rect += 4;
		}
	}
}

void mapdown(uchar * rect, int size)
{
	for (; size > 0; size --) {
		rect[1] = (rect[1] * 166) >> 8;
		rect[2] = (rect[2] * 166) >> 8;
		rect[3] = (rect[3] * 166) >> 8;
		rect += 4;
	}
}

main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf *ibuf, *ebuf, *tbuf;
	long i, j, k, ftype = 0,doit, newtype, flags, col, r, g, b, cbits = 6, verbose = FALSE, ask_dither = 0;
	short over = TRUE;
	char name[100],*ext, prog[100];
	long pup, ret;
	extern rectcpy();
	
	strcpy(prog, argv[0]);
	
#ifndef STRIP
	if (argc == 1){
		printf("usage: %s [-x|-o] [-v] images\n",argv[0]);
		printf(" -x: add extension (override default)\n");
		printf(" -o: DON'T add extension (override default)\n");
		printf(" -v: print message for each image saved \n");
		exit(0);
	}
	
	if (strstr(prog, "imconv")){
		noport();
		winopen("");
		pup = defpup("Convert to ?%t| Ftype%l| Targa| Iris| Tim| Tim 24%l| Tanx| Hamx %l| Iff | CDi%l| yuv13| test| YUVx");
		ret = dopup(pup);
		freepup(pup);
		switch (ret){
		case -1:
			exit(0);
		case 1:
			strcpy(prog, "ftype");
			break;
		case 2:
			strcpy(prog, "tga");
			break;
		case 3:
			strcpy(prog, "iris");
			break;
		case 4:
			strcpy(prog, "tim");
			break;
		case 5:
			strcpy(prog, "24tim");
			break;
		case 6:
			strcpy(prog, "tanx");
			break;
		case 7:
			strcpy(prog, "hamx");
			break;
		case 8:
			strcpy(prog, "iff");
			break;
		case 9:
			strcpy(prog, "cdi");
			break;
		case 10:
			strcpy(prog, "yuv13");
			break;
		case 11:
			strcpy(prog, "test");
			break;
		case 12:
			strcpy(prog, "yuvx");
			break;
		default:
			printf("menu returnd %d\nAborting\n", ret);
			exit(0);
		}
	}

	if (strstr(prog,"tga")){
		ftype = TGA;
		ext = ".tga";
	} else if (strstr(prog, "iff")){
		ftype = AMI;
		ext = ".iff";
	} else if (strstr(prog, "24tim")){
		ftype = TIM_24;
		ext = ".tim";
		over = TRUE;
	} else if (strstr(prog, "tim")){
		ftype = TIM;
		ext = ".tim";
		over = FALSE;
	} else if (strstr(prog, "cdi")){
		ftype = CDI;
		ext = ".cdi";
	} else if (strstr(prog, "iris")){
		ftype = IMAGIC;
		ext = ".rgb";
	} else if (strstr(prog, "ftype")){
		ftype = 0;
		ext = ".img";
	} else if (strstr(prog, "tanx")){
		ftype = AN_tanx;
		ext = ".tanx";
		over = FALSE;
	} else if (strstr(prog, "hamx")){
		ftype = AN_hamx;
		ext = ".hamx";
		over = FALSE;
	} else if (strstr(prog, "yuvx")){
		ftype = AN_yuvx;
		ext = ".yuvx";
		over = FALSE;
	} else if (strstr(prog, "yuv13")){
		ftype = 'y13';
		ext = ".yuv_a";
		over = FALSE;
	} else if (strstr(prog, "test")){
		ftype = -3;
		ext = ".test";
		over = FALSE;
		ask_dither = TRUE;
	} else {
		printf("Unrecognized filetype in %s\nAborting.\n",argv[0]);
		exit(1);
	}


	j = 1 ;
	i = 1;

	if (argv[1][0] == '-'){
		j ++;
		while (argv[1][i]){
			switch (argv[1][i]){
			case 'x':
				over = FALSE;
				break;
			case 'o':
				over = TRUE;
				break;
			case 'b':
				if (schedctl(NDPRI, 0, NDPLOMIN) == -1) printf("no ndpri \n");
				break;
			case 'v':
				verbose = TRUE;
				break;
			default:
				printf("unrecogized option: %c\n");
				break;
			}
			i++;
		}
	}


	if (ftype == 0) {
		/* vraag om voorbeeld */
		
		name[0] = 0;
		if (argv[j][0] != '/') {
			getwd(name);
			strcat(name, "/");
		}
		strcat(name, argv[j]);
		
		if (fileselect("Select Example", name) == 0) exit(0);
		ebuf = loadiffname(name, IB_rect | IB_test);
		if (ebuf == 0) {
			perror(name);
			exit(1);
		}
		
		/*printf("%d %d %d %d\n", ebuf->cbits, ebuf->depth, ebuf->mincol, ebuf->maxcol);*/
		
		freerectImBuf(ebuf);
		freeplanesImBuf(ebuf);
		
		if (ebuf->cmap != 0 && ebuf->depth < 24) ask_dither = 2;
		if (IS_ham(ebuf)) ask_dither = 0;
		if (IS_tim(ebuf)) ask_dither = 1;
		if (ebuf->ftype == TIM_24) ask_dither = 0;
	}
	
	if (ftype & TIM && ftype > 0 && ftype != TIM_24) ask_dither = 1;
	
	if (ask_dither) {
		noport();
		winopen("");
		pup = defpup("Dither%t| None| 2 x 2  <-| 4 x 4| Floyd| 2+");
		ret = dopup(pup);
		freepup(pup);
		switch (ret){
		case 1:
			setdither('0');
			break;
		case 3:
			setdither('4');
			break;
		case 4:
			setdither('f');
			break;
		case 5:
			setdither('2+');
			break;
		default:
			setdither('2');
			break;
		}
		if (ebuf) {
			if (getdither() == '2+') {
				ebuf->cbits = 6;
			} else {
				if (ask_dither > 1) {
					if ((ebuf->cbits > 6 || ebuf->cbits == 0) && getdither() != 'f') {
						pup = defpup("Cmapbits%t| 6  <-| 5| 4| 3");
						ret = dopup(pup);
						freepup(pup);
						switch (ret){
						case -1:
						case 0:
							break;
						default:
							cbits = 7 - ret;
						}
						ebuf->cbits = cbits;
					}
				}
			}
		}
	}

#else
	j = 1;
	if (strstr(prog, "toiff")) ftype = AMI;
	else if (strstr(prog, "fromiff")) ftype = IMAGIC;
	else {
		printf("Don't know what to do, [from | to] iff\n");
		exit(0);
	}
	ext = "";
	over = TRUE;

	if (argc == 1){
		printf("usage: %s images\n",argv[0]);
		exit(0);
	}
	
#endif
	
	for (i = j; i<argc ; i++){
		switch (ftype) {
			case AMI:
			case TGA:
			case CDI:
			case TIM:
				flags = IB_rect | IB_cmap;
				break;
			default:
				flags = IB_rect;
				break;
		}

		strcpy(name,argv[i]);

		ibuf = loadiffname(name, flags);
		if (ibuf == 0){
			printf("couldn't load: %s\n",argv[i]);
		} else{
			doit = TRUE;
			newtype = ftype;
			
			switch (ftype){
			case IMAGIC:
				if (IS_iris(ibuf)) doit = FALSE;
				if (ibuf->cmap){
					ibuf->depth = 32;
					freecmapImBuf(ibuf);
				}
				break;
			case TGA:
				if (IS_tga(ibuf)) doit = FALSE;
				if (IS_ham(ibuf)){
					ibuf->depth = 24;
					freecmapImBuf(ibuf);
				}
				break;
			case AMI:
				if (IS_amiga(ibuf)) doit = FALSE;
				break;
			case TIM:
				if (IS_tim(ibuf)) doit= FALSE;
				break;
			case TIM_24:
				if (ibuf->ftype == TIM_24) doit= FALSE;
				break;
			case CDI:
				if (IS_cdi(ibuf)) doit = FALSE;
				else{
					if (ibuf->cmap) newtype = CD_rl7;
					else newtype = CD_dyuv;
				}
				break;
			case AN_c233:
			case AN_yuvx:
			case AN_tanx:
				freecmapImBuf(ibuf);
				break;
			case 'y13':
				newtype = TGA;
				setdither('f');

				ibuf->depth = 8;
				ibuf->mincol = 0;
				ibuf->maxcol = 256;
				ibuf->cbits = 6;
				ibuf->ftype = TGA;
				addcmapImBuf(ibuf);
				
				fillyuvcmap(ibuf);
				for (k = 0; k < 256; k++) ibuf->cmap[k] = colcspace(ibuf->cmap[k], rgbyuv);
				
				cspace(ibuf, rgbyuv);
				converttocmap(ibuf);
				
				fillyuvcmap(ibuf);

				flags = IB_rect | IB_cmap;
				break;
			case -3:
				flags = IB_rect | IB_cmap;
				freecmapImBuf(ibuf);
				/*ibuf->ftype = TGA;*/
				newtype = TGA;

				ibuf->depth = 8;
				ibuf->cbits = 6;
				
				addcmapImBuf(ibuf);
				fill666cmap(ibuf);
								
				converttocmap(ibuf);
				ibuf->mincol = 0;
				ibuf->maxcol = 256;
				
				break;
			case -4:
				flags = IB_rect;
				freecmapImBuf(ibuf);
				/*ibuf->ftype = TGA;*/
				cspace(ibuf, rgbyuv);
				dity(ibuf, 1, 4);
				dit2(ibuf, 2, 5);
				dity(ibuf, 3, 4);
				cspace(ibuf, yuvrgb);
				break;
			case 0:
				tbuf = dupImBuf(ebuf);
				tbuf->rect = ibuf->rect;
				tbuf->mall |= IB_rect;
				tbuf->x = ibuf->x;
				tbuf->y = ibuf->y;
				
				ibuf->rect = 0;
				
				if (ibuf->depth == 32) alpha_to_col0(TRUE);
				else alpha_to_col0(FALSE);
				
				freeImBuf(ibuf);
				ibuf = tbuf;
				
				newtype = ibuf->ftype;
				break;
			}
			
			if (doit){
				ibuf->ftype = newtype;
				if (over == FALSE) strcat(name,ext);
				if (saveiff(ibuf, name, flags) == 0){
					printf("error in writing %s\n",name);
					perror(0);
				} else if (verbose) {
					printf("saved %s\n",name);
				}
/*				if (IS_tanx(ibuf)) {
					flipy(ibuf);
					tan_space(ibuf);
					cspace(ibuf, yuvrgb);
					strcat(name, ".out");
					ibuf->ftype = TGA;
					ibuf->depth = 32;
					saveiff(ibuf, name, IB_rect);
				}
*/			} else{
				printf("skipped %s\n",name);
			}
			freeImBuf(ibuf);
		}
	}
}

