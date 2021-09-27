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


void calculate_rod(rod)
rod_const *rod;
{
	float vec1[3], co, si, len;
	
	VecSubf(vec1, pdata[rod->e2].r, pdata[rod->e1].r);
	vec_to_quatmat(vec1, rod->quat, rod->mat);
	
	co= fcos(rod->alpha);
	si= fsin(rod->alpha);
	rod->up[0]= co*rod->mat[0][0]+si*rod->mat[1][0];
	rod->up[1]= co*rod->mat[0][1]+si*rod->mat[1][1];
	rod->up[2]= co*rod->mat[0][2]+si*rod->mat[1][2];
}

void init_rod(rod, limit)
rod_const *rod;
float limit;
{
	float *v1, *v2, len;
	
	v1= pdata[rod->e1].r;
	v2= pdata[rod->e2].r;
	rod->len= VecLenf(v1, v2);
	
	rod->l1= (1.0-limit)*len;
	rod->l2= (1.0+limit)*len;
	
	rod->alpha= 0.0;
	
	calculate_rod(rod);
}

void init_joint(joint, minx, maxx, miny, maxy, minu, maxu)
joint_const *joint;
float minx, maxx, miny, maxy, minu, maxu;
{
	rod_const *rod1, *rod2;
	float *v1, *v2, *v3, n1[3], n2[3];
	float len, curang;

	rod1= rdata+joint->r1;
	rod2= rdata+joint->r2;

	v1= (pdata+joint->e1)->r;
	v2= (pdata+joint->e2)->r;
	v3= (pdata+joint->e3)->r;

	VecSubf(n1, v1, v2);
	VecSubf(n2, v3, v2);

	/* met quaternions, loodrecht op joint gezien */
	curang= facos( (n1[0]*n2[0]+n1[1]*n2[1]+n1[2]*n2[2])/(rod1->len*rod2->len) );
	
	
	joint->amin= curang-minx;
	joint->amax= curang+maxx;
	
}

void collide()
{
	point *p;
	float *r, *v;
	int a;
	
	p= pdata;
	for(a=0; a<NP; a++, p++) {
		if(p->r[2]<0.0) {
			p->v[2]= fabs(p->v[2]);
			p->r[2]= 0.0;
		}
	}
	
}

void distribute_reactionforce()
{
	long a;
	float corr[3], div= 0.0;

	/* eerst de oude snelheid van de vastgehouden punten optellen */
	corr[0]= corr[1]= corr[2]= 0.0;
	for(a=0; a<NP; a++) {
		if(pdata[a].s==1) {
			VecAddf(corr, corr, pdata[a].v);
			VECCOPY(pdata[a].vn, pdata[a].v);
			pdata[a].v[0]= 0.0;
			pdata[a].v[1]= 0.0;
			pdata[a].v[2]= 0.0;
		}
		else div+= 1.0;	/* moet eigenlijk massa zijn */
	}
	if(div!=0.0) {
		VecMulf(corr, 1.0/div);
		for(a=0; a<NP; a++) {
			if(pdata[a].s!=1) {
				VecSubf(pdata[a].v, pdata[a].v, corr);
			}
		}
	}
	
}

void calc_mouseforce(mstart)
short mstart[2];
{
	float *vec, co, dvec[3];
	int a;
	short mval[2];
	
	for(a=0; a<NP; a++) {
		if(pdata[a].s==1) VECCOPY(dvec, pdata[a].r);
	}
	
	getmouseco(mval);
	mousevec_to_3D(mstart[0], mstart[1], mval[0], mval[1], dvec);
			
	/* distribute_reactionforce(); */
	
	for(a=0; a<NP; a++) {
		if(pdata[a].s==1) {			
			pc[3*a]= pdata[a].r[0]+ dvec[0];
			pc[3*a+1]= pdata[a].r[1]+ dvec[1];
			pc[3*a+2]= pdata[a].r[2]+ dvec[2];
			
			vec= pdata[a].v;	/* vn is uit distr.reac.f */
			maxmom-= fsqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
			
			vec= pdata[a].v;
			vec[0]= dvec[0]/pdata[a].m;
			vec[1]= dvec[1]/pdata[a].m;
			vec[2]= dvec[2]/pdata[a].m;
			
			maxmom+= fsqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
		}
	}
	
	if(maxmom<0.0) maxmom= 0.0;
	
	mstart[0]= mval[0];
	mstart[1]= mval[1];
}

void apply_forces()
{
	float *v1, damp= 0.999, mom;
	int a;
	
	maxmom*= damp;

	/* de totale momenten mogen niet afwijken */ 
	mom= 0.0;
	for(a=0; a<NP; a++) {
		v1= pdata[a].v;
		mom+= fsqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);
	}
	if(mom>0.0 && mom!=maxmom) {
		mom= maxmom/mom;
		for(a=0; a<NP; a++) {
			VecMulf(pdata[a].v, mom);
		}
	}
	
	/* zwaartekracht? */
	for(a=0; a<NP; a++) {
		v1= pdata[a].v;
		/* v1[1]-= .1; */
	}
}

int lencorrect( rod_const *r)
{
	float d, vec[3], len, *v1, *v2, fac1= 0.35;
	
	v1= pdata[r->e1].n;
	v2= pdata[r->e2].n;
	
	rod->len= len= VecLenf(v1, v2);
	if(len < r->l1 || len > r->l2) {
		if(len<r->l1) d= fac1*(len-r->l1)/len;
		else d= fac1*(len-r->l2)/len;
		
		vec[0]= d*(v1[0]-v2[0]);
		vec[1]= d*(v1[1]-v2[1]);
		vec[2]= d*(v1[2]-v2[2]);
		
		v1= pdata[r->e1].v;
		v1[0]-= vec[0];
		v1[1]-= vec[1];
		v1[2]-= vec[2];
		v2= pdata[r->e2].v;
		v2[0]+= vec[0];
		v2[1]+= vec[1];
		v2[2]+= vec[2];
		
		return 1;
	}
	return 0;
}


int anglecorrect( joint_const *joint)
{
	rod_const *rod1, *rod2;
	float *v1, *v2, *v3, temp[3], n1[3], n2[3], ch3[3], ch1[3];
	float len, curang, da, mat[3][3], matx[3][3], maty[3][3];
	int ret= 0;
	
	rod1= rdata+joint->r1;
	rod2= rdata+joint->r2;

	v1= (pdata+joint->e1)->r;
	v2= (pdata+joint->e2)->r;
	v3= (pdata+joint->e3)->r;

	/* vanuit rod1 gezien: min en max radialen afwijking */
	/* mat is orthogonaal: transpose == inverse */
	
	VecSubf(n1, v1, v2);
	VecSubf(n2, v3, v2);
	Mat3TransMulVecfl(rod1->mat, n2);

	/* x-z vlak */
	/* if(n2[0]!=0.0 && n2[2]!=0.0) curang= fatan2(n2[2], n2[0]); */
	/* else curang= 0.0; */
	
	/* alsof het 2D is: */
	len= fsqrt(n2[0]*n2[0]+ n2[2]*n2[2]);
	if(len!=0.0) {
		curang= facos( n2[0]/len );
		if(n2[2]<0.0) curang= 2*M_PI-curang;
	}
	else curang= 0.0;
	
	Mat3One(matx);
	Mat3One(maty);

	if(curang < joint->amin || curang > joint->amax) {

		if(curang > joint->amax) da= joint->amax-curang;
		else da= joint->amin-curang;

		da*= 0.15;

		matx[0][0]= fcos(da);
		matx[0][2]= fsin(da);
		matx[2][0]= -fsin(da);
		matx[2][2]= fcos(da);


		ret= 1;
	}
	
	if(ret) {
		/* de totale correctiematrix: */
		Mat3MulMat3(mat, maty, matx);

		VECCOPY(ch3, n2);	/* rod2 */
		Mat3MulVecfl(mat, ch3);
		VecSubf(ch3, ch3, n2);	/* deltavec */
		Mat3MulVecfl(rod1->mat, ch3);
		
		ch1[0]=ch1[1]= 0.0; ch1[2]= 1.0;	/* rod1 */
		Mat3TransMulVecfl(mat, ch1);
		ch1[2]-= 1.0;			/* deltavec */
		Mat3MulVecfl(rod1->mat, ch1);
	
		/* en toepassen */
		v1= (pdata+joint->e1)->v;
		v3= (pdata+joint->e3)->v;
		
		VecAddf(v1, v1, ch1);
		VecAddf(v3, v3, ch3);
		
	}
	
	return ret;
}

void itterate()
{
	float speeddamp=0.0, locdamp=0.0, inv;
	point *p;
	int a, loop=0, maxloop= 4, ok;
	
	/* bereken nieuw punt volgens snelheden */
	p= pdata;
	for(a= 0; a<NP; a++, p++) {
		VECCOPY(p->vn, p->v);
		if(p->s==1) {
			VECCOPY(p->n, pc+3*a);
		}
		else {
			p->n[0]= p->r[0]+ p->v[0]/p->m;
			p->n[1]= p->r[1]+ p->v[1]/p->m;
			p->n[2]= p->r[2]+ p->v[2]/p->m;
		}
	}
	
	do {
		ok= 1;
		loop++;
		
		for(a=0; a<NR; a++) {
			if( lencorrect( rdata+a ) ) ok= 0;
		}

		for(a=0; a<NJ; a++) {
			if( anglecorrect( jdata+a ) ) ok= 0;
		}

		p= pdata;
		for(a=0; a<NP; a++, p++) {
			if( p->s==0) {
				
				flerp3(speeddamp, p->v, p->vn);
				
				p->n[0]= p->r[0]+ p->v[0]/p->m;
				p->n[1]= p->r[1]+ p->v[1]/p->m;
				p->n[2]= p->r[2]+ p->v[2]/p->m;
			}
		}


	} while( ok==0 && loop<maxloop );
	
	p= pdata;
	for(a=0; a<NP; a++, p++) {
		if(p->s==1) {
			VECCOPY(p->r, p->n);
		}
		else {
			flerp3(1.0-locdamp, p->r, p->n);
		}
	}
	
	/* hier pas de botsing ? */
	/* collide(); */
}


