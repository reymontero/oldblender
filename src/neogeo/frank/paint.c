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

#include "iff.h"
#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h> 
#include <fcntl.h>
#include <math.h>
#include <fmclient.h>
#include <time.h>
#include <local/util.h>
#include <local/gl_util.h>
#include <gl/image.h>

short mousex,mousey,mousexs,mouseys,mousek,mouse[2];
short brsize,brtype,inmode,nxtval,bmode;
Device dev[2];
long sizex,sizey,ofsx,ofsy,output,input;
long frontcol,backcol,nxtevent;
Screencoord sbl,sbr,sbb,sbt;
Screencoord bsl,bsr,bsb,bst;
struct ImBuf *ibuf = 0, *bbuf = 0, *ubuf = 0, *mbuf = 0, *tbuf = 0;

short *mouselist;
long *keylist;
long maxmouse=4096,maxkey=1024;
long curmouse,curkey,readmouse,readkey;
long keys;

#define BRPI 0
#define BRSQ 1
#define BRCI 2

#define LIVE 0
#define REPLAY 1

/* pseudo devices */

#define FCOL1 USERVALOFFSET+0
#define FCOL2 USERVALOFFSET+1
#define BCOL1 USERVALOFFSET+2
#define BCOL2 USERVALOFFSET+3

typedef struct {
	short y,xl,xr,dy;
} FillSeg;

#define MAXFILL 10000

#define PUSHF(Y,XL,XR,DY)   \
	if (sp<stackmax && Y+(DY)>=0 && Y+(DY)<=may)  \
	{sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define POPF(Y,XL,XR,DY) \
	{sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp ->xr;}


void Fill(x, y, nv, xsize, ysize, rectot)
short x,y;
unsigned long nv;
long xsize, ysize;
unsigned long *rectot;
{
	short l,x1,x2,dy,max,may,minx,maxx;
	unsigned long ov,*rect,*r,*rmin;
	FillSeg stack[MAXFILL], *sp= stack, *stackmax;

	r= rectot+ y*xsize+x;
	ov= *r;
	if(ov==nv) return;

	stackmax=stack+MAXFILL;
	max= xsize-1;
	may= ysize-1;
	PUSHF(y,x,x,1);
	PUSHF(y+1,x,x,-1);

	while (sp>stack) {
		POPF(y,x1,x2,dy);
		rect= rectot+y*xsize;
		r=rect+x1;
		maxx=x1;
		for(x=x1;x>=0 && *r==ov; x--) *(r--)=nv;
		minx=x;
		rmin=r;
		if(x==x1) goto skip;
		l=x+1;
		if(l<x1) PUSHF(y,l,x1-1,-dy);
		x=x1+1;

		do {
			r= rect+x;
			for(;x<=max && *r==ov; x++) *(r++)=nv;
			maxx=x;
			PUSHF(y,l,x-1,dy);
			if(x>x2+1) PUSHF(y,x2+1,x-1,-dy);
skip:
			for(x++,r++;x<=x2 && *r!=ov; x++) r++;
			l=x;
		} while (x<=x2);
	}
}

long dflt_in_out(struct ImBuf * ibuf, long x, long y)
{
	if (ibuf == 0) return (0);
	if (x < 0 || y < 0 || x >= ibuf->x || y >= ibuf->y || ibuf->rect == 0) return (-1);

	if (ibuf->rect[(y * ibuf->x) + x]) return (1);
	return(0);
}


short * outline(
struct ImBuf * ibuf, 
long startx, long starty, 
long (*in_or_out)())
{
	static long dirs[8][2] = {
		-1,  0,		-1,  1,		0,  1,		 1,  1, 
		1,  0,		 1, -1,		0, -1,		-1, -1, 
	};

	long dir, x, y, in, i;
	long count, done;
	short * pnts;

	if (ibuf == 0) return (0);
	if (ibuf->rect == 0) return (0);

	if (in_or_out == 0) in_or_out = dflt_in_out;
	in = in_or_out(ibuf, startx, starty);
	printf("in = %d %d %d %d\n", in, startx, starty, ibuf->rect[(ibuf->x * starty) + startx]);

	/* zoek naar eerste overgang en ga van daar uit 'zoeken' */

	for (y = starty; y < ibuf->y; y++) {
		for (x = startx; x < ibuf->x; x++) {
			if (in_or_out(ibuf, x, y) != in) {
				/* eerste punt gevonden !! */

				if (x != startx) dir = 0;
				else dir = 6;

				startx = x; 
				starty = y;
				count = 1; 
				done = FALSE;
				cpack(0x0000ff);
				do{
					pnt2i(x, y); 
					i = 0;
					while(in_or_out(ibuf, x + dirs[dir][0], y + dirs[dir][1]) == in) {
						dir = (dir + 1) & 0x7;
						if (i++ == 8) break;
					}
					if (i == 8) break;
					x += dirs[dir][0];
					y += dirs[dir][1];
					dir = (dir - 3) & 0x7;
				}while(x != startx || y != starty);
				return(0);
			}
		}
	}
	printf("geen overgang \n");
	return(0);
}

long qutest()
{
	if (inmode == LIVE){
		return qtest();
	}
	if (input) return nxtevent;
	inmode == LIVE;
	return qtest();
}


long quread(val)
short *val;
{
	long thisevent,out;

	if (inmode==REPLAY){
		if (input){
			*val=nxtval;
			thisevent=nxtevent;
			if (read(input,&nxtevent,4) != 4) inmode == LIVE;
			nxtval = nxtevent & 0x0ffff;
			nxtevent >>= 16;
			nxtevent &= 0x0ffff;
			return (thisevent);
		} else{
			inmode == LIVE;
		}
	}
	thisevent=qread(val);

	switch(thisevent){
	case LEFTSHIFTKEY:
		if (*val) keys |= LSHIFT;
		else keys &= ~LSHIFT;
		break;
	case RIGHTSHIFTKEY:
		if (*val) keys |= RSHIFT;
		else keys &= ~RSHIFT;
		break;
	case LEFTMOUSE:
		if (*val) keys |= LMOUSE;
		else keys &= ~LMOUSE;
		break;
	}

	if (output>0){
		out=TRUE;
		switch (thisevent){
		case MOUSEX:
		case MOUSEY:
		case REDRAW:
		case INPUTCHANGE:
		case RIGHTCTRLKEY:
		case LEFTARROWKEY:
		case RIGHTARROWKEY:
		case UPARROWKEY:
		case DOWNARROWKEY:
			out=FALSE;
			break;
		}

		if (out){
			out = (thisevent << 16) | (*val & 0x0ffff);
			if (curkey==maxkey){
				maxkey += maxkey;
				fprintf(stderr,"reallocating keylist %d %d\n",maxkey,curkey);
				keylist = (long *)realloc(keylist,maxkey<<4);
			}
			keylist[curkey]=out;
			curkey++;
		}
	}
	return (thisevent);
}

void mergebrush(x,y)
short x,y;
{
	long i,col;

	lrectread(x-bsl,y-bsb,x+bsr,y+bst,mbuf->rect);

	switch (bmode){
	case 'n':
		{
			uchar *rect1,*rect2;
			uchar a;

			rect1= ( uchar *) bbuf->rect;
			rect2= ( uchar *) mbuf->rect;

			for(i=mbuf->x * mbuf->y;i>0;i--){
				a = rect1[0];
				if (a){
					if (a == 255) {
						rect2[1] = rect1[1];
						rect2[2] = rect1[2];
						rect2[3] = rect1[3];
					} else {
						a++;
						rect2[1] = (a * (rect1[1] - rect2[1]) + (rect2[1] << 8)) >> 8;
						rect2[2] = (a * (rect1[2] - rect2[2]) + (rect2[2] << 8)) >> 8;
						rect2[3] = (a * (rect1[3] - rect2[3]) + (rect2[3] << 8)) >> 8;
					}
				}
				rect1 += 4;
				rect2 += 4;
			}
		}
		break;
	case 'a':
		{
			uchar *rectc1,*rectc2;
			short col1,col2,r,g,b;

			rectc1= ( uchar *) bbuf->rect;
			rectc2= ( uchar *) mbuf->rect;

			for(i=mbuf->x * mbuf->y;i>0;i--){
				*(rectc2++) -= *(rectc1++);
				*(rectc2++) -= *(rectc1++);
				*(rectc2++) -= *(rectc1++);
				*(rectc2++) -= *(rectc1++);
			}
		}
		break;
	case 'z':
		{
			char *rectc1,*rectc2;
			short col1,col2,r,g,b;

			if (bmode=='a') col=frontcol;
			else col=backcol;

			r = col & 0x0ff;
			g = (col>>8) & 0x0ff;
			b = (col>>16) & 0x0ff;

			rectc1= ( char *) bbuf->rect;
			rectc2= ( char *) mbuf->rect;

			for(i=mbuf->x * mbuf->y;i>0;i--){
				col1 = *(rectc1++);
				if (col1>128){
					rectc2 += 1;
					col1 = *(rectc1++);
					col2 = *(rectc2);
					*(rectc2++) = ((col2 << 8) + col1 * (b - col2)) >> 8;
					col1 = *(rectc1++);
					col2 = *(rectc2);
					*(rectc2++) = ((col2 << 8) + col1 * (g - col2)) >> 8;
					col1 = *(rectc1++);
					col2 = *(rectc2);
					*(rectc2++) = ((col2 << 8) + col1 * (r - col2)) >> 8;
				} else{
					rectc1 +=3;
					rectc2 +=4;
				}
			}
		}
		break;
	case 'f':
		swapfilt(mbuf);
		break;
	}

	lrectwrite(x-bsl,y-bsb,x+bsr,y+bst,mbuf->rect);
}


void drawbrushf(x,y)
float x,y;
{
	short ix,iy;

	switch(brtype){
	case BRSQ:
	case BRCI:
		translate(x,y,0);
		callobj(1);
		translate(-x,-y,0);
		break;
	case BRPI:
		ix=x+.5;
		iy=y+.5;
		mergebrush(ix,iy);
		break;
	}
}


void drawbrushi(x,y)
short x,y;
{
	switch(brtype){
	case BRSQ:
	case BRCI:
		translate(x,y,0);
		callobj(1);
		translate(-x,-y,0);
		break;
	case BRPI:
		mergebrush(x,y);
		break;
	}
}


void buildbox()
{
	if (mousex>sbr) sbr=mousex;
	if (mousex<sbl) sbl=mousex;
	if (mousey<sbb) sbb=mousey;
	if (mousey>sbt) sbt=mousey;
}


void initview()
{
	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);
	ortho2(-0.5,sizex-0.5,-0.5,sizey-0.5);
	viewport(0,sizex-1,0,sizey-1);
}


short cont()
{
	long event;
	short val;

	while (event=qutest()){
		switch(event){
		case MOUSEX:
		case MOUSEY:
		case REDRAW:
		case INPUTCHANGE:
			event=quread(&event);
			break;
		default:
			return (FALSE);
		}
	}

	switch (inmode){
	case LIVE:
		getdev(2,dev,mouse);
		mouse[0] -= ofsx;
		mouse[1] -= ofsy;
		if (maxmouse <= curmouse+3){
			maxmouse += maxmouse;
			mouselist = (short *)realloc(mouselist,maxmouse<<2);
			fprintf(stderr,"reallocating %d \n",maxmouse);
		}
		if ((mousex != mouse[0]) | (mousey != mouse[1]) | (curmouse==0)){
			if (curmouse==0){
				mouselist[0] = mousek;
				curmouse++;
			}
			mouselist[curmouse] = mouse[0];
			mouselist[curmouse+1] = mouse[1];
			curmouse += 2;
		}
		return (TRUE);
	case REPLAY:
		if (readmouse < curmouse){
			mouse[0] = mouselist[readmouse];
			mouse[1] = mouselist[readmouse+1];
			readmouse += 2;
			return (TRUE);
		}
		return (FALSE);
	}
}


void sketch()
{
	drawbrushi(mousex,mousey);
	cont();
	mousex=mouse[0];
	mousey=mouse[1];

	while (cont()){
		if ((mousex != mouse[0]) | (mousey != mouse[1])){
			drawbrushi(mousex,mousey);
			mousex=mouse[0];
			mousey=mouse[1];
		}
	}

}

void fill()
{
	Fill(mousex, mousey, frontcol, ibuf->x, ibuf->y, ibuf->rect);
	lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
	while (cont()){
		sginap(1);
	}
}


void drawline(sx,sy,ex,ey)
short sx,sy;
short ex,ey;
{
	short i;
	float xstep,ystep,x,y;

	x=sx;
	y=sy;

	if ((sx == ex) && (sy == ey)){
		i=0;
	} else{

		xstep=ex-sx;
		ystep=ey-sy;

		if (fabs(xstep) > fabs(ystep)){
			ystep /= fabs(xstep);
			i = fabs(xstep);
			if (xstep >0) xstep=1;
			else if (xstep<0) xstep = -1;
		}
		else{
			xstep /= fabs(ystep);
			i = fabs(ystep);
			if (ystep >0) ystep=1;
			else if (ystep<0) ystep = -1;
		}
	}

	for(;i>=0;i--){
		drawbrushf(x,y);
		x += xstep;
		y += ystep;
	}
}


void drawit()
{
	drawbrushi(mousex,mousey);
	while(cont()){
		if ( (mouse[0]!=mousex) || (mouse[1]!=mousey) ){
			drawline(mousex,mousey,mouse[0],mouse[1]);
			mousex=mouse[0];
			mousey=mouse[1];
		}
	}
}


void frect()
{
	short linexs,lineys;

	linexs=mousex;
	lineys=mousey;

	while(cont()){
		if ( (mouse[0]!=mousex) || (mouse[1]!=mousey) ){
			lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
			mousex=mouse[0];
			mousey=mouse[1];
			rectf(linexs,lineys,mousex,mousey);
			sginap(1);
		}
	}

}


void orect()
{
	short linexs,lineys;

	linexs=mousex;
	lineys=mousey;

	while(cont()){
		if ( (mouse[0]!=mousex) || (mouse[1]!=mousey) ){
			lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
			mousex=mouse[0];
			mousey=mouse[1];
			drawline(linexs,lineys,mousex,lineys);
			drawline(mousex,lineys,mousex,mousey);
			drawline(mousex,mousey,linexs,mousey);
			drawline(linexs,mousey,linexs,lineys);
		}
	}

}


void vector()
{
	short linexs,lineys;

	drawbrushi(mousex,mousey);
	cont();
	linexs=mousex;
	lineys=mousey;
	mousex=mouse[0];
	mousey=mouse[1];

	while(cont()){
		if ( (mouse[0]!=mousex) || (mouse[1]!=mousey) ){
			lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
			mousex=mouse[0];
			mousey=mouse[1];
			drawline(linexs,lineys,mousex,mousey);
		}
	}
}


void initubuf()
{
	if (ubuf) freeImBuf(ubuf);
	ubuf=allocImBuf(bsl+bsr+1,bsb+bst+1,32,1,0);
	lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
}


void initvierkant(id)
Object id;
{
	float vec[2],size;

	bsl=bsb=bsr=bst=((brsize+1)>>1);

	size=brsize/2.0;
	makeobj(id);
	bgnpolygon();
	vec[0]= -size;
	vec[1]= -size;
	v2f(vec);

	vec[1]= size;
	v2f(vec);

	vec[0]= size;
	v2f(vec);

	vec[1]= -size;
	v2f(vec);
	endpolygon();
	closeobj();
}


void initvierkanto(id)
Object id;
{
	short vec[2];

	bsl=bsb=(brsize>>1);
	bsr=bst=((brsize+1)>>1);

	makeobj(id);
	bgnpolygon();
	vec[0]= -bsl;
	vec[1]= -bsb;
	v2s(vec);

	vec[1]= bst;
	v2s(vec);

	vec[0]= bsr;
	v2s(vec);

	vec[1]= -bsb;
	v2s(vec);
	endpolygon();
	closeobj();
}


void initcirkel(id)
Object id;
{
	float vec[2];
	float i,step,scale,add;
	short num;

	bsl=bsb=(brsize>>1);
	bsr=bst=((brsize+1)>>1);

	num=(brsize>>1);
	num+=9;		/* voor symetrie */
	num &= (~3);

	step = (2.0 * M_PI)/num;
	i=step/2;
	scale=brsize/2.0;

	scale += .01;

	if (brsize & 1) add=0;
	else add=.5;

	makeobj(id);
	bgnpolygon();
	for(;num>0;num--){
		vec[0]=scale*fcos(i)+add;
		vec[1]=scale*fsin(i)+add;
		v2f(vec);
		i+=step;
	}
	endpolygon();
	closeobj();
}


void drawcross(x,y)
short x,y;
{
	move2i(x,0);
	draw2i(x,sizey);
	move2i(0,y);
	draw2i(sizex,y);
}


void getmask(pos)
short pos;
{
	long i;
	ulong *b, col;

	b=bbuf->rect;
	if (pos){
		for(i = bbuf->x * bbuf->y;i>0;i--){
			col = *b & 0x00ffffff;
			if (col == backcol) *b++ = col;
			else *b++ = col | 0xff000000;
		}
	} else{
		for(i = bbuf->x * bbuf->y;i>0;i--){
			col = *b & 0x00ffffff;
			if (col != backcol) *b++ = col;
			else *b++ = col | 0xff000000;
		}
	}
}

void initpixbrush()
{
	if (mbuf) freeImBuf(mbuf);
	mbuf=allocImBuf(bbuf->x,bbuf->y,32,1,0);
	bsl = bbuf->x  >>1;
	bsr = bbuf->x -1-bsl;

	bsb = bbuf->y  >>1;
	bst = bbuf->y - 1 - bsb;
}


short getbrush()
{
	short minx,maxx,miny,maxy,val;
	long event, ok=TRUE;

	if (cont()==0) return;

	cursoff();
	fullscrn();
	drawmode(PUPDRAW);
	mapcolor(1,255,0,0);

	maxx = getgdesc(GD_XPMAX) -1;
	maxy = getgdesc(GD_YPMAX) -1;
	mousex = -32768;

	while(ok){
		if (getbutton(ESCKEY)) ok = FALSE;
		if (cont()==0){
			event = qutest();
			if (event != LEFTMOUSE && event != MIDDLEMOUSE) ok = FALSE;
		}
		if (mousex!=mouse[0]+ofsx || mousey!=mouse[1]+ofsy){
			color(0);
			recti(0, mousey, maxx, mousey);
			recti(mousex, 0, mousex, maxy);
			mousex=mouse[0]+ofsx;
			mousey=mouse[1]+ofsy;
			color(1);
			recti(0, mousey, maxx, mousey);
			recti(mousex, 0, mousex, maxy);
		}
		sginap(2);
		if (qutest()) event=quread(&val);
		if (getbutton(LEFTMOUSE) || getbutton(MIDDLEMOUSE)) break;
	}

	color(0);
	recti(0, mousey, maxx, mousey);
	recti(mousex, 0, mousex, maxy);
	event = 0;
	if (ok){
		minx=mousex ;
		miny=mousey;
		while (ok){
			if (getbutton(ESCKEY)) ok = FALSE;
			if (cont()==0){
				event = qutest();
				if (event != LEFTMOUSE && event != MIDDLEMOUSE) ok = FALSE;
			}
			if (mousex!=mouse[0]+ofsx || mousey!=mouse[1]+ofsy){
				color(0);
				recti(minx,miny,mousex,mousey);
				mousex=mouse[0]+ofsx;
				mousey=mouse[1]+ofsy;
				color(1);
				recti(minx,miny,mousex,mousey);
			}
			sginap(1);
			if (qutest()) event=quread(&val);
			if (getbutton(LEFTMOUSE)) event = LEFTMOUSE;
			if (getbutton(MIDDLEMOUSE)) event = MIDDLEMOUSE;
			if (getbutton(LEFTMOUSE)==0 && getbutton(MIDDLEMOUSE) == 0) break;
		}
		color(0);
		recti(minx,miny,mousex,mousey);
	}

	endfullscrn();
	initview();
	drawmode(NORMALDRAW);
	setcursor(0,0,0);
	curson();
	cpack(frontcol);

	if ((event==LEFTMOUSE) | (event==MIDDLEMOUSE)){
		maxx=mousex;
		maxy=mousey;
		if (maxx<minx){
			val=maxx;
			maxx=minx;
			minx=val;
		}
		if (maxy<miny){
			val=maxy;
			maxy=miny;
			miny=val;
		}

		minx -= ofsx;
		maxx -= ofsx;
		mousex -= ofsx;
		miny -= ofsy;
		maxy -= ofsy;
		mousey -= ofsy;

		if (minx < 0 && minx > -8) minx = 0;
		if (miny < 0 && miny > -8) miny = 0;
		if (maxx > ibuf->x && maxx < ibuf->x + 8) maxx = ibuf->x -1;
		if (maxy > ibuf->y && maxy < ibuf->y + 8) maxy = ibuf->y -1;

		if (bbuf) freeImBuf(bbuf);
		bbuf=allocImBuf(maxx-minx+1,maxy-miny+1,32,1,0);

		minx += ofsx;
		maxx += ofsx;
		miny += ofsy;
		maxy += ofsy;
		readdisplay(minx,miny,maxx,maxy,bbuf->rect, 0);

		initpixbrush();
		getmask(1);
		return (TRUE);
	}
	mousex -= ofsx;
	mousey -= ofsy;
	brtype = BRCI;
	return (FALSE);
}

short getbrush_oud()
{
	short minx,maxx,miny,maxy,val;
	long event;

	if (cont()==0) return;
	while(cont()){
		setcursor(1,0,0);
		mousex=mouse[0];
		mousey=mouse[1];
	}

	event=qutest();
	if ((event!=LEFTMOUSE) & (event!=MIDDLEMOUSE)){
		brtype = BRCI;
		return (FALSE);
	}
	event=quread(&val);
	minx=mousex ;
	miny=mousey;
	drawmode(OVERDRAW);
	mapcolor(1,255,0,0);
	fullscrn();
	translate(ofsx,ofsy,0);
	cursoff();

	while (cont()){
		if ((mouse[0] != mousex) | (mouse[1] != mousey)){
			color(0);
			recti(minx,miny,mousex,mousey);
			mousex=mouse[0];
			mousey=mouse[1];
			color(1);
			recti(minx,miny,mousex,mousey);
		}
	}
	color(0);
	recti(minx,miny,mousex,mousey);
	endfullscrn();
	initview();
	drawmode(NORMALDRAW);
	setcursor(0,0,0);
	curson();
	cpack(frontcol);

	event=qutest();
	if ((event==LEFTMOUSE) | (event==MIDDLEMOUSE)){
		maxx=mousex;
		maxy=mousey;
		if (maxx<minx){
			val=maxx;
			maxx=minx;
			minx=val;
		}
		if (maxy<miny){
			val=maxy;
			maxy=miny;
			miny=val;
		}

		if (minx < 0 && minx > -8) minx = 0;
		if (miny < 0 && miny > -8) miny = 0;
		if (maxx > ibuf->x && maxx < ibuf->x + 8) maxx = ibuf->x -1;
		if (maxy > ibuf->y && maxy < ibuf->y + 8) maxy = ibuf->y -1;

		if (bbuf) freeImBuf(bbuf);
		bbuf=allocImBuf(maxx-minx+1,maxy-miny+1,32,1,0);
		lrectread(minx,miny,maxx,maxy,bbuf->rect);
		initpixbrush();
		getmask(1);
		return (TRUE);
	}
	brtype = BRCI;
	return (FALSE);
}



void initbrush()
{
	if (ubuf) lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
	switch(brtype){
	case BRPI:
		if (getbrush()) break;
	case BRCI:
		initcirkel(1);
		break;
	case BRSQ:
		initvierkant(1);
		break;
	}
	initubuf();
}


void crosshair()
{
	curstype(CCROSS);
	defcursor(1,0);
}


void readcolor()
{
	short val;
	char rgb[4],string[50];
	long event;
	ulong backcoln,frontcoln;

	if (inmode == LIVE){
		backcoln=backcol;
		frontcoln=frontcol;
		lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
		screenspace();
		drawmode(OVERDRAW);
		mapcolor(1,0,0,0);
		mapcolor(3,255,255,255);
		color(1);
		rectfi(10+ofsx,10+ofsy,150+ofsx,30+ofsy);
		color(3);
		recti(10+ofsx,10+ofsy,150+ofsx,30+ofsy);
		recti(123+ofsx,13+ofsy,147+ofsx,27+ofsy);
		color(2);
		rectfi(125+ofsx,15+ofsy,145+ofsx,25+ofsy);

		val=0;
		do{
			setcursor(1,0,0);
			getdev(2,dev,mouse);

			readdisplay(mouse[0],mouse[1], mouse[0],mouse[1],(ulong *) &rgb, 0);
			mapcolor(2,rgb[3],rgb[2],rgb[1]);
			color(1);
			rectfi(11+ofsx,11+ofsy,122+ofsx,29+ofsy);
			cmov2i(20+ofsx,15+ofsy);
			sprintf(string,"%3d %3d %3d",rgb[3],rgb[2],rgb[1]);
			color(3);
			charstr(string);

			while(qutest() && val == 0){
				switch (qutest()){
				case LEFTMOUSE:
				case MIDDLEMOUSE:
					event = quread(&val);
					if (val == 0){
						if (event==LEFTMOUSE) frontcoln= *((ulong *)rgb);
						else backcoln= *((ulong *)rgb);
						val = 1;
					} else val = 0;
					break;
				case MOUSEX:
				case MOUSEY:
				case INPUTCHANGE:
					quread(&val);
					val=0;
					break;
				default:
					quread(&val);
					val=1;
				}
			}
			sginap(10);
		}while(val==0);

		color(0);
		clear();
		cpack(frontcol);
		drawmode(NORMALDRAW);
		setcursor(0,0,0);
		initview();

		if (backcoln != backcol){
			qenter(BCOL1,backcoln >> 16);
			qenter(BCOL2,backcoln & 0x0ffff);
		}
		if (frontcoln != frontcol){
			qenter(FCOL1,frontcoln >> 16);
			qenter(FCOL2,frontcoln & 0x0ffff);
		}
	}
}


main(argc,argv)
int argc;
char **argv;
{
	short go,val,mode = 'd',cross = FALSE,drbrush = TRUE;
	long paintwin,event;
	char pic[200];

	ubuf=bbuf=mbuf=0;
	input=output=0;

	strcpy(pic,"/pics/untitled");
	sizex=720;
	sizey=576;
	frontcol=0x0ffffff;
	backcol=0;

	if (argc==2){
		ibuf=loadiffname(argv[1],IB_test);
		if (ibuf){
			strcpy(pic,argv[1]);
			sizex=ibuf->x;
			sizey=ibuf->y;
		}
	}

	/*foreground();*/
	prefsize(sizex,sizey);
	paintwin=winopen(pic);
	wintitle(pic);
	RGBmode();
	gconfig();
	cpack(0);
	clear();

	if (ibuf){
		freeImBuf(ibuf);
		ibuf=loadiffname(argv[1],IB_rect);
	}

	if (ibuf==0){
		cpack(0);
		clear();
		ibuf=allocImBuf(sizex,sizey,24,1,0);
		if (ibuf==0){
			printf("Couldn't get undobuffer.\n");
			exit(0);
		}
		lrectread(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
	} else{
		lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
	}

	qdevice(DIAL0);
	qdevice(DIAL1);
	qdevice(DIAL2);
	qdevice(DIAL3);
	qdevice(DIAL4);
	qdevice(DIAL5);
	qdevice(DIAL6);
	qdevice(DIAL7);
	qdevice(DIAL8);

	qdevice(LPENY);
	qdevice(BPADY);
	qdevice(GHOSTY);
	qdevice(SBTY);
	qdevice(SBRY);
	
	qdevice(KEYBD);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(LEFTSHIFTKEY);
	qdevice(RIGHTSHIFTKEY);

	qdevice(LEFTARROWKEY);
	qdevice(RIGHTARROWKEY);

	qdevice(UPARROWKEY);
	qdevice(DOWNARROWKEY);
	qdevice(F2KEY);
	qdevice(F3KEY);
	qdevice(WINQUIT);
	qdevice(WINSHUT);  /* anders verdwijnt sluitsymbool */

	dev[0]=MOUSEX;
	dev[1]=MOUSEY;

	initview();
	go=TRUE;
	cpack(-1);

	brtype=BRCI;
	brsize=3;
	initbrush(1);
	crosshair();
	shademodel(FLAT);
	bmode='n';

	mouselist = (short *)malloc(maxmouse<<2);
	keylist = (long *)malloc(maxkey<<4);
	curmouse=0;
	curkey=0;
	readmouse=0;
	readkey=0;
	inmode == LIVE;
	winpop();
	zwritemask(-1);
	RGBwritemask(0xffff, 0xffff, 0xffff);
	stencil(FALSE, 0, 0, 0, 0, 0, 0);

	do{
		event=quread(&val);
		switch (event){
		case KEYBD:
			switch (val){
			case 27:
				if (val) go=FALSE;
				break;
			case 'd':
			case 's':
			case 'v':
				mode=val;
				drbrush = TRUE;
				cross = FALSE;
				break;
			case 'r':
				mode = val;
				drbrush = TRUE;
				cross = TRUE;
				break;
			case 'R':
				mode = val;
				drbrush = -1;
				cross = TRUE;
				break;
			case 'a':
			case 'n':
				bmode=val;
				break;
			case 6:
				bmode = 'f';
				break;
			case 'f':
				mode = val;
				drbrush = -1;
				cross = TRUE;
				break;
			case 'S':
				sharpenx(ibuf);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case 'F':
				filter(ibuf);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case 18:		/* ctrl r */
				if (curmouse) inmode = REPLAY;
				break;
			case 'b':
				brtype=BRPI;
				initbrush();
				cross = FALSE;
				drbrush = TRUE;
				mode = 's';
				break;
			case 'B':
				if (bbuf){
					brtype=BRPI;
					lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
					initpixbrush();
					initubuf();
					cross = FALSE;
					drbrush = TRUE;
					mode = 's';
				}
				break;
			case 'K':
				cpack(0);
				clear();
				cpack(frontcol);

				zdraw(1);frontbuffer(0);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				frontbuffer(1);zdraw(0);

				lrectread(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case 't':
				if (bbuf){
					extern shearx(struct ImBuf *ibuf, float dist);
					float angle = 15.0 * M_PI/180.0;
					short newx;

					shearx(bbuf, tanf(angle));
					flip270(bbuf);
					shearx(bbuf, 0.5 * tanf(angle));
					flip90(bbuf);
					cropibuf(bbuf);
					newx = (1.0 - (0.5 * tanf(angle))) * bbuf->x;
					scaleImBuf(bbuf, newx, bbuf->y);
					lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
					initpixbrush();
					initubuf();
				}
				break;
			case 'T':
				if (bbuf){
					extern rotibuf(struct ImBuf *ibuf, double angle);
					rotibuf(bbuf, 15.0);
					lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
					initpixbrush();
					initubuf();
				}
				break;
			case 20:
				if (bbuf){
					extern rotibuf(struct ImBuf *ibuf, double angle);
					scalefastImBuf(bbuf, bbuf->x << 1, bbuf->y << 1);
					rotibuf(bbuf, 15.0);
					scaleImBuf(bbuf, bbuf->x >> 1, bbuf->y >> 1);
					lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
					initpixbrush();
					initubuf();
				}
				break;
			case 'o':
				outline(ibuf, mousex, mousey, 0);
				break;
			case 'u':
				if (windepth(paintwin) != 1){
					winpop();
				}
				readsource(SRC_ZBUFFER);
				rectcopy(0,0,sizex-1,sizey-1,0,0);
				readsource(SRC_FRONT);

				zdraw(1);frontbuffer(0);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				frontbuffer(1);zdraw(0);

				lrectread(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case 'l':
				filterlim(ibuf, 3);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case 'x':
				if (bbuf) flipx(bbuf);
				break;
			case 'y':
				if (bbuf) flipy(bbuf);
				break;
			case 'z':
				if (bbuf){
					flip90(bbuf);
					lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
					initpixbrush();
					initubuf();
				}
				break;
			case 'X':
				if (bbuf){
					tbuf = double_x(bbuf);
					if (tbuf) {
						freeImBuf(bbuf);
						bbuf = tbuf;
						lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
						initpixbrush();
						initubuf();
					}
				}
				break;
			case 'Y':
				if (bbuf){
					tbuf = double_y(bbuf);
					if (tbuf) {
						freeImBuf(bbuf);
						bbuf = tbuf;
						lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
						initpixbrush();
						initubuf();
					}
				}
				break;
			case 'h':
				if (bbuf){
					tbuf = half_x(bbuf);
					if (tbuf){
						struct ImBuf * ttbuf;

						ttbuf = half_y(tbuf);
						freeImBuf(tbuf);
						tbuf = ttbuf;
					}
					if (tbuf) {
						freeImBuf(bbuf);
						bbuf = tbuf;
						lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
						initpixbrush();
						initubuf();
					}
				}
				break;
			case 'G':
				cspace(ibuf,rgbbeta);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case '=':
			case '+':
				brsize++;
				initbrush();
				break;
			case '-':
				if (brsize>0){
					brsize--;
					initbrush();
				}
				break;
			case '.':
				if (brtype==BRCI) brtype=BRSQ;
				else brtype=BRCI;
				initbrush();
				break;
			case ',':
				readcolor();
				break;
			case '1':		/*rood*/
				scroll(ibuf,3,-1);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case '2':
				scroll(ibuf,3,1);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case '3':		/*groen*/
				scroll(ibuf,2,-1);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			case '4':
				scroll(ibuf,2,1);
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				break;
			default:
				printf("%d\n",val);
			}
			break;
		case MOUSEX:
			mousex=val-ofsx;
			break;
		case MOUSEY:
			mousey=val-ofsy;
			break;
		case LEFTMOUSE:
			if (val) mousek |= 1;
			else mousek &= (~1);
			break;
		case MIDDLEMOUSE:
			if (val) mousek |= 4;
			else mousek &= (~4);
			break;
		case REDRAW:
			mousex += ofsx;
			mousey += ofsy;
			initview();
			mousex -= ofsx;
			mousey -= ofsy;
			lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
			lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
			break;
		case RIGHTCTRLKEY:
		case INPUTCHANGE:
			break;
		case BCOL1:
			backcol &= 0x0ffff;
			backcol |= val<<16;
			backcol &= 0xffffff;
			break;
		case BCOL2:
			backcol &= 0xffff0000;
			backcol |= (ushort)val;
			break;
		case FCOL1:
			frontcol &= 0x0ffff;
			frontcol |= val<<16;
			break;
		case FCOL2:
			frontcol &= 0xffff0000;
			frontcol |= (ushort)val;
			break;
		case WINQUIT:
			go=FALSE;
			break;
		case LEFTARROWKEY:
			if (val) setvaluator(MOUSEX,mousex+ofsx-1,0,getgdesc(GD_XPMAX) - 1);
			break;
		case RIGHTARROWKEY:
			if (val) setvaluator(MOUSEX,mousex+ofsx+1,0,getgdesc(GD_XPMAX) - 1);
			break;
		case UPARROWKEY:
			if (val) setvaluator(MOUSEY,mousey+ofsy+1,0,getgdesc(GD_YPMAX) - 1);
			break;
		case DOWNARROWKEY:
			if (val) setvaluator(MOUSEY,mousey+ofsy-1,0,getgdesc(GD_YPMAX) - 1);
			break;
		case F2KEY:
			if (val){
				struct ImBuf *tbuf;
				char *string;

				if (keys & SHIFT){
					tbuf = bbuf;
					string = "Save brush as FTYPE";
				} else{
					tbuf = ibuf;
					string = "Save as FTYPE";
				}
				if (tbuf){
					if (fileselect(string,pic)){
						wintitle(pic);
						tbuf = dupImBuf(tbuf);
						saveiff(tbuf,pic,SI_rect);
						freeImBuf(tbuf);
					}
				}
			}
			break;
		case F3KEY:
			if (val){
				struct ImBuf *tbuf;
				char *string;

				if (keys & SHIFT){
					tbuf = bbuf;
					string = "Save brush as 24bits";
				} else{
					tbuf = ibuf;
					string = "Save as 24bits";
				}
				if (tbuf){
					if (fileselect(string,pic)){
						tbuf = dupImBuf(tbuf);
						tbuf->ftype = AMI;
						freecmapImBuf(tbuf);
						tbuf->depth = 24;
						wintitle(pic);
						saveiff(tbuf,pic,SI_rect);
						freeImBuf(tbuf);
					}
				}
			}
			break;
		case LEFTSHIFTKEY:
		case RIGHTSHIFTKEY:
			break;
		default:
			printf("unknown event %d %d\n",event,val);
		}

		if (mousek | (inmode == REPLAY)){
			lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
			if (windepth(paintwin) != 1){
				winpop();
				lrectwrite(0,0,ibuf->x-1,ibuf->y-1,ibuf->rect);
			}

			if (inmode == REPLAY){
				if (mouselist[0]==1) cpack(frontcol);
				else cpack(backcol);
				readmouse=1;
			} else{
				if (mousek==1) cpack(frontcol);
				else cpack(backcol);
				curmouse=0;
			}

			switch (mode){
			case 's':
				sketch();
				break;
			case 'd':
				drawit();
				break;
			case 'v':
				vector();
				break;
			case 'r':
				orect();
				break;
			case 'R':
				frect();
				break;
			case 'f':
				fill();
				break;
			default:
				printf("unknown mode %d \n",mode);
			}

			getdev(2,dev,mouse);
			mouse[0] -= ofsx;
			mouse[1] -= ofsy;
			mousex = mouse[0];
			mousey = mouse[1];

			if (qutest()!=KEYBD){
				zdraw(1);
				frontbuffer(0);	/* dit moet, anders beide buffers! */
				lrectwrite(0,0,sizex-1,sizey-1,ibuf->rect);
				frontbuffer(1);
				zdraw(0);
				lrectread(0,0,sizex-1,sizey-1,ibuf->rect);
			}
			lrectread(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
			cpack(frontcol);
			if (inmode == REPLAY){
				inmode=LIVE;
			}
		} else {
			if (drbrush == TRUE){
				if ((event != INPUTCHANGE) || (val != 0)){
					if (qutest()==0 & go){
						sginap(1);
						if (qutest()==0){
							lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
							cpack(frontcol);
							mousexs=mousex;
							mouseys=mousey;
							lrectread(mousex-bsl,mousey-bsb,mousex+bsr,mousey+bst,ubuf->rect);
							drawbrushi(mousex,mousey);
						}
					}
				}
			} else if (drbrush == -1) {
				lrectwrite(mousexs-bsl,mouseys-bsb,mousexs+bsr,mouseys+bst,ubuf->rect);
				drbrush = FALSE;
			}
		}
		if (cross) setcursor(1,0,0);
		else setcursor(0,0,0);

	}while(go);

	drawmode(OVERDRAW);
	color(0);
	clear();
	drawmode(NORMALDRAW);

	/*gexit();*/
	if (output) close(output);
}

