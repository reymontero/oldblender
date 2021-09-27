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



/* renderfg.c  april 94		GRAPHICS
 * 
 * render foreground routines
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "screen.h"
#include "render.h"

#include <signal.h>

extern ulong pr_scan[];

/* ~~~~~~~~~~~~~TIMECURSOR~~~~~~~~~~~~~~~~~~~~~~~ */

char numbar[10][8]= {
	0, 0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c,
	0, 0x18, 0x8, 0x8, 0x8, 0x8, 0x8, 0x1c, 
	0, 0x3c, 0x42, 0x4, 0x8, 0x10, 0x20, 0x7e, 
	0, 0x3e, 0x4, 0x8, 0x1c, 0x2, 0x42, 0x3c, 
	0, 0x4, 0xc, 0x14, 0x24, 0x7e, 0x4, 0x4, 
	0, 0x3e, 0x20, 0x3c, 0x2, 0x2, 0x22, 0x1c, 
	0, 0x1c, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x1c, 
	0, 0x3e, 0x2, 0x4, 0x8, 0x10, 0x10, 0x10, 
	0, 0x3c, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x3c, 
	0, 0x1c, 0x22, 0x22, 0x1e, 0x2, 0x22, 0x1c,
};

void set_timecursor(int nr)
{
	int a, dig;
	ushort curs[32];
	char *cp;
	
	/* erg raar, maar deze return is vanwege een X 'bug': 
	 * 
	 * Zonder return knalt-ie er in de setmapcolors (screen.c) uit met een X error.
	 * Op deze manier is het ook netter: timecursor bleef 'hangen' na ESC.
	 * 
	 */
	if(winget() == R.win) return;
	
	if(G.machine!=IRIS) curstype(C16X2);
	
	for(a=0; a<16; a++) curs[a]= 0xFFFF;
	for(a=16; a<32; a++) curs[a]= 0x0;
	
	/* kleiner dan 10 */
	cp= ((char *)curs)+1;
	dig= nr % 10;
	
	for(a=7; a>=0; a--, cp+=2) {
		cp[0]= ~numbar[dig][a];
		cp[32]= numbar[dig][a];
	}
	
	if(nr>9) {
		cp= (char *)curs;
		dig= (nr/10) % 10;
		for(a=7; a>=0; a--, cp+=2) {
			cp[0]= ~numbar[dig][a];
			cp[32]= numbar[dig][a];
		}
		if(nr>99) {
			cp= ((char *)(curs))+17;
			dig= (nr/100) % 10;
			for(a=7; a>=0; a--, cp+=2) {
				cp[0]= ~numbar[dig][a];
				cp[32]= numbar[dig][a];
			}
			if(nr>999) {
				cp= ((char *)(curs))+16;
				dig= (nr/1000) % 10;
				for(a=7; a>=0; a--, cp+=2) {
					cp[0]= ~numbar[dig][a];
					cp[32]= numbar[dig][a];
				}
			}
		}
	}
	
	defcursor(15, curs);
	curorigin(15, 8, 8);
	
	setcursor(15, 0, 0);
}

/* ~~~~~~~~~~~~~TIMER~~~~~~~~~~~~~~~~~~~~~~~~~ */


int test_break()
{
	short val;
	
	if(G.background) return 0;
	if(G.afbreek==2) {

		/* queue testen */
		
		G.afbreek= 0;
		while(qtest()) {
			if( extern_qread(&val) == ESCKEY) {
				G.afbreek= 1;
			}
		}
	}

	if(G.afbreek==1) return 1;
	else return 0;
}


/* int curscounter= 0; */

void interruptESC(sig)
long sig;
{

	if(G.afbreek==0) G.afbreek= 2;	/* code voor queue lezen */

	/* opnieuw zetten: wordt namelijk gereset */
	signal(SIGVTALRM, interruptESC);
}

void bgntimer()
{

	struct itimerval tmevalue;

	tmevalue.it_interval.tv_sec = 0;
	tmevalue.it_interval.tv_usec = 250000;
	/* wanneer de eerste ? */
	tmevalue.it_value.tv_sec = 0;
	tmevalue.it_value.tv_usec = 10000;

	signal(SIGVTALRM, interruptESC);
	setitimer(ITIMER_VIRTUAL, &tmevalue, 0);

}

void endtimer()
{
	struct itimerval tmevalue;

	tmevalue.it_value.tv_sec = 0;
	tmevalue.it_value.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &tmevalue, 0);
	signal(SIGVTALRM, SIG_IGN);

}
/* ~~~~~~~~~~~~~DISPLAY~~~~~~~~~~~~~~~~~~~~~~~ */


void init_render_display()
{
	ScrArea *sa;
	View3D *vd;
	float xm, ym;
	static int xs=0, ys;
	long xo, yo;
	int winok=0, div, inview=0;

	if(G.background) return;

	/* eerst uitzoeken waar de display komt */
	if(R.displaymode==R_DISPLAYVIEW) {
		if(R.win) {
			winclose(R.win);
			R.win= 0;
		}
		/* elke area die persp==2 en goede camera heeft: display maken */
		sa= G.curscreen->areabase.first;
		while(sa) {
			if(sa->win && sa->spacetype==SPACE_VIEW3D) {
				vd= sa->spacedata.first;
				if(vd->persp==2 && vd->camera==G.scene->camera) {
					inview= 1;
					areawinset(sa->win);
					drawmode(OVERDRAW); color(0); clear(); drawmode(NORMALDRAW);
					vd->pr_sizex= vd->pr_xmax- vd->pr_xmin;
					vd->pr_sizey= vd->pr_ymax- vd->pr_ymin;
					vd->pr_facx= ( (float)vd->pr_sizex)/((float)R.rectx);
					vd->pr_facy= ( (float)vd->pr_sizey)/((float)R.recty);
				}
				else {
					vd->pr_sizex= 0;
				}
			}
			sa= sa->next;
		}
	
		if(inview==0) {
			/* eerste view3d nemen en dze initialiseren */
			sa= G.curscreen->areabase.first;
			while(sa) {
				if(sa->spacetype==SPACE_VIEW3D) {
					vd= sa->spacedata.first;
					
					areawinset(sa->win);
					calc_viewborder();
					
					drawmode(OVERDRAW); color(0); clear(); drawmode(NORMALDRAW);
					vd->pr_sizex= vd->pr_xmax- vd->pr_xmin;
					vd->pr_sizey= vd->pr_ymax- vd->pr_ymin;
					vd->pr_facx= ( (float)vd->pr_sizex)/((float)R.rectx);
					vd->pr_facy= ( (float)vd->pr_sizey)/((float)R.recty);
					
					break;
				}
				sa= sa->next;
			}
		}
	}
	if(R.displaymode==R_DISPLAYWIN) {

		if(xs==0) {
			xs = displaysizex;
			ys = displaysizey;
		}
		
		/* eerst berekenen: waar moet ie staan */
		if(R.winpos) {
			div= 0; 
			xm= 0.0; 
			ym= 0.0;
			if(R.winpos & (1+8+64)) {
				div++; 
				xm+=.1666;
			}
			if(R.winpos & (2+16+128)) {
				div++; 
				xm+= .50;
			}
			if(R.winpos & (4+32+256)) {
				div++; 
				xm+=.8333;
			}
			xm/= (float)div;
			div= 0;
			if(R.winpos & (1+2+4)) {
				div++; 
				ym+=.1666;
			}
			if(R.winpos & (8+16+32)) {
				div++; 
				ym+= .50;
			}
			if(R.winpos & (64+128+256)) {
				div++; 
				ym+=.8333;
			}
			ym/= (float)div;
			
			R.winxof= (xs*xm- R.rectx/2);
			R.winyof= (xs*ym- R.recty/2);
		}
		if(R.winxof+R.rectx-1>xs) R.winxof= xs-R.rectx-1;
		if(R.winyof+R.recty-1>ys-20) R.winyof= ys-20-R.recty-1;
		if(R.winxof<0) R.winxof=0;
		if(R.winyof<0) R.winyof=0;
		
		if(R.rectx>=xs-2) R.winxof=0;
		if(R.recty>=ys-2) R.winyof=0;
		
		/* window testen */
		if(R.win) {
			winset(R.win);
			if(R.winx!=R.rectx || R.winy!=R.recty) {
				cpack(0x777777);
				clear();
			}
			else winok= 1;
			
			if(R.winpop) {	/* alleen als weggepusht */
				getorigin(&xo, &yo);
				if(xo!=R.winxof || yo!=R.winyof) winok= 0;
			}
		}

		
		if(winok==0) {
	
			/* (voorlopig) altijd sluiten */
			if(R.win) winclose(R.win);
			R.win= 0;
			
			if(R.win==0) {
				prefposition(R.winxof, R.winxof+R.rectx-1, R.winyof, R.winyof+R.recty-1);
				noborder();
				minsize(2, 2);
				/* printf("ervoor\n"); */
				R.win= winopen("render");
				RGBmode();
				singlebuffer();
				gconfig();
				sginap(3);
				/* PRINT(d, R.win); */
			}
			else {
				winposition(R.winxof,R.winxof+R.rectx-1,R.winyof,R.winyof+R.recty-1);
				reshapeviewport();
			}

			R.winx= R.rectx;
			R.winy= R.recty;

			cpack(0x0);
			clear();
	
		}
		if(R.winpop) {
			/* printf("voor winpop\n"); */
			winpop();
			sginap(3);
			/* printf("na winpop\n"); */
			
			cpack(0x0);
			clear();
		}
		R.winpop= 0;
		

	}
	
	/* moet altijd ivm clear overdraw */
	areawinset(G.curscreen->winakt);
}


void display_scanline(View3D *vd, ulong *rect, int starty)
{
	ulong *scan;
	int x, y, dx, dy, endy, startx, endx, sizex;
	int tx=0, ty;
	short *sp;
	
	rect+= starty*R.rectx;
	
	endy= vd->pr_ymin+ ffloor(vd->pr_facy*(starty+1)) -1;
	starty= vd->pr_ymin+ ffloor(vd->pr_facy*starty);
	
	if(starty<curarea->winrct.ymin) return;
	if(endy>=curarea->winrct.ymax) return;
	
	startx= vd->pr_xmin;
	endx= vd->pr_xmax;
	
	if(startx<curarea->winrct.xmin) {
		rect+= (int)(( curarea->winrct.xmin - startx)/vd->pr_facx);
		startx= curarea->winrct.xmin;
	}
	if(endx>=curarea->winrct.xmax) endx= curarea->winrct.xmax;
	
	sizex= endx-startx+1;

	/* scanline uitvergroten */
	dx= (int) (65536.0/vd->pr_facx);
	scan= pr_scan;
	sp= (short *)&tx;
	x= sizex;
	while( x-- ) {
		*scan= *rect;
		scan++;
		tx+= dx;
		while( *sp>0 ) {
			tx-= 65536;
			rect++;
		}
	}
	
	/* scanline neerzetten */
	for(y=starty; y<= endy; y++) {
		lrectwrite(startx, y, endx-1, y, pr_scan);
	}
}


void render_display(int starty, int endy)
{
	ScrArea *sa;
	View3D *vd;
	ulong *rect;
	int y;

	if(G.background) return;

	if(R.displaymode==R_DISPLAYWIN) {
		if(R.win) {
			winset(R.win);
			rect= R.rectot+starty*R.rectx;
			lrectwrite(0, starty, R.rectx-1, endy, rect);
		}
	}
	else {
		sa= G.curscreen->areabase.first;
		while(sa) {
			if(sa->spacetype==SPACE_VIEW3D) {
				vd= sa->spacedata.first;
				if(vd->pr_sizex) {
					if(sa->win != winget()) areawinset(sa->win);
					frontbuffer(TRUE);
					
					for(y= starty; y<=endy; y++) {
						display_scanline(vd, R.rectot, y);
					}
					frontbuffer(FALSE);
					vd->flag |= V3D_DISPIMAGE;
				}
			}
			sa= sa->next;
		}
	}
}

void clear_render_display()
{

	if(G.background) return;

	if(R.win) {
		winset(R.win);
		cpack(0x0);
		clear();
		areawinset(G.curscreen->winakt);
	}
	else {
		;
	}
}

void redraw_render_win(val)
{
	if(val==R.win) {
		if(R.rectot) {
			winset(R.win);
			lrectwrite(0, 0, R.rectx-1, R.recty-1, R.rectot);
		
			areawinset(G.curscreen->winakt);
		}
	}
}

void redraw_render_display()
{
	if(R.win) redraw_render_win(R.win);
	else {
		;
	}
}

void toggle_render_display()
{
	ScrArea *sa;
	View3D *vd;
	int ok= 1;

	
	if(R.displaymode==R_DISPLAYWIN) {
		if(R.rectot) {
			if(R.win) {
				winset(R.win);
				if(R.winpop) {
					init_render_display();	/* controleert size */
					R.winpop= 0;
				}
				else {
					winpush();
					R.winpop= 1;
				}
				winset(G.curscreen->winakt);
			}
			else {
				init_render_display();
				redraw_render_win(R.win);
			}
		}
	}
	else if(R.displaymode==R_DISPLAYVIEW) {
		if(R.rectot) {
			if(R.win && R.winpop==0) {
				winset(R.win);
				winpush();
				R.winpop= 1;
				winset(G.curscreen->winakt);
				return;
			}
			sa= G.curscreen->areabase.first;
			while(sa) {
				if(sa->spacetype==SPACE_VIEW3D) {
					vd= sa->spacedata.first;
					if((vd->flag & V3D_DISPIMAGE)) {
						ok= 0;
						break;
					}
				}
				sa= sa->next;
			}
			if(ok) {
				init_render_display();
				render_display(0, R.recty-1);
			}
			else {
				/* redraw maken */
				sa= G.curscreen->areabase.first;
				while(sa) {
					if(sa->spacetype==SPACE_VIEW3D) {
						vd= sa->spacedata.first;
						if((vd->flag & V3D_DISPIMAGE)) addqueue(sa->win, REDRAW, 1);
					}
					sa= sa->next;
				}
			}
		}
	}
}

void zoomwin()
{
	ScrArea *sa;
	static float zoom=3.0;
	float fx, fy;
	ulong *rz, *rect;
	int q_event;
	short x, y, cx, cy, cxo=12000, cyo=0, lus=1, val, mval[2];
	Device mdev[2];

	mdev[0] = MOUSEX;
    mdev[1] = MOUSEY;
	winset(R.win);
	
	while(lus) {
	
		getdev(2, mdev, mval);
		x= mval[0]-R.winxof;
		y= mval[1]-R.winyof;

		/* (x,y) is het samplepunt in de rect */
		/* de linkeronderhoek van de 'virtuele rect zit op: */
		
		fx= ((float)x)/(float)R.rectx;
		fy= ((float)y)/(float)R.recty;
		
		cx= -fx*( (zoom-1.0)*R.rectx);
		cy= -fy*( (zoom-1.0)*R.recty);
		
		if(cx==cxo && cy==cyo);
		else {
			cxo=cx; cyo=cy;
			rectwrite_part(0, 0, R.rectx, R.recty, cx, cy, R.rectx, R.recty, zoom, R.rectot);
		
		}
		while(qtest()!=0 && lus==1) {
			q_event= qread(&val);
			if(val) {
				if(q_event==ESCKEY) lus=0;
				else if(q_event==INPUTCHANGE) {
					lus=0;
					qenter(q_event, val);
				}
				else if(q_event==PADMINUS) {
					zoom-=1.0; cxo= 0;
					if(zoom<2.0) zoom= 2.0;
					else rectzoom(zoom, zoom);
				}
				else if(q_event==PADPLUSKEY) {
					zoom+=1.0;
					if(zoom>15.0) zoom= 15.0;
					else rectzoom(zoom, zoom);
					cxo=0;
				}
			}
		}
		sginap(1);
	}
	
	lrectwrite(0, 0, R.rectx-1, R.recty-1, R.rectot);
}

/* ***************************************** */

void printrenderinfo(int mode)
{
	ScrArea *sa;
	extern long mem_in_use;
	int x, y;
	char str[300], tstr[32];
	
	if(G.background) return;
	
	timestr(G.cputime, tstr);
	sprintf(str, "RENDER  Fra:%d  Ve:%d Fa:%d  La:%d  Mem:%.2fM Time:%s (%.2f) ",
			CFRA, R.totvert, R.totvlak, R.totlamp,  (mem_in_use>>10)/1024.0, tstr, ((float)(G.time-G.cputime))/100);

	if(R.r.mode & R_FIELDS) {
		if(R.flag & R_SEC_FIELD) strcat(str, "Field B ");
		else strcat(str, "Field A ");
	}

	if(mode!= -1) {
		sprintf(tstr, "Sample: %d ", mode);
		strcat(str, tstr);
	}
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==SPACE_INFO) {
			areawinset(sa->headwin);
			frontbuffer(TRUE);
			
			x= sa->headbutlen-28;
			y= 6;

			cpack(0xA08060);
			sboxfi(x-4, y-2, x+55, y+13);
			
			cpack(0x909090);
			sboxfi(x+55, y-3, x+1280, y+14);
			
			cmov2i(x, y);
		
			cpack(0);
			fmsetfont(G.fonts);
			fmprstr(str);
			frontbuffer(FALSE);
			
		}
		sa= sa->next;
	}

}

/* ***************************************** */

void do_renderfg(int anim)
{

	waitcursor(1);

	G.afbreek= 0;

	if(G.obedit) {
		exit_editmode(0);	/* 0 = geen freedata */
	}

	bgntimer();
	if(anim) {
		animrender();
		do_global_buttons(B_NEWFRAME);
	}
	else {
		initrender();
		if(R.r.mode & R_FIELDS) do_global_buttons(B_NEWFRAME);
	}
	endtimer();

	/* areawinset(G.curscreen->winakt); */

	R.flag= 0;
	
	waitcursor(0);

	free_filesel_spec(G.scene->r.pic);

	G.afbreek= 0;
}

