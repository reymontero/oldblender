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

#include <gl/gl.h>
#include <gl/device.h>
#include <local/gl_util.h>

short mousexN = 0, mouseyN = 0, *mousexNp = &mousexN , *mouseyNp = &mouseyN;
long qualN = 0, winN = 0, *qualNp = &qualN, *winNp = &winN;
long (*queread)() = qread;
Boolean (*getbut)(Device) = getbutton;


long qreadN(val)
short *val;
{
	long event;
	static long qual = 0, mousex = 0, mousey = 0, win = 0;

	switch(event = queread(val)){
	case LEFTMOUSE:
		if (*val) qual |= LMOUSE;
		else qual &= ~LMOUSE;
		break;
	case MIDDLEMOUSE:
		if (*val) qual |= MMOUSE;
		else qual &= ~MMOUSE;
		break;
	case RIGHTMOUSE:
		if (*val) qual |= RMOUSE;
		else qual &= ~RMOUSE;
		break;
	case LEFTSHIFTKEY:
		if (*val) qual |= LSHIFT;
		else qual &= ~LSHIFT;
		break;
	case RIGHTSHIFTKEY:
		if (*val) qual |= RSHIFT;
		else qual &= ~RSHIFT;
		break;
	case LEFTCTRLKEY:
		if (*val) qual |= LCTRL;
		else qual &= ~LCTRL;
		break;
	case RIGHTCTRLKEY:
		if (*val) qual |= RCTRL;
		else qual &= ~RCTRL;
		break;
	case LEFTALTKEY:
		if (*val) qual |= LALT;
		else qual &= ~LALT;
		break;
	case RIGHTALTKEY:
		if (*val) qual |= RALT;
		else qual &= ~RALT;
		break;
	case MOUSEX:
		mousex = *val;
		if (mousexNp) *mousexNp = mousex;
		break;
	case MOUSEY:
		mousey = *val;
		if (mouseyNp) *mouseyNp = mousey;
		break;
	case INPUTCHANGE:
		win = *val;
		if (win){
			qual = 0;
			if (getbut(LEFTMOUSE))		qual |= LMOUSE;
			if (getbut(MIDDLEMOUSE))	qual |= MMOUSE;
			if (getbut(RIGHTMOUSE))	    qual |= RMOUSE;
			if (getbut(LEFTSHIFTKEY))	qual |= LSHIFT;
			if (getbut(RIGHTSHIFTKEY))	qual |= RSHIFT;
			if (getbut(LEFTCTRLKEY))	qual |= LCTRL;
			if (getbut(RIGHTCTRLKEY))	qual |= RCTRL;
			if (getbut(LEFTALTKEY))	    qual |= LALT;
			if (getbut(RIGHTALTKEY))	qual |= RALT;
		}
		if (winNp) *winNp = win;
		break;
	}

	if (qualNp) *qualNp = qual;

	return(event);
}


short findcolor(short r, short g, short b)
{
	static short initdone = FALSE, col[256][3];
	short i, best, dr, dg, db;
	long dist, mdist;

	if (initdone == FALSE) {
		for (i = 0; i < 256; i++){
			getmcolor(i, &col[i][0], &col[i][1], &col[i][2]);
		}
		initdone = TRUE;
	}

	best = 0;
	mdist = 0x800000;

	for (i = 0; i < 256; i++){
		dr = r - col[i][0];
		dg = g - col[i][1];
		db = b - col[i][2];

		dist = (dr * dr) + (dg * dg) + (db * db);
		if (dist < mdist) {
			mdist = dist;
			best = i;
		}
	}

	mapcolor(best, col[best][0], col[best][1], col[best][2]);
	return(best);
}

void glu_drawcross(short x, short y)
{
	sboxi(x, 0, x, 2000);
	sboxi(0, y, 2000, y);
}


void glu_drawrect(short x1, short y1, short x2, short y2)
{
	sboxi(x1,y1,x2,y2);
}


void glu_drawline(short x1, short y1, short x2, short y2)
{
	short vec[2];

	bgnline();
	vec[0] = x1;
	vec[1] = y1;
	v2s(vec);
	vec[0] = x2;
	vec[1] = y2;
	v2s(vec);
	endline();
}


long getcoords(short * minx, short * miny, short * maxx, short * maxy,
	void (*drawfunc1)(short, short),
	void (*drawfunc2)(short, short, short, short))
{
	short x1,x2,y1,y2;
	short dev ,val;
	long ofsx, ofsy, sx, sy;
	long mousex, mousey;
	long attach = FALSE;
	long mxq, myq, lmq, rmq, kbd;
	long abort = FALSE;

	if ((mxq = isqueued(MOUSEX)) == FALSE) qdevice(MOUSEX);
	if ((myq = isqueued(MOUSEY)) == FALSE) qdevice(MOUSEY);
	if ((lmq = isqueued(LEFTMOUSE)) == FALSE) qdevice(LEFTMOUSE);
	if ((rmq = isqueued(RIGHTMOUSE)) == FALSE) qdevice(RIGHTMOUSE);
	if ((kbd = isqueued(KEYBD)) == FALSE) qdevice(KEYBD);

	getorigin(&ofsx,&ofsy);
	cursoff();

	drawmode(PUPDRAW);
	mapcolor(1,255,0,0);

	mousex = getvaluator(MOUSEX);
	mousey = getvaluator(MOUSEY);

	dev = 0;

	while (dev != LEFTMOUSE && abort == FALSE){

		switch (dev = qreadN(&val)){
		case KEYBD:
			abort = TRUE;
			break;
		case LEFTMOUSE:
		case MIDDLEMOUSE:
			if (val == 0) dev =0;
			break;
		case MOUSEX:
			mousex = val;
			break;
		case MOUSEY:
			mousey = val;
			break;
		case RIGHTMOUSE:
			if (val) {
				attach = TRUE;
				pushmatrix();
				pushviewport();
				fullscrn();
				ofsx = ofsy = 0;
			} else{
				attach = FALSE;
				color(0);
				clear();
				endfullscrn();
				popviewport();
				popmatrix();
				getorigin(&ofsx,&ofsy);
			}
			break;
		}
		color(0);
		drawfunc1(x1, y1);
		x1 = mousex - ofsx;
		y1 = mousey - ofsy;
		color(1);
		drawfunc1(x1, y1);
	}

	color(0);
	clear();
	x1 = x2 = mousex - ofsx;
	y1 = y2 = mousey - ofsy;
	dev = 0;

	while (dev!=LEFTMOUSE && dev!=MIDDLEMOUSE && abort == FALSE){
		color(0);
		drawfunc2(x1,y1,x2,y2);
		x2 = mousex - ofsx;
		y2 = mousey - ofsy;
		color(1);
		drawfunc2(x1,y1,x2,y2);
		switch (dev = qreadN(&val)){
		case KEYBD:
			abort = TRUE;
			break;
		case MOUSEX:
			mousex = val;
			break;
		case MOUSEY:
			mousey = val;
			break;
		}
	}

	curson();
	color(0);
	clear();
	if (attach){
		endfullscrn();
		popviewport();
		popmatrix();
		getorigin(&ofsx,&ofsy);
		x1 -= ofsx; 
		x2 -= ofsx;
		y1 -= ofsy; 
		y2 -= ofsy;
	}

	drawmode(NORMALDRAW);

	*minx = x1;
	*maxx = x2;
	*miny = y1;
	*maxy = y2;

	if (!mxq) unqdevice(MOUSEX);
	if (!myq) unqdevice(MOUSEY);
	if (!lmq) unqdevice(LEFTMOUSE);
	if (!rmq) unqdevice(RIGHTMOUSE);
	if (!kbd) unqdevice(KEYBD);

	if (abort) return(0);
	if (attach) return(2);
	return (1);
}


long getrect(minx,miny,maxx,maxy)
short *minx,*miny,*maxx,*maxy;
{
	long ret;
	long x1, x2;
	long y1, y2;
	
	ret = getcoords(minx, miny, maxx, maxy, glu_drawcross, glu_drawrect);

	/* sorteren */
	x1 = *minx; x2 = *maxx;
	y1 = *miny; y2 = *maxy;
	
	if (x1 < x2) {
		*minx = x1;
		*maxx = x2;
	} else {
		*minx = x2;
		*maxx = x1;
	}
	if (y1 < y2) {
		*miny = y1;
		*maxy = y2;
	} else {
		*miny = y2;
		*maxy = y1;
	}
	
	return(ret);
}


long getline(minx,miny,maxx,maxy)
short *minx,*miny,*maxx,*maxy;
{
	return getcoords(minx, miny, maxx, maxy, glu_drawcross, glu_drawline);
}


#define NCURSORS	11

unsigned short curtab[NCURSORS][16] = {
    {0x7FFE, 0x4002, 0x2004, 0x300C, 
    0x2894, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x3FFC, 0x4FF2, 0x7FFE},

    {0x7FFE, 0x43C2, 0x2184, 0x300C, 
    0x2814, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x2FF4, 0x47E2, 0x7FFE},

    {0x7FFE, 0x47E2, 0x23C4, 0x308C, 
    0x2814, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x3FFC, 0x27F4, 0x4182, 0x7FFE},

    {0x7FFE, 0x4FE2, 0x27C4, 0x318C, 
    0x2914, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x37FC, 0x23F4, 0x4082, 0x7FFE},

    {0x7FFE, 0x4FF2, 0x27E4, 0x33CC, 
    0x2914, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x37FC, 0x21C4, 0x4002, 0x7FFE},

    {0x7FFE, 0x4FFA, 0x27F4, 0x33EC, 
    0x2994, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2FF4, 
    0x37CC, 0x2084, 0x4002, 0x7FFE},

    {0x7FFE, 0x5FFA, 0x2FF4, 0x37EC, 
    0x2B94, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2FF4, 
    0x318C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FF4, 0x37EC, 
    0x2BD4, 0x2424, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x27E4, 0x2BD4, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FD4, 0x2424, 0x2244, 0x2244, 
    0x2344, 0x23C4, 0x27E4, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FF4, 0x26E4, 0x2244, 0x2244, 
    0x22C4, 0x23C4, 0x2424, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},

    {0x7FFE, 0x7FFE, 0x3FFC, 0x3FFC, 
    0x2FF4, 0x27E4, 0x23C4, 0x23C4, 
    0x2244, 0x2244, 0x2424, 0x2814, 
    0x300C, 0x2004, 0x4002, 0x7FFE},
};

static int curindex = -1;
static short oldcursor = 0;

void percentdone(float percent)
{
	int index;
	unsigned short dummy;
	long ldummy;

	if((curindex == -1) && (percent == 0.0))
		getcursor(&oldcursor,&dummy,&dummy,&ldummy);
	if (percent > 99.9) {
		setcursor(oldcursor,0xfff,0xfff);
		curindex = -1;
	} else {
		index = percent*NCURSORS/100.0;
		if (index<0)
			index = 0;
		if (index>=NCURSORS)
			index = NCURSORS-1;
		if (index != curindex) {
			defcursor(20,curtab[index]);
			curorigin(20,7,7);
			setcursor(20,0xfff,0xfff);
			curindex = index;
		}
	}
}


short smallpup(char * title, char * str)
{
	char a[250];
	long temp;
	short event, val;
	
	strcpy(a, title);
	strcat(a,"%t |");
	strcat(a,str);
	temp=defpup(a);

	sginap(2);
	event=dopup(temp);
	freepup(temp);
	if(event== -1) event=0;

	/*even een 1/2 seconde de tijd gunnen om op ESC te drukken */
	if (event){
		for(temp = 5; temp >0; temp--){
			while(qtest()){
				if (qreadN(&val) == ESCKEY){
					if (val){
						event = 0;
						break;
					}
				}
			}
			sginap(10);
		}
	}

	while(getbutton(RIGHTMOUSE)) sginap(10);
	return event;
}


short okee(char * str)
{
	smallpup("OK ?", str);
}


