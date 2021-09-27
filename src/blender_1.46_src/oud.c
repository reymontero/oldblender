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




int intersect_dface_nocor(DFace *dface, struct Snijp *sn, float *oldloc, float *speed)
{
	float ndist, labda, s, t, inploc, inpspeed;
	int cox=0, coy=1;
	
	if(dface->v3==0) return 0;

	inpspeed= dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2];

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];
		ndist= dface->dist - inploc;	/* negatief! */
		
		if(ndist>0.0) return 0;
		
		labda= (ndist)/inpspeed;	/* voor s-t */

		/* labda is gegarandeerd > 0! (ndist<0.0) */
		
		if(labda<sn->minlabda) return 0;
		else if(labda> sn->labda) return 0;
		
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}

		s= oldloc[cox] + labda*speed[cox];
		t= oldloc[coy] + labda*speed[coy];

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) - 
			(dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) < 0.0 ) 
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) -
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox])< 0.0 ) 
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox])< 0.0 ) 
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) -
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox])< 0.0 ) 
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox])< 0.0 )
				return 0;
		}

		sn->labda= labda;
		sn->inpspeed= inpspeed;
		sn->face= dface;
		
		return 1;
	}
	
	return 0;
}

#define S14MUL(a, b)			( ((a)*(b))>>14 )

#define DYNAFIX		16384
#define DYNAFIXF	16384.0

#define UNSURE	(320<<14)
#define UNSURE1	(1024)


void isect_vert_conf(short *svec, float *vert)
{
	svec[0]= (DYNAFIXF*vert[0]);
	svec[1]= (DYNAFIXF*vert[1]);
	svec[2]= (DYNAFIXF*vert[2]);
}

struct pSnijp {
	int labda, minlabda, inpspeed;
	pDFace *face;
	MATRIX *obmat;	/* als hier iets staat: ook doen! */
};


/* nieuwe methode: met correctie normaal!!! */

int psx_intersect_dface(pDFace *dface, struct pSnijp *sn, int *oldloc, short *speed)
{
	int labda, labdacor, ndist, s, t, inploc, inpspeed;
	int cox=0, coy=1;
	int a, b;

	if(dface->v3==0) return 0;
	
	inpspeed= (dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2]);

	/* single sided! */	
	if(inpspeed < -160) {
	
		inploc= (dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2]);
		
		ndist= (dface->dist - inploc);
		/*  test 1: mag je delen? */
		if(ndist>=0 || (ndist+UNSURE) <= inpspeed) return 0;

		labdacor= ( (ndist+UNSURE)<<4 )/(inpspeed>>10);
		labda= (ndist<<4)/(inpspeed>>10);

		/* debug test oftie nog leeft */
/* printf("fa %x lab %d min %d\n", dface, labdacor, sn->minlabda); */

		/* dit is een soort interval-wiskunde */
		/*  deze twee regels zijn samen nodig: geval 1 afvangen */
		if(labdacor<=sn->minlabda) {
			if(labda<=sn->minlabda) return 0;
		}
		if(labdacor>=sn->labda) {
			if(labda>=sn->labda) return 0;
			/*  voorkomt hobbelen */
		}

		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}

		s= oldloc[cox] + S14MUL(labda, speed[cox]);
		t= oldloc[coy] + S14MUL(labda, speed[coy]);

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) < 
		    (dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) )
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) <
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox]) )
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox]) )
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) <
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox]) )
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox]) )
				return 0;
		}

		sn->labda= labdacor;
		sn->inpspeed= (inpspeed>>14); 		
		sn->face= dface;

		return 1;
	}

	return 0;
}


int intersect_dface_test(DFace *dface, struct Snijp *sn, float *oldloc, float *speed)
/* testversie voor fixpoint math */
{
	pDFace *pdface, rtdf;
	struct pSnijp psn;
	float labda, s, t, inploc, inpspeed;
	int ilabda, is, it, iinploc, iinpspeed;
	int a, b, cox=0, coy=1, ioldloc[3];
	short verts[4][3], ispeed[3];
	
	if(dface->v3==0) return 0;

	/* **************** CONVERT *********** */
	pdface= &rtdf;
	
	pdface->v1= verts[0]; isect_vert_conf(verts[0], dface->v1);
	pdface->v2= verts[1]; isect_vert_conf(verts[1], dface->v2);
	pdface->v3= verts[2]; isect_vert_conf(verts[2], dface->v3);
	if(dface->v4) {
		pdface->v4= verts[3]; isect_vert_conf(verts[3], dface->v4);
	}
	else pdface->v4= 0;
	
	pdface->dist= (DYNAFIXF*(DYNAFIXF*dface->dist));
	pdface->proj= dface->proj;
	
	pdface->no[0]= (DYNAFIXF*dface->no[0]);
	pdface->no[1]= (DYNAFIXF*dface->no[1]);
	pdface->no[2]= (DYNAFIXF*dface->no[2]);
	
	isect_vert_conf(ispeed, speed);
	ioldloc[0]= (DYNAFIXF*oldloc[0]);
	ioldloc[1]= (DYNAFIXF*oldloc[1]);
	ioldloc[2]= (DYNAFIXF*oldloc[2]);

	psn.labda= (DYNAFIXF*sn->labda);
	psn.minlabda= (DYNAFIXF*sn->minlabda);
	psn.inpspeed= (DYNAFIXF*sn->inpspeed);
	
	/************************* */
	
	if( psx_intersect_dface(pdface, &psn,  ioldloc, ispeed) ) {
		
		sn->labda= ((float)psn.labda)/DYNAFIXF;
		sn->minlabda= ((float)psn.minlabda)/DYNAFIXF;
		sn->inpspeed= ((float)psn.inpspeed)/DYNAFIXF;
		sn->face= dface;
		return 1;
	}
	return 0;
}


