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



/*  drawsector.c     GRAPHICS
 * 
 *  maart 96
 *  
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "sector.h"


float texprops[] = {TX_MINFILTER, TX_POINT,
		    TX_MAGFILTER, TX_POINT,
		    TX_WRAP, TX_REPEAT, TX_NULL};

			
float tevprops[] = {TV_MODULATE, TV_NULL};


Image *curpage=0;
int subdivtex= 0, curtile=0, curmode=0;
short texwindx, texwindy, texwinsx, texwinsy;

void copy_part_from_ibuf(ImBuf *ibuf, ulong *rect, short startx, short starty, short endx, short endy)
{
	ulong *rt, *rp;
	short y, heigth, len;

	/* de juiste offset in rectot */

	rt= ibuf->rect+ (starty*ibuf->x+ startx);

	len= (endx-startx);
	heigth= (endy-starty);

	rp=rect;
	
	for(y=0; y<heigth; y++) {
		memcpy(rp, rt, len*4);
		rt+= ibuf->x;
		rp+= len;
	}

}

void make_reprect(Image *ima)
{
	ulong *rt;
	int dx, dy, sx, sy;
	
	if(ima==0 || ima->ibuf==0) return;
	
	dx= ima->ibuf->x/ima->xrep;
	dy= ima->ibuf->y/ima->yrep;
	
	if(ima->reprect) freeN(ima->reprect);
	ima->reprect= mallocN(4*ima->ibuf->x*ima->ibuf->y, "reprect");
	
	rt= ima->reprect;
	for(sy= 0; sy+dy<=ima->ibuf->y; sy+= dy) {
		for(sx= 0; sx+dx<=ima->ibuf->x; sx+= dx) {
			
			copy_part_from_ibuf(ima->ibuf, rt, sx, sy, sx+dx, sy+dy);
			rt+= dx*dy;
			
		}
	}

}

/* TRAAG OP O2 */
/* oplossing voor dit probleem: alle texdef2d's van te voren doen. In feite als 
 * tpage nummer!
 */


int set_tpage(TFace *tface)
{	
	static int firsttime= 1;
	Image *ima;
	float mat[4][4];
	int tpx, tpy, tilemode;

	/* afschakelen */
	if(tface==0) {
		curtile= 0;
		curpage= 0;
		curmode= 0;
		
		texbind(TX_TEXTURE_0, 0);
		return 0;
	}

	ima= tface->tpage;


	tilemode= tface->mode & TF_TILES;

	if(ima==curpage && curtile==tface->tile && tilemode==curmode ) return ima!=0;

	curpage= ima;
	curmode= tilemode;
	curtile= tface->tile;
	
	if(curpage==0) {
		texbind(TX_TEXTURE_0, 0);
		return 0;
	}

	if(ima->ibuf==0) {
		ima->ibuf = loadiffname(ima->name , LI_rect);
		
		if(ima->ibuf==0) {
			curpage= 0;
			texbind(TX_TEXTURE_0, 0);
			return 0;
		}
		
	}


	tpx= ima->ibuf->x;
	tpy= ima->ibuf->y;
	
	if(tface->mode & TF_TILES) {
		
		if(ima->reprect==0) make_reprect(ima);
		
		texwindx= tpx/ima->xrep;
		texwindy= tpy/ima->yrep;
		
		if(curtile>=ima->xrep*ima->yrep) curtile= ima->xrep*ima->yrep-1;

		texwinsy= curtile / ima->xrep;
		texwinsx= curtile - texwinsy*ima->xrep;

		texwinsx*= texwindx;
		texwinsy*= texwindy;

		if(subdivtex) return 1;

		texdef2d(1, 4, texwindx, texwindy, ima->reprect+curtile*texwindx*texwindy, 0, texprops);
		
		mmode(MTEXTURE);
		loadmatrix(matone);
		scale( (float)ima->xrep/32767.0, (float)ima->yrep/32767.0, 1.0);
		mmode(MVIEWING);

	}
	else {
		if(subdivtex) return 1;

		texdef2d(1, 4, tpx, tpy, ima->ibuf->rect, 0, texprops);

		mmode(MTEXTURE);
		loadmatrix(matone);
		scale( (float)1.0/32767.0, (float)1.0/32767.0, 1.0);
		mmode(MVIEWING);
	}

	if(firsttime) {
		firsttime= 0;
		tevdef(1, 0, tevprops);
		tevbind(TV_ENV0, 1);
		
	}
	
	texbind(TX_TEXTURE_0, 1);
	
	return 1;
}


void get_co_portal(Sector *se, int type, float *ofs, float *cent)
{
	
	cent[0]= cent[1]= cent[2]= 0.0;
	
	if(type==PO_XPOS || type==PO_XNEG) {
		if(type==PO_XPOS) cent[0]= 0.98*se->size[0];
		else cent[0]= -0.98*se->size[0];
		
		cent[1]+= ofs[0];
		cent[2]+= ofs[1];

	}
	else if(type==PO_YPOS || type==PO_YNEG) {
		if(type==PO_YPOS) cent[1]= 0.98*se->size[1];
		else cent[1]= -0.98*se->size[1];

		cent[0]+= ofs[0];
		cent[2]+= ofs[1];

	}
	else if(type==PO_ZPOS || type==PO_ZNEG) {
		if(type==PO_ZPOS) cent[2]= 0.98*se->size[2];
		else cent[2]= -0.98*se->size[2];

		cent[0]+= ofs[0];
		cent[1]+= ofs[1];

	}
}

void draw_portal(Sector *se, Portal *po, int act)
{
	float cent[3], veci[3], vecj[3];
	int cox, coy;
	short r, g, b;
	
	get_co_portal(se, po->type, po->ofs, cent);
	
	if(po->type==PO_XPOS || po->type==PO_XNEG) {
		cox= 1; coy= 2;
	}
	else if(po->type==PO_YPOS || po->type==PO_YNEG) {
		cox= 0; coy= 2;
	}
	else {
		cox= 0; coy= 1;
	}

	gRGBcolor(&r, &g, &b);
	
	if(G.f & G_BACKBUFSEL); else {
		if(po->sector) RGBcolor(0, 255, 0);
		else RGBcolor(0, 0, 0);
	}
	
	cent[cox]+= 0.05;
	cent[coy]-= 0.05;

	bgnpolygon();
		v3f(cent);
		cent[coy]+= 0.10;
		v3f(cent);
		cent[cox]-= 0.10;
		v3f(cent);
		cent[coy]-= 0.10;
		v3f(cent);
		cent[cox]+= 0.10;
	endpolygon();
	
	if(G.f & G_BACKBUFSEL) return;

	if(act) RGBcolor(255, 255, 255);
	else if(po->sector) RGBcolor(225, 200, 0);
	else if(r) RGBcolor(150, 50, 150);
	else RGBcolor(50, 50, 50);
	
	bgnclosedline();
		v3f(cent);
		cent[coy]+= 0.10;
		v3f(cent);
		cent[cox]-= 0.10;
		v3f(cent);
		cent[coy]-= 0.10;
		v3f(cent);
		cent[cox]+= 0.10;
	endclosedline();

	RGBcolor(r, g, b);
}

void spack(ulong ucol)
{
	char *col= (char *)&ucol;
	short scol[3];
	
	scol[0]= (col[3]<<1) - 1;
	scol[1]= (col[2]<<1) - 1;
	scol[2]= (col[1]<<1) - 1;
	
	c3s(scol);
}

int maxsub= 2;

typedef struct Flosh {
	float vec[3];
	short u, v;
	char col[4];
} Flosh;

void FloshMid(Flosh *v1, Flosh *v2, Flosh *v3)
{
	v1->vec[0]= (v2->vec[0]+v3->vec[0])/2;
	v1->vec[1]= (v2->vec[1]+v3->vec[1])/2;
	v1->vec[2]= (v2->vec[2]+v3->vec[2])/2;
	
	v1->u= (v2->u + v3->u)/2;
	v1->v= (v2->v + v3->v)/2;
	
	v1->col[1]= (v2->col[1] + v3->col[1])/2;
	v1->col[2]= (v2->col[2] + v3->col[2])/2;
	v1->col[3]= (v2->col[3] + v3->col[3])/2;
}

void recurs_quad_texpoly(int depth, Flosh *v1, Flosh *v2, Flosh *v3, Flosh *v4)
{
	
	if(depth) {
		Flosh fl[5];
		
		FloshMid(fl  , v1, v2);
		FloshMid(fl+1, v2, v3);
		FloshMid(fl+2, v3, v4);
		FloshMid(fl+3, v4, v1);
		
		FloshMid(fl+4, v1, v3);
		
		recurs_quad_texpoly(depth-1, v1, fl, fl+4, fl+3);
		recurs_quad_texpoly(depth-1, fl, v2, fl+1, fl+4);
		recurs_quad_texpoly(depth-1, fl+4, fl+1, v3, fl+2);
		recurs_quad_texpoly(depth-1, fl+3, fl+4, fl+2, v4);
		
	}
	else {
		ulong *rect;
		int xs= curpage->ibuf->x;
		int ys= curpage->ibuf->y, ofs;
		short scol[3];
		char *c1, *c2, *c3, *c4;
		
		if(curmode & TF_TILES) {
			rect= curpage->reprect +curtile*texwindx*texwindy;
			ofs= texwindx*texwindy;
			
			c1= (char *)(rect+   (((v1->v*ys)>>15) % texwindy)*texwindx +  (((v1->u*xs)>>15) % texwindx) );
			c2= (char *)(rect+   (((v2->v*ys)>>15) % texwindy)*texwindx +  (((v2->u*xs)>>15) % texwindx) );
			c3= (char *)(rect+   (((v3->v*ys)>>15) % texwindy)*texwindx +  (((v3->u*xs)>>15) % texwindx) );
			c4= (char *)(rect+   (((v4->v*ys)>>15) % texwindy)*texwindx +  (((v4->u*xs)>>15) % texwindx) );

			
		}
		else {
			rect= curpage->ibuf->rect;
			
			c1= (char *)(rect+  ((v1->v*ys)>>15)*xs + ((v1->u*xs)>>15) );
			c2= (char *)(rect+  ((v2->v*ys)>>15)*xs + ((v2->u*xs)>>15) );
			c3= (char *)(rect+  ((v3->v*ys)>>15)*xs + ((v3->u*xs)>>15) );
			c4= (char *)(rect+  ((v4->v*ys)>>15)*xs + ((v4->u*xs)>>15) );
		} 
		
		bgnpolygon();
		
		scol[2]= (c1[1]*v1->col[1])>>7;
		scol[1]= (c1[2]*v1->col[2])>>7;
		scol[0]= (c1[3]*v1->col[3])>>7;
		
		c3s(scol);
		v3f(v1->vec);
		
		scol[2]= (c2[1]*v2->col[1])>>7;
		scol[1]= (c2[2]*v2->col[2])>>7;
		scol[0]= (c2[3]*v2->col[3])>>7;
		
		c3s(scol);
		v3f(v2->vec);
		
		scol[2]= (c3[1]*v3->col[1])>>7;
		scol[1]= (c3[2]*v3->col[2])>>7;
		scol[0]= (c3[3]*v3->col[3])>>7;
		
		c3s(scol);
		v3f(v3->vec);
		
		scol[2]= (c4[1]*v4->col[1])>>7;
		scol[1]= (c4[2]*v4->col[2])>>7;
		scol[0]= (c4[3]*v4->col[3])>>7;
		
		c3s(scol);
		v3f(v4->vec);
		
		endpolygon();		
	}
}

void recurs_tria_texpoly(int depth, Flosh *v1, Flosh *v2, Flosh *v3)
{
	
	if(depth) {
		Flosh fl[3];
		
		FloshMid(fl  , v1, v2);
		FloshMid(fl+1, v2, v3);
		FloshMid(fl+2, v3, v1);
		
		recurs_tria_texpoly(depth-1, v1, fl, fl+2);
		recurs_tria_texpoly(depth-1, fl, v2, fl+1);
		recurs_tria_texpoly(depth-1, fl+2, fl+1, v3);
		recurs_tria_texpoly(depth-1, fl, fl+1, fl+2);
		
	}
	else {
		ulong *rect;
		int xs= curpage->ibuf->x;
		int ofs, ys= curpage->ibuf->y;
		short scol[3];
		char *c1, *c2, *c3;
		
		if(curmode & TF_TILES) {
			rect= curpage->reprect +curtile*texwindx*texwindy;
			ofs= texwindx*texwindy;
			
			c1= (char *)(rect+  (((v1->v*ys)>>15)*xs + ((v1->u*xs)>>15) ) % ofs);
			c2= (char *)(rect+  (((v2->v*ys)>>15)*xs + ((v2->u*xs)>>15) ) % ofs);
			c3= (char *)(rect+  (((v3->v*ys)>>15)*xs + ((v3->u*xs)>>15) ) % ofs);
		}
		else {
			rect= curpage->ibuf->rect;
			
			c1= (char *)(rect+  ((v1->v*ys)>>15)*xs + ((v1->u*xs)>>15) );
			c2= (char *)(rect+  ((v2->v*ys)>>15)*xs + ((v2->u*xs)>>15) );
			c3= (char *)(rect+  ((v3->v*ys)>>15)*xs + ((v3->u*xs)>>15) );
		} 

		bgnpolygon();
		
		scol[2]= (c1[1]*v1->col[1])>>7;
		scol[1]= (c1[2]*v1->col[2])>>7;
		scol[0]= (c1[3]*v1->col[3])>>7;
		
		c3s(scol);
		v3f(v1->vec);
		
		scol[2]= (c2[1]*v2->col[1])>>7;
		scol[1]= (c2[2]*v2->col[2])>>7;
		scol[0]= (c2[3]*v2->col[3])>>7;
		
		c3s(scol);
		v3f(v2->vec);
		
		scol[2]= (c3[1]*v3->col[1])>>7;
		scol[1]= (c3[2]*v3->col[2])>>7;
		scol[0]= (c3[3]*v3->col[3])>>7;
		
		c3s(scol);
		v3f(v3->vec);
		
		endpolygon();		
	}
}


void subdivpolygon(TFace *tface, float *v1, float *v2, float *v3, float *v4, float *col)
{
	Flosh fl[4];
	int midx, midy, icol;
	short *uv;
	
	if(curpage) {
		if(v4) {
			VECCOPY(fl[0].vec, v1);
			VECCOPY(fl[1].vec, v2);
			VECCOPY(fl[2].vec, v3);
			VECCOPY(fl[3].vec, v4);
			
			/* *( (int *)(&fl[0].u))= *( (int *)tface->uv[0]); */
			/* *( (int *)(&fl[1].u))= *( (int *)tface->uv[1]); */
			/* *( (int *)(&fl[2].u))= *( (int *)tface->uv[2]); */
			/* *( (int *)(&fl[3].u))= *( (int *)tface->uv[3]); */
			
			uv= tface->uv[0];
			midx= (uv[0]+ uv[2]+ uv[4]+ uv[6])/4;
			midy= (uv[1]+ uv[3]+ uv[5]+ uv[7])/4;
	
			if(uv[0] >= midx) fl[0].u= (uv[0]-64); else fl[0].u= (uv[0]+63);
			if(uv[2] >= midx) fl[1].u= (uv[2]-64); else fl[1].u= (uv[2]+63);
			if(uv[4] >= midx) fl[2].u= (uv[4]-64); else fl[2].u= (uv[4]+63);
			if(uv[6] >= midx) fl[3].u= (uv[6]-64); else fl[3].u= (uv[6]+63);

			if(uv[1] >= midy) fl[0].v= (uv[1]-64); else fl[0].v= (uv[1]+63);
			if(uv[3] >= midy) fl[1].v= (uv[3]-64); else fl[1].v= (uv[3]+63);
			if(uv[5] >= midy) fl[2].v= (uv[5]-64); else fl[2].v= (uv[5]+63);
			if(uv[7] >= midy) fl[3].v= (uv[7]-64); else fl[3].v= (uv[7]+63);

	
			
			if(col) {
				
				icol= 255.0*col[0]; CLAMP(icol, 0, 255);
				fl[0].col[3]= icol;
				icol= 255.0*col[1]; CLAMP(icol, 0, 255);
				fl[0].col[2]= icol;
				icol= 255.0*col[2]; CLAMP(icol, 0, 255);
				fl[0].col[1]= icol;
				
				if(tface->mode & TF_GOUR) {
					col+= 3;
					icol= 255.0*col[0]; CLAMP(icol, 0, 255);
					fl[1].col[3]= icol;
					icol= 255.0*col[1]; CLAMP(icol, 0, 255);
					fl[1].col[2]= icol;
					icol= 255.0*col[2]; CLAMP(icol, 0, 255);
					fl[1].col[1]= icol;

					col+= 3;
					icol= 255.0*col[0]; CLAMP(icol, 0, 255);
					fl[2].col[3]= icol;
					icol= 255.0*col[1]; CLAMP(icol, 0, 255);
					fl[2].col[2]= icol;
					icol= 255.0*col[2]; CLAMP(icol, 0, 255);
					fl[2].col[1]= icol;

					col+= 3;
					icol= 255.0*col[0]; CLAMP(icol, 0, 255);
					fl[3].col[3]= icol;
					icol= 255.0*col[1]; CLAMP(icol, 0, 255);
					fl[3].col[2]= icol;
					icol= 255.0*col[2]; CLAMP(icol, 0, 255);
					fl[3].col[1]= icol;
					
				}
				else {
					COPY_4(fl[1].col, fl[0].col);
					COPY_4(fl[2].col, fl[0].col);
					COPY_4(fl[3].col, fl[0].col);
				}
			}
			else {
				*( (ulong *)fl[0].col)= tface->col[0];
				if(tface->mode & TF_GOUR) {
					*( (ulong *)fl[1].col)= tface->col[1];
					*( (ulong *)fl[2].col)= tface->col[2];
					*( (ulong *)fl[3].col)= tface->col[3];
				}
				else {
					*( (ulong *)fl[1].col)= tface->col[0];
					*( (ulong *)fl[2].col)= tface->col[0];
					*( (ulong *)fl[3].col)= tface->col[0];
				}
			}
			recurs_quad_texpoly(maxsub, fl, fl+1, fl+2, fl+3);
		}
		else if(v3) {

			VECCOPY(fl[0].vec, v1);
			VECCOPY(fl[1].vec, v2);
			VECCOPY(fl[2].vec, v3);
			
			*( (int *)(&fl[0].u))= *( (int *)tface->uv[0]);
			*( (int *)(&fl[1].u))= *( (int *)tface->uv[1]);
			*( (int *)(&fl[2].u))= *( (int *)tface->uv[2]);
			
			if(col) {
				
				icol= 255.0*col[0]; CLAMP(icol, 0, 255);
				fl[0].col[3]= icol;
				icol= 255.0*col[1]; CLAMP(icol, 0, 255);
				fl[0].col[2]= icol;
				icol= 255.0*col[2]; CLAMP(icol, 0, 255);
				fl[0].col[1]= icol;
				

				if(tface->mode & TF_GOUR) {
					col+= 3;
					icol= 255.0*col[0]; CLAMP(icol, 0, 255);
					fl[1].col[3]= icol;
					icol= 255.0*col[1]; CLAMP(icol, 0, 255);
					fl[1].col[2]= icol;
					icol= 255.0*col[2]; CLAMP(icol, 0, 255);
					fl[1].col[1]= icol;

					col+= 3;
					icol= 255.0*col[0]; CLAMP(icol, 0, 255);
					fl[2].col[3]= icol;
					icol= 255.0*col[1]; CLAMP(icol, 0, 255);
					fl[2].col[2]= icol;
					icol= 255.0*col[2]; CLAMP(icol, 0, 255);
					fl[2].col[1]= icol;
				}
				else {
					COPY_4(fl[1].col, fl[0].col);
					COPY_4(fl[2].col, fl[0].col);
				}
				
			}
			else {

				*( (ulong *)fl[0].col)= tface->col[0];
				if(tface->mode & TF_GOUR) {
					*( (ulong *)fl[1].col)= tface->col[1];
					*( (ulong *)fl[2].col)= tface->col[2];
				}
				else {
					*( (ulong *)fl[1].col)= tface->col[0];
					*( (ulong *)fl[2].col)= tface->col[0];
				}
			}
			
			recurs_tria_texpoly(maxsub, fl, fl+1, fl+2);
		}
	}
}

void draw_hide_tfaces(Object *ob, Mesh *me)
{
	TFace *tface;
	MFace *mface;
	float *v1, *v2, *v3, *v4;
	int a;
	
	if(me==0 || me->tface==0) return;

	mface= me->mface;
	tface= me->tface;

	cpack(0x0);
	setlinestyle(1);
	for(a=me->totface; a>0; a--, mface++, tface++) {
		if(mface->v3==0) continue;
		
		if( (tface->flag & TF_HIDE)) {

			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
		
			bgnclosedline();
				v3f( v1 );
				v3f( v2 );
				v3f( v3 );
				if(mface->v4) v3f( v4 );
			endclosedline();			
		}
	}
	setlinestyle(0);
}


void draw_tfaces3D(Object *ob, Mesh *me)
{
	MFace *mface;
	TFace *tface;
	float *v1, *v2, *v3, *v4;
	int a;
	
	if(me==0 || me->tface==0) return;

	zbuffer(0);

	mface= me->mface;
	tface= me->tface;
	
	/* SELECT faces */
	for(a=me->totface; a>0; a--, mface++, tface++) {
		if(mface->v3==0) continue;
		if(tface->flag & ACTIVE) continue;
		if(tface->flag & TF_HIDE) continue;
		
		if(tface->flag & SELECT) {
		
			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
		
			cpack(0x0);
			bgnclosedline();
				v3f( v1 );
				v3f( v2 );
				v3f( v3 );
				if(v4) v3f( v4 );
			endclosedline();

			cpack(0xFFFFFF);
			setlinestyle(1);
			bgnclosedline();
				v3f( v1 );
				v3f( v2 );
				v3f( v3 );
				if(v4) v3f( v4 );
			endclosedline();
			setlinestyle(0);
			
		}
	}
	
	mface= me->mface;
	tface= me->tface;
	
	/* ACTIVE faces */
	for(a=me->totface; a>0; a--, mface++, tface++) {
		if(mface->v3==0) continue;
		
		if( (tface->flag & ACTIVE)  && (tface->flag & SELECT) ) {

			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
		
			/* kleuren: R=x G=y */
			cpack(0xFF);
			bgnline(); v3f(v1); if(v4) v3f(v4); else v3f(v3); endline();
			cpack(0xFF00);
			bgnline(); v3f(v1); v3f(v2); endline();
			cpack(0x0);
			bgnline(); v3f(v2); v3f(v3); if(v4) v3f(v4); endline();
			
			cpack(0xFFFFFF);
			setlinestyle(1);
			bgnclosedline();
				v3f( v1 );
				v3f( v2 );
				v3f( v3 );
				if(mface->v4) v3f( v4 );
			endclosedline();
			setlinestyle(0);
			
		}
	}

	zbuffer(1);
}

float rvecmat[3][3], rcolmat[3][3];
short *punopoin[4];

int do_realtimelight(Object *ob, TFace *tface, float *col)
{
	static Object *lampar[24];
	static int totlamp=0, curlamp=0, need_init=0;
	Lamp *la;
	Life *lf;
	float inp;
	ulong lay;
	int a;
	char *cp;
	
	if(G.vd==0) return 0;
	if(G.vd->localvd) lay= G.vd->localvd->lay;
	else lay= G.vd->lay;
	
	if(ob==0) {
		need_init= 1;
		return 0;
	}
	
	if(ob && need_init) {
		Base *base=FIRSTBASE;

		/* lamp array aanlegen */
		totlamp= 0;
		while(base) {
			if(base->lay & lay) {
				if(base->object->type==OB_LAMP) {
					la= base->object->data;
					if(la->type==LA_SUN) {
						lampar[totlamp]= base->object;
						totlamp++;
						if(totlamp>=24) break;
					}
				}
				else if ELEM(base->object->type, OB_SECTOR, OB_LIFE) {
					/* imat nodig voor lampberekening */
					where_is_object(base->object);
					Mat4Invert(base->object->imat, base->object->obmat);
				}
			}
			base= base->next;
		}
		need_init= 0;
	}
	
	if(totlamp==0) return 0;
	if(G.f & G_VERTEXPAINT) return 0;
		
	if(tface==0) {
		
		/* init matrices */
		curlamp= 0;
		for(a=0; a<totlamp; a++) {
			if(lampar[a]->lay & ob->lay) {
				la= lampar[a]->data;
				
				rvecmat[curlamp][0]= lampar[a]->obmat[2][0];
				rvecmat[curlamp][1]= lampar[a]->obmat[2][1];
				rvecmat[curlamp][2]= lampar[a]->obmat[2][2];

				Mat4Mul3Vecfl(ob->imat, rvecmat[curlamp]);
				
				Normalise(rvecmat[curlamp]);
				
				/* deze schaal wordt terug gecorrigeerd in writeblendpsx */
				rcolmat[curlamp][0]= la->energy*la->r/32767.0;
				rcolmat[curlamp][1]= la->energy*la->g/32767.0;
				rcolmat[curlamp][2]= la->energy*la->b/32767.0;
				
				curlamp++;
				if(curlamp>=3) break;
			}
		}
		return curlamp;
	}
	
	if(curlamp==0) return 0;
	
	/* voorlopig (even) alleen solid */
	bzero(col, 48);
	cp= (char *)tface->col;
	a= curlamp;

	if(tface->mode & TF_SHAREDCOL) {
		while(a--) {
	
			inp= INPR(punopoin[0], rvecmat[a])/255.0;
			if(inp > 0.0) {
				col[0]+= inp*rcolmat[a][0]*cp[3];
				col[1]+= inp*rcolmat[a][1]*cp[2];
				col[2]+= inp*rcolmat[a][2]*cp[1];
			}
			inp= INPR(punopoin[1], rvecmat[a])/255.0;
			if(inp > 0.0) {
				col[3]+= inp*rcolmat[a][0]*cp[7];
				col[4]+= inp*rcolmat[a][1]*cp[6];
				col[5]+= inp*rcolmat[a][2]*cp[5];
			}
			inp= INPR(punopoin[2], rvecmat[a])/255.0;
			if(inp > 0.0) {
				col[6]+= inp*rcolmat[a][0]*cp[11];
				col[7]+= inp*rcolmat[a][1]*cp[10];
				col[8]+= inp*rcolmat[a][2]*cp[9];
			}
			if(punopoin[3]) {
				inp= INPR(punopoin[3], rvecmat[a])/255.0;
				if(inp > 0.0) {
					col[9]+= inp*rcolmat[a][0]*cp[15];
					col[10]+= inp*rcolmat[a][1]*cp[14];
					col[11]+= inp*rcolmat[a][2]*cp[13];
				}
			}

		}
	}
	else {
		while(a--) {
	
			inp= INPR(tface->no, rvecmat[a])/255.0;
			
			if(inp > 0.0) {
				col[0]+= inp*rcolmat[a][0]*cp[3];
				col[1]+= inp*rcolmat[a][1]*cp[2];
				col[2]+= inp*rcolmat[a][2]*cp[1];
				
				if(tface->mode & TF_GOUR) {
					col[3]+= inp*rcolmat[a][0]*cp[7];
					col[4]+= inp*rcolmat[a][1]*cp[6];
					col[5]+= inp*rcolmat[a][2]*cp[5];
	
					col[6]+= inp*rcolmat[a][0]*cp[11];
					col[7]+= inp*rcolmat[a][1]*cp[10];
					col[8]+= inp*rcolmat[a][2]*cp[9];
	
					col[9]+= inp*rcolmat[a][0]*cp[15];
					col[10]+= inp*rcolmat[a][1]*cp[14];
					col[11]+= inp*rcolmat[a][2]*cp[13];
				}
			}
		}
	}
	return 1;
}

void draw_tface_mesh(Object *ob, Mesh *me, int dt)	/* maximum dt: precies volgens ingestelde waardes */
{
	TFace *tface;
	MFace *mface;
	MVert *mvert;
	Image *ima;
	float *v1, *v2, *v3, *v4, col[4][3];
	ulong obcol;
	int a, mode;
	short r, g, b, islight;
	
	if(me==0) return;

	if(ob==OBACT) maxsub= 3;
	else maxsub= 1;
	
	shademodel(GOURAUD);
	gRGBcolor(&r, &g, &b);

	/* als meshes uit lib gelezen zijn en alleen mcol hebben: */
	if(me->tface==0) make_tfaces(me);

	islight= do_realtimelight(ob, 0, 0);

	/* de ob color */
	if(ob->type==OB_SECTOR) {
		Sector *se= ob->data;
		obcol= rgb_to_cpack(se->r, se->g, se->b);
	}
	else if(ob->type==OB_LIFE) {
		Life *lf= ob->data;
		obcol= rgb_to_cpack(lf->r, lf->g, lf->b);
	}
	/* eerst alle texture polys */
	
	backface(TRUE);
	if(G.vd->drawtype==OB_TEXTURE) subdivtex= 0;
	else subdivtex= 1;
	
	if(dt > OB_SOLID) {
		
		mface= me->mface;
		tface= me->tface;
		
		for(a=me->totface; a>0; a--, mface++, tface++) {
			if(mface->v3==0) continue;
			if(tface->flag & TF_HIDE) continue;
			
			mode= tface->mode;
			
			if(mode & TF_OBCOL) tface->col[0]= obcol;
			
			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
			
			if(mode & TF_SHAREDCOL) {
				punopoin[0]= (short *)(v1+3);
				punopoin[1]= (short *)(v2+3);
				punopoin[2]= (short *)(v3+3);
				if(v4) punopoin[3]= (short *)(v4+3);
				else punopoin[3]= 0;
			}

			if(tface->mode & TF_TWOSIDE) backface(FALSE);
			else backface(TRUE);

			if( mode & TF_TEX ) {
								
				/* in set_tpage worden dingen gedaan die niet binnen een bgnpolygon mogen liggen */
				
				if( set_tpage(tface) ) {
					
					if(subdivtex) {
						if(islight && (mode & TF_LIGHT)) {
							do_realtimelight(ob, tface, col[0]);
							subdivpolygon(tface, v1, v2, v3, v4, col[0]);
						}
						else subdivpolygon(tface, v1, v2, v3, v4, 0);
					}
					else {
						
						if(islight && (mode & TF_LIGHT)) {
							do_realtimelight(ob, tface, col[0]);
							
							bgnpolygon();
							
							t2s(tface->uv[0]);
	
							c3f(col[0]);
							v3f(v1);
							
							t2s(tface->uv[1]);
							if(mode & TF_GOUR) c3f(col[1]);
							v3f(v2);
				
							t2s(tface->uv[2]);
							if(mode & TF_GOUR) c3f(col[2]);
							v3f(v3);
				
							if(v4) {
								t2s(tface->uv[3]);
								if(mode & TF_GOUR) c3f(col[3]);
								v3f(v4);
							}
							endpolygon();
						}
						else {
							bgnpolygon();
							
							t2s(tface->uv[0]);
	
							spack(tface->col[0]);
							v3f(v1);
							
							t2s(tface->uv[1]);
							if(mode & TF_GOUR) spack(tface->col[1]);
							v3f(v2);
				
							t2s(tface->uv[2]);
							if(mode & TF_GOUR) spack(tface->col[2]);
							v3f(v3);
				
							if(v4) {
								t2s(tface->uv[3]);
								if(mode & TF_GOUR) spack(tface->col[3]);
								v3f(v4);
							}
							endpolygon();
						}
					}
				}
				else {
					/* waarschuwings polygoon */
					bgnpolygon();
					cpack(0xFF00FF);
					v3f(v1);
					v3f(v2);
					v3f(v3);
					if(v4) v3f(v4);
					endpolygon();
				}
			}
			else {
				
				set_tpage(0);
				
				bgnpolygon();
				
				if(islight && (tface->mode & TF_LIGHT)) {
					do_realtimelight(ob, tface, col[0]);
					
					c3f(col[0]);
					v3f(v1);
					if(mode & TF_GOUR) {
						c3f(col[1]);
						v3f(v2);
						c3f(col[2]);
						v3f(v3);
						if(v4) {c3f(col[3]); v3f(v4);}
					}
					else {
						v3f(v2);
						v3f(v3);
						if(v4) v3f(v4);
					}
				}
				else {

					cpack(tface->col[0]);
					v3f(v1);
					
					if(mode & TF_GOUR) {
						cpack(tface->col[1]);
						v3f(v2);
						cpack(tface->col[2]);
						v3f(v3);
						if(v4) {
							cpack(tface->col[3]);
							v3f(v4);
						}
					}
					else {
						v3f(v2);
						v3f(v3);
						if(v4) v3f(v4);
					}
					
				}
				endpolygon();
			}
		}
		
		/* textures uitzetten */
		set_tpage(0);

	}
	else {
		
		/* alle niet-texture polys */
		
		mface= me->mface;
		tface= me->tface;
	
		for(a=me->totface; a>0; a--, mface++, tface++) {
			if(mface->v3==0) continue;
			if(tface->flag & TF_HIDE) continue;
			
			mode= tface->mode;
			
			if(tface->mode & TF_TWOSIDE) backface(FALSE);
			else backface(TRUE);

			if(mode & TF_OBCOL) tface->col[0]= obcol;

			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
			
			if(mode & TF_SHAREDCOL) {
				punopoin[0]= (short *)(v1+3);
				punopoin[1]= (short *)(v2+3);
				punopoin[2]= (short *)(v3+3);
				if(v4) punopoin[3]= (short *)(v4+3);
				else punopoin[3]= 0;
			}
			
			bgnpolygon();
			
			if(islight && (tface->mode & TF_LIGHT)) {
				do_realtimelight(ob, tface, col[0]);
				
				c3f(col[0]);
				v3f(v1);
				if(mode & TF_GOUR) {
					c3f(col[1]);
					v3f(v2);
					c3f(col[2]);
					v3f(v3);
					if(v4) {c3f(col[3]); v3f(v4);}
				}
				else {
					v3f(v2);
					v3f(v3);
					if(v4) v3f(v4);
				}
			}
			else {
				if(mode & TF_GOUR) {
				
					cpack(tface->col[0]);
					v3f(v1);
					cpack(tface->col[1]);
					v3f(v2);
					cpack(tface->col[2]);
					v3f(v3);
					if(v4) {
						cpack(tface->col[3]);
						v3f(v4);
					}
				}
				else {	/* FLAT */
					
					cpack(tface->col[0]);
					v3f(v1);
					v3f(v2);
					v3f(v3);
					if(v4) v3f(v4);
				}
			}
			endpolygon();
		}
	}
	
	shademodel(FLAT);
	backface(FALSE);
	
	draw_hide_tfaces(ob, me);

	/* alles dat met editing te maken heeft: */
	
	if(ob==OBACT && (G.f & G_FACESELECT)) {
		draw_tfaces3D(ob, me);
	}
	RGBcolor(r, g, b);
}



void drawsector(Object *ob, int dt, ulong col)		/* col: restore van ghost */
{	
	Object *obedit;
	Sector *se;
	Portal *po;
	int a, flag;
	
	se= ob->data;
	
	if(se->flag & SE_GHOST_OTHER) {
		if( (G.f & G_BACKBUFSEL)==0) {
			flag= se->flag;
			obedit= G.obedit;
			G.obedit= 0;
		
			if(se->flag & SE_SHOW_TEXMESH) {
				se->flag &= ~SE_SHOW_TEXMESH;
				setlinestyle(5);
				drawcube_size(se->size);
				setlinestyle(0);
			}
			else {
				se->flag |= SE_SHOW_TEXMESH;
			}
			cpack(0x505050);
			drawmeshwire(ob);
			
			se->flag= flag;
			G.obedit= obedit;
			cpack(col);
		}
	}

	if(se->flag & SE_SHOW_TEXMESH) {

		if(ob==G.obedit) drawmeshwire(ob);
		else if(dt==OB_BOUNDBOX) drawboundbox(ob);
		else if(dt==OB_WIRE) drawmeshwire(ob);
		else {
			
			if((G.f & (G_PICKSEL))) drawmeshwire(ob);
			else if((G.f & (G_BACKBUFSEL))) drawmeshsolid(ob, 0);
			else draw_tface_mesh(ob, se->texmesh, dt);
		}
	}
	else {

		if(G.f & G_SIMULATION);
		else if(ob!=G.obedit) {
			
			setlinestyle(5);
			drawcube_size(se->size);
			setlinestyle(0);
			
			if(ob->dt>OB_WIRE) {
				
				po= se->portals;
				if(ob==OBACT) {
					for(a=1; a<=se->totport; a++) {
						draw_portal(se, po, a==se->actport);
						po++;
					}
				}
				else {
					a= se->totport;
					while(a--) {
						draw_portal(se, po, 0);
						po++;
					}
				}
			}
		}

		if(dt>OB_SOLID) dt= OB_SOLID;	/* shaded bij dynamesh: werkt niet */
		
		if(ob==G.obedit) drawmeshwire(ob);
		else if(dt==OB_BOUNDBOX) drawboundbox(ob);
		else if(dt==OB_WIRE) drawmeshwire(ob);
		else drawDispList(ob, dt);
	}
	

}


