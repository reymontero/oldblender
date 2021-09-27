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

/*  displist.c

 * 
 *  okt 92
 */

#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <string.h>
#include "/usr/people/include/Trace.h"


struct FastLamp *flar[MAXLAMP];
extern struct ListBase editNurb;
struct ListBase *displistbase;
float fviewvec[3];
long totflamp=0;

void initfastshade()
{
	struct Base *base;
	struct LaData *la;
	struct FastLamp *fl;
	struct View *v;
	float bmat[4][4], omat[3][3];
	long a;

	if(totflamp!=0) return;

	initrendertexture();

	for(a=0; a<totflamp; a++) {
		freeN(flar[a]);
		flar[a]=0;
	}
	totflamp= 0;

	VecSubf(fviewvec, view0.obs, view0.tar);
	Normalise(fviewvec);

	if(G.localview) gettempview(&v);
	else v= &view0;

	base= G.firstbase;
	while(base) {
		if( base->soort== 2 && (base->lay & v->lay)) {
			parentbase(base, bmat, omat);
			la= (struct LaData *)base->d;
			fl= mallocN(sizeof(struct FastLamp), "initfastshade2");
			flar[totflamp++]= fl;

			fl->soort= la->soort;
			fl->f= la->f;
			fl->lay= base->lay;
			fl->vec[0]= -bmat[2][0];
			fl->vec[1]= -bmat[2][1];
			fl->vec[2]= -bmat[2][2];
			Normalise(fl->vec);
			fl->co[0]= bmat[3][0];
			fl->co[1]= bmat[3][1];
			fl->co[2]= bmat[3][2];

			fl->dist= la->ld;
			fl->distkw= fl->dist*fl->dist;
			fl->ld1= ((float)la->ld1)/255.0;
			fl->ld2= ((float)la->ld2)/255.0;

			fl->spotsi= fcos( PI*((float)(256- la->spsi))/512.0 );
			fl->spotbl= 1.0 -fcos( PI*((float)(la->spbl))/512.0 );
			fl->r= la->energy*((float)la->r)/255.0;
			fl->g= la->energy*((float)la->g)/255.0;
			fl->b= la->energy*((float)la->b)/255.0;
		}
		base= base->next;
	}

	if(totflamp==0) {
		fl= mallocN(sizeof(struct FastLamp), "initfastshade");
		flar[0]= fl;
		totflamp= 1;

		fl->soort= 2;
		fl->vec[0]= fl->vec[1]= 0.0;
		fl->vec[2]= -1.0;
		fl->r= fl->g= fl->b= 1.0;

	}
}


void freefastshade()
{
	long a;

	for(a=0; a<totflamp; a++) {
		freeN(flar[a]);
		flar[a]=0;
	}
	totflamp= 0;
}


void fastshade(co, vec, orco, colb, col1, col2)
float *co, *vec;
float *orco;
struct ColBlck *colb;
char *col1, *col2;
{
	struct FastLamp *fl;
	float i, t, inp, inpr, inpg, inpb, isr=0, isg=0, isb=0, lv[3], lampdist, Spec();
	float inpr1, inpg1, inpb1, isr1=0, isg1=0, isb1=0;
	long a, back;
	char col[3];

	inpr= inpg= inpb= inpr1= inpg1= inpb1= ((float)colb->emit)/255.0;

	col[0]= colb->r;
	col[1]= colb->g;
	col[2]= colb->b;

	if(orco && colb->tex[0]) externtexcol(colb->tex[0], orco, col);

	if(colb->mode & 16) {
		col1[3]= col[0];
		col1[2]= col[1];
		col1[1]= col[2];
		col2[3]= col[0];
		col2[2]= col[1];
		col2[1]= col[2];
		return;
	}

	for(a=0; a<totflamp; a++) {
		fl= flar[a];

		if(fl->f & 4) if((fl->lay & colb->lay)==0) continue;

		if(fl->soort>=2) {	/* ook hemi */
			VECCOPY(lv, fl->vec);
			lampdist= 1.0;
		}
		else {
			lv[0]= co[0]- fl->co[0];
			lv[1]= co[1]- fl->co[1];
			lv[2]= co[2]- fl->co[2];
			lampdist= fsqrt(lv[0]*lv[0]+lv[1]*lv[1]+lv[2]*lv[2]);
			lv[0]/=lampdist;
			lv[1]/=lampdist;
			lv[2]/=lampdist;

			if(fl->f & 8) {	/* quadlamp */
				t= 1.0;
				if(fl->ld1>0.0)
					t= fl->dist/(fl->dist+fl->ld1*lampdist);
				if(fl->ld2>0.0)
					t*= fl->distkw/(fl->distkw+fl->ld2*lampdist*lampdist);
				lampdist= t;
			}
			else {
				lampdist= (fl->dist/(fl->dist+lampdist));
			}
		}

		if(fl->soort==1) {
			inp= lv[0]*fl->vec[0]+lv[1]*fl->vec[1]+lv[2]*fl->vec[2];
			if(inp<fl->spotsi) continue;
			else {
				t= fl->spotsi;
				t= inp-t;
				if(t<fl->spotbl && fl->spotbl!=0) {
					/* zachte gebied */
					i= t/fl->spotbl;
					t= i*i;
					i= t*i;
					inp*=(3.0*t-2.0*i);
				}
				lampdist*=inp;
			}
		}

		inp= vec[0]*lv[0]+ vec[1]*lv[1]+ vec[2]*lv[2];

		back= 0;
		if(inp<0.0) {
			back= 1;
			inp= -inp;
		}
		inp*= lampdist*((float)colb->kref)/255.0;

		if(back==0) {
			inpr+= inp*fl->r;
			inpg+= inp*fl->g;
			inpb+= inp*fl->b;
		} else {
			inpr1+= inp*fl->r;
			inpg1+= inp*fl->g;
			inpb1+= inp*fl->b;
		}
		if(colb->kspec) {
			lv[0]+= fviewvec[0];
			lv[1]+= fviewvec[1];
			lv[2]+= fviewvec[2];
			Normalise(lv);
			t= vec[0]*lv[0]+vec[1]*lv[1]+vec[2]*lv[2];
			if(t>0) {
				t= colb->kspec*lampdist*Spec(t,colb->spec);
				if(back==0) {
					isr+= t*(fl->r * colb->specr);
					isg+= t*(fl->g * colb->specg);
					isb+= t*(fl->b * colb->specb);
				}
				else {
					isr1+= t*(fl->r * colb->specr);
					isg1+= t*(fl->g * colb->specg);
					isb1+= t*(fl->b * colb->specb);
				}
			}
		}

	}
	isr/=256;
	isg/=256;
	isb/=256;
	isr1/=256;
	isg1/=256;
	isb1/=256;


	a= inpr*col[0] +colb->ambr +isr;
	if(a>255) col1[3]= 255; 
	else col1[3]= a;
	a= inpg*col[1] +colb->ambg +isg;
	if(a>255) col1[2]= 255; 
	else col1[2]= a;
	a= inpb*col[2] +colb->ambb +isb;
	if(a>255) col1[1]= 255; 
	else col1[1]= a;

	a= inpr1*col[0] +colb->ambr +isr1;
	if(a>255) col2[3]= 255; 
	else col2[3]= a;
	a= inpg1*col[1] +colb->ambg +isg1;
	if(a>255) col2[2]= 255; 
	else col2[2]= a;
	a= inpb1*col[2] +colb->ambb +isb1;
	if(a>255) col2[1]= 255; 
	else col2[1]= a;

}


void shadeDispList(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VlakOb *adrvl;
	struct VertOb *adrve;
	struct DispList *dl;
	struct dl_GourFace *dlgf;
	struct ColBlck *col;
	float *data, *fp, *vdata, *ndata, n1[3], *v1, *v2, *v3, *v4, *v5, *v6, *v7, *v8;
	float inp;
	float bmat[4][4], omat[3][3];
	long a, b, p1, p2, p3, p4;

	initfastshade();
	
	dl= 0;
	parentbase(base, bmat, omat);
	ob= (struct ObData *)(base->d);
	if(base->soort==1) {
		Mat4MulFloat3(bmat, ob->vv->ws);
		dl= ob->disp.first;
	}
	else if ELEM(base->soort, 5, 11) {
		Mat4MulFloat3(bmat, ob->cu->ws);
		dl= ob->cu->disp.first;
	}
	if(dl==0) return;
	col= (struct ColBlck *)(ob+1);
	for(a=0; a<ob->c; a++) {
		col->lay= base->lay;
		b= (col->kref*col->amb)>>8;
		col->ambr= (b*wrld.ambr)>>8;
		col->ambg= (b*wrld.ambg)>>8;
		col->ambb= (b*wrld.ambb)>>8;
		col++;
	}
	/* normalen en kleur */

	while(dl) {

		col= (struct ColBlck *)(ob+1);
		col+= dl->col;

		if(dl->type==DL_TRIA_GOUR) {
			vv= ob->vv;
			dlgf= (struct dl_GourFace *)(dl+1);
			a= dl->parts;

			adrve= (struct VertOb *)(vv+1);
			adrvl= (struct VlakOb *)(adrve+vv->vert);

			/* gebruik omat voor geroteerde vertices */

			while(a--) {
				VECCOPY(omat[0], dlgf->co[0]);
				VECCOPY(omat[1], dlgf->co[1]);
				VECCOPY(omat[2], dlgf->co[2]);
				Mat4MulVecfl(bmat, omat[0]);
				Mat4MulVecfl(bmat, omat[1]);
				Mat4MulVecfl(bmat, omat[2]);
				CalcNormFloat(omat[2], omat[1], omat[0], n1);
				
				b= (adrvl->ec & 31);
				fastshade(omat[0], n1, dlgf->co[0], col+b, dlgf->col, dlgf->col+1);
				fastshade(omat[1], n1, dlgf->co[1], col+b, dlgf->col+2, dlgf->col+3);
				fastshade(omat[2], n1, dlgf->co[2], col+b, dlgf->col+4, dlgf->col+5);

				adrvl++;
				dlgf++;
			}
		}
		else if(dl->type==DL_SURF_GOUR || dl->type==DL_SURF_SOLID) {

			if(col->mode & 1) dl->type= DL_SURF_GOUR;
			else dl->type= DL_SURF_SOLID;

			data= (float *)(dl+1);

			vdata= mallocN(dl->nr*dl->parts*12, "shadeDispList1");  /* vert */
			ndata= callocN(dl->nr*dl->parts*12, "shadeDispList2");  /* norm */

			/* roteer */
			fp= vdata;
			v1= data;
			a= dl->parts*dl->nr;
			while(a--) {
				VECCOPY(fp, v1);
				Mat4MulVecfl(bmat, fp);

				fp+= 3; 
				v1+= 5;
			}

			for(a=0; a<dl->parts; a++) {

				DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);

				v1= vdata+ 3*p1; 
				v5= ndata+ 3*p1;
				v2= vdata+ 3*p2; 
				v6= ndata+ 3*p2;
				v3= vdata+ 3*p3; 
				v7= ndata+ 3*p3;
				v4= vdata+ 3*p4; 
				v8= ndata+ 3*p4;
				for(; b<dl->nr; b++) {

					CalcNormFloat(v1, v2, v3, n1);

					VecAddf(v5, v5, n1);
					VecAddf(v6, v6, n1);
					VecAddf(v7, v7, n1);
					VecAddf(v8, v8, n1);

					v2= v1; 
					v1+=3;
					v4= v3; 
					v3+=3;
					v6= v5; 
					v5+=3;
					v8= v7; 
					v7+=3;
				}
			}

			a= dl->parts*dl->nr;
			fp= ndata;
			v1= vdata;
			v2= data;
			while(a--) {
				Normalise(fp);

				fastshade(v1, fp, 0, col, v2+3, v2+4);

				v1+= 3;
				v2+= 5;
				fp+= 3;
			}
			freeN(vdata);
			freeN(ndata);
		}
		dl= dl->next;
	}
}

void convertDispList(dlbase, solid)
struct ListBase *dlbase;
short solid;
{
	struct DispList *dl, *dlnew;
	float *data, *data1;
	long len, a;

	dl= dlbase->first;
	while(dl) {
		if(solid && dl->type==DL_SURF) {
			len= dl->parts*dl->nr;
			dlnew= mallocN( sizeof(struct DispList)+5*4*len, "makeDispList4");
			memcpy(dlnew, dl, sizeof(struct DispList));
			dlnew->type= DL_SURF_GOUR;
			data= (float *)(dl+1);
			data1= (float *)(dlnew+1);
			a= len;
			while(a--) {
				VECCOPY(data1, data);
				data1+=5; 
				data+=3;
			}
			addhead(dlbase, dlnew);
			remlink(dlbase, dl);
			freeN(dl);
		}
		else if(solid==0) {
			if ELEM(dl->type, DL_SURF_GOUR, DL_SURF_SOLID) {
				len= dl->parts*dl->nr;
				dlnew= mallocN( sizeof(struct DispList)+3*4*len, "makeDispList4");
				memcpy(dlnew, dl, sizeof(struct DispList));
				dlnew->type= DL_SURF;
				data= (float *)(dl+1);
				data1= (float *)(dlnew+1);
				a= len;
				while(a--) {
					VECCOPY(data1, data);
					data1+=3; 
					data+=5;
				}
				addhead(dlbase, dlnew);
				remlink(dlbase, dl);
				freeN(dl);
			}
		}
		dl= dl->next;
	}
}

void curve_to_displist(nubase, dispbase)
struct ListBase *nubase, *dispbase;
{
	struct Nurb *nu;
	struct DispList *dl;
	struct BezTriple *bezt, *prevbezt;
	float *data, *v1, *v2;
	int a, len;
	
	nu= nubase->first;
	
	while(nu) {
		if(nu->hide==0) {
			if((nu->type & 7)==1) {
				len= 1+(nu->pntsu-1+(nu->flagu & 1))*nu->resolu;
				dl= callocN(sizeof(struct DispList)+len*3*4, "makeDispList0");
				addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->col;

				data= (float *)(dl+1);

				if(nu->flagu & 1) {
					dl->type= DL_POLY;
					a= nu->pntsu;
				}
				else {
					dl->type= DL_SEGM;
					a= nu->pntsu-1;
				}
				
				prevbezt= nu->bezt;
				bezt= prevbezt+1;
				
				while(a--) {
					if(a==0 && dl->type== DL_POLY) bezt= nu->bezt;
					v1= prevbezt->vec[1];
					v2= bezt->vec[0];
					maakbez(v1[0], v1[3], v2[0], v2[3], data, nu->resolu);
					maakbez(v1[1], v1[4], v2[1], v2[4], data+1, nu->resolu);
					if((nu->type & 8)==0)
						maakbez(v1[2], v1[5], v2[2], v2[5], data+2, nu->resolu);
					data+= 3*nu->resolu;
					prevbezt= bezt;
					bezt++;
				}
			}
			else if((nu->type & 7)==4) {
				len= nu->pntsu*nu->resolu;
				dl= callocN(sizeof(struct DispList)+len*3*4, "makeDispList0");
				addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->col;

				data= (float *)(dl+1);
				if(nu->flagu & 1) dl->type= DL_POLY;
				else dl->type= DL_SEGM;
				makeNurbcurve(nu, data);
			}
		}
		nu= nu->next;
	}
}

void bgnDisplist(listbase)		/* hiermee globals zetten */
struct ListBase *listbase;
{

	freelistN(listbase);
	displistbase= listbase;

}

void makeDispList(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VlakOb *adrvl;
	struct VertOb *adrve, *a1, *a2, *a3;
	struct MoData *mo;
	struct Nurb *nu;
	struct BezTriple *bezt, *prevbezt;
	struct BPoint *bp;
	struct ListBase dlbev;
	struct DispList *dl, *dlnew, *dlb, *dlpoly, *dle, *dls, *dlt;
	struct BevList *bl;
	struct BevPoint *bevp;
	struct dl_GourFace *dlgf;
	float *data, *data1, *fp1, *v1, *v2, *v3, widfac, vec[3], vec1[3];
	long test, len, a, b, c, draw=0, edges, segs, trias;

	if(base==0) return;


	if(base->soort==1) {

		if(base==G.ebase) return;

		ob= (struct ObData *)base->d;
		vv= ob->vv;
		draw= ob->dt;
		bgnDisplist(&(ob->disp));

		if(ob->ef && ob->ef<4) {
			bgneffect(vv);
			if(ob->ef==3) vertexTex(ob);
			else if(ob->ef==2) vlag(base);
			else if(ob->ef==1) golf(base);
		}

		if(draw==2 && G.zbuf) {
			/* deze diplist is nogal primitief: geen echte gouraud */
			/* en het gaat gelijk op met het VV datablok */

			len= vv->vlak*sizeof(struct dl_GourFace);
			dl= mallocN(sizeof(struct DispList)+len, "makeDispList67");
			addtail(&(ob->disp), dl);
			dl->type= DL_TRIA_GOUR;
			dl->parts= vv->vlak;
			dl->nr= 1;
			dl->col= 0;
			adrve= (struct VertOb *)(vv+1);
			adrvl= (struct VlakOb *)(adrve+vv->vert);
			dlgf= (struct dl_GourFace *)(dl+1);
			for(a=0; a<vv->vlak; a++) {
				a1= adrve+adrvl->v1;
				a2= adrve+adrvl->v2;
				a3= adrve+adrvl->v3;
				VECCOPY(dlgf->co[0], a1->c);
				VECCOPY(dlgf->co[1], a2->c);
				VECCOPY(dlgf->co[2], a3->c);
				dlgf++;
				adrvl++;
			}
		}
		else if(draw==2) {
			/* aantal tellen */
			edges= segs= trias= 0;
			adrve= (struct VertOb *)(vv+1);
			adrvl= (struct VlakOb *)(adrve+vv->vert);
			for(a=0;a<vv->vlak;a++) {

				test= ~((adrvl->ec) >>5) ;
				test &= 7;
				if(test) {
					switch(test) {
					case 1: 
					case 2: 
					case 4:
						edges++;
						break;
					case 3: 
					case 5: 
					case 6:
						segs++;
						break;
					case 7:
						trias++;
						break;
					}
				}
				adrvl++;

			}
			if(edges) {
				dle= mallocN(sizeof(struct DispList)+2*4*3*edges, "makeDispList10");
				addtail(&(ob->disp), dle);
				dle->type= DL_SEGM;
				dle->parts= edges;
				dle->nr= 2;
				dle->col= 0;
				v1= (float *)(dle +1);
			}
			if(segs) {
				dls= mallocN(sizeof(struct DispList)+3*4*3*segs, "makeDispList10");
				addtail(&(ob->disp), dls);
				dls->type= DL_SEGM;
				dls->parts= segs;
				dls->nr= 3;
				dls->col= 0;
				v2= (float *)(dls +1);
			}
			if(trias) {
				dlt= mallocN(sizeof(struct DispList)+3*4*3*trias, "makeDispList10");
				addtail(&(ob->disp), dlt);
				dlt->type= DL_POLY;
				dlt->parts= trias;
				dlt->nr= 3;
				dlt->col= 0;
				v3= (float *)(dlt +1);
			}

			adrve= (struct VertOb *)(vv+1);
			adrvl= (struct VlakOb *)(adrve+vv->vert);
			for(a=0;a<vv->vlak;a++) {

				test= ~((adrvl->ec) >>5);
				test &= 7;
				if(test) {
					a1=adrve+adrvl->v1;
					a2=adrve+adrvl->v2;
					a3=adrve+adrvl->v3;

					switch(test) {
					case 4:
						VECCOPY(v1, a1->c); 
						v1+=3;
						VECCOPY(v1, a2->c); 
						v1+=3;
						break;
					case 2:
						VECCOPY(v1, a2->c); 
						v1+=3;
						VECCOPY(v1, a3->c); 
						v1+=3;
						break;
					case 1:
						VECCOPY(v1, a3->c); 
						v1+=3;
						VECCOPY(v1, a1->c); 
						v1+=3;
						break;
					case 6:
						VECCOPY(v2, a1->c); 
						v2+=3;
						VECCOPY(v2, a2->c); 
						v2+=3;
						VECCOPY(v2, a3->c); 
						v2+=3;
						break;
					case 3:
						VECCOPY(v2, a2->c); 
						v2+=3;
						VECCOPY(v2, a3->c); 
						v2+=3;
						VECCOPY(v2, a1->c); 
						v2+=3;
						break;
					case 5:
						VECCOPY(v2, a3->c); 
						v2+=3;
						VECCOPY(v2, a1->c); 
						v2+=3;
						VECCOPY(v2, a2->c); 
						v2+=3;
						break;
					case 7:
						VECCOPY(v3, a1->c); 
						v3+=3;
						VECCOPY(v3, a2->c); 
						v3+=3;
						VECCOPY(v3, a3->c); 
						v3+=3;
						break;
					}
				}
				adrvl++;
			}
		}
		if(ob->ef && ob->ef<4) {
			endeffect(vv);
		}
	}
	else if(base->soort==5) {
		ob= (struct ObData *)base->d;
		draw= ob->dt;
		bgnDisplist(&(ob->cu->disp));

		if(base==G.ebase) nu= editNurb.first;
		else nu= ob->cu->curve.first;

		while(nu) {
			if(nu->hide==0) {
				if(nu->pntsv==1) {
					if(draw==0) len= nu->pntsu;
					else len= nu->pntsu*nu->resolu;
					
					dl= callocN(sizeof(struct DispList)+len*3*4, "makeDispList0");
					addtail(&(ob->cu->disp), dl);
					dl->parts= 1;
					dl->nr= len;
					dl->col= nu->col;

					data= (float *)(dl+1);
					if(nu->flagu & 1) dl->type= DL_POLY;
					else dl->type= DL_SEGM;
					
					if(draw==0) {
						bp= nu->bp;
						while(len--) {
							VECCOPY(data, bp->vec);
							bp++;
							data+= 3;
						}
					}
					else makeNurbcurve(nu, data);
				}
				else {
					if(draw==0 && base==G.ebase) ;
					else {
						if(draw==0) len= nu->pntsu*nu->pntsv;
						else len= nu->resolu*nu->resolv;
						
						dl= callocN(sizeof(struct DispList)+len*3*4, "makeDispList1");
						addtail(&(ob->cu->disp), dl);
	
						if(draw==0) {
							dl->parts= nu->pntsv;
							dl->nr= nu->pntsu;
							if(nu->flagu & 1) dl->flag|= 1;
							if(nu->flagv & 1) dl->flag|= 2;
						}
						else {
							dl->parts= nu->resolu;	/* andersom want makeNurbfaces gaat zo */
							dl->nr= nu->resolv;
							if(nu->flagv & 1) dl->flag|= 1;	/* ook andersom ! */
							if(nu->flagu & 1) dl->flag|= 2;
						}
						dl->col= nu->col;
	
						data= (float *)(dl+1);
						dl->type= DL_SURF;
						
						if(draw==0) {
							bp= nu->bp;
							while(len--) {
								VECCOPY(data, bp->vec);
								bp++;
								data+= 3;
							}
						}
						else makeNurbfaces(nu, data);
					}
				}
			}
			nu= nu->next;
		}
	}
	else if(base->soort==11) {
		ob= (struct ObData *)base->d;
		draw= ob->dt;
		bgnDisplist(&(ob->cu->disp));

		if(base== G.ebase) {
			curve_to_displist(&editNurb, &(ob->cu->disp));
		}
		else {
			dlbev.first= dlbev.last= 0;
			if(ob->cu->ext1 || ob->cu->ext2 || ob->cu->bevbase) {
				if(ob->dt!=0) makebevelcurve(ob->cu, &dlbev);
			}

			/* met bevellist werken */
			widfac= (ob->cu->width-100)/ob->cu->ws;
			bl= ob->cu->bev.first;
			nu= ob->cu->curve.first;
			while(bl) {

				if(dlbev.first==0) {
					dl= mallocN(sizeof(struct DispList) +12*bl->nr, "makeDisplist2");
					addtail(displistbase, dl);
					if(bl->poly!= -1) dl->type= DL_POLY;
					else dl->type= DL_SEGM;
					dl->parts= 1;
					dl->nr= bl->nr;
					dl->col= nu->col;

					a= dl->nr;
					bevp= (struct BevPoint *)(bl+1);
					data= (float *)(dl+1);
					while(a--) {
						data[0]= bevp->x+widfac*bevp->sin;
						data[1]= bevp->y+widfac*bevp->cos;
						data[2]= bevp->z;
						bevp++;
						data+=3;
					}
				}
				else {
					data1= data;
					/* voor iedere stuk van de bevel een aparte dispblok maken */
					dlb= dlbev.first;
					while(dlb) {
						dl= mallocN(sizeof(struct DispList) +12*dlb->nr*bl->nr, "makeDisplist3");
						addtail(displistbase, dl);
						/* dl->type= dlb->type; */

						dl->type= DL_SURF;
						dl->flag= 0;
						if(dlb->type==DL_POLY) dl->flag++;
						if(bl->poly>=0) dl->flag+=2;

						dl->parts= bl->nr;
						dl->nr= dlb->nr;
						dl->col= nu->col;

						data= (float *)(dl+1);
						bevp= (struct BevPoint *)(bl+1);
						a= bl->nr;
						while(a--) {	/* voor ieder punt van poly een bevelstuk maken */

							/* roteer bevelstuk en schrijf in data */
							fp1= (float *)(dlb+1);
							b= dlb->nr;

							while(b--) {
								if(ob->cu->flag & 256) {	/* 3D */
									vec[0]= fp1[1]+widfac;
									vec[1]= 0.0;
									vec[2]= fp1[2];
									Mat3MulVecfl(bevp->mat, vec);
									
									data[0]= bevp->x+ vec[0];
									data[1]= bevp->y+ vec[1];
									data[2]= bevp->z+ vec[2];
								}
								else {
									data[0]= bevp->x+ (fp1[1]+widfac)*bevp->sin;
									data[1]= bevp->y+ (fp1[1]+widfac)*bevp->cos;
									data[2]= bevp->z+ fp1[2];
								}
								data+=3;
								fp1+=3;
							}

							bevp++;
						}

						dlb= dlb->next;
					}
				}

				bl= bl->next;
				nu= nu->next;
			}

			if(ob->cu->ext1 || ob->cu->ext2 || ob->cu->bevbase) {
				freebevelcurve(&dlbev);
			}
		}
	}
	else if(base->soort== -2) {
		draw= 0;
		mo= (struct MoData *)base->d;
		bgnDisplist(&(mo->disp));

		if(base== G.ebase) 
			curve_to_displist(&editNurb, &(mo->disp));
		else
			curve_to_displist(&(mo->curve), &(mo->disp));
		berekenpad(base);
	}

	if(draw>1 && G.zbuf) {	/* maak van DispList solid */
		convertDispList(displistbase, 1);
		shadeDispList(base);
	}
}


void testDispLists()
{
	/* testen op G.zbuf, drawtype en layers */
	struct Base *base;
	struct ObData *ob;
	struct DispList *dl;
	struct VV *vv;
	short solid;

	if(G.ebase && G.ebase->soort==1) {	/* displisten uit obdups vrijgeven */
		ob= (struct ObData *)G.ebase->d;
		solid= (G.zbuf && ob->dt==2);
		if(solid==0) {
			clearDisplistObdups();	/* in verdit.c */
		}
	}
	else vv= 0;

	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base->soort==1) {
				ob= (struct ObData *)base->d;
				solid= (G.zbuf && ob->dt==2);
				if(solid) {
					if(ob->disp.first) {
						dl= ob->disp.first;
						if(dl->type<100) makeDispList(base);
					}
					else makeDispList(base);
				}
				else if(ob->disp.first) freelistN( &(ob->disp) );
			}
			else if(base->soort==5) {
				ob= (struct ObData *)base->d;
				solid= (G.zbuf && ob->dt==2);
				convertDispList( &(ob->cu->disp), solid );
				if(solid) shadeDispList(base);
			}
			else if(base->soort==11) {
				ob= (struct ObData *)base->d;
				solid= (G.zbuf && ob->dt==2);
				convertDispList( &(ob->cu->disp), solid );
				if(solid) shadeDispList(base);
			}
		}
		base= base->next;
	}
	
}

void reshadeall()
{
	struct Base *base;
	struct ObData *ob;

	if(G.zbuf==0) return;

	base= G.firstbase;
	while(base) {
		if(base->soort & 1) {
			if(base->lay & view0.lay) {
				ob= (struct ObData *)base->d;
				if(ob->dt==2) shadeDispList(base);
			}
		}
		base= base->next;
	}
}

void filldisplist(dl, disp)	/* LET OP POINTERPOINTER, en gebruik mallocN voor displist */
struct DispList *dl, **disp;
{
	extern struct ListBase fillvertbase;
	extern struct ListBase filledgebase;
	extern struct ListBase fillvlakbase;
	struct EditVert *eve,*v1,*vstart,*vlast;
	struct EditEdge *eed,*e1;
	struct EditVlak *evl,*nextvl;
	struct DispList *dlnew;
	float *f1;
	int tot, a, b;
	
	
	if(dl==0) return;
	if(dl->type!=DL_POLY) return;
	if(disp==0) return;	

	/* editverts en edges maken */
	f1= (float *)(dl+1);
	tot= dl->nr;
	eve= v1= 0;
	
	while(tot--) {
		vlast= eve;
		eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
		addtail(&fillvertbase, eve);
		VECCOPY(eve->co, f1);
		if(vlast==0) v1= eve;
		else {
			eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
			addtail(&filledgebase,eed);
			eed->v1= vlast;
			eed->v2= eve;
		}
		f1+=3;
	}

	if(eve!=0 && v1!=0) {
		eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
		addtail(&filledgebase,eed);
		eed->v1= eve;
		eed->v2= v1;
	}

	if(edgefill(0)!=0) {
		tot= 0;
		evl= fillvlakbase.first;
		while(evl) {
			tot++;
			evl= evl->next;
		}
		if(tot) {
			dlnew= mallocN(sizeof(struct DispList)+tot*3*3*4, "filldisplist");
			*dlnew= *dl;
			dlnew->nr= 3;
			dlnew->parts= tot;
			f1= (float *)(dlnew+1);
			evl= fillvlakbase.first;
			while(evl) {
				VECCOPY(f1, evl->v1->co);
				f1+=3;
				VECCOPY(f1, evl->v2->co);
				f1+=3;
				VECCOPY(f1, evl->v3->co);
				f1+=3;
				evl= evl->next;
			}
			*disp= dlnew;
		}
	}
	
	freelist(&fillvertbase);
	freelist(&filledgebase);
	freelist(&fillvlakbase);
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;
}


/*******************************/
/*****       OUTLINE       *****/
/*******************************/

typedef struct Sample{
	short x, y;
}Sample;

typedef struct Segment{
	/* coordinaten */
	struct Segment * next, * prev;
	float co[2];
}Segment;


long alpha_in_out(struct ImBuf * ibuf, long x, long y)
{
	
	if (ibuf == 0) return (0);
	if (x < 0 || y < 0 || x >= ibuf->x || y >= ibuf->y || ibuf->rect == 0) return (-1);
	
	if (ibuf->rect[(y * ibuf->x) + x] > 0x81000000) return (1);
	return(0);
}

long dflt_in_out(struct ImBuf * ibuf, long x, long y)
{
	
	if (ibuf == 0) return (0);
	if (x < 0 || y < 0 || x >= ibuf->x || y >= ibuf->y || ibuf->rect == 0) return (-1);
	
	return (ibuf->rect[(y * ibuf->x) + x]);
}


Sample * outline(struct ImBuf * ibuf, long (*in_or_out)())
{
	static long dirs[8][2] = {
		-1,  0,		-1,  1,		0,  1,		 1,  1, 
		 1,  0,		 1, -1,		0, -1,		-1, -1, 
	};
	
	long dir, x, y, in, i;
	long count, sampcount;
	long startx = 0, starty = 0;
	uchar * rect;
	Sample * samp, * oldsamp;
	
	/* wat erin gaat:
	 * 1 - plaatje waarvan outline berekent moet worden, 
	 * 2 - pointer naar functie die bepaalt welke pixel in of uit is
	 */
	
	if (ibuf == 0) return (0);
	if (ibuf->rect == 0) return (0);
	
	if (in_or_out == 0) {
		in_or_out = dflt_in_out;
		if (ibuf->depth == 32) in_or_out = alpha_in_out;
	}
	
	in = in_or_out(ibuf, 0, 0);
	
	/* zoek naar eerste overgang en ga van daar uit 'zoeken' */	
	for (y = 0; y < ibuf->y; y++) {
		for (x = 0; x < ibuf->x; x++) {
			if (in_or_out(ibuf, x, y) != in) {
				/* eerste 'andere' punt gevonden !! */
				
				if (x != startx) dir = 0;
				else dir = 6;
				
				startx = x; starty = y;
				count = 1;
				sampcount = 2000;
				samp = mallocN(sampcount * sizeof(Sample), "wire_samples");
				
				do{
					samp[count].x = x; samp[count].y = y;
					count++;
					
					if (count >= sampcount) {
						oldsamp = samp;
						samp = mallocN(2 * sampcount * sizeof(Sample), "wire_samples");
						memcpy(samp, oldsamp, sampcount * sizeof(Sample));
						sampcount *= 2;
						freeN(oldsamp);
					}
					
					i = 0;
					while(in_or_out(ibuf, x + dirs[dir][0], y + dirs[dir][1]) == in) {
						dir = (dir + 1) & 0x7;
						if (i++ == 9) break;
					}
					
					if (i >= 8) {
						/* dit moet een losse punt geweest zijn */
						break;
					}
					
					x += dirs[dir][0];
					y += dirs[dir][1];
					dir = (dir - 3) & 0x7;
				}while(x != startx || y != starty);
				
				if (i >= 8) {
					/* losse punten patch */
					freeN(samp);
				} else {
					count = count - 1;
					samp[0].x = count >> 16;
					samp[0].y = count;
					return(samp);
				}
			}
		}
	}
	printf("geen overgang \n");
	return(0);
}



/*******************************/
/*****      WIREFRAME      *****/
/*******************************/


float DistToLine2D(v1,v2,v3)   /* met formule van Hesse :GEEN LIJNSTUK! */
short *v1,*v2,*v3;
{
	float a[2],deler;

	a[0] = v2[1]-v3[1];
	a[1] = v3[0]-v2[0];
	deler = fsqrt(a[0]*a[0]+a[1]*a[1]);
	if(deler == 0.0) return 0;

	return fabsf((v1[0]-v2[0])*a[0]+(v1[1]-v2[1])*a[1])/deler;

}

float ComputeMaxShpError(samp, first, last, splitPoint)
    Sample	*samp;			/*  Array of digitized points	*/
    int		first, last;	/*  Indices defining region	*/
    int		*splitPoint;	/*  Point of maximum error	*/
{
    int		i;
    float	maxDist;				/*  Maximum error		*/
    float	dist;					/*  Current error		*/
 
    *splitPoint = (last - first + 1) / 2;
    maxDist = 0.0;
	
    for (i = first + 1; i < last; i++) {				
		dist = DistToLine2D(samp+i, samp+first, samp+last);

		if (dist >= maxDist) {
	    	maxDist = dist;
	    	*splitPoint = i;
		}
    }

    return (maxDist);
}


void FitPoly(samp, first, last, shperr, seglist)
    Sample	*samp;				/*  Array of digitized points */
    int		first, last;		/* Indices of first and last pts in region */
    float	shperr;				/*  User-defined error squared	   */
	ListBase * seglist;
{
    Segment	* seg;				/* Control points segment*/
    float	maxError;			/*  Maximum fitting error	 */
	float	x, y;
    int		splitPoint;			/*  Point to split point set at	 */
    int		nPts;				/*  Number of points in subset  */
    int		i;
	
    nPts = last - first + 1;

    /*  Use heuristic if region only has two points in it */

	seg = mallocN(sizeof(Segment), "wure_segment");

	seg->co[0] = samp[first].x;
	seg->co[1] = samp[first].y;
	
    if (nPts == 2) {
		addtail(seglist, seg);
		return;
    }

	maxError = ComputeMaxShpError(samp, first, last, &splitPoint);
	if (maxError < shperr) {
		addtail(seglist, seg);
		return;
	}
 	
    /* Fitting failed -- split at max error point and fit recursively */
	
    FitPoly(samp, first, splitPoint, shperr, seglist);
    FitPoly(samp, splitPoint, last, shperr, seglist);
	
	freeN(seg);
}


void ibuf2wire(ListBase * wireframe, struct ImBuf * ibuf)
{
	long count;
	Sample * samp;
	
	/* eerst een lijst met samples maken */
	
	samp = outline(ibuf, 0);
	if (samp == 0) return;
	
	count = (samp[0].x << 16) + samp[0].y;
	if (count) FitPoly(samp, 1, count, 1.0, wireframe); /* was 3.0. Frank */

	freeN(samp);
}



void imagestodisplist()
{
	struct Base *base;
	struct ObData *ob;
	struct ColBlck *col;
	struct Tex *tex;
	ListBase _wireframe, *wireframe;
	struct DispList *dl;
	Segment *seg;
	float *data, xfac, yfac, xsi, ysi;
	int tot, texnr;
	
	_wireframe.first= 0;
	_wireframe.last= 0;
	wireframe = &_wireframe;
	
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if( base->soort==1) {
				ob= (struct ObData *)base->d;
				col= (struct ColBlck *)(ob+1);
				
				texnr = 0;
				
				while(col->tex[texnr]) {
					
					tex= G.adrtex[col->tex[texnr] ];
						
					if(tex->ima && tex->ima->ibuf) {
						
						ibuf2wire(wireframe, tex->ima->ibuf);
						tot= 0;
						seg = wireframe->first;
						while (seg) {
							tot++;
							seg = seg->next;
						}
	
						if(tot) {
							bgnDisplist(&(ob->disp));
							dl= callocN(sizeof(struct DispList)+tot*3*4, "makeDispList0");
							addtail(&(ob->disp), dl);
							dl->type= DL_POLY;
							dl->parts= 1;
							dl->nr= tot;
							
							xsi= 0.5*(tex->ima->ibuf->x);
							ysi= 0.5*(tex->ima->ibuf->y);
							xfac= 32767.0/xsi;
							yfac= 32767.0/ysi;
							/* xsi/= 2.0; */
							/* ysi/= 2.0; */
												
							data= (float *)(dl+1);
							seg = wireframe->first;
							while (seg) {
								data[0]= xfac*(seg->co[0]-xsi);
								data[1]= yfac*(seg->co[1]-ysi);
								data+= 3;
								seg = seg->next;
							}
							freelistN(wireframe);
						}
						break;
					}
					
					texnr++;
					if(texnr>3) break;
					
				}
			}
		}
		base= base->next;
	}
	projektie();
}

void makeimagesize(int both)
{
	struct Base *base;
	struct ObData *ob;
	struct ColBlck *col;
	struct Tex *tex;
	float *data, xfac, yfac, xsi, ysi;
	int tot, texnr;
	
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if( base->soort==1) {
				ob= (struct ObData *)base->d;
				col= (struct ColBlck *)(ob+1);
				
				texnr= 0;
				while(col->tex[texnr]) {
					tex= G.adrtex[col->tex[texnr] ];
					
					if(tex->ima && tex->ima->ibuf) {
						xfac= tex->ima->ibuf->x;
						yfac= tex->ima->ibuf->y;
						if (both) {
							xfac /= 400;
							yfac /= 400;
							
							Mat3One(base->m);
							base->m[0][0]= xfac;
							base->m[1][1]= yfac;
						} else {
							yfac/= xfac;
							xfac= base->m[0][0];
							Mat3One(base->m);
							base->m[0][0]= xfac;
							base->m[1][1]= xfac*yfac;
						}
						base->f &= ~8;
						
						break;
					}
					
					texnr++;
					if(texnr>3) break;
				}
			}
		}
		base= base->next;
	}
	projektie();
}

