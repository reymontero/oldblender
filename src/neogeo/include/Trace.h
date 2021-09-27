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

	/* header voor trace  		*/


	/* struct EditVert   		*/
	/* struct EditEdge   		*/
	/* struct EditVlak   		*/

	/* struct Bezier   		*/
	/* struct Key			*/

	/* struct Base   		*/
	/* struct ColBlck   		*/
	/* struct LaData		*/
	/* struct ObData		*/
	/* struct PerfSph		*/
	/* struct VV			*/
	/* struct VertOb		*/
	/* struct VlakOb		*/
	/* struct MoData		*/
	/* struct PolyData		*/
	/* struct Tex			*/

	/* struct Wrld			*/

	/* struct View			*/
	/* struct Global		*/
	/* struct Textudata		*/

	/* struct VertRen		*/
	/* struct VlakRen		*/
	/* struct LampRen		*/
	/* struct PolyRen		*/

	/* struct Render		*/
	/* struct Osa			*/

	/* struct Node			*/
	/* struct Branch		*/

	/* struct Matrix3		*/
	/* struct Matrix4		*/

/* ******************* DEFINES ********************************* */

	/********************** X11 PATCH !!! ********************* */
#define _XLIB_H_


#include <sys/types.h>
#include <local/util.h>
#include <local/objfnt.h>
#include <local/iff.h>
#include "/usr/people/trace/ika.h"

#define IRIS 1
#define ELAN 2
#define ENTRY 3

#define DL_POLY		  0
#define DL_POLY_SOLID	100
#define DL_POLY_GOUR	200

#define DL_SEGM		  1

#define DL_SURF		  2
#define DL_SURF_SOLID	102
#define DL_SURF_GOUR	202

#define DL_TRIA		  3
#define DL_TRIA_SOLID	103
#define DL_TRIA_GOUR	203

#define DL_INDEX	  4

#define FNT_PDRAW 1
#define FNT_HAEBERLI 2

#define drawmode mydrawmode

#define DL_SURFINDEX(cyclu, cyclv, sizeu, sizev)	    \
							    \
    if( (cyclv)==0 && a==(sizev)-1) break;		    \
    if(cyclu) {						    \
	p1= sizeu*a;					    \
	p2= p1+ sizeu-1;				    \
	p3= p1+ sizeu;					    \
	p4= p2+ sizeu;					    \
	b= 0;						    \
    }							    \
    else {						    \
	p2= sizeu*a;					    \
	p1= p2+1;					    \
	p4= p2+ sizeu;					    \
	p3= p1+ sizeu;					    \
	b= 1;						    \
    }							    \
    if( (cyclv) && a==sizev-1) {			    \
	p3-= sizeu*sizev;				    \
	p4-= sizeu*sizev;				    \
    }

#define SHLX (-.577349)
#define SHLY (-.577349)
#define SHLZ (0.577349)

#define SHADESPHEREX 62
#define SHADESPHEREY 62

#define SHLX1 (SHLX*SHADESPHEREX)
#define SHLY1 (SHLY*SHADESPHEREX)
#define SHLZ1 (SHLZ*SHADESPHEREX)

#define IPOMINX 640
#define IPOMAXX 1260
#define IPOMINY 20
#define IPOMAXY (223-30)

#define PI 3.14159265358979323846

#define VECCOPY(v1,v2) 		{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2);}
#define QUATCOPY(v1,v2) 	{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2); *(v1+3)= *(v2+3);}
#define LEN(x,y)		{fsqrt((x)*(x)+(y)*(y))}
#define SGN(x)			( (x)>0 ? 1: (x)<0 ? -1: 0 )


#define TESTBASE(base)	(base->f & 1)!=0 && (base->lay & view0.lay)!=0

	/* limits */
#define MAXTEX		160
	/* renderlimits */
#define MAXPOLY 256
#define MAXLAMP 256
#define MAXVERT (2<<18)
#define MAXVLAK (2<<18)

	/* handles */
#define HD_FREE 0
#define HD_AUTO 1
#define HD_VECT 2
#define HD_ALIGN 3


/* *********************** FUNCTIES ***************************** */

/*
Deze zitten al in de util.h (en die wordt geincluded)
extern char *mallocN(long,char *);
extern char *callocN(long,char *);
extern short freeN(char *);
*/
extern float initgrabz(long x,long y,long z);
extern short testquESC();
extern short okee(char *);
extern short traces_qread(short *);

/* *********************** STRUCTS ***************************** */

extern struct Global G;
extern struct View view0;
extern struct Render R;
extern struct Wrld wrld;
extern struct Screen S;
extern struct UserDef User;

/* *************** */

struct dl_GourFace {
	float co[3][3];
	ulong col[6];
};
struct FastLamp {
	short soort, f, lay;
	float co[3];
	float vec[3];
	float dist, distkw, ld1, ld2, spotsi, spotbl, r, g, b;
};

/* ******** */

struct BevList {
    struct BevList *next, *prev;
    short nr, flag;
    short poly, gat;
};

struct BevPoint {
    float x, y, z, sin, cos, mat[3][3];
    short f1, f2;
};

struct Bezier
{
	short cp;
	char f,f1;
};

struct BezTriple
{
    float vec[3][3];
    short s[3][2];
    char h1, h2;
    char f1, f2, f3, hide;
    short rt;
};

struct BPoint
{
    float vec[4];
    short s[2], f1, hide;
};

struct Base
{
	struct Base *next;
	short soort;
	char f, f1;
	short sx, sy;
	long v[3];
	ushort lay;
	short sf,len;
	short f2;
	long *d, o[3];
	float q[4], m[3][3];
	long ivec[3];
	float imat[3][3];
	long rt1;
	char str[14];
	short nnr;
	long selcol;
	struct Base *p;
	struct Base *w;
	struct Base *t;
	struct Key *pkey;
	struct Bezier *ipopkey;
	struct Key *key;
	struct Bezier *ipokey;
	struct Base *borig;
	long r[3], r2[3];
	char tflag, upflag;
	char wflag, lastipo;
	short noise, noisefreq, nvecint, nrotint;
	struct Bezier *iponoise;
	short rodnr, skelnr;
	char dupli, duplirot;
	short rt4;
	short holo1, holo2;
};	

struct ColBlck
{
	char r,g,b,mode;
	char kref,kspec,spec,tra,mir,ang,amb;
	char specr,specg,specb,mirr,mirg,mirb;
	char texco;
	short hasize, mode1;	/* mode1: omdat floatcolblock short heeft */
	char ambr,ambg,ambb;
	char emit;
	ushort lay;
	char tex[4];
	ushort map[8];
	struct Base *base[4];
};

struct ColBlckF
{
	float a,r,g,b;
	short mode, spec;
	float kref,kspec,tra,mir, emit;
	float specr,specg,specb,mirr,mirg,mirb;
	float ambr,ambg,ambb;
	float ang,amb,speclim;
	short copysize, texco;
	ushort lay, rt1;
	long rt2, rt3;
	void (*filtfunc)();
	char tex[4];
	ushort map[8];
	struct Base *base[4];
};

struct CurveData
{
	struct ListBase curve;
	struct ListBase disp;
	struct ListBase bev;
	struct Base *bevbase;
	float ws;
	short us, flag;
	short width, ext1, ext2, presetbev;
	float depth;
	struct Key *key;
	short keyverts, rt1;
	short *orig;
	long rt2;
};

struct DispList {
    struct DispList *next, *prev;
    short type, flag;
    long parts, nr;
    short col, rt;
};

struct FontData
{
	short type;
	char f,f1;
	struct VectorFont *vf;
	short len;
	short lines;
	char *str;
	short set[6];
	struct Bezier *bez;
	short pos,rt;
	long xof,yof;
};

struct Global
{
	short qual,mainb,mainbo,hie,move1,move2,layact;
	short totobj,totlamp,totmove,totpoly;
	long totvert,totvlak;
	short f,actp2,actp3,actkey,keydraw,ipomode;
	struct Base *firstbase, *basact, *lastbase,*ebase,*workbase;
	struct Bezier *spline;
	float persmat[4][4], persinv[4][4], winmat[4][4], viewmat[4][4], viewinv[4][4];
	short cfra,sfra,efra,frs,images,framapto;
	float framelen;
	short size,filt,schaduw,trace,border;
	short skybuf,ali,script,background,disc,holo,osa,genlock;
	short bufscript, zbuf, moving, localview;
	short xminb,xmaxb,yminb,ymaxb,sview,obszoom,tradep;
	short totex,colact;
	struct Tex **adrtex;
	long near, far;
	short keymode, holopadlen;
	struct Bezier *timeipo;
	short obslimits;
	char obj[80],ima[80],sce[80],skb[80],pic[80],scr[80],ftype[80];
	char bufscr[80];
	long winar[10],winakt,*font;
	struct ListBase edve;
	struct ListBase eded;
	struct ListBase edvl;
	long totvertsel,totvlaksel,tothalo,totvlakt,time,cputime;
	struct ListBase vfbase;
	short afbreek, machine;
	float textcurs[4][2];
};

struct Imap
{
	char type,ok;
	short us;
	char name[88];
	short lastframe, lastquality;
	struct anim *anim;
	struct MipMap *mipmap;
	struct ImBuf *ibuf;
};

struct Key
{
	struct Key *next;
	float pos;
	short soort;
	char f,f1;
};

struct LaData
{
	float energy;
	short rt,soort;
	long ld;
	char r,g,b,haint;
	char spsi,spbl,f,f1;
	char ld1,ld2;
	short bufsize,clipsta,clipend;
	short samp,bias;
	float soft;
	char tex[4];
	short map[4];
	struct Base *base[4];
	long rt4;
	long rt3;
};

struct MipMap
{
    struct MipMap *next;
    struct ImBuf *ibuf;
};

struct MoData
{
	struct Bezier *beztra;
	struct Bezier *ipotra;
	float *data,*mkey;
	struct Bezier *ipovkey;
	struct ListBase curve;
	long rt1;
	struct Bezier *ipoqfi;
	short qf,ease1,ease2,n[3];
	short qfo;
	short dupon;
	short fr;
	char f,f1;
	short dupoff;
	struct ListBase disp;
	struct Key *key;
	short keyverts, lock;
	long rt4[1];
};


struct Nurb {
    struct Nurb *next, *prev;
    short type;
    short pntsu, pntsv, orderu, orderv, resolu, resolv;
    char flagu, flagv;
    struct BezTriple *bezt;
    struct BPoint *bp;
    float *knotsu, *knotsv;
    struct ListBase trim;
    short col, hide;
    long rt2, rt3, rt4;
};

struct ObData
{
	struct VV *vv;
	float vertloc[3];		/* location vertexparent */
	ushort vl1, vl2, vl3, vl4;	/* vertnrs vertexparent */
	long rt[4];
	struct PolyData *po;
	struct FontData *fo;
	struct CurveData *cu;
	char c,dt;	/* aantal colbl, drawtype */
				/* LET OP: EEN GAT? */
	struct ListBase disp;
	char e,ef;				
	short estep,elen;
	short rt3;	/* keepnorm */
	float *floatverts;
	struct Bezier *ipovvkey;
};

struct PerfSph
{
	long rt1;
	long rad, radf;
	struct VertOb *vert;
	long rt2[4];
	struct ColBlck c;
};

struct PolyData
{
	short vert,poly;
	long afm[3];
	float ws;
	char f,f1;
	short us,ext,f45p,f45m,n[3];
	short *f45;
	float depth;
	long rt2[6];
};

struct Tex
{
	char soort,type,f,cint,bint,vint;
	char c1[3],c2[3],c3[3],li[3];
	short div[3],add[3],var[12];
	char str[12];
	float cintf, vintf;
	float c1f[3], c2f[3], c3f[3];
	struct Imap *ima;
	long rt1, rt2;
	float lif[3], divf[3], bintf;
};

struct Textudata
{
	char tekst[12][10];
	short min[12],max[12],def[12];
	long c1,c2,c3,li;
	char f;
	short div[3],add[3];
};

struct VectorFont {
	struct VectorFont *next,*prev;
	char *name;
	unsigned char *base;
	unsigned char *index[256];
	short kcount,us;
	short *kern;
	struct objfnt * objfnt;
	char type;
};


struct VertOb
{
	short c[3],n[3];
};

struct View
{
	struct View *next;
	short num, rt1;
	long obs[3], tar[3];
	short lens, view, persp, rt2;
	float phi, theta, dproj;
	short rt3, grid;	/* rt want grid was vroeger long */
	long ofs[3], muis[3];
	ushort lay, rt4;
	float tilt;
	struct Base *opar, *tpar, *cambase;
	short mx,my;
	short near, far;
};

struct VlakOb
{
	ushort v1,v2,v3;
	short n[3];
	char rt1,rt2,f,ec;
};

struct VV
{
	long vert,vlak,afm[3];
	short us;
	char f,f1;
	float ws;
	struct Key *key;
};

struct Wrld
{
	char horr,horg,horb,zenr,zeng,zenb;
	char ambr,ambg,ambb,gror,grog,grob;
	char misi, dofi, rt01, rt02;
	char fs,fg;
	short misthi;
	long mistdist;
	char stex[4],smap[4];
	struct Base *sbase[4];
	short dofsta, dofend, dofmin, dofmax;
	float expos;
	long rt5[3];
	short globsh, starmindist;
	float grvec[3],inprh,inprz;
	long miststa;
};

/*  *****************  RENDER  ***************  */

struct Branch
{
	struct Branch *b[8];
};

struct HaloRen
{	
	float xs,ys,rad,radsq, co[3];
	ulong zs,zd;
	short miny,maxy;
	char alfa,b,g,r,type,f;
	char starpoints, tex;
	struct ColBlckF *col;
};

struct LampRen
{
	float xs,ys,dist;
	long co[3];
	short soort;
	float r, g, b;
	char haint,f,f1, f2;
	ushort lay;
	float spotsi,spotbl;
	char tex[4];
	short map[4];
	struct Base *base[4];
	float vec[3];
	float xsp,ysp,distkw,inpr;
	float halokw,halo;
	float ld1,ld2;
	struct ShadBuf *shb;
	float imat[3][3];
	float spottexfac;
	/* float face[4][3]; */
	float sh_invcampos[3], sh_zfac;	/* sh_= spothalo */
	float energy;
};

struct PolyRen
{
	short vert,poly;
	struct ColBlckF *col;
	float n[3];
	double *nurb;
	double *data;
};

struct FaceStrip
{
	float n[3];
};

struct Node
{
	struct VlakRen *v[7];
	struct Node *next;
};

struct Osa
{
	long dxco[3],dyco[3];
	float dxlo[3],dylo[3],dxgl[3],dygl[3],dxuv[2],dyuv[2];
	float dxref[3],dyref[3],dxorn[3],dyorn[3];
	float dxno[3], dyno[3], dxview, dyview;
	float dxlv[3], dylv[3];
};

struct Part
{
	struct Part *next, *prev;
	ulong *rect;
	short x, y;
};

struct PixStr
{
	struct PixStr *next;
	long vlak0,vlak;
	ulong z;
	ulong mask;
	short aantal, ronde;
};

struct PixStrMain
{
	struct PixStr *ps;
	struct PixStrMain *next;
};
	
struct QStrip
{
	struct QStrip *next, *prev;
	short pntsu, pntsv;
	short flagu, flagv;
	long firstface;
	struct ColBlckF *col;
	struct VertStrip *verts;
	struct FaceStrip *faces;
};

struct Render
{
	float co[3];
	long lo[3],gl[3],uv[2],ref[3],orn[3];
	short itot,i,ic,rgb,norm;
	float vn[3],view[3],xcor,ycor,zcor,*vno;
	struct ColBlckF col;
	short xsch,ysch,afmx,afmy,xasp,yasp,size,d,f,f1,anim;
	short xstart,xend,ystart,yend,xparts,yparts;
	long svlako;
	struct VertRen **blove;
	struct VlakRen **blovl,*vlr;
	struct PolyRen **poly;
	struct LampRen **la;
	struct HaloRen **bloha;
	struct VlakRen **blovlt;
	ulong *rectot, *rectz, *rectdaps, *rectaccu, *rectf1, *rectf2;
	struct ListBase parts;
	short xof,yof,rectx,recty;
	short schaduwvoeler,osatex;
	short winpos,planes,imtype, safety;
	float pixsize, near, far;
	short edgeint, dogamma;
	ulong *rectspare;
	long rt2[3];
};

struct ShadBuf {
	short samp,rt;
	float persmat[4][4];
	float winmat[4][4];
	float jit[25][2];
	float d,far,pixsize,soft;
	long co[3];
	long size,bias;
	ulong *zbuf;
	char *cbuf;
};

struct VertRen
{
	float co[3];
	float n[3];
	float ho[4];
	struct VertOb *v;
	ulong colg;	/* voor gouroud */
	short clip,rt;
};

struct VertStrip
{
	float co[3];
	float n[3];
	float ho[4];
	short clip, flag;  
};

struct VlakRen
{
	struct VertRen *v1,*v2,*v3;
	float n[3],len;
	struct ColBlckF *col;
	struct VlakOb *vl;
	char snproj,puno;
	char rt1, ec;
	ulong colg;	/* voor gouroud */
};


/* *************  MISC  *************** */

struct BGpic {
    struct ImBuf *ibuf;
    float dproj, zoom;
    short xim, yim, size, blend;
    char name[100];
};


struct EditVert
{
	struct EditVert *next,*prev,*vn;
	float co[3];
	short xs,ys;
	char f, h;
	short f1, hash, rt;
	ulong vnorm;
};

struct EditEdge
{
	struct EditEdge *next,*prev;
	struct EditVert *v1,*v2,*vn;
	char f,h;
	short f1;
};

struct EditVlak
{
	struct EditVlak *next,*prev;
	struct EditVert *v1,*v2,*v3;
	float n[3];
	short rt;
	char f,f1;
};

struct Matrix3
{
	 float i[3][3];
};

struct Matrix4
{
	 float i[4][4]; 
};

typedef struct rct {
    int xmin, xmax;
    int ymin, ymax;
} rct;

typedef struct rctf {
    float xmin, xmax;
    float ymin, ymax;
} rctf;

struct Screen
{
	short bxs, bys, bxw, byw; 
	short vxs, vys, vxw, vyw;
	short x, y;
};

struct UserDef
{
	short back[4];
	short but0[3][4], but1[3][4], but2[3][4], but3[3][4], but4[3][4], but[3][5];
	long rt[24];
};




