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

#include <local/iff.h>
#include <gl/image.h>
#include <gl/device.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <local/Button.h>
#include <fmclient.h>
#include <local/gl_util.h>
#include <math.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>

fmfonthandle helvfont,helv12=0;
long rgbwin = 0, butwin = 0;
short rcent = 90, gcent = 100, bcent = 255, mind = 140, maxd = 180;

struct keyentry
{
	short r, g, b, a;
	short num;
	short sub;
	float fr, fg, fb;
};

extern void rectcpy();

struct keyentry keys[128];
short curkey, maxkey, filters;
int flags;
int intflags;
extern void rectsub2();
extern void rectadd2();

#define CHR_SUBb	0
#define CHR_SUB		(1 << CHR_SUBb)
#define CHR_CLNb	1
#define CHR_CLN		(1 << CHR_CLNb)

#define THEREISADBUF 1

#define FILTUP		1
#define FILTSTD		0
#define FILTDWN		2

/*

zipfork "make yuvrgb > /dev/console"                                                                  
zipfork "chromakey /pics/ahsgrab/0251 /pics/ahsgrab/back_mask_start > /dev/console"                                                                  

*/
/*
 * chromakey: extra buttons
 * sub, filter
 * 
 * ook in file wegschrijven: min & max.
 * 
 */

void make_premul(struct ImBuf *ibuf)
{
	uchar * rect;
	long i, alpha;
	
	rect = (uchar * ) ibuf->rect;
	
	for (i = ibuf->x * ibuf->y; i > 0; i--){
		alpha = rect[0];
		rect[1] = (rect[1] * alpha ) / 255;
		rect[2] = (rect[2] * alpha ) / 255;
		rect[3] = (rect[3] * alpha ) / 255;
		rect += 4;
	}
}



void drawbuttons()
{
	static short dark, light, b1, b2, b3, b4;
	
	if (butwin == 0) {
		prefsize(240, 340);
		butwin = winopen("Buttons");
	
		if(helv12==0) {
			fminit();
			if((helvfont=fmfindfont("Helvetica-Bold")) == 0) exit(1);
			helv12=fmscalefont(helvfont,11.0);
		}
	
		light = findcolor(0,122,122);
		dark = findcolor(0,81,81);
	
		b1 = findcolor(0,20,20);
		b2 = findcolor(120,190,190);
		b3 = findcolor(0,50,50);
		b4 = findcolor(160,240,240);
	
	}

	DefButBlock("DisTape",butwin,helv12,100,1,0);
	DefButCol(1,NORMALDRAW,0,dark,light,WHITE,WHITE,b1,b2,b3,b4);

	winset(butwin);
	color(BLACK); clear();

	BGadd(NUM|CHA,1,			"filt:",	0,0,20,15,  ((char *) &filters) + 1 , 0.0, 5.0);
	BGadd(ROW|CHA,1,			"-",		0,0,10,15,  &filters, 0.0, (float) FILTDWN);
	BGadd(ROW|CHA,1,			"|",		0,0,10,15,  &filters, 0.0, (float) FILTSTD);
	BGadd(ROW|CHA,1,			"+",		0,0,10,15,  &filters, 0.0, (float) FILTUP);
	
	BGnewline();

	if (intflags & THEREISADBUF) BGadd(TOG|LON|BIT|CHR_SUBb,1,	"diff",		0,0,10,15,  &flags);
	BGadd(TOG|LON|BIT|CHR_CLNb,1,	"cln",		0,0,10,15,  &flags);
	BGadd(BUT,99,					"org",		0,0,10,15);
	BGadd(BUT,98,					"calc",		0,0,10,15);
	BGnewline();

	BGadd(BUT,100,"new",	0,0,10,15);
	BGadd(BUT,97,"del",	0,0,10,15);
	BGnewline();
	BGadd(SLI|SHO,101,"cur",	0,0,10,10,	&curkey,0.0,(float) maxkey,0,0);

	BGnewline();
	BGnewline();
	BGadd(SLI|SHO,102,"r",		0,0,10,10,	&(keys[curkey].r),0.0,255.0,0,0);
	BGnewline();
	BGadd(SLI|SHO,102,"g",		0,0,10,10,	&(keys[curkey].g),0.0,255.0,0,0);
	BGnewline();
	BGadd(SLI|SHO,102,"b",		0,0,10,10,	&(keys[curkey].b),0.0,255.0,0,0);

	BGnewline();
	BGnewline();
	BGadd(SLI|SHO,101,"a",		0,0,10,10,	&(keys[curkey].a),0.0,255.0,0,0);
	BGnewline();
	BGadd(SLI|SHO,101,"sub",	0,0,10,10,	&(keys[curkey].sub),0.0,255.0,0,0);

	BGnewline();
	BGnewline();
	BGnewline();
	BGadd(SLI|SHO,101,"min",	0,0,10,10,	&mind,0.0,255.0,0,0);
	BGnewline();
	BGadd(SLI|SHO,101,"max",	0,0,10,10,	&maxd,0.0,255.0,0,0);
	
	BGposition(40,30,160,280);
	BGspacing(4,6);
	BGdirection('d');
	BGdraw();
		
	if (rgbwin) winset(rgbwin);
}


void chromakey_rgbdist(struct ImBuf *ibuf)
{
	long i;
	short a, r, g, b;
	float af;
	long dist, mindist, maxdist, range;
	uchar * rect;
	
	mindist = mind * mind;
	maxdist = maxd * maxd;
	range = maxdist - mindist;
	if (range <= 0) range = 1;
	
	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		b = rect[1] - bcent;
		g = rect[2] - gcent;
		r = rect[3] - rcent;
		dist = (b * b) + (g * g) + (r * r);

		if (dist < mindist) rect[0] = rect[1] = rect[2] = rect[3] = 0;
		else if (dist > maxdist) rect[0] = 255;
		else {
			af = (dist - mindist) / (float) range;
			af = sqrt(af);
			/*rect[0] = a = (255 * (dist - mindist)) / range;*/
			rect[0] = a = 255.0 * af;
			
			b = rect[1]; g = rect[2]; r = rect[3];
			b -= ((255 - a) * bcent) >> 8;
			g -= ((255 - a) * gcent) >> 8;
			r -= ((255 - a) * rcent) >> 8;
			
			if (b & 0x100) {
				if (b < 0) b = 0;
				else b = 255;
			}
			if (g & 0x100) {
				if (g < 0) g = 0;
				else g = 255;
			}
			if (r & 0x100) {
				if (r < 0) r = 0;
				else r = 255;
			}
			
			if (b > a) b = a;
			if (g > a) g = a;
			if (r > a) r = a;
			
			rect[1] = b; rect[2] = g; rect[3] = r;
		}
		rect += 4;
	}
}


void chromakey_minalpha(struct ImBuf *ibuf)
{
	/* berekend wat alpha minimaal moet zijn om deze kleur te bereiken */
	
	long i, j;
	short a, na, kc, sc, diff;
	short b, g, r;
	uchar * rect;
	short key[4];
	
	key[1] = bcent; key[2] = gcent; key[3] = rcent;
	diff = maxd - mind;
	if (diff <= 0) diff = 1;
	
	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		a = 0;
		for (j = 1; j <= 3; j++){
			kc = key[j];
			sc = rect[j];
			if (sc < kc) {
				na = 255 - ((256 * sc) / kc);
			}else if (sc > kc){
				na = (255 * (sc - kc)) / (256 - kc);
			}
			if (na > a) a = na;
		}
		if (a < mind) a = 0;
		else if (a > maxd) a = 255;
		else a = (255 * (a - mind)) / diff;

		rect[0] = a;
		b = rect[1]; g = rect[2]; r = rect[3];
		b -= ((255 - a) * bcent) >> 8;
		g -= ((255 - a) * gcent) >> 8;
		r -= ((255 - a) * rcent) >> 8;
		
		if (b & 0x100) {
			if (b < 0) b = 0;
			else b = 255;
		}
		if (g & 0x100) {
			if (g < 0) g = 0;
			else g = 255;
		}
		if (r & 0x100) {
			if (r < 0) r = 0;
			else r = 255;
		}
		
		if (b > a) b = a;
		if (g > a) g = a;
		if (r > a) r = a;
		
		rect[1] = b; rect[2] = g; rect[3] = r;

		rect += 4;
	}
}


void chromakey(struct ImBuf *ibuf, struct ImBuf *cbuf)
{
	long i, j;
	short a, na, kc, sc, diff;
	short b, g, r, s;
	uchar * rect, * crect;
	short key[4];
	
	diff = maxd - mind;
	if (diff <= 0) diff = 1;

	bcent = keys[0].b;
	gcent = keys[0].g;
	rcent = keys[0].r;
	
	rect = (uchar *) ibuf->rect;
	crect = (uchar *) cbuf->rect;
	
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		a = crect[3]; s = crect[2];

		if (a <= mind) rect[0] = rect[1] = rect[2] = rect[3] = 0;
		else if (a >= maxd) rect[0] = 255;
		else {
			a = (255 * (a - mind)) / diff;
			s = 255 - a;
			b = rect[1]; g = rect[2]; r = rect[3];
			
			b -= (s * bcent) >> 8;
			g -= (s * gcent) >> 8;
			r -= (s * rcent) >> 8;

			if (b > a) b = a;
			if (g > a) g = a;
			if (r > a) r = a;
			
			if (b & 0x100) {
				if (b < 0) b = 0;
				else b = 255;
			}
			if (g & 0x100) {
				if (g < 0) g = 0;
				else g = 255;
			}
			if (r & 0x100) {
				if (r < 0) r = 0;
				else r = 255;
			}
			
			rect[0] = a; rect[1] = b; rect[2] = g; rect[3] = r;
		}
		rect += 4; crect += 4;
	}
}


void phase1(struct ImBuf *ibuf, struct ImBuf *cbuf)
{
	long i;
	short a, diff;
	uchar * rect, * crect;
	
	diff = maxd - mind;
	if (diff <= 0) diff = 1;
	
	rect = (uchar *) ibuf->rect;
	crect = (uchar *) cbuf->rect;
	
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		a = crect[3];

		if (a <= mind) rect[3] = 0;
		else if (a >= maxd) rect[3] = 255;
		else rect[3] = (255 * (a - mind)) / diff;
		rect[1] = rect[2] = rect[3];
		
		rect += 4; crect += 4;
	}
}


void phase2(struct ImBuf *ibuf, struct ImBuf *cbuf)
{
	long i;
	short b, g, r, a, s;
	uchar * rect, * crect;
	
	bcent = keys[0].b;
	gcent = keys[0].g;
	rcent = keys[0].r;
	
	rect = (uchar *) ibuf->rect;
	crect = (uchar *) cbuf->rect;
	
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		a = crect[3];
		if (a == 0) rect[0] = /*rect[1] = rect[2] = rect[3] =*/ 0;
		else if (a == 255) rect[0] = 255;
		else {
			b = rect[1]; g = rect[2]; r = rect[3];
			s = 255 - a;
			
			b -= (s * bcent) >> 8;
			g -= (s * gcent) >> 8;
			r -= (s * rcent) >> 8;
			
			if (b & 0x100) {
				if (b < 0) b = 0;
				else b = 255;
			}
			if (g & 0x100) {
				if (g < 0) g = 0;
				else g = 255;
			}
			if (r & 0x100) {
				if (r < 0) r = 0;
				else r = 255;
			}

/*
			NO MORE PREMUL
			b = (b * a) / 255;
			g = (g * a) / 255;
			r = (r * a) / 255;
*/
			rect[0] = a; rect[1] = b; rect[2] = g; rect[3] = r;
		}
		rect += 4; crect += 4;
	}
}


void dist_to_col_oud(struct ImBuf *ibuf)
{
	long i;
	short a, r, g, b;
	long dist, mindist, maxdist, range;
	uchar * rect;
		
	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		b = rect[1] - bcent;
		g = rect[2] - gcent;
		r = rect[3] - rcent;
		dist = (b * b) + (g * g) + (r * r);
		rect[1] = rect[2] = rect[3] = dist / 0x300;
		rect += 4;
	}
}


void dist_to_col_alpha(struct ImBuf *ibuf)
{
	/* berekend wat alpha minimaal moet zijn om deze kleur te bereiken */
	
	long i, j;
	short a, na, kc, sc;
	uchar * rect;
	short key[4];
	
	key[1] = bcent; key[2] = gcent; key[3] = rcent;
	
	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		a = 0;
		for (j = 1; j <= 3; j++){
			kc = key[j];
			sc = rect[j];
			if (sc < kc) {
				na = 255 - ((256 * sc) / kc);
			}else if (sc > kc){
				na = (255 * (sc - kc)) / (256 - kc);
			}
			if (na > a) a = na;
			rect[j] = na;
		}
		rect[0] = rect[1] = rect[2] = rect[3] = a;
		rect += 4;
	}
}


void dist_to_col_inp(struct ImBuf *ibuf)
{
	/* inproduct met keykleur */
	
	long i, j;
	short a, na, kc, sc;
	float r, g, b;
	float kr, kg, kb, len;
	uchar * rect;
	short key[4];
	
	kb = (bcent - 128.0) / 128.0;
	kg = (gcent - 128.0) / 128.0;
	kr = (rcent - 128.0) / 128.0;
	len = (kb * kb) + (kg * kg) + (kr * kr);
	len = sqrtf(len);
	kb /= len; kg /= len; kr /= len;
	
	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		b = (rect[1] - 128.0) / 128.0;
		g = (rect[2] - 128.0) / 128.0;
		r = (rect[3] - 128.0) / 128.0;

		len = (b * b) + (g * g) + (r * r);
		len = sqrtf(len);
		b /= len; g /= len; r /= len;
		len = (b * kb) + (g * kg) + (r * kr);
		a = (127.0 * len) + 128.0;
		rect[0] = 255 - a;
		rect += 4;
	}
}


void dist_to_col(struct ImBuf *cbuf, struct ImBuf *ibuf)
{
	long i, j;
	float r, g, b;
	float dr, dg, db, len, totf, tota, tots, falloff[128];
	uchar *rect, *crect;

	/* afstanden tot krachtvelden */

	rect = (uchar *) ibuf->rect;
	crect = (uchar *) cbuf->rect;
	
	for (j = 0; j <= maxkey; j++){
		r = keys[j].sub;
		falloff[j] = 1.0 / ((r * r) + (r * r) + (r * r) + 1.0);
	}
	
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		b = rect[1]; g = rect[2]; r = rect[3];
		totf = tota = tots = 0.0;
		
		for (j = 0; j <= maxkey; j++){
			db = keys[j].b - b;
			dg = keys[j].g - g;
			dr = keys[j].r - r;
	
			len = 1.0 / ((db * db) + (dg * dg) + (dr * dr) + 1.0);
			/* sub gebruiken om "radius" in te stellen */

			if (keys[j].sub) {
				if (len < falloff[j]) len = 0;
				else {
					len = (len - falloff[j]);
				}
			}
			
			totf += len;
			tota += keys[j].a * len;
		}
		
		crect[3] = (tota / totf) + 0.5;
		crect[0] = crect[1] = crect[2] = 0;
		rect += 4;
		crect += 4;
	}
}

void filter_r(struct ImBuf *ibuf)
{
	long i;
	uchar * rect;

	rect = (uchar *) ibuf->rect;
	rect += 3;
	for (i = ibuf->y; i > 0; i--){
		filtrow(rect, ibuf->x);
		rect += 4 * ibuf->x;
	}

	rect = (uchar *) ibuf->rect;
	rect += 3;
	for (i = ibuf->x; i > 0; i--){
		filtcolum(rect, ibuf->y, 4 * ibuf->x);
		rect += 4;
	}
}

struct ImBuf * createmask(struct ImBuf * ibuf,  struct ImBuf * cbuf)
{
	struct ImBuf * obuf, *tbuf;
	long i, j;
	uchar * rect1, * rect2;
	
	
	obuf = dupImBuf(ibuf);
	phase1(obuf, cbuf);
	lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);
	
	if (flags & CHR_CLN) {
		tbuf = dupImBuf(obuf);
		
		if (2 * tbuf->y > tbuf->x) {
			tbuf->x *= 2;
			tbuf->y /= 2;
		}
		
		filter_r(tbuf);
		
		rect1 = (uchar *) obuf->rect;
		rect2 = (uchar *) tbuf->rect;
				
		for (i = obuf->x * obuf->y; i > 0 ; i--){
			if (ABS(rect1[3] - rect2[3]) & 0x80) {
				/* GROTE verandering */
				/* gefilterde pixel = 1/4 origineel + 3/4 rest ->
				 * rest = (4 * filt - org ) / 3 */
				 
				j = ((rect2[3] << 2) - rect1[3]) / 3;
				if (j & 0x100){
					if (j < 0) j = 0;
					else j = 255;
				}
				rect1[1] = rect1[2] = rect1[3] = j;
			}
			rect1 += 4; rect2 += 4;
		}
		
		freeImBuf(tbuf);
	}
	
	lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);
	
	for (j = filters & 0xff; j > 0; j--){		/* FILTERUUUH */
		tbuf = dupImBuf(obuf);
		
		if (2 * tbuf->y > tbuf->x) {
			tbuf->x *= 2;
			tbuf->y /= 2;
		}
		
		filter_r(tbuf);

		rect1 = (uchar *) obuf->rect;
		rect2 = (uchar *) tbuf->rect;
		switch (filters >> 8) {
		case FILTUP:
			for (i = obuf->x * obuf->y; i > 0 ; i--){
				if (rect2[3] > rect1[3]) rect1[3] = rect2[3];
				rect1 += 4; rect2 += 4;
			}
			break;
		case FILTDWN:
			for (i = obuf->x * obuf->y; i > 0 ; i--){
				if (rect2[3] < rect1[3]) rect1[3] = rect2[3];
				rect1 += 4; rect2 += 4;
			}
			break;
		default:
			for (i = obuf->x * obuf->y; i > 0 ; i--){
				rect1[3] = rect2[3];
				rect1 += 4; rect2 += 4;
			}
			break;
		}
		
		lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);
		freeImBuf(tbuf);
	}
	
	tbuf = obuf;
	lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);
	
	obuf = dupImBuf(ibuf);
	phase2(obuf, tbuf);
	freeImBuf(tbuf);
	
	return(obuf);
}

void readwrite(file, rw)
int file;
int (*rw)(int fildes, const void *buf, unsigned nbyte);
{
	rw(file, &maxkey, 2);
	rw(file, &keys, (maxkey + 1) * sizeof(struct keyentry));
	rw(file, &flags, 4);
	rw(file, &mind, 2);
	rw(file, &maxd, 2);							
	rw(file, &filters, 2);							
}

chroma(argc,argv)
long argc;
char **argv;
{
	short val;
	long event, actwin, i;
	ulong rgb;
	uchar * rect, *rect1, *rect2;
	struct ImBuf *ibuf = 0, *obuf = 0, *bbuf = 0, *tbuf = 0, *cbuf = 0, *dbuf = 0;
	float r, g, b;
	int file;
	char name[128], imag[128], omag[128];
	extern void rectalphaover();
	extern void rectfill();

	if (argc >= 2) ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) {
		printf("usage: %s inimage\n", argv[0]);
		exit(1);
	}

	if (argc > 2) bbuf = loadiffname(argv[2], IB_rect);

	if (bbuf) {
		/* bereken vershil plaatje */
		dbuf = dupImBuf(ibuf);
		rectoptot(dbuf, bbuf, rectsub2);
		intflags |= THEREISADBUF;
	}
	
	strcpy(name, "/data/");
	strcpy(imag, argv[1]);
	
	prefsize(ibuf->x, ibuf->y);
	rgbwin = winopen("Mask");
	RGBmode();
	gconfig();
	lrectwrite(0, 0, ibuf->x - 1,  ibuf->y - 1,  ibuf->rect);
	
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
	qdevice(F1KEY); qdevice(F2KEY); qdevice(F3KEY); qdevice(F12KEY);
	qdevice(MOUSEX); qdevice(MOUSEY);
	drawbuttons();
	
	while(event = qreadN(&val)){
		switch(event) {
			case F1KEY:
				if (val) {
					if (fileselect("Open Data", name)) {
						file = open(name, O_RDONLY);
						if (file != -1) {
							readwrite(file, read);
							curkey = maxkey;
							if ((intflags & THEREISADBUF) == 0) flags &= ~CHR_SUB;
							drawbuttons();
							close(file);
						}else perror(name);
					}
				}
				break;
			case F2KEY:
				if (val) {
					if (fileselect("Save Data", name)) {
						file=open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
						if (file != -1) {
							readwrite(file, write);
							close(file);
						}else perror(name);
					}
				}
				break;
			case F3KEY:
				if (obuf && val) {
					if (fileselect("Save Image", imag)) {
						obuf->depth = 32;
						saveiff(obuf, imag, IB_rect);
					}
				}
				break;
			case F12KEY:
				if (val) {
					strcpy(omag, imag);
					if (fileselect("Select Output Image", omag)) {
						char ihead[128], itail[128], ohead[128], otail[128], name[128];
						long ipic, opic;
						ushort idig, odig;
						
						ipic = stringdec(imag, ihead, itail, &idig);
						opic = stringdec(omag, ohead, otail, &odig);

						tbuf = ibuf;
						ibuf = 0;
						winset(rgbwin);
						do {
							stringenc(name, ihead, itail, idig, ipic++);
							ibuf = loadiffname(name, IB_rect);
							if (ibuf) {
								lrectwrite(0, 0, ibuf->x - 1,  ibuf->y - 1, ibuf->rect);

								if (obuf) freeImBuf(obuf);
								if (cbuf) freeImBuf(cbuf);
								obuf = cbuf = 0;
								
								cbuf = dupImBuf(ibuf);
								if (flags & CHR_SUB){
									/* nieuwe deltabuf aanmaken */
									
									if (dbuf) freeImBuf(dbuf);
									dbuf = dupImBuf(ibuf);
									rectoptot(dbuf, bbuf, rectsub2);
									dist_to_col(cbuf, dbuf);
								} else dist_to_col(cbuf, ibuf);
								
								winset(rgbwin);
								lrectwrite(0, 0, cbuf->x - 1,  cbuf->y - 1, cbuf->rect);
								
								obuf = createmask(ibuf, cbuf);
								freeImBuf(ibuf);
								make_premul(obuf);
								
								lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);

								obuf->depth = 32;
								obuf->ftype = TGA;
								stringenc(name, ohead, otail, odig, opic++);
								saveiff(obuf, name, IB_rect);
								printf("saved %s\n", name);
							}
						}while(ibuf);

						ibuf = tbuf;
						tbuf = 0;
					}
				}
				break;
			case LEFTMOUSE:
				if (val && actwin == butwin){
					winset(butwin);
					val = DoButtons();
					switch (val) {
						case 1:
							/* void button */
							break;
						case 98:
							if (cbuf) freeImBuf(cbuf);
							cbuf = dupImBuf(ibuf);
							
							if (flags & CHR_SUB) dist_to_col(cbuf, dbuf);
							else dist_to_col(cbuf, ibuf);
							
							winset(rgbwin);
							lrectwrite(0, 0, cbuf->x - 1,  cbuf->y - 1, cbuf->rect);
							break;
						case 99:
							winset(rgbwin);
							if (flags & CHR_SUB) lrectwrite(0, 0, dbuf->x - 1, dbuf->y - 1, dbuf->rect);
							else lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
							break;
						case 97:
							if (maxkey){
								memcpy(keys + curkey, keys + curkey + 1, (maxkey - curkey) * sizeof(struct keyentry));
								maxkey--;
								if (curkey > maxkey) curkey = maxkey;
								drawbuttons();
							}
							break;
						case 100:
							maxkey++;
							memset(keys + maxkey, 0, sizeof(struct keyentry));
							curkey = maxkey;
						case 101:
							drawbuttons();
							break;
						case 102:
							keys[curkey].num = 10;
							keys[curkey].fr = keys[curkey].r;
							keys[curkey].fg = keys[curkey].g;
							keys[curkey].fb = keys[curkey].b;
							break;
					}
					break;
				}
			case MOUSEY:
				if (actwin == rgbwin && (qualN & LMOUSE)){
					readdisplay(mousexN, mouseyN, mousexN, mouseyN, &rgb, 0);
					r = keys[curkey].fr * keys[curkey].num;
					g = keys[curkey].fg * keys[curkey].num;
					b = keys[curkey].fb * keys[curkey].num;
					r += (rgb >>  0) & 0xff;
					g += (rgb >>  8) & 0xff;
					b += (rgb >> 16) & 0xff;
					keys[curkey].num++;
					r /= keys[curkey].num;
					g /= keys[curkey].num;
					b /= keys[curkey].num;
					keys[curkey].fr = r;
					keys[curkey].fg = g;
					keys[curkey].fb = b;					

					keys[curkey].r = r + 0.5;
					keys[curkey].g = g + 0.5;
					keys[curkey].b = b + 0.5;
					SetButs(102, 102);
				}
				break;
			case MIDDLEMOUSE:
				if (val) {
				}
				break;
			case RIGHTMOUSE:
				if (val) {
					winset(rgbwin);
					if (cbuf == 0) {
						cbuf = dupImBuf(ibuf);
						if (flags & CHR_SUB) dist_to_col(cbuf, dbuf);
						else dist_to_col(cbuf, ibuf);
						lrectwrite(0, 0, cbuf->x - 1,  cbuf->y - 1, cbuf->rect);
					}
					if (obuf) freeImBuf(obuf);
					obuf = createmask(ibuf, cbuf);
					lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);

					if (bbuf) tbuf = dupImBuf(bbuf);
					else {
						tbuf = dupImBuf(obuf);
						/*rectoptot(tbuf, 0, rectfill, 0xff000000);*/
						rectoptot(tbuf, 0, rectfill, 0x00000000);
					}
					rectoptot(tbuf, obuf, rectalphaover);
					freeImBuf(obuf);
					obuf = tbuf;
					lrectwrite(0, 0, obuf->x - 1,  obuf->y - 1, obuf->rect);
				} break;
			case REDRAW:
				drawbuttons();
				winset(rgbwin);
				if (obuf) lrectwrite(0, 0, obuf->x - 1, obuf->y - 1,  obuf->rect);
				else lrectwrite(0, 0, ibuf->x - 1,  ibuf->y - 1,  ibuf->rect); break; case
			INPUTCHANGE:
				actwin = val;
				break;
		}
	}
}


int exist(name)
char *name;
{
    struct stat st;

    if (stat(name,&st)) return(0);
    return(st.st_mode);
}

void diff_limit(ibuf1, ibuf2, lim)
struct ImBuf *ibuf1, *ibuf2;
long lim;
{
	short tab[512];
	uchar * rect1, * rect2;
	long db, dg, dr;
	long i, j;
	
	if (ibuf1 == 0) return;
	if (ibuf1->rect == 0) return;
	if (ibuf2 == 0) return;
	if (ibuf2->rect == 0) return;
	if (lim <= 0) return;
	
	/* tabel genereren */
	
	for (i = 511 ; i >= 0 ; i--) tab[i] = 0;
	for (i = 1; i <= (lim + 1) >> 1; i ++){
		tab[256 + i] = i;
		tab[256 - i] = -i;
	}
	for (j = lim >> 1; j > 0; j--){
		tab[256 + i] = j;
		tab[256 - i] = -j;
		i++;
	}
	
	/* zodat nul ook meetelt, maar geen verandering veroorzaakt */
	tab[256] = 0x100;
	
 	rect1 = (uchar *) ibuf1->rect;
	rect2 = (uchar *) ibuf2->rect;
	for (i = ibuf1->x * ibuf1->y; i > 0; i--){
		if (db = tab[256 + rect2[1] - rect1[1]]) {
			if (dg = tab[256 + rect2[2] - rect1[2]]) {
				if (dr = tab[256 + rect2[3] - rect1[3]]) {
					rect1[1] += db;
					rect1[2] += dg;
					rect1[3] += dr;
				}
			}
		}
		rect1 += 4;
		rect2 += 4;	
	}
}


void temp_filt(ibuf1, obuf, ibuf2)
struct ImBuf * ibuf1, *obuf, *ibuf2;
{
	long i;
	long erra, errb, errg, errr, c;
	uchar * rect1, * recto, * rect2;
	
	rect1 = (uchar *) ibuf1->rect;
	rect2 = (uchar *) ibuf2->rect;
	recto = (uchar *) obuf->rect;
	
	erra = errb = errg = errr = 2;
	for (i = obuf->x * obuf->y; i > 0; i--){
	
		c = (recto[0] << 1) + rect1[0] + rect2[0] + erra;
		erra = c & 0x3;
		recto[0] = c >> 2;
		
		c = (recto[1] << 1) + rect1[1] + rect2[1] + errb;
		errb = c & 0x3;
		recto[1] = c >> 2;
		
		c = (recto[2] << 1) + rect1[2] + rect2[2] + errg;
		errg = c & 0x3;
		recto[2] = c >> 2;
		
		c = (recto[3] << 1) + rect1[3] + rect2[3] + errr;
		errr = c & 0x3;
		recto[3] = c >> 2;
		
		rect1 += 4; recto += 4; rect2 += 4;
	}
}


grabopt(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256];
	ushort indig, outdig;
	long inpic, outpic, limit = 7, tga = FALSE;
	struct ImBuf * ibuf[3], * obuf, *fbuf;
	short last = FALSE;
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [limit]\n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);
	if (argc > 3) {
		limit = atoi(argv[3]);
		printf("using limit %d\n", limit);
	}
	if (argc > 4) tga = TRUE;
	
	stringenc(name, inhead, intail, indig, inpic++);
	ibuf[0] = loadiffname(name, IB_rect);
	if (ibuf[0] == 0) {
		perror(name);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	
	do {
		stringenc(name, inhead, intail, indig, inpic++);
		ibuf[2] = loadiffname(name, IB_rect);
		if (ibuf[2] == 0) {
			ibuf[2] = dupImBuf(ibuf[1]);
			last = TRUE;
		}
		
		obuf = dupImBuf(ibuf[1]);
		fbuf = dupImBuf(ibuf[1]);
		
		temp_filt(ibuf[0], obuf, ibuf[2]);
		diff_limit(fbuf, obuf, limit);
		
		/*cspace(fbuf, yuvrgb);*/
		if (tga) fbuf->ftype = TGA;
		
		stringenc(name, outhead, outtail, outdig, outpic++);
		if (saveiff(fbuf, name, IB_rect) == 0){
			perror(name);
			exit(3);
		}
		printf("saved %s\n", name);
		
		freeImBuf(obuf);
		freeImBuf(fbuf);
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = 0;
	} while (last == FALSE);
}


to23(long argc, char **argv)
{
	char name1[256], name2[256], inhead[256], intail[256], outhead[256], outtail[256];
	char cmd[512];
	ushort indig, outdig;
	long inpic, outpic;
	struct ImBuf * ibuf1, * ibuf2;
	short last = FALSE;
	
	if (argc < 3) {
		printf("usage: %s inimage outimage \n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);
	
	do {
		stringenc(name1, inhead, intail, indig, inpic++);
		ibuf1 = loadiffname(name1, IB_rect);
		if (ibuf1 == 0) exit(0);
		
		stringenc(name2, outhead, outtail, outdig, outpic++);
		sprintf(cmd, "cp %s %s", name1, name2);
		system(cmd);
		
		stringenc(name1, inhead, intail, indig, inpic++);
		ibuf2 = loadiffname(name1, IB_rect);
		if (ibuf2 == 0) exit(0);
		
		rectopodd(ibuf1, ibuf2, rectcpy);
		stringenc(name2, outhead, outtail, outdig, outpic++);
		if (saveiff(ibuf1, name2, IB_rect) == 0){
			perror(name1);
			exit(0);
		}

		stringenc(name2, outhead, outtail, outdig, outpic++);
		sprintf(cmd, "cp %s %s", name1, name2);
		system(cmd);

		freeImBuf(ibuf1);
		freeImBuf(ibuf2);
	} while (last == FALSE);
}


main_(argc,argv)
long argc;
char **argv;
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256];
	ushort indig, outdig;
	long inpic, outpic, event;
	struct ImBuf * ibuf[3], * obuf, *fbuf;
	short last = FALSE, lim = 5, val, redraw;
	
	if (argc < 3) {
		printf("usage: %s inimage outimage \n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);

	stringenc(name, inhead, intail, indig, inpic++);
	ibuf[0] = loadiffname(name, IB_rect);

	if (ibuf[0] == 0){
		perror(name);
		exit(2);
	}
	prefsize(ibuf[0]->x, ibuf[0]->y);
	winopen("");
	RGBmode();
	gconfig();
	cpack(0); clear();

	stringenc(name, inhead, intail, indig, inpic++);
	ibuf[1] = loadiffname(name, IB_rect);
	stringenc(name, inhead, intail, indig, inpic++);
	ibuf[2] = loadiffname(name, IB_rect);
	
	if (ibuf[0] == 0 || ibuf[1] == 0 || ibuf[2] == 0) {
		perror(name);
		exit(2);
	}

	qdevice(PADPLUSKEY);
	qdevice(PADMINUS);
	
	do {
		obuf = dupImBuf(ibuf[1]);
		fbuf = dupImBuf(ibuf[1]);
		
		temp_filt(ibuf[0], obuf, ibuf[2]);
		diff_limit(fbuf, obuf, lim);
		lrectwrite(0, 0, fbuf->x - 1, fbuf->y - 1, fbuf->rect);
		printf("%d\n", lim);
		freeImBuf(obuf);
		freeImBuf(fbuf);

		redraw = FALSE;
		while (redraw == FALSE) {
			event = qread(&val);
			if (val){
				redraw = TRUE;
				switch(event){
				case PADPLUSKEY:
					lim++;
					break;
				case PADMINUS:
					if (lim) lim --;
					break;
				}
			}
		}
	} while (last == FALSE);
}


mr_scan(argc,argv)
long argc;
char **argv;
{
	ulong * mem;
	uchar * bmem;
	short x = 5, y, val, offs = 1, r, g, b;
	long event, i;
	ulong  c, d;
	struct ImBuf * ibuf = 0;
	
	if (argc < 2) exit(0);
	bmem = load_to_mem(argv[1]);
	
	prefsize(512, 512);
	
	winopen("");
	for (i = 0; i < 256 * 8; i++) {
		r = g = b = i >> 3;
		
		if (i & 1) b++;
		if (i & 2) r++;
		if (i & 4) g++;
		if (b > 255) b = 255;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		mapcolor(i + 2048, r, g, b);
	}
	
	RGBmode();
	gconfig();

	qdevice(PADMINUS);
	qdevice(PADPLUSKEY);
	qdevice(SPACEKEY);
	qdevice(AKEY);
	rectzoom(2.0, 2.0);
	
	do{
/*
		for (i = 0; i < 98324; i += 6){			

			c = bmem[i + 0]; c &= 0xf;
			d = bmem[i + 0]; d >>= 4; d &= 0xf;

			c += d << 4;
			c <<= 3;
			mem[(4 * (i / 6)) + 0] = (c << 16) + (c << 8) + (c << 0);


			c = bmem[i + 3]; c >>= 4; c &= 0xf;
			d = bmem[i + 2]; d &= 0xf;

			c += d << 4;
			c <<= 3;
			mem[(4 * (i / 6)) + 1] = (c << 16) + (c << 8) + (c << 0);
			

			c = bmem[i + 5]; c &= 0xf;
			d = bmem[i + 5]; d >>= 4; d &= 0xf;

			c += d << 4;
			c <<= 3;
			mem[(4 * (i / 6)) + 2] = (c << 16) + (c << 8) + (c << 0);


			c = bmem[i + 4]; c >>= 4; c &= 0xf;
			d = bmem[i + 7]; d &= 0xf;

			c += d << 4;
			c <<= 3;
			mem[(4 * (i / 6)) + 3] = (c << 16) + (c << 8) + (c << 0);
		}
*/
		if (ibuf == 0) {
			ibuf = allocImBuf(256, 256, 8, IB_rect, 0);
			ibuf->ftype = TGA;
		}
		mem = ibuf->rect;
		
		for (i = 20; i < 98324; i += 6){
			c = bmem[i + 1]; c >>= 4; c &= 0xf;
			d = bmem[i + 0]; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 0] = (c << 16) + (c << 8) + (c << 0);
			

			c = bmem[i + 3]; c &= 0xf;
			d = bmem[i + 3]; d >>= 4; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 1] = (c << 16) + (c << 8) + (c << 0);


			c = bmem[i + 2]; c >>= 4; c &= 0xf;
			d = bmem[i + 5]; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 2] = (c << 16) + (c << 8) + (c << 0);
					

			c = bmem[i + 4]; c &= 0xf;
			d = bmem[i + 4]; d >>= 4; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 3] = (c << 16) + (c << 8) + (c << 0);
		}
		
		y = (256 * 256) / 256;
		lrectwrite(0, 0, 256 - 1, y - 1, mem);
		event = qread(&val);
		switch(event) {
		case PADMINUS:
			if (val) x--;
			break;
		case PADPLUSKEY:
			if (val) x++;
			break;
		case SPACEKEY:
			if (val) {
				printf("x:%d offs:%d\n", x, offs);
				saveiff(ibuf, "/data/rt", IB_rect);
			}
			break;
		case AKEY:
			if (val) offs++;
			if (offs >= 12) offs = 0;
		}
	}while (1);
}



mr_to_tga_erik(argc,argv)
long argc;
char **argv;
{
	ulong * mem;
	uchar * bmem = 0;
	long i, j;
	ulong  c, d;
	struct ImBuf * ibuf = 0;
	char name[128];
	
	if (argc < 2) {
		printf("%s: im1 im2 ... imn\n", argv[0]);
		exit(1);
	}
	
	prefsize(512, 512);
	
	winopen("");
	RGBmode();
	gconfig();
	rectzoom(2.0, 2.0);
	
	ibuf = allocImBuf(256, 256, 8, IB_rect, 0);
	ibuf->ftype = TGA;

	for (j = 1; j < argc; j++){
		if (bmem) freeN(bmem);
		bmem = load_to_mem(argv[j]);
		if (bmem == 0) exit(0);
		
		mem = ibuf->rect;
		for (i = 20; i < 98324; i += 6){
			c = bmem[i + 1]; c >>= 4; c &= 0xf;
			d = bmem[i + 0]; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 0] = (c << 16) + (c << 8) + (c << 0);
			

			c = bmem[i + 3]; c &= 0xf;
			d = bmem[i + 3]; d >>= 4; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 1] = (c << 16) + (c << 8) + (c << 0);


			c = bmem[i + 2]; c >>= 4; c &= 0xf;
			d = bmem[i + 5]; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 2] = (c << 16) + (c << 8) + (c << 0);
					

			c = bmem[i + 4]; c &= 0xf;
			d = bmem[i + 4]; d >>= 4; d &= 0xf;

			c += d << 4; c <<= 3; if (c > 255) c = 255;
			mem[(4 * ((i - 20) / 6)) + 3] = (c << 16) + (c << 8) + (c << 0);
		}
		
		lrectwrite(0, 0, 256 - 1, 256 - 1, mem);
		strcpy(name, argv[j] + 12);
		if (strlen(name)) name[strlen(name) - 1] = 0;
		flipy(ibuf);
		saveiff(ibuf, name, IB_rect);
	}
}


mr_to_tga(argc,argv)
long argc;
char **argv;
{
	ulong * rect;
	uchar * bmem = 0;
	long i, j;
	ulong  c, d;
	struct ImBuf * ibuf = 0;
	char name[128];
	
	if (argc < 2) {
		printf("%s: im1 im2 ... imn\n", argv[0]);
		exit(1);
	}
	
	prefsize(512, 512);
	
	winopen("");
	RGBmode();
	gconfig();
	rectzoom(2.0, 2.0);
	
	ibuf = allocImBuf(256, 256, 8, IB_rect, 0);
	ibuf->ftype = TGA;

	for (j = 1; j < argc; j++){
		if (bmem) freeN(bmem);
		bmem = load_to_mem(argv[j]);
		if (bmem == 0) exit(0);
		
		rect = ibuf->rect;
		for (i = 8192; i < 139264; i += 2){
			c = (bmem[i] >> 1) | (bmem[i + 1] << 7);
			if (c > 255) c = 255;
			*rect++ = 0x10101 * c;
		}
		
		lrectwrite(0, 0, 256 - 1, 256 - 1, ibuf->rect);
		strcpy(name, argv[j]);
		strcat(name, ".tga");
		
		flipy(ibuf);
		saveiff(ibuf, name, IB_rect);
	}
}


void ibufadd(struct ImBuf * dest, struct ImBuf * src)
{
	uchar *rect1, *rect2;
	long i, col;
	
	rect1 = (uchar *) dest->rect;
	rect2 = (uchar *) src->rect;
	
	for (i = src->x * src->y * 4; i > 0; i--){
		col = *rect1 + *rect2++ - 128;
		if (col < 0) col = 0;
		if (col > 255) col = 255;
		*rect1++ = col;
	}
}

void temp(long argc, char **argv)
{
	struct ImBuf * ibuf, *tbuf;
	uchar * cmap;
	long i, j;

	if (argc > 1) tbuf = loadiffname(argv[1], IB_planes | IB_cmap);
	if (tbuf == 0) exit(0);

	ibuf = dupImBuf(tbuf);
	j = ibuf->maxcol * 4;
	cmap = (uchar *) ibuf->cmap;
	for (i = 0; i < j; i++) cmap[i] = (cmap[i] & 0xfc);
	saveiff(ibuf, "/data/0001",IB_planes | IB_cmap);
	ibuf = dupImBuf(tbuf);

	ibuf = dupImBuf(tbuf);
	j = ibuf->maxcol * 4;
	cmap = (uchar *) ibuf->cmap;
	for (i = 0; i < j; i++) cmap[i] = (cmap[i] & 0xfc) + (cmap[i] >> 6);
	saveiff(ibuf, "/data/0002",IB_planes | IB_cmap);

	ibuf = dupImBuf(tbuf);
	j = ibuf->maxcol * 4;
	cmap = (uchar *) ibuf->cmap;
	for (i = 0; i < j; i++) cmap[i] = (cmap[i] & 0xfc) + 3;
	saveiff(ibuf, "/data/0003",IB_planes | IB_cmap);

	ibuf = dupImBuf(tbuf);
	j = ibuf->maxcol * 4;
	cmap = (uchar *) ibuf->cmap;
	for (i = 0; i < j; i++) cmap[i] = (cmap[i] & 0xf0) + (cmap[i] >> 4);
	saveiff(ibuf, "/data/0004",IB_planes | IB_cmap);
}


#define RATE 18

void mond(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256];
	struct ImBuf * ib128, *ibadd, *ibuf, *tbuf, *obuf;
	ushort indig, outdig;
	long inpic, outpic, i, started = FALSE;
	long ending = 2 * RATE;
	float fl;
	extern rectblend();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage\n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);

	ib128 = loadiffname("/data/mond/voorb/128", IB_rect);
	ibadd = loadiffname("/data/mond/voorb/sub2", IB_rect);

	i = 0;
	do{
		stringenc(name, inhead, intail, indig, inpic);
		if (started) inpic++;
		ibuf = loadiffname(name, IB_rect);

		if (ibuf == 0) {
			if (ending > 0 && obuf != 0) {
				ibuf = dupImBuf(obuf);
				ending--;
			}
		}

		if (ibuf) {
			if (obuf) freeImBuf(obuf);
			obuf = dupImBuf(ibuf);

			printf("loaded %s\n", ibuf->name);
			if (i >= 2 * RATE) i -= 2 * RATE;
			fl = i / (float) RATE ; if (fl > 1.0) fl = 2.0 - fl;

			tbuf = dupImBuf(ib128);
			rectoptot(tbuf, ibadd, rectblend, *((long *) &fl));
			ibufadd(ibuf, tbuf);
			freeImBuf(tbuf);
			
			stringenc(name, outhead, outtail, outdig, outpic++);
			saveiff(ibuf, name, IB_rect);
			printf("saved %s\n\n", name);
			freeImBuf(ibuf);
		}
		i++;
		if (i > 2 * RATE) break;
	}while (ibuf);
}


void copyalpha(long argc, char **argv)
{
	struct ImBuf * ibuf1, *ibuf2;
	uchar * rect1,  *rect2;
	long i;
	
	if (argc < 4) {
		printf("%s: in_rgb in_a out_rgba\n", argv[0]);
		exit(0);
	}
	
	ibuf1 = loadiffname(argv[1], IB_rect);
	ibuf2 = loadiffname(argv[2], IB_rect);
	
	if (ibuf1 == 0 || ibuf2 == 0) exit(0);
	
	rect1 = (uchar *) ibuf1->rect;
	rect2 = (uchar *) ibuf2->rect;
	
	for (i = ibuf1->x * ibuf1->y; i > 0; i--){
		rect1[0] = rect2[0];
		/*if (rect1[0] == 0) rect1[1] = rect1[2] = rect1[3] = 0;*/
		rect1 +=4; rect2 += 4;
	}
	
	ibuf1->depth = 32;
	freecmapImBuf(ibuf1);
	
	saveiff(ibuf1, argv[3], IB_rect);
}


void col_to_alpha(long argc, char **argv)
{
	struct ImBuf * ibuf1, *ibuf2;
	uchar * rect1,  *rect2;
	long i, alpha;
	
	if (argc < 4) {
		printf("%s: inim.rgb inim.r outim.rgba\n", argv[0]);
		exit(0);
	}
	
	ibuf1 = loadiffname(argv[1], IB_rect);
	ibuf2 = loadiffname(argv[2], IB_rect);
	
	if (ibuf1 == 0 || ibuf2 == 0) exit(0);
	
	rect1 = (uchar *) ibuf1->rect;
	rect2 = (uchar *) ibuf2->rect;
	
	for (i = ibuf1->x * ibuf1->y; i > 0; i--){
		alpha = rect2[3];
		rect1[0] = alpha;
		rect1[1] = rect1[1] * alpha / 255;
		rect1[2] = rect1[2] * alpha / 255;
		rect1[3] = rect1[3] * alpha / 255;
		rect1 +=4; rect2 += 4;
	}
	
	ibuf1->depth = 32;
	freecmapImBuf(ibuf1);
	
	saveiff(ibuf1, argv[3], IB_rect);
}


void cspac(long argc, char **argv)
{
	struct ImBuf * ibuf;
	
	ibuf = loadiffname(argv[1], IB_rect);
	cspace(ibuf, yuvrgb);
	saveiff(ibuf, argv[2], IB_rect);
}

void cspac2(long argc, char **argv)
{
	struct ImBuf * ibuf;
	
	ibuf = loadiffname(argv[1], IB_rect);
	cspace(ibuf, rgbyuv);
	saveiff(ibuf, argv[2], IB_rect);
}

void mask(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256];
	ushort indig, outdig;
	long inpic, outpic, i;
	struct ImBuf * ibuf, * obuf, *fbuf;
	ulong * rect1, col;
	uchar * rect2;
	short val;
		
	if (argc < 4) {
		printf("usage: %s inimage outimage mask\n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);

	fbuf = loadiffname(argv[3], IB_rect);
	if (fbuf == 0) exit(3);

	if (argc == 5) {
		prefsize(fbuf->x, fbuf->y);
		winopen("mask");
		RGBmode();
		gconfig();
	}
	
	do {
		stringenc(name, inhead, intail, indig, inpic++);
		ibuf = loadiffname(name, IB_rect);
		if (ibuf == 0) break;
		if (ibuf->cmap) col = ibuf->cmap[0];
		else col = 0;
		
		rect1 = ibuf->rect;
		rect2 = (uchar *) fbuf->rect;
		
		for (i = ibuf->x * ibuf->y; i > 0; i--){
			if (rect2[0]  > 128) *rect1 = col;
			rect1++;
			rect2 += 4;
		}
		if (argc == 5){
			lrectwrite(0, 0, ibuf->x - 1, ibuf->y -1,  ibuf->rect);
			while(qtest()) qread(&val);
		}
		
		stringenc(name, outhead, outtail, outdig, outpic++);
		if (saveiff(ibuf, name, IB_rect) == 0){
			perror(name);
			exit(4);
		}
		
		freeImBuf(ibuf);
		printf("saved %s\n", name);
		
	} while (ibuf);
}

void scalecdi(long argc, char ** argv)
{
	char dir[256], name[256];
	long i, newx = 384, newy = 280, first = 1, convert = FALSE, dofilt = TRUE, crop = FALSE, nocmap = FALSE, halve = FALSE, doubl = FALSE;
	struct ImBuf * ibuf, * tbuf;
	
	strcpy(dir, "");
	
	/* kijk of laatste entry een directory is */
	if (argc > 2) {
		if (S_ISDIR(exist(argv[argc-1]))) {
			strcpy(dir, argv[argc-1]);
			strcat(dir, "/");
			argc --;
		}
		while ((argv[first])[0] == '-'){
			switch ((argv[first])[1]){
				case 'c':
					crop = TRUE;
					break;
				case 'x':
					newx = atoi(argv[first + 1]);
					first++;
					break;
				case 'y':
					newy = atoi(argv[first + 1]);
					first++;
					break;
				case 'h':
					halve = TRUE;
					break;
				case 'd':
					doubl = TRUE;
					break;
				case 'r':
					convert= TRUE;
					break;
				case 'n':
					dofilt = FALSE;
					break;
				case '3':
					nocmap = TRUE;
					break;
				default:
					printf("unknown option: %s\n", argv[first]);
					argc = 1;
					break;
			}
			first++;
			
			if (first >= argc) {
				printf("x: %d y: %d c: %d\n", newx,  newy, convert);
				break;
			}
		}
	}

	if (argc == 1) {
		printf("%s: [-rgb] [-crop] [-halve] [-double] [-x nnn] [-y nnn] inpics [output dir] \n", argv[0]);
		printf("  -rgb    : convert from yuv to rgb\n");
		printf("  -crop   : remove videoframer lines\n");
		printf("  -halve  : halve image\n");
		printf("  -double : double image\n");
		printf("  -x nnn  : newx (default 384)\n");
		printf("  -x 0    : newx is calculated from aspectratio\n");
		printf("  -y nnn  : newy (default 280)\n");
		printf("  -y 0    : newy is calculated from aspectratio\n");
		printf("  -n      : don't filter image\n");
		printf("  -32     : make image 32 bits\n");
		exit(0);
	}
	
	for (i = first; i < argc; i++){
		ibuf = loadiffname(argv[i], IB_rect);
		if (ibuf == 0) {
			printf("skipping %s\n", argv[i]);
		} else {
			printf("loaded %s\n", argv[i]);
			if (nocmap) {
				freecmapImBuf(ibuf);
				ibuf->depth = 32;
			}
			if (crop) {
				tbuf = allocImBuf(ibuf->x - 24, ibuf->y - 12, ibuf->depth, IB_rect, 0);
				rectop(tbuf, ibuf, 0, 0, 12, 6, 32767, 32767, rectcpy);
				freeImBuf(ibuf);
				ibuf = tbuf;
			}
			if (convert) cspace(ibuf, yuvrgb);
			if (dofilt) filter(ibuf);
			if (halve) {
				tbuf = onehalf(ibuf);
				freerectImBuf(ibuf);
				ibuf->x = tbuf->x;
				ibuf->y = tbuf->y;
				ibuf->rect = tbuf->rect;
				tbuf->rect = 0;
				freeImBuf(tbuf);
			} else if (doubl) {
				scaleImBuf(ibuf, ibuf->x * 2, ibuf->y * 2);				
			} else if (newx == 0) {
				scaleImBuf(ibuf, ((ibuf->x * newy) / (double) ibuf->y) + 0.5, newy);
			} else if (newy == 0) {
				scaleImBuf(ibuf, newx, ((ibuf->y * newx) / (double) ibuf->x) + 0.5);				
			} else scaleImBuf(ibuf, newx, newy);
			
			strcpy(name, dir);
			strcat(name, argv[i]);
			
			if (saveiff(ibuf, name, IB_rect)) {
				printf("saved %s\n", name);
			}else{
				printf("error while writeing %s\nAborting...\n", name);
				exit(0);
			}
			freeImBuf(ibuf);
		}
	}
}


void rt1(int argc, char **argv)
{
	struct ImBuf *ibuf;
	short val;
	
	if (argc > 1){
		ibuf = loadiffname(argv[1], IB_rect);
		if (ibuf) {
			prefsize(ibuf->x, ibuf->y);
			winopen("");
			RGBmode();
			gconfig();
			cspace(ibuf, rgbbeta);
			dit2(ibuf, 1, 4);
			dit2(ibuf, 2, 5);
			dit2(ibuf, 3, 4);
			cspace(ibuf, yuvrgb);
			while (qread(&val)) lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
		}
	}
}


void deint(int argc, char **argv)
{
	struct ImBuf *ibuf;
	long i;
	uchar * rect;
		
	if (argc > 1){
		ibuf = loadiffname(argv[1], IB_rect);
		if (ibuf) {
			ibuf->x *= 2;
			ibuf->y /= 2;
			saveiff(ibuf, argv[1], IB_rect);
		}
	}
}


void reint(int argc, char **argv)
{
	struct ImBuf *ibuf;
	long i;
	uchar * rect;
		
	if (argc > 1){
		ibuf = loadiffname(argv[1], IB_rect);
		if (ibuf) {
			ibuf->x /= 2;
			ibuf->y *= 2;
			saveiff(ibuf, argv[1], IB_rect);
		}
	}
}

void nop(int argc, char **argv)
{
	struct ImBuf *ibuf;
	long i;
	uchar * rect;
		
	if (argc > 1){
		ibuf = loadiffname(argv[1], IB_rect | IB_cmap);
		if (ibuf) saveiff(ibuf, argv[1], IB_rect | IB_cmap);
	}
}

void tompegusage(char * prog)
{
	printf("usage: %s [-l(lace)] [-y(.yuv)] first_inimage outimage\n", prog);
	exit(1);	
}

void tompeg(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256], * prog;
	ushort indig, outdig;
	long inpic, outpic, i, delace = FALSE, yuv = FALSE, once = FALSE;
	uchar * rect;
	struct ImBuf * ibuf;
	FILE * file;
	
	
	prog = argv[0];
	
	while (argc > 1) {
		if (argv[1][0] == '-') {
			i = 1;
			while (argv[1][i]) {
				switch(argv[1][i]) {
				case 'l':
					delace = TRUE;
					break;
				case 'y':
					yuv = TRUE;
					break;
				case 'o':
					once = TRUE;
					break;
				default:
					printf("unknown option %c\n", argv[1][i]);
					tompegusage(prog);
				}
				i++;
			}
			argc--;
			argv++;
		} else break;
	}
	
	if (argc < 3) tompegusage(prog);
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);

	do {
		stringenc(name, inhead, intail, indig, inpic++);
		ibuf = loadiffname(name, IB_rect);
		if (ibuf == 0) break;
		
		printf("loaded: %s\n", name);
		if (delace) {
			ibuf->x *= 2;
			ibuf->y /= 2;
		}
		
		flipy(ibuf);
		cspace(ibuf, rgbbeta);
		
		if (yuv == FALSE) {
			stringenc(name, outhead, ".Y", 1, outpic);
		} else {
			stringenc(name, outhead, ".yuv", outdig, outpic);
		}
		
		file = fopen(name, "w");
		if (file == 0) exit(0);		
		rect = (uchar *) ibuf->rect;
		rect += 2;
		for(i = ibuf->x * ibuf->y; i > 0; i--){
			if (putc(rect[0], file) == EOF) {
				perror(0);
				break;
			}
			rect += 4;
		}
		if (yuv == FALSE) fclose(file);
		
		scaleImBuf(ibuf, ibuf->x / 2,  ibuf->y / 2);

		if (yuv == FALSE) {
			stringenc(name, outhead, ".U", 1, outpic);
			file = fopen(name, "w");
			if (file == 0) exit(0);
		}
		rect = (uchar *) ibuf->rect;
		rect += 1;
		for(i = ibuf->x * ibuf->y; i > 0; i--){
			if (putc(rect[0], file) == EOF) {
				perror(0);
				break;
			}
			rect += 4;
		}
		if (yuv == FALSE) {
			fclose(file);
			stringenc(name, outhead, ".V", 1, outpic);
			file = fopen(name, "w");
			if (file == 0) exit(0);	
		}
		rect = (uchar *) ibuf->rect;
		rect += 3;
		for(i = ibuf->x * ibuf->y; i > 0; i--){
			if (putc(rect[0], file) == EOF) {
				perror(0);
				break;
			}
			rect += 4;
		}
		fclose(file);

		freeImBuf(ibuf);
		printf("saved %s\n", name);
		outpic++;
		
		if (once) break;
	} while (ibuf);
}


void frommpegusage(char * prog)
{
	printf("usage: %s [-l] xsize ysize inimage outimage\n", prog);
	exit(1);	
}

void frommpeg(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256], * prog;
	ushort indig, outdig;
	long inpic, outpic, i, delace = FALSE;
	uchar * rect;
	struct ImBuf * ibuf;
	long file;
	long xsize, ysize;
	uchar * buf;
	
	prog = argv[0];
	
	if (argc != 1) {
		if (argv[1][0] == '-') {
			i = 1;
			while (argv[1][i]) {
				switch(argv[1][i]) {
				case 'l':
					delace = TRUE;
					break;
				default:
					printf("unknown option %c\n", argv[1][i]);
					frommpegusage(prog);
				}
				i++;
			}
			argc--;
			argv++;
		}
	}
	
	if (argc < 5) frommpegusage(prog);
	
	xsize = atoi(argv[1]);
	ysize = atoi(argv[2]);
	buf = (uchar *) malloc(xsize * ysize);
	
	inpic = stringdec(argv[3], inhead, intail, &indig);
	outpic = stringdec(argv[4], outhead, outtail, &outdig);

	do {
		ibuf = allocImBuf(xsize / 2, ysize / 2, 24, IB_rect, 0);
		
		stringenc(name, inhead, ".U", 1, inpic);
		file = open(name, O_RDONLY);
		if (file == -1) exit(0);
		if (read(file, buf, ibuf->x * ibuf->y) != (ibuf->x * ibuf->y)) {
			perror(0);
			exit(1);
		}
		close(file);
		
		rect = (uchar *) ibuf->rect;
		rect += 1;
		for(i = 0; i < ibuf->x * ibuf->y; i++){
			rect[0] = buf[i];
			rect += 4;
		}

		stringenc(name, inhead, ".V", 1, inpic);
		file = open(name, O_RDONLY);
		if (file == -1) exit(0);
		if (read(file, buf, ibuf->x * ibuf->y) != (ibuf->x * ibuf->y)) {
			perror(0);
			exit(1);
		}
		close(file);
		
		rect = (uchar *) ibuf->rect;
		rect += 3;
		for(i = 0; i < ibuf->x * ibuf->y; i++){
			rect[0] = buf[i];
			rect += 4;
		}

		scaleImBuf(ibuf, ibuf->x * 2,  ibuf->y * 2);

		stringenc(name, inhead, ".Y", 1, inpic);
		file = open(name, O_RDONLY);
		if (file == -1) exit(0);
		if (read(file, buf, ibuf->x * ibuf->y) != (ibuf->x * ibuf->y)) {
			perror(0);
			exit(1);
		}
		close(file);
		
		rect = (uchar *) ibuf->rect;
		rect += 2;
		for(i = 0; i < ibuf->x * ibuf->y; i++){
			rect[0] = buf[i];
			rect += 4;
		}
		printf("loaded: %s\n", name);

		flipy(ibuf);		

		if (delace) {
			ibuf->x /= 2;
			ibuf->y *= 2;
		}

		stringenc(name, outhead, outtail, outdig, outpic);
		if (saveiff(ibuf, name, IB_rect) == 0) exit(0);

		freeImBuf(ibuf);
		printf("saved %s\n", name);
		inpic++;
		outpic++;
		
	} while (ibuf);
}


void allbutblack(long argc, char **argv)
{
	struct ImBuf * ibuf1, * ibuf2;
	ulong * rect1, * rect2;
	long i;

	if (argc < 4) return; 
	ibuf1 = loadiffname(argv[1], IB_cmap | IB_rect);
	ibuf2 = loadiffname(argv[2], IB_cmap | IB_rect);
	if (ibuf1 == 0 || ibuf2 == 0) return;

	rect1 = ibuf1->rect;
	rect2 = ibuf2->rect;

	for (i = ibuf1->x * ibuf1->y - 1; i >= 0; i--) {
		if (ibuf2->cmap[rect2[i]] & 0xffffff) rect1[i] = rect2[i];
	}

	saveiff(ibuf1, argv[3], IB_cmap | IB_rect);
}


void sub2(long argc, char **argv)
{
	struct ImBuf *ibuf1, *ibuf2;
	
	if (argc < 4) {
		printf("sub2: in1 in2 out\n");
		exit(0);
	}
	
	ibuf1 = loadiffname(argv[1], IB_rect);
	if (ibuf1 == 0) {
		printf("%s: not an image ?\n", argv[1]);
		exit(0);
	}
	
	ibuf2 = loadiffname(argv[2], IB_rect);
	if (ibuf2 == 0) {
		printf("%s: not an image ?\n", argv[2]);
		exit(0);
	}
	
	rectoptot(ibuf1, ibuf2, rectsub2);
	saveiff(ibuf1, argv[3], IB_rect);
}


void fieldsplit(long argc, char **argv)
{
	struct ImBuf *ibuf;
	char name[256];
	
	if (argc == 1) {
		printf("fieldsplit: images\n");
		exit(0);
	}
	
	
	while (argc > 1) {
		ibuf = loadiffname(argv[1], IB_rect | IB_cmap);
		if (ibuf == 0) {
			printf("%s: not an image ?\n", argv[1]);
		} else {
			de_interlace(ibuf);
			
			strcpy(name, argv[1]); strcat(name, "a");
			saveiff(ibuf, name, IB_rect | IB_cmap);
			ibuf->rect += ibuf->x * ibuf->y;
			strcpy(name, argv[1]); strcat(name, "b");
			saveiff(ibuf, name, IB_rect | IB_cmap);
			ibuf->rect -= ibuf->x * ibuf->y;
			
			freeImBuf(ibuf);
		}
		argv++;
		argc--;
	}
}

void fieldmerge(long argc, char **argv)
{
	struct ImBuf *ibuf, *fbuf1, *fbuf2;
	char name[256];
	
	if (argc != 4) {
		printf("fieldmerge: in1 in2 out\n");
		exit(0);
	}
	
	fbuf1 = loadiffname(argv[1], IB_rect | IB_cmap);
	fbuf2 = loadiffname(argv[2], IB_rect | IB_cmap);
	
	if (fbuf1 != 0 && fbuf2 != 0) {
		ibuf = allocImBuf(fbuf1->x, 2* fbuf1->y, fbuf1->depth, IB_rect, 0);
		rectop(ibuf, fbuf1, 0, 0, 0, 0, 32767, 32767, rectcpy, 0);;
		rectop(ibuf, fbuf2, 0, fbuf1->y, 0, 0, 32767, 32767, rectcpy, 0);
		ibuf->y /= 2;
		interlace(ibuf);
		ibuf->ftype = fbuf1->ftype;
		
		saveiff(ibuf, argv[3], IB_rect | IB_cmap);
	}
}

void fieldswap(long argc, char **argv)
{
	struct ImBuf *ibuf;
	
	if (argc == 1) {
		printf("fieldswap: images\n");
		exit(0);
	}
	
	
	while (argc > 1) {
		ibuf = loadiffname(argv[1], IB_rect | IB_cmap);
		if (ibuf == 0) {
			printf("%s: not an image ?\n", argv[1]);
		} else {
			rectop(ibuf, ibuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
			saveiff(ibuf, argv[1], IB_rect | IB_cmap);
			printf("saved %s\n", argv[1]);
			freeImBuf(ibuf);
		}
		argv++;
		argc--;
	}
}

#define MAXRMCRAWL 16

void rmcrawl(long argc, char **argv)
{
	struct ImBuf *ibuf[MAXRMCRAWL];
	char name[256];
	short * buf;
	uchar * rect;
	long index, x, y, z, sizex, add;
	
	if (argc == 1) {
		printf("rmcrawl: inimage(2-%d) [outimage]\n", MAXRMCRAWL);
		exit(0);
	}
	
	strcpy(name, argv[1]);
	
	for (index = 0; index < MAXRMCRAWL; index++) {
		ibuf[index] = loadiffname(name, IB_rect);
		if (ibuf[index] == 0) break;
		newname(name, +1);
	}
	
	if (index == 0) {
		printf("noimage: %s\n", name);
		exit(0);
	}
	
	if (index == 1) {
		printf("only one image: aborting\n");
		exit(0);
	}
	
	sizex = ibuf[0]->x * 4;
	add = index / 2;
	buf = mallocstruct(short, sizex);
	for (y = 0; y < ibuf[0]->y; y++) {
		
		for (x = 0; x < sizex; x++) buf[x] = 0;
		
		for (z = 0; z < index; z++) {
			rect = (uchar *) (ibuf[z]->rect + y * ibuf[z]->x);
			for (x = 0; x < sizex; x++) buf[x] += rect[x];
		}
		rect = (uchar *) (ibuf[0]->rect + y * ibuf[0]->x);
		for (x = 0; x < sizex; x++) rect[x] = (buf[x] + add) / index;
	}
	
	if (argc > 2) strcpy(name, argv[2]);
	saveiff(ibuf[0], name, IB_rect);
}


void medsys(long argc, char **argv)
{
	struct ImBuf *ibuf;
	char in[256], out[256];
	short * buf;
	uchar * rect;
	float * array, *fpnt, count = 0.0;
	long i;
	
	
	if (argc < 3) {
		printf("medsys: firstinimage firstoutimage\n", MAXRMCRAWL);
		exit(0);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);
	
	while (ibuf = loadiffname(in, IB_rect)) {
	
		if (array == 0) array = callocstruct(float, ibuf->x * ibuf->y * 4);

		fpnt = array;
		rect = (uchar *) ibuf->rect;
		count += 1.0;
		
		for (i = 4 * ibuf->x * ibuf->y; i > 0; i--) {
			*fpnt += *rect;
			fpnt++;
			rect++;
		}
		
		fpnt = array;
		rect = (uchar *) ibuf->rect;
		
		for (i = 4 * ibuf->x * ibuf->y; i > 0; i--) {
			*rect = (*fpnt / count) + 0.5;
			fpnt++;
			rect++;
		}
		
		saveiff(ibuf, out, IB_rect);
		printf("saved %s\n", out);
		newname(in, +1);
		newname(out, +1);
		freeImBuf(ibuf);	
	}
}


void dvd_tga(long argc, char **argv)
{
	ImBuf *ibuf, *ybuf = 0, *uvbuf = 0;
	char in[256], out[256], tmp[256];
	uchar * rect, *yrect = 0, *uvrect = 0;
	long x, y;
	extern float rgbdyuv[4][4];

	
	if (argc < 2) {
		printf("medsys: firstinimage [firstoutimage]\n", MAXRMCRAWL);
		exit(0);
	}
	
	strcpy(in, argv[1]);
	if (argc > 2) strcpy(out, argv[2]);
	else strcpy(out, argv[1]);
	
	while (ibuf = loadiffname(in, IB_rect)) {
		cspace(ibuf, rgbyuv);
		
		if (ybuf) freeImBuf(ybuf);
		ybuf = allocImBuf(ibuf->x , ibuf->y, 8, IB_rect, 0);

		if (uvbuf) freeImBuf(uvbuf);
		uvbuf = allocImBuf(ibuf->x , ibuf->y, 8, IB_rect, 0);
		
		rect = (uchar *) ibuf->rect;
		yrect = (uchar *) ybuf->rect;
		uvrect = (uchar *) uvbuf->rect;
		
		for (y = 0; y < ibuf->y ; y++){
			for (x = 0; x < ibuf->x ; x++){
				yrect[3] = rect[2];

				if (x & 1) uvrect[3] = rect[3];
				else uvrect[3] = rect[1];

				rect += 4; yrect += 4; uvrect += 4;
			}
		}
		
		strcpy(tmp, out);
		strcat(tmp, ".Y");
		saveiff(ybuf, tmp, IB_rect);

		strcpy(tmp, out);
		strcat(tmp, ".UV");
		saveiff(uvbuf, tmp, IB_rect);
		
		freeImBuf(ibuf);
		printf("processed %s\n", in);
		newname(in, +1);
		newname(out, +1);
	}
}


void dumpimg(char * name, ImBuf * ibuf) {
	int fd,  size;
	
	if (0) {
		saveiff(ibuf, name, IB_rect);
	} else {
		fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd != -1) {
			size = ibuf->x * ibuf->y * 4;
			if (write(fd, ibuf->rect, size) != size) {
				perror(name);
				remove(name);
			}
			close (fd);
		}
	}
}

void dvd(long argc, char **argv)
{
	ImBuf *ibuf, *ybuf = 0, *uvbuf = 0;
	char in[256], out[256], tmp[256];
	uchar * rect, *yrect = 0, *uvrect = 0;
	long x, y;
	extern float rgbdyuv[4][4];

	
	if (argc < 2) {
		printf("dvd: firstinimage [firstoutimage]\n");
		exit(0);
	}
	
	strcpy(in, argv[1]);
	if (argc > 2) strcpy(out, argv[2]);
	else strcpy(out, argv[1]);
	
	while (ibuf = loadiffname(in, IB_rect)) {
		cspace(ibuf, rgbyuv);
		
		if (ybuf) freeImBuf(ybuf);
		ybuf = allocImBuf(ibuf->x / 4, ibuf->y, 32, IB_rect, 0);

		if (uvbuf) freeImBuf(uvbuf);
		uvbuf = allocImBuf(ibuf->x / 4, ibuf->y, 32, IB_rect, 0);
		
		rect = (uchar *) ibuf->rect;
		yrect = (uchar *) ybuf->rect;
		uvrect = (uchar *) uvbuf->rect;
		
		for (y = 0; y < ibuf->y ; y++){
			for (x = 0; x < ibuf->x ; x++){
				* yrect++ = rect[2];

				if (x & 1) *uvrect++ = rect[3];
				else *uvrect++ = rect[1];

				rect += 4;
			}
		}
				
		flipy(ybuf); flipy(uvbuf);
		
/*		rectop(ybuf, ybuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
		rectop(uvbuf, uvbuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
		
*/		strcpy(tmp, out); strcat(tmp, ".Y");
		dumpimg(tmp, ybuf);

		strcpy(tmp, out); strcat(tmp, ".UV");
		dumpimg(tmp, uvbuf);
		
		freeImBuf(ibuf);
		printf("processed %s\n", in);
		newname(in, +1);
		newname(out, +1);
	}
}

void dvd_old(long argc, char **argv)
{
	ImBuf *ibuf, *ybuf = 0, *uvbuf = 0;
	char in[256], out[256], tmp[256];
	uchar * rect, *yrect = 0, *uvrect = 0;
	long x, y;
	extern float rgbdyuv[4][4];

	
	if (argc < 2) {
		printf("dvd: firstinimage [firstoutimage]\n");
		exit(0);
	}
	
	strcpy(in, argv[1]);
	if (argc > 2) strcpy(out, argv[2]);
	else strcpy(out, argv[1]);
	
	while (ibuf = loadiffname(in, IB_rect)) {
		cspace(ibuf, rgbyuv);
		
		if (ybuf) freeImBuf(ybuf);
		ybuf = allocImBuf(ibuf->x / 4, ibuf->y, 32, IB_rect, 0);

		if (uvbuf) freeImBuf(uvbuf);
		uvbuf = allocImBuf(ibuf->x / 4, ibuf->y, 32, IB_rect, 0);
		
		rect = (uchar *) ibuf->rect;
		yrect = (uchar *) ybuf->rect;
		uvrect = (uchar *) uvbuf->rect;
		
		for (y = 0; y < ibuf->y ; y++){
			for (x = 0; x < ibuf->x ; x++){
				* yrect++ = rect[2];

				if (x & 1) *uvrect++ = rect[3];
				else *uvrect++ = rect[1];

				rect += 4;
			}
		}
		
		strcpy(tmp, "bottom_first/flipped_fields/"); strcat(tmp, out); strcat(tmp, ".Y");
		dumpimg(tmp, ybuf);

		strcpy(tmp, "bottom_first/flipped_fields/"); strcat(tmp, out); strcat(tmp, ".UV");
		dumpimg(tmp, uvbuf);
		
		flipy(ybuf); flipy(uvbuf);

		strcpy(tmp, "top_first/flipped_fields/"); strcat(tmp, out); strcat(tmp, ".Y");
		dumpimg(tmp, ybuf);

		strcpy(tmp, "top_first/flipped_fields/"); strcat(tmp, out); strcat(tmp, ".UV");
		dumpimg(tmp, uvbuf);
		
		rectop(ybuf, ybuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
		rectop(uvbuf, uvbuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
		
		strcpy(tmp, "top_first/standard_fields/"); strcat(tmp, out); strcat(tmp, ".Y");
		dumpimg(tmp, ybuf);

		strcpy(tmp, "top_first/standard_fields/"); strcat(tmp, out); strcat(tmp, ".UV");
		dumpimg(tmp, uvbuf);
		
		flipy(ybuf); flipy(uvbuf);

		strcpy(tmp, "bottom_first/standard_fields/"); strcat(tmp, out); strcat(tmp, ".Y");
		dumpimg(tmp, ybuf);

		strcpy(tmp, "bottom_first/standard_fields/"); strcat(tmp, out); strcat(tmp, ".UV");
		dumpimg(tmp, uvbuf);

		freeImBuf(ibuf);
		printf("processed %s\n", in);
		newname(in, +1);
		newname(out, +1);
	}
}


void add2(long argc, char **argv)
{
	struct ImBuf *ibuf1, *ibuf2;

	if (argc < 4){
		printf("add2: in1 in2 out\n");
		exit(0);
	}
	
	ibuf1 = loadiffname(argv[1], IB_rect);
	if (ibuf1 == 0) {
		printf("%s: not an image ?\n", argv[1]);
		exit(0);
	}
	
	ibuf2 = loadiffname(argv[2], IB_rect);
	if (ibuf2 == 0) {
		printf("%s: not an image ?\n", argv[2]);
		exit(0);
	}
	
	rectoptot(ibuf1, ibuf2, rectadd2);
	saveiff(ibuf1, argv[3], IB_rect);
}


void stripalpha(long argc, char **argv)
{
	struct ImBuf *ibuf;
	char * name;
	
	if (argc < 2){
		printf("stripalpha: in1 [out]\n");
		exit(0);
	}
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) {
		printf("%s: not an image ?\n", argv[1]);
		exit(0);
	}
	
	if (ibuf->depth == 32) {
		ibuf->depth = 24;
		
		if (argc == 3) name = argv[2];
		else name = argv[1];
		
		saveiff(ibuf, name, IB_rect);
	} else {
		printf("%s has no alpha\n", argv[1]);
	}
}


void lace(long argc, char **argv)
{
	char name[256], inhead[256], intail[256], outhead[256], outtail[256];
	ushort indig, outdig;
	long inpic, outpic, i;
	struct ImBuf * ibuf1, * ibuf2;
	ulong * rect1, col;
	uchar * rect2;
	short val;
	
	if (argc < 3) {
		printf("usage: %s inimage outimage \n", argv[0]);
		exit(1);
	}
	
	inpic = stringdec(argv[1], inhead, intail, &indig);
	outpic = stringdec(argv[2], outhead, outtail, &outdig);

	do {
		stringenc(name, inhead, intail, indig, inpic++);
		ibuf1 = loadiffname(name, IB_rect);
		if (ibuf1 == 0) break;

		stringenc(name, inhead, intail, indig, inpic++);
		ibuf2 = loadiffname(name, IB_rect);
		if (ibuf2 == 0) break;

		filtery(ibuf1);
		filtery(ibuf2);

		rectopeven(ibuf1, ibuf2, rectcpy);
		
		stringenc(name, outhead, outtail, outdig, outpic++);
		if (saveiff(ibuf1, name, IB_rect) == 0){
			perror(name);
			exit(4);
		}
		
		freeImBuf(ibuf1);
		freeImBuf(ibuf2);
		printf("saved %s\n", name);
	} while (ibuf1);
}


void premul(long argc, char **argv)
{
	char inname[256], outname[256];
	struct ImBuf * ibuf;
	short val;
	extern void rectcpy();
	
	if (argc < 3) {
		printf("usage: %s first_inimage first_outimage \n", argv[0]);
		exit(1);
	}
	
	strcpy(inname, argv[1]);
	strcpy(outname, argv[2]);
	
	do {
		ibuf = loadiffname(inname, IB_rect);
		if (ibuf == 0) break;
		
		printf("Loaded %s\n", inname);
		make_premul(ibuf);
				
		if (ibuf->cmap) {
			freecmapImBuf(ibuf);
			ibuf->depth = 32;
		}
		
		if (saveiff(ibuf, outname, IB_rect) == 0){
			perror(outname);
			exit(4);
		}
		
		freeImBuf(ibuf);
		printf("Saved %s\n", outname);
		
		newname(inname, +1);
		newname(outname, +1);
	} while (ibuf);
}


void demul(long argc, char **argv)
{
	char inname[256], outname[256];
	long i, alpha, col;
	struct ImBuf * ibuf;
	uchar * rect;
	short val;
	extern void rectcpy();
	
	if (argc < 3) {
		printf("usage: %s first_inimage first_outimage \n", argv[0]);
		exit(1);
	}
	
	strcpy(inname, argv[1]);
	strcpy(outname, argv[2]);
	
	do {
		ibuf = loadiffname(inname, IB_rect);
		if (ibuf == 0) break;
		
		printf("Loaded %s\n", inname);
		rect = (uchar * ) ibuf->rect;
		
		for (i = ibuf->x * ibuf->y; i > 0; i--){
			alpha = rect[0];
			if (alpha) {
				col =  (rect[1] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[1] = col;

				col =  (rect[2] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[2] = col;

				col =  (rect[3] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[3] = col;
			}
			rect += 4;
		}
		
		if (ibuf->cmap) {
			freecmapImBuf(ibuf);
			ibuf->depth = 32;
		}

		if (saveiff(ibuf, outname, IB_rect) == 0){
			perror(outname);
			exit(4);
		}
		
		freeImBuf(ibuf);
		printf("Saved %s\n", outname);
		
		newname(inname, +1);
		newname(outname, +1);
	} while (ibuf);
}


changecolor(ibuf, p1)
struct Imbuf * ibuf;
float p1;
{
	long x, y;
	float u,v,s,c,mat1[4][4], mat2[4][4], mat3[4][4];
	long i,j;

	Mat4One(mat1);
	mat1[3][0] = -128.0;
	mat1[3][2] = -128.0;

	Mat4MulMat4(mat2, rgbbeta, mat1);

	s = fsin(p1);
	c = fcos(p1);

	Mat4One(mat1);
	mat1[0][0] = mat1[2][2] = c;
	mat1[0][2] = -s;
	mat1[2][0] = s;

	Mat4MulMat4(mat3, mat2, mat1);

	Mat4One(mat1);
	mat1[3][0] = 128.0;
	mat1[3][2] = 128.0;

	Mat4MulMat4(mat2, mat3, mat1);
	Mat4Invert(mat1,rgbbeta);
	Mat4MulMat4(mat3, mat2, mat1);
	
	cspace(ibuf,mat3);
}


void rotcol(long argc, char **argv)
{
	struct ImBuf * ibuf;
	float f;
		
	if (argc < 4) {
		printf("usage: %s inimage outimage degrees\n", argv[0]);
		exit(1);
	}
	
	f = atof(argv[3]);
	f = (M_PI * f) / 180.0;
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) {
		perror(argv[1]);
		exit(4);
	}
	
	changecolor(ibuf, f);
	
	if (saveiff(ibuf, argv[2], IB_rect) == 0){
		perror(argv[2]);
		exit(4);
	}
	
	freeImBuf(ibuf);
}


void graph(long argc, char **argv)
{
	struct ImBuf * ibuf;
	int dev, x;
	short val;
	float co[2];

	if (argc < 2) {
		printf("usage: %s inimage\n", argv[0]);
		exit(1);
	}
	
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) {
		perror(argv[1]);
		exit(4);
	}
	
	prefsize(ibuf->x, 256);
	winopen(argv[1]);
	RGBmode();
	doublebuffer();
	gconfig();
	cpack(0);
	clear();
	subpixel(TRUE);
	
	qdevice(LEFTMOUSE);
	
	do {
		cpack(0);
		clear();
		cpack(-1);
		bgnline();
		for (x = 0; x < ibuf->x; x++) {
			co[0] = x;
			co[1] = ibuf->rect[x] & 0xff;
			v2f(co);
		}
		endline();
		swapbuffers();
	} while (dev = qread(&val));
}

void gamma1(long argc, char **argv)
{
	struct ImBuf * ibuf;
		
	if (argc < 3) {
		printf("usage: %s gamma inimage [outimage]\n", argv[0]);
		exit(1);
	}
	
	
	ibuf = loadiffname(argv[2], IB_rect);
	if (ibuf == 0) {
		perror(argv[2]);
		exit(4);
	}
	
	
	gamwarp(ibuf, atof(argv[1]));
	
	if (saveiff(ibuf, argv[argc - 1], IB_rect) == 0){
		perror(argv[argc - 1]);
		exit(4);
	}
	
	freeImBuf(ibuf);
}

void gamma2(long argc, char **argv)
{
	struct ImBuf * ibuf;
	int i;
	
	if (argc < 3) {
		printf("usage: %s gamma inimage [outimage]\n", argv[0]);
		exit(1);
	}
	
	
	ibuf = loadiffname(argv[2], IB_rect);
	if (ibuf == 0) {
		perror(argv[2]);
		exit(4);
	}
	
	
	for (i = ibuf->x * ibuf->y - 1; i >= 0; i--) {
		ibuf->rect[i] ^= -1;
	}
	
	gamwarp(ibuf, 1.0 / atof(argv[1]));
	
	for (i = ibuf->x * ibuf->y - 1; i >= 0; i--) {
		ibuf->rect[i] ^= -1;
	}

	if (saveiff(ibuf, argv[argc - 1], IB_rect) == 0){
		perror(argv[argc - 1]);
		exit(4);
	}
	
	freeImBuf(ibuf);
}


void tojst(long argc, char **argv)
{
	struct ImBuf * ibuf;
	long quality, i, start, opt = FALSE;
	long orgx, orgy;
	struct ImBuf * (*scaleimbuf)(struct ImBuf *  , short  , short);
	
	if (argc < 3) {
		printf("usage: %s quality [-quick] inimage [inimage]\n", argv[0]);
		exit(1);
	}
	
	quality = atoi(argv[1]);
	if (quality < 0) {
		opt = TRUE;
		quality = -quality;
	}
	
	scaleimbuf = scaleImBuf;
	
	start = 2;
	while (argv[start][0] == '-') {
		i = 1;
		while (argv[start][i]) {
			switch(argv[start][i]) {
			case 'q':
				scaleimbuf = scalefastImBuf;
				break;
			default:
				printf("unknown option %c\n", argv[1][i]);
				exit(0);
			}
			i++;
		}
		start++;
	}
	
	cspace_err_diff(TRUE);
	
	for (i = start; i < argc; i++) {
		ibuf = loadiffname(argv[i], IB_rect);
		if (ibuf == 0) {
			perror(argv[1]);
		} else {
			ibuf->depth = 24;
			freecmapImBuf(ibuf);
			ibuf->ftype = JPG_JST | quality;
			
			if (ibuf->x != 720 || ibuf->y != 576) {
				if (ibuf->y > 300) {
					ibuf->x *= 2;
					ibuf->y /= 2;
					scaleimbuf(ibuf, 1440, 288);
					ibuf->x = 720; ibuf->y = 576;
				} else scaleimbuf(ibuf, 720, 576);
			}
			
			cspace(ibuf, rgbbeta);
			safechroma(ibuf);
			if (opt) safeluma(ibuf);
			
			ibuf->flags |= IB_yuv;
			saveiff(ibuf, argv[i], IB_rect | IB_yuv);
			printf("converted: %s\n", argv[i]);
		}
		freeImBuf(ibuf);
	}
}

uchar * find_or_die (uchar * mem, long len, long match)
{
	printf("%d, %8x\n", len, match);
	
	while (len > 0) {
		if (mem[0] == (match >> 24) & 0xff) {
			if (mem[1] == (match >> 18) & 0xff) {
				if (mem[2] == (match >> 8) & 0xff) {
					if (mem[3] == (match >> 0) & 0xff) {
						return(mem);
					}
				}
			}
		}
		len--;
		mem++;
	}
	
	printf("no (more) match found\n");
	exit(0);
}

void fromjstream(long argc, char **argv)
{
	uchar * stream, *point1 = 0, *point2 = 0, *streamend;
	uchar * example;
	long streamsize, file, headersize, marker;
	uchar frame[256], field[260];
	
	if (argc < 4) {
		printf("usage: %s in.jst example.jf0 out\n", argv[0]);
		exit(1);
	}
	
	file = open(argv[1],O_RDONLY);
	if (file == -1) {
		perror(argv[1]);
		exit(1);
	}
	
	streamsize = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	stream = (uchar *) mmap(0, streamsize, PROT_READ, MAP_SHARED, file, 0);
	streamend = stream + streamsize;
	
	point1 = find_or_die (stream, streamsize, 'DATA');
	point1 += 8;
	
	memcpy(&marker, point1, 4);

	printf("%d\n", marker);
	
	file = open(argv[2],O_RDONLY);
	if (file == -1) {
		perror(argv[2]);
		exit(1);
	}
	
	headersize = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	example = (uchar *) mmap(0, headersize, PROT_READ, MAP_SHARED, file, 0);
	point2 = find_or_die (example, headersize, marker);
	
	headersize = point2 - example;
	
	strcpy(frame, argv[3]);
	
	while (1) {
		point2 = find_or_die (point1 + 4, (streamend - point1) - 4, marker);
		
		strcpy(field, frame);
		strcat(field, ".jf0");
		file = open(field, O_RDWR | O_CREAT | O_TRUNC);
		
		if (file == -1) {
			perror(field);
			exit(0);
		}
		
		write(file, example, headersize);
		if (write(file, point1, point2 - point1) != point2 - point1){
			perror(field);
			exit(0);
		}
		close(file);
		
		point1 = point2;
		
/* */

		point2 = find_or_die (point1 + 4, (streamend - point1) - 4, marker);
		
		strcpy(field, frame);
		strcat(field, ".jf1");
		file = open(field, O_RDWR | O_CREAT | O_TRUNC);
		
		if (file == -1) {
			perror(field);
			exit(0);
		}
		
		write(file, example, headersize);
		if (write(file, point1, point2 - point1) != point2 - point1){
			perror(field);
			exit(0);
		}
		close(file);
		
		point1 = point2;

		exit(0);
	} 
}


main(argc,argv)
long argc;
char **argv;
{
	if (strcmp(argv[0], "cpyalpha") == 0) copyalpha(argc, argv);
	else if (strcmp(argv[0], "col_to_alpha") == 0) col_to_alpha(argc, argv);
	else if (strcmp(argv[0], "yuvrgb") == 0) cspac(argc, argv);
	else if (strcmp(argv[0], "rgbyuv") == 0) cspac2(argc, argv);
	else if (strcmp(argv[0], "mask") == 0) mask(argc, argv);
	else if (strcmp(argv[0], "grabopt") == 0) grabopt(argc, argv);
	else if (strcmp(argv[0], "2to3") == 0) to23(argc, argv);
	else if (strcmp(argv[0], "scalecdi") == 0) scalecdi(argc, argv);
	else if (strcmp(argv[0], "deint") == 0) deint(argc, argv);
	else if (strcmp(argv[0], "reint") == 0) reint(argc, argv);
	else if (strcmp(argv[0], "nop") == 0) nop(argc, argv);
	else if (strcmp(argv[0], "chromakey") == 0) chroma(argc, argv);
	else if (strcmp(argv[0], "temp") == 0) temp(argc, argv);
	else if (strcmp(argv[0], "mrtotga") == 0) mr_to_tga(argc, argv);
	else if (strcmp(argv[0], "tompeg") == 0) tompeg(argc, argv);
	else if (strcmp(argv[0], "frommpeg") == 0) frommpeg(argc, argv);
	else if (strcmp(argv[0], "abb") == 0) allbutblack(argc, argv);
	else if (strcmp(argv[0], "sub2") == 0) sub2(argc, argv);
	else if (strcmp(argv[0], "add2") == 0) add2(argc, argv);
	else if (strcmp(argv[0], "lace") == 0) lace(argc, argv);	
	else if (strcmp(argv[0], "premul") == 0) premul(argc, argv);	
	else if (strcmp(argv[0], "demul") == 0) demul(argc, argv);	
	else if (strcmp(argv[0], "stripalpha") == 0) stripalpha(argc, argv);
	else if (strcmp(argv[0], "fieldsplit") == 0) fieldsplit(argc, argv);
	else if (strcmp(argv[0], "fieldswap") == 0) fieldswap(argc, argv);
	else if (strcmp(argv[0], "fieldmerge") == 0) fieldmerge(argc, argv);
	else if (strcmp(argv[0], "rmcrawl") == 0) rmcrawl(argc, argv);
	else if (strcmp(argv[0], "rotcol") == 0) rotcol(argc, argv);
	else if (strcmp(argv[0], "tojst") == 0) tojst(argc, argv);
	else if (strcmp(argv[0], "fromjstream") == 0) fromjstream(argc, argv);
	else if (strcmp(argv[0], "medsys") == 0) medsys(argc, argv);
	else if (strcmp(argv[0], "dvd") == 0) dvd(argc, argv);
	else if (strcmp(argv[0], "gamma1") == 0) gamma1(argc, argv);
	else if (strcmp(argv[0], "gamma2") == 0) gamma2(argc, argv);
	else if (strcmp(argv[0], "graph") == 0) graph(argc, argv);
	else printf("%s: don't know what to do \n", argv[0]);
}

