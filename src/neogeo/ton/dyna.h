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
extern char *mallocN(long,char *);
extern char *callocN(long,char *);
extern short freeN(char *);

#define VECCOPY(v1,v2) 		{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2);}
#define QUATCOPY(v1,v2) 	{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2); *(v1+3)= *(v2+3);}

#define MAXBODIES 32

#define BLOCK 0

struct RigidBody {
	float mass, invmass;
	int type;
	void *data;
	float Ibody[3][3], Ibodyinv[3][3];
	float x[3], q[4], P[3], L[3];
	float Iinv[3][3], R[3][3];
	float v[3], omega[3];
	float force[3], torque[3];
};


struct Block {
	float minx, maxx;
	float miny, maxy;
	float minz, maxz;
};

struct Contact {
	struct RigidBody *a, *b;
	float p[3], n[3], ea[3], eb[3], time;
	int vf;
};

