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

/* previewrender.c  		GRAPHICS

 * 
 * maart 95
 * 
 * Version: $Id: previewrender.c,v 1.3 2000/09/21 13:46:23 nzc Exp $
 */

/* global includes */
#include "blender.h"
#include "graphics.h"
#include "image_ext.h"

/* local includes */
#include "render.h"
#include "rendercore.h"
#include "texture.h"
#include "initrender.h"
#include "matrixops.h"

/* own includes */
#include "previewrender.h"


#define PR_RECTX	101
#define PR_RECTY	101
#define PR_XMIN		10
#define PR_YMIN		10
#define PR_XMAX		190
#define PR_YMAX		190

#define PR_FACY		(PR_YMAX-PR_YMIN-4)/(PR_RECTY)

#define SCANL	1280

uint pr_scan[SCANL];

rcti prerect;
int pr_sizex, pr_sizey;
float pr_facx, pr_facy;


short snijpunt(float *v1, float *v2, float *v3, float *rtlabda, float *ray1, float *ray2)
{
	float x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	float m0,m1,m2,deeldet,det1,det2,det3;
	float rtu, rtv;
	
	t00= v3[0]-v1[0];
	t01= v3[1]-v1[1];
	t02= v3[2]-v1[2];
	t10= v3[0]-v2[0];
	t11= v3[1]-v2[1];
	t12= v3[2]-v2[2];
	t20= ray1[0]-ray2[0];
	t21= ray1[1]-ray2[1];
	t22= ray1[2]-ray2[2];
	
	x0= t11*t22-t12*t21;
	x1= t12*t20-t10*t22;
	x2= t10*t21-t11*t20;

	deeldet= t00*x0+t01*x1+t02*x2;
	if(deeldet!=0.0) {
		m0= ray1[0]-v3[0];
		m1= ray1[1]-v3[1];
		m2= ray1[2]-v3[2];
		det1= m0*x0+m1*x1+m2*x2;
		rtu= det1/deeldet;
		if(rtu<=0.0) {
			det2= t00*(m1*t22-m2*t21);
			det2+= t01*(m2*t20-m0*t22);
			det2+= t02*(m0*t21-m1*t20);
			rtv= det2/deeldet;
			if(rtv<=0.0) {
				if(rtu+rtv>= -1.0) {
					
					det3=  m0*(t12*t01-t11*t02);
					det3+= m1*(t10*t02-t12*t00);
					det3+= m2*(t11*t00-t10*t01);
					*rtlabda= det3/deeldet;
					
					if(*rtlabda>=0.0 && *rtlabda<=1.0) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

float rcubev[7][3]= {
	{-0.002055,  6.627364, -3.369742}, 
	{-6.031684, -3.750204, -1.992980}, 
	{-6.049086,  3.817431,  1.969788}, 
	{ 6.031685,  3.833064,  1.992979}, 
	{ 6.049086, -3.734571, -1.969787}, 
	{ 0.002054, -6.544502,  3.369744}, 
	{-0.015348,  1.023131,  7.332510} };

int rcubi[3][4]= {
	{3,  6,  5,  4},
	{1,  5,  6,  2},  
	{3,  0,  2,  6} };


int ray_previewrender(int x, int y, float *vec)
{
	float scalef= 12.8/100.0;
	float ray1[3], ray2[3];
	float minlabda, labda;
	int totface= 3, hitface= -1;
	int a;

	ray1[0]= ray2[0]= x*scalef;
	ray1[1]= ray2[1]= y*scalef;
	ray1[2]= -10.0;
	ray2[2]= 10.0;
	
	minlabda= 1.0;
	for(a=0; a<totface; a++) {
		if(snijpunt( rcubev[rcubi[a][0]], rcubev[rcubi[a][1]], rcubev[rcubi[a][2]], &labda, ray1, ray2)) {
			if( labda < minlabda) {
				minlabda= labda;
				hitface= a;
			}
		}
		if(snijpunt( rcubev[rcubi[a][0]], rcubev[rcubi[a][2]], rcubev[rcubi[a][3]], &labda, ray1, ray2)) {
			if( labda < minlabda) {
				minlabda= labda;
				hitface= a;
			}
		}
	}
	
	if(hitface > -1) {
		
		CalcNormFloat(rcubev[rcubi[hitface][0]], rcubev[rcubi[hitface][1]], rcubev[rcubi[hitface][2]], R.vn);
		
		vec[0]= (minlabda*(ray1[0]-ray2[0])+ray2[0])/3.7;
		vec[1]= (minlabda*(ray1[1]-ray2[1])+ray2[1])/3.7;
		vec[2]= (minlabda*(ray1[2]-ray2[2])+ray2[2])/3.7;
		
		return 1;
	}
	return 0;
}


uint RE_previewback(int type, int x, int y)
{
	if(type & MA_DARK) {
		if(abs(x)>abs(y)) return 0;
		else return 0x40404040;
	}
	else {
		if(abs(x)>abs(y)) return 0x40404040;
		else return 0xa0a0a0a0;
	}
}

void set_previewrect(int xmin, int ymin, int xmax, int ymax)
{
	/* coordinaten omzetten naar absolute windowco's */
	float x, y;
	
	x= xmin; y= ymin;
	view2d_to_window(&x, &y);

	prerect.xmin= ffloor(x);;
	prerect.ymin= ffloor(y);
	
	x= xmax; y= ymax;
	view2d_to_window(&x, &y);

	prerect.xmax= fceil(x);
	prerect.ymax= fceil(y);
	
	pr_sizex= (prerect.xmax-prerect.xmin);
	pr_sizey= (prerect.ymax-prerect.ymin);

	pr_facx= ( (float)pr_sizex-1)/PR_RECTX;
	pr_facy= ( (float)pr_sizey-1)/PR_RECTY;
}

void display_pr_scanline(uint *rect, int recty)
{
	/* we display 3 new scanlines, one old */

	if(recty % 2) return;
	if(recty<2) return;
	
	rect+= (recty-2)*PR_RECTX;

	/* iets meer uitvergroten in y om GL/mesa bugje te verhelpen */
	glPixelZoom(pr_facx, pr_facy);

	glRasterPos2f( (float)PR_XMIN+0.5, 1.0+(float)PR_YMIN + (recty*PR_FACY) );
	glDrawPixels(PR_RECTX, 3, GL_RGBA, GL_UNSIGNED_BYTE,  rect);

	glPixelZoom(1.0, 1.0);
}

void previewdraw()
{
	int y;
	
	init_view2d_calc();
	
	set_previewrect(PR_XMIN, PR_YMIN, PR_XMAX, PR_YMAX);

	if(G.buts->rect==0) {
		RE_preview_changed(curarea->win);
		return;
	}
	if(G.buts->cury==0) {
		RE_preview_changed(curarea->win);
		return;
	}

	for(y=0; y<PR_RECTY; y++) display_pr_scanline(G.buts->rect, y);

	if(G.buts->mainb==BUTS_TEX) {
		draw_tex_crop(G.buts->lockpoin);
	}
}

void previewdrawo()
{
	init_view2d_calc();
	
	set_previewrect(PR_XMIN, PR_YMIN, PR_XMAX, PR_YMAX);

	if(G.buts->rect==0) {
		RE_preview_changed(curarea->win);
		return;
	}
	if(G.buts->cury==0) {
		RE_preview_changed(curarea->win);
		return;
	}

	/* Winsetting is for shifting images */	
	winset(G.curscreen->mainwin);
	rectwrite_part(curarea->winrct.xmin, curarea->winrct.ymin, curarea->winrct.xmax, curarea->winrct.ymax,
		prerect.xmin, prerect.ymin, PR_RECTX, PR_RECTY, pr_facx, pr_facx, G.buts->rect);
	winset(curarea->win);

	if(G.buts->mainb==BUTS_TEX) {
		draw_tex_crop(G.buts->lockpoin);
	}
}


void RE_preview_changed(short win)
{
	G.buts->cury= 0;
	addafterqueue(win, RENDERPREVIEW, 1);
}

void draw_tex_crop(Tex *tex)
{
	rcti rct;
	int ret= 0;
	
	if(tex==0) return;
	
	if(tex->type==TEX_IMAGE) {
		if(tex->cropxmin==0.0) ret++;
		if(tex->cropymin==0.0) ret++;
		if(tex->cropxmax==1.0) ret++;
		if(tex->cropymax==1.0) ret++;
		if(ret==4) return;
		
		rct.xmin= PR_XMIN+2+tex->cropxmin*(PR_XMAX-PR_XMIN-4);
		rct.xmax= PR_XMIN+2+tex->cropxmax*(PR_XMAX-PR_XMIN-4);
		rct.ymin= PR_YMIN+2+tex->cropymin*(PR_YMAX-PR_YMIN-4);
		rct.ymax= PR_YMIN+2+tex->cropymax*(PR_YMAX-PR_YMIN-4);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

		cpack(0x0);
		glRecti(rct.xmin+1,  rct.ymin-1,  rct.xmax+1,  rct.ymax-1); 

		cpack(0xFFFFFF);
		glRecti(rct.xmin,  rct.ymin,  rct.xmax,  rct.ymax);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			
	}
	
}


void sky_preview_pixel(float lens, int x, int y, char *rect)
{
	
	if(R.wrld.skytype & WO_SKYPAPER) {
		R.view[0]= (2*x)/(float)PR_RECTX;
		R.view[1]= (2*y)/(float)PR_RECTY;
		R.view[2]= 0.0;
	}
	else {
		R.view[0]= x;
		R.view[1]= y;
		R.view[2]= -lens*PR_RECTX/32.0;
		Normalise(R.view);
	}
	sky(rect);
}

void lamp_preview_pixel(LampRen *la, int x, int y, char *rect)
{
	float inpr, i, t, dist, distkw, vec[3];
	int col;
	
	R.co[0]= (float)x/(PR_RECTX/4);
	R.co[1]= (float)y/(PR_RECTX/4);
	R.co[2]= 0;
	
	vec[0]= 0.02*x;
	vec[1]= 0.02*y;
	vec[2]= 0.005*PR_RECTX;
	VECCOPY(R.view, vec);
	dist= Normalise(R.view);

	if(la->mode & LA_TEXTURE) do_lamp_tex(la, vec);

	if(la->type==LA_SUN || la->type==LA_HEMI) {
		dist= 1.0;
	}
	else {
		
		if(la->mode & LA_QUAD) {
			
			t= 1.0;
			if(la->ld1>0.0)
				t= la->dist/(la->dist+la->ld1*dist);
			if(la->ld2>0.0) {
				distkw= la->dist*la->dist;
				t= t*distkw/(t*distkw+la->ld2*dist*dist);
			}
			dist= t;
		}
		else {
			dist= (la->dist/(la->dist+dist));
		}
	}

	if(la->type==LA_SPOT) {

		
		if(la->mode & LA_SQUARE) {
			/* slightly smaller... */
			inpr= 1.7*fcos(MAX2(fabs(R.view[0]/R.view[2]) , fabs(R.view[1]/R.view[2]) ));
		}
		else {
			inpr= R.view[2];
		}
		
		t= la->spotsi;
		if(inpr<t) dist= 0.0;
		else {
			t= inpr-t;
			if(t<la->spotbl && la->spotbl!=0.0) {
				/* zachte gebied */
				i= t/la->spotbl;
				t= i*i;
				i= t*i;
				inpr*=(3.0*t-2.0*i);
			}
		}
		dist*=inpr;
	}
	else if(la->type==LA_LOCAL) dist*= R.view[2];
	
	col= 255.0*dist*la->r;
	if(col<=0) rect[0]= 0; else if(col>=255) rect[0]= 255; else rect[0]= col;

	col= 255.0*dist*la->g;
	if(col<=0) rect[1]= 0; else if(col>=255) rect[1]= 255; else rect[1]= col;

	col= 255.0*dist*la->b;
	if(col<=0) rect[2]= 0; else if(col>=255) rect[2]= 255; else rect[2]= col;
}

void init_previewhalo(HaloRen *har, Material *mat)
{
	
	har->type= 0;
	if(mat->mode & MA_HALO_XALPHA) har->type |= HA_XALPHA;
	har->mat= mat;
	har->hard= mat->har;
	har->rad= PR_RECTX/2.0;
	har->radsq= PR_RECTX*PR_RECTX/4.0;
	har->alfa= mat->alpha;
	har->add= 255.0*mat->add;
	har->r= 255.0*mat->r;
	har->g= 255.0*mat->g; 
	har->b= 255.0*mat->b;
	har->xs= PR_RECTX/2.0;
	har->ys= PR_RECTX/2.0;
	har->zs= har->zd= 0;
	har->seed= (mat->seed1 % 256);
	
	if( (mat->mode & MA_HALOTEX) && mat->mtex[0] ) har->tex= 1; else har->tex=0;

	if(mat->mode & MA_STAR) har->starpoints= mat->starc; else har->starpoints= 0;
	if(mat->mode & MA_HALO_LINES) har->linec= mat->linec; else har->linec= 0;
	if(mat->mode & MA_HALO_RINGS) har->ringc= mat->ringc; else har->ringc= 0;
	if(mat->mode & MA_HALO_FLARE) har->flarec= mat->flarec; else har->flarec= 0;
	
	if(har->flarec) {
		har->xs-= PR_RECTX/3;
		har->ys+= PR_RECTX/3;
		
		har->rad*= 0.3;
		har->radsq= har->rad*har->rad;
		
		har->pixels= har->rad*har->rad*har->rad;
	}
}	

void halo_preview_pixel(HaloRen *har, int startx, int endx, int y, char *rect)
{
	float dist, xn, yn, xsq, ysq;
	int x;
	char front[4];
	
	if(har->flarec) yn= y-PR_RECTX/3;
	else yn= y;
	ysq= yn*yn;
	
	for(x=startx; x<endx; x++) {
		
		if(har->flarec) xn= x+PR_RECTX/3;
		else xn= x;
		
		xsq= xn*xn;
		dist= xsq+ysq;

		
		
		if(dist<har->radsq) {
			shadehalo(har, front, 0, dist, xn, yn, har->flarec);
			addalphaAddfac(rect, front, har->add);
		}
		rect+= 4;
	}
}

void previewflare(HaloRen *har, uint *rect)
{
	float ycor;
	uint *rectot;
	int afmx, afmy, rectx, recty;
	
	/* temps */
	ycor= R.ycor;
	rectx= R.rectx;
	recty= R.recty;
	afmx= R.afmx;
	afmy= R.afmy;
	rectot= R.rectot;

	R.ycor= 1.0;
	R.rectx= PR_RECTX;	
	R.recty= PR_RECTY;
	R.afmx= PR_RECTX/2;
	R.afmy= PR_RECTY/2;
	R.rectot= rect;

	waitcursor(1);

	renderflare(har);

	previewdraw();
	
	waitcursor(0);
	
	/* temps */
	R.ycor= ycor;
	R.rectx= rectx;
	R.recty= recty;
	R.afmx= afmx;
	R.afmy= afmy;
	R.rectot= rectot;
}

void texture_preview_pixel(Tex *tex, int x, int y, char *rect)
{
	extern float Tin, Tr, Tg, Tb, Ta;
	float i, v1, xsq, ysq, texvec[3];
	int rgbnor, tracol, skip=0;
		
	if(tex->type==TEX_IMAGE) {
		v1= 1.0/PR_RECTX;
		
		texvec[0]= 0.5+v1*x;
		texvec[1]= 0.5+v1*y;
		
		/* geen coordmapping, uitzondering: repeat */
		if(tex->xrepeat>1) {
			texvec[0] *= tex->xrepeat;
			if(texvec[0]>1.0) texvec[0] -= (int)(texvec[0]);
		}
		if(tex->yrepeat>1) {
			texvec[1] *= tex->yrepeat;
			if(texvec[1]>1.0) texvec[1] -= (int)(texvec[1]);
		}

	}
	else if(tex->type==TEX_ENVMAP) {
		if(tex->env) {
			ysq= y*y;
			xsq= x*x;
			if(xsq+ysq < (PR_RECTX/2)*(PR_RECTY/2)) {
				texvec[2]= fsqrt( (float)((PR_RECTX/2)*(PR_RECTY/2)-xsq-ysq) );
				texvec[0]= -x;
				texvec[1]= -y;
				Normalise(texvec);

				i= 2.0*(texvec[2]);
				texvec[0]= (i*texvec[0]);
				texvec[1]= (i*texvec[1]);
				texvec[2]= (-1.0+i*texvec[2]);

			}
			else {
				skip= 1;
				Ta= 0.0;
			}
		}
		else {
			skip= 1;
			Ta= 0.0;
		}
	}
	else {
		v1= 2.0/PR_RECTX;
	
		texvec[0]= v1*x;
		texvec[1]= v1*y;
		texvec[2]= 0.0;
	}
	
	/* geeft geen Tin terug */
	if(tex->type==TEX_STUCCI) {
		tex->nor= R.vn;
		R.vn[0]= 1.0;
		R.vn[1]= R.vn[2]= 0.0;
	}
	
	if(skip==0) rgbnor= multitex(tex, texvec, 0, 0);
	else rgbnor= 1;
	
	if(rgbnor & 1) {
		
		rect[0]= 255.0*Tr;
		rect[1]= 255.0*Tg;
		rect[2]= 255.0*Tb;
		
		if(Ta!=1.0) {
			tracol=  64+100*(abs(x)>abs(y));
			tracol= (1.0-Ta)*tracol;
			
			rect[0]= tracol+ (rect[0]*Ta) ;
			rect[1]= tracol+ (rect[1]*Ta) ;
			rect[2]= tracol+ (rect[2]*Ta) ;
					
		}
	}
	else {
	
		if(tex->type==TEX_STUCCI) {
			Tin= 0.5 + 0.7*tex->nor[0];
			CLAMP(Tin, 0.0, 1.0);
		}
		rect[0]= 255.0*Tin;
		rect[1]= 255.0*Tin;
		rect[2]= 255.0*Tin;
	}
}

float pr1_lamp[3]= {2.3, -2.4, -4.6};
float pr2_lamp[3]= {-8.8, -5.6, -1.5};
float pr1_col[3]= {0.8, 0.8, 0.8};
float pr2_col[3]= {0.5, 0.6, 0.7};

void shade_preview_pixel(float *vec, int x, int y, char *rect, int smooth)
{
	Material *mat;
	float v1,inp, inprspec=0, isr=0.0, isb=0.0, isg=0.0;
	float ir=0.0, ib=0.0, ig=0.0;
	float view[3], lv[3], *la, alpha;
	float eul[3], tmat[3][3], imat[3][3];
	int temp, a;
	char tracol;
	
	mat= R.matren;
	/* pr1_lamp[0]= mat->mtex[0]->ofs[0]; */
	/* pr1_lamp[1]= mat->mtex[0]->ofs[1]; */
	/* pr1_lamp[2]= mat->mtex[0]->ofs[2]; */

	/* pr2_lamp[0]= mat->mtex[0]->size[0]; */
	/* pr2_lamp[1]= mat->mtex[0]->size[1]; */
	/* pr2_lamp[2]= mat->mtex[0]->size[2]; */

	v1= 1.0/PR_RECTX;
	view[0]= v1*x;
	view[1]= v1*y;
	view[2]= 1.0;
	Normalise(view);
	
	R.refcol[0]= R.refcol[1]= R.refcol[2]= R.refcol[3]= 0.0;

	/* texture afhandeling */
	if(mat->texco) {
		
		VECCOPY(R.lo, vec);
		
		if(mat->pr_type==MA_CUBE) {
			
			eul[0]= (297)*M_PI/180.0;
			eul[1]= 0.0;
			eul[2]= (45)*M_PI/180.0;
			EulToMat3(eul, tmat);

			MTC_Mat3MulVecfl(tmat, R.lo);
			MTC_Mat3MulVecfl(tmat, R.vn);
			/* hack for cubemap, why!!! */
			SWAP(float, R.vn[0], R.vn[1]);
		}

		if(mat->texco & TEXCO_GLOB) {
			VECCOPY(R.gl, R.lo);
		}
		if(mat->texco & TEXCO_WINDOW) {
			VECCOPY(R.winco, R.lo);
		}
		if(mat->texco & TEXCO_STICKY) {
			VECCOPY(R.sticky, R.lo);
		}
		if(mat->texco & TEXCO_UV) {
			VECCOPY(R.uv, R.lo);
		}
		if(mat->texco & TEXCO_OBJECT) {
			VECCOPY(R.co, R.lo);
		}
		if(mat->texco & TEXCO_NORM) {
			R.orn[0]= R.vn[0];
			R.orn[1]= -R.vn[1];
			R.orn[2]= R.vn[2];
		}
		if(mat->texco & TEXCO_REFL) {
			/* for bump texture */
			VECCOPY(R.view, view);
			
			inp= -2.0*(R.vn[0]*view[0]+R.vn[1]*view[1]+R.vn[2]*view[2]);
			R.ref[0]= (view[0]+inp*R.vn[0]);
			R.ref[1]= -(view[1]+inp*R.vn[1]);
			R.ref[2]= (view[2]+inp*R.vn[2]);
		}

		do_material_tex();

		if(mat->pr_type==MA_CUBE) {
			/* rotate normal back for normals texture */
			SWAP(float, R.vn[0], R.vn[1]);
			MTC_Mat3Inv(imat, tmat);
			MTC_Mat3MulVecfl(imat, R.vn);
		}
		
	}
	
	if(mat->mode & MA_SHLESS) {
		temp= 255.0*(mat->r);
		if(temp>255) rect[0]= 255; else if(temp<0) rect[0]= 0; else rect[0]= temp;

		temp= 255.0*(mat->g);
		if(temp>255) rect[1]= 255; else if(temp<0) rect[1]= 0; else rect[1]= temp;

		temp= 255.0*(mat->b);
		if(temp>255) rect[2]= 255; else if(temp<0) rect[2]= 0; else rect[2]= temp;
	}
	else {
		
		for(a=0; a<2; a++) {
			
			if(a==0) la= pr1_lamp;
			else la= pr2_lamp;
			
			lv[0]= vec[0]-la[0];
			lv[1]= vec[1]-la[1];
			lv[2]= vec[2]-la[2];
			Normalise(lv);
			
			inp= R.vn[0]*lv[0]+R.vn[1]*lv[1]+R.vn[2]*lv[2];
			if(inp<0.0) inp= 0.0;
			
			if(mat->spec)  {
				
				lv[0]+= view[0];
				lv[1]+= view[1];
				lv[2]+= view[2];
				Normalise(lv);
				
				if(inp>0.0) {
					v1= lv[0]*R.vn[0]+lv[1]*R.vn[1]+lv[2]*R.vn[2];
					if(v1>0.0) {
						v1= RE_Spec(v1, mat->har);
						inprspec= v1*mat->spec;
						isr+= inprspec*mat->specr;
						isg+= inprspec*mat->specg;
						isb+= inprspec*mat->specb;
					}
				}
			}
			inp= (mat->ref*inp + mat->emit);
			
			if(a==0) la= pr1_col;
			else la= pr2_col;

			ir+= inp*la[0];
			ig+= inp*la[1];
			ib+= inp*la[2];
		}

		if(R.refcol[0]==0.0) {
			a= 255.0*( mat->r*ir +mat->ambr +isr);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[0]= a;
			a= 255.0*(mat->g*ig +mat->ambg +isg);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[1]= a;
			a= 255*(mat->b*ib +mat->ambb +isb);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[2]= a;
		}
		else {
			a= 255.0*( mat->mirr*R.refcol[1] + (1.0 - mat->mirr*R.refcol[0])*(mat->r*ir +mat->ambr) +isr);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[0]= a;
			a= 255.0*( mat->mirg*R.refcol[2] + (1.0 - mat->mirg*R.refcol[0])*(mat->g*ig +mat->ambg) +isg);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[1]= a;
			a= 255.0*( mat->mirb*R.refcol[3] + (1.0 - mat->mirb*R.refcol[0])*(mat->b*ib +mat->ambb) +isb);
			if(a>255) a=255; else if(a<0) a= 0;
			rect[2]= a;
		}
	}

	if(mat->alpha!=1.0) {
		
		alpha= mat->alpha;
		
			/* ztra shade */
		if(mat->spectra!=0.0) {
			inp= mat->spectra*inprspec;
			if(inp>1.0) inp= 1.0;
			
			alpha= (1.0-inp)*alpha+ inp;
		}

		tracol=  RE_previewback(mat->pr_back, x, y) & 255;
		
		tracol= (1.0-alpha)*tracol;
		
		rect[0]= tracol+ (rect[0]*alpha) ;
		rect[1]= tracol+ (rect[1]*alpha) ;
		rect[2]= tracol+ (rect[2]*alpha) ;

	}
}


void RE_previewrender()
{
	Material *mat=0;
	Tex *tex = NULL;
	Image *ima;
	Lamp *la;
	LampRen *lar = NULL;
	HaloRen har;
	Object *ob;
	World *wrld;
	float lens = 0.0, vec[3];
	int x, y, starty, startx, endy, endx, radsq, xsq, ysq, last = 0;
	uint *rect;
	
	if(G.vd==0) return;	/* kan voorkomen als er geen 3D win geopend is */ 
	if(G.buts->cury>=PR_RECTY) return;
	
	if ELEM4(G.buts->mainb, BUTS_MAT, BUTS_TEX, BUTS_LAMP, BUTS_WORLD);
	else return;
	
	har.flarec= 0;	/* verderop test op postrender flare */
	
	usleep(2);
	if(qtest()) {
		addafterqueue(curarea->win, RENDERPREVIEW, 1);
		return;
	}

	MTC_Mat4One(R.viewmat);
	MTC_Mat4One(R.viewinv);
	
	R.osatex= 0;
	if(G.buts->mainb==BUTS_MAT) {
		mat= G.buts->lockpoin;
		if(mat==0) return;

		/* rendervars */
		init_render_world();
		init_render_material(mat);
		
		/* imats wissen */
		for(x=0; x<8; x++) {
			if(mat->mtex[x]) {
				if(mat->mtex[x]->tex) {
					init_render_texture(mat->mtex[x]->tex);
					
					if(mat->mtex[x]->tex->env && mat->mtex[x]->tex->env->object) 
						MTC_Mat4One(mat->mtex[x]->tex->env->object->imat);
				}
				if(mat->mtex[x]->object) MTC_Mat4One(mat->mtex[x]->object->imat);
				if(mat->mtex[x]->object) MTC_Mat4One(mat->mtex[x]->object->imat);
			}
		}
		R.vlr= 0;
		R.mat= mat;
		R.matren= mat->ren;
		
		if(mat->mode & MA_HALO) init_previewhalo(&har, mat);
	}
	else if(G.buts->mainb==BUTS_TEX) {
		tex= G.buts->lockpoin;
		if(tex==0) return;
		ima= tex->ima;
		if(ima) last= ima->lastframe;
		init_render_texture(tex);
		free_unused_animimages();
		if(tex->ima) {
			if(tex->ima!=ima) allqueue(REDRAWBUTSTEX, 0);
			else if(last!=ima->lastframe) allqueue(REDRAWBUTSTEX, 0);
		}
		if(tex->env && tex->env->object) 
			MTC_Mat4Invert(tex->env->object->imat, tex->env->object->obmat);
	}
	else if(G.buts->mainb==BUTS_LAMP) {
		ob= OBACT;
		if(ob==0 || ob->type!=OB_LAMP) return;
		la= ob->data;
		init_render_world();
		init_render_textures();	/* ze mogen niet twee keer!! (brightness) */
		R.totlamp= 0;
		add_render_lamp(ob, 0);	/* 0=no shadbuf */
		lar= R.la[0];
		
		/* uitzonderingen: */
		lar->spottexfac= 1.0;
		lar->spotsi= fcos( M_PI/3.0 );
		lar->spotbl= (1.0-lar->spotsi)*la->spotblend;
		
		MTC_Mat3One(lar->imat);
	}
	else {
		wrld= G.buts->lockpoin;
		if(wrld==0) return;
		
		lens= 35.0;
		if(G.scene->camera) {
			lens= ( (Camera *)G.scene->camera->data)->lens;
		}
		MTC_Mat4CpyMat4(R.viewmat, G.vd->viewmat);
		MTC_Mat4CpyMat4(R.viewinv, G.vd->viewinv);
		init_render_world();
		init_render_textures();	/* ze mogen niet twee keer!! (brightness) */
	}

	init_view2d_calc();
	set_previewrect(PR_XMIN, PR_YMIN, PR_XMAX, PR_YMAX);

	if(G.buts->rect==0) {
		G.buts->rect= callocN(sizeof(int)*PR_RECTX*PR_RECTY, "butsrect");
		
		/* built in emboss */
		rect= G.buts->rect;
		for(y=0; y<PR_RECTY; y++, rect++) *rect= 0xFFFFFFFF;
		
		rect= G.buts->rect + PR_RECTX-1;
		for(y=0; y<PR_RECTY; y++, rect+=PR_RECTX) *rect= 0xFFFFFFFF;
	}
	
	starty= -PR_RECTY/2;
	endy= starty+PR_RECTY;
	starty+= G.buts->cury;
	
	/* offset +1 for emboss */
	startx= -PR_RECTX/2 +1;
	endx= startx+PR_RECTX -2;

	radsq= (PR_RECTX/2)*(PR_RECTY/2);
	
	glDrawBuffer(GL_FRONT);
	
	if(mat) {
		if(mat->pr_type==MA_SPHERE) {
			pr1_lamp[0]= 2.3; pr1_lamp[1]= -2.4; pr1_lamp[2]= -4.6;
			pr2_lamp[0]= -8.8; pr2_lamp[1]= -5.6; pr2_lamp[2]= -1.5;
		}
		else {
			pr1_lamp[0]= 1.9; pr1_lamp[1]= 3.1; pr1_lamp[2]= -8.5;
			pr2_lamp[0]= 1.2; pr2_lamp[1]= -18; pr2_lamp[2]= 3.2;
		}
	}
	
	for(y=starty; y<endy; y++) {
		
		rect= G.buts->rect + 1 + PR_RECTX*G.buts->cury;
		
		if(y== -PR_RECTY/2 || y==endy-1);		/* emboss */
		else if(G.buts->mainb==BUTS_MAT) {
			
			if(mat->mode & MA_HALO) {
				for(x=startx; x<endx; x++, rect++) {
					rect[0]= RE_previewback(mat->pr_back, x, y);
				}

				if(har.flarec) {
					if(y==endy-2) previewflare(&har, G.buts->rect);
				}
				else {
					halo_preview_pixel(&har, startx, endx, y, (char *) (rect-PR_RECTX));
				}
			}
			else {
				ysq= y*y;
				for(x=startx; x<endx; x++, rect++) {
					xsq= x*x;
					if(mat->pr_type==MA_SPHERE) {
					
						if(xsq+ysq < radsq) {
							R.vn[0]= x;
							R.vn[1]= y;
							R.vn[2]= fsqrt( (float)(radsq-xsq-ysq) );
							Normalise(R.vn);
							
							vec[0]= R.vn[0];
							vec[1]= R.vn[2];
							vec[2]= -R.vn[1];
							
							shade_preview_pixel(vec, x, y, (char *)rect, 1);
						}
						else {
							rect[0]= RE_previewback(mat->pr_back, x, y);
						}
					}
					else if(mat->pr_type==MA_CUBE) {
						if( ray_previewrender(x, y, vec) ) {
							
							shade_preview_pixel(vec, x, y, (char *)rect, 0);
						}
						else {
							rect[0]= RE_previewback(mat->pr_back, x, y);
						}
					}
					else {
						vec[0]= x*(2.0/PR_RECTX);
						vec[1]= y*(2.0/PR_RECTX);
						vec[2]= 0.0;
						
						R.vn[0]= R.vn[1]= 0.0;
						R.vn[2]= 1.0;
						
						shade_preview_pixel(vec, x, y, (char *)rect, 0);
					}
				}
			}
		}
		else if(G.buts->mainb==BUTS_TEX) {
			for(x=startx; x<endx; x++, rect++) {
				texture_preview_pixel(tex, x, y, (char *)rect);
			}
		}
		else if(G.buts->mainb==BUTS_LAMP) {
			for(x=startx; x<endx; x++, rect++) {
				lamp_preview_pixel(lar, x, y, (char *)rect);
			}
		}
		else  {
			for(x=startx; x<endx; x++, rect++) {				
				sky_preview_pixel(lens, x, y, (char *)rect);
			}
		}
		
		if(y<endy-2) {

			if(qtest()) {
				addafterqueue(curarea->win, RENDERPREVIEW, 1);
				break;
			}
		}

		display_pr_scanline(G.buts->rect, G.buts->cury);
		
		G.buts->cury++;
	}

	if(G.buts->cury>=PR_RECTY && G.buts->mainb==BUTS_TEX) 
		draw_tex_crop(G.buts->lockpoin);
	
	glDrawBuffer(GL_BACK);
	#if defined(__WIN32) || defined(MESA31)
	previewdraw();
	#else 
	curarea->win_swap= WIN_FRONT_OK;
	#endif
	
	if(G.buts->mainb==BUTS_MAT) {
		end_render_material(mat);
		for(x=0; x<8; x++) {
			if(mat->mtex[x] && mat->mtex[x]->tex) end_render_texture(mat->mtex[x]->tex);
		}	
	}
	else if(G.buts->mainb==BUTS_TEX) {
		end_render_texture(tex);
	}
	else if(G.buts->mainb==BUTS_WORLD) {
		end_render_textures();
	}
	else if(G.buts->mainb==BUTS_LAMP) {
		if(R.totlamp) {
			if(R.la[0]->org) freeN(R.la[0]->org);
			freeN(R.la[0]);
		}
		R.totlamp= 0;
		end_render_textures();
	}
}

