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

 *		effect.c		1992
 * 
 *		d.o.f.			jan 93
 *		edge_enhance	jan 93
 *		spothalo		jan/maart 94
 *
 * 
 */



#include <math.h>
#include <gl/device.h>
#include <string.h>
#include <time.h>
#include <gl/gl.h>
#include "/usr/people/include/Trace.h"
#include <stdlib.h>			/* rand */

struct VV *effvv;

extern void VecMulf(float *v1, float f);
float spglob[3], dxt[3], dyt[3];
short test= 0;

/* ********************* LAMP HALO ******************* */


void spothalo(struct LampRen *lar, float *view, float *intens)
{
	float t0, t1, t2, t3, a, b, c, disc, Normalise(), VecLenf();
	float *npos, nray[3], p1[3], p2[3], p3[3], ladist, maxz, maxy;
	int snijp, doclip=1, use_yco=0;
	int ok1=0, ok2=0, ok3=0;
	
	*intens= 0.0;
	
	npos= lar->sh_invcampos;	/* in initlamp berekend */
	
	/* view roteren */
	VECCOPY(nray, view);
	Mat3MulVecfl(lar->imat, nray);
	
	/* maxz roteren */
	if(R.co[2]==0) doclip= 0;
	else {
		p1[0]= R.co[0]-lar->co[0];
		p1[1]= R.co[1]-lar->co[1];
		p1[2]= R.co[2]-lar->co[2];
	
		maxz= lar->imat[0][2]*p1[0]+lar->imat[1][2]*p1[1]+lar->imat[2][2]*p1[2];
		maxz*= lar->sh_zfac;

		maxy= lar->imat[0][1]*p1[0]+lar->imat[1][1]*p1[1]+lar->imat[2][1]*p1[2];

		if( fabs(nray[2]) <0.000001 ) use_yco= 1;
	}
	
	/* z scalen zodat het volume genormaliseerd is */	
	nray[2]*= lar->sh_zfac;
	/* nray hoeft niet genormaliseerd */

	ladist= lar->sh_zfac*lar->dist;
	
	/* oplossen */
	a = nray[0] * nray[0] + nray[1] * nray[1] - nray[2]*nray[2];
	b = nray[0] * npos[0] + nray[1] * npos[1] - nray[2]*npos[2];
	c = npos[0] * npos[0] + npos[1] * npos[1] - npos[2]*npos[2];

	snijp= 0;
	if (fabs(a) < 0.000001) {
		/*
		 * Only one intersection point...
		 */
		return;
	}
	else {
		disc = b*b - a*c;

		if (disc >= 0.0) {
			disc = fsqrt(disc);
			t1 = (-b + disc) / a;
			t2 = (-b - disc) / a;
			snijp= 2;
		}
	}
	if(snijp) {
		/* sorteren */
		if(t1>t2) {
			a= t1; t1= t2; t2= a;
		}

		/* z van snijpunten met diabolo */
		p1[2]= npos[2] + t1*nray[2];
		p2[2]= npos[2] + t2*nray[2];

		/* beide punten evalueren */
		if(p1[2]<=0.0) ok1= 1;
		if(p2[2]<=0.0 && t1!=t2) ok2= 1;
		
		/* minstens 1 punt met negatieve z */
		if(ok1==0 && ok2==0) return;
		
		/* snijpunt met -ladist, de bodem van de kegel */
		if(use_yco==0) {
			t3= (-ladist-npos[2])/nray[2];
				
			/* moet 1 van de snijpunten worden vervangen? */
			if(ok1) {
				if(p1[2]<-ladist) t1= t3;
			}
			else {
				ok1= 1;
				t1= t3;
			}
			if(ok2) {
				if(p2[2]<-ladist) t2= t3;
			}
			else {
				ok2= 1;
				t2= t3;
			}
		}
		else if(ok1==0 || ok2==0) return;
		
		/* minstens 1 zichtbaar snijpunt */
		if(t1<0.0 && t2<0.0) return;
		
		if(t1<0.0) t1= 0.0;
		if(t2<0.0) t2= 0.0;
		
		if(t1==t2) return;
		
		/* voor zekerheid nog eens sorteren */
		if(t1>t2) {
			a= t1; t1= t2; t2= a;
		}
		
		/* t0 berekenen: is de maximale zichtbare z (als halo door vlak doorsneden wordt) */ 
		if(doclip) {
			if(use_yco==0) t0= (maxz-npos[2])/nray[2];
			else t0= (maxy-npos[1])/nray[1];

			if(t0<t1) return;
		}

		/* bereken punten */
		p1[0]= npos[0] + t1*nray[0];
		p1[1]= npos[1] + t1*nray[1];
		p1[2]= npos[2] + t1*nray[2];
		p2[0]= npos[0] + t2*nray[0];
		p2[1]= npos[1] + t2*nray[1];
		p2[2]= npos[2] + t2*nray[2];
		
			
		/* nu hebben we twee punten, hiermee maken we drie lengtes */
		
		a= fsqrt(p1[0]*p1[0]+p1[1]*p1[1]+p1[2]*p1[2]);
		b= fsqrt(p2[0]*p2[0]+p2[1]*p2[1]+p2[2]*p2[2]);
		c= VecLenf(p1, p2);
		
		a/= ladist;
		a= fsqrt(a);
		b/= ladist; 
		b= fsqrt(b);
		c/= ladist;
		
		*intens= c*( (1.0-a)+(1.0-b) );
		
		/* LET OP: a,b en c NIET op 1.0 clippen, dit geeft kleine
		   overflowtjes op de rand (vooral bij smalle halo's) */
		if(*intens<0.0) return;
		
		/* zachte gebied */
		if(doclip && t0<t2) {
			*intens *= (t0-t1)/(t2-t1);
		}
	}
}



/* ********************* SCRIPT ******************* */

extern void addalphaOver();
extern void addalphaUnder();
extern void addalphaUnderGamma();


void dobluro(struct ImBuf *mbuf, float fac)
{
	/* maak mipmap structuur aan
	 * fac is aantal pixels
	 */
	struct ImBuf *ibuf, *previbuf;
	struct Imap *ima;
	struct MipMap *mm, *mmn;
	float pixsize, maxd, fx, fy, colr, colg, colb, cola;
	float fac1, fac2, fac3, fac4, dx;
	float maxx, minx, maxy, miny, buf1fac, buf2fac;
	ulong *trect;
	int x, y;
	char *rt, *rm;
	
	ima= callocN(sizeof(struct Imap), "doblur");
	ima->ibuf= mbuf;
	
	makemipmap(ima);

	/* bereken welke twee ibufs nodig zjn */
	/* maxd = relatieve afmeting samplegebied */
	
	if(mbuf->x > mbuf->y) maxd= fac/mbuf->x;
	else maxd= fac/mbuf->y;
	if(maxd>0.5) maxd= 0.5;

	pixsize = 2.0 / (float) MAX2(mbuf->x, mbuf->y);

	mm= ima->mipmap;
	previbuf= ibuf= ima->ibuf;
	while(mm) {
		if(maxd <= pixsize) break;
		previbuf= ibuf;
		ibuf= mm->ibuf;
		pixsize= 2.0 / (float)MAX2(ibuf->x, ibuf->y); 
		mm= mm->next;
	}

	buf1fac= 2.0*(pixsize-maxd)/pixsize;
	buf2fac= 1.0-buf1fac;

	trect= mallocN(4*mbuf->x*mbuf->y, "doblur1");
	rt= (char *)trect;
	rm= (char *)mbuf->rect;
	
	for(y=0; y<mbuf->y; y++) {

		/* float coordinaat */
		dx= 1.0/(float)(mbuf->x);
		fx= 0.5*dx;
		fy= (((float)y)+.5) /(float)(mbuf->y);

		maxx= fx+0.5*maxd;
		minx= fx-0.5*maxd;;
		maxy= fy+0.5*maxd;
		miny= fy-0.5*maxd;;

		for(x=0; x<mbuf->x; x++, rt+= 4, rm+= 4) {

			boxsample(ibuf, minx, miny, maxx, maxy, &colr, &colg, &colb, &cola);
			
			if(previbuf!=ibuf) {  /* interpoleren */
				boxsample(previbuf, minx, miny, maxx, maxy, &fac1, &fac2, &fac3, &fac4);

				if(buf1fac>=1.0) {
					cola= fac4; colb= fac3;
					colg= fac2; colr= fac1;
				} else {
					colb= buf2fac*colb+ buf1fac*fac3;
					colg= buf2fac*colg+ buf1fac*fac2;
					colr= buf2fac*colr+ buf1fac*fac1;
					cola= buf2fac*cola+ buf1fac*fac4;
				}
			}
			rt[0]= 255.0*cola;
			rt[1]= 255.0*colb;
			rt[2]= 255.0*colg;
			rt[3]= 255.0*colr;
			
			minx+= dx;
			maxx+= dx;
		}
	}
	
	/* vrijgeven */
	mm= ima->mipmap;
	while(mm) {
		mmn= mm->next;
		freeImBuf(mm->ibuf);
		freeN(mm);
		mm= mmn;
	}
	freeN(ima);
	
	freeN(mbuf->rect);
	mbuf->rect= trect;
	
}

void blurbuf(struct ImBuf *ibuf, int nr)
{
	/* nr = aantal keer verkleinen en vergroten */
	/* vervangt de rect van ibuf */
	struct ImBuf *tbuf, *ttbuf;
	int i, x4;
	
	tbuf= dupImBuf(ibuf);
	x4= ibuf->x/4;
	
	/* verkleinen */
	for(i=0; i<nr; i++) {
		ttbuf = onehalf(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		if(tbuf->x<4 || tbuf->y<4) break;
	}
	
	/* vergroten */
	for(i=0; i<nr; i++) {
		ttbuf = double_x(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		ttbuf = double_y(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		
		if(tbuf->x > x4) {
			scaleImBuf(tbuf, ibuf->x, ibuf->y);
			break;
		}
	}
	
	freeN(ibuf->rect);
	ibuf->rect= tbuf->rect;
	freeN(tbuf);
}

void doblur(struct ImBuf *mbuf, float fac)
{
	/* maak mipmap structuur aan
	 * fac is aantal pixels
	 */
	struct ImBuf *ibuf, *pbuf;
	float ifac, pfac;
	int n, b1, b2;
	char *irect, *prect, *mrect;
	
	/* welke twee buffers zijn nodig? */
				
	if(fac>7.0) fac= 7.0;
	if(fac<=1.0) return;
	
	pfac= 2.0;
	pbuf= dupImBuf(mbuf);
	n= 1;
	while(pfac < fac) {
		blurbuf(pbuf, n);
		blurbuf(pbuf, n);
		
		n++;
		pfac+= 1.0;
	}

	ifac= pfac;
	pfac-= 1.0;
	ibuf= dupImBuf(pbuf);
	blurbuf(ibuf, n);	
	blurbuf(ibuf, n);	
	
	fac= 255.0*(fac-pfac)/(ifac-pfac);
	b1= fac;
	if(b1>255) b1= 255;
	b2= 255-b1;
	
	if(b1==255) {
		freeN(mbuf->rect);
		mbuf->rect= ibuf->rect;
		freeN(ibuf);
		freeImBuf(pbuf);
	}
	else if(b1==0) {
		freeN(mbuf->rect);
		mbuf->rect= pbuf->rect;
		freeN(pbuf);
		freeImBuf(ibuf);
	}
	else {	/* interpoleren */
		n= ibuf->x*ibuf->y;
		irect= (char *)ibuf->rect;
		prect= (char *)pbuf->rect;
		mrect= (char *)mbuf->rect;
		while(n--) {
			mrect[0]= (irect[0]*b1+ prect[0]*b2)>>8;
			mrect[1]= (irect[1]*b1+ prect[1]*b2)>>8;
			mrect[2]= (irect[2]*b1+ prect[2]*b2)>>8;
			mrect[3]= (irect[3]*b1+ prect[3]*b2)>>8;
			mrect+= 4;
			irect+= 4;
			prect+= 4;
		}
		freeImBuf(ibuf);
		freeImBuf(pbuf);
	}
}

void bufblend(rt, ri, b1, b2)
char *rt, *ri;
int b1, b2;
{

	rt[1]= (rt[1]*b1+ ri[1]*b2)>>8;
	rt[2]= (rt[2]*b1+ ri[2]*b2)>>8;
	rt[3]= (rt[3]*b1+ ri[3]*b2)>>8;

}

void bufadd(rt, ri, b1, b2)
char *rt, *ri;
long b1, b2;
{
	long c;
	
	c= rt[1]+ ((ri[1]*b2)>>8);
	if(c>255) rt[1]= 255; else rt[1]= c;
	c= rt[2]+ ((ri[2]*b2)>>8);
	if(c>255) rt[2]= 255; else rt[2]= c;
	c= rt[3]+ ((ri[3]*b2)>>8);
	if(c>255) rt[3]= 255; else rt[3]= c;

}

void bufsub(rt, ri, b1, b2)
char *rt, *ri;
long b1, b2;
{
	long c;
	
	c= rt[1]- ((ri[1]*b2)>>8);
	if(c<0) rt[1]= 0; else rt[1]= c;
	c= rt[2]- ((ri[2]*b2)>>8);
	if(c<0) rt[2]= 0; else rt[2]= c;
	c= rt[3]- ((ri[3]*b2)>>8);
	if(c<0) rt[3]= 0; else rt[3]= c;

}

void onlyinblack(rt, ri, b1, b2)
ulong *rt, *ri;
long b1, b2;
{
	long c;
	
	if(*rt==0) *rt= *ri;
}

void addalphaOverNA(rt, ri, b1, b2)		/* Normal Alpha */
char *rt, *ri;
long b1, b2;
{
	long c;
	/* premullen */

	ri[0]= (b2*ri[0])>>8;
	c= ri[0];
	ri[1]= (c*ri[1])>>8;
	ri[2]= (c*ri[2])>>8;
	ri[3]= (c*ri[3])>>8;
	
	addalphaOver(rt, ri);
}

void addalphaUnderNA(rt, ri, b1, b2)		/* Normal Alpha */
char *rt, *ri;
long b1, b2;
{
	long c;

	ri[0]= (b2*ri[0])>>8;
	ri[1]= (b2*ri[1])>>8;
	ri[2]= (b2*ri[2])>>8;
	ri[3]= (b2*ri[3])>>8;
	
	addalphaUnderGamma(rt, ri, b1, b2);
}

void dobufscript()
{
	extern void rectcpy(), rectfill();
	void (*blendfunc)();
	struct ImBuf *ibuf, *ibuf2;
	FILE *fp;
	float x1,x2,fac,ease();
	ulong *rt, *ri;
	long cfra,sf,ef, ofs,b1,b2, x, y;
	short end=1,scr,ipo;
	char *strp, str[100], ipostr[20];

	strcpy(str,G.bufscr);
	convertstringcode(str);
	fp=fopen(str,"r");
	if(fp==NULL) {
		error("Can't open script");
		return;
	}
	while(end>0) {
		cfra= G.cfra;
		/* cfra= G.framelen*G.cfra; OOK HIERONDER NOG EEN KEER */
		if(cfra<1) cfra= 1;

		end=fscanf(fp,"%s",str);
		if(end<=0) break;
		scr=0;
					/* operators op R.rectot */
		if( strcmp(str, "LOAD")==0) scr=1;
		else if( strcmp(str, "BLEND")==0) scr=2;
		else if( strcmp(str, "ADDALPHAOVER")==0) scr=3;
		else if( strcmp(str, "ADDALPHAUNDER")==0) scr=4;
		else if( strcmp(str, "ONLYINBLACK")==0) scr=5;
		else if( strcmp(str, "LOADCROP")==0) scr=6;		/* LET OP: begin-eind waarde is x-y ofset */
		else if( strcmp(str, "DOUBLECROSS")==0) scr=7;	/* LET OP: eerst plaatje laden met LOAD */
		else if( strcmp(str, "ADD")==0) scr=10;
		else if( strcmp(str, "SUB")==0) scr=11;
		else if( strcmp(str, "BLUR_LOAD")==0) scr=101;
		/* else if( strcmp(str, "BLUR_BLEND")==0) scr=102; */
		/* else if( strcmp(str, "BLUR_ADDALPHAOVER")==0) scr=103; */
		
		else if( strcmp(str,"END")==0 ) {
			end= -1234;
			break;
		}

		if(scr) {
			end= fscanf(fp,"%s",str); 
			if(end<0) break;
			if( str[strlen(str)-1]=='/' ) {
				end= fscanf(fp, "%d", &ofs); 
				if(end<=0) break;
				/* ofs= G.framelen*(G.cfra+ofs); afgeschakeld!! */
				ofs= (G.cfra+ofs);
				if(ofs<1) ofs= 1;
				makepicstring2(str, ofs);
			}
			end= fscanf(fp,"%d %d",&sf,&ef); 
			if(end<=0) break;
			end= fscanf(fp,"%f %f",&x1,&x2); 
			if(end<=0) break;
			end= fscanf(fp,"%s",ipostr); 
			if(end<=0) break;

			ipo=0;
			if(ipostr[0]=='i') ipo=1;
			else if(ipostr[0]=='c') ipo=2;

			if(ipo) strp= ipostr+1;
			else strp= ipostr;
			if( strcmp(strp,"EASE")==0 ) {
				end=fscanf(fp,"%d %d",&b1,&b2); 
				if(end<=0) break;
			}
			else b1=b2=0;

			if( (cfra>=sf && cfra<=ef) || (ef==0 && sf==0) ) {
				fac=ef-sf;
				if(fac==0) fac=1;
				else {
					if(b1!=0 || b2!=0) {
						fac=ease((float)cfra-sf,(float)ef-sf,(float)b1,(float)b2);
					} else {
						fac=(cfra-sf)/fac;
						if(fac<0) fac=0;
						else if(fac>1) fac=1;
					}
				}
			}
			else if(ipo==1) fac= -1;
			else if(cfra<sf) fac=0;
			else fac=1;

			if(fac>=0) {
				ibuf= loadiffname(str,LI_rect);
				if(ibuf) {
					if(R.rectx!=ibuf->x || R.recty!=ibuf->y) {
						if(scr==6) {
							ibuf2= allocImBuf(R.rectx, R.recty, 32, 1, 0);
							rectoptot(ibuf2, 0, rectfill, 0);
							x= (R.rectx-ibuf->x)/2 +x1;
							y= (R.recty-ibuf->y)/2 +x2;
							if(y & 1) y--;
							rectop(ibuf2, ibuf, x, y, 0, 0, 32767, 32767, rectcpy);
							freeImBuf(ibuf);
							ibuf= ibuf2;
							scr= 1;
						}
						else scalefastImBuf(ibuf, R.rectx, R.recty);
					}

					if(scr>100) {
						doblur(ibuf, (1-fac)*x1+fac*x2);
						scr-= 100;
						
					}

					if(scr==1) {
						memcpy(R.rectot, ibuf->rect, 4*R.rectx*R.recty);
					}
					else {
						if(scr==2) blendfunc= bufblend;
						else if(scr==3) blendfunc= addalphaOverNA;
						else if(scr==4) blendfunc= addalphaUnderNA;
						else if(scr==5) blendfunc= onlyinblack;
						else if(scr==7) blendfunc= bufblend;
						else if(scr==10) blendfunc= bufadd;
						else if(scr==11) blendfunc= bufsub;

						fac= (1-fac)*x1+fac*x2;

						b2= (long)fac;
						if(b2>255) b2= 255;
						b1= 255- b2;

						if(scr==7) {	/* doublecross */
							/* x1= ((float)b1)/255.0; */
							/* x2= ((float)b2)/255.0; */
							/* b1= 255.0*(3*x1*x1-2*x1*x1*x1); */
							/* b2= 255.0*(3*x2*x2-2*x2*x2*x2); */
							b1= (b1*b1)>>8;
							b2= (b2*b2)>>8;
						}
						
						rt= R.rectot;
						ri= ibuf->rect;
						for(y=0;y<R.recty;y++) {
							for(x=0;x<R.rectx;x++, rt++, ri++) {
								blendfunc(rt, ri, b1, b2);
							}
						}
					}
				}
				else {
					error("BufScript: can't load image");
					end= -1;
					break;
				}
				freeImBuf(ibuf);
			}
		}
		if(G.afbreek) {
			if(getbutton(LEFTSHIFTKEY)) break;
		}
	}
	/*if(G.afbreek==0 && end!= -1234) error("Syntax error");*/
	fclose(fp);

}

void doscript_geenfield()
{
	struct Base *base;
	FILE *fp;
	float x1,x2,fac,ease();
	long nnr,cfra,sf,ef,b1,b2,ofs,nr, tbl, delta;
	short end=1,scr,ipo,ok, viewchanged= 0;
	char str[100],naam[40],data[40],ipostr[40], string1[256], string2[256], stringn[256],*strp,*adr;

	data[0]= 0;

	strcpy(str,G.scr);
	convertstringcode(str);
	fp=fopen(str,"r");
	if(fp==NULL) {
		error("Can't open script");
		return;
	}
	while(end>0) {

		end=fscanf(fp,"%s",str);
		if(end<=0) break;
		scr=0;
		if( strcmp(str,"BYTE")==0 ) scr=1;
		else if( strcmp(str,"WORD")==0 ) scr=2;
		else if( strcmp(str,"LONG")==0 ) scr=3;
		else if( strcmp(str,"FLOAT")==0 ) scr=4;
		else if( strcmp(str,"DOUBLE")==0 ) scr=5;
		else if( strcmp(str,"STRING")==0 ) scr=6;
		else if( strcmp(str,"TEXT")==0 ) scr=7;
		else if( strcmp(str,"LOAD")==0 ) scr=10;
		else if( strcmp(str,"REND")==0 ) scr=11;
		else if( strcmp(str,"ANIM")==0 ) scr=12;
		else if( strcmp(str,"PATCH")==0 ) scr=13;
		else if( strcmp(str,"PATCH2")==0 ) scr=14;
		else if( strcmp(str,"END")==0 ) {
			end= -1234;
			break;
		}

		if(scr>=1 && scr<=7) {
			nnr= 0;
			end=fscanf(fp,"%s %d",naam,&nnr); 
			if(end<0) break;
			end=fscanf(fp,"%s %d",data,&ofs); 
			if(end<=0) break;
			end=fscanf(fp,"%d %d",&sf,&ef); 
			if(end<=0) break;

			if (scr <= 5) {
				end=fscanf(fp,"%f %f",&x1,&x2); 
			} else {
				end=fscanf(fp," %256[^#]# %256[^#]# ", string1, string2);
			}
			if(end<=0) break;
			end=fscanf(fp,"%s",ipostr); 
			if(end<=0) break;
			
			ipo=0;
			if(ipostr[0]=='i') ipo=1;
			else if(ipostr[0]=='c') ipo=2;
			else if(ipostr[0]=='p') ipo=3;

			if(ipo) strp= ipostr+1;
			else strp= ipostr;
			if( strcmp(strp,"EASE")==0 ) {
				end=fscanf(fp,"%d %d",&b1,&b2); 
				if(end<=0) break;
			}
			else b1=b2=0;

			/* dit stuk (tot aan do ) moet hier staan, als meerdere bases
			 * worden veranderd,  wordt dit maar 1 x uitgerekend.
			 */
			cfra= G.framelen*G.cfra;
			if(cfra<1) cfra= 1;

				/* cyclic */
			if(ipo==2 || ipo==3) {
				delta= abs(ef-sf+1);
				while(cfra>ef) cfra-= delta;
				while(cfra<sf) cfra+= delta;
				
				if(ipo==3) {	/* pingpong */
					if(cfra<= (sf+ef)/2 ) {
						ef= (sf+ef)/2;
					}
					else {
						sf= (sf+ef)/2;
						ef+= 1;
						fac= x1; x1= x2; x2= fac;
					}
				}
			}

			/* loopt eerst af voor TEX, VIEW, WRLD dan de bases */
			base= 0;
			do {
				ok= 0;
				if(base==0) {
					if(naam[0]=='~') ok= 1;
				}
				else if( strncmp(naam,base->str,14)==0 ) {
					if(nnr== -1) ok= 1;
					else if(nnr==base->nnr) ok= 1;
				}
				if(ok) {
					adr=0;
					if(strcmp(data,"BASE")==0) adr= (char *)base;
					if(strcmp(data,"BADA")==0) adr= (char *)base->d;
					if(strcmp(data,"VIEW")==0) {
						adr= (char *)&view0;
						viewchanged= 1;
					}
					if(strcmp(data,"WRLD")==0) adr= (char *)&wrld;
					if(strncmp(data,"TEX", 3)==0) {
						tbl= atol( data+3 );
						if(tbl>0 && tbl<=G.totex) adr= (char *)G.adrtex[tbl];
					}
					if(adr==0) {
						end=EOF;
						break;
					}
					adr+=ofs;
					
					if( (cfra>=sf && cfra<=ef) || (ef==0 && sf==0) ) {
						fac=ef-sf;
						if(fac==0) fac=1;
						else {
							if(b1!=0 || b2!=0) {
								fac=ease((float)cfra-sf,(float)ef-sf,(float)b1,(float)b2);
							} else {
								fac=(cfra-sf)/fac;
								if(fac<0) fac=0;
								else if(fac>1) fac=1;
							}
						}
					}
					else if(ipo==1) fac= -1;
					else if(cfra<sf) fac=0;
					else fac=1;
					if(fac>=0) {
						if (scr==4) {
							*(float *)adr= (1-fac)*x1+fac*x2;
						} else if (scr == 6) {
							strcpy(adr, string1);
						} else if (scr == 7) {
							adr = (char *) (((struct ObData *) adr)->fo);
							if (adr) {
								adr = (char *) (((struct FontData *) adr)->str);
								if (adr) {
									long len1, len2, i;
									float f1, f2;
									
									len1 = strlen(string1);
									len2 = strlen(string2);
									if (len1 != len2) {
										if (len1 > len2) strcpy(stringn, string1);
										else strcpy(stringn, string2);
										len1 = len1 + 0.5 + fac * (len2 - len1);
										stringn[len1] = 0;
									} else {
										for (i = 0; i < len1; i++) {
											f1 = string1[i];
											f2 = string2[i];
											stringn[i] = f1 + 0.5 + fac * (f2 - f1);
										}
										stringn[len1] = 0;
									}
									
									strcpy(adr, stringn);
									makepolytext(base);
									extrudepoly(base);
								}
							}
						} else {
							nr= ffloor( (1-fac)*x1+fac*x2+.1 );
							if(scr==1) *adr= nr;
							else if(scr==2) *((short *)adr)= nr;
							else if(scr==3) *((long *)adr)= nr;
						}
					}
				}
				if(base==0) {
					if(naam[0]!='~')  base= G.firstbase;
				}
				else base= base->next;
			} while(base);
		}
		else if(scr==10) {   /* load */
			end=fscanf(fp,"%s",str); 
			if(end<0) break;
			leesscene(str,0);
			if(G.winar[0]) projektie();
		}
		else if(scr==11) {   /* rend */
			end=fscanf(fp,"%d",&b1); 
			if(end<0) break;
			G.cfra= b1;
			initrender(0);
		}
		else if(scr==12) {   /* anim */
			initrender(R.anim);
		}
		else if(scr==13) {   /* patch */
			G.genlock = 0;
		}
		else if(scr==14) {   /* patch */
			G.genlock = 1;
		}

		if(G.afbreek) {
			if(getbutton(LEFTSHIFTKEY)) break;
		}
	}
	/*if(G.afbreek==0 && end!= -1234) error("Syntax error");*/
	fclose(fp);
	
	if(viewchanged && !G.background) {
		SetButs(111, 125);
	}
}


void doscript(long field)
{
	float ipo(), frametotime();
	struct Base *base;
	FILE *fp;
	float x1,x2,fac,ease();
	long nnr,tsf,tef,b1,b2,ofs,nr, tbl, delta;
	float cfra, sf, ef;
	short end=1,scr,ipo1,ok, viewchanged= 0;
	char str[100],naam[40],data[40],ipostr[40], string1[256], string2[256], stringn[256],*strp,*adr;
	float add = 0.0;
	long debug = FALSE;
	
	data[0]= 0;

	if(G.background==0 && G.localview) return;

	strcpy(str,G.scr);
	convertstringcode(str);
	fp=fopen(str,"r");
	if(fp==NULL) {
		error("Can't open script");
		return;
	}
	while(end>0) {

		end=fscanf(fp,"%s",str);
		if(end<=0) {
			if (debug) printf("end: %d\n", end);
			break;
		}
		scr=0;
		
		if( strcmp(str,"BYTE")==0 ) scr=1;
		else if( strcmp(str,"WORD")==0 ) scr=2;
		else if( strcmp(str,"LONG")==0 ) scr=3;
		else if( strcmp(str,"FLOAT")==0 ) scr=4;
		else if( strcmp(str,"DOUBLE")==0 ) scr=5;
		else if( strcmp(str,"STRING")==0 ) scr=6;
		else if( strcmp(str,"TEXT")==0 ) scr=7;
		else if( strcmp(str,"LOAD")==0 ) scr=10;
		else if( strcmp(str,"REND")==0 ) scr=11;
		else if( strcmp(str,"ANIM")==0 ) scr=12;
		else if( strcmp(str,"CAMERA")==0 ) scr=13;
		else if( strcmp(str,"DEBUG")==0 ) {debug = TRUE; scr = -1; printf("debug on\n");}
		else if( strcmp(str,"+0.0")==0 ) add = 0.0;
		else if( strcmp(str,"+0.5")==0 ) add = 0.5;
		else if( strcmp(str,"+1.0")==0 ) add = 1.0;
		else if( strcmp(str,"END")==0 ) {
			end= -1234;
			break;
		}
		
		if (scr == 0 && debug) printf("skipping: %s\n", str);
		
		if(scr>=1 && scr<=7) {
			nnr= 0;
			end=fscanf(fp,"%s %d",naam,&nnr); 
			if(end<0) break;
			end=fscanf(fp,"%s %d",data,&ofs); 
			if(end<=0) break;
			end=fscanf(fp,"%d %d",&tsf,&tef); 
			if(end<=0) break;
			sf = tsf;
			ef = tef + add;
			
			if (scr <= 5) {
				end=fscanf(fp,"%f %f",&x1,&x2); 
			} else {
				end=fscanf(fp," %256[^#]# %256[^#]# ", string1, string2);
			}
			if(end<=0) break;
			end=fscanf(fp,"%s",ipostr); 
			if(end<=0) break;
			
			ipo1=0;
			if(ipostr[0]=='i') ipo1=1;
			else if(ipostr[0]=='c') ipo1=2;
			else if(ipostr[0]=='p') ipo1=3;

			if(ipo1) strp= ipostr+1;
			else strp= ipostr;
			if( strcmp(strp,"EASE")==0 ) {
				end=fscanf(fp,"%d %d",&b1,&b2); 
				if(end<=0) break;
			}
			else b1=b2=0;

			/* dit stuk (tot aan do ) moet hier staan, als meerdere bases
			 * worden veranderd,  wordt dit maar 1 x uitgerekend.
			 */
			
			cfra= frametotime(G.cfra);
			if(cfra < 1.0) cfra= 1.0;

				/* cyclic */
			if(ipo1==2 || ipo1==3) {
				delta= abs(ef-sf+1);
				while(cfra>ef) cfra-= delta;
				while(cfra<sf) cfra+= delta;
				
				if(ipo1==3) {	/* pingpong */
					if(cfra<= (sf+ef)/2.0 ) {
						ef= (sf+ef)/2.0;
					}
					else {
						sf= (sf+ef)/2;
						ef+= 1;
						fac= x1; x1= x2; x2= fac;
					}
				}
			}

			/* loopt eerst af voor TEX, VIEW, WRLD dan de bases */
			base= 0;
			do {
				ok= 0;
				if(base==0) {
					if(naam[0]=='~') ok= 1;
				}
				else if( strncmp(naam,base->str,14)==0 ) {
					if(nnr== -1) ok= 1;
					else if(nnr==base->nnr) ok= 1;
				}
				if(ok) {
					adr=0;
					if(strcmp(data,"BASE")==0) adr= (char *)base;
					if(strcmp(data,"BADA")==0) adr= (char *)base->d;
					if(strcmp(data,"VIEW")==0) {
						adr= (char *)&view0;
						if(ofs==80) viewchanged= 1;
					}
					if(strcmp(data,"WRLD")==0) adr= (char *)&wrld;
					if(strncmp(data,"TEX", 3)==0) {
						tbl= atol( data+3 );
						if(tbl>0 && tbl<=G.totex) adr= (char *)G.adrtex[tbl];
					}
					if(adr==0) {
						end=EOF;
						break;
					}
					adr+=ofs;
					
					if( (cfra>=sf && cfra<=ef) || (ef==0 && sf==0) ) {
						fac=ef-sf;
						if(fac==0) fac=1;
						else {
							if(b1!=0 || b2!=0) {
								fac=ease((float)cfra-sf,(float)ef-sf,(float)b1,(float)b2);
							} else {
								fac=(cfra-sf)/fac;
								if(fac<0) fac=0;
								else if(fac>1) fac=1;
							}
						}
					}
					else if(ipo1==1) fac= -1;
					else if(cfra<sf) fac=0;
					else fac=1;
					if(fac>=0) {
						if (scr==4) {
							*(float *)adr= (1-fac)*x1+fac*x2;
						} else if (scr == 6) {
							strcpy(adr, string1);
						} else if (scr == 7) {
							adr = (char *) (((struct ObData *) adr)->fo);
							if (adr) {
								adr = (char *) (((struct FontData *) adr)->str);
								if (adr) {
									long len1, len2, i;
									float f1, f2;
									
									len1 = strlen(string1);
									len2 = strlen(string2);
									if (len1 != len2) {
										if (len1 > len2) strcpy(stringn, string1);
										else strcpy(stringn, string2);
										len1 = len1 + 0.5 + fac * (len2 - len1);
										stringn[len1] = 0;
									} else {
										for (i = 0; i < len1; i++) {
											f1 = string1[i];
											f2 = string2[i];
											stringn[i] = f1 + 0.5 + fac * (f2 - f1);
										}
										stringn[len1] = 0;
									}
									
									strcpy(adr, stringn);
									makepolytext(base);
									extrudepoly(base);
								}
							}
						} else {
							nr= ffloor( (1.0-fac)*x1+fac*x2+.5 );
							if(scr==1) *adr= nr;
							else if(scr==2) *((short *)adr)= nr;
							else if(scr==3) *((long *)adr)= nr;
						}
					}
				}
				if(base==0) {
					if(naam[0]!='~')  base= G.firstbase;
				}
				else base= base->next;
			} while(base);
		}
		else if(scr==10) {   /* load */
			end=fscanf(fp,"%s",str); 
			if(end<0) break;
			leesscene(str,0);
			if(G.winar[0]) projektie();
		}
		else if(scr==11) {   /* rend */
			end=fscanf(fp,"%d",&b1); 
			if(end<0) break;
			G.cfra= b1;
			initrender(0);
		}
		else if(scr==12) {   /* anim */
			initrender(R.anim);
		}
		else if(scr==13) {	 /* camera */
			end=fscanf(fp,"%s %d",naam, &nnr); 
			if(end<0) break;
			end=fscanf(fp,"%d %d",&tsf,&tef);
			if(end<0) break;

			cfra= frametotime(G.cfra);
			if( tsf<=cfra && tef>=cfra) {
				/* op zoek naar de camera met deze naam */
				base= G.firstbase;
				while(base) {
					if(base->nnr==nnr && strncmp(naam, base->str, 14)==0) {
						view0.cambase= base;
					}
					base= base->next;
				}
			}
		}

		if(G.afbreek) {
			if(getbutton(LEFTSHIFTKEY)) break;
		}
	}
	/*if(G.afbreek==0 && end!= -1234) error("Syntax error");*/
	fclose(fp);
	
	if(viewchanged && !G.background) {
		SetButs(111, 125);
	}
}

void speedtest1()
{
	float fx, fy, minx, maxx, miny, maxy;


	spglob[0]= R.co[0];
	spglob[1]= R.co[1];
	spglob[2]= R.co[2];
	VECCOPY(dxt, spglob);
	VECCOPY(dyt, spglob);
	
	switch(test) {
	case 0:
		fx = (spglob[0] + 32768.0) / 65536.0;
		fy = (spglob[1] + 32768.0) / 65536.0;
		dxt[0]/=65536.0; 
		dxt[1]/=65536.0;
		dyt[0]/=65536.0; 
		dyt[1]/=65536.0;
		break;
	case 1:
		break;
	}

	if(fx<0.0) fx= 0.0; else if(fx>1.0) fx= 1.0;
	if(fy<0.0) fy= 0.0; else if(fy>1.0) fy= 1.0;

	/* pixel coordinaten */

	minx= MIN3(dxt[0],dyt[0],dxt[0]+dyt[0] );
	maxx= MAX3(dxt[0],dyt[0],dxt[0]+dyt[0] );
	miny= MIN3(dxt[1],dyt[1],dxt[1]+dyt[1] );
	maxy= MAX3(dxt[1],dyt[1],dxt[1]+dyt[1] );
}

void speedtest()
{
	long a;
	
	for(a=0; a<20; a++) {
		speedtest1();
	}
}

void initeffects()
{

	/* initspeakerdata(); */

}

void bgneffect(vv)
struct VV *vv;
{
	long len;

	len= sizeof(struct VV)+vv->vert*sizeof(struct VertOb)+vv->vlak*sizeof(struct VlakOb);
	effvv= (struct VV *)mallocN(len,"bgneffect");
	memcpy(effvv,vv,len);

}

void endeffect(vv)
struct VV *vv;
{
	long len;

	len= sizeof(struct VV)+vv->vert*sizeof(struct VertOb)+vv->vlak*sizeof(struct VlakOb);
	memcpy(vv,effvv,len);
	freeN(effvv);
	effvv= 0;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
short build(cur,tot,base, len) /* nog primitief */
short cur,tot;
struct Base *base;
short len;
{
	short cfra, grens;

	cfra= G.cfra-(base->sf-1);
	if(cfra<0) return 0;

	if( cfra >= base->len ) return 1;
	if( cfra==G.sfra) return 0;

	grens= (cfra*tot)/base->len;
	if(cur>=grens) return 0;
	if(cur< grens-len) return 0;
	return 1;
}

int build_schiphol(base, tot)	/* geeft aantal vlakken terug op current frame */
struct Base *base;
int tot;
{
	float frametofloat();
	float fac;
	
	fac= frametofloat( (float)G.cfra, base->sf, base->len, base->f2 & 1);
	fac*= tot;
	
	return ( (int)fac );
}



/* VLAKLAMP */


float calcStokefactor(lar, nor)
struct LampRen *lar;
float *nor;
{
	float tvec[3], cent[3], fac;
	float vec[4][3];	/* vectoren van shootcent naar vertices rp */
	float cross[4][3];	/* uitprodukten hiervan */
	float rad[4];	/* hoeken tussen vecs */

	/* vlak: lar->face[4][3] bestaat niet meer! Include veranderen!
	 * cent: R.co
	 * norm: nor
	 */
	VECCOPY(cent, R.co);

	/* test op richting */
	/* VecSubf(tvec, shoot->cent, rp->cent); */
	/* if( tvec[0]*shoot->norm[0]+ tvec[1]*shoot->norm[1]+ tvec[2]*shoot->norm[2]>0.0) */
	/* 	return 0.0; */

	/* hoekvectors */
	/* VecSubf(vec[0], cent, lar->face[0]); */
	/* VecSubf(vec[1], cent, lar->face[1]); */
	/* VecSubf(vec[2], cent, lar->face[2]); */
	/* VecSubf(vec[3], cent, lar->face[3]); */

	Normalise(vec[0]);
	Normalise(vec[1]);
	Normalise(vec[2]);
	Normalise(vec[3]);

	/* uitprod */
	Crossf(cross[0], vec[0], vec[1]);
	Crossf(cross[1], vec[1], vec[2]);
	Crossf(cross[2], vec[2], vec[3]);
	Crossf(cross[3], vec[3], vec[0]);
	Normalise(cross[0]);
	Normalise(cross[1]);
	Normalise(cross[2]);
	Normalise(cross[3]);

	/* hoeken */
	rad[0]= vec[0][0]*vec[1][0]+ vec[0][1]*vec[1][1]+ vec[0][2]*vec[1][2];
	rad[1]= vec[1][0]*vec[2][0]+ vec[1][1]*vec[2][1]+ vec[1][2]*vec[2][2];
	rad[2]= vec[2][0]*vec[3][0]+ vec[2][1]*vec[3][1]+ vec[2][2]*vec[3][2];
	rad[3]= vec[3][0]*vec[0][0]+ vec[3][1]*vec[0][1]+ vec[3][2]*vec[0][2];

	rad[0]= facos(rad[0]);
	rad[1]= facos(rad[1]);
	rad[2]= facos(rad[2]);
	rad[3]= facos(rad[3]);

	/* Stoke formule */
	VecMulf(cross[0], rad[0]);
	VecMulf(cross[1], rad[1]);
	VecMulf(cross[2], rad[2]);
	VecMulf(cross[3], rad[3]);

	fac= nor[0]*cross[0][0]+ nor[1]*cross[0][1]+ nor[2]*cross[0][2];
	fac+= nor[0]*cross[1][0]+ nor[1]*cross[1][1]+ nor[2]*cross[1][2];
	fac+= nor[0]*cross[2][0]+ nor[1]*cross[2][1]+ nor[2]*cross[2][2];
	fac+= nor[0]*cross[3][0]+ nor[1]*cross[3][1]+ nor[2]*cross[3][2];

	return (-fac);
}







/* ***************** EDGE ENHANCE ****************************** */


void subtractcol(c1, c2)
char *c1, *c2;
{
	long val;
	
	val= c1[1]-c2[1];
	if(val<0) c1[1]= 0; else c1[1]= val;
	val= c1[2]-c2[2];
	if(val<0) c1[2]= 0; else c1[2]= val;
	val= c1[3]-c2[3];
	if(val<0) c1[3]= 0; else c1[3]= val;
}

void edge_enhance(osa)
long osa;
{
	/* maak van zbuffer eerste afgeleide */
	float fval;
	long val, y, x, col, *rz, *rnew, *rz1, *rz2, *rz3;
	char *cp;
	
	/* alle getallen in zbuffer 3 naar rechts shiften */
	rz= (long *)R.rectz;
	if(rz==0) return;
	
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, rz++) {
			(*rz)>>= 3;
		}
	}

	/* eerste order:
			val= abs(rz1[0]-rz2[1])+ 2*abs(rz1[1]-rz2[1])+ abs(rz1[2]-rz2[1]);
			val+= 2*abs(rz2[0]-rz2[1])+ 2*abs(rz1[2]-rz2[1]);
			val+= abs(rz3[0]-rz2[1])+ 2*abs(rz3[1]-rz2[1])+ abs(rz3[2]-rz2[1]);
			*rz= val;
	*/
	
	rz1= (long *)R.rectz;
	rz2= rz1+R.rectx;
	rz3= rz2+R.rectx;
	rz= (long *)R.rectot+R.rectx;

	if(G.ali) {
		cp= (char *)(R.rectaccu+R.rectx);
	}
	else {
		cp= (char *)(R.rectot+R.rectx);
	}
	
	for(y=0; y<R.recty-2; y++) {

		rz++;
		for(x=0; x<R.rectx-2; x++, rz++, rz1++, rz2++, rz3++, cp+=4) {
			col= abs(8*rz2[1]-rz1[0]-rz1[1]-rz1[2]-rz2[0]-rz2[2]-rz3[0]-rz3[1]-rz3[2])/3;
			col= (R.edgeint*col)>>14;
			if(col>255) col= 255;
			if(col) {
				if(G.ali) {
					col/= G.osa;
					
					val= cp[0]+col;
					if(val>255) cp[0]= 255; else cp[0]= val;
					
					if(R.f & 16) {	/* transp!! */
						val= cp[1]- col;
						if(val<0) cp[1]= 0; else cp[1]= val;
						val= cp[2]- col;
						if(val<0) cp[2]= 0; else cp[2]= val;
						val= cp[3]- col;
						if(val<0) cp[3]= 0; else cp[3]= val;
					}				
					
				}
				else {
					val= cp[1]- col;
					if(val<0) cp[1]= 0; else cp[1]= val;
					val= cp[2]- col;
					if(val<0) cp[2]= 0; else cp[2]= val;
					val= cp[3]- col;
					if(val<0) cp[3]= 0; else cp[3]= val;
				}
			}
		}
		rz++;
		rz1+= 2;
		rz2+= 2;
		rz3+= 2;
		cp+= 8;
	}

}

/* ***************** D.O.F. ****************************** */

struct zxysort {
	long z;
	short x, y;
};

long vergzval(x1,x2)
struct zxysort *x1,*x2;
{

	if( x1->z > x2->z ) return -1;
	else if( x1->z < x2->z) return 1;
	return 0;
}

void setzvalues_dof()
{
	struct PixStr *ps;
	ulong *rd;
	long *rz, z, x, y, aantal, tot;

	rz= (long *)R.rectz;
	rd= R.rectdaps;
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, rd++, rz++) {

			if(*rd & 0xFF000000) { /* is PS */
			
				ps= (struct PixStr *)*rd;
				
				while(ps) {

					if(*rz> (long)(ps->z) ) *rz= (long)(ps->z);
					
					ps= ps->next;
				}
			}
		}
	}
}

float *disttab, *dofbuf;

void addtodofbufo(xc, yc, z, col, rad, buf, minz, maxz, distz)
long xc, yc, z;
char *col;
float rad, *buf;
long minz, maxz, distz;
{
	float *db, *dtab, weight, dist, r, g, b;
	long x, y, mul, *rz, ofs, temp, xmin, xmax, ymin, ymax;
	
	temp= xc+0.5;
	xmin= ffloor(temp-rad);
	xmax= ffloor(temp+rad);
	temp= yc+0.5;
	ymin= ffloor(temp-rad);
	ymax= ffloor(temp+rad);
	
	if(xmin<0) xmin= 0;
	if(ymin<0) ymin= 0;
	if(xmax>R.rectx-1) xmax= R.rectx-1;
	if(ymax>R.recty-1) ymax= R.recty-1;
	
	/* threshold */
	z-= 1000;

	r= col[3];
	g= col[2];
	b= col[1];
	mul= wrld.dofi+1;
/*
	if(rad>1.0) {
		if(rad>4.0) {r= 255; g= 0; b= 0;}
		else if(rad>3.0) {g= 255; r= 0; b= 0;}
		else if(rad>2.0) {b= 255; g= 0; r= 0;}
		else {r= 255; g= 255; b= 0;}
	}
*/
	
	for(y= ymin; y<=ymax; y++) {
		ofs= (y*R.rectx+xmin);
		db= buf+4*ofs;
		rz= (long *)(R.rectz+ofs);
		dtab= disttab+ mul*abs(y-yc) ;
		for(x= xmin; x<=xmax; x++, db+=4, rz++) {
			if(*rz>minz) {
				/* afstand centrum - centrum */
				dist= *(dtab+abs(x-xc));
				
				if(dist==0.0) weight= 1.0;
				else if(dist < rad-0.5) weight= 1.0;
				else if(dist>rad+0.5) weight= 0.0;
				else {
					weight= rad+0.5-dist;
				}
				if(weight!=0.0) {
					weight/= 6.3*rad*rad;
					if(*rz>maxz || *rz>z) {
						db[0]+= weight;
						db[1]+= weight*b;
						db[2]+= weight*g;
						db[3]+= weight*r;
					}
					else  {
						dist= *rz-minz;
						dist/= ((float)(z-minz));
						weight*= fsqrt(fsqrt(dist));
						db[0]+= weight;
						db[1]+= weight*b;
						db[2]+= weight*g;
						db[3]+= weight*r;
					}
				}
			}
		}
	}
}

float calcz_dof(vec)
float *vec;
{
	float hoco[4];

	projectverto(vec, hoco);
	if(hoco[2]>hoco[3] || hoco[2]<0.0) return 0.0;
    else {
		return (float)0x7FFFFFFF *(hoco[2]/hoco[3]);
	}
	
}

void dofo()
{
	
	/* alleen als geen parts */
	float size, filtsize, *dofbuf, *db, vec[3], hoco[4];
	ulong *rz;
	ulong minz, maxz, staz, endz, distz;
	long x, y, min;
	char *rt;
	
	db= disttab= mallocN(4*(wrld.dofi+1)*(wrld.dofi+1), "dof");
	for(y=0; y<=wrld.dofi; y++) {
		for(x=0; x<=wrld.dofi; x++, db++) {
			*db= fsqrt( (float)(x*x+y*y) );
		}
	}
	
	rt= (char *)R.rectot;
	rz= R.rectz;
	db= dofbuf= callocN(4*4*R.rectx*R.recty, "dof1");
	
	/* near= 0 en far= 0x7FFFFFFF */
	vec[0]= vec[1]= 0.0;
	vec[2]= -R.near- 1000*wrld.dofmin;
	minz= calcz_dof(vec);	
	vec[2]= -R.near- 1000*wrld.dofmax;
	maxz= calcz_dof(vec);
	vec[2]= -R.near- 1000*wrld.dofsta;
	staz= calcz_dof(vec);
	vec[2]= -R.near- 1000*wrld.dofend;
	endz= calcz_dof(vec);

	distz= maxz-minz;
	
	filtsize= wrld.dofi;
	
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, rz++, rt+=4, db+=4) {
			if(*rz<minz) {
				db[0]+= 1.0;
				db[1]+= rt[1];
				db[2]+= rt[2];
				db[3]+= rt[3];
			}
			else {
				if(*rz>maxz || distz==0) size= filtsize;
				else {
					size= *rz-minz;
					size/= ((float)distz);
					
					size*= filtsize;
				}
				addtodofbufo(x, y, *rz, rt, size, dofbuf, minz, maxz, distz);
			}
		}
		if(G.afbreek) break;
	}
	
	rt= (char *)R.rectot;
	db= dofbuf;
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, db+=4, rt+=4) {
			if(db[0]==1.0) ;
			else if(db[0]!=0.0) {
				rt[0]= 255;
				rt[1]= db[1]/db[0];
				rt[2]= db[2]/db[0];
				rt[3]= db[3]/db[0];
			}
		}
	}
	freeN(dofbuf);
	freeN(disttab);

	/* test */
	/* printf("sta \n"); */
	/* qsort(R.rectz,R.rectx*R.recty,4,vergzval); */
	/* printf("end \n"); */
}


void addtodofbuf(xc, yc, rad)
long xc, yc;
float rad;
{
	float *db, *dtab, weight, dist, r, g, b;
	long x, y, mul, ofs, temp, xmin, xmax, ymin, ymax;
	char *col;	
	
	temp= xc+0.5;
	xmin= ffloor(temp-rad);
	xmax= ffloor(temp+rad);
	temp= yc+0.5;
	ymin= ffloor(temp-rad);
	ymax= ffloor(temp+rad);
	
	if(xmin<0) xmin= 0;
	if(ymin<0) ymin= 0;
	if(xmax>R.rectx-1) xmax= R.rectx-1;
	if(ymax>R.recty-1) ymax= R.recty-1;
	
	col= (char *)(R.rectot+yc*R.rectx+xc);
	r= col[3];
	g= col[2];
	b= col[1];
	
	mul= wrld.dofi+1;

	/* if(rad>1.0) { */
	/* 	if(rad>4.0) {r= 255; g= 0; b= 0;} */
	/* 	else if(rad>3.0) {g= 255; r= 0; b= 0;} */
	/* 	else if(rad>2.0) {b= 255; g= 0; r= 0;} */
	/* 	else {r= 255; g= 255; b= 0;} */
	/* } */
	
	
	for(y= ymin; y<=ymax; y++) {
		ofs= (y*R.rectx+xmin);
		db= dofbuf+4*ofs;
		dtab= disttab+ mul*abs(y-yc) ;
		for(x= xmin; x<=xmax; x++, db+=4) {
			/* afstand centrum - centrum */
			dist= *(dtab+abs(x-xc));
			
			if(dist==0.0) weight= 1.0;
			else if(dist < rad-0.5) weight= 1.0;
			else if(dist>rad+0.5) weight= 0.0;
			else {
				weight= rad+0.5-dist;
			}
			if(weight!=0.0) {
				weight/= 6.3*rad*rad;
	
				db[0]+= weight;
				db[1]+= weight*b;
				db[2]+= weight*g;
				db[3]+= weight*r;
			}
		}
	}	
}

void dof()
{
	/* alleen als geen parts */
	struct zxysort *sb, *sortbuf;
	float size, filtsize, *db, vec[3], distz1, distz2;
	ulong *rz;
	ulong minz, maxz, staz, endz;
	long x, y, ofs;
	char *rt;
	
	ofs= 0;	/* error */
	if(wrld.dofmin>=wrld.dofsta) ofs= 1;
	if(wrld.dofsta>=wrld.dofend) ofs= 1;
	if(wrld.dofend>=wrld.dofmax) ofs= 1;
	if(ofs) {
		error("DOF: min<sta<end<max");
		return;
	}

	/* afstandtabel */
	db= disttab= mallocN(4*(wrld.dofi+1)*(wrld.dofi+1), "dof");
	for(y=0; y<=wrld.dofi; y++) {
		for(x=0; x<=wrld.dofi; x++, db++) {
			*db= fsqrt( (float)(x*x+y*y) );
		}
	}
	
	
	/* near= 0 en far= 0x7FFFFFFF */
	/* bereken minz en maxz */
	vec[0]= vec[1]= 0.0;
	vec[2]= -R.near- 1000*wrld.dofmin;
	minz= calcz_dof(vec);	
	vec[2]= -R.near- 1000*wrld.dofmax;
	maxz= calcz_dof(vec);
	vec[2]= -R.near- 1000*wrld.dofsta;
	staz= calcz_dof(vec);
	vec[2]= -R.near- 1000*wrld.dofend;
	endz= calcz_dof(vec);

	distz1= staz-minz;
	distz2= maxz-endz;
	
	filtsize= wrld.dofi;
	
	/* maak rect met Zvalues en x,y. Sorteren */
	sb=sortbuf= mallocN(sizeof(struct zxysort)*R.rectx*R.recty, "dof0");
	rz= R.rectz;
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, rz++, sb++) {
			sb->z= *rz;
			sb->x= x;
			sb->y= y;
		}
	}
	
	qsort(sortbuf, R.rectx*R.recty, sizeof(struct zxysort), vergzval);
	
	/* accumuleren in dofbuf, is (voorlopig) float */
	db= dofbuf= callocN(4*4*R.rectx*R.recty, "dof1");

	sb= sortbuf;

	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, sb++) {
			
			/* z binnen scherptegebied? */
			if(sb->z>=staz && sb->z<=endz) {
				ofs=  sb->y*R.rectx + sb->x;
				rt= (char *)(R.rectot+ofs);
				db= dofbuf+4*ofs;
				db[0]+= 1.0;
				db[1]+= rt[1];
				db[2]+= rt[2];
				db[3]+= rt[3];
			}
			else {
				/* afmeting filtercirkel */
				if(sb->z<=minz || sb->z>=maxz) {
					size= 1.0;
				}
				else if(sb->z<staz) {
					size= staz- sb->z;
					size= size/distz1;
				}
				else {
					size= sb->z-endz;
					size= size/distz2;
				}
				size*= filtsize;
				
				if(size>0.5) {
					addtodofbuf(sb->x, sb->y, size);
				} else {
					ofs=  sb->y*R.rectx + sb->x;
					rt= (char *)(R.rectot+ofs);
					db= dofbuf+4*ofs;
					db[0]+= 1.0;
					db[1]+= rt[1];
					db[2]+= rt[2];
					db[3]+= rt[3];
				}

			}
		}

		if(G.afbreek) break;
	}
	
	rt= (char *)R.rectot;
	db= dofbuf;
	for(y=0; y<R.recty; y++) {
		for(x=0; x<R.rectx; x++, db+=4, rt+=4) {
			if(db[0]==1.0) ;
			else if(db[0]!=0.0) {
				rt[0]= 255;
				rt[1]= db[1]/db[0];
				rt[2]= db[2]/db[0];
				rt[3]= db[3]/db[0];
			}
		}
	}
	freeN(dofbuf);
	freeN(disttab);
	freeN(sortbuf);
}


/* *********************************************** */




void lenVVcorrect(vv1,vv2)	/* corrigeert vv1 met vv2 */
struct VV *vv1,*vv2;
{
	struct VertOb *adrve1,*adrve2,*v1,*v2;
	struct VlakOb *adrvl1,*adrvl2;
	float fac;
	long a,c,dx,dy,dz,dis1,dis2,cx,cy,cz;

	adrve1= (struct VertOb *)(vv1+1);
	adrve2= (struct VertOb *)(vv2+1);
	
	for(c=0;c<3;c++) {
		adrvl1= (struct VlakOb *)(adrve1+vv1->vert);
		for(a=0;a<vv1->vlak;a++) {
			if(c==0) {
				v1= adrve2+ adrvl1->v1;
				v2= adrve2+ adrvl1->v2;
			} else if(c==1) {
				v1= adrve2+ adrvl1->v2;
				v2= adrve2+ adrvl1->v3;
			} else {
				v1= adrve2+ adrvl1->v3;
				v2= adrve2+ adrvl1->v1;
			}
			
			dis1= abs(v1->c[0]-v2->c[0]);
			dis1+= abs(v1->c[1]-v2->c[1]);
			dis1+= abs(v1->c[2]-v2->c[2]);
			if(c==0) {
				v1= adrve1+ adrvl1->v1;
				v2= adrve1+ adrvl1->v2;
			} else if(c==1) {
				v1= adrve1+ adrvl1->v2;
				v2= adrve1+ adrvl1->v3;
			} else {
				v1= adrve1+ adrvl1->v3;
				v2= adrve1+ adrvl1->v1;
			}
			dx= (v1->c[0]-v2->c[0]);
			dis2= abs(dx);
			dy= (v1->c[1]-v2->c[1]);
			dis2+= abs(dy);
			dz= (v1->c[2]-v2->c[2]);
			dis2+= abs(dz);
			if(dis1!=dis2) {
				
				fac= 0.5- .2*((float)(dis2))/((float)(dis1));
				if(fac>.9) fac= .9;
				if(fac<.1) fac= .1;
				dx*=fac;
				dy*=fac;
				dz*=fac;
				cx= (v1->c[0]+v2->c[0])/2;
				cy= (v1->c[1]+v2->c[1])/2;
				cz= (v1->c[2]+v2->c[2])/2;

				v1->c[0]= cx+dx;
				v2->c[0]= cx-dx;
				v1->c[1]= cy+dy;
				v2->c[1]= cy-dy;
				v1->c[2]= cz+dz;
				v2->c[2]= cz-dz;
			}
			adrvl1++;
		}
	}

}

void vlag(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	float hoek,si,co,fac,frametofloat(),x,y,z;
	long a;

	ob= (struct ObData *)base->d;
	vv= ob->vv;
	
	fac= frametofloat( (float)G.cfra, base->sf, base->len, base->f2 & 1);
	fac*= 2*PI;

	hoek= 15*PI/180.0;
	si= fsin(hoek);
	co= fcos(hoek);

	adrve= (struct VertOb *)(vv+1);
	for(a=0;a<vv->vert;a++) {
		x= adrve->c[0];
		
		x= co*x;

		x/= (10767.0/PI);

		z= 0.12*(32767+adrve->c[0])*(fsin(x-fac));

		adrve->c[2]= z;
		adrve++;
	}
}

void vlagdiag(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	float hoek,si,co,fac,frametofloat(),x,y,z;
	long a;

	ob= (struct ObData *)base->d;
	vv= ob->vv;
	
	fac= frametofloat( (float)G.cfra,base->sf,base->len,base->f2 & 1);
	fac*= 2*PI;

	hoek= 15*PI/180.0;
	si= fsin(hoek);
	co= fcos(hoek);

	adrve= (struct VertOb *)(vv+1);
	for(a=0;a<vv->vert;a++) {
		x= adrve->c[0];
		y= adrve->c[1];
		z= adrve->c[2];
		
		x= co*x+ si*y;
		y= -si*adrve->c[0]+ co*y;

		x/= (16767.0/PI);
		y/= (16767.0/PI);
		z/= (16767.0/PI);
		
		x+= fsin(x)/2.0;
		z= 0.2*(32767+adrve->c[0])*(fsin(x-fac));

		adrve->c[2]= z;
		adrve++;
	}
}

void vlago(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	float hoek,si,co,fac,fac2,frametofloat(),x,y,z;
	long a;

	ob= (struct ObData *)base->d;
	vv= ob->vv;

	hoek= 15*PI/180.0;
	si= fsin(hoek);
	co= fcos(hoek);
	
	fac= frametofloat( (float)G.cfra,base->sf,base->len,base->f2 & 1);
	fac*= 2*PI;

	adrve= (struct VertOb *)(vv+1);
	for(a=0;a<vv->vert;a++) {
		x= adrve->c[0];
		y= adrve->c[1];
		z= adrve->c[2];
		
		x= co*x+ si*y;
		y= -si*x+ co*y;

		x/= (32767.0/PI);
		
		fac2= (32767.0+ (float)adrve->c[0])/32767.0;

		x+= fcos(x)/2;
		z= fac2*6000.0*(fsin(-2.5*x+fac));
		adrve->c[0]= (fac2-1)*30200+ fac2*z/26;
		adrve->c[1]+= fac2*(z/16);
		adrve->c[2]= z;

		adrve++;
	}
}


void flexa()
{
	float fac1,fac2,fac3,breedkw,*data,*vec;
	ulong *rect,*rd;
	short absx,absy,cx,cy,dx,dy,a,x,y,mval[2];
	short breed,breeddy;
	Device mdev[2];
	
	ortho2(-.49,R.rectx-.5,-.49,R.recty-.5);	/* voor zekerheid */

	mdev[0] = MOUSEX;
    	mdev[1] = MOUSEY;
	breed=25;
	breedkw= breed*breed;
	breeddy= 2*(2*breed+1);


	getdev(2, mdev, mval);
	cx= mval[0]-R.xof; cy=mval[1]-R.yof;

	if((cx-breed<0) || (cx+breed>R.rectx) || (cy-breed<0) || (cy+breed>R.recty)) {
		return;
	}	
	
	
	rect=(ulong *)mallocN(4*(2*breed)*(2*breed)+breed, "rt");
	data=(float *)mallocN(8*(2*breed+1)*(2*breed+1)+breed, "rt");

	lrectread(cx-breed,cy-breed,cx+breed-1,cy+breed-1,rect);

	while(getbutton(LEFTMOUSE)) {

		/* bereken dx en dy */
		getdev(2, mdev, mval);
		dx= -cx+mval[0]-R.xof; dy= -cy+mval[1]-R.yof;
		if(dx<-breed) dx= -breed;
		else if(dx>breed) dx=breed;
		if(dy<-breed) dy= -breed;
		else if(dy>breed) dy=breed;
	
		/* init vertexdata */
		vec= data;
		for(y= -breed;y<=breed;y++) {
			for(x= -breed;x<=breed;x++) {
				absx= abs(x);
				absy= abs(y);
				if(absx==breed || absy==breed) {
					vec[0]= cx+x;
					vec[1]= cy+y;
				} else {

					/* afstand tot centrum kwadraat */
					fac1= x*x+y*y;
					fac1= 1.0-fac1/breedkw;

					fac2= 1.0- absx/(float)breed;
					fac3= 1.0- absy/(float)breed;
					fac2= fsqrt(fac2);
					fac3= fsqrt(fac3);

					fac1=fac2*fac3;

					vec[0]= cx+x+ (fac1)*dx;
					vec[1]= cy+y+ (fac1)*dy;
				}
				vec+=2;
			}
		}

		/* tekenpolys */
		vec= data;
		rd= rect;
		for(y= -breed;y<breed;y++) {
			bgnqstrip();
				for(x= -breed;x<=breed;x++) {
					cpack(*rd);
					if(x!=breed-1) rd++;
					v2f(vec);
					v2f(vec+breeddy);
					vec+=2;
				}
			endqstrip();
		}
	}

	freeN(rect);
	freeN(data);

	if(getbutton(LEFTSHIFTKEY)) lrectread(0,0,R.rectx-1,R.recty-1,R.rectot);
}

void scalefastrect(recto, rectn, oldx, oldy, newx, newy)
ulong *recto, *rectn;
short oldx, oldy, newx,newy;
{
	ulong *rect, *newrect;
	long x,y;
	long ofsx,ofsy,stepx,stepy;

	stepx = (65536.0 * (oldx - 1.0) / (newx - 1.0)) + 0.5;
	stepy = (65536.0 * (oldy - 1.0) / (newy - 1.0)) + 0.5;
	ofsy = 32768;
	newrect= rectn;
	
	for (y = newy; y > 0 ; y--){
		rect = recto;
		rect += (ofsy >> 16) * oldx;
		ofsy += stepy;
		ofsx = 32768;
		for (x = newx ; x>0 ; x--){
			*newrect++ = rect[ofsx >> 16];
			ofsx += stepx;
		}
	}
}



void zoomwin()
{
	static float zoom=3.0;
	ulong *rz,*rect;
	long q_event;
	short x,y,cx,cy,cxo=12000,cyo=0,lus=1,val,mval[2];
	Device mdev[2];	
	
	mdev[0] = MOUSEX;
    	mdev[1] = MOUSEY;

	if(R.rectot==0) return;
	
	zbuffer(0);
	zdraw(1);
	lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot);
	zdraw(0);

	rectzoom(zoom,zoom);
	while(lus) {
		getdev(2, mdev, mval);
		cx= mval[0]-R.xof;
		cy=mval[1]-R.yof;

		x= R.rectx/(2*zoom)+1;
		y= R.recty/(2*zoom)+1;

		if(cx<x) cx=x;
		if(cy<y) cy=y;
		if(cx>R.rectx-x) cx=R.rectx-x;
		if(cy>R.recty-y) cy=R.recty-y;
		if(cx==cxo && cy==cyo);
		else {
			cxo=cx; cyo=cy;
			readsource(SRC_ZBUFFER);
			rectcopy(cx-x,cy-y,cx+x,cy+y,0,0);
			readsource(SRC_AUTO);
			finish();	/* wachten tot GLpipe klaar is */
		}
		while(qtest()!=0 && lus==1) {
			q_event=traces_qread(&val);
			if(val) {
				if(q_event==ESCKEY) lus=0;
				else if(q_event==INPUTCHANGE) {
					lus=0;
					qenter(q_event,val);
				}
				else if(q_event==PADMINUS) {
					zoom-=1.0; cxo= 0;
					if(zoom<2.0) zoom=2.0;
					else rectzoom(zoom,zoom);
				}
				else if(q_event==PADPLUSKEY) {
					zoom+=1.0;
					if(zoom>15.0) zoom=15.0;
					else rectzoom(zoom,zoom);
					cxo=0;
				}
				else if(q_event==JKEY) {
					if(R.rectspare==0) R.rectspare= (ulong *)callocN(4*R.rectx*R.recty, "rectot");
					SWAP(ulong *, R.rectspare, R.rectot);
					
					/* naar zbuf schrijven */
					rectzoom(1.0, 1.0);
					zbuffer(0);
					zdraw(1);
					lrectwrite(0, 0, R.rectx-1, R.recty-1, R.rectot);
					zdraw(0);
					
					rectzoom(zoom, zoom);
					cxo= 12000;	/* redraw */
				}
			}
		}
		sginap(1);
	}

	rectzoom(1.0,1.0);
	readsource(SRC_ZBUFFER);
	rectcopy(0,0,R.rectx-1,R.recty-1,0,0);
	readsource(SRC_AUTO);
	/* lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot); */

}



void husselverts_o()
{
	struct VV *vv;
	struct VertOb *adrve,*adrven,*ve;
	struct VlakOb *adrvl;
	float mult;
	long ra;
	short a,new,oud,*hash,*h;

	if(G.ebase!=0 || G.basact==0 || G.basact->soort!= 1) return;
	if(okee("Hash vertices")==0) return;

	vv= ((struct ObData *)G.basact->d)->vv;

	/* array maken van husselgetallen */
	h= hash= (short *)mallocN(2*vv->vert,"husselverts");
	for(a=0;a<vv->vert;a++) {
		*(h++)=a;
	}

	srandom(1);
	mult= ((float)vv->vert)/32768.0;
	h= hash;
	for(a=0;a<vv->vert;a++) {
		new= (short) (mult*( (float)(random() & 32767) ));
		if(new>=0 && new<vv->vert) {
			oud= *h;
			*h= hash[new];
			hash[new]= oud;
		}
		h++;
	}

	/* nieuw temp vertexblock */
	adrven= (struct VertOb *)mallocN(vv->vert*sizeof(struct VertOb),"temphussel");
	ve= adrve= (struct VertOb *)(vv+1);
	h= hash;
	for(a=0;a<vv->vert;a++) {
		memcpy((adrven+ *h),ve,sizeof(struct VertOb));
		h++;
		ve++;
	}

	/* nieuw naar oud moven */
	memcpy(adrve,adrven,vv->vert*sizeof(struct VertOb));

	/* vlakpointers goedzetten */
	adrvl= (struct VlakOb *)(adrve+vv->vert);	
	for(a=0;a<vv->vlak;a++) {
		adrvl->v1= hash[adrvl->v1];
		adrvl->v2= hash[adrvl->v2];
		adrvl->v3= hash[adrvl->v3];
		adrvl++;
	}

	freeN(hash);
	freeN(adrven);
}

struct VertOb *verg_vert;

int verg_hoogste_zco(x1, x2)
struct VlakOb *x1,*x2;
{
	int z1, z2;
	struct VertOb *v1, *v2, *v3;
	
	v1= verg_vert+x1->v1;
	v2= verg_vert+x1->v2;
	v3= verg_vert+x1->v3;
	z1= MAX3(v1->c[2], v2->c[2], v3->c[2]);
	
	v1= verg_vert+x2->v1;
	v2= verg_vert+x2->v2;
	v3= verg_vert+x2->v3;
	z2= MAX3(v1->c[2], v2->c[2], v3->c[2]);

	if( z1 > z2 ) return 1;
	else if( z1 < z2) return -1;
	return 0;
}


void sortfaces()
{
	int a;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	
	if(G.ebase!=0 || G.basact==0 || G.basact->soort!= 1) return;
	if(okee("Sort faces")==0) return;

	vv= ((struct ObData *)G.basact->d)->vv;

	adrve= verg_vert= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+vv->vert);	

	qsort(adrvl, vv->vlak, sizeof(struct VlakOb), verg_hoogste_zco);
}

int vergbaseco(x1,x2)
struct Base **x1,**x2;
{

	if( (*x1)->sy > (*x2)->sy ) return 1;
	else if( (*x1)->sy < (*x2)->sy) return -1;
	else if( (*x1)->sx > (*x2)->sx ) return 1;
	else if( (*x1)->sx < (*x2)->sx ) return -1;

	return 0;
}

void renamebase()
{
	struct Base *base,**basesort,**bs;
	short start, tot=0, a, delta = -1;

	if(G.basact==0) return;
	if(G.qual & 3) {
		delta= 1;
		if(okee("Automatic startframes")==0) return;
		start= G.basact->sf;
		if(button(&start,1,1000,"Start")==0) return;
		if(button(&delta,0,100,"Step")==0) return;
	}
	else {
		if(okee("Rename & number bases")==0) return;
		start= G.basact->nnr;
	}

	/* maak lijst van alle bases, xco yco (scherm) */
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			tot++;
		}
		base= base->next;
	}

	bs= basesort= (struct Base **)mallocN(4*tot,"renamebase");
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			*bs= base;
			bs++;
		}
		base= base->next;
	}
	qsort(basesort,tot,4,vergbaseco);

	bs= basesort;
	for(a=0; a<tot; a++) {
		if(delta != -1) {
			(*bs)->sf= start;
			start+= delta;
		}
		else {
			if(*bs!=G.basact) strcpy((*bs)->str,G.basact->str);
			(*bs)->nnr= start++;
		}
		bs++;
	}
	freeN(basesort);
	
	projektie();
}


void golf(base)
struct Base *base;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *a1;
	float f1, cfra, div1, div2;
	short a,b,c,size,*mem,*point,val1,val2,val3;

	if(base->soort!=1) return;
	
	ob= (struct ObData *)base->d;
	vv= ob->vv;	
	size = sqrt(vv->vert);

	mem=(short *) mallocN(2*vv->vert,"golfmem");
	if (mem==0) return;
	
	/* random grid */
	srand(1);
	point=mem;
	cfra= G.cfra;
	if(G.f & 64) cfra+= .5;
	
	f1= ((float)base->len)/80.0;
	div1= 11.0*f1;
	div2= 13.0*f1;
	
	for(a=0; a<vv->vert; a++) {
		f1 =  rand()*sin( cfra/div1 + rand()/1000.0 )/60.0;
		f1 += rand()*sin( cfra/div2 + rand()/1000.0 )/60.0;
		*(point++) = f1;
	}
	
	/* filteren: */
	for(c=2;c>0;c--){
		for(a=0;a<size;a++){	
			point = mem + a;
			val1 = *point;
			val2 = val1;
			for (b=size-1;b>0;b--){
				val3 = point[size];
				*point = (val1 + (val2<<1) + val3) >> 2;
				val1 = val2;
				val2 = val3;
				point += size;
			}
			*point = (val2 + 3*val3) >> 2;
		}

		point = mem;
		for(a=0;a<size;a++){
			val1 = *point;
			val2 = val1;
			for (b=size-1;b>0;b--){
				val3 = point[1];
				*point = (val1 + (val2<<1) + val3) >> 2;
				val1 = val2;
				val2 = val3;
				point += 1;
			}
			*(point++) = (val2 + 3*val3) >> 2;
		}
	}
	
	/* in z waardes vertices schrijven */
	a1=(struct VertOb *)(vv+1);
	point=mem;
	for(a=0;a<vv->vert;a++) {
		a1->c[2] = *(point++);
		a1++;
	}
	freeN(mem);

}

void vertexTex(struct ObData *ob)
{
	struct Tex *rtex;
	struct VertOb *ve;
	float b2,t[3],vec[3],hnoise(),ofs;
	int tot, nr;

	if(ob->ef!=3) return;
	nr= ob->elen;
	if(G.totex<nr) return;
	if(nr==0) return;
	
	rtex= G.adrtex[nr];
	ofs= rtex->var[1];
	
	ve= (struct VertOb *)((ob->vv)+1);
	tot= ob->vv->vert;
	
	while(tot--) {

		VECCOPY(t, ve->c);
		if(rtex->soort!=8) {
			externtex(nr, t, &b2);
			vec[0]= 0.0;
			vec[1]= 0.0;
			vec[2]= b2;
		}
		else {
			b2= hnoise(rtex->var[0],t[0],t[1],t[2]);
			if(rtex->type > 1) ofs*=(b2*b2);
			vec[0]= b2-hnoise(rtex->var[0],t[0]+ofs,t[1],t[2]);
			vec[1]= b2-hnoise(rtex->var[0],t[0],t[1]+ofs,t[2]);
			vec[2]= b2-hnoise(rtex->var[0],t[0],t[1],t[2]+ofs);
		}
		t[0]+= 4000*vec[0];
		if(t[0]<-32767) t[0]= -32767; if(t[0]>32767) t[0]= 32767;
		t[1]+= 4000*vec[1];
		if(t[1]<-32767) t[1]= -32767; if(t[1]>32767) t[1]= 32767;
		t[2]+= 4000*vec[2];
		if(t[2]<-32767) t[2]= -32767; if(t[2]>32767) t[2]= 32767;
		
		if(view0.grid==777) {
			if(ve->c[2]>0) VECCOPY(ve->c, t);
		}
		else VECCOPY(ve->c, t);
		
		ve++;
	}
}

extern unsigned char hash[512];

/* - er moet een 'vast' aantal sterren gegenereerd worden tussen near en far.
 * - alle sterren moeten bij voorkeur op de far liggen en uitsluitend in
 *   helderheid / kleur verschillen.
 * - 
 */

void make_stars(wire)
long wire;
{
	float vec[4], fx, fy, fz;
	long x, y, z, sx, sy, sz, ex, ey, ez, done = 0;
	float minx, maxx, miny, maxy, minz, maxz, fac, starmindist;
	double dblrand, hlfrand;
	ushort seed[3];
	float mat[4][4];
	struct HaloRen *har;
	extern struct HaloRen *initstar();	
	
	
	float stargrid = 1000 * wrld.rt5[0];		/* om de hoeveel een ster ? */
	float maxrand = 2.0;						/* hoeveel mag een ster verschuiven (uitgedrukt in grid eenheden) */
	int maxjit = 2 * wrld.rt5[1];			/* hoeveel mag een kleur veranderen */
	float force = 100.0 * wrld.rt5[2];			/* hoe sterk zijn de sterren */

	float far = 1000.0 * view0.far;
	float near = view0.near;
	
	/* schermafmeting sterren */
	force= ( (float)wrld.rt5[2] )/20.0;
	force*= ( (float)R.xsch )/640.0;
	
	/* minimale vrije ruimte */
	starmindist= 1000.0*wrld.starmindist;

	if (wrld.rt5[0] <= 10) return;
	if (wire == 0) R.f |= 1;
	else stargrid*= 2.0;				/* tekent er minder */
	
	minx = miny = minz = HUGE;
	maxx = maxy = maxz = -HUGE;
	
	Mat4Invert(mat, G.viewmat);
	
	/* BOUNDINGBOX BEREKENING
	 * bbox loopt van z = near | far,
	 * x = -z | +z,
	 * y = -z | +z
	 */
	 
	for (z = 0; z <= 1; z++) {
		if (z) fz = - far;
		else fz = - near;
		for (y = -1; y <= 1; y += 2){
			for (x = -1; x <= 1; x += 2){
				vec[0] = x * fz;
				vec[1] = y * fz;
				vec[2] = fz;
				vec[3] = 1.0;
					/*printf("%.2f %.2f %.2f       ", vec[0], vec[1], vec[2]);*/
				Mat4MulVecfl(mat, vec);
					/*printf("%.2f %.2f %.2f\n", vec[0], vec[1], vec[2]);*/
				if (vec[0] < minx) minx = vec[0];
				if (vec[0] > maxx) maxx = vec[0];
				if (vec[1] < miny) miny = vec[1];
				if (vec[1] > maxy) maxy = vec[1];
				if (vec[2] < minz) minz = vec[2];
				if (vec[2] > maxz) maxz = vec[2];
			}
		}
	}
	
/*	printf("x: %.2f %.2f %.0f\n", minx, maxx, (maxx - minx) / stargrid);
	printf("y: %.2f %.2f %.0f\n", miny, maxy, (maxy - miny) / stargrid);
	printf("z: %.2f %.2f %.0f\n", minz, maxz, (maxz - minz) / stargrid);
*/	
	/* omzetten naar grid coordinaten */
	
	sx = (minx / stargrid) - maxrand;
	sy = (miny / stargrid) - maxrand;
	sz = (minz / stargrid) - maxrand;
	
	ex = (maxx / stargrid) + maxrand;
	ey = (maxy / stargrid) + maxrand;
	ez = (maxz / stargrid) + maxrand;	

	dblrand = maxrand * stargrid;
	hlfrand = 2.0 * dblrand / RAND_MAX;
	/*hlfrand = 2.0 * dblrand / 1.0;*/

	if (wire) {
		cpack(-1);
		bgnpoint();
	}
	
	for (x = sx, fx = sx * stargrid; x <= ex; x++, fx += stargrid) {
		for (y = sy, fy = sy * stargrid; y <= ey ; y++, fy += stargrid) {
			for (z = sz, fz = sz * stargrid; z <= ez; z++, fz += stargrid) {
				
				if( fabs(fx)+fabs(fy)+fabs(fz) >starmindist ) {
				
					srand((hash[z & 0xff] << 24) + (hash[y & 0xff] << 16) + (hash[x & 0xff] << 8));
					vec[0] = fx + (hlfrand * rand()) - dblrand;
					vec[1] = fy + (hlfrand * rand()) - dblrand;
					vec[2] = fz + (hlfrand * rand()) - dblrand;
					vec[3] = 1.0;
	
					if (wire) {
						v3f(vec);
						done++;
					} else {
						Mat4MulVecfl(G.viewmat, vec);
						
						if (vec[2] <= -near) {
							minx = (far + vec[2]) / (far - near);
							if (minx < 0.0) minx = 0.0;
							if (minx > 1.0) minx = 1.0;
		
							if(minx>0.0) {
								minx= fsqrt(fsqrt(minx));
								
								fac= force * rand();
								fac/= RAND_MAX;
								har = initstar(vec, fac);
								
								if (har) {
									har->alfa = 255.0 * minx;
									har->r = har->g = har->b = 255;
									if (maxjit) {
										har->r += ((maxjit * rand()) / RAND_MAX) - maxjit;
										har->g += ((maxjit * rand()) / RAND_MAX) - maxjit;
										har->b += ((maxjit * rand()) / RAND_MAX) - maxjit;
									}
									har->type = 32;	/* even getal, geen 'ster' */
		
									har->f= 2;		/* only in sky */
									done++;
								}
							}
						}
					}
				}
			}
			if(G.afbreek) break;
		}
		if(G.afbreek) break;
	}
	
	if (wire) endpoint();
	/*printf("done: %d\n", done);*/
}

/* ************************* SCHUIFPUZZEL **************** */

float schuifset[4][4][2];	/* coordinaten stukjes */
int sch_x, sch_y;			/* huidige gat */
int sch_lastframe= 0;

#define SCH_GRIDX 1600;
#define SCH_GRIDY 900;



void sch_moveto(int newx, int newy, float fac)
{
	int a, b, c, deltax, deltay;
	float start, end, delta;
	
	if(newx<0 || newy<0 || newx>3 || newy>3) {
		printf("error in schuifscript: moveto %d %d\n", newx, newy);
		return;
	}
	
	deltax= newx-sch_x;
	deltay= newy-sch_y;
	
	/* alle stukjes in interval <sch_x,newx> moeten verplaatst */
	
	/* gat wordt: */
	schuifset[3][0][0]= newx;
	schuifset[3][0][1]= newy;
	
	if(deltax) {
		
		if(deltax>0) {
			start= sch_x+1;
			end= newx;
			delta= -fac;
		}
		else {
			start= newx;
			end= sch_x-1;
			delta= fac;
		}

		for(a=0; a<4; a++) {
			for(b=0; b<4; b++) {
				if( ffloor(schuifset[a][b][1]+0.5) == (float)newy ) {
					if( ffloor(schuifset[a][b][0]+0.5) >= start && ffloor(schuifset[a][b][0]+0.5) <= end ) {
						schuifset[a][b][0] += delta;
					}
				}
			}
		}
	}
	else if(deltay) {
		
		if(deltay>0) {
			start= sch_y+1;
			end= newy;
			delta= -fac;
		}
		else {
			start= newy;
			end= sch_y-1;
			delta= fac;
		}
		
		for(a=0; a<4; a++) {
			for(b=0; b<4; b++) {
				
				if( ( (int)ffloor(schuifset[a][b][0]+0.5)) == newx ) {
					if( ffloor(schuifset[a][b][1]+0.5) >= start && ffloor(schuifset[a][b][1]+0.5) <= end ) {
						schuifset[a][b][1] += delta;
					}
				}
			}
		}
	}
	sch_x= newx;
	sch_y= newy;
}

void schuifpuzzel_set()
{
	FILE *fp;
	float frametotime();
	float cfra, fac;
	int a, b, newx, newy, sf, ef, end=1, delta;
	char str[80];
	
	/* SYNTAX SCRIPT: 
	 * 
	 * START x
	 *   x is startframe
	 * 
	 * DELTA x
	 *   x is delta, startframe wordt iedere moveto opgehoogd,  ef= sf+delta
	 * 
	 * MOVETO x y
	 *   x en y coordinaat gat
	 */
	 
	
	strcpy(str, "//schuifscript");
	convertstringcode(str);
	fp= fopen(str, "r");
	if(fp==NULL) {
		printf("can't read schuifscript\n");
		return;
	}


	/* initialiseren */
	for(b=0; b<4; b++) {
		for(a=0; a<4; a++) {
			schuifset[a][b][0]= a;
			schuifset[a][b][1]= b;
		}
	}
	sch_x= 3;
	sch_y= 0;
	/* waar dus [3][0] staat is het gat! */
	
	cfra= frametotime(G.cfra);
	
	while(end>0) {
		
		end= fscanf(fp, "%s ", str);
		if(end<0) break;

		if(strncmp(str, "START", 5)==0) {
			end= fscanf(fp, "%d", &sf);
		}
		else if(strncmp(str, "DELTA", 5)==0) {
			end= fscanf(fp, "%d", &delta);
		}
		else if(strncmp(str, "MOVETO", 6)==0) {
			end= fscanf(fp, "%d %d", &newx, &newy, &sf, &ef);
			if(end<0) break;
		
			ef= sf+delta;
		
			if( ((float)sf) > cfra ) break;
			
			if( cfra < ((float)ef) ) {
				fac= (cfra -sf);
				fac/= (float)delta;
				
				fac= 3.0*fac*fac-2.0*fac*fac*fac;
			}
			else fac= 1.0;
			
			sch_moveto(newx, newy, fac);
			
			sf+= delta;
		}
	}
	
	fclose(fp);
}

void schuifpuzzel_base(struct Base *base)
/* leest schuifset en schrijft locatie in base */
{
	int a, b;
	float xfac, yfac;
	
	if( sch_lastframe != G.cfra ) {
		schuifpuzzel_set();
		sch_lastframe= G.cfra;
	}
	
	switch( base->nnr ) {
	case 0:
		a= 0; b= 3;
		break;
	case 1:
		a= 1; b= 3;
		break;
	case 2:
		a= 2; b= 3;
		break;
	case 3:
		a= 3; b= 3;
		break;

	case 4:
		a= 0; b= 2;
		break;
	case 5:
		a= 1; b= 2;
		break;
	case 6:
		a= 2; b= 2;
		break;
	case 7:
		a= 3; b= 2;
		break;

	case 8:
		a= 0; b= 1;
		break;
	case 9:
		a= 1; b= 1;
		break;
	case 10:
		a= 2; b= 1;
		break;
	case 11:
		a= 3; b= 1;
		break;
		
	case 12:
		a= 0; b= 0;
		break;
	case 13:
		a= 1; b= 0;
		break;
	case 14:
		a= 2; b= 0;
		break;
	default:
		a=0; b=0;
	}
	
	xfac= schuifset[a][b][0]-1.5;
	yfac= schuifset[a][b][1]-1.5;
	
	base->o[0]= xfac*SCH_GRIDX;
	base->o[1]= yfac*SCH_GRIDY;

}


