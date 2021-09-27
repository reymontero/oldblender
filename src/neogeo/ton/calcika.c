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



#include "ika.h"

void vec_to_quat(vec, quat)		/* altijd neg. z-as, geen up */
float *vec, *quat;
{
	float nor[3], si, co, len, hoek, x2, y2, z2;
	
	x2= vec[0] ; y2= vec[1] ; z2= vec[2];
	
	quat[0]=1.0; 
	quat[1]=quat[2]=quat[3]= 0.0;

	len= fsqrt(x2*x2+y2*y2+z2*z2);
	if(len == 0.0) return;

	nor[0]= -y2;
	nor[1]= x2;
	nor[2]= 0.0;
	co= z2/len;

	Normalise(nor);
	
	hoek= 0.5*facos(co);
	co= fcos(hoek);
	si= fsin(hoek);
	quat[0]= co;
	quat[1]= nor[0]*si;
	quat[2]= nor[1]*si;
	quat[3]= nor[2]*si;
}

void give_rodvec(rod, vec, mode)
struct Rod *rod;
float *vec;
char mode;
{
	float mid[3];
	
	if(rod->type==T_ROD) {
		if(mode=='n') {
			VecMidf(vec, rod->p2->n, rod->p3->n);
			VecSubf(vec, vec, rod->p1->n);
		}
		else {
			VecMidf(vec, rod->p2->r, rod->p3->r);
			VecSubf(vec, vec, rod->p1->r);
		}
	}
	else if(rod->type==H_ROD) {
		if(mode=='n') {
			VecMidf(vec, rod->p1->n, rod->p2->n);
			VecMidf(mid, rod->p3->n, rod->p4->n);
			VecSubf(vec, mid, vec);
		}
		else {
			VecMidf(vec, rod->p1->r, rod->p2->r);
			VecMidf(mid, rod->p3->r, rod->p4->r);
			VecSubf(vec, mid, vec);
		}
	}
	else {
		if(mode=='n') VecSubf(vec, rod->p2->n, rod->p1->n);
		else VecSubf(vec, rod->p2->r, rod->p1->r);
	}
	
}

void calculate_rod(rod, mode)		/* updaten quaternion en matrix */
struct Rod *rod;
char mode;
{
	float co, si, vec1[3], n1[3], len, q1[4], q2[4];
	
	if(rod->type== ROD) {
		/* eerst asrotatie, (die van T en H_ROD zijn gefixeerd) */
		q1[0]= fcos(0.5*rod->alpha);
		q1[1]= q1[2]= 0.0;
		q1[3]= fsin(0.5*rod->alpha);
	
		/* rotatie van z-as op vec1 */
		give_rodvec(rod, vec1, mode);
		vec_to_quat(vec1, q2);
	
		QuatMul(rod->quat, q2, q1);
		QuatToMat3(rod->quat, rod->mat);

	}
	else if(rod->type== T_ROD || rod->type== H_ROD) {
		/* eerst rotatie op hoofdas */
		give_rodvec(rod, vec1, mode);
		vec_to_quat(vec1, q2);
		QuatToMat3(q2, rod->mat);
		
		/* de up */
		if(rod->type==H_ROD) {
			/* pak kruislijnen voor normaal */
			if(mode=='n') {
				VecSubf(vec1, rod->p1->n, rod->p4->n);
				VecSubf(n1, rod->p2->n, rod->p3->n);
			}
			else {
				VecSubf(vec1, rod->p1->r, rod->p4->r);
				VecSubf(n1, rod->p2->r, rod->p3->r);
			}
		}
		else {
			if(mode=='n') VecSubf(n1, rod->p2->n, rod->p3->n);
			else VecSubf(n1, rod->p2->r, rod->p3->r);
		}
		Crossf(rod->up, vec1, n1);
		
		/* asrotatie berekenen */
		Mat3TransMulVecfl(rod->mat, rod->up);
		rod->alpha= M_PI+fatan2(rod->up[1], rod->up[0]);
		/* als fatan2== -PI, dan klappert de quaternion, hoe kan dat nou?. Deze patch werkt */
		
		/* het totaal */
		q1[0]= fcos(0.5*rod->alpha);
		q1[1]= q1[2]= 0.0;
		q1[3]= fsin(0.5*rod->alpha);
		
		QuatMul(rod->quat, q2, q1);
		QuatToMat3(rod->quat, rod->mat);
		
	}
	VECCOPY(rod->up, rod->mat[0]);
		
}

void init_rod(rod, limit)
struct Rod *rod;
float limit;
{
	float mid[3], mid1[3];
	
	if(rod->p4) rod->type= H_ROD;
	else if(rod->p3) rod->type= T_ROD;
	else rod->type= ROD;
	
	if(rod->type==ROD) {
		rod->len= rod->lenn= VecLenf(rod->p1->r, rod->p2->r);
	}
	else if(rod->type==T_ROD) {
		VecMidf(mid, rod->p2->r, rod->p3->r);
		rod->len= rod->lenn= VecLenf(rod->p1->r, mid);
		rod->d2= VecLenf(rod->p2->r, rod->p3->r);
		rod->dia1= VecLenf(rod->p1->r, rod->p2->r);
		rod->dia2= VecLenf(rod->p1->r, rod->p3->r);
	}
	else if(rod->type==H_ROD) {
		VecMidf(mid, rod->p1->r, rod->p2->r);
		VecMidf(mid1, rod->p3->r, rod->p4->r);
		rod->len= rod->lenn= VecLenf(mid1, mid);
		
		rod->len1= VecLenf(rod->p1->r, rod->p3->r);
		rod->len2= VecLenf(rod->p2->r, rod->p4->r);

		rod->d1= VecLenf(rod->p1->r, rod->p2->r);
		rod->d2= VecLenf(rod->p3->r, rod->p4->r);
		
		rod->dia1= VecLenf(rod->p1->r, rod->p4->r);
		rod->dia2= VecLenf(rod->p2->r, rod->p3->r);
	}
	
	rod->alpha= 0.0;
	
	calculate_rod(rod, 'r');
}

void init_joint(joint, mina, maxa, minu, maxu, uprodflag)
struct Joint *joint;
float mina, maxa, minu, maxu;
int uprodflag;
{
	struct Rod *rod1, *rod2;
	float nor[3], n1[3], n2[3], p1[3], p2[3], p3[3];
	float len, curang;

	rod1= joint->r1;
	rod2= joint->r2;

	if(joint->p1a) VecMidf(p1, joint->p1->r, joint->p1a->r);
	else VECCOPY(p1, joint->p1->r);

	if(joint->p2a) VecMidf(p2, joint->p2->r, joint->p2a->r);
	else VECCOPY(p2, joint->p2->r);

	if(joint->p3a) VecMidf(p3, joint->p3->r, joint->p3a->r);
	else VECCOPY(p3, joint->p3->r);

	VecSubf(n1, p1, p2);
	VecSubf(n2, p3, p2);

	/* met quaternions, loodrecht op joint gezien */
	curang= facos( (n1[0]*n2[0]+n1[1]*n2[1]+n1[2]*n2[2])/(rod1->len*rod2->len) );
	
	joint->amin= curang-mina;
	joint->amax= curang+maxa;
	
	/* bereken hoek van normaal op joint met ups */
	/* uprodflag == 1: ook rod2 alpha corrigeren */
	
	Crossf(nor, n1, n2);
	Normalise(nor);
	
	curang= facos(n1[0]*rod1->up[0]+n1[1]*rod1->up[1]+n1[2]*rod1->up[2]);

	rod1->alpha= curang;
	joint->umin= curang-minu;
	joint->umax= curang+maxu;
	
	joint->flag= uprodflag;
	if(uprodflag) {
		rod2->alpha= curang;
	}
	
}

void collide()
{
	struct BodyPoint *p;
	float *r, *v;
	int a;
	
	p= bpbase.first;
	while(p) {
		if(p->r[2]<0.0) {
			p->v[2]= fabs(p->v[2]);
			p->r[2]= 0.0;
		}
		p= p->next;
	}
	
}

void calc_mouseforce(mstart)
short *mstart;
{
	struct BodyPoint *bp;
	float *vec, co, dvec[3];
	int a;
	short mval[2];
	
	bp= bpbase.first;
	while(bp) {
		if(bp->s==1) VECCOPY(dvec, bp->r);
		bp= bp->next;
	}
	
	getmouseco(mval);
	mousevec_to_3D(mstart[0], mstart[1], mval[0], mval[1], dvec);
	
	bp= bpbase.first;
	while(bp) {
		if(bp->s==1) {			
			bp->hold[0]= bp->r[0]+ dvec[0];
			bp->hold[1]= bp->r[1]+ dvec[1];
			bp->hold[2]= bp->r[2]+ dvec[2];
			
			vec= bp->v;
			maxmom-= bp->m*fsqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
			
			vec[0]= dvec[0];
			vec[1]= dvec[1];
			vec[2]= dvec[2];
			
			maxmom+= bp->m*fsqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
		}
		bp= bp->next;
	}
	
	if(maxmom<0.0) maxmom= 0.0;
	
	mstart[0]= mval[0];
	mstart[1]= mval[1];
}

void apply_forces()
{
	struct BodyPoint *bp;
	float *v1, damp= 0.999, mom;
	int a;
	
	maxmom*= damp;

	/* de totale momenten mogen niet afwijken */ 
	mom= 0.0;
	bp= bpbase.first;
	while(bp) {
		v1= bp->v;
		mom+= bp->m*fsqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);
		bp= bp->next;
	}
	if(mom>0.0 && mom!=maxmom) {
		mom= maxmom/mom;

		bp= bpbase.first;
		while(bp) {
			VecMulf(bp->v, mom);
			bp= bp->next;
		}
	}
	
	/* zwaartekracht? */
	bp= bpbase.first;
	bp= 0;
	while(bp) {
		v1= bp->v;
		mom= bp->m*fsqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);
		v1[2]-= .5;
		maxmom+= bp->m*fsqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2])-mom;
		bp= bp->next;
	}
}

void correctfunc(v, len, vec)
float *v, len, *vec;
{

	len*= 0.5;

	v[0]-= len*vec[0];	
	v[1]-= len*vec[1];	
	v[2]-= len*vec[2];
}

float lencorrect(p1, p2, len, fac)	/* geeft huidige lengte terug */
struct BodyPoint *p1, *p2;
float len, fac;
{
	float testlen, d, vec[3];

	testlen= VecLenf(p1->n, p2->n);

	if(len != testlen ) {
		d= fac*(testlen-len)/testlen;
		
		vec[0]= d*(p1->n[0]-p2->n[0]);
		vec[1]= d*(p1->n[1]-p2->n[1]);
		vec[2]= d*(p1->n[2]-p2->n[2]);
		
		p1->v[0]-= vec[0];
		p1->v[1]-= vec[1];
		p1->v[2]-= vec[2];

		p2->v[0]+= vec[0];
		p2->v[1]+= vec[1];
		p2->v[2]+= vec[2];
		
	}
	return testlen;
}

void rodcorrect( struct Rod *rod)
{
	float mid[3], mid2[3], fac1= 0.25;	/* als 2 keer hetzelfde lijnstuk gecorrigeerd wordt -> */
	float *fp, x[4], gemx;		/* is 0.25 het maximum. Anders rare dingen */
				
										
	if(rod->type==ROD) {
		rod->lenn= lencorrect(rod->p1, rod->p2, rod->len, fac1);
	}
	else if(rod->type==T_ROD) {
		lencorrect(rod->p1, rod->p2, rod->dia1, fac1);
		lencorrect(rod->p1, rod->p3, rod->dia2, fac1);
		lencorrect(rod->p2, rod->p3, rod->d2, fac1);
		
		/* tijdelijk? */
		VecMidf(mid, rod->p2->n, rod->p3->n);
		rod->lenn= VecLenf(rod->p1->n, mid);
	}
	else if(rod->type==H_ROD) {
		/* dwing alle punten in een vlak, alleen x is van belang */
		
		/* Hier zit iets bijzonders: de rod->up is gelijk aan mat[0][0,1,2]. 
		 * Toch kunnen de x componten als hieronder worden gecorrigeerd.
		 * Je zou verwachten dat dit met inverse moet! Nu is het een soort inprodukt: vlakvergelijking!!!.
		 * Doe je toch de inverse dan dwingt dit de H_ROD in bepaalde standen (b.v. H_ROD+T_ROD).
		 */
		fp= rod->p1->n;
		x[0]= rod->mat[0][0]*fp[0]+ rod->mat[0][1]*fp[1]+ rod->mat[0][2]*fp[2];
		fp= rod->p2->n;
		x[1]= rod->mat[0][0]*fp[0]+ rod->mat[0][1]*fp[1]+ rod->mat[0][2]*fp[2];
		fp= rod->p3->n;
		x[2]= rod->mat[0][0]*fp[0]+ rod->mat[0][1]*fp[1]+ rod->mat[0][2]*fp[2];
		fp= rod->p4->n;
		x[3]= rod->mat[0][0]*fp[0]+ rod->mat[0][1]*fp[1]+ rod->mat[0][2]*fp[2];
		gemx= 0.25*(x[0]+x[1]+x[2]+x[3]);
		
		correctfunc(rod->p1->v, x[0]-gemx, rod->up);
		correctfunc(rod->p2->v, x[1]-gemx, rod->up);
		correctfunc(rod->p3->v, x[2]-gemx, rod->up);
		correctfunc(rod->p4->v, x[3]-gemx, rod->up);
		
		/* de lengtes */
		lencorrect(rod->p1, rod->p2, rod->d1, fac1);	/* scharnierlengtes */
		lencorrect(rod->p3, rod->p4, rod->d2, fac1);
		lencorrect(rod->p1, rod->p3, rod->len1, fac1);	/* lengtes */
		lencorrect(rod->p2, rod->p4, rod->len2, fac1);
		lencorrect(rod->p1, rod->p4, rod->dia1, fac1);	/* diagonaal */
		lencorrect(rod->p2, rod->p3, rod->dia2, fac1);
		
		/* tijdelijk? */
		VecMidf(mid, rod->p1->n, rod->p2->n);			/* lengte as */
		VecMidf(mid2, rod->p3->n, rod->p4->n);
		rod->lenn= VecLenf(mid, mid2);

	}
	
}


void jointcorrect( struct Joint *joint)
{
	struct Rod *rod1, *rod2;
	float *v1, *v2, *v3, quat[4], nor[3], n1[3], n2[3], ch3[3], ch1[3];
	float co, si, curang, da, mat[3][3], p1[3], p2[3], p3[3];
	int ret= 0;
	
	rod1= joint->r1;
	rod2= joint->r2;

	if(joint->p1a) VecMidf(p1, joint->p1->n, joint->p1a->n);
	else VECCOPY(p1, joint->p1->n);

	if(joint->p2a) VecMidf(p2, joint->p2->n, joint->p2a->n);
	else VECCOPY(p2, joint->p2->n);

	if(joint->p3a) VecMidf(p3, joint->p3->n, joint->p3a->n);
	else VECCOPY(p3, joint->p3->n);

	VecSubf(n1, p1, p2);
	VecSubf(n2, p3, p2);

	/* de ALPHA: met quaternions, loodrecht op joint gezien */
	curang= (n1[0]*n2[0]+n1[1]*n2[1]+n1[2]*n2[2])/(rod1->lenn*rod2->lenn);
	if(curang<= -1.0) curang= M_PI;
	else if(curang>= 1.0) curang= 0.0;
	else curang= facos(curang);

	if(curang < joint->amin || curang > joint->amax) {

		if(curang > joint->amax) da= joint->amax-curang;
		else da= joint->amin-curang;

		da*= 0.10;

		Crossf(nor, n1, n2);
		Normalise(nor);
		co= fcos(da);
		si= fsin(da);
		quat[0]= co;
		quat[1]= nor[0]*si;
		quat[2]= nor[1]*si;
		quat[3]= nor[2]*si;
		
		ret= 1;
	}
	
	/* de UP */
	
	if(rod1->type==ROD || rod2->type==ROD) {
	
		if(ret==0) {	/* hoeft ie niet twee keer */
			Crossf(nor, n1, n2);
			Normalise(nor);
		}
		
		if(rod1->type==ROD) {
			curang= facos(nor[0]*rod1->up[0]+nor[1]*rod1->up[1]+nor[2]*rod1->up[2]);
		
			/* nor terug roteren vanuit rod1 gezien, alleen y is nodig want */
			/* up is de positieve x-as, z.comp. van uitprodukt bepaalt draairichting */
			da= nor[0]*rod1->mat[1][0]+ nor[1]*rod1->mat[1][1]+ nor[2]*rod1->mat[1][2];
			if(da>0.0) curang= -curang;
			
			if(curang < joint->umin || curang > joint->umax) {
				
				if(curang > joint->umax) da= joint->umax-curang;
				else da= joint->umin-curang;
		
				da*= 0.5;
				
				rod1->alpha+= da;
			}
		}
		if(rod2->type==ROD && joint->flag & 1) {	/* beide ups */
			curang= facos(nor[0]*rod2->up[0]+nor[1]*rod2->up[1]+nor[2]*rod2->up[2]);
			
			da= nor[0]*rod2->mat[1][0]+ nor[1]*rod2->mat[1][1]+ nor[2]*rod2->mat[1][2];
			if(da>0.0) curang= -curang;
	
			if(curang < joint->umin || curang > joint->umax) {
		
				if(curang > joint->umax) da= joint->umax-curang;
				else da= joint->umin-curang;
		
				da*= 0.5;
				
				rod2->alpha+= da;
			}
		}
	}
	
	if(ret) {
		/* de correctiequat: */
		QuatToMat3(quat, mat);

		VECCOPY(ch3, n2);	/* rod2 */
		Mat3MulVecfl(mat, ch3);
		VecSubf(ch3, ch3, n2);	/* deltavec */
		
		VECCOPY(ch1, n1);	/* rod1 */
		Mat3TransMulVecfl(mat, ch1);
		VecSubf(ch1, ch1, n1);	/* deltavec */
	
		/* en toepassen */
		v1= joint->p1->v;
		v3= joint->p3->v;
		
		VecAddf(v1, v1, ch1);
		VecAddf(v3, v3, ch3);
		if(joint->p1a) {
			v1= joint->p1a->v;
			VecAddf(v1, v1, ch1);
		}
		if(joint->p3a) {
			v3= joint->p3a->v;
			VecAddf(v3, v3, ch3);
		}
		
	}
}

void itterate()
{
	float speeddamp=0.0, locdamp=0.0, inv;
	struct BodyPoint *bp;
	struct Rod *rod;
	struct Joint *joint;
	int loop=0, maxloop= 4;
	
	/* bereken nieuw punt volgens snelheden */
	bp= bpbase.first;
	while(bp) {
		if(bp->s==1) {
			VECCOPY(bp->n, bp->hold);
		}
		else {
			bp->n[0]= bp->r[0]+ bp->v[0]/bp->m;
			bp->n[1]= bp->r[1]+ bp->v[1]/bp->m;
			bp->n[2]= bp->r[2]+ bp->v[2]/bp->m;
		}
		bp= bp->next;
	}
	
	while(loop<maxloop) {
		loop++;
		
		rod= rodbase.first;

		while(rod) {
			rodcorrect(rod);
			rod= rod->next;
		}
		
		joint= jointbase.first;
		while(joint) {
			jointcorrect(joint);
			joint= joint->next;
		}

		bp= bpbase.first;
		while(bp) {
			if( bp->s==0) {
				
				/* flerp3(speeddamp, bp->v, bp->vn); */
				
				bp->n[0]= bp->r[0]+ bp->v[0]/bp->m;
				bp->n[1]= bp->r[1]+ bp->v[1]/bp->m;
				bp->n[2]= bp->r[2]+ bp->v[2]/bp->m;
			}
			bp= bp->next;
		}
		
		rod= rodbase.first;
		while(rod) {
			calculate_rod(rod, 'n');
			rod= rod->next;
		}
	}
	
	bp= bpbase.first;
	while(bp) {
		if(bp->s==1) {
			VECCOPY(bp->r, bp->n);
		}
		else {
			flerp3(1.0-locdamp, bp->r, bp->n);
		}
		bp= bp->next;
	}
	
	/* hier pas de botsing ? */
	/* collide(); */
}


