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

/* screen.h    dec 93 jan 94 */


/*
 * 
 *	LET OP: util.h (ListBase) en graphics.h (vec2s) ook nodig
 * 
 */

#ifndef SCREEN_H
#define SCREEN_H


#define MAXWIN		128

/* Als je EDGEWIDTH verandert, ook globale array edcol[] goedzetten */
#define EDGEWIDTH	5
#define EDGEWIDTH2	((EDGEWIDTH-1)/2)
#define AREAGRID	4
#define AREAMINX	32
#define HEADERY		21
#define AREAMINY	(HEADERY+EDGEWIDTH)

#define NOHEADER	0
#define HEADERDOWN	1
#define HEADERTOP	2

#define L_SCROLL 1			/* left scrollbar */
#define R_SCROLL 2
#define VERT_SCROLL 3
#define T_SCROLL 4
#define B_SCROLL 8
#define HOR_SCROLL 12

#define SPACE_EMPTY		0	/* deze getallen moeten kleiner dan 100 zijn ivm buttoncodes */
#define SPACE_VIEW3D	1
#define SPACE_IPO		2
#define SPACE_OOPS		3
#define SPACE_BUTS		4
#define SPACE_FILE		5
#define SPACE_IMAGE		6
#define SPACE_INFO		7
#define SPACE_SEQ		8
#define SPACE_PAINT		9
#define SPACE_IMASEL	10

#define MAXQUEUE 256

/* queue events: naar blendef.h */

typedef struct bScreen {
	ID id;
	short startx, endx, starty, endy;	/* framebuffer coords */
	short sizex, sizey;
	ListBase vertbase, edgebase, areabase;
	short mainwin, winakt;
	Scene *scene;
	short scenenr, screennr;			/* alleen voor pupmenu */
	short full, rt;
} bScreen;


typedef struct ScrVert {
	struct ScrVert *next, *prev, *new;
	vec2s vec;
	int flag;
} ScrVert;

typedef struct ScrEdge {
	struct ScrEdge *next, *prev;
	ScrVert *v1, *v2;
	short border;			/* 1 als op rand screen */
	short flag;
} ScrEdge;

typedef struct ScrArea {
	struct ScrArea *next, *prev;
	ScrVert *v1, *v2, *v3, *v4;
	short headwin, win;
	short headertype;		/* 0=niets, 1= down, 2= up */
	char spacetype, butspacetype;	/* SPACE_...  */
	bScreen *full;			/* als area==full, dit is de parent */
	rcti totrct, headrct, winrct;
	short winx, winy;		/* size */
	char head_swap, head_equal;
	char win_swap, win_equal;
	short *headqueue, *hq, *winqueue, *wq;
	float winmat[4][4];
	
	short headbutlen, headbutofs;
	short cursor, rt;
	
	void (*headchange)(), (*winchange)();
	void (*headdraw)(), (*windraw)();
	void (*headqread)(), (*winqread)();
	
	ListBase spacedata;
} ScrArea;

/* ********** MYWINDOW ******* */
#
#
typedef struct bWindow {
	struct bWindow *next, *prev;
	int id, parent_id;

	int xmin, xmax, ymin, ymax;
	float viewmat[4][4], winmat[4][4];
	
} bWindow;

/* ivm casten: */
void myortho2(float x1, float x2, float y1, float y2);
void myortho(float x1, float x2, float y1, float y2, float n, float f);
void mywindow(float x1, float x2, float y1, float y2, float n, float f);

#define winset		mywinset
#define winget		mywinget
#define ortho		myortho
#define ortho2		myortho2
#define window(a, b, c, d, e, f)		mywindow(a, b, c, d, e, f)
#define winclose	mywinclose
#define loadmatrix	myloadmatrix
#define getmatrix	mygetmatrix
#define multmatrix	mymultmatrix
#define getsize		mygetsize
#define getorigin	mygetorigin

extern ListBase swindowbase;	/* uit mywindow.c */

/* ********** END MYWINDOW ******* */

/* GLOBALS   (uit screen.c) */

extern ScrArea *curarea;
extern int displaysizex, displaysizey;

/* EXPORTS   (uit screen.c) */
extern short extern_qread(short *val);
extern int select_area(int spacetype);
extern ScrArea *closest_bigger_area();

#endif /* SCREEN_H */

