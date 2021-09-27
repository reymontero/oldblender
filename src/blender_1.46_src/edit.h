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

/* edit.h maart 95 */


/*
 * 
 * alles wat niet in de sdna hoeft
 * 
 */

#ifndef EDIT_H
#define EDIT_H

#define BEZSELECTED(bezt)   (((bezt)->f1 & 1) || ((bezt)->f2 & 1) || ((bezt)->f3 & 1))

#define TOB_IPO	1
#define TOB_IPODROT	2
#define TOB_IKA	4

typedef struct TransOb {
	float *loc;
	float oldloc[9];
	float *eff;
	float oldeff[3];
	float *rot;
	float oldrot[12];
	float olddrot[3];
	float *size;
	float oldsize[12];
	float olddsize[3];
	float obmat[3][3];
	float obinv[3][3];
	float parmat[3][3];
	float parinv[3][3];
	float obvec[3];
	int flag; /* keys */
	float *locx, *locy, *locz;
	float *rotx, *roty, *rotz;
	float *sizex, *sizey, *sizez;
	Object *ob;
} TransOb;

typedef struct TransVert {
	float *loc;
	float oldloc[3];
	float *val, oldval;
	int flag;
} TransVert;

typedef struct EditVert
{
	struct EditVert *next, *prev, *vn;
	float co[3];
	short xs, ys;
	char f, h, f1, hash;
} EditVert;

typedef struct EditEdge
{
	struct EditEdge *next, *prev;
	struct EditVert *v1, *v2, *vn;
	char f,h;
	short f1;
} EditEdge;

typedef struct EditVlak
{
	struct EditVlak *next, *prev;
	struct EditVert *v1, *v2, *v3, *v4;
	struct EditEdge *e1, *e2, *e3, *e4;
	float n[3];
	struct TFace *tface;
	char mat_nr, flag;
	char f, f1;
} EditVlak;

typedef struct CfraElem {
	struct CfraElem *next, *prev;
	float cfra;
	int sel;
} CfraElem;

typedef struct VPaint {
	float r, g, b, a;
	float size;		/* van brush */
	float gamma, mul;
	short mode, flag;
} VPaint;


	/* editmesh.c */
extern int vlakselectedOR(EditVlak *evl, int flag);
extern int vlakselectedAND(EditVlak *evl, int flag);
extern int comparevlak(EditVlak *vl1, EditVlak *vl2);
extern EditEdge *findedgelist(EditVert *v1, EditVert *v2);
extern EditEdge *addedgelist(EditVert *v1, EditVert *v2);
extern EditVlak *addvlaklist(EditVert *v1, EditVert *v2, EditVert *v3, EditVert *v4, int mat_nr, int flag, struct TFace *tface);

	/* scanfill.c */
extern EditVert *addfillvert(float *vec);
extern EditEdge *addfilledge(EditVert *v1, EditVert *v2);
extern ListBase fillvlakbase;
extern ListBase fillvertbase;

#endif /* EDIT_H */

