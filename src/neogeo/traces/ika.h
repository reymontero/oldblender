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

	int rt[8];
};

struct BodyPoint {
	struct BodyPoint *next, *prev, *new;
	float r[3], v[3], n[3], hold[3];
	float m;
	short sx, sy;
	short f;			/* f==1: selected */
	char f1, h;			/* f1==1: locked */
	int rt[4];
};

struct Rod {
	struct Rod *next, *prev, *new;
	short flag, type;	/* flag is volgelvrij */
	struct BodyPoint *p1, *p2, *p3, *p4;
	float len, lenn, d1, d2, dia1, dia2, len1, len2, alpha;
	float up[3];
	float mat[3][3], quat[4];
	struct RodDeform *rdef;
	float drawwidth;
	int rt[3];
};

struct Joint {
	struct Joint *next, *prev;
	struct Rod *r1, *r2;
	struct BodyPoint *p1, *p2, *p3, *p1a, *p2a, *p3a;
	float amin, amax, afac;
	float umin, umax, ufac;
	int flag;		/* 1: beide ups corrigeren */
	float rt[4];
};

struct Skeleton {
	struct Skeleton *next, *prev;
	int nr;		/* op (skel+1) staan de rods */
	float fac;
	int rt[4];
};

struct IkaData {
	ListBase bpbase, rodbase, jointbase, skelbase;
	
	short dt, flag;
	short keyverts, keyrods;
	struct Key *key;
	struct Bezier *ipokey;
	
	int rt[16];
};

extern struct ListBase bpbase, rodbase, jointbase, skelbase;
extern float maxmom;

/* extern int orgx, orgy, sizex, sizey; */

/* extern struct Skeleton *skel; */

