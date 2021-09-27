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

#include <stdlib.h>

#include <gl/gl.h>
#include <gl/device.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <local/util.h>

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
	LCOLOR, 0.99, 0.99, 0.99,
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

#define SEGS 30
#define PTS 18

int num[SEGS];
int extralight= SEGS;

Matrix matone = {        /* identity matrix */
	1.0, 0.0, 0.0, 0.0,
	    0.0, 1.0, 0.0, 0.0,
	    0.0, 0.0, 1.0, 0.0,
	    0.0, 0.0, 0.0, 1.0
};

float v[8][3] = {
	{-100.2, -100.2, -100.0},
	{-100.0, -100.0,  100.0},
	{-100.0,  100.0,  100.0},
	{-100.2,  100.2, -100.0},
	{ 100.2, -100.2, -100.0},
	{ 100.0, -100.0,  100.0},
	{ 100.0,  100.0,  100.0},
	{ 100.2,  100.2, -100.0},
};

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



float tube[SEGS][PTS][3];
float norms[SEGS][PTS][3];

float Normalise(n)
float *n;
{
	register float d;

	d= n[0]*n[0]+n[1]*n[1]+n[2]*n[2];
	if(d>0.0) {
		d= fsqrt(d);

		n[0]/=d; 
		n[1]/=d; 
		n[2]/=d;
	} else {
		n[0]=n[1]=n[2]= 0.0;
	}
	return d;
}

float CalcNormFloat(v1,v2,v3,n)
float *v1,*v2,*v3;
float *n;
{
	float n1[3],n2[3];

	n1[0]= v1[0]-v2[0];
	n2[0]= v2[0]-v3[0];
	n1[1]= v1[1]-v2[1];
	n2[1]= v2[1]-v3[1];
	n1[2]= v1[2]-v2[2];
	n2[2]= v2[2]-v3[2];
	n[0]= n1[1]*n2[2]-n1[2]*n2[1];
	n[1]= n1[2]*n2[0]-n1[0]*n2[2];
	n[2]= n1[0]*n2[1]-n1[1]*n2[0];
	return Normalise(n);
}


void getmouse(adr)
short *adr;
{
	/* map muiscoordinaat invers naar geprojecteerde coordinaat */
	static Device mdev[2]= {MOUSEX, MOUSEY};
	long x, y;

	getdev(2, mdev, adr);

	/* coordinaat binnen window */
	getorigin(&x, &y);

	adr[0]-=x; 
	adr[1]-=y;

}



void extrude(int seg)
{
	float nor[3], dvec[3], fac, *v1, *v2, *v3, *v4, *n1, *n2, *n3, *n4;
	int i, segmin1;
	short mval[2];
	
	segmin1= seg-1;
	if(segmin1 < 0) segmin1= SEGS-1;
	/* normaal berekenen */
	CalcNormFloat(tube[segmin1][0], tube[segmin1][1], tube[segmin1][2], nor);
	
	/* perturben */
	dvec[0]= rand();
	dvec[1]= rand();
	dvec[2]= rand();
	Normalise(dvec);
	
	getmouse(mval);
	fac= ((float) mval[0])/800.0;
	/*fac= 0.0;*/
	
	nor[0]+= dvec[0]/2.0;
	nor[1]+= dvec[1]/2.0;
	nor[2]+= dvec[2]/(1.1+fac); 
	Normalise(nor);
	nor[0]*= 750.0;
	nor[1]*= 750.0;
	nor[2]*= 750.0;
	
	for(i=0; i<PTS; i++) {
		tube[seg][i][0]= tube[segmin1][i][0]+nor[0];
		tube[seg][i][1]= tube[segmin1][i][1]+nor[1];
		tube[seg][i][2]= tube[segmin1][i][2]+nor[2];
	}

	v2= tube[segmin1][PTS-1];
	v1= tube[segmin1][0];

	v4= tube[seg][PTS-1];
	v3= tube[seg][0];

	n2= norms[seg][PTS-1];
	n1= norms[seg][0];

	for(i=0; i<PTS; i++) {
		CalcNormFloat(v1, v2, v3, n2);
		v2= v1;
		v1+= 3;
		v4= v3; 
		v3+= 3;
		
		n2= n1;
		n1+=3;
	}
	
}

void initcircs()
{
	float rad, drad;
	int i;
	
	for(i=0; i<SEGS; i++) num[i]= i;

	drad= (2.0*M_PI)/PTS;
	rad= 0.0;
	for(i=0; i<PTS; i++) {
		tube[0][i][0]= 500.0*fsin(rad);
		tube[0][i][1]= 500.0*fcos(rad);
		tube[0][i][2]= 0.0;
		rad+= drad;
	}

	for(i=1; i<SEGS; i++) extrude(i);
}

void calccent(int seg, float *vec)
{
	int i;
	
	vec[0]= vec[1]= vec[2]= 0.0;
	for(i=0; i<PTS; i++) {
		vec[0]+= tube[seg][i][0];
		vec[1]+= tube[seg][i][1];
		vec[2]+= tube[seg][i][2];
	}
	vec[0]/= (float)PTS;
	vec[1]/= (float)PTS;
	vec[2]/= (float)PTS;
}

void randcol(float facx, float facy, float facz, int seg)
{	
	float depcue;

	depcue= 1.3-1.2*( (float)seg)/((float)SEGS);

	facx/= 3000;
	facy/= 3000;
	facz/= 3000;
	
	if(seg!=extralight) {
		tex[5]= depcue*fabs(fsin(facx));
		tex[6]= depcue*fabs(fsin(facy));
		tex[7]= depcue*fabs(fsin(facz));
	}
	else {
		tex[5]= depcue*2.0*fabs(fsin(facx));
		tex[6]= depcue*2.0*fabs(fsin(facy));
		tex[7]= depcue*2.0*fabs(fsin(facz));
	}
	lmdef(DEFMATERIAL, 1, 0, tex);
}

void drawtube()
{
	int seg=SEGS;
	int i, seg1, seg2;
	float *v1, *v2, *v3, *v4, norm[3];
	float *n1, *n2, *n3, *n4;

	cpack(0);
	clear();
	/* czclear(0x0, 0x7FFFFF); */
	
	if(extralight != SEGS) {
		extralight++;
	}
	
	/* van achter naar voren */
	while(seg>2) {
		seg--;
		
		seg1= num[seg];
		seg2= num[seg-1];
		
		v2= tube[seg1][PTS-1];
		v1= tube[seg1][0];

		v4= tube[seg2][PTS-1];
		v3= tube[seg2][0];

		n2= norms[seg1][PTS-1];
		n1= norms[seg1][0];

		n4= norms[seg2][PTS-1];
		n3= norms[seg2][0];
		
		randcol(v1[0], v1[1], v1[2], seg);
		
		for(i=0; i<PTS; i++) {
			/* CalcNormFloat(v1, v2, v3, norm); */
			bgnpolygon();
				n3f(n1);
				v3f(v1);
				n3f(n2);
				v3f(v2);
				n3f(n4);
				v3f(v4);
				n3f(n3);
				v3f(v3);
			endpolygon();
			v2= v1;
			v1+= 3;
			v4= v3; 
			v3+= 3;

			n2= n1;
			n1+= 3;
			n4= n3; 
			n3+= 3;
		}
		
		/* kubus */
		
		/* v1= tube[seg1][0]; */
		/* v2= tube[seg1][PTS/2]; */
		/* norm[0]= (v1[0]+v2[0])/2.0; */
		/* norm[1]= (v1[1]+v2[1])/2.0; */
		/* norm[2]= (v1[2]+v2[2])/2.0; */
		
		/* pushmatrix(); */
		/* cpack(0xFFFFFF); */
		/* translate(norm[0], norm[1], norm[2]); */
		/* drawcube(); */
		/* popmatrix(); */
		
	}
	swapbuffers();
}

setfouripo(float d, float *data)
{
	float d2, d3, fc;

	d2= d*d;
	d3= d2*d;

			/* B spline */
	data[0]= -0.1666*d3	+0.5*d2	-0.5*d	+0.16666;
	data[1]= 0.5*d3		-d2				+0.6666;
	data[2]= -0.5*d3		+0.5*d2	+0.5*d	+0.1666;
	data[3]= 0.1666*d3			;
}

void flerp(aantal,in,f0,f1,f2,f3,t)	/* float */
short aantal;
float *in,*f0,*f1,*f2,*f3;
float *t;
{
	short a;

	for(a=0;a<aantal;a++) {
		in[a]=t[0]*f0[a]+t[1]*f1[a]+t[2]*f2[a]+t[3]*f3[a];
	}
}


main(argc, argv)
int argc;
char **argv;
{
	float v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3], v8[3];
	float dv[3], dp[3];
	float ipo[4], twist=0.0;
	float v[3], p[3], look[3], at[3];
	float fac, dlab;
	long x, y;
	int totstep=4, a, i;
	short event, val;
	
	if(argc==2) totstep= atoi(argv[1]);
	
	noborder();
	winopen("tunnel");
	RGBmode();
	doublebuffer();
	gconfig();
	qdevice(ESCKEY);
	qdevice(SPACEKEY);
	
	getsize(&x, &y);
	fac= ((float)x)/((float)y);
	
	mmode(MVIEWING);
	perspective(400, fac, 100.0, 30000.0);
	shademodel(GOURAUD);
	loadmatrix(matone);

	/* zbuffer(1); */

	/* lRGBrange(0, 0, 0, 255, 255, 255, 0x500000, 0x7FFFFF); */
	/* depthcue(1); */
	
	deflighting();
	bindlighting();

	initcircs();
	
	calccent(num[0], look);
	calccent(num[6], at);

	/* animatie van seg 0 tot 1 */
	while(TRUE) {
		
		dlab= 1.0/((float)totstep);
		calccent(num[0], v1);
		calccent(num[1], v2);
		calccent(num[2], v3);
		calccent(num[3], v4);
		calccent(num[SEGS-5], v5);
		calccent(num[SEGS-4], v6);
		calccent(num[SEGS-3], v7);
		calccent(num[SEGS-2], v8);
		
		for(a=0; a<totstep; a++) {
			
			fac= ((float)a)/((float)totstep);
			setfouripo(fac, ipo);
			
			flerp(3, look, v1, v2, v3, v4, ipo);
			flerp(3, at, v5, v6, v7, v8, ipo);
			
			loadmatrix(matone);
			lookat(look[0], look[1], look[2], at[0], at[1], at[2], twist);
			twist+= 7.0;
			
			drawtube();
					
			if( qtest() ) {
				event= qread(&val);
				if(val) {
					if(event==ESCKEY) gexit();
					if(event==SPACEKEY) extralight= 2;
				}
			}

		}
		extrude(num[0]);
		
		for(i=0; i<SEGS-1; i++) num[i]= num[i+1];
		num[i]++;
		if(num[i] >= SEGS) num[i]= 0;
		
	}
	
	
}

