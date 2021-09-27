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

/* blender.h    feb 98

 * 
 * 
 */

#ifndef BLENDER_H
#define BLENDER_H


#include <fcntl.h>
#include <local/util.h>
#include <local/iff.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/schedctl.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include "blendef.h"

/* *********** vec ************* */

typedef struct vec2s {
	short x, y;
} vec2s;

typedef struct vec2i {
	int x, y;
} vec2i;

typedef struct vec2f {
	float x, y;
} vec2f;

typedef struct vec2d {
	double x, y;
} vec2d;

typedef struct vec3i {
	int x, y, z;
} vec3i;

typedef struct vec3f {
	float x, y, z;
} vec3f;

typedef struct vec3d {
	double x, y, z;
} vec3d;

typedef struct vec4i {
	int x, y, z, w;
} vec4i;

typedef struct vec4f {
	float x, y, z, w;
} vec4f;

typedef struct vec4d {
	double x, y, z, w;
} vec4d;

typedef struct rcti {
    int xmin, xmax;
    int ymin, ymax;
} rcti;

typedef struct rctf {
    float xmin, xmax;
    float ymin, ymax;
} rctf;

/* ******************************************* */
/* * * * * * * *  L I B R A R Y  * * * * * * * */
/* ******************************************* */


/* **************** MAIN ********************* */

#
#
typedef struct BoundBox {
	float vec[8][3];
} BoundBox;

#
#
typedef struct OcInfo {
	float dvec[3];
	float size[3];
} OcInfo;

/* let op: Sequence heeft identiek begin */
typedef struct ID {
	void *next, *prev;
	struct ID *new;
	struct Library *lib;
	char name[24];
	short us, flag;
} ID;

typedef struct Library {
	ID id;
	int tot;		/* tot, idblock en filedata zijn voor lezen en schrijven */
	ID *idblock;
	char *filedata;
	char name[80];
} Library;


#
#
typedef struct Main {
	struct Main *next, *prev;
	char name[80];
	short versionfile, rt;
	Library *curlib;
	ListBase scene;
	ListBase library;
	ListBase object;
	ListBase mesh;
	ListBase curve;
	ListBase mball;
	ListBase mat;
	ListBase tex;
	ListBase image;
	ListBase ika;
	ListBase wave;
	ListBase latt;
	ListBase sector;
	ListBase life;
	ListBase lamp;
	ListBase camera;
	ListBase ipo;
	ListBase key;
	ListBase world;
	ListBase screen;
	ListBase vfont;
	
} Main;


/* **************** IPO ********************* */

typedef struct Ipo {
	ID id;
	
	short blocktype, showkey;
	ListBase curve;
	
	rctf cur;
	
} Ipo;

#
#
typedef struct Path {
	int len;
	float *data;
	float totdist;
} Path;



/* **************** KEY ********************* */

typedef struct KeyBlock {
	struct KeyBlock *next, *prev;
	
	float pos;
	short flag, totelem;
	short type, rt;
	
	void *data;
	
} KeyBlock;


typedef struct Key {
	ID id;
	
	short type, totkey;
	short slurph, actkey;
	
	float curval;
	
	KeyBlock *refkey;
	char elemstr[32];
	int elemsize;
	
	ListBase block;
	Ipo *ipo;
	
	ID *from;
} Key;

/* **************** CAMERA ********************* */

typedef struct Camera {
	ID id;
	
	short type, flag, drawzoom, hold;
	float clipsta, clipend;
	float netsta, netend;			/* network camera */
	float lens, drawsize;
	
	Ipo *ipo;
	
} Camera;

/* **************** IMAGE ********************* */

typedef struct Image {
	ID id;
	
	char name[80];
	short ok, flag;
	short lastframe, lastquality;
	
	struct anim *anim;
	struct ImBuf *ibuf;
	struct ImBuf *mipmap[10];
	
	/* texture pagina */
	short tpageflag, tpagenr;
	short xrep, yrep;
	short twsta, twend;
	ulong *reprect;	/* voor SGI, om subregio's te kunnen repeaten */
	
} Image;

/* **************** TEX ********************* */

typedef struct MTex {

	short texco, mapto, maptoneg, blendtype;
	struct Object *object;
	
	char projx, projy, projz, mapping;
	float ofs[3], size[3];
	
	struct Tex *tex;
	short texflag, colormodel;
	float r, g, b, k;
	float def_var;
	
	float colfac, norfac, varfac;
	
} MTex;

typedef struct PluginTex {
	char name[80];
	void *handle;
	
	char *pname;

	int stypes;
	char *stnames;
	
	int vars;
	void *varstr;
	float *result;
	
	float data[32];

	int (*doit)();
	
} PluginTex;

typedef struct CBData {
	float r, g, b, a, pos;
	int cur;
} CBData;

typedef struct ColorBand {
	short flag, tot, cur, ipotype;
	CBData data[16];
	
} ColorBand;

typedef struct Tex {
	ID id;
	
	short type, stype;
	
	float noisesize, turbul;
	float bright, contrast, rfac, gfac, bfac;
	float filtersize;
	short noisedepth, noisetype;
	
	short imaflag, flag;
	float cropxmin, cropymin, cropxmax, cropymax;
	short xrepeat, yrepeat;
	short extend, len;
	short frames, offset, sfra, fie_ima;
	float *nor, norfac;
	
	Ipo *ipo;
	Image *ima;
	PluginTex *plugin;
	ColorBand *coba;
	
	short fradur[4][2];
	
} Tex;

/* **************** LAMP ********************* */

typedef struct Lamp {
	ID id;
	
	short type, mode;
	
	short colormodel, totex;
	float r, g, b, k;
	
	float energy, dist, spotsize, spotblend;
	float haint;
	float att1, att2;
	
	short bufsize, samp;
	float clipsta, clipend, shadspotsize;
	float bias, soft;
	
	/* onderstaand is voor buttons */
	short texact, rt;
	
	MTex *mtex[8];
	Ipo *ipo;
	
} Lamp;

/* **************** WAVE ********************* */

typedef struct Wave {
	ID id;
	
	Ipo *ipo;
	
} Wave;

/* **************** IKA ********************* */


/* **************** MATERIAL ********************* */

/* LET OP: type veranderen? ook in ipo.h doen */

typedef struct Material {
	ID id;
	
	short colormodel, lay;		/* lay: voor dynamics */
	float r, g, b;
	float specr, specg, specb;
	float mirr, mirg, mirb;
	float ambr, ambb, ambg;
	
	float amb, emit, ang, spectra;
	float alpha, ref, spec, zoffs, add;
	short har;
	char seed1, seed2;
	
	int mode;
	short flarec, starc, linec, ringc;
	float hasize, flaresize, subsize, flareboost;
	
	/* onderstaand is voor buttons en render*/
	char rgbsel, texact, pr_type, septex;
	short texco, mapto;
	
	MTex *mtex[8];
	Ipo *ipo;
	struct Material *ren;
	
} Material;


/* **************** VFont ********************* */
#
#
typedef struct VFontData {
	struct ListBase nurbsbase[256];
	float	    resol[256];
	float	    width[256];
	float	    *points[256];
	short kcount, rt;
	short *kern;	
} VFontData;

typedef struct VFont {
	ID id;
	
	char name[255], namenull;
	float scale;
	short flag, rt;
	VFontData *data;
} VFont;

/* **************** METABALL ********************* */

typedef struct MetaElem {
	struct MetaElem *next, *prev;
	
	short type, lay, flag, selcol;
	float x, y, z;
	float expx, expy, expz;
	float rad, rad2, s, len, maxrad2;
	
	float *mat, *imat;
	
} MetaElem;

typedef struct MetaBall {
	ID id;
	
	BoundBox *bb;

	int texflag;
	float loc[3];
	float size[3];
	float rot[3];
	
	float wiresize, rendersize, thresh;
	
	ListBase elems;
	ListBase disp;
	Ipo *ipo;
	
	short flag, totcol;
	Material **mat;
	
} MetaBall;

/* **************** CURVE ********************* */

#
#
typedef struct BevList {
    struct BevList *next, *prev;
    short nr, flag;
    short poly, gat;
} BevList;

#
#
typedef struct BevPoint {
    float x, y, z, alfa, sin, cos, mat[3][3];
    short f1, f2;
} BevPoint;

typedef struct BezTriple {
	float vec[3][3];
	float alfa;
	short s[3][2];
	short h1, h2;
	char f1, f2, f3, hide;
} BezTriple;

typedef struct BPoint {
	float vec[4];
	float alfa;
	short s[2];
	short f1, hide;
} BPoint;

typedef struct Nurb {
	struct Nurb *next, *prev;
	short type, mat_nr;
	short hide, flag;
	short pntsu, pntsv;
	short resolu, resolv;
	short orderu, orderv;
	short flagu, flagv;
	
	float *knotsu, *knotsv;
	BPoint *bp;
	BezTriple *bezt;
	
} Nurb;


typedef struct Curve {
	ID id;
	
	BoundBox *bb;

	int texflag;
	float loc[3];
	float size[3];
	float rot[3];
	
	ListBase nurb;
	ListBase disp;
	struct Object *bevobj, *textoncurve;
	Ipo *ipo;
	Path *path;
	Key *key;
	short pathlen, totcol;
	Material **mat;
	
	short flag, bevresol;
	float width, ext1, ext2;
	
	ListBase bev;
	float *orco;
	
	/* default */
	short resolu, resolv;
	
	/* font stuk */
	short len, lines, pos, spacemode;
	float spacing, linedist, shear, fsize;
	float xof, yof;
	char *str, family[24];
	VFont *vfont;

} Curve;

/* **************** IPOCURVE (Ipo staat bovenin) ********************* */

typedef struct IpoCurve {
	struct IpoCurve *next,  *prev;
	
	short blocktype, adrcode, vartype;
	short totvert;
	short ipo, extrap;
	short flag, rt;
	float ymin, ymax;
	ulong bitmask;
	rctf maxrct, totrct;
	
	float curval;
	
	BPoint *bp;
	BezTriple *bezt;
	
} IpoCurve;

/* **************** MESH ********************* */

typedef struct MFace {
	ushort v1, v2, v3, v4;
	char puno, mat_nr;
	char edcode, flag;
} MFace;

typedef struct MFaceInt {
	int v1, v2, v3, v4;
	char puno, mat_nr;
	char edcode, flag;
} MFaceInt;

typedef struct MVert {
	float co[3];
	short no[3];
	char flag, mat_nr;
} MVert;

typedef struct MCol {
	char a, r, g, b;
} MCol;

typedef struct MSticky {
	float co[2];
} MSticky;

typedef struct Mesh {
	ID id;

	BoundBox *bb;

	int totvert, totface;
	
	int texflag;
	float loc[3];
	float size[3];
	float rot[3];
	
	short smoothresh, flag;
	
	ListBase effect;
	ListBase disp;
	
	void *mface, *dface, *tface;
	MVert *mvert;
	MCol *mcol;
	MSticky *msticky;
	struct Mesh *texcomesh;
	float *orco;
	
	Ipo *ipo;
	Key *key;
	short rt, totcol;
	Material **mat;
	
	float cubemapsize;
	
	OcInfo *oc;		/* niet in file */
	
} Mesh;

/* **************** LATTICE ********************* */

typedef struct Lattice {
	ID id;
	
	short pntsu, pntsv, pntsw, flag;
	char typeu, typev, typew, type;
	
	BPoint *def;
	int rt;
	
	Ipo *ipo;
	Key *key;
	
} Lattice;


/* **************** OBJECT ********************* */

typedef struct Object {
	ID id;
	short type, partype;
	int par1, par2, par3;	/* kunnen vertexnrs zijn */
	
	struct Object *parent, *track;
	struct Ipo *ipo;
	Path *path;
	BoundBox *bb;
	void *data;
	
	/* rot en drot moeten achterelkaar! (transform('r' en 's')) */
	float loc[3], dloc[3], orig[3];
	float size[3], dsize[3];
	float rot[3], drot[3];
	float obmat[4][4];
	float parentinv[4][4];
	float imat[4][4];	/* voor bij render, tijdens simulate, tijdelijk: ipokeys van transform  */
	
	ulong lay;			/* kopie van Base */
	short flag;			/* kopie van Base */
	short colbits;		/* nul==van obdata */
	char transflag, ipoflag;
	char trackflag, upflag;
	short ipowin, infoend;	/* ipowin: blocktype laatste ipowindow */
	short infoflag, infostart;
	
	short dupon, dupoff, dupsta, dupend;

	ListBase effect;
	ListBase network;
	
	float sf, ctime;
	ushort dfras, rt;		/* tijdens realtime */
	
	ListBase disp;
	char dt, dtx;
	char totcol;	/* kopie van mesh of curve of meta */
	char actcol;
	Material **mat;

} Object;




/* **************** WORLD ********************* */


typedef struct World {
	ID id;
	
	short colormodel, totex;
	short texact, mistype;
	
	float horr, horg, horb, hork;
	float zenr, zeng, zenb, zenk;
	float ambr, ambg, ambb, ambk;
	ulong fastcol;
	
	float exposure;
	
	short skytype, mode;
	
	float misi;
	float miststa, mistdist, misthi;
	
	float starr, starg, starb, stark;
	float starsize, starmindist, stardist, starcolnoise;
	
	short dofsta, dofend, dofmin, dofmax;
	
	Ipo *ipo;
	MTex *mtex[8];

} World;


/* **************** SCENE ********************* */

typedef struct RenderData {
	
	/* hoe gaat tijd gedefinieerd worden? */
	short cfra, sfra, efra;	/* plaatjes */
	short images, framapto, flag;
	float ctime;			/* hiermee rekenen? */
	float framelen, blurfac;

	short size, maximsize;	/* size in %, max in Kb */
	short xsch, ysch, xasp, yasp;	/* uit buttons */
	short xparts, yparts;
	rctf safety, border;
	
	short winpos, planes, imtype, bufflag, quality, scemode;
	short mode, alphamode, dogamma, osa;

	char backbuf[80], pic[80], ftype[80], movie[80];
	
} RenderData;

typedef struct Base {
	struct Base *next, *prev;
	ulong lay, selcol;
	int flag;
	short sx, sy;
	Object *object;
	
} Base;

typedef struct Scene {
	ID id;
	Object *camera;
	World *world;
	struct Scene *set;
	struct Image *ima;
	ulong lay;
	
	ListBase base;
	Base *basact;
	
	float cursor[3];

	struct RenderData r;

	/* enkele realtime vars */
	float grav, rtf;
	short maxdrawsector, flag;
	
	void *ed;
} Scene;

/* ******************************************* */
/* * * * * * * *  SPACE STRUCTS  * * * * * * * */
/* ******************************************* */

typedef struct BGpic {
    Image *ima;
	Tex *tex;
    float xof, yof, size, zoom, blend;
    short xim, yim;
	ulong *rect;
} BGpic;



typedef struct View3D {
	struct View3D *next, *prev;
	int spacetype; 
	float viewmat[4][4];
	float viewinv[4][4];
	float persmat[4][4];
	float persinv[4][4];
	float viewquat[4], dist;
	short persp, view, drawtype, localview;
	int lay, layact;
	short scenelock, around, camzoom, flag;
	
	float lens, grid, near, far;
	float ofs[3], cursor[3];
	
	Object *camera;
	short mx, my;	/* moeten achter elkaar blijven staan ivm als pointer doorgeven */
	short mxo, myo;

	short pr_xmin, pr_xmax, pr_ymin, pr_ymax;
	short pr_sizex, pr_sizey;
	short gridlines, rt;
	float pr_facx, pr_facy;

	BGpic *bgpic;
	struct View3D *localvd;
	
} View3D;

typedef struct View2D {
	rctf tot, cur;
	rcti vert, hor, mask;
	float min[2], max[2];
	float minzoom, maxzoom;
	short scroll, keeptot;
	short keepaspect, keepzoom;
} View2D;

typedef struct SpaceIpo {
	struct SpaceIpo *next, *prev;
	int spacetype; 
	View2D v2d;
	
	short totipo, lock;
	void *editipo;
	ListBase ipokey;
	Ipo *ipo;
	ID *from;
	short butofs, channel;
	short showkey, blocktype;
	short menunr, rt;
	ulong rowbut;
	rctf tot;
} SpaceIpo;

typedef struct SpaceButs {
	struct SpaceButs *next, *prev;
	int spacetype;
	View2D v2d;
	
	short mainb, menunr;	/* texnr en menunr moeten shorts blijven */
	short lock, mainbo;	
	short texnr;
	char texfrom, rt2;		/* temps */
	void *lockpoin;
	
	/* preview render */
	short rectx, recty;
	ulong *rect;
	short cury, modeltype;
	
} SpaceButs;

typedef struct SpaceSeq {
	struct SpaceSeq *next, *prev;
	int spacetype;
	View2D v2d;
	
	short mainb, zoom;

} SpaceSeq;

typedef struct SpaceFile {
	struct SpaceFile *next, *prev;
	int spacetype;
	
	struct direntry *filelist;
	int totfile;
	char title[24];
	char dir[120];
	char file[80];
	short type, ofs, flag, sort;
	short maxnamelen, collums;
	char *libfiledata;
	short retval, ipotype;
	short menu, act;
	
	void (*returnfunc)();
	
} SpaceFile;

typedef struct SpaceOops {
	struct SpaceOops *next, *prev;
	int spacetype;
	View2D v2d;
	
	ListBase oops;
	short lock, visiflag, flag, rt;
	void *lockpoin;
	
} SpaceOops;

typedef struct SpaceImage {
	struct SpaceImage *next, *prev;
	int spacetype;
	View2D v2d;
	
	Image *image;
	short mode, zoom;
	short imanr, curtile;
	short xof, yof;
	short flag, rt;
	
} SpaceImage;


/* SpaceImaSel in aparte headerfile */
/* SpacePaint in aparte headerfile */


/* ************* GLOBAL ******************* */

#
#
typedef struct dl_GourFace {
	float co[3][3];
	ulong col[6];
} dl_GourFace;
#
#
typedef struct FastLamp {
	short type, mode, lay, rt;
	float co[3];
	float vec[3];
	float dist, distkw, att1, att2, spotsi, spotbl, r, g, b;
} FastLamp;
#
#
typedef struct DispList {
    struct DispList *next, *prev;
    short type, flag;
    int parts, nr;
    short col, rt;		/* rt wordt gebruikt door initrenderNurbs */
	float *verts, *nors;
	int *index;
	ulong *col1, *col2;
} DispList;


typedef struct UserDef {
	short flag, dupflag;
	int savetime;
	char tempdir[32];
	char fontdir[32];
	char renderdir[32];
	char textudir[32];
	char plugseqdir[32];
	short versions, rt;
} UserDef;


#
#
typedef struct Global {
	
	/* active pointers */
	View3D *vd;
	View2D *v2d;
	SpaceIpo *sipo;
	SpaceButs *buts;
	SpaceImage *sima;
	SpaceOops *soops;
	Main *main;
	Scene *scene;				/* denk aan file.c */
	struct bScreen *curscreen;
	Object *obedit;
	
	/* fonts, allocated global data */
	long *font, *fonts, *fontss;
	ListBase mainbase;

	/* strings: lastsaved */
	char ima[80], sce[80], lib[80], psx[80];

	/* totalen */
	short totobj, totlamp, totobjsel, totcurve, totmesh, totmat;
	long totvert, totface, totvertsel, totfacesel;
	long time, cputime;
	
	short machine, afbreek, f, moving, colact, zbuf;
	short qual, background, imagewin, animspeed;
	short version, versionfile, simulf, fields, order, rt;
	
	struct ListBase edve;
	struct ListBase eded;
	struct ListBase edvl;
	
	float textcurs[4][2];
	
	/* realtime */
	struct Sector *cursector, *sectorbuf[SE_MAXBUF];
	Object *lifebuf[LF_MAXBUF];
	short *actvarar;

	int dfra, dfrao;
	short dfras, totsect, maxsect, totlife;

	/* variabelen van frank */
	int renderd;
	int real_sfra, real_efra;
	int	save_over;
	
	
} Global;

#include "exports.h"


#endif /* BLENDER_H */

