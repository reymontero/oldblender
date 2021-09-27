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

/*  nurb.c

 * 
 *  aug/sept/okt 92
 */

#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <string.h>
#include "/usr/people/include/Trace.h"

#define BEZSELECTED(bezt)   (((bezt)->f1 & 1) || ((bezt)->f2 & 1) || ((bezt)->f3 & 1))
#define KNOTSU(nu)	    ( (nu)->orderu+ (nu)->pntsu+ (nu->orderu-1)*((nu)->flagu & 1) )
#define KNOTSV(nu)	    ( (nu)->orderv+ (nu)->pntsv+ (nu->orderv-1)*((nu)->flagv & 1) )

struct ListBase editNurb;
struct BPoint *lastselbp;
struct Nurb *lastnu;		/* voor selected */

extern ulong rectyellow[5][5],rectpurple[5][5];
extern void VecMulf(float *v1, float f);

void freeNurblist(struct ListBase *lb);

float nurbcircle[8][2]= {
	0.0, -1.0,  -1.0, -1.0,  -1.0, 0.0,  -1.0, 1.0,
	0.0, 1.0,    1.0, 1.0,   1.0, 0.0,  1.0, -1.0
};

short isNurbsel(nu)
struct Nurb *nu;
{
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a;

	if((nu->type & 7)==1) {
		bezt= nu->bezt;
		a= nu->pntsu;
		while(a--) {
			if( (bezt->f1 & 1) || (bezt->f2 & 1) || (bezt->f3 & 1) ) return 1;
			bezt++;
		}
	}
	else {
		bp= nu->bp;
		a= nu->pntsu*nu->pntsv;
		while(a--) {
			if( (bp->f1 & 1) ) return 1;
			bp++;
		}
	}
	return 0;
}


void printknots()
{
	struct Nurb *nu;
	long a, num;

	nu= editNurb.first;
	while(nu) {
		if(isNurbsel(nu) &&  (nu->type & 7)==4) {
			if(nu->knotsu) {
				num= KNOTSU(nu);
				for(a=0;a<num;a++) printf("knotu %d: %f\n", a, nu->knotsu[a]);
			}
			if(nu->knotsv) {
				num= KNOTSV(nu);
				for(a=0;a<num;a++) printf("knotv %d: %f\n", a, nu->knotsv[a]);
			}
		}
		nu= nu->next;
	}
}

void printweightsNurb()
{
	struct Nurb *nu;
	struct BPoint *bp;
	long a;
	char str[30];

	if(G.ebase==0) return;

	winset(G.winar[0]);
	persp(0);
	frontbuffer(1); 
	backbuffer(0);

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==4) {
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {
				if(bp->f1 & 1) {
					if(bp->s[0]!= 3200) {
						sprintf(str,"%2.2f", bp->vec[3]);

						cpack(0x737373);
						cmov2i(bp->s[0]-1,bp->s[1]-1);
						charstr(str);
						cmov2i(bp->s[0]+1,bp->s[1]+1);
						charstr(str);
						cpack(0xFFFFFF);
						cmov2i(bp->s[0],bp->s[1]);
						charstr(str);
					}
				}
				bp++;
			}
		}
		nu= nu->next;
	}

	frontbuffer(0); 
	backbuffer(1);
	persp(1);
}

void freeNurb(nu)
struct Nurb *nu;
{

	if(nu==0) return;

	if(nu->bezt) freeN(nu->bezt);
	nu->bezt= 0;
	if(nu->bp) freeN(nu->bp);
	nu->bp= 0;
	if(nu->knotsu) freeN(nu->knotsu);
	nu->knotsu= 0;
	if(nu->knotsv) freeN(nu->knotsv);
	nu->knotsv= 0;
	if(nu->trim.first) freeNurblist(&(nu->trim));

	freeN(nu);

}


void freeNurblist(lb)
struct ListBase *lb;
{
	struct Nurb *nu, *next;

	if(lb==0) return;

	nu= lb->first;
	while(nu) {
		next= nu->next;
		freeNurb(nu);
		nu= next;
	}
	lb->first= lb->last= 0;
}

struct Nurb *duplicateNurb(nu)
struct Nurb *nu;
{
	struct Nurb *new;
	long len;

	new= mallocstructN(struct Nurb, 1, "duplicateNurb");
	if(new==0) return 0;
	memcpy(new, nu, sizeof(struct Nurb));

	if(nu->bezt) {
		new->bezt= mallocstructN(struct BezTriple, nu->pntsu, "duplicateNurb2");
		memcpy(new->bezt, nu->bezt, nu->pntsu*sizeof(struct BezTriple));
	}
	else {
		len= nu->pntsu*nu->pntsv;
		new->bp= mallocstructN(struct BPoint, len, "duplicateNurb3");
		memcpy(new->bp, nu->bp, len*sizeof(struct BPoint));

		if(nu->knotsu) {
			len= KNOTSU(nu);
			new->knotsu= mallocN(len*4, "duplicateNurb4");
			memcpy(new->knotsu, nu->knotsu, 4*len);
		}
		if(nu->knotsv) {
			len= KNOTSV(nu);
			new->knotsv= mallocN(len*4, "duplicateNurb5");
			memcpy(new->knotsv, nu->knotsv, 4*len);
		}
	}
	return new;
}

void test2DNurb(nu)
struct Nurb *nu;
{
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a;

	if( nu->type== 9 ) {
		a= nu->pntsu;
		bezt= nu->bezt;
		while(a--) {
			bezt->vec[0][2]= 0.0; 
			bezt->vec[1][2]= 0.0; 
			bezt->vec[2][2]= 0.0;
			bezt++;
		}
	}
	else if(nu->type & 8) {
		a= nu->pntsu*nu->pntsv;
		bp= nu->bp;
		while(a--) {
			bp->vec[2]= 0.0;
			bp++;
		}
	}
}

void minmaxNurb(nu, min, max)
struct Nurb *nu;
float *min, *max;
{
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a, b;

	if( (nu->type & 7)==1 ) {
		a= nu->pntsu;
		bezt= nu->bezt;
		while(a--) {
			for(b=0;b<3;b++) {
				min[0]= MIN2(bezt->vec[b][0], min[0]);
				min[1]= MIN2(bezt->vec[b][1], min[1]);
				min[2]= MIN2(bezt->vec[b][2], min[2]);
				max[0]= MAX2(bezt->vec[b][0], max[0]);
				max[1]= MAX2(bezt->vec[b][1], max[1]);
				max[2]= MAX2(bezt->vec[b][2], max[2]);
			}
			bezt++;
		}
	}
	else {
		a= nu->pntsu*nu->pntsv;
		bp= nu->bp;
		while(a--) {
			min[0]= MIN2(bp->vec[0], min[0]);
			min[1]= MIN2(bp->vec[1], min[1]);
			min[2]= MIN2(bp->vec[2], min[2]);
			max[0]= MAX2(bp->vec[0], max[0]);
			max[1]= MAX2(bp->vec[1], max[1]);
			max[2]= MAX2(bp->vec[2], max[2]);
			bp++;
		}
	}

}

/* ~~~~~~~~~~~~~~~~~~~~Non Uniform Rational B Spline berekeningen ~~~~~~~~~~~ */


/* voor de goede orde: eigenlijk horen hier doubles gebruikt te worden */

void extend_spline(float * pnts, long in, long out)
{
	float *_pnts;
	double * add;
	long i, j, k;

	_pnts = pnts;
	add = mallocstructN(double, in, "extend_spline");

	for (k = 3; k > 0; k--){
		pnts = _pnts;

		/* punten kopieren naar add */
		for (i = 0; i < in; i++){
			add[i] = *pnts;
			pnts += 3;
		}

		/* inverse forward differencen */
		for (i = 0; i < in - 1; i++){
			for (j = in - 1; j > i; j--){
				add[j] -= add[j - 1];
			}
		}

		pnts = _pnts;
		for (i = out; i > 0; i--){
			*pnts = add[0];
			pnts += 3;
			for (j = 0; j < in - 1; j++){
				add[j] += add[j+1];
			}
		}

		_pnts++;
	}

	freeN(add);
}


void calcknots(knots, aantal, order, type)
float *knots;			/* aantal pnts NIET gecorrigeerd voor cyclic */
short aantal, order, type;	/* 0: uniform, 1: endpoints, 2: bezier */
{
	float k;
	long a;

	if(type==0) {
		for(a=0;a<aantal+order;a++) {
			knots[a]= a;
		}
	}
	else if(type==1) {
		k= 0.0;
		for(a=1;a<=aantal+order;a++) {
			knots[a-1]= k;
			if(a>=order && a<=aantal) k+= 1.0;
		}
	}
	else if(type==2) {
		if(order==4) {
			k= 0.34;
			for(a=0;a<aantal+order;a++) {
				knots[a]= ffloor(k);
				k+= (1.0/3.0);
			}
		}
		else if(order==3) {
			k= 0.6;
			for(a=0;a<aantal+order;a++) {
				if(a>=order && a<=aantal) k+= (0.5);
				knots[a]= ffloor(k);
			}
		}
	}
}

void makecyclicknots(knots, pnts, order)
float *knots;
short pnts, order;  /* aantal pnts NIET gecorrigeerd voor cyclic */
{
	long a, b;

	if(knots==0) return;

	/* eerst lange rijen (order -1) dezelfde knots aan uiteinde verwijderen */
	if(order>2) {
		b= pnts+order-1;
		for(a=1; a<order-1; a++) {
			if(knots[b]!= knots[b-a]) break;
		}
		if(a==order-1) knots[pnts+order-2]+= 1.0;
	}

	b= order;
	for(a=pnts+order-1; a<pnts+order+order-1; a++) {
		knots[a]= knots[a-1]+ (knots[b]-knots[b-1]);
		b--;
	}
}

void makeknots(nu, uv, type)	/* 0: uniform, 1: endpoints, 2: bezier */
struct Nurb *nu;
short uv, type;
{
	if( (nu->type & 7)==4 ) {
		if(uv & 1) {
			if(nu->knotsu) freeN(nu->knotsu);
			nu->knotsu= callocN(4+4*KNOTSU(nu), "makeknots");
			calcknots(nu->knotsu, nu->pntsu, nu->orderu, type);
			if(nu->flagu & 1) makecyclicknots(nu->knotsu, nu->pntsu, nu->orderu);
		}
		if(uv & 2) {
			if(nu->knotsv) freeN(nu->knotsv);
			nu->knotsv= callocN(4+4*KNOTSV(nu), "makeknots");
			calcknots(nu->knotsv, nu->pntsv, nu->orderv, type);
			if(nu->flagv & 1) makecyclicknots(nu->knotsv, nu->pntsv, nu->orderv);
		}
	}
}


void basisNurb(t, order, pnts, knots, basis, start, end)
float t;
short order, pnts;
float *knots, *basis;
long *start, *end;
{
	float d, e;
	long i, j, i1, i2, k, orderpluspnts;

	orderpluspnts= order+pnts;

	/* dit stuk is order '1' */
	for(i=0;i<orderpluspnts-1;i++) {
		if(t>= knots[i] && t<knots[i+1]) {
			basis[i]= 1.0;
			i1= i-order+1;
			if(i1<0) i1= 0;
			i2= i;
			i++;
			while(i<orderpluspnts-1) {
				basis[i]= 0.0;
				i++;
			}
			break;
		}
		else basis[i]= 0.0;
	}
	basis[i]= 0.0;

	/* printf("u %2.2f\n", t); for(k=0;k<orderpluspnts;k++) printf(" %2.2f",basis[k]); printf("\n");    */
	/* dit is order 2,3,... */
	for(j=2; j<=order; j++) {

		if(i2+j>= orderpluspnts) i2= orderpluspnts-j-1;

		for(i= i1; i<=i2; i++) {
			if(basis[i]!=0.0)
				d= ((t-knots[i])*basis[i]) / (knots[i+j-1]-knots[i]);
			else
				d= 0.0;

			if(basis[i+1]!=0.0)
				e= ((knots[i+j]-t)*basis[i+1]) / (knots[i+j]-knots[i+1]);
			else
				e= 0.0;

			basis[i]= d+e;
		}
	}

	*start= 1000;
	*end= 0;

	for(i=i1; i<=i2; i++) {
		if(basis[i]>0.0) {
			*end= i;
			if(*start==1000) *start= i;
		}
	}

	/* waarvoor was deze patch? Komt uit de oude GFA bsurf */
	if(t== knots[orderpluspnts]) {
		/* printf("patch\n"); */
		/* basis[pnts]= 1.0; */
		/* *start= *end= pnts; */
		/* return; */
	}
}


void makeNurbfaces(nu, data)
struct Nurb *nu;
float *data;	    /* moet 3*4*resolu*resolv lang zijn en op nul staan */
{
	struct BPoint *bp;
	float *basisu, *basis, *basisv, *sum, *fp, *vec, *in;
	float u, v, ustart, uend, ustep, vstart, vend, vstep, sumdiv;
	long i, j, iofs, jofs, cycl, len, resolu, resolv;
	long istart, iend, jsta, jen, *jstart, *jend, ratcomp;

	if(nu->knotsu==0 || nu->knotsv==0) return;
	if(nu->orderu>nu->pntsu) return;
	if(nu->orderv>nu->pntsv) return;
	if(data==0) return;

	/* alloceren en vars goedzetten */
	len= nu->pntsu*nu->pntsv;
	if(len==0) return;
	sum= (float *)callocN(4*len, "makeNurbfaces1");

	resolu= nu->resolu;
	resolv= nu->resolv;
	len= resolu*resolv;
	if(len==0) {
		freeN(sum);
		return;
	}

	bp= nu->bp;
	i= nu->pntsu*nu->pntsv;
	ratcomp=0;
	while(i--) {
		if(bp->vec[3]!=1.0) {
			ratcomp= 1;
			break;
		}
		bp++;
	}

	fp= nu->knotsu;
	ustart= fp[nu->orderu-1]+0.0005;
	if(nu->flagu & 1) uend= fp[nu->pntsu+nu->orderu-1];
	else uend= fp[nu->pntsu];
	ustep= (uend-ustart-0.001)/(resolu-1+(nu->flagu & 1));
	basisu= (float *)mallocN(4*KNOTSU(nu), "makeNurbfaces3");


	fp= nu->knotsv;
	vstart= fp[nu->orderv-1]+0.0005;
	;
	if(nu->flagv & 1) vend= fp[nu->pntsv+nu->orderv-1];
	else vend= fp[nu->pntsv];
	vstep= (vend-vstart-0.001)/(resolv-1+(nu->flagv & 1));
	len= KNOTSV(nu);
	basisv= (float *)mallocN(4*len*resolv, "makeNurbfaces3");
	jstart= (long *)mallocN(4*resolv, "makeNurbfaces4");
	jend= (long *)mallocN(4*resolv, "makeNurbfaces5");

	/* voorberekenen basisv en jstart,jend */
	if(nu->flagv & 1) cycl= nu->orderv-1; 
	else cycl= 0;
	v= vstart;
	basis= basisv;
	while(resolv--) {
		basisNurb(v, nu->orderv, nu->pntsv+cycl, nu->knotsv, basis, jstart+resolv, jend+resolv);
		basis+= KNOTSV(nu);
		v+= vstep;
	}

	if(nu->flagu & 1) cycl= nu->orderu-1; 
	else cycl= 0;
	in= data;
	u= ustart;
	while(resolu--) {

		basisNurb(u, nu->orderu, nu->pntsu+cycl, nu->knotsu, basisu, &istart, &iend);

		basis= basisv;
		resolv= nu->resolv;
		while(resolv--) {

			jsta= jstart[resolv];
			jen= jend[resolv];

			/* bereken sum */
			sumdiv= 0.0;
			fp= sum;

			for(j= jsta; j<=jen; j++) {

				if(j>=nu->pntsv) jofs= (j - nu->pntsv);
				else jofs= j;
				bp= nu->bp+ nu->pntsu*jofs+istart-1;

				for(i= istart; i<=iend; i++, fp++) {

					if(i>= nu->pntsu) {
						iofs= i- nu->pntsu;
						bp= nu->bp+ nu->pntsu*jofs+iofs;
					}
					else bp++;

					if(ratcomp) {
						*fp= basisu[i]*basis[j]*bp->vec[3];
						sumdiv+= *fp;
					}
					else *fp= basisu[i]*basis[j];
				}
			}
			if(ratcomp) {
				fp= sum;
				for(i= istart; i<=iend; i++) {
					for(j= jsta; j<=jen; j++, fp++) {
						*fp/= sumdiv;
					}
				}
			}

			/* een! (1.0) echt punt nu */
			fp= sum;
			for(j= jsta; j<=jen; j++) {

				if(j>=nu->pntsv) jofs= (j - nu->pntsv);
				else jofs= j;
				bp= nu->bp+ nu->pntsu*jofs+istart-1;

				for(i= istart; i<=iend; i++, fp++) {

					if(i>= nu->pntsu) {
						iofs= i- nu->pntsu;
						bp= nu->bp+ nu->pntsu*jofs+iofs;
					}
					else bp++;

					if(*fp!=0.0) {
						in[0]+= (*fp) * bp->vec[0];
						in[1]+= (*fp) * bp->vec[1];
						in[2]+= (*fp) * bp->vec[2];
					}
				}
			}

			in+=3;
			basis+= KNOTSV(nu);
		}
		u+= ustep;
	}

	/* vrijgeven */
	freeN(sum);
	freeN(basisu);
	freeN(basisv);
	freeN(jstart);
	freeN(jend);
}


void makeNurbcurve_forw(nu, data)
struct Nurb *nu;
float *data;	    /* moet 3*4*pntsu*resolu lang zijn en op nul staan */
{
	struct BPoint *bp;
	float *basisu, *sum, *fp, *vec, *in;
	float u, ustart, uend, ustep, sumdiv;
	long i, j, k, len, resolu, istart, iend;
	long wanted, org;

	if(nu->knotsu==0) return;
	if(data==0) return;

	/* alloceren en vars goedzetten */
	len= nu->pntsu;
	if(len==0) return;
	sum= (float *)callocN(4*len, "makeNurbcurve1");

	resolu= nu->resolu*nu->pntsu;
	if(resolu==0) {
		freeN(sum);
		return;
	}

	fp= nu->knotsu;
	ustart= fp[nu->orderu-1];
	uend= fp[nu->pntsu];
	ustep= (uend-ustart-0.000001)/(resolu-1);
	basisu= (float *)mallocN(4*(nu->orderu+nu->pntsu), "makeNurbcurve3");

	in= data;
	u= ustart;
	for (k = nu->orderu - 1; k < nu->pntsu; k++){

		wanted = ((nu->knotsu[k+1] - nu->knotsu[k]) / ustep);
		org = 4;	/* gelijk aan order */
		if (org > wanted) org = wanted;

		for (j = org; j > 0; j--){

			basisNurb(u, nu->orderu, nu->pntsu, nu->knotsu, basisu, &istart, &iend);
			/* bereken sum */
			sumdiv= 0.0;
			fp= sum;
			for(i= istart; i<=iend; i++, fp++) {
				/* hier nog rationele component doen */
				*fp= basisu[i];
				sumdiv+= *fp;
			}
			if(sumdiv!=0.0) if(sumdiv<0.999 || sumdiv>1.001) {
				/* is dit normaliseren ook nodig? */
				fp= sum;
				for(i= istart; i<=iend; i++, fp++) {
					*fp/= sumdiv;
				}
			}

			/* een! (1.0) echt punt nu */
			fp= sum;
			bp= nu->bp+ istart;
			for(i= istart; i<=iend; i++, bp++, fp++) {

				if(*fp!=0.0) {
					in[0]+= (*fp) * bp->vec[0];
					in[1]+= (*fp) * bp->vec[1];
					in[2]+= (*fp) * bp->vec[2];
				}
			}

			in+=3;

			u+= ustep;
		}

		if (wanted > org){
			extend_spline(in - 3 * org, org, wanted);
			in += 3 * (wanted - org);
			u += ustep * (wanted - org);
		}
	}

	/* vrijgeven */
	freeN(sum);
	freeN(basisu);
}


void makeNurbcurve(nu, data)
struct Nurb *nu;
float *data;	    /* moet 3*4*pntsu*resolu lang zijn en op nul staan */
{
	struct BPoint *bp;
	float *basisu, *sum, *fp, *vec, *in;
	float u, ustart, uend, ustep, sumdiv;
	long i, len, resolu, istart, iend, cycl;

	if(nu->knotsu==0) return;
	if(nu->orderu>nu->pntsu) return;
	if(data==0) return;

	/* alloceren en vars goedzetten */
	len= nu->pntsu;
	if(len==0) return;
	sum= (float *)callocN(4*len, "makeNurbcurve1");

	resolu= nu->resolu*nu->pntsu;
	if(resolu==0) {
		freeN(sum);
		return;
	}

	fp= nu->knotsu;
	ustart= fp[nu->orderu-1];
	if(nu->flagu & 1) uend= fp[nu->pntsu+nu->orderu-1];
	else uend= fp[nu->pntsu];
	ustep= (uend-ustart-0.01)/(resolu-1+(nu->flagu & 1));
	basisu= (float *)mallocN(4*KNOTSU(nu), "makeNurbcurve3");

	if(nu->flagu & 1) cycl= nu->orderu-1; 
	else cycl= 0;

	in= data;
	u= ustart;
	while(resolu--) {

		basisNurb(u, nu->orderu, nu->pntsu+cycl, nu->knotsu, basisu, &istart, &iend);
		/* bereken sum */
		sumdiv= 0.0;
		fp= sum;
		bp= nu->bp+ istart-1;
		for(i= istart; i<=iend; i++, fp++) {

			if(i>=nu->pntsu) bp= nu->bp+(i - nu->pntsu);
			else bp++;

			*fp= basisu[i]*bp->vec[3];
			sumdiv+= *fp;
		}
		if(sumdiv!=0.0) if(sumdiv<0.999 || sumdiv>1.001) {
			/* is dit normaliseren ook nodig? */
			fp= sum;
			for(i= istart; i<=iend; i++, fp++) {
				*fp/= sumdiv;
			}
		}

		/* een! (1.0) echt punt nu */
		fp= sum;
		bp= nu->bp+ istart-1;
		for(i= istart; i<=iend; i++, fp++) {

			if(i>=nu->pntsu) bp= nu->bp+(i - nu->pntsu);
			else bp++;

			if(*fp!=0.0) {
				in[0]+= (*fp) * bp->vec[0];
				in[1]+= (*fp) * bp->vec[1];
				in[2]+= (*fp) * bp->vec[2];
			}
		}

		in+=3;

		u+= ustep;
	}

	/* vrijgeven */
	freeN(sum);
	freeN(basisu);
}



/* ***************** BEVEL ****************** */

void makebevelcurve(cu, disp)
struct CurveData *cu;
struct ListBase *disp;
{
	struct DispList *dl, *dlnew;
	struct ObData *ob;
	float *fp, facx, facy, hoek, dhoek;
	long nr, a;

	if(cu->bevbase) {
		if(isbase(cu->bevbase)) {
			if(cu->bevbase->soort==11) {
				ob= (struct ObData *)cu->bevbase->d;
				if(ob->cu->ext1==0 && ob->cu->ext2==0) {
					facx= cu->bevbase->m[0][0]*ob->cu->ws/cu->ws;
					facy= cu->bevbase->m[1][1]*ob->cu->ws/cu->ws;

					dl= ob->cu->disp.first;
					if(dl==0) {
						makeDispList(cu->bevbase);
						dl= ob->cu->disp.first;
					}
					
					while(dl) {
						a= sizeof(struct DispList)+12*dl->parts*dl->nr;
						dlnew= mallocN(a, "makebevelcurve1");
						memcpy(dlnew, dl, a);
						addtail(disp, dlnew);
						fp= (float *)(dlnew+1);
						nr= dlnew->parts*dlnew->nr;
						while(nr--) {
							fp[2]= fp[1]*facy;
							fp[1]= -fp[0]*facx;
							fp[0]= 0.0;
							fp+= 3;
						}
						dl= dl->next;
					}
				}
			}
		}
	}
	else if(cu->ext2==0) {
		dl= mallocN(sizeof(struct DispList)+2*12, "makebevelcurve2");
		addtail(disp, dl);
		dl->type= DL_SEGM;
		dl->parts= 1;
		dl->nr= 2;
		fp= (float *)(dl+1);
		fp[0]= fp[1]= 0.0;
		fp[2]= -8*cu->ext1/cu->ws;
		fp[3]= fp[4]= 0.0;
		fp[5]= 8*cu->ext1/cu->ws;
	}
	else {
		nr= 4+2*cu->presetbev;

		dl= mallocN(sizeof(struct DispList)+nr*12, "makebevelcurve3");
		addtail(disp, dl);
		dl->type= DL_SEGM;
		dl->parts= 1;
		dl->nr= nr;

		/* eerst cirkel maken */
		fp= (float *)(dl+1);
		hoek= -0.5*PI;
		dhoek= PI/(nr-2);
		for(a=0; a<nr; a++) {
			fp[0]= 0.0;
			fp[1]= fcos(hoek)*(8.0*cu->ext2)/cu->ws;
			fp[2]= fsin(hoek)*(8.0*cu->ext2)/cu->ws;
			hoek+= dhoek;
			fp+= 3;
			if(cu->ext1!=0 && a==((nr/2)-1) ) {
				VECCOPY(fp, fp-3);
				fp+=3;
				a++;
			}
		}
		if(cu->ext1==0) dl->nr--;
		else {
			fp= (float *)(dl+1);
			for(a=0; a<nr; a++) {
				if(a<=(nr/2-1)) fp[2]-= (8.0*cu->ext1)/cu->ws;
				else fp[2]+= (8.0*cu->ext1)/cu->ws;
				fp+= 3;
			}
		}
	}

}

void freebevelcurve(disp)
struct ListBase *disp;
{
	struct DispList *dl, *dlnext;

	dl= disp->first;
	while(dl) {
		dlnext= dl->next;
		freeN(dl);
		dl= dlnext;
	}
}

short bevelinside(bl1,bl2)
struct BevList *bl1,*bl2;
{
	/* is bl2 INSIDE bl1 ? met links-rechts methode en "labda's" */
	/* geeft als correct gat 1 terug  */
	struct BevPoint *bevp, *prevbevp;
	float min,max,vec[3],hvec1[3],hvec2[3],lab,mu;
	long nr, links=0,rechts=0,mode;
	short IsectLL();

	/* neem eerste vertex van het mogelijke gat */

	bevp= (struct BevPoint *)(bl2+1);
	hvec1[0]= bevp->x; 
	hvec1[1]= bevp->y; 
	hvec1[2]= 0.0;
	VECCOPY(hvec2,hvec1);
	hvec2[0]+=1000;

	/* test deze met alle edges van mogelijk omringende poly */
	/* tel aantal overgangen links en rechts */

	bevp= (struct BevPoint *)(bl1+1);
	nr= bl1->nr;
	prevbevp= bevp+(nr-1);

	while(nr--) {
		min= prevbevp->y;
		max= bevp->y;
		if(max<min) {
			min= max;
			max= prevbevp->y;
		}
		if(min!=max) {
			if(min<=hvec1[1] && max>=hvec1[1]) {
				/* er is een overgang, snijpunt berekenen */
				mode= IsectLL(&(prevbevp->x),&(bevp->x),hvec1,hvec2,0,1,&lab,&mu,vec);
				/* als lab==0.0 of lab==1.0 dan snijdt de edge exact de overgang
				 * alleen toestaan voor lab= 1.0 (of andersom,  maakt niet uit)
				 */
				if(mode>=0 && lab!=0.0) {
					if(vec[0]<hvec1[0]) links++;
					else rechts++;
				}
			}
		}
		prevbevp= bevp;
		bevp++;
	}
	
	if( (links & 1) && (rechts & 1) ) return 1;
	return 0;
}


struct bevelsort {
	float left;
	struct BevList *bl;
	long dir;
};

long vergxcobev(x1,x2)
struct bevelsort *x1,*x2;
{

	if( x1->left > x2->left ) return 1;
	else if( x1->left < x2->left) return -1;
	return 0;
}

void calc_bevel_sin_cos(x1, y1, x2, y2, sin, cos)
float x1, y1, x2, y2, *sin, *cos;
{
	float t01, t02, x3, y3;

	t01= fsqrt(x1*x1+y1*y1);
	t02= fsqrt(x2*x2+y2*y2);
	if(t01==0.0) t01= 1.0;
	if(t02==0.0) t02= 1.0;

	x1/=t01; 
	y1/=t01;
	x2/=t02; 
	y2/=t02;

	t02= x1*x2+y1*y2;
	if(fabs(t02)>=1.0) t02= .5*PI;
	else t02= (facos(t02))/2.0;

	t02= fsin(t02);
	if(t02==0.0) t02= 1.0;

	x3= x1-x2;
	y3= y1-y2;
	if(x3==0 && y3==0) {
		/* printf("x3 en y3  nul \n"); */
		x3= y1;
		y3= -x1;
	} else {
		t01= fsqrt(x3*x3+y3*y3);
		x3/=t01; 
		y3/=t01;
	}

	*sin= -y3/t02;
	*cos= x3/t02;

}

void makeBevelList(base)
struct Base *base;
{
	/* - alle curves omzetten in poly's, met aangegeven resol en vlaggen voor dubbele punten
       - eventueel intelligent punten verwijderen (geval Nurb) 
       - scheiden in verschillende blokken met Boundbox
       - Autogat detectie */
	struct CurveData *cu;
	struct ObData *ob;
	struct Nurb *nu;
	struct BezTriple *bezt, *prevbezt;
	struct BPoint *bp;
	struct BevList *bl, *blnew, *blnext;
	struct BevPoint *bevp, *bevp2, *bevp1, *bevp0;
	float *fp, *data, *v1, *v2, min, inp, x1, x2, y1, y2, vec[3];
	struct bevelsort *sortdata, *sd, *sd1;
	long a, b, len, nr, poly;

	/* deze fie moet base hebben in verband met tflag en upflag */
	ob= (struct ObData *)base->d;
	cu= ob->cu;

	/* STAP 1: POLY'S MAKEN */

	freelistN(&(cu->bev));
	nu= cu->curve.first;
	while(nu) {
		if((nu->type & 7)==0) {		/* Poly */

			len= nu->pntsu;
			bl= callocN(sizeof(struct BevList)+len*sizeof(struct BevPoint), "makeBevelList");
			addtail(&(cu->bev), bl);

			if(nu->flagu & 1) bl->poly= 0;
			else bl->poly= -1;
			bl->nr= len;
			bl->flag= 0;
			bevp= (struct BevPoint *)(bl+1);
			bp= nu->bp;

			while(len--) {
				bevp->x= bp->vec[0];
				bevp->y= bp->vec[1];
				bevp->z= bp->vec[2];
				bevp->f1= 1;
				bevp++;
				bp++;
			}
		}
		else if((nu->type & 7)==1) {	/* Bezier curve */

			len= nu->resolu*(nu->pntsu+ (nu->flagu & 1) -1)+1;	/* voor laatste punt niet cyclic */
			bl= callocN(sizeof(struct BevList)+len*sizeof(struct BevPoint), "makeBevelList");
			addtail(&(cu->bev), bl);

			if(nu->flagu & 1) bl->poly= 0;
			else bl->poly= -1;
			bevp= (struct BevPoint *)(bl+1);

			a= nu->pntsu-1;
			bezt= nu->bezt;
			if(nu->flagu & 1) {
				a++;
				prevbezt= nu->bezt+(nu->pntsu-1);
			}
			else {
				prevbezt= bezt;
				bezt++;
			}
			while(a--) {
				if(prevbezt->h2==2 && bezt->h1==2) {

					bevp->x= prevbezt->vec[1][0];
					bevp->y= prevbezt->vec[1][1];
					bevp->z= prevbezt->vec[1][2];
					bevp->f1= 1;
					bevp->f2= 0;
					bevp++;
					bl->nr++;
					bl->flag= 1;
				}
				else {
					data= callocN(12*(nu->resolu+1), "makeBevelList2");

					v1= prevbezt->vec[1];
					v2= bezt->vec[0];
					maakbez(v1[0], v1[3], v2[0], v2[3], data, nu->resolu);
					maakbez(v1[1], v1[4], v2[1], v2[4], data+1, nu->resolu);
					if((nu->type & 8)==0)
						maakbez(v1[2], v1[5], v2[2], v2[5], data+2, nu->resolu);
						
					v1= data;
					nr= nu->resolu;
					/* met handlecodes dubbele punten aangeven */
					if(prevbezt->h1==prevbezt->h2) {
						if(prevbezt->h1==0 || prevbezt->h1==2) bevp->f1= 1;
					}
					else {
						if(prevbezt->h1==0 || prevbezt->h1==2) bevp->f1= 1;
						else if(prevbezt->h2==0 || prevbezt->h2==2) bevp->f1= 1;
					}
					while(nr--) {
						bevp->x= v1[0]; 
						bevp->y= v1[1];
						bevp->z= v1[2];

						bevp++;
						v1+=3;
					}
					bl->nr+= nu->resolu;

					freeN(data);
				}
				prevbezt= bezt;
				bezt++;
			}
			if((nu->flagu & 1)==0) {	    /* niet cyclic: endpoint */
				bevp->x= prevbezt->vec[1][0];
				bevp->y= prevbezt->vec[1][1];
				bevp->z= prevbezt->vec[1][2];
				bl->nr++;
			}

		}
		else if((nu->type & 7)==4) {    /* Nurb */
			if(nu->pntsv==1) {
				len= nu->resolu*nu->pntsu;
				bl= mallocN(sizeof(struct BevList)+len*sizeof(struct BevPoint), "makeBevelList3");
				addtail(&(cu->bev), bl);
				bl->nr= len;
				bl->flag= 0;
				if(nu->flagu & 1) bl->poly= 0;
				else bl->poly= -1;
				bevp= (struct BevPoint *)(bl+1);

				data= callocN(12*len, "makeBevelList4");    /* moet op nul staan */
				makeNurbcurve(nu, data);
				v1= data;
				while(len--) {
					bevp->x= v1[0]; 
					bevp->y= v1[1];
					bevp->z= v1[2];
					bevp->f1= bevp->f2= 0;
					bevp++;
					v1+=3;
				}
				freeN(data);
			}
		}

		nu= nu->next;
	}

	/* STAP 2: DUBBELE PUNTEN EN AUTOMATISCHE RESOLUTIE, DATABLOKKEN VERKLEINEN */
	bl= cu->bev.first;
	while(bl) {
		nr= bl->nr;
		bevp1= (struct BevPoint *)(bl+1);
		bevp0= bevp1+(nr-1);
		nr--;
		while(nr--) {
			if( fabs(bevp0->x-bevp1->x)<1.0 ) {
				if( fabs(bevp0->y-bevp1->y)<1.0 ) {
					if( fabs(bevp0->z-bevp1->z)<1.0 ) {
						bevp0->f2= 1;
						bl->flag++;
					}
				}
			}
			bevp0= bevp1;
			bevp1++;
		}
		bl= bl->next;
	}
	bl= cu->bev.first;
	while(bl) {
		blnext= bl->next;
		if(bl->flag) {
			nr= bl->nr- bl->flag+1;	/* +1 want vectorbezier zet ook flag */
			blnew= mallocN(sizeof(struct BevList)+nr*sizeof(struct BevPoint), "makeBevelList");
			memcpy(blnew, bl, sizeof(struct BevList));
			blnew->nr= 0;
			remlink(&(cu->bev), bl);
			insertlinkbefore(&(cu->bev),blnext,blnew);	/* zodat bevlijst met nurblijst gelijk loopt */
			bevp0= (struct BevPoint *)(bl+1);
			bevp1= (struct BevPoint *)(blnew+1);
			nr= bl->nr;
			while(nr--) {
				if(bevp0->f2==0) {
					memcpy(bevp1, bevp0, sizeof(struct BevPoint));
					bevp1++;
					blnew->nr++;
				}
				bevp0++;
			}
			freeN(bl);
			blnew->flag= 0;
		}
		bl= blnext;
	}

	/* STAP 3: POLY'S TELLEN EN AUTOGAT */
	bl= cu->bev.first;
	poly= 0;
	while(bl) {
		if(bl->poly>=0) {
			poly++;
			bl->poly= poly;
			bl->gat= 0;
		}
		bl= bl->next;
	}
	/* meest linkse punten vinden, tevens richting testen */
	if(poly>0) {
		sd= sortdata= mallocN(sizeof(struct bevelsort)*poly, "makeBevelList5");
		bl= cu->bev.first;
		while(bl) {
			if(bl->poly>0) {

				min= 300000.0;
				bevp= (struct BevPoint *)(bl+1);
				nr= bl->nr;
				while(nr--) {
					if(min>bevp->x) {
						min= bevp->x;
						bevp1= bevp;
					}
					bevp++;
				}
				sd->bl= bl;
				sd->left= min;

				bevp= (struct BevPoint *)(bl+1);
				if(bevp1== bevp) bevp0= bevp+ (bl->nr-1);
				else bevp0= bevp1-1;
				bevp= bevp+ (bl->nr-1);
				if(bevp1== bevp) bevp2= (struct BevPoint *)(bl+1);
				else bevp2= bevp1+1;

				inp= (bevp1->x- bevp0->x)*(bevp0->y- bevp2->y)
				    +(bevp0->y- bevp1->y)*(bevp0->x- bevp2->x);

				if(inp>0.0) sd->dir= 1;
				else sd->dir= 0;

				sd++;
			}

			bl= bl->next;
		}
		qsort(sortdata,poly,sizeof(struct bevelsort),vergxcobev);

		sd= sortdata+1;
		for(a=1; a<poly; a++, sd++) {
			bl= sd->bl;	    /* is bl een gat? */
			sd1= sortdata+ (a-1);
			for(b=a-1; b>=0; b--, sd1--) {	/* alle polys links ervan */
				if(bevelinside(sd1->bl, bl)) {
					bl->gat= 1- sd1->bl->gat;
					break;
				}
			}
		}

		/* draairichting */
		sd= sortdata;
		for(a=0; a<poly; a++, sd++) {
			if(sd->bl->gat==sd->dir) {
				bl= sd->bl;
				bevp1= (struct BevPoint *)(bl+1);
				bevp2= bevp1+ (bl->nr-1);
				nr= bl->nr/2;
				while(nr--) {
					SWAP(struct BevPoint, *bevp1, *bevp2);
					bevp1++;
					bevp2--;
				}
			}
		}

		freeN(sortdata);
	}

	/* STAP 4: COSINUSSEN */
	bl= cu->bev.first;
	while(bl) {
	
		if(bl->nr==2) {	/* 2 pnt, apart afhandelen */
			bevp2= (struct BevPoint *)(bl+1);
			bevp1= bevp2+1;

			x1= bevp1->x- bevp2->x;
			y1= bevp1->y- bevp2->y;

			calc_bevel_sin_cos(x1, y1, -x1, -y1, &(bevp1->sin), &(bevp1->cos));
			bevp2->sin= bevp1->sin;
			bevp2->cos= bevp1->cos;
		}
		else {
			bevp2= (struct BevPoint *)(bl+1);
			bevp1= bevp2+(bl->nr-1);
			bevp0= bevp1-1;

		
			nr= bl->nr;
	
			while(nr--) {
	
				if(cu->flag & 256) {	/* 3D */
					extern float *vectoquat();
					float *quat;
				
					vec[0]= bevp2->x - bevp0->x;
					vec[1]= bevp2->y - bevp0->y;
					vec[2]= bevp2->z - bevp0->z;
					
					/* quat= vectoquat(vec, base->tflag, base->upflag); */
					quat= vectoquat(vec, 1, 2);
					QuatToMat3(quat, bevp1->mat);
				}
				else {
					x1= bevp1->x- bevp0->x;
					x2= bevp1->x- bevp2->x;
					y1= bevp1->y- bevp0->y;
					y2= bevp1->y- bevp2->y;
				
					calc_bevel_sin_cos(x1, y1, x2, y2, &(bevp1->sin), &(bevp1->cos));
				}
				
				bevp0= bevp1;
				bevp1= bevp2;
				bevp2++;
			}
			/* niet cyclic gevallen corrigeren */
			if(bl->poly== -1) {
				if(bl->nr>2) {
					bevp= (struct BevPoint *)(bl+1);
					bevp1= bevp+1;
					bevp->sin= bevp1->sin;
					bevp->cos= bevp1->cos;
					Mat3CpyMat3(bevp->mat, bevp1->mat);
					bevp= (struct BevPoint *)(bl+1);
					bevp+= (bl->nr-1);
					bevp1= bevp-1;
					bevp->sin= bevp1->sin;
					bevp->cos= bevp1->cos;
					Mat3CpyMat3(bevp->mat, bevp1->mat);
				}
			}
		}
		bl= bl->next;
	}


}


/* ********************* LOAD EN MAKE *************** */

void load_ebaseNurb()
{
	/* laad editNurb in base */
	struct Base *base, *tbase;
	struct ObData *ob;
	struct MoData *mo= 0;
	struct CurveData *cu= 0;
	struct Nurb *nu, *new;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Key *key, *prevk, *newk;
	long a, b, dispdone= 0, diffvert= 0, len;

	if(G.ebase==0) return;

	if ELEM3(G.ebase->soort, 5, 11, -2) {
		ob= (struct ObData *)G.ebase->d;
		
		if(G.ebase->soort== -2) {
			mo= (struct MoData *)G.ebase->d;
			freeNurblist(&(mo->curve));
			key= mo->key;
		}
		else {
			cu= ob->cu;
			freeNurblist(&(cu->curve));
			key= cu->key;
		}

		/* als keys: goede aantal vertices? */
		if(G.ipomode==22 && key && G.actkey) {
			if(G.ebase->soort== -2) {
				if(G.totvert!= mo->keyverts) diffvert= 1;
				mo->keyverts= G.totvert;
			}
			else {
				if(G.totvert!= cu->keyverts) diffvert= 1;
				cu->keyverts= G.totvert;
			}
		}

		/* wordruimte: eerst doen als ebase blijft bestaan (render) */
		testWspace(G.ebase);

		nu= editNurb.first;
		while(nu) {
			new= duplicateNurb(nu);
			new->hide= 0;
			if(mo) addtail(&(mo->curve), new);
			else addtail(&(cu->curve), new);
			
			nu= nu->next;
		}
		
		
		if(G.ipomode==22 && key && G.actkey) {
			if(diffvert) {
				/* allemaal nieuwe keyblokken maken */
				
				prevk= 0;
				len= sizeof(struct Key)+4+12*G.totvert;
				while(key) {
					newk= (struct Key *)mallocN(len, "loadebaseNurb");
					memcpy(newk, key, sizeof(struct Key));
					
					curve_to_key(newk, cu, mo);
					
					freeN(key);
	
					if(prevk==0) {
						if(cu) cu->key= newk;
						else mo->key= newk;
					}
					else prevk->next= newk;
					prevk= newk;
	
					key= newk->next;
				}
				
			}
			else {

				a= 1;
				while(key) {
					if(a== G.actkey) break;
					a++;
					key= key->next;
				}
				if(key) {
					curve_to_key(key, cu, mo);
				}
			}
			
			/* nu zit waarschijnlijk verkeerde key in curvedata: dus... */
			base= G.ebase;
			G.ebase= 0;	
			loadkeypos(base);
			G.ebase= base;
			dispdone= 1;
		}
		
		if(G.ebase->soort==11) {
			if(dispdone==0) {
				makeBevelList(G.ebase);
			}
		}
		makeDispList(G.ebase);
	}
	
	lastnu= 0;	/* voor selected */
	
}

void make_ebaseNurb()
{
	/* maak kopie van baseNurb in editNurb */
	void calchandlesNurb();
	struct ObData *ob=0;
	struct MoData *mo=0;
	struct CurveData *cu=0;
	struct Nurb *nu, *new;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Key *key;
	long a, tot=0;

	if(G.ebase==0) return;

	lastselbp= 0;   /* global voor select row */

	if ELEM3(G.ebase->soort, 5, 11, -2) {
		freeNurblist(&editNurb);
		
		if ELEM(G.ebase->soort, 5, 11) {
			ob= (struct ObData *)G.ebase->d;
			cu= ob->cu;
			key= ob->cu->key;
			nu= cu->curve.first;
		}
		else {
			mo= (struct MoData *)G.ebase->d;
			key= mo->key;
			nu= mo->curve.first;
		}
		
		if(key && G.ipomode==22 && G.actkey) {

			a= 1;
			while(key) {
				if(a== G.actkey) break;
				a++;
				key= key->next;
			}
			if(key) {
				key_to_curve(key, cu, mo);
				
				while(nu) {
					calchandlesNurb(nu);
					nu= nu->next;
				}
			}
		}
		
		if ELEM(G.ebase->soort, 5, 11) nu= ob->cu->curve.first;
		else nu= mo->curve.first;
		
		while(nu) {
			new= duplicateNurb(nu);
			addtail(&editNurb, new);
			/* flags op nul */
			new->hide= 0;
			if((nu->type & 7)==1) {
				a= nu->pntsu;
				bezt= new->bezt;
				while(a--) {
					bezt->f1= bezt->f2= bezt->f3= bezt->hide= 0;
					bezt++;
					tot+= 3;
				}
			}
			else {
				a= nu->pntsu*nu->pntsv;
				bp= new->bp;
				while(a--) {
					bp->f1= bp->hide= 0;
					bp++;
					tot++;
				}
			}
			nu= nu->next;
		}
		if(ob) {
			ob->cu->keyverts= tot;
			make_obdups();
		}
		else if(mo) mo->keyverts= tot;
		
		makeDispList(G.ebase);
	}
	else G.ebase= 0;

	lastnu= 0;	/* voor selected */
}

void separateNurb()
{
	struct Nurb *nu, *nu1, *new;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Base *oldbase, *base;
	struct ObData *ob;
	struct CurveData *cu;
	struct ListBase editnurbo;

	if( (view0.lay & G.ebase->lay)==0 ) return;

	if(okee("Separate")==0) return;
	winset(G.winar[0]);

	setcursorN(2);
	
	ob= (struct ObData *)G.ebase->d;
	cu= ob->cu;
	if(cu->key) {
		error("Can't separate with vertex keys");
		return;
	}
	
	/* we gaan de zaak als volgt neppen:
	 * 1. duplicate base: dit wordt de nieuwe,  oude pointer onthouden
	 * 2. alle NIET geselecteerde curves/nurbs apart zetten
	 * 3. load_ebaseNurb(): dit is de nieuwe base
	 * 4. freelist en oude nurbs weer terughalen
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
	
	/* apart zetten: alles wat maar enigszins NIET select is */
	editnurbo.first= editnurbo.last= 0;
	nu= editNurb.first;
	while(nu) {
		nu1= nu->next;
		if(isNurbsel(nu)==0) {
			remlink(&editNurb, nu);
			addtail(&editnurbo, nu);
		}
		nu= nu1;
	}

	load_ebaseNurb();
	
	if(editNurb.first) freeNurblist(&editNurb);
	
	editNurb= editnurbo;
	
	G.ebase= 0;	/* rare patch ivm displisten */
	if(G.basact->soort==11) makeBevelList(G.basact);
	makeDispList(G.basact);	/* de gesepareerde */
	
	G.ebase= G.basact= oldbase;
	
	make_obdups();

	setcursorN(1);

	countall();
	projektie();

	lastnu= 0;	/* voor selected */
}


/* ****************** HANDLES ************** */

/*
 *   handlcodes:
 *		1: niets,  1:auto,  2:vector,  3:aligned
 */


void calchandleNurb(bezt,prev, next)
struct BezTriple *bezt, *prev, *next;
{
	float *p1,*p2,*p3,pt[3];
	float dx1,dy1,dz1,dx,dy,dz,vx,vy,vz,len,len1,len2;
	float VecLenf();

	if(bezt->h1==0 && bezt->h2==0) return;

	p2= bezt->vec[1];

	if(prev==0) {
		p3= next->vec[1];
		pt[0]= 2*p2[0]- p3[0];
		pt[1]= 2*p2[1]- p3[1];
		pt[2]= 2*p2[2]- p3[2];
		p1= pt;
	}
	else p1= prev->vec[1];

	if(next==0) {
		pt[0]= 2*p2[0]- p1[0];
		pt[1]= 2*p2[1]- p1[1];
		pt[2]= 2*p2[2]- p1[2];
		p3= pt;
	}
	else p3= next->vec[1];

	dx= *p2 - *p1;
	dy= *(p2+1)- *(p1+1);
	dz= *(p2+2)- *(p1+2);
	len1=fsqrt(dx*dx+dy*dy+dz*dz);
	dx1= *p3 - *p2;
	dy1= *(p3+1)- *(p2+1);
	dz1= *(p3+2)- *(p2+2);
	len2=fsqrt(dx1*dx1+dy1*dy1+dz1*dz1);

	if(len1==0.0) len1=1.0;
	if(len2==0.0) len2=1.0;


	if(bezt->h1==1 || bezt->h2==1) {    /* auto */
		vx=dx1/len2+dx/len1;
		vy=dy1/len2+dy/len1;
		vz=dz1/len2+dz/len1;
		len=2.71*fsqrt(vx*vx+vy*vy+vz*vz);
		if(len!=0) {
			if(bezt->h1==1) {
				len1/=len;
				*(p2-3)= *p2-vx*len1;
				*(p2-2)= *(p2+1)-vy*len1;
				*(p2-1)= *(p2+2)-vz*len1;
			}
			if(bezt->h2==1) {
				len2/=len;
				*(p2+3)= *p2+vx*len2;
				*(p2+4)= *(p2+1)+vy*len2;
				*(p2+5)= *(p2+2)+vz*len2;
			}
		}
	}

	if(bezt->h1==2) {	/* vector */
		dx/=3.0; 
		dy/=3.0; 
		dz/=3.0;
		*(p2-3)= *p2-dx;
		*(p2-2)= *(p2+1)-dy;
		*(p2-1)= *(p2+2)-dz;
	}
	if(bezt->h2==2) {
		dx1/=3.0; 
		dy1/=3.0; 
		dz1/=3.0;
		*(p2+3)= *p2+dx1;
		*(p2+4)= *(p2+1)+dy1;
		*(p2+5)= *(p2+2)+dz1;
	}

	len2= VecLenf(p2, p2+3);
	len1= VecLenf(p2, p2-3);
	if(len1==0.0) len1=1.0;
	if(len2==0.0) len2=1.0;
	if(bezt->f1 & 1) { /* volgorde van berekenen */
		if(bezt->h2==3) {	/* aligned */
			len= len2/len1;
			p2[3]= p2[0]+len*(p2[0]-p2[-3]);
			p2[4]= p2[1]+len*(p2[1]-p2[-2]);
			p2[5]= p2[2]+len*(p2[2]-p2[-1]);
		}
		if(bezt->h1==3) {
			len= len1/len2;
			p2[-3]= p2[0]+len*(p2[0]-p2[3]);
			p2[-2]= p2[1]+len*(p2[1]-p2[4]);
			p2[-1]= p2[2]+len*(p2[2]-p2[5]);
		}
	}
	else {
		if(bezt->h1==3) {
			len= len1/len2;
			p2[-3]= p2[0]+len*(p2[0]-p2[3]);
			p2[-2]= p2[1]+len*(p2[1]-p2[4]);
			p2[-1]= p2[2]+len*(p2[2]-p2[5]);
		}
		if(bezt->h2==3) {	/* aligned */
			len= len2/len1;
			p2[3]= p2[0]+len*(p2[0]-p2[-3]);
			p2[4]= p2[1]+len*(p2[1]-p2[-2]);
			p2[5]= p2[2]+len*(p2[2]-p2[-1]);
		}
	}
}

void calchandlesNurb(nu) /* wel eerst (zonodig) de handlevlaggen zetten */
struct Nurb *nu;
{
	struct BezTriple *bezt, *prev, *next;
	short a;

	if((nu->type & 7)!=1) return;
	if(nu->pntsu<2) return;
	
	a= nu->pntsu;
	bezt= nu->bezt;
	if(nu->flagu & 1) prev= bezt+(a-1);
	else prev= 0;
	next= bezt+1;

	while(a--) {
		calchandleNurb(bezt, prev, next);
		prev= bezt;
		if(a==1) {
			if(nu->flagu & 1) next= nu->bezt;
			else next= 0;
		}
		else next++;

		bezt++;
	}
}

void testhandlesNurb(nu)
struct Nurb *nu;
{
	/* Te gebruiken als er iets an de handles is veranderd.
	 * Loopt alle BezTriples af met de volgende regels:
     * FASE 1: types veranderen?
     *  Autocalchandles: worden ligned als NOT(000 || 111)
     *  Vectorhandles worden 'niets' als (selected en andere niet) 
     * FASE 2: handles herbereken
     */
	struct BezTriple *bezt;
	short flag, a;

	if((nu->type & 7)!=1) return; /* geen bezier */

	bezt= nu->bezt;
	a= nu->pntsu;
	while(a--) {
		flag= 0;
		if(bezt->f1 & 1) flag++;
		if(bezt->f2 & 1) flag += 2;
		if(bezt->f3 & 1) flag += 4;

		if( !(flag==0 || flag==7) ) {
			if(bezt->h1==1) {   /* auto */
				bezt->h1= 3;
			}
			if(bezt->h2==1) {   /* auto */
				bezt->h2= 3;
			}

			if(bezt->h1==2) {   /* vector */
				if(flag < 4) bezt->h1= 0;
			}
			if(bezt->h2==2) {   /* vector */
				if( flag > 3) bezt->h2= 0;
			}
		}
		bezt++;
	}

	calchandlesNurb(nu);
}

void autocalchandlesNurb(struct Nurb *nu, int flag)
{
	/* Kijkt naar de coordinaten van de handles en berekent de soort */
	
	struct BezTriple *bezt2, *bezt1, *bezt0;
	float DistVL2Dfl(), VecLenf();
	long i, align, leftsmall, rightsmall;

	if(nu==0 || nu->bezt==0) return;
	
	bezt2 = nu->bezt;
	bezt1 = bezt2 + (nu->pntsu-1);
	bezt0 = bezt1 - 1;
	i = nu->pntsu;

	while(i--) {
		
		align= leftsmall= rightsmall= 0;
		
		/* linker handle: */
		if(flag==0 || (bezt1->f1 & flag) ) {
			bezt1->h1= 0;
			/* afstand te klein: vectorhandle */
			if( VecLenf( bezt1->vec[1], bezt0->vec[1] ) < 8.0) {
				bezt1->h1= HD_VECT;
				leftsmall= 1;
			}
			else {
				/* aligned handle? */
				if(DistVL2Dfl(bezt1->vec[1], bezt1->vec[0], bezt1->vec[2]) < 8.0) {
					align= 1;
					bezt1->h1= HD_ALIGN;
				}
				/* of toch vector handle? */
				if(DistVL2Dfl(bezt1->vec[0], bezt1->vec[1], bezt0->vec[1]) < 8.0)
					bezt1->h1= HD_VECT;
				
			}
		}
		/* rechter handle: */
		if(flag==0 || (bezt1->f3 & flag) ) {
			bezt1->h2= 0;
			/* afstand te klein: vectorhandle */
			if( VecLenf( bezt1->vec[1], bezt2->vec[1] ) < 8.0) {
				bezt1->h2= HD_VECT;
				rightsmall= 1;
			}
			else {
				/* aligned handle? */
				if(align) bezt1->h2= HD_ALIGN;

				/* of toch vector handle? */
				if(DistVL2Dfl(bezt1->vec[2], bezt1->vec[1], bezt2->vec[1]) < 8.0)
					bezt1->h2= HD_VECT;
				
			}
		}
		if(leftsmall && bezt1->h2==HD_ALIGN) bezt1->h2= 0;
		if(rightsmall && bezt1->h1==HD_ALIGN) bezt1->h1= 0;
		
		/* onzalige combinatie: */
		if(bezt1->h1==HD_ALIGN && bezt1->h2==HD_VECT) bezt1->h1= 0;
		if(bezt1->h2==HD_ALIGN && bezt1->h1==HD_VECT) bezt1->h2= 0;
		
		bezt0= bezt1;
		bezt1= bezt2;
		bezt2++;
	}

	calchandlesNurb(nu);
}

void autocalchandlesNurb_all(flag)
int flag;
{
	struct Nurb *nu;
	
	nu= editNurb.first;
	while(nu) {
		autocalchandlesNurb(nu, flag);
		nu= nu->next;
	}
}

void sethandlesNurb(code)
short code;
{
	/* code==1: set autohandle */
	/* code==2: set vectorhandle */
	/* als code==3 (HD_ALIGN) toggelt het, vectorhandles worden HD_FREE */
	struct Nurb *nu;
	struct BezTriple *bezt;
	short a, ok=0;

	if(code==1 || code==2) {
		nu= editNurb.first;
		while(nu) {
			if( (nu->type & 7)==1) {
				bezt= nu->bezt;
				a= nu->pntsu;
				while(a--) {
					if(bezt->f1) bezt->h1= code;
					if(bezt->f3) bezt->h2= code;
					bezt++;
				}
				calchandlesNurb(nu);
			}
			nu= nu->next;
		}
	}
	else {
		/* is er 1 handle NIET vrij: alles vrijmaken, else ALIGNED maken */
		
		nu= editNurb.first;
		while(nu) {
			if( (nu->type & 7)==1) {
				bezt= nu->bezt;
				a= nu->pntsu;
				while(a--) {
					if(bezt->f1 && bezt->h1) ok= 1;
					if(bezt->f3 && bezt->h2) ok= 1;
					if(ok) break;
					bezt++;
				}
			}
			nu= nu->next;
		}
		if(ok) ok= HD_FREE;
		else ok= HD_ALIGN;
		
		nu= editNurb.first;
		while(nu) {
			if( (nu->type & 7)==1) {
				bezt= nu->bezt;
				a= nu->pntsu;
				while(a--) {
					if(bezt->f1) bezt->h1= ok;
					if(bezt->f3 ) bezt->h2= ok;
	
					bezt++;
				}
				calchandlesNurb(nu);
			}
			nu= nu->next;
		}
	}
}

/* ******************* FLAGS ********************* */


short isNurbselUV(nu, u, v, flag)
struct Nurb *nu;
long *u, *v, flag;
{
	/* return u!=-1:   1 rij in u-richting geselecteerd. U heeft de waarde tussen 0-pntsv 
     * return v!=-1: 1 kolom in v-richting geselecteerd. V heeft de waarde tussen 0-pntsu 
     */
	struct BPoint *bp;
	long a, b, sel;

	*u= *v= -1;

	bp= nu->bp;
	for(b=0; b<nu->pntsv; b++) {
		sel= 0;
		for(a=0; a<nu->pntsu; a++, bp++) {
			if(bp->f1 & flag) sel++;
		}
		if(sel==nu->pntsu) {
			if(*u== -1) *u= b;
			else return 0;
		}
		else if(sel>1) return 0;    /* want sel==1 is nog goed */
	}

	for(a=0; a<nu->pntsu; a++) {
		sel= 0;
		bp= nu->bp+a;
		for(b=0; b<nu->pntsv; b++, bp+=nu->pntsu) {
			if(bp->f1 & flag) sel++;
		}
		if(sel==nu->pntsv) {
			if(*v== -1) *v= a;
			else return 0;
		}
		else if(sel>1) return 0;
	}

	if(*u==-1 && *v>-1) return 1;
	if(*v==-1 && *u>-1) return 1;
	return 0;
}

void setflagsNurb(flag)
short flag;
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a;

	nu= editNurb.first;
	while(nu) {
		if( (nu->type & 7)==1) {
			a= nu->pntsu;
			bezt= nu->bezt;
			while(a--) {
				bezt->f1= bezt->f2= bezt->f3= flag;
				bezt++;
			}
		}
		else {
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {
				bp->f1= flag;
				bp++;
			}
		}
		nu= nu->next;
	}
}

void rotateflagNurb(flag,cent,rotmat)
short flag;
float *cent,*rotmat;
{
	/* alle verts met (flag & 'flag') rotate */
	struct Nurb *nu;
	struct BPoint *bp;
	long a;

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==4) {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;

			while(a--) {
				if(bp->f1 & flag) {
					bp->vec[0]-=cent[0];
					bp->vec[1]-=cent[1];
					bp->vec[2]-=cent[2];
					Mat3MulVecfl(rotmat,bp->vec);
					bp->vec[0]+=cent[0];
					bp->vec[1]+=cent[1];
					bp->vec[2]+=cent[2];
				}
				bp++;
			}
		}
		nu= nu->next;
	}
}


void translateflagNurb(flag, vec)
short flag;
float *vec;
{
	/* alle verts met (->f & flag) translate */
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a;

	nu= editNurb.first;
	while(nu) {
		if( (nu->type & 7)==1) {
			a= nu->pntsu;
			bezt= nu->bezt;
			while(a--) {
				if(bezt->f1 & flag) VecAddf(bezt->vec[0], bezt->vec[0], vec);
				if(bezt->f2 & flag) VecAddf(bezt->vec[1], bezt->vec[1], vec);
				if(bezt->f3 & flag) VecAddf(bezt->vec[2], bezt->vec[2], vec);
				bezt++;
			}
		}
		else {
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {
				if(bp->f1 & flag) VecAddf(bp->vec, bp->vec, vec);
				bp++;
			}
		}

		test2DNurb(nu);

		nu= nu->next;
	}
}

void weightflagNurb(flag, w, mode)	/* mode==0: vervangen, mode==1: vermenigvuldigen */
short flag;
float w;
long mode;
{
	struct Nurb *nu;
	struct BPoint *bp;
	long a;

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==4) {
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {
				if(bp->f1 & flag) {
					if(mode==1) bp->vec[3]*= w;
					else bp->vec[3]= w;
				}
				bp++;
			}
		}
		nu= nu->next;
	}
}

short deleteflagNurb(flag)
short flag;
{
	struct Nurb *nu, *next;
	struct BPoint *bp, *bpn, *new;
	long a, b, newu, newv, len, sel;

	if(G.ebase && G.ebase->soort==5);
	else return 0;

	lastselbp= 0;

	nu= editNurb.first;
	while(nu) {
		next= nu->next;

		/* is de hele nurb geselecteerd */
		bp= nu->bp;
		a= nu->pntsu*nu->pntsv;
		while(a) {
			a--;
			if(bp->f1 & flag);
			else break;
			bp++;
		}
		if(a==0) {
			freeNurb(nu);
			remlink(&editNurb, nu);
		}
		else {
			/* is de nurb in U richting geselecteerd */
			newv= nu->pntsv;
			bp= nu->bp;
			for(b=0; b<nu->pntsv; b++) {
				sel= 0;
				for(a=0; a<nu->pntsu; a++, bp++) {
					if(bp->f1 & flag) sel++;
				}
				if(sel==nu->pntsu) {
					newv--;
				}
				else if(sel>1) break;
			}
			if(newv!=nu->pntsv && b==nu->pntsv)	{
				/* deleten */
				bp= nu->bp;
				bpn=new= mallocstructN(struct BPoint, newv*nu->pntsu, "deleteNurb");
				for(b=0; b<nu->pntsv; b++) {
					if((bp->f1 & flag)==0) {
						memcpy(bpn, bp, nu->pntsu*sizeof(struct BPoint));
						bpn+= nu->pntsu;
					}
					bp+= nu->pntsu;
				}
				nu->pntsv= newv;
				freeN(nu->bp);
				nu->bp= new;
				if(nu->orderv>nu->pntsv) nu->orderv= nu->pntsv;

				makeknots(nu, 2, nu->flagv>>1);
			}
			else {
				/* is de nurb in V richting geselecteerd */
				newu= nu->pntsu;
				for(a=0; a<nu->pntsu; a++) {
					bp= nu->bp+a;
					sel= 0;
					for(b=0; b<nu->pntsv; b++, bp+=nu->pntsu) {
						if(bp->f1 & flag) sel++;
					}
					if(sel==nu->pntsv) {
						newu--;
					}
					else if(sel>1) break;
				}
				if(newu!=nu->pntsu && a==nu->pntsu)	{
					/* deleten */
					bp= nu->bp;
					bpn=new= mallocstructN(struct BPoint, newu*nu->pntsv, "deleteNurb");
					for(b=0; b<nu->pntsv; b++) {
						for(a=0; a<nu->pntsu; a++, bp++) {
							if((bp->f1 & flag)==0) {
								*bpn= *bp;
								bpn++;
							}
						}
					}
					freeN(nu->bp);
					nu->bp= new;
					if(newu==1 && nu->pntsv>1) {    /* maak een U spline */
						nu->pntsu= nu->pntsv;
						nu->pntsv= 1;
						SWAP(short, nu->orderu, nu->orderv);
						if(nu->orderu>nu->pntsu) nu->orderu= nu->pntsu;
						if(nu->knotsv) freeN(nu->knotsv);
						nu->knotsv= 0;
					}
					else {
						nu->pntsu= newu;
						if(nu->orderu>nu->pntsu) nu->orderu= nu->pntsu;
					}
					makeknots(nu, 1, nu->flagu>>1);
				}
			}
		}
		nu= next;
	}
}

short extrudeflagNurb(flag)
long flag;
{
	struct Nurb *nu;
	struct BPoint *bp, *bpn, *new;
	long ok= 0, a, b, u, v, len;

	if(G.ebase && G.ebase->soort==5);
	else return 0;

	nu= editNurb.first;
	while(nu) {

		if(nu->pntsv==1) {
			bp= nu->bp;
			a= nu->pntsu;
			while(a) {
				if(bp->f1 & flag);
				else break;
				bp++;
				a--;
			}
			if(a==0) {
				ok= 1;
				new= mallocstructN(struct BPoint, 2*nu->pntsu, "extrudeNurb1");
				memcpy(new, nu->bp, nu->pntsu*sizeof(struct BPoint) );
				bp= new+ nu->pntsu;
				memcpy(bp, nu->bp, nu->pntsu*sizeof(struct BPoint) );
				freeN(nu->bp);
				nu->bp= new;
				a= nu->pntsu;
				while(a--) {
					bp->f1 |= flag;
					new->f1 &= ~flag;
					bp++; 
					new++;
				}

				nu->pntsv= 2;
				nu->orderv= 2;
				makeknots(nu, 2, nu->flagv>>1);
			}
		}
		else {
			/* welke rij of kolom is geselecteerd */

			if( isNurbselUV(nu, &u, &v, flag) ) {

				/* alles deselecteren */
				bp= nu->bp;
				a= nu->pntsu*nu->pntsv;
				while(a--) {
					bp->f1 &= ~flag;
					bp++;
				}

				if(u==0 || u== nu->pntsv-1) {	    /* rij in u-richting geselecteerd */
					ok= 1;
					new= mallocstructN(struct BPoint, nu->pntsu*(nu->pntsv+1), "extrudeNurb1");
					if(u==0) {
						len= nu->pntsv*nu->pntsu;
						memcpy(new+nu->pntsu, nu->bp, len*sizeof(struct BPoint) );
						memcpy(new, nu->bp, nu->pntsu*sizeof(struct BPoint) );
						bp= new;
					}
					else {
						len= nu->pntsv*nu->pntsu;
						memcpy(new, nu->bp, len*sizeof(struct BPoint) );
						memcpy(new+len, nu->bp+len-nu->pntsu, nu->pntsu*sizeof(struct BPoint) );
						bp= new+len;
					}

					a= nu->pntsu;
					while(a--) {
						bp->f1 |= flag;
						bp++;
					}

					freeN(nu->bp);
					nu->bp= new;
					nu->pntsv++;
					makeknots(nu, 2, nu->flagv>>1);
				}
				else if(v==0 || v== nu->pntsu-1) {	    /* kolom in v-richting geselecteerd */
					ok= 1;
					bpn=new= mallocstructN(struct BPoint, (nu->pntsu+1)*nu->pntsv, "extrudeNurb1");
					bp= nu->bp;

					for(a=0; a<nu->pntsv; a++) {
						if(v==0) {
							*bpn= *bp;
							bpn->f1 |= flag;
							bpn++;
						}
						memcpy(bpn, bp, nu->pntsu*sizeof(struct BPoint));
						bp+= nu->pntsu;
						bpn+= nu->pntsu;
						if(v== nu->pntsu-1) {
							*bpn= *(bp-1);
							bpn->f1 |= flag;
							bpn++;
						}
					}

					freeN(nu->bp);
					nu->bp= new;
					nu->pntsu++;
					makeknots(nu, 1, nu->flagu>>1);
				}
			}
		}
		nu= nu->next;
	}

	return ok;
}


void adduplicateflagNurb(flag)
short flag;
{
	struct Nurb *nu, *new;
	struct BezTriple *bezt, *bezt1;
	struct BPoint *bp, *bp1;
	long a, b, starta, enda, newu, newv;
	char *usel;

	nu= editNurb.last;
	while(nu) {
		if( (nu->type & 7)==1) {
			bezt= nu->bezt;
			for(a=0; a<nu->pntsu; a++) {
				enda= -1;
				starta= a;
				while( (bezt->f1 & flag) || (bezt->f2 & flag) || (bezt->f3 & flag) ) {
					bezt->f1 &= ~flag;
					bezt->f2 &= ~flag;
					bezt->f3 &= ~flag;
					enda=a;
					if(a>=nu->pntsu-1) break;
					a++;
					bezt++;
				}
				if(enda>=starta) {
					new= mallocstructN(struct Nurb, 1, "adduplicateN");
					memcpy(new, nu, sizeof(struct Nurb));
					addtail(&editNurb, new);
					new->pntsu= enda-starta+1;
					new->bezt= mallocstructN(struct BezTriple, (enda-starta+1), "adduplicateN1");
					memcpy(new->bezt, nu->bezt+starta, new->pntsu*sizeof(struct BezTriple));

					b= new->pntsu;
					bezt1= new->bezt;
					while(b--) {
						bezt1->f1 |= flag;
						bezt1->f2 |= flag;
						bezt1->f3 |= flag;
						bezt1++;
					}

					if(nu->flagu & 1) {
						if(starta!=0 || enda!=nu->pntsu-1) new->flagu--;
					}
				}
				bezt++;
			}
		}
		else if(nu->pntsv==1) {	/* want UV Nurb heeft andere duplimethode */
			bp= nu->bp;
			for(a=0; a<nu->pntsu; a++) {
				enda= -1;
				starta= a;
				while(bp->f1 & flag) {
					bp->f1 &= ~flag;
					enda= a;
					if(a>=nu->pntsu-1) break;
					a++;
					bp++;
				}
				if(enda>=starta) {
					new= mallocstructN(struct Nurb, 1, "adduplicateN3");
					memcpy(new, nu, sizeof(struct Nurb));
					addtail(&editNurb, new);
					new->pntsu= enda-starta+1;
					new->bp= mallocstructN(struct BPoint, (enda-starta+1), "adduplicateN4");
					memcpy(new->bp, nu->bp+starta, new->pntsu*sizeof(struct BPoint));

					b= new->pntsu;
					bp1= new->bp;
					while(b--) {
						bp1->f1 |= flag;
						bp1++;
					}

					if(nu->flagu & 1) {
						if(starta!=0 || enda!=nu->pntsu-1) new->flagu--;
					}

					/* knots */
					new->knotsu= 0;
					makeknots(new, 1, new->flagu>>1);
				}
				bp++;
			}
		}
		else {
			/* een rechthoekig gebied in de nurb moet geselecteerd zijn */
			if(isNurbsel(nu)) {
				usel= callocN(nu->pntsu, "adduplicateN4");
				bp= nu->bp;
				for(a=0; a<nu->pntsv; a++) {
					for(b=0; b<nu->pntsu; b++, bp++) {
						if(bp->f1 & flag) usel[b]++;
					}
				}
				newu= 0;
				newv= 0;
				for(a=0; a<nu->pntsu; a++) {
					if(usel[a]) {
						if(newv==0 || usel[a]==newv) {
							newv= usel[a];
							newu++;
						}
						else {
							newv= 0;
							break;
						}
					}
				}
				if(newu==0 || newv==0) {
					printf("Can't duplicate Nurb\n");
				}
				else {

					if(newu==1) SWAP(short, newu, newv);

					new= mallocstructN(struct Nurb, 1, "adduplicateN5");
					memcpy(new, nu, sizeof(struct Nurb));
					addtail(&editNurb, new);
					new->pntsu= newu;
					new->pntsv= newv;
					new->bp= mallocstructN(struct BPoint, newu*newv, "adduplicateN6");
					new->orderu= MIN2(nu->orderu, newu);
					new->orderv= MIN2(nu->orderv, newv);

					bp= new->bp;
					bp1= nu->bp;
					for(a=0; a<nu->pntsv; a++) {
						for(b=0; b<nu->pntsu; b++, bp1++) {
							if(bp1->f1 & flag) {
								memcpy(bp, bp1, sizeof(struct BPoint));
								bp1->f1 &= ~flag;
								bp++;
							}
						}
					}
					if(nu->pntsu==new->pntsu) {
						new->knotsu= mallocN(4*KNOTSU(nu), "adduplicateN6");
						memcpy(new->knotsu, nu->knotsu, 4*KNOTSU(nu));
					}
					else {
						new->knotsu= 0;
						makeknots(new, 1, new->flagu>>1);
					}
					if(nu->pntsv==new->pntsv) {
						new->knotsv= mallocN(4*KNOTSV(nu), "adduplicateN7");
						memcpy(new->knotsv, nu->knotsv, 4*KNOTSV(nu));
					}
					else {
						new->knotsv= 0;
						makeknots(new, 2, new->flagv>>1);
					}

				}
				freeN(usel);
			}
		}

		nu= nu->prev;
	}
}

void swapdata(adr1, adr2, len)
void *adr1, *adr2;
long len;
{

	if(len<=0) return;

	if(len<65) {
		char adr[64];

		memcpy(adr, adr1, len);
		memcpy(adr1, adr2, len);
		memcpy(adr2, adr, len);
	}
	else {
		char *adr;

		adr= (char *)malloc(len);
		memcpy(adr, adr1, len);
		memcpy(adr1, adr2, len);
		memcpy(adr2, adr, len);
		free(adr);
	}
}

void switchdirectionNurb(nu)
struct Nurb *nu;
{
	struct BezTriple *bezt1, *bezt2;
	long ltemp;
	struct BPoint *bp1, *bp2;
	float *fp1, *fp2, *tempf;
	long a;

	if((nu->type & 7)==1) {
		a= nu->pntsu;
		bezt1= nu->bezt;
		bezt2= bezt1+(a-1);
		if(a & 1) a+= 1;	/* bij oneven ook van middelste inhoud swappen */
		a/= 2;
		while(a>0) {
			if(bezt1!=bezt2) SWAP(struct BezTriple, *bezt1, *bezt2);

			swapdata(bezt1->vec[0], bezt1->vec[2], 12);
			if(bezt1!=bezt2) swapdata(bezt2->vec[0], bezt2->vec[2], 12);

			SWAP(char, bezt1->h1, bezt1->h2);
			SWAP(short, bezt1->f1, bezt1->f3);
			
			if(bezt1!=bezt2) {
				SWAP(char, bezt2->h1, bezt2->h2);
				SWAP(short, bezt2->f1, bezt2->f3);
			}
			a--;
			bezt1++; 
			bezt2--;
		}
	}
	else if(nu->pntsv==1) {
		a= nu->pntsu;
		bp1= nu->bp;
		bp2= bp1+(a-1);
		a/= 2;
		while(bp1!=bp2 && a>0) {
			SWAP(struct BPoint, *bp1, *bp2);
			a--;
			bp1++; 
			bp2--;
		}
		if((nu->type & 7)==4) {
			/* de knots omkeren*/
			a= KNOTSU(nu);
			fp1= nu->knotsu;
			fp2= fp1+(a-1);
			a/= 2;
			while(fp1!=fp2 && a>0) {
				SWAP(float, *fp1, *fp2);
				a--;
				fp1++; 
				fp2--;
			}
			/* en weer in stijgende lijn maken */
			a= KNOTSU(nu);
			fp1= nu->knotsu;
			fp2=tempf= mallocN(4*a, "switchdirect");
			while(a--) {
				fp2[0]= fabs(fp1[1]-fp1[0]);
				fp1++;
				fp2++;
			}
	
			a= KNOTSU(nu)-1;
			fp1= nu->knotsu;
			fp2= tempf;
			fp1[0]= 0.0;
			fp1++;
			while(a--) {
				fp1[0]= fp1[-1]+fp2[0];
				fp1++;
				fp2++;
			}
			freeN(tempf);
		}
	}
}

/* **************** EDIT ************************ */

void hideNurb()
{
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	long a, sel;

	if(G.ebase==0) return;

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==1) {
			bezt= nu->bezt;
			a= nu->pntsu;
			sel= 0;
			while(a--) {
				if(BEZSELECTED(bezt)) {
					sel++;
					bezt->f1 &= ~1;
					bezt->f2 &= ~1;
					bezt->f3 &= ~1;
					bezt->hide= 1;
				}
				bezt++;
			}
			if(sel==nu->pntsu) nu->hide= 1;
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			sel= 0;
			while(a--) {
				if(bp->f1 & 1) {
					bp->f1 &= ~1;
					bp->hide= 1;
					sel++;
				}
				bp++;
			}
			if(sel==nu->pntsu*nu->pntsv) nu->hide= 1;
		}
		nu= nu->next;
	}

	makeDispList(G.ebase);
	projektie();
}

void revealNurb()
{
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	long a;

	if(G.ebase==0) return;

	nu= editNurb.first;
	while(nu) {
		nu->hide= 0;
		if((nu->type & 7)==1) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				if(bezt->hide) {
					bezt->f1 |= 1;
					bezt->f2 |= 1;
					bezt->f3 |= 1;
					bezt->hide= 0;
				}
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				if(bp->hide) {
					bp->f1 |= 1;
					bp->hide= 0;
				}
				bp++;
			}
		}
		nu= nu->next;
	}

	makeDispList(G.ebase);
	projektie();
}

void selectswapNurb()
{
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	long a;

	if(G.ebase==0) return;

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==1) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				if(bezt->hide==0) {
					if(bezt->f1 & 1) bezt->f1 &= ~1; 
					else bezt->f1 |= 1;
					if(bezt->f2 & 1) bezt->f2 &= ~1; 
					else bezt->f2 |= 1;
					if(bezt->f3 & 1) bezt->f3 &= ~1; 
					else bezt->f3 |= 1;
				}
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				if(bp->hide==0) {
					if(bp->f1 & 1) bp->f1 &= ~1; 
					else bp->f1 |= 1;
				}
				bp++;
			}
		}
		nu= nu->next;
	}

	projektie();
}

void subdivideNurb()
{
	struct Nurb *nu;
	struct BezTriple *prevbezt, *bezt, *beztnew, *beztn;
	struct BPoint *bp, *prevbp, *bpnew, *bpn;
	float vec[12];
	long a, b, sel, aantal, *usel, *vsel;

	nu= editNurb.first;
	while(nu) {
		aantal= 0;
		if((nu->type & 7)==1) {
			/* tellen */
			if(nu->flagu & 1) {
				a= nu->pntsu;
				bezt= nu->bezt;
				prevbezt= bezt+(a-1);
			}
			else {
				a= nu->pntsu-1;
				prevbezt= nu->bezt;
				bezt= prevbezt+1;
			}
			while(a--) {
				if( BEZSELECTED(prevbezt) && BEZSELECTED(bezt) ) aantal++;
				prevbezt= bezt;
				bezt++;
			}

			if(aantal) {
				/* inserten */
				beztnew= mallocstructN(struct BezTriple, aantal+nu->pntsu, "subdivNurb");
				beztn= beztnew;
				if(nu->flagu & 1) {
					a= nu->pntsu;
					bezt= nu->bezt;
					prevbezt= bezt+(a-1);
				}
				else {
					a= nu->pntsu-1;
					prevbezt= nu->bezt;
					bezt= prevbezt+1;
				}
				while(a--) {
					memcpy(beztn, prevbezt, sizeof(struct BezTriple));
					beztn++;

					if( BEZSELECTED(prevbezt) && BEZSELECTED(bezt) ) {
						memcpy(beztn, bezt, sizeof(struct BezTriple));
						maakbez(prevbezt->vec[1][0],prevbezt->vec[2][0],
						    bezt->vec[0][0],bezt->vec[1][0],vec,2);
						maakbez(prevbezt->vec[1][1],prevbezt->vec[2][1],
						    bezt->vec[0][1],bezt->vec[1][1],vec+1,2);
						maakbez(prevbezt->vec[1][2],prevbezt->vec[2][2],
						    bezt->vec[0][2],bezt->vec[1][2],vec+2,2);
						VECCOPY(beztn->vec[1], vec+3);
						beztn->h1= beztn->h2= 1;
						beztn++;
					}

					prevbezt= bezt;
					bezt++;
				}
				/* laatste punt */
				if((nu->flagu & 1)==0) memcpy(beztn, prevbezt, sizeof(struct BezTriple));

				freeN(nu->bezt);
				nu->bezt= beztnew;
				nu->pntsu+= aantal;

				calchandlesNurb(nu);
			}
		}
		else if(nu->pntsv==1) {
			/* tellen */
			if(nu->flagu & 1) {
				a= nu->pntsu*nu->pntsv;
				bp= nu->bp;
				prevbp= bp+(a-1);
			}
			else {
				a= nu->pntsu-1;
				prevbp= nu->bp;
				bp= prevbp+1;
			}
			while(a--) {
				if( (bp->f1 & 1) && (prevbp->f1 & 1) ) aantal++;
				prevbp= bp;
				bp++;
			}

			if(aantal) {
				/* inserten */
				bpnew= mallocstructN(struct BPoint, aantal+nu->pntsu, "subdivNurb2");
				bpn= bpnew;

				if(nu->flagu & 1) {
					a= nu->pntsu;
					bp= nu->bp;
					prevbp= bp+(a-1);
				}
				else {
					a= nu->pntsu-1;
					prevbp= nu->bp;
					bp= prevbp+1;
				}
				while(a--) {
					memcpy(bpn, prevbp, sizeof(struct BPoint));
					bpn++;

					if( (bp->f1 & 1) && (prevbp->f1 & 1) ) {
						memcpy(bpn, bp, sizeof(struct BPoint));
						bpn->vec[0]= (prevbp->vec[0]+bp->vec[0])/2.0;
						bpn->vec[1]= (prevbp->vec[1]+bp->vec[1])/2.0;
						bpn->vec[2]= (prevbp->vec[2]+bp->vec[2])/2.0;
						bpn->vec[3]= (prevbp->vec[3]+bp->vec[3])/2.0;
						bpn++;

					}
					prevbp= bp;
					bp++;
				}
				if((nu->flagu & 1)==0) memcpy(bpn, prevbp, sizeof(struct BPoint));	/* laatste punt */

				freeN(nu->bp);
				nu->bp= bpnew;
				nu->pntsu+= aantal;

				if(nu->type & 4) {
					makeknots(nu, 1, nu->flagu>>1);
				}
			}
		}
		else if((nu->type & 7)==4) {
			/* selecteer-arrays aanleggen */
			usel= callocN(4*nu->pntsu, "subivideNurb3");
			vsel= callocN(4*nu->pntsv, "subivideNurb3");
			sel= 0;

			bp= nu->bp;
			for(a=0; a<nu->pntsv; a++) {
				for(b=0; b<nu->pntsu; b++) {
					if(bp->f1 & 1) {
						usel[b]++;
						vsel[a]++;
						sel++;
					}
					bp++;
				}
			}
			if(sel== nu->pntsu*nu->pntsv) {	/* hele nurb subdividen */
				bpn=bpnew= mallocN( (2*nu->pntsu-1)*(2*nu->pntsv-1)*sizeof(struct BPoint), "subdivideNurb4");
				bp= nu->bp;
				/* eerst de rijen subdividen */
				for(a=0; a<nu->pntsv; a++) {
					for(b=0; b<nu->pntsu; b++) {
						*bpn= *bp;
						bpn++; 
						bp++;
						if(b<nu->pntsu-1) {
							*bpn= *bp;
							prevbp= bp-1;
							bpn->vec[0]= (prevbp->vec[0]+bp->vec[0])/2.0;
							bpn->vec[1]= (prevbp->vec[1]+bp->vec[1])/2.0;
							bpn->vec[2]= (prevbp->vec[2]+bp->vec[2])/2.0;
							bpn->vec[3]= (prevbp->vec[3]+bp->vec[3])/2.0;
							bpn++;
						}
					}
					bpn+= (2*nu->pntsu-1);
				}
				/* nu nieuwe invoegen */
				bpn= bpnew+(2*nu->pntsu-1);
				bp= bpnew+(4*nu->pntsu-2);
				prevbp= bpnew;
				for(a=1; a<nu->pntsv; a++) {

					for(b=0; b<2*nu->pntsu-1; b++) {
						*bpn= *bp;
						bpn->vec[0]= (prevbp->vec[0]+bp->vec[0])/2.0;
						bpn->vec[1]= (prevbp->vec[1]+bp->vec[1])/2.0;
						bpn->vec[2]= (prevbp->vec[2]+bp->vec[2])/2.0;
						bpn->vec[3]= (prevbp->vec[3]+bp->vec[3])/2.0;
						bpn++; 
						bp++; 
						prevbp++;
					}
					bp+= (2*nu->pntsu-1);
					bpn+= (2*nu->pntsu-1);
					prevbp+= (2*nu->pntsu-1);
				}
				freeN(nu->bp);
				nu->bp= bpnew;
				nu->pntsu= 2*nu->pntsu-1;
				nu->pntsv= 2*nu->pntsv-1;
				makeknots(nu, 1, nu->flagu>>1);
				makeknots(nu, 2, nu->flagv>>1);
			}
			else {
				/* in v richting subdividen? */
				sel= 0;
				for(a=0; a<nu->pntsv-1; a++) {
					if(vsel[a]==nu->pntsu && vsel[a+1]==nu->pntsu) sel++;
				}

				if(sel) {   /* V ! */
					bpn=bpnew= mallocN( (sel+nu->pntsv)*nu->pntsu*sizeof(struct BPoint), "subdivideNurb4");
					bp= nu->bp;
					for(a=0; a<nu->pntsv; a++) {
						for(b=0; b<nu->pntsu; b++) {
							*bpn= *bp;
							bpn++; 
							bp++;
						}
						if( (a<nu->pntsv-1) && vsel[a]==nu->pntsu && vsel[a+1]==nu->pntsu ) {
							prevbp= bp- nu->pntsu;
							for(b=0; b<nu->pntsu; b++) {
								*bpn= *prevbp;
								bpn->vec[0]= (prevbp->vec[0]+bp->vec[0])/2.0;
								bpn->vec[1]= (prevbp->vec[1]+bp->vec[1])/2.0;
								bpn->vec[2]= (prevbp->vec[2]+bp->vec[2])/2.0;
								bpn->vec[3]= (prevbp->vec[3]+bp->vec[3])/2.0;
								bpn++;
								prevbp++;
								bp++;
							}
							bp-= nu->pntsu;
						}
					}
					freeN(nu->bp);
					nu->bp= bpnew;
					nu->pntsv+= sel;
					makeknots(nu, 2, nu->flagv>>1);
				}
				else {
					/* of in u richting? */
					sel= 0;
					for(a=0; a<nu->pntsu-1; a++) {
						if(usel[a]==nu->pntsv && usel[a+1]==nu->pntsv) sel++;
					}

					if(sel) {	/* U ! */
						bpn=bpnew= mallocN( (sel+nu->pntsu)*nu->pntsv*sizeof(struct BPoint), "subdivideNurb4");
						bp= nu->bp;
						for(a=0; a<nu->pntsv; a++) {
							for(b=0; b<nu->pntsu; b++) {
								*bpn= *bp;
								bpn++; 
								bp++;
								if( (b<nu->pntsu-1) && usel[b]==nu->pntsv && usel[b+1]==nu->pntsv ) {
									prevbp= bp- 1;
									*bpn= *prevbp;
									bpn->vec[0]= (prevbp->vec[0]+bp->vec[0])/2.0;
									bpn->vec[1]= (prevbp->vec[1]+bp->vec[1])/2.0;
									bpn->vec[2]= (prevbp->vec[2]+bp->vec[2])/2.0;
									bpn->vec[3]= (prevbp->vec[3]+bp->vec[3])/2.0;
									bpn++;
								}
							}
						}
						freeN(nu->bp);
						nu->bp= bpnew;
						nu->pntsu+= sel;
						makeknots(nu, 1, nu->flagu>>1);
					}
				}
			}
			freeN(usel); 
			freeN(vsel);
		}
		nu= nu->next;
	}

	makeDispList(G.basact);
	projektie();
}


short findnearestNurbvert(sel,nurb, bezt, bp)
short sel;
struct Nurb **nurb;
struct BezTriple **bezt;
struct BPoint **bp;
{
	/* sel==1: selected krijgen een nadeel */
	/* in nurb en bezt of bp wordt nearest weggeschreven */
	/* return 0 1 2: handlepunt */
	struct Nurb *nu;
	struct BezTriple *bezt1;
	struct BPoint *bp1;
	short dist= 100, temp, mval[2], a, hpoint=0;

	*nurb= 0;
	*bezt= 0;
	*bp= 0;

	winset(G.winar[0]);
	getmouseco(mval);

	nu= editNurb.first;
	while(nu) {
		if((nu->type & 7)==1) {
			bezt1= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				if(bezt1->hide==0) {
					temp= abs(mval[0]- bezt1->s[0][0])+ abs(mval[1]- bezt1->s[0][1]);
					if( (bezt1->f1 & 1)==sel) temp+=5;
					if(temp<dist) { 
						hpoint=0; 
						*bezt=bezt1; 
						dist= temp; 
						*nurb= nu; 
						*bp= 0; 
					}

					/* middelste punten een klein nadeel */
					temp= 3+abs(mval[0]- bezt1->s[1][0])+ abs(mval[1]- bezt1->s[1][1]);
					if( (bezt1->f2 & 1)==sel) temp+=5;
					if(temp<dist) { 
						hpoint=1; 
						*bezt=bezt1; 
						dist= temp; 
						*nurb= nu; 
						*bp= 0; 
					}

					temp= abs(mval[0]- bezt1->s[2][0])+ abs(mval[1]- bezt1->s[2][1]);
					if( (bezt1->f3 & 1)==sel) temp+=5;
					if(temp<dist) { 
						hpoint=2; 
						*bezt=bezt1; 
						dist= temp; 
						*nurb= nu; 
						*bp= 0; 
					}
				}
				bezt1++;
			}
		}
		else {
			bp1= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				if(bp1->hide==0) {
					temp= abs(mval[0]- bp1->s[0])+ abs(mval[1]- bp1->s[1]);
					if( (bp1->f1 & 1)==sel) temp+=5;
					if(temp<dist) { 
						hpoint=0; 
						*bp=bp1; 
						dist= temp; 
						*nurb= nu; 
						*bezt= 0; 
					}
				}
				bp1++;
			}
		}
		nu= nu->next;
	}

	return hpoint;
}


void findselectedNurbvert(nu, bezt, bp)
struct Nurb **nu;
struct BezTriple **bezt;
struct BPoint **bp;
{
	/* in nu en (bezt of bp) wordt selected weggeschreven als er 1 sel. is */
	/* als er meer punten in 1 spline selected: alleen nu terug, bezt en bp zijn 0 */
	struct Nurb *nu1;
	struct BezTriple *bezt1;
	struct BPoint *bp1;
	short a;

	*nu= 0;
	*bezt= 0;
	*bp= 0;
	nu1= editNurb.first;
	while(nu1) {
		if((nu1->type & 7)==1) {
			bezt1= nu1->bezt;
			a= nu1->pntsu;
			while(a--) {
				if( (bezt1->f1 & 1) || (bezt1->f2 & 1) || (bezt1->f3 & 1) ) {
					if(*nu!=0 && *nu!= nu1) {
						*nu= 0;
						*bp= 0;
						*bezt= 0;
						return;
					}
					else if(*bezt || *bp) {
						*bp= 0;
						*bezt= 0;
					}
					else {
						*bezt= bezt1;
						*nu= nu1;
					}
				}
				bezt1++;
			}
		}
		else {
			bp1= nu1->bp;
			a= nu1->pntsu*nu1->pntsv;
			while(a--) {
				if( bp1->f1 & 1 ) {
					if(*nu!=0 && *nu!= nu1) {
						*bp= 0;
						*bezt= 0;
						*nu= 0;
						return;
					}
					else if(*bezt || *bp) {
						*bp= 0;
						*bezt= 0;
					}
					else {
						*bp= bp1;
						*nu= nu1;
					}
				}
				bp1++;
			}
		}
		nu1= nu1->next;
	}
}

void setsplinetype(type)
short type;
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	long a, c, nr;

	nu= editNurb.first;
	while(nu) {
		if(isNurbsel(nu)) {

			if((nu->type & 7)==0) {		/* Poly */
				if(type==1) {			    /* naar Bezier met vecthandles  */
					nr= nu->pntsu;
					bezt= callocstructN(struct BezTriple, nr, "setsplinetype2");
					nu->bezt= bezt;
					a= nr;
					bp= nu->bp;
					while(a--) {
						VECCOPY(bezt->vec[1], bp->vec);
						bezt->f1=bezt->f2=bezt->f3= bp->f1;
						bezt->h1= bezt->h2= 2;
						bp++;
						bezt++;
					}
					freeN(nu->bp);
					nu->bp= 0;
					nu->pntsu= nr;
					nu->type &= ~7;
					nu->type |= 1;
					calchandlesNurb(nu);
				}
				else if(type==4) {		    /* naar Nurb */
					nu->type &= ~7;
					nu->type+= 4;
					nu->orderu= 4;
					nu->flagu &= 1;
					nu->flagu += 4;
					makeknots(nu, 1, nu->flagu>>1);
					a= nu->pntsu*nu->pntsv;
					bp= nu->bp;
					while(a--) {
						bp->vec[3]= 1.0;
						bp++;
					}
				}
			}
			else if((nu->type & 7)==1) {	/* Bezier */
				if(type==0 || type==4) {	    /* naar Poly of Nurb */
					nr= 3*nu->pntsu;
					nu->bp= callocstructN(struct BPoint, nr, "setsplinetype");
					a= nu->pntsu;
					bezt= nu->bezt;
					bp= nu->bp;
					while(a--) {
						if(type==0 && bezt->h1==2 && bezt->h2==2) {
							/* vectorhandle wordt 1 polyvert */
							VECCOPY(bp->vec, bezt->vec[1]);
							bp->vec[3]= 1.0;
							bp->f1= bezt->f2;
							nr-= 2;
							bp++;
						}
						else {
							for(c=0;c<3;c++) {
								VECCOPY(bp->vec, bezt->vec[c]);
								bp->vec[3]= 1.0;
								if(c==0) bp->f1= bezt->f1;
								else if(c==1) bp->f1= bezt->f2;
								else bp->f1= bezt->f3;
								bp++;
							}
						}
						bezt++;
					}
					freeN(nu->bezt); 
					nu->bezt= 0;
					nu->pntsu= nr;
					nu->pntsv= 1;
					nu->orderu= 4;
					nu->orderv= 1;
					nu->type &= ~7;
					nu->type+= type;
					if(nu->flagu & 1) c= nu->orderu-1; 
					else c= 0;
					if(type== 4) {
						nu->flagu &= 1;
						nu->flagu += 4;
						makeknots(nu, 1, nu->flagu>>1);
					}
				}
			}
			else if((nu->type & 7)==4 && ELEM(G.ebase->soort, 11, -2)) {	/* Nurb */
				if(type==0) {			/* naar Poly */
					nu->type &= ~7;
					freeN(nu->knotsu);
					nu->knotsu= 0;
					if(nu->knotsv) freeN(nu->knotsv);
					nu->knotsv= 0;
				}
				else if(type==1) {		/* naar Bezier */
					nr= nu->pntsu/3;
					bezt= callocstructN(struct BezTriple, nr, "setsplinetype2");
					nu->bezt= bezt;
					a= nr;
					bp= nu->bp;
					while(a--) {
						VECCOPY(bezt->vec[0], bp->vec);
						bezt->f1= bp->f1;
						bp++;
						VECCOPY(bezt->vec[1], bp->vec);
						bezt->f2= bp->f1;
						bp++;
						VECCOPY(bezt->vec[2], bp->vec);
						bezt->f3= bp->f1;
						bp++;
						bezt++;
					}
					freeN(nu->bp);
					nu->bp= 0;
					freeN(nu->knotsu);
					nu->knotsu= 0;
					nu->pntsu= nr;
					nu->type &= ~7;
					nu->type+= 1;
				}
			}
		}
		nu= nu->next;
	}
}

void addsegmentNurb()
{
	/* voegt twee curves samen */
	struct Nurb *nu, *nu1=0, *nu2=0;
	struct BezTriple *bezt, *bezt1, *bezt2;
	struct BPoint *bp;
	float *fp, offset;
	long a;

	/* vind de beide nurben en punten, nu1 wordt achter nu2 gezet */
	nu= editNurb.first;
	while(nu) {
		if((nu->flagu & 1)==0) {    /* niet cyclic */
			if( (nu->type & 7)==1 ) {
				bezt= nu->bezt;
				if(nu1==0) {
					if( BEZSELECTED(bezt) ) nu1= nu;
					else {
						bezt= bezt+(nu->pntsu-1);
						if( BEZSELECTED(bezt) ) {
							nu1= nu;
							switchdirectionNurb(nu);
						}
					}
				}
				else if(nu2==0) {
					if( BEZSELECTED(bezt) ) {
						nu2= nu;
						switchdirectionNurb(nu);
					}
					else {
						bezt= bezt+(nu->pntsu-1);
						if( BEZSELECTED(bezt) ) {
							nu2= nu;
						}
					}
				}
				else break;
			}
			else if(nu->pntsv==1) {
				bp= nu->bp;
				if(nu1==0) {
					if( bp->f1 & 1) nu1= nu;
					else {
						bp= bp+(nu->pntsu-1);
						if( bp->f1 & 1 ) {
							nu1= nu;
							switchdirectionNurb(nu);
						}
					}
				}
				else if(nu2==0) {
					if( bp->f1 & 1) {
						nu2= nu;
						switchdirectionNurb(nu);
					}
					else {
						bp= bp+(nu->pntsu-1);
						if( bp->f1 & 1 ) {
							nu2= nu;
						}
					}
				}
				else break;
			}
		}
		nu= nu->next;
	}

	if((nu1 && nu2) && (nu1!=nu2)) {
		if( nu1->type==nu2->type) {
			if((nu1->type & 7)==1) {
				bezt= mallocstructN(struct BezTriple, nu1->pntsu+nu2->pntsu, "addsegmentN");
				memcpy(bezt, nu2->bezt, nu2->pntsu*sizeof(struct BezTriple));
				memcpy(bezt+nu2->pntsu, nu1->bezt, nu1->pntsu*sizeof(struct BezTriple));
				freeN(nu1->bezt);
				nu1->bezt= bezt;
				nu1->pntsu+= nu2->pntsu;
				remlink(&editNurb, nu2);
				freeNurb(nu2);
				calchandlesNurb(nu1);
			}
			else {
				bp= mallocstructN(struct BPoint, nu1->pntsu+nu2->pntsu, "addsegmentN2");
				memcpy(bp, nu2->bp, nu2->pntsu*sizeof(struct BPoint) );
				memcpy(bp+nu2->pntsu, nu1->bp, nu1->pntsu*sizeof(struct BPoint));
				freeN(nu1->bp);
				nu1->bp= bp;

				a= nu1->pntsu+nu1->orderu;

				nu1->pntsu+= nu2->pntsu;
				remlink(&editNurb, nu2);

				/* en de knots aaneenrijgen */
				if((nu1->type & 7)==4) {
					fp= mallocN(4*KNOTSU(nu1), "addsegment3");
					memcpy(fp, nu1->knotsu, 4*a);
					freeN(nu1->knotsu);
					nu1->knotsu= fp;
	
					offset= nu1->knotsu[a-1] +1.0;
					fp= nu1->knotsu+a;
					for(a=0; a<nu2->pntsu; a++, fp++) {
						*fp= offset+nu2->knotsu[a+1];
					}
				}
				freeNurb(nu2);
			}
		}
		makeDispList(G.ebase);
		projektie();
	}
	else error("Can't make segment");
}

void muis_Nurb()
{
	struct Nurb *nu;
	struct BezTriple *bezt=0;
	struct BPoint *bp=0;
	ulong *rect;
	long a;
	short mval[2], xs, ys, hand;

	hand= findnearestNurbvert(1, &nu, &bezt, &bp);

	if(bezt || bp) {
		if((G.qual & 3)==0) {
		
			setflagsNurb(0);

			if(bezt) {

				if(hand==1) {
					bezt->f1|= 1;
					bezt->f2|= 1;
					bezt->f3|= 1;
				}
				else if(hand==0) bezt->f1|= 1;
				else bezt->f3|= 1;
			}
			else {
				lastselbp= bp;
				bp->f1 |= 1;
			}

			projektie();
		}
		else {
			if(bezt) {
				if(hand==1) {
					if(bezt->f2 & 1) {
						bezt->f1 &= ~1;
						bezt->f2 &= ~1;
						bezt->f3 &= ~1; 
						a= 0;
					}
					else {
						bezt->f1 |= 1;
						bezt->f2 |= 1;
						bezt->f3 |= 1;	
						a= 1;
					}
				}
				else if(hand==0) {
					if(bezt->f1 & 1) {
						bezt->f1 &= ~1; 
						a= 0; 
					}
					else {
						bezt->f1 |= 1; 
						a= 1; 
					}
				}
				else {
					if(bezt->f3 & 1) { 
						bezt->f3 &= ~1; 
						a= 0; 
					}
					else { 
						bezt->f3 |= 1; 
						a= 1; 
					}
				}
			}
			else {
				if(bp->f1 & 1) bp->f1 &= ~1;
				else {
					bp->f1 |= 1;
					lastselbp= bp;
				}
			}

			projektie();

		}

		countall();
		tekenoverdraw(0);
	}

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
	if(nu!=lastnu) {
		lastnu= nu;
		if(G.mainb==9) tekeneditbuts(1);
	}
	
}

void extrudeNurb()
{
	struct Nurb *nu;
	struct BPoint *bp, *new;
	long ok= 0, a;

	if(G.ebase && G.ebase->soort==5);
	else return;
	if(okee("Extrude")==0) return;

	ok= extrudeflagNurb(1); /* '1'= flag */

	if(ok) {
		makeDispList(G.ebase);
		vertgrabber();
	}
	else error("Can't extrude");
}

void spinNurb(dvec,mode)
float *dvec;
short mode;	/* 0 is extrude, 1 is duplicate */
{
	struct Nurb *nu;
	struct BPoint *bp;
	float si,phi,n[3],q[4],cmat[3][3],tmat[4][4],imat[3][3];
	float cent[3],bmat[3][3], rotmat[3][3], scalemat1[3][3], scalemat2[3][3];
	float persmat[3][3], persinv[3][3];
	short a,ok;

	if(G.ebase==0 || G.ebase->soort!=5) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;
	winset(G.winar[0]);

	Mat3CpyMat4(persmat,G.persmat);
	Mat3CpyMat4(persinv,G.persinv);

	/* imat en centrum en afmeting */
	parentbase(G.ebase,tmat,bmat);
	Mat3CpyMat4(bmat,tmat);

	phi= ((struct ObData *)G.ebase->d)->cu->ws;
	Mat3MulFloat(bmat,phi);
	Mat3Inv(imat,bmat);

	VECCOPY(cent,view0.muis);
	cent[0]-= tmat[3][0];
	cent[1]-= tmat[3][1];
	cent[2]-= tmat[3][2];
	Mat3MulVecfl(imat,cent);

	if(dvec) {
		n[0]=n[1]= 0.0;
		n[2]= 1.0;
	} else {
		n[0]= -G.persinv[2][0];
		n[1]= -G.persinv[2][1];
		n[2]= -G.persinv[2][2];
		Normalise(n);
	}
	phi= PI/8.0;
	q[0]= fcos(phi);
	si= fsin(phi);
	q[1]= n[0]*si;
	q[2]= n[1]*si;
	q[3]= n[2]*si;
	QuatToMat3(q,cmat);
	Mat3MulMat3(tmat,cmat,bmat);
	Mat3MulMat3(rotmat,imat,tmat);

	Mat3One(scalemat1);
	scalemat1[0][0]= fsqrt(2.0);
	scalemat1[1][1]= fsqrt(2.0);

	Mat3MulMat3(tmat,persmat,bmat);
	Mat3MulMat3(cmat,scalemat1,tmat);
	Mat3MulMat3(tmat,persinv,cmat);
	Mat3MulMat3(scalemat1,imat,tmat);

	Mat3One(scalemat2);
	scalemat2[0][0]/= fsqrt(2.0);
	scalemat2[1][1]/= fsqrt(2.0);

	Mat3MulMat3(tmat,persmat,bmat);
	Mat3MulMat3(cmat,scalemat2,tmat);
	Mat3MulMat3(tmat,persinv,cmat);
	Mat3MulMat3(scalemat2,imat,tmat);

	ok= 1;

	for(a=0;a<7;a++) {
		if(mode==0) ok= extrudeflagNurb(1);
		else adduplicateflagNurb(1);
		if(ok==0) {
			error("Can't spin");
			break;
		}
		rotateflagNurb(1,cent,rotmat);

		if(mode==0) {
			if( (a & 1)==0 ) {
				rotateflagNurb(1,cent,scalemat1);
				weightflagNurb(1, 0.25*fsqrt(2.0), 1);
			}
			else {
				rotateflagNurb(1,cent,scalemat2);
				weightflagNurb(1, 4.0/fsqrt(2.0), 1);
			}
		}
		if(dvec) {
			Mat3MulVecfl(bmat,dvec);
			translateflagNurb(1,dvec);
		}
	}

	if(ok) {
		nu= editNurb.first;
		while(nu) {
			if(isNurbsel(nu)) {
				nu->orderv= 4;
				nu->flagv |= 1;
				makeknots(nu, 2, nu->flagv>>1);
			}
			nu= nu->next;
		}
	}
}


void addvert_Nurb()
{
	struct Nurb *nu;
	struct BezTriple *bezt, *newbezt;
	struct BPoint *bp, *newbp;
	float *fp, fac,mat[3][3],imat[3][3],tmat[4][4], temp[3];
	long len;
	short val;

	if(G.ebase==0) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	parentbase(G.ebase,tmat,mat);
	Mat3CpyMat4(mat,tmat);
	if ELEM(G.ebase->soort, 5, 11) {
		fac= ((struct ObData *)G.ebase->d)->cu->ws;
		Mat3MulFloat(mat,fac);
	}
	Mat3Inv(imat,mat);

	findselectedNurbvert(&nu, &bezt, &bp);
	if(bezt==0 && bp==0) return;

	if((nu->type & 7)==1) {
		/* welk bezpoint? */
		if(bezt== nu->bezt) {   /* eerste */
			bezt->f1= bezt->f2= bezt->f3= 0;
			newbezt= callocstructN(struct BezTriple, nu->pntsu+1, "addvert_Nurb");
			memcpy(newbezt+1, bezt, nu->pntsu*sizeof(struct BezTriple));
			*newbezt= *bezt;
			newbezt->f1= newbezt->f2= newbezt->f3= 1;
			if(bezt->h1 & 1) newbezt->h1= newbezt->h2= 1;
			else newbezt->h1= newbezt->h2= 2;
			VECCOPY(temp, bezt->vec[1]);
			freeN(nu->bezt);
			nu->bezt= newbezt;
		}
		else if(bezt== (nu->bezt+nu->pntsu-1)) {  /* laatste */
			bezt->f1= bezt->f2= bezt->f3= 0;
			newbezt= callocstructN(struct BezTriple, nu->pntsu+1, "addvert_Nurb");
			memcpy(newbezt, nu->bezt, nu->pntsu*sizeof(struct BezTriple));
			*(newbezt+nu->pntsu)= *bezt;
			VECCOPY(temp, bezt->vec[1]);
			freeN(nu->bezt);
			nu->bezt= newbezt;
			newbezt+= nu->pntsu;
			newbezt->f1= newbezt->f2= newbezt->f3= 1;
			if(bezt->h2 & 1) newbezt->h1= newbezt->h2= 1;
			else newbezt->h1= newbezt->h2= 2;
		}
		else bezt= 0;

		if(bezt) {
			nu->pntsu++;
			newbezt->s[1][0]= view0.mx;
			newbezt->s[1][1]= view0.my;

			VECCOPY(newbezt->vec[1], view0.muis);
			VecSubf(newbezt->vec[1],newbezt->vec[1],tmat[3]);
			Mat3MulVecfl(imat,newbezt->vec[1]);
			VecSubf(temp, newbezt->vec[1],temp);
			VecAddf(newbezt->vec[0], bezt->vec[0],temp);
			VecAddf(newbezt->vec[2], bezt->vec[2],temp);
			calchandlesNurb(nu);
		}
	}
	else if(nu->pntsv==1) {
		/* welk b-point? */
		if(bp== nu->bp) {   /* eerste */
			bp->f1= 0;
			newbp= callocstructN(struct BPoint, nu->pntsu+1, "addvert_Nurb3");
			memcpy(newbp+1, bp, nu->pntsu*sizeof(struct BPoint));
			*newbp= *bp;
			newbp->f1= 1;
			freeN(nu->bp);
			nu->bp= newbp;
		}
		else if(bp== (nu->bp+nu->pntsu-1)) {  /* laatste */
			bp->f1= 0;
			newbp= callocstructN(struct BPoint, nu->pntsu+1, "addvert_Nurb4");
			memcpy(newbp, nu->bp, nu->pntsu*sizeof(struct BPoint));
			*(newbp+nu->pntsu)= *bp;
			freeN(nu->bp);
			nu->bp= newbp;
			newbp+= nu->pntsu;
			newbp->f1= 1;
		}
		else bp= 0;

		if(bp) {
			nu->pntsu++;
			newbp->s[0]= view0.mx;
			newbp->s[1]= view0.my;

			makeknots(nu, 1, nu->flagu>>1);

			VECCOPY(newbp->vec, view0.muis);
			VecSubf(newbp->vec,newbp->vec,tmat[3]);
			Mat3MulVecfl(imat,newbp->vec);
			newbp->vec[3]= 1.0;
		}
	}

	test2DNurb(nu);
	makeDispList(G.ebase);
	countall();
	projektie();

	while(getbutton(RIGHTMOUSE)) gsync();

	while(qtest()) {
		if(traces_qread(&val)==INPUTCHANGE) {
			if(val) G.winakt=val;
		}
	}
}


void makecyclicNurb()
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float *fp, temp;
	long a, b, c, cyclmode=0;

	nu= editNurb.first;
	while(nu) {
		if( (nu->type & 7)==0 ) {
			a= nu->pntsu;
			bp= nu->bp;
			while(a--) {
				if( bp->f1 & 1 ) {
					if(nu->flagu & 1) nu->flagu--;
					else nu->flagu++;
					break;
				}
				bp++;
			}
		}
		else if( (nu->type & 7)==1 ) {
			a= nu->pntsu;
			bezt= nu->bezt;
			while(a--) {
				if( BEZSELECTED(bezt) ) {
					if(nu->flagu & 1) nu->flagu--;
					else nu->flagu++;
					break;
				}
				bezt++;
			}
			calchandlesNurb(nu);
		}
		else if(nu->pntsv==1 && (nu->type & 7)==4) {
			a= nu->pntsu;
			bp= nu->bp;
			while(a--) {
				if( bp->f1 & 1 ) {
					if(nu->flagu & 1) nu->flagu--;
					else {
						nu->flagu++;
						fp= mallocN(4*KNOTSU(nu), "makecyclicN");
						b= (nu->orderu+nu->pntsu);
						memcpy(fp, nu->knotsu, 4*b);
						freeN(nu->knotsu);
						nu->knotsu= fp;
						makecyclicknots(nu->knotsu, nu->pntsu, nu->orderu);
					}
					break;
				}
				bp++;
			}
		}
		else if(nu->type==4) {
			if(cyclmode==0) {
				cyclmode= pupmenu("Toggle %t|cyclic U%x1|cyclic V%x2");
				if(cyclmode== -1) return;
			}
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {

				if( bp->f1 & 1) {
					if(cyclmode==1) {
						if(nu->flagu & 1) nu->flagu--;
						else {
							nu->flagu++;
							fp= mallocN(4*KNOTSU(nu), "makecyclicN");
							b= (nu->orderu+nu->pntsu);
							memcpy(fp, nu->knotsu, 4*b);
							freeN(nu->knotsu);
							nu->knotsu= fp;
							makecyclicknots(nu->knotsu, nu->pntsu, nu->orderu);
						}
					}
					if(cyclmode==2) {
						if(nu->flagv & 1) nu->flagv--;
						else {
							nu->flagv++;
							fp= mallocN(4*KNOTSV(nu), "makecyclicN");
							b= (nu->orderv+nu->pntsv);
							memcpy(fp, nu->knotsv, 4*b);
							freeN(nu->knotsv);
							nu->knotsv= fp;
							makecyclicknots(nu->knotsv, nu->pntsv, nu->orderv);
						}
					}
					break;
				}
				bp++;
			}

		}
		nu= nu->next;
	}
	makeDispList(G.ebase);
}

void selectconnectedNurb()
{
	struct Nurb *nu;
	struct BezTriple *bezt, *bezt1;
	struct BPoint *bp;
	long a;

	findnearestNurbvert(1, &nu, &bezt, &bp);
	if(bezt) {
		a= nu->pntsu;
		bezt= nu->bezt;
		while(a--) {
			if(bezt->hide==0) {
				if(G.qual & 3) {
					bezt->f1 &= ~1;
					bezt->f2 &= ~1;
					bezt->f3 &= ~1;
				}
				else {
					bezt->f1 |= 1;
					bezt->f2 |= 1;
					bezt->f3 |= 1;
				}
			}
			bezt++;
		}
	}
	else if(bp) {
		a= nu->pntsu*nu->pntsv;
		bp= nu->bp;
		while(a--) {
			if(bp->hide==0) {
				if(G.qual & 3) {
					bp->f1 &= ~1;
				}
				else {
					bp->f1 |= 1;
				}
			}
			bp++;
		}
	}

	projektie();
}

void selectrowNurb()
{
	static struct BPoint *last=0;
	static long direction=0;
	struct Nurb *nu;
	struct BPoint *bp;
	long u, v, a, b, ok=0;

	if(editNurb.first==0) return;
	if(G.ebase==0 || G.ebase->soort!=5) return;
	if(lastselbp==0) return;

	/* zoek de juiste nurb en toggle met u of v */
	nu= editNurb.first;
	while(nu) {
		bp= nu->bp;
		for(v=0; v<nu->pntsv; v++) {
			for(u=0; u<nu->pntsu; u++, bp++) {
				if(bp==lastselbp) {
					if(bp->f1 & 1) {
						ok= 1;
						break;
					}
				}
			}
			if(ok) break;
		}
		if(ok) {
			if(last==lastselbp) {
				direction= 1-direction;
				setflagsNurb(0);
			}
			last= lastselbp;

			bp= nu->bp;
			for(a=0; a<nu->pntsv; a++) {
				for(b=0; b<nu->pntsu; b++, bp++) {
					if(direction) {
						if(a==v) if(bp->hide==0) bp->f1 |= 1;
					}
					else {
						if(b==u) if(bp->hide==0) bp->f1 |= 1;
					}
				}
			}
			projektie();
			return;
		}
		nu= nu->next;
	}
}

void adduplicateNurb()
{

	if( (view0.lay & G.ebase->lay)==0 ) return;

	winset(G.winar[0]);
	adduplicateflagNurb(1);

	countall();
	vertgrabber();
}

void delNurb()
{
	struct Nurb *nu, *next, *nu1;
	struct BezTriple *bezt, *bezt1, *bezt2;
	struct BPoint *bp, *bp1, *bp2;
	float *fp;
	long a;
	short event, cut;

	if(G.ebase==0 ) return;
	if( (view0.lay & G.ebase->lay)==0 ) return;

	if(G.ebase->soort==5) event= pupmenu("ERASE %t|Selected%x0|All%x2");
	else if ELEM(G.ebase->soort, 11, -2) event= pupmenu("ERASE %t|Selected%x0|Segment%x1|All%x2");
	else return;

	if(event== -1) return;

	if(G.ebase->soort==5) {
		if(event==0) deleteflagNurb(1);
		else freeNurblist(&editNurb);

		countall();
		makeDispList(G.ebase);
		projektie();
		return;
	}

	if(event==0) {
		/* eerste doorloop, kunnen hele stukken weg? */
		nu= editNurb.first;
		while(nu) {
			next= nu->next;
			if( (nu->type & 7)==1 ) {
				bezt= nu->bezt;
				a= nu->pntsu;
				if(a) {
					while(a) {
						if( BEZSELECTED(bezt) );
						else break;
						a--;
						bezt++;
					}
					if(a==0) {
						remlink(&editNurb, nu);
						freeNurb(nu);
					}
				}
			}
			else {
				bp= nu->bp;
				a= nu->pntsu*nu->pntsv;
				if(a) {
					while(a) {
						if(bp->f1 & 1 );
						else break;
						a--;
						bp++;
					}
					if(a==0) {
						remlink(&editNurb, nu);
						freeNurb(nu);
					}
				}
			}
			nu= next;
		}
		/* tweede doorloop, kleine stukken weg: alleen curves */
		nu= editNurb.first;
		while(nu) {
			next= nu->next;
			event= 0;
			if( (nu->type & 7)==1 ) {
				bezt= nu->bezt;
				for(a=0;a<nu->pntsu;a++) {
					if( BEZSELECTED(bezt) ) {
						memcpy(bezt, bezt+1, (nu->pntsu-a-1)*sizeof(struct BezTriple));
						nu->pntsu--;
						a--;
						event= 1;
					}
					else bezt++;
				}
				if(event) {
					bezt1= mallocstructN(struct BezTriple, nu->pntsu, "delNurb");
					memcpy(bezt1, nu->bezt, (nu->pntsu)*sizeof(struct BezTriple) );
					freeN(nu->bezt);
					nu->bezt= bezt1;
					calchandlesNurb(nu);
				}
			}
			else if(nu->pntsv==1) {
				bp= nu->bp;
				fp= nu->knotsu;
				if(fp) {
					fp+= (nu->orderu-1);
					if(nu->flagu & 1) fp+= (nu->orderu-1);
				}
				for(a=0;a<nu->pntsu;a++) {
					if( bp->f1 & 1 ) {
						memcpy(bp, bp+1, (nu->pntsu-a-1)*sizeof(struct BPoint));
						if(fp) memcpy(fp, fp+1, (nu->pntsu-a-1)*4);
						nu->pntsu--;
						a--;
						event= 1;
					}
					else {
						bp++;
						if(fp) fp++;
					}
				}
				if(event) {
					bp1= mallocstructN(struct BPoint, nu->pntsu, "delNurb2");
					memcpy(bp1, nu->bp, (nu->pntsu)*sizeof(struct BPoint) );
					freeN(nu->bp);
					nu->bp= bp1;
				}
			}
			nu= next;
		}
	}
	else if(event==1) {	/* erase segment */
		/* vind de twee geselecteerde punten */
		bezt1= bezt2= 0;
		bp1= bp2= 0;
		nu= editNurb.first;
		nu1= 0;
		while(nu) {
			next= nu->next;
			if( (nu->type & 7)==1 ) {
				bezt= nu->bezt;
				for(a=0;a<nu->pntsu;a++) {
					if( BEZSELECTED(bezt) ) {
						bezt1= bezt;
						bezt2= bezt+1;
						if( (bezt2->f1 & 1) || (bezt2->f2 & 1) || (bezt2->f3 & 1) ) ;
						else {	/* misschien niet cyclic maken */
							if(a==0 && (nu->flagu & 1) ) {
								bezt2= bezt+(nu->pntsu-1);
								if( (bezt2->f1 & 1) || (bezt2->f2 & 1) || (bezt2->f3 & 1) ) {
									nu->flagu--;
									makeDispList(G.ebase);
									projektie();
								}
							}
							return;
						}
						cut= a;
						nu1= nu;
						break;
					}
					bezt++;
				}
			}
			else if(nu->pntsv==1) {
				bp= nu->bp;
				for(a=0; a<nu->pntsu; a++) {
					if( bp->f1 & 1 ) {
						bp1= bp;
						bp2= bp+1;
						if( bp2->f1 & 1 ) ;
						else {	/* misschien niet cyclic maken */
							if(a==0 && (nu->flagu & 1) ) {
								bp2= bp+(nu->pntsu-1);
								if( bp2->f1 & 1 ) {
									nu->flagu--;
									makeDispList(G.ebase);
									projektie();
								}
							}
							return;
						}
						cut= a;
						nu1= nu;
						break;
					}
					bp++;
				}
			}
			if(nu1) break;

			nu= nu->next;
		}
		if(nu1) {
			if(bezt1) {
				if(nu1->pntsu==2) {	/* helemaal weg */
					remlink(&editNurb, nu);
					freeNurb(nu);
				}
				else if(nu1->flagu & 1) {	/* cyclic */
					bezt= mallocstructN(struct BezTriple, cut+1, "delNurb1");
					memcpy(bezt, nu1->bezt,(cut+1)*sizeof(struct BezTriple));
					a= nu1->pntsu-cut-1;
					memcpy(nu1->bezt, bezt2, a*sizeof(struct BezTriple));
					memcpy(nu1->bezt+a, bezt, (cut+1)*sizeof(struct BezTriple));
					nu1->flagu--;
					freeN(bezt);
					calchandlesNurb(nu);
				}
				else {			/* nieuwe curve erbij */
					nu= mallocstructN(struct Nurb, 1, "delNurb2");
					memcpy(nu, nu1, sizeof(struct Nurb));
					addtail(&editNurb, nu);
					nu->bezt= mallocstructN(struct BezTriple, cut+1, "delNurb3");
					memcpy(nu->bezt, nu1->bezt,(cut+1)*sizeof(struct BezTriple));
					a= nu1->pntsu-cut-1;
					bezt= mallocstructN(struct BezTriple, a, "delNurb4");
					memcpy(bezt, nu1->bezt+cut+1,a*sizeof(struct BezTriple));
					freeN(nu1->bezt);
					nu1->bezt= bezt;
					nu1->pntsu= a;
					nu->pntsu= cut+1;

					calchandlesNurb(nu);
					calchandlesNurb(nu1);
				}
			}
			else if(bp1) {
				if(nu1->pntsu==2) {	/* helemaal weg */
					remlink(&editNurb, nu);
					freeNurb(nu);
				}
				else if(nu1->flagu & 1) {	/* cyclic */
					bp= mallocstructN(struct BPoint, cut+1, "delNurb5");
					memcpy(bp, nu1->bp,(cut+1)*sizeof(struct BPoint));
					a= nu1->pntsu-cut-1;
					memcpy(nu1->bp, bp2, a*sizeof(struct BPoint));
					memcpy(nu1->bp+a, bp, (cut+1)*sizeof(struct BPoint));
					nu1->flagu--;
					freeN(bp);
				}
				else {			/* nieuwe curve erbij */
					nu= mallocstructN(struct Nurb, 1, "delNurb6");
					memcpy(nu, nu1, sizeof(struct Nurb));
					addtail(&editNurb, nu);
					nu->bp= mallocstructN(struct BPoint, cut+1, "delNurb7");
					memcpy(nu->bp, nu1->bp,(cut+1)*sizeof(struct BPoint));
					a= nu1->pntsu-cut-1;
					bp= mallocstructN(struct BPoint, a, "delNurb8");
					memcpy(bp, nu1->bp+cut+1,a*sizeof(struct BPoint));
					freeN(nu1->bp);
					nu1->bp= bp;
					nu1->pntsu= a;
					nu->pntsu= cut+1;
				}
			}
		}
	}
	else if(event==2) {
		freeNurblist(&editNurb);
	}

	countall();
	makeDispList(G.ebase);
	projektie();
}

struct Nurb *addNurbprim(type, soort)
short type, soort;
/* type: &8= 2D;  0=poly,1 bez, 4 nurb
 * soort:   0: 2/4 punts curve
 *	    1: 8 punts cirkel
 *	    2: 4x4 patch Nurb
 *	    3: tube 4:sphere 5:donut
 *		6: 5 punts,  5e order rechte lijn (pad) alleen nurbspline!
 */
{
	static long xzproj= 0;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float phi, theta, cent[3],vec[3],imat[3][3],mat[3][3],tmat[4][4];
	float fac, si,co,q[4],cmat[3][3], pmat[4][4];
	long a, b, cycl;

	/* imat en centrum en afmeting */
	if(G.ebase) {
		parentbase(G.ebase,tmat,mat);
		Mat3CpyMat4(mat,tmat);
		if(G.ebase->soort==5 || G.ebase->soort==11) {
			phi= ((struct ObData *)G.ebase->d)->cu->ws;
			Mat3MulFloat(mat,phi);
		}

		VECCOPY(cent,view0.muis);
		cent[0]-= tmat[3][0];
		cent[1]-= tmat[3][1];
		cent[2]-= tmat[3][2];

		winset(G.winar[0]);
		getmatrix(pmat);
		Mat3CpyMat4(imat,pmat);
		Mat3MulVecfl(imat, cent);
		Mat3MulMat3(cmat, imat, mat);
		Mat3Inv(imat,cmat);

		setflagsNurb(0);
	}
	else {
		Mat3One(imat);
		cent[0]= cent[1]= cent[2]= 0.0;
	}

	if ELEM5(soort, 0, 1, 2, 4, 6) {
		nu= callocstructN(struct Nurb, 1, "addNurbprim");
		nu->type= type;
		nu->resolu= 12;
		nu->resolv= 12;
		if(G.ebase && (G.mainb==5 || G.mainb==9)) nu->col= G.colact-1;
		if(G.ebase && G.ebase->soort== -2) nu->resolu= 64;
	}

	switch(soort) {
	case 0:	/* curve */
		if((type & 7)==1) { /* bezier */
			nu->pntsu= 2;
			nu->bezt= callocstructN(struct BezTriple, 2, "addNurbprim1");
			bezt= nu->bezt;
			bezt->h1= bezt->h2= 3;
			bezt->f1= bezt->f2= bezt->f3= 1;

			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->vec[1][0]+= -view0.grid;
			bezt->vec[0][0]+= -1.5*view0.grid;
			bezt->vec[0][1]+= -0.5*view0.grid;
			bezt->vec[2][0]+= -0.5*view0.grid;
			bezt->vec[2][1]+=  0.5*view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			bezt++;
			bezt->h1= bezt->h2= 1;
			bezt->f1= bezt->f2= bezt->f3= 1;

			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->vec[1][0]+= view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			calchandlesNurb(nu);
		}
		else {
			nu->pntsu= 4;
			nu->pntsv= 1;
			nu->orderu= 4;
			nu->bp= callocstructN(struct BPoint, 4, "addNurbprim3");

			bp= nu->bp;
			for(a=0;a<4;a++, bp++) {
				VECCOPY(bp->vec, cent);
				bp->vec[3]= 1.0;
				bp->f1= 1;
			}

			bp= nu->bp;
			bp->vec[0]+= -1.5*view0.grid; 
			bp++;
			bp->vec[0]+= -view0.grid;
			bp->vec[1]+= view0.grid; 
			bp++;
			bp->vec[0]+= view0.grid;
			bp->vec[1]+= view0.grid; 
			bp++;
			bp->vec[0]+= 1.5*view0.grid;

			bp= nu->bp;
			for(a=0;a<4;a++, bp++) Mat3MulVecfl(imat,bp->vec);

			if((type & 7)==4) {
				nu->knotsu= 0;	/* makeknots alloceert */
				makeknots(nu, 1, nu->flagu>>1);
			}

		}
		break;
	case 6:	/* 5 punts pad */
		nu->pntsu= 5;
		nu->pntsv= 1;
		nu->orderu= 5;
		nu->flagu= 2;	/* endpoint */
		nu->bp= callocstructN(struct BPoint, 5, "addNurbprim3");

		bp= nu->bp;
		for(a=0;a<5;a++, bp++) {
			VECCOPY(bp->vec, cent);
			bp->vec[3]= 1.0;
			bp->f1= 1;
		}

		bp= nu->bp;
		bp->vec[0]+= -2.0*view0.grid; 
		bp++;
		bp->vec[0]+= -view0.grid;
		bp++; bp++;
		bp->vec[0]+= view0.grid;
		bp++;
		bp->vec[0]+= 2.0*view0.grid;

		bp= nu->bp;
		for(a=0;a<5;a++, bp++) Mat3MulVecfl(imat,bp->vec);

		if((type & 7)==4) {
			nu->knotsu= 0;	/* makeknots alloceert */
			makeknots(nu, 1, nu->flagu>>1);
		}

		break;
	case 1:	/* cirkel */
		if((type & 7)==1) { /* bezier */
			nu->pntsu= 4;
			nu->bezt= callocstructN(struct BezTriple, 4, "addNurbprim1");
			nu->flagu= 1;
			bezt= nu->bezt;

			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->h1= bezt->h2= 1;
			bezt->f1= bezt->f2= bezt->f3= 1;
			bezt->vec[1][0]+= -view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			bezt++;
			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->h1= bezt->h2= 1;
			bezt->f1= bezt->f2= bezt->f3= 1;
			bezt->vec[1][1]+= view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			bezt++;
			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->h1= bezt->h2= 1;
			bezt->f1= bezt->f2= bezt->f3= 1;
			bezt->vec[1][0]+= view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			bezt++;
			for(a=0;a<3;a++) {
				VECCOPY(bezt->vec[a], cent);
			}
			bezt->h1= bezt->h2= 1;
			bezt->f1= bezt->f2= bezt->f3= 1;
			bezt->vec[1][1]+= -view0.grid;
			for(a=0;a<3;a++) Mat3MulVecfl(imat,bezt->vec[a]);

			calchandlesNurb(nu);
		}
		else if( (type & 7)==4 ) {  /* nurb */
			nu->pntsu= 8;
			nu->pntsv= 1;
			nu->orderu= 4;
			nu->bp= callocstructN(struct BPoint, 8, "addNurbprim6");
			nu->flagu= 1;
			bp= nu->bp;

			for(a=0; a<8; a++) {
				bp->f1= 1;
				VECCOPY(bp->vec, cent);

				if(xzproj==0) {
					bp->vec[0]+= nurbcircle[a][0]*view0.grid;
					bp->vec[1]+= nurbcircle[a][1]*view0.grid;
				}
				else {
					bp->vec[0]+= 0.25*nurbcircle[a][0]*view0.grid-.75*view0.grid;
					bp->vec[2]+= 0.25*nurbcircle[a][1]*view0.grid;
				}
				if(a & 1) bp->vec[3]= 0.25*fsqrt(2.0);
				else bp->vec[3]= 1.0;
				Mat3MulVecfl(imat,bp->vec);
				bp++;
			}

			makeknots(nu, 1, nu->flagu>>1);
		}
		break;
	case 2:	/* 4x4 patch */
		if( (type & 7)==4 ) {  /* nurb */
			nu->pntsu= 4;
			nu->pntsv= 4;
			nu->orderu= 4;
			nu->orderv= 4;
			nu->bp= callocstructN(struct BPoint, 4*4, "addNurbprim6");
			nu->flagu= 0;
			nu->flagv= 0;
			bp= nu->bp;

			for(a=0; a<4; a++) {
				for(b=0; b<4; b++) {
					VECCOPY(bp->vec, cent);
					bp->f1= 1;
					fac= (float)a -1.5;
					bp->vec[0]+= fac*view0.grid;
					fac= (float)b -1.5;
					bp->vec[1]+= fac*view0.grid;
					if(a==1 || a==2) if(b==1 || b==2) {
						bp->vec[2]+= view0.grid;
					}
					Mat3MulVecfl(imat,bp->vec);
					bp->vec[3]= 1.0;
					bp++;
				}
			}

			makeknots(nu, 1, nu->flagu>>1);
			makeknots(nu, 2, nu->flagv>>1);
		}
		break;
	case 3:	/* tube */
		if( (type & 7)==4 ) {
			nu= addNurbprim(4, 1);  /* cirkel */
			nu->resolu= 32;
			addtail(&editNurb, nu); /* tijdelijk voor extrude en translate */
			vec[0]=vec[1]= 0.0;
			vec[2]= view0.grid;
			Mat3MulVecfl(imat,vec);
			translateflagNurb(1, vec);
			extrudeflagNurb(1);
			vec[0]= -2*vec[0]; 
			vec[1]= -2*vec[1]; 
			vec[2]= -2*vec[2];
			translateflagNurb(1, vec);

			remlink(&editNurb, nu);

			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a-- >0) {
				bp->f1 |= 1;
				bp++;
			}
		}
		break;
	case 4:	/* sphere */
		if( (type & 7)==4 ) {
			nu->pntsu= 5;
			nu->pntsv= 1;
			nu->orderu= 3;
			nu->resolu= 24;
			nu->resolv= 32;
			nu->bp= callocstructN(struct BPoint, 5, "addNurbprim6");
			nu->flagu= 0;
			bp= nu->bp;

			for(a=0; a<5; a++) {
				bp->f1= 1;
				VECCOPY(bp->vec, cent);
				bp->vec[0]+= nurbcircle[a][0]*view0.grid;
				bp->vec[2]+= nurbcircle[a][1]*view0.grid;
				if(a & 1) bp->vec[3]= 0.5*fsqrt(2.0);
				else bp->vec[3]= 1.0;
				Mat3MulVecfl(imat,bp->vec);
				bp++;
			}
			nu->flagu= 4;
			makeknots(nu, 1, nu->flagu>>1);

			addtail(&editNurb, nu); /* tijdelijk voor spin */
			spinNurb(0, 0);

			makeknots(nu, 2, nu->flagv>>1);

			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a-- >0) {
				bp->f1 |= 1;
				bp++;
			}
			remlink(&editNurb, nu);
		}
		break;
	case 5:	/* donut */
		if( (type & 7)==4 ) {
			xzproj= 1;
			nu= addNurbprim(4, 1);  /* cirkel */
			xzproj= 0;
			nu->resolu= 24;
			nu->resolv= 32;
			addtail(&editNurb, nu); /* tijdelijk voor extrude en translate */
			spinNurb(0, 0);

			remlink(&editNurb, nu);

			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a-- >0) {
				bp->f1 |= 1;
				bp++;
			}

		}
		break;
	}

	test2DNurb(nu);
	return nu;
}

displist_to_triface(disp, ob)
struct ListBase *disp;
struct ObData *ob;
{
	struct DispList *dl;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	float fac, min[3], max[3], *data;
	long a, b, len, ofs, vertcount, startvert, totvert=0, totvlak=0;
	long p1, p2, p3, p4;

	min[0]= min[1]= min[2]= 1.0e10;
	max[0]= max[1]= max[2]= -1.0e10;

	/* tellen */
	dl= disp->first;
	while(dl) {
		if(dl->type==DL_SEGM) {
			totvert+= dl->parts*dl->nr;
			totvlak+= dl->parts*(dl->nr-1);
		}
		else if(dl->type==DL_POLY) {
			totvert+= dl->parts*dl->nr;
			totvlak+= dl->parts*dl->nr;
		}
		else if(dl->type==DL_SURF) {
			totvert+= dl->parts*dl->nr;
			totvlak+= 2*(dl->parts-1+((dl->flag & 2)==2))*(dl->nr-1+(dl->flag & 1));
		}
		a= dl->parts*dl->nr;
		data= (float *)(dl+1);
		while(a--) {
			for(b=0; b<3; b++) {
				min[b]= MIN2(min[b], data[b]);
				max[b]= MAX2(max[b], data[b]);
			}
			data+=3;
		}
		dl= dl->next;
	}
	if(totvert==0) {
		error("can't convert");
		return;
	}

	/* VV maken */
	len= sizeof(struct VV)+totvert*sizeof(struct VertOb)+totvlak*sizeof(struct VlakOb);
	vv= callocN(len, "displist_to_triface");
	vv->vert= totvert;
	vv->vlak= totvlak;
	vv->us= 1;

	max[0]= MAX2(fabs(min[0]),fabs(max[0]));
	max[1]= MAX2(fabs(min[1]),fabs(max[1]));
	max[2]= MAX2(fabs(min[2]),fabs(max[2]));
	fac= MAX3(max[0],max[1],max[2]);
	fac= 32767/fac;

	vv->ws= 1.0/fac;
	if(ob->cu) vv->ws*= ob->cu->ws;
	vv->afm[0]= vv->ws*max[0];
	vv->afm[1]= vv->ws*max[1];
	vv->afm[2]= vv->ws*max[2];

	/* verts en vlakken */
	vertcount= 0;
	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+vv->vert);
	dl= disp->first;
	while(dl) {
		if(dl->type==DL_SEGM) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= (float *)(dl+1);
			while(a--) {
				adrve->c[0]= data[0]*fac;
				adrve->c[1]= data[1]*fac;
				adrve->c[2]= data[2]*fac;
				data+=3;
				vertcount++;
				adrve++;
			}

			for(a=0; a<dl->parts; a++) {
				ofs= a*dl->nr;
				for(b=1; b<dl->nr; b++) {
					adrvl->v1= startvert+ofs+b-1;
					adrvl->v2= startvert+ofs+b-1;
					adrvl->v3= startvert+ofs+b;
					adrvl++;
				}
			}

		}
		else if(dl->type==DL_POLY) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= (float *)(dl+1);
			while(a--) {
				adrve->c[0]= data[0]*fac;
				adrve->c[1]= data[1]*fac;
				adrve->c[2]= data[2]*fac;
				data+=3;
				vertcount++;
				adrve++;
			}

			for(a=0; a<dl->parts; a++) {
				ofs= a*dl->nr;
				for(b=0; b<dl->nr; b++) {
					adrvl->v1= startvert+ofs+b;
					adrvl->v2= startvert+ofs+b;
					if(b==dl->nr-1) adrvl->v3= startvert+ofs;
					else adrvl->v3= startvert+ofs+b+1;
					adrvl++;
				}
			}

		}
		else if(dl->type==DL_SURF) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= (float *)(dl+1);
			while(a--) {
				adrve->c[0]= data[0]*fac;
				adrve->c[1]= data[1]*fac;
				adrve->c[2]= data[2]*fac;
				data+=3;
				vertcount++;
				adrve++;
			}

			for(a=0; a<dl->parts; a++) {

				if( (dl->flag & 2)==0 && a==dl->parts-1) break;

				if(dl->flag & 1) {		/* p2 -> p1 -> */
					p1= startvert+ dl->nr*a;	/* p4 -> p3 -> */
					p2= p1+ dl->nr-1;		/* -----> volgende rij */
					p3= p1+ dl->nr;
					p4= p2+ dl->nr;
					b= 0;
				}
				else {
					p2= startvert+ dl->nr*a;
					p1= p2+1;
					p4= p2+ dl->nr;
					p3= p1+ dl->nr;
					b= 1;
				}
				if( (dl->flag & 2) && a==dl->parts-1) {
					p3-= dl->parts*dl->nr;
					p4-= dl->parts*dl->nr;
				}

				for(; b<dl->nr; b++) {
					adrvl->v1= p2;
					adrvl->v2= p3;
					adrvl->v3= p4;
					adrvl++;

					adrvl->v1= p2;
					adrvl->v2= p1;
					adrvl->v3= p3;
					adrvl++;

					p4= p3; 
					p3++;
					p2= p1; 
					p1++;
				}
			}

		}

		dl= dl->next;
	}

	ob->vv= vv;
	if(ob->cu) {
		freeNurblist(&(ob->cu->curve));
		freelistN(&(ob->cu->disp));
		if(ob->cu->bev.first) freelistN(&(ob->cu->bev));
		if(ob->cu->key) freekeys(ob->cu->key);
		freeN(ob->cu);
		ob->cu= 0;
	}
	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+vv->vert);
	normalen(adrve,adrvl,vv->vert,vv->vlak);
	edgesnew(adrvl,vv->vert,vv->vlak);
}

void addprimitiveCurve(soort)
short soort;
{
	struct Nurb *nu;
	long type;

	if(soort>=10 && soort<20) type= 1;
	else if(soort>=20 && soort<30) type= 2;
	else if(soort>=30 && soort<40) type= 3;
	else if(soort>=40 && soort<50) type= 4;
	else type= 0;
	  
	soort= (soort % 10);
	if(G.ebase->soort== -2) nu= addNurbprim(type, soort);	/* 3D */
	else nu= addNurbprim(8+type, soort);   /* 2D */
	
	addtail(&editNurb,nu);
	makeDispList(G.ebase);
	if(G.mainb==9) tekeneditbuts(1);
	projektie();	
}

void addprimitiveNurb(soort)
short soort;
{
	struct Nurb *nu;

	nu= addNurbprim(4, soort);
	addtail(&editNurb,nu);
	makeDispList(G.ebase);
	if(G.mainb==9) tekeneditbuts(1);
	projektie();
}


void nurbs_to_triface(base)
struct Base *base;
{
	struct ObData *ob;
	struct Nurb *nu;
	struct ListBase disp;
	struct DispList *dl;
	float *data;
	long a, len;

	ob= (struct ObData *)base->d;
	
	convertDispList( &(ob->cu->disp), 0 );	/* maakt wire */
	displist_to_triface(&(ob->cu->disp), ob);
	
	if(ob->vv==0) return;
	base->soort= 1;
	makeDispList(base);
}

void move_to_curve(struct Base *base)
{
	struct MoData *mo;
	struct CurveData *cu;

	if(base->soort!= -2) return;
	
}

void curve_to_move(struct Base *base)
{
	struct ObData *ob;
	struct MoData *mo;
	struct CurveData *cu;
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	int a, b;
	
	if(base->soort!=11) return;

	/* wordspace apply */
	ob= (struct ObData *)base->d;
	cu= ob->cu;

	nu= ob->cu->curve.first;
	while(nu) {
		nu->type &= ~8;	/* 3D curve */
		if( (nu->type & 7)==1 ) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				for(b=0; b<3; b++) {
					VecMulf(bezt->vec[b], cu->ws);
				}
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				VecMulf(bp->vec, cu->ws);
				bp++;
			}
		}
		nu= nu->next;
	}

	mo= callocN(sizeof(struct MoData),"addbase8");
	mo->curve= cu->curve;

	freelistN(&(cu->disp));
	freelistN(&(cu->bev));
	if(cu->key) freekeys(cu->key);
	if(ob->ipovvkey) freeN(ob->ipovvkey);
	freeN(cu);
	freeN(ob);
	G.totobj--;

	base->soort= -2;
	base->d= (long *)mo;
	mo->fr= G.frs;
	mo->f= 1;
	mo->n[2]= 32767;
	G.totmove++;
	
	makeDispList(base);
	berekenpad(base);
}

