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
#include <fcntl.h>
#include "/usr/people/include/iff.h"
#include "/usr/people/include/Trace.h"
#include "/usr/people/include/Button.h"
#include <malloc.h>
#include <sys/schedctl.h>
#include <stdio.h>

#include <sys/time.h>		/* voor de interruptESC */
#include <signal.h>
#include <sys/schedctl.h>

/* *************** GLOBALE VARIABELEN ***************** */

struct View view0 = 
{
	0,1,1,			/* next, nummer rt1*/
	{0,-35000,23000},{0,0,0},	/* obs[3],tar[3] */
	35,0,1,0,		/* lens,view,persp rt2*/
	0.959,-1.570,1,		/* phi,theta,dproj */
	0,5000,			/* rt3,grid (rt want grid was vroeger long) */
	{0,0,0},{0,0,0},	/* ofs[3],muis[3] */
	1,0,0,0,0,0,0,0, 	/* lay,rt4,tilt,opar,tpar,cambase,mx,my */
	0,0			/* near,far */
};
struct Wrld wrld=
{
	200,0,0,0,0,200,	/* horr,horg,horb,zenr,zeng,zenb; */
	10,10,20,100,130,160,	/* ambr,ambg,ambb,gror,grog,grob; */
	0,0,0,0,		/* misr,misg,misb,misi; */
	1,0,0,1000,		/* fs,fg, rt2, mistdist */
	{0,0,0,0},		/* stex[4],*/
	{10,10,10,10},{0,0,0,0},/* smap[4], *sbase[4] */
	0,0,0,			/*  */
	1,0			/* short globsh,rt3; */
};

struct Global G =
{
	0,1,1,1,0,0,1,		/* qual,mainb,,mainbo,hie,move1,move2,layact */
	0,0,0,0,0,0,		/* totobj,totlamp,totmove,totvert,totvlak,totpoly */
	0,					/*,f, */
	0,0,0,0,20,			/* actp2,actp3,actkey,keydraw,ipomode;  */
	0,0,0,0,0,			/* firstbase, *basact, *lastbase, *ebase *workbase;*/
	0,					/* struct Bezier *spline; */
	0,0,0,0,0,0,0,0,	/* float persmat[4][4], persinv[4][4]; */
	0,0,0,0,0,0,0,0,	/* float winmat[4][4] float viewmat[4][4],viewinv[4][4] */
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
/* NOTITIE: VERPLAATS ALLE GLOBALE RENDER-INFO NAAR STRUCT RENDER */
	1,1,80,80,80,80,	/* short cfra,sfra,efra,frs,framap,framapto; */
	1.0,			/* float framelen */
	10,0,0,0,0,		/* short size,filt,schaduw,trace,border;*/
	0,0,0,0,0,0,5,0,	/* short skybuf,ali,script,background,disc,holo,osa,genlock;*/
	0, 0, 0, 0,		/* short bufscript,rt1,rt2,rt3 ;*/
	0,0,0,0,1,0,2,		/* short xminb,xmaxb,yminb,ymaxb,sview,tracetype,tradep;*/
	0,1,			/* short totex,colact  */
	0,			/* struct Tex **adrtex */
				/* char wf0[3],wf1[3],wf2[3],wf3[3],wf4[3],wf5[3];*/
				/* char obj[80],ima[80],sce[80],skb[80],pic[80],scr[80];*/
};

struct UserDef User =
{
	{100, 78, 78, 78}, 
	
	{120,104,138,167}, 
	{121,31,31,40}, 
	{122,174,198,207}, 

	{124,162,162,162}, 
	{125,76,76,76}, 
	{126,226,226,226}, 

	{128,46,147,137}, 
	{129,0,56,0}, 
	{130,76,217,207}, 

	{132,152,125,160}, 
	{133,82,0,85}, 
	{134,222,185,230}
};

float bezmat[4][4]={
	{-1.0, 3.0,-3.0, 1.0},
	{ 3.0,-6.0, 3.0, 0.0},
	{-3.0, 3.0, 0.0, 0.0},
	{ 1.0, 0.0, 0.0, 0.0},
};

struct Screen S;

ulong rect_desel[16],rect_sel[16];
ulong rectl_desel[81],rectl_sel[81];
ulong rectq_desel[81],rectq_sel[81];
short activeview = 1;

/* **************************************************** */

void projektie()
{
	
}

short okee(char *str)
{
	return 1;
}

short saveover(char *str)
{
	return 1;
}

void error(str)
{
	printf("ERROR: %s\n",str);
}

short domenu(str)
char *str;
{
	return 1;
}


void main(argc,argv)
long argc;
char **argv;
{
	extern void savecore();			/* files.c */
	extern struct Branch *adroct[];
	extern char versionstr[32];
	struct VectorFont *initvfont(),*vf;
	float bepaalphitheta();
	long file;
	short a, mval[2], cursmap[16], xmax, ymax;
	char pic[100], tstr[100], *home;
	
	if (argc == 2) {
		if (strcmp(argv[1], "debug") == 0){
			foreground();
			argc = 1;
		}
	}

	S.bxw= 0;

	/* BACKGROUND? */

	printf("TRACES Version %s\n", versionstr+14);
	foreground();

	mallopt(M_MXFAST,sizeof(struct EditVert)+4);	/* blokken kleiner dan (32+4) snel uitdelen (t.o.v 28 standaard) */
	mallopt(M_MXCHK,2048);	/* normaal zoekt 'ie max. 100 blokken af voor ie opgeeft. Onzin */
	mallopt(M_FREEHD,1);

/* 	sigset(SIGBUS, savecore); */
/* 	sigset(SIGSEGV, savecore); */

	strcpy(G.obj,"//");
	strcpy(G.ima,"//");
	strcpy(G.sce,"//rt.trace");
	strcpy(G.skb,"//buf");
	strcpy(G.pic,"//");
	strcpy(G.scr,"//script");
	strcpy(G.ftype,"/usr/people/trace/ftypes/amiga.rgb");
	strcpy(G.bufscr, "//buf.script");
	G.time= G.f= 0;
	G.spline=(struct Bezier *)callocN(4+4*7200,"spline"); /*max 400 contrpunten */
	G.adrtex=(struct Tex **)callocN(MAXTEX*4,"G.adrtex");
	G.adrtex[0]=(struct Tex *)callocN(152,"tex[0]");
	G.edve.first= G.edve.last= 0;
	G.eded.first= G.eded.last= 0;
	G.edvl.first= G.edvl.last= 0;
	G.xminb= 320;
	G.xmaxb= 960;
	G.yminb= 200;
	G.ymaxb= 600;
	view0.near= 1000; 
	view0.far= 1000;
	G.zbuf= 0;

	G.workbase=(struct Base *)callocN(256,"workbase");
	G.workbase->soort=1;
	G.workbase->f=12;
	G.workbase->f1=42;
	G.workbase->lay= 0xFFFF;
	G.workbase->sf=1;
	G.workbase->q[0]=1.0;
	G.workbase->m[0][0]=1.0;
	G.workbase->m[1][1]=1.0;
	G.workbase->m[2][2]=1.0;
	R.blove=(struct VertRen **)callocN(4*1024,"Blove");
	R.blovl=(struct VlakRen **)callocN(4*1024,"Blovl");
	R.xsch=720;
	R.ysch=576;
	R.xasp=54;
	R.yasp=51;
	R.size=2;
	R.rectot=R.rectz=R.rectdaps=R.rectaccu= 0;
	R.xof=450;
	R.yof=1000;
	R.rectx=R.recty= 0;
	R.xparts=R.yparts= 1;
	R.winpos= 128;
	R.anim= 6; 
	R.planes= 1; 
	R.imtype= 1; 
	R.safety= 100;
	for(a=0;a<256;a++) adroct[a]=0;
	init_filt_mask();


	initeffects(); /* uit effect.c */

	/*START*/

	G.background=1;

	if (schedctl(NDPRI, 0, NDPLOMIN) == -1) printf("no ndpri \n");
	G.winar[0]=G.winar[1]= 0;
	for(a=1; a<argc; a++) {
		if(argv[a][0] == '-') {
			switch(argv[a][1]) {
			case 'f':
				a++;
				G.cfra = atoi(argv[a]);
				initrender(0);
				makepicstring(pic,G.cfra);
				schrijfplaatje(pic);
				timestr(G.cputime,tstr);
				printf("Saved: %s Time: %s (%.2f)\n",pic,tstr,((float)(G.time-G.cputime))/100);

				break;
			case 'a':
				initrender(R.anim);
				break;
			case 'p':
				if (schedctl(NDPRI, 0, NDPLOMIN) == -1) printf("no ndpri \n");
				break;
			case 's':
				if (++a < argc) G.sfra = atoi(argv[a]);
				break;
			case 'e':
				if (++a < argc) G.efra = atoi(argv[a]);
				break;
			}
		}
		exit(0);
	}
}

