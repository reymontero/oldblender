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

float dproj=1.0, phi=0.959, theta= -0.770, viewmat[4][4], winmat[4][4], persmat[4][4], persinv[4][4];

float mat1[4][4]= {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

float v[8][3] = {
	{-1.2, -1.2, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.2,  1.2, -1.0},
	{ 1.2, -1.2, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.2,  1.2, -1.0},
};

float tex[]= {
	AMBIENT, .1, .1, .1,
	DIFFUSE, .8, .9, 1.0,
	SPECULAR, 0.0, 0.0, 0.0,
	SHININESS, 0.0,
	LMNULL,
};
float lm[]= {
	AMBIENT, .1, .1, .1,
	LOCALVIEWER, 1,
	LMNULL,
};
float lt[]= {
	LCOLOR, 1.0, 1.0, 1.0,
	POSITION, -1.2, -1.0, 1.0, 0.0,
	LMNULL,
};
float blue_light[] =
    {LCOLOR,    0.3,0.3,0.9,   /* blue light */
     POSITION,  0.2,0.1,0.0,0.0,  /* Y axis at infinity */
     LMNULL};

void deflighting()
{
	lmdef(DEFMATERIAL, 1, 0, tex);
	lmdef(DEFLIGHT, 1, 10, lt);
	lmdef(DEFLIGHT, 2, 10, blue_light);
	lmdef(DEFLMODEL, 1, 0, lm);
}

void bindlighting()
{
	lmbind(MATERIAL, 1);
	lmbind(LIGHT0, 1);
	lmbind(LIGHT1, 2);
	lmbind(LMODEL, 1);
}

/* ****************** VIEW IKA ********************* */

void drawgourcube()
{
	float vec[3];
	float n[3];

	n[0]=0; n[1]=0; n[2]=0;
	bgnpolygon();
		n[0]= -1.0;
		n3f(n); 
		v3f(v[0]); v3f(v[1]); v3f(v[2]); v3f(v[3]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= -1.0;
		n3f(n); 
		v3f(v[0]); v3f(v[4]); v3f(v[5]); v3f(v[1]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[0]= 1.0;
		n3f(n); 
		v3f(v[4]); v3f(v[7]); v3f(v[6]); v3f(v[5]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= 1.0;
		n3f(n); 
		v3f(v[7]); v3f(v[3]); v3f(v[2]); v3f(v[6]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[2]= 1.0;
		n3f(n); 
		v3f(v[1]); v3f(v[5]); v3f(v[6]); v3f(v[2]);
		n[2]=0;
	endpolygon();

	bgnpolygon();
		n[2]= -1.0;
		n3f(n); 
		v3f(v[7]); v3f(v[4]); v3f(v[0]); v3f(v[3]);
	endpolygon();
}

void drawcube()
{

	bgnline();
		v3f(v[0]); v3f(v[1]);v3f(v[2]); v3f(v[3]);
		v3f(v[0]); v3f(v[4]);v3f(v[5]); v3f(v[6]);
		v3f(v[7]); v3f(v[4]);
	endline();
	bgnline();
		v3f(v[1]); v3f(v[5]);
	endline();
	bgnline();
		v3f(v[2]); v3f(v[6]);
	endline();
	bgnline();
		v3f(v[3]); v3f(v[7]);
	endline();
}


void mousevec_to_3D(mx1, my1, mx2, my2, vec)
short mx1, my1, mx2, my2;
float *vec;
{
	float z, dx, dy;

	z= persmat[0][3]*vec[0]+persmat[1][3]*vec[1]+persmat[2][3]*vec[2]+persmat[3][3];

	dx= 2.0*(mx2-mx1)*z/(float)sizex;
	dy= 2.0*(my2-my1)*z/(float)sizey;

	vec[0] = (persinv[0][0]*dx + persinv[1][0]*dy);
	vec[1] = (persinv[0][1]*dx + persinv[1][1]*dy);
	vec[2] = (persinv[0][2]*dx + persinv[1][2]*dy);
}

void persp(a)
short a;
{
	static short old_a=1;

	if(old_a==a) return;  /* dat scheelt weer !? */
	
	if(a== 0) {
		ortho2(0.0, (float)(sizex-1), 0.0, (float)(sizey-1));
		loadmatrix(mat1);
	}
	else if(a== 1) {
		mmode(MPROJECTION);
		loadmatrix(winmat);
		mmode(MVIEWING);
		loadmatrix(viewmat);
	}

	old_a= a;
}


void berekenschermcofl(vec, adr)
float *vec;
short *adr;
{
	float fx, fy, vec4[4];

	adr[0]= 3200;
	VECCOPY(vec4, vec);
	vec4[3]= 1.0;
	Mat4MulVec4fl(persmat, vec4);
	if( vec4[3]>0.1 ) {
		fx= (sizex/2)+(sizex/2)*vec4[0]/vec4[3];
		if( fx>0 && fx<sizex) {
			fy= (sizey/2)+(sizey/2)*vec4[1]/vec4[3];
			if(fy>0 && fy<sizey) {
				adr[0]= fx;
				adr[1]= fy;
			}
		}
	}
}


void tekengrond()
{
	float vec[3];
	float a;
	
	cpack(0x404040);
	vec[2]= 0.0;
	for(a= -400.0; a<=400.0; a+= 100.0) {
		bgnline();
			vec[0]= a;
			vec[1]= -400.0;
			v3f(vec);
			vec[1]= 400.0;
			v3f(vec);
		endline();
	}
	for(a= -400.0; a<=400.0; a+= 100.0) {
		bgnline();
			vec[1]= a;
			vec[0]= -400.0;
			v3f(vec);
			vec[0]= 400.0;
			v3f(vec);
		endline();
	}
	
}

void drawwiretube(struct Tube *tu)
{
	float *data;
	int parts, nr, ofs;

	data= tu->data;

	parts= tu->v;
	while(parts--) {
		nr= tu->u;
		bgnline();
		while(nr--) {
			v3f(data);
			data+=3;
		}
		endline();
	}
	ofs= 3*tu->u;
	nr= tu->u;
	while(nr--) {
		data= ( tu->data )+3*nr;
		parts= tu->v;
		bgnline();
		while(parts--) {
			v3f(data);
			data+=ofs;
		}
		endline();
	}
}

void drawgourtube(struct Tube *tu)
{
	float *v1, *v2, *v3, *v4, *data, norm[3];
	int a, b, p1, p2, p3, p4;
	
	shademodel(GOURAUD);
	data= tu->data;
	for(a=0; a<tu->v-1; a++) {

		p2= tu->u*a;
		p1= p2+1;
		p4= p2+ tu->u;
		p3= p1+ tu->u;

		v1= data+ 3*p1; 
		v2= data+ 3*p2;
		v3= data+ 3*p3; 
		v4= data+ 3*p4;

		for(b=1; b<tu->u; b++) {
			CalcNormFloat(v1, v2, v3, norm);
			bgnpolygon();
			n3f(norm);
			v3f(v1);
			v3f(v2);
			v3f(v4);
			v3f(v3);
			endpolygon();

			v2= v1; 
			v1+= 3;
			v4= v3; 
			v3+= 3;
		}
	}
	
}

float *tempdata=0;
extern float VecLenf();

void calcdeform(struct Tube *tu, struct Skeleton *skel)
{
	struct RodDeform *rdef;
	float *newdata, *fromdata, vec1[3], vec2[3], fac[4], totfac;
	float new[4][3];
	int nr, rodnr, totrod=3;
	
	if(skel==0) return;
	
	nr= tu->u*tu->v;
	newdata= mallocN(4*3*nr, "calcdeform");
	tempdata= fromdata= tu->data;
	tu->data= newdata;
	
	/* we lopen punt voor punt af, rod voor rod */
	while(nr--) {
		VecAddf(vec1, fromdata, tu->vec);
		totfac= 0.0;
		
		for(rodnr= 0; rodnr<totrod; rodnr++) {
		
			/* dit doen we even primitief */
			rdef= skel->r1->rdef;
			if(rodnr==1) rdef= skel->r2->rdef;
			else if(rodnr==2) rdef= skel->r3->rdef;
			
			fac[rodnr]= VecLenf(rdef->orig1, vec1) + VecLenf(rdef->orig2, vec1);
			fac[rodnr]= 1.0/fac[rodnr];
			fac[rodnr]*= fac[rodnr];
			fac[rodnr]*= fac[rodnr];
			totfac+= fac[rodnr];
			
			/* rotatie t.o.v. middelpunt edge */
			VecSubf(new[rodnr], vec1, rdef->origmid);
	
			Mat3MulVecfl(rdef->originv, new[rodnr]);
			Mat3MulVecfl(rdef->mat, new[rodnr]);

			VecAddf(new[rodnr], new[rodnr], rdef->origmid);
			
			/* translatie */
			VecMidf(vec2, rdef->v1, rdef->v2);
			VecSubf(vec2, vec2, rdef->origmid);
			VecAddf(new[rodnr], new[rodnr], vec2);
		}

		newdata[0]= newdata[1]= newdata[2]= 0.0;
		for(rodnr= 0; rodnr<totrod; rodnr++) {
			fac[rodnr]/= totfac;
			VecMulf(new[rodnr], fac[rodnr]);
			VecAddf(newdata, newdata, new[rodnr]);
		}
		newdata+= 3;
		fromdata+= 3;
	}
}

void restoredeform(struct Tube *tu)
{
	if(tempdata) {
		freeN(tu->data);
		tu->data= tempdata;
		tempdata= 0;
	}
}

void drawscenesub()
{
	struct Rod *r;
	struct BodyPoint *p;
	struct Tube *tu;
	float vec[3], mat4[4][4], x, y, z, *data;
	int ofs, nr, parts, a, drawtype= 2;
	
	mmode(MVIEWING);
	
	nr= winget();
	if(nr==subwin2) {
		loadmatrix(mat1);
		polarview(dproj*1400.0, (1800/M_PI)*(theta+2.5), (1800/M_PI)*(phi+2.0), 0.0);
		translate(0.0, 0.0, -50.0);
	}
	else if(nr==subwin) {
		loadmatrix(mat1);
		polarview(dproj*500.0, (1800/M_PI)*(theta+2.5), (1800/M_PI)*phi, 0.0);
		translate(0.0, 0.0, -50.0);
	}
	else {
		loadmatrix(mat1);
		polarview(dproj*1400.0, (1800/M_PI)*theta, (1800/M_PI)*phi, 0.0);
		translate(0.0, 0.0, -50.0);
	}
	
	getmatrix(viewmat);
	perspective(300, (float)sizex/(float)sizey, 5.0, 50000.0);
	mmode(MPROJECTION);
	getmatrix(winmat);
	Mat4MulMat4(persmat, viewmat, winmat);
	Mat4Inv(persinv, persmat);
	mmode(MVIEWING);

	if(drawtype==2) {
		czclear(0x707070,0x7FFFFF);
		zbuffer(1);
		shademodel(GOURAUD);
		bindlighting();
	}
	else {
		cpack(0x707070);
		clear();
		zbuffer(0);
	}

	tekengrond();

	/* teken en bereken rods */
	r= rodbase.first;
	while(r) {
		
		if(r->rdef) copy_defstate(r);
		
		cpack(0);
		if(drawtype==0) {
			if(r->type==ROD) {
				bgnline();
					v3f( r->p1->r );
					v3f( r->p2->r );
				endline();
			}
			else  {
				bgnclosedline();
					v3f( r->p1->r );
					v3f( r->p2->r );
					v3f( r->p3->r );
					if(r->type==H_ROD) v3f( r->p4->r );
				endclosedline();
			}
		}
		else {
			pushmatrix();

			Mat4CpyMat3(mat4, r->mat);
			if(r->type==ROD)
				VecMidf(mat4[3], r->p1->r, r->p2->r);
			else if(r->type==T_ROD) {
				VecMidf(vec, r->p2->r, r->p3->r);
				VecMidf(mat4[3], r->p1->r, vec);
			}
			else if(r->type==H_ROD) {
				CalcCent4f(mat4[3], r->p1->r, r->p2->r, r->p3->r, r->p4->r);
			}
			x= 0.1*r->len;
			z= 0.45*r->lenn;
			mat4[0][0]*=x; mat4[0][1]*=x; mat4[0][2]*=x;
			mat4[1][0]*=x; mat4[1][1]*=x; mat4[1][2]*=x;
			mat4[2][0]*=z; mat4[2][1]*=z; mat4[2][2]*=z;
			multmatrix(mat4);
			if(drawtype==1) drawcube();
			else drawgourcube();
			
			popmatrix();

			if(r->type==T_ROD) {
				cpack(0);
				bgnline();
					v3f( r->p2->r );
					v3f( r->p3->r );
				endline();
			}
			else if(r->type==H_ROD) {
				cpack(0);
				bgnline();
					v3f( r->p1->r );
					v3f( r->p2->r );
				endline();
				bgnline();
					v3f( r->p3->r );
					v3f( r->p4->r );
				endline();
			}
		}
		if(TRUE) {	/* teken ups met mat4 */
			if(drawtype==2) cpack(0);
			else cpack(0x404040);
			bgnline();
				v3f(mat4[3]);
				mat4[3][0]+= 0.3*r->len*r->up[0];
				mat4[3][1]+= 0.3*r->len*r->up[1];
				mat4[3][2]+= 0.3*r->len*r->up[2];
				v3f(mat4[3]);
			endline();
		}

		r= r->next;
	}
	
	tu= tubase.first;
	while(tu) {
		pushmatrix();
		if(skel==0) translate(tu->vec[0], tu->vec[1], tu->vec[2]);
		
		if(skel) calcdeform(tu, skel);
		
		if(drawtype==2) drawgourtube(tu);
		else drawwiretube(tu);
				
		if(skel) restoredeform(tu);

		tu= tu->next;
		popmatrix();
	}
	
	if(FALSE) {	/* teken speedvec */
		cpack(0xFFFF);
		p= bpbase.first;
		while(p) {
			bgnline();
				v3f(p->r);
				VECCOPY(vec, p->v);
				/* VecMulf(vec, 3.0); */
				VecAddf(vec, p->r, vec);
				v3f(vec);
			endline();
			p= p->next;
		}
		
	}
	if(TRUE) {	/* teken ups */
		if(drawtype==2) cpack(0);
		else cpack(0x404040);
		r= rodbase.first;
		while(r) {
			if(r->type==ROD) {
				VecMidf(vec, r->p1->r, r->p2->r);
				bgnline();
					v3f(vec);
					vec[0]+= 0.3*r->len*r->up[0];
					vec[1]+= 0.3*r->len*r->up[1];
					vec[2]+= 0.3*r->len*r->up[2];
					v3f(vec);
				endline();
			}
			r= r->next;
		}		
	}
	
	/* teken verts en berekenschermco's */
	p= bpbase.first;
	persp(0);
	while(p) {
		berekenschermcofl(p->r, &(p->sx));
		if(p->sx!=3200) {
			if(p->s==1) cpack(0xFFFF);
			else cpack(0xFF00FF);
			sboxfs(p->sx-1, p->sy-1, p->sx+1, p->sy+1 );
		}
		p= p->next;
	}
	
	persp(1);
}

void drawscene()
{
	
	/* winset(subwin2); */
	/* drawscenesub(); */
	/* swapbuffers(); */
	/* winset(subwin); */
	/* drawscenesub(); */
	/* swapbuffers(); */
	winset(mainwin);
	drawscenesub();
	swapbuffers();
	sginap(1);
}

void viewmove()
{
	float dx=0, dy=0, zoom;
	Device mdev[2];
	short mval[2],xo,yo,mode=0;

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	getdev(2, mdev, mval);
	xo=mval[0]; 
	yo=mval[1];

	while(getbutton(MIDDLEMOUSE)){
		getdev(2, mdev, mval);
	
		if(xo!=mval[0] || yo!=mval[1]) {
			phi-= (float)(yo-mval[1])/300.0;
			theta+= (float)(xo-mval[0])/300.0;
		}
		xo= mval[0];
		yo= mval[1];
		drawscene();
	}
}

