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
 *	Kurt Akeley
 *	December 1992
 *
 *	Use stencil to draw hidden-line and outlined images
 *	Depress the middle mouse button to remove hidden lines
 *	    (by filling the cube's faces with background color).
 *	Depress the left mouse button to outline the shaded polygons
 *	    (by filling the cube's faces with shaded color).
 *	Depress the escape key to exit.
 *
 *	Notes:
 *
 *	    1.	All lines are rendered single width
 *
 *	    2.	Exact regeneration of z values is not required, so
 *		there are no dropouts.
 *
 *	    3.	Using the INVERT stencil operation allows the stencil
 *		bits to be set and cleared using the same stencil mode.
 *		As a result, only two stencil mode changes are required
 *		per quadrilateral, rather than the three that would be
 *		required otherwise.  This improves the performance of
 *		the hidden line loop substantially.
 */

#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h>

#define MAXQUAD 6

#define LINECOLOR 0xffffffff
#define BACKCOLOR 0

typedef float Vertex[4];
typedef Vertex Quad[5];

/* data to define the six faces of a cube */
Quad cubequads[MAXQUAD] = {
	0,0,0,1, .8,0,0,1, .8,.8,0,1, 0,.8,0,1,		0,0,-1,0,
	    0,0,.8,1, .8,0,.8,1, .8,.8,.8,1, 0,.8,.8,1,		0,0,1,0,
	    0,0,0,1, .8,0,0,1, .8,0,.8,1, 0,0,.8,1,		0,-1,0,0,
	    0,.8,0,1, .8,.8,0,1, .8,.8,.8,1, 0,.8,.8,1,		0,1,0,0,
	    0,0,0,1, 0,0,.8,1, 0,.8,.8,1, 0,.8,0,1,		-1,0,0,0,
	    .8,0,0,1, .8,0,.8,1, .8,.8,.8,1, .8,.8,0,1,		1,0,0,0
};

/* global variables */
Quad* quads;
float nullarray[1] = {
	LMNULL};
int dims = 2;

/* routine declarations */
void addquad(int index, float x, float y, float z, Quad quad);
void fill(Quad quad);
void outline(Quad quad);

main(int argc, char** argv) {
	int i;
	int leftmouse;
	int middlemouse;
	short dev,val;
	int x,y,z;
	int index;
	int totalquads;

	/* check for required capabilities */
	if (getgdesc(GD_BITS_STENCIL) < 1) {
		fprintf(stderr,"stencil capability required and not present.  abort\n");
		exit(1);
	}

	/* read arguments */
	if (argc > 1)
		dims = atoi(argv[1]);
	printf("dimensions = %d\n", dims);

	/* initialize graphics */
	winopen("hidden");
	RGBmode();
	doublebuffer();
	stensize(1);
	gconfig();
	subpixel(TRUE);
	mmode(MVIEWING);
	perspective(900,1.0,0.5,5.0);
	translate(0.0,0.0,-2.2);
	lsetdepth(getgdesc(GD_ZMAX),0);	/* I like clearing z to zero */
	zfunction(ZF_GEQUAL);
	zbuffer(TRUE);
	qdevice(ESCKEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	swritemask(1);
	sclear(0);
	lmdef(DEFLMODEL,1,0,nullarray);
	lmdef(DEFLIGHT,1,0,nullarray);
	lmdef(DEFMATERIAL,1,0,nullarray);
	lmbind(LMODEL,1);
	lmbind(LIGHT0,1);

	/* create data */
	totalquads = dims * dims * dims * MAXQUAD;
	quads = (Quad*)malloc(totalquads * sizeof(Quad));
	index = 0;
	for (z=0; z < dims; z++) {
		for (y=0; y < dims; y++) {
			for (x=0; x < dims; x++) {
				for (i=0; i < MAXQUAD; i++) {
					addquad(index++,x,y,z,cubequads[i]);
				}
			}
		}
	}

	/* loop drawing the cubes */
	while (1) {

		/* handle events */
		while (qtest()) {
			dev = qread(&val);
			switch(dev) {
			case REDRAW:
				reshapeviewport();
				sclear(0);
				break;
			case ESCKEY:
				exit(0);
			case LEFTMOUSE:
				leftmouse = val;
				break;
			case MIDDLEMOUSE:
				middlemouse = val;
				break;
			}
		}

		/* track the mouse */
		pushmatrix();
		rot(0.5 * getvaluator(MOUSEX),'y');
		rot(0.5 * getvaluator(MOUSEY),'z');

		/* clear the framebuffer and initialize the matrix */
		czclear(BACKCOLOR,0);
		scale(1.5,1.5,1.5);
		translate(-0.5,-0.5,-0.5);
		scale(1.0/dims,1.0/dims,1.0/dims);

		if (leftmouse) {
			/* render outlined, shaded quadrilaterals */
			cpack(0xff0000ff);
			stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
			for (i=0; i<totalquads; i++) {
				outline(quads[i]);
				lmbind(MATERIAL,1);
				n3f(quads[i][4]);
				stencil(TRUE,0,SF_EQUAL,1,ST_KEEP,ST_KEEP,ST_KEEP);
				fill(quads[i]);
				lmbind(MATERIAL,0);
				cpack(0xff0000ff);
				stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
				outline(quads[i]);
			}
			stencil(FALSE,0,0,0,0,0,0);
		} else if (middlemouse) {
			/* render visible lines only */
			cpack(LINECOLOR);
			stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
			for (i=0; i<totalquads; i++) {
				outline(quads[i]);
				cpack(BACKCOLOR);
				stencil(TRUE,0,SF_EQUAL,1,ST_KEEP,ST_KEEP,ST_KEEP);
				fill(quads[i]);
				cpack(LINECOLOR);
				stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
				outline(quads[i]);
			}
			stencil(FALSE,0,0,0,0,0,0);
		}
		else {
			/* render all outlines */
			cpack(LINECOLOR);
			for (i=0; i<totalquads; i++)
				outline(quads[i]);
		}
		popmatrix();

		swapbuffers();
	}
}

void addquad(int index, float x, float y, float z, Quad quad) {
	/* add a quad to the list of quads */
	int i,j;
	/* copy vertexes with offset */
	for (i=0; i < 4; i++) {
		quads[index][i][0] = x + quad[i][0];
		quads[index][i][1] = y + quad[i][1];
		quads[index][i][2] = z + quad[i][2];
		quads[index][i][3] = 1.0;
	}
	/* copy normal with no offset */
	quads[index][4][0] = quad[4][0];
	quads[index][4][1] = quad[4][1];
	quads[index][4][2] = quad[4][2];
	quads[index][4][3] = 0;
}

void fill(Quad quad) {
	/* fill the polygon */
	bgnpolygon();
	v3f(quad[0]);
	v3f(quad[1]);
	v3f(quad[2]);
	v3f(quad[3]);
	endpolygon();
}

void outline(Quad quad) {
	/* outline the polygon */
	bgnclosedline();
	v3f(quad[0]);
	v3f(quad[1]);
	v3f(quad[2]);
	v3f(quad[3]);
	endclosedline();
}

