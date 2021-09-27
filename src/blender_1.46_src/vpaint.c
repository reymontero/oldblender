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




/*  vpaint.c      GRAPHICS
 * 
 *  jan 96
 *  
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "sector.h"
#include "edit.h"

	/* Gvp.mode */
#define VP_MIX	0
#define VP_ADD	1
#define VP_SUB	2
#define VP_MUL	3
#define VP_FILT	4

	/* Gvp.flag */
#define VP_COLINDEX	1
#define VP_AREA		2
#define VP_SOFT		4
#define VP_NORMALS	8

#define MAXINDEX	4096

VPaint Gvp= {0.0, 0.5, 1.0, 0.2, 25.0, 1.0, 1.0, 0, VP_AREA+VP_SOFT};
float vpimat[3][3];
ulong *vpaintundobuf=0;
int totvpaintundo;
short *indexar= 0;

void do_shared_vertexcol(Mesh *me)
{
	/* als geen mcol: niet doen */
	/* als tface: alleen de betreffende vlakken, anders alles */
	MFace *mface;
	TFace *tface;
	MVert *mvert;
	int a;
	short *scolmain, *scol;
	char *mcol;
	
	if(me->mcol==0 || me->totvert==0 || me->totface==0) return;
	
	scolmain= callocN(4*2*me->totvert, "colmain");
	
	tface= me->tface;
	mface= me->mface;
	mcol= (char *)me->mcol;
	for(a=me->totface; a>0; a--, mface++, mcol+=16) {
		if(mface->v3) {
			if(tface==0 || (tface->mode & TF_SHAREDCOL)) {
				
				scol= scolmain+4*mface->v1;
				scol[0]++; scol[1]+= mcol[1]; scol[2]+= mcol[2]; scol[3]+= mcol[3];
				scol= scolmain+4*mface->v2;
				scol[0]++; scol[1]+= mcol[5]; scol[2]+= mcol[6]; scol[3]+= mcol[7];
				scol= scolmain+4*mface->v3;
				scol[0]++; scol[1]+= mcol[9]; scol[2]+= mcol[10]; scol[3]+= mcol[11];
				if(mface->v4) {
					scol= scolmain+4*mface->v4;
					scol[0]++; scol[1]+= mcol[13]; scol[2]+= mcol[14]; scol[3]+= mcol[15];
				}
			}
		}
		if(tface) tface++;
	}
	
	a= me->totvert;
	scol= scolmain;
	while(a--) {
		if(scol[0]>1) {
			scol[1]/= scol[0];
			scol[2]/= scol[0];
			scol[3]/= scol[0];
		}
		scol+= 4;
	}
	
	tface= me->tface;
	mface= me->mface;
	mcol= (char *)me->mcol;
	for(a=me->totface; a>0; a--, mface++, mcol+=16) {
		if(mface->v3) {
			if(tface==0 || (tface->mode & TF_SHAREDCOL)) {
				
				scol= scolmain+4*mface->v1;
				mcol[1]= scol[1]; mcol[2]= scol[2]; mcol[3]= scol[3];
				
				scol= scolmain+4*mface->v2;
				mcol[5]= scol[1]; mcol[6]= scol[2]; mcol[7]= scol[3];
				
				scol= scolmain+4*mface->v3;
				mcol[9]= scol[1]; mcol[10]= scol[2]; mcol[11]= scol[3];
				
				if(mface->v4) {
					scol= scolmain+4*mface->v4;
					mcol[13]= scol[1]; mcol[14]= scol[2]; mcol[15]= scol[3];
				}
			}
		}
		if(tface) tface++;
	}

	freeN(scolmain);
}

void tface_to_mcol(Mesh *me)
{
	TFace *tface;
	ulong *mcol;
	int a;
	
	me->mcol= mallocN(4*4*me->totface, "nepmcol");
	mcol= (ulong *)me->mcol;
	
	a= me->totface;
	tface= me->tface;
	while(a--) {
		memcpy(mcol, tface->col, 16);
		mcol+= 4;
		tface++;
	}
}

void mcol_to_tface(Mesh *me, int freedata)
{
	TFace *tface;
	ulong *mcol;
	int a;
	
	a= me->totface;
	tface= me->tface;
	mcol= (ulong *)me->mcol;
	while(a--) {
		memcpy(tface->col, mcol, 16);
		mcol+= 4;
		tface++;
	}
	
	if(freedata) {
		freeN(me->mcol);
		me->mcol= 0;
	}
}


void copy_vpaint_undo(ulong *mcol, int tot)
{
	if(vpaintundobuf) freeN(vpaintundobuf);
	vpaintundobuf= 0;
	
	if(mcol==0 || tot==0) return;
	
	vpaintundobuf= mallocN(4*4*tot, "vpaintundobuf");
	memcpy(vpaintundobuf, mcol, 4*4*tot);
	totvpaintundo= tot;
	
}

void vpaint_undo()
{
	Mesh *me;
	Object *ob;
	ulong temp, *from, *to;
	int a;
	

	if((G.f & G_VERTEXPAINT)==0) return;
	if(vpaintundobuf==0) return;

	ob= OBACT;
	me= get_mesh(ob);
	if(me==0 || me->totface==0) return;

	if(me->tface) tface_to_mcol(me);
	else if(me->mcol==0) return;
	
	a= MIN2(me->totface, totvpaintundo);
	from= vpaintundobuf;
	to= (ulong *)me->mcol;
	a*= 4;
	while(a--) {
		temp= *to;
		*to= *from;
		*from= temp;
		to++; from++;
	}
	allqueue(REDRAWVIEW3D, 0);
	if(me->tface) mcol_to_tface(me, 1);
}

void clear_vpaint()
{
	Mesh *me;
	Object *ob;
	ulong *to, paintcol;
	int a;
	
	if((G.f & G_VERTEXPAINT)==0) return;

	ob= OBACT;
	me= get_mesh(ob);
	if(me==0 || me->totface==0) return;

	if(me->tface) tface_to_mcol(me);
	if(me->mcol==0) return;

	paintcol= rgb_to_cpack(Gvp.r, Gvp.g, Gvp.b);
	
	to= (ulong *)me->mcol;
	copy_vpaint_undo(to, me->totface);
	a= 4*me->totface;
	while(a--) {
		*to= paintcol;
		to++; 
	}
	allqueue(REDRAWVIEW3D, 0);
	if(me->tface) mcol_to_tface(me, 1);
}

void vpaint_dogamma()
{
	Mesh *me;
	Object *ob;
	float igam, fac;
	int a, temp;
	char *cp, gamtab[256];

	if((G.f & G_VERTEXPAINT)==0) return;

	ob= OBACT;
	me= get_mesh(ob);
	if(me==0 || me->totface==0) return;

	if(me->tface) tface_to_mcol(me);
	else if(me->mcol==0) return;

	copy_vpaint_undo((ulong *)me->mcol, me->totface);

	igam= 1.0/Gvp.gamma;
	for(a=0; a<256; a++) {
		
		fac= ((float)a)/255.0;
		fac= Gvp.mul*powf( fac, igam);
		
		temp= 255.9*fac;
		
		if(temp<=0) gamtab[a]= 0;
		else if(temp>=255) gamtab[a]= 255;
		else gamtab[a]= temp;
	}

	a= 4*me->totface;
	cp= (char *)me->mcol;
	while(a--) {
		
		cp[1]= gamtab[ cp[1] ];
		cp[2]= gamtab[ cp[2] ];
		cp[3]= gamtab[ cp[3] ];
		
		cp+= 4;
	}
	allqueue(REDRAWVIEW3D, 0);
	
	if(me->tface) mcol_to_tface(me, 1);
}


void sample_vpaint()	/* frontbuf */
{
	ulong col, col1;
	int x, y;
	short mval[2];
	
	getmouseco_areawin(mval);
	x= mval[0]; y= mval[1];
	
	if(x<0 || y<0) return;
	if(x>=curarea->winx || y>=curarea->winy) return;
	
	x+= curarea->winrct.xmin;
	y+= curarea->winrct.ymin;
	
	/* lrectread(x, y, x, y, &col); */
	
	/* col1= col & 0xF0F0F0; */
	
	/* if(col1) { */
		
		readsource(SRC_FRONT);
		lrectread(x, y, x, y, &col);
		readsource(SRC_AUTO);

		cpack_to_rgb(col, &Gvp.r, &Gvp.g, &Gvp.b);
		allqueue(REDRAWBUTSGAME, 0);
	/* } */
}

void init_vertexpaint()
{
	
	indexar= mallocN(2*MAXINDEX, "vertexpaint");
}


void free_vertexpaint()
{
	
	if(indexar) freeN(indexar);
	indexar= 0;
	if(vpaintundobuf) freeN(vpaintundobuf);
	vpaintundobuf= 0;
}


ulong cpack_blend(ulong col1, ulong col2, int fac)
{
	char *cp1, *cp2, *cp;
	int mfac;
	ulong col=0;
	
	if(fac==0) return col1;
	if(fac>=255) return col2;

	mfac= 255-fac;
	
	cp1= (char *)&col1;
	cp2= (char *)&col2;
	cp=  (char *)&col;
	
	cp[1]= (mfac*cp1[1]+fac*cp2[1])>>8;
	cp[2]= (mfac*cp1[2]+fac*cp2[2])>>8;
	cp[3]= (mfac*cp1[3]+fac*cp2[3])>>8;
	
	return col;
}

ulong cpack_add(ulong col1, ulong col2, int fac)
{
	char *cp1, *cp2, *cp;
	int temp;
	ulong col=0;
	
	if(fac==0) return col1;
	
	cp1= (char *)&col1;
	cp2= (char *)&col2;
	cp=  (char *)&col;
	
	temp= cp1[1] + ((fac*cp2[1])>>8);
	if(temp>254) cp[1]= 255; else cp[1]= temp;
	temp= cp1[2] + ((fac*cp2[2])>>8);
	if(temp>254) cp[2]= 255; else cp[2]= temp;
	temp= cp1[3] + ((fac*cp2[3])>>8);
	if(temp>254) cp[3]= 255; else cp[3]= temp;
	
	return col;
}

ulong cpack_sub(ulong col1, ulong col2, int fac)
{
	char *cp1, *cp2, *cp;
	int temp;
	ulong col=0;
	
	if(fac==0) return col1;
	
	cp1= (char *)&col1;
	cp2= (char *)&col2;
	cp=  (char *)&col;
	
	temp= cp1[1] - ((fac*cp2[1])>>8);
	if(temp<0) cp[1]= 0; else cp[1]= temp;
	temp= cp1[2] - ((fac*cp2[2])>>8);
	if(temp<0) cp[2]= 0; else cp[2]= temp;
	temp= cp1[3] - ((fac*cp2[3])>>8);
	if(temp<0) cp[3]= 0; else cp[3]= temp;
	
	return col;
}

ulong cpack_mul(ulong col1, ulong col2, int fac)
{
	char *cp1, *cp2, *cp;
	int mfac;
	ulong col=0;
	
	if(fac==0) return col1;

	mfac= 255-fac;
	
	cp1= (char *)&col1;
	cp2= (char *)&col2;
	cp=  (char *)&col;
	
	/* eerstmullen, dan fac blenden */
	cp[1]= (mfac*cp1[1] + fac*((cp2[1]*cp1[1])>>8)  )>>8;
	cp[2]= (mfac*cp1[2] + fac*((cp2[2]*cp1[2])>>8)  )>>8;
	cp[3]= (mfac*cp1[3] + fac*((cp2[3]*cp1[3])>>8)  )>>8;

	
	return col;
}


void vpaint_blend( ulong *col, ulong paintcol, int alpha)
{
	
	if(Gvp.mode==VP_MIX || Gvp.mode==VP_FILT) *col= cpack_blend( *col, paintcol, alpha);
	else if(Gvp.mode==VP_ADD) *col= cpack_add( *col, paintcol, alpha);
	else if(Gvp.mode==VP_SUB) *col= cpack_sub( *col, paintcol, alpha);
	else if(Gvp.mode==VP_MUL) *col= cpack_mul( *col, paintcol, alpha);
}


int sample_backbuf_area(int x, int y)
{
	ulong rect[129*129], *rt, last=0;
	int x1, y1, x2, y2, size, a, tot=0, index;
	
	if(Gvp.size>64.0) Gvp.size= 64.0;
	
	x1= x-Gvp.size;
	x2= x+Gvp.size;
	CLAMP(x1, 0, curarea->winx);
	CLAMP(x2, 0, curarea->winx);
	y1= y-Gvp.size;
	y2= y+Gvp.size;
	CLAMP(y1, 0, curarea->winy);
	CLAMP(y2, 0, curarea->winy);
	
	lrectread(x1+curarea->winrct.xmin, y1+curarea->winrct.ymin, x2+curarea->winrct.xmin, y2+curarea->winrct.ymin, rect);
	
	rt= rect;
	size= (y2-y1)*(x2-x1);
	if(size<=0) return 0;

	bzero(indexar, 2*totvpaintundo);
	
	while(size--) {

		*rt &= 0xF0F0F0;
		if(*rt && *rt != last) {
			last= *rt;
			
			index= ( (last & 0xF00000) >>12) + ((last & 0xF000)>>8) + ((last & 0xF0)>>4);
			
			indexar[index] = 1;
		}
	
		rt++;
	}
	
	for(a=1; a<=totvpaintundo; a++) {
		if(indexar[a]) indexar[tot++]= a;
	}
	
	return tot;
}

ulong sample_backbuf(int x, int y)
{
	ulong col;
	
	if(x>=curarea->winx || y>=curarea->winy) return 0;
	
	x+= curarea->winrct.xmin;
	y+= curarea->winrct.ymin;
	
	lrectread(x, y, x, y, &col);

	col &= 0xF0F0F0;

	return ( (col & 0xF00000) >>12) + ((col & 0xF000)>>8) + ((col & 0xF0)>>4) ;
	
}

int calc_vp_alpha(MVert *mvert, short *mval)
{
	float fac, dx, dy, nor[3];
	int alpha;
	short vertco[2];
	
		
	if(Gvp.flag & VP_SOFT) {
		project_short_noclip(mvert->co , vertco);
		dx= mval[0]-vertco[0];
		dy= mval[1]-vertco[1];
		
		fac= fsqrt(dx*dx + dy*dy);
		if(fac > Gvp.size) return 0;
	
		alpha= 255.0*Gvp.a*(1.0-fac/Gvp.size);
	}
	else {
		alpha= 255.0*Gvp.a;
	}
	if(Gvp.flag & VP_NORMALS) {
		VECCOPY(nor, mvert->no);
		
		/* transpose ! */
		fac= vpimat[2][0]*nor[0]+vpimat[2][1]*nor[1]+vpimat[2][2]*nor[2];
		if(fac>0.0) {
			dx= vpimat[0][0]*nor[0]+vpimat[0][1]*nor[1]+vpimat[0][2]*nor[2];
			dy= vpimat[1][0]*nor[0]+vpimat[1][1]*nor[1]+vpimat[1][2]*nor[2];
			
			alpha*= fac/fsqrt(dx*dx + dy*dy + fac*fac);
		}
		else return 0;
	}
	
	return alpha;

}


void vertex_paint()
{
	Object *ob;
	Mesh *me;
	DispList *dl;
	MVert *mvert;
	MFace *mface;
	TFace *tface;
	float mat[4][4], imat[4][4];
	ulong paintcol=0, *mcol, fcol1, fcol2;
	int index, alpha, totindex, total;
	short mval[2], mvalo[2];
	
	if((G.f & G_VERTEXPAINT)==0) return;
	if(G.obedit) return;
	
	if(indexar==0) init_vertexpaint();
	
	ob= OBACT;
	me= get_mesh(ob);
	if(me==0 || me->totface==0) return;
	if(ob->lay & G.vd->lay); else error("Active object not in this layer!");

	if(me->tface) tface_to_mcol(me);
	if(me->mcol==0) return;
	
	/* imat voor normalen */
	Mat4MulMat4(mat, ob->obmat, G.vd->viewmat);
	Mat4Invert(imat, mat);
	Mat3CpyMat4(vpimat, imat);
	
	/* projektiematrix laden */
	multmatrix(ob->obmat);
	mygetsingmatrix(mat);
	loadmatrix(G.vd->viewmat);
	
	paintcol= rgb_to_cpack(Gvp.r, Gvp.g, Gvp.b);
	
	getmouseco_areawin(mvalo);
	mvalo[0]-= 1;
	
	copy_vpaint_undo( (ulong *)me->mcol, me->totface);
	
	while( getbutton(LEFTMOUSE) ) {
		getmouseco_areawin(mval);
		
		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
		
			draw_sel_circle(mval, mvalo, Gvp.size, Gvp.size, 0);
			
			/* welke vlakken doen mee */
			if(Gvp.flag & VP_AREA) {
				totindex= sample_backbuf_area(mval[0], mval[1]);
			}
			else {
				indexar[0]= sample_backbuf(mval[0], mval[1]);
				if(indexar[0]) totindex= 1;
				else totindex= 0;
			}
			
			Mat4SwapMat4(G.vd->persmat, mat);
			
			if(Gvp.flag & VP_COLINDEX) {
				for(index=0; index<totindex; index++) {
					if(indexar[index] && indexar[index]<=me->totface) {
					
						mface= ((MFace *)me->mface) + (indexar[index]-1);
					
						if(mface->mat_nr!=ob->actcol-1) {
							indexar[index]= 0;
						}
					}					
				}
			}
			if((G.f & G_FACESELECT) && me->tface) {
				for(index=0; index<totindex; index++) {
					if(indexar[index] && indexar[index]<=me->totface) {
					
						tface= ((TFace *)me->tface) + (indexar[index]-1);
					
						if((tface->flag & SELECT)==0) {
							indexar[index]= 0;
						}
					}					
				}
			}
			
			for(index=0; index<totindex; index++) {

				if(indexar[index] && indexar[index]<=me->totface) {
				
					mface= ((MFace *)me->mface) + (indexar[index]-1);
					mcol= ( (ulong *)me->mcol) + 4*(indexar[index]-1);

					if(Gvp.mode==VP_FILT) {
						fcol1= cpack_blend( mcol[0], mcol[1], 128);
						if(mface->v4) {
							fcol2= cpack_blend( mcol[2], mcol[3], 128);
							paintcol= cpack_blend( fcol1, fcol2, 128);
						}
						else {
							paintcol= cpack_blend( mcol[2], fcol1, 170);
						}
						
					}
					
					total= 0;
					
					total+= alpha= calc_vp_alpha(me->mvert+mface->v1, mval);
					if(alpha) vpaint_blend( mcol, paintcol, alpha);
					
					total+= alpha= calc_vp_alpha(me->mvert+mface->v2, mval);
					if(alpha) vpaint_blend( mcol+1, paintcol, alpha);
	
					total+= alpha= calc_vp_alpha(me->mvert+mface->v3, mval);
					if(alpha) vpaint_blend( mcol+2, paintcol, alpha);

					if(mface->v4) {
						total+= alpha= calc_vp_alpha(me->mvert+mface->v4, mval);
						if(alpha) vpaint_blend( mcol+3, paintcol, alpha);
					}
					
					/* if(total==0) { */
					/* 	alpha= 25*Gvp.a; */
					/* 	vpaint_blend( mcol, paintcol, alpha); */
					/* 	vpaint_blend( mcol+1, paintcol, alpha); */
					/* 	vpaint_blend( mcol+2, paintcol, alpha); */
					/* 	if(mface->v4) vpaint_blend( mcol+3, paintcol, alpha); */
					/* } */
				}
			}
			
			Mat4SwapMat4(G.vd->persmat, mat);
			
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];
			
		}
		else sginap(1);
		
		if(me->tface) {
			do_shared_vertexcol(me);
			mcol_to_tface(me, 0);
		}
		curarea->windraw();
		screen_swapbuffers();
		backdrawview3d(0);
		
	}
	
	if(me->tface) {
		freeN(me->mcol);
		me->mcol= 0;
	}
	
	/* cirkel wissen */
	draw_sel_circle(0, mvalo, 0, Gvp.size, 1);
	
	allqueue(REDRAWVIEW3D, 0);
}


void set_vpaint()		/* toggle */
{		
	Object *ob;
	Mesh *me;
	char str[64];
	
	addqueue(curarea->headwin, REDRAW, 1);
	ob= OBACT;
	me= get_mesh(ob);
	if(me==0) return;
	if(me->totface>=MAXINDEX) {
		sprintf(str, "Maximum number of faces: %d", MAXINDEX-1);
		error(str);
		G.f &= ~G_VERTEXPAINT;
		return;
	}

	if(G.f & G_VERTEXPAINT) G.f &= ~G_VERTEXPAINT;
	else G.f |= G_VERTEXPAINT;
	
	allqueue(REDRAWVIEW3D, 0);
	allqueue(REDRAWBUTSGAME, 0);
	
	if(G.f & G_VERTEXPAINT) {
		setcursor_space(SPACE_VIEW3D, 3);
	}
	else {
		freefastshade();	/* voor zekerheid */
		shadeDispList(ob);
		if((G.f & G_FACESELECT)==0) setcursor_space(SPACE_VIEW3D, 0);
	}
}

void set_faceselect()	/* toggle */
{
	Object *ob;
	Mesh *me;
	
	addqueue(curarea->headwin, REDRAW, 1);
	ob= OBACT;
	me= get_mesh(ob);
	if(me==0) return;
	
	if(G.f & G_FACESELECT) G.f &= ~G_FACESELECT;
	else G.f |= G_FACESELECT;

	allqueue(REDRAWVIEW3D, 0);
	allqueue(REDRAWBUTSGAME, 0);
	allqueue(REDRAWIMAGE, 0);
	
	if(G.f & G_FACESELECT) {
		setcursor_space(SPACE_VIEW3D, 4);
		if(ob->type==OB_SECTOR) {
			Sector *se= ob->data;
			se->flag|= SE_SHOW_TEXMESH;
		}
		set_lasttface();
	}
	else if((G.f & G_VERTEXPAINT)==0) {
		reveal_tface();
		setcursor_space(SPACE_VIEW3D, 0);
	}
	countall();
}

