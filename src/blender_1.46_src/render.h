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



/* render.h
 *
 *
 * maart 95
 *
 */


#ifndef RENDER_H
#define RENDER_H

#define BRICON		Tin= (Tin-0.5)*tex->contrast+tex->bright-0.5; \
					if(Tin<0.0) Tin= 0.0; else if(Tin>1.0) Tin= 1.0;

#define BRICONRGB	Tr= tex->rfac*((Tr-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tr<0.0) Tr= 0.0; else if(Tr>1.0) Tr= 1.0; \
					Tg= tex->gfac*((Tg-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tg<0.0) Tg= 0.0; else if(Tg>1.0) Tg= 1.0; \
					Tb= tex->bfac*((Tb-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tb<0.0) Tb= 0.0; else if(Tb>1.0) Tb= 1.0;

#define MAXVERT (2<<18)
#define MAXVLAK (2<<18)


typedef struct Branch
{
	struct Branch *b[8];
} Branch;

typedef struct HaloRen
{	
	float alfa, xs, ys, rad, radsq, sin, cos, co[3];
	ulong zs, zd;
	short miny, maxy;
	char hard, b, g, r;
	char starpoints, add, type, tex;
	char linec, ringc, seed, flarec;
	float hasize;
	int pixels;
	
	Material *mat;
} HaloRen;

typedef struct LampRen
{
	float xs, ys, dist;
	float co[3];
	short type, mode;
	float r, g, b;
	float energy, haint;
	int lay;
	float spotsi,spotbl;
	float vec[3];
	float xsp, ysp, distkw, inpr;
	float halokw, halo;
	float ld1,ld2;
	struct ShadBuf *shb;
	float imat[3][3];
	float spottexfac;
	float sh_invcampos[3], sh_zfac;	/* sh_= spothalo */
	
	struct LampRen *org;
	MTex *mtex[8];
	
} LampRen;

typedef struct FaceStrip
{
	float n[3];
} FaceStrip;

typedef struct Node
{
	struct VlakRen *v[7];
	struct Node *next;
} Node;

typedef struct Osa
{
	float dxco[3], dyco[3];
	float dxlo[3], dylo[3], dxgl[3], dygl[3], dxuv[3], dyuv[3];
	float dxref[3], dyref[3], dxorn[3], dyorn[3];
	float dxno[3], dyno[3], dxview, dyview;
	float dxlv[3], dylv[3];
	float dxwin[3], dywin[3];
	float dxsticky[3], dysticky[3];
} Osa;

typedef struct Part
{
	struct Part *next, *prev;
	ulong *rect;
	short x, y;
} Part;

typedef struct PixStr
{
	struct PixStr *next;
	long vlak0, vlak;
	ulong z;
	ulong mask;
	short aantal, ronde;
} PixStr;

typedef struct PixStrMain
{
	struct PixStr *ps;
	struct PixStrMain *next;
} PixStrMain;
	
typedef struct QStrip
{
	struct QStrip *next, *prev;
	short pntsu, pntsv;
	short flagu, flagv;
	long firstface;
	Material *mat;
	struct VertStrip *verts;
	struct FaceStrip *faces;
} QStrip;

typedef struct ShadBuf {
	short samp,rt;
	float persmat[4][4];
	float winmat[4][4];
	float jit[25][2];
	float d,far,pixsize,soft;
	long co[3];
	long size,bias;
	ulong *zbuf;
	char *cbuf;
} ShadBuf;

typedef struct VertRen
{
	float co[3];
	float n[3];
	float ho[4];
	float *orco;
	float *sticky;
	short clip, texofs;		/* texofs= flag */
} VertRen;

typedef struct VertStrip
{
	float co[3];
	float n[3];
	float ho[4];
	short clip, flag;  
} VertStrip;

typedef struct VlakRen
{
	struct VertRen *v1, *v2, *v3, *v4;
	float n[3], len;
	Material *mat;
	MFace *mface;
	ulong *vcol;
	char snproj, puno;
	char flag, ec;
	ulong lay;
} VlakRen;



typedef struct Render
{
	float co[3];
	float lo[3], gl[3], uv[3], ref[3], orn[3], winco[3], sticky[3], vcol[3];
	float itot, i, ic, rgb, norm;
	float vn[3], view[3], *vno;

	float grvec[3], inprz, inprh;
	float imat[3][3];

	float viewmat[4][4], viewinv[4][4];
	float persmat[4][4], persinv[4][4];
	float winmat[4][4];
	
	short flag, osatex, osa, rt;
	short xstart, xend, ystart, yend, afmx, afmy, rectx, recty;
	float near, far, ycor, zcor, pixsize, viewfac;
	
	RenderData r;
	World wrld;
	ListBase parts;
	
	int totvlak, totvert, tothalo, totlamp;
	
	VlakRen *vlr;
	int vlaknr;
	
	Material *mat, *matren;
	LampRen **la;
	VlakRen **blovl;
	VertRen **blove;
	HaloRen **bloha;
	
	ulong *rectaccu, *rectz, *rectf1, *rectf2;
	ulong *rectot, *rectspare;
	int *rectdaps;
	
	short win, winpos, winx, winy, winxof, winyof;
	short winpop, displaymode, sparex, sparey;
	
	Image *backbuf, *frontbuf;
	
} Render;



extern Render R;
extern Osa O;


	/* initrender.c */
extern VlakRen *addvlak(int nr);
extern VertRen *addvert(int nr);
extern HaloRen *addhalo(int nr);

	/* render.c */
extern void shadepixel(float x, float y, int vlaknr);
extern void sky(char *col);
extern void shadehalo(HaloRen *har, char *col, ulong zz, float dist, float xn, float yn, short flarec);

#endif /* RENDER_H */

