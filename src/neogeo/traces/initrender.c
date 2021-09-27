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

/* 	render.c	 */


/*
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

		jitterate1(jit1,jit2,num)
		jitterate2(jit1,jit2,num)
		initjit(jit1,num)

struct VertRen	*addvert(nr)
struct VlakRen	*addface(nr)

		initobject(base,col,colt)
		initlamp(base)
		initsphere(base,mat,imat,dim)
		polyzijvlakken()
		initpolyren(ob,cmat,bmat)

		makepicstring(string,frame)
		setwindowclip(mode)
		openwindow(ysize);

short		render(fra)
		initrender(anim)
*/

#include "/usr/people/include/Trace.h"
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <string.h>
#include "iff.h"
#include <malloc.h>
#include "storage.h"
#include <sys/times.h>
#include <fcntl.h>
#include <sys/param.h>


#include <sys/time.h>		/* voor de interruptESC */
#include <signal.h>
#include <stdlib.h>		/* getenv("HOST") */

					/* uit zbuf.c */
extern void projectverto();
extern void projectvert();
					/* uit view.c */
extern float mat1[4][4];
extern short panoramaview;
					/* uit render.c */
extern float fmask[256], centLut[16];
extern ushort *mask1[9], *mask2[9], *igamtab1, *igamtab2, *gamtab;
extern char cmask[256], *centmask;

struct ColBlckF *colbfdata= 0;
short pa; /* pa is globaal part ivm print */
short allparts[17][4];
float jit[64][2], jitweight[64], Normalise(), CalcNormFloat();
long qscount, totcolbf;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


void makestringcode(str)
char *str;
{
	char *slash,len,temp[100];

	strcpy(temp,G.sce);
	slash = strrchr(temp,'/');
	if(slash) {
		*(slash+1)= 0;
		len= strlen(temp);
		if(len) {
			if(strncmp(str,temp,(long)len-1)==0) {
				temp[0]='/';
				temp[1]='/';
				strcpy(temp+2,str+len);
				strcpy(str,temp);
			}
		}
	}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void makepicstring(string,frame)
char *string;
short frame;
{
	short i,len;
	char num[10];

	if (string==0) return;

	strcpy(string,G.pic);
	convertstringcode(string);
	len=strlen(string);

	i=4-sprintf(num,"%d",frame);
	for(;i>0;i--){
		string[len]='0';
		len++;
	}
	string[len]=0;
	strcat(string,num);
}

void makepicstring2(string,frame)   /* zonder G.pic */
char *string;
short frame;
{
	short i,len;
	char num[10];

	if (string==0) return;

	convertstringcode(string);
	len= strlen(string);

	i=4-sprintf(num,"%d",frame);
	for(;i>0;i--){
		string[len]='0';
		len++;
	}
	string[len]=0;
	strcat(string,num);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void jitterate1(jit1,jit2,num,rad1)
float *jit1, *jit2,rad1;
short num;
{
	long i , j , k;
	float vecx,vecy,dvecx,dvecy,x,y,len;

	for (i = 2*num-2; i>=0 ; i-=2) {
		dvecx = dvecy = 0.0;
		x = jit1[i];
		y = jit1[i+1];
		for (j = 2*num-2; j>=0 ; j-=2) {
			if (i != j){
				vecx = jit1[j] - x - 1.0;
				vecy = jit1[j+1] - y - 1.0;
				for (k = 3; k>0 ; k--){
					if( fabs(vecx)<rad1 && fabs(vecy)<rad1) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad1) {
							len= len/rad1;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx += 1.0;

					if( fabs(vecx)<rad1 && fabs(vecy)<rad1) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad1) {
							len= len/rad1;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx += 1.0;

					if( fabs(vecx)<rad1 && fabs(vecy)<rad1) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad1) {
							len= len/rad1;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx -= 2.0;
					vecy += 1.0;
				}
			}
		}

		x -= dvecx/18.0 ; 
		y -= dvecy/18.0;
		x -= floor(x) ; 
		y -= floor(y);
		jit2[i] = x;
		jit2[i+1] = y;
	}
	memcpy(jit1,jit2,2 * num * sizeof(float));
}

void jitterate2(jit1,jit2,num,rad2)
float *jit1, *jit2,rad2;
short num;
{
	long i , j , k;
	float vecx,vecy,dvecx,dvecy,x,y,len;

	for (i=2*num -2; i>= 0 ; i-=2){
		dvecx = dvecy = 0.0;
		x = jit1[i];
		y = jit1[i+1];
		for (j =2*num -2; j>= 0 ; j-=2){
			if (i != j){
				vecx = jit1[j] - x - 1.0;
				vecy = jit1[j+1] - y - 1.0;

				if( fabs(vecx)<rad2) dvecx+= vecx*rad2;
				vecx += 1.0;
				if( fabs(vecx)<rad2) dvecx+= vecx*rad2;
				vecx += 1.0;
				if( fabs(vecx)<rad2) dvecx+= vecx*rad2;

				if( fabs(vecy)<rad2) dvecy+= vecy*rad2;
				vecy += 1.0;
				if( fabs(vecy)<rad2) dvecy+= vecy*rad2;
				vecy += 1.0;
				if( fabs(vecy)<rad2) dvecy+= vecy*rad2;

			}
		}

		x -= dvecx/2 ; 
		y -= dvecy/2;
		x -= floor(x) ; 
		y -= floor(y);
		jit2[i] = x;
		jit2[i+1] = y;
	}
	memcpy(jit1,jit2,2 * num * sizeof(float));
}

void initjit(jit,num)
float *jit;
short num;
{
	float *jit2,x,y, rad1,rad2, rad3;
	long col;
	short i;

	if(num==0) return;

	jit2= (float *)mallocN(2*4*num,"initjit");
	rad1=  1.0/fsqrt((float)num);
	rad2= 1.0/((float)num);
	rad3= fsqrt((float)num)/((float)num);

	srand48(31415926 + num);
	x= 0;
	for(i=0;i<2*num;i+=2) {
		jit[i]= x+ rad1*(0.5-drand48());
		jit[i+1]= ((float)i/2)/num +rad1*(0.5-drand48());
		x+= rad3;
		x -= floor(x);
	}

	for (i=0 ; i<24 ; i++) {
		jitterate1(jit,jit2,num,rad1);
		jitterate1(jit,jit2,num,rad1);
		jitterate2(jit,jit2,num,rad2);
	}

	freeN(jit2);
}

/* ****************** GAMMA, MASKERS en LUTS **************** */

float  calc_weight(weight, i, j)
float *weight;
long i, j;
{
	float x, y, dist, totw= 0.0;
	long a;

	for(a=0; a<G.osa; a++) {
		x= jit[a][0]-0.5+ i;
		y= jit[a][1]-0.5+ j;
		dist= fsqrt(x*x+y*y);

		weight[a]= 0.0;

		if(G.filt==1) {	/* GAUSS */
			if(dist<1.5) {
				x = dist*1.5;
				weight[a]= (1.0/exp(x*x) - 1.0/exp(1.5*1.5*1.5*1.5));
			}
		}
		else if(G.filt==2) {  /* CONE */
			if(dist<1.5) weight[a]= 1.5-dist;
		}
		else if(G.filt==3) {	/* QUAD*/
			if(dist<1.5) {
				x= dist/1.5;
				weight[a]= 1.0- 3*x*x+ 2*x*x*x;
			}
		}
		else {
			if(i==0 && j==0) weight[a]= 1.0;
		}
		totw+= weight[a];

	}
	return totw;
}

void tekenjit(tot)
float tot;
{
	float weight[32], temp[3];
	ulong col;
	long a, i, j;


	prefsize(300,300);
	winopen(0);
	singlebuffer();
	RGBmode();
	gconfig();

	cpack(0);
	clear();

	cpack(0xFFFFFF);
	temp[2]= 0.0;
	temp[0]= 100; 
	temp[1]= 100;
	bgnclosedline();
	v3f(temp);
	temp[0]= 200;
	v3f(temp);
	temp[1]= 200;
	v3f(temp);
	temp[0]= 100;
	v3f(temp);
	endclosedline();


	for(j=0; j<3; j++) {
		for(i=0; i<3; i++) {
			calc_weight(weight, i-1, j-1);

			for(a=0; a<G.osa;a++) {
				temp[0]= 100+ 100*(i-1+jit[a][0]);
				temp[1]= 100+ 100*(j-1+jit[a][1]);
				col= 12*255*(weight[a]/tot);
				if(col>255) col= 0xFF00FF;
				else col*= 0x010101;
				cpack(col);
				sboxf(temp[0], temp[1], temp[0]+2, temp[1]+2);
			}

		}
	}


}

void init_filt_mask()
{
	static long firsttime=1;
	static float lastgamma= 0.0;
	float gamma;
	float weight[32], totw, val, *fpx1, *fpx2, *fpy1, *fpy2;
	long i, j, a;
	ushort *m1, *m2, shweight[32];

	if(firsttime) {
		for(a=0; a<9;a++) {
			mask1[a]= mallocN(256*2, "initfilt");
			mask2[a]= mallocN(256*2, "initfilt");
		}
		for(a=0; a<256; a++) {
			cmask[a]= 0;
			if(a &   1) cmask[a]++;
			if(a &   2) cmask[a]++;
			if(a &   4) cmask[a]++;
			if(a &   8) cmask[a]++;
			if(a &  16) cmask[a]++;
			if(a &  32) cmask[a]++;
			if(a &  64) cmask[a]++;
			if(a & 128) cmask[a]++;
		}
		centmask= mallocN(65536, "Initfilt3");
		for(a=0; a<16; a++) {
			centLut[a]= -0.45+((float)a)/16.0;
		}

		gamtab= mallocN(65536*2, "initGaus2");
		igamtab1= mallocN(256*2, "initGaus2");
		igamtab2= mallocN(65536*2, "initGaus2");

	}
	
	gamma= 2.0;
	if(G.genlock) gamma= 1.0;
	
	if(gamma!= lastgamma) {
		lastgamma= gamma;

		/* gamtab: in short, uit short */
		for(a=0; a<65536; a++) {
			val= a;
			val/= 65535.0;
			
			if(gamma==2.0) val= fsqrt(val);
			
			gamtab[a]= (65535.99*val);
		}
		/* inv gamtab1 : in byte, uit short */
		for(a=1; a<=256; a++) {
			if(gamma==2.0) igamtab1[a-1]= a*a-1;
			else igamtab1[a-1]= 256*a-1;
		}
		
		/* inv gamtab2 : in short, uit short */
		for(a=0; a<65536; a++) {
			val= a;
			val/= 65535.0;
			if(gamma==2.0) val= val*val;
			igamtab2[a]= 65535.0*val;
		}
	}
	
	if(firsttime) {
		firsttime= 0;
		return;
	}

	val= 1.0/((float)G.osa);
	for(a=0; a<256; a++) {
		fmask[a]= ((float)cmask[a])*val;
	}

	for(a=0; a<9;a++) {
		blkclr(mask1[a], 256*2);
		blkclr(mask2[a], 256*2);
	}

	/* bereken totw */
	totw= 0.0;
	for(j= -1; j<2; j++) {
		for(i= -1; i<2; i++) {
			totw+= calc_weight(weight, i, j);
		}
	}

	for(j= -1; j<2; j++) {
		for(i= -1; i<2; i++) {
			/* bereken ahv jit met ofset de gewichten */

			blkclr(weight, 32*2);
			calc_weight(weight, i, j);

			for(a=0; a<16; a++) shweight[a]= weight[a]*(65535.0/totw);

			m1= mask1[ 3*(j+1)+i+1 ];
			m2= mask2[ 3*(j+1)+i+1 ];

			for(a=0; a<256; a++) {
				if(a &   1) { 
					m1[a]+= shweight[0]; 
					m2[a]+= shweight[8]; 
				}
				if(a &   2) { 
					m1[a]+= shweight[1]; 
					m2[a]+= shweight[9]; 
				}
				if(a &   4) { 
					m1[a]+= shweight[2]; 
					m2[a]+= shweight[10]; 
				}
				if(a &   8) { 
					m1[a]+= shweight[3]; 
					m2[a]+= shweight[11]; 
				}
				if(a &  16) { 
					m1[a]+= shweight[4]; 
					m2[a]+= shweight[12]; 
				}
				if(a &  32) { 
					m1[a]+= shweight[5]; 
					m2[a]+= shweight[13]; 
				}
				if(a &  64) { 
					m1[a]+= shweight[6]; 
					m2[a]+= shweight[14]; 
				}
				if(a & 128) { 
					m1[a]+= shweight[7]; 
					m2[a]+= shweight[15]; 
				}
			}
		}
	}

	/* centmask: de juiste subpixel ofset per masker */

	fpx1= mallocN(256*4, "initgauss4");
	fpx2= mallocN(256*4, "initgauss4");
	fpy1= mallocN(256*4, "initgauss4");
	fpy2= mallocN(256*4, "initgauss4");
	for(a=0; a<256; a++) {
		fpx1[a]= fpx2[a]= 0.0;
		fpy1[a]= fpy2[a]= 0.0;
		if(a & 1) {
			fpx1[a]+= jit[0][0]; 
			fpy1[a]+= jit[0][1];
			fpx2[a]+= jit[8][0]; 
			fpy2[a]+= jit[8][1];
		}
		if(a & 2) {
			fpx1[a]+= jit[1][0]; 
			fpy1[a]+= jit[1][1];
			fpx2[a]+= jit[9][0]; 
			fpy2[a]+= jit[9][1];
		}
		if(a & 4) {
			fpx1[a]+= jit[2][0]; 
			fpy1[a]+= jit[2][1];
			fpx2[a]+= jit[10][0]; 
			fpy2[a]+= jit[10][1];
		}
		if(a & 8) {
			fpx1[a]+= jit[3][0]; 
			fpy1[a]+= jit[3][1];
			fpx2[a]+= jit[11][0]; 
			fpy2[a]+= jit[11][1];
		}
		if(a & 16) {
			fpx1[a]+= jit[4][0]; 
			fpy1[a]+= jit[4][1];
			fpx2[a]+= jit[12][0]; 
			fpy2[a]+= jit[12][1];
		}
		if(a & 32) {
			fpx1[a]+= jit[5][0]; 
			fpy1[a]+= jit[5][1];
			fpx2[a]+= jit[13][0]; 
			fpy2[a]+= jit[13][1];
		}
		if(a & 64) {
			fpx1[a]+= jit[6][0]; 
			fpy1[a]+= jit[6][1];
			fpx2[a]+= jit[14][0]; 
			fpy2[a]+= jit[14][1];
		}
		if(a & 128) {
			fpx1[a]+= jit[7][0]; 
			fpy1[a]+= jit[7][1];
			fpx2[a]+= jit[15][0]; 
			fpy2[a]+= jit[15][1];
		}
	}

	for(a= (1<<G.osa)-1; a>0; a--) {
		val= count_mask(a);
		i= (15.9*(fpy1[a & 255]+fpy2[a>>8])/val);
		i<<=4;
		i+= (15.9*(fpx1[a & 255]+fpx2[a>>8])/val);
		centmask[a]= i;
	}

	freeN(fpx1); 
	freeN(fpx2);
	freeN(fpy1); 
	freeN(fpy2);

	/* tekenjit(totw); */
}

void free_filt_mask()
{
	long a;

	for(a=0; a<9; a++) {
		freeN(mask1[a]);
		freeN(mask2[a]);
	}
	freeN(gamtab);
	freeN(igamtab1);
	freeN(igamtab2);

	freeN(centmask);
}


/* ~~~~~~~~~~~~~TIMER~~~~~~~~~~~~~~~~~~~~~~~~~ */

void bgntimer()
{
	struct itimerval tmevalue;		/* interruptESC */
	extern void interruptESC();

	tmevalue.it_interval.tv_sec = 0;
	tmevalue.it_interval.tv_usec = 250000;
	/* wanneer de eerste ? */
	tmevalue.it_value.tv_sec = 0;
	tmevalue.it_value.tv_usec = 10000;

	signal(SIGVTALRM,interruptESC);
	setitimer(ITIMER_VIRTUAL, &tmevalue, 0);

}


void suspendtimer()
{
	if (G.background & 1) return;

	signal(SIGVTALRM,SIG_IGN);
}


void restarttimer()
{
	extern void interruptESC();

	if (G.background & 1) return;

	signal(SIGVTALRM,interruptESC);
}


void endtimer()
{
	struct itimerval tmevalue;

	tmevalue.it_value.tv_sec = 0;
	tmevalue.it_value.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &tmevalue, 0);
	signal(SIGVTALRM,SIG_IGN);

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


void initrendertexture()
{
	struct Tex *tex;
	struct MipMap *mm, *mmn;
	long a, c, imanr;
	ushort numlen;
	char name[100], head[100], tail[100];

	for(a=1; a<=G.totex; a++) {
		tex= G.adrtex[a];
		for(c=0; c<3; c++) {
			tex->c1f[c]= ((float)tex->c1[c])/255.0;
			tex->c2f[c]= ((float)tex->c2[c])/255.0;
			tex->c3f[c]= ((float)tex->c3[c])/255.0;
			tex->lif[c]= ((float)tex->li[c])/255.0;
			tex->divf[c]= ((float)tex->div[c])/255.0;
		}

		tex->cintf= ((float)tex->cint)/255.0;
		tex->bintf= ((float)tex->bint)/255.0;
		tex->vintf= ((float)tex->vint)/255.0;
		
		if(tex->soort==5) {
			if(tex->type==3) {
				if(tex->ima && tex->ima->name) {
					if(tex->ima->ibuf) {	/* testen of nieuw frame */
						
						/*                cfra     animlen      sfra          fie/ima      cycl    */
						imanr= calcimanr(G.cfra, tex->var[9], tex->var[11], tex->var[7], tex->var[10]);
						
						if(imanr!=tex->ima->lastframe || G.ali>tex->ima->lastquality) {
							freeImBuf(tex->ima->ibuf);
							tex->ima->ibuf= 0;

							mm= tex->ima->mipmap;
							while(mm) {
								mmn= mm->next;
								freeImBuf(mm->ibuf);
								freeN(mm);
								mm= mmn;
							}
							tex->ima->mipmap= 0;
						}
					}
					tex->ima->ok= 1;
				}
			}
			else if(tex->var[9] && tex->ima && tex->ima->name) {	/* frames */
				strcpy(name, tex->ima->name);
				
				if(tex->var[6] && tex->type==2) {	/* mipmap && fields */
					tex->var[6]= 0;
				}
				
				/* imanr= G.cfra-tex->var[11]+1; */	/* 11= sfra */
				
				imanr= calcimanr(G.cfra, tex->var[9], tex->var[11], tex->var[7], tex->var[10]);
				
				if(tex->var[10]) {	/* cycl */
					while(imanr<=0) imanr+= tex->var[9];
					while(imanr>tex->var[9]) imanr-= tex->var[9];
				}
				else {
					if(imanr<1) imanr= 1;
					if(imanr>tex->var[9]) imanr= tex->var[9];
				}
				/* imanr+= tex->var[7];	was offset, nu F/Im */
				
				tex->ima->lastframe= imanr;	/* voor patch field-ima rendering */
				
				stringdec(name, head, tail, &numlen);
				stringenc(name, head, tail, numlen, imanr);
				addImap(tex, name);
			}
		}
	}
}

struct VertRen *addvert(nr)
long nr;
{
	struct VertRen *v;
	short a;

	if(nr<0 || nr>MAXVERT ) {
		printf("error in addvert: %d\n",nr);
		return R.blove[0];
	}
	a= nr>>8;
	v= R.blove[a];
	if(v==0) {
		v= (struct VertRen *)callocN(256*sizeof(struct VertRen),"addvert");
		R.blove[a]= v;
	}
	v+= (nr & 255);
	return v;
}

struct HaloRen *addhalo(nr)
long nr;
{
	struct HaloRen *h;
	short a;

	if(nr<0 || nr>MAXVERT ) {
		printf("error in addhalo: %d\n",nr);
		return R.bloha[0];
	}
	a= nr>>8;
	h= R.bloha[a];
	if(h==0) {
		h= (struct HaloRen *)callocN(256*sizeof(struct HaloRen),"addhalo");
		R.bloha[a]= h;
	}
	h+= (nr & 255);
	return h;
}

struct VlakRen *addvlak(nr)
long nr;
{
	struct VlakRen *v;
	short a;

	if(nr<0 || nr>MAXVLAK ) {
		printf("error in addvlak: %d\n",nr);
		return R.blovl[0];
	}
	a= nr>>8;
	v= R.blovl[a];
	if(v==0) {
		v= (struct VlakRen *)callocN(256*sizeof(struct VlakRen),"addvlak");
		R.blovl[a]= v;
	}
	v+= (nr & 255);
	return v;
}

struct HaloRen *inithalo(vec, hasize)
float *vec, hasize;
{
	struct HaloRen *har;
	short testclip();
	float zn, hoco[4];

	vec[2]-= hasize;
	projectverto(vec, hoco);
	hoco[3]*= 4.0;	/* testclip op 4x de afstand */
	if( testclip(hoco)==0 ) {
		
		vec[2]+= hasize;
		projectverto(vec, hoco);
		har= addhalo(G.tothalo++);
		
		/* projectvert wordt in zbufvlaggen overgedaan ivm parts */
		VECCOPY(har->co, vec);
		zn= hoco[3];
		har->xs= 0.5*R.rectx*(hoco[0]/zn);
		har->ys= 0.5*R.recty*(hoco[1]/zn);
		har->zs= 0x7FFFFF*(1.0+hoco[2]/zn);
		vec[0]+= hasize;
		projectverto(vec, hoco);
		zn= hoco[3];
		har->rad= fabs(har->xs- 0.5*R.rectx*(hoco[0]/zn));
		har->radsq= har->rad*har->rad;
		har->miny= har->ys-har->rad;
		har->maxy= har->ys+har->rad;
		
		vec[0]-= hasize;
		vec[2]-= hasize;	/* z is negatief, wordt anders geclipt */
		projectverto(vec, hoco);
		zn= hoco[3];
		har->zd= abs(har->zs - 0x7FFFFF*(1.0+hoco[2]/zn) );
		
		return har;
	}
	return 0;
}

struct HaloRen *initstar(vec, hasize)
float *vec, hasize;
{
	struct HaloRen *har;
	short testclip();
	float zn, hoco[4];

	projectverto(vec, hoco);
	if( testclip(hoco)==0 ) {
		har= addhalo(G.tothalo++);
		
		/* projectvert wordt in zbufvlaggen overgedaan ivm parts */
		VECCOPY(har->co, vec);
		zn= hoco[3];
		har->xs= 0.5*R.rectx*(hoco[0]/zn);
		har->ys= 0.5*R.recty*(hoco[1]/zn);
		har->zs= 0x7FFFFF*(1.0+hoco[2]/zn);

		har->rad= hasize;
		har->radsq= har->rad*har->rad;
		har->miny= har->ys-har->rad;
		har->maxy= har->ys+har->rad;
		
		har->zd= 0.0;
		
		return har;
	}
	return 0;
}


void convertColBlck(new, old)
struct ColBlckF *new;
struct ColBlck *old;
{
	long a;

	new->r= ((float)old->r)/255.0;
	new->g= ((float)old->g)/255.0;
	new->b= ((float)old->b)/255.0;
	new->a= ((float)old->tra)/255.0;
	new->mode= old->mode+ (old->mode1 & 0xFF00);
	new->kref= ((float)old->kref)/255.0;
	new->kspec= ((float)old->kspec)/255.0;
	new->spec= old->spec;
	new->tra= ((float)old->tra)/255.0;
	new->mir= ((float)old->mir)/255.0;
	new->ang= ((float)old->ang)/255.0;
	new->amb= ((float)old->amb)/255.0;
	new->emit= ((float)old->emit)/255.0;
	new->specr= ((float)old->specr)/255.0;
	new->specg= ((float)old->specg)/255.0;
	new->specb= ((float)old->specb)/255.0;
	new->mirr= ((float)old->mirr)/255.0;
	new->mirg= ((float)old->mirg)/255.0;
	new->mirb= ((float)old->mirb)/255.0;
	new->ambr= ((float)old->ambr)/255.0;
	new->ambg= ((float)old->ambg)/255.0;
	new->ambb= ((float)old->ambb)/255.0;
	new->speclim= 0.0;
	new->texco= old->texco;
	new->lay= old->lay;

	for(a=0; a<4; a++) {
		new->tex[a]= old->tex[a];
		new->map[a]= old->map[a];
		new->map[a+4]= old->map[a+4];
		new->base[a]= old->base[a];
	}
}

void make_orco(base, type)		/* voor Curve & Nurb objecten met keys of UV tex */
struct Base *base;				/* type 1: orco's, 2: UV	*/
long type;
{
	struct CurveData *cu;
	struct Nurb *nu;
	struct DispList *dl;
	float *vec, *fp;
	long tot=0;
	short *data;
	
	if(base->soort==5) {
		cu= ( (struct ObData *)base->d )->cu;
		if(type==1) {
			key_to_curve(cu->key, cu, 0);	/* eerste keypositie */
			
			/* eerst voorspellen hoelang datablok moet worden */
			nu= cu->curve.first;
			while(nu) {
				if(nu->pntsv>1) tot+= nu->resolu*nu->resolv;
				nu= nu->next;
			}
			
			data=cu->orig= mallocN(3*2*tot, "make_orco");
			
			nu= cu->curve.first;
			while(nu) {
				if(nu->pntsv>1) {
					tot= nu->resolu*nu->resolv;
					fp=vec= callocN(3*4*tot, "make_orco1");
					makeNurbfaces(nu, vec);
					while(tot--) {
						VECCOPY(data, fp);
						data+= 3;
						fp+= 3;
					}
					freeN(vec);
				}
				nu= nu->next;
			}
			loadkeypostype(22, base, base);
		}
	}
	
}

struct ColBlckF *initobject(base,col,colt)
struct Base *base;
struct ColBlck *col;
short colt;
{
	struct CurveData *cu;
	struct ColBlckF *colbf;
	double si,spec;
	float fac;
	long a,c,texco, map, make_orcoflag=0, ok;

	for(a=0;a<colt;a++,col++) {

		colbf= colbfdata+totcolbf;
		totcolbf++;
		convertColBlck(colbf, col);
		
		si= col->kspec;
		spec= col->spec;
		if(si!=0.0 && spec!=0.0) colbf->speclim= pow(1/si,1/spec);
		else colbf->speclim= 1.0;

		map= 0;

		/* de normale texco's */
		for(c=0;c<4;c++) {
			if(base->soort==9) if(col->map[2*c] & 2048) {	/* geen orco's toegestaan */
				colbf->map[2*c]-= 2048;
				colbf->map[2*c]|= 16384;
				colbf->base[c]= base;
			}
			if ELEM(base->soort, 5, 11) {  /* geen orco's toegestaan? */
				if(col->map[2*c] & 8192) {	/* UV */
					cu= ( (struct ObData *)base->d )->cu;
					make_orcoflag= 2;
				}
				else if(col->map[2*c] & 2048) {	/* orco */
					cu= ( (struct ObData *)base->d )->cu;
					if(cu->key ) {	/* tenzij keyframing */
						make_orcoflag= 1;
					}
					else {
						colbf->map[2*c]-= 2048;
						colbf->map[2*c]|= 16384;
						colbf->base[c]= base;
					}
				}
				if(base->soort==11 && make_orcoflag) {	/* tijdelijk */
					make_orcoflag= 0;
					colbf->map[2*c]-= 2048;
					colbf->map[2*c]|= 16384;
					colbf->base[c]= base;
				}

			}
			if(col->tex[c]) {
				map|= col->map[2*c];
			}
			if(colbf->map[2*c] & 16384) {
				if(colbf->base[c]==0) colbf->base[c]= base;
			}
		}
		
		texco= (map>>9);
		if( (col->mode & 1) || (texco & 22) ) texco+=64;

		/* moet er osatex? */
		if(G.ali) {
			for(c=0;c<4;c++) {
				if(col->tex[c])
					if(G.adrtex[col->tex[c]]->soort==5) texco|=128;
			}
			if(col->mir) if(R.f & 4) texco|=129;
		}
		
		colbf->texco= texco;

		/* rare uitzondering: skyshadow */
		if(colbf->mode & 256) colbf->texco= 0;

		/* afmeting te kopieeren stuk van ColBlckF bij scanline coherentie */
		colbf->copysize= 0;
		if(texco & 63) {
			if(map & 256) colbf->copysize= 16;    /* a r g b */
			if(map & 127) colbf->copysize= 64;
		}
		if( (R.f & 4) && (colbf->mir>0.0) ) colbf->copysize= sizeof(struct ColBlckF);
		if(colbf->copysize<40) {
			if(col->mode & 128)		/* ztra shade en spec tra */
				if(col->ang) colbf->copysize= 40;
			if(colbf->mode & 256)	/* skyshadow: transp */
				colbf->copysize= 40;
		}

		fac= colbf->kref*colbf->amb;
		fac/= 255.0;
		colbf->ambr= fac*wrld.ambr;
		colbf->ambg= fac*wrld.ambg;
		colbf->ambb= fac*wrld.ambb;

		if(col->mode & 192) {	/* ztra */
			ok= 1;
			
			if(col->tra==0) {
				ok= 0;
				for(c=0; c<4; c++) {
					if(col->tex[c]) {
						if( (col->map[2*c] & 2) || (col->map[2*c+1] & 2) ) ok= 1;
					}
				}
			}

			if(ok) R.f|=16; /* transpflag */
			else colbf->mode &= ~192;
		}

		if(col->mode & 32) R.f|=128; /* env */

		colbf->lay= base->lay;
	}

	if(make_orcoflag) {
		make_orco(base, make_orcoflag);
	}

	return (colbf-colt+1);
}

void initlamp(base)
struct Base *base;
{
	struct VertRen *ver;
	struct LaData *la;
	struct LampRen *lar;
	struct HaloRen *har;
	float mat[4][4],bmat[4][4],hoco[4], vec[3], hasize, hoek, xn, yn, zn;
	long ok, z,c;
	short testclip();
	char *b;

	if(G.totlamp>=MAXLAMP) {
		printf("lamp overflow\n");
		return;
	}
	la= (struct LaData *)base->d;
	lar= (struct LampRen *)callocN(sizeof(struct LampRen),"lampren");
	R.la[G.totlamp++]= lar;
	parentbase(base, bmat, mat);
	Mat4MulMat4(mat, bmat, G.viewmat);
	
	Mat4Inv(bmat, mat); /* imat  */
	Mat3CpyMat4(base->imat, bmat);
	base->ivec[0]= -mat[3][0];
	base->ivec[1]= -mat[3][1];
	base->ivec[2]= -mat[3][2];

	lar->soort= la->soort;
	lar->f= la->f;
	lar->energy= la->energy*wrld.expos;
	lar->vec[0]= -mat[2][0];
	lar->vec[1]= -mat[2][1];
	lar->vec[2]= -mat[2][2];
	Normalise(lar->vec);
	lar->co[0]= mat[3][0];
	lar->co[1]= mat[3][1];
	lar->co[2]= mat[3][2];
	lar->dist= la->ld;
	lar->haint= la->haint;
	lar->distkw= lar->dist*lar->dist;
	lar->r= lar->energy*((float)la->r)/255.0;
	lar->g= lar->energy*((float)la->g)/255.0;
	lar->b= lar->energy*((float)la->b)/255.0;
	lar->f1= la->f1;
	lar->spotsi= fcos( PI*((float)(256- la->spsi))/512.0 );
	lar->spotbl= 1.0 -fcos( PI*((float)(la->spbl))/512.0 );
	memcpy(lar->tex,la->tex,28);

	lar->lay= base->lay;

	lar->ld1= ((float)la->ld1)/255.0;
	lar->ld2= ((float)la->ld2)/255.0;

	if(lar->soort==1) {
		Mat3CpyMat3(lar->imat, base->imat);
		Normalise(lar->imat[0]);
		Normalise(lar->imat[1]);
		Normalise(lar->imat[2]);

		xn= facos(lar->spotsi);
		xn= fsin(xn)/fcos(xn);
		lar->spottexfac= 32760/(xn);
	}
	
	/* imat bases */
	for(c=0;c<4;c++) {
		if(lar->map[c] & 1024) {
			if(lar->base[c]==0) lar->base[c]= base;
		}
	}

	/* flag zetten coor spothalo en initvars */
	if(la->soort==1 && la->f & 2) {
		if(la->haint>0) {
			R.f |= 4096;
			
			/* camerapos (0,0,0) roteren rondom lamp */
			lar->sh_invcampos[0]= -lar->co[0];
			lar->sh_invcampos[1]= -lar->co[1];
			lar->sh_invcampos[2]= -lar->co[2];
			Mat3MulVecfl(lar->imat, lar->sh_invcampos);
				
			/* z factor, zodat het volume genormaliseerd is */
			hoek= facos(lar->spotsi);
			xn= lar->spotsi;
			yn= fsin(hoek);
			lar->sh_zfac= yn/xn;
			/* alvast goed scalen */
			lar->sh_invcampos[2]*= lar->sh_zfac;
			
		}
	}
	
	if(R.f & 2) if(lar->f & 1) if(la->soort==1) {
		parentbase(base, mat, bmat);	/* transf.matrix */
		initshadowbuf(lar, la, mat);
	}
}

void defaultlamp()
{
	struct LampRen *lar;

	lar= (struct LampRen *)callocN(sizeof(struct LampRen),"lampren");
	R.la[G.totlamp++]=lar;

	lar->soort= 2;
	lar->vec[0]= -G.viewmat[2][0];
	lar->vec[1]= -G.viewmat[2][1];
	lar->vec[2]= -G.viewmat[2][2];
	Normalise(lar->vec);
	lar->r= 1.0;
	lar->g= 1.0;
	lar->b= 1.0;
	lar->lay= 65535;
}

void initsphere(base,mat,imat,dim)
struct Base *base;
float *mat,imat[][4];
short dim;
{
	struct PerfSph *ps;
	struct ColBlck *cb;
	struct VertRen *ver;
	struct VertOb *adrve;
	struct VlakRen *vlr;
	float fi=0;
	float si[60],co[60],*vec,*v;
	long vlakstart,vlak,temp;
	short a,b,di;

	ps=(struct PerfSph *)base->d;
	cb= &ps->c;

	for(a=0;a<=2*dim;a++) {
		si[a]= fsin(fi);
		co[a]= fcos(fi);
		fi+=PI/dim;
	}
	vec=(float *)mallocN(2*(dim+1)*(dim+1)*3*4,"spherevec");
	v=vec;
	for(a=0;a<=2*dim;a++) {
		for(b=0;b<=dim;b++) {
			*(v++)=si[a]*si[b];
			*(v++)=co[a]*si[b];
			*(v++)=co[b];
		}
	}

	di=dim+1;
	adrve=ps->vert=(struct VertOb *)mallocN( di*2*di*sizeof(struct VertOb),"sphereadrve");
	temp=G.totvert;
	v=vec;
	vlakstart= G.totvlak;

	for(a=0;a<=2*dim;a++) {
		for(b=0;b<=dim;b++) {
			if(b<dim && a<2*dim) {
				vlr=addvlak(G.totvlak++);
				vlr->v1=addvert(temp+a*di+b);
				vlr->v2=addvert(temp+a*di+di+b);
				vlr->v3=addvert(temp+a*di+di+1+b);
				/* vlr->col= cb; */
				vlr=addvlak(G.totvlak++);
				vlr->v1=addvert(temp+a*di+di+1+b);
				vlr->v2=addvert(temp+a*di+1+b);
				vlr->v3=addvert(temp+a*di+b);
				/* vlr->col=cb; */
			}
			adrve->c[0]=adrve->n[0]= 32767*v[0];			
			adrve->c[1]=adrve->n[1]= 32767*v[1];
			adrve->c[2]=adrve->n[2]= 32767*v[2];
			ver=addvert(G.totvert++);
			ver->n[0]= v[0]*imat[0][0]+v[1]*imat[0][1]+v[2]*imat[0][2];
			ver->n[1]= v[0]*imat[1][0]+v[1]*imat[1][1]+v[2]*imat[1][2];
			ver->n[2]= v[0]*imat[2][0]+v[1]*imat[2][1]+v[2]*imat[2][2];
			Normalise(ver->n);
			ver->v=adrve;

			Mat4MulVecfl(mat,v);
			VECCOPY(ver->co,v);
			v+=3;
			adrve++;
		}
	}
	/* dit vanwege correcte omklap normaal en puno */
	for(vlak=vlakstart;vlak<G.totvlak;vlak++) {
		vlr= addvlak(vlak);
		CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co,vlr->n);
	}
	freeN(vec);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


void polyzijvlakken(ob,cmat, colbf)
struct ObData *ob;
float cmat[][4];
struct ColBlckF *colbf;
{
	struct VertRen *v1,*v2,*v3;
	struct VlakRen *vlr;
	struct PolyData *po;
	struct ColBlckF *cbf;
	float *vec,*vec1,*vec2,fac,xn,yn,zn,n[3],*n1,*n2;
	long b1,b2;
	short *f45,d,ext=1,vert,a,b,c,*p,hoek,overslaan;

	po= ob->po;
	if(po==0) return;

	if(po->ext) ext++;
	if(po->f45m!=0 || po->f45p!=0) ext*=2;

	if(ext==1) {
		return;
	}

	/* voorberekenen alle punten */
	vec1=vec=(float *)mallocN(ext*3*4*po->vert,"polyzijvl");
	for(c=0;c<ext;c++) {
		if(po->f45m!=0 || po->f45p!=0) {

			if(c<2) d= po->depth*po->ext;
			else d= -po->depth*po->ext;
			if(c==0) {
				fac= -po->depth*po->f45m;
				d+= po->depth*po->f45m;
			} else if(c==1) {
				fac= po->depth*po->f45p;
				d-= po->depth*po->f45p;
			} else if (c==2) {
				fac= po->depth*po->f45p;
				d+= po->depth*po->f45p;
			} else {
				fac= -po->depth*po->f45m;
				d-= po->depth*po->f45m;
			}
			fac/= 1000.0;

			p=(short *)(po+1);
			f45=po->f45;
			for(a=0;a<po->poly;a++) {
				vert= *p;
				p++;
				for(b=0;b<vert;b++) {
					vec1[0]= p[0]+fac*f45[0];
					vec1[1]= p[1]+fac*f45[1];
					vec1[2]= d;
					Mat4MulVecfl(cmat,vec1);
					p+=3;
					f45+=3;
					vec1+=3;
				}
			}
		}
		else {
			if(c==0) d= po->depth*po->ext;
			else d= -po->depth*po->ext;

			p=(short *)(po+1);
			for(a=0;a<po->poly;a++) {
				vert= *p;
				p++;
				for(b=0;b<vert;b++) {
					vec1[0]= p[0];
					vec1[1]= p[1];
					vec1[2]= d;
					Mat4MulVecfl(cmat,vec1);
					vec1+=3;
					p+=3;
				}
			}
		}
	}
	/* de zijvlakken */
	vec1= vec;
	vec2= vec+3*po->vert;
	for(c=1;c<ext;c++) {
		/* kleurblokken goedzetten */
		cbf= colbf;
		if(po->f45m!=0 || po->f45p!=0) {
			if(ext==4) {
				if(c==2) cbf++; 
				else cbf+=2;
			} else cbf+=2;
		} else cbf++;

		overslaan= 0;
		if(ext==2) {
			if(po->f45m!=0 || po->f45p!=0) {
				if( (po->f1 & 2)==0 ) overslaan= 1;
			}
			else if( (po->f1 & 4)==0) overslaan= 1;
		}
		else {
			if(c==1 && (po->f1 & 2)==0 ) overslaan= 1;
			if(c==2 && (po->f1 & 4)==0 ) overslaan= 1;
			if(c==3 && (po->f1 & 8)==0 ) overslaan= 1;
		}

		if(overslaan) {
			vec1+= 3*po->vert;
			vec2+= 3*po->vert;
		}
		else {
			p=(short *)(po+1);
			for(a=0;a<po->poly;a++) {
				vert= *(p++);
				for(b=0;b<vert;b++) {
					v1= addvert(G.totvert++);
					VECCOPY(v1->co,vec1);

					v1= addvert(G.totvert++);
					VECCOPY(v1->co,vec2);

					/* eerste vlak */
					vlr= addvlak(G.totvlak++);
					vlr->v1= addvert(G.totvert-1);
					vlr->v2= addvert(G.totvert-2);
					if(b<vert-1) {
						vlr->v3= addvert(G.totvert);
						vlr->len= CalcNormFloat(vec1,vec2,vec1+3,n);
					}
					else {
						vlr->v3= addvert(G.totvert-2*vert);
						vlr->len= CalcNormFloat(vec1,vec2,vec1-3*vert+3,n);
					}
					VECCOPY(vlr->n,n);
					vlr->col= cbf;
					/* tweede vlak */
					vlr= addvlak(G.totvlak++);
					vlr->v1= addvert(G.totvert-1);
					if(b<vert-1) {
						vlr->v2= addvert(G.totvert);
						vlr->v3= addvert(G.totvert+1);
					}
					else {
						vlr->v2= addvert(G.totvert-2*vert);
						vlr->v3= addvert(G.totvert-2*vert+1);
					}
					VECCOPY(vlr->n,n);
					if(R.f & 4) 
						vlr->len= CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co,vlr->n);
						
					vlr->col= cbf;

					p+=3;
					vec1+=3; 
					vec2+=3;
				}

				if(cbf->mode & 1) { 	/* puno's */
					b2= G.totvlak-2*vert;
					b1= G.totvert-2*vert;
					for(b=0;b<vert;b++) {
						n1= addvlak(b2)->n;
						if(b==0) n2= addvlak(b2+2*vert-2)->n;
						else n2= addvlak(b2-2)->n;

						v2= addvert(b1);
						/* test dubbele punten */
						if(b==0) v1= addvert(b1+2*vert-2);
						else v1= addvert(b1-2);
						if(b==vert-1) v3=addvert(G.totvert-2*vert);
						else v3=addvert(b1+2);
						hoek=0;
						if(v1->co[0]==v2->co[0])
							if(v1->co[1]==v2->co[1])
								if(v1->co[2]==v2->co[2])
									hoek=1;
						if(v3->co[0]==v2->co[0])
							if(v3->co[1]==v2->co[1])
								if(v3->co[2]==v2->co[2])
									hoek=2;
						if(hoek==0) {
							v2->n[0]= n1[0]+n2[0];
							v2->n[1]= n1[1]+n2[1];
							v2->n[2]= n1[2]+n2[2];
							Normalise(v2->n);
						} else if(hoek==1) {
							VECCOPY(v2->n,n1);
						} else {
							VECCOPY(v2->n,n2);
						}
						v3= addvert(b1+1);
						VECCOPY(v3->n,v2->n);
						b2+=2; 
						b1+=2;
					}
				}
			}
		}
	}

	freeN(vec);
}

void initpolyren(ob,cmat,bmat, colbf)
struct ObData *ob;
float cmat[][4],bmat[][4];
struct ColBlckF *colbf;
{
	extern struct ListBase fillvertbase;
	extern struct ListBase filledgebase;
	extern struct ListBase fillvlakbase;
	struct EditVert *eve,*v1,*vstart,*vlast;
	struct EditEdge *eed,*e1;
	struct EditVlak *evl,*nextvl;
	struct PolyData *po;
	struct VertRen *ver;
	struct VlakRen *vlr;
	struct ColBlckF *cbf;
	float dvec[3],n[3],xn,yn,zn,fac,*vertdata,*f1;
	short a,b,ext=1,c,d,len,*f45,*v,*v0,edgefill();


	po= ob->po;
	if(po->vert==0) return;

	/* boundboxclip */

	if(po->ext==0 && po->f45==0 && (R.f & 2)==0 && R.xparts*R.yparts==1 ) {
		float sq[4][3],ho[4];
		short min[2],max[2],c1,c2,c3,c4,testclip();

		min[0]=min[1]=  32767;
		max[0]=max[1]= -32767;

		v= (short *)(po+1);
		for(b=0;b<po->poly;b++) {
			len= *(v++);
			for(c=0;c<len;c++) {
				min[0]= MIN2(min[0],v[0]);
				min[1]= MIN2(min[1],v[1]);
				max[0]= MAX2(max[0],v[0]);
				max[1]= MAX2(max[1],v[1]);
				v+=3;
			}
		}
		sq[0][0]= min[0]; 
		sq[0][1]= min[1]; 
		sq[0][2]= 0.0;
		sq[1][0]= max[0]; 
		sq[1][1]= min[1]; 
		sq[1][2]= 0.0;
		sq[2][0]= max[0]; 
		sq[2][1]= max[1]; 
		sq[2][2]= 0.0;
		sq[3][0]= min[0]; 
		sq[3][1]= max[1]; 
		sq[3][2]= 0.0;
		Mat4MulVecfl(cmat,sq[0]);
		Mat4MulVecfl(cmat,sq[1]);
		Mat4MulVecfl(cmat,sq[2]);
		Mat4MulVecfl(cmat,sq[3]);

		projectvert(sq[0],ho);
		c1= testclip(ho);
		projectvert(sq[1],ho);
		c2= testclip(ho);
		projectvert(sq[2],ho);
		c3= testclip(ho);
		projectvert(sq[3],ho);
		c4= testclip(ho);

		if(c1 & c2 & c3 & c4) {		/* helemaal eruit */
			/* printf("%d %d %d %d\n", c1, c2, c3, c4); */
			return;
		}
	}

	polyzijvlakken(ob,cmat, colbf);

	if( (po->f1 & 17)==0) return;	/* geen voor/achtervlak */
	if( (po->f1 & 1)==0 && po->ext==1) return;	/* geen voorvlak */

	/* vertex data maken */
	if(po->ext) ext= 2;
	f1= vertdata= (float *)mallocN(4*3*po->vert,"initpolyren");

	v= (short *)(po+1);
	f45= po->f45;

	fac= -po->depth*po->f45m/1000.0;

	for(b=0;b<po->poly;b++) {
		len= *(v++);
		if(po->f45m) {
			for(c=0;c<len;c++) {
				f1[0]= v[0]+fac*f45[0];
				f1[1]= v[1]+fac*f45[1];
				f1[2]= 0;
				v+=3; 
				f45+=3; 
				f1+=3;
			}
		} else {
			for(c=0;c<len;c++) {
				f1[0]= v[0];
				f1[1]= v[1];
				f1[2]= 0;
				v+=3; 
				f1+=3;
			}
		}
	}

	/* editverts en edges maken */
	f1= vertdata;
	v= (short *)(po+1);
	for(b=0;b<po->poly;b++) {
		len= *(v++);
		eve= v1= 0;
		for(c=0;c<len;c++) {
			if(c>0) {
				while( f1[-3]==f1[0] && f1[-2]==f1[1] ) {
					v+=3; 
					f1+=3; 
					c++;
					if(c>=len) break;
				}
				if(c>=len) break;
			}
			vlast= eve;
			eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
			addtail(&fillvertbase,eve);
			VECCOPY(eve->co,f1);
			if(vlast==0) v1= eve;
			else {
				eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
				addtail(&filledgebase,eed);
				eed->v1= vlast;
				eed->v2= eve;
			}
			v+=3; 
			f1+=3;
		}
		if(eve!=0 && v1!=0) {
			eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
			addtail(&filledgebase,eed);
			eed->v1= eve;
			eed->v2= v1;
		}
	}
	if(edgefill(0)==0) {
		printf("can't fill poly\n");
	}
	else {

		/* de vlaknormaal en extrude offset */
		xn= po->n[0]; 
		yn= po->n[1]; 
		zn= po->n[2];

		n[0]= bmat[0][0]*xn+bmat[0][1]*yn+bmat[0][2]*zn;
		n[1]= bmat[1][0]*xn+bmat[1][1]*yn+bmat[1][2]*zn;
		n[2]= bmat[2][0]*xn+bmat[2][1]*yn+bmat[2][2]*zn;
		Normalise(n);
		d= po->depth*po->ext+po->depth*po->f45m;

		/* de verts krijgen op ->vn een pointer naar de VertRen, tevens roteren */

		a=0;
		if((po->f1 & 1)==0) a=1;
		if((po->f1 & 16)==0) ext= 1;
		for(;a<ext;a++) {
			eve= (struct EditVert *)fillvertbase.first;
			while(eve) {
				ver= addvert(G.totvert++);
				VECCOPY(ver->co,eve->co);
				ver->co[2]+= d;
				Mat4MulVecfl(cmat,ver->co);
				eve->vn= (struct EditVert *)ver;
				eve= eve->next;
			}
			cbf= colbf;
			evl= (struct EditVlak *)fillvlakbase.first;
			while(evl) {
				vlr= addvlak(G.totvlak++);
				VECCOPY(vlr->n,n);
				vlr->v1= (struct VertRen *)evl->v1->vn;
				vlr->v2= (struct VertRen *)evl->v2->vn;
				vlr->v3= (struct VertRen *)evl->v3->vn;
				vlr->col= cbf;
				if(R.f & 4)
					vlr->len= CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co,vlr->n);

				evl= evl->next;
			}
			d= -d;
		}
	}

	freeN(vertdata);

	freelist(&fillvertbase);
	freelist(&filledgebase);
	freelist(&fillvlakbase);
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;
}

void split_u_renderfaces(startvlak, startvert, usize, plek, cyclu)
long startvlak, startvert, usize, plek, cyclu;
{
	struct VlakRen *vlr;
	struct VertRen *v1, *v2;
	long a, v;

	/* geef eerst alle betreffende vertices een pointer naar de nieuwe mee */
	v= startvert+ plek*usize;
	for(a=0; a<usize; a++) {
		v2= addvert(G.totvert++);
		v1= addvert(v++);
		*v2= *v1;
		v1->colg= (ulong)v2;
	}

	/* loop betreffende vlakken af en vervang pointers */
	v= startvlak+plek*2*(usize-1+cyclu);
	for(a=1-cyclu; a<usize; a++) {
		vlr= addvlak(v++);
		vlr->v1= (struct VertRen *)(vlr->v1->colg);
		vlr= addvlak(v++);
		vlr->v1= (struct VertRen *)(vlr->v1->colg);
		vlr->v2= (struct VertRen *)(vlr->v2->colg);
	}

}

void split_v_renderfaces(startvlak, startvert, usize, vsize, plek, cyclu, cyclv)
long startvlak, startvert, usize, vsize, plek, cyclu, cyclv;
{
	struct VlakRen *vlr;
	struct VertRen *v1=0, *v2;
	long a, vert, vlak, ofs;

	if(vsize<2) return;
	
	/* loop betreffende vlakken af en maak dubbels */
	/* omdat (evt) split_u al gedaan is kan niet met vertex->colg pointers worden gewerkt  */
	/* want vlakken delen al geen punten meer */

	if(plek+cyclu==usize) plek= -1;

	vlak= startvlak+(plek+cyclu)*2;
	ofs= 2*(usize-1+cyclu);

	for(a=1; a<vsize; a++) {

		vlr= addvlak(vlak);
		if (vlr->v1 == 0) return; /* OEPS, als niet cyclic */
		
		v1= addvert(G.totvert++);
		*v1= *(vlr->v1);
		
		vlr->v1= v1;

		vlr= addvlak(vlak+1);
		vlr->v1= v1;

		if(a>1) {
			vlr= addvlak(vlak-ofs);
			if(vlr->v3->colg) {
				v1= addvert(G.totvert++);
				*v1= *(vlr->v3);
				vlr->v3= v1;
			}
			else vlr->v3= v1;
		}

		if(a== vsize-1) {
			if(cyclv) {
				;
			}
			else {
				vlr= addvlak(vlak);
				v1= addvert(G.totvert++);
				*v1= *(vlr->v3);
				vlr->v3= v1;
			}
		}

		vlak+= ofs;
	}

}

void initNurbren(ob, cmat, colbf)
struct ObData *ob;
float cmat[][4];
struct ColBlckF *colbf;
{
	struct Nurb *nu=0;
	struct ListBase displist;
	struct DispList *dl;
	struct VertRen *ver, *v1, *v2, *v3, *v4;
	struct VlakRen *vlr;
	float *data, n1[3], n2[3], flen;
	long len, a, b, startvlak, startvert, p1, p2, p3, p4;
	short *orco;
	
	if(ob->cu) nu= ob->cu->curve.first;
	if(nu==0) return;

	/* een complete displist maken, de basedisplist kan compleet anders zijn */
	displist.first= displist.last= 0;
	nu= ob->cu->curve.first;
	while(nu) {
		if(nu->pntsv>1) {
			len= nu->resolu*nu->resolv;
			dl= callocN(sizeof(struct DispList)+len*3*4, "makeDispList1");
			addtail(&displist, dl);

			dl->parts= nu->resolu;	/* andersom want makeNurbfaces gaat zo */
			dl->nr= nu->resolv;
			dl->col= nu->col;

			data= (float *)(dl+1);
			dl->type= DL_SURF;
			if(nu->flagv & 1) dl->flag|= 1;	/* ook andersom ! */
			if(nu->flagu & 1) dl->flag|= 2;

			makeNurbfaces(nu, data);
		}
		nu= nu->next;
	}

	orco= ob->cu->orig;

	dl= displist.first;
	while(dl) {
		if(dl->type==DL_SURF) {
			startvert= G.totvert;
			a= dl->nr*dl->parts;
			data= (float *)(dl+1);
			while(a--) {
				ver= addvert(G.totvert++);
				VECCOPY(ver->co, data);
				if(orco) {
					ver->v= (struct VertOb *)orco;
					orco+= 3;
				}
				Mat4MulVecfl(cmat, ver->co);
				data+= 3;
			}

			startvlak= G.totvlak;

			for(a=0; a<dl->parts; a++) {

				DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);
				p1+= startvert; 
				p2+= startvert; 
				p3+= startvert; 
				p4+= startvert;

				for(; b<dl->nr; b++) {
					v1= addvert(p1); 
					v2= addvert(p2);
					v3= addvert(p3); 
					v4= addvert(p4);

					flen= CalcNormFloat(v2->co, v3->co, v4->co, n1);
					if(flen>0.0) {
						vlr= addvlak(G.totvlak++);
						vlr->v1= v2;
						vlr->v2= v3;
						vlr->v3= v4;
						VECCOPY(vlr->n, n1);
						vlr->len= flen;
												
						vlr->col= colbf+dl->col;
						vlr->ec |= 4;
					}

					flen= CalcNormFloat(v2->co, v1->co, v3->co, n2);
					if(flen>0.0) {
						vlr= addvlak(G.totvlak++);
						vlr->v1= v2;
						vlr->v2= v1;
						vlr->v3= v3;
						VECCOPY(vlr->n, n2);
						vlr->len= flen;
						
						vlr->col= colbf+dl->col;
						vlr->ec |= 1;
					}

					VecAddf(n1, n1, n2);
					VecAddf(v1->n, v1->n, n1);
					VecAddf(v2->n, v2->n, n1);
					VecAddf(v3->n, v3->n, n1);
					VecAddf(v4->n, v4->n, n1);

					p4= p3; 
					p3++;
					p2= p1; 
					p1++;
				}
			}

			for(a=startvert; a<G.totvert; a++) {
				ver= addvert(a);
				Normalise(ver->n);
			}
		}
		dl= dl->next;
	}
	freelistN(&displist);
}

void initpolyrenN(ob,cmat, bmat, colbf)
struct ObData *ob;
float cmat[][4],bmat[][4];
struct ColBlckF *colbf;
{
	extern struct ListBase fillvertbase;		/* isect.c */
	extern struct ListBase filledgebase;
	extern struct ListBase fillvlakbase;
	extern short holefill;
	struct VertRen *ver;
	struct VlakRen *vlr;
	struct EditVert *eve,*v1,*vstart,*vlast;
	struct EditEdge *eed,*e1;
	struct EditVlak *evl,*nextvl;
	struct ListBase dlbev;
	struct Nurb *nu=0;
	struct DispList *dlb;
	struct BevList *bl;
	struct BevPoint *bevp;
	struct QStrip *qs;
	struct VertStrip *vs1;
	float len, *data, *polydata, *dpolyd, *fp, *fp1;
	float n[3], vec[3], widfac;
	long parts, nr, startvert, startvlak, a, b, p1, p2, p3, p4;
	long verts, polyofs, ofs, end;
	short edgefill(), ext=1;

	if(ob->cu) nu= ob->cu->curve.first;
	if(nu==0) return;


	/* boundboxclip nog doen */

	/* bevelcurve in displist */
	dlbev.first= dlbev.last= 0;
	if(ob->cu->ext1 || ob->cu->ext2 || ob->cu->bevbase) {
		makebevelcurve(ob->cu, &dlbev);
	}

	/* aantal tellen voor polyfill */
	if(dlbev.first) {
		if(dlbev.first==dlbev.last) ext= 2;
		else ext= 0;
	}
	if(ext) {
		verts= 0;
		bl= ob->cu->bev.first;
		while(bl) {
			if(bl->poly>0) verts+= bl->nr;

			bl= bl->next;
		}
		if(verts==0) ext= 0;
		else {
			if(ext==2) polyofs= 3*verts;

			polydata= mallocN(3*4*ext*verts, "initpolyrenN0");
			dpolyd= polydata;
		}
	}

	/* polyzijvlakken:  met bevellist werken */
	widfac= (ob->cu->width-100)/ob->cu->ws;

	bl= ob->cu->bev.first;
	nu= ob->cu->curve.first;
	while(bl) {

		if(dlbev.first==0) {    /* alleen een poly */
			if(bl->poly>0) {
				bevp= (struct BevPoint *)(bl+1);
				a= bl->nr;
				while(a--) {

					dpolyd[0]= bevp->x+widfac*bevp->sin;
					dpolyd[1]= bevp->y+widfac*bevp->cos;
					dpolyd[2]= bevp->z;
					/* hier niet al MatMullen: polyfill moet uniform werken, ongeacht frame */
					dpolyd+= 3;
					bevp++;
				}
			}
		}
		else {

			dlb= dlbev.first;   /* bevel lus */
			while(dlb) {
				data= mallocN(12*dlb->nr*bl->nr, "initpolyrenN3");
				fp= data;

				/* voor ieder punt van bevelcurve de hele poly doen */
				fp1= (float *)(dlb+1);
				b= dlb->nr;
				while(b--) {

					bevp= (struct BevPoint *)(bl+1);
					a= bl->nr;
					while(a--) {

						if(ob->cu->flag & 256) {	/* 3D */
							vec[0]= fp1[1]+widfac;
							vec[1]= 0.0;
							vec[2]= fp1[2];
							Mat3MulVecfl(bevp->mat, vec);
							
							fp[0]= bevp->x+ vec[0];
							fp[1]= bevp->y+ vec[1];
							fp[2]= bevp->z+ vec[2];
						}
						else {

							fp[0]= bevp->x+ (widfac+fp1[1])*bevp->sin;
							fp[1]= bevp->y+ (widfac+fp1[1])*bevp->cos;
							fp[2]= bevp->z+ fp1[2];
							/* hier niet al MatMullen: polyfill moet uniform werken, ongeacht frame */
						}
						fp+= 3;
						bevp++;
					}
					fp1+=3;
				}

				/* uit deze data de polyfilldata halen */
				if(ext==2 && bl->poly>0) {
					fp= data;
					nr= bl->nr;
					ofs= 3*bl->nr*(dlb->nr-1);
					while(nr--) {
						VECCOPY(dpolyd, fp);
						VECCOPY(dpolyd+polyofs, fp+ofs);
						dpolyd+= 3;
						fp+= 3;
					}
				}

				/* rendervertices maken */
				fp= data;
				startvert= G.totvert;
				nr= dlb->nr*bl->nr;

				while(nr--) {
					ver= addvert(G.totvert++);
					VECCOPY(ver->co, fp);
					Mat4MulVecfl(cmat, ver->co);
					fp+= 3;
				}

				startvlak= G.totvlak;

				for(a=0; a<dlb->nr; a++) {

					DL_SURFINDEX(bl->poly>0, dlb->type==DL_POLY, bl->nr, dlb->nr);
					p1+= startvert; 
					p2+= startvert; 
					p3+= startvert; 
					p4+= startvert;

					for(; b<bl->nr; b++) {
						vlr= addvlak(G.totvlak++);
						vlr->v1= addvert(p2);
						vlr->v2= addvert(p3);
						vlr->v3= addvert(p4);
						vlr->ec= 4;
						
						/* vlaknormaal: beide vlakken dezelfde geven: anders vieze
						* effecten bij gekruiste vlakken in de hoeken
						*/
						vlr->len= CalcNormFloat(vlr->v1->co, vlr->v2->co, vlr->v3->co, n);
						VECCOPY(vlr->n, n);
						vlr->col= colbf+nu->col;

						vlr= addvlak(G.totvlak++);
						vlr->v1= addvert(p2);
						vlr->v2= addvert(p1);
						vlr->v3= addvert(p3);
						vlr->ec= 1;
						
						VECCOPY(vlr->n, n);
						if(R.f & 4)
							vlr->len= CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co, vlr->n);
						
						vlr->col= colbf+nu->col;

						p4= p3; 
						p3++;
						p2= p1; 
						p1++;
					}

				}

				/* dubbele punten maken: POLY SPLITSEN */
				if(dlb->nr==4 && ob->cu->bevbase==0) {
					split_u_renderfaces(startvlak, startvert, bl->nr, 1, bl->poly>0);
					split_u_renderfaces(startvlak, startvert, bl->nr, 2, bl->poly>0);
				}
				/* dubbele punten maken: BEVELS SPLITSEN */
				bevp= (struct BevPoint *)(bl+1);
				for(a=0; a<bl->nr; a++) {
					if(bevp->f1)
						split_v_renderfaces(startvlak, startvert, bl->nr, dlb->nr, a, bl->poly>0,
						    dlb->type==DL_POLY);
					bevp++;
				}

				/* puntnormalen */
				b= 1;
				for(a= startvlak; a<G.totvlak; a++) {
					vlr= addvlak(a);

					if(b & 1) {
						VecAddf(vlr->v1->n, vlr->v1->n, vlr->n);
						VecAddf(vlr->v3->n, vlr->v3->n, vlr->n);
					}
					else {
						VecAddf(vlr->v2->n, vlr->v2->n, vlr->n);
						VecAddf(vlr->v3->n, vlr->v3->n, vlr->n);
					}

					b++;
				}
				for(a=startvert; a<G.totvert; a++) {
					ver= addvert(a);
					len= Normalise(ver->n);
					if(len==0) ver->colg==1;
					else ver->colg== 0;
				}
				for(a= startvlak; a<G.totvlak; a++) {
					vlr= addvlak(a);
					if(vlr->v1->colg) VECCOPY(vlr->v1->n, vlr->n);
					if(vlr->v2->colg) VECCOPY(vlr->v2->n, vlr->n);
					if(vlr->v3->colg) VECCOPY(vlr->v3->n, vlr->n);
				}

				dlb= dlb->next;

				freeN(data);
			}

		}
		bl= bl->next;
		nu= nu->next;
	}

	if(dlbev.first) {
		freebevelcurve(&dlbev);
	}

	/* poly's vullen het polydata array */

	if(ext==0) return;
	if((ob->cu->flag & 3)==0 || (ob->cu->flag & 256) ) {
		freeN(polydata);
		return;
	}

	dpolyd= polydata;
	bl= ob->cu->bev.first;
	nu= ob->cu->curve.first;
	while(bl) {

		eve= v1= vlast= 0;
		if(bl->poly>0) {
			nr= bl->nr;

			while(nr-- >0 ) {		/* >0 moet! */
				if(nr<bl->nr-1) {
					while(dpolyd[-3]==dpolyd[0] && dpolyd[-2]==dpolyd[1]) {
						nr--; 
						dpolyd+=3;
						if(nr<=0) break;
					}
				}
				vlast= eve;
				eve= (struct EditVert *)calloc(sizeof(struct EditVert),1);
				addtail(&fillvertbase,eve);
				VECCOPY(eve->co,dpolyd);
				eve->xs= nu->col;	/* de edgefill laat deze ongemoeid */
				eve->vn= (struct EditVert *)dpolyd;

				if(vlast==0) v1= eve;
				else {
					eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
					addtail(&filledgebase,eed);
					eed->v1= vlast;
					eed->v2= eve;
				}

				dpolyd+=3;
			}
			if(eve!=0 && v1!=0) {
				eed= (struct EditEdge *)calloc(sizeof(struct EditEdge),1);
				addtail(&filledgebase,eed);
				eed->v1= eve;
				eed->v2= v1;
			}
		}
		bl= bl->next;
		nu= nu->next;
		if(nu==0 && bl!=0) {
			printf("aantal bevels!=nurbs\n");
			break;
		}
	}

	if(edgefill(0)==0) {
		printf("can't fill poly\n");
	}
	else {

		/* de vlaknormaal en extrude offset */

		n[0]= bmat[0][2];
		n[1]= bmat[1][2];
		n[2]= bmat[2][2];
		Normalise(n);

		/* de verts krijgen op ->vn een pointer naar de VertRen, tevens roteren */

		if(ext==2 && (ob->cu->flag & 1)) {
			eve= (struct EditVert *)fillvertbase.first;
			while(eve) {
				ver= addvert(G.totvert++);
				fp= (float *)eve->vn;
				fp+= polyofs;
				VECCOPY(ver->co,fp);
				Mat4MulVecfl(cmat, ver->co);
				VECCOPY(ver->n,n);
				ver->colg= eve->xs;	/* kleur */

				eve->vn= (struct EditVert *)ver;
				eve= eve->next;
			}
			evl= (struct EditVlak *)fillvlakbase.first;
			while(evl) {
				vlr= addvlak(G.totvlak++);
				vlr->v1= (struct VertRen *)evl->v1->vn;
				vlr->v2= (struct VertRen *)evl->v2->vn;
				vlr->v3= (struct VertRen *)evl->v3->vn;
				VECCOPY(vlr->n,n);
				if(R.f & 4)
					vlr->len= CalcNormFloat(vlr->v1->co, vlr->v2->co, vlr->v3->co, vlr->n);

				vlr->col= colbf+vlr->v1->colg;
				evl= evl->next;
			}
		}
		if(ob->cu->flag & 2) {
			eve= (struct EditVert *)fillvertbase.first;
			while(eve) {
				ver= addvert(G.totvert++);
				VECCOPY(ver->co,eve->co);
				Mat4MulVecfl(cmat, ver->co);
				VECCOPY(ver->n,n);
				ver->colg= eve->xs;	/* kleur */

				eve->vn= (struct EditVert *)ver;
				eve= eve->next;
			}
			evl= (struct EditVlak *)fillvlakbase.first;
			while(evl) {
				vlr= addvlak(G.totvlak++);
				vlr->v1= (struct VertRen *)evl->v1->vn;
				vlr->v2= (struct VertRen *)evl->v2->vn;
				vlr->v3= (struct VertRen *)evl->v3->vn;
				VECCOPY(vlr->n,n);

				if(R.f & 4)
					vlr->len= CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co,vlr->n);

				vlr->col= colbf+vlr->v1->colg;
				evl= evl->next;
			}
		}
	}

	freelist(&fillvertbase);
	freelist(&filledgebase);
	freelist(&fillvlakbase);
	fillvertbase.first=fillvertbase.last= 0;
	filledgebase.first=filledgebase.last= 0;
	fillvlakbase.first=fillvlakbase.last= 0;

	freeN(polydata);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void setzbufvlaggen(projectfunc)	/* ook homoco's */
void (*projectfunc)();
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct HaloRen *har;
	struct QStrip *qs;
	struct VertStrip *vs;
	float zn, vez[12], hoco[4];
	long a;
	short testclip();

	for(a=0;a<G.totvert;a++) {
		if((a & 255)==0) ver= R.blove[a>>8];
		else ver++;
		projectfunc(ver->co,ver->ho);
		ver->clip = testclip(ver->ho);
	}

	for(a=0;a<G.tothalo;a++) {
		if((a & 255)==0) har= R.bloha[a>>8];
		else har++;
		
		projectfunc(har->co, hoco);

		zn= hoco[3];
		har->xs= 0.5*R.rectx*(hoco[0]/zn);
		har->ys= 0.5*R.recty*(hoco[1]/zn);
		har->zs= 0x7FFFFF*(1.0+hoco[2]/zn);
		har->miny= har->ys-har->rad;
		har->maxy= har->ys+har->rad;
		
		if(har->f & 1) {	/* spot, dus overslaan */
			a++;
			if((a & 255)==0) har= R.bloha[a>>8];
			else har++;
		}
	}

	/* vlaggen op 0 zetten als eruit geclipt */
	for(a=0;a<G.totvlak;a++) {
		if((a & 255)==0) vlr= R.blovl[a>>8];
		else vlr++;

		vlr->rt1= 1;
		if(vlr->v1->clip & vlr->v2->clip & vlr->v3->clip) vlr->rt1= 0;
	}

	/*	qs= R.qstrip.first;
	while(qs) {
	    vs= qs->verts;
	    a= qs->pntsu*qs->pntsv;
	    while(a--) {
		projectfunc(vs->co, vs->ho);
		vs->clip= testclip(vs->ho);
		vs++;
	    }
	    qs= qs->next;
	}
*/
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void schrijfplaatje(name)
char *name;
{
	struct ImBuf *ibuf=0;
	long i,flags;
	char str[100];
	extern float rgb_to_bw[];
	extern int alpha_to_col0(int);
	
	/* Staat RGBA aan? Zo ja: gebruik alphakanaal voor kleur 0 */

	alpha_to_col0(FALSE);

	if (R.planes == 2) {
			/* alles met minder dan 50 % alpha -> col 0 */
		if (G.genlock == 1) alpha_to_col0(2);
			/* uitsluitend met 0 alpha -> col 0 */
		else alpha_to_col0(1);
	}

	if (R.imtype == 4) {	/* Ftype */
		strcpy(str, G.ftype);
		convertstringcode(str);
		ibuf = loadiffname(str, LI_rect | LI_test);
		if (ibuf) {
			freerectImBuf(ibuf);
			freeplanesImBuf(ibuf);
			ibuf->x = R.rectx;
			ibuf->y = R.recty;
		} else {
			error("Can't find filetype");
			G.afbreek= 1;
			return;
		}
		setdither(2);
	}
	if (ibuf == 0) {
		if (R.planes == 0){
			ibuf= allocImBuf(R.rectx,R.recty,8,0,0);
		} else if (R.planes == 2){
			ibuf= allocImBuf(R.rectx,R.recty,32,0,0);
		} else {
			ibuf= allocImBuf(R.rectx,R.recty,24,0,0);
		}
	}
	if (ibuf) {
		ibuf->rect= (ulong *) R.rectot;

		if (R.planes == 0) cspace(ibuf, rgb_to_bw);
		if(R.imtype==0) ibuf->ftype= IMAGIC;
		else if(R.imtype==1) ibuf->ftype= TGA;
		else if(R.imtype==2) {
			flags = AMI;
			if (ibuf->x > 400) flags |= AM_hires;
			if (R.ycor<.6) flags |= AM_lace;
			ibuf->ftype= flags;
		}
		else if(R.imtype==3) ibuf->ftype= AN_hamx;
		else if(R.imtype==5) {
			if(R.f & 32) {	/* field */
				ibuf->ftype= JPG_JST|90;
				if(R.rectx==360 && R.recty==576) scaleImBuf(ibuf, 720, 576);
			}
			else ibuf->ftype= JPG;
		}
		if (saveiff(ibuf, name, SI_rect)==0) {
			perror(name);
			G.afbreek=TRUE;
		}
		if(R.imtype==2) flipy(ibuf);
		freeImBuf(ibuf);
	} else {
		G.afbreek=TRUE;
	}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void printrenderinfo(jmode)
short jmode;
{
	short a;
	char str[40],str1[20];

	if(G.background) return;
	suspendtimer();

	if( (jmode!= -1) || (R.f & 32) || R.xparts>1 || R.yparts>1 ) {
		a= 0;
		str[0]= 0;
		if(R.f & 64) a=1;
		if(R.f & 32) sprintf(str," Field:%d ",a);
		if(R.xparts>1 || R.yparts>1) {
			sprintf(str1," Part:%d",pa);
			strcat(str,str1);
		}
		if(jmode!= -1) {
			sprintf(str1," Sample:%d",jmode);
			strcat(str,str1);
		}
		tekenoverdraw(str);
		winset(G.winar[9]);
	}
	else tekenoverdraw(1);
	winset(G.winar[9]);

	restarttimer();
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setwindowclip(mode,jmode)
short mode,jmode;
{
	/* jmode>=0: alleen jitter doen, anders berekenen  */
	/* mode==1 zet persmat en grvec */
	extern float holoofs;
	float fac, minx,miny,maxx,maxy, focalplane, near, far;
	float xd, yd, bmat[4][4];
	
	minx= R.xstart+.5;
	miny= R.ycor*(R.ystart+.5);
	maxx= R.xend+.4999;
	maxy= R.ycor*(R.yend+.4999);

	if(R.f & 64) {	/* tweede field */
		miny+= .5*R.ycor;
		maxy+= .5*R.ycor;
	}

	if(G.holo==3) {
		fac= (G.cfra-G.sfra)/((float)(G.efra-G.sfra))-0.5;
		fac*= (R.rectx);
		fac*= ( (float)G.holopadlen )/100.0;

		holoofs= -fac;
		minx-= fac;
		maxx-= fac;
	}

	xd= yd= 0.0;
	if(jmode!= -1) {
		xd= jit[jmode % G.osa][0];
		yd= R.ycor*jit[jmode % G.osa][1];

	}
	
	near= view0.near;	/* andere dan R.near! (is misschien ortho) */
	far= R.far;
	
	minx= R.pixsize*(minx+xd);
	maxx= R.pixsize*(maxx+xd);
	miny= R.pixsize*(miny+yd);
	maxy= R.pixsize*(maxy+yd);

	if(view0.cambase && view0.cambase->f2 & 64) {
		/* hier de near & far vermenigvuldigen is voldoende! */
		near*= 100.0;
		far*= 100.0;
	}

	if(G.background==1) {
		i_window(minx, maxx, miny, maxy, near, far, G.winmat);
	}
	else {
		window(minx, maxx, miny, maxy, near, far);
		
		mmode(MPROJECTION);
		getmatrix(G.winmat);
		mmode(MVIEWING);
	}

	if(mode== 1) {
		/* de G.persmat wordt in render niet meer gebruikt */
		Mat4Invert(G.viewinv, G.viewmat);
		VECCOPY(wrld.grvec, G.viewmat[2]);
		Normalise(wrld.grvec);
		Mat3CpyMat4(G.workbase->imat, G.viewinv);
		G.workbase->ivec[0]= -G.viewmat[3][0];
		G.workbase->ivec[1]= -G.viewmat[3][1];
		G.workbase->ivec[2]= -G.viewmat[3][2];
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void openwindow()
{
	float xm,ym;
	long xs, ys, xo, yo;
	short matflag=0,winok=0,div;
	/* opent window met R.rectx en R.recty */

	if(G.background==1) {
		return;
	}

	if(G.winar[9]) {  /* window testen */
		winset(G.winar[9]);
		getsize(&xs,&ys);
		getorigin(&xo,&yo);
		if(xs!=R.rectx || ys!=R.recty || xo!=R.xof || yo!=R.yof) {
			cpack(0x777777);
			clear();
		}
		else winok= 1;
	}

	if(winok==0) {
		suspendtimer(); /* vanwege BUG: combinatie timer en winopen/position loopt vast */

		/* berekenen offset */
		/* if(G.winar[9]) winclose(G.winar[9]); */
		/* G.winar[9]= 0; */

		if(R.rectspare) {
			freeN(R.rectspare);
			R.rectspare= 0;
		}

		xs = getgdesc(GD_XPMAX);
		ys = getgdesc(GD_YPMAX);

		if(R.winpos) {
			div= 0; 
			xm= 0.0; 
			ym= 0.0;
			if(R.winpos & (1+8+64)) {
				div++; 
				xm+=.1666;
			}
			if(R.winpos & (2+16+128)) {
				div++; 
				xm+= .50;
			}
			if(R.winpos & (4+32+256)) {
				div++; 
				xm+=.8333;
			}
			xm/= (float)div;
			div= 0;
			if(R.winpos & (1+2+4)) {
				div++; 
				ym+=.1666;
			}
			if(R.winpos & (8+16+32)) {
				div++; 
				ym+= .50;
			}
			if(R.winpos & (64+128+256)) {
				div++; 
				ym+=.8333;
			}
			ym/= (float)div;
			R.xof= (xs*xm- R.rectx/2);
			R.yof= (xs*ym- R.recty/2);
		}
		if(R.xof<0) R.xof=0;
		if(R.yof<0) R.yof=0;
		if(R.xof+R.rectx-1>xs) R.xof=xs-R.rectx-1;
		if(R.yof+R.recty-1>ys) R.yof=ys-R.recty-1;
		if(R.rectx>=xs-2) R.xof=0;
		if(R.recty>=ys-2) R.yof=0;

		if(G.winar[9]==0) {
			prefposition(R.xof,R.xof+R.rectx-1,R.yof,R.yof+R.recty-1);
			noborder();
			minsize(2,2);
			G.winar[9]= winopen("render");
			RGBmode();
			singlebuffer();
			gconfig();
			sginap(3);
		} else {
			winposition(R.xof,R.xof+R.rectx-1,R.yof,R.yof+R.recty-1);
			reshapeviewport();
		}
		if(G.winar[9]) {
			czclear(0,0x7FFFFE);	/* anders geeft indiElan vlekken */
		}
		if(G.winar[0]==0) {
			qdevice(ESCKEY);
			qdevice(PAUSEKEY);
		}

		drawmode(OVERDRAW);
		color(0);
		clear();
		drawmode(NORMALDRAW);

		if(qtest()) interruptESC(0);
		restarttimer();

	}
	winset(G.winar[9]);
	zbuffer(1); 
	shademodel(FLAT);
	mmode(MVIEWING);

	loadmatrix(mat1);
}

void maakpanorama()
{
	struct ImBuf *ibuf;
	long cfra,len,x,partx;
	char str[100];

	if(G.afbreek || G.background==1) return;
	len= (G.efra-G.sfra+1)*R.xsch;
	if(len>1280) return;

	if(R.rectot) freeN(R.rectot);
	R.rectot= 0;
	partx= R.xsch;
	R.rectx= R.xsch= len;
	openwindow();
	x= 0;
	for(cfra=G.sfra;cfra<=G.efra;cfra++) {
		makepicstring(str,cfra);

		ibuf= loadiffname(str,LI_rect);
		if(ibuf) {
			lrectwrite(x,0,x+partx-1,R.recty-1,ibuf->rect);
			freeImBuf(ibuf);
		}
		x+=partx;
	}
	R.rectot= mallocN(4*R.rectx*R.recty, "rectot");
	lrectread(0,0,R.rectx-1,R.recty-1,R.rectot);

}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void initparts()
{
	short nr,xd,yd,xpart,ypart,xparts,yparts;
	short a,xminb,xmaxb,yminb,ymaxb;

	if(G.border) {
		xminb= ((G.xminb-S.vxw/4)*R.xsch)/(S.vxw/2);
		xmaxb= ((G.xmaxb-S.vxw/4)*R.xsch)/(S.vxw/2);
		a= (S.vxw*R.ysch*R.yasp)/(4*R.xsch*R.xasp);
		yminb= ((G.yminb-S.vyw/2+a)*R.ysch)/(2*a);
		ymaxb= ((G.ymaxb-S.vyw/2+a)*R.ysch)/(2*a);
		if(xminb<0) xminb= 0;
		if(xmaxb>R.xsch) xmaxb= R.xsch;
		if(yminb<0) yminb= 0;
		if(ymaxb>R.ysch) ymaxb= R.ysch;
	}
	else {
		xminb=yminb= 0;
		xmaxb= R.xsch;
		ymaxb= R.ysch;
	}

	xparts= R.xparts;	/* voor border */
	yparts= R.yparts;

	for(nr=0;nr<xparts*yparts;nr++)
		allparts[nr][0]= -1;	/* array leegmaken */

	xpart= R.xsch/xparts;
	ypart= R.ysch/yparts;

	/* als border: testen of aantal parts minder kan */
	if(G.border) {
		a= (xmaxb-xminb-1)/xpart+1; /* zoveel parts in border */
		if(a<xparts) xparts= a;
		a= (ymaxb-yminb-1)/ypart+1; /* zoveel parts in border */
		if(a<yparts) yparts= a;

		xpart= (xmaxb-xminb)/xparts;
		ypart= (ymaxb-yminb)/yparts;
	}

	for(nr=0;nr<xparts*yparts;nr++) {
		xd= (nr % xparts);
		yd= (nr-xd)/xparts;

		allparts[nr][0]= xminb+ xd*xpart;
		allparts[nr][1]= yminb+ yd*ypart;
		if(xd<R.xparts-1) allparts[nr][2]= allparts[nr][0]+xpart;
		else allparts[nr][2]= xmaxb;
		if(yd<R.yparts-1) allparts[nr][3]= allparts[nr][1]+ypart;
		else allparts[nr][3]= ymaxb;

		if(allparts[nr][2]-allparts[nr][0]<=0) allparts[nr][0]= -1;
		if(allparts[nr][3]-allparts[nr][1]<=0) allparts[nr][0]= -1;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
short setpart(nr)	/* return 0 als geen goede part */
short nr;
{
	short xd,yd,xpart,ypart;

	if(allparts[nr][0]== -1) return 0;

	R.xstart= allparts[nr][0]-R.afmx;
	R.ystart= allparts[nr][1]-R.afmy;
	R.xend= allparts[nr][2]-R.afmx;
	R.yend= allparts[nr][3]-R.afmy;
	R.rectx= R.xend-R.xstart;
	R.recty= R.yend-R.ystart;
	return 1;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void addparttorect(nr, part)
short nr;
struct Part *part;
{
	ulong *rt, *rp;
	short y,heigth,len;

	/* de juiste offset in rectot */

	rt= R.rectot+ (allparts[nr][1]*R.rectx+ allparts[nr][0]);
	rp= part->rect;
	len= (allparts[nr][2]-allparts[nr][0]);
	heigth= (allparts[nr][3]-allparts[nr][1]);

	for(y=0;y<heigth;y++) {
		memcpy(rt, rp, 4*len);
		rt+=R.rectx;
		rp+= len;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void freeroteerscene()
{
	extern struct Branch *adroct[];
	struct Base *base;
	struct PerfSph *ps;
	struct ShadBuf *shb;
	struct CurveData *cu;
	struct QStrip *qs, *nextqs;
	ulong *ztile;
	long a,b,v;
	char *ctile;

	/* VRIJGEVEN */

	if(colbfdata) freeN(colbfdata);
	colbfdata= 0;

	for(a=0;a<G.totlamp;a++) {
		if(R.la[a]->shb) {
			shb= R.la[a]->shb;
			v= (shb->size*shb->size)/256;
			ztile= shb->zbuf;
			ctile= shb->cbuf;
			for(b=0;b<v;b++,ztile++,ctile++) {
				if(*ctile) freeN((void *) *ztile);
			}
			freeN(shb->zbuf);
			freeN(shb->cbuf);
			freeN(R.la[a]->shb);
		}
		freeN(R.la[a]);
	}
	for(a=0;a<G.totpoly;a++) {
		freeN(R.poly[a]->nurb);
		if(R.poly[a]->data) freeN(R.poly[a]->data);
		if(a<G.totpoly-1)   /* dit is vanwege extrude */
			if(R.poly[a]->data==R.poly[a+1]->data)
				R.poly[a+1]->data=0;
		freeN(R.poly[a]);
	}
	a=0;
	while(R.blove[a]) {
		freeN(R.blove[a]);
		R.blove[a]=0;
		a++;
	}
	a=0;
	while(R.blovl[a]) {
		freeN(R.blovl[a]);
		R.blovl[a]=0;
		a++;
	}
	a=0;
	while(R.bloha[a]) {
		freeN(R.bloha[a]);
		R.bloha[a]=0;
		a++;
	}
	if(R.blovlt) {
		freeN(R.blovlt);
		R.blovlt= 0;
	}
	a=0;
	while(adroct[a]) {
		freeN(adroct[a]);
		adroct[a]=0;
		a++;
	}

		/*	qs= R.qstrip.first;
		while(qs) {
			nextqs= qs->next;
			freeN(qs->verts);
			freeN(qs->faces);
			freeN(qs);
			qs= nextqs;
		}
		R.qstrip.first= R.qstrip.last= 0;
		*/

	base=G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base->soort==5) {
				cu= ( (struct ObData *)base->d )->cu;
				if(cu->orig) {
					freeN(cu->orig);
					cu->orig= 0;
				}
			}
		}
		base=base->next;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void addsky()		/* voegt met alpha sky toe */
{
	struct ImBuf *ibuf=0;
	float viewy, papery, ximf,yimf,dx,dy;
	ulong solid,col,*c,*rt1,*rs, *rectsky, *rectmaxsky;
	long file;
	short x,y,mul,fast=0,xim,yim,othersize=0, bufscript=1;
	char str[100],*rt;

	/* is sky zwart of solid ? */
	if((wrld.fs & 7)==0) {
		solid= ((ulong)wrld.horb<<16)+((ulong)wrld.horg<<8)+wrld.horr;
		fast= 1;
	}
	if(G.genlock==2) {
		solid= 0;
		fast= 1;
	}

	if(G.skybuf) {
		strcpy(str,G.skb);
		convertstringcode(str);	/* haalt "//" weg */

		ibuf=loadiffname(str,LI_rect);
		if(ibuf) {
			xim= ibuf->x; 
			yim= ibuf->y;
			if(R.rectx!=xim || R.recty!=yim) {
				othersize= 1;
				dx= xim; 
				dx/=R.rectx;
				dy= yim; 
				dy/=R.recty;
				ximf=yimf= 0.0;
			}
			rectsky= ibuf->rect;
			rectmaxsky= rectsky+ xim*yim;
		}
		else {
			error("Where is the skybuf?");
			return;
		}

	}
	rt= (char *)R.rectot;
	rt1= R.rectot;

	for(y=0;y<R.recty;y++) {
		if(G.genlock==1) {
			for(x=0;x<R.rectx;x++,rt+=4) {
				if(rt[0]!=255 && rt[0]!=0) {
					if(rt[0]!=255) premulalpha(rt);
				}
			}
		}
		else if(G.skybuf) {
			if(othersize) {
				while(yimf>0.0) {
					rectsky+= xim;
					yimf-=1.0;
				}
				if( ((long)rectsky)>=((long)rectmaxsky) ) rectsky= rectmaxsky-xim;
				
				ximf= 0.0;
				rs= rectsky;
				for(x=0;x<R.rectx;x++,rt+=4) {
					if(rt[0]!=255) {
						rs= rectsky+( (short)ximf );
						if(rt[0]==0) *((ulong *)rt)= *rs;
						else addalphaUnderGamma(rt,rs);
					}
					ximf+=dx;
				}
				yimf+=dy;
			} else {
				rs= rectsky;
				for(x=0;x<R.rectx;x++,rt+=4,rs++) {
					if(rt[0]!=255) {
						if(rt[0]==0) *((ulong *)rt)= *rs;
						else addalphaUnderGamma(rt,rs);
					}
				}
				rectsky+= xim;
			}
		}
		else if(fast) {
			for(x=0;x<R.rectx;x++,rt+=4) {
				if(rt[0]!=255) {
					if(rt[0]==0) *((ulong *)rt)= solid;
					else {
						addalphaUnderGamma(rt,&solid);
					}
				}
			}
		}
		else {
			papery= (y-R.afmy);
			viewy= R.ycor*(papery);
			papery/= R.afmy; 
			
			if(R.f & 32) viewy/=2.0;
			
			for(x=0;x<R.rectx;x++,rt+=4) {
				
				if(rt[0]<254) {
					R.view[0]= x-(R.afmx);
					if(wrld.fs & 8) {	/* paper */
						R.view[0]/= R.afmx; 
						R.view[1]= papery;
						R.view[2]= 0.0;
					}
					else {
						R.view[1]= viewy;
						R.view[2]= -R.near;
						Normalise(R.view);
					}
					
					sky(&col);
					
					if(rt[0]==0) *((ulong *)rt)= col;
					else addalphaUnderGamma(rt,&col);
				}
			}
		}

		if(G.afbreek) break;

		if( (y & 3)==3 && G.background!=1) {
			suspendtimer();
			lrectwrite(0,y-3,R.rectx-1,y,rt1);
			restarttimer();
			rt1+= 4*R.rectx;
		}
	}

	if(ibuf) freeImBuf(ibuf);

	if(G.bufscript) {
		dobufscript();
		if(G.background!=1) {
			suspendtimer();
			lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot);
			restarttimer();
		}
		return;
	}
}

void normalenrender(int startvert, int startvlak)
{
	struct VlakRen *vlr,*vlro;
	struct VertRen *ver,*adrve1,*adrve2,*adrve3;
	float n1[3],n2[3],n3[3],*adrco,*adrno,*tfl,f,fac,*f1,*temp;
	long *adrpu=0, *temp1, len, a;
	short pumanorm=0, x,y,z,*z1=0;

	if(G.totvlak==0 || G.totvert==0) return;
	if(startvert==G.totvert || startvlak==G.totvlak) return;
	
	adrco= (float *)callocN(12+12*(G.totvlak-startvlak), "normalen1");

	tfl= adrco;
	/* berekenen cos hoeken en puntmassa's */
	for(a=startvlak; a<G.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;
		n1[0]= adrve2->co[0]-adrve1->co[0];
		n1[1]= adrve2->co[1]-adrve1->co[1];
		n1[2]= adrve2->co[2]-adrve1->co[2];
		n2[0]= adrve3->co[0]-adrve2->co[0];
		n2[1]= adrve3->co[1]-adrve2->co[1];
		n2[2]= adrve3->co[2]-adrve2->co[2];
		n3[0]= adrve1->co[0]-adrve3->co[0];
		n3[1]= adrve1->co[1]-adrve3->co[1];
		n3[2]= adrve1->co[2]-adrve3->co[2];
		Normalise(n1);
		Normalise(n2);
		Normalise(n3);
		*(tfl++)= facos(-n1[0]*n3[0]-n1[1]*n3[1]-n1[2]*n3[2]);
		*(tfl++)= facos(-n1[0]*n2[0]-n1[1]*n2[1]-n1[2]*n2[2]);
		*(tfl++)= facos(-n2[0]*n3[0]-n2[1]*n3[1]-n2[2]*n3[2]);
	}
	/* alle puntnormalen leegmaken */
	for(a=startvert; a<G.totvert; a++) {
		ver= addvert(a);
		
		ver->n[0]=ver->n[1]=ver->n[2]= 0.0;
	}

	/* berekenen normalen en optellen bij puno's */
	tfl= adrco;
	for(a=startvlak; a<G.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;

		temp= adrve1->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n,temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];
		temp= adrve2->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n,temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];
		temp= adrve3->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n,temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];
	}
	
	/* normaliseren puntnormalen */
	for(a=startvert; a<G.totvert; a++) {
		ver= addvert(a);
		
		Normalise(ver->n);
	}
	
	/* puntnormaal omklap-vlaggen voor bij shade */
	for(a=startvlak; a<G.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;

		vlr->puno= 0;
		fac= vlr->n[0]*adrve1->n[0]+vlr->n[1]*adrve1->n[1]+vlr->n[2]*adrve1->n[2];
		if(fac<0) vlr->puno= 1;
		fac= vlr->n[0]*adrve2->n[0]+vlr->n[1]*adrve2->n[1]+vlr->n[2]*adrve2->n[2];
		if(fac<0) vlr->puno+= 2;
		fac= vlr->n[0]*adrve3->n[0]+vlr->n[1]*adrve3->n[1]+vlr->n[2]*adrve3->n[2];
		if(fac<0) vlr->puno+= 4;
	}

	freeN(adrco);
}



void make_render_halos(struct Base *base, float *mat, struct ColBlckF *cbf)
{
	extern float Tinc, Tin, Tr, Tg, Tb;
	struct ColBlck *cb;
	struct ObData *ob, *iob;
	struct Base *imatbase;
	struct VV *vv;
	struct VertOb *adrve, *keyvert;
	struct HaloRen *har;
	float bmat[4][4], imat[4][4], omat[4][4], vec[3], zn, hasize, *fp;
	int a, totvert;
	short *sp;
	
	R.f|=1;
	
	ob= (struct ObData *)base->d;
	cb= (struct ColBlck *)(ob+1);
	cb->hasize--;
	
	if(cb->map[0] & 16384) {
		imatbase= cb->base[0];
		if(imatbase) {
			parentbase(imatbase, bmat, omat);
			Mat4MulMat4(omat, bmat, G.viewmat);
			
			if(imatbase->soort==1) {
				iob= (struct ObData *)imatbase->d;
				Mat4MulFloat3(omat, iob->vv->ws);
			}

			Mat4Inv(imat, omat);
			Mat3CpyMat4(imatbase->imat, imat);

			imatbase->ivec[0]= -omat[3][0];
			imatbase->ivec[1]= -omat[3][1];
			imatbase->ivec[2]= -omat[3][2];	
		}
	}
	
	vv= ob->vv;
	adrve= (struct VertOb *)(vv+1);
	
	if(vv->key) {
		sp= (short *)(vv->key+1);
		keyvert= (struct VertOb *)(sp+2);
	}
	
	fp= ob->floatverts;
	
	totvert= vv->vert;
	if(ob->ef==4) totvert= build_schiphol(base, vv->vert);

	for(a=0; a<totvert; a++, adrve++, keyvert++) {

		hasize= 100*cb->hasize;

		if(fp) {
			VECCOPY(vec, fp);
			fp+= 3;
		}
		else {
			VECCOPY(vec, adrve->c);
		}
		Mat4MulVecfl(mat, vec);
		
		har= inithalo(vec, hasize);
		if(har) {

			har->alfa= cb->kref; 
			har->r= cb->r;
			har->g= cb->g; 
			har->b= cb->b;
			if(cb->tex[0]) {
				if(cb->map[0] & 4096) {
					;
				}
				else if(cb->map[0] & 16384) {
					vec[0]+= imatbase->ivec[0];
					vec[1]+= imatbase->ivec[1];
					vec[2]+= imatbase->ivec[2];
					Mat3MulVecfl(imatbase->imat, vec);
				}
				else {
					if(vv->key) {
						VECCOPY(vec, keyvert->c);
					}
					else {
						VECCOPY(vec, adrve->c);
					}
				}
				externtex(cb->tex[0], vec, &zn);
				if(cb->map[0] & 256) {
					zn= 1.0-Tinc;
					har->r= 255*(Tinc*Tr+ zn*cbf->r);
					har->g= 255*(Tinc*Tg+ zn*cbf->g);
					har->b= 255*(Tinc*Tb+ zn*cbf->b);
				}
				if(cb->map[0] & 16) {
					har->alfa= 255.0*Tin;
				}
			}
			har->col= cbf;
			har->type= cb->spec;
			
			if(cb->mode & 2) {	/* star */
				if(cb->kspec>2) har->starpoints= cb->kspec;
			}
			if(cb->mode & 1) har->tex= 1;
			
		}
	}
	cb->hasize++;
}

void roteerscene()
{
	extern int slurph_opt;	/* anim.c */
	struct Base *base,*duplibase();
	struct ObData *ob;
	struct PerfSph *ps;
	struct VV *vv;
	struct ColBlck *cb;
	struct ColBlckF *cbf;
	struct VlakOb *adrvl;
	struct VlakRen *vlr;
	struct VertOb *adrve, *keyvert;
	struct VertRen *ver;
	float bepaalphitheta(), dia;
	float fac, xn,yn,zn, mat[4][4], cmat[4][4], bmat[4][4], imat[4][4], vec[4];
	float hoco[4], *fp;
	long a,a1,vertofs,win,z,totvlako, totverto,totvlak, vv_vlak;
	short wire, x,doe_imat,do_puno=0,ok, testclip(), flipnorm;
	char *b,*b1,col,str[40];

	slurph_opt= 0;

	G.totvlak=G.totvert=G.totlamp=G.totpoly=G.tothalo=G.totvlakt=0;
	totcolbf= 0;
	/* qscount= 0x00100000; */
	/* R.qstrip.first= R.qstrip.last= 0; */


	base=G.firstbase;	/* eerst keys */
	while(base) {
		loadkeypostype(20, base, base);
		if(G.hie) {
			loadkeypostype(21, base, base);
			/* vertexparent? of dupli */
			if(base->f2 & 4) loadkeypostype(22, base, base);
			else if(base->dupli) loadkeypostype(22, base, base);
			else if(base->soort== -4) loadkeypostype(22, base, base);
		}
		base= base->next;
	}

	/* dupli's na keys, de duplibases mogen niet opnieuw keypos berekend! */
	if(G.f & 4) if(G.firstbase) G.lastbase->next= duplibase();

	base=G.firstbase;	/* vervolgens track testen en colblcks tellen */
	while(base) {
		if(base->t || base==view0.opar || base==view0.tpar) {
			parentbase(base->t,mat,bmat);
			/* parentbase(base,mat,bmat); */
		}
		if(base->soort & 1) {
			if(base->lay & view0.lay) {
				ob= (struct ObData *)base->d;
				totcolbf+= ob->c;
			}
		}
		base=base->next;
	}

	if(totcolbf) colbfdata= mallocN(totcolbf*sizeof(struct ColBlckF), "roteerscene");
	totcolbf= 0;

	initrendertexture();

	/* GEEN GL VIEWMAT MEER!!! : gebruik G.viewmat, G.winmat en G.persmat */

	R.far= 1000*view0.far;
	R.zcor= R.far/(R.far+R.d); 
	R.zcor/= 0xFFFFFF;  /* ook voor halo */

	if(G.holo) holoview();
	else if(panoramaview) panorview();

	if(view0.cambase) {
		parentbase(view0.cambase, bmat, cmat);
		
		Mat4Ortho(bmat);
		Mat4Invert(G.viewmat, bmat);
		
		if(view0.cambase->f2 & 64) G.viewmat[3][2]*= 100.0;
		
		bepaalphitheta();
	}
	else {
		dia= bepaalphitheta();
		i_polarview(dia, (180/PI)*view0.theta, (180/PI)*view0.phi, 0.0, G.viewmat);
		i_translate(-(float)view0.tar[0],-(float)view0.tar[1],-(float)view0.tar[2], G.viewmat);
	}

	setwindowclip(1,-1);
	/*  geen jit:(-1) */
	/* tevens viewinv in de imat van G.workbase */

	if(wrld.fs & 16) make_stars(0);

	/* WRLD	skyflag */
	wrld.fs &= ~4;
	if(wrld.stex[0]) wrld.fs+=4;

	/* init mist */
	mistfactorN(0);

	/* MAAK RENDERDATA */

	base=G.firstbase;
	while(base) {
		doe_imat=0;
		if(base->lay & view0.lay) {
			
			if(base->soort==2) initlamp(base);
			else if ELEM4(base->soort, 5, 7, 9, 11) {
				parentbase(base, bmat, cmat);
				ob= (struct ObData *)base->d;
				cb= (struct ColBlck *)(ob+1);
				cbf= initobject(base,cb,ob->c);
				
				if ELEM(base->soort, 5, 11) {
					if(ob->cu->key) loadkeypostype(22, base, base);
					fac= ob->cu->ws;
				}
				else {
					if(base->soort==9 && ob->ef==4) {
						makepolytext(base);
						extrudepoly(base);					
					}
					fac= ob->po->ws;
				}
				
				Mat4MulFloat3(bmat, fac);
				Mat4MulMat4(mat, bmat, G.viewmat);
				Mat4Invert(imat,mat); /* imat & normalen */
				
				Mat3CpyMat4(base->imat, imat);
				base->ivec[0]= -mat[3][0];
				base->ivec[1]= -mat[3][1];
				base->ivec[2]= -mat[3][2];
				
				if(base->skelnr) calc_deform(base);
				
				if(base->soort==5) initNurbren(ob, mat, cbf);
				else if(base->soort==11) {
					initpolyrenN(ob, mat, imat, cbf);
				}
				else {
					initpolyren(ob, mat, imat, cbf);
				}
			}
			else if(base->soort==3) {
				printf("PerfSphere is not supported anymore\n");
			}
			else if(base->soort==1) {
				parentbase(base, bmat, mat);

				ob=(struct ObData *)base->d;
				vv=ob->vv;

				if(vv->key) loadkeypostype(22, base, base);

				cb= (struct ColBlck *)(ob+1);
				cbf= initobject(base, cb, ob->c);
				
				Mat4MulMat4(mat, bmat, G.viewmat);
				Mat4MulFloat3(mat, vv->ws);
				
				Mat4Invert(imat, mat); /* imat & normalen */
				
				if(base->skelnr) calc_deform(base);

				Mat3CpyMat4(base->imat, imat);
				base->ivec[0]= -mat[3][0];
				base->ivec[1]= -mat[3][1];
				base->ivec[2]= -mat[3][2];

				adrve= (struct VertOb *)(vv+1);
				adrvl= (struct VlakOb *)(adrve+vv->vert);
				
				totvlako= G.totvlak;
				totverto= G.totvert;
				do_puno= 0;

				if(ob->ef && ob->ef<4) {
					bgneffect(vv);
					
					adrve= (struct VertOb *)(vv+1);	/* moet opnieuw ivm bgneffect */
					adrvl= (struct VlakOb *)(adrve+vv->vert);

					if(ob->ef==3) vertexTex(ob);
					else if(ob->ef==2) vlag(base);
					else if(ob->ef==1) golf(base);
					
					do_puno= 1;
				}
				else if(vv->key) do_puno= 1;
				else if(base->skelnr) do_puno= 1;
				
				if(cb->hasize) {
					make_render_halos(base, mat, cbf);
				}
				else {
					fp= ob->floatverts;
					
					for(a=0; a<vv->vert; a++, adrve++) {
						ver= addvert(G.totvert++);
						if(fp) {
							VECCOPY(ver->co, fp);
							fp+= 3;
						}
						else {
							VECCOPY(ver->co, adrve->c);
						}
						Mat4MulVecfl(mat, ver->co);
						xn= adrve->n[0]; 
						yn= adrve->n[1]; 
						zn= adrve->n[2];
						if(do_puno==0) {
							ver->n[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
							ver->n[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
							ver->n[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
							Normalise(ver->n);
						}
						ver->v= adrve;
					}
					if(vv->key) {   /* de juiste lokale textu coordinaat */
						a= G.totvert- vv->vert;
						b= (char *)vv->key;
						adrve= (struct VertOb *)(b+sizeof(struct Key)+4);
						for(;a<G.totvert;a++,adrve++) {
							ver=addvert(a);
							ver->v= adrve;
						}
					}
					
					flipnorm= -1;
					/* Testen of er een flip in de matrix zit: dan vlaknormaal ook flippen */
					
					/* vlakken in volgorde colblocks */
					vertofs= G.totvert- vv->vert;
					for(a1=0; a1<ob->c; a1++) {
						adrve= (struct VertOb *)(vv+1);
						adrvl= (struct VlakOb *)(adrve+vv->vert);
						
						wire= (cb->mode1 & 1024);
						
						/* testen op 100% transparant */
						ok= 1;
						if(cb->tra==255) {
							ok= 0;
							/* texture op transp? */
							for(a=0; a<4; a++) {
								if(cb->tex[a]) {
									if( (cb->map[2*a] & 2) || (cb->map[2*a+1] & 2) ) ok= 1;
								}
							}
						}
						
						if(ok) {
						
							if(ob->ef==4) vv_vlak= build_schiphol(base, vv->vlak);
							else vv_vlak= vv->vlak;	
						
							for(a=0; a<vv_vlak; a++, adrvl++) {
								if( (adrvl->ec & 31)==a1 ) {
									if(adrvl->v1!=adrvl->v2 || wire) {
										
										vlr= addvlak(G.totvlak++);
										vlr->v1= addvert(vertofs+adrvl->v1);
										vlr->v2= addvert(vertofs+adrvl->v2);
										vlr->v3= addvert(vertofs+adrvl->v3);
										
										vlr->len= CalcNormFloat(vlr->v1->co,vlr->v2->co,vlr->v3->co,vlr->n);
										
										vlr->vl= adrvl;
										/* CalcN. is net zo snel! als roteren */

										if(wire==0 && vlr->len==0) G.totvlak--;
										else {
											if(flipnorm== -1) {	/* per base 1 x testen */
											
												xn= adrvl->n[0]; 
												yn= adrvl->n[1]; 
												zn= adrvl->n[2];
												vec[0]= imat[0][0]*xn+ imat[0][1]*yn+ imat[0][2]*zn;
												vec[1]= imat[1][0]*xn+ imat[1][1]*yn+ imat[1][2]*zn;
												vec[2]= imat[2][0]*xn+ imat[2][1]*yn+ imat[2][2]*zn;

												xn= vec[0]*vlr->n[0]+vec[1]*vlr->n[1]+vec[2]*vlr->n[2];
												if(xn<0.0) flipnorm= 1;
												else flipnorm= 0;
											}
											if(flipnorm) {
												vlr->n[0]= -vlr->n[0];
												vlr->n[1]= -vlr->n[1];
												vlr->n[2]= -vlr->n[2];
											}
										}
										vlr->col= cbf;
										vlr->puno= adrvl->f;
									}
								}
							}
						}
						cb++;
						cbf++;
					}
				}
				if(ob->ef && ob->ef<4) {
					endeffect(vv);
				}
				
				if(do_puno) normalenrender(totverto, totvlako);
				
			}
			else doe_imat=1;
		}
		else doe_imat= 1;
		if(doe_imat) {
			parentbase(base, bmat, mat);
			Mat4MulMat4(mat, bmat, G.viewmat);
			
			if(base->soort==1) {
				ob=(struct ObData *)base->d;
				Mat4MulFloat3(mat, ob->vv->ws);
			}
			else if ELEM(base->soort, 5, 11) {
				ob=(struct ObData *)base->d;
				Mat4MulFloat3(mat, ob->cu->ws);
			}

			Mat4Inv(imat, mat);
			Mat3CpyMat4(base->imat, imat);

			base->ivec[0]= -mat[3][0];
			base->ivec[1]= -mat[3][1];
			base->ivec[2]= -mat[3][2];
		}
		base= base->next;
		if(G.afbreek) break;
	}
	
	slurph_opt= 1;
	
	if(G.lastbase) G.lastbase->next=0;  /* tegen dupli's */
	if(G.afbreek) return;
	
	if(G.totlamp==0) defaultlamp();
	
	/* KLAP NORMAAL EN SNIJ PROJECTIE */
	for(a1=0;a1<G.totvlak;a1++) {
		if((a1 & 255)==0) vlr= R.blovl[a1>>8];
		else vlr++;
		
		vec[0]= (float)vlr->v1->co[0];
		vec[1]= (float)vlr->v1->co[1];
		vec[2]= (float)vlr->v1->co[2];

		if( (vec[0]*vlr->n[0] +vec[1]*vlr->n[1] +vec[2]*vlr->n[2])<0.0 ) {
			vlr->puno= ~(vlr->puno);
			vlr->n[0]= -vlr->n[0];
			vlr->n[1]= -vlr->n[1];
			vlr->n[2]= -vlr->n[2];
		}
	
		xn= fabs(vlr->n[0]);
		yn= fabs(vlr->n[1]);
		zn= fabs(vlr->n[2]);
		if(zn>=xn && zn>=yn) vlr->snproj=0;
		else if(yn>=xn && yn>=zn) vlr->snproj=1;
		else vlr->snproj=2;

		if(G.afbreek) return;
	}

	/* OCTREE ? */
	if(R.f & 4) makeoctree();	/* niet bij gour */
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void render()  /* hierbinnen de PART en FIELD lussen */
{
	struct Part *part;
	float mat[4][4];
	ulong *rt, *rt1, *rt2;
	long file1,file2,len,a1;
	short a,fields,fi,parts;  /* pa is globaal ivm print */


	/* FIELDLUS */
	fields= 1;
	R.f= R.f1;
	parts= R.xparts*R.yparts;
	if(R.f & 32) {
		fields= 2;
		R.rectf1= R.rectf2= 0;	/* fieldrecten */
		R.ysch/=2; 
		R.afmy/=2;
		R.yasp*=2;
		R.ycor= R.yasp;
		R.ycor= R.ycor/R.xasp;
		R.f&= ~64;
	}
	
	for(fi=0;fi<fields;fi++) {

		/* INIT */
		srand( 2*G.cfra+fi);
		
		R.svlako= -1;
		R.f= R.f1;
		if(fi==1) R.f|=64;
		if(R.f & 8) R.f&= ~6;	/* geen shad en mir bij gouraud */

		/* SCRIPT */
		if(G.script) doscript();

		/* WINDOW */
		R.rectx= R.xsch; 
		R.recty= R.ysch;
		R.xstart= -R.afmx; 
		R.ystart= -R.afmy;
		R.xend= R.xstart+R.xsch-1;
		R.yend= R.ystart+R.ysch-1;

		initparts(); /* altijd doen ivm border */
		setpart(0);
		openwindow();
		if(G.winar[9]) {
			czclear(0,0x7FFFFE);
		}

		/* ROTEERSCENE */
		
		if(G.holo==2) projectholo(0, 0);

		if(R.rectot) freeN(R.rectot); R.rectot= 0;
		if(R.rectz) freeN(R.rectz); R.rectz= 0;

		roteerscene();

		tekenoverdraw(1);

		/* SCHADUWBUFFER */
		for(a=0;a<G.totlamp;a++) {
			if(G.afbreek) break;
			if(R.la[a]->shb) makeshadowbuf(R.la[a]->shb);
		}
				
		/* PARTS */
		R.parts.first= R.parts.last= 0;
		for(pa=0;pa<parts;pa++) {
			if(G.afbreek) break;
			if(pa) {	/* want pa==0 is al gedaan */
				if(setpart(pa)==0) break;
				openwindow();
				setwindowclip(0,-1);
			}

			/* HOMOGENE COORDINATEN EN ZBUF EN CLIP OPT (per part) */
			setzbufvlaggen(projectverto);
			if(G.afbreek) break;

			/* ZBUFFER & SHADE */
			R.rectot= (ulong *)callocN(4*R.rectx*R.recty, "rectot");
			R.rectz=  (ulong *)mallocN(4*R.rectx*R.recty, "rectz");

			printrenderinfo(-1);

			if( G.genlock==2 ) accumkey();
			else if(R.f & (8 | 2048)) accumgour();
			else if(G.ali) zbufshadeDA();
			else zbufshade();

			if(G.afbreek) break;

			/* PART OF BORDER AFHANDELEN */
			if(parts>1 || G.border) {
				part= callocN(sizeof(struct Part), "part");
				addtail(&R.parts, part);
				part->rect= R.rectot;
				R.rectot= 0;
				
				freeN(R.rectz);
				R.rectz= 0;
			}
		}

		/* EINDE */
		freeroteerscene();

		/* PARTS SAMENVOEGEN OF BORDER INVOEGEN */

		R.rectx= R.xsch; 
		R.recty= R.ysch;

		if(parts>1 || G.border) {
			if(R.rectot) freeN(R.rectot);
			if(G.border) R.rectot=(ulong *)callocN(4*R.rectx*R.recty, "rectot");
			else R.rectot=(ulong *)mallocN(4*R.rectx*R.recty, "rectot");

			if(G.afbreek==0) {
				part= R.parts.first;
				for(pa=0;pa<parts;pa++) {
					if(allparts[pa][0]== -1) break;
					if(part==0) break;
					
					addparttorect(pa, part);
					part= part->next;
				}
			}
			part= R.parts.first;
			while(part) {
				freeN(part->rect);
				part= part->next;
			}
			freelistN(&R.parts);
		}

		if(G.afbreek) break;

		/* FIELD AFHANDELEN */
		if(R.f & 32) {
			if(R.f & 64) R.rectf2= R.rectot;
			else R.rectf1= R.rectot;
			R.rectot= 0;
		}
	}

	/* FIELDS SAMENVOEGEN */
	if(R.f & 32) {
		R.ysch*=2; 
		R.afmy*=2;
		R.rectx= R.xsch; 
		R.recty= R.ysch;
		R.yasp/=2;

		if(R.rectot) freeN(R.rectot);	/* komt voor bij afbreek */
		R.rectot=(ulong *)mallocN(4*R.xsch*R.ysch, "rectot");
		if(G.afbreek==0) {
			rt= R.rectot;
			rt1= R.rectf1;
			rt2= R.rectf2;
			len= 4*R.xsch;
			for(a=0;a<R.ysch;a+=2) {
				memcpy(rt, rt1, len);
				rt+= R.xsch;
				rt1+= R.xsch;
				memcpy(rt, rt2, len);
				rt+= R.xsch;
				rt2+= R.xsch;
			}
		}
	}

	R.rectx= R.xsch; 
	R.recty= R.ysch;
	openwindow();

	/* ALPHA INVULLEN ? */
	if(G.afbreek==0 && G.genlock<2) {
		setwindowclip(1,-1);	/* als fields: opnieuw grvec en matrix */
		addsky();
		if(G.border==0) {
			if(G.ali==0 && (R.f & 1024)) {
				edge_enhance(0);
				if(G.winar[9]) lrectwrite(0,0,R.xsch-1,R.ysch-1,R.rectot);
			}
			if(wrld.dofi) {
				dof();	
				if(G.winar[9]) lrectwrite(0,0,R.xsch-1,R.ysch-1,R.rectot);
			}
		}
	}
	else if(G.winar[9] && R.rectot) lrectwrite(0,0,R.xsch-1,R.ysch-1,R.rectot);

	/* VRIJGEVEN */
	if(R.rectz) freeN(R.rectz);
	R.rectz= 0;
	if(R.rectf1) freeN(R.rectf1);
	R.rectf1= 0;
	if(R.rectf2) freeN(R.rectf2);
	R.rectf2= 0;

}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void printmemblocks()
{
	extern long mem_in_use;
	extern short totblock;
	/* struct mallinfo mal; */
	long maltot, malNtot;
	
	/* mal= mallinfo(); */
	/* printf("%d total space in arena\n", mal.arena); */
	/* printf("%d number of ordinary blocks\n", mal.ordblks); */
	/* printf("%d number of small blocks\n", mal.smblks); */
	/* printf("%d space in holding block headers\n", mal.hblkhd); */
	/* printf("%d number of holding blocks\n", mal.hblks); */
	/* printf("%d space in small blocks in use\n", mal.usmblks); */
	/* printf("%d space in free small blocks\n", mal.fsmblks); */
	/* printf("%d space in ordinary blocks in use\n", mal.uordblks); */
	/* printf("%d space in free ordinary blocks\n", mal.fordblks); */
	/* printf("%d space penalty if keep option is used\n", mal.keepcost); */
	/* maltot= ( mal.usmblks+mal.uordblks); */
	/* malNtot= mem_in_use+(sizeof(struct MemHead)+sizeof(struct MemTail))*totblock; */
	/* printf("total malloc: %d  mallocN: %d  diff: %d\n", maltot, malNtot, maltot-malNtot); */
}

void initrender(anim)
short anim;
{
	extern ushort usegamtab;
	struct tms timebuf1,timebuf2;
	struct stat fileinfo;
	ulong max;
	ulong *fieldrect,*rect;
	float fac, z, phi, theta, lens;
	long file,tijd,ticks,len;
	short a,xsch,ysch,cfrao;
	char str[256],str1[64],sbname[100];
	char this[256],prev[256],dlta[256];

	/* printmemblocks(); */

	/* MAG ER WEL WORDEN GERENDERD */
	if(R.xparts*R.yparts>16) {
		error("No more than 16 parts");
		G.afbreek= 1;
		return;
	}
	if(G.background==1) {
		if(R.f & 8) {
			printf("ERROR: need window for Gouraud!\n");
			sluit(0);
		}
	}

	if(!G.background) G.afbreek = 0;

	if(G.background!=1) {
		bgntimer();
	}

	/* ONTHOUDEN */
	xsch= R.xsch; 
	ysch= R.ysch;
	cfrao= G.cfra;
	phi= view0.phi; 
	theta= view0.theta;

	/* INIT 1 */

	if(G.ebase) {
		if(G.ebase->soort==1) load_ebasedata();
		else if ELEM(G.ebase->soort, 5, 11) load_ebaseNurb();
	}
	setcursorN(2);

	R.la= (struct LampRen **)mallocN(MAXLAMP*4,"renderlamparray");
	R.poly= (struct PolyRen **)mallocN(MAXPOLY*4,"renderpolyarray");
	R.blovlt= 0;

	usegamtab= 0; /* zie hieronder */
	
	if(G.ali) {
		if(G.osa>16) G.osa= 16;
		blkclr(jit, 64*2*4);
		initjit(jit,G.osa);
		init_filt_mask();
		
		usegamtab= 1;		/* wordt af en toe tijdelijk op nul gezet, o.a. in transp zbuf */
	}

	/* WINDOW */
	R.xsch-= (R.size*R.xsch)/4;
	R.ysch-= (R.size*R.ysch)/4;
	R.afmx= R.xsch/2;
	R.afmy= R.ysch/2;
	if(R.afmx<1 || R.afmy<1) {
		error("Image too small");
		return;
	}
	R.ycor= R.yasp;
	R.ycor= R.ycor/R.xasp;
	
	lens= view0.lens;
	
	if(view0.cambase && view0.cambase->soort==2) {
		struct LaData *la;
		
		la= (struct LaData *)view0.cambase->d;
		fac= fcos( PI*((float)(256- la->spsi))/512.0 );
		
		phi= facos(fac);
		lens= 16.0*fac/fsin(phi);
	}
	
	
	if(R.xasp*R.afmx>=R.yasp*R.afmy) {
		R.d= (R.afmx*view0.lens)/16;
		R.near= (R.afmx*view0.lens)/16.0;   /* R.near moet R.d gaan vervangen */
	}
	else {
		R.d= R.ycor*(R.afmy*view0.lens)/16;
		R.near= R.ycor*(R.afmy*view0.lens)/16.0;
	}
	R.pixsize= ((float)view0.near)/R.near;

	if(view0.cambase && (view0.cambase->f2 & 64)) {
		R.near*= 100.0;
		/* R.far niet doen */
	}

	/* SKYBUF */
	/* hier stond een diskfree beveiliging */

	if(G.skybuf) {				/* alleen testen */
		strcpy(sbname,G.skb);
		convertstringcode(sbname);	/* haalt "//" weg */

		file= open(sbname,O_RDONLY);
		if(file== -1) {
			error("No skybuf there!");
			G.afbreek= 1;
			goto eindeinitrender;
		}
		close(file);
	}

	/* START ANIMLUS  */
	if(anim) G.cfra=G.sfra;
animlus:

	tijd=clock();
	ticks= times(&timebuf1);

	/* if(diskfree(TTEMPDIR)<max) { */
		/*		error("Not enough diskspace");
		G.afbreek= 1;
		goto eindeinitrender;
		*/
	/* } */

	/* SCRIPT */
	/*if(G.script) doscript();*/

	/* RENDER  */
	render(0); /* keert terug met complete rect xsch-ysch */

	/* SCHRIJF/DISPLAY/RECORD PLAATJE */
	/* ANIM:  1=none 5=tape 6=hdisk */

	if(anim && R.anim>4 && G.afbreek==0) {

		makepicstring(this,G.cfra);
		schrijfplaatje(this);
		if(G.afbreek==0) {	/* wordt ook door schrijfplaatje gezet */

			if ( G.disc && R.imtype<=2 ) {

				makepicstring(prev,G.cfra-1);
				if ((G.cfra % 10) != (G.sfra % 10)){
					strcpy(dlta,this);
					strcat(dlta,".dlta");
					if (makedelta(prev,this,dlta)!=TRUE) {
						G.afbreek= TRUE;
					}
				} else{
					strcpy(dlta,this);
					strcat(dlta,".full");
					remove(dlta);
					if (link(this, dlta)) {
						perror(0);
						G.afbreek= TRUE;
					}
/*					if (copy(dlta,this)) {
						G.afbreek= TRUE;
					}
*/				}
				if(G.afbreek==0) {
					if (G.cfra  != G.sfra ){
						if (remove(prev)) {
							printf("Couldn't delete %s\n",prev);
							G.afbreek=TRUE;
						}
					}
					if (G.cfra  == G.efra ){
						if (remove(this)) {
							printf("Couldn't delete %s\n",this);
							G.afbreek=TRUE;
						}
					}
					strcpy(this,dlta);
				}
			}

			if(R.anim==5) {
				if (appendtape(this)!=TRUE) G.afbreek= TRUE;
				else if(remove(this)){
					printf("Couldn't delete %s\n",this);
					G.afbreek=TRUE;
				}
				else printf("Recorded: %s",this);
			}
			else printf("Saved: %s",this);
		}
	}

	/* TIJD PRINTEN */
	G.time= times(&timebuf2)-ticks;	/* echte tijd */
	G.cputime= timebuf2.tms_utime+timebuf2.tms_stime-timebuf1.tms_utime-timebuf1.tms_stime;

	if(anim && R.anim>4 && G.afbreek==0) {
		timestr(G.cputime,this);
		if(R.f & 512) if(R.f & 32) printf(" NO FIELD ");
		printf(" Time: %s (%.2f)\n",this,((float)(G.time-G.cputime))/100);
	}
	if(G.winar[0]) {
		tekenoverdraw(1);
		winset(G.winar[9]);
	}
	/* GOTO ANIMLUS ? */
	if (anim && G.afbreek==0){
		
		if(G.cfra<G.efra) {
			G.cfra++;
			goto animlus;
		}
		else if(R.anim==0) {
			G.cfra=G.sfra;
			goto animlus;
		}
	}

	/* DEFINIEF VRIJGEVEN */
eindeinitrender:
	freeN(R.la); 
	freeN(R.poly);

	/* VARIABELEN WEER GOED ZETTEN */
	if(panoramaview && anim) maakpanorama();

	G.cfra=cfrao;
	R.xsch=xsch; 
	R.ysch=ysch;
	R.osatex= 0;
	R.f= R.f1;
	view0.phi=phi; 
	view0.theta=theta;
	if(G.winar[9]) zbuffer(0);
	
	if(G.ebase) countall();	/* anders is G.totvert verkeerd */
	if(G.winar[0]) {
		setmatriceswork();
		setcursorN(0);
	}

	if(G.background!=1) {
		endtimer();
	}
	if (!G.background) G.afbreek= 0;
	
	/* printmemblocks(); */
}

