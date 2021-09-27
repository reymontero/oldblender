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

extern float VecLenf(),  Normalise();

#define VECCOPY(v1,v2) 		{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2);}


#define ROD 0
#define T_ROD 1
#define H_ROD 2

struct RodDeform {
	/* original state */
	float orig1[3], orig2[3];
	float origmat[3][3], originv[3][3];
	float origmid[3];
	/* deformed */
	float v1[3], v2[3];
	float mat[3][3];
};

struct BodyPoint {
	struct BodyPoint *next, *prev;
	float r[3], v[3], n[3], hold[3];
	float m;
	short sx, sy;
	int s;			/* 1: grabbing */
};

struct Rod {
	struct Rod *next, *prev;
	int type;
	struct BodyPoint *p1, *p2, *p3, *p4;
	float len, lenn, d1, d2, dia1, dia2, len1, len2, alpha;
	float up[3];
	float mat[3][3], quat[4];
	struct RodDeform *rdef;
};

struct Joint {
	struct Joint *next, *prev;
	struct Rod *r1, *r2;
	struct BodyPoint *p1, *p2, *p3, *p1a, *p2a, *p3a;
	float amin, amax;
	float umin, umax;
	int flag;		/* 1: beide ups corrigeren */
};

struct Tube {
	struct Tube *next, *prev;
	float vec[3];
	int u, v;
	float *data;
};

struct Skeleton {
	struct Rod *r1, *r2, *r3, *r4;
};

extern struct ListBase bpbase, rodbase, jointbase, tubase;
extern float maxmom;

extern int mainwin, subwin, subwin2, NP, NR, NJ;
extern int orgx, orgy, sizex, sizey;

extern struct Skeleton *skel;

