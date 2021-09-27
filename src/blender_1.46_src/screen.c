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



/* screen.c  dec/jan 93/94		GRAPHICS
 * 
 * 
 * 
 * 
 * 
 */


#include "blender.h"
#include "graphics.h"
#include "render.h"
#include <sys/syssgi.h>

#ifndef FREE
#include "/usr/people/ton/code.h"
#endif

/* TIPS:
 * 
 * - LET OP DE EDGES,  VERTICES ERVAN MOETEN IN VOLGORDE
	 (laagste pointer eerst). Anders onvoorspelbare effecten!
 * - probleem: flags zijn nog niet echt netjes. Altijd na gebruik
	 op nul zetten.

/* ************************* */

/* ********* Globals *********** */

extern bWindow *swinarray[];	/* mywindow.c */

ScrArea *curarea= 0;
ScrEdge *curedge= 0;
int displaysizex= 0, displaysizey= 0;
int prefsizx= 0, prefsizy= 0, prefstax= 0, prefstay= 0;
short mainwin=0, winqueue_break= 0, cursonedge=0, keycode[100];
ScrArea *areawinar[MAXWIN];

/* ulong edcol[EDGEWIDTH]= {0x0, 0x303030, 0x606060, 0x808080, 0x909090, 0xF0F0F0, 0x0}; */
ulong edcol[EDGEWIDTH]= {0x0, 0x505050, 0x909090, 0xF0F0F0, 0x0};

int autosavetime;

/* ********* Funkties *********** */

void getdisplaysize();
bScreen *default_foursplit(), *default_twosplit();
ScrEdge *findscredge(ScrVert *v1, ScrVert *v2);
void setscreen(bScreen *sc);
void drawscreen();
void initscreen();
void moveareas();
void scrollheader();
void testareas();
void area_fullscreen();
void addqueue(short win, short event, short val);
void editsplitpoint();
void splitarea(ScrArea *sa, char dir, float fac);
void joinarea(ScrArea *sa);



static ushort hor_ptr_bits[] = {
	0x0000, 0x0000, 0x0000, 0x0000, 
	0x1008, 0x1818, 0x1c38, 0xffff, 
	0xffff, 0x1c38, 0x1818, 0x1008, 
	0x0000, 0x0000, 0x0000, 0x0000, 

	0x0000, 0x0000, 0x0000, 0x381c, 
	0x3c3c, 0x3e7c, 0xffff, 0xffff, 
	0xffff, 0xffff, 0x3e7c, 0x3c3c, 
	0x381c, 0x0000, 0x0000, 0x0000, 
};

static ushort vert_ptr_bits[] = {
	0x0180, 0x0180, 0x0180, 0x0ff0, 
	0x07e0, 0x03c0, 0x0180, 0x0, 
	0x0, 0x0180, 0x03c0, 0x07e0, 
	0x0ff0, 0x0180, 0x0180, 0x0180, 

	0x03c0, 0x03c0, 0x1ff8, 0x1ff8, 
	0x1ff8, 0x0ff0, 0x07e0, 0x03c0, 
	0x03c0, 0x07e0, 0x0ff0, 0x1ff8, 
	0x1ff8, 0x1ff8, 0x03c0, 0x03c0, 
};

static ushort win_ptr_bits[] = {
	0x0000, 0x0180, 0x0180, 0x0180, 
	0x0000, 0x89b3, 0xfdb3, 0xfdb7, 
	0xb5bf, 0xb5bb, 0x85b3, 0x0000, 
	0x0180, 0x0180, 0x0180, 0x0000, 

	0x03c0, 0x03c0, 0x03c0, 0x03c0, 
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0x03c0, 0x03c0, 0x03c0, 0x03c0
};


void init_screen_cursors()
{
	if(G.machine!=IRIS) curstype(C16X2);
	defcursor(10, (ushort *)hor_ptr_bits);
	curorigin(10, 8, 8);
	defcursor(11, (ushort *)vert_ptr_bits);
	curorigin(11, 8, 8);
	defcursor(12, (ushort *)win_ptr_bits);
	curorigin(12, 8, 8);

	/* 15 is gereserveerd voor timecursor, zie renderfg.c */
}

void setcursorN(short cur)
{
	if(curarea) {
		curarea->cursor= cur;
		if(G.curscreen->winakt==curarea->win) setcursor(cur, 0, 0);
	}
}

void waitcursor(int val)
{
	ScrArea *sa;
	int oldwin;
	
	if(G.curscreen==0) return;
	if(G.background) return;
	
	if(val) {
		if(R.win && R.win==winget()) {
			oldwin= winget();
			winset(G.curscreen->mainwin);
			setcursor(2, 0, 0);
			winset(oldwin);
		}
		else setcursor(2, 0, 0);
	}
	else if(G.curscreen->winakt>3) {
		if(R.flag & R_RENDERING) return;
		sa= areawinar[G.curscreen->winakt];
		if(sa->win==G.curscreen->winakt) setcursor(sa->cursor, 0, 0);
		else  setcursor(0, 0, 0);
	}
	else setcursor(0, 0, 0);
}

void tempcursor(int curs)
{
	ScrArea *sa;
	
	if(G.curscreen==0) return;
	
	if(curs) setcursor(curs, 0, 0);
	else if(G.curscreen->winakt) {
		sa= areawinar[G.curscreen->winakt];
		if(sa->win==G.curscreen->winakt) setcursor(sa->cursor, 0, 0);
		else  setcursor(0, 0, 0);
	}
}

void wich_cursor(ScrArea *sa)
{
	/* o.a. als nieuwe space gemaakt */
	
	sa->cursor= 0;
	if(sa->spacetype==SPACE_VIEW3D) {
		if(G.obedit) sa->cursor= 1;
		else if(G.f & G_VERTEXPAINT) sa->cursor= 3;
		else if(G.f & G_FACESELECT) sa->cursor= 4;
	}
	if(sa->win==G.curscreen->winakt) setcursor(sa->cursor, 0, 0);
}


void setcursor_space(int spacetype, short cur)
{
	bScreen *sc;
	ScrArea *sa;
	int oldwin;
	
	sc= G.main->screen.first;
	while(sc) {
		sa= sc->areabase.first;
		while(sa) {
			if(sa->spacetype==spacetype) {
				sa->cursor= cur;
				if(cur==0) wich_cursor(sa);	/* extra test, bv nodig bij verlaten editmode in vertexpaint */
				if(sc==G.curscreen && sc->winakt==sa->win) setcursor(sa->cursor, 0, 0);
				
			}
			sa= sa->next;
		}
		sc= sc->id.next;
	}
}



void mydrawmode(int mode)
{
	if(G.machine==ENTRY ) {
		if(mode==OVERDRAW) mode=PUPDRAW;	
	}
#undef drawmode
	drawmode(mode);
#define drawmode mydrawmode
}


/* *********** CODETAB ******************* */

void decodekeytab()
{
	extern unsigned char hash[512];
	char ihash[512];
	long a,b,file;
	char *c, hus1, hus2, sl[MAXSYSIDSIZE], str[30];

	file= open("/.Bcode",O_RDONLY);
	if(file== -1) exit(0);

	read(file,keycode,200);
	close(file);

	syssgi(SGI_SYSID,sl);

	/* inverse hashtab */
	for(a=0;a<256;a++) {
		for(b=0;b<256;b++) {
			if(a==hash[b]) break;
		}
		ihash[a]= b;
	}
	for(a=0;a<256;a++) ihash[256+a]= ihash[a];

	/* bereken hus1 en hus2 ahv sleutel */
	hus1= hash[ sl[0]+ hash[ sl[2]] ];
	hus2= hash[ sl[1]+ hash[ sl[3]] ];

	c= (char *)keycode;
	for(a=0; a<100; a++) {
		c[0]= ( ihash[ (ihash[ c[0] ] -hus1) & 255 ]  ) & 255 ;
		c[1]= ( ihash[ (ihash[ c[1] ] -hus2) & 255 ]  ) & 255 ;
		c+=2;
	}
}



/* *********  IN/OUT  ************* */

void getmouseco_sc(short *mval)		/* screen coordinaten */
{
	static Device mdev[2]= {MOUSEX, MOUSEY};

	if(G.curscreen==0) return;

	getdev(2, mdev, mval);
	mval[0]-= G.curscreen->startx;
	mval[1]-= G.curscreen->starty;
}

void getmouseco_areawin(short *mval)		/* interne area coordinaten */
{

	getmouseco_sc(mval);
	if(curarea->win) {
		mval[0]-= curarea->winrct.xmin;
		mval[1]-= curarea->winrct.ymin;
	}
}

void getmouseco_headwin(short *mval)		/* interne area coordinaten */
{

	getmouseco_sc(mval);
	if(curarea->headwin) {
		mval[0]-= curarea->headrct.xmin;
		mval[1]-= curarea->headrct.ymin;
	}
}

/* ********  WINDOW & QUEUE ********* */

/* *********** AUTOSAVE ************** */

void init_autosave()
{
	if(U.flag & AUTOSAVE) {
		if( isqueued(TIMER0)==0 ) {
			qdevice(TIMER0);
			noise(TIMER0, 3600);
		}
	}
	else {
		if( isqueued(TIMER0) ) unqdevice(TIMER0);
	}
	autosavetime= 0;
}

void set_cursonedge(short mx, short my)
{
	ScrEdge *se, *se1=0, *se2=0, *se3=0, *se4=0;
	float dist, mindist= 100.0, vec1[2], vec2[2], vec3[2], PdistVL2Dfl();

	vec1[0]= mx;
	vec1[1]= my;
	curedge= 0;
	
	/* als de edge element is van curarea: extra voordeel. Dit voor juiste split en join  */
	if(curarea) {
		se1= findscredge(curarea->v1, curarea->v2);
		se2= findscredge(curarea->v2, curarea->v3);
		se3= findscredge(curarea->v3, curarea->v4);
		se4= findscredge(curarea->v4, curarea->v1);
	}
	
	se= G.curscreen->edgebase.first;
	while(se) {

		vec2[0]= se->v1->vec.x;
		vec2[1]= se->v1->vec.y;
		vec3[0]= se->v2->vec.x;
		vec3[1]= se->v2->vec.y;
		dist= PdistVL2Dfl(vec1, vec2, vec3);
		
		if(se==se1 || se==se2 || se==se3 || se==se4) dist-= 5.0;
		
		if(dist<mindist) {
			mindist= dist;
			curedge= se;
		}

		se= se->next;
	}
	
	if(curedge==0) return;
	cursonedge= 1;
	
	if(curedge->v1->vec.x==curedge->v2->vec.x) setcursor(10, 0, 0);
	else setcursor(11, 0, 0);
}

void rgbmul(ulong *col, int mul)
{
	int temp;
	char *rt;
	
	rt= (char *)col;

	temp= (mul*rt[1])>>8;
	if(temp>255) rt[1]= 255; else rt[1]= temp;
	temp= (mul*rt[2])>>8;
	if(temp>255) rt[2]= 255; else rt[2]= temp;
	temp= (mul*rt[3])>>8;
	if(temp>255) rt[3]= 255; else rt[3]= temp;
}

void areawinset(short win)
{
	ScrArea *sa= 0;
	SpaceSeq *sseq;
	
	if(win<4) return;
	
	curarea= areawinar[win];
	if(curarea==0) {
		printf("error in areawinar %d ,areawinset\n", win);
		return;
		
	}
	
	switch(curarea->spacetype) {
	case SPACE_VIEW3D:
		G.vd= curarea->spacedata.first;
		break;
	case SPACE_IPO:
		if(G.sipo != curarea->spacedata.first) allqueue(REDRAWBUTSANIM, 0);
		G.sipo= curarea->spacedata.first;
		G.v2d= &G.sipo->v2d;
		break;
	case SPACE_BUTS:
		G.buts= curarea->spacedata.first;
		G.v2d= &G.buts->v2d;
		break;
	case SPACE_SEQ:
		sseq= curarea->spacedata.first;
		G.v2d= &sseq->v2d;
		
	case SPACE_OOPS:
		G.v2d= &((SpaceOops *)curarea->spacedata.first)->v2d;
		G.soops= curarea->spacedata.first;
		break;
	case SPACE_IMASEL:
	
		break;
	case SPACE_IMAGE:
		G.v2d= &((SpaceImage *)curarea->spacedata.first)->v2d;
		G.sima= curarea->spacedata.first;
		break;
	case SPACE_PAINT:

		break;
	case SPACE_FILE:

		break;
	}
	winset(win);
}

void headerbox(ulong selcol, int width)
{
	cpack(selcol);
	clear();
	
	cpack(0x0);
	sboxi(0, HEADERY, width, HEADERY);
	cpack(0xE0E0E0);
	sboxi(0, HEADERY-1, width, HEADERY-1);
	cpack(0xB0B0B0);
	sboxi(0, HEADERY-2, width, HEADERY-2);
	
	cpack(0x707070);
	sboxi(0, 2, width, 2);
	cpack(0x404040);
	sboxi(0, 1, width, 1);
	cpack(0x0);
	sboxi(0, 0, width, 0);	
}

void defheaddraw()
{
	ulong selcol;
	
	/* aktieve kleur */
	selcol= 0x909090;

	if(G.curscreen->winakt) {
		if(curarea->headwin == G.curscreen->winakt) selcol= 0x90A0A0;
		else if(curarea->win == G.curscreen->winakt) selcol= 0x90A0A0;
	}
	
	headerbox(selcol, curarea->winx+100);
	
	/* buttons, functies in window.c */
	switch(curarea->spacetype) {
	case SPACE_EMPTY:
		break;
	case SPACE_FILE:
		file_buttons();
		break;
	case SPACE_INFO:
		info_buttons();
		break;
	case SPACE_VIEW3D:
		view3d_buttons();
		break;
	case SPACE_IPO:
		ipo_buttons();
		break;
	case SPACE_BUTS:
		buts_buttons();
		break;
	case SPACE_SEQ:
		seq_buttons();
		break;
	case SPACE_IMAGE:
		image_buttons();
		break;
	case SPACE_IMASEL:
		imasel_buttons();
		break;
	case SPACE_OOPS:
		oops_buttons();
		break;
	case SPACE_PAINT:
		paint_buttons();
		break;
	}
	curarea->head_swap= 1;
}

void defwindraw()
{

	if(curarea->win && curarea->windraw) {
		curarea->windraw();
	}
	else {
		cpack(0x707070);
		clear();
	}
	curarea->win_swap= 1;
}

void defheadchange()
{
	float ofs;
	
	drawmode(OVERDRAW);
	color(0);
	clear();
	drawmode(NORMALDRAW);	
	

	if(curarea->headchange) {
		curarea->headchange();
	}
	else {
		ofs= curarea->headbutofs;

		if(curarea->headertype==HEADERDOWN)
			ortho2(-0.5+ofs, curarea->headrct.xmax-curarea->headrct.xmin-0.5+ofs, 0.5, curarea->headrct.ymax-curarea->headrct.ymin+0.5);
		else
			ortho2(-0.5+ofs, curarea->headrct.xmax-curarea->headrct.xmin-0.5+ofs, -0.5, curarea->headrct.ymax-curarea->headrct.ymin-0.5);
	}
}

void defwinchange()
{

	drawmode(OVERDRAW);
	color(0);
	clear();
	drawmode(NORMALDRAW);	
	
	if(curarea->winchange) {
		curarea->winchange();
	}
	else {
		ortho2(-0.5, curarea->winrct.xmax-curarea->winrct.xmin-0.5, -0.5, curarea->winrct.ymax-curarea->winrct.ymin-0.5);
		loadmatrix(matone);
	}
}

void defwinmat()
{

	ortho2(-0.5, curarea->winrct.xmax-curarea->winrct.xmin-0.5, -0.5, curarea->winrct.ymax-curarea->winrct.ymin-0.5);
	loadmatrix(matone);
}

void headmenu(ScrArea *sa)
{
	float fac;
	
	if(curarea->full) {
		confirm("", "Full window");
		return;
	}

	if( okee("Switch header")) {
		if(sa->headertype==1) sa->headertype=2;
		else if(sa->headertype==2) sa->headertype=1;
		testareas();
	}
}

void defheadqread(ScrArea *sa)
{
	ScrArea *tempsa;
	float fac;
	short event, val, do_redraw=0, do_change=0;
	
	areawinset(sa->headwin);
	
	while(sa->hq != sa->headqueue) {
		sa->hq-= 2;
		
		event= sa->hq[0];
		val= sa->hq[1];

		if(val) {
			if(event==LEFTMOUSE) {

				FrontbufferButs(TRUE);
				event= DoButtons();
				FrontbufferButs(FALSE);
				
				if(event) {
					if(event<1000) do_headerbuttons(event);
				}
				else {
					winset(G.curscreen->mainwin);
					if(G.qual & LR_CTRLKEY) winpush();
					else {
						winpop();
						sginap(3);
						R.winpop= 1;	/* flag: need pop */
					}
					areawinset(sa->headwin);
				}
			}
			else if(event==MIDDLEMOUSE) {
				scrollheader();
				break;
			}
			else if(event==RIGHTMOUSE) {
				headmenu(sa);
				break;
			}
			else if(event==REDRAW) {
				do_redraw= 1;
				break;
			}
			else if(event==CHANGED) {
				do_change= 1;
				do_redraw= 1;
				break;
			}
			else {
				if(sa->headqread) sa->headqread(event, val);
			}
			
			if(winqueue_break) return;
		}
	}

	/* test: bestaat window nog */	
	tempsa= areawinar[sa->headwin];
	if(tempsa==0) return;
	
	if(do_change || do_redraw) areawinset(sa->headwin);
	if(do_change) defheadchange();
	if(do_redraw) defheaddraw();
}

void defwinqread(ScrArea *sa)
{
	ScrArea *tempsa;
	short event, val, do_redraw=0, do_change=0;
	
	if(sa!=curarea || sa->win!=winget()) areawinset(sa->win);

	while(sa->wq != sa->winqueue) {
		sa->wq-= 2;
		
		event= sa->wq[0];
		val= sa->wq[1];
		
		if(event==REDRAW) {
			do_redraw= 1;
		}
		else if(event==CHANGED) {
			do_change= 1;
			do_redraw= 1;
		}
		else {
			if(sa->winqread) sa->winqread(event, val);
		}
		if(winqueue_break) return;
	}

	/* test: bestaat window nog */	
	tempsa= areawinar[sa->win];
	if(tempsa==0) return;

	if(do_change || do_redraw) areawinset(sa->win);
	if(do_change) defwinchange(sa->win);
	if(do_redraw) defwindraw();
	
}

void addqueue(short win, short event, short val)
{
	ScrArea *sa;
	int size;
	short *end;
	
	/* nieuwe events worden vooraan in het array gezet */
	
	sa= areawinar[win];
	if(sa) {
		if(win==sa->headwin) {
			end= sa->headqueue+MAXQUEUE;	/* NIET MET 2 VERMENIGV. !!! */
			if( (long)sa->hq < (long)end) {
				size= ((long)sa->hq)-((long)sa->headqueue);
				
				memcpy( sa->headqueue+2, sa->headqueue, size);
				sa->headqueue[0]= event;
				sa->headqueue[1]= val;
				sa->hq+= 2;
			}
		}
		else if(win==sa->win) {
			end= sa->winqueue+MAXQUEUE;
			if( (long)sa->wq < (long)end) {
				size= ((long)sa->wq)-((long)sa->winqueue);

				memcpy( sa->winqueue+2, sa->winqueue, size);
				sa->winqueue[0]= event;
				sa->winqueue[1]= val;
				sa->wq+= 2;
			}
		}
	}
}

short afterqueue[3*MAXQUEUE], *afterq=afterqueue;

void addafterqueue(short win, short event, short val)
{
	long poin;
	
	poin= (long)afterqueue;
	poin+= 6*(MAXQUEUE-1);
	
	if( (long)afterq < poin ) {
		afterq[0]= win;
		afterq[1]= event;
		afterq[2]= val;
		afterq+= 3;
	}
}

void append_afterqueue()
{
	while( afterqueue != afterq) {
		afterq-= 3;
		addqueue(afterq[0], afterq[1], afterq[2]);
	}
}

void setmapcolors()
{
	drawmode(PUPDRAW);
	mapcolor(1, 170, 170, 170); 
	mapcolor(2, 0, 0, 0); 
	mapcolor(3, 240, 240, 240);
	drawmode(OVERDRAW);
	mapcolor(1, 0, 0, 0); 
	mapcolor(2, 255, 0, 0); 
	mapcolor(3, 255, 255, 255);
	drawmode(CURSORDRAW);
	mapcolor(1, 255, 0, 0); 
	drawmode(NORMALDRAW);
}

void remake_qual()
{
	G.qual= 0;

	if(getbutton(RIGHTSHIFTKEY)) G.qual += R_SHIFTKEY;
	if(getbutton(LEFTSHIFTKEY)) G.qual += L_SHIFTKEY;
	if(getbutton(RIGHTALTKEY)) G.qual += R_ALTKEY;
	if(getbutton(LEFTALTKEY)) G.qual += L_ALTKEY;
	if(getbutton(RIGHTCTRLKEY)) G.qual += R_CTRLKEY;
	if(getbutton(LEFTCTRLKEY)) G.qual += L_CTRLKEY;
	
}

short ext_redraw=0, ext_inputchange=0, ext_mousemove=0, in_ext_qread=0;

short screen_qread(short *val)
{
	static short mx=0, my=0, oldwin= 0;
	ScrArea *sa;
	bWindow *win;
	short event, newwin, rt, devs[2], vals[2];
	
	if(in_ext_qread==0) {
		if(ext_inputchange) {
			*val= ext_inputchange;
			ext_inputchange= 0;
			event= INPUTCHANGE;
		}
		else if(ext_redraw) {
			*val= ext_redraw;
			ext_redraw= 0;
			event= REDRAW;
		}
		else if(ext_mousemove) {
			getmouseco_sc(vals);
			mx= vals[0];
			*val= vals[1]+G.curscreen->starty;
			ext_mousemove= 0;
			event= MOUSEY;
		}
		else if(afterqueue!=afterq && qtest()==0) {
			*val= 0;
			event= AFTERQUEUE;
		}
		else event= qread(val);
		
	}
	else event= qread(val);

	if(G.curscreen==0) return event;

	/* zet vlaggen in G.qual */
	if(event==RIGHTSHIFTKEY) {
		if(*val) G.qual |= R_SHIFTKEY;
		else G.qual &= ~R_SHIFTKEY;
	}
	else if(event==LEFTSHIFTKEY) {
		if(*val) G.qual |= L_SHIFTKEY;
		else G.qual &= ~L_SHIFTKEY;
	}
	else if(event==RIGHTALTKEY) {
		if(*val) G.qual |= R_ALTKEY;
		else G.qual &= ~R_ALTKEY;
	}
	else if(event==LEFTALTKEY) {
		if(*val) G.qual |= L_ALTKEY;
		else G.qual &= ~L_ALTKEY;
	}
	else if(event==RIGHTCTRLKEY) {
		if(*val) G.qual |= R_CTRLKEY;
		else G.qual &= ~R_CTRLKEY;
	}
	else if(event==LEFTCTRLKEY) {
		if(*val) G.qual |= L_CTRLKEY;
		else G.qual &= ~L_CTRLKEY;
	}
	else if(event==WINFREEZE || event==WINTHAW) {
		if((R.flag & R_RENDERING)==0) {
			if(R.win) winclose(R.win);
			R.win= 0;
			G.qual= 0;
		}
	}
	else if(event==INPUTCHANGE || event==DRAWOVERLAY || event==REDRAW || event==DRAWEDGES) {
			/* overlay: waarschijnlijk was dit windowmenu ALT/RIGHTMOUSE */
			/* DRAWEDGES: komt vanuit setscreen, qual opnieuw berekenen */

		remake_qual();
		
		if(event==INPUTCHANGE && in_ext_qread==0) {
			if(*val) {
				winset(*val);
				G.curscreen->winakt= *val;
				oldwin= *val;
			}
			oldwin= 0;
		}

		if(event==REDRAW ) {
			setmapcolors();
			
			/* kunstmatige mousy voor herberekenen winakt (als b.v. R.win naar achter gepusht */
			qenter(MOUSEY, -1);
		}
		
	}
	else if(event==MOUSEX || event==MOUSEY) {
		
		if(event==MOUSEX) {
			*val-= G.curscreen->startx;
			mx= *val;
		}

		if(event==MOUSEY && in_ext_qread==0 && (R.win==0 || G.curscreen->winakt!=R.win)) {
			if(*val != -1) {
				*val-= G.curscreen->starty;
				my= *val;
			}
	
			/* testen waar muis staat */
			newwin= 0;
			
			win= swindowbase.first;
			while(win) {
				if(mx>win->xmin && mx<win->xmax) {
					/* deze uitzondering betreft onderste en bovenste edge: voor edit cursonedge */
					if( (my==0 && my==win->ymin) || (my==G.curscreen->endy && my==win->ymax)) {
						if(my>win->ymin && my<win->ymax) {
							newwin= win->id;
							break;
						}
					}
					else if(my>=win->ymin && my<=win->ymax) {
						newwin= win->id;
						break;
					}
				}
				win= win->next;
			}

			/* cursor */
			if(newwin != oldwin) {
				if(newwin==0) {
					set_cursonedge(mx, my);
				}
				else if(oldwin==0) {
					cursonedge= 0;
				}
				if(newwin) {
					sa= areawinar[newwin];
					if(sa->win==newwin) setcursor(sa->cursor, 0, 0);
					else setcursor(0, 0, 0);
				}
			}
			else if(newwin==0 && oldwin==0) {
				set_cursonedge(mx, my);
			}

			if(newwin!=0) {

				if(newwin != oldwin || G.curscreen->winakt==0) {
					event= INPUTCHANGE;
					*val= newwin;
				}
			}

			oldwin= newwin;
			
		}
	}
	else if(event==TIMER0) {
		event= 0;
		autosavetime++;
		if(autosavetime > U.savetime) {
			if(in_ext_qread==0) {
				write_autosave();
				autosavetime= 0;
			}
		}
	}

	return event;
}

short special_qread(short *val)
{
	/* simul alternatief voor extern_qread */

	/* bewaart de laatste INPUTCHANGE en de laatste REDRAW */
	short event;
	
	event= qread(val);
	
	if(event==REDRAW) ext_redraw= *val;
	else if(event==INPUTCHANGE) ext_inputchange= *val;
	else if(event==MOUSEY || event==MOUSEX) ext_mousemove= 1;

	return event;
}


short extern_qread(short *val)
{
	/* bewaart de laatste INPUTCHANGE en de laatste REDRAW */
	short event;
	
	in_ext_qread= 1;	/* niet zo net, wel zo handig (zie screen_qread) */
	
	event= screen_qread(val);
	if(event==REDRAW) ext_redraw= *val;
	else if(event==INPUTCHANGE) ext_inputchange= *val;
	else if(event==MOUSEY || event==MOUSEX) ext_mousemove= 1;
	
	in_ext_qread= 0;

	return event;
}

short my_qtest()
{
	if(ext_inputchange) return INPUTCHANGE;
	else if(ext_redraw) return REDRAW;
	else if(ext_mousemove) return MOUSEY;
	else return qtest();
}

void markdirty_all()
{
	ScrArea *sa;

		
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->win) addqueue(sa->win, REDRAW, 1);
		if(sa->headwin) addqueue(sa->headwin, REDRAW, 1);
		sa= sa->next;
	}
}

void screen_swapbuffers()
{
	ScrArea *sa;
	int doswap= 0, headswap=0, oldwin;

	/* test op swap */
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->win_swap) doswap= 1;
		if(sa->head_swap) headswap= 1;
		
		sa= sa->next;
	}

	if(doswap==0 && headswap==0) return;

	oldwin= winget();
	winset(G.curscreen->mainwin);

	if(doswap) {
		readsource(SRC_FRONT);
		sa= G.curscreen->areabase.first;
		while(sa) {
			
			if(sa->win_swap) {
				sa->win_swap= 0;
				sa->win_equal= 0;
			}
			else if(sa->win_equal==0) {
				rectcopy(sa->winrct.xmin, sa->winrct.ymin, sa->winrct.xmax, sa->winrct.ymax, sa->winrct.xmin, sa->winrct.ymin);
				sa->win_equal= 1;
			}

			sa= sa->next;

		}
		swapbuffers();
		readsource(SRC_AUTO);
	}
	
	/* headers moeten in front en back gelijk zijn: dus: */
	/* heads van back naar frontbuffer? of omgekeerd? */
	if(doswap==0) {
		frontbuffer(TRUE);
		backbuffer(FALSE);	/* voor O2 */
	}
	else readsource(SRC_FRONT);
	
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->head_swap) {
			rectcopy(sa->headrct.xmin, sa->headrct.ymin, sa->headrct.xmax, sa->headrct.ymax, sa->headrct.xmin, sa->headrct.ymin);
			sa->head_swap= 0;
		}
		sa= sa->next;
	}
	
	frontbuffer(FALSE);
	backbuffer(TRUE);
	readsource(SRC_AUTO);
	
	if(oldwin) winset(oldwin);
}

int is_allowed_to_change_screen(bScreen *new)
{
	/* niet als curscreen is full
	 * niet als obedit && old->scene!=new->scene
	 */
	
	if(new==0) return 0;
	if(G.curscreen->full) return 0;
	if(curarea->full) return 0;
	if(G.obedit) {
		if(G.curscreen->scene!=new->scene) return 0;
	}
	return 1;
}


void screenmain()
{
	ScrArea *sa;
	bScreen *sc;
	float fac;
	long wx, wy, orx, ory;
	short a, event, val, towin, dodrawscreen= 0, inqueue;
	
	ext_mousemove= 1;	/* voor test aktieve area eerste keer */
	
	while(TRUE) {
		event= screen_qread(&val);

		towin= event;
		
		/* if(event!=MOUSEX && event!=MOUSEY && val) printf("%d %d\n", event, val); */
		
		if(event==LEFTMOUSE) {
			if(val && cursonedge) {
				moveareas();
				towin= 0;
			}
		}
		else if(event==MIDDLEMOUSE) {
			if(val && cursonedge) {
				editsplitpoint();
				towin= 0;
			}
		}
		else if(event==RIGHTMOUSE) {
			if(val && cursonedge) {
				joinarea(curarea);
				towin= 0;
			}
		}
		else if(event==QKEY) {
			if(G.obedit && G.obedit->type==OB_FONT && curarea->spacetype==SPACE_VIEW3D);
			else {
				if(okee("QUIT BLENDER")) exit_usiblender();
				towin= 0;
			}
		}
		else if(event==SPACEKEY) {
			if(G.obedit && G.obedit->type==OB_FONT && curarea->spacetype==SPACE_VIEW3D);
			else {
				if(val) toolbox();
				towin= 0;
			}
		}
		else if(event==INPUTCHANGE) {
			/* welke headers moeten redraw? */
			if(val>3) {	/* eerste drie nummers voor GL */
				if( G.curscreen->winakt  !=  val ) {
					sa= areawinar[G.curscreen->winakt];
					if(sa) addqueue(sa->headwin, REDRAW, 1);
					sa= areawinar[val];
					if(sa) addqueue(sa->headwin, REDRAW, 1);
				}

				/* testen of window nog bestaat (oude event bij join b.v.) */
				if(areawinar[val]) {
					/* als winakt==R.win mag alleen een GL-INPUTCHANGE winakt zetten */
					if(R.win==0 || G.curscreen->winakt!=R.win) G.curscreen->winakt= val;
					clear_global_filesel_vars();

				}
				else {
					G.curscreen->winakt= 0;
				}
			}
			towin= 0;
		}
		else if(event==DRAWEDGES) {
			towin= 0;
			dodrawscreen= 1;
		}
		else if(event==REDRAW) {
			towin= 0;
			if(val==G.curscreen->mainwin) {
				
				init_my_mainwin(mainwin);
				
				getsize(&wx, &wy);
				prefsizx= wx;
				prefsizy= wy;
				getorigin(&orx, &ory);
				prefstax= orx;
				prefstay= ory;

				G.curscreen->startx= orx; G.curscreen->starty= ory;
				G.curscreen->endx= wx+orx-1; G.curscreen->endy= wy+ory-1;
				G.curscreen->sizex= wx;
				G.curscreen->sizey= wy;
				
				markdirty_all();
				
				dodrawscreen= 1;

			}
			else if(val>3) {
				addqueue(val, REDRAW, val);
			}
			else if(R.win && val==R.win) {
				winset(R.win);
				getorigin(&orx, &ory);
				
				R.winxof= orx;
				R.winyof= ory;
				redraw_render_win(val);
			}
			
		}
		else if(event==RIGHTARROWKEY) {
			if(val && (G.qual & LR_CTRLKEY)) {
				sc= G.curscreen->id.next;
				if(is_allowed_to_change_screen(sc)) setscreen(sc);
				towin= 0;
			}
		}
		else if(event==LEFTARROWKEY) {
			if(val&& (G.qual & LR_CTRLKEY)) {
				sc= G.curscreen->id.prev;
				if(is_allowed_to_change_screen(sc)) setscreen(sc);
				towin= 0;
			}
		}
		else if(event==UPARROWKEY || event==DOWNARROWKEY) {
			if(val && (G.qual & LR_CTRLKEY)) {
				area_fullscreen();
				towin= 0;
			}
		}
		else if(event==AFTERQUEUE) {
			append_afterqueue();
		}

		if(towin) {

			towin= blenderqread(event, val);
			
			if(towin && G.curscreen->winakt) addqueue(G.curscreen->winakt, event, val);
		}
		
		/* window queues en swapbuffers */
		event= my_qtest();
		if(event==0 || event==EXECUTE) {		/* || event==MOUSEY ?? */
		
			inqueue= 1;
			while(inqueue) {
				inqueue= 0;
				winqueue_break= 0;
				
				sa= G.curscreen->areabase.first;
				while(sa) {
					/* bewust eerst header afhandelen, dan rest. Header is soms init */
					if(sa->headwin && sa->headqueue!=sa->hq) {
						defheadqread(sa); inqueue= 1;
					}
					if(winqueue_break) { /* mogelijk nieuwe G.curscreen */
						inqueue= 1;
						break;
					}

					if(sa->win && sa->winqueue!=sa->wq) {
						defwinqread(sa); inqueue= 1;
					}
					if(winqueue_break) { /* mogelijk nieuwe G.curscreen */
						inqueue= 1;
						break;
					}
					sa= sa->next;
				}
			}
			screen_swapbuffers();
			
			if(dodrawscreen) {
				drawscreen();
				dodrawscreen= 0;
			}
		}
		/* restore actieve area */
		if(G.curscreen->winakt != winget()) areawinset(G.curscreen->winakt);
	}
}

/* *********  AREAS  ************* */

void getdisplaysize()
{
	if(displaysizex==0) {	/* anders is setdisplaysize gebruikt */
		displaysizex= getgdesc(GD_XPMAX);
		displaysizey= getgdesc(GD_YPMAX);
	}
}


void setdisplaysize(int ex, int ey)
{
	displaysizex= ex;
	displaysizey= ey;
}

void setprefsize(int stax, int stay, int sizx, int sizy)
{

	if(stax<0) stax= 0;
	if(stay<0) stay= 0;
	if(sizx<320) sizx= 320;
	if(sizy<256) sizy= 256;

	if(stax+sizx>displaysizex) sizx= displaysizex-stax;
	if(stay+sizy>displaysizey) sizy= displaysizey-stay;
	if(sizx<320 || sizy<256) {
		printf("ERROR: illegal prefsize\n");
		return;
	}
	
	prefstax= stax;
	prefstay= stay;
	prefsizx= sizx;
	prefsizy= sizy;
}


ScrArea *findcurarea()
{
	ScrArea *sa;
	short mval[2];
	
	getmouseco_sc(mval);
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->v1->vec.x<=mval[0] && sa->v3->vec.x>=mval[0]) {
			if(sa->v1->vec.y<=mval[1] && sa->v2->vec.y>=mval[1]) {
				return sa;
			}
		}
		sa= sa->next;
	}
	return 0;
}

ScrVert *addscrvert(ListBase *lb, short x, short y)
{
	ScrVert *sv;
	
	sv= callocN(sizeof(ScrVert), "addscrvert");
	sv->vec.x= x;
	sv->vec.y= y;
	
	if(lb) addtail(lb, sv);
	
	return sv;
}

ScrEdge *addscredge(ListBase *lb, ScrVert *v1, ScrVert *v2)
{
	ScrEdge *se;
	
	se= callocN(sizeof(ScrEdge), "addscredge");
	if( (long)v1 > (long)v2 ) {
		se->v1= v2;
		se->v2= v1;
	}
	else {
		se->v1= v1;
		se->v2= v2;
	}
	
	if(lb) addtail(lb, se);

	return se;
}

ScrEdge *findscredge(ScrVert *v1, ScrVert *v2)
{
	ScrVert *sv;
	ScrEdge *se;
	
	if( (long)v1 > (long)v2 ) {
		sv= v1;
		v1= v2;
		v2= sv;
	}
	se= G.curscreen->edgebase.first;
	while(se) {
		if(se->v1==v1 && se->v2==v2) return se;
		se= se->next;
	}
	return 0;
}

void removedouble_scrverts()
{
	ScrVert *v1, *verg;
	ScrEdge *se;
	ScrArea *sa;
	
	verg= G.curscreen->vertbase.first;
	while(verg) {
		if(verg->new==0) {	/* !!! */
			v1= verg->next;
			while(v1) {
				if(v1->new==0) {	/* !?! */
					if(v1->vec.x==verg->vec.x && v1->vec.y==verg->vec.y) {
						/* printf("doublevert\n"); */
						v1->new= verg;
					}
				}
				v1= v1->next;
			}
		}
		verg= verg->next;
	}
	
	/* vervang pointers in edges en vlakken */
	se= G.curscreen->edgebase.first;
	while(se) {
		if(se->v1->new) se->v1= se->v1->new;
		if(se->v2->new) se->v2= se->v2->new;
		/* edges zijn veranderd: dus.... */
		if( (long)se->v1 > (long)se->v2 ) {
			v1= se->v1;
			se->v1= se->v2;
			se->v2= v1;
		}
		se= se->next;
	}
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->v1->new) sa->v1= sa->v1->new;
		if(sa->v2->new) sa->v2= sa->v2->new;
		if(sa->v3->new) sa->v3= sa->v3->new;
		if(sa->v4->new) sa->v4= sa->v4->new;
		sa= sa->next;
	}
	
	/* verwijderen */
	verg= G.curscreen->vertbase.first;
	while(verg) {
		v1= verg->next;
		if(verg->new) {
			remlink(&G.curscreen->vertbase, verg);
			freeN(verg);
		}
		verg= v1;
	}
	
}

void removenotused_scrverts()
{
	ScrVert *sv, *svn;
	ScrEdge *se;
	ScrArea *sa;

	/* ga ervan uit dat de edges goed zijn */
	
	se= G.curscreen->edgebase.first;
	while(se) {
		se->v1->flag= 1;
		se->v2->flag= 1;
		se= se->next;
	}
	
	sv= G.curscreen->vertbase.first;
	while(sv) {
		svn= sv->next;
		if(sv->flag==0) {
			remlink(&G.curscreen->vertbase, sv);
			freeN(sv);
		}
		else sv->flag= 0;
		sv= svn;
	}
}

void removedouble_scredges()
{
	ScrVert *v1;
	ScrEdge *verg, *se, *sn;
	
	/* vergelijken */
	verg= G.curscreen->edgebase.first;
	while(verg) {
		se= verg->next;
		while(se) {
			sn= se->next;
			if(verg->v1==se->v1 && verg->v2==se->v2) {
				/* printf("edge removed \n"); */
				remlink(&G.curscreen->edgebase, se);
				freeN(se);
			}
			se= sn;
		}
		verg= verg->next;
	}
}

void removenotused_scredges()
{
	ScrVert *sv;
	ScrEdge *se, *sen;
	ScrArea *sa;
	int a=0;
	
	/* zet flag als edge gebruikt wordt in area */
	sa= G.curscreen->areabase.first;
	while(sa) {
		se= findscredge(sa->v1, sa->v2);
		if(se==0) printf("error: area %d edge 1 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v2, sa->v3);
		if(se==0) printf("error: area %d edge 2 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v3, sa->v4);
		if(se==0) printf("error: area %d edge 3 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v4, sa->v1);
		if(se==0) printf("error: area %d edge 4 bestaat niet\n", a);
		else se->flag= 1;
		sa= sa->next;
		a++;
	}
	se= G.curscreen->edgebase.first;
	while(se) {
		sen= se->next;
		if(se->flag==0) {
			remlink(&G.curscreen->edgebase, se);
			freeN(se);
		}
		else se->flag= 0;
		se= sen;
	}
}

void calc_arearcts(ScrArea *sa)
{

	if(sa->v1->vec.x>0) sa->totrct.xmin= sa->v1->vec.x+EDGEWIDTH2+1;
	else sa->totrct.xmin= sa->v1->vec.x;
	if(sa->v4->vec.x<G.curscreen->sizex-1) sa->totrct.xmax= sa->v4->vec.x-EDGEWIDTH2-1;
	else sa->totrct.xmax= sa->v4->vec.x;
	
	if(sa->v1->vec.y>0) sa->totrct.ymin= sa->v1->vec.y+EDGEWIDTH2+1;
	else sa->totrct.ymin= sa->v1->vec.y;
	if(sa->v2->vec.y<G.curscreen->sizey-1) sa->totrct.ymax= sa->v2->vec.y-EDGEWIDTH2-1;
	else sa->totrct.ymax= sa->v2->vec.y;
		
	sa->winrct= sa->totrct;
	if(sa->headertype) {
		sa->headrct= sa->totrct;
		if(sa->headertype==HEADERDOWN) {
			sa->headrct.ymax= sa->headrct.ymin+HEADERY-1;
			sa->winrct.ymin= sa->headrct.ymax+1;
		}
		else if(sa->headertype==HEADERTOP) {
			sa->headrct.ymin= sa->headrct.ymax-HEADERY+1;
			sa->winrct.ymax= sa->headrct.ymin-1;
		}
	}
	if(sa->winrct.ymin>sa->winrct.ymax) sa->winrct.ymin= sa->winrct.ymax;
	
	/* als speedup voor berekeningen */
	sa->winx= sa->winrct.xmax-sa->winrct.xmin;
	sa->winy= sa->winrct.ymax-sa->winrct.ymin;
}

void openheadwin(ScrArea *sa)
{

	sa->headwin= myswinopen(G.curscreen->mainwin,
		sa->headrct.xmin, sa->headrct.xmax, sa->headrct.ymin, sa->headrct.ymax);

	mmode(MVIEWING);
	
	if(sa->headqueue==0) sa->headqueue= sa->hq= mallocN(2*MAXQUEUE, "headqueue");
	
	areawinar[sa->headwin]= sa;	/* anders doet addqueue het niet */
	addqueue(sa->headwin, CHANGED, 1);
	
}

void openareawin(ScrArea *sa)
{

	sa->win= myswinopen(G.curscreen->mainwin, 
		sa->winrct.xmin, sa->winrct.xmax, sa->winrct.ymin, sa->winrct.ymax);


	if(sa->winqueue==0) sa->winqueue= sa->wq= mallocN(2*MAXQUEUE, "winqueue");

	areawinar[sa->win]= sa;	/* anders doet addqueue het niet */
	addqueue(sa->win, CHANGED, 1);

}

void closeheadwin(ScrArea *sa)
{
	
	if(G.curscreen && sa->headwin==G.curscreen->winakt) G.curscreen->winakt= 0;
	
	if(sa->headwin) winclose(sa->headwin);
	sa->headwin= 0;
	sa->headdraw= 0;
	if(sa->headqueue) {
		freeN(sa->headqueue);
		sa->headqueue= sa->hq= 0;
	}
}

void closeareawin(ScrArea *sa)
{

	if(G.curscreen && sa->win==G.curscreen->winakt) G.curscreen->winakt= 0;
	
	if(sa->win) winclose(sa->win);
	sa->win= 0;
	if(sa->winqueue) {
		freeN(sa->winqueue);
		sa->winqueue= sa->wq= 0;
	}
}

void del_area(ScrArea *sa)
{
	closeareawin(sa);
	closeheadwin(sa);

	freespacelist(&sa->spacedata);
}

void copy_areadata(ScrArea *sa1, ScrArea *sa2)
{

	sa1->headertype= sa2->headertype;
	sa1->spacetype= sa2->spacetype;
	sa1->butspacetype= sa2->spacetype;
	sa1->headdraw= sa2->headdraw;
	sa1->windraw= sa2->windraw;
	sa1->headqread= sa2->headqread;
	sa1->winqread= sa2->winqread;
	sa1->headchange= sa2->headchange;
	sa1->winchange= sa2->winchange;
	
	freespacelist(&sa1->spacedata);
	
	duplicatespacelist(&sa1->spacedata, &sa2->spacedata);
	
}

ScrArea *addscrarea(lb, v1, v2, v3, v4, headertype, spacetype)
ListBase *lb;
ScrVert *v1, *v2, *v3, *v4;
short headertype, spacetype;
{
	ScrArea *sa;
	
	sa= callocN(sizeof(ScrArea), "addscrarea");
	sa->v1= v1;
	sa->v2= v2;
	sa->v3= v3;
	sa->v4= v4;
	sa->headertype= headertype;
	sa->spacetype= sa->butspacetype= spacetype;

	calc_arearcts(sa);
	
	if(headertype) {
		openheadwin(sa);
	}
	if(sa->winrct.ymin<sa->winrct.ymax) {
		openareawin(sa);
	}

	if(lb) addtail(lb, sa);

	return sa;
}

void testareas()
{
	ScrArea *sa;
	rcti tempw, temph;
	
	/* testen of header er moet zijn, of weg moet, of verplaatst */
	/* testen of window er moet zijn, of weg moet, of verplaatst */

	sa= G.curscreen->areabase.first;
	while(sa) {
		
		tempw= sa->winrct;
		temph= sa->headrct;
		calc_arearcts(sa);

		/* test header */
		if(sa->headertype) {
			if(sa->headwin==0) openheadwin(sa);
			else {
				/* is header op goede plek? */
				if(temph.xmin!=sa->headrct.xmin || temph.xmax!=sa->headrct.xmax ||
					temph.ymin!=sa->headrct.ymin || temph.ymax!=sa->headrct.ymax) {

					addqueue(sa->headwin, CHANGED, 1);
					mywinposition(sa->headwin, sa->headrct.xmin, sa->headrct.xmax, sa->headrct.ymin, sa->headrct.ymax);

				}
				
				if(sa->headbutlen<sa->winx) {
					sa->headbutofs= 0;
					addqueue(sa->headwin, CHANGED, 1);
				}
				else if(sa->headbutofs+sa->winx > sa->headbutlen) {
					sa->headbutofs= sa->headbutlen-sa->winx;
					addqueue(sa->headwin, CHANGED, 1);
				}
			}
		}
		else {
			if(sa->headwin) closeheadwin(sa);
		}

		/* test areawindow */
		if(sa->win==0) {
			if(sa->winrct.ymin<sa->winrct.ymax) openareawin(sa);
		}
		else { /* window te klein? */
			if(sa->winrct.ymin==sa->winrct.ymax) closeareawin(sa);
			else {  /* window veranderd? */
				if(tempw.xmin!=sa->winrct.xmin || tempw.xmax!=sa->winrct.xmax ||
					tempw.ymin!=sa->winrct.ymin || tempw.ymax!=sa->winrct.ymax) {

					addqueue(sa->win, CHANGED, 1);
					mywinposition(sa->win, sa->winrct.xmin, sa->winrct.xmax, sa->winrct.ymin, sa->winrct.ymax);
				}
				
			}
		}

		sa= sa->next;
	}
	
	/* remake global windowarray */
	bzero(areawinar, 4*MAXWIN);
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->headwin) areawinar[sa->headwin]= sa;
		if(sa->win) areawinar[sa->win]= sa;
		sa= sa->next;
	}
	
	/* test of winakt in orde is */	
	if( areawinar[G.curscreen->winakt]==0) G.curscreen->winakt= 0;
}

ScrArea *test_edge_area(ScrArea *sa, ScrEdge *se)
{
	/* test of edge in area ligt, zo niet, 
	   vind een area die 'm wel heeft */
  
	ScrEdge *se1=0, *se2=0, *se3=0, *se4=0;
	
	if(sa) {
		se1= findscredge(sa->v1, sa->v2);
		se2= findscredge(sa->v2, sa->v3);
		se3= findscredge(sa->v3, sa->v4);
		se4= findscredge(sa->v4, sa->v1);
	}
	if(se1!=se && se2!=se && se3!=se && se4!=se) {
		
		sa= G.curscreen->areabase.first;
		while(sa) {
			/* een beetje optimaliseren? */
			if(se->v1==sa->v1 || se->v1==sa->v2 || se->v1==sa->v3 || se->v1==sa->v4) {
				se1= findscredge(sa->v1, sa->v2);
				se2= findscredge(sa->v2, sa->v3);
				se3= findscredge(sa->v3, sa->v4);
				se4= findscredge(sa->v4, sa->v1);
				if(se1==se || se2==se || se3==se || se4==se) return sa;
			}
			sa= sa->next;
		}
	}

	return sa;	/* is keurig 0 als niet gevonden */
}

ScrArea *closest_bigger_area()
{
	ScrArea *sa, *big=0;
	float cent[3], vec[3],len, len1, len2, len3, dist=1000;
	short mval[2];
	
	getmouseco_sc(mval);
	
	cent[0]= mval[0];
	cent[1]= mval[1];
	cent[2]= vec[2]= 0;

	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa!=curarea) {
			if(sa->winy>=curarea->winy) {
			
				/* mimimum van vier hoekpunten */
				vec[0]= sa->v1->vec.x; vec[1]= sa->v1->vec.y;
				len= VecLenf(vec, cent);
				vec[0]= sa->v2->vec.x; vec[1]= sa->v2->vec.y;
				len1= VecLenf(vec, cent);
				vec[0]= sa->v3->vec.x; vec[1]= sa->v3->vec.y;
				len2= VecLenf(vec, cent);
				vec[0]= sa->v4->vec.x; vec[1]= sa->v4->vec.y;
				len3= VecLenf(vec, cent);
				
				len= MIN4(len, len1, len2, len3);
				
				/* plus centrum */
				vec[0]= (sa->v2->vec.x+sa->v3->vec.x)/2;
				vec[1]= (sa->v1->vec.y+sa->v2->vec.y)/2;

				len+= 0.5*VecLenf(vec, cent);
				
				if(len<dist) {
					dist= len;
					big= sa;
				}
			}
		}
		sa= sa->next;
	}
	
	if(big) return big;
	else return curarea;
}

/* ************ SCREENBEHEER ************** */


bScreen *addscreen(char *name)		/* gebruik de setprefsize() als je anders dan fullscreen wilt */
{
	/* deze functie zet de variabele G.curscreen
	 * omdat alle hulpfuncties moeten weten welk screen
	 */
	bScreen *sc;
	ScrVert *sv1, *sv2, *sv3, *sv4;
	ScrEdge *se;
	ScrArea *sa;
	short startx, starty, endx, endy;	
	
	sc= G.curscreen= alloc_libblock(&G.main->screen, ID_SCR, name);

	if(prefsizx) {
		startx= prefstax;
		starty= prefstay;
		endx= prefstax+prefsizx-1;
		endy= prefstay+prefsizy-1;
	}
	else {
		prefstax= startx= 0;
		prefstay= starty= 0;
		endx= displaysizex-1;
		endy= displaysizey-1;
		prefsizx= displaysizex;
		prefsizy= displaysizey;
	}

	sc->startx= startx;	sc->starty= starty;
	sc->endx= endx;	sc->endy= endy;
	sc->sizex= sc->endx-sc->startx+1;
	sc->sizey= sc->endy-sc->starty+1;
	
	sc->scene= G.scene;
	
	if(mainwin==0) {
		
		#ifndef FREE
		decodekeytab();
		#endif
		
		prefposition(sc->startx, sc->endx, sc->starty, sc->endy);
		noborder();
		sc->mainwin= mainwin= winopen("Blender");
		RGBmode();
		doublebuffer();
		gconfig();
		cpack(0x707070); clear(); swapbuffers();
		cpack(0x707070); clear(); swapbuffers();
		setmapcolors();
		init_my_mainwin(mainwin);
		
		setvaluator(MOUSEX, (sc->startx+sc->endx-20)/2, 0, displaysizex);
		setvaluator(MOUSEY, (sc->starty+sc->endy-20)/2, 0, displaysizey);
		
		qdevice(RAWKEYBD);
		qdevice(LEFTMOUSE);
		qdevice(MIDDLEMOUSE);
		qdevice(RIGHTMOUSE);
		qdevice(MOUSEX);
		qdevice(MOUSEY);
		qdevice(WINFREEZE);
		qdevice(WINTHAW);
		qdevice(DRAWOVERLAY);
		
	}
	else sc->mainwin= mainwin;
	
	sv1= addscrvert(&sc->vertbase, 0, 0);
	sv2= addscrvert(&sc->vertbase, 0, sc->endy-sc->starty);
	sv3= addscrvert(&sc->vertbase, sc->sizex-1, sc->sizey-1);
	sv4= addscrvert(&sc->vertbase, sc->sizex-1, 0);
	
	se= addscredge(&sc->edgebase, sv1, sv2);
	se= addscredge(&sc->edgebase, sv2, sv3);
	se= addscredge(&sc->edgebase, sv3, sv4);
	se= addscredge(&sc->edgebase, sv4, sv1);
	
	sa= addscrarea(&sc->areabase, sv1, sv2, sv3, sv4, HEADERDOWN, SPACE_EMPTY);
	
	G.curscreen= sc;
	
	return sc;
}

/* niet screen zelf vrijgeven */
void free_screen(bScreen *sc)
{
	ScrArea *sa;

	freelistN(&sc->vertbase);
	freelistN(&sc->edgebase);
	
	sa= sc->areabase.first;
	while(sa) {
		del_area(sa);
		if(sa==curarea) curarea= 0;
		sa= sa->next;
	}
	freelistN(&sc->areabase);
	if(G.curscreen==sc) {
		G.curscreen= 0;
		winqueue_break= 1;	/* overal uit queue's gaan */
	}
}

void setscreen(bScreen *sc)
{
	bScreen *sc1;
	ScrArea *sa;
	bWindow *win;
	int firstwin= 0;
	short mval[2];
	
	if(sc->full) {				/* vind de bijhorende full */
		sc1= G.main->screen.first;
		while(sc1) {
			sa= sc1->areabase.first;
			if(sa->full==sc) {
				sc= sc1;
				break;
			}
			sc1= sc1->id.next;
		}
		if(sc1==0) printf("setscreen error\n");
	}

	/* G.curscreen de-activeren */
	if(G.curscreen && G.curscreen!=sc) {

		sa= G.curscreen->areabase.first;
		while(sa) {
			if(sa->win) winclose(sa->win);
			sa->win= 0;
			if(sa->headwin) winclose(sa->headwin);
			sa->headwin= 0;
			
			sa->hq= sa->headqueue; /* queue leeg */
			sa->wq= sa->winqueue;
			
			sa= sa->next;
		}		
	}
	if(G.curscreen != sc) {
		winset(sc->mainwin);
		drawmode(OVERDRAW);
		color(0);
		clear();
		drawmode(NORMALDRAW);
	}

	G.curscreen= sc;
	G.scene= sc->scene;
	countall();
	
	/* recalculate winakt */
	getmouseco_sc(mval);

	testareas();
	
	G.curscreen->winakt= 0;
	win= swindowbase.first;
	while(win) {
		if(firstwin==0) firstwin= win->id;
		if(mval[0]>=win->xmin-1 && mval[0]<=win->xmax+2) {
			if(mval[1]>=win->ymin-1 && mval[1]<=win->ymax+2) {
				G.curscreen->winakt= win->id;
				break;
			}
		}
		win= win->next;
	}
	/* als buhv cursor op edge staat */
	if(G.curscreen->winakt==0) G.curscreen->winakt= firstwin;
	
	areawinset(G.curscreen->winakt);	/* zet curarea */
	
	qenter(DRAWEDGES, 1);
	winqueue_break= 1;		/* overal uit de queue's gaan */
	
	curedge= 0;		/* global voor move en join */
}

void area_fullscreen()	/* met curarea */
{
	/* deze funktie toggelt: als area full is wordt de parent weer zichtbaar */
	bScreen *sc, *oldscreen;
	ScrArea *new, *old;
	
	if(curarea->full) {
		sc= curarea->full;	/* de oude screen */
		sc->full= 0;
		
		/* vind oude area */
		old= sc->areabase.first;
		while(old) {
			if(old->full) break;
			old= old->next;
		}
		if(old==0) {error("something wrong in areafullscreen"); return;}
		
		copy_areadata(old, curarea);
		old->full= 0;
		
		free_libblock(&G.main->screen, G.curscreen);
		setscreen(sc);
		
	}
	else {
		/* is er maar 1 area? */
		if(G.curscreen->areabase.first==G.curscreen->areabase.last) return;
		if(curarea->spacetype==SPACE_INFO) return;
		
		G.curscreen->full= 1;
		
		old= curarea;		
		oldscreen= G.curscreen;
		sc= addscreen("temp");		/* deze zet G.curscreen */

		splitarea( (ScrArea *)sc->areabase.first, 'h', 0.99);
		new= sc->areabase.first;
		newspace(new->next, SPACE_INFO);
		
		curarea= old;
		G.curscreen= oldscreen;	/* moet voor setscreen */
		
		/* area kopieeren */
		copy_areadata(new, curarea);
		
		curarea->full= oldscreen;
		new->full= oldscreen;
		new->next->full= oldscreen;
		
		setscreen(sc);
		wich_cursor(new);
		
	}
}

void copy_screen(bScreen *to, bScreen *from)
{
	ScrVert *s1, *s2;
	ScrEdge *se;
	ScrArea *sa;
	ListBase lbase;
	
	/* alles van to vrijgeven */
	free_screen(to);
		
	duplicatelist(&to->vertbase, &from->vertbase);
	duplicatelist(&to->edgebase, &from->edgebase);
	duplicatelist(&to->areabase, &from->areabase);
	
	s1= from->vertbase.first;
	s2= to->vertbase.first;
	while(s1) {
		s1->new= s2;
		s2= s2->next;
		s1= s1->next;
	}
	se= to->edgebase.first;
	while(se) {
		se->v1= se->v1->new;
		se->v2= se->v2->new;
		if( (long)se->v1 > (long)se->v2 ) {
			s1= se->v1;
			se->v1= se->v2;
			se->v2= s1;
		}
		se= se->next;
	}

	sa= to->areabase.first;
	while(sa) {
		sa->v1= sa->v1->new;
		sa->v2= sa->v2->new;
		sa->v3= sa->v3->new;
		sa->v4= sa->v4->new;
		sa->win= 0;
		sa->headwin= 0;
		sa->headqueue= sa->hq= 0;
		sa->winqueue= sa->wq= 0;
		
		duplicatespacelist(&lbase, &sa->spacedata);
		sa->spacedata= lbase;
		
		sa= sa->next;
	}
	
	/* op nul zetten (nodig?) */
	s1= from->vertbase.first;
	while(s1) {
		s1->new= 0;
		s1= s1->next;
	}
}

void duplicate_screen()
{
	bScreen *sc, *oldscreen;
	
	if(G.curscreen->full) return;
	
	/* nieuw screen maken: */

	oldscreen= G.curscreen;
	sc= addscreen(oldscreen->id.name+2);	/* deze zet G.curscreen */
	copy_screen(sc, oldscreen);

	G.curscreen= oldscreen;
	setscreen(sc);

}

void test_scale_screen(bScreen *sc)
/* testen of screenvertices vergroot/verkleind moeten worden */
/* testen of offset nog klopt */
{
	ScrVert *sv;
	float facx, facy, tempf;

	sc->startx= prefstax;
	sc->starty= prefstay;
	sc->endx= prefstax+prefsizx-1;
	sc->endy= prefstay+prefsizy-1;

	if(sc->sizex!= prefsizx || sc->sizey!= prefsizy) {
		facx= prefsizx;
		facx/= sc->sizex;
		facy= prefsizy;
		facy/= sc->sizey;

		sv= sc->vertbase.first;
		while(sv) {
			tempf= (sv->vec.x)*facx;
			sv->vec.x= (short)(tempf+0.5);
			tempf= (sv->vec.y )*facy;
			sv->vec.y= (short)(tempf+0.5);

			sv= sv->next;
		}
		
		sc->sizex= prefsizx;
		sc->sizey= prefsizy;
	}

	
}

/* ************ END SCREENBEHEER ************** */
/* ************  JOIN/SPLIT/MOVE ************** */

void joinarea(ScrArea *sa)
{
	ScrArea *sa2, *san;
	ScrArea *up=0, *down=0, *right=0, *left=0;
	ScrEdge *setest;
	short event, val=0;
	char str[64];
	
	/* welke edge? */
	if(curedge==0) return;
	
	/* zit edge in area? of anders: welke area */
	sa= test_edge_area(sa, curedge);
	if(sa==0) return;
		
	/* welke edges kunnen ermee? */
	/* vind richtingen met zelfde edge */
	sa2= G.curscreen->areabase.first;
	while(sa2) {
		if(sa2 != sa) {
			setest= findscredge(sa2->v1, sa2->v2);
			if(curedge==setest) right= sa2;
			setest= findscredge(sa2->v2, sa2->v3);
			if(curedge==setest) down= sa2;
			setest= findscredge(sa2->v3, sa2->v4);
			if(curedge==setest) left= sa2;
			setest= findscredge(sa2->v4, sa2->v1);
			if(curedge==setest) up= sa2;
		}
		sa2= sa2->next;
	}
	
	sa2= 0;
	setest= 0;
	
	if(left) val++;
	if(up) val++;
	if(right) val++;
	if(down) val++;
	
	if(val==0) return;
	else if(val==1) {
		if(left) sa2= left;
		else if(up) sa2= up;
		else if(right) sa2= right;
		else if(down) sa2= down;
	}
	
	if(okee("Join")) {
		
		if(sa2) {
			/* nieuwe area is oude sa */
			if(sa2==left) {
				sa->v1= sa2->v1;
				sa->v2= sa2->v2;
				addscredge(&G.curscreen->edgebase, sa->v2, sa->v3);
				addscredge(&G.curscreen->edgebase, sa->v1, sa->v4);
			}
			else if(sa2==up) {
				sa->v2= sa2->v2;
				sa->v3= sa2->v3;
				addscredge(&G.curscreen->edgebase, sa->v1, sa->v2);
				addscredge(&G.curscreen->edgebase, sa->v3, sa->v4);
			}
			else if(sa2==right) {
				sa->v3= sa2->v3;
				sa->v4= sa2->v4;
				addscredge(&G.curscreen->edgebase, sa->v2, sa->v3);
				addscredge(&G.curscreen->edgebase, sa->v1, sa->v4);
			}
			else if(sa2==down) {
				sa->v1= sa2->v1;
				sa->v4= sa2->v4;
				addscredge(&G.curscreen->edgebase, sa->v1, sa->v2);
				addscredge(&G.curscreen->edgebase, sa->v3, sa->v4);
			}
		
			/* edge en area weg */
			/* remlink(&G.curscreen->edgebase, setest); */
			/* freeN(setest); */
			del_area(sa2);
			remlink(&G.curscreen->areabase, sa2);
			if(curarea==sa2) curarea=0;
			freeN(sa2);
			
			removedouble_scredges();
			removenotused_scredges();
			removenotused_scrverts();	/* moet als laatste */
			
			testareas();
			qenter(DRAWEDGES, 1);
			qenter(MOUSEY, -1);		/* test cursor en inputwindow */
		}
	}
}

short testsplitpoint(ScrArea *sa, char dir, float fac)
/* return 0: geen split mogelijk */
/* else return (integer) screencoordinaat splitpunt */
{
	short val, x, y;
	
	/* area groot genoeg? */
	val= 0;
	if(sa->v4->vec.x- sa->v1->vec.x <= 2*AREAMINX) return 0;
	if(sa->v2->vec.y- sa->v1->vec.y <= 2*AREAMINY) return 0;

	/* voor zekerheid */
	if(fac<0.0) fac= 0.0;
	if(fac>1.0) fac= 1.0;
	
	if(dir=='h') {
		y= sa->v1->vec.y+ fac*(sa->v2->vec.y- sa->v1->vec.y);
		
		if(sa->v2->vec.y==G.curscreen->sizey-1 && sa->v2->vec.y- y < HEADERY+EDGEWIDTH2) 
			y= sa->v2->vec.y- HEADERY-EDGEWIDTH2;

		else if(sa->v1->vec.y==0 && y- sa->v1->vec.y < HEADERY+EDGEWIDTH2)
			y= sa->v1->vec.y+ HEADERY+EDGEWIDTH2;

		else if(y- sa->v1->vec.y < AREAMINY) y= sa->v1->vec.y+ AREAMINY;
		else if(sa->v2->vec.y- y < AREAMINY) y= sa->v2->vec.y- AREAMINY;
		else y-= (y % AREAGRID);

		return y;
	}
	else {
		x= sa->v1->vec.x+ fac*(sa->v4->vec.x- sa->v1->vec.x);
		if(x- sa->v1->vec.x < AREAMINX) x= sa->v1->vec.x+ AREAMINX;
		else if(sa->v4->vec.x- x < AREAMINX) x= sa->v4->vec.x- AREAMINX;
		else x-= (x % AREAGRID);

		return x;
	}
}

void splitarea(ScrArea *sa, char dir, float fac)
{
	bScreen *sc;
	ScrArea *new;
	ScrVert *sv1, *sv2;
	short split;
	
	if(sa==0) return;
	
	split= testsplitpoint(sa, dir, fac);
	if(split==0) return;
	
	sc= G.curscreen;
	
	areawinset(sa->win);
	drawmode(OVERDRAW); color(0); clear(); drawmode(NORMALDRAW);
	
	if(dir=='h') {
		/* nieuwe vertices */
		sv1= addscrvert(&sc->vertbase, sa->v1->vec.x, split);
		
		sv2= addscrvert(&sc->vertbase, sa->v4->vec.x, split);
		
		/* nieuwe edges */
		addscredge(&sc->edgebase, sa->v1, sv1);
		addscredge(&sc->edgebase, sv1, sa->v2);
		addscredge(&sc->edgebase, sa->v3, sv2);
		addscredge(&sc->edgebase, sv2, sa->v4);
		addscredge(&sc->edgebase, sv1, sv2);
		
		/* nieuwe areas: boven */
		new= addscrarea(&sc->areabase, sv1, sa->v2, sa->v3, sv2, sa->headertype, sa->spacetype);
		copy_areadata(new, sa);

		/* area onder */
		sa->v2= sv1;
		sa->v3= sv2;
		
	}
	else {
		/* nieuwe vertices */
		sv1= addscrvert(&sc->vertbase, split, sa->v1->vec.y);
		
		sv2= addscrvert(&sc->vertbase, split, sa->v2->vec.y);
		
		/* nieuwe edges */
		addscredge(&sc->edgebase, sa->v1, sv1);
		addscredge(&sc->edgebase, sv1, sa->v4);
		addscredge(&sc->edgebase, sa->v2, sv2);
		addscredge(&sc->edgebase, sv2, sa->v3);
		addscredge(&sc->edgebase, sv1, sv2);
		
		/* nieuwe areas: links */
		new= addscrarea(&sc->areabase, sa->v1, sa->v2, sv2, sv1, sa->headertype, sa->spacetype);
		copy_areadata(new, sa);

		/* area rechts */
		sa->v1= sv1;		
		sa->v2= sv2;
	}
	
	/* dubbele vertices en edges verwijderen */
	removedouble_scrverts();
	removedouble_scredges();
	removenotused_scredges();
	
	testareas();
}

void editsplitpoint()
{
	ScrArea *sa;
	float fac;
	short ok= 0, event, val, split, mval[2], mvalo[2]={-1010, -1010}, col[3];
	char dir;
	
	sa= test_edge_area(curarea, curedge);
	if(sa==0) return;
	
	if(sa->win==0) return;
	if(sa->full) return;
	if(curedge==0) return;
	if(okee("Split")==0) return;	

	if(curedge->v1->vec.x==curedge->v2->vec.x) dir= 'h';
	else dir= 'v';
	
	winset(sa->win);
	
	drawmode(PUPDRAW);
	getmcolor(2, col, col+1, col+2);
	mapcolor(2, 255, 0, 0);
	
	/* standaard ortho */
	ortho2(-0.5, sa->winrct.xmax-sa->winrct.xmin-0.5, -0.5, sa->winrct.ymax-sa->winrct.ymin-0.5);
	loadmatrix(matone);

	addqueue(sa->win, CHANGED, 1);
	addqueue(sa->headwin, CHANGED, 1);
		
	/* rekening houden met grid en minsize */
	while(ok==0) {
		getmouseco_sc(mval);
		
		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];
			
			if(dir=='h') {
				fac= mval[1]- (sa->v1->vec.y);
				fac/= sa->v2->vec.y- sa->v1->vec.y;
			} else {
				fac= mval[0]- sa->v1->vec.x;
				fac/= sa->v4->vec.x- sa->v1->vec.x;
			}
			
			split= testsplitpoint(sa, dir, fac);
			if(split==0) {
				ok= -1;
			}
			else {
				color(0); clear(); color(1);

				if(dir=='h') {
					if(sa->headertype==1) split-= HEADERY;
					color(2);
					sboxs(0, split-sa->totrct.ymin, sa->totrct.xmax - sa->totrct.xmin, split-sa->totrct.ymin);
					color(3);
					sboxs(1, split-sa->totrct.ymin+1, sa->totrct.xmax - sa->totrct.xmin+1, split-sa->totrct.ymin+1);
					if(sa->headertype==1) split+= HEADERY;
				}
				else {
					color(2);
					sboxs(split-sa->totrct.xmin, 0, split-sa->totrct.xmin, sa->totrct.ymax - sa->totrct.ymin);
					color(3);
					sboxs(split-sa->totrct.xmin+1, 1, split-sa->totrct.xmin+1, sa->totrct.ymax - sa->totrct.ymin+1);
				}
			}
		}
		
		event= extern_qread(&val);
		if(val && event==LEFTMOUSE) {
			if(dir=='h') {
				fac= split- (sa->v1->vec.y);
				fac/= sa->v2->vec.y- sa->v1->vec.y;
			}
			else {
				fac= split- sa->v1->vec.x;
				fac/= sa->v4->vec.x- sa->v1->vec.x;
			}
			ok= 1;
		}
		if(val && event==ESCKEY) {
			ok= -1;
		}

	}

	color(0); clear();
	mapcolor(2, col[0], col[1], col[2]);
	drawmode(NORMALDRAW);
	
	if(ok==1) {
		splitarea(sa, dir, fac);
		qenter(DRAWEDGES, 1);
	}
}

void moveareas()
{
	ScrEdge *se;
	ScrVert *v1;
	ScrArea *sa;
	vec2s addvec;
	float PdistVL2Dfl(), vec1[2], vec2[2], vec3[2];
	int dist, mindist= 1<<30, oneselected;
	short event=0, val, split, mval[2], mvalo[2];
	short x1, x2, y1, y2, bigger, smaller, col[3];
	char dir;
	
	if(curarea->full) return;
	
	if(curedge==0 || curedge->border) return;
	
	/* select connected, alleen in de juiste richting */
	/* 'dir' is de richting van de EDGE */
	curedge->v1->flag= 1;
	curedge->v2->flag= 1;
	if(curedge->v1->vec.x==curedge->v2->vec.x) dir= 'v';
	else dir= 'h';
	
	oneselected= 1;
	while(oneselected) {
		se= G.curscreen->edgebase.first;
		oneselected= 0;
		while(se) {
			if(se->v1->flag+ se->v2->flag==1) {
				if(dir=='h') if(se->v1->vec.y==se->v2->vec.y) {
					se->v1->flag= se->v2->flag= 1;
					oneselected= 1;
				}
				if(dir=='v') if(se->v1->vec.x==se->v2->vec.x) {
					se->v1->flag= se->v2->flag= 1;
					oneselected= 1;
				}
			}
			se= se->next;
		}
	}
	
	/* nu zijn alle vertices met 'flag==1' degene die verplaatst kunnen worden. */
	/* we lopen de areas af en testen vrije ruimte met MINSIZE */
	bigger= smaller= 10000;
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(dir=='h') {	/* als top of down edge select, test hoogte */
			if(sa->v1->flag && sa->v4->flag) {
				if(sa->v2->vec.y==G.curscreen->sizey-1)	/* bovenste edge */
					y1= sa->v2->vec.y - sa->v1->vec.y-HEADERY-EDGEWIDTH2;
				else 
					y1= sa->v2->vec.y - sa->v1->vec.y-AREAMINY;
				if(y1<bigger) bigger= y1;
			}
			else if(sa->v2->flag && sa->v3->flag) {
				if(sa->v1->vec.y==0)	/* onderste edge */
					y1= sa->v2->vec.y - sa->v1->vec.y-HEADERY;
				else
					y1= sa->v2->vec.y - sa->v1->vec.y-AREAMINY;
				if(y1<smaller) smaller= y1;
			}
		}
		else {	/* als left of right edge select, test breedte */
			if(sa->v1->flag && sa->v2->flag) {
				x1= sa->v4->vec.x - sa->v1->vec.x-AREAMINX;
				if(x1<bigger) bigger= x1;
			}
			else if(sa->v3->flag && sa->v4->flag) {
				x1= sa->v4->vec.x - sa->v1->vec.x-AREAMINX;
				if(x1<smaller) smaller= x1;
			}
		}
		sa= sa->next;
	}
	winset(G.curscreen->mainwin);

	ortho2(-0.5, G.curscreen->sizex-0.5, -0.5, G.curscreen->sizey-0.5);

	drawmode(PUPDRAW);
	getmcolor(2, col, col+1, col+2);
	mapcolor(2, 255, 0, 0);
	
	getmouseco_sc(mvalo);
	addvec.x= 0;
	addvec.y= 0;
	
	while(getbutton(LEFTMOUSE)) {
		getmouseco_sc(mval);
		
		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
			
			if(dir=='h') {
				addvec.y+= mval[1]-mvalo[1];
				if(addvec.y>bigger) addvec.y= bigger;
				if(addvec.y<-smaller) addvec.y= -smaller;
			}
			else {
				addvec.x+= mval[0]-mvalo[0];
				if(addvec.x>bigger) addvec.x= bigger;
				if(addvec.x<-smaller) addvec.x= -smaller;
			}
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];

			/* edges tekenen */
			color(0); clear();

			se= G.curscreen->edgebase.first;
			while(se) {
				if(se->v1->flag && se->v2->flag) {
					/* met areagrid even behelpen, verderop is OK! */
					x1= se->v1->vec.x+addvec.x-(addvec.x % AREAGRID);
					x2= se->v2->vec.x+addvec.x-(addvec.x % AREAGRID);
					y1= se->v1->vec.y+addvec.y-(addvec.y % AREAGRID);
					y2= se->v2->vec.y+addvec.y-(addvec.y % AREAGRID);
					
					color(2);
					sboxs(x1, y1, x2, y2);
					color(3);
					sboxs(x1+1, y1+1, x2+1, y2+1);
				}
				se= se->next;
			}
		}
		
		event= extern_qread(&val);
		if(val) {
			if(event==ESCKEY) break;
			if(event==LEFTMOUSE) break;
			if(event==SPACEKEY) break;
		}

	}
	
	v1= G.curscreen->vertbase.first;
	while(v1) {
		if(v1->flag && event!=ESCKEY) {
		
			/* zo is AREAGRID netjes */
			if(addvec.x && v1->vec.x>0 && v1->vec.x<G.curscreen->sizex-1) {
				v1->vec.x+= addvec.x;
				if(addvec.x != bigger && addvec.x != -smaller) v1->vec.x-= (v1->vec.x % AREAGRID);
			}
			if(addvec.y && v1->vec.y>0 && v1->vec.y<G.curscreen->sizey-1) {
				v1->vec.y+= addvec.y;
				if(addvec.y != bigger && addvec.y != -smaller) v1->vec.y-= (v1->vec.y % AREAGRID);
			}
		}
		v1->flag= 0;
		v1= v1->next;
	}
	
	color(0); clear();
	mapcolor(2, col[0], col[1], col[2]);
	drawmode(NORMALDRAW);
	
	if(event!=ESCKEY) {
		removedouble_scrverts();
		removedouble_scredges();
		testareas();
	}
	
	qenter(DRAWEDGES, 1);
}

void scrollheader()
{
	short mval[2], mvalo[2];
	
	if(curarea->headbutlen<curarea->winx) {
		curarea->headbutofs= 0;
	}
	else if(curarea->headbutofs+curarea->winx > curarea->headbutlen) {
		curarea->headbutofs= curarea->headbutlen-curarea->winx;
	}

	getmouseco_sc(mvalo);

	while( getbutton(MIDDLEMOUSE) ) {
		getmouseco_sc(mval);
		if(mval[0]!=mvalo[0]) {
			curarea->headbutofs-= (mval[0]-mvalo[0]);

			if(curarea->headbutlen-curarea->winx < curarea->headbutofs) curarea->headbutofs= curarea->headbutlen-curarea->winx;
			if(curarea->headbutofs<0) curarea->headbutofs= 0;

			defheadchange();
			defheaddraw();
			screen_swapbuffers();
			
			mvalo[0]= mval[0];
		}
		else sginap(2);
	}
}

int select_area(int spacetype)
{
	/* vanuit editroutines aanroepen, als er meer area's
	 * zijn van type 'spacetype' kan er een area aangegeven worden
	 */
	ScrArea *sa, *sact;
	bWindow *win;
	int tot=0;
	short event, val, mval[2];
	
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==spacetype) {
			sact= sa;
			tot++;
		}
		sa= sa->next;
	} 
	
	if(tot==0) {
		error("Can't do this! Open correct window");
		return 0;
	}
	
	if(tot==1) {
		if(curarea!=sact) areawinset(sact->win);
		return 1;
	}
	else if(tot>1) {
		tempcursor(12);
		while(1) {
			event= extern_qread(&val);
			
			if(val) {
				if(event==ESCKEY) break;
				if(event==LEFTMOUSE) break;
				if(event==SPACEKEY) break;
			}
		}
		tempcursor(0);
		
		/* recalculate winakt */
		getmouseco_sc(mval);
	
		G.curscreen->winakt= 0;
		win= swindowbase.first;
		while(win) {
			if(mval[0]>=win->xmin-1 && mval[0]<=win->xmax+2) {
				if(mval[1]>=win->ymin-1 && mval[1]<=win->ymax+2) {
					G.curscreen->winakt= win->id;
					break;
				}
			}
			win= win->next;
		}
		
		if(event==LEFTMOUSE) {
			sa= areawinar[G.curscreen->winakt];
			if(sa->spacetype==spacetype) areawinset(G.curscreen->winakt);
			else {
				error("wrong window");
				return 0;
			}
		}
	}
	
	if(event==LEFTMOUSE) return 1;
	else return 0;
}

/* ************  END JOIN/SPLIT/MOVE ************** */
/* **************** DRAW SCREENEDGES ***************** */

void drawedge(short x1, short y1, short x2, short y2)
{
	int a, dir;
	vec2s v1, v2;

	v1.x= x1;
	v1.y= y1;
	v2.x= x2;
	v2.y= y2;
	
	if(v1.x== v2.x) {		/* vertikaal */
		dir= v1.y-v2.y;
		if(dir>0) dir= 1; else dir= -1;
		
		v1.y-= dir*EDGEWIDTH2;
		v2.y+= dir*EDGEWIDTH2;
		
		v1.x+= EDGEWIDTH2;
		v2.x+= EDGEWIDTH2;
		for(a=0; a<EDGEWIDTH; a++) {
			cpack(edcol[a]);
			LINE2S(&v1, &v2);
			v1.x--; v2.x--;
					/* en dit voor de afgeronde tuitjes */
			if(a<EDGEWIDTH2) { v1.y+= dir; v2.y-=dir;}
			else { v1.y-= dir; v2.y+=dir;}
		}
		
		drawmode(OVERDRAW);
		color(0);
		sboxfs(v1.x-EDGEWIDTH, v1.y, v1.x+EDGEWIDTH, v2.y);
		drawmode(NORMALDRAW);

	}
	else {					/* horizontaal */
		dir= v1.x-v2.x;
		if(dir>0) dir= 1; else dir= -1;
		v1.x-= dir*EDGEWIDTH2;
		v2.x+= dir*EDGEWIDTH2;
		
		v1.y-= EDGEWIDTH2;
		v2.y-= EDGEWIDTH2;
		for(a=0; a<EDGEWIDTH; a++) {
			cpack(edcol[a]);
			LINE2S(&v1, &v2);
			v1.y++; v2.y++;
					/* en dit voor de afgeronde tuitjes */
			if(a<EDGEWIDTH2) { v1.x+= dir; v2.x-=dir;}
			else { v1.x-= dir; v2.x+=dir;}
		}

		drawmode(OVERDRAW);
		color(0);
		sboxfs(v1.x, v1.y-EDGEWIDTH, v2.x, v1.y+EDGEWIDTH);
		drawmode(NORMALDRAW);
		
	}
	
}

void drawscredge(ScrEdge *se)
{
	bScreen *sc;
	vec2s *v1, *v2;
	
	sc= G.curscreen;
	
	v1= &(se->v1->vec);
	v2= &(se->v2->vec);
	
	/* borders screen niet tekenen */
	se->border= 1;
	if(v1->x<=0 && v2->x<=0) return;
	if(v1->x>=sc->sizex-1 && v2->x>=sc->sizex-1) return;
	if(v1->y<=0 && v2->y<=0) return;
	if(v1->y>=sc->sizey-1 && v2->y>=sc->sizey-1) return;
	se->border= 0;

	drawedge(v1->x, v1->y, v2->x, v2->y);
}

void drawscreen()
{
	ScrEdge *se;

	winset(G.curscreen->mainwin);
	ortho2(-0.5, G.curscreen->sizex-0.5, -0.5, G.curscreen->sizey-0.5);

	/* edges in mainwin */
	frontbuffer(TRUE);
	se= G.curscreen->edgebase.first;
	while(se) {
		drawscredge(se);
		se= se->next;
	}

	frontbuffer(FALSE);
}

/* ********************************* */

bScreen *default_foursplit() 
{
	bScreen *sc;
	ScrArea *sa;
	View3D *vd;
	int tel;
	
	sc= addscreen("screen");
	
	splitarea( (ScrArea *)sc->areabase.first, 'h', 0.25);
	splitarea( (ScrArea *)sc->areabase.last, 'h', 0.49);
	splitarea( (ScrArea *)sc->areabase.last, 'h', 0.99);

	sa= sc->areabase.last;
	newspace(sa, SPACE_INFO);
	/* sa->headertype= NOHEADER; */

			/* alle nieuwe areas komen op einde lijst! */
	sa= sc->areabase.first;
	splitarea( sa->next, 'v', 0.5);
	sa= sc->areabase.first;
	splitarea( sa->next, 'v', 0.5);
	
	sa= sc->areabase.first;
	sa->headertype= HEADERTOP;
	newspace(sa, SPACE_BUTS);
	sa= sa->next;
	
	/* zet de view3d's en geef ze de juiste aanzicht */
	tel= 1;
	while(sa) {
		if(sa->spacetype==0) {
			newspace(sa, SPACE_VIEW3D);
			vd= sa->spacedata.first;
			vd->persp= 0;

			if(tel==1) {
				vd->view= 1;
				vd->viewquat[0]= fcos(M_PI/4.0);
				vd->viewquat[1]= -fsin(M_PI/4.0);
			}
			else if(tel==2) {
				vd->view= 3;
				vd->viewquat[0]= 0.5;
				vd->viewquat[1]= -0.5;
				vd->viewquat[2]= -0.5;
				vd->viewquat[3]= -0.5;
			}
			else if(tel==3) {
				vd->view= 7;
			}
			else vd->persp= 1;
			
			tel++;
		}
		sa= sa->next;
	}
	
	testareas();
	
	return sc;
}

bScreen *default_twosplit() 
{
	bScreen *sc;
	ScrArea *sa;

	sc= addscreen("screen");
	
	splitarea( (ScrArea *)sc->areabase.first, 'h', 0.99);
	sa= sc->areabase.first;
	newspace(sa, SPACE_VIEW3D);
	newspace(sa->next, SPACE_INFO);
	
	return sc;
}

void initscreen()
{
	/* opent en initialiseert */
	getdisplaysize();
	
	default_twosplit();
		
}


