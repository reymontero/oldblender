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

/* 

	isect.c		maart/april 1992

struct ISEdge
struct PolyFill

long vergpolyfillx(x1,x2)
long vergpolyfilly(x1,x2)

short IsectFL(v1,v2,v3,v4,v5,vec)	Intersect Face and Linesegment 
short IsectLL(v1,v2,v3,v4,cox,coy,labda,mu,vec)   intersect Line-Line
short IsectLLtest(v1,v2,v3,v4,cox,coy)   intersect Line-Line 

struct EditEdge *addfilledge(v1,v2)
void addfillvlak(v1,v2,v3)

short boundinside(pf1,pf2)

short filltestedge(v1,v2,nr)	 test v1-v2 op doorsnijden poly nr 
void mergepolys(pf1,pf2)	 maakt een edge erbij en voegt pf2 bij pf1 
struct EditEdge *existfilledge(ed)

short testedgeside(v1,v2,v3)  is v3 rechts van v1-v2 ? 

short edgefill()   DE HOOFD FILL ROUTINE 
void fillVV()

void addisedge(vec1,vec2,e1,e2,e3,e4,vl1,vl2)
void intersectVV()

*/


#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <string.h>
#include "/usr/people/include/Trace.h"
#include "/usr/people/include/Button.h"
#include <sys/times.h>


struct PolyFill {
	long edges,verts;
	float min[3],max[3];
	short f,nr;
};

struct ScFillVert {
	struct EditVert *v1;
	struct EditEdge *first,*last;
	short f,f1;
};

struct ISEdge {
	struct ISEdge *next,*prev;
	struct EditVert *v1,*v2;
	struct EditVlak *vl1,*vl2;
	short edflag1,edflag2;
};


struct ScFillVert *scdata;
struct ListBase isedgebase = {
	0,0};
struct ListBase isvertbase = {
	0,0};
struct ListBase fillvertbase = {
	0,0};
struct ListBase filledgebase = {
	0,0};
struct ListBase fillvlakbase = {
	0,0};

short cox, coy, holefill;
extern short FloatCompare();
#define COMPLIMIT	0.3

/* ****  FUNKTIES VOOR QSORT *************************** */


long vergscdata(x1,x2)
struct ScFillVert *x1,*x2;
{

	if( x1->v1->co[coy] < x2->v1->co[coy] ) return 1;
	else if( x1->v1->co[coy] > x2->v1->co[coy]) return -1;
	else if( x1->v1->co[cox] > x2->v1->co[cox] ) return 1;
	else if( x1->v1->co[cox] < x2->v1->co[cox]) return -1;

	return 0;
}


/* ****  ARITH  *************************** */


short IsectFL(v1,v2,v3,v4,v5,vec)	/* Intersect Face and Linesegment */
float *v1,*v2,*v3,*v4,*v5,*vec;	/* 1,2,3=face 4,5=line vec=answer */
{
	/* return:
		-1: colliniar
		 0: no intersection
		 1: exact intersection of edge and line
		 2: cross-intersection
	*/

	double x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	double m0,m1,m2,deeldet,det1,det2,det3;
	double rtu,rtv,rtlabda;

	t00= (double)v3[0]-(double)v1[0];
	t01= (double)v3[1]-(double)v1[1];
	t02= (double)v3[2]-(double)v1[2];
	t10= (double)v3[0]-(double)v2[0];
	t11= (double)v3[1]-(double)v2[1];
	t12= (double)v3[2]-(double)v2[2];
	t20= (double)v4[0]-(double)v5[0];
	t21= (double)v4[1]-(double)v5[1];
	t22= (double)v4[2]-(double)v5[2];

	x0=t11*t22-t12*t21;
	x1=t12*t20-t10*t22;
	x2=t10*t21-t11*t20;

	deeldet=t00*x0+t01*x1+t02*x2;
	if(deeldet==0.0) return -1;

	m0= (double)v4[0]-(double)v3[0];
	m1= (double)v4[1]-(double)v3[1];
	m2= (double)v4[2]-(double)v3[2];
	det1=m0*x0+m1*x1+m2*x2;
	rtu= det1/deeldet;
	if(rtu<=0.0) {
		det2=t00*(m1*t22-m2*t21);
		det2+=t01*(m2*t20-m0*t22);
		det2+=t02*(m0*t21-m1*t20);
		rtv= det2/deeldet;
		if(rtv<=0.0) {
			if(rtu+rtv>= -1.0) {
				det3=t00*(t11*m2-t12*m1);
				det3+=t01*(t12*m0-t10*m2);
				det3+=t02*(t10*m1-t11*m0);
				rtlabda=det3/deeldet;
				if(rtlabda>= 0.0 && rtlabda<=1.0) {
					vec[0]= v4[0]+ rtlabda*(v5[0]-v4[0]);
					vec[1]= v4[1]+ rtlabda*(v5[1]-v4[1]);
					vec[2]= v4[2]+ rtlabda*(v5[2]-v4[2]);
					if(rtu==0.0 || rtv==0.0 || (rtu+rtv== -1.0) || rtlabda==0.0 || rtlabda==1.0)
						return 1;
					return 2;
				}
			}
		}
	}
	return 0;
}


short IsectLL(v1,v2,v3,v4,cox,coy,labda,mu,vec)  /* intersect Line-Line */
float *v1,*v2,*v3,*v4;	/* vertices */
short cox,coy;			/* projection */
float *labda,*mu,*vec;		/* = answer */
{
	/* return:
		-1: colliniar
		 0: no intersection of segments
		 1: exact intersection of segments
		 2: cross-intersection of segments
	*/
	float deler;

	deler= (v1[cox]-v2[cox])*(v3[coy]-v4[coy])-(v3[cox]-v4[cox])*(v1[coy]-v2[coy]);
	if(deler==0.0) return -1;

	*labda= (v1[coy]-v3[coy])*(v3[cox]-v4[cox])-(v1[cox]-v3[cox])*(v3[coy]-v4[coy]);
	*labda= -(*labda/deler);

	deler= v3[coy]-v4[coy];
	if(deler==0) {
		deler=v3[cox]-v4[cox];
		*mu= -(*labda*(v2[cox]-v1[cox])+v1[cox]-v3[cox])/deler;
	} else {
		*mu= -(*labda*(v2[coy]-v1[coy])+v1[coy]-v3[coy])/deler;
	}
	vec[cox]= *labda*(v2[cox]-v1[cox])+v1[cox];
	vec[coy]= *labda*(v2[coy]-v1[coy])+v1[coy];

	if(*labda>=0.0 && *labda<=1.0 && *mu>=0.0 && *mu<=1.0) {
		if(*labda==0.0 || *labda==1.0 || *mu==0.0 || *mu==1.0) return 1;
		return 2;
	}
	return 0;
}


/* ****  FILL ROUTINES *************************** */

struct EditEdge *addfilledge(v1,v2)
struct EditVert *v1,*v2;
{
	struct EditEdge *newed;

	newed= (struct EditEdge *)calloc(sizeof(struct EditEdge), 1);
	addtail(&filledgebase,newed);

	newed->v1= v1;
	newed->v2= v2;

	return newed;
}

void addfillvlak(v1,v2,v3)
struct EditVert *v1,*v2,*v3;
{
	/* maakt geen edges aan */
	struct EditVlak *evl;

	evl= (struct EditVlak *)malloc(sizeof(struct EditVlak));
	addtail(&fillvlakbase,evl);
	evl->v1= v1;
	evl->v2= v2;
	evl->v3= v3;
	evl->f= 2;
}


short boundinside(pf1,pf2)
struct PolyFill *pf1,*pf2;
{
	/* is pf2 INSIDE pf1 ? met boundingbox */
	/* eerst testen of poly's bestaan */

	if(pf1->edges==0 || pf2->edges==0) return 0;

	if(pf2->max[cox]<pf1->max[cox])
		if(pf2->max[coy]<pf1->max[coy])
			if(pf2->min[cox]>pf1->min[cox])
				if(pf2->min[coy]>pf1->min[coy]) return 1;
	return 0;
}




void mergepolysSimp(pf1,pf2)	/* pf2 bij pf1 */
struct PolyFill *pf1,*pf2;
{
	struct EditVert *eve;
	struct EditEdge *eed;

	/* alle oude polynummers vervangen */
	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		if(eve->f1== pf2->nr) eve->f1= pf1->nr;
		eve= eve->next;
	}
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		if(eed->f1== pf2->nr) eed->f1= pf1->nr;
		eed= eed->next;
	}

	pf1->verts+= pf2->verts;
	pf1->edges+= pf2->edges;
	pf2->verts= pf2->edges= 0;
	pf1->f= (pf1->f | pf2->f);
}



struct EditEdge *existfilledge(v1,v2)
struct EditVert *v1,*v2;
{
	struct EditEdge *eed;

	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		if(eed->v1==v1 && eed->v2==v2) return eed;
		if(eed->v2==v1 && eed->v1==v2) return eed;
		eed= eed->next;
	}
	return 0;
}


short testedgeside(v1,v2,v3) /* is v3 rechts van v1-v2 ? Met uizondering: v3==v1 || v3==v2*/
float *v1,*v2,*v3;
{
	float inp;

	inp= (v2[cox]-v1[cox])*(v1[coy]-v3[coy])
	    +(v1[coy]-v2[coy])*(v1[cox]-v3[cox]);

	if(inp<0.0) return 0;
	else if(inp==0) {
		if(v1[cox]==v3[cox] && v1[coy]==v3[coy]) return 0;
		if(v2[cox]==v3[cox] && v2[coy]==v3[coy]) return 0;
	}
	return 1;
}

short testedgeside2(v1,v2,v3) /* is v3 rechts van v1-v2 ? niet doorsnijden! */
float *v1,*v2,*v3;
{
	float inp;

	inp= (v2[cox]-v1[cox])*(v1[coy]-v3[coy])
	    +(v1[coy]-v2[coy])*(v1[cox]-v3[cox]);

	if(inp<=0.0) return 0;
	return 1;
}

short addedgetoscanvert(sc,eed)
struct ScFillVert *sc;
struct EditEdge *eed;
{
	/* zoek eerste edge die rechts van eed ligt en stop eed daarvoor */
	struct EditEdge *ed;
	float fac,fac1,x,y;

	if(sc->first==0) {
		sc->first= sc->last= eed;
		eed->prev= eed->next=0;
		return 1;
	}

	x= eed->v1->co[cox];
	y= eed->v1->co[coy];

	fac1= eed->v2->co[coy]-y;
	if(fac1==0.0) {
		fac1= 1.0e10*(eed->v2->co[cox]-x);

	}
	else fac1= (x-eed->v2->co[cox])/fac1;

	ed= sc->first;
	while(ed) {

		if(ed->v2==eed->v2) return 0;

		fac= ed->v2->co[coy]-y;
		if(fac==0.0) {
			fac= 1.0e10*(ed->v2->co[cox]-x);

		}
		else fac= (x-ed->v2->co[cox])/fac;
		if(fac>fac1) break;

		ed= ed->next;
	}
	if(ed) insertlinkbefore(&(sc->first),ed,eed);
	else addtail(&(sc->first),eed);

	return 1;
}


struct ScFillVert *addedgetoscanlist(eed,len)
struct EditEdge *eed;
long len;
{
	/* voegt edge op juiste plek in ScFillVert list */
	/* geeft sc terug als edge al bestaat */
	struct ScFillVert *sc,scsearch;
	struct EditVert *eve;

	/* welke vert is linksboven */
	if(eed->v1->co[coy] == eed->v2->co[coy]) {
		if(eed->v1->co[cox] > eed->v2->co[cox]) {
			eve= eed->v1;
			eed->v1= eed->v2;
			eed->v2= eve;
		}
	}
	else if(eed->v1->co[coy] < eed->v2->co[coy]) {
		eve= eed->v1;
		eed->v1= eed->v2;
		eed->v2= eve;
	}
	/* zoek plek in lijst */
	scsearch.v1= eed->v1;
	sc= (struct ScFillVert *)bsearch(&scsearch,scdata,len,
	    sizeof(struct ScFillVert),vergscdata);

	if(sc==0) printf("Error in search edge: %x\n",eed);
	else if(addedgetoscanvert(sc,eed)==0) return sc;

	return 0;
}

short boundinsideEV(eed,eve)	/* is eve binnen boundbox eed */
struct EditEdge *eed;
struct EditVert *eve;
{
	float minx,maxx,miny,maxy;

	if(eed->v1->co[cox]<eed->v2->co[cox]) {
		minx= eed->v1->co[cox];
		maxx= eed->v2->co[cox];
	} else {
		minx= eed->v2->co[cox];
		maxx= eed->v1->co[cox];
	}
	if(eve->co[cox]>=minx && eve->co[cox]<=maxx) {
		if(eed->v1->co[coy]<eed->v2->co[coy]) {
			miny= eed->v1->co[coy];
			maxy= eed->v2->co[coy];
		} else {
			miny= eed->v2->co[coy];
			maxy= eed->v1->co[coy];
		}
		if(eve->co[coy]>=miny && eve->co[coy]<=maxy) return 1;
	}
	return 0;
}


void testvertexnearedge()
{
	/* alleen de vertices met ->h==1 worden getest op
		nabijheid van edge, zo ja invoegen */

	struct EditVert *eve,*v1;
	struct EditEdge *eed,*ed1;
	float dist,vec1[2],vec2[2],vec3[2],DistVL2Dfl();

	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		if(eve->h==1) {
			vec3[0]= eve->co[cox];
			vec3[1]= eve->co[coy];
			/* de bewuste edge vinden waar eve aan zit */
			ed1= (struct EditEdge *)filledgebase.first;
			while(ed1) {
				if(ed1->v1==eve || ed1->v2==eve) break;
				ed1= ed1->next;
			}
			if(ed1->v1==eve) {
				ed1->v1= ed1->v2;
				ed1->v2= eve;
			}
			eed= (struct EditEdge *)filledgebase.first;
			while(eed) {
				if(eve!=eed->v1 && eve!=eed->v2 && eve->f1==eed->f1) {
					if(FloatCompare(eve->co,eed->v1->co, COMPLIMIT)) {
						ed1->v2= eed->v1;
						eed->v1->h++;
						eve->h= 0;
						break;
					}
					else if(FloatCompare(eve->co,eed->v2->co, COMPLIMIT)) {
						ed1->v2= eed->v2;
						eed->v2->h++;
						eve->h= 0;
						break;
					}
					else {
						vec1[0]= eed->v1->co[cox];
						vec1[1]= eed->v1->co[coy];
						vec2[0]= eed->v2->co[cox];
						vec2[1]= eed->v2->co[coy];
						if(boundinsideEV(eed,eve)) {
							dist= DistVL2Dfl(vec1,vec2,vec3);
							if(dist<.3) {
								/* nieuwe edge */
								ed1= addfilledge(eed->v1,eve);
								/* printf("fill: vertex near edge %x\n",eve); */
								ed1->f= ed1->h= 0;
								ed1->f1= eed->f1;
								eed->v1= eve;
								eve->h= 3;
								break;
							}
						}
					}
				}
				eed= eed->next;
			}
		}
		eve= eve->next;
	}
}


void addlisttolist(list1,list2)
struct ListBase *list1,*list2;
{

	if(list2->first==0) return;

	if(list1->first==0) {
		list1->first= list2->first;
		list1->last= list2->last;
	}
	else {
		((struct Link *)list1->last)->next= list2->first;
		((struct Link *)list2->first)->prev= list1->last;
		list1->last= list2->last;
	}
	list2->first= list2->last= 0;
}

void splitlist(tempve,temped,nr)
struct ListBase *tempve,*temped;
short nr;
{
	/* alles zit in de templist, alleen poly nr naar fillist schrijven */
	struct EditVert *eve,*nextve;
	struct EditEdge *eed,*nexted;

	addlisttolist(tempve,&fillvertbase);
	addlisttolist(temped,&filledgebase);

	eve= tempve->first;
	while(eve) {
		nextve= eve->next;
		if(eve->f1==nr) {
			remlink(tempve,eve);
			addtail(&fillvertbase,eve);
		}
		eve= nextve;
	}
	eed= temped->first;
	while(eed) {
		nexted= eed->next;
		if(eed->f1==nr) {
			remlink(temped,eed);
			addtail(&filledgebase,eed);
		}
		eed= nexted;
	}
}


void scanfill(pf)
struct PolyFill *pf;
{
	struct ScFillVert *sc,*sc1;
	struct EditVert *eve,*v1,*v2,*v3;
	struct EditEdge *eed,*nexted,*ed1,*ed2,*ed3,*ed;
	float miny,addfac,x;
	long a,b,verts, teller=0, maxvlak, totvlak;
	short nr,test,testedgeside(),twoconnected=0;

	nr= pf->nr;
	verts= pf->verts;

	/* PRINTS
	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		printf("vert: %x co: %f %f\n",eve,eve->co[cox],eve->co[coy]);
		eve= eve->next;
	}	
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		printf("edge: %x  verts: %x %x\n",eed,eed->v1,eed->v2);
		eed= eed->next;
	} */

	/* STAP 0: alle nul lange edges eruit */
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		if(eed->v1->co[cox]==eed->v2->co[cox]) {
			if(eed->v1->co[coy]==eed->v2->co[coy]) {
				if(eed->v1->f==255 && eed->v2->f!=255) {
					eed->v2->f==255;
					eed->v2->vn== eed->v1->vn;
				}
				else if(eed->v2->f==255 && eed->v1->f!=255) {
					eed->v1->f==255;
					eed->v1->vn== eed->v2->vn;
				}
				else if(eed->v2->f==255 && eed->v1->f==255) {
					eed->v1->vn== eed->v2->vn;
				}
				else {
					eed->v2->f= 255;
					eed->v2->vn= eed->v1;
				}
			}
		}
		eed= eed->next;
	}

	/* STAP 1: maak ahv van FillVert en FillEdge lijsten een gesorteerde
		ScFillVert lijst
	*/
	sc= scdata= (struct ScFillVert *)callocN(pf->verts*sizeof(struct ScFillVert),"Scanfill1");
	eve= (struct EditVert *)fillvertbase.first;
	verts= 0;
	while(eve) {
		if(eve->f1==nr) {
			if(eve->f!= 255) {
				verts++;
				eve->f= 0;	/* vlag later voor connectedges */
				sc->v1= eve;
				sc++;
			}
		}
		eve= eve->next;
	}

	qsort(scdata,verts,sizeof(struct ScFillVert),vergscdata);

	sc= scdata;
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		nexted= eed->next;
		eed->f= 0;
		remlink(&filledgebase,eed);
		if(eed->v1->f==255) {
			v1= eed->v1;
			while(eed->v1->f==255 && eed->v1->vn!=v1) eed->v1= eed->v1->vn;
		}
		if(eed->v2->f==255) {
			v2= eed->v2;
			while(eed->v2->f==255 && eed->v2->vn!=v2) eed->v2= eed->v2->vn;
		}
		if(eed->v1!=eed->v2) addedgetoscanlist(eed,verts);
		else free(eed);

		eed= nexted;
	}
	/*
	sc= scdata;
	for(a=0;a<verts;a++) {
		printf("\nscvert: %x\n",sc->v1);
		eed= sc->first;
		while(eed) {
			printf(" ed %x %x %x\n",eed,eed->v1,eed->v2);
			eed= eed->next;
		}
		sc++;
	}*/


	/* STAP 2: FILL LUS */

	if(pf->f==0) twoconnected= 1;

	/* (tijdelijke) beveiliging: nooit veel meer vlakken dan vertices */
	totvlak= 0;
	maxvlak= 2*verts;		/* 2*verts: cirkel in driehoek, beide gevuld */

	sc= scdata;
	for(a=0;a<verts;a++) {
		/* printf("VERTEX %d %x\n",a,sc->v1); */
		ed1= sc->first;
		while(ed1) {	/* connectflags zetten */
			nexted= ed1->next;
			if(ed1->v1->h==1 || ed1->v2->h==1) {
				remlink(&(sc->first),ed1);
				addtail(&filledgebase,ed1);
				if(ed1->v1->h>1) ed1->v1->h--;
				if(ed1->v2->h>1) ed1->v2->h--;
			}
			else ed1->v2->f= 1;

			ed1= nexted;
		}
		while(sc->first) {	/* zolang er edges zijn */
			ed1= sc->first;
			ed2= ed1->next;

			if(G.afbreek) break;
			if(totvlak>maxvlak) {
				/* printf("Fill error: endless loop. Escaped at vert %d,  tot: %d.\n", a, verts); */
				a= verts;
				break;
			}
			if(ed2==0) {
				sc->first=sc->last= 0;
				/* printf("maar 1 edge aan vert\n"); */
				addtail(&filledgebase,ed1);
				ed1->v2->f= 0;
				ed1->v1->h--; 
				ed1->v2->h--;
			} else {
				/* test rest vertices */
				v1= ed1->v2;
				v2= ed1->v1;
				v3= ed2->v2;
				/* hieronder komt voor bij serie overlappende edges */
				if(v1==v2 || v2==v3) break;
				/* printf("test verts %x %x %x\n",v1,v2,v3); */

				miny= MIN2(v1->co[coy],v3->co[coy]);
				sc1= sc+1;
				test= 0;

				for(b=a+1;b<verts;b++) {
					if(sc1->v1->f==0) {
						if(sc1->v1->co[coy] <= miny) break;

						if(testedgeside(v1->co,v2->co,sc1->v1->co))
							if(testedgeside(v2->co,v3->co,sc1->v1->co))
								if(testedgeside(v3->co,v1->co,sc1->v1->co)) {
									/* punt in driehoek */
								
									test= 1;
									break;
								}
					}
					sc1++;
				}
				if(test) {
					/* nieuwe edge maken en overnieuw beginnen */
					/* printf("add new edge %x %x and start again\n",v2,sc1->v1); */
					ed3= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
					insertlinkbefore(&(sc->first),ed2,ed3);
					ed3->v1= v2;
					ed3->v2= sc1->v1;
					ed3->v2->f= 1;
					ed3->f= 2;
					ed3->v1->h++; 
					ed3->v2->h++;
				}
				else {
					/* nieuwe driehoek */
					/* printf("add vlak %x %x %x\n",v1,v2,v3); */
					addfillvlak(v1,v2,v3);
					totvlak++;
					remlink(&(sc->first),ed1);
					addtail(&filledgebase,ed1);
					ed1->v2->f= 0;
					ed1->v1->h--; 
					ed1->v2->h--;
					/* ed2 mag ook weg als het een oude is */
					if(ed2->f==0 && twoconnected) {
						remlink(&(sc->first),ed2);
						addtail(&filledgebase,ed2);
						ed2->v2->f= 0;
						ed2->v1->h--; 
						ed2->v2->h--;
					}

					/* nieuwe edge */
					ed3= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
					ed3->v1= v1;
					ed3->v2= v3;
					ed3->f= 2;
					ed3->v1->h++; 
					ed3->v2->h++;
					/* printf("add new edge %x %x\n",v1,v3); */
					sc1= addedgetoscanlist(ed3,verts);
					if(sc1) {	/* ed3 bestaat al: verwijderen*/
						/* printf("Edge bestaat al\n"); */
						ed3->v1->h--; 
						ed3->v2->h--;
						free(ed3);
						if(twoconnected) ed3= sc1->first;
						else ed3= 0;
						while(ed3) {
							if( (ed3->v1==v1 && ed3->v2==v3) || (ed3->v1==v3 && ed3->v2==v1) ) {
								remlink(&(sc1->first),ed3);
								addtail(&filledgebase,ed3);
								ed3->v1->h--; 
								ed3->v2->h--;
								break;
							}
							ed3= ed3->next;
						}
					}

				}
			}
			/* test op loze edges */
			ed1= sc->first;
			while(ed1) {
				nexted= ed1->next;
				if(ed1->v1->h<2 || ed1->v2->h<2) {
					remlink(&(sc->first),ed1);
					addtail(&filledgebase,ed1);
					if(ed1->v1->h>1) ed1->v1->h--;
					if(ed1->v2->h>1) ed1->v2->h--;
				}

				ed1= nexted;
			}
		}
		sc++;
	}

	freeN(scdata);
}



short edgefill(mode)  /* DE HOOFD FILL ROUTINE */
short mode;
{
	/*
	  - fill werkt met eigen lijsten, eerst aanmaken dus (geen vlakken)
	  - geef vertices in ->vn de oude pointer mee
	  - alleen xs en ys worden hier niet gebruikt: daar kan je iets in verstoppen
	  - edge flag ->f wordt 2 als het nieuwe edge betreft
	  - mode: & 1 is kruispunten testen, edges maken  (NOG DOEN )
	*/
	struct ListBase tempve,temped;
	struct EditVert *eve;
	struct EditEdge *eed,*nexted;
	struct EditVlak *evl;
	struct PolyFill *pflist,*pf;
	float *minp, *maxp, *v1, *v2, CalcNormFloat(), norm[3], len;
	short a,c,poly=0,ok=0,toggle=0;

	/* variabelen resetten*/
	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		eve->f= 0;
		eve->f1= 0;
		eve->h= 0;
		eve= eve->next;
	}

	/* eerst de vertices testen op aanwezigheid in edges */
	/* plus flaggen resetten */
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		eed->f= eed->f1= eed->h= 0;
		eed->v1->f= 1;
		eed->v2->f= 1;

		eed= eed->next;
	}

	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		if(eve->f & 1) {
			ok=1; 
			break;
		}
		eve= eve->next;
	}

	if(ok==0) return 0;

	/* NEW NEW! projektie bepalen: met beste normaal */
	/* pak de eerste drie verschillende verts */
	
	/* DIT STUK IS NOG STEEDS TAMELIJK ZWAK! */
	
	eve= fillvertbase.last;
	len= 0.0;
	v1= eve->co;
	v2= 0;
	eve= fillvertbase.first;
	while(eve) {
		if(v2) {
			if( FloatCompare(v2, eve->co,  10.0)==0) {
				len= CalcNormFloat(v1, v2, eve->co, norm);
				if(len != 0.0) break;
			}
		}
		else if(FloatCompare(v1, eve->co,  10.0)==0) {
			v2= eve->co;
		}
		eve= eve->next;
	}

	if(len==0.0) return 0;	/* geen fill mogelijk */

	norm[0]= fabs(norm[0]);
	norm[1]= fabs(norm[1]);
	norm[2]= fabs(norm[2]);
	
	if(norm[2]>=norm[0] && norm[2]>=norm[1]) {
		cox= 0; coy= 1;
	}
	else if(norm[1]>=norm[0] && norm[1]>=norm[2]) {
		cox= 0; coy= 2;
	}
	else {
		cox= 1; coy= 2;
	}

	/* STAP 1: AANTAL POLY'S TELLEN */
	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		/* pak eerste vertex zonder polynummer */
		if(eve->f1==0) {
			poly++;
			/* nu een soort select connected */
			ok= 1;
			eve->f1= poly;
			while(ok==1) {
				ok= 0;
				toggle++;
				if(toggle & 1) eed= (struct EditEdge *)filledgebase.first;
				else eed= (struct EditEdge *)filledgebase.last;

				while(eed) {
					if(eed->v1->f1==0 && eed->v2->f1==poly) {
						eed->v1->f1= poly;
						eed->f1= poly;
						ok= 1;
					}
					else if(eed->v2->f1==0 && eed->v1->f1==poly) {
						eed->v2->f1= poly;
						eed->f1= poly;
						ok= 1;
					}
					else if(eed->f1==0) {
						if(eed->v1->f1==poly && eed->v2->f1==poly) {
							eed->f1= poly;
							ok= 1;
						}
					}
					if(toggle & 1) eed= eed->next;
					else eed= eed->prev;
				}
			}
		}
		eve= eve->next;
	}
	/* printf("aantal poly's: %d\n",poly); */

	/* STAP 2: LOSSE EDGES EN SLIERTEN VERWIJDEREN */
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		if(eed->v1->h++ >250) break;
		if(eed->v2->h++ >250) break;
		eed= eed->next;
	}
	if(eed) {
		/* anders kan hierna niet met zekerheid vertices worden gewist */
		error("No vertices with 250 edges allowed!");
		return 0;
	}

	testvertexnearedge();	/* doet alleen vertices met ->h==1 */

	ok= 1;
	while(ok) {
		ok= 0;
		toggle++;
		if(toggle & 1) eed= (struct EditEdge *)filledgebase.first;
		else eed= (struct EditEdge *)filledgebase.last;
		while(eed) {
			if(toggle & 1) nexted= eed->next;
			else nexted= eed->prev;
			if(eed->v1->h==1) {
				eed->v2->h--;
				remlink(&fillvertbase,eed->v1); 
				free(eed->v1);
				remlink(&filledgebase,eed); 
				free(eed);
				ok= 1;
			}
			else if(eed->v2->h==1) {
				eed->v1->h--;
				remlink(&fillvertbase,eed->v2); 
				free(eed->v2);
				remlink(&filledgebase,eed); 
				free(eed);
				ok= 1;
			}
			eed= nexted;
		}
	}
	if(filledgebase.first==0) {
		/* printf("All edges removed\n"); */
		return 0;
	}


	/* STAND VAN ZAKEN:
	- eve->f  :1= aanwezig in edges
	- eve->f1 :polynummer
	- eve->h  :aantal edges aan vertex
	- eve->vn :bewaren! oorspronkelijke vertexnummer

	- eed->f  :
	- eed->f1 :polynummer
*/


	/* STAP 3: POLYFILL STRUCT MAKEN */
	pflist= (struct PolyFill *)callocN(poly*sizeof(struct PolyFill),"edgefill");
	pf= pflist;
	for(a=1;a<=poly;a++) {
		pf->nr= a;
		pf->min[0]=pf->min[1]=pf->min[2]= 1.0e20;
		pf->max[0]=pf->max[1]=pf->max[2]= -1.0e20;
		pf++;
	}
	eed= (struct EditEdge *)filledgebase.first;
	while(eed) {
		pflist[eed->f1-1].edges++;
		eed= eed->next;
	}

	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		pflist[eve->f1-1].verts++;
		minp= pflist[eve->f1-1].min;
		maxp= pflist[eve->f1-1].max;

		minp[cox]= MIN2(minp[cox],eve->co[cox]);
		minp[coy]= MIN2(minp[coy],eve->co[coy]);
		maxp[cox]= MAX2(maxp[cox],eve->co[cox]);
		maxp[coy]= MAX2(maxp[coy],eve->co[coy]);
		if(eve->h>2) pflist[eve->f1-1].f= 1;

		eve= eve->next;
	}
	
	/* pf= pflist; */
	/* for(a=1;a<=poly;a++) { */
	/* 	printf("poly:%d edges:%d verts:%d flag: %d\n",a,pf->edges,pf->verts,pf->f); */
	/* 	pf++; */
	/* } */

	/* STAP 4: GATEN OF BOUNDS VINDEN EN SAMENVOEGEN */
	/* LET OP: WERKT ALLEEN ALS POLY'S GESORTEERD ZIJN!!! */
	
	if(poly>1) {
		short *polycache, *pc;
		
		polycache= pc= callocN(2*poly, "polycache");
		
		for(a=0;a<poly;a++) {
			for(c=0;c<poly;c++) {
				if(c!=a) {
					/* als 'a' inside 'c': samenvoegen
					 * Pas Op: 'a' kan ook inside andere poly zijn.
					 */
					if(boundinside(pflist+c,pflist+a)) {
						*pc= c;
						pc++;
					}
				}
			}
			while(pc!=polycache) {
				pc--;
				mergepolysSimp(pflist+ *pc, pflist+ a);
			}
		}
		freeN(polycache);
	}
	
	/* pf= pflist; */
	/* printf("na merge\n"); */
	/* for(a=1;a<=poly;a++) { */
	/* 	printf("poly:%d edges:%d verts:%d flag: %d\n",a,pf->edges,pf->verts,pf->f); */
	/* 	pf++; */
	/* } */

	/* STAP 5: DRIEHOEKEN MAKEN */

	tempve.first= fillvertbase.first;
	tempve.last= fillvertbase.last;
	temped.first= filledgebase.first;
	temped.last= filledgebase.last;
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;

	pf= pflist;
	for(a=0;a<poly;a++) {
		if(pf->edges>1) {
			splitlist(&tempve,&temped,pf->nr);
			scanfill(pf);
		}
		pf++;
	}
	addlisttolist(&fillvertbase,&tempve);
	addlisttolist(&filledgebase,&temped);

	/* 
	evl= (struct EditVlak *)fillvlakbase.first;	
	while(evl) {
		printf("nieuw vlak %x %x %x\n",evl->v1,evl->v2,evl->v3);
		evl= evl->next;
	} */


	/*  VRIJGEVEN */

	freeN(pflist);
	return 1;

}

void fillVV()
{
	struct EditVert *eve,*v1;
	struct EditEdge *eed,*e1,*nexted;
	struct EditVlak *evl,*nextvl;
	long tijd;
	short ok;

	if(G.ebase==0 || G.ebase->soort!=1) return;

	setcursorN(2);

	/* alle selected vertices kopieeren */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->f & 1) {
			v1= (struct EditVert *)malloc(sizeof(struct EditVert));
			memcpy(v1,eve,sizeof(struct EditVert));
			addtail(&fillvertbase,v1);
			eve->vn= v1;
			v1->vn= eve;
			v1->h= 0;
		}
		eve= eve->next;
	}
	/* alle selected edges kopieeren */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		if( (eed->v1->f & 1) && (eed->v2->f & 1) ) {
			e1= (struct EditEdge *)malloc(sizeof(struct EditEdge));
			addtail(&filledgebase,e1);
			e1->v1= eed->v1->vn;
			e1->v2= eed->v2->vn;
			e1->v1->h++; 
			e1->v2->h++;
		}
		eed= eed->next;
	}
	/* van alle selected vlakken vertices en edges verwijderen om dubbels te voorkomen */
	/* alle edges tellen punten op, vlakken trekken af,
	   edges met vertices ->h<2 verwijderen */
	evl= (struct EditVlak *)G.edvl.first;
	ok= 0;
	while(evl) {
		nextvl= evl->next;
		if( (evl->v1->f & 1) && (evl->v2->f & 1) && (evl->v3->f & 1) ) {
			evl->v1->vn->h--;
			evl->v2->vn->h--;
			evl->v3->vn->h--;
			ok= 1;
			/* remlink(&G.edvl,evl);
			free(evl); */
		}
		evl= nextvl;
	}
	if(ok) {	/* er zijn vlakken geselecteerd */
		eed= (struct EditEdge *)filledgebase.first;
		while(eed) {
			nexted= eed->next;
			if(eed->v1->h<2 || eed->v2->h<2) {
				remlink(&filledgebase,eed);
				free(eed);
			}
			eed= nexted;
		}
	}

	tijd=clock();

	ok= edgefill(0);

	/*	printf("time: %d\n",(clock()-tijd)/1000);
*/
	if(ok!=0) {
		evl= (struct EditVlak *)fillvlakbase.first;
		while(evl) {
			addvlaklist(evl->v1->vn,evl->v2->vn,evl->v3->vn,G.colact-1);
			evl= evl->next;
		}
	}

	freelist(&fillvertbase);
	freelist(&filledgebase);
	freelist(&fillvlakbase);
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;

	setcursorN(1);

	countall();
	projektie();
}


/* **** INTERSECT  ******************** */

void empty() {
}

/* #define printf empty */

void addisedge(vec,edflag,vl1,vl2,tel)
float *vec;
short *edflag;
struct EditVlak *vl1,*vl2;
short tel;
{
	/* maakt edge en zo nodig nieuwe vertices */
	struct ISEdge *ise;
	struct EditVert *eve,*v1=0,*v2=0;
	float swapvec[3];
	short sw;


	/* test op volgorde snijpunten, dubbels swappen */
	if(tel>2) {
		if(FloatCompare(vec, vec+3,  COMPLIMIT)) {
			VECCOPY(swapvec,vec);
			VECCOPY(vec,vec+6);
			VECCOPY(vec+6,swapvec);
			sw= edflag[0];
			edflag[0]= edflag[2];
			edflag[2]= sw;
		}
	}
	if(FloatCompare(vec, vec+3,  COMPLIMIT)) return;

	/* test op dubbele vertices */
	eve= (struct EditVert *)isvertbase.first;
	while(eve) {
		if(v1==0) if(FloatCompare(eve->co,vec,  COMPLIMIT)) v1= eve;
		if(v2==0) if(FloatCompare(eve->co,vec+3,  COMPLIMIT)) v2= eve;

		if(v1!=0 && v2!=0) break;
		eve= eve->next;
	}
	if(v1!=0 || v2!=0) if(v1==v2) return;

	/* if(v1!=0) printf("double removed\n"); */
	/* if(v2!=0) printf("double removed\n"); */

	/* nieuwe vertices? */
	if(v1==0) {
		v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
		addtail(&isvertbase,v1);
		VECCOPY(v1->co,vec);
	}
	if(v2==0) {
		v2= (struct EditVert *)calloc(sizeof(struct EditVert),1);
		addtail(&isvertbase,v2);
		VECCOPY(v2->co,vec+3);
	}

	/* de nieuwe ISEdge */
	ise= (struct ISEdge *)calloc(sizeof(struct ISEdge),1);
	addtail(&isedgebase,ise);
	ise->v1= v1;
	ise->v2= v2;
	ise->vl1= vl1;
	ise->vl2= vl2;
	ise->edflag1= edflag[0];
	ise->edflag2= edflag[1];

	/* printf("edflag: %d %d %d %d\n",edflag[0],edflag[1],edflag[2],edflag[3]); */

	if(tel>2) {
		if(FloatCompare(vec+6,vec, COMPLIMIT)) ise->edflag1|= edflag[2];
		else ise->edflag2|= edflag[2];
		if(tel>3) {
			if(FloatCompare(vec+9,vec, COMPLIMIT)) ise->edflag1|= edflag[3];
			else ise->edflag2|= edflag[3];
		}
	}
	/* printf("edflag: %d %d \n",ise->edflag1,ise->edflag2); */
}

void oldedsort_andmake(olded, edcount, proj)
struct EditVert **olded;
int edcount, proj;
{
	struct EditVert *eve;
	int a, ok= 1;

	if(edcount==2) {
		ok= 0;
	}
	while(ok) {
		ok= 0;
		for(a=1; a<edcount; a++) {
			if(olded[a-1]->co[proj] > olded[a]->co[proj]) {
				eve= olded[a-1];
				olded[a-1]= olded[a];
				olded[a]= eve;
				ok= 1;
			}
		}
	}
	for(a=1;a<edcount;a++) {
		addfilledge(olded[a-1], olded[a]);
	}
}

short maxco(v1, v2)
float *v1, *v2;
{
	float x, y, z;

	x= fabs(v1[0]-v2[0]);
	y= fabs(v1[1]-v2[1]);
	z= fabs(v1[2]-v2[2]);

	if(x>=y && x>=z) return 0;
	else if(y>=x && y>=z) return 1;
	return 2;
	
}


void newfillvert(v1)
struct EditVert *v1;
{
	/* v1 is vert uit ISEdge struct
	   bestaat deze al in fill lijst ?
	*/
	struct EditVert *eve,*vn;

	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		if(eve->vn==v1) return;
		eve= eve->next;
	}

	eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	VECCOPY(eve->co, v1->co);
	addtail(&fillvertbase, eve);
	v1->vn= eve;

	eve->vn= v1;
}

void addisfaces(evl)
struct EditVlak *evl;
{
	/* -pak alle ISEdges bij elkaar en genereer filledge lijst
	   -genereer fillvert lijst (geen dubbels)
	   
	ise->v1,v2:   nieuwe vertices
	ise->vl1,vl2: vlakken waarin de voornoemde edges liggen.
	ise->edflag:  code welke edges zijn gesneden
	*/
	struct ISEdge *ise;
	struct EditVert *eve,*v1,*v2,*v3,*nextve,*addvertlist();
	struct EditEdge *eed;
	struct EditVert **olded1,**olded2,**olded3;
	int edcount1=2, edcount2=2, edcount3=2, edcount=0;
	int a, proj, ok=1, bitmask, col;

	/* printf("\n in addisfaces \n"); */
	/* hoeveel edges doen mee? kun je oldedgeblock size berekenen */
	ise= (struct ISEdge *)isedgebase.first;
	while(ise) {
		if(ise->vl1==evl || ise->vl2==evl) edcount++;
		ise= ise->next;
	}
	if(edcount==0) return;
	/* printf("edcount: %d\n",edcount); */

	olded1= (struct EditVert **)mallocN(8+8*edcount,"addisfaces1");
	olded2= (struct EditVert **)mallocN(8+8*edcount,"addisfaces2");
	olded3= (struct EditVert **)mallocN(8+8*edcount,"addisfaces3");
	olded1[0]= evl->v1;
	olded1[1]= evl->v2;
	olded2[0]= evl->v2;
	olded2[1]= evl->v3;
	olded3[0]= evl->v3;
	olded3[1]= evl->v1;

	/* fill edges maken en oldedge arrays maken*/

	ise= (struct ISEdge *)isedgebase.first;
	while(ise) {
		if(ise->vl1==evl || ise->vl2==evl) {
			eed= addfilledge(ise->v1, ise->v2);

			bitmask= 1;
			if(evl== ise->vl2) bitmask= 8;

			/* moet olded1 doormidden? */
			if(ise->edflag1 & bitmask) olded1[edcount1++]= ise->v1;
			if(ise->edflag2 & bitmask) olded1[edcount1++]= ise->v2;
			bitmask*=2;

			/* moet olded2 doormidden? */
			if(ise->edflag1 & bitmask) olded2[edcount2++]= ise->v1;
			if(ise->edflag2 & bitmask) olded2[edcount2++]= ise->v2;
			bitmask*=2;

			/* moet olded3 doormidden? */
			if(ise->edflag1 & bitmask) olded3[edcount3++]= ise->v1;
			if(ise->edflag2 & bitmask) olded3[edcount3++]= ise->v2;
		}
		ise= ise->next;
	}

	/* van oude edges vertices sorteren en filledges maken */

	/* printf("edcount123 %d %d %d\n", edcount1, edcount2, edcount3); */

	proj= maxco(evl->v1->co, evl->v2->co);
	oldedsort_andmake(olded1, edcount1, proj);

	proj= maxco(evl->v2->co, evl->v3->co);
	oldedsort_andmake(olded2, edcount2, proj);

	proj= maxco(evl->v3->co, evl->v1->co);
	oldedsort_andmake(olded3, edcount3, proj);

	freeN(olded1); 
	freeN(olded2); 
	freeN(olded3);

	/* - de filledges verwijzen naar oude(Edit) en nieuwe(Isect) vertices
	     met deze verts een tijdelijke fill lijst maken
	   - als een fillvert uiteindelijk een ->vn heeft is het een 'oude' vertex
	*/

	ise= (struct ISEdge *)isedgebase.first;
	while(ise) {
		if(ise->vl1==evl || ise->vl2==evl) {
			newfillvert(ise->v1);
			newfillvert(ise->v2);
			/* printf("verts uit ISEdge %x %x\n",ise->v1->vn, ise->v2->vn); */
			
		}
		ise= ise->next;
	}

	/* oude vertices kopieeren, flag zetten */
	v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	addtail(&fillvertbase,v1);
	VECCOPY(v1->co,evl->v1->co);
	evl->v1->vn= v1;
	v1->vn= evl->v1;
	v1->f= 1;
	/* printf("oude vert %x\n",v1); */

	v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	addtail(&fillvertbase,v1);
	VECCOPY(v1->co,evl->v2->co);
	evl->v2->vn= v1;
	v1->vn= evl->v2;
	v1->f= 1;
	/* printf("oude vert %x\n",v1); */

	v1= (struct EditVert *)calloc(sizeof(struct EditVert),1);
	addtail(&fillvertbase,v1);
	VECCOPY(v1->co,evl->v3->co);
	evl->v3->vn= v1;
	v1->vn= evl->v3;
	v1->f= 1;
	/* printf("oude vert %x\n",v1); */

	/* nieuwe pointers in edges schrijven */
	eed= (struct EditEdge *)filledgebase.first;
	edcount= 0;
	while(eed) {
		edcount++;
		eed->v1= eed->v1->vn;
		eed->v2= eed->v2->vn;
		/* printf("edge: %x %x\n",eed->v1,eed->v2); */
		eed= eed->next;
	}

	/* printf("voor fill: %d edges\n",edcount); */
	/* de ->vn pointers van nieuwe vertices wissen */
	eve= (struct EditVert *)fillvertbase.first;
	while(eve) {
		/* printf("vert: %x\n",eve); */
		if(eve->f==0) eve->vn=0;
		eve= eve->next;
	}

	/*	FILL */
	ok= edgefill(0);	/* (1) is kruispunten testen */
	/* printf("FILL\n"); */

	if(ok!=0) {
		/* De nieuwe vlakken */
		edcount= 0;
		col= evl->f1;
		evl= (struct EditVlak *)fillvlakbase.first;
		while(evl) {
			edcount++;
			v1= evl->v1->vn;
			if(v1==0) {
				v1= addvertlist(evl->v1->co);
				/* evl->v1->vn= v1; */
			}
			v2= evl->v2->vn;
			if(v2==0) {
				v2= addvertlist(evl->v2->co);
				/* evl->v2->vn= v2; */
			}
			v3= evl->v3->vn;
			if(v3==0) {
				v3= addvertlist(evl->v3->co);
				/* evl->v3->vn= v3; */
			}

			addvlaklist(v1,v2,v3,col);
			/* printf("EdVl %x %x %x\n",v1,v2,v3); */
			evl= evl->next;
		}
		/* printf("Na fill: %d vlakken\n", edcount); */
	}

	freelist(&fillvertbase);
	freelist(&filledgebase);
	freelist(&fillvlakbase);
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;
}

void intersectVV()
{
	static long numberofcalls=0;
	struct ISEdge *ise;
	struct EditVlak *evl,*evl1,*evl2,**eld,**evlist,*nextvl;
	struct EditVert *eve,*v1,*v2,*addvertlist();
	struct EditEdge *eed,*nexted,*findedgelist(),*edar[8];
	float min[3],max[3],afm[3],vec[15],*fp;
	ulong *octlist,*oct,*oct1;
	long a,b,c,totvlak;
	short ok, tel,oc1,oc2,oc3,ocmin,ocmax,edflag[8],comparevlak();

	if(G.ebase==0 || G.ebase->soort!=1) return;

	isedgebase.first=isedgebase.last= 0;
	isvertbase.first=isvertbase.last= 0;
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;

	/* array van vlakpointers maken */
	evl= (struct EditVlak *)G.edvl.first;
	totvlak= 0;
	while(evl) {
		if( (evl->v1->f & 1) && (evl->v2->f & 1) && (evl->v3->f & 1) ) {
			totvlak++;
			evl->f= 1;
		}
		else evl->f= 0;
		evl= evl->next;
	}
	if(totvlak<2) return;

	setcursorN(2);
	eld=evlist= (struct EditVlak **)mallocN(totvlak*4,"intersectVV");
	evl= (struct EditVlak *)G.edvl.first;
	while(evl) {
		if(evl->f==1) {
			*(eld++)= evl;
		}
		evl= evl->next;
	}

	/* min en max bepalen */
	eld= evlist;
	min[0]=min[1]=min[2]= 1.0e20;
	max[0]=max[1]=max[2]= -1.0e20;
	for(a=0;a<totvlak;a++) {
		evl= *(eld++);
		for(c=0;c<3;c++) {
			min[c]= MIN4(min[c],evl->v1->co[c],evl->v2->co[c],evl->v3->co[c]);
			max[c]= MAX4(max[c],evl->v1->co[c],evl->v2->co[c],evl->v3->co[c]);
		}
	}

	/* array octreegetallen maken en invullen */
	oct=octlist= (ulong *)callocN(totvlak*12,"intersectVV2");
	afm[0]= max[0]-min[0]+1.0;
	afm[1]= max[1]-min[1]+1.0;
	afm[2]= max[2]-min[2]+1.0;
	eld= evlist;
	for(a=0;a<totvlak;a++) {
		evl= *(eld++);
		for(c=0;c<3;c++) {
			oc1= (short)(31.9*(evl->v1->co[c]-min[c])/afm[c]);
			oc2= (short)(31.9*(evl->v2->co[c]-min[c])/afm[c]);
			oc3= (short)(31.9*(evl->v3->co[c]-min[c])/afm[c]);
			
			ocmin= MIN3(oc1, oc2, oc3);
			ocmax= MAX3(oc1, oc2, oc3);
			
			for(b=ocmin; b<=ocmax; b++) {
				*oct= BSET(*oct, b);
			}
			oct++;
		}
	}

	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		eed->f= 0;
		eed= eed->next;
	}

	/* snijlus */
	eld= evlist;
	oct= octlist;
	for(a=0;a<totvlak;a++) {
		evl= *(eld++);
		oct1= oct+3;
		for(b=a+1; b<totvlak; b++) {
			if( (oct[0] & oct1[0]) && (oct[1] & oct1[1]) && (oct[2] & oct1[2]) ) {
				evl1= evlist[b];
				if(comparevlak(evl,evl1)==0) {
					tel= 0;
					fp= vec;
					edflag[0]=edflag[1]=edflag[2]=edflag[3]= 0;
					/* edflag: van rechts naar links bitjes gezet als edge gesneden */
					if(IsectFL(evl->v1->co,evl->v2->co,evl->v3->co,evl1->v1->co,evl1->v2->co,fp)>0) {
						edflag[tel]= 1;
						edar[tel]= findedgelist(evl1->v1,evl1->v2);
						tel++; 
						fp+=3;
					}
					if(IsectFL(evl->v1->co,evl->v2->co,evl->v3->co,evl1->v2->co,evl1->v3->co,fp)>0) {
						edflag[tel]= 2;
						edar[tel]= findedgelist(evl1->v2,evl1->v3);
						tel++; 
						fp+=3;
					}
					if(IsectFL(evl->v1->co,evl->v2->co,evl->v3->co,evl1->v3->co,evl1->v1->co,fp)>0) {
						edflag[tel]= 4;
						edar[tel]= findedgelist(evl1->v3,evl1->v1);
						tel++; 
						fp+=3;
					}
					if(IsectFL(evl1->v1->co,evl1->v2->co,evl1->v3->co,evl->v1->co,evl->v2->co,fp)>0) {
						edflag[tel]= 8;
						edar[tel]= findedgelist(evl->v1,evl->v2);
						tel++; 
						fp+=3;
					}
					if(IsectFL(evl1->v1->co,evl1->v2->co,evl1->v3->co,evl->v2->co,evl->v3->co,fp)>0) {
						edflag[tel]= 16;
						edar[tel]= findedgelist(evl->v2,evl->v3);
						tel++; 
						fp+=3;
					}
					if(IsectFL(evl1->v1->co,evl1->v2->co,evl1->v3->co,evl->v3->co,evl->v1->co,fp)>0) {
						edflag[tel]= 32;
						edar[tel]= findedgelist(evl->v3,evl->v1);
						tel++; 
						fp+=3;
					}
					/* printf("aantal snijps: %d\n",tel); */
					/* intersect edge gevonden? */
					if(tel>=2) {
						addisedge(vec,edflag,evl1,evl,tel);
						for(tel--;tel>=0;tel--) {
							if(edar[tel]) edar[tel]->f= 1;
						}
						evl->f= 2;
						evl1->f= 2;
					}
				}
			}
			oct1+=3;
		}
		oct+=3;
	}

		/* DEBUG: nu even tijdelijk de snijedges wegschrijven?
		
		ise= (struct ISEdge *)isedgebase.first;
		while(ise) {
			v1= addvertlist(ise->v1->co);
			v2= addvertlist(ise->v2->co);
			addedgelist(v1,v2);

			ise= ise->next;
		}
		*/

	/* gesneden edges verwijderen  */
	eed= (struct EditEdge *)G.eded.first;
	while(eed) {
		nexted= eed->next;
		if(eed->f) {
			/* printf("edge verwijderd %x %x\n",eed->v1,eed->v2); */
			remedge(eed);
		}
		eed= nexted;
	}

	/* nieuwe vlakken maken met fill en verwijderen */
	eld= evlist;
	for(a=0;a<totvlak;a++) {
		evl= *(eld++);
		if(evl->f==2) {
			addisfaces(evl);
			remlink(&G.edvl,evl);
			/* printf("vlak verwijderd %x %x %x\n",evl->v1,evl->v2,evl->v3); */
		}
	}


	freeN(evlist); 
	freeN(octlist);
	freelist(&isedgebase);
	freelist(&isvertbase);

	/* BEVEILIGING */

/* #undef printf */

	/* loop edgelijst af en controleer of alle vertices bestaan */
	eve= G.edve.first;

	v1= (struct EditVert *)numberofcalls;
	numberofcalls++;
	while(eve) {
		eve->vn= v1;
		eve= eve->next;
	}
	a= 0;
	eed= G.eded.first;
	while(eed) {
		nexted= eed->next;
		if(eed->v1->vn!=v1 || eed->v2->vn!=v1) {
			a++;
			remlink(&G.eded, eed);
		}
		eed= nexted;
	}
	if(a) printf("error na intersect: %d edges wijzen naar niet bestaande vertices\n", a);

	/* loop vlakkenlijst af en controleer of alle edges bestaan */
	a= 0;

	evl= G.edvl.first;
	while(evl) {
		eed= findedgelist(evl->v1,evl->v2);
		if(eed==0) {
			a++;
			addedgelist(evl->v1,evl->v2);
		}
		eed= findedgelist(evl->v2,evl->v3);
		if(eed==0) {
			a++;
			addedgelist(evl->v2,evl->v3);
		}
		eed= findedgelist(evl->v3,evl->v1);
		if(eed==0) {
			a++;
			addedgelist(evl->v3,evl->v1);
		}
		evl= evl->next;
	}
	if(a) printf("error na intersect: %d edges in vlakken bestonden niet\n", a);


/* HET UIT ELKAAR PLUKKEN VAN DE ONDERDELEN */
	/* nu zijn alle oude select, zet f1 flag */
	eve= (struct EditVert *)G.edve.first;
	while(eve) {
		if(eve->h==0) {
			if(eve->f & 1) {
				eve->f1= 1;
			}
			eve->f= 0;
		}
		eve= eve->next;
	}

	/* begin met een oude vertex, selecteer connected zonder nieuwe vertices
	 * te passeren,  removedoubles en clear f1 vlag.
	 */
	eve= G.edve.first;
	while(eve) {
		if(eve->f1==1) {
			ok= 1;
			while(ok) {
				ok= 0;
				eve->f= 1;
				eed= G.eded.first;
				while(eed) {
					if(eed->h==0) {
						if( (eed->v1->f & 1) && (eed->v2->f & 1)==0) {
							eed->v2->f= 1;
							ok= 1;
						}
						else if( (eed->v2->f & 1) && (eed->v1->f & 1)==0) {
							eed->v1->f= 1;
							ok= 1;
						}
					}
					eed= eed->next;
				}
			}
			
			removedoublesflag(1, 3.0);

			/* en flags wissen */
			v1= G.edve.first;
			while(v1) {
				if(v1->f & 1) {
					v1->f= 0;
					v1->f1= 0;
				}
				v1= v1->next;
			}
			
			eve= G.edve.first;	/* ivm removedoubles */
		}
		eve= eve->next;
	}

	setcursorN(1);
	countall();
	projektie();

}

