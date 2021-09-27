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



/*  

 
*/


#include <local/iff.h>
#include <local/util.h>

#include <string.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <fmclient.h>
#include "/usr/people/include/Button.h"
#include <stdio.h>

typedef struct { 
	short y,xl,xr,dy; 
} FillSeg;

#define MAX 10000

#define PUSHF(Y,XL,XR,DY)   \
	if (sp<stackmax && Y+(DY)>=0 && Y+(DY)<=may)  \
	{sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define POPF(Y,XL,XR,DY) \
	{sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp ->xr;}


void Fill(x, y, nv, xsize, ysize, rectot)
short x,y;
unsigned long nv;
long xsize, ysize;
unsigned long *rectot;
{
	short l,x1,x2,dy,max,may,minx,maxx;
	unsigned long ov,*rect,*r,*rmin;
	FillSeg stack[MAX], *sp= stack, *stackmax;

	r= rectot+ y*xsize+x;
	ov= *r;
	if(ov==nv) return;

	stackmax=stack+MAX;
	max= xsize-1; 
	may= ysize-1;
	PUSHF(y,x,x,1);
	PUSHF(y+1,x,x,-1);

	while (sp>stack) {
		POPF(y,x1,x2,dy);
		rect= rectot+y*xsize;
		/* lrectread(0,y,max,y,rect); */
		r=rect+x1;
		maxx=x1;
		for(x=x1;x>=0 && *r==ov; x--) *(r--)=nv;
		minx=x;
		rmin=r;
		if(x==x1) goto skip;
		l=x+1;
		if(l<x1) PUSHF(y,l,x1-1,-dy);
		x=x1+1;

		do {
			r= rect+x;
			for(;x<=max && *r==ov; x++) *(r++)=nv;
			maxx=x;
			PUSHF(y,l,x-1,dy);
			if(x>x2+1) PUSHF(y,x2+1,x-1,-dy);
skip:		
			for(x++,r++;x<=x2 && *r!=ov; x++) r++;
			l=x;
		} while (x<=x2);
	}
}


void gem4(c,r)
register short c[];
unsigned long *r;
{
	register char *b,*d,*e;
	short x;
	b=(char *)r;
	b++; 
	d=b+1 ;
	e=d+1;

	c[0]= c[1]= c[2]=0;

	for(x=0;x<4;x++) {
		c[2]+= *b; 
		c[1]+= *d; 
		c[0]+= *e;
		b+=4; 
		d+=4; 
		e+=4;
	}

	c[0]>>=2; 
	c[1]>>=2; 
	c[2]>>=2;
}

void filterp(c,r)
register short c[];
unsigned long *r;
{
	register char *b,*d,*e;
	short x;
	b=(char *)r;
	b++; 
	d=b+1 ;
	e=d+1;


	c[2]= *b +8; 
	c[1]= *d +8; 
	c[0]= *e +8;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= (*b)<<1; 
	c[1]+= (*d)<<1; 
	c[0]+= (*e)<<1;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= *b; 
	c[1]+= *d; 
	c[0]+= *e;
	b+=4; 
	d+=4; 
	e+=4;

	c[2]+= (*b)<<1; 
	c[1]+= (*d)<<1; 
	c[0]+= (*e)<<1;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= (*b)<<2; 
	c[1]+= (*d)<<2; 
	c[0]+= (*e)<<2;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= (*b)<<1; 
	c[1]+= (*d)<<1; 
	c[0]+= (*e)<<1;
	b+=4; 
	d+=4; 
	e+=4;


	c[2]+= *b; 
	c[1]+= *d; 
	c[0]+= *e;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= (*b)<<1; 
	c[1]+= (*d)<<1; 
	c[0]+= (*e)<<1;
	b+=4; 
	d+=4; 
	e+=4;
	c[2]+= *b; 
	c[1]+= *d; 
	c[0]+= *e;


	c[0]>>=4; 
	c[1]>>=4; 
	c[2]>>=4;
}





void copyFtoZ(x1,y1,x2,y2)
short x1,y1,x2,y2;
{
	zdraw(1);
	rectcopy(x1,y1,x2,y2,x1,y1);
	zdraw(0);
}

void copyZtoF(x1,y1,x2,y2)
short x1,y1,x2,y2;
{
	readsource(SRC_ZBUFFER);
	rectcopy(x1,y1,x2,y2,x1,y1);
	readsource(SRC_AUTO);

}


void initvierkant(id,afm)
Object id;
short afm;
{
	short vec[2];

	makeobj(id);
	bgnpolygon();
	vec[0]= -afm; 
	vec[1]= -afm;
	v2s(vec);

	vec[1]= afm;
	v2s(vec);

	vec[0]= afm;
	v2s(vec);

	vec[1]= -afm;
	v2s(vec);
	endpolygon();
	closeobj();
}


main()
{
	struct ImBuf *ibuf;
	extern struct ButCol BGbutcol[];
	Object brush;
	double numd;
	float slif,numf,xx,yy,fx,fy,tel;
	unsigned long rect[4000],kl;
	unsigned long *rectot;
	fmfonthandle helvfont,helv10;
	long driestandl,numl,org[2], size[2], tijd, winar[10];
	Device mdev[2];
	short nums,mval[2], lastval[2],totbut,event;
	short x1,y1,a,x,y,trek,val,tekenkleur,c1[3],c2[3],c3[3],c4[3],vec[2];
	short run=1,xo,yo,b1,xsq,aa,bitvar,driestand, menuvar;
	char numc,*c,*mstr, kleur,schuif,string[40],tekst[20],fill,col[3];


	prefposition(380,780,400,800);
	winar[0]=winopen("Buts");
	RGBmode();
	ortho2(0,400,0,400);
	gconfig();

	prefposition(800,1200,400,800);
	winar[1]=winopen("PAINT");
	RGBmode();
	ortho2(0,400,0,400);
	gconfig();
	getorigin(&org[0], &org[1]);
	getsize(&size[0], &size[1]);

	qdevice(REDRAW);
	qdevice(INPUTCHANGE);
	qdevice(WINSHUT);
	qdevice(WINQUIT);

	qdevice(LEFTMOUSE);
	qdevice(RAWKEYBD);

	cpack(0x0); 
	clear();
	fminit();
	if( (helvfont=fmfindfont("Helvetica-Bold")) == 0) exit(0);
	helv10=fmscalefont(helvfont,10.0);
	fmsetfont(helv10);

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;
	
	/*  DefButCol(nr,drawmode,back,pap_sel,pap_dsel,
		pen_sel,pen_dsel,b1,b2,b3,b4) */
	DefButCol(2, RGBDRAW, 0x505050, 0xA0A0A0, 0, 0xFFFFFF, 0);
	DefButCol(3, RGBDRAW, 0x505050, 0xD0A070, 0, 0xFFFFFF, 0);
	DefButCol(4, RGBDRAW, 0x505050, 0xA0D0F0, 0, 0xFFFFFF, 0);

			/* (str,win,font,aantal,col,drawtype) */
	DefButBlock("Naam",winar[0], helv10, 35, 2, 2);

	winset(winar[0]); 
	cpack(BGbutcol[2].back); 
	clear();

	DefBut(BUT,	201,"BUT",	10,10,55,17);
	DefBut(TOG|SHO,	201,"TOG",	70,10,55,17,	&aa);

	bitvar= 12;
	DefBut(TOG|SHO|BIT|0,	203,"TOGbit",	10,40,55,17,	&bitvar);
	DefBut(TOG|SHO|BIT|1,	203,"TOGbit",	70,40,55,17,	&bitvar);
	DefBut(TOG|SHO|BIT|2,	203,"TOGbit",	130,40,55,17,	&bitvar);
	DefBut(TOG|SHO|BIT|3,	203,"TOGbit",	190,40,55,17,	&bitvar);

	kleur=1;
	DefBut(ROW|CHA,	202,"ROW ",	10,70,55,17,	&kleur,0.0,0.0);
	DefBut(ROW|CHA,	202,"Norm",	70,70,55,17,	&kleur,0.0,1.0);
	DefBut(ROW|CHA,	202,"Fill",	130,70,55,17,	&kleur,0.0,2.0);
	DefBut(ROW|CHA,	202,"Smooth",	190,70,55,17,	&kleur,0.0,3.0);

	numc=nums=numl=10;
	numf=numd= 10.0;
	DefBut(NUM|CHA,	204,"Cha:",	10,120,85,17,	&numc,0.0,100.0);
	DefBut(NUM|SHO,	204,"Sho:",	100,120,85,17,	&nums,0.0,100.0);
	DefBut(NUM|LON,	204,"Lon:",	190,120,85,17,	&numl,0.0,100.0);
	DefBut(NUM|FLO,	204,"Flo:",	10,100,85,17,	&numf,0.0,100.0);
	DefBut(NUM|DOU,	204,"Dou:",	100,100,85,17,	&numd,0.0,100.0);

	col[0]=100; 
	col[1]=34; 
	col[2]=56;
	DefBut(SLI|CHA,	205,"B",	45,150,120,8,	col+2,0.0,255.0,100,2);
	DefBut(SLI|CHA,	205,"G",	45,165,120,8,	col+1,0.0,255.0,100,1);
	DefBut(SLI|CHA,	205,"R",	45,180,120,8,	col,0.0,255.0,100,0);
	slif=.4;
	DefBut(SLI|FLO,	205,"Flo",	45,195,120,8,	&slif,0.0,1.0,0,0);

	strcpy(string,"paint");
	DefBut(TEX,	206,"Naam:",	10,215,120,17,	string,0.0,15.0);

	driestandl=0;
	DefBut(TOG3|CHA|BIT|0,207,"TOG3",	10,240,85,17,	&driestandl);

	menuvar= 11;
	mstr= "MENU %t|Optie1 %x3|Optie2 %x4|Optie3 %x11|Optie4 %x12|Optie5 %x13|Optie 6 %x14";
	DefBut(MENU|SHO,208, mstr,	100, 240, 85, 17,	&menuvar);

	/* IKONEN INLADEN */
	ibuf= loadiffname("/usr/people/blend/blenderbuttons1" , LI_rect);
	DefButIcon(0, ibuf->rect, ibuf->x, ibuf->y, 24, 25);
	ibuf->rect= 0;
	freeImBuf(ibuf);
	
	SetButCol(4);
	DefBut(BUT,208, "ICON 0 0 0",	100,270,26,25);
	DefBut(BUT,208, "ICON 0 0 1",	136,270,26,25);

	SetButCol(3);
	DefBut(BUT,208, "ICON 0 6 1",	100,300,26,25);
	DefBut(BUT,208, "ICON 0 7 1",	136,300,26,25);
	

	tekenkleur=RED;
	fill=0;

	while (run) {
		event= qread(&val);
		switch (event) {
		case LEFTMOUSE:
			if (val) {
				event=DoButtons();
				if(event) {
					if(event==1) tekenkleur=11;
					if(event==3) tekenkleur=kleur;
					/* if(event==203) printf("TOGbit %d\n",bitvar); */
					/* if(event==207) printf("Driestand %d\n",driestandl); */
				}
				else if(winget()==winar[1] ) {

					/* tekenprog */
					kl= (col[2]<<16)+(col[1]<<8)+col[0];
					getdev(2, mdev, mval);
					if(kleur==2) {
						rectot= (unsigned long *)malloc(4*size[0]*size[1]);
						lrectread(0, 0, size[0]-1, size[1]-1, rectot);
						Fill(mval[0]-org[0],mval[1]-org[1],kl,size[0], size[1], rectot);						
						lrectwrite(0, 0, size[0]-1, size[1]-1, rectot);
						free(rectot);
					}
					else {

						x= mval[0];
						y= mval[1];
						if(kleur==1) {
							brush=1; 
							xo=x-org[0]; 
							yo=y-org[1];
							cpack(kl);
							initvierkant(1,10);
							while (getbutton(LEFTMOUSE) == TRUE) {
								getdev(2, mdev, mval);
								x=mval[0]-org[0]; 
								y=mval[1]-org[1];
								if(x!=xo || y!=yo) {
									xx=xo; 
									yy=yo;
									fx=(float)x-xo; 
									fy=(float)y-yo;
									translate(xo,yo,0);
									callobj(1);
									translate(-xo,-yo,0);
								
									tel=sqrt(fx*fx+fy*fy);
									fx= fx/tel; 
									fy=fy/tel;
									for(b1=0;b1<(short)tel;b1++) {
										xx+=fx; 
										yy+=fy;
										xo=(short)xx; 
										yo=(short)yy;
										translate(xo,yo,0);
										callobj(1);
										translate(-xo,-yo,0);
									}
								}
							}
						}
						else if(kleur==0) {
							while (getbutton(LEFTMOUSE) == TRUE) {
								getdev(2, mdev, mval);
								x=mval[0]-org[0]; 
								y=mval[1]-org[1];
								lrectread(x-3,y-3,x-1,y-1,rect);
								filterp(c1,rect);
								lrectread(x-3,y+1,x-1,y+3,rect);
								filterp(c2,rect);
								lrectread(x+1,y+1,x+3,y+3,rect);
								filterp(c3,rect);
								lrectread(x+1,y-3,x+3,y-1,rect);
								filterp(c4,rect);
								bgnpolygon();
								vec[0]=x-2; 
								vec[1]=y-2;
								c3s(c1); 
								v2s(vec);

								vec[1]=y+2;
								c3s(c2); 
								v2s(vec);

								vec[0]=x+2;
								c3s(c3); 
								v2s(vec);

								vec[1]=y-2;
								c3s(c4); 
								v2s(vec);
								endpolygon();
							}
						}
						else {
							char temp[1600],*dt;

							dt= (char *)temp;
							for(x1= -10;x1<11;x1++) {
								xsq= x1*x1;
								for(y1= -10;y1<11;y1++) {
									fx= slif- .1*slif*fsqrt( (float)(xsq+y1*y1) );
									if(fx<0) fx= 0.0;
									dt[0]= fx*col[2];
									dt[1]= fx*col[1];
									dt[2]= fx*col[0];
									dt+=3;
								}
							}
							xo= -32000;
							while (getbutton(LEFTMOUSE) == TRUE) {
								getdev(2, mdev, mval);
								x=mval[0]-org[0]; 
								y=mval[1]-org[1];
								if(xo!=x || yo!=y) {
									lrectread(x-10,y-10,x+10,y+10,rect);
									c=(char *)rect;
									dt= (char *)temp;
									for(x1= -10;x1<11;x1++) {
										for(y1= -10;y1<11;y1++) {
											if(dt[0]>0) {
												b1= c[1] +dt[0];
												if(b1>255) c[1]=255; 
												else c[1]=b1;
											}
											if(dt[1]>0) {
												b1= c[2] +dt[1];
												if(b1>255) c[2]=255; 
												else c[2]=b1;
											}
											if(dt[2]>0) {
												b1= c[3] +dt[2];
												if(b1>255) c[3]=255; 
												else c[3]=b1;
											}
											dt+=3;
											c+=4;
										}
									}
									lrectwrite(x-10,y-10,x+10,y+10,rect);
								}
								xo=x; 
								yo=y;
							}
						}
					}
				}
			}
			break;
		case YKEY:
			if(winget()==winar[1]) {
				copyFtoZ(0,0,399,399,rect,4000);
			}
			break;
		case UKEY:
			if(winget()==winar[1]) {
				copyZtoF(0,0,399,399,rect,4000);
			}
			break;
		case DEPTHCHANGE:
			break;
		case ESCKEY:
			if(val) run= 0;
			break;
		case REDRAW:
			if(winget()==winar[1]) {
				getorigin(&org[0], &org[1]);
				getsize(&size[0], &size[1]);
			}
			else if(winget()==winar[0]) {
				cpack(BGbutcol[2].back); 
				clear(); 
				SetButs(1,1000);
			}
			reshapeviewport();
			break;
		case INPUTCHANGE:
			if(val) winset(val);
			break;
		case WINQUIT:
			FreeButs(); 
			gexit(); 
			exit(0);
		case WINSHUT:
			FreeButs(); 
			gexit(); 
			exit(0);
		}
	}
	FreeButs();
	gexit();
	return 0;
}

