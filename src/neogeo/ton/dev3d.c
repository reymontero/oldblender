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


/*
 * 
 *   dev3d.c  jan 96
 * 
 * 

cc dev3d.c /usr/people/trace/arith.o /usr/people/trace/util.o -lgl_s -lc_s -lfm_s -lm -o dev3d

 * 
 * 
 * 
 * 
 */


#include <stdlib.h>
#include <gl/mygl.h>
#include <gl/device.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <local/util.h>

#include "/usr/people/blend/blender.h"


extern float VecLenf(),  Normalise();

#define VECCOPY(v1,v2) 		{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2);}



int mainwin, orgx=250, orgy=300, sizex=800, sizey=600;

float zoom=1.0, phi=0.959, theta= -0.770, viewmat[4][4], winmat[4][4], persmat[4][4], persinv[4][4];

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

/* ****************** VOORBEELD ********************* */

typedef struct Face1 {
	short v1, v2, v3, rt;
	ulong col;
} Face1;

#define OB_CUBE		0
#define OB_VIDSC	1

typedef struct Live {
	struct Live *next, *prev;
	int type;
	float sizex, sizey, sizez;
	short xwin, ywin;
	float loc[3];
	float dloc[3];
	float rot[3];
	float drot[3];
	
	int totvert, totface;
	
	Mesh *me;
	
} Live;


ListBase cubase= {0, 0}; 

/* ****************** VIEW ********************* */

void getmouseco(mval)
short *mval;
{
	static Device mdev[2]= {MOUSEX, MOUSEY};

	getdev(2, mdev, mval);
	mval[0]-= orgx;
	mval[1]-= orgy;
	
}

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


void draw_meshgour(Mesh *me)
{
	MVert *v1, *v2, *v3, *v4;
	MFace *mface;
	ulong *mcol;
	float nor[3];
	int a;
	
	mface= me->mface;
	mcol= (ulong *)me->mcol;
	
	if(mcol==0) {
		
		return;
	}
	
	a= me->totface;
	while(a--) {
		v1= me->mvert+mface->v1;
		v2= me->mvert+mface->v2;
		v3= me->mvert+mface->v3;
		v4= me->mvert+mface->v4;

		if(mface->v4) {
			bgnpolygon();
				cpack( *(mcol) );
				v3f(v1->co);
				cpack( *(mcol+1) );
				v3f(v2->co);
				cpack( *(mcol+2) );
				v3f(v3->co);
				cpack( *(mcol+3) );
				v3f(v4->co);
			endpolygon();
			mcol+= 4;
			
		}
		else {
			bgnpolygon();
				cpack( *(mcol) );
				v3f(v1->co);
				cpack( *(mcol+1) );
				v3f(v2->co);
				cpack( *(mcol+2) );
				v3f(v3->co);
			endpolygon();
			mcol+= 3;
		}
		mface++;
		
	}
}

typedef struct Vertex {
	long co[3];
	ulong rt;
} Vertex;

typedef struct Face {
	short v1, v2, v3, v4;
	short nor[3];
	short col[3];
} Face;



void read_draw_exotic()
{
	Vertex *ve, *vebase;
	Face *fa, *fabase;
	float vec[3];
	int file, filelen;
	int a, totvert, totface;
	
	file= open("/render/wipe_out/track01/track.trv", O_RDONLY);
	if(file<=0) {
		printf("Can't open/find file\n");
		return;
	}
	filelen= lseek(file, 0, 2);	/* seek end */
	lseek(file, 0, 0);		/* en weer terug */
	
	vebase= mallocN( filelen, "vertex");
	totvert= filelen/sizeof(Vertex);

	read(file, vebase, filelen);
	close(file);
		
	/* verts tekenen */
	a= totvert;
	ve= vebase;
	while(a--) {
		vec[0]= 0.01*ve->co[0];
		vec[1]= 0.01*ve->co[1];
		vec[2]= 0.01*ve->co[2];

		ve->co[0]= vec[0];
		ve->co[2]= -vec[1];
		ve->co[1]= vec[2];

		ve++;
	}
	

	/* faces */
	file= open("/render/wipe_out/track01/track.trf", O_RDONLY);
	if(file) {
		
		
		filelen= lseek(file, 0, 2);	/* seek end */
		lseek(file, 0, 0);		/* en weer terug */
	
		fabase= mallocN( filelen, "vertex");
		totface= filelen/sizeof(Face);

		read(file, fabase, filelen);
		close(file);
		
		a= totface;
		fa= fabase;
		cpack(0x0);
		while(a--) {
			
			if(fa->v1<totvert && fa->v2<totvert && fa->v3<totvert) {
				
				vec[0]= fa->nor[0];
				vec[1]= fa->nor[1];
				vec[2]= fa->nor[2];
				Normalise(vec);
				
				bgnpolygon();
				n3f(vec);
				
				ve= vebase+fa->v1; v3i(ve->co);
				ve= vebase+fa->v2; v3i(ve->co);
				ve= vebase+fa->v3; v3i(ve->co);
				ve= vebase+fa->v4; v3i(ve->co);
				endpolygon();
			}
			/* if(a<30) printf("%d %d %d\n", fa->v1, fa->v2, fa->v3); */
			
			fa++;
		}
		
	}
	
	/* verts tekenen */
	a= totvert;
	ve= vebase;
	cpack(0xFFFFFF);
	bgnpoint();
	while(a--) {
		v3i(ve->co);

		ve++;
	}
	endpoint();
	
	
	freeN(vebase);
}

void drawscene()
{
	Live *cu;
	float vec[3], mat4[4][4], x, y, z, *data;
	int ofs, nr, parts, a, drawtype= 2;
	
	mmode(MVIEWING);
	
	loadmatrix(mat1);
	polarview(zoom*1400.0, (1800/M_PI)*theta, (1800/M_PI)*phi, 0.0);
	translate(0.0, 0.0, -50.0);

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

	/* read_draw_exotic(); */
	

	/* teken en bereken cubes */
	cu= cubase.first;
	while(cu) {
		

		cpack(0);
		pushmatrix();
		
		EulToMat4(cu->rot, mat4);

		mat4[0][0]*=cu->sizex; mat4[0][1]*=cu->sizex; mat4[0][2]*=cu->sizex;
		mat4[1][0]*=cu->sizex; mat4[1][1]*=cu->sizex; mat4[1][2]*=cu->sizex;
		mat4[2][0]*=cu->sizez; mat4[2][1]*=cu->sizez; mat4[2][2]*=cu->sizez;
		
		mat4[3][0]= cu->loc[0];
		mat4[3][1]= cu->loc[1];
		mat4[3][2]= cu->loc[2];
		
		multmatrix(mat4);
		
		draw_meshgour(cu->me);
		
		popmatrix();
		
		cu= cu->next;
	}
	
	/* teken verts en berekenschermco's */
	persp(0);
	cpack(0xFF00FF);
	cu= cubase.first;
	while(cu) {
		berekenschermcofl(cu->loc, &(cu->xwin));
		if(cu->xwin!=3200) {
			sboxfs(cu->xwin-1, cu->ywin-1, cu->xwin+1, cu->ywin+1 );
		}
		cu= cu->next;
	}
	
	persp(1);

	swapbuffers();
}

Live *addLive()
{
	Live *cu;
	
	cu= callocN(sizeof(Live), "Live");
	addtail(&cubase, cu);
	
	cu->dloc[0]= 0.0002*(rand()-16384);
	cu->dloc[1]= 0.0002*(rand()-16384);
	cu->dloc[2]= 0.0002*(rand()-16384);
	
	cu->drot[0]= 0.00001*(rand()-16384);
	cu->drot[1]= 0.00001*(rand()-16384);
	cu->drot[2]= 0.00001*(rand()-16384);
	
	cu->sizex= 30.0;
	cu->sizey= 30.0;
	cu->sizez= 30.0;
	
	return cu;
}

void update_speed()
{
	Live *cu;
	
	cu= cubase.first;
	
	while(cu) {
		
		/* cu->loc[0]+= cu->dloc[0]; */
		/* cu->loc[1]+= cu->dloc[1]; */
		/* cu->loc[2]+= cu->dloc[2]; */

		cu->rot[0]+= cu->drot[0];
		cu->rot[1]+= cu->drot[1];
		cu->rot[2]+= cu->drot[2];
		
		cu= cu->next;
	}
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

void freeAll()
{
	Live *cu;
	
	while(cubase.first) {
		cu= cubase.first;
		remlink(&cubase, cu);
		freeN(cu);
	}
}


main()
{
	Mesh *me;
	Live *cu;
	short loop=1, event, val, mval[2], grabbing=0, mstart[2];
	Device mdev[2];
	
	prefposition(orgx, orgx+sizex-1, orgy, orgy+sizey);
	mainwin= winopen("dev3d");
	RGBmode();
	doublebuffer();
	gconfig();
	sginap(3);
	cpack(0);
	clear();

	winset(mainwin);

	qdevice(RAWKEYBD);
	qdevice(ESCKEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
	qdevice(SPACEKEY);
	mdev[0]= MOUSEX;
	mdev[1]= MOUSEY;

	/* init vars */
	
	cu= addLive();
	
	
	open_blender("/data/heel_mooi.blend");
	cu->me= (Mesh *)append_part(ID_ME, "koeie");
	
	winset(mainwin);
	mmode(MVIEWING);
	deflighting();

	bindlighting();

	drawscene();
	
	
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
				case SPACEKEY:
					break;
				case PADPLUSKEY:
					zoom/= 1.1;
					break;
				case PADMINUS:
					zoom*= 1.1;
					break;
				case ESCKEY:
					gexit();
					freeAll();
					exit(0);
				}
			}

		}
		else sginap(1);
		
	}
}

