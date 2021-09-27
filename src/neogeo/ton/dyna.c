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

/* dyna.c 

 * 
 * okt 93
 * 
 * naar aanleiding van SIGGRAPH coursenotes 60
 */


#include "/usr/people/ton/dyna.h"

#define STATE_SIZE 13
#define THRESHOLD 1.0

int mainwin;
int orgx=250, orgy=300, sizex=800, sizey=600;
float *ystart, *yfinal, *ydot, *ytemp;

int nbodies=0, ncontacts=0, patchvar=0;

struct RigidBody Bodies[MAXBODIES];
struct Contact Contacts[MAXBODIES];

/* ************ INIT VIEW AND DRAW ************ */

float dproj=1.0, phi=0.959, theta= -0.770, viewmat[4][4], winmat[4][4], persmat[4][4], persinv[4][4];
float mat1[4][4]= {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

float v[8][3] = {
	{-1.0, -1.0, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.0,  1.0, -1.0},
	{ 1.0, -1.0, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.0,  1.0, -1.0},
};

float tex[]= {
	AMBIENT, .1, .1, .1,
	DIFFUSE, .8, .9, 1.0,
	SPECULAR, 0.3, 0.3, 0.3,
	SHININESS, 0.0,
	LMNULL,
};
float lm[]= {
	AMBIENT, 0.0, 0.0, 0.0,
	LOCALVIEWER, 1.0,
	TWOSIDE,  0.0,
	ATTENUATION, 1.0, 0.0, 
	ATTENUATION2, 0.0, 
	LMNULL,
};
float lt[]= {
	LCOLOR, 1.0, 1.0, 1.0,
	POSITION, -2000.0, -1000.0, 1000.0, 1.00,   /* w==0.0: infinity */
	LMNULL,
};
float blue_light[] =
    {LCOLOR,    0.3,0.3,0.9,   /* blue light */
     POSITION,  2000.0, 1000.0, 0.0, 1.0,  /* w==0.0: infinity */
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

/* ****************** VIEW DYNA ********************* */

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


void DisplayBodies()
{
	struct RigidBody *rb, *rrb=0;
	struct Block *bl;
	float vec[3], mat4[4][4], x, y, z;
	int c, a, drawtype= 1;
	
	mmode(MVIEWING);
	
	loadmatrix(mat1);

	polarview(dproj*1400.0, (1800/M_PI)*theta, (1800/M_PI)*phi, 0.0);

	translate(0.0, 0.0, -50.0);
	
	getmatrix(viewmat);
	perspective(300, (float)sizex/(float)sizey, 5.0, 5000.0);
	mmode(MPROJECTION);
	getmatrix(winmat);
	Mat4MulMat4(persmat, viewmat, winmat);
	Mat4Inv(persinv, persmat);
	mmode(MVIEWING);

	if(drawtype==2) {
		shademodel(GOURAUD);
		czclear(0x707070,0x7FFFFF);
		zbuffer(1);
		bindlighting();
	}
	else {
		cpack(0x707070);
		clear();
		zbuffer(0);
	}

	tekengrond();
	
	for(a=0; a<nbodies; a++) {
		rb= Bodies+a;
		
		pushmatrix();
		
		Mat4One(mat4);
		Mat4CpyMat3(mat4, rb->R);

		if(rb->type==BLOCK) {
			bl= rb->data;
			x= bl->maxx;
			y= bl->maxy;
			z= bl->maxz;

			mat4[0][0]*=x; mat4[0][1]*=x; mat4[0][2]*=x;
			mat4[1][0]*=y; mat4[1][1]*=y; mat4[1][2]*=y;
			mat4[2][0]*=z; mat4[2][1]*=z; mat4[2][2]*=z;
			
			mat4[3][0]= rb->x[0];
			mat4[3][1]= rb->x[1];
			mat4[3][2]= rb->x[2];

			multmatrix(mat4);

			if(drawtype==1) drawcube();
			else drawgourcube();
		}
		popmatrix();
		
	}
	
	swapbuffers();

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
		DisplayBodies();
	}
}


/* ***************** DYNAMIC ARITH *********** */

void State_to_Array(struct RigidBody *rb, float *y)
{
	
	VECCOPY(y, rb->x);
	y+= 3;
	QUATCOPY(y, rb->q);
	y+= 4;
	VECCOPY(y, rb->P);
	y+= 3;
	VECCOPY(y, rb->L);
	y+= 3;
	
}

void Array_to_State(struct RigidBody *rb, float *y)
{
	float transp[3][3];
	
	VECCOPY(rb->x, y);
	y+= 3;
	QUATCOPY(rb->q, y);
	NormalQuat(rb->q);
	y+= 4;
	VECCOPY(rb->P, y);
	y+= 3;
	VECCOPY(rb->L, y);
	y+= 3;
	
	rb->v[0]= rb->P[0]*rb->invmass;
	rb->v[1]= rb->P[1]*rb->invmass;
	rb->v[2]= rb->P[2]*rb->invmass;
	
	QuatToMat3(rb->q, rb->R);
	
	Mat3CpyMat3(transp, rb->R);
	Mat3Transp(transp);

	/* deze volgorde is omgekeerd als die uit de coursenotes */
	Mat3MulSerie(rb->Iinv, transp, rb->Ibodyinv, rb->R, 0);

	VECCOPY(rb->omega, rb->L);
	Mat3MulVecfl(rb->Iinv, rb->omega);
}

void Array_to_Bodies(float *y)
{
	int i;
	
	for(i=0; i<nbodies; i++)
		Array_to_State(Bodies+i, y+i*STATE_SIZE);
}

void Bodies_to_Array(float *y)
{
	int i;
	
	for(i=0; i<nbodies; i++)
		State_to_Array(Bodies+i, y+i*STATE_SIZE);
}


void Compute_Force_and_Torque(float t, struct RigidBody *rb)
{
	/* gravity, collisions, wind, etc */
	/* store in rb->force and rb->torque */
	
	/* gravity */
	rb->force[0]= rb->force[1]= rb->force[2]= 0.0;
	rb->torque[0]= rb->torque[1]= rb->torque[2]= 0.0;
	
	rb->force[2]= -30.0*rb->mass;
}

void simulatestep(t, dt)
float t, dt;
{
	struct RigidBody *rb;
	float qdot[4], q[4];
	float qomega[4];
	float mat[3][3], *fp;
	int i, len;
	
	rb= Bodies;
	fp= ydot;
	for(i=0; i<nbodies; i++, rb++) {
		Compute_Force_and_Torque(t, rb);

		/* bereken ydot */
		VECCOPY(fp, rb->v);
		fp+= 3;
	
		qomega[0]= 0;
		VECCOPY(qomega+1, rb->omega);
		QUATCOPY(fp, qomega);
		QuatMul(fp, qomega, rb->q);
		
		fp[0]*= 0.5;
		fp[1]*= 0.5;
		fp[2]*= 0.5;
		fp[3]*= 0.5;
		fp+= 4;
		
		VECCOPY(fp, rb->force);
		fp+= 3;
		VECCOPY(fp, rb->torque);
		fp+= 3;

	}
	
	len= STATE_SIZE*nbodies;
	for(i=0; i<len; i++) {
		yfinal[i]+= dt*ydot[i];
	}

}

/* ***************** COLLISION *********** */

void pt_velocity(float *ans, struct RigidBody *body, float *p)
{
	float vec[3];
	
	VecSubf(vec, p, body->x);
	Crossf(ans, body->omega, vec);
	VecAddf(ans, body->v, ans);
}

int colliding(struct Contact *c)
{
	float padot[3], pbdot[3], vrel;
	
	pt_velocity(padot, c->a, c->p);
	pt_velocity(pbdot, c->b, c->p);
	
	VecSubf(padot, padot, pbdot);
	vrel= c->n[0]*padot[0]+c->n[1]*padot[1]+c->n[2]*padot[2];
	
	if(vrel>THRESHOLD) return 0;	/* moving away */
	if(vrel>-THRESHOLD) return 0;	/* resting contact */
	return 1;
}

void collision(struct Contact *c, float epsilon)
{
	float padot[3], pbdot[3], n[3], ra[3], rb[3], vrel, numerator;
	float term1, term2, term3, term4;
	float j, force[3], cross[3], temp[3];
	
	pt_velocity(padot, c->a, c->p);
	pt_velocity(pbdot, c->b, c->p);
	VECCOPY(n, c->n);
	VecSubf(ra, c->p, c->a->x);
	VecSubf(rb, c->p, c->b->x);
	
	VecSubf(padot, padot, pbdot);
	vrel= n[0]*padot[0]+n[1]*padot[1]+n[2]*padot[2];
	numerator= -(1.0+epsilon)*vrel;
	
	term1= c->a->invmass;
	term2= c->b->invmass;
	
	Crossf(cross, ra, n);
	Mat3MulVecfl(c->a->Iinv, cross);
	Crossf(temp, cross, ra);
	term3= n[0]*temp[0]+ n[1]*temp[1]+ n[2]*temp[2];

	Crossf(cross, rb, n);
	Mat3MulVecfl(c->b->Iinv, cross);
	Crossf(temp, cross, rb);
	term4= n[0]*temp[0]+ n[1]*temp[1]+ n[2]*temp[2];
	
	j= numerator/(term1+term2+term3+term4);
	force[0]= j*n[0];
	force[1]= j*n[1];
	force[2]= j*n[2];

	/* apply impulse to bodies */
	VecAddf(c->a->P, c->a->P, force);
	VecSubf(c->b->P, c->a->P, force);
	
	Crossf(cross, ra, force);
	VecAddf(c->a->L, c->a->L, cross);
	Crossf(cross, rb, force);
	VecSubf(c->b->L, c->b->L, cross);
	
	/* recompute */
	VECCOPY(c->a->v, c->a->P);
	VecMulf(c->a->v, c->a->invmass);
	VECCOPY(c->b->v, c->b->P);
	VecMulf(c->b->v, c->b->invmass);

	VECCOPY(c->a->omega, c->a->L);
	Mat3MulVecfl(c->a->Iinv, c->a->omega);
	VECCOPY(c->b->omega, c->b->L);
	Mat3MulVecfl(c->b->Iinv, c->b->omega);
}

void find_collisiontime(struct Contact *ct, float time, float dt)
{
	

	/* kopieer start-state naar temp */
	memcpy(ytemp, ystart, STATE_SIZE*nbodies*4);
	
	while(dt>0.001) {
	
		dt/= 2.0;
	}

	/* herstel start-state */
	memcpy(ytemp, ystart, STATE_SIZE*nbodies*4);
}

void findcontacts()
{
	struct RigidBody *rb;
	struct Block *bl;
	struct Contact *ct;
	float vec[3];
	int a, b, c;

	ncontacts= 0;
	
	/* alleen voor bodies met grond (body 0) berekenen */
	rb= Bodies;
	
	if(rb->type==BLOCK) {
		bl= rb->data;

		for(a= -1; a<2; a+= 2) {
			for(b= -1; b<2; b+= 2) {
				for(c= -1; c<2; c+= 2) {
					vec[0]= a*bl->maxx;
					vec[1]= b*bl->maxy;
					vec[2]= c*bl->maxz;
					Mat3MulVecfl(rb->R, vec);
					VecAddf(vec, vec, rb->x);
					
					if(vec[2]<0.0) {
						ct= &Contacts[ncontacts++];
						ct->a= rb;
						ct->b= rb+1;
						VECCOPY(ct->p, vec);
						ct->n[0]= ct->n[1]= 0.0;
						ct->n[2]= 1.0;
					}
				}
			}
		}
	}
	
}

void testcollisions(float time, float dt)
{
	struct RigidBody *rb;
	struct Block *bl;
	struct Contact *ct;
	float vec[3];
	int a, b, c;
	
	findcontacts();
	if(ncontacts==0) return;
	
	/* testen welke contact het eerste was */
	ct= Contacts;
	for(a=0; a<ncontacts; a++, ct++) {
		if( colliding(ct) ) {
			/* states terug naar t0 */
			Array_to_Bodies(ystart);
			
			find_collisiontime(ct, time, dt);
		}
	}
	
	ct= Contacts;
	for(a=0; a<ncontacts; a++, ct++) {
		if( colliding(ct) ) {
			/* terug in tijd */
			/* memcpy(y0, yfinal, STATE_SIZE*nbodies*4); */
			/* ode(y0, yfinal, STATE_SIZE*nbodies, time, time-dt); */
			/* Array_to_Bodies(yfinal); */
			
			collision(ct, 0.6);
			
			/* Bodies_to_Array(yfinal); */
		}
	}
}

/* ************************************************ */


void simulatemain(float t, float dt)
{

	Bodies_to_Array(yfinal);
	memcpy(ystart, yfinal, STATE_SIZE*nbodies*4);

	simulatestep(t, dt);
	
	Array_to_Bodies(yfinal);
	
	/* test op botsingen, afhandelen */
	testcollisions(t, dt);
}			


struct RigidBody *addblock(float xs, float ys, float zs, float x, float y, float z, float mass)
{
	float temp[STATE_SIZE];
	struct RigidBody *rb;
	struct Block *bl;
	
	rb= Bodies+nbodies;
	nbodies++;
	
	/* location */
	rb->x[0]= x;
	rb->x[1]= y;
	rb->x[2]= z;

	/* speed (v= P/m) */
	rb->P[0]= 0.0;
	rb->P[1]= 0.0;
	rb->P[2]= 0.0;

	/* rot */
	rb->q[0]= 1.0;
	rb->q[1]= 0.0;
	rb->q[2]= 0.0;
	rb->q[3]= 0.0;

	/* angular speed */
	rb->L[0]= 0.0;
	rb->L[1]= 0.0;
	rb->L[2]= 0.0;
	
	rb->mass= mass;
	if(mass==0.0) rb->invmass= 0.0;
	else rb->invmass= 1.0/mass;
	
	/* vertexdata */
	bl= mallocN(sizeof(struct Block), "addblock");
	rb->type= BLOCK;
	rb->data= bl;
	
	bl->minx= -0.5*xs;
	bl->maxx= 0.5*xs;
	bl->miny= -0.5*ys;
	bl->maxy= 0.5*ys;
	bl->minz= -0.5*zs;
	bl->maxz= 0.5*zs;
	
	/* inertia tensor */
	Mat3One(rb->Ibody);
	rb->Ibody[0][0]= rb->mass*(ys*ys+zs*zs)/12.0;
	rb->Ibody[1][1]= rb->mass*(xs*xs+zs*zs)/12.0;
	rb->Ibody[2][2]= rb->mass*(xs*xs+ys*ys)/12.0;

	if(mass==0.0) Mat3Clr(rb->Ibodyinv);
	else Mat3Inv(rb->Ibodyinv, rb->Ibody);
	
	/* initialize Derived quantities */
	State_to_Array(rb, temp);
	Array_to_State(rb, temp);
	
	return rb;
}

void freeAll()
{
	
}

void initbodies()
{
	struct RigidBody *rb;
	
	nbodies= 0;

	rb= addblock(100.0, 100.0, 200.0, 0, 0, 240, 1.0);	
	rb->L[0]= 2000.0;
	/* rb->L[1]= 1000.0; */
	rb->L[2]= 0.0;

	/* grond */
	addblock(1000.0, 1000.0, 2.0, 0, 0, 0, 0.0);
	
	
}

void main()
{
	struct RigidBody *rb;
	float t=0.0, dt=1.0/30.0;
	short loop=1, runsim=0, event, val, mval[2], grabbing=0, mstart[2];
	Device mdev[2];
	
	prefposition(orgx, orgx+sizex-1, orgy, orgy+sizey);
	mainwin= winopen("IKA");
	RGBmode();
	doublebuffer();
	gconfig();
	sginap(3);
	cpack(0);
	clear();
	
	qdevice(RAWKEYBD);
	qdevice(ESCKEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
	qdevice(SPACEKEY);
	mdev[0]= MOUSEX;
	mdev[1]= MOUSEY;

	mmode(MVIEWING);
	deflighting();
	bindlighting();
	
	initbodies();

	ystart= mallocN(STATE_SIZE*nbodies*4, "RunSim1");
	yfinal= mallocN(STATE_SIZE*nbodies*4, "RunSim2");
	ydot= mallocN(STATE_SIZE*nbodies*4, "RunSim3");
	ytemp= mallocN(STATE_SIZE*nbodies*4, "RunSim3");

	DisplayBodies();
	
	while(loop) {
		
		if(qtest()) {
			event= qread(&val);
			if(val) {
				switch(event) {
				case MIDDLEMOUSE:
					viewmove();
					break;
				case RIGHTMOUSE:
					break;
				case PADPLUSKEY:
					dproj/= 1.1;
					DisplayBodies();
					break;
				case SPACEKEY:
					runsim= ~runsim;
					break;
				case PADMINUS:
					dproj*= 1.1;
					DisplayBodies();
					break;
				case RKEY:
					initbodies();
					runsim= 0;
					DisplayBodies();
					break;
				case ESCKEY:
					gexit();
					freeAll();
					exit(0);
				}
			}
		}
		
		if(runsim) {
			
			simulatemain(t, dt);
			
			DisplayBodies();
			
			t+= dt;
		}
	}
}

