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



/* usiblender.c  jan 94		GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "file.h"
#include <sys/schedctl.h>
#include <signal.h>

char *bprogname;
UserDef U;

void inituserdef()
{
	U.flag= 0;
	U.savetime= 5;
	strcpy(U.tempdir, "/usr/tmp/");
	
}

void exit_usiblender()
{
	extern char *textbuf, *oldstr;	/* editfont.c */
	extern char *fsmenu;	/* filesel.c */
	extern ImBuf * izbuf;
	
	if(G.background && G.scene) {		/* network patch */
		end_camera_network();
		end_sectors();
	}
	free_blender();	/* blender.c, doet hele library */
	free_hashedgetab();
	FreeButs();
	free_ipocopybuf();
	freefastshade();
	free_vertexpaint();
	
	/* editnurb kan blijven bestaan buiten editmode */
	freeNurblist(&editNurb);
	 
	if(G.obedit) {
		if(G.obedit->type==OB_FONT) freeN(oldstr);
		else if(G.obedit->type==OB_MBALL) freelistN(&editelems);
		free_editmesh();
	}
	else {
		if(textbuf) freeN(textbuf);
	}
	
	free_editLatt();
	if(fsmenu) freeN(fsmenu);
	
	free_render_data();
	free_filt_mask();
	
	if (izbuf) freeImBuf(izbuf);
	izbuf = 0;
	
	if(totblock!=0) {
		printf("Error Totblck: %d\n",totblock);
		printmemlist();
	}
	gexit();
	delete_autosave();
	printf("Blender quit\n");
	
	exit(G.afbreek);
}



void initcursors()
{
	static unsigned short map[32];
	short a;

	/* cursor 1= kruisje
	 * cursor 2= wait
	 * cursor 3= vpaint
	 * cursor 4= faceselect
	 */

	if(G.machine!=IRIS) curstype(C16X2);
	for(a=0; a<32; a++) map[a]= 0;
	
	/* wait */
	for(a=0;a<16;a++) map[a]= 0xFFFF;
	defcursor(2, map);
	curorigin(2, 8, 8);
	
	/* kruisje */
	for(a=0;a<16;a++) map[a]= 256+128;
	map[7]=map[8]= 0xFFFF-512-256-128-64;
	map[6]=map[9]=0;
	defcursor(1,map);
	curorigin(1, 8, 8);

	if(G.machine==IRIS) {	
		/* vpaint */
		defcursor(2, map);
		curorigin(2, 8, 8);
		/* facesel */
		defcursor(3, map);
		curorigin(3, 8, 8);
	}
	else {
		/* vpaint */
		for(a=0;a<16;a++) map[a]= 0;
		for(a=16;a<32;a++) map[a]= 256+128;
		map[7+16]=map[8+16]= 0xFFFF-512-256-128-64;
		map[6+16]=map[9+16]=0;
		
		defcursor(3, map);
		curorigin(3, 8, 8);
		
		/* facesel */
		for(a=0;a<16;a++) map[a]= 256+128;
		for(a=16;a<32;a++) map[a]= 256+128;
		map[7]=map[8]= 0xFFFF-512-256-128-64;
		map[6]=map[9]=0;
		map[7+16]=map[8+16]= 0xFFFF-512-256-128-64;
		map[6+16]=map[9+16]=0;
	
		defcursor(4, map);
		curorigin(4, 8, 8);
	}	
}

void initbuttons()
{
	struct ImBuf *bbuf, *pbuf;
	fmfonthandle helvfont;
	char str[256];
	
	fminit();
	if( (helvfont=fmfindfont("Helvetica-Narrow-Bold")) == 0) {
		printf("Can't find font Helvetica-Narrow-Bold\n");
		exit_usiblender();
	}
	
	if(G.machine==ENTRY) {
		G.font= fmscalefont(helvfont,13.0);

		helvfont=fmfindfont("Helvetica-Bold");
		G.fonts= fmscalefont(helvfont, 9.0);
	}
	else if(G.machine==IRIS) {
		G.font= fmscalefont(helvfont,11.0);

		helvfont=fmfindfont("Helvetica-Bold");
		G.fonts= fmscalefont(helvfont, 8.0);
	}
	else {
		G.font= fmscalefont(helvfont,11.0);

		helvfont=fmfindfont("Helvetica-Bold");
		G.fonts= fmscalefont(helvfont, 8.5);
	}
	/* else { */
	/* 	G.font= fmscalefont(helvfont,13.0); */

	/* 	helvfont=fmfindfont("Helvetica-Bold"); */
	/* 	G.fonts= fmscalefont(helvfont, 8.5); */
	/* } */
	
	
	
	fmsetfont(G.font);
	AutoButFontSize(G.fonts, G.fontss);
	
	/* kleuren voor headerbuttons */
	DefButCol(1, PUPDRAW, 0,1,2,3,2, 2,3,0,0);
	DefButCol(2, RGBDRAW, 0x909090, 0xA0A0A0, 0, 0xFFFFFF, 0);	/* grijs */
	DefButCol(3, RGBDRAW, 0x909090, 0x80A080, 0, 0xFFFFFF, 0);	/* groen */
	DefButCol(4, RGBDRAW, 0x909090, 0xA08080, 0, 0xFFFFFF, 0);	/* blauw */
	DefButCol(5, RGBDRAW, 0x909090, 0x80A0B0, 0, 0xFFFFFF, 0);	/* zalm */
	DefButCol(6, RGBDRAW, 0x909090, 0x808080, 0, 0xFFFFFF, 0);	/* middelgrijs */
	DefButCol(7, RGBDRAW, 0x909090, 0x4040B0, 0, 0x0000FF, 0);	/* red alert */

	/* kleuren voor blenderbuttons */
	DefButCol(10, RGBDRAW, 0x606060, 0x909090, 0, 0xFFFFFF, 0);	/* grijs */
	DefButCol(11, RGBDRAW, 0x606060, 0x949070, 0, 0xFFFFFF, 0);	/* groen */
	DefButCol(12, RGBDRAW, 0x606060, 0xA09090, 0, 0xFFFFFF, 0);	/* blauw */
	DefButCol(13, RGBDRAW, 0x606060, 0x8090A0, 0, 0xFFFFFF, 0);	/* zalm */
	DefButCol(14, RGBDRAW, 0x607070, 0x998892, 0, 0xFFFFFF, 0);	/* paars */

	/* IKONEN INLADEN */
	
	sprintf(str, "%s/blenderbuttons", getenv("BLENDERDIR"));
	bbuf= loadiffname(str , LI_rect);
	if(bbuf==0) {
		printf("Can't find buttonimage\n");
		exit(0);
	}
	DefButIcon(0, bbuf->rect, bbuf->x, bbuf->y, 20, 21);
	bbuf->rect= 0;
	freeImBuf(bbuf);

	#ifndef FREE
	sprintf(str, "%s/bpaintbuttons", getenv("BLENDERDIR"));
	pbuf= loadiffname(str , LI_rect);
	DefButIcon(1, pbuf->rect, pbuf->x, pbuf->y, 25, 25);
	pbuf->rect= 0;
	freeImBuf(pbuf);
	#endif
	
}

void initpatterns()
{
	ushort pat[16], x;

	deflinestyle(1, 0xF0F0);
	deflinestyle(2, 0x0F0F);
	deflinestyle(3, 0x000F);
	deflinestyle(4, 0xFF18);
	deflinestyle(5, 0x5555);

	for(x=0; x<16; x+=2) pat[x]=0x5555;
	for(x=1; x<16; x+=2) pat[x]=0xAAAA;
	defpattern(1, 16, pat);
}	


void breekaf(int sig)
{
	static int count = 0;
	
	G.afbreek = 2;
	
	if (sig == 2) {
		if (G.renderd == FALSE) {
			if (count) exit(2);
			printf("Press ^C again if once doesn't work\n");
		} else {
			if (count == 20) exit(2);
		}
		count++;
	}
}


void savecore()
{
	char scestr[120], tstr[120];
	extern int noBlog;
	extern void write_file();
	
	noBlog= 1;		
	strcpy(scestr, G.sce);	/* even bewaren */
	G.f |= G_DISABLE_OK;
	
	write_file("/usr/tmp/core.blend");
	
	G.f &= ~G_DISABLE_OK;
	strcpy(G.sce, scestr);
	noBlog= 0;
}


void main(argc,argv)	/* deze mainfunktie alleen voor editor, background krijgt andere */
long argc;
char **argv;
{
	Scene *sce;
	extern char DNAstr[];
	int a, stax, stay, sizx, sizy;
	char tstr[100];
	
	bprogname= argv[0];	/* voor play anim */
	
	initglobals();	/* blender.c */
	
	
	/* eerste testen op background */
	for(a=1; a<argc; a++) {
		if(argv[a][0] == '-') {
			switch(argv[a][1]) {
			case 'a':
				main_playanim(argc-1, argv+1);
				exit(0);
				break;
			case 'b':
				/* background met lage prioriteit */
				
				if (schedctl(NDPRI, 0, NDPLOMIN - 2) == -1) printf("no ndpri \n");
				/*if (schedctl(SLICE, 0, 1) == -1) printf("no slice \n");*/
				schedctl(SLICE, 0, 1);
				
			case 'B':
				/* background met normale prioriteit */

				G.background = 1;
				a = argc;
				break;
			}
		}
	}

	setuid(getuid()); /* einde superuser */
	
	/* Signalen zetten. Even wachten met USR1 en INT, want die kunnen
	 * ook het lezen van de file onderbreken. Geeft van die slordige
	 * foutmeldingen....
	 */
	
	sigset(SIGUSR2, savecore);	/* als er een SIGUSR2 gegeven word dump dan de file */
	sighold(SIGUSR1);
	sighold(SIGINT);
	sigset(SIGUSR1, breekaf);   /* nette kill gebruikt door [traces|render]daemon */
	sigset(SIGINT, breekaf);   /* nette kill gebruikt door [traces|render]daemon */

	bzero(&cur_sdna, sizeof(struct SDNA));
	cur_sdna.data= DNAstr+4;
	cur_sdna.datalen= *( (int *)DNAstr);
	init_structDNA(&cur_sdna);

	init_render_data();	/* moet vooraan staan ivm R.winpos uit defaultfile */

	if(G.background==0) {
		getdisplaysize();
		
		for(a=1; a<argc; a++) {
			if(argv[a][0] == '-') {
				switch(argv[a][1]) {
				case 'p':	/* prefsize */
					a++;
					stax= atoi(argv[a]);
					a++;
					stay= atoi(argv[a]);
					a++;
					sizx= atoi(argv[a]);
					a++;
					sizy= atoi(argv[a]);
	
					setprefsize(stax, stay, sizx, sizy);
					a= argc;
					break;
				case 'd':	/* dirview */
					break;
				case 'f':	/* dirview */
					foreground();
					break;
				}
			}
		}

		inituserdef();

		if( getenv("BLENDERDIR")==0 ) {
			printf("error: BLENDERDIR environment variable not set\n");
			exit(0);
		}
		
		load_firstfont();
		if(G.main->vfont.first==0) {
			printf("can't find default vectorfont\n");
			exit(0);
		}

		gversion(tstr);
		G.machine= ELAN;
		
		/* entry staat voor 8 bits, 2x4 bits doublebuffer ! */

/*
Crimson RealityEngine	GL4DRE-6.2
Crimson/VGXT		GL4DVGX-6.2
Indigo/Elan		GL4DXG-6.2
Indy/XL24		GL4DNP-6.2
*/		
		if(strncmp(tstr, "GL4DPI", 6)==0) G.machine= IRIS;
		else if(strncmp(tstr, "GL4DXG", 6)==0) G.machine= ELAN;
		else if(strncmp(tstr, "GL4DLG", 6)==0) G.machine= ENTRY;
		else if(strncmp(tstr, "GL4DS", 5)==0) G.machine= M_O2; /* (dit is de octane GL4DSLD) */
		else if(strncmp(tstr, "GL4DNP", 6)==0) G.machine= M_O2; /* (dit is de indy) */
		else if(strncmp(tstr, "GL4DC", 5)==0) G.machine= M_O2;

		initpatterns();
		initscreen();	/* voor (visuele) snelheid, dit eerst, dan setscreen */
		initcursors();
		init_screen_cursors();
		initbuttons();
		set_ipofont();	/* drawipo.c */
		deflighting();
		
		read_homefile();

		readBlog();
		strcpy(G.lib, G.sce);
		
		
	}

	init_filt_mask();
	
	/* OK we zijn er klaar voor */
	
	sigrelse(SIGUSR1);
	sigrelse(SIGINT);

	for(a=1; a<argc; a++) {
		if (G.afbreek) break;
		
		if(argv[a][0] == '-') {
			switch(argv[a][1]) {
			case 'p':	/* prefsize */
				a+= 4;
				break;
			case 'I':	/* lees standard in voor nieuwe frames */
				if (G.scene) {
					G.renderd = TRUE;
					
					G.real_sfra = SFRA;
					G.real_efra = EFRA;
					
					printf("blender: reading stdin\n");
					fflush(stdout);
					while (gets(tstr)) {
						if (G.afbreek) break;
						
						if (strcmp(tstr, "EXIT") == 0) {
							EFRA= G.real_efra;
							SFRA= G.real_sfra;
							exit_render_stuff();
							
						} else {
							SFRA = atoi(tstr);
							EFRA = SFRA;
							animrender();
						}
						
						/* zorg ervoor dat alle printf's aangekomen zijn */
						fflush(stdout);
						fflush(stderr);
						sginap(10);

						if (G.afbreek) break;
						/* zend signaal naar ouder dat ik klaar ben */
						if (getppid()) kill(getppid(), SIGUSR1);
					}
				}
				break;
			case 'f':
				a++;
				if (G.scene) {
					G.real_sfra = SFRA;
					G.real_efra = EFRA;
					SFRA = atoi(argv[a]);
					EFRA = SFRA;
					animrender();
				}
				break;
			case 'a':
				if (G.scene) {
					G.real_sfra = SFRA;
					G.real_efra = EFRA;
					animrender();
					exit_render_stuff();
				}
				break;
			case 'v':
				G.machine= VIDEO;
				printf("videomode\n");
				break;
			case 'S':
				if(++a < argc) {
					set_scene_name(argv[a]);
				}
				break;
			case 's':
				if(G.scene) {
					if (++a < argc) SFRA = atoi(argv[a]);
				}
				break;
			case 'e':
				if(G.scene) {
					if (++a < argc) EFRA = atoi(argv[a]);
				}
				break;
			}
		}
		else {
			read_file(argv[a]);
		}
	}
	
	if(G.background) exit_usiblender();

	
	if(G.main->scene.first==0) {
		sce= add_scene("1");
		set_scene(sce);
	}
	
	qreset();	/* anders queue in de war, korrekte aktieve area berekening */
	
	setscreen(G.curscreen);
	
	bgnpupdraw();	/* init toolbox */
	endpupdraw();

	screenmain();
}

