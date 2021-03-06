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



/*  imageprocess.c        MIXED MODEL
 * 
 *  april 95
 * 
 */

#include <local/util.h>
#include <local/iff.h>
#include "blender.h"
#include "render.h"


					/* uit render.c */
extern ushort *igamtab1, *igamtab2, *gamtab;


void keyalpha(doel)   /* maakt premul 255 */
register char *doel;
{
	register long c;
	register short div;

	div= doel[0];

	c= (doel[3]<<8)/div;
	if(c>255) doel[3]=255; 
	else doel[3]= c;
	c= (doel[2]<<8)/div;
	if(c>255) doel[2]=255; 
	else doel[2]= c;
	c= (doel[1]<<8)/div;
	if(c>255) doel[1]=255; 
	else doel[1]= c;
}

void addalphaUnder(doel,bron)   /* vult bron onder doel in met alpha van doel*/
register char *doel,*bron;
{
	register int c;
	register int mul;

	if(doel[0]==255) return;
	if( doel[0]==0) {	/* is getest, scheelt  */
		*((ulong *)doel)= *((ulong *)bron);
		return;
	}

	mul= 255-doel[0];

	c= doel[3]+ ((mul*bron[3])/255);
	if(c>255) doel[3]=255; 
	else doel[3]= c;
	c= doel[2]+ ((mul*bron[2])/255);
	if(c>255) doel[2]=255; 
	else doel[2]= c;
	c= doel[1]+ ((mul*bron[1])/255);
	if(c>255) doel[1]=255; 
	else doel[1]= c;
	
	c= doel[0]+ ((mul*bron[0])/255);
	if(c>255) doel[0]=255; 
	else doel[0]= c;
	
	/* doel[0]= MAX2(doel[0], bron[0]); */
}

void addalphaUnderGamma(doel,bron)   /* gamma-gecorr: vult bron onder doel in met alpha van doel */
register char *doel,*bron;
{
	register ulong tot;
	register int c, doe, bro;
	register int mul;

	/* hier doel[0]==0 of doel==255 afvangen gebeurt al in skylus */
	mul= 256-doel[0];
	
	doe= igamtab1[doel[3]];
	bro= igamtab1[bron[3]];
	tot= (doe+ ((mul*bro)>>8));
	if(tot>65535) tot=65535;
	doel[3]= *((char *)(gamtab+tot));
	
	doe= igamtab1[doel[2]];
	bro= igamtab1[bron[2]];
	tot= (doe+ ((mul*bro)>>8));
	if(tot>65535) tot=65535;
	doel[2]= *((char *)(gamtab+tot));

	doe= igamtab1[doel[1]];
	bro= igamtab1[bron[1]];
	tot= (doe+ ((mul*bro)>>8));
	if(tot>65535) tot=65535;
	doel[1]= *((char *)(gamtab+tot));

	c= doel[0]+ ((mul*bron[0])/255);
	if(c>255) doel[0]=255; 
	else doel[0]= c;
	/* doel[0]= MAX2(doel[0], bron[0]); */
}


void addalphaOver(doel,bron)   /* doel= bron over doel  */
register char *doel,*bron;	/* controleert op overflow */
{
	register int c;
	register int mul;

	if(bron[0]==0) return;
	if( bron[0]==255) {	/* is getest, scheelt  */
		*((ulong *)doel)= *((ulong *)bron);
		return;
	}

	mul= 255-bron[0];

	c= ((mul*doel[3])/255)+bron[3];
	if(c>255) doel[3]=255; 
	else doel[3]= c;
	c= ((mul*doel[2])/255)+bron[2];
	if(c>255) doel[2]=255; 
	else doel[2]= c;
	c= ((mul*doel[1])/255)+bron[1];
	if(c>255) doel[1]=255; 
	else doel[1]= c;
	c= ((mul*doel[0])/255)+bron[0];
	if(c>255) doel[0]=255; 
	else doel[0]= c;
}

void addalphaAdd(doel,bron)   /* telt bron bij doel */
register char *doel,*bron;	/* controleert op overflow */
{
	register int c;
	register int mul;

	if( doel[0]==0 || bron[0]==255) {	/* is getest, scheelt veel */
		*((ulong *)doel)= *((ulong *)bron);
		return;
	}
	c= doel[3]+bron[3];
	if(c>255) doel[3]=255; 
	else doel[3]= c;
	c= doel[2]+bron[2];
	if(c>255) doel[2]=255; 
	else doel[2]= c;
	c= doel[1]+bron[1];
	if(c>255) doel[1]=255; 
	else doel[1]= c;
	c= doel[0]+bron[0];
	if(c>255) doel[0]=255; 
	else doel[0]= c;
}

void addalphaAddshort(doel,bron)   /* telt bron bij doel */
register ushort *doel,*bron;	/* controleert op overflow */
{
	register int c;
	register int mul;

	if( doel[0]==0 || bron[0]==65535) {	/* is getest, scheelt veel */
		*((ulong *)doel)= *((ulong *)bron);
		*((ulong *)(doel+2))= *((ulong *)(bron+2));
		return;
	}
	c= doel[3]+bron[3];
	if(c>65535) doel[3]=65535; 
	else doel[3]= c;
	c= doel[2]+bron[2];
	if(c>65535) doel[2]=65535; 
	else doel[2]= c;
	c= doel[1]+bron[1];
	if(c>65535) doel[1]=65535; 
	else doel[1]= c;
	c= doel[0]+bron[0];
	if(c>65535) doel[0]=65535; 
	else doel[0]= c;
}

/* ALPHADDFAC: 
 * 
 *  Z= X alphaover Y:
 *  Zrgb= (1-Xa)*Yrgb + Xrgb
 * 
 *	Om ook de add te doen moet (1-Xa) moduleren met 1 via fac
 *  (1-fac)*(1-Xa) + fac <=>
 *  1-Xa-fac+fac*Xa+fac <=> 
 *  Xa*(fac-1)+1
 */


void addalphaAddfac(doel, bron, addfac)    /* doel= bron over doel  */
register char *doel, *bron, addfac;
{
	
	register int c, mul;

	if( doel[0]==0) {
		*((ulong *)doel)= *((ulong *)bron);
		return;
	}

	mul= 255 - (bron[0]*(255-addfac))/255;

	c= ((mul*doel[3])/255)+bron[3];
	if(c>255) doel[3]=255; 
	else doel[3]= c;
	c= ((mul*doel[2])/255)+bron[2];
	if(c>255) doel[2]=255; 
	else doel[2]= c;
	c= ((mul*doel[1])/255)+bron[1];
	if(c>255) doel[1]=255; 
	else doel[1]= c;
	
	/* c= ((mul*doel[0])/255)+bron[0]; */
	c= doel[0]+bron[0];
	if(c>255) doel[0]=255; 
	else doel[0]= c;
}

void addalphaAddfacshort(doel, bron, addfac)    /* doel= bron over doel  */
register ushort *doel, *bron, addfac;
{
	
	register int c, mul;

	if( doel[0]==0) {
		*((ulong *)doel)= *((ulong *)bron);
		*((ulong *)(doel+2))= *((ulong *)(bron+2));
		return;
	}

	mul= 0xFFFF - (bron[0]*(255-addfac))/255;
	
	c= ((mul*doel[3])>>16)+bron[3];
	if(c>=0xFFF0) doel[3]=0xFFF0; 
	else doel[3]= c;
	c= ((mul*doel[2])>>16)+bron[2];
	if(c>=0xFFF0) doel[2]=0xFFF0; 
	else doel[2]= c;
	c= ((mul*doel[1])>>16)+bron[1];
	if(c>=0xFFF0) doel[1]=0xFFF0; 
	else doel[1]= c;
	c= ((mul*doel[0])>>16)+bron[0];
	if(c>=0xFFF0) doel[0]=0xFFF0; 
	else doel[0]= c;

}




void clustered_dots(ibuf, size)
struct ImBuf *ibuf;
long size;
{
	extern float rgb_to_bw[];
	float fx, fy, fsize, halfsize;
	int a, b, x, y, tot;
	ulong *rect;
	char *cp, *rt;
	
	cspace(ibuf,rgb_to_bw);

	halfsize= 0.5*size;
	fsize= 256/(halfsize*halfsize);
	
	cp= (char *)ibuf->rect;
	for(y=0; y<ibuf->y; y++) {
		for(x=0; x<ibuf->x; x++, cp+=4) {
			
			fx= ((x+y) % size) -halfsize;
			fy= ((1000+y-x) % size) -halfsize;
			
			tot= cp[1]+ fsize*( (fx*fx+ fy*fy) ) -128;
			
			if(tot<0) tot= 0;
			else if(tot>255) tot= 255;
			cp[1]= cp[2]= cp[3]= tot;
			
		}
	}

}

void scalefastrect(recto, rectn, oldx, oldy, newx, newy)
ulong *recto, *rectn;
short oldx, oldy, newx,newy;
{
	ulong *rect, *newrect;
	int x, y;
	int ofsx, ofsy, stepx, stepy;

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

#define CMBB MAKE_ID('C','M','B','B')


void alpha_dither(ImBuf *sbuf, char mode)
{
	int *dp, dit[4], x, y, a;
	char *rt;
	
	if(mode=='r') {
		x= sbuf->x*sbuf->y;
		rt= (char *)sbuf->rect;
		while(x--) {
			if(rt[0]==0 || rt[0]==255);
			else {
				a= R.rt + rt[0] + (rand()>>8);
				if(a>128) rt[0]= 255;
				else *((ulong *)rt)= 0;
			}
			rt+=4;
		}
	}
	else if(mode=='2') {
		dit[0]=0x80;
		dit[1]=0x00; 
		dit[2]=0xC0; 
		dit[3]=0x40;

		rt= (char *)sbuf->rect;
		
		for(y=0; y<sbuf->y; y++) {
			
			if(y & 1) dp= dit;
			else dp= dit+2;
			
			for(x=sbuf->x; x>0; x--, rt+=4) {
				if(rt[0]==0 || rt[0]==255);
				else {
					a= rt[0] + dp[ x & 1 ];
					if(a>128) rt[0]= rt[0]= 255;
					else *((ulong *)rt)= 0;
				}
			}
		}
	}
}

void no_isolated_pixels(ImBuf *sbuf, int max)
{
	ulong *rt, *rt1, iso=0;
	int x, y;
	char *cp;
	
	if(sbuf->x<5 || sbuf->y<5) return;

	rt= sbuf->rect+sbuf->x;
	for(y=2; y<sbuf->y; y++) {
		rt++;
		for(x=2; x<sbuf->x; x++, rt++) {
			if( *rt && rt[-1]==0 && rt[1]==0) {
				rt1= rt - sbuf->x;
				if( rt1[-1]==0 && rt1[0]==0 && rt1[1]==0) {
					rt1= rt+sbuf->x;
					if( rt1[-1]==0 && rt1[0]==0 && rt1[1]==0) {
						cp= (char *)rt;
						if( cp[1]<max && cp[2]<max && cp[3]<max) {
							iso++;
							rt[0]= 0;
						}
					}
				}
				
			}
		}
	}
}

void write_cmbb(ImBuf *sbuf, char *str)
{
	ImBuf * vbuf;
	ulong *lp;
	int file, x, y;
	short isdone=0, minx, miny, maxx, maxy, size;
	char *rt;
	

	sbuf->cbits = 6;	/* diepte van tabel */

	alpha_dither(sbuf, 'r');

	no_isolated_pixels(sbuf, 128);

	alpha_to_col0(2);	/* 1: alleen alpha 0,  2: alpha<128 */	
	setdither(0);
	converttocmap(sbuf);

	/* long to char */
	x= sbuf->x*sbuf->y;
	lp= (ulong *)sbuf->rect;
	rt= (char *)lp;
	while(x--) {
		*rt= (char)*lp;
		rt++; lp++;
	}

	/* boundbox berekenen */
	minx= sbuf->x-1;
	miny= sbuf->y-1;
	maxx= maxy= 0;
	
	rt= (char *)sbuf->rect;
	for(y=0; y<sbuf->y; y++) {
		for(x=0; x<sbuf->x; x++, rt++) {
			if( *rt ) {
				if(minx>x) minx= x;
				if(maxx<x) maxx= x;
				if(miny>y) miny= y;
				if(maxy<y) maxy= y;
				isdone= 1;
			}
		}
	}
	
	if(isdone==0) {
		minx= maxx= sbuf->x/2;
		miny= maxy= sbuf->y/2;
	}
	
	if(minx & 1) minx--;
	if(miny & 1) miny--;
	if(maxx & 1); else maxx++;
	if(maxy & 1); else maxy++;
	
	/* schrijven */
	file= open(str, O_WRONLY+O_CREAT+O_TRUNC, 0666);
	if(file== -1) {
		error("Can't write file");
		return;
	}

	x= CMBB;
	write(file, &x, 4);
	
	size= maxx-minx+1;
	write(file, &size, 2);
	size= maxy-miny+1;
	write(file, &size, 2);
	
	size= (minx+maxx)/2;
	size -= (sbuf->x/2);
	write(file, &size, 2);
	size= sbuf->y -1- ((miny+maxy)/2);
	size -= (sbuf->y/2);
	write(file, &size, 2);

	/* y-geflipt wegschrijven */
	rt= ( (char *)sbuf->rect) + sbuf->x*maxy + minx;
	size= maxx-minx+1;
	for(y=miny; y<=maxy; y++) {
		write(file, rt, size);
		
		rt-= sbuf->x;
	}
	
	close(file);
}


void write_fourparts(ImBuf *totbuf, char *str)
{
	extern rectcpy();
	ImBuf *ibuf;
	int sx;
	char name[200], di[120], fi[80];
	int i, len;
	
	strcpy(di, str);
	splitdirstring(di, fi);
	
	ibuf = allocImBuf(totbuf->x/4, totbuf->y, R.r.planes, IB_rect, 0);
	ibuf->ftype= totbuf->ftype;
	sx= totbuf->x/8;

	/* front */
	rectop(ibuf, totbuf, 0, 0, 3*sx, 0, 2*sx, totbuf->y, rectcpy, 0);
	i= saveiff(ibuf, str, IB_rect);	
	if(i==0) perror(str);
	
	/* left */
	sprintf(name, "%sL%s", di, fi);
	rectop(ibuf, totbuf, 0, 0, sx, 0, 2*sx, totbuf->y, rectcpy, 0);
	i= saveiff(ibuf, name, IB_rect);
	if(i==0) perror(str);
	
	/* right */
	sprintf(name, "%sR%s", di, fi);
	rectop(ibuf, totbuf, 0, 0, 5*sx, 0, 2*sx, totbuf->y, rectcpy, 0);
	i= saveiff(ibuf, name, IB_rect);
	if(i==0) perror(str);
	
	/* back */
	sprintf(name, "%sB%s", di, fi);
	rectop(ibuf, totbuf, 0, 0, 7*sx, 0, sx, totbuf->y, rectcpy, 0);
	rectop(ibuf, totbuf, sx, 0, 0, 0, sx, totbuf->y, rectcpy, 0);
	i= saveiff(ibuf, name, IB_rect);
	if(i==0) perror(str);
	
	freeImBuf(ibuf);
}




void convert_rgba_to_abgr(int size, ulong *rect)
{
	char *cp= (char *)rect, rt;
	
	while(size-- > 0) {
		rt= cp[0];
		cp[0]= cp[3];
		cp[3]= rt;
		rt= cp[1];
		cp[1]= cp[2];
		cp[2]= rt;
		cp+= 4;
	}
}


