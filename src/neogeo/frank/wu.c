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

#include <local/iff.h>

#include <gl/image.h>
#include <local/util.h>
#include <local/gl_util.h>
#include <local/Button.h>
#include <fmclient.h>

/*

bindkeyo -r f1,'make wu \n'
bindkeyo -r f2,'time wu \n'

*/

#define float double

struct WU_box
{
	struct WU_box *next,*prev;
	short num[4];
	short min[4];
	short max[4];
	float error;
	float value[6];
};

fmfonthandle helvfont,helv12=0;
short bits = 5, minc = 0, maxc = 256, dither2 = 3, zoom = 1;

#define POW_I(x,y,z) {long i ; (x)=1 ; for(i=0 ; i<(z); i++) (x) *= (y);}
#define calcerr4(a,b,g,r) ((a) + (b) + 4.0 * (g) + 2.0 * (r))
#define calcerr3(b,g,r) ((b) + 4.0 * (g) + 2.0 * (r))
#define add_r 1
extern short floydmax;




#undef add_r

void initview()
{
	long ofsx, ofsy, sizex, sizey;

	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);
	ortho2(-0.5,sizex-0.5,-0.5,sizey-0.5);
	viewport(0,sizex-1,0,sizey-1);
}

#define BUTWHITE 7
#define BUTDARK 513
#define BUTLIGHT 514
#define BUTBLACK 0

#define BUTB1 515
#define BUTB2 516
#define BUTB3 517
#define BUTB4 518

void drawbuttons(long win)
{
	long szex, szey;
	
	winset(win);

	if(helv12==0) {
		fminit();
		if( (helvfont=fmfindfont("Helvetica-Bold")) == 0) exit(1);
		helv12=fmscalefont(helvfont,11.0);
	}

	frontbuffer(FALSE);
	backbuffer(TRUE);
	
	color(BUTBLACK);
	clear();

	mapcolor(BUTLIGHT,0,122,122);
	mapcolor(BUTDARK,0,81,81);
	mapcolor(BUTWHITE,255,255,255);
	mapcolor(BUTBLACK,0,0,0);

	mapcolor(BUTB1,0,30,30);
	mapcolor(BUTB2,120,190,190);
	mapcolor(BUTB3,0,60,60);
	mapcolor(BUTB4,160,240,240);

	DefButBlock("DisTape",win,helv12,50,2,0);
	DefButCol(2,NORMALDRAW,0,BUTDARK,BUTLIGHT,BUTWHITE,BUTWHITE,BUTB1,BUTB2,BUTB3,BUTB4);
	SetButCol(2);
	BGflush();
	BGnewline();
	BGadd(NUM|SHO,1,	"bits:",    0,0,8,9,	&bits,1.0,6.0);
	BGadd(NUM|SHO,201,	"free:",    0,0,10,9,	&minc,0.0,(float) maxc - 1.0);
	BGadd(NUM|SHO,201,	"maxc:",    0,0,10,9,	&maxc,minc + 1.0,4096.0);
	BGnewline();
	BGadd(ROW|SHO,200,	"8",    0,0,10,9,	&maxc,2.0,8.0);
	BGadd(ROW|SHO,200,	"16",    0,0,10,9,	&maxc,2.0,16.0);
	BGadd(ROW|SHO,200,	"32",    0,0,10,9,	&maxc,2.0,32.0);
	BGadd(ROW|SHO,200,	"64",    0,0,10,9,	&maxc,2.0,64.0);
	BGadd(ROW|SHO,200,	"128",    0,0,10,9,	&maxc,2.0,128.0);
	BGadd(ROW|SHO,200,	"256",    0,0,10,9,	&maxc,2.0,256.0);
	BGnewline();
	BGadd(ROW|SHO,200,	"512",    0,0,10,9,	&maxc,2.0,512.0);
	BGadd(ROW|SHO,200,	"1024",    0,0,10,9,&maxc,2.0,1024.0);
	BGadd(ROW|SHO,200,	"2048",    0,0,10,9,&maxc,2.0,2048.0);
	BGadd(ROW|SHO,200,	"4096",    0,0,10,9,&maxc,2.0,4096.0);
	BGnewline();

	BGadd(ROW|SHO, 300,	"0",	    0,0,10,10,		&dither2,1.0,(float) 0);
	BGadd(ROW|SHO, 300,	"2",	    0,0,10,10,		&dither2,1.0,(float) 2);
	BGadd(ROW|SHO, 300,	"4",	    0,0,10,10,		&dither2,1.0,(float) 4);
	BGadd(ROW|SHO, 300,	"Floyd",    0,0,10,10,		&dither2,1.0,(float) 'f');
	BGnewline();
	BGadd(SLI|SHO,1,	"maxerr",    0,0,10,5,	&floydmax,0.0,256.0, 0, 0);
	BGnewline();
	BGadd(BUT,100,	"Make",    0,0,10,10);
	BGadd(BUT,101,	"Org",     0,0,10,10);
	BGadd(NUM|SHO,400,	"zoom:",    0,0,10,10,	&zoom,1.0,4.0);
	
	BGnewline();
	BGposition(60,20,360 - 2 * 60,200);

	BGspacing(4,7);
	BGdirection('d');
	BGdraw();
	frontbuffer(TRUE);
	
	getsize(&szex, &szey);
	rectcopy(0, 0, szex - 1, szey - 1, 0, 0);
	gflush();
}


main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf * ibuf, * bbuf, * tbuf;
	void *ditfunc = 0;
	long buts, pic, event, oldditfunc = -1, oldfloydmax, i;
	short val;
	char name[1024];
	
#undef float
	
	extern void (*wu_percentdone)(float);
	wu_percentdone = percentdone;

	if (argc == 1){
		ibuf = loadiffname("/data/irisdata/codim/klaar/pano4.tga",LI_rect);
	} else {
		ibuf = loadiffname(argv[1],LI_rect);
	}
	if (ibuf == 0) {
		printf("nopic\n");
		exit(0);
	}

	setdither('f');
	ditfunc = floyd;
	dither2 = 'f';
	
	prefsize(2 * ibuf->x,2 * ibuf->y);
	prefsize(ibuf->x,ibuf->y);
	pic = winopen(ibuf->name);
	RGBmode();
	gconfig();
	rectzoom(2.0, 2.0);
	rectzoom(1.0, 1.0);
	lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->rect);
	bbuf = dupImBuf(ibuf);

	prefsize(360, 240);
	buts = winopen("buts");
	doublebuffer();
	gconfig();
	drawbuttons(buts);

	qdevice(INPUTCHANGE);
	qdevice(LEFTMOUSE);
	qdevice(RAWKEYBD);
	qdevice(WINSHUT);
	qdevice(WINQUIT);
	qdevice(INSERTKEY);
	qdevice(HOMEKEY);
	qdevice(PAGEUPKEY);
	qdevice(F2KEY);

	freecmapImBuf(bbuf);
	
	while(event = qreadN(&val)){
		switch(event){
		case LEFTMOUSE:
			if (val){
				winset(buts);
				switch(DoButtons()){
				case 100:
					percentdone(0);
					if (bbuf->cbits != bits || bbuf->mincol != minc 
							|| bbuf->maxcol != maxc || bbuf->cmap == 0) {

						/* bereken nieuw palette */
						bbuf->cbits = bits;
						bbuf->mincol = minc;
						bbuf->maxcol = maxc;
						
						freecmapImBuf(bbuf);
	
						if (ibuf) freeImBuf(ibuf);
						ibuf = dupImBuf(bbuf);
						losecmapbits(ibuf, 0);
						
						makecmap(ibuf,0,bits,maxc - minc,ditfunc);
						percentdone(100);
	
						addcmapImBuf(bbuf);
						addcmapbits(ibuf);
						memcpy(bbuf->cmap + minc, ibuf->cmap, (maxc - minc) * sizeof(long));
						oldditfunc = -1;
					}
					
					if (oldditfunc != dither2 || (dither2 == 'f' && floydmax != oldfloydmax)) {
						/* voer berekeningen opnieuw uit */

						percentdone(50);
						oldditfunc = dither2;
						oldfloydmax = floydmax;
						
						freeImBuf(ibuf);
						ibuf = dupImBuf(bbuf);
						
						converttocmap(ibuf);
						applycmap(ibuf);
					}
					percentdone(100);
					winset(pic);
					lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->rect);
					break;
				case 101:
					winset(pic);
					lrectwrite(0, 0, bbuf->x-1, bbuf->y-1, bbuf->rect);
					break;
				case 200:
					minc = 0;
				case 201:
					drawbuttons(buts);
					break;
				case 300:
					switch(dither2){
					case 0:
						setdither('0');
						ditfunc = dit0;
						break;
					case 2:
						setdither('2');
						ditfunc = dit2;
						break;
					case 4:
						setdither('4');
						ditfunc = dit4;
						break;
					case 'f':
						setdither('f');
						ditfunc = floyd;
						break;
					}
					break;
				case 400:
					winset(pic);
					prefsize(zoom * ibuf->x, zoom * ibuf->y);
					winconstraints();
					reshapeviewport();
					rectzoom(zoom, zoom);					
				}
			}
			break;
		case F1KEY:
			if (val) {
				if (name[0] == 0) strcpy(name, ibuf->name);
				if (name[0] == 0) strcpy(name, "/data/");
				if (fileselect("Read image: ",name)){
					freeImBuf(ibuf);
					ibuf = bbuf;
					bbuf = loadiffname(name, IB_rect);
					if (bbuf == 0) {
						printf("couldn't load %s\n", name);
						bbuf = ibuf;
						ibuf = 0;
					} else freeImBuf(ibuf);

					freecmapImBuf(bbuf);
					ibuf = dupImBuf(bbuf);
					winset(pic);
					prefsize(zoom * bbuf->x, zoom * bbuf->y);
					winconstraints();
					reshapeviewport();
					/*lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->rect);*/
					wintitle(ibuf->name);
				}
			}
			break;
		case F2KEY:
			if (val) {
				if (name[0] == 0) strcpy(name, ibuf->name);
				if (name[0] == 0) strcpy(name, "/data/");
				if (fileselect("Save IFF: ",name)){
					tbuf = dupImBuf(bbuf);
					tbuf->ftype= AM_hilace;		/* HACK ALERT */
					saveiff(tbuf,name,IB_rect);
					freeImBuf(tbuf);
				}
			}
			break;
		case F3KEY:
			if (val) {
				if (name[0] == 0) strcpy(name, ibuf->name);
				if (name[0] == 0) strcpy(name, "/data/");
				if (fileselect("Save Targa: ",name)){
					tbuf = dupImBuf(bbuf);
					tbuf->ftype= TGA;
					saveiff(tbuf,name,IB_rect);
					freeImBuf(tbuf);
				}
			}
			break;
		case REDRAW:
			if (val) {
				winset(val);
				if (val == pic) lrectwrite(0, 0, ibuf->x-1, ibuf->y-1, ibuf->rect);
				else drawbuttons(buts);
			}
			break;
		case WINSHUT:
		case WINQUIT:
			exit(0);
			break;
		}
	}
}

