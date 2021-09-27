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



/* 
 * playanim.c	blenderversie jan 98
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "screen.h"

#include <sys/mman.h>	/* mapped memory */



/* ***************** gl_util.c ****************** */

#include <local/gl_util.h>

short mousexN = 0, mouseyN = 0, *mousexNp = &mousexN , *mouseyNp = &mouseyN;
long qualN = 0, winN = 0, *qualNp = &qualN, *winNp = &winN;
long (*queread)() = qread;
Boolean (*getbut)(Device) = getbutton;


long qreadN(val)
short *val;
{
	long event;
	static long qual = 0, mousex = 0, mousey = 0, win = 0;

	switch(event = queread(val)){
	case LEFTMOUSE:
		if (*val) qual |= LMOUSE;
		else qual &= ~LMOUSE;
		break;
	case MIDDLEMOUSE:
		if (*val) qual |= MMOUSE;
		else qual &= ~MMOUSE;
		break;
	case RIGHTMOUSE:
		if (*val) qual |= RMOUSE;
		else qual &= ~RMOUSE;
		break;
	case LEFTSHIFTKEY:
		if (*val) qual |= LSHIFT;
		else qual &= ~LSHIFT;
		break;
	case RIGHTSHIFTKEY:
		if (*val) qual |= RSHIFT;
		else qual &= ~RSHIFT;
		break;
	case LEFTCTRLKEY:
		if (*val) qual |= LCTRL;
		else qual &= ~LCTRL;
		break;
	case RIGHTCTRLKEY:
		if (*val) qual |= RCTRL;
		else qual &= ~RCTRL;
		break;
	case LEFTALTKEY:
		if (*val) qual |= LALT;
		else qual &= ~LALT;
		break;
	case RIGHTALTKEY:
		if (*val) qual |= RALT;
		else qual &= ~RALT;
		break;
	case MOUSEX:
		mousex = *val;
		if (mousexNp) *mousexNp = mousex;
		break;
	case MOUSEY:
		mousey = *val;
		if (mouseyNp) *mouseyNp = mousey;
		break;
	case INPUTCHANGE:
		win = *val;
		if (win){
			qual = 0;
			if (getbut(LEFTMOUSE))		qual |= LMOUSE;
			if (getbut(MIDDLEMOUSE))	qual |= MMOUSE;
			if (getbut(RIGHTMOUSE))	    qual |= RMOUSE;
			if (getbut(LEFTSHIFTKEY))	qual |= LSHIFT;
			if (getbut(RIGHTSHIFTKEY))	qual |= RSHIFT;
			if (getbut(LEFTCTRLKEY))	qual |= LCTRL;
			if (getbut(RIGHTCTRLKEY))	qual |= RCTRL;
			if (getbut(LEFTALTKEY))	    qual |= LALT;
			if (getbut(RIGHTALTKEY))	qual |= RALT;
		}
		if (winNp) *winNp = win;
		break;
	}

	if (qualNp) *qualNp = qual;

	return(event);
}




short smallpup(char * title, char * str)
{
	char a[250];
	long temp;
	short event, val;
	
	strcpy(a, title);
	strcat(a,"%t |");
	strcat(a,str);
	temp=defpup(a);

	sginap(2);
	event=dopup(temp);
	freepup(temp);
	if(event== -1) event=0;

	/*even een 1/2 seconde de tijd gunnen om op ESC te drukken */
	if (event){
		for(temp = 5; temp >0; temp--){
			while(qtest()){
				if (qreadN(&val) == ESCKEY){
					if (val){
						event = 0;
						break;
					}
				}
			}
			sginap(10);
		}
	}

	while(getbutton(RIGHTMOUSE)) sginap(10);
	return event;
}





/* ***************** gl_util.c ****************** */




typedef struct pict{
	struct pict *next, *prev;
	char *mem;
	long size;
	char *name;
	struct ImBuf *ibuf;
	struct anim *anim;
	int frame;
	int IB_flags;
}Pict;

struct ListBase _picsbase = {
    0,0};
struct ListBase *picsbase = &_picsbase;
struct ImBuf * tbuf = 0;
long warn = 0, fromdisk = FALSE;
float zoomx = 1.0 , zoomy = 1.0;
double ptottime = 0.0, swaptime = 0.04;
long halvex = FALSE;

void minmaxibuf(ibuf, x1, y1, x2, y2)
struct ImBuf *ibuf;
short *x1, *y1, *x2, *y2;
{
	long x, y, found, step;
	ulong *rect;

	rect = ibuf->rect;
	found = FALSE;
	for (y = 0; y<ibuf->y ; y++){
		for (x = ibuf->x; x>0; x--){
			if (*rect++) {
				found = TRUE;
				break;
			}
		}
		if (found) break;
	}

	if (found == FALSE){
		*x1 = *x2 = *y1 = *y2 = 0;
		return;
	}

	*y1 = y;

	rect = ibuf->rect;
	rect += ibuf->x * ibuf->y - 1;

	found = FALSE;
	for (y = ibuf->y - 1; y >= 0 ; y--){
		for (x = ibuf->x; x>0; x--){
			if (*rect--) {
				found = TRUE;
				break;
			}
		}
		if (found) break;
	}

	*y2 = y;


	step = ibuf->x;
	found = FALSE;
	for (x = 0; x < ibuf->x ; x++){
		rect = ibuf->rect + x;
		for (y = ibuf->y; y > 0; y--){
			if (*rect) {
				found = TRUE;
				break;
			}
			rect += step;
		}
		if (found) break;
	}

	*x1 = x;

	step = ibuf->x;
	found = FALSE;
	for (x = ibuf->x - 1; x > 0; x--){
		rect = ibuf->rect + x;
		for (y = ibuf->y; y > 0; y--){
			if (*rect) {
				found = TRUE;
				break;
			}
			rect += step;
		}
		if (found) break;
	}

	*x2 = x;
}


long pupdate_time()
{
	struct tms voidbuf;
	static long ltime;
	long time;

	time = times(&voidbuf);
	ptottime += (time - ltime) / 100.0;
	ltime = time;
	return (ptottime < 0);
}

void getmouseco(mval)
short *mval;
{
	long ofsx, ofsy, sizex, sizey;
	static Device mdev[2]= {MOUSEX, MOUSEY};

	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);

	getdev(2, mdev, mval);
	mval[0]-= ofsx;
	mval[1]-= ofsy;

	if(mval[0] < 0) mval[0] = 0;
	if(mval[0] > sizex) mval[0] = sizex;
	
}

void toscreen(ibuf)
struct ImBuf *ibuf;
{
	short mval[2];
	long ofsx, ofsy;
	long sizex, sizey;
	long scrollx;

	/* deze routine is veranderd er van uitgaand dat swapbuffers()
     * pas blokkeert op het moment dat er grafiese output plaatsvindt.
     */

	if (ibuf == 0){
		printf("no ibuf !\n");
		return;
	}

	if(halvex) {
		getmouseco(mval);
		scrollx = -mval[0];
		ofsx = zoomx * ibuf->xorig + scrollx;
	} else	
		ofsx = zoomx * ibuf->xorig;

	ofsy = zoomy * ibuf->yorig;
	sizex = ibuf->x;
	sizey = ibuf->y;

	lrectwrite(ofsx,ofsy,sizex + ofsx - 1,sizey + ofsy - 1,ibuf->rect);
	pupdate_time();
	if (warn && ptottime >= swaptime){
		cpack(-1);
		color(4095);
		rectf(10, 10, 20, 20);
	}

	swapbuffers();
}


void animcompres()
{
	struct pict * picture;
	struct ImBuf * ibuf, * ibuf1, * ibuf2, * ibuf3;
	uchar *rect;
	ushort *srect, *_srect;
	long i;
	short x1, y1, x2, y2;
	extern void rectxor();
	extern void rectcpy();

	picture = picsbase->first;
	if (picture == 0) return;

	if (picture->mem) ibuf = loadiffmem((long *) picture->mem, picture->IB_flags);
	else if (picture->anim) ibuf = anim_absolute(picture->anim, picture->frame);
	else ibuf = loadiffname(picture->name, picture->IB_flags);
	
	RGBmode();
	pixmode(PM_TTOB, 0);
	pixmode(PM_SIZE, 32);
	gconfig();
	
	if (ibuf == 0) return;
	picture->ibuf = ibuf;
	if (picture->mem) free(picture->mem);
	picture->mem = 0;
	ibuf1 = dupImBuf(ibuf);
	ibuf->type = 1;
	ibuf->ftype = TGA;
	toscreen(ibuf);

	picture = picture->next;
	if (picture == 0) return;

	if (picture->mem) ibuf = loadiffmem((long *) picture->mem, picture->IB_flags);
	else if (picture->anim) ibuf = anim_absolute(picture->anim, picture->frame);
	else ibuf = loadiffname(picture->name, picture->IB_flags);

	if (ibuf == 0) return;
	picture->ibuf = ibuf;
	if (picture->mem) free(picture->mem);
	picture->mem = 0;
	ibuf2 = dupImBuf(ibuf);
	ibuf->type = 1;
	ibuf->ftype = TGA;
	toscreen(ibuf);

	/* XOR de plaatjes met elkaar, bereken daarna boundingbox 
 * pak daana een rect op ter grote van verschil
 */

	picture = picture->next;
	while (picture){
		if (picture->mem) ibuf3 = loadiffmem((long *) picture->mem, picture->IB_flags);
		else if (picture->anim) ibuf3 = anim_absolute(picture->anim, picture->frame);
		else ibuf3 = loadiffname(picture->name, picture->IB_flags);
		if (ibuf3 == 0) break;

		rectoptot(ibuf1, ibuf3, rectxor);
		minmaxibuf(ibuf1, &x1, &y1, &x2, &y2);

		/* laat delta zien */
		lrectwrite(0, 0, ibuf1->x - 1, ibuf1->y - 1, ibuf1->rect);
		cpack(-1);
		recti(x1, y1, x2, y2);
		/*printf("bytes: %d = %.2f%%\n", (x2 - x1) * (y2 - y1), 100.0 * (x2 - x1) * (y2 - y1) / ((float) ibuf1->x * ibuf1->y));*/
		swapbuffers();

		ibuf = allocImBuf(x2 - x1 + 1, y2 - y1 + 1, ibuf3->depth, 1, 0);
		rectop(ibuf, ibuf3, 0, 0, x1, y1, 32767, 32767, rectcpy);

		ibuf->xorig = x1;
		/*ibuf->yorig = ibuf3->y -1 - y2;*/
		ibuf->yorig = y1;
		ibuf->type = 1;
		ibuf->ftype = TGA;
		/*toscreen(ibuf);*/

		freeImBuf(ibuf1);
		ibuf1 = ibuf2;
		ibuf2 = ibuf3;

		picture->ibuf = ibuf;
		if (picture->mem) free(picture->mem);
		picture->mem = 0;
		picture = picture->next;
	}

	freeImBuf(ibuf1);
	freeImBuf(ibuf2);
}


void saveanim(name)
char *name;
{
	char string[100],head[100],tail[100];
	long pic;
	ushort numlen;
	struct pict * picture;
	struct ImBuf *ibuf;

	pic = stringdec(name, head, tail, &numlen);
	stringenc(string,head,tail,numlen,pic);

	picture = picsbase->first;

	while(picture){
		ibuf = picture->ibuf;
		if (ibuf){
			saveiff(ibuf,string,SI_rect);
		}
		pic ++;
		stringenc(string,head,tail,numlen,pic);
		picture = picture->next;
	}

}


void build_pict_list(char * first)
{
	long size,pic,file;
	char *mem, name[256];
	long type, event;
	short val;
	struct pict * picture = 0;
	struct ImBuf *ibuf = 0;
	long count = 0;
	char str[100];
	struct anim * anim;
	
	type = isanim(first);
	if (type == ANIM_SEQUENCE) type = 0;
	
	if (type) {
		anim = open_anim(first, ANIM_DFLT, IB_rect);
		if (anim) {
			ibuf = anim_absolute(anim, 0);
			if (ibuf) {
				toscreen(ibuf);
				freeImBuf(ibuf);
			}
			
			for (pic = 0; pic < anim->duration; pic ++) {
				picture = CLN(Pict);
				picture->anim = anim;
				picture->frame = pic;
				picture->IB_flags = IB_rect;
				sprintf(str, "%s : %d", first, pic + 1);
				picture->name = strdup(str);
				addtail(picsbase, picture);
			}
		} else printf("couldn't open anim %s\n", first);
	} else {
	
		strcpy(name,first);
	
		pupdate_time();
		ptottime = 1.0;
		
/*
     O_DIRECT
            If set, all reads and writes on the resulting file descriptor will
            be performed directly to or from the user program buffer, provided
            appropriate size and alignment restrictions are met.  Refer to the
            F_SETFL and F_DIOINFO commands in the fcntl(2) manual entry for
            information about how to determine the alignment constraints.
            O_DIRECT is a Silicon Graphics extension and is only supported on
            local EFS and XFS file systems.
*/
		while(type = ispic(name)){
			file = open(name, O_RDONLY, 0);
			if (file < 0) return;
			picture = callocstruct(struct pict , 1);
			if (picture == 0){
				printf("Not enough memory for pict struct \n");
				close(file);
				return;
			}
			size = lseek(file,0L,SEEK_END);
			lseek(file,0L,SEEK_SET);
			picture->size = size;
			picture->IB_flags = IB_rect;
			
			/* if (type == JPG) picture->IB_flags |= IB_rgba; */
			/* if (type == Anim) picture->IB_flags |= IB_rgba; */
			
			if ((type & JPG) == FALSE && fromdisk == FALSE) {
				mem=(char *)malloc(size);
				if (mem==0){
					printf("Couldn't get memory\n");
					close(file);
					free(picture);
					return;
				}
		
				if (read(file,mem,size) != size){
					printf("Error while reading %s\n",name);
					close(file);
					free(picture);
					free(mem);
					return;
				}
			} else mem = 0;
			
			picture->mem = mem;
			picture->name = strdup(name);
			close(file);
			addtail(picsbase,picture);
			count++;
			
			pupdate_time();
			
			if (ptottime > 1.0) {				
				if (picture->mem) ibuf = loadiffmem((long *) picture->mem, picture->IB_flags);
				else ibuf = loadiffname(picture->name, picture->IB_flags);
				if (ibuf) {
					toscreen(ibuf);
					freeImBuf(ibuf);
					frontbuffer(TRUE);
					color(WHITE);
					cpack(-1);
					cmov2i(10, 10);
					sprintf(str, "%4d: %s", count, name);
					charstr(str);
					frontbuffer(FALSE);
				}
				pupdate_time();
				ptottime = 0.0;
			}
			
			newname(name, +1);
			
			while(qtest()){
				switch(event = qreadN(&val)){
				case KEYBD:
					if (val == 27) return;
					break;
				}
			}
		}
	}
	return;
}

void initcmap(int offset)
{
	float r,g,b;
	short c;

	for (b = 0 ; b <= 1.0 ; b += 1.0/3.0){
		for (g = 0 ; g <= 1.0 ; g += 1.0/7.0){
			for (r = 0 ; r <= 1.0 ; r += 1.0/7.0){
				mapcolor(offset , 255.0 * r + 0.5 , 255.0 * g + 0.5 , 255.0 * b + 0.5 );
				offset++;
			}
		}
	}
}


main_playanim(argc,argv)
int argc;
char **argv;
{
	struct ImBuf *ibuf = 0;
	struct pict *picture = 0;
	char name[256], colormap[256][4], * exe = 0, instr[256], killit[256], pupstr[256];
	short val = 0, go = TRUE, ibufx = 0, ibufy = 0, h_ibufx = 0;
	long event, win, stopped = FALSE, maxwinx, maxwiny;
	short c233 = FALSE, yuvx = FALSE, once = FALSE, sstep = FALSE, wait2 = FALSE, resetmap = FALSE, pause = 0;
	short pingpong = FALSE, direction = 1, next = 1, turbo = FALSE, doubleb = TRUE, noskip = FALSE;
	long sizex, sizey, ofsx, ofsy, i;
	pid_t child;
	extern int scalecmapY;
	struct anim * anim = 0;
	int type;
	
	int cd = FALSE;
	
	while (argc > 1) {
		if (argv[1][0] == '-'){
			switch(argv[1][1]) {
				case 'm':
					fromdisk = TRUE;
					break;
				case 'x':
					exe = argv[2];
					argc--;
					argv++;
					break;
				case 'p':
					sprintf(instr, "playaifc %s", argv[2]);
					exe = instr;
					argc--;
					argv++;
					pause = 16;
					break;
				default:
					printf("unknown option '%c': skipping\n", argv[1][1]);
					break;
			}
			argc--;
			argv++;
		} else break;
	}
	
	if (exe) {
		strcpy(killit, "/etc/killall ");
		strcat(killit, exe);
	}
	
	if (argc > 1) strcpy(name,argv[1]);
	else {
		getwd(name);
		if (name[strlen(name)-1] != '/') strcat(name,"/");
	}
	
#ifdef __sgi
	initmoviepointers();
#endif
	
	type = isanim(name);
	if (type == ANIM_SEQUENCE) type = 0;

	if (type) {
		anim = open_anim(name, ANIM_DFLT, IB_rect);
		if (anim) {
			ibuf = anim_absolute(anim, 0);
			close_anim(anim);
		}
	} else if (ispic(name) == FALSE){
		if (fileselect("Select Firstpicture: ",name) == 0) exit(0);
	}

	if (ibuf == 0) ibuf = loadiffname(name, IB_rect);
	if (ibuf == 0){
		printf("couldn't open %s\n",name);
		exit(0);
	}

	if (ibuf->ftype == AN_c233) c233 = TRUE;
	if (ibuf->ftype == AN_yuvx) yuvx = TRUE;
	if (ibuf->ftype == AN_tanx) yuvx = TRUE;


	prefsize(ibuf->x, ibuf->y);
	printf("Size: %d*%d\n",ibuf->x,ibuf->y);
	noborder();
	win = winopen(name);

	
	/* hier stond getgdesc */
	
	if (c233 == FALSE && yuvx == FALSE) RGBmode();
	else {
		pixmode(PM_TTOB, 1);	
		pixmode(PM_SIZE, 16);
		/* IB_flags = IB_word; */
	}
	
	if (doubleb) doublebuffer();
	rectzoom(zoomx,zoomy);
	gconfig();

	ibufx = ibuf->x; h_ibufx = ibufx / 1.57; 
	ibufy = ibuf->y;

	stepunit(ibufx, ibufy);
	stepunit(1, 1);
	noborder();
	minsize(ibufx, ibufy);
	
	maxwinx = displaysizex;
	maxwiny = displaysizey;
	
	if (maxwinx % ibuf->x) maxwinx = ibuf->x * (1 + (maxwinx / ibuf->x));
	if (maxwiny % ibuf->y) maxwiny = ibuf->y * (1 + (maxwiny / ibuf->y));
	
	maxsize(maxwinx, maxwiny);
	winconstraints();

	cpack(0); 
	clear();
	swapbuffers();

	qdevice(LEFTMOUSE); qdevice(LEFTARROWKEY); qdevice(RIGHTARROWKEY); qdevice(UPARROWKEY);
	qdevice(DOWNARROWKEY);
	qdevice(MOUSEX);
	qdevice(PADPLUSKEY); qdevice(PADMINUS); qdevice(PADENTER); qdevice(PADPERIOD);
	qdevice(PAD0); qdevice(PAD1); qdevice(PAD2); qdevice(PAD3); qdevice(PAD4);
	qdevice(PAD5); qdevice(PAD6); qdevice(PAD7); qdevice(PAD8); qdevice(PAD9);
	qdevice(KEYBD);
	qdevice(WINSHUT); qdevice(WINQUIT);
	qdevice(LEFTSHIFTKEY); qdevice(RIGHTSHIFTKEY);
	qdevice(MINUSKEY);qdevice(EQUALKEY);
	
	if (c233) {
		/* hier stond getgdesc */
		initcmap(256);
	}
	
	if (yuvx) {
		for (i = ibuf->mincol; i < ibuf->maxcol; i++) {
			mapcolor(i , ibuf->cmap[i] & 0xff, (ibuf->cmap[i] >> 8) & 0xff, (ibuf->cmap[i] >> 16) & 0xff);
		}
	}
	
	build_pict_list(name);
	
	for (i = 2; i < argc; i++){
		strcpy(name, argv[i]);
		build_pict_list(name);
	}


	freeImBuf(ibuf); 
	ibuf = 0;

	pupdate_time();
	ptottime = 0;

	while (go){
		if (pingpong) direction = -direction;

		if (direction == 1) picture = picsbase->first;
		else picture = picsbase->last;

		if (picture == 0){
			printf("couldn't find pictures\n");
			go = FALSE;
		}
		if (pingpong){
			if (direction == 1) picture = picture->next;
			else picture = picture->prev;
		}
		if (ptottime > 0.0) ptottime = 0.0;

		while (picture){
			if (ibuf != 0 && ibuf->type == 0) freeImBuf(ibuf);

			if (picture->ibuf) ibuf = picture->ibuf;
			else if (picture->anim) ibuf = anim_absolute(picture->anim, picture->frame);
			else if (picture->mem) ibuf = loadiffmem((long *) picture->mem, picture->IB_flags);
			else ibuf = loadiffname(picture->name, picture->IB_flags);
			
			if (ibuf){
				while (pupdate_time()) sginap(1);
				ptottime -= swaptime;
				toscreen(ibuf);
			} /* else deleten */

			if (once){
				if (picture->next == 0) wait2 = TRUE;
				else if (picture->prev == 0) wait2 = TRUE;
			}
			
			next = direction;
			
			while (qtest() != 0 | wait2 != 0){

				if (wait2 && stopped) {
					system("/etc/killall -CONT clock");
					stopped = FALSE;
				}
			
				event = qreadN(&val);
				if (wait2){
					pupdate_time();
					ptottime = 0;
				}
				switch (event){
				case LEFTARROWKEY:
					if (val){
						sstep = TRUE;
						wait2 = FALSE;
						if (qualN & SHIFT) {
							picture = picsbase->first;
							next = 0;
						} else {
							next = -1;
						}
					}
					break;
				case DOWNARROWKEY:
					if (val){
						wait2 = FALSE;
						if (qualN & SHIFT) {
							next = direction = -1;
						} else {
							next = -10;
							sstep = TRUE;
						}
					}
					break;
				case RIGHTARROWKEY:
					if (val){
						sstep = TRUE;
						wait2 = FALSE;
						if (qualN & SHIFT) {
							picture = picsbase->last;
							next = 0;
						} else {
							next = 1;
						}
					}
					break;
				case UPARROWKEY:
					if (val){
						wait2 = FALSE;
						if (qualN & SHIFT) {
							next = direction = 1;
						} else {
							next = 10;
							sstep = TRUE;
						}
					}
					break;
				case LEFTMOUSE:
				case MOUSEX:
					if (qualN & LMOUSE){
						getorigin(&ofsx,&ofsy);
						getsize(&sizex,&sizey);
						picture = picsbase->first;
						i = 0;
						while (picture){
							i ++;
							picture = picture->next;
						}
						i = (i * (mousexN - ofsx)) / sizex;
						picture = picsbase->first;
						for (; i > 0; i--){
							if (picture->next == 0) break;
							picture = picture->next;
						}
						sstep = TRUE;
						wait2 = FALSE;
						next = 0;
					}
					break;
				case EQUALKEY:
					if (val) {
						if (qualN & SHIFT) {
							pause ++;
							printf("pause:%d\n", pause);
						} else swaptime /= 1.1;
					}
					break;
				case MINUSKEY:
					if (val) {
						if (qualN & SHIFT) {
							pause --;
							printf("pause:%d\n", pause);
						} else swaptime *= 1.1;
					}
					break;
				case PAD0:
					if (val){
						if (once) once = wait2 = FALSE;
						else {
							picture = 0;
							once = TRUE;
							wait2 = FALSE;
						}
					}
					break;
				case PADENTER:
					if (val){
						wait2 = sstep = FALSE;
						if (once && picture && picture->prev == 0) {
							if (cd){
								/*CDplayabs(cd, 5, 35, 0, 1);*/
							} else if (exe) {
								system(killit);
								wait(0);
								if ((child = fork()) == 0) {
									if (pause < 0) sginap(-pause);
									system(exe);
									exit(0);
								}
								if (pause > 0) sginap(pause);
								pupdate_time();
								ptottime = 0;
							}
						}
					}
					break;
				case PADPERIOD:
					if (val){
						if (sstep) wait2 = FALSE;
						else {
							sstep = TRUE;
							wait2 = !wait2;
						}
					}
					break;
				case PAD1:
					swaptime = 1.0 / 60.0;
					break;
				case PAD2:
					swaptime = 1.0 / 50.0;
					break;
				case PAD3:
					swaptime = 1.0 / 30.0;
					break;
				case PAD4:
					swaptime = 1.0 / 25.0;
					break;
				case PAD5:
					swaptime = 1.0 / 20.0;
					break;
				case PAD6:
					swaptime = 1.0 / 15.0;
					break;
				case PAD7:
					swaptime = 1.0 / 12.0;
					break;
				case PAD8:
					swaptime = 1.0 / 10.0;
					break;
				case PAD9:
					swaptime = 1.0 / 6.0;
					break;
				case PADPLUSKEY:
					if (val == 0) break;
					zoomx += 2.0;
					zoomy += 2.0;
				case PADMINUS:
					if (val == 0) break;
					if (zoomx > 1.0) zoomx -= 1.0;
					if (zoomy > 1.0) zoomy -= 1.0;
					getorigin(&ofsx,&ofsy);
					getsize(&sizex,&sizey);
					ofsx += sizex/2;
					ofsy += sizey/2;
					sizex = zoomx * ibufx;
					sizey = zoomy * ibufy;
					ofsx -= sizex/2;
					ofsy -= sizey/2;
					prefposition(ofsx,ofsx+sizex - 1, ofsy,ofsy+sizey -1);
					noborder();
					winconstraints();
				case REDRAW:
					getsize(&sizex,&sizey);
					zoomx = (float) sizex / ibufx;
					zoomy = (float) sizey / ibufy;
					zoomx = floorf(zoomx + 0.5);
					zoomy = floorf(zoomy + 0.5);
					if (zoomx < 1.0) zoomx = 1.0;
					if (zoomy < 1.0) zoomy = 1.0;

					sizex = zoomx * ibufx;
					sizey = zoomy * ibufy;
					prefsize(sizex, sizey);
					noborder();
					winconstraints();
					stepunit(1, 1);
					noborder();
					minsize(ibufx, ibufy);
					maxsize(maxwinx, maxwiny);
					winconstraints();

					rectzoom(zoomx,zoomy);
					/*ortho2(-0.5, sizex - 0.5, -0.5, sizey - 0.5);*/
					viewport(0, sizex-1, 0, sizey-1);
					ptottime = 0.0;
					toscreen(ibuf);
					while (qtest()) qreadN(&val);
					break;
				case WINSHUT:
				case WINQUIT:
					go = FALSE;
					break;
				case KEYBD:
					switch(val){
					case 27:
						go = FALSE;
						break;
					case 'a':
					case 'A':
						if (noskip) noskip = FALSE;
						else noskip = TRUE;
						break;
					case 'p':
					case 'P':
						if (pingpong) pingpong = 0;
						else pingpong = 1;
						break;
					case 'd':
					case 'D':
						if (doubleb) {
							singlebuffer();
							doubleb = FALSE;
						} else {
							doublebuffer();
							doubleb = TRUE;
						}
						gconfig();
						break;
					case 't':
						if (turbo == FALSE) {
							animcompres();
							turbo = TRUE;
							ptottime = 0.0;
						}
						break;
					case '?':
						if (ibuf) {
							sprintf(pupstr, " Name: %s | Speed: %.2f frames/s", ibuf->name, 1.0 / swaptime);
							smallpup("Info:", pupstr);
							/* qualN (SHIFT) resetten */
							qenter(INPUTCHANGE, win);
						}
						break;
					case 'w':
					case 'W':
						warn = (warn == 0);
						break;
					case 'x':
					case 'X':
						halvex = 1 - halvex;
						if(halvex) ibufx = h_ibufx; else ibufx = ibuf->x;
						getorigin(&ofsx,&ofsy);
						getsize(&sizex,&sizey);
						ofsx += sizex/2;
						ofsy += sizey/2;
						sizex = zoomx * ibufx;
						sizey = zoomy * ibufy;
						ofsx -= sizex/2;
						ofsy -= sizey/2;
						prefposition(ofsx,ofsx+sizex - 1, ofsy,ofsy+sizey -1);
						noborder();
						winconstraints();
						break;
					case 'y':
						if (scalecmapY) {
						    scalecmapY = FALSE;
						    ibufy /= 2;
						} else {
						    scalecmapY = TRUE;
						    ibufy *= 2;
						}
						printf("scalecmapY %d\n",  scalecmapY);
						
						break;
					case '/':
						swaptime = 1.0 / 5.0;
						break;
/*					case '2':
						qweqwe = 2;
						break;
					case '1':
						qweqwe = 1;
						break;
					case '0':
						qweqwe = 0;
						break;
*//*					default:
						printf("key: %d\n", val);
*/					}
				}
				if (go == FALSE) break;
			}
			
			wait2 = sstep;
			
			if (wait2 == 0 && stopped == 0) {
				system("/etc/killall -STOP clock");
				stopped = TRUE;
			}

			pupdate_time();
					
			if (picture && next) {
				/* altijd minstens 1 stap zetten */
				while (picture){
					if (next < 0) picture = picture->prev;
					else picture = picture->next;
	
					if (once && picture != 0){
						if (picture->next == 0) wait2 = TRUE;
						else if (picture->prev == 0) wait2 = TRUE;
					}

					if (wait2 || ptottime < swaptime || turbo || noskip) break;
					ptottime -= swaptime;
				}
				if (picture == 0 && sstep) {
					if (next < 0) picture = picsbase->last;
					else if (next > 0) picture = picsbase->first;									
				}
			}
			if (go == FALSE) break;
		}
	}
	if (resetmap){
		for (i = 0; i<256; i++){
			mapcolor(i, colormap[i][3], colormap[i][2], colormap[i][1]);
		}
		gflush();
	}
	if (stopped) system("/etc/killall -CONT clock");
	gexit();
}

