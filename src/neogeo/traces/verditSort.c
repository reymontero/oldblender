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

/*	verdit.c	*/


/*

struct EditVert *addvertlist(vec)
		addedgelist(v1,v2)
struct EditVlak *addvlaklist(v1,v2,v3)

struct EditEdge *findedgelist(v1,v2)
short 		exist_vlak(v1,v2,v3)

		make_ebasedata()
		load_ebasedata()

		muis_ebase()
		selectconnected()

short		extrudeflag(flag,type)
		rotateflag(flag)
		extrudeVV()

		addvert_ebase()
		addedgevlak_ebase()
		delvert()
		adduplicateVV()
		addprimitive()

		vertgrabber()
		vertroteer()
		vertsizing()
*/


#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "/usr/people/include/Trace.h"

extern ListBase editNurb;
extern ListBase bpbase;
extern short rodedit;	/* trabuts.c */
extern short editbutflag;

struct EditEdge *lasted=0,**edsortblock;
long totedgesb,oblen;
char *obdup=0;

float icovert[12][3] =
	{0,0,-200, 144.72,-105.144,-89.443,
	-55.277,-170.128,-89.443, -178.885,0,-89.443,
	-55.277,170.128,-89.443, 144.72,105.144,-89.443,
	55.277,-170.128,89.443, -144.72,-105.144,89.443,
	-144.72,105.144,89.443, 55.277,170.128,89.443,
	178.885,0,89.443, 0,0,200};
short icovlak[20][3] = {
	1,0,2, 1,0,5, 2,0,3, 3,0,4,
	4,0,5, 1,5,10, 2,1,6, 3,2,7,
	4,3,8, 5,4,9, 10,1,6, 6,2,7,
	7,3,8, 8,4,9, 9,5,10, 6,10,11,
	7,6,11, 8,7,11, 9,8,11, 10,9,11};


/* ************ FAST MALLOC ******************** */

struct ListBase fastmallocbase;

struct fastmalloc {
	struct fastmalloc *next, *prev;
	int size, nr;
	struct ListBase subbase;
	void *data;
	int *dataflags;
	int flag;		/* 1: full */
};

void initfastmalloc(int size, int nr)
{
	
}



/********* qsort routines *********/


struct xvertsort {
	float x;
	struct EditVert *v1;
};


long vergxco(x1,x2)
struct xvertsort *x1,*x2;
{

	if( x1->x > x2->x ) return 1;
	else if( x1->x < x2->x) return -1;
	return 0;
}

long vergedge(e1,e2)
struct EditEdge **e1,**e2;
{
	struct EditVert *v3;

	if( (*e1)->v1 > (*e2)->v1 ) return 1;
	else if( (*e1)->v1 < (*e2)->v1) return -1;

	else if( (*e1)->v2 > (*e2)->v2 ) return 1;
	else if( (*e1)->v2 < (*e2)->v2 ) return -1;

	return 0;
}

long vergedge2(e1,e2)
long *e1,*e2;
{
	if( e1[0] > e2[0] ) return 1;
	else if( e1[0] < e2[0] ) return -1;
	else if( e1[1] > e2[1] ) return 1;
	else if( e1[1] < e2[1] ) return -1;

	return 0;
}
long vergedge3(e1,e2)
long *e1,*e2;
{
	if( e1[0] > e2[0] ) return 1;
	else if( e1[0] < e2[0] ) return -1;

	return 0;
}

void maakedsortblock(a)
short a;	/* a: clear edgeflags */
{
	struct EditEdge *eed,**eds;

	totedgesb=0;
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		totedgesb++;
		if(a) {
			eed->f= eed->f1= 0;
		}
		eed= eed->next;
	}

	if(totedgesb==0) return;

	eds=edsortblock= (struct EditEdge **)mallocN(4*totedgesb,"edsort");
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		*(eds++)= eed;
		eed= eed->next;
	}
}


/******* *************/

void insertlinkbefore(listbase,nextlink,newlink)
struct ListBase *listbase;
struct Link *nextlink,*newlink;
{
	/* newlink komt voor nextlink */

	if (newlink == 0) return;
	if (listbase == 0) return;

	if(listbase->first==0) { /* lege lijst */
		listbase->first= newlink;
		listbase->last= newlink;
		return;
	}
	if (nextlink== 0) {	/* inserten aan einde lijst */
		newlink->prev= listbase->last;
		newlink->next= 0;
		((struct Link *)listbase->last)->next= newlink;
		listbase->last= newlink;
		return;
	}

	if (listbase->first== nextlink) /* aan begin lijst */
		listbase->first = newlink;

	newlink->next= nextlink;
	newlink->prev= nextlink->prev;
	nextlink->prev= newlink;
	if(newlink->prev) newlink->prev->next= newlink;
}


struct EditVert *addvertlist(vec)
float *vec;
{
	struct EditVert *eve;

	eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	addtail(&G.edve,eve);
	VECCOPY(eve->co,vec);

	return eve;
}

void remedge(eed)
struct EditEdge *eed;
{
	remlink(&G.eded,eed);
	if(eed==lasted) lasted=0;

}

struct EditEdge *findedgelist(v1,v2)
struct EditVert *v1,*v2;
{
	struct EditVert *v3;
	struct EditEdge *eed,ed,**edp;
	long direction,beg=0,end=0,last=0x7FFFFFF;

	/* swap ? */
	if(v1>v2) {
		v3= v2; 
		v2= v1; 
		v1= v3;
	}
	else if(v1==v2) return 0;

	if(edsortblock) {
		ed.v1= v1;
		ed.v2= v2;
		eed= &ed;
		edp= (struct EditEdge **)bsearch(&eed,edsortblock,totedgesb,4,vergedge);
		if(edp) return *edp;
		else return 0;
	}

	/* voorwaarts of achterwaarts */
	if(G.eded.first) {
		eed= (struct EditEdge *)G.eded.first;
		beg= abs( (long)eed->v1 - (long)v1 );
		eed= (struct EditEdge *)G.eded.last;
		end= abs( (long)eed->v1 - (long)v1 );
		if(lasted) last= abs( (long)lasted->v1 - (long)v1 );
	}
	if( last<beg && last<end ) {  /* vanuit laatst gevonden edge */
		if(last==0) direction= (long)v2 - (long)lasted->v2;
		else direction= (long)v1 - (long)lasted->v1;
	}
	else if(beg<end) {  /* vanuit eerste edge */
		lasted= (struct EditEdge *)G.eded.first;
		direction= 1;
	}
	else {  /* vanuit laatste edge */
		lasted= (struct EditEdge *)G.eded.last;
		direction= -1;
	}

	/* vind plek in edgelijst */
	if(direction<0) {
		while(lasted) {
			if(v1==lasted->v1 && v2==lasted->v2) return lasted;
			if(v1> lasted->v1) return 0;
			lasted= lasted->prev;
		}
	} else {
		while(lasted) {
			if(v1==lasted->v1 && v2==lasted->v2) return lasted;
			if(v1< lasted->v1) return 0;
			lasted= lasted->next;
		}
	}
	return 0;
}

struct EditEdge *addedgeSimp(v1,v2)
struct EditVert *v1,*v2;
{
	/* gewoon aan einde lijst toevoegen */
	struct EditEdge *new;

	new= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
	new->v1= v1;
	new->v2= v2;
	addtail(&G.eded,new);

	return new;
}


struct EditEdge *addedgelist(v1,v2)
struct EditVert *v1,*v2;
{
	struct EditVert *v3;
	struct EditEdge *eed,*new;
	long beg=0,end=0,last=0x7FFFFFFF,direction;


	/* swap ? */
	if(v1>v2) {
		v3= v2; 
		v2= v1; 
		v1= v3;
	}
	else if(v1==v2) return 0;

	/* voorwaarts of achterwaarts */
	if(G.eded.first) {
		eed= (struct EditEdge *)G.eded.first;
		beg= abs( (long)eed->v1 - (long)v1 );
		eed= (struct EditEdge *)G.eded.last;
		end= abs( (long)eed->v1 - (long)v1 );
		if(lasted) last= abs( (long)lasted->v1 - (long)v1 );
	}
	if( last<beg && last<end ) {  /* vanuit laatst gevonden edge */
		if(last==0) direction= (long)v2 - (long)lasted->v2;
		else direction= (long)v1 - (long)lasted->v1;
	}
	else if(beg<end) {  /* vanuit eerste edge */
		lasted= (struct EditEdge *)G.eded.first;
		direction= 1;
	}
	else {  /* vanuit laatste edge */
		lasted= (struct EditEdge *)G.eded.last;
		direction= -1;
	}

	/* vind plek in edgelijst */
	if(direction<0) {
		while(lasted) {
			if(lasted->v1== v1) {
				if(lasted->v2 <= v2) break;
			}
			else if(lasted->v1 < v1) break;
			lasted= lasted->prev;
		}
	} else {
		while(lasted) {
			if(lasted->v1== v1) {
				if(lasted->v2 >= v2) break;
			}
			else if(lasted->v1 > v1) break;
			lasted= lasted->next;
		}
	}
	/* niewe edge? */
	if(lasted) {
		if(lasted->v1==v1 && lasted->v2==v2) return lasted;
	}
	new= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
	new->v1= v1;
	new->v2= v2;
	if(direction<0) insertlink(&G.eded,lasted,new);
	else insertlinkbefore(&G.eded,lasted,new);

	return new;
}

struct EditVlak *addvlaklistZE(v1,v2,v3,col)   /* zonder edges */
struct EditVert *v1,*v2,*v3;
short col;
{
	struct EditVlak *evl;

	/* voeg vlak toe aan lijst */
	if(v1==v2 || v2==v3 || v1==v3) return 0;

	evl= (struct EditVlak *)malloc(sizeof(struct EditVlak));
	evl->v1= v1;
	evl->v2= v2;
	evl->v3= v3;
	evl->f1= col;
	evl->f= 0;
	addtail(&G.edvl,evl);
	CalcNormFloat(v1->co, v2->co, v3->co, evl->n);

	return evl;
}

struct EditVlak *addvlaklist(v1,v2,v3,col)
struct EditVert *v1,*v2,*v3;
short col;
{
	struct EditVlak *evl;

	/* voeg vlak toe aan lijst en doe meteen de edges */
	addedgelist(v1,v2);
	addedgelist(v2,v3);
	addedgelist(v3,v1);
	if(v1==v2 || v2==v3 || v1==v3) return 0;

	evl= (struct EditVlak *)malloc(sizeof(struct EditVlak));
	evl->v1= v1;
	evl->v2= v2;
	evl->v3= v3;
	evl->f1= col;
	evl->f= 0;
	addtail(&G.edvl,evl);
	CalcNormFloat(v1->co, v2->co, v3->co, evl->n);

	return evl;
}

struct EditVlak *addvlakSimp(v1,v2,v3,col)
struct EditVert *v1,*v2,*v3;
short col;
{
	struct EditVlak *evl;

	/* voeg vlak toe aan lijst en doe de edges Simp*/
	addedgeSimp(v1,v2);
	addedgeSimp(v2,v3);
	addedgeSimp(v3,v1);
	if(v1==v2 || v2==v3 || v1==v3) return 0;

	evl= (struct EditVlak *)malloc(sizeof(struct EditVlak));
	evl->v1= v1;
	evl->v2= v2;
	evl->v3= v3;
	evl->f1= col;
	evl->f= 0;
	addtail(&G.edvl,evl);
	CalcNormFloat(v1->co, v2->co, v3->co, evl->n);
	return evl;
}


short exist_vlak(v1,v2,v3)
struct EditVert *v1,*v2,*v3;
{
	struct EditVlak *evl;

	if(findedgelist(v1,v2)==0) return 0;
	if(findedgelist(v2,v3)==0) return 0;
	if(findedgelist(v3,v1)==0) return 0;

	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		if(evl->v1==v1 || evl->v2==v1 || evl->v3==v1) {
			if(evl->v1==v2 || evl->v2==v2 || evl->v3==v2) {
				if(evl->v1==v3 || evl->v2==v3 || evl->v3==v3) {
					return 1;
				}
			}
		}
		evl= evl->next;
	}
	return 0;
}

short comparevlak(vl1, vl2)
struct EditVlak *vl1,*vl2;	/* return: aantal punten gelijk */
{
	short tot=0;

	if(vl1->v1==vl2->v1 || vl1->v2==vl2->v1 || vl1->v3==vl2->v1) tot++;
	if(vl1->v1==vl2->v2 || vl1->v2==vl2->v2 || vl1->v3==vl2->v2) tot++;
	if(vl1->v1==vl2->v3 || vl1->v2==vl2->v3 || vl1->v3==vl2->v3) tot++;
	return tot;
}

short dubbelvlak(struct EditVlak *evltest)
{
	struct EditVert *v1, *v2, *v3;
	struct EditVlak *evl;
	
	v1= evltest->v1;
	v2= evltest->v2;
	v3= evltest->v3;

	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		if(evl!=evltest) {
			if(evl->v1==v1 || evl->v2==v1 || evl->v3==v1) {
				if(evl->v1==v2 || evl->v2==v2 || evl->v3==v2) {
					if(evl->v1==v3 || evl->v2==v3 || evl->v3==v3) {
						return 1;
					}
				}
			}
		}
		evl= evl->next;
	}
	return 0;
}

void recalc_editnormals()
{
	struct EditVlak *evl;

	evl= G.edvl.first;
	while(evl) {
		CalcNormFloat(evl->v1->co, evl->v2->co, evl->v3->co, evl->n);
		evl= evl->next;
	}
}

void remake_edgelist()
{
	struct EditVert **eld,**edlist;
	struct EditEdge *eed;
	long a,toted=0;

	/* eerst aantal tellen */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		toted++;
		eed= eed->next;
	}
	if(toted==0) return;

	/* in array wegschrijven */
	edlist=eld= (struct EditVert **)mallocN(3*4*toted,"edlist");
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if(eed->v1< eed->v2) {
			*(eld++)= eed->v1;
			*(eld++)= eed->v2;
		} else {
			*(eld++)= eed->v2;
			*(eld++)= eed->v1;
		}
		*((long *)eld++)= *((long *)&(eed->f)); /* voor hideflag */
		eed= eed->next;
	}
	/* qsorten en edges maken */
	freelist(&G.eded);

	qsort(edlist,toted,12,vergedge2);
	eld= edlist;
	for(a=0;a<toted;a++) {
		if(eld[0]!=0 && eld[1]!=0) {
			if(a==0 || ( eld[-3]!=eld[0] || eld[-2]!=eld[1] ) ) {
				eed= addedgeSimp(eld[0],eld[1]);
				*((long *)&(eed->f))= *((long *)(eld+2));
			}
		}
		eld+=3;
	}
	freeN(edlist);
	lasted= 0;
	recalc_editnormals();
}

/* ************************ IN & OUT ***************************** */


void make_obdups()
{
	struct Base *base;
	struct ObData *ob,*obd;
	struct VV *vv;
	char *obdup1;

	if(obdup) freeN(obdup);
	obdup= 0;
	ob= (struct ObData *)G.ebase->d;
	if(G.ebase->soort==1) {
		vv= ob->vv;

		oblen= sizeof(struct ObData) +ob->c*sizeof(struct ColBlck);
		obdup1=obdup= mallocN(vv->us*oblen,"obdup");
		base= G.firstbase;
		while(base) {
			if(base->soort==1) {
				obd= (struct ObData *)base->d;
				if(obd->vv== vv) {
					memcpy(obdup1,obd,oblen);
					obdup1+=oblen;
				}
			}
			base= base->next;
		}
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		oblen= sizeof(struct ObData) +ob->c*sizeof(struct ColBlck);
		obdup= mallocN(oblen,"obdup");
		memcpy(obdup,ob,oblen);
	}
}

void make_ebasedata()
{
	struct Base *base;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct Key *key;
	struct EditVert *eve,**eld,**evlist,**edlist,*eve1,*eve2,*eve3;
	struct EditEdge *eed;
	struct EditVlak *evl;
	struct ObData *ob;
	struct VV *vv;
	float *fp;
	long tot=0,a,toted, tijd;

	if(G.ebase==0) return;

	setcursorN(2);

	if(G.edve.first) freelist(&G.edve);
	if(G.eded.first) freelist(&G.eded);
	if(G.edvl.first) freelist(&G.edvl);
	if(obdup) freeN(obdup);
	obdup=0;
	lasted= 0;

	if(G.ebase->soort==1) {
		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		G.totvert= tot= vv->vert;

		if(G.colact==0) G.colact=1;
		if(ob->c <G.colact) G.colact=1;

		freelistN(&(ob->disp));

		/* kopie van ObData en ColBlck blokken maken bij duplicates */
		make_obdups();

		if(tot==0) {
			setcursorN(1);
			return;
		}

		adrve= (struct VertOb *)(vv+1);
		adrvl= (struct VlakOb *)(adrve+tot);
		if(G.ipomode==22 && vv->key && G.actkey) {
			key= vv->key;
			a= 1;
			while(key) {
				if(a== G.actkey) break;
				a++;
				key= key->next;
			}
			if(key) {
				fp= (float *)(key+1);
				vv->ws= *fp; /* anders wordt het verkeerd getekend. Bij verlaten wordt het weer goed gezet */
				adrve= (struct VertOb *)(fp+1);
			}
		}

		/* editverts aanmaken */

		evlist= (struct EditVert **)mallocN(tot*4,"evlist");
		for(a=0;a<tot;a++) {
			eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
			evlist[a]= eve;
			addtail(&G.edve,eve);
			VECCOPY(eve->co,adrve->c);
			adrve++;
		}

tijd= clock();

		if(vv->vlak) {
			/* lijst van ongesorteerde en dubbele edges */
			/* meteen ook vlakken maken */

			edlist=eld= (struct EditVert **)mallocN(6*4*vv->vlak,"edlist");
			for(a=0;a<vv->vlak;a++)	{
				eve1= evlist[adrvl->v1];
				eve2= evlist[adrvl->v2];
				eve3= evlist[adrvl->v3];
				if(eve1>eve2) {
					*(eld++)= eve2; 
					*(eld++)= eve1;
				} else if(eve1<eve2) {
					*(eld++)= eve1; 
					*(eld++)= eve2;
				}
				if(eve2>eve3) {
					*(eld++)= eve3; 
					*(eld++)= eve2;
				} else if(eve2<eve3) {
					*(eld++)= eve2; 
					*(eld++)= eve3;
				}
				if(eve3>eve1) {
					*(eld++)= eve1; 
					*(eld++)= eve3;
				} else if(eve3<eve1) {
					*(eld++)= eve3; 
					*(eld++)= eve1;
				}
				evl= addvlaklistZE(eve1,eve2,eve3,(adrvl->ec & 31));
				adrvl++;
			}
			/* qsorten en edges maken */
			toted= ( ((long)eld)- ((long)edlist) )/8;

			qsortN2(edlist,toted);

			eld= edlist;
			for(a=0;a<toted;a++) {
				if(eld[0]!=0 && eld[1]!=0) {
					if(a==0 || ( eld[-2]!=eld[0] || eld[-1]!=eld[1] ) ) {
						addedgeSimp(eld[0],eld[1]);
					}
				}
				eld+=2;
			}

			freeN(edlist);
		}

printf("time %d\n", clock()-tijd);

		freeN(evlist);
	}

	setcursorN(1);
}

void load_ebasedata()
{
	struct ListBase tempbase;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct EditVert *eve;
	struct EditEdge *eed;
	struct EditVlak *evl;
	struct Base *base;
	struct ObData *ob, *obtemp;
	struct VV *vv,*vvn;
	struct Key *key=0, *prevk, *newk;
	struct Bezier *bez;
	float fac,min[3],max[3],cent[3],f1, *fp;
	long tot=0,a,len;
	short c, diffvert=0;
	char *obdup1;

	if(G.ebase==0) return;
	lasted= 0;

	setcursorN(2);
	countall();	/* alles eerst tellen ivm lengte datablok */
	
	if(G.totvlak>65000) {
		error("Too many faces");
		setcursorN(1);
		return;
	}
	if(G.totvert>65000) {
		error("Too many vertices");
		setcursorN(1);
		return;
	}

	if(G.ebase->soort==1) {
		/* zijn er vertex keys? 
		 * zo ja: doe dan alles als gewoonlijk, maak een nieuwe
		 * keypos en interpoleer weer de vertices.
		 * Is wat omslachtig,  maar wel de helderste methode.
		 * Als verschillend aantal vertices: twee gevallen: wel of geen actkey.
		 */
		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		if(vv->key) {
			if(G.totvert!= vv->vert) {
				diffvert= 1;
			}
			if(G.ipomode==22 && G.actkey) {
				key= vv->key;
				a= 1;
				while(key) {
					if(a== G.actkey) break;
					a++;
					key= key->next;
				}
			}
		}

		/* tellen of edges niet in vlakken zitten, en array maken */
		maakedsortblock(1); /* (1) zet edgeflaggen op nul */
		evl= (struct EditVlak *)G.edvl.first;
		while(evl) {
			eed= findedgelist(evl->v1,evl->v2);
			eed->f= 1;
			eed= findedgelist(evl->v2,evl->v3);
			eed->f= 1;
			eed= findedgelist(evl->v3,evl->v1);
			eed->f= 1;
			evl= evl->next;
		}
		if(edsortblock) freeN(edsortblock);
		edsortblock= 0;

		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			if(eed->f==0) G.totvlak++;
			eed= eed->next;
		}

		/* nieuw vv datablok */
		ob= (struct ObData *)G.ebase->d;
		if(ob->floatverts) freeN(ob->floatverts);
		ob->floatverts= 0;
		vv= ob->vv;
		len= sizeof(struct VV)+G.totvert*sizeof(struct VertOb)
		    +G.totvlak*sizeof(struct VlakOb);
		vvn= (struct VV *)callocN(len,"loadebasedataVV");
		memcpy(vvn,vv,sizeof(struct VV));

		/* testen op users */
		if(vv->us>1) {
			if(okee("Copy data to duplicates")) {
				base= G.firstbase;
				while(base) {
					if(base->soort==1) {
						ob= (struct ObData *)base->d;
						if(ob->vv== vv) ob->vv= vvn;
						if(ob->floatverts) freeN(ob->floatverts);
						ob->floatverts= 0;
					}
					base= base->next;
				}
				freeN(vv);
			}
			else {
				vv->us--;
				vvn->us= 1;
				obdup1= obdup;
				base= G.firstbase;
				while(base) {
					if(base!=G.ebase && base->soort==1) {
						ob= (struct ObData *)base->d;
						if(ob->vv== vv) {
							tempbase= ob->disp;
							freeN(ob);
							ob= (struct ObData *)mallocN(oblen,"newObblock");
							memcpy(ob, obdup1, oblen);
							ob->disp= tempbase;
							base->d= (long *)ob;
							obdup1+=oblen;
						}
					}
					else if(base==G.ebase) obdup1+=oblen;

					base= base->next;
				}
				if(vv->key) {	/* dupliceer keys */
					key= vv->key;
					prevk= 0;
					len= sizeof(struct Key)+ 6*2*vv->vert+4;
					while(key) {
						
						newk= (struct Key *)mallocN(len,"loadebasenewKey1");
						memcpy(newk, key, len);
						
						if(prevk==0) {	/* eerste */
							vvn->key= newk;
							prevk= key;
						}
						else {
							prevk->next= newk;
							newk->next= 0;
							prevk= newk;
						}
						key= key->next;
					}
				}
			}
		}
		else freeN(vv);

		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv= vvn;
		vv->vert= G.totvert;
		vv->vlak= G.totvlak;

		/* opnieuw kopie van ObData en ColBlck blokken */
		make_obdups();

		/* wordruimte */
		adrve= (struct VertOb *)(vv+1);
		min[0]=min[1]=min[2]= 1.0e20;
		max[0]=max[1]=max[2]= -1.0e20;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			for(c=0;c<3;c++) {
				if(eve->co[c]>max[c]) max[c]= eve->co[c];
				if(eve->co[c]<min[c]) min[c]= eve->co[c];
			}
			eve= eve->next;
		}
		max[0]= MAX2(fabs(min[0]),fabs(max[0]));
		max[1]= MAX2(fabs(min[1]),fabs(max[1]));
		max[2]= MAX2(fabs(min[2]),fabs(max[2]));
		fac= MAX3(max[0],max[1],max[2]);
		cent[0]=cent[1]=cent[2]= 0.0;

		vv->afm[0]= vv->ws*max[0];
		vv->afm[1]= vv->ws*max[1];
		vv->afm[2]= vv->ws*max[2];

		if(fac>1.0e+9 || fac<1.0e-3) fac= 1.0;
		else {
			if(key) {
				fac/=32767.0;	/* waarom stond hier ooit 16384? Vanwege Bspline! */ 
				fp= (float *)(key+1);
				(*fp)*= fac;
			}
			else {
				fac/=32767;
				vv->ws*=fac;
			}
		}

		/* de vertices */
		eve= (struct EditVert *)G.edve.first;
		a=0;
		while(eve) {
			for(c=0;c<3;c++) {
				eve->co[c]= (eve->co[c]-cent[c])/fac;
				/* hierboven moet eerst als ebasedata blijft
				   bestaan (b.v. tijdens render) */
				adrve->c[c]= (short)( eve->co[c] );
			}
			eve->f1= a++;  /* teller */
			eve= eve->next;
			adrve++;
		}

		/* de vlakken */
		adrve= (struct VertOb *)(vv+1);
		adrvl= (struct VlakOb *)(adrve+vv->vert);
		evl= (struct EditVlak *)G.edvl.first;
		while(evl) {
			adrvl->v1= evl->v1->f1;
			adrvl->v2= evl->v2->f1;
			adrvl->v3= evl->v3->f1;
			adrvl->f= 0;
			adrvl->ec= evl->f1;
			adrvl++;
			evl= evl->next;
		}
		/* losse edges als vlak toevoegen */
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			if( eed->f==0 ) {
				adrvl->v1= eed->v1->f1;	    /* v1==v2 wordt door initrender afgevangen */
				adrvl->v2= eed->v1->f1;
				adrvl->v3= eed->v2->f1;
				adrvl->ec= 0;
				adrvl++;
			}
			eed= eed->next;
		}
		adrve= (struct VertOb *)(vv+1);
		adrvl= (struct VlakOb *)(adrve+vv->vert);

		normalen(adrve,adrvl,vv->vert,vv->vlak);

		if(editbutflag & 8) edgesnew(adrvl,vv->vert,vv->vlak);
		else edges(adrvl,vv->vert,vv->vlak);

		if(key && diffvert==0) {
			fp= (float *)(key+1);
			memcpy(fp+1, vv+1, 2*6*vv->vert);
			/* loadkeypos(G.ebase, G.ebase); */
		}
		else if(diffvert) {
			if( key) {
				/* wspace uit key halen */
				fp= (float *)(key+1);
				vv->ws= *fp;
			}
			key= vv->key;
			prevk= 0;
			len= sizeof(struct Key)+2*6*vv->vert+4;
			while(key) {
				newk= (struct Key *)mallocN(len, "loadebase4");
				memcpy(newk, key, sizeof(struct Key));
				fp= (float *)(newk+1);
				*fp= vv->ws;
				memcpy(fp+1, vv+1, 2*6*vv->vert);
				freeN(key);

				if(prevk==0) vv->key= newk;
				else prevk->next= newk;
				prevk= newk;

				key= newk->next;
			}
		}
		
			/* displisten van alle users, ook deze base */
		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		base= G.firstbase;
		while(base) {
			if(base->soort==1 && base->lay & view0.lay) {
				ob= (struct ObData *)base->d;
				if(ob->vv == vv) {
					makeDispList(base);
					if(base->skelnr) calc_deform(base);
				}
			}
			base= base->next;
		}

		/* tijdelijk: controleer aantal users
		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		if(vv->us>1) {
			tot= 0;
			base= G.firstbase;
			while(base) {
				if(base->soort==1) {
					obtemp= (struct ObData *)base->d;
					if(obtemp->vv == vv) tot++;
				}
				base= base->next;
			}
			if(tot!= vv->us) {
				error("Iets mis met users, lees console");
				printf("ERROR in aantal users: object %s %d\n", G.ebase->str, G.ebase->nnr);
				printf("Getelde users %d, in vv: %d\n", tot, vv->us);
				printf("Wat heb je gedaan en kun je het herhalen?\n");
				printf("(Fout is wel gerepareerd)\n");
				vv->us= tot;
			}
		}
		*/
		 
	}

	

	setcursorN(1);
}

void remake_ebasedata()
{
	struct ObData *ob;
	struct VV *vv;
	struct Base *base;
	char *obduptemp=0;

	if(G.ebase==0) return;
	if(okee("Reload Original data")==0) return;

	if (G.ebase->soort==1) {
		/* de de obdups terugkopieeren */
		obduptemp= obdup;

		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		base= G.firstbase;
		while(base) {
			if(base->soort==1) {
				ob= (struct ObData *)base->d;
				if(ob->vv==vv) {
					ob= mallocN(oblen, "remake_ebase");
					memcpy(ob, obduptemp, oblen);
					freeN(base->d);
					base->d= (long *)ob;
					obduptemp+= oblen;
				}
			}
			base= base->next;
		}
		make_ebasedata();
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		ob= mallocN(oblen, "remake_ebase");
		memcpy(ob, obdup, oblen);
		freeN(G.ebase->d);
		G.ebase->d= (long *)ob;

		make_ebaseNurb();
		makeDispList(G.ebase);
	}
	else if(G.ebase->soort== -2) {
		make_ebaseNurb();
		makeDispList(G.ebase);
	}
	else if(G.ebase->soort== -4) {
		make_ebaseIka();
	}
	
	projektie();
	if(G.mainb==9) tekeneditbuts(1);
	if(G.mainb==5) tekenobjbuts(1);
}

void clearDisplistObdups()
{
	struct ObData *ob;
	char *obduptemp=0;
	int a, tot;
	
	if(G.ebase==0) return;
	ob= (struct ObData *)G.ebase->d;
	tot= ob->vv->us;
	
	obduptemp= obdup;
	for(a=0; a<tot; a++) {
		ob= (struct ObData *)obduptemp;
		ob->disp.first= ob->disp.last= 0;
		obduptemp+= oblen;
	}
}

/* *********************  TOOLS  ********************* */

void righthandfaces()	/* maakt vlakken rechtsdraaiend */
{
	struct EditVert *eve;
	struct EditEdge *eed, *ed1, *ed2, *ed3, **edpoin;
	struct EditVlak *evl, *startvl;
	float minx, miny, minz, cent[3];
	long totsel, found, foundone, direct, turn;

	/* in deze routine wordt tijdelijk in vlaknormaal de edgepointers opgeslagen;
    /* op basis selectconnected om losse objecten te onderscheiden */

	/* tel per edge hoeveel vlakken het heeft */

	/* vind het meest linkse, voorste, bovenste vlak */

	/* zet normaal naar buiten en de eerste richtings vlaggen in de edges */

	/* loop object af en zet richtingen / richtingsvlaggen: alleen bij edges van 1 of 2 vlakken */
	/* dit is in feit de select connected */

	/* indien nog (selected) vlakken niet gedaan: opnieuw vind de meest linkse ... */

	maakedsortblock(1); /* (1) zet edgeflaggen op nul */

	/* vlakken en edges tellen */
	totsel= 0;
	evl= G.edvl.first;
	while(evl) {
		if( (evl->v1->f & 1) && (evl->v2->f & 1) && (evl->v3->f & 1) ) {
			evl->f= 1;
			totsel++;
			edpoin= (struct EditEdge **)evl->n;
			eed= findedgelist(evl->v1, evl->v2);
			eed->f1++;
			edpoin[0]= eed;
			eed= findedgelist(evl->v2, evl->v3);
			eed->f1++;
			edpoin[1]= eed;
			eed= findedgelist(evl->v3, evl->v1);
			eed->f1++;
			edpoin[2]= eed;
		}
		else evl->f= 0;

		evl= evl->next;
	}

	while(totsel>0) {
		/* de meest linkse, bovenste... */

		evl= G.edvl.first;
		startvl= 0;
		minx= miny= minz= 1.0e10;

		while(evl) {
			if(evl->f) {
				CalcCent3f(cent, evl->v1->co, evl->v2->co, evl->v3->co);

				if(cent[0]==minx) {
					if(cent[1]==miny) {
						if(cent[2]<minz) {
							minz= cent[2];
							startvl= evl;
						}
					}
					else if(cent[1]<miny) {
						miny= cent[1];
						startvl= evl;
					}
				}
				else if(cent[0]<minx) {
					minx= cent[0];
					startvl= evl;
				}
			}
			evl= evl->next;
		}

		/* eerste vlak goedzetten: normaal berekenen vanwege edgepointers */
		CalcNormFloat(startvl->v1->co, startvl->v2->co, startvl->v3->co, cent);
		if(cent[0]+ cent[1]+ cent[2] > 0.0) {
			eve= startvl->v2;
			startvl->v2= startvl->v3;
			startvl->v3= eve;
		}

		eed= findedgelist(startvl->v1, startvl->v2);
		if(eed->v1==startvl->v1) eed->f= 1; 
		else eed->f= 2;
		eed= findedgelist(startvl->v2, startvl->v3);
		if(eed->v1==startvl->v2) eed->f= 1; 
		else eed->f= 2;
		eed= findedgelist(startvl->v3, startvl->v1);
		if(eed->v1==startvl->v3) eed->f= 1; 
		else eed->f= 2;
		startvl->f= 0;
		totsel--;

		/* de normalen testen */
		found= 1;
		direct= 1;
		while(found) {
			found= 0;
			if(direct) evl= G.edvl.first;
			else evl= G.edvl.last;
			while(evl) {
				if(evl->f) {
					turn= 0;
					foundone= 0;

					edpoin= (struct EditEdge **)evl->n;
					ed1= edpoin[0];
					ed2= edpoin[1];
					ed3= edpoin[2];

					if(ed1->f) {
						if(ed1->v1==evl->v1 && ed1->f==1) turn= 1;
						if(ed1->v2==evl->v1 && ed1->f==2) turn= 1;
						foundone= 1;
					}
					else if(ed2->f) {
						if(ed2->v1==evl->v2 && ed2->f==1) turn= 1;
						if(ed2->v2==evl->v2 && ed2->f==2) turn= 1;
						foundone= 1;
					}
					else if(ed3->f) {
						if(ed3->v1==evl->v3 && ed3->f==1) turn= 1;
						if(ed3->v2==evl->v3 && ed3->f==2) turn= 1;
						foundone= 1;
					}

					if(foundone) {
						found= 1;
						totsel--;
						evl->f= 0;

						if(turn) {
							if(ed1->v1==evl->v1) ed1->f= 2; 
							else ed1->f= 1;
							if(ed2->v1==evl->v2) ed2->f= 2; 
							else ed2->f= 1;
							if(ed3->v1==evl->v3) ed3->f= 2; 
							else ed3->f= 1;

							eve= evl->v2;
							evl->v2= evl->v3;
							evl->v3= eve;

						}
						else {
							if(ed1->v1==evl->v1) ed1->f= 1; 
							else ed1->f= 2;
							if(ed2->v1==evl->v2) ed2->f= 1; 
							else ed2->f= 2;
							if(ed3->v1==evl->v3) ed3->f= 1; 
							else ed3->f= 2;
						}
					}
				}
				if(direct) evl= evl->next;
				else evl= evl->prev;
			}
			direct= 1-direct;
		}
	}

	if(edsortblock) freeN(edsortblock);
	edsortblock= 0;

	recalc_editnormals();
}

struct EditVert *findnearestvert(sel)
short sel;
{
	/* als sel==1 krijgen vertices met flag==1 een nadeel */
	struct EditVert *eve,*act=0;
	static struct EditVert *acto=0;
	short dist=100,temp,mval[2];

	if(G.edve.first==0) return 0;

	winset(G.winar[0]);

	/* er wordt geteld van acto->next tot last en van first tot acto */
	/* bestaat acto ? */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve==acto) break;
		eve= eve->next;
	}
	if(eve==0) acto=(struct EditVert *)G.edve.first;

	if(acto==0) return 0;

	/* is er een aangegeven vertex? deel 1 */
	getmouseco(mval);
	eve= acto->next;
	while(eve) {
		if(eve->h==0) {
			temp= abs(mval[0]- eve->xs)+ abs(mval[1]- eve->ys);
			if( (eve->f & 1)==sel ) temp+=5;
			if(temp<dist) {
				act= eve;
				dist= temp;
				if(dist<4) break;
			}
		}
		eve= eve->next;
	}
	/* is er een aangegeven vertex? deel 2 */
	if(dist>3) {
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->h==0) {
				temp= abs(mval[0]- eve->xs)+ abs(mval[1]- eve->ys);
				if( (eve->f & 1)==sel ) temp+=5;
				if(temp<dist) {
					act= eve;
					if(temp<4) break;
					dist= temp;
				}
				if(eve== acto) break;
			}
			eve= eve->next;
		}
	}

	acto= act;
	return act;
}


void muis_ebase()
{
	void vertgrabber();
	extern ulong rectyellow[5][5],rectpurple[5][5];
	struct EditVert *eve,*act=0;
	long a;
	short xs,ys,mval[2], dist=100,temp;

	if(G.zbuf) zbuffer(0);

	act= findnearestvert(1);
	if(act) {
		frontbuffer(1);
		backbuffer(0);
		if((G.qual & 3)==0) {
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve!=act) {
					eve->f&= -2;
					if(eve->xs!=3200) {
						xs= eve->xs;
						ys= eve->ys;
						lrectwrite(xs-1,ys-1,xs+1,ys,rectpurple);
					}
				}
				eve= eve->next;
			}
		}
		if( (act->f & 1)==0) act->f+= 1;
		else if(G.qual & 3) act->f-= 1;

		if(act->xs!=3200) {
			xs= act->xs;
			ys= act->ys;
			if(act->f & 1) lrectwrite(xs-1,ys-1,xs+1,ys,rectyellow);
			else lrectwrite(xs-1,ys-1,xs+1,ys,rectpurple);
		}
		frontbuffer(0);
		backbuffer(1);
		countall();
		tekenoverdraw(0);
	}

	if(G.zbuf) zbuffer(1);

	getmouseco(mval);
	xs= mval[0]; 
	ys= mval[1];
	while(getbutton(RIGHTMOUSE)) {
		gsync();
		getmouseco(mval);
		if(abs(mval[0]-xs)+abs(mval[1]-ys) > 10) {
			vertgrabber();
			while(getbutton(RIGHTMOUSE))  gsync();
		}
	}
}

void selectconnectedAll()
{
	struct EditVert *eve,*v1,*v2,*act= 0;
	struct EditEdge *eed;
	short flag=1,sel,toggle=0;

	if(G.eded.first==0) return;

	while(flag==1) {
		flag= 0;
		toggle++;
		if(toggle & 1) eed= (struct EditEdge *)G.eded.first;
		else eed= (struct EditEdge *)G.eded.last;
		while(eed) {
			v1= eed->v1;
			v2= eed->v2;
			if(eed->h==0) {
				if(v1->f & 1) {
					if( (v2->f & 1)==0 ) {
						v2->f |= 1;
						flag= 1;
					}
				}
				else if(v2->f & 1) {
					if( (v1->f & 1)==0 ) {
						v1->f |= 1;
						flag= 1;
					}
				}
			}
			if(toggle & 1) eed= eed->next;
			else eed= eed->prev;
		}
	}
	winset(G.winar[0]);
	countall();
	tekenoverdraw(0);

	tekenvertices_ext();

}


void selectconnected()
{
	struct EditVert *eve,*v1,*v2,*act= 0;
	struct EditEdge *eed;
	short flag=1,sel,toggle=0;

	if(G.eded.first==0) return;

	if(G.qual > 3) {
		selectconnectedAll();
		return;
	}

	sel= 3;
	if(G.qual & 3) sel=2;

	act= findnearestvert(sel-2);
	if(act==0) {
		error(" Nothing indicated ");
		return;
	}

	/* testflaggen wissen */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		eve->f&= ~2;
		eve= eve->next;
	}
	act->f= (act->f & ~3) | sel;

	while(flag==1) {
		flag= 0;
		toggle++;
		if(toggle & 1) eed= (struct EditEdge *)G.eded.first;
		else eed= (struct EditEdge *)G.eded.last;
		while(eed) {
			v1= eed->v1;
			v2= eed->v2;
			if(eed->h==0) {
				if(v1->f & 2) {
					if( (v2->f & 2)==0 ) {
						v2->f= (v2->f & ~3) | sel;
						flag= 1;
					}
				}
				else if(v2->f & 2) {
					if( (v1->f & 2)==0 ) {
						v1->f= (v1->f & ~3) | sel;
						flag= 1;
					}
				}
			}
			if(toggle & 1) eed= eed->next;
			else eed= eed->prev;
		}
	}
	winset(G.winar[0]);
	countall();
	tekenoverdraw(0);
	
	tekenvertices_ext();
}

short extrudeflag(flag,type)
short flag,type;
{
	/* als type=1 worden oude extrudevlakken verwijderd (ivm spin etc) */
	/* alle verts met (flag & 'flag') extrude */
	/* van oude wordt flag 'flag' gewist, van nieuwe gezet */

	struct EditVert *eve,*v1,*v2,*v3,*nextve;
	struct EditEdge *eed,*e1,*e2,*e3,*nexted;
	struct EditVlak *evl,*nextvl,*evln;
	static long toggle=0;
	short sel=0,a0=0,a1=0,a2=0,a3=0,deloud= 0;

	if(G.ebase==0 || G.ebase->soort!=1) return 0;


	/* de vert flag f1 wissen, hiermee test op losse geselecteerde vert */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & flag) eve->f1= 1;
		else eve->f1= 0;
		eve= eve->next;
	}
	/* de edges tellerflag wissen, als selected op 1 zetten */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if( (eed->v1->f & flag) && (eed->v2->f & flag) ) {
			eed->f= 1;
			eed->v1->f1= 0;
			eed->v2->f1= 0;
		}
		else eed->f= 0;
		eed->f1=0;
		eed= eed->next;
	}


	/* in alle vlak sel een dupl.flag zetten en bijhorende edgeflags ophogen */
	maakedsortblock(0); /* (1) zet edgeflaggen op nul */
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		evl->f= 0;
		sel= 0;
		if(evl->v1->f & flag) sel++;
		if(evl->v2->f & flag) sel+=2;
		if(evl->v3->f & flag) sel+=4;
		if(sel==3) {
			e1= findedgelist(evl->v1,evl->v2);
			e1->f1++;
		} else if(sel==6) {
			e2= findedgelist(evl->v2,evl->v3);
			e2->f1++;
		} else if(sel==5) {
			e3= findedgelist(evl->v3,evl->v1);
			e3->f1++;
		} else if(sel==7) {
			e1= findedgelist(evl->v1,evl->v2);
			e2= findedgelist(evl->v2,evl->v3);
			e3= findedgelist(evl->v3,evl->v1);
			if(e1==0 || e2==0 || e3==0);
			else {
				e1->f++;
				e2->f++;
				e3->f++;
				evl->f= 1;
			}
		}
		evl= evl->next;
	}
	if(edsortblock) freeN(edsortblock);
	edsortblock= 0;

	/* de stand van zaken nu:
		eve->f1==1: losse selected vertex 

		eed->f==0 : edge niet selected, geen extrude
		eed->f==1 : edge selected, komt niet in vlak voor, extrude
		eed->f==2 : edge selected, komt 1 keer in vlak voor, extrude
		eed->f>=3 : edge selected, komt in meer vlakken voor, geen extrude

		eed->f1>0 : edge selected, komt in NIET selected vlak voor
			    als eed->f1>0 && eed->f==2 voorkomt: oude vlak weg
		evl->f==1 : vlak dupliceren
	*/

	/* alle geselecteerde vertices kopieeren, */
	/* de pointer naar nieuwe vert in oude struct schrijven op eve->vn */
	eve= (struct EditVert *)G.edve.last;
	while(eve) {
		eve->f&= ~128;  /* wissen voor test later op losse verts */
		if(eve->f & flag) {
			sel= 1;
			v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
			addtail(&G.edve,v1);
			VECCOPY(v1->co,eve->co);
			v1->f= eve->f;
			eve->f-= flag;
			eve->vn= v1;
		}
		else eve->vn= 0;
		eve= eve->prev;
	}

	if(sel==0) return 0;

	/* alle edges met eed->f==1 of eed->f==2 worden vlakken */
	/* als deloud==1 worden edges eed->f>2 verwijderd */
	toggle++;
	eed= (struct EditEdge *)G.eded.last;
	while(eed) {
		nexted= eed->prev;
		if( eed->f<3) {
			eed->v1->f|=128;  /* =geen losse vert! */
			eed->v2->f|=128;
		}
		if( (eed->f==1 || eed->f==2) ) {
			if(eed->f1) deloud=1;

			v1= eed->v1->vn;
			v2= eed->v2->vn;
			if(toggle & 1) {
				addvlakSimp(eed->v1,v1,v2,G.colact-1);
				addvlakSimp(eed->v1,v2,eed->v2,G.colact-1);
			}
			else {
				addvlakSimp(eed->v2,eed->v1,v1,G.colact-1);
				addvlakSimp(eed->v2,v1,v2,G.colact-1);
			}
		}

		eed= nexted;
	}
	if(deloud) {
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			nexted= eed->next;
			if(eed->f>2 && eed->f1==0) {
				remedge(eed);
				free(eed);
			}
			eed= nexted;
		}
	}
	/* de vlakken dupliceren, eventueel oude verwijderen  */
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		nextvl= evl->next;
		if(evl->f & 1) {
			v1= evl->v1->vn;
			v2= evl->v2->vn;
			v3= evl->v3->vn;
			evln= addvlakSimp(v1,v2,v3,evl->f1);
			if(deloud) {
				remlink(&G.edvl,evl);
				free(evl);
			}
		}
		evl= nextvl;
	}
	/* alle verts met eve->vn!=0 
		als eve->f1==1: edge maken
		als flag!=128 :als deloud==1:  verwijderen
	*/
	eve= (struct EditVert *)G.edve.last;
	while(eve) {
		nextve= eve->prev;
		if(eve->vn) {
			if(eve->f1==1) addedgeSimp(eve,eve->vn);
			else if( (eve->f & 128)==0) {
				if(deloud) {
					remlink(&G.edve,eve);
					free(eve);
				}
			}
		}
		eve= nextve;
	}

	remake_edgelist();  /* ivm Simp */

	return 1;
}

void rotateflag(flag,cent,rotmat)
short flag;
float *cent,*rotmat;
{
	/* alle verts met (flag & 'flag') rotate */

	struct EditVert *eve;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & flag) {
			eve->co[0]-=cent[0];
			eve->co[1]-=cent[1];
			eve->co[2]-=cent[2];
			Mat3MulVecfl(rotmat,eve->co);
			eve->co[0]+=cent[0];
			eve->co[1]+=cent[1];
			eve->co[2]+=cent[2];
		}
		eve= eve->next;
	}
}

void translateflag(flag,vec)
short flag;
float *vec;
{
	/* alle verts met (flag & 'flag') translate */

	struct EditVert *eve;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & flag) {
			eve->co[0]+=vec[0];
			eve->co[1]+=vec[1];
			eve->co[2]+=vec[2];
		}
		eve= eve->next;
	}
}

short removedoublesflag(flag,limit)		/* return aantal */
short flag;
float limit;
{
	/* alle verts met (flag & 'flag') worden getest */
	struct EditVert *eve,*v1,*nextve,*vn;
	struct EditEdge *eed,*e1,*nexted;
	struct EditVlak *evl,*nextvl;
	struct xvertsort *sortblock,*sb,*sb1;
	float dist,cx,cy,cz;
	short a,b,test,aantal;

	/* flag 128 wordt gewist, aantal tellen */
	eve= (struct EditVert *)G.edve.first;
	aantal= 0;
	while(eve) {
		eve->f&= ~128;
		aantal++;
		eve= eve->next;
	}
	if(aantal==0) return 0;

	/* geheugen reserveren en qsorten */
	sb= sortblock= (struct xvertsort *)mallocN(8*aantal,"sortremovedoub");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		sb->x= eve->co[0];
		sb->v1= eve;
		eve= eve->next;
		sb++;
	}
	qsort(sortblock,aantal,8,vergxco);

	/* testen op doubles */
	sb= sortblock;
	for(a=0;a<aantal;a++) {
		eve= sb->v1;
		if( (eve->f & 128)==0 && (eve->f & flag) ) {
			sb1= sb+1;
			for(b=a+1;b<aantal;b++) {
				v1= sb1->v1;
				dist= fabs(v1->co[0]-eve->co[0]);
				if(dist<limit) {
					if( (v1->f & 128)==0 && (v1->f & flag) ) {
						dist= fabs(v1->co[1]-eve->co[1]);
						if(dist<=limit) {
							dist= fabs(v1->co[2]-eve->co[2]);
							if(dist<=limit) {
								v1->f|= 128;
								v1->vn= eve;
							}
						}
					}
				}
				else break;
				sb1++;
			}
		}
		sb++;
	}
	freeN(sortblock);

	/* edges testen en opnieuw invoegen */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		eed->f= 0;
		eed= eed->next;
	}
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		nexted= eed->next;
		if(eed->f==0) {
			if( (eed->v1->f & 128) || (eed->v2->f & 128) ) {
				if(eed->v1->f & 128) eed->v1= eed->v1->vn;
				if(eed->v2->f & 128) eed->v2= eed->v2->vn;
				remedge(eed);
				e1= addedgelist(eed->v1,eed->v2);
				if(e1) e1->f= 1;
				free(eed);
			}
		}
		eed= nexted;
	}

	/* vlakken testen en eventueel verwijderen */
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		nextvl= evl->next;
		test= 0;
		if(evl->v1->f & 128) {
			evl->v1= evl->v1->vn;
			test++;
		}
		if(evl->v2->f & 128) {
			evl->v2= evl->v2->vn;
			test++;
		}
		if(evl->v3->f & 128) {
			evl->v3= evl->v3->vn;
			test++;
		}
		if(test) {
			if(dubbelvlak(evl) ) {
				remlink(&G.edvl, evl);
				free(evl);
			}
			else {
				test= 0;
				if(evl->v1==evl->v2) test++;
				if(evl->v2==evl->v3) test++;
				if(evl->v3==evl->v1) test++;
				if(test) {
					remlink(&G.edvl,evl);
					free(evl);
				}
			}
		}
		evl= nextvl;
	}
	/* dubbels eruit */
	a= 0;
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		nextve= eve->next;
		if(eve->f & 128) {
			a++;
			remlink(&G.edve,eve);
			free(eve);
		}
		eve= nextve;
	}	
	
	return a;	/* aantal */
}

void xsortvert_flag(flag)
short flag;
{
	/* alle verts met (flag & 'flag') worden gesorteerd */
	struct EditVert *eve, *nextve;
	struct xvertsort *sortblock, *sb;
	ListBase tbase;
	int aantal;
	
	/* aantal tellen */
	eve= G.edve.first;
	aantal= 0;
	while(eve) {
		if(eve->f & flag) aantal++;
		eve= eve->next;
	}
	if(aantal==0) return;

	/* geheugen reserveren en qsorten */
	sb= sortblock= (struct xvertsort *)mallocN(8*aantal,"sortremovedoub");
	eve= G.edve.first;
	while(eve) {
		if(eve->f & flag) {
			sb->x= eve->co[0];
			sb->v1= eve;
			sb++;
		}
		eve= eve->next;
	}
	qsort(sortblock, aantal, 8, vergxco);
	
	/* tijdelijke listbase maken */
	tbase.first= tbase.last= 0;
	sb= sortblock;
	while(aantal--) {
		eve= sb->v1;
		remlink(&G.edve, eve);
		addtail(&tbase, eve);
		sb++;
	}
	
	addlisttolist(&G.edve, &tbase);
	
	freeN(sortblock);
}


void hashvert_flag(flag)
short flag;
{
	struct EditVert *eve;
	struct xvertsort *sortblock, *sb, onth, *new;
	ListBase tbase;
	float mult;
	int aantal, a, b;
	
	/* aantal tellen */
	eve= G.edve.first;
	aantal= 0;
	while(eve) {
		if(eve->f & flag) aantal++;
		eve= eve->next;
	}
	if(aantal==0) return;

	/* geheugen reserveren */
	sb= sortblock= (struct xvertsort *)mallocN(8*aantal,"sortremovedoub");
	eve= G.edve.first;
	while(eve) {
		if(eve->f & flag) {
			sb->v1= eve;
			sb++;
		}
		eve= eve->next;
	}

	srandom(1);
	mult= ((float)aantal)/32768.0;
	
	sb= sortblock;
	for(a=0; a<aantal; a++, sb++) {
		b= (int) (mult*( (float)(random() & 32767) ));
		if(b>=0 && b<aantal) {
			new= sortblock+b;
			onth= *sb;
			*sb= *new;
			*new= onth;
		}
	}

	/* tijdelijke listbase maken */
	tbase.first= tbase.last= 0;
	sb= sortblock;
	while(aantal--) {
		eve= sb->v1;
		remlink(&G.edve, eve);
		addtail(&tbase, eve);
		sb++;
	}
	
	addlisttolist(&G.edve, &tbase);
	
	freeN(sortblock);
}

void subdivideflag(flag,rad)
short flag;
float rad;
{
	/* divide alle vlakken met (vertflag & flag) */
	/* als rad>0.0 zet dan nieuw vert op afstand rad van 0,0,0 */
	struct EditVert *eve;
	struct EditEdge *eed,*e1,*e2,*e3,*nexted;
	struct EditVlak *evl;
	float fac,vec[3],vec1[3],VecLenf();
	short test;

	/* edges testen, nieuw punt maken en in edge wegschrijven, flag wissen */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		eed->f= 0;
		if( (eed->v1->f & flag) && (eed->v2->f & flag) ) {
			vec[0]= (eed->v1->co[0]+eed->v2->co[0])/2;
			vec[1]= (eed->v1->co[1]+eed->v2->co[1])/2;
			vec[2]= (eed->v1->co[2]+eed->v2->co[2])/2;
			if(rad>0.0) {   /* perf sph */
				Normalise(vec);
				vec[0]*= rad;
				vec[1]*= rad;
				vec[2]*= rad;
			}
			else if(rad< 0.0) {  /* fract */
				fac= rad* VecLenf(eed->v1->co,eed->v2->co)/32767.0;
				vec1[0]= fac*( (float)(random() & 32767) -16384);
				vec1[1]= fac*( (float)(random() & 32767) -16384);
				vec1[2]= fac*( (float)(random() & 32767) -16384);
				VecAddf(vec,vec,vec1);
			}
			eed->vn= addvertlist(vec);
			eed->vn->f= eed->v1->f;
		}
		else eed->vn= 0;
		eed= eed->next;
	}
	/* alle vlakken testen op subdiv edges, drie gevallen! */
	maakedsortblock(0);
	evl= (struct EditVlak *)G.edvl.last;
	while(evl) {
		if( (evl->v1->f & flag) || (evl->v2->f & flag) || (evl->v2->f & flag) ) {
			e1= findedgelist(evl->v1,evl->v2);
			e2= findedgelist(evl->v2,evl->v3);
			e3= findedgelist(evl->v3,evl->v1);
			test= 0;
			if(e1 && e1->vn) { 
				test+=1; 
				e1->f=1;
			}
			if(e2 && e2->vn) { 
				test+=2; 
				e2->f=1;
			}
			if(e3 && e3->vn) { 
				test+=4; 
				e3->f=1;
			}
			if(test) {
				if((test & 3)==3) addvlakSimp(evl->v2,e2->vn,e1->vn,evl->f1);
				if((test & 6)==6) addvlakSimp(evl->v3,e3->vn,e2->vn,evl->f1);
				if((test & 5)==5) addvlakSimp(evl->v1,e1->vn,e3->vn,evl->f1);
				if(test==7) {  /* vier nieuwe vlakken, oude vernieuwt */
					evl->v1= e1->vn;
					evl->v2= e2->vn;
					evl->v3= e3->vn;
				}
				else if(test==3) {
					addvlakSimp(e1->vn,e2->vn,evl->v3,evl->f1);
					evl->v2= e1->vn;
				}
				else if(test==6) {
					addvlakSimp(e2->vn,e3->vn,evl->v1,evl->f1);
					evl->v3= e2->vn;
				}
				else if(test==5) {
					addvlakSimp(e3->vn,e1->vn,evl->v2,evl->f1);
					evl->v1= e3->vn;
				}
				else if(test==1) {
					addvlakSimp(e1->vn,evl->v2,evl->v3,evl->f1);
					evl->v2= e1->vn;
					addedgeSimp(evl->v1,evl->v2);
				}
				else if(test==2) {
					addvlakSimp(e2->vn,evl->v3,evl->v1,evl->f1);
					evl->v3= e2->vn;
					addedgeSimp(evl->v2,evl->v3);
				}
				else if(test==4) {
					addvlakSimp(e3->vn,evl->v1,evl->v2,evl->f1);
					evl->v1= e3->vn;
					addedgeSimp(evl->v3,evl->v1);
				}
			}
		}
		evl= evl->prev;
	}
	if(edsortblock) freeN(edsortblock);
	edsortblock= 0;

	/* alle oude edges verwijderen, eventueel nog nieuwe maken */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		nexted= eed->next;
		if( eed->vn ) {
			if(eed->f==0) {  /* niet gebruikt in vlak */
				addedgeSimp(eed->v1,eed->vn);
				addedgeSimp(eed->vn,eed->v2);
			}
			remedge(eed);
			free(eed);
		}
		eed= nexted;
	}

	remake_edgelist();	/* vanwege addvlakSimp) */

}

void adduplicateflag(flag)
short flag;
{
	/* oude verts hebben flag 128 gezet en flag 'flag' gewist
	   nieuwe verts hebben flag 'flag' gezet */
	struct EditVert *eve,*v1,*v2,*v3;
	struct EditEdge *eed,*e1,*e2,*e3;
	struct EditVlak *evl;

	/* eerst vertices */
	eve= (struct EditVert *)G.edve.last;
	while(eve) {
		eve->f&= ~128;
		if(eve->f & flag) {
			v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
			addtail(&G.edve,v1);
			VECCOPY(v1->co,eve->co);
			v1->f= eve->f;
			eve->f-= flag;
			eve->f|= 128;
			eve->vn= v1;
		}
		eve= eve->prev;
	}
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if( (eed->v1->f & 128) && (eed->v2->f & 128) ) {
			v1= eed->v1->vn;
			v2= eed->v2->vn;
			addedgeSimp(v1,v2);
		}
		eed= eed->next;
	}

	/* tenslotte de vlakken dupliceren */
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		if( (evl->v1->f & 128) && (evl->v2->f & 128) && (evl->v3->f & 128) ) {
			v1= evl->v1->vn;
			v2= evl->v2->vn;
			v3= evl->v3->vn;
			addvlakSimp(v1,v2,v3,evl->f1);
		}
		evl= evl->next;
	}

	remake_edgelist();	/* vanwege Simp() */
}

void delvlakflag(flag)
short flag;
{
	/* alle vlak 3 verts flag + edges + losse vertices deleten */
	/* van alle verts wordt 'flag' gewist */
	struct EditVert *eve,*nextve;
	struct EditEdge *eed;
	struct EditVlak *evl,*nextvl;
	short test;

	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		nextvl= evl->next;
		if( evl->v1->f & flag) {
			if( evl->v2->f & flag) {
				if( evl->v3->f & flag) {
					eed= findedgelist(evl->v1,evl->v2);
					if(eed) {
						remedge(eed);
						free(eed);
					}
					eed= findedgelist(evl->v2,evl->v3);
					if(eed) {
						remedge(eed);
						free(eed);
					}
					eed= findedgelist(evl->v3,evl->v1);
					if(eed) {
						remedge(eed);
						free(eed);
					}
					remlink(&G.edvl,evl);
					free(evl);
				}
			}
		}
		evl= nextvl;
	}
	/* alle vlakken 1,2 verts select weer edges maken */
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		test= 0;
		if(evl->v1->f & flag) test++;
		if(evl->v2->f & flag) test+=2;
		if(evl->v3->f & flag) test+=4;

		if(test & 3) addedgelist(evl->v1,evl->v2);
		if(test & 6) addedgelist(evl->v2,evl->v3);
		if(test & 5) addedgelist(evl->v3,evl->v1);

		evl= evl->next;
	}
	/* alle edges testen op vertices met flag en wissen */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if( (eed->v1->f & flag) || (eed->v2->f & flag) ) {
			eed->v1->f&= ~flag;
			eed->v2->f&= ~flag;
		}
		eed= eed->next;
	}
	/* vertices met flag nog gezet zijn losse en worden gewist */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		nextve= eve->next;
		if(eve->f & flag) {
			remlink(&G.edve,eve);
			free(eve);
		}
		eve= nextve;
	}

}

void extrudeVV()
{
	void vertgrabber();
	struct EditVert *eve,*nextve;
	short a;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	if(okee("Extrude")==0) return;

	setcursorN(2);

	a= extrudeflag(1,1);
	setcursorN(1);
	if(a==0) {
		error("Can't extrude");
	}
	else {
		countall();  /* voor G.totvert in calc_ebaseverts() */
		winset(G.winar[0]);
		frontbuffer(0);
		backbuffer(1);
		calc_ebaseverts();
		vertgrabber();
	}
}

void adduplicateVV()
{
	void vertgrabber();

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	winset(G.winar[0]);
	setcursorN(2);
	adduplicateflag(1);
	setcursorN(1);

	countall();  /* voor G.totvert in calc_ebaseverts() */
	calc_ebaseverts();
	vertgrabber();
}

void splitVV()
{

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	if(okee(" Split ")==0) return;
	winset(G.winar[0]);

	setcursorN(2);

	/* eerste duplicate maken */
	adduplicateflag(1);
	/* oude vlakken hebben 3x flag 128 gezet, deze deleten */
	delvlakflag(128);

	setcursorN(1);

	countall();
	projektie();
}

void separateVV()
{
	struct EditVert *eve, *v1;
	struct EditEdge *eed, *e1;
	struct EditVlak *evl, *vl1;
	struct Base *oldbase, *base;
	struct ObData *ob;
	struct VV *vv;
	struct ListBase edve, eded, edvl;
	int ok, flag;
	
	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	if(okee("Separate")==0) return;
	winset(G.winar[0]);

	setcursorN(2);
	
	ob= (struct ObData *)G.ebase->d;
	vv= ob->vv;
	if(vv->key) {
		error("Can't separate with vertex keys");
		return;
	}
	
	/* we gaan de zaak als volgt neppen:
	 * 1. duplicate base: dit wordt de nieuwe,  oude pointer onthouden
	 * 2: split doen als modig.
	 * 3. alle NIET geselecteerde verts, edges, vlakken apart zetten
	 * 4. loadebasedata(): dit is de nieuwe base
	 * 5. freelist en oude verts, eds, vlakken weer terughalen
	 */
	
	/* alleen ebase geselecteerd */
	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base==G.ebase) base->f |= 1;
			else base->f &= ~1;
		}
		base= base->next;
	}
	G.f |= 1;	/* grabber uitgeschakeld */
	adduplicate();
	G.f -= 1;

	oldbase= G.ebase;
	G.ebase= G.basact;	/* basact wordt in adduplicate() gezet */
	
	/* testen of split */
	ok= 0;
	eed= G.eded.first;
	while(eed) {
		flag= (eed->v1->f & 1)+(eed->v2->f & 1);
		if(flag==1) {
			ok= 1;
			break;
		}
		eed= eed->next;
	}
	if(ok) {
		/* SPLIT: eerst duplicate maken */
		adduplicateflag(1);
		/* SPLIT: oude vlakken hebben 3x flag 128 gezet, deze deleten */
		delvlakflag(128);
	}
	
	/* apart zetten: alles wat maar enigszins NIET select is */
	edve.first= edve.last= eded.first= eded.last= edvl.first= edvl.last= 0;
	eve= G.edve.first;
	while(eve) {
		v1= eve->next;
		if((eve->f & 1)==0) {
			remlink(&G.edve, eve);
			addtail(&edve, eve);
		}
		eve= v1;
	}
	eed= G.eded.first;
	while(eed) {
		e1= eed->next;
		if( (eed->v1->f & 1)==0 || (eed->v2->f & 1)==0 ) {
			remlink(&G.eded, eed);
			addtail(&eded, eed);
		}
		eed= e1;
	}
	evl= G.edvl.first;
	while(evl) {
		vl1= evl->next;
		if( (evl->v1->f & 1)==0 || (evl->v2->f & 1)==0 || (evl->v3->f & 1)==0 ) {
			remlink(&G.edvl, evl);
			addtail(&edvl, evl);
		}
		evl= vl1;
	}
	
	/* alvorens loadebasedata, eerst aantal users op 1 */
	vv= mallocN(sizeof(struct VV), "Separate");
	memcpy(vv, ob->vv, sizeof(struct VV));
	ob->vv->us--;	/* oldbase */
	
	ob= (struct ObData *)G.ebase->d;
	ob->vv= vv;
	vv->us= 1;
	load_ebasedata();
	
	if(G.edve.first) freelist(&G.edve);
	if(G.eded.first) freelist(&G.eded);
	if(G.edvl.first) freelist(&G.edvl);
	
	G.edve= edve;
	G.eded= eded;
	G.edvl= edvl;
	
	makeDispList(G.ebase);	/* nieuwe base */

	G.ebase= G.basact= oldbase;
	
	make_obdups();

	setcursorN(1);

	countall();
	projektie();

}

void extrude_repeatVV(steps, offs)
short steps, offs;
{
	struct EditVert *eve, *nextve;
	float dvec[3], tmat[4][4], bmat[3][3], phi;
	short a,ok;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;
	winset(G.winar[0]);
	setcursorN(2);

	/* dvec */
	dvec[0]= G.persinv[2][0];
	dvec[1]= G.persinv[2][1];
	dvec[2]= G.persinv[2][2];
	Normalise(dvec);
	dvec[0]*= offs;
	dvec[1]*= offs;
	dvec[2]*= offs;

	/* base correctie */
	parentbase(G.ebase, tmat, bmat);
	Mat3CpyMat4(bmat,tmat);
	phi= ((struct ObData *)G.ebase->d)->vv->ws;
	Mat3MulFloat(bmat, phi);
	Mat3Inv(tmat, bmat);
	Mat3MulVecfl(tmat, dvec);

	for(a=0;a<steps;a++) {
		ok= extrudeflag(1,1);
		if(ok==0) {
			error("Can't extrude");
			break;
		}
		translateflag(1, dvec);
	}

	countall();
	projektie();
}

void spinVV(steps,degr,dvec,mode)
short steps,degr;
float *dvec;
short mode;	/* 0 is extrude, 1 is duplicate */
{
	struct EditVert *eve,*nextve;
	float si,phi,n[3],q[4],cmat[3][3],tmat[4][4],imat[3][3];
	float cent[3],bmat[3][3], tvec[3];
	short a,ok;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;
	winset(G.winar[0]);
	setcursorN(2);

	/* imat en centrum en afmeting */
	parentbase(G.ebase,tmat,bmat);
	Mat3CpyMat4(bmat,tmat);
	if(G.ebase->soort==1) {
		phi= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat3MulFloat(bmat,phi);
	}
	Mat3Inv(imat,bmat);

	VECCOPY(cent,view0.muis);
	cent[0]-= tmat[3][0];
	cent[1]-= tmat[3][1];
	cent[2]-= tmat[3][2];
	Mat3MulVecfl(imat,cent);

	phi= degr*PI/360.0;
	phi/= steps;
	if(editbutflag & 1) phi= -phi;

	if(dvec) {
		n[0]=n[1]= 0.0;
		n[2]= 1.0;
	} else {
		n[0]= -G.persinv[2][0];
		n[1]= -G.persinv[2][1];
		n[2]= -G.persinv[2][2];
		Normalise(n);
	}

	q[0]= fcos(phi);
	si= fsin(phi);
	q[1]= n[0]*si;
	q[2]= n[1]*si;
	q[3]= n[2]*si;
	QuatToMat3(q,cmat);

	Mat3MulMat3(tmat,cmat,bmat);
	Mat3MulMat3(bmat,imat,tmat);

	if(mode==0) if(editbutflag & 2) adduplicateflag(1);
	ok= 1;

	for(a=0;a<steps;a++) {
		if(mode==0) ok= extrudeflag(1,1);
		else adduplicateflag(1);
		if(ok==0) {
			error("Can't spin");
			break;
		}
		rotateflag(1,cent,bmat);
		if(dvec) {
			Mat3MulVecfl(bmat,dvec);
			translateflag(1,dvec);
		}
	}

	setcursorN(1);
	if(ok==0) {
		/* geen of alleen losse verts select, dups verwijderen */
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			nextve= eve->next;
			if(eve->f & 1) {
				remlink(&G.edve,eve);
				free(eve);
			}
			eve= nextve;
		}
	}
	countall();
	projektie();
}

void screwVV(steps,turns)
short steps,turns;
{
	struct EditVert *eve,*v1=0,*v2=0;
	struct EditEdge *eed;
	float dvec[3];

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	/* eerste voorwaarde: frontview! */
	if(view0.view!=1) {
		error("Only in frontview!");
		return;
	}

	/* flags wissen */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		eve->f1= 0;
		eve= eve->next;
	}
	/* edges zetten flags in verts */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if(eed->v1->f & 1) {
			if(eed->v2->f & 1) {
				eed->v1->f1++;
				eed->v2->f1++;
			}
		}
		eed= eed->next;
	}
	/* vind twee vertices met eve->f1==1, meer of minder is fout */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f1==1) {
			if(v1==0) v1= eve;
			else if(v2==0) v2= eve;
			else {
				v1=0;
				break;
			}
		}
		eve= eve->next;
	}
	if(v1==0 || v2==0) {
		error("No curve selected");
		return;
	}

	/* bereken dvec */
	dvec[0]= ( fabs(v1->co[0]- v2->co[0]) )/(steps);
	dvec[1]= ( fabs(v1->co[1]- v2->co[1]) )/(steps);
	dvec[2]= ( fabs(v1->co[2]- v2->co[2]) )/(steps);

	spinVV(turns*steps,turns*360,dvec,0);


}

void selectswapVV()
{
	struct EditVert *eve;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->h==0) {
			if(eve->f & 1) eve->f&= ~1;
			else eve->f|= 1;
		}
		eve= eve->next;
	}
	countall();
	projektie();

}

/* *******************************  ADD  ********************* */

void addvert_ebase()
{
	struct EditVert *eve,*v1=0;
	float fac,mat[3][3],imat[3][3],tmat[4][4];
	short val;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	parentbase(G.ebase,tmat,mat);
	Mat3CpyMat4(mat,tmat);
	if(G.ebase->soort==1) {
		fac= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat3MulFloat(mat,fac);
	}
	Mat3Inv(imat,mat);

	if(editbutflag & 4) {
		v1= (struct EditVert *)G.edve.first;
		while(v1) {
			if(v1->f & 1) break;
			v1= v1->next;
		}
		eve= v1;	/* voorkomen dat er nog meer select zijn */
		while(eve) {
			eve->f&= ~1;
			eve= eve->next;
		}
	}

	eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	addtail(&G.edve,eve);

	VECCOPY(eve->co,view0.muis);
	eve->xs= view0.mx;
	eve->ys= view0.my;
	VecSubf(eve->co,eve->co,tmat[3]);

	Mat3MulVecfl(imat,eve->co);
	eve->f= 1;

	if( (editbutflag & 4) && (v1!=0) ) {
		addedgelist(v1,eve);
		v1->f=0;
		countall();
		projektie();
	}
	else {
		countall();
		winset(G.winar[0]);
		tekenoverdraw(0);

		tekenvertices_ext();

	}

	while(getbutton(RIGHTMOUSE));

	while(qtest()) {
		if( traces_qread(&val)==INPUTCHANGE) {
			if(val) G.winakt=val;
		}
	}
}

void addedgevlak_ebase()
{
	struct EditVert *eve,*new[4];
	struct EditVlak *evl;
	short aantal=0;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	/* hoeveel geselecteerd ? */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			new[aantal++]= eve;
			if(aantal>3) break;
		}
		eve= eve->next;
	}
	if(aantal<2 || aantal>3) {
		error("Can't make edge/face");
		return;
	}

	addedgelist(new[0],new[1]);
	if(aantal==3) {
		if(exist_vlak(new[0],new[1],new[2])==0) {
			addvlaklist(new[0],new[1],new[2],G.colact-1);
		}
		else error("Already a face");
	}
	countall();
	projektie();
}

void delVV()
{
	struct EditVert *eve,*nextve;
	struct EditVlak *evl,*nextvl;
	struct EditEdge *eed,*nexted;
	short event;

	if(G.ebase==0 || G.ebase->soort!=1) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	event= pupmenu("ERASE %t|Vertices%x0|Edges%x1|Faces%x2|All%x3|Edges & Faces%x4");
	if(event== -1) return;

	if(event==0 || event==4) {
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			nexted= eed->next;
			if( (eed->v1->f & 1) || (eed->v2->f & 1) ) {
				remedge(eed);
				free(eed);
			}
			eed= nexted;
		}
		evl= (struct EditVlak *)G.edvl.first;
		while(evl) {
			nextvl= evl->next;
			if( (evl->v1->f & 1) || (evl->v2->f & 1) || (evl->v3->f & 1) ) {
				remlink(&G.edvl, evl);
				free(evl);
			}
			evl= nextvl;
		}
		if(event==0) {
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				nextve= eve->next;
				if(eve->f & 1) {
					remlink(&G.edve, eve);
					free(eve);
				}
				eve= nextve;
			}
		}
	} else if(event==1) {
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			nexted= eed->next;
			if( (eed->v1->f & 1) && (eed->v2->f & 1) ) {
				remedge(eed);
				free(eed);
			}
			eed= nexted;
		}
		evl= (struct EditVlak *)G.edvl.first;
		while(evl) {
			nextvl= evl->next;
			event=0;
			if( evl->v1->f & 1) event++;
			if( evl->v2->f & 1) event++;
			if( evl->v3->f & 1) event++;
			if(event>1) {
				remlink(&G.edvl,evl);
				free(evl);
			}
			evl= nextvl;
		}
		/* om losse vertices te wissen: */
		eed= (struct EditEdge *)G.eded.first;
		while(eed) {
			if( eed->v1->f & 1) eed->v1->f-=1;
			if( eed->v2->f & 1) eed->v2->f-=1;
			eed= eed->next;
		}
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			nextve= eve->next;
			if(eve->f & 1) {
				remlink(&G.edve,eve);
				free(eve);
			}
			eve= nextve;
		}

	}
	else if(event==2) delvlakflag(1);
	else if(event==3) {
		if(G.edve.first) freelist(&G.edve);
		if(G.eded.first) freelist(&G.eded);
		if(G.edvl.first) freelist(&G.edvl);
	}

	countall();
	projektie();
}


void addprimitiveVV(soort)
short soort;
{
	struct EditVert *eve, *v1, *v2, *v3, *v4, *vtop, *vdown;
	float d, dia, phi, phid, cent[3], vec[3], imat[3][3], mat[3][3], tmat[4][4];
	float si, co, q[4], cmat[3][3], pmat[4][4];
	static short tot=32, seg=32, subdiv=2;
	static short resx = 4, resy = 4, resz = 4;
	short a, b, sym=0, ext=0, fill, type, totoud, button();

	if(soort!= -1) {
		/* deselectall */
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->f & 1) eve->f&= ~1;
			eve= eve->next;
		}

		totoud= tot; /* onthouden en terugzetten als cube/plane */

		/* imat en centrum en afmeting */
		parentbase(G.ebase,tmat,mat);
		Mat3CpyMat4(mat,tmat);
		if(G.ebase->soort==1) {
			phi= ((struct ObData *)G.ebase->d)->vv->ws;
			Mat3MulFloat(mat,phi);
		}

		VECCOPY(cent,view0.muis);
		cent[0]-= tmat[3][0];
		cent[1]-= tmat[3][1];
		cent[2]-= tmat[3][2];

		if(soort!= 11) {
			winset(G.winar[0]);
			getmatrix(pmat);
			Mat3CpyMat4(imat,pmat);
			Mat3MulVecfl(imat, cent);
			Mat3MulMat3(cmat, imat, mat);
			Mat3Inv(imat,cmat);
		} else {
			Mat3Inv(imat,mat);
		}
		/* ext=extrudeflag tot=aantal verts in basis */
		switch(soort) {
		case 0:		/* plane */
			tot= 4;
			ext= 0;
			fill= 1;
			break;
		case 1:		/* cube  */
			tot= 4;
			ext= 1;
			fill= 1;
			break;
		case 4:		/* circle  */
			if(button(&tot,4,100,"Vertices:")==0) return;
			ext= 0;
			fill= 0;
			break;
		case 5:		/* cylinder  */
			if(button(&tot,4,100,"Vertices:")==0) return;
			ext= 1;
			fill= 1;
			break;
		case 6:		/* tube  */
			if(button(&tot,4,100,"Vertices:")==0) return;
			ext= 1;
			fill= 0;
			break;
		case 7:		/* cone  */
			if(button(&tot,4,100,"Vertices:")==0) return;
			ext= 0;
			fill= 1;
			break;
		case 10:	/* grid */
			if(button(&tot,3,100,"X res:")==0) return;
			if(button(&seg,3,100,"Y res:")==0) return;
			break;
		case 11:	/* UVsphere */
			if(button(&seg,3,100,"Segments:")==0) return;
			if(button(&tot,3,100,"Rings:")==0) return;
			break;
		case 12:	/* Icosphere */
			if(button(&subdiv,1,5,"Subdivision:")==0) return;
			break;
		case 13:		/* lattice */
			if(button(&resx,1,10,"X res:")==0) return;
			if(button(&resy,1,10,"Y res:")==0) return;
			if(button(&resz,1,10,"Z res:")==0) return;
			break;
		}

		dia= fsqrt(2.0)*view0.grid;
		d= -view0.grid;
		phid= 2*PI/tot;
		phi= .25*PI;

		setcursorN(2);

		if(soort<10) {	/* alles behalve grid of sphere */
			if(ext==0 && soort!=7) d=0;

			/* de vertices */
			vtop= vdown= v1= v2= 0;
			for(b=0;b<=ext;b++) {
				for(a=0;a<tot;a++) {
					vec[0]= cent[0]+dia*fsin(phi);
					vec[1]= cent[1]+dia*fcos(phi);
					vec[2]= cent[2]+d;
					Mat3MulVecfl(imat,vec);
					eve= addvertlist(vec);
					eve->f= 1;
					if(a==0) {
						if(b==0) v1= eve;
						else v2= eve;
					}
					phi+=phid;
				}
				d= -d;
			}
			/* centrum vertices */
			if(fill & soort>1) {
				VECCOPY(vec,cent);
				vec[2]-= -d;
				Mat3MulVecfl(imat,vec);
				vdown= addvertlist(vec);
				if(ext || soort==7) {
					VECCOPY(vec,cent);
					vec[2]-= d;
					Mat3MulVecfl(imat,vec);
					vtop= addvertlist(vec);
				}
			} else {
				vdown= v1;
				vtop= v2;
			}
			if(vtop) vtop->f= 1;
			if(vdown) vdown->f= 1;

			/* boven en ondervlak */
			if(fill) {
				v3= v1;
				v4= v2;
				for(a=1;a<tot;a++) {
					addvlaklist(vdown,v3,v3->next,G.colact-1);
					v3= v3->next;
					if(ext) {
						addvlaklist(vtop,v4,v4->next,G.colact-1);
						v4= v4->next;
					}
				}
				if(soort>1) {
					addvlaklist(vdown,v3,v1,G.colact-1);
					if(ext) addvlaklist(vtop,v4,v2,G.colact-1);
				}
			}
			else if(soort==4) {  /* wel edges bij circle */
				v3= v1;
				for(a=1;a<tot;a++) {
					addedgelist(v3,v3->next);
					v3= v3->next;
				}
				addedgelist(v3,v1);
			}
			/* zijvlakken */
			if(ext) {
				v3= v1;
				v4= v2;
				for(a=1;a<tot;a++) {
					addvlaklist(v3,v3->next,v4,G.colact-1);
					addvlaklist(v4,v3->next,v4->next,G.colact-1);
					v3= v3->next;
					v4= v4->next;
				}
				addvlaklist(v3,v1,v4,G.colact-1);
				addvlaklist(v4,v1,v2,G.colact-1);
			}
			else if(soort==7) { /* cone */
				v3= v1;
				for(a=1;a<tot;a++) {
					addvlaklist(vtop,v3->next,v3,G.colact-1);
					v3= v3->next;
				}
				addvlaklist(vtop,v1,v3,G.colact-1);
			}
		}
		else if(soort==10) {	/*  grid */
			/* alle flags wissen */
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				eve->f= 0;
				eve= eve->next;
			}
			dia= view0.grid;
			/* eerst een segment: de X as */
			phi= -1.0; 
			phid= 2.0/((float)tot-1);
			for(a=0;a<tot;a++) {
				vec[0]= cent[0]+dia*phi;
				vec[1]= cent[1]- dia;
				vec[2]= cent[2];
				Mat3MulVecfl(imat,vec);
				eve= addvertlist(vec);
				eve->f= 1+2+4;
				if (a) addedgelist(eve->prev,eve);
				phi+=phid;
			}
			/* extruden en transleren */
			vec[0]= vec[2]= 0.0;
			vec[1]= dia*phid;
			Mat3MulVecfl(imat, vec);
			for(a=0;a<seg-1;a++) {
				extrudeflag(2,0);
				translateflag(2, vec);
			}
		}
		else if(soort==11) {	/*  UVsphere */
			/* alle flags wissen */
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				eve->f= 0;
				eve= eve->next;
			}
			/* eerst een segment */
			phi= 0; 
			phid/=2;
			for(a=0;a<=tot;a++) {
				vec[0]= cent[0]+dia*fsin(phi);
				vec[1]= cent[1];
				vec[2]= cent[2]+dia*fcos(phi);
				Mat3MulVecfl(imat,vec);
				eve= addvertlist(vec);
				eve->f= 1+2+4;
				if(a==0) v1= eve;
				else addedgelist(eve->prev,eve);
				phi+=phid;
			}
			/* extruden en roteren */
			phi= PI/seg;
			q[0]= fcos(phi);
			q[3]= fsin(phi);
			q[1]=q[2]= 0;
			QuatToMat3(q,cmat);
			Mat3MulMat3(tmat,cmat,mat);
			Mat3MulMat3(cmat,imat,tmat);
			for(a=0;a<seg;a++) {
				extrudeflag(2,0);
				rotateflag(2,v1->co,cmat);
			}
			removedoublesflag(4,3.0);
		}
		else if(soort==12) {	/* Icosphere */
			struct EditVert *eva[12];

			/* alle flags wissen */
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				eve->f= 0;
				eve= eve->next;
			}
			dia/=200;
			for(a=0;a<12;a++) {
				vec[0]= dia*icovert[a][0];
				vec[1]= dia*icovert[a][1];
				vec[2]= dia*icovert[a][2];
				eva[a]= addvertlist(vec);
				eva[a]->f= 1+2;
			}
			for(a=0;a<20;a++) {
				v1= eva[ icovlak[a][0] ];
				v2= eva[ icovlak[a][1] ];
				v3= eva[ icovlak[a][2] ];
				addvlaklist(v1,v2,v3,G.colact-1);
			}

			dia*=200;
			for(a=1;a<subdiv;a++) subdivideflag(2,dia);
			/* nu pas met imat */
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 2) {
					VecAddf(eve->co,eve->co,cent);
					Mat3MulVecfl(imat,eve->co);
				}
				eve= eve->next;
			}
		}

		setcursorN(1);

		if(soort<2) tot= totoud;

	}
	countall();
	recalc_editnormals();
	projektie();
}


void vertexsmooth()
{
	struct EditVert *eve;
	struct EditEdge *eed;
	float *adror,*adr;
	long teller=0,count;
	float fvec[3];

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;
	winset(G.winar[0]);

	/* aantal tellen en centrum */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) teller++;
		eve= eve->next;
	}
	if(teller==0) return;

	adr=adror= (float *)mallocN(3*4*teller,"vertgrab");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			fvec[0]=fvec[1]=fvec[2]=0;
			count = 0;

			eed= (struct EditEdge *)G.eded.first;
			while(eed) {
				if( eed->v1 == eve) {
					fvec[0] += eed->v2->co[0];
					fvec[1] += eed->v2->co[1];
					fvec[2] += eed->v2->co[2];
					count++;
				} else if(eed->v2 == eve){
					fvec[0] += eed->v1->co[0];
					fvec[1] += eed->v1->co[1];
					fvec[2] += eed->v1->co[2];
					count++;
				}
				eed= eed->next;
			}

			if (count){
				adr[0] = ((fvec[0] / count) + eve->co[0]) / 2.0;
				adr[1] = ((fvec[1] / count) + eve->co[1]) / 2.0;
				adr[2] = ((fvec[2] / count) + eve->co[2]) / 2.0;
			} else {
				VECCOPY(adr,eve->co);
			}
			adr+=3;
		}
		eve= eve->next;
	}

	adr= adror;
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			VECCOPY(eve->co,adr);
			adr+=3;
		}
		eve= eve->next;
	}
	freeN(adror);
	clearprintoverdraw();
	projektie();
}


void vertexnoise()
{
	struct Tex *rtex;
	struct EditVert *eve;
	float b2,t[3],vec[3],hnoise(),ofs;
	short tex=1, button();


	if(G.totex==0) return;
	if(button(&tex,1,G.totex,"Textblock")==0) return;

	rtex= G.adrtex[tex];

	initrendertexture();
	
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			ofs=rtex->var[1];
			VECCOPY(t,eve->co);
			if(rtex->soort!=8) {
				externtex(tex, t, &b2);
				vec[0]= 0.0;
				vec[1]= 0.0;
				vec[2]= b2;
			}
			else {
				b2= hnoise(rtex->var[0],t[0],t[1],t[2]);
				if(rtex->type > 1) ofs*=(b2*b2);
				vec[0]= b2-hnoise(rtex->var[0],t[0]+ofs,t[1],t[2]);
				vec[1]= b2-hnoise(rtex->var[0],t[0],t[1]+ofs,t[2]);
				vec[2]= b2-hnoise(rtex->var[0],t[0],t[1],t[2]+ofs);
			}
			eve->co[0]+= 4000*vec[0];
			eve->co[1]+= 4000*vec[1];
			eve->co[2]+= 4000*vec[2];
		}
		eve= eve->next;
	}

	projektie();
}

void hideVV()
{
	struct EditVert *eve;
	struct EditEdge *eed;

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			eve->f-=1;
			eve->xs= 3200;
			eve->h= 1;
		}
		eve= eve->next;
	}

	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if(eed->v1->h || eed->v2->h) eed->h= 1;
		else eed->h= 0;
		eed= eed->next;
	}

	projektie();
}

void revealVV()
{
	struct EditVert *eve;
	struct EditEdge *eed;

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->h) {
			eve->h= 0;
			eve->f|=1;
		}
		eve= eve->next;
	}

	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		eed->h= 0;
		eed= eed->next;
	}

	projektie();
}

/* ************************** MOVE  ************************ */

void copyVedit()
{
	/* dit is een 'patch', als ebase->soort==NURB worden de EditVertco's 
     * naar de nurbvertco's gekopieerd.
     * Vervolgens handles testen.
     */
	struct Base *base;
	struct EditVert *eve;
	struct Nurb *nu;
	
	if ELEM4(G.ebase->soort, 5, 11, -2, -4) {
		eve= G.edve.first;
		while(eve) {
			memcpy(eve->vn, eve->co, 12);
			eve= eve->next;
		}
		
		if(G.ebase->soort== -4) {
			calc_ika();
			/* bases die deze ika als deformparent hebben: herberekenen */
			base= G.firstbase;
			while(base) {
				if(view0.lay & base->lay) {
					if(base->p==G.ebase && base->skelnr) calc_deform(base);
				}
				base= base->next;
			}
		}
		else {
			nu= editNurb.first;
			while(nu) {
				test2DNurb(nu);
				testhandlesNurb(nu); /* test ook op bezier */
				nu= nu->next;
			}
		}
	}

	makeDispList(G.ebase);
}

void projektieV()
{

	copyVedit();
	if(G.f & 32) recalc_editnormals();
	projektie();
}

void bgnVedit()
{
	/* indien ebase->soort==NURB worden EditVerts gemaakt */
	struct EditVert *eve;
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	struct BodyPoint *bop;
	long a;

	winset(G.winar[0]);

	if ELEM3(G.ebase->soort, 5, 11, -2) {
		nu= editNurb.first;
		while(nu) {
			if((nu->type & 7)==1) {
				a= nu->pntsu;
				bezt= nu->bezt;
				while(a--) {
					if(bezt->f1 & 1) {
						eve= addvertlist(bezt->vec[0]);
						eve->vn= (struct EditVert *)(bezt->vec[0]);
						eve->f= 1;
					}
					if(bezt->f2 & 1) {
						eve= addvertlist(bezt->vec[1]);
						eve->vn= (struct EditVert *)(bezt->vec[1]);
						eve->f= 1;
					}
					if(bezt->f3 & 1) {
						eve= addvertlist(bezt->vec[2]);
						eve->vn= (struct EditVert *)(bezt->vec[2]);
						eve->f= 1;
					}
					bezt++;
				}
			}
			else {
				a= nu->pntsu*nu->pntsv;
				bp= nu->bp;
				while(a--) {
					if(bp->f1 & 1) {
						eve= addvertlist(bp->vec);
						eve->vn= (struct EditVert *)(bp->vec);
						eve->f= 1;
					}
					bp++;
				}
			}
			nu= nu->next;
		}
	}
	else if(G.ebase->soort== -4) {
		bop= bpbase.first;
		while(bop) {
			if(bop->f & 1) {
				if(rodedit) {
					eve= addvertlist(bop->r);
					eve->vn= (struct EditVert *)(bop->r);
				}
				else {
					VECCOPY(bop->hold, bop->r);
					eve= addvertlist(bop->hold);
					eve->vn= (struct EditVert *)(bop->hold);
				}
				eve->f= 1;
			}
			bop= bop->next;
		}
		
	}
}

void endVedit()
{
	/* indien ebase->soort==NURB worden EditVerts vrijgegeven */

	if ELEM4(G.ebase->soort, 5, 11, -2, -4) {
		freelist(&G.edve);
	}
}

void vertgrabber()
{
	struct EditVert *eve;
	float *adror,*adr,dx,dy,z=0,mat[3][3],cmat[3][3],tmat[4][4];
	long teller=0,a,dvec[3],vec[3];
	short val,mval[2],xn,yn,xm,ym,toets,midtog=0,proj;
	Device mdev[2];
	char s[40];

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	bgnVedit();	/* indien NurbEdit: worden editverts gemaakt */

	/* aantal tellen en centrum */
	vec[0]=vec[1]=vec[2]=0;
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			teller++;
			vec[0]+= eve->co[0];
			vec[1]+= eve->co[1];
			vec[2]+= eve->co[2];
		}
		eve= eve->next;
	}
	if(teller==0) return;

	/* opslaan */
	adr=adror= (float *)mallocN(3*4*teller,"vertgrab");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			VECCOPY(adr,eve->co);
			adr+=3;
		}
		eve= eve->next;
	}

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	getdev(2, mdev, mval);
	mval[1]-= S.vys;
	xn=xm=mval[0]; 
	yn=ym=mval[1];

	initprintoverdraw();
	sprintf(s,"Dx: 0"); 
	cmov2i(10,204); 
	charstr(s);
	sprintf(s,"Dy: 0"); 
	cmov2i(160,204); 
	charstr(s);
	sprintf(s,"Dz: 0"); 
	cmov2i(310,204); 
	charstr(s);
	endprintoverdraw();

	/* mat, imat en initgrabz */
	parentbase(G.ebase,tmat,mat);
	Mat3CpyMat4(mat,tmat);
	if(G.ebase->soort==1) {
		dx= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat3MulFloat(mat,dx);
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		dx= ((struct ObData *)G.ebase->d)->cu->ws;
		Mat3MulFloat(mat,dx);
	}
	Mat3Inv(cmat,mat);

	vec[0]/=teller; 
	vec[1]/=teller; 
	vec[2]/=teller;
	Mat3MulVec(mat,vec);
	vec[0]+= tmat[3][0];
	vec[1]+= tmat[3][1];
	vec[2]+= tmat[3][2];
	z=initgrabz(vec[0],vec[1],vec[2]);

	while(TRUE) {
		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		if(mval[0]!=xm || mval[1]!=ym) {
			xm=mval[0]; 
			ym=mval[1];
			dx= 2.0*(xn-xm)*z/S.vxw;
			dy= 2.0*(yn-ym)*z/S.vyw;
			dvec[0] = (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
			dvec[1] = (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
			dvec[2] = (G.persinv[0][2]*dx + G.persinv[1][2]*dy);

			if(G.move2==1) dvec[1]=dvec[2]=0;
			if(G.move2==2) dvec[0]=dvec[2]=0;
			if(G.move2==3) dvec[0]=dvec[1]=0;
			if(midtog) {
				if(proj==0) dvec[1]=dvec[2]=0;
				if(proj==1) dvec[0]=dvec[2]=0;
				if(proj==2) dvec[0]=dvec[1]=0;
			}

			if(G.qual & (48+3) ) {
				dy= view0.grid;
				if(G.qual & 3) dy/= 10.0;
				
				dx= dvec[0]/dy;
				dx= ffloor(dx+.5);
				dvec[0]= dx*dy;
				dx= dvec[1]/dy;
				dx= ffloor(dx+.5);
				dvec[1]= dx*dy;
				dx= dvec[2]/dy;
				dx= ffloor(dx+.5);
				dvec[2]= dx*dy;
			}

			VECCOPY(vec,dvec);
			Mat3MulVec(cmat,vec);

			adr= adror;
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					eve->co[0]= adr[0]-vec[0];
					eve->co[1]= adr[1]-vec[1];
					eve->co[2]= adr[2]-vec[2];
					adr+=3;
				}
				eve= eve->next;
			}

			projektieV();

			initprintoverdraw();
			sprintf(s,"Dx: %d",-dvec[0]); 
			cmov2i(10,204); 
			charstr(s);
			sprintf(s,"Dy: %d",-dvec[1]); 
			cmov2i(160,204); 
			charstr(s);
			sprintf(s,"Dz: %d",-dvec[2]); 
			cmov2i(310,204); 
			charstr(s);
			endprintoverdraw();
		}
		else {
			if(G.ebase && G.ebase->soort== -4) {
				calc_ika();
				projektieV();
			}
			sginap(2);
		}
		
		if(qtest()) {
			toets= traces_qread(&val);
			if(val) {
				if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
				if(toets==MIDDLEMOUSE) {
					midtog= ~midtog;
					if(midtog) proj= testproj(xn,yn,mval);
				}
				arrowsmovecursor(toets);
			}
			if(toets==INPUTCHANGE) G.winakt= val;
			toets=0;
			xm--;
		}
		
	}
	if(toets==ESCKEY) {
		adr= adror;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->f & 1) {
				VECCOPY(eve->co,adr);
				adr+=3;
			}
			eve= eve->next;
		}
	}
	freeN(adror);
	clearprintoverdraw();
	projektieV();
	endVedit();
}

void vertroteer()
{
	struct EditVert *eve;
	struct Base *b;
	struct MoData *mo;
	float dx,dy,bmat[3][3],mat[4][4],cmat[3][3],imat[3][3],pmat[3][3],n[3];
	float *adror,*adr,hoek=0,hoeko=0,deler,choek,dhoek,si,co;
	float rotn[3],vec[3],vec1[3],q1[4],q2[4],q3[4],hoek1=0,hoek2=0;
	long  a;
	short teller=0,xc=0,yc=0,val,mval[2],toets;
	short dx1,dy1,dx2,dy2,midtog=0,proj,xn,yn;
	Device mdev[2];
	char s[40];

	vec[0]=vec[1]=vec[2]=0;

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	bgnVedit();	/* indien NurbEdit: worden editverts gemaakt */

	/* aantal tellen  en centrum */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			teller++;
			VecAddf(vec,vec,eve->co);
		}
		eve= eve->next;
	}
	if(teller==0) return;

	if(G.move2) {
		n[0]=n[1]=n[2]=0;
		if(G.move2==1) n[0]=1.0;
		if(G.move2==2) n[1]=1.0;
		if(G.move2==3) n[2]=1.0;
	} else {
		n[0]= -G.persinv[2][0];
		n[1]= -G.persinv[2][1];
		n[2]= -G.persinv[2][2];
		Normalise(n);
	}

	/* opslaan */
	adr=adror= (float *)mallocN(3*4*teller,"vertrot");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			VECCOPY(adr,eve->co);
			adr+=3;
		}
		eve= eve->next;
	}

	parentbase(G.ebase,mat,bmat);
	Mat3CpyMat4(bmat,mat);
	if(G.ebase->soort==1) {
		dx= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat3MulFloat(bmat,dx);
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		dx= ((struct ObData *)G.ebase->d)->cu->ws;
		Mat3MulFloat(bmat,dx);
	}
	Mat3Inv(imat,bmat);

	if(G.move1==1) {	/* around cursor */
		berekenschermco_noclip(view0.muis,mval);
		xc= mval[0]; 
		yc= mval[1];
		VECCOPY(vec,view0.muis);
		VecSubf(vec,vec,mat[3]);
		Mat3MulVecfl(imat,vec);
	} else {
		vec[0]/=teller; 
		vec[1]/=teller; 
		vec[2]/=teller;
		VECCOPY(vec1,vec);
		Mat3MulVecfl(bmat,vec1);
		VecAddf(vec1,vec1,mat[3]);
		berekenschermcofl(vec1,mval);
		xc= mval[0]; 
		yc= mval[1];
		if (G.move1 == 2) vec[0] = vec[1] = vec[2] = 0.0;
	}

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;
	getdev(2, mdev, mval);
	mval[1]-= S.vys;
	dx1=xc-mval[0]; 
	dy1=yc-mval[1];
	xn= mval[0]; 
	yn=mval[1];

	initprintoverdraw();
	sprintf(s,"Rotate: %3.3f\n",hoek); 
	cmov2i(10,204); 
	charstr(s);
	endprintoverdraw();

	while(TRUE) {
		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		if(midtog) {
			dx2= -xn+mval[0]; 
			dy2= yn-mval[1];

			hoek1+= .2*(float)(dy2-dy1);
			if(G.qual & 32) hoek1= 5*ffloor(hoek1/5.0 +.5);
			choek= PI*hoek1/360.0;
			VECCOPY(rotn,(G.persinv[0]));
			Normalise(rotn);
			si= fsin(choek); 
			co= fcos(choek);
			q2[0]= co;
			q2[1]= si*rotn[0];
			q2[2]= si*rotn[1];
			q2[3]= si*rotn[2];

			hoek2+= .2*(float)(dx2-dx1);
			if(G.qual & 32) hoek2= 5*ffloor(hoek2/5.0 +.5);
			choek= PI*hoek2/360.0;
			VECCOPY(rotn,(G.persinv[1]));
			Normalise(rotn);
			si= fsin(choek); 
			co= fcos(choek);
			q3[0]= co;
			q3[1]= si*rotn[0];
			q3[2]= si*rotn[1];
			q3[3]= si*rotn[2];
			QuatMul(q1,q2,q3);
			if( q1[0]!=hoeko ) {
				dx1=dx2; 
				dy1=dy2;
			}
		}
		else {
			dx2=xc-mval[0]; 
			dy2=yc-mval[1];
			deler=fsqrt( (float)(dx1*dx1+dy1*dy1)*(dx2*dx2+dy2*dy2));
			if(deler>1) {
				choek=(dx1*dx2+dy1*dy2)/deler;
				dhoek= 180.0*facos(choek)/PI;
				if( (dx1*dy2-dx2*dy1)<0 ) dhoek= -dhoek;
				if(G.qual & 3) hoek+= dhoek/30.0; 
				else hoek+=dhoek;
				if(hoek>=360) hoek-=360;
				else if(hoek<-360) hoek+=360;
				if(G.qual & 32) if(G.qual & 3) hoek=ffloor(hoek+.5);
				else hoek= 5*ffloor(hoek/5.0 +.5);
				if(hoek>=360) hoek-=360;
				else if(hoek<-360) hoek+=360;

				choek=PI*hoek/360.0;
				si= fsin(choek);
				co= fcos(choek);
				q1[0]= co;
				q1[1]= si*n[0];
				q1[2]= si*n[1];
				q1[3]= si*n[2];
				if( q1[0]!=hoeko ) {
					dx1=dx2; 
					dy1=dy2;
				}
			}
		}
		if( q1[0]!=hoeko ) {
			hoeko= q1[0];
			QuatToMat3(q1,cmat);
			Mat3MulMat3(pmat,cmat,bmat);
			Mat3MulMat3(cmat,imat,pmat);

			adr= adror;
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					vec1[0]= adr[0]-vec[0];
					vec1[1]= adr[1]-vec[1];
					vec1[2]= adr[2]-vec[2];
					Mat3MulVecfl(cmat,vec1);
					eve->co[0]= vec1[0]+vec[0];
					eve->co[1]= vec1[1]+vec[1];
					eve->co[2]= vec1[2]+vec[2];
					adr+=3;
				}
				eve= eve->next;
			}

			projektieV();

			initprintoverdraw();
			if(midtog) {
				sprintf(s,"Rotate: %3.3f %3.3f\n",hoek1,hoek2);
				cmov2i(10,204); 
				charstr(s);
			}
			else {
				sprintf(s,"Rotate: %3.3f\n",hoek);
				cmov2i(10,204); 
				charstr(s);
			}
			endprintoverdraw();
		}
		else {
			if(G.ebase && G.ebase->soort== -4) {
				calc_ika();
				projektieV();
			}
			sginap(2);
		}

		if(qtest()) {
			toets= traces_qread(&val);
			if(val) {
				if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
				if(toets==MIDDLEMOUSE) {
					midtog= ~midtog;
					if(midtog) {
						dx1=xn-mval[0]; 
						dy1=yn-mval[1];
					}
					else {
						dx1=xc-mval[0]; 
						dy1=yc-mval[1];
					}
				}
				arrowsmovecursor(toets);
			}
			if(toets==INPUTCHANGE) G.winakt= val;
			toets=0;
			hoeko-=1.0;
		}
	}

	if(toets==ESCKEY) {
		adr= adror;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->f & 1) {
				VECCOPY(eve->co,adr);
				adr+=3;
			}
			eve= eve->next;
		}
	}
	freeN(adror);
	clearprintoverdraw();
	projektieV();
	endVedit();
}

void vertbend()
{
	struct EditVert *eve;
	struct Base *b;
	float fz, fw, dx, dy, bmat[4][4],mat[4][4], cmat[3][3],imat[4][4],n[3];
	float *adror, *adr, dist, si, co, viewinv[4][4];
	float min[3], max[3];
	float vec[3], cent[3], axis[3], q1[4], rad, startomtrekfac, hoek, omtrekfac;
	long  a, *rt=0;
	short teller=0, ys, yc, yco=0, val, mval[2], toets;
	char s[40];

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	bgnVedit();	/* indien NurbEdit: worden editverts gemaakt */

	parentbase(G.ebase, bmat, cmat);

	dx= 1.0;
	if(G.ebase->soort==1) {
		dx= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat4MulFloat3(bmat,dx);
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		dx= ((struct ObData *)G.ebase->d)->cu->ws;
		Mat4MulFloat3(bmat,dx);
	}
	Mat4Invert(imat, bmat);

	/* aantal tellen  en centrum */
	min[0]= min[1]= min[2]= 1.0e20;
	max[0]= max[1]= max[2]= -1.0e20;

	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			teller++;
			MinMax3(min, max, eve->co);
		}
		eve= eve->next;
	}
	if(teller<2) return;
		
	Mat4Invert(viewinv, G.viewmat);
	
	Mat4MulVecfl(bmat, min);
	Mat4MulVecfl(G.viewmat, min);
	Mat4MulVecfl(bmat, max);
	Mat4MulVecfl(G.viewmat, max);
	
	cent[0]= (min[0]+max[0])/2.0;
	cent[1]= (min[1]+max[1])/2.0;
	cent[2]= (min[2]+max[2])/2.0;

	/* opslaan */
	adr=adror= (float *)mallocN(3*4*teller,"vertrot");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			VECCOPY(adr,eve->co);
			adr+=3;
		}
		eve= eve->next;
	}

	getmouseco(mval);
	ys= mval[1];
	
		/* middelpunt is cursor */
	VECCOPY(axis, view0.muis);
	Mat4MulVecfl(G.viewmat, axis);
	rad= fsqrt( (axis[0]-cent[0])*(axis[0]-cent[0])+(axis[1]-cent[1])*(axis[1]-cent[1]) );

	startomtrekfac= (90*rad*PI)/(360.0*(max[0]-cent[0]));
	
	while(TRUE) {

		getmouseco(mval);
		yc= mval[1];
		
		if(yc!=yco) {
			yco= yc;
			omtrekfac= startomtrekfac+0.004*(yc-ys);

			/* berekenen hoek voor print */
			dist= max[0]-cent[0];
			hoek= 360*omtrekfac*dist/(rad*PI);
			
			if(G.qual & 48) {
				hoek= 5.0*ffloor(hoek/5.0);
				omtrekfac= (hoek*rad*PI)/(360.0*dist);
			}
			
			initprintoverdraw();
			sprintf(s,"Warp %3.3f", hoek); 
			cmov2i(10,204); 
			charstr(s);
			endprintoverdraw();

			/* elke vertex moet apart geroteerd */
			adr= adror;
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					
					/* punt transleren naar cent, zodanig roteren dat omtrekafstand==afstand */
					
					VECCOPY(vec, adr);
					Mat4MulVecfl(bmat, vec);
					Mat4MulVecfl(G.viewmat, vec);
					dist= vec[0]-cent[0];
					
					hoek= omtrekfac*dist/rad-0.5*PI;
					co= fcos(hoek);
					si= fsin(hoek);
					
					vec[0]= (cent[0]-axis[0]);
					vec[1]= (vec[1]-axis[1]);
					
					eve->co[0]= si*vec[0]+co*vec[1]+axis[0];
					eve->co[1]= co*vec[0]-si*vec[1]+axis[1];
					eve->co[2]= vec[2];
					
					Mat4MulVecfl(viewinv, eve->co);
					Mat4MulVecfl(imat, eve->co);

					adr+= 3;
					
				}
				eve= eve->next;
			}
			projektieV();
		
		}
		else {
			if(G.ebase && G.ebase->soort== -4) {
				calc_ika();
				projektieV();
			}
			sginap(2);
		}

		if(qtest()) {
			toets= traces_qread(&val);
			if(val) {
				if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
				arrowsmovecursor(toets);
			}
			yco= 3000;	/* gegarandeerd redraw */
			if(toets==INPUTCHANGE) G.winakt= val;
			toets=0;
		}
	}

	if(toets==ESCKEY) {
		adr= adror;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->f & 1) {
				VECCOPY(eve->co,adr);
				adr+=3;
			}
			eve= eve->next;
		}
	}
	freeN(adror);
	clearprintoverdraw();
	projektieV();
	endVedit();
}



void vertsizing()
{
	struct EditVert *eve;
	float *adror,*adr,mat[4][4],bmat[3][3],pmat[3][3],imat[3][3],cmat[3][3];
	float xref, yref, dx,sizex,sizey,sizez,sizexo,sizeyo,sizezo,fac;
	float vec[3],vec1[3],persmat[3][3],persinv[3][3];
	long a;
	short teller=0,teller2,a1, val,mval[2],xm,ym,xn,yn,xc=0,yc=0,toets;
	short midtog=0,proj;
	Device mdev[2];
	char s[20];

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;
	winset(G.winar[0]);

	xref= yref= 1.0;

	bgnVedit();	/* indien NurbEdit: worden editverts gemaakt */

	/* aantal tellen  en centrum */
	vec[0]= vec[1]= vec[2]= 0;
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			teller++;
			vec[0]+= eve->co[0];
			vec[1]+= eve->co[1];
			vec[2]+= eve->co[2];
		}
		eve= eve->next;
	}
	if(teller==0) return;

	/* opslaan */
	adr=adror= (float *)mallocN(3*4*teller,"vertrot");
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			VECCOPY(adr,eve->co);
			adr+=3;
		}
		eve= eve->next;
	}

	parentbase(G.ebase,mat,cmat);
	Mat3CpyMat4(bmat,mat);
	if(G.ebase->soort==1) {
		dx= ((struct ObData *)G.ebase->d)->vv->ws;
		Mat3MulFloat(bmat,dx);
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		dx= ((struct ObData *)G.ebase->d)->cu->ws;
		Mat3MulFloat(bmat,dx);
	}
	Mat3Inv(imat,bmat);

	Mat3CpyMat4(persmat,G.persmat);
	Mat3CpyMat4(persinv,G.persinv);

	if(G.move1==1) {	/* around cursor */
		berekenschermco_noclip(view0.muis,mval);
		xc= mval[0]; 
		yc= mval[1];
		VECCOPY(vec1, view0.muis);
		VecSubf(vec,vec1,mat[3]);
		Mat3MulVecfl(imat,vec);
	} else {
		vec[0]/=teller; 
		vec[1]/=teller; 
		vec[2]/=teller;
		VECCOPY(vec1,vec);
		Mat3MulVecfl(bmat,vec1);
		VecAddf(vec1,vec1,mat[3]);
		berekenschermcofl(vec1,mval);
		xc= mval[0]; 
		yc= mval[1];
		if (G.move1 == 2) vec[0] = vec[1] = vec[2] = 0.0;
	}

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;
	getdev(2, mdev, mval);
	mval[1]-= S.vys;
	xm=mval[0]; 
	ym=mval[1];
	sizexo=sizeyo=sizezo=1.0;
	fac= fsqrt( (float)((yc-ym)*(yc-ym)+(xm-xc)*(xm-xc)) );

	if(fac<1.0) fac= 1.0;

	initprintoverdraw();
	sprintf(s,"Size X: %3.3f",sizexo); 
	cmov2i(10,204); 
	charstr(s);
	sprintf(s,"Size Y: %3.3f",sizeyo); 
	cmov2i(160,204); 
	charstr(s);
	sprintf(s,"Size Z: %3.3f",sizezo); 
	cmov2i(310,204); 
	charstr(s);
	if(G.move2==4) {
		sprintf(s,"SizeView");
		cmov2i(460,204); 
		charstr(s);
	} else if(G.move2==5) {
		sprintf(s,"Shear");
		cmov2i(460,204); 
		charstr(s);
	}
	endprintoverdraw();

	while(TRUE) {
		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		xn=mval[0]; 
		yn=mval[1];
		if(G.move2>=4) {
			sizex=1-(float)(xm-xn)*0.005;
			if(G.move2==4) sizey=1+(float)(ym-yn)*0.005;
			sizez=1;
		} else {
			sizex=sizey=sizez= (fsqrt( (float)((yc-yn)*(yc-yn)+(xn-xc)*(xn-xc)) ))/fac;
			if(G.move2==1) sizey=sizez=1;
			if(G.move2==2) sizex=sizez=1;
			if(G.move2==3) sizey=sizex=1;
		}
		if(midtog) {
			if(proj==0) sizey=sizez=1;
			if(proj==1) sizex=sizez=1;
			if(proj==2) sizey=sizex=1;
		}
		if(G.qual & 32) {
			if (G.qual & 3) {
				sizex= (ffloor(100.0*sizex))/100.0;
				sizey= (ffloor(100.0*sizey))/100.0;
				sizez= (ffloor(100.0*sizez))/100.0;
				if(sizex==0.0) sizex== 0.01;
				if(sizey==0.0) sizey== 0.01;
				if(sizez==0.0) sizez== 0.01;
			} else {
				sizex= (ffloor(10.0*sizex))/10.0;
				sizey= (ffloor(10.0*sizey))/10.0;
				sizez= (ffloor(10.0*sizez))/10.0;
				if(sizex==0.0) sizex== 0.1;
				if(sizey==0.0) sizey== 0.1;
				if(sizez==0.0) sizez== 0.1;
			}
		} else if (G.qual & 3) {
			sizex = ((sizex - 1.0) / 10.0) + 1.0;
			sizey = ((sizey - 1.0) / 10.0) + 1.0;
			sizez = ((sizez - 1.0) / 10.0) + 1.0;
		}
			/* x flip */
		val= testproj(mval[0]+10, mval[1], mval);
		if(val==0) sizex*= xref;
		else if(val==1) sizey*= xref;
		else sizez*=xref;
			/* y flip */
		val= testproj(mval[0], mval[1]+10, mval);
		if(val==0) sizex*= yref;
		else if(val==1) sizey*= yref;
		else sizez*=yref;

		if(sizex!=sizexo || sizey!=sizeyo || sizez!=sizezo) {
			Mat3One(cmat);
			cmat[0][0]= sizex;
			cmat[1][1]= sizey;
			cmat[2][2]= sizez;
			if(G.move2>=4) {
				if(G.move2==5) {
					cmat[0][0]= cmat[2][2]= cmat[1][1]= 1.0;
					cmat[1][0]=sizex-1.0;
				}
				Mat3MulMat3(pmat,persmat,bmat);
				Mat3MulMat3(mat,cmat,pmat);
				Mat3MulMat3(pmat,persinv,mat);
				Mat3MulMat3(cmat,imat,pmat);
			} else {
				Mat3MulMat3(pmat,cmat,bmat);
				Mat3MulMat3(cmat,imat,pmat);
			}
			adr= adror;
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					vec1[0]= adr[0]-vec[0];
					vec1[1]= adr[1]-vec[1];
					vec1[2]= adr[2]-vec[2];
					Mat3MulVecfl(cmat,vec1);

					eve->co[0]= vec1[0]+vec[0];
					eve->co[1]= vec1[1]+vec[1];
					eve->co[2]= vec1[2]+vec[2];
					adr+=3;
				}
				eve= eve->next;
			}

			projektieV();
			initprintoverdraw();
			sprintf(s,"Size X: %3.3f",sizex); 
			cmov2i(10,204); 
			charstr(s);
			sprintf(s,"Size Y: %3.3f",sizey); 
			cmov2i(160,204); 
			charstr(s);
			sprintf(s,"Size Z: %3.3f",sizez); 
			cmov2i(310,204); 
			charstr(s);
			if(G.move2==4) {
				sprintf(s,"SizeView");
				cmov2i(460,204); 
				charstr(s);
			} else if(G.move2==5) {
				sprintf(s,"Shear");
				cmov2i(460,204); 
				charstr(s);
			}
			endprintoverdraw();
		}
		else {
			if(G.ebase && G.ebase->soort== -4) {
				calc_ika();
				projektieV();
			}
			sginap(2);
		}
		
		sizexo=sizex; 
		sizeyo=sizey; 
		sizezo=sizez;

		if(qtest()) {
			toets= traces_qread(&val);
			if(val) {
				if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
				else if(toets==MIDDLEMOUSE) {
					midtog= ~midtog;
					if(midtog) proj= testproj(xm,ym,mval);
				}
				else if(toets==XKEY) xref= -xref;
				else if(toets==YKEY) yref= -yref;
				arrowsmovecursor(toets);
			}
			if(toets==INPUTCHANGE) G.winakt= val;
			toets=0;
			sizexo-= 1.0;
		}
	}

	if(toets==ESCKEY) {
		adr= adror;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			if(eve->f & 1) {
				VECCOPY(eve->co,adr);
				adr+=3;
			}
			eve= eve->next;
		}
	}

	freeN(adror);
	clearprintoverdraw();
	projektieV();
	endVedit();
}

