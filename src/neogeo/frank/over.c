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

#include <sys/types.h>

#include <local/iff.h>
#include <local/storage.h>
#include <local/util.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <string.h>
#include <stdio.h> 
#include <fcntl.h>

/*
cc -O2 over.c util.o -lgl -lX11 -limbuf -lm -o over

zipfork "cc -O3 over.c storage.o util.o -lgl_s -limbuf -limage -lm -o over > /dev/console"                                                     
zipfork "overrabo /pics/rabo2/back.00 /pics/rabo2/front.02 /pics/rabo2/back.02"                                    

*/


void over(brect,frect,x)
long *brect,*frect,x;
{
	long col;
	for(;x>0;x--){
		col =  *(frect++);
		if (col < 0) *brect = col;
		brect++;
	}
}

void over1(brect,frect,x)
long *brect,*frect,x;
{
	long col;
	for(;x>0;x--){
		col =  *(frect++);
		if (col != 0) *brect = col;
		brect++;
	}
}

/*
void drop(brect,frect,x)
long *frect,x;
uchar *brect;
{
	long col;

	for(;x>0;x--){
		col =  *(frect++);
		if (col >= 0){

			if (brect[1] > 16){
				brect[1] -= 16;
			}else{
				brect[1] = 0;
			}
			if (brect[2] > 16){
				brect[2] -= 16;
			}else{
				brect[2] = 0;
			}
			if (brect[3] > 16){
				brect[3] -= 16;
			}else{
				brect[3] = 0;
			}
		}
		brect+=4;
	}
}

*/

void drop(brect,frect,x)
long *frect,x;
long *brect;
{
	long col;

	for(;x>0;x--){
		col =  *(frect++);
		if (col < 0) brect[0] -= (brect[0] & 0xfcfcfc) >> 2;
		brect++;
	}
}



void doitscanl(brect,frect,x,col)
char *brect,*frect;
long x,col;
{
	short col1,col2,r,g,b;

	r = col & 0x0ff;
	g = (col>>8) & 0x0ff;
	b = (col>>16) & 0x0ff;
	
	for(; x>0 ; x--){
		frect++ ; brect++;

		if (col1 = *(frect++)) {
			 col2 = *(brect);
			*(brect++) = ((col2 << 8) + col1 * (b - col2)) >> 8;
		}else brect++;

		if (col1 = *(frect++)) {
			 col2 = *(brect);
			*(brect++) = ((col2 << 8) + col1 * (g - col2)) >> 8;
		}else brect++;

		if (col1 = *(frect++)) {
			 col2 = *(brect);
			*(brect++) = ((col2 << 8) + col1 * (r - col2)) >> 8;
		}else brect++;
	}
}


void doit(bbuf,fbuf,ofsx,ofsy,function)
struct ImBuf *bbuf,*fbuf;
long ofsx,ofsy;
void (*function)();
{
	long lines,pixels;
	long fx,fy,bx,by;
	ulong *brect,*frect;
	
	brect = bbuf->rect ; bx = bbuf->x ; by = bbuf->y;
	frect = fbuf->rect ; fx = fbuf->x ; fy = fbuf->y;
	
	if (ofsy > 0){
		brect += ofsy * bx;
		by -= ofsy;
	}else{
		frect -= ofsy * fx;
		fy += ofsy;
	}

	if (ofsx > 0){
		brect += ofsx;
		bx -= ofsx;
	}else{
		frect -= ofsx;
		fx += ofsx;
	}
	
	if (fy < by) lines = fy;
	else lines = by;

	if (fx < bx) pixels = fx;
	else pixels = bx;

	if (lines <= 0 ) return;
	if (pixels <= 0) return;

	for (;lines > 0; lines--){
		function(brect,frect,pixels);
		brect += bbuf->x;
		frect += fbuf->x;
	}
}


main_oud(argc,argv)
int argc;
char **argv;
{
	char name[100],headin[100],tailin[100],headout[100],tailout[100],backpic[100], frontpic[100], outpic[100];
	char this[100];
	long picin,picout,win = 0;
	ushort lenin,lenout;
	struct ImBuf *bbuf,*fbuf, *tbuf,*backbuf, *frontbuf;
	long ofsx,ofsy;
	long frontanim = TRUE;
	
	long i,*rect;

	if (argc == 1){
		strcpy(name,"/pics/");

		if (fileselect("Backpic",name) == 0) exit(0);
		strcpy(backpic,name);
		if (fileselect("Over pics",name) == 0) exit(0);
		strcpy(frontpic,name);
		if (fileselect("Output pics",name) == 0) exit(0);
		strcpy(outpic, name);
	}else if (argc < 4){
		printf("usage: %s backpic frontpic outpic\n",argv[0]);
		exit(1);
	}else{
		strcpy(backpic,argv[1]);
		strcpy(frontpic,argv[2]);
		strcpy(outpic,argv[3]);
	}
	
	if (frontanim) picin = stringdec(frontpic,headin,tailin,&lenin);
	else picin = stringdec(backpic,headin,tailin,&lenin);

	picout = stringdec(outpic,headout,tailout,&lenout);

	if (frontanim) {
		backbuf = loadiffname(backpic,LI_rect);
		if (backbuf == 0) {
			printf("couldn't load %s\n",backpic);
			exit(1);
		}
		tbuf = backbuf;
	}else{
		frontbuf = loadiffname(frontpic, LI_rect);
		if (frontbuf == 0) {
			printf("couldn't load %s\n",frontpic);
			exit(1);
		}
		tbuf = frontbuf;
	}
	
	if (argc == 5){
		foreground();
		prefsize(tbuf->x,tbuf->y);
		win=winopen("Over");
		RGBmode();
		gconfig();
		lrectwrite(0,0,tbuf->x-1,tbuf->y-1,tbuf->rect);
	}

	while(1){
		stringenc(name,headin,tailin,lenin,picin);
		picin++;
		
		tbuf = loadiffname(name,LI_rect);
		if (tbuf == 0) exit(0);
		printf("Loaded: %s\n",tbuf->name);

		if (frontanim){
			fbuf = tbuf;
			bbuf = dupImBuf(backbuf);
		}else{
			bbuf = tbuf;
			fbuf = dupImBuf(frontbuf);
		} 
		
		ofsx = (bbuf->x - fbuf->x ) >>1;
		ofsy = (bbuf->y - fbuf->y ) >>1;

		doit(bbuf,fbuf,ofsx +1, ofsy+1,drop);
		doit(bbuf,fbuf,ofsx -1, ofsy+1,drop);

		doit(bbuf,fbuf,ofsx +1, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -1, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -3, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -5, ofsy-1,drop);

		doit(bbuf,fbuf,ofsx -1, ofsy-3,drop);
		doit(bbuf,fbuf,ofsx -3, ofsy-3,drop);
		doit(bbuf,fbuf,ofsx -5, ofsy-3,drop);

		doit(bbuf,fbuf,ofsx -3, ofsy-5,drop);
		doit(bbuf,fbuf,ofsx -2, ofsy-2,drop);

		if (argc == 5) lrectwrite(0,0,bbuf->x-1,bbuf->y-1,bbuf->rect);
		doit(bbuf,fbuf,ofsx,ofsy, over);
		if (argc == 5) lrectwrite(0,0,bbuf->x-1,bbuf->y-1,bbuf->rect);

		stringenc(this,headout,tailout,lenout,picout);

		if (saveiff(bbuf,this,SI_rect) == 0) exit(0);

		printf("Saved: %s\n",this);
		picout++;

		freeImBuf(fbuf);
		freeImBuf(bbuf);
	}
}


main(argc,argv)
int argc;
char **argv;
{
	char progname[100], name[100];

	char headfrnt[128], tailfrnt[128], frnt[256];
	ushort lenfrnt;
	long picfrnt;

	char headback[128], tailback[128], back[256];
	ushort lenback;
	long picback;
	
	char headout[128], tailout[128], out[256];
	ushort lenout;
	long picout;

	long win = 0, new, first, flags = IB_rect;
	struct ImBuf *bbuf, *fbuf, *backbuf, *frntbuf;
	long ofsx, ofsy;	
	long i,*rect;
	
	char fb[2] = {'a', 'a'};
	
	strcpy(progname, argv[0]);
	
	if (argc == 1){
		strcpy(name,"/pics/");

		if (fileselect("Backpic",name) == 0) exit(0);
		strcpy(back,name);
		if (fileselect("Over pics",name) == 0) exit(0);
		strcpy(frnt,name);
		if (fileselect("Output pics",name) == 0) exit(0);
		strcpy(out, name);
	} else {
		if (argv[1][0] == '-') {
			fb[0] = argv[1][1];
			fb[1] = argv[1][2];
			argc--;argv++;
		}
		if (argc < 4){
			printf("usage: %s [ -aa | -ap | -pa | -pp ] backpic frontpic outpic\n",progname);
			printf("where a = auto, p = picture. Default = -aa \n");
			printf("first = back, second = front\n");
			exit(1);
		}else{
			strcpy(back,argv[1]);
			strcpy(frnt,argv[2]);
			strcpy(out,argv[3]);
		}
	}
	
	picfrnt = stringdec(frnt, headfrnt, tailfrnt, &lenfrnt);
	picback = stringdec(back, headback, tailback, &lenback);
	picout = stringdec(out, headout, tailout, &lenout);

	if (fb[0] == 'p') lenback = 0;
	if (fb[1] == 'p') lenfrnt = 0;
	
	if (argc == 5){
		bbuf = loadiffname(back, flags);
		if (bbuf == 0) {
			perror(back);
			exit(0);
		}
		foreground();
		prefsize(bbuf->x,bbuf->y);
		win=winopen("Over");
		RGBmode();
		gconfig();
		sginap(1);
		lrectwrite(0,0,bbuf->x-1,bbuf->y-1,bbuf->rect);
		freeImBuf(bbuf);
	}

	first = TRUE;

	/* HACK ALERT */	
	if( strcmp(progname, "overjoeri")==0 ) flags |= IB_cmap;

	while(1){
		new = FALSE;
		fbuf = bbuf = 0;
		
		/* VOORGROND PLAATJE LADEN */
		
		if (lenfrnt) {
			stringenc(name, headfrnt, tailfrnt, lenfrnt, picfrnt++);
			fbuf = loadiffname(name, flags);
		}
		
		if (fbuf == 0) {
			lenfrnt = 0;
			if (frntbuf == 0) {
				frntbuf = loadiffname(frnt, flags);
				if (frntbuf == 0) {
					printf("couldn't load %s\n", frnt);
					exit(0);
				}
			}
			fbuf = dupImBuf(frntbuf);
		} else new = TRUE;
		strcpy(frnt, fbuf->name);
		
		/* ACHTERGROND PLAATJE LADEN */
		
		if (lenback) {
			stringenc(name, headback, tailback, lenback, picback++);
			bbuf = loadiffname(name, flags);
		}
		
		if (bbuf == 0) {
			lenback = 0;
			if (backbuf == 0) {
				backbuf = loadiffname(back, flags);
				if (backbuf == 0) {
					printf("couldn't load %s\n", back);
					exit(0);
				}
			}
			bbuf = dupImBuf(backbuf);
		} else new = TRUE;
		strcpy(back, bbuf->name);
		
		if (new == FALSE && first == FALSE) break;
		
		printf("using %s %s\n", bbuf->name, fbuf->name);

		ofsx = (bbuf->x - fbuf->x ) >>1;
		ofsy = (bbuf->y - fbuf->y ) >>1;

if(ofsx== 120120) {		/* HACK ALERT */
		doit(bbuf,fbuf,ofsx +1, ofsy+1,drop);
		doit(bbuf,fbuf,ofsx -1, ofsy+1,drop);

		doit(bbuf,fbuf,ofsx +1, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -1, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -3, ofsy-1,drop);
		doit(bbuf,fbuf,ofsx -5, ofsy-1,drop);

		doit(bbuf,fbuf,ofsx -1, ofsy-3,drop);
		doit(bbuf,fbuf,ofsx -3, ofsy-3,drop);
		doit(bbuf,fbuf,ofsx -5, ofsy-3,drop);

		doit(bbuf,fbuf,ofsx -3, ofsy-5,drop);
		doit(bbuf,fbuf,ofsx -2, ofsy-2,drop);
}		
		if (argc == 5) lrectwrite(0,0,bbuf->x-1,bbuf->y-1,bbuf->rect);
		doit(bbuf, fbuf, ofsx, ofsy, over1);
		if (argc == 5) lrectwrite(0,0,bbuf->x-1,bbuf->y-1,bbuf->rect);

		if (lenout) stringenc(name,headout,tailout,lenout,picout);
		else strcpy(name, out);
		
/* HACK ALERT */
		if( strcmp(progname, "overjoeri")==0 ) {
			memcpy(bbuf->cmap, fbuf->cmap, 4*fbuf->maxcol);
			memmove(bbuf->cmap, bbuf->cmap+16, 48*4);
		}

		if (saveiff(bbuf, name, flags) == 0) exit(0);

		printf("Saved: %s\n", name);
		
		if(lenout==0) exit(0);
		
		picout++;

		freeImBuf(fbuf);
		freeImBuf(bbuf);
		first = FALSE;
	}
}

