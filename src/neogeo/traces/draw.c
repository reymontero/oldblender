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

/*   	draw.c         */


/*	initkubs()
	initsymbols()
	tekenrect3(s,rect)
	
	teken_ebaseverts()
	wire(ob)
	tekenpolyfast()
	tekenpoly()
	tekenpolyslow()
	tekensphere()
	gouraud(ob)

	tekenbase(base)
	tekenbase_ext(base)


 ****************** ************ **************** */

#include <stdlib.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "iff.h"
#include "/usr/people/include/Button.h"
#include "/usr/people/include/Trace.h"

extern ulong rectyellow[5][5],rectpurple[5][5];
extern void VecMulf(float *v1, float f);


void mydrawmode(mode)
long mode;
{
	if(G.machine==ENTRY) {
		if(mode==OVERDRAW) mode=PUPDRAW;	
	}
#undef drawmode
	drawmode(mode);
#define drawmode mydrawmode
}

float v[8][3] = {
	{-1.0, -1.0, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.0,  1.0, -1.0},
	{ 1.0, -1.0, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.0,  1.0, -1.0},
};

float v1[8][3] = {
	{-1.2, -1.2, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.2,  1.2, -1.0},
	{ 1.2, -1.2, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.2,  1.2, -1.0},
};



int do_drawaxes= 0;

extern ulong rect_desel[16],rect_sel[16];
extern ulong rectl_desel[81],rectl_sel[81];
extern ulong rectq_desel[81],rectq_sel[81], rectq_key[81];

#define LINE3F(v1, v2)	{bgnline(); v3f((float *)(v1)); v3f((float *)(v2)); endline();}

void initkubs()
{
	float vec[3];
	float n[3];

	makeobj(1);

	bgnline();	v3f(v[0]); 
	v3f(v[1]);	v3f(v[2]); 
	v3f(v[3]);	v3f(v[0]); 
	v3f(v[4]);	v3f(v[5]); 
	v3f(v[6]);	v3f(v[7]); 
	v3f(v[4]);	endline();
	bgnline();
	v3f(v[1]); 	v3f(v[5]);
	endline();	bgnline();
	v3f(v[2]); 	v3f(v[6]);
	endline();	bgnline();
	v3f(v[3]); 	v3f(v[7]);
	endline();	closeobj();

	makeobj(2);
	n[0]=0; 	n[1]=0; 	n[2]=0;
	bgnpolygon();
	n[0]= -1.0;
	n3f(n); 
	v3f(v[0]); 	n3f(n); 
	v3f(v[1]);	n3f(n); 
	v3f(v[2]); 	n3f(n); 
	v3f(v[3]);	n[0]=0;
	endpolygon();
	bgnpolygon();
	n[1]= -1.0;	n3f(n); 
	v3f(v[0]); 	n3f(n); 
	v3f(v[4]);	n3f(n); 
	v3f(v[5]); 	n3f(n); 
	v3f(v[1]);	n[1]=0;
	endpolygon();
	bgnpolygon();
	n[0]= 1.0;	n3f(n); 
	v3f(v[4]);	n3f(n); 
	v3f(v[7]);	n3f(n); 
	v3f(v[6]);	n3f(n); 
	v3f(v[5]);	n[0]=0;
	endpolygon();
	bgnpolygon();
	n[1]= 1.0;	n3f(n); 
	v3f(v[7]);	n3f(n); 
	v3f(v[3]);	n3f(n); 
	v3f(v[2]);	n3f(n); 
	v3f(v[6]);	n[1]=0;
	endpolygon();
	bgnpolygon();
	n[2]= 1.0;	n3f(n); 
	v3f(v[1]);	n3f(n); 
	v3f(v[5]);	n3f(n); 
	v3f(v[6]);	n3f(n); 
	v3f(v[2]);	n[2]=0;
	endpolygon();
	bgnpolygon();
	n[2]= -1.0;	n3f(n); 
	v3f(v[7]);	n3f(n); 
	v3f(v[4]);	n3f(n); 
	v3f(v[0]);	n3f(n); 
	v3f(v[3]);
	endpolygon();
	closeobj();
	makeobj(3);
	circs(0,0,1);
	rotate(900,'z');
	rotate(900,'x');
	circs(0,0,1);
	rotate(900,'y');
	rotate(900,'z');
	circs(0,0,1);
	closeobj();
}

void drawaxes()
{
	float v1[3], v2[3];

	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	
	/* X-AS: pijl */
	v2[0]= 1.0;
	LINE3F(v1, v2);
	v1[0]= 0.8;
	v1[1]= -0.125;
	LINE3F(v1, v2);
	v1[1]= 0.125;
	LINE3F(v1, v2);
	
	/* Y-AS: pijl */
	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	v2[1]= 1.0;
	LINE3F(v1, v2);
	v1[1]= 0.8;
	v1[0]= -0.125;
	LINE3F(v1, v2);
	v1[0]= 0.125;
	LINE3F(v1, v2);
	
	/* Z-AS: pijl */
	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	v2[2]= 1.0;
	LINE3F(v1, v2);
	v1[2]= 0.8;
	v1[0]= -0.125;
	LINE3F(v1, v2);
	v1[0]= 0.125;
	LINE3F(v1, v2);

	linewidth(4);

	/* X-AS: tekst */
	v1[2]=v2[2]= 0.0;
	v1[0]= 1.125;	v1[1]= 0.125;
	v2[0]= 1.375;	v2[1]= -0.125;
	LINE3F(v1, v2);
	v1[0]= 1.125;	v1[1]= -0.125;
	v2[0]= 1.375;	v2[1]= 0.125;
	LINE3F(v1, v2);

	/* Y-AS: tekst */
	v1[1]= 1.125;	v1[0]= -0.1;
	v2[1]= 1.375;	v2[0]= 0.1;
	LINE3F(v1, v2);
	v1[1]= 1.375;	v1[0]= -0.1;
	v2[1]= 1.25;	v2[0]= 0.0;
	LINE3F(v1, v2);

	/* Z-AS: tekst */
	v1[1]=v2[1]= 0.0;
	v1[0]= 0.125;	v1[2]= 1.125;
	v2[0]= -0.125;	v2[2]= 1.125;
	LINE3F(v1, v2);
	v1[0]= 0.125;	v1[2]= 1.375;
	LINE3F(v1, v2);
	v2[0]= -0.125;	v2[2]= 1.375;
	LINE3F(v1, v2);

	if(G.machine==ENTRY) linewidth(2);
	else linewidth(1);
}

void drawgourcube()		/* voor ika */
{
	float vec[3];
	float n[3];

	n[0]=0; n[1]=0; n[2]=0;
	bgnpolygon();
		n[0]= -1.0;
		n3f(n); 
		v3f(v1[0]); v3f(v1[1]); v3f(v1[2]); v3f(v1[3]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= -1.0;
		n3f(n); 
		v3f(v1[0]); v3f(v1[4]); v3f(v1[5]); v3f(v1[1]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[0]= 1.0;
		n3f(n); 
		v3f(v1[4]); v3f(v1[7]); v3f(v1[6]); v3f(v1[5]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= 1.0;
		n3f(n); 
		v3f(v1[7]); v3f(v1[3]); v3f(v1[2]); v3f(v1[6]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[2]= 1.0;
		n3f(n); 
		v3f(v1[1]); v3f(v1[5]); v3f(v1[6]); v3f(v1[2]);
		n[2]=0;
	endpolygon();

	bgnpolygon();
		n[2]= -1.0;
		n3f(n); 
		v3f(v1[7]); v3f(v1[4]); v3f(v1[0]); v3f(v1[3]);
	endpolygon();
}

void drawcube()
{

	bgnline();
		v3f(v1[0]); v3f(v1[1]);v3f(v1[2]); v3f(v1[3]);
		v3f(v1[0]); v3f(v1[4]);v3f(v1[5]); v3f(v1[6]);
		v3f(v1[7]); v3f(v1[4]);
	endline();
	bgnline();
		v3f(v1[1]); v3f(v1[5]);
	endline();
	bgnline();
		v3f(v1[2]); v3f(v1[6]);
	endline();
	bgnline();
		v3f(v1[3]); v3f(v1[7]);
	endline();
}


void initsymbols()
{
	int a;
	ulong *r1,*r2;
	short vec[2];

	r1=rect_desel; 
	r2=rect_sel;
	r1[0]= r2[0]= r1[3]= r2[3]= r1[15]= r2[15]=0x707070;
	r1[1]= r2[1]= r1[2]= r2[2]= r1[7]= r2[7]= r1[11]= r2[11]=0;
	r1[9]= r2[9]=0xFFFFFF;
	r1[4]= r1[6]= r1[14]= 0x407070; /* geel 40% */
	r2[4]= r2[6]= r2[14]= 0x702070; /* paars 40% */
	r1[5]= r1[10]= r1[12]= 0x70CCCC;  /* geel 70% */
	r2[5]= r2[10]= r2[12]= 0xCC50CC;  /* paars 70% */
	r1[8]= r1[13]= 0xAAFFFF;  /* geel 100% */
	r2[8]= r2[13]= 0xFF80FF;  /* paars 100% */

	cpack(0);
	sboxfs(100,100,110,110);
	cpack(0xAAFFFF);
	circ(104,104,4);
	lrectread(100,100,108,108,rectl_desel);
	cpack(0xFF80FF);
	circ(104,104,4);
	lrectread(100,100,108,108,rectl_sel);

	cpack(0);
	sboxfs(100,100,110,110);

	cpack(0x101010);
	circ(104,104,4);
	cpack(0xAAFFFF);
	vec[0]=104; 
	vec[1]=100;
	bgnline(); 
	v2s(vec); 
	vec[1]=108; 
	v2s(vec); 
	endline();
	vec[0]=100; 
	vec[1]=104;
	bgnline(); 
	v2s(vec); 
	vec[0]=108; 
	v2s(vec); 
	endline();
	lrectread(100,100,108,108,rectq_desel);

	cpack(0xFF80FF);
	vec[0]=104; 
	vec[1]=100;
	bgnline(); 
	v2s(vec); 
	vec[1]=108; 
	v2s(vec); 
	endline();
	vec[0]=100; 
	vec[1]=104;
	bgnline(); 
	v2s(vec); 
	vec[0]=108; 
	v2s(vec); 
	endline();
	lrectread(100,100,108,108,rectq_sel);

	cpack(0xFF8800);
	vec[0]=104; 
	vec[1]=100;
	bgnline(); 
	v2s(vec); 
	vec[1]=108; 
	v2s(vec); 
	endline();
	vec[0]=100; 
	vec[1]=104;
	bgnline(); 
	v2s(vec); 
	vec[0]=108; 
	v2s(vec); 
	endline();
	lrectread(100,100,108,108,rectq_key);

}

void tekenlampcirc(short sx, short sy, short sel)
{

	/* persp(0); */
	if(sel) cpack(0xFF80FF); else cpack(0xAAFFFF);
	
	circs(sx,sy,4);
	/* persp(1); */
}

void tekenrect9(s,rect)
short *s;
ulong *rect;
{
	ulong rectt[81],*r;
	short a;

	if(s[0]<S.x && s[0]>0 && s[1]<S.vyw && s[1]>0) {
		lrectread(s[0]-4,s[1]-4,s[0]+4,s[1]+4,rectt);
		r=rectt;
		for(a=0;a<81;a++,rect++,r++) {
			if(*rect!=0) *r= *rect;
		}
		lrectwrite(s[0]-4,s[1]-4,s[0]+4,s[1]+4,rectt);
	}
}

void tekenrect4(s,rect)
short *s;
ulong *rect;
{

	if(s[0]<S.x && s[0]>0 && s[1]<S.vyw && s[1]>0) 
		lrectwrite(s[0]-1,s[1]-1,s[0]+2,s[1]+2,rect);
}

void tekenrect3(s,rect)
short *s;
ulong *rect;
{

	if(s[0]!=3200)
		lrectwrite(s[0]-1,s[1]-1,s[0]+1,s[1]+1,rect);

}

/* ****************** BACKGROUND IMAGE **************** */

short bgpicmode=0;

struct BGpic bgpic= {
	0, 0.0 ,1.0, 0, 0, 25, 100, "//back"
};

void freeBGpic()
{
	if(bgpic.ibuf) freeImBuf(bgpic.ibuf);
}

void drawBGpic()
{
	/* zet variabelen goed en schrijft naar Zbuf en kopieert naar scherm */
	float fac, tzoom, zfac;
	ulong *rect;
	long sizex, sizey, ofsx, ofsy;
	long x1, x2, y1, y2, sx1, sx2, sy1, sy2, vec[3];
	short ofs[2];
	
	if(bgpic.ibuf==0) return;

	winset(G.winar[0]);

	if(bgpic.dproj!= view0.dproj) {
		bgpic.dproj= view0.dproj;
		/* bgpicsize= 1 betekent 1 pixel= 1 wereldcoordinaat */
		/* dproj= 1.0 betekent 10000= 333.3 pixels */
		/* dproj > 1.0 betekent uitzomen */
		bgpic.zoom= (bgpic.size*333.3)/(view0.dproj*10000.0);	/* afm 1 pixel */
		if(bgpic.zoom>1.0) {
			tzoom= bgpic.zoom/fceil(bgpic.zoom);
			bgpic.zoom= fceil(bgpic.zoom);
			bgpic.xim= tzoom*bgpic.ibuf->x;
			bgpic.yim= tzoom*bgpic.ibuf->y;
			/* ipv verkleinen, kan het ook met vergroten? */
			if(bgpic.yim<S.vyw/2 && bgpic.xim<640) {
				bgpic.zoom= (bgpic.size*333.3)/(view0.dproj*10000.0);
				tzoom= bgpic.zoom/ffloor(bgpic.zoom);
				bgpic.zoom= ffloor(bgpic.zoom);
				bgpic.xim= tzoom*bgpic.ibuf->x;
				bgpic.yim= tzoom*bgpic.ibuf->y;
			}
			rect= (ulong *)mallocN(bgpic.xim*bgpic.yim*4, "drawBGpic");
			scalefastrect(bgpic.ibuf->rect, rect, bgpic.ibuf->x, bgpic.ibuf->y, bgpic.xim, bgpic.yim);
		}
		else {
			bgpic.xim= bgpic.zoom*bgpic.ibuf->x;
			bgpic.yim= bgpic.zoom*bgpic.ibuf->y;
			rect= (ulong *)mallocN(bgpic.xim*bgpic.yim*4, "drawBGpic");
			scalefastrect(bgpic.ibuf->rect, rect, bgpic.ibuf->x, bgpic.ibuf->y, bgpic.xim, bgpic.yim);
			bgpic.zoom= 1.0;
		}

		if(G.machine!=ELAN) {
			struct ImBuf ibuf;

			memcpy(&ibuf, bgpic.ibuf, sizeof(struct ImBuf));
			ibuf.rect= rect;
			ibuf.x= bgpic.xim;
			ibuf.y= bgpic.yim;
			dit2(&ibuf, 1, 4);
			dit2(&ibuf, 2, 4);
			dit2(&ibuf, 3, 4);
		}

		zdraw(1);
		frontbuffer(0);
		backbuffer(0);
		lrectwrite(0, 0, bgpic.xim-1, bgpic.yim-1, rect);
		frontbuffer(0);
		backbuffer(1);
		zdraw(0);
		freeN(rect);

	}

	/* de offset berekenen */
	sizex= bgpic.zoom*bgpic.xim;
	sizey= bgpic.zoom*bgpic.yim;

	if(view0.persp==2) {
		ofsx= S.vxw/2;
		ofsy= S.vyw/2;
	}
	else {
		vec[0]=vec[1]=vec[2]= 0;
		berekenschermco_noclip(vec, ofs);
		ofsx= ofs[0];
		ofsy= ofs[1];
	}
	
	x1=y1= 0;
	x2= bgpic.xim-1;
	y2= bgpic.yim-1;
	sx1= -sizex/2+ofsx;
	sx2= sizex/2+ofsx;
	sy1= -sizey/2+ofsy;
	sy2= sizey/2+ofsy;
	if(sx1>=S.vxw || sx2<=0 || sy1>=S.vyw || sy2<=0) return;
	if(sx1<0) {
		fac= ((float)-sx1)/(float)sizex;
		sx1= 0;
		x1+= fac*bgpic.xim;
	}
	if(sy1<0) {
		fac= ((float)-sy1)/(float)sizey;
		sy1= 0;
		y1+= fac*bgpic.yim;
	}
	if(sx2>S.vxw) {
		fac= ((float)sx2-S.x)/(float)sizex;
		x2-= fac*bgpic.xim;
	}
	if(sy2>S.vyw) {
		fac= ((float)sy2-S.vyw)/(float)sizey;
		y2-= fac*bgpic.yim;
	}

	rectzoom(bgpic.zoom, bgpic.zoom);

	readsource(SRC_ZBUFFER);
	rectcopy(x1, y1, x2, y2, sx1, sy1);
	finish();
	readsource(SRC_AUTO);

	rectzoom(1.0, 1.0);

	lrectread(100, 100, 100, 100, (ulong *)&x1);/* BELACHELIJK! dit moet omdat anders de
						  rectwrite het niet meer doet */
}


void loadorfreeBGpic()
{
	/* wordt vanuit viewbuts() aangeroepen na indrukken button of verlaten tekstbut */
	float fac;
	ulong blend1, blend2, tel;
	char str[100], *rt;

	if(bgpic.ibuf) freeImBuf(bgpic.ibuf);
	bgpic.ibuf= 0;

	if(bgpicmode & 1) {
		if(G.zbuf==0) {
			strcpy(str,bgpic.name);
			convertstringcode(str);
			bgpic.ibuf= loadiffname(str,LI_rect);
		}
		if(bgpic.ibuf) {
			bgpic.dproj= 0;
			if(bgpic.ibuf->x>S.x) {
				fac= S.x/(float)bgpic.ibuf->x;
				scalefastImBuf(bgpic.ibuf, S.x, (short)(fac*bgpic.ibuf->y));
			}
			if(bgpic.ibuf->y>S.vyw) {
				fac= ((float)S.vyw)/(float)bgpic.ibuf->y;
				scalefastImBuf(bgpic.ibuf, (short)(fac*bgpic.ibuf->x), S.vyw);
			}
			if(bgpic.blend) {
				rt= (char *)bgpic.ibuf->rect;
				tel= bgpic.ibuf->x*bgpic.ibuf->y;
				blend1= 0x73*bgpic.blend;
				blend2= 256-bgpic.blend;
				while(tel>0) {
					rt[1]= (blend1+ blend2*rt[1])>>8;
					rt[2]= (blend1+ blend2*rt[2])>>8;
					rt[3]= (blend1+ blend2*rt[3])>>8;
					rt+=4;
					tel--;
				}
			}
			/* kleur veranderen */

			projektie();
		}
		else {
			if(G.zbuf) error("No BackGround AND Zbuffer!");
			else error("Can't load BackGround");

			bgpicmode-= 1;
			SetButs(151, 151);
		}
	}
	else projektie();
}


/* ****************** ************ **************** */

void tekenverticeso(sel)
short sel;
{
	ulong *rect;
	struct EditVert *eve;
	short xs,ys;

	if(sel) rect= (ulong *)rectyellow;
	else rect= (ulong *)rectpurple;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if( (eve->f & 1)==sel ) {
			if(eve->xs!=3200) {
				xs= eve->xs;
				ys= eve->ys;
				lrectwrite(xs-1,ys-1,xs+1,ys,rect);
			}
		}
		eve= eve->next;
	}
}

void tekenvertices(sel)
short sel;
{
	extern short editbutflag;
	struct EditVert *eve;

	if((editbutflag & 32)==0) {
		tekenverticeso(sel);
		return;
	}

	if(sel) cpack(0x77FFFF);
	else cpack(0xFF70FF);
	
	bgnpoint();
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if( (eve->f & 1)==sel ) {
			if(eve->h==0) v3f(eve->co);
		}
		eve= eve->next;
	}
	
	endpoint();
}

void tekenvertices_ext()
{
	float omat[3][3], mat[3][3];
	struct ObData *ob;
	
	if(G.zbuf) zbuffer(0);

	if(editbutflag & 32) {
		parentbase(G.ebase, mat, omat);
	
		ob= (struct ObData *)G.ebase->d;
			
		Mat4MulFloat3(mat, ob->vv->ws);
		multmatrix(mat);
	}

	frontbuffer(1);
	backbuffer(0);
	tekenvertices(0);
	tekenvertices(1);
	frontbuffer(0);
	backbuffer(1);

	if(G.zbuf) zbuffer(1);
	
	if(editbutflag & 32) loadmatrix(G.viewmat);
}

void getsingmatrix(mat)
float *mat;
{
	float pmat[4][4], wmat[4][4];

	getmatrix(pmat);
	mmode(MPROJECTION);
	getmatrix(wmat); 
	wmat[2][2]= -1;
	Mat4MulMat4(mat,pmat,wmat);
	mmode(MVIEWING);
}

void calc_ebaseverts()
{
	struct EditVert *eve;
	float mat[4][4];
	long a;

	if(G.edve.first==0) return;
	eve= (struct EditVert *)G.edve.first;

	getsingmatrix(mat);
	Mat4SwapMat4(G.persmat, mat);

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->h==0) {
			berekenschermcofl(eve->co, &(eve->xs));
		}
		eve= eve->next;
	}

	Mat4SwapMat4(G.persmat, mat);
}

void calc_Nurbverts(nurb)
struct Nurb *nurb;
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float mat[4][4];
	long a, tot, b;
	short *sp;

	winset(G.winar[0]);
	getsingmatrix(mat);
	Mat4SwapMat4(G.persmat, mat);

	nu= nurb;
	while(nu) {
		if((nu->type & 7)==1) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				berekenschermcofl(bezt->vec[0], bezt->s[0]);
				berekenschermcofl(bezt->vec[1], bezt->s[1]);
				berekenschermcofl(bezt->vec[2], bezt->s[2]);
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				berekenschermcofl(bp->vec, bp->s);
				bp++;
			}
		}
		nu= nu->next;
	}

	Mat4SwapMat4(G.persmat, mat);
}


void drawDispList(dlbase)
struct ListBase *dlbase;
{
	struct DispList *dl;
	struct dl_GourFace *dlgf;
	long parts, nr, ofs, p1, p2, p3, p4, a, b;
	float *data, *v1, *v2, *v3, *v4, side;
	short col[3];
	char *cp;

	if(dlbase==0) return;

	dl= dlbase->first;
	while(dl) {
		data= (float *)(dl+1);

		switch(dl->type) {
		case DL_SEGM:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				bgnline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				endline();
			}
			break;
		case DL_POLY:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				bgnclosedline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				endclosedline();
			}
			break;
		case DL_SURF:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				if(dl->flag & 1) bgnclosedline();
				else bgnline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				if(dl->flag & 1) endclosedline();
				else endline();
			}
			ofs= 3*dl->nr;
			nr= dl->nr;
			while(nr--) {
				data= ( (float *)(dl+1) )+3*nr;
				parts= dl->parts;
				if(dl->flag & 2) bgnclosedline();
				else bgnline();
				while(parts--) {
					v3f(data);
					data+=ofs;
				}
				if(dl->flag & 2) endclosedline();
				else endline();
			}

			break;
		case DL_TRIA_GOUR:
			if(G.f & 16) {  /* backbufproj */
				dlgf= (struct dl_GourFace *)data;
				for(a=0; a<dl->parts; a++) {
					bgnpolygon();
					v3f(dlgf->co[0]);
					v3f(dlgf->co[1]);
					v3f(dlgf->co[2]);
					endpolygon();
					dlgf++;
				}

			}
			else {
				shademodel(GOURAUD);
				if( (G.f & 256)==0 ) backface(TRUE);
				dlgf= (struct dl_GourFace *)data;
				for(a=0; a<dl->parts; a++) {
					bgnpolygon();
					cpack(dlgf->col[0]);
					v3f(dlgf->co[0]);
					cpack(dlgf->col[2]);
					v3f(dlgf->co[1]);
					cpack(dlgf->col[4]);
					v3f(dlgf->co[2]);
					endpolygon();
					if( (G.f & 256)==0 ) {
						bgnpolygon();
						cpack(dlgf->col[5]);
						v3f(dlgf->co[2]);
						cpack(dlgf->col[3]);
						v3f(dlgf->co[1]);
						cpack(dlgf->col[1]);
						v3f(dlgf->co[0]);
						endpolygon();
					}
					dlgf++;
				}
				shademodel(FLAT);
				if( (G.f & 256)==0 ) backface(FALSE);
			}
			break;

		case DL_SURF_GOUR: 
		case DL_SURF_SOLID:
			if(G.f & 16) {  /* backbufproj */
				for(a=0; a<dl->parts; a++) {

					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);

					v1= data+ 5*p1; 
					v2= data+ 5*p2;
					v3= data+ 5*p3; 
					v4= data+ 5*p4;

					for(; b<dl->nr; b++) {
						bgnpolygon();
						v3f(v1);
						v3f(v2);
						v3f(v4);
						v3f(v3);
						endpolygon();
						v2= v1; 
						v1+= 5;
						v4= v3; 
						v3+= 5;
					}
				}
			}
			else if(dl->type==DL_SURF_SOLID) {
				shademodel(GOURAUD);
				if( (G.f & 256)==0 ) backface(TRUE);
				for(a=0; a<dl->parts; a++) {

					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);

					v1= data+ 5*p1; 
					v2= data+ 5*p2;
					v3= data+ 5*p3; 
					v4= data+ 5*p4;

					for(; b<dl->nr; b++) {
						cp= (char *)(v1+3);
						bgnpolygon();
						cpack( *( (ulong *)v1+4) );
						v3f(v1);
						v3f(v2);
						v3f(v4);
						v3f(v3);
						endpolygon();
						if( (G.f & 256)==0 ) {
							bgnpolygon();
							cpack( *( (ulong *)v1+3) );
							v3f(v3);
							v3f(v4);
							v3f(v2);
							v3f(v1);
							endpolygon();
						}
						v2= v1; 
						v1+= 5;
						v4= v3; 
						v3+= 5;
					}
				}
				shademodel(FLAT);
				if( (G.f & 256)==0 ) backface(FALSE);
			}
			else {
				shademodel(GOURAUD);
				backface(TRUE);
				for(a=0; a<dl->parts; a++) {

					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);

					v1= data+ 5*p1; 
					v2= data+ 5*p2;
					v3= data+ 5*p3; 
					v4= data+ 5*p4;

					for(; b<dl->nr; b++) {
						bgnpolygon();
						cpack( *( (long *)v1+4) );
						v3f(v1);
						cpack( *( (long *)v2+4) );
						v3f(v2);
						cpack( *( (long *)v4+4) );
						v3f(v4);
						cpack( *( (long *)v3+4) );
						v3f(v3);
						endpolygon();
						bgnpolygon();
						cpack( *( (long *)v3+3) );
						v3f(v3);
						cpack( *( (long *)v4+3) );
						v3f(v4);
						cpack( *( (long *)v2+3) );
						v3f(v2);
						cpack( *( (long *)v1+3) );
						v3f(v1);
						endpolygon();
						v2= v1; 
						v1+=5;
						v4= v3; 
						v3+=5;
					}
				}
				shademodel(FLAT);
				backface(FALSE);
			}
			break;
		}
		dl= dl->next;
	}

}

void drawhaloverts(base, kleur)
struct Base *base;
ulong kleur;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve,*a1,*a2,*a3;
	float fvec[3], *fp;
	int a, vec[3], totvert ;

	ob= (struct ObData *)base->d;
	vv= ob->vv;
	adrve= (struct VertOb *)(vv+1);

	cpack(kleur);

	linewidth(2);

	totvert= vv->vert;
	if(ob->ef==4) totvert= build_schiphol(base, vv->vert);

	/* bgnpoint(); */
	if(ob->floatverts) {
		fp= ob->floatverts;
		for(a=0; a<totvert; a++) {
			bgnline();
				v3f(fp); v3f(fp);
			endline();
			fp+=3;
		}
	}
	else {
		for(a=0; a<totvert; a++) {
			bgnline();
				v3s(adrve->c); v3s(adrve->c);
			endline();
			adrve++;
		}
	}
	/* endpoint(); */
	
	if(G.machine==ENTRY) linewidth(2);
	else linewidth(1);
}

void wire(base,kleur)
struct Base *base;
ulong kleur;
{
	struct ObData *ob;
	struct VV *vv;
	struct VlakOb *adrvl;
	struct VertOb *adrve;
	struct EditEdge *eed;
	struct EditVlak *evl;
	float fvec[3], *f1, *f2, *f3;
	long a, vec[3], vv_vlak;
	short *a1, *a2, *a3;
	char test,colbcol=0;


	ob= (struct ObData *)base->d;
	vv= ob->vv;
	adrve=(struct VertOb *)(vv+1);
	adrvl=(struct VlakOb *)(adrve+vv->vert);

	if(G.ebase==base) kleur=0;
	if(G.basact==base && G.mainb==5 && kleur!=base->selcol && ob->c>1) colbcol=1;
	cpack(kleur);

	if(base==G.ebase) {
		/* tekenen met G.eded */
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			if(eed->h==0) {
				bgnline();
				v3f(eed->v1->co);
				v3f(eed->v2->co);
				endline();
			}
			eed= eed->next;
		}
		calc_ebaseverts();

		if(G.zbuf) zbuffer(0);
		tekenvertices(0);
		tekenvertices(1);
		if(G.zbuf) zbuffer(1);

		if(G.f & 32) {	/* normals */
			cpack(0xDDDD22);
			evl= G.edvl.first;
			while(evl) {
				if(evl->v1->h==0 && evl->v2->h==0 && evl->v3->h==0) {
					CalcCent3f(fvec, evl->v1->co, evl->v2->co, evl->v3->co);
					bgnline();
					v3f(fvec);
					fvec[0]+= view0.grid*evl->n[0];
					fvec[1]+= view0.grid*evl->n[1];
					fvec[2]+= view0.grid*evl->n[2];
					v3f(fvec);
					endline();
				}
				evl= evl->next;
			}
		}
	}
	else {
		
		if(ob->ef==4) vv_vlak= build_schiphol(base, vv->vlak);
		else vv_vlak= vv->vlak;
		
		if(G.hie && ob->floatverts) {
			for(a=0;a<vv_vlak;a++) {
				test= adrvl->ec & 224;
				if(test!=224) {
					f1= (ob->floatverts+3*adrvl->v1);
					f2= (ob->floatverts+3*adrvl->v2);
					f3= (ob->floatverts+3*adrvl->v3);
	
					if(colbcol) {
						if( G.colact-1 == (adrvl->ec & 15) ) cpack(0x00FFFF);
						else cpack(kleur);
					}
	
					if (test) {
						bgnline();
						if (test & 128) {
							if (test & 64){
								v3f(f1);v3f(f3);			/* 001 */
							} else {
								v3f(f2); v3f(f3);			/* 010 */
								if ((test & 32)==0) v3f(f1);/* 011 */
							}
						} else {
							if (test & 64) {
								v3f(f2); v3f(f1);			/* 100 */
								if ((test & 32)==0) v3f(f3);/* 101 */
							} else {
								v3f(f1); v3f(f2); v3f(f3);	/* 110 */
							}
						}
						endline();
					} else {
						bgnclosedline();
						v3f(f1); v3f(f2); v3f(f3);			/* 111 */
						endclosedline();
					}
	
				}
				adrvl++;
			}
		}
		else {
			for(a=0;a<vv_vlak;a++) {
				test= adrvl->ec & 224;
				if(test!=224) {
					a1= (adrve+adrvl->v1)->c;
					a2= (adrve+adrvl->v2)->c;
					a3= (adrve+adrvl->v3)->c;
	
					if(colbcol) {
						if( G.colact-1 == (adrvl->ec & 15) ) cpack(0x00FFFF);
						else cpack(kleur);
					}
	
					if (test) {
						bgnline();
						if (test & 128) {
							if (test & 64){
								v3s(a1);v3s(a3);			/* 001 */
							} else {
								v3s(a2); v3s(a3);			/* 010 */
								if ((test & 32)==0) v3s(a1);/* 011 */
							}
						} else {
							if (test & 64) {
								v3s(a2); v3s(a1);			/* 100 */
								if ((test & 32)==0) v3s(a3);/* 101 */
							} else {
								v3s(a1); v3s(a2); v3s(a3);	/* 110 */
							}
						}
						endline();
					} else {
						bgnclosedline();
						v3s(a1); v3s(a2); v3s(a3);			/* 111 */
						endclosedline();
					}
	
				}
				adrvl++;
			}
		}
	}

	/*  NORMALEN !!
	adrve=(struct VertOb *)(vv+1);
	cpack(0x00FFFF);
	for(a=0;a<vv->vert;a++) {
		VECCOPY(vec,adrve->c);
		bgnline();
			v3i(vec);
			vec[0]+=adrve->n[0]/10;
			vec[1]+=adrve->n[1]/10;
			vec[2]+=adrve->n[2]/10;
			v3i(vec);
		endline();
		adrve++;
	}
	 */
}

void tekenpolyfast(po)
struct PolyData *po;
{
	short a,b,*p,vert;

	if(po==0 || po->vert==0) return;
	scale(po->ws,po->ws,po->ws);

	p=(short *)(po+1);
	for(a=0;a<po->poly;a++) {
		vert= *p;
		p++;
		bgnclosedline();
		for(b=0;b<vert;b++) {
			v2s(p);
			p+=3;
		}
		endclosedline();
	}
}

void tekenpolyzijpart(vert,vec1,vec2)
long vert;
long *vec1,*vec2;
{
	long b;

	for(b=0;b<vert;b++) {
		bgnline();
		v3i(vec1); 
		v3i(vec2);
		endline();
		vec1+=3; 
		vec2+=3;
	}
}

void tekenpoly(po)
struct PolyData *po;
{
	long *vec,*vec1,*vec2,*vec3,*vec4,vert;
	short *f45,d,ext=1,a,b,c,*p,tel=0;
	float fac;

	if(po==0 || po->vert==0) return;

	if(po->ext) ext++;
	if(po->f45m!=0 || po->f45p!=0) ext*=2;

	if(ext==1) {
		tekenpolyfast(po);
		return;
	}
	scale(po->ws,po->ws,po->ws);

	/* voorberekenen alle punten */
	vec1=vec=(long *)mallocN(ext*3*4*po->vert,"tekenpoly");
	for(c=0;c<ext;c++) {
		if( (po->f45m!=0 || po->f45p!=0) && (po->f45!=0) ) {

			if(c<2) d= po->depth*po->ext;
			else d= -po->depth*po->ext;
			if(c==0) {
				fac= -po->depth*po->f45m;
				d+= po->depth*po->f45m;
			} else if(c==1) {
				fac= po->depth*po->f45p;
				d-= po->depth*po->f45p;
			} else if (c==2) {
				fac= po->depth*po->f45p;
				d+= po->depth*po->f45p;
			} else {
				fac= -po->depth*po->f45m;
				d-= po->depth*po->f45m;
			}
			fac/= 1000.0;

			p=(short *)(po+1);
			f45=po->f45;
			for(a=0;a<po->poly;a++) {
				vert= *p;
				p++;
				for(b=0;b<vert;b++) {
					vec1[0]= p[0]+fac*f45[0];
					vec1[1]= p[1]+fac*f45[1];
					vec1[2]= d;
					p+=3;
					f45+=3;
					vec1+=3;
				}
			}
		}
		else {
			if(c==0) d= po->depth*po->ext;
			else d= -po->depth*po->ext;

			p=(short *)(po+1);
			for(a=0;a<po->poly;a++) {
				vert= *p;
				p++;
				for(b=0;b<vert;b++) {
					vec1[0]= p[0];
					vec1[1]= p[1];
					vec1[2]= d;
					vec1+=3;
					p+=3;
				}
			}
		}
	}
	/* tekenen, vec1-4 zijn de verschillende polys */
	vec1=vec;
	vec2=vec1+3*po->vert;
	vec3=vec2+3*po->vert;
	vec4=vec3+3*po->vert;

	if(ext==2) vec4= vec2;

	p=(short *)(po+1);
	for(a=0;a<po->poly;a++) {
		vert= *p;
		p++;
		if(po->f1 & 3 ) {
			bgnclosedline();
			for(b=0;b<vert;b++,vec1+=3) v3i(vec1);
			endclosedline();
		}
		if(po->f1 & 24) {
			bgnclosedline();
			for(b=0;b<vert;b++,vec4+=3) v3i(vec4);
			endclosedline();
		}
		if(ext==4) {
			if(po->f1 & 6) {
				bgnclosedline();
				for(b=0;b<vert;b++,vec2+=3) v3i(vec2);
				endclosedline();
			}
			if(po->f1 & 12) {
				bgnclosedline();
				for(b=0;b<vert;b++,vec3+=3) v3i(vec3);
				endclosedline();
			}
		}
		p+= 3*vert;
	}

	vec1=vec;
	vec2=vec1+3*po->vert;
	vec3=vec2+3*po->vert;
	vec4=vec3+3*po->vert;
	if(ext==4 && (po->f1 & 14)==14) {
		p=(short *)(po+1);
		for(a=0;a<po->poly;a++) {
			vert= *p;
			p++;
			for(b=0;b<vert;b++) {
				bgnline();
				v3i(vec1); 
				v3i(vec2);
				v3i(vec3); 
				v3i(vec4);
				endline();
				vec1+=3; 
				vec2+=3; 
				vec3+=3; 
				vec4+=3;
			}
			p+= 3*vert;
		}
	} else {
		p=(short *)(po+1);
		for(a=0;a<po->poly;a++) {
			vert= *p;
			p++;
			c= 0;
			if(ext==2) {
				if(po->ext && (po->f1 & 4)!=0) c=1;
				else if( (po->f45m | po->f45p) && (po->f1 & 2)!=0) c=1;
			}
			else {
				if(po->f1 & 2) c=1;
				if(po->f1 & 4) c+=2;
				if(po->f1 & 8) c+=4;
			}
			if(c & 1) tekenpolyzijpart(vert,vec1,vec2);
			if(c & 2) tekenpolyzijpart(vert,vec2,vec3);
			if(c & 4) tekenpolyzijpart(vert,vec3,vec4);

			vert*= 3;
			vec1+= vert;
			vec2+= vert;
			vec3+= vert;
			vec4+= vert;
			p+= vert;
		}
	}

	freeN(vec);
}

void tekentextcurs()
{
	cpack(0);
	bgnpolygon();
	v2f(G.textcurs[0]);
	v2f(G.textcurs[1]);
	v2f(G.textcurs[2]);
	v2f(G.textcurs[3]);
	endpolygon();
}

void tekensphere(dim)
short dim;
{
	float fi=0;
	float si[60],co[60],*vec,*v;
	short a,b;

	for(a=0;a<=2*dim;a++) {
		si[a]= fsin(fi);
		co[a]= fcos(fi);
		fi+=PI/dim;
	}
	vec=(float *)mallocN(2*(dim+1)*(dim+1)*3*4,"spheremem");
	v=vec;
	for(a=0;a<=2*dim;a++) {
		for(b=0;b<=dim;b++) {
			*(v++)=si[a]*si[b];
			*(v++)=co[a]*si[b];
			*(v++)=co[b];
		}
	}
	v=vec;
	for(a=0;a<=2*dim;a++) {
		bgnline();
		for(b=0;b<=dim;b++) {
			v3f(v);
			v+=3;
		}
	}
	v=vec;
	for(b=0;b<=dim;b++) {
		v=vec+3*b;
		bgnline();
		for(a=0;a<=2*dim;a++) {
			v3f(v);
			v+=3*dim+3;
		}
	}
	freeN(vec);
}

ulong nurbcol[8]= {
	0, 0x9090, 0x8000, 0x502080, 0, 0xd0d0, 0xdd33, 0xA090F0 };

void tekenhandlesN(nu, sel)
struct Nurb *nu;
short sel;
{
	struct BezTriple *bezt;
	float *fp;
	ulong *col;
	long a;

	if(nu->hide) return;
	if( (nu->type & 7)==1) {
		if(sel) col= nurbcol+4;
		else col= nurbcol;

		bezt= nu->bezt;
		a= nu->pntsu;
		while(a--) {
			if(bezt->hide==0) {
				if( (bezt->f2 & 1)==sel) {
					fp= bezt->vec[0];
					cpack(col[bezt->h1]);
					bgnline(); 
					v3f(fp);
					v3f(fp+3); 
					endline();
					cpack(col[bezt->h2]);
					bgnline(); 
					v3f(fp+3); 
					v3f(fp+6); 
					endline();
				}
				else if( (bezt->f1 & 1)==sel) {
					fp= bezt->vec[0];
					cpack(col[bezt->h1]);
					bgnline(); 
					v3f(fp); 
					v3f(fp+3); 
					endline();
				}
				else if( (bezt->f3 & 1)==sel) {
					fp= bezt->vec[1];
					cpack(col[bezt->h2]);
					bgnline(); 
					v3f(fp); 
					v3f(fp+3); 
					endline();
				}
			}
			bezt++;
		}
	}
}

void tekenvertsN(nu, sel)
struct Nurb *nu;
short sel;
{
	struct BezTriple *bezt;
	struct BPoint *bp;
	ulong *rect;
	long a;

	if(nu->hide) return;

	if(sel) rect= (ulong *)rectyellow;
	else rect= (ulong *)rectpurple;

	if((nu->type & 7)==1) {

		bezt= nu->bezt;
		a= nu->pntsu;
		while(a--) {
			if(bezt->hide==0) {
				if((bezt->f1 & 1)==sel) tekenrect3(bezt->s[0], rect);
				if((bezt->f2 & 1)==sel) tekenrect3(bezt->s[1], rect);
				if((bezt->f3 & 1)==sel) tekenrect3(bezt->s[2], rect);
			}
			bezt++;
		}
	}
	else {
		bp= nu->bp;
		a= nu->pntsu*nu->pntsv;
		while(a--) {
			if(bp->hide==0) {
				if((bp->f1 & 1)==sel) tekenrect3(bp->s, rect);
			}
			bp++;
		}
	}
}

void tekenNurb(base, nurb, kleur)
struct Base *base;
struct Nurb *nurb;
ulong kleur;
{
	struct ObData *ob;
	struct MoData *mo;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp, *bp1;
	float seg[12], *fp, *data;
	long a, b, ofs;

	/* eerst handles niet select */
	nu= nurb;
	while(nu) {
		if((nu->type & 7)==1) {
			tekenhandlesN(nu, 0);
		}
		nu= nu->next;
	}
	
	/* dan DispList */
	cpack(kleur);
	if(base->soort==5 || base->soort==11) {
		ob= (struct ObData *)base->d;
		drawDispList( &(ob->cu->disp) );
	}
	else if(base->soort== -2) {
		mo= (struct MoData *)base->d;
		drawDispList( &(mo->disp) );
	}

	nu= nurb;
	while(nu) {
		if(nu->hide==0) {
			switch(nu->type & 7) {
			case 0:
				cpack(nurbcol[3]);
				bp= nu->bp;
				for(b=0; b<nu->pntsv; b++) {
					if(nu->flagu & 1) bgnclosedline();
					else bgnline();

					for(a=0; a<nu->pntsu; a++, bp++) {
						v3f(bp->vec);
					}

					if(nu->flagu & 1) endclosedline();
					else endline();
				}
				break;
			case 4:

				bp= nu->bp;
				for(b=0; b<nu->pntsv; b++) {
					bp1= bp;
					bp++;
					for(a=nu->pntsu-1; a>0; a--, bp++) {
						if(bp->hide==0 && bp1->hide==0) {
							if( (bp->f1 & 1) && ( bp1->f1 & 1) ) cpack(nurbcol[5]);
							else cpack(nurbcol[1]);
							bgnline();
							v3f(bp->vec); 
							v3f(bp1->vec);
							endline();
						}
						bp1= bp;
					}
				}
				if(nu->pntsv > 1) {
					ofs= nu->pntsu;
					for(b=0; b<nu->pntsu; b++) {
						bp1= nu->bp+b;
						bp= bp1+ofs;
						for(a=nu->pntsv-1; a>0; a--, bp+=ofs) {
							if(bp->hide==0 && bp1->hide==0) {
								if( (bp->f1 & 1) && ( bp1->f1 & 1) ) cpack(nurbcol[7]);
								else cpack(nurbcol[3]);
								bgnline();
								v3f(bp->vec); 
								v3f(bp1->vec);
								endline();
							}
							bp1= bp;
						}
					}
				}
				break;
			}
		}
		nu= nu->next;
	}

	calc_Nurbverts(nurb);

	if(G.zbuf) zbuffer(0);
	
	nu= nurb;
	while(nu) {
		if((nu->type & 7)==1) tekenhandlesN(nu, 1);
		tekenvertsN(nu, 0);
		tekenvertsN(nu, 1);
		nu= nu->next;
	}
	
	if(G.zbuf) zbuffer(1);
}

void tekencambase(base)
struct Base *base;
{
	/* een staande piramide met (0,0,0) als top */
	float vec[8][3], fac, facx, facy, depth;

	depth= -view0.lens*1000.0/16.0;
	vec[0][0]= 0; vec[0][1]= 0; vec[0][2]= 1;	/* GLBUG: z niet op nul vanwege picking op entry */
	vec[1][0]= S.x; vec[1][1]= S.y; vec[1][2]= depth;
	vec[2][0]= S.x; vec[2][1]= -S.y; vec[2][2]= depth;
	vec[3][0]= -S.x; vec[3][1]= -S.y; vec[3][2]= depth;
	vec[4][0]= -S.x; vec[4][1]= S.y; vec[4][2]= depth;

	

	bgnclosedline();
		v3f(vec[0]); 
		v3f(vec[1]); 
		v3f(vec[2]); 
		v3f(vec[0]); 
		v3f(vec[3]); 
		v3f(vec[4]);
	endclosedline();

	bgnline();
		v3f(vec[2]); 
		v3f(vec[3]);
	endline();
	bgnline();
		v3f(vec[4]); 
		v3f(vec[1]);
	endline();

	/* pijl aan top */
	concave(1);
	vec[0][2]= depth;
	bgnpolygon();
		vec[0][0]= -200; 
		vec[0][1]= 1000;
		v3f(vec[0]);
		vec[0][0]= 200;
		v3f(vec[0]);
		vec[0][1]= 1600;
		v3f(vec[0]);
		vec[0][0]= 400;
		v3f(vec[0]);
		vec[0][0]= 0; 
		vec[0][1]= 2000;
		v3f(vec[0]);
		vec[0][0]= -400; 
		vec[0][1]= 1600;
		v3f(vec[0]);
		vec[0][0]= -200;
		v3f(vec[0]);
	endpolygon();
	concave(0);
	
	if(base==view0.cambase && strcmp(view0.cambase->str, "Holo")==0) {
		/* viewplane op 'near' dist */
		depth= -view0.lens*1000.0/12.0;		/* deze klopt beter dan vorige */
		fac= ((float)view0.near)/depth;
		
		vec[1][0]= fac*S.x; vec[1][1]= fac*S.y; vec[1][2]= -view0.near;
		vec[2][0]= fac*S.x; vec[2][1]= -fac*S.y; vec[2][2]= -view0.near;
		vec[3][0]= -fac*S.x; vec[3][1]= -fac*S.y; vec[3][2]= -view0.near;
		vec[4][0]= -fac*S.x; vec[4][1]= fac*S.y; vec[4][2]= -view0.near;
		
		bgnclosedline();
			v3f(vec[1]); 
			v3f(vec[2]); 
			v3f(vec[3]); 
			v3f(vec[4]); 
		endclosedline();
		
		/* x verdwijnpunt op 'len' procent afstand */
		
		facx= (1.0+ 0.01*view0.cambase->len)*(float)view0.near;
		
		if(view0.cambase->holo1) {
			/* y verdwijnpunt */
			facy= facx+(1.0+ 0.1*view0.cambase->holo1)*(float)view0.near;
		
			/* hoogte strookje op H2: 1.0 - (facx-near)/(facy-near) */
			facy= fac*S.y*(1.0-  (facx-view0.near)/(facy-view0.near) );
		}
		else {
			facy= 0;
			facy= fac*S.y*(1.0-  (facx-view0.near)/(facy-view0.near) );
		}
		
		vec[0][0]= 0.0; vec[0][1]= facy; vec[0][2]= -facx;
		vec[5][0]= 0.0; vec[5][1]= -facy; vec[5][2]= -facx;
		
		bgnline();
			v3f(vec[4]); v3f(vec[0]); v3f(vec[5]); v3f(vec[3]);
		endline();
		bgnline();
			v3f(vec[1]); v3f(vec[0]);
		endline();
		bgnline();
			v3f(vec[2]); v3f(vec[5]);
		endline();

		if(view0.cambase->holo1) {

			/* verdwijnlijn in y: z= facy, breedte= (facy-facx)*fac*S.x/(facx-near) */
			facy= facx+(1.0+ 0.1*view0.cambase->holo1)*(float)view0.near;
			
			fac= (facy-facx)*fac*S.x/(facx-view0.near);
			
			vec[6][0]= -fac;  vec[6][1]= 0.0; vec[6][2]= -facy;
			vec[7][0]= fac;  vec[7][1]= 0.0; vec[7][2]= -facy;
			
			bgnline();
				v3f(vec[0]); v3f(vec[6]); v3f(vec[5]);
			endline();
			bgnline();
				v3f(vec[0]); v3f(vec[7]); v3f(vec[5]);
			endline();
			bgnline();
				v3f(vec[6]); v3f(vec[7]);
			endline();
		}
	}
}

void tekenobslimits()
{
	float sta[3],end[3],len,vec[3];
	short s[2];

	cpack(0);

	vec[0]= view0.tar[0]- view0.obs[0];
	vec[1]= view0.tar[1]- view0.obs[1];
	vec[2]= view0.tar[2]- view0.obs[2];
	Normalise(vec);

	len= ((float)view0.near);
	sta[0]= view0.obs[0]+ len*vec[0];
	sta[1]= view0.obs[1]+ len*vec[1];
	sta[2]= view0.obs[2]+ len*vec[2];

	len= 1000*((float)view0.far);
	end[0]= view0.obs[0]+ len*vec[0];
	end[1]= view0.obs[1]+ len*vec[1];
	end[2]= view0.obs[2]+ len*vec[2];

	bgnline();
	v3f(sta);
	v3f(end);
	endline();

	berekenschermcofl(sta,s);
	if(s[0]!=3200) tekenrect3(s,rectyellow);
	berekenschermcofl(end,s);
	if(s[0]!=3200) tekenrect3(s,rectyellow);

	if(wrld.dofi) {
		cpack(0xFFFFFF);
		len= 1000*wrld.dofmin+view0.near;
		sta[0]= view0.obs[0]+len*vec[0];
		sta[1]= view0.obs[1]+len*vec[1];
		sta[2]= view0.obs[2]+len*vec[2];
		len= 1000*wrld.dofsta+view0.near;
		end[0]= view0.obs[0]+len*vec[0];
		end[1]= view0.obs[1]+len*vec[1];
		end[2]= view0.obs[2]+len*vec[2];
		bgnline();
		v3f(sta);
		v3f(end);
		endline();

		len= 1000*wrld.dofend+view0.near;
		sta[0]= view0.obs[0]+len*vec[0];
		sta[1]= view0.obs[1]+len*vec[1];
		sta[2]= view0.obs[2]+len*vec[2];
		len= 1000*wrld.dofmax+view0.near;
		end[0]= view0.obs[0]+len*vec[0];
		end[1]= view0.obs[1]+len*vec[1];
		end[2]= view0.obs[2]+len*vec[2];
		bgnline();
		v3f(sta);
		v3f(end);
		endline();
	}
}

void tekenshadbuflimits(la,mat)
struct LaData *la;
float mat[][4];
{
	extern ulong rectwhite[3][3];
	float sta[3],end[3],len,lavec[3];
	short s[2];

	lavec[0]= -mat[2][0];
	lavec[1]= -mat[2][1];
	lavec[2]= -mat[2][2];
	Normalise(lavec);

	len= 1000*((float)la->clipsta);
	sta[0]= mat[3][0]+ len*lavec[0];
	sta[1]= mat[3][1]+ len*lavec[1];
	sta[2]= mat[3][2]+ len*lavec[2];

	len= 1000*((float)la->clipend);
	end[0]= mat[3][0]+ len*lavec[0];
	end[1]= mat[3][1]+ len*lavec[1];
	end[2]= mat[3][2]+ len*lavec[2];

	bgnline();
	v3f(sta);
	v3f(end);
	endline();

	berekenschermcofl(sta, s);
	if(s[0]!=3200) tekenrect3(s,rectwhite);
	berekenschermcofl(end, s);
	if(s[0]!=3200) tekenrect3(s,rectwhite);
}

void tekenjoint( struct Joint *joint)
{
	struct Rod *rod1, *rod2;
	float quat[4], nor[3], n1[3], n2[3];
	float co, co1, si, len, mat[3][3], p1[3], p2[3], p3[3];
	float amin, amax;
	
	rod1= joint->r1;
	rod2= joint->r2;
	
	amin= joint->amin;
	amax= joint->amax;
	
	if(joint->p1a) VecMidf(p1, joint->p1->r, joint->p1a->r);
	else VECCOPY(p1, joint->p1->r);

	if(joint->p2a) VecMidf(p2, joint->p2->r, joint->p2a->r);
	else VECCOPY(p2, joint->p2->r);

	if(joint->p3a) VecMidf(p3, joint->p3->r, joint->p3a->r);
	else VECCOPY(p3, joint->p3->r);

	VecSubf(n1, p1, p2);
	VecSubf(n2, p3, p2);

	Crossf(nor, n1, n2);
	Normalise(nor);
	
	/* p1 wordt vector van waaruit constraint wordt berekend */
	VecAddf(p1, n1, n2);
	Normalise(p1);
	len= 0.3*MIN2(rod1->len, rod2->len);
	VecMulf(p1, len);

	/* p2 blijft draaipunt */
	/* n1 wordt aminvec door p1 met 0.5*amin te roteren */
	
	co= fcos(0.25*amin);	/* halve hoek! */
	si= fsin(0.25*amin);

	quat[0]= co;
	quat[1]= nor[0]*si;
	quat[2]= nor[1]*si;
	quat[3]= nor[2]*si;
	QuatToMat3(quat, mat);
	VECCOPY(n1, p1);
	Mat3MulVecfl(mat, n1);

	/* zelfde voor amax: */
	co1= fcos(0.25*amax);	
	si= fsin(0.25*amax);

	quat[0]= co1;
	quat[1]= nor[0]*si;
	quat[2]= nor[1]*si;
	quat[3]= nor[2]*si;
	QuatToMat3(quat, mat);
	VECCOPY(n2, p1);
	Mat3MulVecfl(mat, n2);
	
	/* tekenen */
	cpack(0x505050);
	bgnclosedline();
	VecAddf(p3, n1, p2);
	v3f(p3);
	VecAddf(p3, n2, p2);
	v3f(p3);
	v3f(p2);
	endclosedline();
	
	/* de gespiegelde vectoren */
	co*= 2.0*fcos(0.5*amin);
	co1*= 2.0*fcos(0.5*amax);
	n1[0]= co*p1[0]-n1[0];
	n1[1]= co*p1[1]-n1[1];
	n1[2]= co*p1[2]-n1[2];
	n2[0]= co1*p1[0]-n2[0];
	n2[1]= co1*p1[1]-n2[1];
	n2[2]= co1*p1[2]-n2[2];

	/* tekenen */
	bgnclosedline();
	v3f(p2);
	VecAddf(p3, n1, p2);
	v3f(p3);
	VecAddf(p3, n2, p2);
	v3f(p3);
	endclosedline();
	
}

void tekenIka(struct Base *base, ulong kleur)
{
	extern ListBase bpbase, rodbase, jointbase;
	struct IkaData *ika;
	struct Rod *r;
	struct BodyPoint *p;
	struct Joint *joint;
	float vec[3], mat4[4][4], x, y, z, *data;
	int ofs, nr, parts, a;
	
	ika= (struct IkaData *)base->d;
	
	/* teken en bereken rods */
	if(base==G.ebase) r= rodbase.first;
	else r= ika->rodbase.first;
	
	if(ika->dt==2 && (G.f & 16)==0 && G.zbuf) startlighting();
	
	/*	p= bpbase.first;
		while(p) {
			bgnline();
				v3f(p->r);
				VECCOPY(vec, p->v);
				VecAddf(vec, p->r, vec);
				v3f(vec);
			endline();
			p= p->next;
		}
	*/
	
	while(r) {
		
		cpack(kleur);
		if(ika->dt==0) {
			if(r->type==ROD) {
				bgnline();
					v3f( r->p1->r );
					v3f( r->p2->r );
				endline();
			}
			else  {
				bgnclosedline();
					v3f( r->p1->r );
					v3f( r->p2->r );
					v3f( r->p3->r );
					if(r->type==H_ROD) v3f( r->p4->r );
				endclosedline();
			}
		}
		else {
			pushmatrix();

			Mat4CpyMat3(mat4, r->mat);
			if(r->type==ROD)
				VecMidf(mat4[3], r->p1->r, r->p2->r);
			else if(r->type==T_ROD) {
				VecMidf(vec, r->p2->r, r->p3->r);
				VecMidf(mat4[3], r->p1->r, vec);
			}
			else if(r->type==H_ROD) {
				CalcCent4f(mat4[3], r->p1->r, r->p2->r, r->p3->r, r->p4->r);
			}
			x= r->drawwidth;
			z= 0.45*r->lenn;
			mat4[0][0]*=x; mat4[0][1]*=x; mat4[0][2]*=x;
			mat4[1][0]*=x; mat4[1][1]*=x; mat4[1][2]*=x;
			mat4[2][0]*=z; mat4[2][1]*=z; mat4[2][2]*=z;
			
			multmatrix(mat4);
			
			if(G.zbuf && ika->dt==2) drawgourcube();
			else drawcube();
			
			popmatrix();

			if(r->type==T_ROD) {
				bgnline();
					v3f( r->p2->r );
					v3f( r->p3->r );
				endline();
			}
			else if(r->type==H_ROD) {
				bgnline();
					v3f( r->p1->r );
					v3f( r->p2->r );
				endline();
				bgnline();
					v3f( r->p3->r );
					v3f( r->p4->r );
				endline();
			}
		}
		if(ika->dt) {	/* teken ups met mat4 */
		
			bgnline();
				v3f(mat4[3]);
				mat4[3][0]+= 0.3*r->len*r->up[0];
				mat4[3][1]+= 0.3*r->len*r->up[1];
				mat4[3][2]+= 0.3*r->len*r->up[2];
				v3f(mat4[3]);
			endline();
		}

		r= r->next;
	}
	
	if(ika->dt==2 && (G.f & 16)==0 && G.zbuf) stoplighting();

	if(base==G.ebase) {	
	
		/* tekenjoints */
		joint= jointbase.first;
		while(joint) {
			tekenjoint(joint);
			joint= joint->next;
		}

		/* teken verts en berekenschermco's */
		if(G.zbuf) zbuffer(0);
		
		getsingmatrix(mat4);
		Mat4SwapMat4(G.persmat, mat4);

		p= bpbase.first;
	
		while(p) {
			berekenschermcofl(p->r, &(p->sx));
			if(p->sx!=3200) {
				if(p->f & 1) lrectwrite(p->sx-2, p->sy-2, p->sx+2, p->sy+2, (ulong *)rectyellow);
				else lrectwrite(p->sx-2, p->sy-2, p->sx+2, p->sy+2, (ulong *)rectpurple);
				
			}
			p= p->next;
		}
		if(G.zbuf) zbuffer(1);
		Mat4SwapMat4(G.persmat, mat4);
		
	}
	else if(base==G.basact) {	/* berekenen schermco's voor tekenrodnummers */
		getsingmatrix(mat4);
		Mat4SwapMat4(G.persmat, mat4);

		p= ika->bpbase.first;
		while(p) {
			berekenschermcofl(p->r, &(p->sx));
			p= p->next;
		}
		Mat4SwapMat4(G.persmat, mat4);
	}
}

int scr_centre_rod(struct Rod *rod, short *cx, short *cy)
{
	int div=0;

	*cx= 0;
	*cy= 0;
	if(rod->p1->sx!=3200) {
		div++;
		*cx+= rod->p1->sx;
		*cy+= rod->p1->sy;
	}
	if(rod->p2->sx!=3200) {
		div++;
		*cx+= rod->p2->sx;
		*cy+= rod->p2->sy;
	}
	if(rod->p3 && rod->p3->sx!=3200) {
		div++;
		*cx+= rod->p3->sx;
		*cy+= rod->p3->sy;
	}
	if(rod->p4 && rod->p4->sx!=3200) {
		div++;
		*cx+= rod->p4->sx;
		*cy+= rod->p4->sy;
	}
	if(div) {
		*cx /= div;
		*cy /= div;
	}
	return div;
}

void tekenrodnummers(base)
struct Base *base;
{
	extern ListBase bpbase, rodbase, jointbase;
	struct IkaData *ika;
	struct Rod *rod;
	short cx, cy, rodnr=1;
	char str[12];
	
	ika= (struct IkaData *)base->d;
	
	if(base==G.ebase) rod= rodbase.first;
	else rod= ika->rodbase.first;
	
	persp(0);
	frontbuffer(1);
	backbuffer(0);
	
	while(rod) {
		
		if(scr_centre_rod(rod, &cx, &cy)) {
			sprintf(str, "%d", rodnr);
			
			cpack(0);
			sboxfs(cx-2, cy-2, cx+20, cy+12);
			cmov2i(cx, cy);
			cpack(0xFFFFFF);
			charstr(str);
			
		}
		
		rod= rod->next;
		rodnr++;
	}
	
	persp(1);
	frontbuffer(0);
	backbuffer(1);
}

void tekenskelnummers(base)
struct Base *base;
{
	extern ListBase bpbase, rodbase, jointbase, skelbase;
	struct IkaData *ika;
	struct Rod *rod, **rodpp;
	struct Skeleton *skel;
	short cx, cy, a, skelnr=1, ofs;
	char str[12];
	
	ika= (struct IkaData *)base->d;
	
	if(base==G.ebase) skel= skelbase.first;
	else skel= ika->skelbase.first;
	
	persp(0);
	frontbuffer(1);
	backbuffer(0);
	
	/* in rod->flag schrijven we een offset, eerst wissen */
	if(base==G.ebase) rod= rodbase.first;
	else rod= ika->rodbase.first;
	while(rod) {
		rod->flag= 0;
		rod= rod->next;
	}
	
	while(skel) {

		sprintf(str, "%d", skelnr);

		rodpp= (struct Rod **)(skel+1);
		for(a=0; a<skel->nr; a++, rodpp++) {
			
			if(scr_centre_rod(*rodpp, &cx, &cy)) {
				
				ofs= (*rodpp)->flag;
				
				cpack(0);
				sboxfs(cx-2+ofs, cy-2+ofs, cx+20+ofs, cy+12+ofs);
				cmov2i(cx+ofs, cy+ofs);
				cpack(0xFFFFFF);
				charstr(str);
				
				(*rodpp)->flag+= 10;
			}
		}
		skel= skel->next;
		skelnr++;
	}
	
	persp(1);
	frontbuffer(0);
	backbuffer(1);
}

/* ***************************************************** */


void tekenbase(base, kleur)	/* alleen doen als proj-mat 'aktief' */
struct Base *base;		/* kleur!=0 : backbufproj */
ulong kleur;
{
	extern ulong rect_desel[16],rect_sel[16];
	extern struct ListBase editNurb;
	struct Base *pbase;
	struct ObData *ob;
	struct VV *vv;
	struct PerfSph *ps;
	struct LaData *la;
	struct MoData *mo;
	struct PolyData *po;
	struct CurveData *cu;
	struct Bezier *bez;
	struct Key *key;
	struct ColBlck *cb;
	float mat[4][4],x,y,z,omat[3][3];
	long vec[3], *l0, tot, a, b;
	short solid, ok, countkeys();
	static short keydraw= -1,keynr;

	if(base==0) return;

	if((base->lay & view0.lay)==0) {
		if(G.moving) parentbase(base,mat,omat);
		return;
	}

	/* duplicates voor keyposities */
	/* G.keydraw 0: alleen actkey van basact
		     1: alle keys van basact
		     2: alle keys van alle bases
	*/
	if(G.ipomode==20 && base->key!=0 && keydraw== -1) {
		if( (base==G.basact && G.keydraw!=2) || G.keydraw==2 ) {
			if( countkeys() ) {
				keydraw=1;
				key= base->key;
				keynr=1;
				if(G.actkey==0) pushpopdata(base,(long)sizeof(struct Base),0);
				while(key) {
					ok= G.keydraw;
					if(G.keydraw==0) {
						if(keynr==G.actkey) ok=1;
					}
					if(ok) {
						l0= (long *)(key+1);
						memcpy(base->v,l0,12);
						memcpy(base->o,l0+3,12+16+36);
						tekenbase(base,kleur);
					}
					key=key->next;
					keynr++;
				}
				keydraw=0;
				if(G.actkey==0) pushpopdata(base,(long)sizeof(struct Base),1);
				else loadkeypostype(20, base, base);
			}
		}
	}
	/* i.v.m. parentkeys dit: */
	if(G.moving && keydraw!=1 && base->pkey) {
		loadkeypos(base, base);
	}

	/* kleuren */
	if(kleur==0) {
		kleur= 1;
		if(base->f & 1) {
			if(G.moving) kleur=0xFFFFFF;
			else kleur=0xFFA0FF;
		}
		if(G.moving) if(base->f & 2) kleur=0xFFFFFF;
		if(base->f & 128) kleur= 0x505050;  /* dup */
		else if(keydraw==1) {
			if(keynr==G.actkey) kleur=0xE0C080;
			else kleur=0x404000;
		}
		else if(keydraw==0) {
			if(G.actkey!=0 && (base->f & 1)) kleur=0xCA60FF;
			keydraw= -1;
		}
	}
	else {
		if(keydraw== 0) keydraw= -1;   /* wordt hierboven gebruikt */
	}

	cpack(kleur);

	parentbase(base,mat,omat);
	berekenschermcofl(mat[3], &base->sx);
	vec[0]=vec[1]=vec[2]=0;

	switch(base->soort) {
	case 1:	/* object */
		ob= (struct ObData *)base->d;
		vv= ob->vv;
		
		if(ob->ef && ob->ef<4) {
			bgneffect(vv);
			if(ob->ef==3) vertexTex(ob);
			else if(ob->ef==2) vlag(base);
			else if(ob->ef==1) golf(base);
		}
		if(ob->dt==0 && G.ebase!=base) {
			Mat4MulFloat3(mat, 32767.0*vv->ws);

			multmatrix(mat);
			callobj(1);
			if(do_drawaxes) {
				cpack(0xB0F060);
				drawaxes();
			}

		} else {
			Mat4MulFloat3(mat,vv->ws);
			if(vv->key!=0 && vv->us>1) loadkeypos(base, base);
			multmatrix(mat);
			/* solid= (G.zbuf && ob->dt==2); */
			
			cb= (struct ColBlck *)(ob+1);
			
			if(base->skelnr && G.moving && (base->f & 1) && base!=G.ebase) calc_deform(base);
			
			if(base==G.ebase) wire(base, kleur);
			else if(cb->hasize || vv->vlak==0) drawhaloverts(base, kleur);
			else if(ob->disp.first==0) wire(base, kleur);
			else drawDispList(&(ob->disp));
		}

		loadmatrix(G.viewmat);
		if(ob->ef && ob->ef<4) {
			endeffect(vv);
		}
		break;
	case 2: /* lamp */
		la= (struct LaData *)base->d;
		multmatrix(mat);
		setlinestyle(1);
		if(la->soort==1) {
			float lvec[3],vvec[3],x,y,z;

			lvec[0] =  lvec[1] = 0 ; 
			lvec[2] = 1;
			x = G.persmat[0][2];
			y = G.persmat[1][2];
			z = G.persmat[2][2];
			vvec[0]=x*mat[0][0] + y*mat[0][1] + z*mat[0][2];
			vvec[1]=x*mat[1][0] + y*mat[1][1] + z*mat[1][2];
			vvec[2]=x*mat[2][0] + y*mat[2][1] + z*mat[2][2];

			y = fcos( PI*((float)(256- la->spsi))/512.0 );
			spotvolume(lvec,vvec,y);
			x = -la->ld;
			lvec[0] *=  x ; 
			lvec[1] *=  x ; 
			lvec[2] *=  x;
			vvec[0] *= x ; 
			vvec[1] *= x ; 
			vvec[2] *= x;
			bgnline();
			v3f(vvec);
			v3i(vec);
			v3f(lvec);
			endline();
			z = x*fsqrt(1 - y*y);
			x *= y;
			translate(0.0 , 0.0 , x);
			circ(0.0 , 0.0 , z);
			/*
			translate(0.0 , 0.0 , -x);
			bgnline();
				v3i(vec); vec[2]= x; v3i(vec);
			endline();
			*/
		} else if(la->soort>=2) {
			bgnline();
			v3i(vec); 
			vec[2]= -la->ld; 
			v3i(vec);
			endline();
		}
		loadmatrix(G.viewmat);
		VECCOPY(vec,base->r);
		bgnline();
		v3i(vec); 
		vec[2]= 0; 
		v3i(vec);
		endline();
		setlinestyle(0);
		if(la->soort==1 && (la->f & 1) ) {
			tekenshadbuflimits(la, mat);
		}
		break;

	case 4:	    /* cambase */
		multmatrix(mat);
		tekencambase(base);
		loadmatrix(G.viewmat);
		break;
	case 7: 
	case 9:
		if(base==G.ebase) cpack(0xFFFF90);
		ob=(struct ObData *)base->d;
		po= ob->po;

		multmatrix(mat);
		
		if(ob->dt==0 && base!=G.ebase) {
			z= 32767.0*po->ws;
			scale(z, z, z);
			callobj(1);

			if(do_drawaxes) {
				cpack(0xB0F060);
				drawaxes();
			}
		}
			/* tekenpolyfast(po); */
		else tekenpoly(po);
		
		if(base==G.ebase && base->soort==9) tekentextcurs();
		loadmatrix(G.viewmat);
		break;
	case 5: 
	case 11:
		ob=(struct ObData *)base->d;
		cu= ob->cu;
		multmatrix(mat);
		scale(cu->ws, cu->ws, cu->ws);

		if(base==G.ebase) tekenNurb(base, editNurb.first, kleur);
		else {
			cpack(kleur);
			if(ob->dt==0) {
				scale(32767.0, 32767.0, 32767.0);
				callobj(1);

				if(do_drawaxes) {
					cpack(0xB0F060);
					drawaxes();
				}
			}
			else drawDispList(&(cu->disp));
		}
		loadmatrix(G.viewmat);
		break;
	case -2:
		mo= (struct MoData *)base->d;
		
		if(mo->disp.first) {
			multmatrix(mat);
			if(base==G.ebase) tekenNurb(base, editNurb.first, kleur);
			else {
				cpack(kleur);
				drawDispList(&(mo->disp));
			}
			loadmatrix(G.viewmat);
		}
		break;
	case -4:
		multmatrix(mat);
		tekenIka(base, kleur);
		loadmatrix(G.viewmat);
		
		break;
	}

	if(keydraw==1 || (base->f & 128)) {
		/* return, behalve bij Quat */
		if(base->soort== -2) {
			mo=(struct MoData *)base->d;
			if(mo->disp.first==0) {
				tekenrect9(&base->sx,rectq_key);
			}
		}
		return;
	}
	
	if(G.f & 16) {	/* in backbufproj */

		bgnpoint(); v3f(mat[3]); endpoint();
		
		return;
	}

	/* parent lijnen */
	if(G.hie) {
		if(base->p!=0 && (base->p->lay & view0.lay)) {
			setlinestyle(1);
			if(base->p->soort== -2) {
				mo=(struct MoData *)base->p->d;
				if(mo->data) {
					bgnline();
					VECCOPY(vec,base->r);
					v3i(vec);
					vec[0]-=base->r2[0];
					vec[1]-=base->r2[1];
					vec[2]-=base->r2[2];
					v3i(vec);
					endline();
				}
				else {
					bgnline();
					v3i(base->p->r);
					v3i(base->r);
					endline();
				}
			} else {
				bgnline();
				v3i(base->p->r);
				v3i(base->r);
				endline();
			}
			setlinestyle(0);
		}
		if(base->pkey) {
			setlinestyle(1);
			key= base->pkey;
			while(key) {
				pbase= (struct Base *)(*( (long *)(key+1) ));
				if(pbase && (pbase->lay & view0.lay) ) {
					bgnline();
					v3i(pbase->r);
					v3i(base->r);
					endline();
				}
				key= key->next;
			}
			setlinestyle(0);
		}

		/* track lijnen */
		if(base->t!=0 && (base->t->lay & view0.lay)) {
			setlinestyle(3);
			bgnline();
			v3i(base->t->r);
			v3i(base->r);
			endline();
			setlinestyle(0);
		}
	}

	/* symbooltjes */
	if( (base->soort & 1) || base->soort==4 || base->soort== -4) {
		if(base->f & 1) tekenrect4(&base->sx,rect_sel);
		else tekenrect4(&base->sx,rect_desel);
	} else if(base->soort==2) {
		if(base->f & 1) tekenrect9(&base->sx,rectl_sel);
		else tekenrect9(&base->sx,rectl_desel);
		/* tekenlampcirc(base->sx, base->sy, base->f & 1); */
	} else if(base->soort== -2) {
		mo=(struct MoData *)base->d;
		if(mo->disp.first) {
			if(base->f & 1) tekenrect4(&base->sx,rect_sel);
			else tekenrect4(&base->sx,rect_desel);
		} else {
			if(base->f & 1) tekenrect9(&base->sx,rectq_sel);
			else tekenrect9(&base->sx,rectq_desel);
		}
	}

}


void tekenbase_ext(base)  /* maakt alles eerst in orde */
struct Base *base;
{

	winset(G.winar[0]);
	if(G.zbuf) zbuffer(1);
	frontbuffer(1);
	backbuffer(0);

	tekenbase(base,0);

	frontbuffer(0);
	backbuffer(1);
	if(G.zbuf) zbuffer(0);
}

void draw_textuspace_ext()
{
	struct Base *base;
	struct ObData *ob;
	int temp;
	
	winset(G.winar[0]);
	if(G.zbuf) zbuffer(1);
	frontbuffer(1);
	backbuffer(0);

	do_drawaxes= 1;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {

			if ELEM5(base->soort, 1, 5, 7, 9, 11) {
				ob= (struct ObData *)base->d;
				temp= ob->dt;
				ob->dt= 0;
				
				tekenbase(base, 0x60E030);
				
				ob->dt= temp;
			}
		}
		base= base->next;
	}
	
	do_drawaxes= 0;

	frontbuffer(0);
	backbuffer(1);
	if(G.zbuf) zbuffer(0);
}

