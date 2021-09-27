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

/*	traces.c	*/


/*	

Alle c progs backuppen met kermit nieuwer dan zbuf.c :

find /usr/people/trace -name '*.c'  -cnewer zbuf.c -print -exec kermit -s {} \;


 
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

short	okee(str)
	error(str)
	sluit(type)

short	testquESC()
	setcursorN(cursor)
	ledata()

	mainbuts()
	layerbuts(event)
short 	ffileselect(tekst,dir)

	teststructs()

	main()
*/

	/********************** X11 PATCH !!! ********************* */
#define _XLIB_H_		


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
#include <ctype.h>			/* toupper() */
#include <sys/time.h>		/* voor de interruptESC */
#include <signal.h>
#include <fmclient.h>

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
	0,0,0,0,		/* misi, dofi, rt01, rt02; */
	1,0,0,1000,		/* fs,fg, rt2, mistdist */
	{0,0,0,0},		/* stex[4],*/
	{10,10,10,10},{0,0,0,0},/* smap[4], *sbase[4] */
	0,0,0, 0,			/* dofsta, dofend, dofmin, dofmax */
	1.0,			/* expos */
	0,0
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
	100, 78, 78, 78, 
	
	120,104,138,167, 
	121,31,31,40, 
	122,174,198,207, 

	124,162,162,162, 
	125,76,76,76, 
	126,226,226,226, 

	128,46,147,137, 
	129,0,56,0, 
	130,76,217,207, 

	132,152,125,160, 
	133,82,0,85, 
	134,222,185,230
};

float bezmat[4][4]={
	{-1.0, 3.0,-3.0, 1.0},
	{ 3.0,-6.0, 3.0, 0.0},
	{-3.0, 3.0, 0.0, 0.0},
	{ 1.0, 0.0, 0.0, 0.0},
};

struct Screen S;

ulong rect_desel[16], rect_sel[16];
ulong rectl_desel[81], rectl_sel[81];
ulong rectq_desel[81], rectq_sel[81], rectq_key[81];
short activeview = 1;

/* **************************************************** */

void error(char *str)
{
	char astr[100];
	
	if(G.background || (G.f & 2)) {  /* flag schakelt error uit */
		printf("ERROR: %s\n",str);
		return;
	}
	strcpy(astr, "ERROR: ");
	strcat(astr, str);
	confirm("", astr);
}

void sluit(type)
short type;
{
	extern char *obdup, *vfontbutstr;
	extern struct Bezier *copybez;
	extern long *copyK_data;
	struct Base *duplibase();
	struct VectorFont *vfont, *vfontn;
	long *adr;

	if(type==1 && okee("QUIT TRACES ")==0) return;

	if(G.winar[0]) {
		fullscrn();
		drawmode(OVERDRAW); 
		color(0); 
		clear(); 
		drawmode(NORMALDRAW);
		endfullscrn();
		G.winar[0]= 0; 
		G.winar[1]= 0;
		fmfreefont(G.font);
	}
	wisalles(0);
	freefastshade();
	FreeButs();
	duplibase(); /* geeft alle dupli's vrij */

	freeN(G.spline);
	freeN(G.workbase);
	freeN(G.adrtex[0]);
	freeN(G.adrtex);
	freeN(R.blove); 
	freeN(R.blovl);
	freeN(R.bloha);

	if(G.edve.first) freelist(&G.edve.first);
	if(G.eded.first) freelist(&G.eded.first);
	if(G.edvl.first) freelist(&G.edvl.first);
	free_hashedgetab();
	if(obdup) freeN(obdup); /* zit in verdit.c */
	if(copybez) freeN(copybez); /* zit in spline.c */
	if(copyK_data) freeN(copyK_data); /* zit in anim.c */
	free_filt_mask();  /* render.c */

	if(R.rectot) freeN(R.rectot);	/* komt niet meer van allocimbuf af */
	if(R.rectz) freeN(R.rectz);
	if(R.rectspare) freeN(R.rectspare);

	if(vfontbutstr) freeN(vfontbutstr);	/* in trabuts.c */
	vfont= (struct VectorFont *)G.vfbase.first;
	while(vfont) {
		vfontn= vfont->next;
		vfont->us= 1;
		freevfont(vfont);
		vfont= vfontn;
	}

	freeImBufdata();
	freeBGpic();

	freeAllika();

	if(totblock!=0) {
		printf("Error Totblck: %d\n",totblock);
		printmemlist();
	}
	gexit();
	if (G.afbreek == 1) exit(1);
	else exit(0);
}


void	pushpopdata(adr,len,mode)
char *adr;
long len,mode;
{
	static char *stack=0;

	if(mode==1) {	/* pop */
		if(stack==0) {
			error("pop error");
			sluit(0);
		}
		memcpy(adr,stack,len);
		freeN(stack);
		stack=0;
	} else {	/* push */
		if(stack) {
			error("push error");
			sluit(0);
		}
		stack=mallocN(len,"push");
		memcpy(stack,adr,len);
	}
}

void interruptESC(sig)
long sig;
{
	short toets,val,pause = FALSE;

	while(qtest() | pause) {
		toets= traces_qread(&val);
		if(val) {
			if(toets==ESCKEY) {
				G.afbreek = TRUE;
				pause = FALSE;
			} else if(toets==PAUSEKEY) {
				if (G.afbreek == FALSE) pause = ~pause;
				else pause = FALSE;
			}
			else if(toets==INPUTCHANGE) G.winakt=val;
			else if(toets==RIGHTMOUSE && G.winar[0]) {
				if(okee("Push windows")) {
					winset(G.winar[0]);
					winpush();
					winset(G.winar[1]);
					winpush();
					if(G.winar[9]) winset(G.winar[9]);
				}
			}
		}
	}
	/* opnieuw zetten: wordt namelijk gereset */
	signal(SIGVTALRM,interruptESC);
}


short testquESC()
{
	/* test op ESC event in queue, als INPUTCHANGE: winakt goedzetten,
	   INPUTCHANGE in q terugstoppen werkt niet omdat deze pas NA een
	   nieuw (ander) event wordt afgehandeld, =fout in GL?		*/

	short toets,val;

	while(qtest()) {
		toets= traces_qread(&val);
		if(val) {
			if(toets==ESCKEY) return 1;
			else if(toets==INPUTCHANGE) G.winakt=val;
			else if(toets==RIGHTMOUSE) {
				if(okee("Push windows")) {
					winset(G.winar[0]);
					winpush();
					winset(G.winar[1]);
					winpush();
					if(G.winar[9]) winset(G.winar[9]);
				}
			}
		}
	}

	return 0;
}

void setcursorN(cur)
short cur;
{
	extern short cursortype;    /* toets.c */

	if(!G.winar[0]) return;

	curstype(C16X1);
	if(cur==0) {
		winset(G.winar[1]);
		setcursor(0,0,0);
		winset(G.winar[0]);
		setcursor(0,0,0);
	}
	else if(cur==1) {
		winset(G.winar[1]);
		setcursor(0,0,0);
		winset(G.winar[0]);
		setcursor(1,0,0);
	}
	else if(cur==2) {
		winset(G.winar[1]);
		setcursor(2,0,0);
		winset(G.winar[0]);
		setcursor(2,0,0);
	}
	cursortype= cur;
}

short getcursorN()
{
	extern short cursortype;    /* toets.c */

	return cursortype;	
}

void setUserdefs()	/* o.a. vanuit file.c */
{
	long a;
	
	if(G.background) return;

	winset(G.winar[1]);

	mapcolor(User.back[0], User.back[1], User.back[2], User.back[3]);
	
	for(a=0; a<3; a++) {
		mapcolor(User.but0[a][0], User.but0[a][1], User.but0[a][2], User.but0[a][3]);
		mapcolor(User.but1[a][0], User.but1[a][1], User.but1[a][2], User.but1[a][3]);
		mapcolor(User.but2[a][0], User.but2[a][1], User.but2[a][2], User.but2[a][3]);
		mapcolor(User.but3[a][0], User.but3[a][1], User.but3[a][2], User.but3[a][3]);
	}

}

void ledata()
{
	struct Base *base;
	FILE *fp;
	long i;

	fp=fopen("/usr/people/normaaldata","r");
	if(fp==NULL) return;

	base=G.firstbase;
	while(base) {
		if(base->soort==1) {
			if( fscanf(fp,"%d",&i)==EOF ) break;
			if( fscanf(fp,"%d",&i)==EOF ) break;
			if( fscanf(fp,"%d",&i)==EOF ) break;
			if( fscanf(fp,"%d",&base->o[0])==EOF ) break;
			if( fscanf(fp,"%d",&base->o[1])==EOF ) break;
			if( fscanf(fp,"%d",&base->o[2])==EOF ) break;
			if( fscanf(fp,"%f",&base->q[0])==EOF ) break;
			if( fscanf(fp,"%f",&base->q[1])==EOF ) break;
			if( fscanf(fp,"%f",&base->q[2])==EOF ) break;
			if( fscanf(fp,"%f",&base->q[3])==EOF ) break;
		}
		base=base->next;
	}
	fclose(fp);
}


void mainbuts(event)
short event;
{
	extern short bgpicmode;

	if(event==103) {	/* cfra */
		loadkeyposall();
		if(G.mainb==7) tekenspline2d(0);
		if(G.script) doscript();
		projektie();
	}
	if(event==104) {	/* zbuf en drawtypebut */
		if(G.zbuf && bgpicmode) {
			error("No Zbuf and BGpic");
			G.zbuf= 0;
			SetButs(104, 104);
			return;
		}
		if(G.basact) makeDispList(G.basact);
		testDispLists();
		projektie();
	}
	if(G.mainb!=G.mainbo) {
		winset(G.winar[1]);
		frontbuffer(1);
		backbuffer(1);
		switch(G.mainb) {
		case 1:
			tekenviewbuts(0);
			break;
		case 4:
			if(G.basact==0 || G.basact->soort!=2) {
				G.mainb=G.mainbo;
				SetButs(100,100);
			} else tekenlampbuts(0);
			break;
		case 5:
			if(G.basact==0 || (G.basact->soort & 1)!=1) {
				G.mainb=G.mainbo;
				SetButs(100,100);
			} else tekenobjbuts(0);
			break;
		case 6:
			tekenwrldbuts(0);
			break;
		case 7:
			if(G.basact==0) {
				G.mainb=G.mainbo;
				SetButs(100,100);
			} else tekenmovebuts(0);
			break;
		case 8:
			tekendisplaybuts(0);
			break;
		case 9:
			if(G.basact==0) {
				G.mainb=G.mainbo;
				SetButs(100,100);
			} else tekeneditbuts(0);
			break;
		}
		G.mainbo=G.mainb;
	}
}

void getview(nr)
short nr;
{
	struct View *v;
	long a, *adr1,*adr2;

	v= view0.next;

	while(v) {
		if(v->num==nr) {
			adr1= (long *)&view0;
			adr2= (long *)v;
			memcpy(adr1+1,adr2+1,sizeof(struct View)-4);
			projektie();
			SetButs(100,199);
			activeview = nr;

			if( BTST(view0.lay,G.layact-1)==0 ) {
				for(a=0;a<16;a++) {
					if(BTST(view0.lay,a)) {
						G.layact=a+1;
						a=15;
					}
				}
			}

			return;
		}
		v= v->next;
	}
	error("No view stored");
}

void storeview()
{
	struct View *v;
	long *adr1,*adr2;
	short nr,button();

	nr = activeview;
	if( button(&nr,1,12,"Store View:")==0) return;

	v= view0.next;

	while(v) {
		if(v->num==nr) break;
		v= v->next;
	}

	if(v==0) {
		v= (struct View *)mallocN(sizeof(struct View),"storeview");
		v->next= view0.next;
		view0.next= v;
	} else{
		if (okee("Replace View") == 0) return;
	}
	activeview = nr;
	adr2= (long *)&view0;
	adr1= (long *)v;
	memcpy(adr1+1,adr2+1,sizeof(struct View)-4);
	v->num= nr;
}

void layerbuts(event)
short event;
{
	short a;

	if(G.localview) {
		view0.lay= 32768;
		SetButs(111,126);
		return;
	}
	if(G.qual & 48) {
		if(event>=0 && event<12) getview(event+1);
		return;
	}

	a=event;
	if(event>100) event-=111;
	else {
		if( BTST(view0.lay,event) )
			view0.lay=BCLR(view0.lay,event);
		else view0.lay=BSET(view0.lay,event);
	}

	if(G.qual==0 || view0.lay==0) {
		view0.lay=0;
		view0.lay= 1<<event;
		SetButs(111,126);
	}
	if(G.qual!=0 && a<100) SetButs(111,126); /* toets ingedrukt */
	if( BTST(view0.lay,G.layact-1)==0 ) {
		for(a=0;a<16;a++) {
			if(BTST(view0.lay,a)) {
				G.layact=a+1;
				a=15;
			}
		}
	}
	countall();

	testDispLists();
	reshadeall();

	projektie();
}


void windowmenu()
{
	long menu;
	short event;
	char str[100];

	strcpy(str,"TRACES %t|");
	strcat(str,"push buts %x10|");
	strcat(str,"pop buts %x11|");
	strcat(str,"push work %x12|");
	strcat(str,"pop work %x13|");
	strcat(str,"quit %x99");

	event= pupmenu(str);
	
	if(event==10) {
		winset(G.winar[1]); 
		winpush();
	}
	if(event==11) {
		winset(G.winar[1]); 
		winpop();
	}
	if(event==12) {
		winset(G.winar[0]); 
		winpush();
	}
	if(event==13) {
		winset(G.winar[0]); 
		winpop();
	}
	if(event==99) sluit(1);
}

short ffileselect(str1,str2)
char *str1,*str2;
{
	short val;

	val= fileselect(str1,str2);

	return(val);
}

void teststructs()
{

	printf("Base:    lengte: %d\n",sizeof(struct Base));
	printf("ObData:  lengte: %d\n",sizeof(struct ObData));
	printf("ColBlck: lengte: %d\n",sizeof(struct ColBlck));
	printf("ColBlckF:lengte: %d\n",sizeof(struct ColBlckF));
	printf("VV:      lengte: %d\n",sizeof(struct VV));
	printf("VertOb:  lengte: %d\n",sizeof(struct VertOb));
	printf("VlakOb:  lengte: %d\n",sizeof(struct VlakOb));
	printf("LaData:  lengte: %d\n",sizeof(struct LaData));
	printf("PerfSph: lengte: %d\n",sizeof(struct PerfSph));
	printf("PolyData:lengte: %d\n",sizeof(struct PolyData));
	printf("MoData:  lengte: %d\n",sizeof(struct MoData));
	printf("Bezier:  lengte: %d\n",sizeof(struct Bezier));
	printf("Key:     lengte: %d\n",sizeof(struct Key));
	printf("EditVert:lengte: %d\n",sizeof(struct EditVert));
	printf("EditEdge:lengte: %d\n",sizeof(struct EditEdge));
	printf("EditVlak:lengte: %d\n",sizeof(struct EditVlak));

	printf("Wrld:    lengte: %d\n",sizeof(struct Wrld));
	printf("View:    lengte: %d\n",sizeof(struct View));
	printf("Global:  lengte: %d\n",sizeof(struct Global));
	printf("But:     lengte: %d\n",sizeof(struct But));
	printf("Tex:     lengte: %d\n",sizeof(struct Tex));

	printf("VertRen: lengte: %d\n",sizeof(struct VertRen));
	printf("VlakRen: lengte: %d\n",sizeof(struct VlakRen));
	printf("LampRen: lengte: %d\n",sizeof(struct LampRen));
	printf("PolyRen: lengte: %d\n",sizeof(struct PolyRen));
	printf("Render:  lengte: %d\n",sizeof(struct Render));
	printf("Osa:     lengte: %d\n",sizeof(struct Osa));
	printf("Node:    lengte: %d\n",sizeof(struct Node));
	printf("Branch:  lengte: %d\n",sizeof(struct Branch));

/*
	Base:    lengte: 256
	ObData:  lengte: 80
	ColBlck: lengte: 64
	ColBlckF:lengte: 144
	VV:      lengte: 32
	VertOb:  lengte: 12
	VlakOb:  lengte: 16
	LaData:  lengte: 72
	PerfSph: lengte: 96
	PolyData:lengte: 68
	MoData:  lengte: 80
	Bezier:  lengte: 4
	Key:     lengte: 12
	EditVert:lengte: 32
	EditEdge:lengte: 24
	EditVlak:lengte: 36
	Wrld:    lengte: 100
	View:    lengte: 108
	Global:  lengte: 1196
	But:     lengte: 68
	Tex:     lengte: 152
	VertRen: lengte: 52
	VlakRen: lengte: 44
	LampRen: lengte: 220
	PolyRen: lengte: 28
	Render:  lengte: 400
	Osa:     lengte: 192
	Node:    lengte: 32
	Branch:  lengte: 32
 */
}

void initcursors(map)
unsigned short *map;
{
	short a;

	for(a=0;a<16;a++) map[a]= 256+128;
	map[7]=map[8]= 0xFFFF-512-256-128-64;
	map[6]=map[9]=0;
	defcursor(1,map);
	curorigin(1,8,8);

	for(a=0;a<16;a++) map[a]= 0xFFFF;
	defcursor(2,map);
	curorigin(2,8,8);

}

void readTlog()
{
	long file,len;
	char name[100],*home;

	home = getenv("HOME");
	if (home){
		strcpy(name,home);
		strcat(name,"/.Tlog");
		file=open(name, O_RDONLY);
		if (file >= 0){
			len = read(file,G.sce,sizeof(G.sce));
			close(file);
			if (len > 0) G.sce[len] = 0;
		}/* else perror("$HOME/.Tlog"); */
	} else perror("$HOME");
}

void testfunc()
{
	fmfonthandle helvfont;
	int rt;
	
	
	rt= winopen("rt");
	gconfig();
	
	
	fminit();
	helvfont= fmfindfont("Helvetica-Bold");

	
}

void openviewwin()
{
	static long firsttime= 1;

	prefposition(S.vxs, S.vxs+S.vxw-1, S.vys, S.vys+S.vyw-1 );

	noborder();
	G.winar[0]=G.winakt=winopen("Traces ");
	RGBmode();
	
	doublebuffer();
	gconfig();

	if(firsttime) {
		initsymbols();	/* op deze plek want rectread */
		firsttime= 0;
	}

	drawmode(PUPDRAW);
	color(0); 
	clear();
	mapcolor(1, 170, 170, 170); 
	mapcolor(2, 0, 0, 0); 
	mapcolor(3, 240, 240, 240);
	drawmode(OVERDRAW);
	color(0); 
	clear();
	mapcolor(1, 0, 0, 0); 
	mapcolor(2, 255, 0, 0); 
	mapcolor(3, 255, 255, 255);
	drawmode(CURSORDRAW);
	mapcolor(1, 255, 0, 0); 
	drawmode(NORMALDRAW);
	
	if(G.machine==ENTRY) cpack(0x708070);
	else cpack(0x707070); 
	clear();
	swapbuffers();
	clear();
	
	if(G.machine==ENTRY) linewidth(2);
	else linewidth(1);

	mmode(MVIEWING);
	Mat4One(G.persmat);
	loadmatrix(G.persmat);

	pushmatrix();
	swapinterval(2);
	
/*	voor geantialiaste wireframes

	subpixel(TRUE);
	linesmooth(SML_ON + SML_SMOOTHER + SML_END_CORRECT);
	blendfunction(BF_SA, BF_MSA);
*/

}

void openbuttonwin()
{

	prefposition(S.bxs, S.bxs+S.bxw-1, S.bys, S.bys+S.byw-1);

	noborder();
	G.winar[1]=winopen("Traces ");
	cmode();

	if(G.machine!=ENTRY) {
		doublebuffer();
		gconfig();
		frontbuffer(1);
		backbuffer(1);
		color(0); 
		clear();
		swapbuffers(); 
		clear();
	}
	else gconfig();

	drawmode(PUPDRAW);
	color(0); 
	clear();
	mapcolor(1, 170, 170, 170); 
	mapcolor(2, 0, 0, 0); 
	mapcolor(3, 240, 240, 240);
	drawmode(OVERDRAW);
	color(0); 
	clear();
	mapcolor(1, 0, 0, 0); 
	mapcolor(2, 255, 0, 0); 
	mapcolor(3, 255, 255, 255);
	drawmode(NORMALDRAW);

	linewidth(1);

	mmode(MSINGLE);

	ortho2(0,1279,0,223-1);
}


void traces_mem_error(block,error)
char *block,*error;
{
	fprintf(stderr,"Memoryblock %s: %s\n",block,error);
	mapcolor(100,255,0,0);
}


void breekaf(int sig)
{
	G.afbreek = 2;
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
	long file, dopri = FALSE;
	short a, mval[2], cursmap[16], xmax, ymax, debugflag=0;
	char pic[100], tstr[100], *home;
	
	if (argc == 2) {
		if (strcmp(argv[1], "debug") == 0){
			foreground();
			argc = 1;
		}
	}

	S.bxw= 0;

	/* testfunc(); */

	/* BACKGROUND? */
	if(argc>1) {
		for(a=1; a<argc; a++) {
			if(argv[a][0] == '-') {
				if(toupper(argv[a][1]) == 'B') {
					if (argv[a][1] == 'b') dopri = TRUE;
					G.background= 1;
					printf("TRACES Version %s\n", versionstr+14);
					/*foreground();*/
				}
				else if (strcmp(argv[a], "-PAL") == 0) {
					S.bxs= 0;	S.bys= 0;
					S.bxw= 780; S.byw= 135;
					S.vxs= 0;	S.vys= 136;
					S.vxw= 780; S.vyw= 439;
				}
				else if(strcmp(argv[a], "-debug") == 0) {
					debugflag= 1;
				}
			}
		}
	}

	/* INIT */
	
	if (G.background != 1) {
		gversion(tstr);
		G.machine= 0;
		if(strncmp(tstr, "GL4DPI", 6)==0) G.machine=IRIS;
		else if(strncmp(tstr, "GL4DXG", 6)==0) G.machine=ELAN;
		else if(strncmp(tstr, "GL4DLG", 6)==0) G.machine=ENTRY;
		else G.machine=ENTRY;
		/* Indy entry: GL4DNP */
	}
	
	/* mallopt(M_MXFAST,sizeof(struct EditVert)+4);	 */
	/* blokken kleiner dan (32+4) snel uitdelen (t.o.v 28 standaard) */
	/* mallopt(M_MXCHK, 16*2048);	  */
	/* normaal zoekt 'ie max. 100 blokken af voor ie opgeeft. Onzin */
	/* mallopt(M_FREEHD,1); */

	if(debugflag==0) {
		sigset(SIGBUS, savecore);	/* in de funktie savecore moet dit weer worden ongedaan gemaakt */
		sigset(SIGSEGV, savecore);
	}
	
	sigset(SIGUSR2, savecore);	/* als er een SIGUSR2 gegeven word dump dan de file */
	sigset(SIGUSR1, breekaf);   /* nette kill gebruikt door tracesdeamon */
	
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
	G.obszoom= 0;

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
	R.blove= (struct VertRen **)callocN(4*(MAXVERT>>8),"Blove");
	R.blovl= (struct VlakRen **)callocN(4*(MAXVLAK>>8),"Blovl");
	R.bloha= (struct HaloRen **)callocN(4*(MAXVERT>>8),"Bloha");
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

	/* ook in backgroundrender doen ivm border */
	if(S.bxw==0) {	/* niet door -PAL gezet */
		S.x= xmax = 1280;
		S.y= ymax = 1024;
		
		if (G.background != 1) {
			S.x= xmax = getgdesc(GD_XPMAX);
			S.y= ymax = getgdesc(GD_YPMAX);
		}
		S.bxs= 0;	S.bys= 0;
		S.bxw= xmax; S.byw= 0.218*ymax;
		S.vxs= 0;	S.vys= S.byw;
		S.vxw= xmax; S.vyw= ymax-S.vys-1;
	}
		
	if(!G.background) {
	
		setuid(getuid()); /* einde superuser */
		memory_error = traces_mem_error;
	
		openviewwin();
		deflighting();	/* op deze plek ivm mmode */
	
		openbuttonwin();
		
		tekenmainbuts(1);	/* 1 betekent init, 0 is redraw  */

		/*decodekeytab();*/

		qdevice(LEFTMOUSE);
		qdevice(MIDDLEMOUSE);
		qdevice(RIGHTMOUSE);
		qdevice(ESCKEY);
		qdevice(WINSHUT); 
		qdevice(WINQUIT);
		qdevice(RAWKEYBD);
		qdevice(RIGHTARROWKEY);
		qdevice(LEFTARROWKEY);
		qdevice(PAUSEKEY);
		qdevice(WINFREEZE);
		qdevice(WINTHAW);
		qdevice(DRAWOVERLAY);
		
		/* INIT */
		deflinestyle(1, 0xF0F0);
		deflinestyle(2, 0x0F0F);
		deflinestyle(3, 0x000F);
		deflinestyle(4, 0xFF18);
		
		initcursors(cursmap);
		initkubs();  /* fastdraws */
		defbasis(1,bezmat);

		G.vfbase.first= G.vfbase.last= 0;
		vf= initvfont("/usr/people/trace/font/u/Univers/Univers");
		if(vf) vf->us= 1;		
		vf= initvfont("/usr/people/trace/font/g/GillSans/GillSans");
		if(vf) vf->us= 1;
		vf= initvfont("/usr/people/trace/font/f/Futura/Futura");
		if(vf) vf->us= 1;
	}

	initeffects(); /* uit effect.c */

	/*START*/

	if(G.background) {
		if (dopri) {
			if (schedctl(NDPRI, 0, NDPLOMIN - 2) == -1) printf("no ndpri \n");
			if (schedctl(SLICE, 0, 1) == -1) printf("no slice \n");
		}
		
		setuid(getuid()); /* einde superuser */
		
		G.winar[0]=G.winar[1]= 0;
		for(a=1; a<argc; a++) {
			if(argv[a][0] == '-') {
				switch(argv[a][1]) {
				case 'w':
					G.background= 2;
					break;
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
				case 's':
					if (++a < argc) G.sfra = atoi(argv[a]);
					break;
				case 'e':
					if (++a < argc) G.efra = atoi(argv[a]);
					break;
				}
			}
			else {
				leesscene(argv[a], 0);
			}
		}
		/*if (signalproc != 0 && G.afbreek == FALSE) kill(signalproc, SIGUSR1);*/
		sluit(0);
	}

	file= 0;
	home = getenv("HOME");
	if (home) {
		strcpy(tstr,home);
		strcat(tstr,"/.T.trace");
		file= open(tstr, O_RDONLY);
		if(file>0) {
			leesscene(tstr);
		}
	}
	setUserdefs();	/* o.a. cmap */
	
	readTlog();

	bepaalphitheta();
	
	for(a=1; a<argc; a++) {
		if (argv[a][0]!='-') leesscene(argv[a], 0);
	}
	projektie();

	qreset();
	setvaluator(MOUSEX, S.vxw/2, 0, S.vxw);
	setvaluator(MOUSEY, (S.vyw+S.vys)/2, 0, S.vyw+S.vys);

	mainlus();

}

