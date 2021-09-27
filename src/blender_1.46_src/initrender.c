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




/*  initrender.c        RENDER
 * 
 *  april 95
 * 
 */

#include "blender.h"
#include "edit.h"
#include "render.h"
#include "effect.h"
#include <sys/time.h>	/* struct timeval */
#include <sys/resource.h>	/* struct rusage */


				/* uit zbuf.c */
extern void projectverto();
extern void projectvert();

					/* uit render.c */
extern float fmask[256], centLut[16];
extern ushort *mask1[9], *mask2[9], *igamtab1, *igamtab2, *gamtab;
extern char cmask[256], *centmask;
extern ushort usegamtab;

		/* hieruit */
void setpanorot(int part);

Material defmaterial;
short pa; /* pa is globaal part ivm print */
short allparts[65][4];
float jit[64][2], panophi=0.0;
int qscount;
ImBuf * izbuf = 0;

/* ********************* *********************** */


void init_def_material()
{
	Material *ma;
	
	ma= &defmaterial;
	
	ma->r= ma->g= ma->b= ma->ref= 0.8;
	ma->specr= ma->specg= ma->specb= 1.0;
	ma->mirr= ma->mirg= ma->mirb= 1.0;
	ma->amb= 0.05;
	ma->alpha= 1.0;
	ma->spec= 0.5;
	ma->har= 50;
	
	ma->mode= MA_TRACEBLE+MA_SHADOW;
	
	init_render_material(ma);
}

void init_render_data()
{
	bzero(&R, sizeof(Render));
	bzero(&O, sizeof(Osa));
	O.dxwin[0]= 1.0;
	O.dywin[1]= 1.0;
	R.displaymode= R_DISPLAYWIN;

	R.blove= (VertRen **)callocN(4*(MAXVERT>>8),"Blove");
	R.blovl= (VlakRen **)callocN(4*(MAXVLAK>>8),"Blovl");
	R.bloha= (HaloRen **)callocN(4*(MAXVERT>>8),"Bloha");
	R.la= (LampRen **)mallocN(MAXLAMP*4,"renderlamparray");
	
	init_def_material();
}

void free_render_data()
{
	freeN(R.blove);
	R.blove= 0;
	freeN(R.blovl);
	R.blovl= 0;
	freeN(R.bloha);
	R.bloha= 0;
	freeN(R.la);
	R.la= 0;
	if(R.rectot) freeN(R.rectot);
	if(R.rectspare) freeN(R.rectspare);
	R.rectot= 0;
	R.rectspare= 0;
	
	end_render_material(&defmaterial);
}


/* ****************** JITTER *********************** */

void jitsort(float * jit2, int num)
{
	float small, large, x, y;
	int * sortbuf, i, j;

	sortbuf = mallocN(num * sizeof(int), "jitsort");
	
	small = 0.0;
	
	for (i = 0; i < num; i++) {
		large = 1.0;
		for (j = 0; j < num; j++) {
			if (jit2[2 * j] > small && jit2[2 * j] < large) {
				large = jit2[2 * j];
				sortbuf[i] = j;
			}
		}
		small = large;
	}
	
	y = 0.0;
	
	for (i = 0; i < num; i++) {
		x = jit2[2 * sortbuf[i]];
		x += (y - x) / 10;
		jit2[2 * sortbuf[i]] = x;
		
		y += 1.0 / (num - 1.0);
	}
	
	freeN(sortbuf);
}

void jitterate1(float *jit1, float *jit2, int num, float rad1)
{
	int i , j , k;
	float vecx, vecy, dvecx, dvecy, x, y, len;

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

/*	jitsort(jit2, num);
	jitsort(jit2 + 1,  num);
*/
	memcpy(jit1,jit2,2 * num * sizeof(float));
}

void jitterate2(float *jit1, float *jit2, int num, float rad2)
{
	int i , j , k;
	float vecx, vecy, dvecx, dvecy, x, y, len;

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

void initjit(float *jit, int num)
{
	float *jit2, x, y, rad1, rad2, rad3;
	int col, i;

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

void init_render_jit(int nr)
{
	static int lastjit= 0;
	int a;
	
	if(lastjit==nr) return;
	
	bzero(jit, 64*2*4);
	initjit(jit[0], nr);
	
	lastjit= nr;
	
}
/* ****************** GAMMA, MASKERS en LUTS **************** */

float  calc_weight(float *weight, int i, int j)
{
	float x, y, dist, totw= 0.0;
	int a;

	for(a=0; a<R.osa; a++) {
		x= jit[a][0]-0.5+ i;
		y= jit[a][1]-0.5+ j;
		dist= fsqrt(x*x+y*y);

		weight[a]= 0.0;

		if(R.r.mode & R_GAUSS) {
			if(dist<1.5) {
				x = dist*1.5;
				weight[a]= (1.0/exp(x*x) - 1.0/exp(1.5*1.5*1.5*1.5));
			}
		}
		else {
			if(i==0 && j==0) weight[a]= 1.0;
		}

		totw+= weight[a];

	}
	return totw;
}

void init_filt_mask()
{
	static int firsttime=1;
	static float lastgamma= 0.0;
	float gamma, igamma;
	float weight[32], totw, val, *fpx1, *fpx2, *fpy1, *fpy2;
	int i, j, a;
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
	
	if(R.r.alphamode==R_ALPHAKEY) gamma= 1.0;	/* ??? */

	if(R.r.mode & R_GAMMA) gamma= 2.0;
	else gamma= 1.0;
	igamma= 1.0/gamma;
	
	if(gamma!= lastgamma) {
		lastgamma= gamma;

		/* gamtab: in short, uit short */
		for(a=0; a<65536; a++) {
			val= a;
			val/= 65535.0;
			
			if(gamma==2.0) val= fsqrt(val);
			else if(gamma!=1.0) val= powf(val, igamma);
			
			gamtab[a]= (65535.99*val);
		}
		/* inv gamtab1 : in byte, uit short */
		for(a=1; a<=256; a++) {
			if(gamma==2.0) igamtab1[a-1]= a*a-1;
			else if(gamma==1.0) igamtab1[a-1]= 256*a-1;
			else {
				val= a/256.0;
				igamtab1[a-1]= (65535.0*powf(val, gamma)) -1 ;
			}
		}
		
		/* inv gamtab2 : in short, uit short */
		for(a=0; a<65536; a++) {
			val= a;
			val/= 65535.0;
			if(gamma==2.0) val= val*val;
			else val= powf(val, gamma);
			
			igamtab2[a]= 65535.0*val;
		}
	}
	
	if(firsttime) {
		firsttime= 0;
		return;
	}

	val= 1.0/((float)R.osa);
	for(a=0; a<256; a++) {
		fmask[a]= ((float)cmask[a])*val;
	}

	for(a=0; a<9;a++) {
		bzero(mask1[a], 256*2);
		bzero(mask2[a], 256*2);
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

			bzero(weight, 32*2);
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

	for(a= (1<<R.osa)-1; a>0; a--) {
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

}

void free_filt_mask()
{
	int a;

	for(a=0; a<9; a++) {
		freeN(mask1[a]);
		freeN(mask2[a]);
	}
	freeN(gamtab);
	freeN(igamtab1);
	freeN(igamtab2);

	freeN(centmask);
}

/* ************** INFO FILES ******************* */


void make_info_file_string(char *string, int frame)
{
	int len, i;
	char num[8];
	
	if (string==0) return;

	strcpy(string, G.scene->r.pic);
	convertstringcode(string);

	strcat(string, "info/");
	
	/* dir testen, desnoods maken */
	if (exist(string) == 0) {
		recurdir(string);
	}
		
	len= strlen(string);

	i= 4-sprintf(num, "%d", frame);
	for(; i>0; i--){
		string[len]= '0';
		len++;
	}
	string[len]= 0;
	strcat(string, num);
}

int panotestclip(float *v)
{
	/* gebruiken voor halo's en info's */
	float abs4;
	short c=0;

	if((R.r.mode & R_PANORAMA)==0) return testclip(v);
	
	abs4= fabs(v[3]);

	if(v[2]< 0.0) c=16;		/* hier stond vroeger " if(v[2]<-abs4) ", zie clippz() */
	else if(v[2]> abs4) c+= 32;

	if( v[1]>abs4) c+=4;
	else if( v[1]< -abs4) c+=8;

	abs4*= R.r.xparts;
	if( v[0]>abs4) c+=2;
	else if( v[0]< -abs4) c+=1;

	return c;
}


int info_calc_schermco(float *vec, float *sco)
{
	float si, co, v1[3], hoco[4], xpartsfac;
	int pa= 0;
	
	xpartsfac= 0.5/(float)R.r.xparts;
	sco[0]= sco[1]= sco[2]= 0.0;
	
	/* eerst de normale versie */
	projectverto(vec, hoco);

	hoco[3]*= 2.0;	/* clipgrens iets ruimer */

	if(	testclip(hoco)==0) {
		hoco[3]/= 2.0;
		sco[0]= 0.5 + xpartsfac*(hoco[0]/ hoco[3]);
		sco[1]= 0.5 - 0.5*(hoco[1]/ hoco[3]);	/* antieke schermco's */
		sco[2]= (1.0+hoco[2]/ hoco[3]);
	}

	/* elk part testen */
	if(R.r.mode & R_PANORAMA) {

		for(pa=0; pa<R.r.xparts; pa++) {
			setpanorot(pa);
	
			si= fsin(panophi);
			co= fcos(panophi);

			v1[0]= co*vec[0] + si*vec[2];
			v1[1]= vec[1];
			v1[2]= -si*vec[0] + co*vec[2];
			
			projectverto(v1, hoco);
			hoco[3]*= 2.0;	/* clipgrens iets ruimer */
			
			if(	testclip(hoco)==0) {
				
				hoco[3]/= 2.0;	/* clipgrens iets ruimer */
				
				sco[0]= 0.5 + xpartsfac*(hoco[0]/ hoco[3]) + pa*2.0*xpartsfac - (R.r.xparts-1)*xpartsfac;
				sco[1]= 0.5 - 0.5*(hoco[1]/ hoco[3]);	/* antieke schermco's */
				sco[2]= (1.0+hoco[2]/ hoco[3]);
			}
		}
	}
	
	return 1;
}

void info_calc_drot(Object *ob, float *co)
{
	float vec1[3], vec2[3], len, co1, co2;

	*co= 0.0;

	VECCOPY(vec1, ob->obmat[2]);

	pushdata(ob, sizeof(Object));
	CFRA--;
	do_ob_ipo(ob);
	where_is_object(ob);
	VECCOPY(vec2, ob->obmat[2]);
	poplast(ob, sizeof(Object));

	CFRA++;
	
	len= Normalise(vec1);
	if(len < 0.001) return;
	len= Normalise(vec2);
	if(len < 0.001) return;
	
	/* co= INPR(vec1, vec2); */
	
	co1= fatan2(vec1[1], vec1[0]);
	co2= fatan2(vec2[1], vec2[0]);
	
	*co= -co1+co2;
	if(*co > M_PI) *co-= 2.0*M_PI;
	else if(*co < -M_PI) *co+= 2.0*M_PI;

}

void info_file(Object *ob)
{
	/* als ob==0: file sluiten
	 * als file==0: dir testen en file openen
	 * van deze functie staat kopie in writefile.c (Mdec versie)
	 */
	static FILE *fp=0;
	float mat[4][4], vec1[3], co;
	char name[100];
	
	if(ob==0) {
		if(fp) fclose(fp);
		fp= 0;
		return;
	}

	if(fp==0) {
		make_info_file_string(name, CFRA);
		fp= fopen(name, "w");
		
		if(fp) {
			fprintf(fp, "INFO\n");
		}
	}
	if(fp==0) return;
	
	if(ob->infostart > CFRA) return;
	if(ob->infoend!=0 && ob->infoend < CFRA) return;

	fprintf(fp, "NAME\n%s\n", ob->id.name+2);
	
	if(ob->infoflag & OB_INFO_LOC) fprintf(fp, "LOC\n%f %f %f\n", ob->obmat[3][0], ob->obmat[3][1], ob->obmat[3][2]);
	
	if(ob->infoflag & OB_INFO_SCO) {
		
		Mat4MulMat4(mat, ob->obmat, R.viewmat);
		info_calc_schermco(mat[3], vec1);
		fprintf(fp, "SCO\n%f %f %f\n", vec1[0], vec1[1], vec1[2]);
	}
	
	if(ob->infoflag & OB_INFO_DROT) {
		
		info_calc_drot(ob, &co);
		
		fprintf(fp, "DROT\n%f\n", co);
	}
}



/* ************** END INFO FILES ******************* */


/* ~~~~~~~~~~~~~~~~~ADD~~~~~~~~~~~~~~~~~~~~~ */

VertRen *addvert(int nr)
{
	VertRen *v;
	int a;

	if(nr<0 || nr>MAXVERT ) {
		printf("error in addvert: %d\n",nr);
		return R.blove[0];
	}
	a= nr>>8;
	v= R.blove[a];
	if(v==0) {
		v= (VertRen *)callocN(256*sizeof(VertRen),"addvert");
		R.blove[a]= v;
	}
	v+= (nr & 255);
	return v;
}

HaloRen *addhalo(int nr)
{
	HaloRen *h;
	int a;

	if(nr<0 || nr>MAXVERT ) {
		printf("error in addhalo: %d\n",nr);
		return R.bloha[0];
	}
	a= nr>>8;
	h= R.bloha[a];
	if(h==0) {
		h= (HaloRen *)callocN(256*sizeof(HaloRen),"addhalo");
		R.bloha[a]= h;
	}
	h+= (nr & 255);
	return h;
}

VlakRen *addvlak(int nr)
{
	VlakRen *v;
	int a;

	if(nr<0 || nr>MAXVLAK ) {
		printf("error in addvlak: %d\n",nr);
		return R.blovl[0];
	}
	a= nr>>8;
	v= R.blovl[a];
	if(v==0) {
		v= (VlakRen *)callocN(256*sizeof(VlakRen),"addvlak");
		R.blovl[a]= v;
	}
	v+= (nr & 255);
	return v;
}

HaloRen *inithalo(Material *ma, float *vec, float *vec1, float *orco, float hasize, float vectsize)
{
	extern float Tin, Tr, Tg, Tb;
	HaloRen *har;
	MTex *mtex;
	float xn, yn, zn, sco1[3], sco2[3], texvec[3], hoco[4], hoco1[4];

	if(hasize==0) return 0;
	
	projectverto(vec, hoco);
	if(hoco[3]==0.0) return 0;
	if(vec1) {
		projectverto(vec1, hoco1);
		if(hoco1[3]==0.0) return 0;
	}
	
	har= addhalo(R.tothalo++);
	VECCOPY(har->co, vec);
	har->hasize= hasize;

	/* projectvert wordt in zbufvlaggen gedaan ivm parts/border/pano */

	/* halovect */
	if(vec1) {
	
		har->type |= HA_VECT;
		 
		zn= hoco[3];
		har->xs= 0.5*R.rectx*(hoco[0]/zn);
		har->ys= 0.5*R.recty*(hoco[1]/zn);
		har->zs= 0x7FFFFF*(1.0+hoco[2]/zn);
	
		xn=  har->xs - 0.5*R.rectx*(hoco1[0]/hoco1[3]);
		yn=  har->ys - 0.5*R.recty*(hoco1[1]/hoco1[3]);
		if(xn==0.0 || (xn==0.0 && yn==0.0)) zn= 0.0;
		else zn= atan2f(yn, xn);
		
		har->sin= fsin(zn);
		har->cos= fcos(zn);
		zn= VecLenf(vec1, vec);

		har->hasize= vectsize*zn + (1.0-vectsize)*hasize;
	}

	if(ma->mode & MA_HALO_XALPHA) har->type |= HA_XALPHA;
	
	har->alfa= ma->alpha; 
	har->r= 255.0*ma->r;
	har->g= 255.0*ma->g; 
	har->b= 255.0*ma->b;
	har->add= 255.0*ma->add;
	har->mat= ma->ren;
	har->hard= ma->har;
	har->seed= ma->ren->seed1 % 256;

	if(ma->mode & MA_STAR) har->starpoints= ma->starc;
	if(ma->mode & MA_HALO_LINES) har->linec= ma->linec;
	if(ma->mode & MA_HALO_RINGS) har->ringc= ma->ringc;
	if(ma->mode & MA_HALO_FLARE) har->flarec= ma->flarec;


	if(ma->mtex[0]) {
		
		if( (ma->mode & MA_HALOTEX) ) har->tex= 1;
		else {
		
			mtex= ma->mtex[0];
			VECCOPY(texvec, vec);
			
			if(mtex->texco & TEXCO_NORM) {
				;
			}
			else if(mtex->texco & TEXCO_OBJECT) {
				/* texvec[0]+= imatbase->ivec[0]; */
				/* texvec[1]+= imatbase->ivec[1]; */
				/* texvec[2]+= imatbase->ivec[2]; */
				/* Mat3MulVecfl(imatbase->imat, texvec); */
			}
			else {
				if(orco) {
					VECCOPY(texvec, orco);\
				}
			}
			
			externtex(mtex, texvec);
			
			yn= Tin*mtex->colfac;
			zn= Tin*mtex->varfac;
			
			if(mtex->mapto & MAP_COL) {
				zn= 1.0-yn;
				har->r= 255.0*(yn*Tr+ zn*ma->r);
				har->g= 255.0*(yn*Tg+ zn*ma->g);
				har->b= 255.0*(yn*Tb+ zn*ma->b);
			}
			if(mtex->texco & 16) {
				har->alfa= 255.0*Tin;
			}
		}
	}

	return har;
}

HaloRen *initstar(vec, hasize)
float *vec, hasize;
{
	HaloRen *har;
	float zn, hoco[4];

	projectverto(vec, hoco);
	if(	(R.r.mode & R_PANORAMA) ||  testclip(hoco)==0 ) {
		har= addhalo(R.tothalo++);
		
		/* projectvert wordt in zbufvlaggen overgedaan ivm parts */
		VECCOPY(har->co, vec);
		har->hasize= hasize;
		
		har->zd= 0.0;
		
		return har;
	}
	return 0;
}

void add_render_lamp(Object *ob, int doshadbuf)
{
	VertRen *ver;
	Lamp *la;
	LampRen *lar;
	HaloRen *har;
	float mat[4][4], hoco[4], vec[3], hasize, hoek, xn, yn, zn;
	int ok, z,c;
	char *b;

	if(R.totlamp>=MAXLAMP) {
		printf("lamp overflow\n");
		return;
	}
	la= ob->data;
	lar= (LampRen *)callocN(sizeof(LampRen),"lampren");
	R.la[R.totlamp++]= lar;

	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat4Invert(ob->imat, mat);
	
	Mat3CpyMat4(lar->imat, ob->imat);

	lar->type= la->type;
	lar->mode= la->mode;
	
	lar->energy= la->energy;
	lar->energy= la->energy*R.wrld.exposure;	
	if(la->mode & LA_NEG) lar->energy= -lar->energy;
	
	lar->vec[0]= -mat[2][0];
	lar->vec[1]= -mat[2][1];
	lar->vec[2]= -mat[2][2];
	Normalise(lar->vec);
	lar->co[0]= mat[3][0];
	lar->co[1]= mat[3][1];
	lar->co[2]= mat[3][2];
	lar->dist= la->dist;
	lar->haint= la->haint;
	lar->distkw= lar->dist*lar->dist;
	lar->r= lar->energy*la->r;
	lar->g= lar->energy*la->g;
	lar->b= lar->energy*la->b;
	lar->spotsi= 0.5;
	
	lar->spotsi= fcos( M_PI*la->spotsize/360.0 );
	lar->spotbl= (1.0-lar->spotsi)*la->spotblend;
	
	memcpy(lar->mtex, la->mtex, 8*4);
	
	
	lar->lay= ob->lay;

	lar->ld1= la->att1;
	lar->ld2= la->att2;

	if(lar->type==LA_SPOT) {

		Normalise(lar->imat[0]);
		Normalise(lar->imat[1]);
		Normalise(lar->imat[2]);

		xn= safacos(lar->spotsi);
		xn= fsin(xn)/fcos(xn);
		lar->spottexfac= 1.0/(xn);
		
		if(lar->mode & LA_ONLYSHADOW) {
			if((lar->mode & LA_SHAD)==0) lar->mode -= LA_ONLYSHADOW;
			else if((R.r.mode & R_SHADOW)==0) lar->mode -= LA_ONLYSHADOW;
		}

	}
	
	/* imat bases */


	/* flag zetten voor spothalo en initvars */
	if(la->type==LA_SPOT && (la->mode & LA_HALO)) {
		if(la->haint>0.0) {
			R.flag |= R_LAMPHALO;
			
			/* camerapos (0,0,0) roteren rondom lamp */
			lar->sh_invcampos[0]= -lar->co[0];
			lar->sh_invcampos[1]= -lar->co[1];
			lar->sh_invcampos[2]= -lar->co[2];
			Mat3MulVecfl(lar->imat, lar->sh_invcampos);
				
			/* z factor, zodat het volume genormaliseerd is */
			hoek= safacos(lar->spotsi);
			xn= lar->spotsi;
			yn= fsin(hoek);
			lar->sh_zfac= yn/xn;
			/* alvast goed scalen */
			lar->sh_invcampos[2]*= lar->sh_zfac;
			
		}
	}
	
	for(c=0; c<6; c++) {
		if(la->mtex[c] && la->mtex[c]->tex) {
			lar->mode |= LA_TEXTURE;

			if(R.flag & R_RENDERING) {
				if(R.osa) {
					if(la->mtex[c]->tex->type==TEX_IMAGE) lar->mode |= LA_OSATEX;
				}
			}
		}
	}
	
	if(R.r.mode & R_SHADOW) if(lar->mode & LA_SHAD) if(la->type==LA_SPOT) {
		if(doshadbuf) initshadowbuf(lar, la, ob->obmat);
	}
	
	lar->org= dupallocN(lar);
}

void defaultlamp()
{
	LampRen *lar;

	lar= (LampRen *)callocN(sizeof(LampRen),"lampren");
	R.la[R.totlamp++]=lar;

	lar->type= LA_SUN;
	lar->vec[0]= -R.viewmat[2][0];
	lar->vec[1]= -R.viewmat[2][1];
	lar->vec[2]= -R.viewmat[2][2];
	Normalise(lar->vec);
	lar->r= 1.0;
	lar->g= 1.0;
	lar->b= 1.0;
	lar->lay= 65535;
}

void split_u_renderfaces(startvlak, startvert, usize, plek, cyclu)
int startvlak, startvert, usize, plek, cyclu;
{
	VlakRen *vlr;
	VertRen *v1, *v2;
	int a, v;

	if(cyclu) cyclu= 1;

	/* geef eerst alle betreffende vertices een pointer naar de nieuwe mee */
	v= startvert+ plek*usize;
	for(a=0; a<usize; a++) {
		v2= addvert(R.totvert++);
		v1= addvert(v++);
		*v2= *v1;
		v1->sticky= (float *)v2;
	}

	/* loop betreffende vlakken af en vervang pointers */
	v= startvlak+plek*(usize-1+cyclu);
	for(a=1-cyclu; a<usize; a++) {
		vlr= addvlak(v++);
		vlr->v1= (VertRen *)(vlr->v1->sticky);
		vlr->v2= (VertRen *)(vlr->v2->sticky);
	}

}

void split_v_renderfaces(startvlak, startvert, usize, vsize, plek, cyclu, cyclv)
int startvlak, startvert, usize, vsize, plek, cyclu, cyclv;
{
	VlakRen *vlr;
	VertRen *v1=0, *v2;
	int a, vert, vlak, ofs;

	if(vsize<2) return;
	
	/* loop betreffende vlakken af en maak dubbels */
	/* omdat (evt) split_u al gedaan is kan niet met vertex->sticky pointers worden gewerkt  */
	/* want vlakken delen al geen punten meer */

	if(plek+cyclu==usize) plek= -1;

	vlak= startvlak+(plek+cyclu);
	ofs= (usize-1+cyclu);

	for(a=1; a<vsize; a++) {

		vlr= addvlak(vlak);
		if (vlr->v1 == 0) return; /* OEPS, als niet cyclic */
		
		v1= addvert(R.totvert++);
		*v1= *(vlr->v1);
		
		vlr->v1= v1;

		/* vlr= addvlak(vlak+1); */
		/* vlr->v1= v1; */

		if(a>1) {
			vlr= addvlak(vlak-ofs);
			if(vlr->v4->sticky) {
				v1= addvert(R.totvert++);
				*v1= *(vlr->v4);
				vlr->v4= v1;
			}
			else vlr->v4= v1;
		}

		if(a== vsize-1) {
			if(cyclv) {
				;
			}
			else {
				vlr= addvlak(vlak);
				v1= addvert(R.totvert++);
				*v1= *(vlr->v4);
				vlr->v4= v1;
			}
		}

		vlak+= ofs;
	}

}


Material *give_render_material(Object *ob, int nr)
{
	Object *temp;
	
	if(ob->flag & OB_FROMDUPLI) {
		temp= (Object *)ob->id.new;
		if(temp && temp->type==OB_FONT) {
			ob= temp;
		}
	}
	
	return give_current_material(ob, nr);
}

void init_render_surf(Object *ob)
{
	Nurb *nu=0;
	Curve *cu;
	ListBase displist;
	DispList *dl;
	VertRen *ver, *v1, *v2, *v3, *v4;
	VlakRen *vlr;
	Material *matar[32];
	float *data, *fp, *orco, n1[3], n2[3], flen, mat[4][4];
	int len, a, b, need_orco=0, startvlak, startvert, p1, p2, p3, p4;
	
	cu= ob->data;
	nu= cu->nurb.first;
	if(nu==0) return;

	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat4Invert(ob->imat, mat);

	/* material array */
	bzero(matar, 4*32);
	matar[0]= &defmaterial;
	for(a=0; a<ob->totcol; a++) {
		matar[a]= give_render_material(ob, a+1);
		if(matar[a]==0) matar[a]= &defmaterial;
		if(matar[a] && matar[a]->ren->texco & TEXCO_ORCO) {
			need_orco= 1;			
		}
	}
	
	if(ob->parent && ob->parent->type==OB_LATTICE) need_orco= 1;
	
	if(cu->orco==0 && need_orco) make_orco_surf(cu);
	orco= cu->orco;
	
	/* een complete displist maken, de basedisplist kan compleet anders zijn */
	displist.first= displist.last= 0;
	nu= cu->nurb.first;
	while(nu) {
		if(nu->pntsv>1) {
			len= nu->resolu*nu->resolv;
					/* makeNurbfaces wil nullen */
					
			dl= callocN(sizeof(DispList)+len*3*4, "makeDispList1");
			dl->verts= callocN(len*3*4, "makeDispList01");
			addtail(&displist, dl);

			dl->parts= nu->resolu;	/* andersom want makeNurbfaces gaat zo */
			dl->nr= nu->resolv;
			dl->col= nu->mat_nr;
			dl->rt= nu->flag;
			
			data= dl->verts;
			dl->type= DL_SURF;
			if(nu->flagv & 1) dl->flag|= 1;	/* ook andersom ! */
			if(nu->flagu & 1) dl->flag|= 2;

			makeNurbfaces(nu, data);
		}
		nu= nu->next;
	}

	if(ob->parent && ob->parent->type==OB_LATTICE) {
		init_latt_deform(ob->parent, ob);		
		dl= displist.first;
		while(dl) {
			
			fp= dl->verts;
			len= dl->nr*dl->parts;
			for(a=0; a<len; a++, fp+=3)  calc_latt_deform(fp);
			
			dl= dl->next;
		}
		end_latt_deform();
	}

	dl= displist.first;
	while(dl) {
		if(dl->type==DL_SURF) {
			startvert= R.totvert;
			a= dl->nr*dl->parts;
			data= dl->verts;
			while(a--) {
				ver= addvert(R.totvert++);
				VECCOPY(ver->co, data);
				if(orco) {
					ver->orco= orco;
					orco+= 3;
				}
				Mat4MulVecfl(mat, ver->co);
				data+= 3;
			}

			startvlak= R.totvlak;

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
						vlr= addvlak(R.totvlak++);
						vlr->v1= v1;
						vlr->v2= v3;
						vlr->v3= v4;
						vlr->v4= v2;
						VECCOPY(vlr->n, n1);
						vlr->len= flen;
						vlr->lay= ob->lay;			
						vlr->mat= matar[ dl->col];
						vlr->ec= ME_V1V2+ME_V2V3;
						vlr->flag= dl->rt;
					}

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
			
			for(a=startvert; a<R.totvert; a++) {
				ver= addvert(a);
				Normalise(ver->n);
			}
			
			
		}
		dl= dl->next;
	}
	freedisplist(&displist);
}

void init_render_curve(Object *ob)
{
	Curve *cu;
	VertRen *ver;
	VlakRen *vlr, *vlr1;
	ListBase dlbev;
	Nurb *nu=0;
	DispList *dlb, *dl;
	BevList *bl;
	BevPoint *bevp;
	Material *matar[32];
	float len, *data, *fp, *fp1, fac;
	float n[3], vec[3], widfac, size[3], mat[4][4];
	int nr, startvert, startvlak, a, b, p1, p2, p3, p4;
	int totvert, frontside, ofs, end, need_orco=0, firststartvert, *index;

	cu= ob->data;
	nu= cu->nurb.first;
	if(nu==0) return;
	
	/* displist testen */
	if(cu->disp.first==0) makeDispList(ob);
	dl= cu->disp.first;
	if(cu->disp.first==0) return;
	
	if(dl->type!=DL_INDEX3) {
		curve_to_filledpoly(cu, &cu->disp);
	}

	if(cu->bev.first==0) makeBevelList(ob);

	firststartvert= R.totvert;

	Mat4MulMat4(mat, ob->obmat, R.viewmat);	
	Mat4Invert(ob->imat, mat);
	
	/* material array */
	bzero(matar, 4*32);
	matar[0]= &defmaterial;
	for(a=0; a<ob->totcol; a++) {
		matar[a]= give_render_material(ob, a+1);
		if(matar[a]==0) matar[a]= &defmaterial;
		if(matar[a]->ren->texco & TEXCO_ORCO) {
			need_orco= 1;			
		}
	}

	/* bevelcurve in displist */
	dlbev.first= dlbev.last= 0;
	
	if(cu->ext1!=0.0 || cu->ext2!=0.0 || cu->bevobj!=0) {
		makebevelcurve(ob, &dlbev);
	}

	/* uv orco's? aantal punten tellen en malloccen */
	if(need_orco && (cu->flag & CU_UV_ORCO)) {
		if(cu->flag & CU_PATH);
		else {
			totvert= 0;
			bl= cu->bev.first;
			while(bl) {
				dlb= dlbev.first;
				while(dlb) {
					totvert+= dlb->nr*bl->nr;
					dlb= dlb->next;
				}
				bl= bl->next;
			}
			
			fp= cu->orco= mallocN(3*4*totvert, "cu->orco");
			
			bl= cu->bev.first;
			while(bl) {
				dlb= dlbev.first;
				while(dlb) {
					for(b=0; b<dlb->nr; b++) {
						fac= (2.0*b/(float)(dlb->nr-1)) - 1.0;
						for(a=0; a<bl->nr; a++, fp+=3) {
							fp[0]= (2.0*a/(float)(bl->nr-1)) - 1.0;
							fp[1]= fac;
							fp[2]= 0.0;
						}
					}
					dlb= dlb->next;
				}
				bl= bl->next;
			}
		}
	}
	
	/* keypos doen? NOTITIE: pas op : orco's */
	
	/* effect op text? */

	/* boundboxclip nog doen */	

	/* polyzijvlakken:  met bevellist werken */
	widfac= (cu->width-1.0);

	bl= cu->bev.first;
	nu= cu->nurb.first;
	while(bl) {

		if(dlbev.first) {    /* anders alleen een poly */

			dlb= dlbev.first;   /* bevel lus */
			while(dlb) {
				data= mallocN(12*dlb->nr*bl->nr, "init_render_curve3");
				fp= data;

				/* voor ieder punt van bevelcurve de hele poly doen */
				fp1= dlb->verts;
				b= dlb->nr;
				while(b--) {

					bevp= (BevPoint *)(bl+1);
					a= bl->nr;
					while(a--) {

						if(cu->flag & CU_3D) {
							vec[0]= fp1[1]+widfac;
							vec[1]= fp1[2];
							vec[2]= 0.0;
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

				/* rendervertices maken */
				fp= data;
				startvert= R.totvert;
				nr= dlb->nr*bl->nr;

				while(nr--) {
					ver= addvert(R.totvert++);
					VECCOPY(ver->co, fp);
					Mat4MulVecfl(mat, ver->co);
					fp+= 3;
				}

				startvlak= R.totvlak;

				for(a=0; a<dlb->nr; a++) {

					frontside= (a >= dlb->nr/2);

					DL_SURFINDEX(bl->poly>0, dlb->type==DL_POLY, bl->nr, dlb->nr);
					p1+= startvert; 
					p2+= startvert; 
					p3+= startvert; 
					p4+= startvert;
					
					for(; b<bl->nr; b++) {

						vlr= addvlak(R.totvlak++);
						vlr->v1= addvert(p2);
						vlr->v2= addvert(p1);
						vlr->v3= addvert(p3);
						vlr->v4= addvert(p4);
						vlr->ec= ME_V2V3+ME_V3V4;
						if(a==0) vlr->ec+= ME_V1V2;
						
						vlr->flag= nu->flag;
						vlr->lay= ob->lay;
						
						/* dit is niet echt wetenschappelijk: de vertices
						 * 2, 3 en 4 geven betere puno's dan 1 2 3: voor en achterkant anders!!
						 */
						 
						if(frontside)
							vlr->len= CalcNormFloat(vlr->v2->co, vlr->v3->co, vlr->v4->co, vlr->n);
						else 
							vlr->len= CalcNormFloat(vlr->v1->co, vlr->v2->co, vlr->v3->co, vlr->n);
						
						vlr->mat= matar[ nu->mat_nr ];

						p4= p3; 
						p3++;
						p2= p1; 
						p1++;
						
					}
					
				}

				/* dubbele punten maken: POLY SPLITSEN */
				if(dlb->nr==4 && cu->bevobj==0) {
					split_u_renderfaces(startvlak, startvert, bl->nr, 1, bl->poly>0);
					split_u_renderfaces(startvlak, startvert, bl->nr, 2, bl->poly>0);
				}
				/* dubbele punten maken: BEVELS SPLITSEN */
				bevp= (BevPoint *)(bl+1);
				for(a=0; a<bl->nr; a++) {
					if(bevp->f1)
						split_v_renderfaces(startvlak, startvert, bl->nr, dlb->nr, a, bl->poly>0,
						    dlb->type==DL_POLY);
					bevp++;
				}

				/* puntnormalen */
				for(a= startvlak; a<R.totvlak; a++) {
					vlr= addvlak(a);

					VecAddf(vlr->v1->n, vlr->v1->n, vlr->n);
					VecAddf(vlr->v3->n, vlr->v3->n, vlr->n);
					VecAddf(vlr->v2->n, vlr->v2->n, vlr->n);
					VecAddf(vlr->v4->n, vlr->v4->n, vlr->n);
				}
				for(a=startvert; a<R.totvert; a++) {
					ver= addvert(a);
					len= Normalise(ver->n);
					if(len==0.0) ver->sticky= (float *)1;
					else ver->sticky= 0;
				}
				for(a= startvlak; a<R.totvlak; a++) {
					vlr= addvlak(a);
					if(vlr->v1->sticky) VECCOPY(vlr->v1->n, vlr->n);
					if(vlr->v2->sticky) VECCOPY(vlr->v2->n, vlr->n);
					if(vlr->v3->sticky) VECCOPY(vlr->v3->n, vlr->n);
					if(vlr->v4->sticky) VECCOPY(vlr->v4->n, vlr->n);
				}

				dlb= dlb->next;

				freeN(data);
			}

		}
		bl= bl->next;
		nu= nu->next;
	}

	if(dlbev.first) {
		freedisplist(&dlbev);
	}

	if(cu->flag & CU_PATH) return;

	/* uit de displist kunnen de vulvlakken worden gehaald */
	dl= cu->disp.first;
	
	while(dl) {
		if(dl->type==DL_INDEX3) {
			
			startvert= R.totvert;
			data= dl->verts;
				
			n[0]= ob->imat[0][2];
			n[1]= ob->imat[1][2];
			n[2]= ob->imat[2][2];
			Normalise(n);

			for(a=0; a<dl->nr; a++, data+=3) {
		
				ver= addvert(R.totvert++);
				VECCOPY(ver->co, data);
				Mat4MulVecfl(mat, ver->co);
				
				VECCOPY(ver->n, n);
			}
			
			startvlak= R.totvlak;
			index= dl->index;
			for(a=0; a<dl->parts; a++, index+=3) {
				
				vlr= addvlak(R.totvlak++);
				vlr->v1= addvert(startvert+index[0]);
				vlr->v2= addvert(startvert+index[1]);
				vlr->v3= addvert(startvert+index[2]);
				vlr->v4= 0;
				
				VECCOPY(vlr->n, n);
		
				vlr->mface= 0;
				vlr->mat= matar[ dl->col ];
				vlr->puno= 0;
				vlr->flag= 0;
				vlr->ec= 0;
				vlr->lay= ob->lay;
			}
			
		}
		dl= dl->next;
	}

	if(need_orco) {	/* de domme methode: snel vervangen; rekening houden met keys! */

		VECCOPY(size, cu->size);

		nr= R.totvert-firststartvert;
		if(nr) {
			if(cu->orco) {
				fp= cu->orco;
				while(nr--) {
					ver= addvert(firststartvert++);
					ver->orco= fp;
					fp+= 3;
				}
			}
			else {
				fp= cu->orco= mallocN(4*3*nr, "cu orco");
				while(nr--) {
					ver= addvert(firststartvert++);
					ver->orco= fp;
					
					VECCOPY(fp, ver->co);
					Mat4MulVecfl(ob->imat, fp);
	
					fp[0]= (fp[0]-cu->loc[0])/size[0];
					fp[1]= (fp[1]-cu->loc[1])/size[1];
					fp[2]= (fp[2]-cu->loc[2])/size[2];
					fp+= 3;
				}
			}
		}
	}
}

void render_particle_system(Object *ob, PartEff *paf)
{
	extern float Tin, Tr, Tg, Tb;
	Particle *pa;
	HaloRen *har;
	Material *ma;
	MTex *mtex;
	float xn, yn, zn, imat[3][3], mat[4][4], hasize, ptime, ctime, vec[3], vec1[3], view[3], nor[3];
	int a, mat_nr=1;
	
	pa= paf->keys;
	if(pa==0) {
		build_particle_system(ob);
		pa= paf->keys;
		if(pa==0) return;
	}

	ma= give_render_material(ob, 1);
	if(ma==0) ma= &defmaterial;	
	
	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat4Invert(ob->imat, mat);	/* hoort zo, voor imat texture */
	
	Mat4Invert(mat, R.viewmat);	/* particles hebben geen ob transform meer */
	Mat3CpyMat4(imat, mat);
	
	R.flag |= R_HALO;
	
	if(ob->ipoflag & OB_OFFS_PARTICLE) ptime= ob->sf;
	else ptime= 0.0;
	ctime= system_time(0, 0, (float)CFRA, ptime);

	ma->ren->seed1= ma->seed1;

	for(a=0; a<paf->totpart; a++, pa+=paf->totkey) {
		
		if(ctime > pa->time) {
			if(ctime < pa->time+pa->lifetime) {
				
				/* let op: ook nog de normaal van de particle berekenen */
				if(paf->stype==PAF_VECT) {
					where_is_particle(paf, pa, ctime, vec);
					Mat4MulVecfl(R.viewmat, vec);
					where_is_particle(paf, pa, ctime+1.0, vec1);
					Mat4MulVecfl(R.viewmat, vec1);
				}
				else {
					where_is_particle(paf, pa, ctime, vec);
					Mat4MulVecfl(R.viewmat, vec);
				}
				
				if(pa->mat_nr != mat_nr) {
					mat_nr= pa->mat_nr;
					ma= give_render_material(ob, mat_nr);
					if(ma==0) ma= &defmaterial;	
				}
				
				if(ma->ipo) {
					/* correctie voor lifetime */
					calc_ipo(ma->ipo, 100.0*(ctime-pa->time)/pa->lifetime );
					execute_ipo((ID *)ma, ma->ipo);
				}			
	
				hasize= ma->hasize;
				
				if(ma->mode & MA_HALOPUNO) {
					xn= pa->no[0]; 
					yn= pa->no[1]; 
					zn= pa->no[2];
				
					/* transpose ! */
					nor[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
					nor[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
					nor[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
					Normalise(nor);
					
					VECCOPY(view, vec);
					Normalise(view);
					
					zn= nor[0]*view[0]+nor[1]*view[1]+nor[2]*view[2];
					if(zn>=0.0) hasize= 0.0;
					else hasize*= zn*zn*zn*zn;
				}
				
				if(paf->stype==PAF_VECT) har= inithalo(ma, vec, vec1, pa->co, hasize, paf->vectsize);
				else har= inithalo(ma, vec, 0, pa->co, hasize, 0);
				
			}
		}
		ma->ren->seed1++;	
	}
	
}

void make_render_halos(Object *ob, Mesh *me, Material *ma, float *extverts)
{
	HaloRen *har;
	MVert *mvert;
	MTex *mtex;
	float xn, yn, zn, nor[3], view[3];
	float *orco, tinc, vec[3], hasize, *fp, mat[4][4], imat[3][3];
	int start, end, a, ok;
	
	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat3CpyMat4(imat, ob->imat);
	
	R.flag |= R_HALO;
	mvert= me->mvert;
	
	orco= me->orco;
	
	start= 0; end= me->totvert;
	set_buildvars(ob, &start, &end);
	mvert+= start;
	if(extverts) extverts+= 3*start;
	
	ma->ren->seed1= ma->seed1;
	
	for(a=start; a<end; a++, mvert++) {
		ok= 1;
		
		if(ok) {
			hasize= ma->hasize;
			
			if(extverts) {
				VECCOPY(vec, extverts);
				extverts+= 3;
			}
			else {
				VECCOPY(vec, mvert->co);
			}
			Mat4MulVecfl(mat, vec);
			
			if(ma->mode & MA_HALOPUNO) {	
				xn= mvert->no[0]; 
				yn= mvert->no[1]; 
				zn= mvert->no[2];
			
				/* transpose ! */
				nor[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
				nor[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
				nor[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
				Normalise(nor);
				
				VECCOPY(view, vec);
				Normalise(view);
				
				zn= nor[0]*view[0]+nor[1]*view[1]+nor[2]*view[2];
				if(zn>=0.0) hasize= 0.0;
				else hasize*= zn*zn*zn*zn;
			}
			
			if(orco) har= inithalo(ma, vec, 0, orco, hasize, 0);
			else inithalo(ma, vec, 0, mvert->co, hasize, 0);
	
		}
		if(orco) orco+= 3;
		ma->ren->seed1++;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int contrpuntnormr(float *n, float *puno)
{
	float inp;

	inp=n[0]*puno[0]+n[1]*puno[1]+n[2]*puno[2];
	if(inp<0.0) return 1;
	return 0;
}

float safacos(float fac)
{
	if(fac<= -1.0) return M_PI;
	else if(fac>=1.0) return 0.0;
	else return facos(fac);
}

void normalenrender(int startvert, int startvlak)
{
	VlakRen *vlr,*vlro;
	VertRen *ver, *adrve1, *adrve2, *adrve3, *adrve4;
	float n1[3], n2[3], n3[3], n4[3], *adrco, *adrno, *tfl, fac, *f1, *temp;
	int a;
	short x, y, z, *z1=0;

	if(R.totvlak==0 || R.totvert==0) return;
	if(startvert==R.totvert || startvlak==R.totvlak) return;
	
	adrco= (float *)callocN(12+16*(R.totvlak-startvlak), "normalen1");

	tfl= adrco;
	/* berekenen cos hoeken en puntmassa's */
	for(a= startvlak; a<R.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;
		adrve4= vlr->v4;
		
		VecSubf(n1, adrve2->co, adrve1->co);
		Normalise(n1);
		VecSubf(n2, adrve3->co, adrve2->co);
		Normalise(n2);
		if(adrve4==0) {
			VecSubf(n3, adrve1->co, adrve3->co);
			Normalise(n3);
			
			*(tfl++)= safacos(-n1[0]*n3[0]-n1[1]*n3[1]-n1[2]*n3[2]);
			*(tfl++)= safacos(-n1[0]*n2[0]-n1[1]*n2[1]-n1[2]*n2[2]);
			*(tfl++)= safacos(-n2[0]*n3[0]-n2[1]*n3[1]-n2[2]*n3[2]);
		}
		else {
			VecSubf(n3, adrve4->co, adrve3->co);
			Normalise(n3);
			VecSubf(n4, adrve1->co, adrve4->co);
			Normalise(n4);
			
			*(tfl++)= safacos(-n4[0]*n1[0]-n4[1]*n1[1]-n4[2]*n1[2]);
			*(tfl++)= safacos(-n1[0]*n2[0]-n1[1]*n2[1]-n1[2]*n2[2]);
			*(tfl++)= safacos(-n2[0]*n3[0]-n2[1]*n3[1]-n2[2]*n3[2]);
			*(tfl++)= safacos(-n3[0]*n4[0]-n3[1]*n4[1]-n3[2]*n4[2]);
		}
	}

	/* alle puntnormalen leegmaken */
	for(a=startvert; a<R.totvert; a++) {
		ver= addvert(a);
		
		ver->n[0]=ver->n[1]=ver->n[2]= 0.0;
	}

	/* berekenen normalen en optellen bij puno's */
	tfl= adrco;
	for(a=startvlak; a<R.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;
		adrve4= vlr->v4;

		temp= adrve1->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n, temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];

		temp= adrve2->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n, temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];

		temp= adrve3->n;
		fac= *(tfl++);
		if( contrpuntnormr(vlr->n, temp) ) fac= -fac ;
		*(temp++) +=fac*vlr->n[0];
		*(temp++) +=fac*vlr->n[1];
		*(temp)   +=fac*vlr->n[2];

		if(adrve4) {
			temp= adrve4->n;
			fac= *(tfl++);
			if( contrpuntnormr(vlr->n, temp) ) fac= -fac ;
			*(temp++) +=fac*vlr->n[0];
			*(temp++) +=fac*vlr->n[1];
			*(temp)   +=fac*vlr->n[2];
		}
	}
	
	/* normaliseren puntnormalen */
	for(a=startvert; a<R.totvert; a++) {
		ver= addvert(a);
		
		Normalise(ver->n);
	}
	
	/* puntnormaal omklap-vlaggen voor bij shade */
	for(a=startvlak; a<R.totvlak; a++) {
		vlr= addvlak(a);
		
		adrve1= vlr->v1;
		adrve2= vlr->v2;
		adrve3= vlr->v3;
		adrve4= vlr->v4;

		vlr->puno= 0;
		fac= vlr->n[0]*adrve1->n[0]+vlr->n[1]*adrve1->n[1]+vlr->n[2]*adrve1->n[2];
		if(fac<0.0) vlr->puno= 1;
		fac= vlr->n[0]*adrve2->n[0]+vlr->n[1]*adrve2->n[1]+vlr->n[2]*adrve2->n[2];
		if(fac<0.0) vlr->puno+= 2;
		fac= vlr->n[0]*adrve3->n[0]+vlr->n[1]*adrve3->n[1]+vlr->n[2]*adrve3->n[2];
		if(fac<0.0) vlr->puno+= 4;
		
		if(adrve4) {
			fac= vlr->n[0]*adrve4->n[0]+vlr->n[1]*adrve4->n[1]+vlr->n[2]*adrve4->n[2];
			if(fac<0.0) vlr->puno+= 8;
		}
	}

	freeN(adrco);
}

void init_render_mesh(Object *ob)
{
	MFace *mface;
	MVert *mvert;
	Mesh *me;
	VlakRen *vlr, *vlr1;
	VertRen *ver;
	Material *ma;
	MSticky *ms;
	PartEff *paf;
	DispList *dl;
	ulong *vertcol;
	float len, xn, yn, zn, nor[3], imat[3][3], mat[4][4];
	float *extverts=0, *orco, *v1, *v2, *v3, vec[3];
	int a, a1, ok, do_puno, need_orco=0, totvlako, totverto, vertofs;
	int start, end, flipnorm, warning=0;
	
	
	if( paf=give_parteff(ob) ) {
		render_particle_system(ob, paf);
		return;
	}
	
	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat4Invert(ob->imat, mat);
	Mat3CpyMat4(imat, ob->imat);

	me= ob->data;
	if(me->totvert==0) return;
	mvert= me->mvert;
	
	do_puno= object_deform(ob);

	dl= test_displist(&ob->disp, DL_VERTS);
	if(dl) extverts= dl->verts;

	totvlako= R.totvlak;
	totverto= R.totvert;

	if(me->key) do_puno= 1;
	
	
	if(me->orco==0) {
		need_orco= 0;
		for(a=1; a<=ob->totcol; a++) {
			ma= give_render_material(ob, a);
			if(ma) {
				if(ma->ren->texco & TEXCO_ORCO) {
					need_orco= 1;
					break;
				}
			}
		}
		if(need_orco) {
			make_orco_mesh(me);
		}
	}
	orco= me->orco;
	ms= me->msticky;
	vertcol= (ulong *)me->mcol;
	
	ma= give_render_material(ob, 1);
	if(ma==0) ma= &defmaterial;	
	
	
	if(ma->mode & MA_HALO) {
		make_render_halos(ob, me, ma, extverts);
	}
	else {
		
		for(a=0; a<me->totvert; a++, mvert++) {
			
			ver= addvert(R.totvert++);
			if(extverts) {
				VECCOPY(ver->co, extverts);
				extverts+= 3;
			}
			else {
				VECCOPY(ver->co, mvert->co);
			}
			Mat4MulVecfl(mat, ver->co);

			xn= mvert->no[0]; 
			yn= mvert->no[1]; 
			zn= mvert->no[2];
			if(do_puno==0) {
				/* transpose ! */
				ver->n[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
				ver->n[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
				ver->n[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
				Normalise(ver->n);
			}
			if(orco) {
				ver->orco= orco;
				orco+=3;
			}
			if(ms) {
				ver->sticky= (float *)ms;
				ms++;
			}
		}
		/* nog doen bij keys: de juiste lokale textu coordinaat */
		
		flipnorm= -1;
		/* Testen of er een flip in de matrix zit: dan vlaknormaal ook flippen */
		
		/* vlakken in volgorde colblocks */
		vertofs= R.totvert- me->totvert;
		for(a1=0; (a1<ob->totcol || (a1==0 && ob->totcol==0)); a1++) {
		
			ma= give_render_material(ob, a1+1);
			if(ma==0) ma= &defmaterial;

			/* testen op 100% transparant */
			ok= 1;
			if(ma->alpha==0.0 && ma->spectra==0.0) {
				ok= 0;
				/* texture op transp? */
				for(a=0; a<8; a++) {
					if(ma->mtex[a] && ma->mtex[a]->tex) {
						if(ma->mtex[a]->mapto & MAP_ALPHA) ok= 1;
					}
				}
			}
			
			if(ok) {
			
				start= 0; end= me->totface;
				set_buildvars(ob, &start, &end);
				mvert= me->mvert;
				mface= me->mface;
				mface+= start;
				
				for(a=start; a<end; a++, mface++) {

					if( mface->mat_nr==a1 ) {
					
						if(mface->v3) {
							
							vlr= addvlak(R.totvlak++);
							vlr->v1= addvert(vertofs+mface->v1);
							vlr->v2= addvert(vertofs+mface->v2);
							vlr->v3= addvert(vertofs+mface->v3);
							if(mface->v4) vlr->v4= addvert(vertofs+mface->v4);
							else vlr->v4= 0;
							
							/* rendernormalen zijn omgekeerd */
							vlr->len= CalcNormFloat(vlr->v3->co, vlr->v2->co, vlr->v1->co, vlr->n);
							vlr->mface= mface;
							vlr->mat= ma;
							vlr->puno= mface->puno;
							vlr->flag= mface->flag;
							if(me->flag & ME_NOPUNOFLIP) {
								vlr->flag |= R_NOPUNOFLIP;
								vlr->puno= 15;
							}
							vlr->ec= mface->edcode;
							vlr->lay= ob->lay;
							
if(R.r.scemode & R_HOTSPOT) {
	if(ob->infoflag & OB_INFO_HOTSPOT) {
		*( (char *)&vlr->lay ) = ob->infostart;
		vlr->flag |= R_CMAPCODE;
		if(ob->infoend==0) vlr->flag |= R_VISIBLE;
	}
}							
							
							if(vlr->len==0) R.totvlak--;
							else {
								if(flipnorm== -1) {	/* per object 1 x testen */
									v1= (me->mvert+mface->v1)->co;
									v2= (me->mvert+mface->v2)->co;
									v3= (me->mvert+mface->v3)->co;
									
									CalcNormFloat(v1, v2, v3, nor);
									vec[0]= imat[0][0]*nor[0]+ imat[0][1]*nor[1]+ imat[0][2]*nor[2];
									vec[1]= imat[1][0]*nor[0]+ imat[1][1]*nor[1]+ imat[1][2]*nor[2];
									vec[2]= imat[2][0]*nor[0]+ imat[2][1]*nor[1]+ imat[2][2]*nor[2];

									xn= vec[0]*vlr->n[0]+vec[1]*vlr->n[1]+vec[2]*vlr->n[2];
									if(xn<0.0) flipnorm= 1;
									else flipnorm= 0;
									
								}
								if(flipnorm) {
									vlr->n[0]= -vlr->n[0];
									vlr->n[1]= -vlr->n[1];
									vlr->n[2]= -vlr->n[2];
								}								
								
								if(vertcol)  vlr->vcol= vertcol+4*a;
								else vlr->vcol= 0;
								
								/* testen of een vierhoek als driehoek gerenderd moet */
								if(vlr->v4) {
									if(ma->mode & MA_WIRE);
									else {
										CalcNormFloat(vlr->v4->co, vlr->v3->co, vlr->v1->co, nor);
										if(flipnorm) {
											nor[0]= -nor[0];
											nor[1]= -nor[1];
											nor[2]= -nor[2];
										}
										
										xn= INPR(nor, vlr->n);
										
										if( xn < 0.9990 ) {
											vlr1= addvlak(R.totvlak++);
											*vlr1= *vlr;
											VECCOPY(vlr1->n, nor);
											vlr1->v2= vlr->v3;
											vlr1->v3= vlr->v4;
											vlr->v4= vlr1->v4= 0;
											
											if(vertcol) warning= 1;
										}
									}
								}
							}
						}
						else if(mface->v2 && (ma->mode & MA_WIRE)) {
							vlr= addvlak(R.totvlak++);
							vlr->v1= addvert(vertofs+mface->v1);
							vlr->v2= addvert(vertofs+mface->v2);
							vlr->v3= vlr->v2;
							vlr->v4= 0;
							
							vlr->n[0]=vlr->n[1]=vlr->n[2]= 0.0;
							
							vlr->mface= mface;
							vlr->mat= ma;
							vlr->puno= mface->puno;
							vlr->flag= mface->flag;
							vlr->ec= ME_V1V2;
							vlr->lay= ob->lay;
						}
					}
				}
			}
		}
	}

	if(do_puno) normalenrender(totverto, totvlako);
	
	if(warning) printf("WARNING: ob %s with vertcol, non-flat squares\n", ob->id.name+2);
}


void init_render_mball(Object *ob)
{
	MetaBall *mb;
	MetaElem *ml;
	DispList *dl, *dlo;
	VertRen *ver, *adrve1, *adrve2, *adrve3, *adrve4;
	VlakRen *vlr, *vlr1;
	Material *ma;
	float *data, *nors, mat[4][4], imat[3][3], vec[3], fac, xn, yn, zn;
	int a, need_orco, startvlak, startvert, *index;
	
	Mat4MulMat4(mat, ob->obmat, R.viewmat);
	Mat4Invert(ob->imat, mat);
	Mat3CpyMat4(imat, ob->imat);

	if( has_id_number((ID *)ob) ) return;

	ma= give_render_material(ob, 1);
	if(ma==0) ma= &defmaterial;

	need_orco= 0;
	if(ma->ren->texco & TEXCO_ORCO) {
		need_orco= 1;
	}
	
	dlo= ob->disp.first;
	if(dlo) remlink(&ob->disp, dlo);
	
	makeDispList(ob);
	dl= ob->disp.first;
	if(dl==0) return;
	
	startvert= R.totvert;
	data= dl->verts;
	nors= dl->nors;
	
	for(a=0; a<dl->nr; a++, data+=3, nors+=3) {

		ver= addvert(R.totvert++);
		VECCOPY(ver->co, data);
		Mat4MulVecfl(mat, ver->co);
		
		/* rendernormalen zijn omgekeerd */
		xn= -nors[0]; 
		yn= -nors[1]; 
		zn= -nors[2];

		/* transpose ! */
		ver->n[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
		ver->n[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
		ver->n[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
		Normalise(ver->n);
		
		if(need_orco) ver->orco= data;
	}
	
	startvlak= R.totvlak;
	index= dl->index;
	for(a=0; a<dl->parts; a++, index+=4) {
		
		vlr= addvlak(R.totvlak++);
		vlr->v1= addvert(startvert+index[0]);
		vlr->v2= addvert(startvert+index[1]);
		vlr->v3= addvert(startvert+index[2]);
		vlr->v4= 0;
		
		/* rendernormalen zijn omgekeerd */
		vlr->len= CalcNormFloat(vlr->v3->co, vlr->v2->co, vlr->v1->co, vlr->n);

		vlr->mface= 0;
		vlr->mat= ma;
		vlr->puno= 0;
		vlr->flag= ME_SMOOTH+R_NOPUNOFLIP;
		vlr->ec= 0;
		vlr->lay= ob->lay;
		
		/* mball -helaas- altijd driehoeken maken omdat vierhoeken erg onregelmatig zijn */
		if(index[3]) {
			vlr1= addvlak(R.totvlak++);
			*vlr1= *vlr;
			vlr1->v2= vlr1->v3;
			vlr1->v3= addvert(startvert+index[3]);
			vlr->len= CalcNormFloat(vlr1->v3->co, vlr1->v2->co, vlr1->v1->co, vlr1->n);
		}
	}
	
	if(need_orco) {
		/* displist bewaren en scalen */
		make_orco_mball(ob);
		if(dlo) addhead(&ob->disp, dlo);
		
	}
	else {
		freedisplist(&ob->disp);
		if(dlo) addtail(&ob->disp, dlo);
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void setzbufvlaggen(projectfunc)	/* ook homoco's */
void (*projectfunc)();
{
	VlakRen *vlr;
	VertRen *ver;
	HaloRen *har;
	QStrip *qs;
	VertStrip *vs;
	float abs4, zn, vec[3], si, co, hoco[4];
	int a, doit;

	si= fsin(panophi);
	co= fcos(panophi);

	for(a=0; a< R.totvert;a++) {
		if((a & 255)==0) ver= R.blove[a>>8];
		else ver++;
		
		if(R.r.mode & R_PANORAMA) {
			vec[0]= co*ver->co[0] + si*ver->co[2];
			vec[1]= ver->co[1];
			vec[2]= -si*ver->co[0] + co*ver->co[2];
		}
		else {
			VECCOPY(vec, ver->co);
		}
		
		projectfunc(vec, ver->ho);
		ver->clip = testclip(ver->ho);
	
	}
	
	for(a=0; a<R.tothalo; a++) {
		if((a & 255)==0) har= R.bloha[a>>8];
		else har++;
		
		if(R.r.mode & R_PANORAMA) {
			vec[0]= co*har->co[0] + si*har->co[2];
			vec[1]= har->co[1];
			vec[2]= -si*har->co[0] + co*har->co[2];
		}
		else {
			VECCOPY(vec, har->co);
		}

		projectfunc(vec, hoco);
		
		hoco[3]*= 2.0;
		
		if( panotestclip(hoco) ) {
			har->miny= har->maxy= -100000;	/* de render clipt 'm weg */
		}
		else {
			zn= hoco[3]/2.0;
			har->xs= 0.5*R.rectx*(1.0+hoco[0]/zn);
			har->ys= 0.5*R.recty*(1.0+hoco[1]/zn);
			har->zs= 0x7FFFFF*(1.0+hoco[2]/zn);
			
			vec[0]+= har->hasize;
			projectfunc(vec, hoco);
			vec[0]-= har->hasize;
			zn= hoco[3];
			har->rad= fabs(har->xs- 0.5*R.rectx*(1.0+hoco[0]/zn));
			
			/* deze clip is eigenlijk niet OK */
			if(har->type & HA_ONLYSKY) {
				if(har->rad>3.0) har->rad= 3.0;
			}
			
			har->radsq= har->rad*har->rad;

			har->miny= har->ys - har->rad/R.ycor;
			har->maxy= har->ys + har->rad/R.ycor;

	/* de Zd is bij pano nog steeds verkeerd: zie testfile in blenderbugs/halo+pano.blend */

			vec[2]-= har->hasize;	/* z is negatief, wordt anders geclipt */
			projectfunc(vec, hoco);
			zn= hoco[3];
			har->zd= abs(har->zs - 0x7FFFFF*(1.0+hoco[2]/zn) );
			
			/* if( har->zs < 2*har->zd) { */
				/* PRINT2(d, d, har->zs, har->zd); */
				/* har->alfa= har->mat->alpha * ((float)(har->zs))/(float)(2*har->zd); */
			/* } */
			
		}

	}

	/* vlaggen op 0 zetten als eruit geclipt */
	for(a=0; a<R.totvlak; a++) {
		if((a & 255)==0) vlr= R.blovl[a>>8];
		else vlr++;
		
		if(vlr->flag & R_CMAPCODE) {	
			;	/* niet helemaal nette hotspot patch */
		}
		else {
			vlr->flag |= R_VISIBLE;
			if(vlr->v4) {
				if(vlr->v1->clip & vlr->v2->clip & vlr->v3->clip & vlr->v4->clip) vlr->flag &= ~R_VISIBLE;
			}
			else if(vlr->v1->clip & vlr->v2->clip & vlr->v3->clip) vlr->flag &= ~R_VISIBLE;
			
		}
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

struct halosort {
	HaloRen *har;
	ulong z;
};

long verghalo(x1, x2)
struct halosort *x1, *x2;
{

	if( x1->z < x2->z ) return 1;
	else if( x1->z > x2->z) return -1;
	return 0;
}


void sort_halos()
{
	struct halosort *hablock, *haso;
	HaloRen *har, **bloha;
	int a;
	
	if(R.tothalo==0) return;
	
	/* datablok maken met halopointers, sorteren */
	haso= hablock= mallocN(8*R.tothalo, "hablock");
	
	for(a=0; a<R.tothalo; a++) {
		if((a & 255)==0) har= R.bloha[a>>8]; else har++;
		haso->har= har;
		haso->z= har->zs;
		haso++;
	}
	
	qsort(hablock, R.tothalo, 8, (void *)verghalo);
	
	/* opnieuw samenstellen van R.bloha */
	
	bloha= R.bloha;
	R.bloha= (HaloRen **)callocN(4*(MAXVERT>>8),"Bloha");
	
	haso= hablock;
	for(a=0; a<R.tothalo; a++) {
		har= addhalo(a);
		*har= *(haso->har);
		
		haso++;
	}
	
	/* vrijgeven */
	a= 0;
	while(bloha[a]) {
		freeN(bloha[a]);
		a++;
	}
	freeN(bloha);
	freeN(hablock);
	
}

void setpanorot(int part)
{
	extern float panovco, panovsi;
	VertRen *ver;
	VlakRen *vlr;
	HaloRen *har;
	static float alpha;
	float si, co, x, z;
	int a;
		
	/* part==0 alles initialiseren */
	
	if(part==0) {
	
		alpha= ((float)R.r.xsch)/R.viewfac;
		alpha= 2.0*fatan(alpha/2.0);
	}
	
	
	/* we roteren alles om de y-as met phi graden */
	
	panophi= -0.5*(R.r.xparts-1)*alpha + part*alpha;

	panovsi= fsin(-panophi);
	panovco= fcos(-panophi);
	
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void make_existing_file(char *name)
{
	char di[120], fi[80];
	int i, len;
	
	strcpy(di, name);
	splitdirstring(di, fi);

	/* exist testen */
	if (exist(di) == 0) {
		recurdir(di);
	}
}

void schrijfplaatje(name)
char *name;
{
	extern float rgb_to_bw[];
	extern int alpha_to_col0(int);
	struct ImBuf *ibuf=0;
	ulong *temprect=0;
	int i, flags;
	char str[100];
	
	/* Staat RGBA aan? Zo ja: gebruik alphakanaal voor kleur 0 */

	alpha_to_col0(FALSE);

	if(R.r.planes == 32) {
			/* alles met minder dan 50 % alpha -> col 0 */
		if(R.r.alphamode == R_ALPHAKEY) alpha_to_col0(2);
			/* uitsluitend met 0 alpha -> col 0 */
		else alpha_to_col0(1);
	}

	if ELEM(R.r.imtype, R_FTYPE, R_CMBB) {
		strcpy(str, R.r.ftype);
		convertstringcode(str);

		ibuf = loadiffname(str, IB_test);
		if(ibuf) {
			ibuf->x = R.rectx;
			ibuf->y = R.recty;
		}
		else {
			error("Can't find filetype");
			G.afbreek= 1;
			return;
		}
		setdither(2);
	}
	
	if(ibuf == 0) {
		ibuf= allocImBuf(R.rectx, R.recty, R.r.planes, 0, 0);
	}

	if(ibuf) {
		ibuf->rect= (ulong *) R.rectot;

		if(R.r.planes == 8) cspace(ibuf, rgb_to_bw);
		
		if(R.r.imtype== R_IRIS) ibuf->ftype= IMAGIC;
		else if(R.r.imtype==R_IRIZ) {
			ibuf->ftype= IMAGIC;
			if (ibuf->zbuf == 0) {
				if (izbuf) {
					if (izbuf->zbuf && ibuf->x == izbuf->x && ibuf->y == izbuf->y) {
						ibuf->zbuf = izbuf->zbuf;
					} else printf("incompatible zbuf\n");
				} else printf("nozbuf\n");
			}
		} 
		else if ELEM3(R.r.imtype, R_TARGA, R_MDEC, R_TGA4) {
			ibuf->ftype= TGA;
		}
		else if(R.r.imtype == R_HAMX) {
			/* kopie maken */
			temprect= dupallocN(R.rectot);
			ibuf->ftype= AN_hamx;
		}
		else if(R.r.imtype==R_TANX) {
			temprect= dupallocN(R.rectot);
			ibuf->ftype= AN_tanx;
		}
		else if ELEM(R.r.imtype, R_JPEG90, R_MOVIE) {
			if(R.r.quality < 10) R.r.quality= 90;
			
			if(R.r.mode & R_FIELDS) ibuf->ftype= JPG_VID|R.r.quality;
			else ibuf->ftype= JPG|R.r.quality;
			
		}
		
		make_existing_file(name);

		if ELEM(R.r.imtype, R_TGA4, R_HAMX4) write_fourparts(ibuf, name);
		else if(R.r.imtype==R_CMBB) write_cmbb(ibuf, name);
		else if(saveiff(ibuf, name, IB_rect | IB_zbuf)==0) {
			perror(name);
			G.afbreek= 1;
		}

		freeImBuf(ibuf);
		
		if ELEM(R.r.imtype, R_HAMX, R_TANX) {
			freeN(R.rectot);
			R.rectot= temprect;
		}
	}
	else {
		G.afbreek= 1;
	}
}

void write_image(char *name)
{
	/* vanuit filesel */
	char str[256];
	
	strcpy(str, name);
	convertstringcode(str);
	
	if(saveover(str)) {
		if(testextensie(str,".blend")) {
			error("Wrong filename");
			return;
		}
		waitcursor(1);
		schrijfplaatje(str);
		strcpy(G.ima, name);
		waitcursor(0);
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setwindowclip(mode, jmode)
short mode,jmode;
{
	/* jmode>=0: alleen jitter doen, anders berekenen  */
	/* mode==1 zet persmat en grvec */
	Camera *cam=0;
	Lamp *la=0;
	float lens, fac, minx, miny, maxx, maxy;
	float xd, yd;
	
	if(G.scene->camera==0) return;
	
	if(mode) {
	
		if(G.scene->camera->type==OB_LAMP) {
			la=  G.scene->camera->data;
			/* fac= fcos( PI*((float)(256- la->spsi))/512.0 ); */
			
			/* phi= facos(fac); */
			/* lens= 16.0*fac/fsin(phi); */
			lens= 35.0;
			R.near= 0.1;
			R.far= 1000.0;
		}
		else if(G.scene->camera->type==OB_CAMERA) {
			cam= G.scene->camera->data;
			
			lens= cam->lens;
			R.near= cam->clipsta;
			R.far= cam->clipend;
		}
		else {
			lens= 35.0;
			R.near= 0.1;
			R.far= 1000.0;
		}

		if(R.r.xasp*R.afmx>=R.r.yasp*R.afmy) {
			R.viewfac= (R.afmx*lens)/16.0;
		}
		else {
			R.viewfac= R.ycor*(R.afmy*lens)/16.0;
		}
		if(R.r.mode & R_ORTHO) {
			R.near*= 100.0; /* R.far niet doen */
			R.viewfac*= 100.0;
		}
		
		R.pixsize= R.near/R.viewfac;
	
	}


	minx= R.xstart+.5;
	miny= R.ycor*(R.ystart+.5);
	maxx= R.xend+.4999;
	maxy= R.ycor*(R.yend+.4999);

	if(R.flag & R_SEC_FIELD) {
		if(R.r.mode & R_ODDFIELD) {
			miny-= .5*R.ycor;
			maxy-= .5*R.ycor;
		}
		else {
			miny+= .5*R.ycor;
			maxy+= .5*R.ycor;
		}
	}

	xd= yd= 0.0;
	if(jmode!= -1) {
		xd= jit[jmode % R.osa][0];
		yd= R.ycor*jit[jmode % R.osa][1];

	}
	
	minx= R.pixsize*(minx+xd);
	maxx= R.pixsize*(maxx+xd);
	miny= R.pixsize*(miny+yd);
	maxy= R.pixsize*(maxy+yd);

	if(R.r.mode & R_ORTHO) {
		/* hier de near & far vermenigvuldigen is voldoende! */
		
		i_window(minx, maxx, miny, maxy, R.near, 100.0*R.far, R.winmat);
	}
	else  i_window(minx, maxx, miny, maxy, R.near, R.far, R.winmat);

}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void initparts()
{
	short nr, xd, yd, xpart, ypart, xparts, yparts;
	short a, xminb, xmaxb, yminb, ymaxb;

	if(R.r.mode & R_BORDER) {
		xminb= R.r.border.xmin*R.rectx;
		xmaxb= R.r.border.xmax*R.rectx;

		yminb= R.r.border.ymin*R.recty;
		ymaxb= R.r.border.ymax*R.recty;

		if(xminb<0) xminb= 0;
		if(xmaxb>R.rectx) xmaxb= R.rectx;
		if(yminb<0) yminb= 0;
		if(ymaxb>R.recty) ymaxb= R.recty;
	}
	else {
		xminb=yminb= 0;
		xmaxb= R.rectx;
		ymaxb= R.recty;
	}

	xparts= R.r.xparts;	/* voor border */
	yparts= R.r.yparts;

	for(nr=0;nr<xparts*yparts;nr++)
		allparts[nr][0]= -1;	/* array leegmaken */

	xpart= R.rectx/xparts;
	ypart= R.recty/yparts;

	/* als border: testen of aantal parts minder kan */
	if(R.r.mode & R_BORDER) {
		a= (xmaxb-xminb-1)/xpart+1; /* zoveel parts in border */
		if(a<xparts) xparts= a;
		a= (ymaxb-yminb-1)/ypart+1; /* zoveel parts in border */
		if(a<yparts) yparts= a;

		xpart= (xmaxb-xminb)/xparts;
		ypart= (ymaxb-yminb)/yparts;
	}

	for(nr=0; nr<xparts*yparts; nr++) {

		if(R.r.mode & R_PANORAMA) {
			allparts[nr][0]= 0;
			allparts[nr][1]= 0;
			allparts[nr][2]= R.rectx;
			allparts[nr][3]= R.recty;
		}
		else {
			xd= (nr % xparts);
			yd= (nr-xd)/xparts;
	
			allparts[nr][0]= xminb+ xd*xpart;
			allparts[nr][1]= yminb+ yd*ypart;
			if(xd<R.r.xparts-1) allparts[nr][2]= allparts[nr][0]+xpart;
			else allparts[nr][2]= xmaxb;
			if(yd<R.r.yparts-1) allparts[nr][3]= allparts[nr][1]+ypart;
			else allparts[nr][3]= ymaxb;
	
			if(allparts[nr][2]-allparts[nr][0]<=0) allparts[nr][0]= -1;
			if(allparts[nr][3]-allparts[nr][1]<=0) allparts[nr][0]= -1;
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
short setpart(nr)	/* return 0 als geen goede part */
short nr;
{
	short xd, yd, xpart, ypart;

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
Part *part;
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
	Base *base;
	ShadBuf *shb;
	Object *ob;
	Mesh *me;
	Curve *cu;
	MetaBall *mb;
	DispList *dl;
	ulong *ztile, lay;
	int a, b, v;
	char *ctile;

	/* VRIJGEVEN */

	for(a=0; a<R.totlamp; a++) {
		if(R.la[a]->shb) {
			shb= R.la[a]->shb;
			v= (shb->size*shb->size)/256;
			ztile= shb->zbuf;
			ctile= shb->cbuf;
			for(b=0; b<v; b++, ztile++, ctile++) {
				if(*ctile) freeN((void *) *ztile);
			}
			freeN(shb->zbuf);
			freeN(shb->cbuf);
			freeN(R.la[a]->shb);
		}
		if(R.la[a]->org) freeN(R.la[a]->org);
		freeN(R.la[a]);
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
	
	/* layers: in foreground current 3D window renderen */
	lay= G.scene->lay;
	if(G.vd) lay= G.vd->lay;
	
	/* orco vrijgeven. ALle ob's aflopen ivm dupli's en sets */
	ob= G.main->object.first;
	while(ob) {
	
		if ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT) {
			cu= ob->data;
			if(cu->orco) {
				freeN(cu->orco);
				cu->orco= 0;
			}
		}
		else if(ob->type==OB_MESH) {
			me= ob->data;
			if(me->orco) {
				freeN(me->orco);
				me->orco= 0;
			}
		}
		else if(ob->type==OB_MBALL) {
			if(ob->disp.first && ob->disp.first!=ob->disp.last) {
				dl= ob->disp.first;
				remlink(&ob->disp, dl);
				freedisplist(&ob->disp);
				addtail(&ob->disp, dl);
			}
		}
		ob= ob->id.next;
	}

	end_render_textures();
	end_render_materials();

	R.totvlak=R.totvert=R.totlamp=R.tothalo= 0;
}


extern unsigned char hash[512];

/* - er moet een 'vast' aantal sterren gegenereerd worden tussen near en far.
 * - alle sterren moeten bij voorkeur op de far liggen en uitsluitend in
 *   helderheid / kleur verschillen.
 * - 
 */

void make_stars(int wire)
{
	HaloRen *har;
	double dblrand, hlfrand;
	float vec[4], fx, fy, fz;
	float fac, starmindist, clipend;
	float mat[4][4], stargrid, maxrand, far, near, force, alpha;
	int x, y, z, sx, sy, sz, ex, ey, ez, maxjit, done = 0;
	ushort seed[3];
	Camera * camera;
	int first = TRUE;
	
	if(wire) R.wrld= *(G.scene->world);
	
	stargrid = R.wrld.stardist;		/* om de hoeveel een ster ? */
	maxrand = 2.0;						/* hoeveel mag een ster verschuiven (uitgedrukt in grid eenheden) */
	maxjit = (256.0* R.wrld.starcolnoise);			/* hoeveel mag een kleur veranderen */

	far = R.far;
	near = R.near;
	
	
	/* afmeting sterren */
	force = ( R.wrld.starsize );
	
	/* minimale vrije ruimte */
	starmindist= R.wrld.starmindist;

	if (stargrid <= 0.10) return;
	
	if (wire == 0) R.flag |= R_HALO;
	else stargrid *= 1.0;				/* tekent er minder */
	
	
	Mat4Invert(mat, R.viewmat);
	
	/* BOUNDINGBOX BEREKENING
	 * bbox loopt van z = near | far,
	 * x = -z | +z,
	 * y = -z | +z
	 */
	 
	camera = G.scene->camera->data;
	clipend = camera->clipend;
	
	/* omzetten naar grid coordinaten */
	
	sx = ((mat[3][0] - clipend) / stargrid) - maxrand;
	sy = ((mat[3][1] - clipend) / stargrid) - maxrand;
	sz = ((mat[3][2] - clipend) / stargrid) - maxrand;

	ex = ((mat[3][0] + clipend) / stargrid) + maxrand;
	ey = ((mat[3][1] + clipend) / stargrid) + maxrand;
	ez = ((mat[3][2] + clipend) / stargrid) + maxrand;
	
	dblrand = maxrand * stargrid;
	hlfrand = 2.0 * dblrand / RAND_MAX;

	if (wire) {
		cpack(-1);
		bgnpoint();
	}
	
	for (x = sx, fx = sx * stargrid; x <= ex; x++, fx += stargrid) {
		for (y = sy, fy = sy * stargrid; y <= ey ; y++, fy += stargrid) {
			for (z = sz, fz = sz * stargrid; z <= ez; z++, fz += stargrid) {
				
				srand((hash[z & 0xff] << 24) + (hash[y & 0xff] << 16) + (hash[x & 0xff] << 8));
				vec[0] = fx + (hlfrand * rand()) - dblrand;
				vec[1] = fy + (hlfrand * rand()) - dblrand;
				vec[2] = fz + (hlfrand * rand()) - dblrand;
				vec[3] = 1.0;

				if (wire) {
					v3f(vec);
					done++;
				}
				else {
					Mat4MulVecfl(R.viewmat, vec);
					
					/* in vec staan globale coordinaten
					 * bereken afstand tot de kamera
					 * en bepaal aan de hand daarvan de alpha
					 */
					
					{
						float tx, ty, tz;
						
						tx = vec[0];
						ty = vec[1];
						tz = vec[2];
						
						alpha = fsqrt(tx * tx + ty * ty + tz * tz);
						
						if (alpha >= clipend) alpha = 0.0;
						else if (alpha <= starmindist) alpha = 0.0;
						else if (alpha <= 2.0 * starmindist) {
							alpha = (alpha - starmindist) / starmindist;
						} else {
							alpha -= 2.0 * starmindist;
							alpha /= (clipend - 2.0 * starmindist);
							alpha = 1.0 - alpha;
						}
					}
						
					
					if (alpha != 0.0) {				
						fac = force * rand();
						fac /= RAND_MAX;
						har = initstar(vec, fac);
						
						if (har) {
							har->alfa = fsqrt(fsqrt(alpha));
							har->add= 255;
							har->r = har->g = har->b = 255;
							if (maxjit) {
								har->r += ((maxjit * rand()) / RAND_MAX) - maxjit;
								har->g += ((maxjit * rand()) / RAND_MAX) - maxjit;
								har->b += ((maxjit * rand()) / RAND_MAX) - maxjit;
							}
							har->hard = 32;
							
							har->type |= HA_ONLYSKY;
							done++;
						}
					}
				}
			}
			if(done > MAXVERT) {
				printf("Too many stars\n");
				break;
			}
			if(test_break()) break;
		}
		
		if(done > MAXVERT) break;
		
		if(test_break()) break;
	}
	
	if (wire) endpoint();
}

void init_render_object(Object *ob)
{
	float mat[4][4];

	ob->flag |= OB_DONE;

	if(ob->type==OB_LAMP) 
		add_render_lamp(ob, 1);
	else if ELEM(ob->type, OB_FONT, OB_CURVE)
		init_render_curve(ob);
	else if(ob->type==OB_SURF)
		init_render_surf(ob);
	else if(ob->type==OB_MESH)
		init_render_mesh(ob);
	else if(ob->type==OB_MBALL)
		init_render_mball(ob);
	else {
		Mat4MulMat4(mat, ob->obmat, R.viewmat);
		Mat4Invert(ob->imat, mat);
	}
}

void roteerscene()
{
	extern int slurph_opt;	/* key.c */
	Base *base;
	Object *ob;
	Scene *sce;
	Camera *cam;
	VlakRen *vlr;
	ulong lay;
	float vec[3], xn, yn, zn, mat[4][4];
	int a1;
	
	if(G.scene->camera==0) return;
	
	O.dxwin[0]= 0.5/(float)R.r.xsch;
	O.dywin[1]= 0.5/(float)R.r.ysch;

	slurph_opt= 0;

	R.totvlak=R.totvert=R.totlamp=R.tothalo= 0;

	do_all_ipos();
	do_all_keys();
	do_all_ikas();
	test_all_displists();
	
	/* niet erg nette calc_ipo en where_is forceer */
	ob= G.main->object.first;
	while(ob) {
		ob->ctime= -123.456;
		ob= ob->id.next;
	}
	
	/* cameranetwerk? */
	if(R.r.scemode & R_NETWORK) render_frame_to_camera();

	/* ivm met optimale berekening track / lattices / etc: extra where_is_ob */ 
	base= FIRSTBASE;
	while(base) {
		where_is_object(base->object);
		if(base->next==0 && G.scene->set && base==G.scene->base.last) base= G.scene->set->base.first;
		else base= base->next;
	}	
	
	Mat4CpyMat4(R.viewinv, G.scene->camera->obmat);
	Mat4Ortho(R.viewinv);
	Mat4Invert(R.viewmat, R.viewinv);
	
	/* is niet netjes: nu is de viewinv ongelijk aan de viewmat. voor Texco's enzo. Beter doen! */
	if(R.r.mode & R_ORTHO) R.viewmat[3][2]*= 100.0;
		
	setwindowclip(1,-1); /*  geen jit:(-1) */

	/* imatflags wissen */
	ob= G.main->object.first;
	while(ob) {
		ob->flag &= ~OB_DO_IMAT;
		ob= ob->id.next;
	}

	init_render_world();	/* moet eerst ivm ambient */
	init_render_textures();
	init_render_materials();
	
	/* MAAK RENDERDATA */

	/* elk object maar 1 x renderen */
	ob= G.main->object.first;
	while(ob) {
		ob->flag &= ~OB_DONE;	
		ob= ob->id.next;
	}
	

	/* layers: in foreground current 3D window renderen */
	lay= G.scene->lay;
	if(G.vd) lay= G.vd->lay;
	sce= G.scene;
	
	base= FIRSTBASE;
	while(base) {
		
		if(base->object->flag & OB_DONE);
		else {
			
			where_is_object(base->object);
			
			if( (base->lay & lay) || (base->object->type==OB_LAMP && (base->lay & G.scene->lay)) ) {
				
				if(base->object->infoflag && (R.flag & R_ANIMRENDER)) info_file(base->object);
				
				if(base->object->transflag & OB_DUPLI) {
					extern ListBase duplilist;
					
					make_duplilist(sce, base->object);
					ob= duplilist.first;
					while(ob) {
						init_render_object(ob);
						ob= ob->id.next;
					}
					free_duplilist();
				}
				else init_render_object(base->object);
				
			}
			else {
				Mat4MulMat4(mat, base->object->obmat, R.viewmat);
				Mat4Invert(base->object->imat, mat);
			}
			
			base->object->flag &= ~OB_DO_IMAT;
		}
		if(test_break()) break;
		
		if(base->next==0 && G.scene->set && base==G.scene->base.last) {
			base= G.scene->set->base.first;
			sce= G.scene->set;
		}
		else base= base->next;
		
	}

	/* infofile sluiten */
	info_file(0);
	
	/* imat objecten */
	ob= G.main->object.first;
	while(ob) {
		if(ob->flag & OB_DO_IMAT) {
			
			ob->flag &= ~OB_DO_IMAT;
			
			Mat4MulMat4(mat, ob->obmat, R.viewmat);
			Mat4Invert(ob->imat, mat);
		}
		ob= ob->id.next;
	}

	sort_halos();

	if(R.wrld.mode & WO_STARS) make_stars(0);
	
	slurph_opt= 1;
	
	if(test_break()) return;
	
	/* if(R.totlamp==0) defaultlamp(); */
	
	/* KLAP NORMAAL EN SNIJ PROJECTIE */
	for(a1=0; a1<R.totvlak; a1++) {
		if((a1 & 255)==0) vlr= R.blovl[a1>>8];
		else vlr++;
		
		if(vlr->flag & R_NOPUNOFLIP) ;
		else {
			vec[0]= vlr->v1->co[0];
			vec[1]= vlr->v1->co[1];
			vec[2]= vlr->v1->co[2];
	
			if( (vec[0]*vlr->n[0] +vec[1]*vlr->n[1] +vec[2]*vlr->n[2])<0.0 ) {
				vlr->puno= ~(vlr->puno);
				vlr->n[0]= -vlr->n[0];
				vlr->n[1]= -vlr->n[1];
				vlr->n[2]= -vlr->n[2];
			}
		}
		xn= fabs(vlr->n[0]);
		yn= fabs(vlr->n[1]);
		zn= fabs(vlr->n[2]);
		if(zn>=xn && zn>=yn) vlr->snproj= 0;
		else if(yn>=xn && yn>=zn) vlr->snproj= 1;
		else vlr->snproj= 2;
		
	}

}

/* cmapcode handleiding:
 * 
 * 
 * - globale optie: R.r.scemode & R_HOTSPOT
 * - visible truuk: vlr->flag R_VISIBLE vantevoren zetten, functie zbufvlaggen!
 * - ob->infosta == hotspot nr
 * - ob->infoend == don't render
 */


void add_cmapcode()
{
	ImBuf *ibuf;
	ulong len, *temprect, *rt;
	char name[256];
	
	temprect= R.rectot;
	if(temprect==0) return;
	
	len= R.rectx*R.recty;
	R.rectot= callocN(4*len, "rectcmap");
	
	zbufferall_cmapcode();

	/* kleurnummers als cmap wegschrijven */

	rt= R.rectot;
	
	ibuf= allocImBuf(R.rectx, R.recty, 8, 0, 0);
	ibuf->rect= R.rectot;
	
	sprintf(name, "%scode%04d", G.scene->r.pic, CFRA);
	
	saveiff(ibuf, name, IB_rect);
	printf("Saved: %s\n", name);
	
	freeImBuf(ibuf);
	freeN(R.rectot);
	R.rectot= temprect;
}

void add_to_blurbuf(blur)
{
	static ulong *blurrect= 0;
	int tot, gamval;
	short facr, facb;
	char *rtr, *rtb;
		
	if(blur<0) {
		if(blurrect) {
			if(R.rectot) freeN(R.rectot);
			R.rectot= blurrect;
			blurrect= 0;
		}
	}
	else if(blur==R.osa-1) {
		/* eerste keer */
		blurrect= mallocN(R.rectx*R.recty*4, "rectblur");
		if(R.rectot) memcpy(blurrect, R.rectot, R.rectx*R.recty*4);
	}
	else if(blurrect) {
		/* accumuleren */
		
		facr= 256/(R.osa-blur);
		facb= 256-facr;

		if(R.rectot) {
			rtr= (char *)R.rectot;
			rtb= (char *)blurrect;
			tot= R.rectx*R.recty;
			while(tot--) {
				if( *((ulong *)rtb) != *((ulong *)rtr) ) {
					
					if(R.r.mode & R_GAMMA) {
						gamval= (facr* igamtab2[ rtr[0]<<8 ] + facb* igamtab2[ rtb[0]<<8 ])>>8;
						rtb[0]= gamtab[ gamval ]>>8;
						gamval= (facr* igamtab2[ rtr[1]<<8 ] + facb* igamtab2[ rtb[1]<<8 ])>>8;
						rtb[1]= gamtab[ gamval ]>>8;
						gamval= (facr* igamtab2[ rtr[2]<<8 ] + facb* igamtab2[ rtb[2]<<8 ])>>8;
						rtb[2]= gamtab[ gamval ]>>8;
						gamval= (facr* igamtab2[ rtr[3]<<8 ] + facb* igamtab2[ rtb[3]<<8 ])>>8;
						rtb[3]= gamtab[ gamval ]>>8;
					}
					else {
						rtb[0]= (facr*rtr[0] + facb*rtb[0])>>8;
						rtb[1]= (facr*rtr[1] + facb*rtb[1])>>8;
						rtb[2]= (facr*rtr[2] + facb*rtb[2])>>8;
						rtb[3]= (facr*rtr[3] + facb*rtb[3])>>8;
					}
				}
				rtr+= 4;
				rtb+= 4;
			}
		}
		if(blur==0) {
			/* laatste keer */
			if(R.rectot) freeN(R.rectot);
			R.rectot= blurrect;
			blurrect= 0;
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void render()  /* hierbinnen de PART en FIELD lussen */
{
	Part *part;
	float mat[4][4];
	ulong *rt, *rt1, *rt2;
	int file1,file2,len,a1, y;
	short blur, a,fields,fi,parts;  /* pa is globaal ivm print */


	if (izbuf) freeImBuf(izbuf);
	izbuf = 0;

	/* FIELDLUS */
	fields= 1;
	parts= R.r.xparts*R.r.yparts;
	
	if(R.r.mode & R_FIELDS) {
		fields= 2;
		R.rectf1= R.rectf2= 0;	/* fieldrecten */
		R.r.ysch/= 2; 
		R.afmy/= 2;
		R.r.yasp*= 2;
		R.ycor= ( (float)R.r.yasp)/( (float)R.r.xasp);
		
	}
	
	for(fi=0; fi<fields; fi++) {

		/* INIT */
		srand( 2*CFRA+fi);
		
		R.vlaknr= -1;
		R.flag|= R_RENDERING;
		if(fi==1) R.flag |= R_SEC_FIELD;

		/* MOTIONBLUR lus */
		if(R.r.mode & R_MBLUR) blur= R.osa;
		else blur= 1;
		
		while(blur--) {

			/* WINDOW */
			R.rectx= R.r.xsch; 
			R.recty= R.r.ysch;
			R.xstart= -R.afmx; 
			R.ystart= -R.afmy;
			R.xend= R.xstart+R.rectx-1;
			R.yend= R.ystart+R.recty-1;
	
			if(R.r.mode & R_MBLUR) set_mblur_offs(R.osa-blur);
			
			initparts(); /* altijd doen ivm border */
			setpart(0);
			
			init_render_display();
			clear_render_display();
			set_timecursor(CFRA);

			/* ROTEERSCENE */
			
			if(R.rectot) freeN(R.rectot); R.rectot= 0;
			if(R.rectz) freeN(R.rectz); R.rectz= 0;
	
			roteerscene();
	
			/* SCHADUWBUFFER */
			for(a=0; a<R.totlamp; a++) {
				if(test_break()) break;
				if(R.la[a]->shb) makeshadowbuf(R.la[a]);
			}
			
			/* PARTS */
			R.parts.first= R.parts.last= 0;
			for(pa=0; pa<parts; pa++) {
			
				if(test_break()) break;
				
				if(pa) {	/* want pa==0 is al gedaan */
					if(setpart(pa)==0) break;
				}
				
				if(R.r.mode & R_MBLUR) setwindowclip(0, blur);
				else setwindowclip(0,-1);
				
				if(R.r.mode & R_PANORAMA) setpanorot(pa);
	
				/* HOMOGENE COORDINATEN EN ZBUF EN CLIP OPT (per part) */
				setzbufvlaggen(projectverto);
				if(test_break()) break;
	
				/* ZBUFFER & SHADE */
				R.rectot= (ulong *)callocN(4*R.rectx*R.recty, "rectot");
				R.rectz=  (ulong *)mallocN(4*R.rectx*R.recty, "rectz");
				
				if(R.r.mode & R_MBLUR) printrenderinfo(R.osa - blur);
				else printrenderinfo(-1);
	
				if(R.r.mode & R_OSA) zbufshadeDA();
				else zbufshade();
	
				if(test_break()) break;
	
				/* uitzondering */
				if( (R.r.mode & R_BORDER) && (R.r.mode & R_MOVIECROP));
				else {
					/* PART OF BORDER AFHANDELEN */
					if(parts>1 || (R.r.mode & R_BORDER)) {
						
						part= callocN(sizeof(Part), "part");
						addtail(&R.parts, part);
						part->rect= R.rectot;
						R.rectot= 0;
						
						freeN(R.rectz);
						R.rectz= 0;
					}
				}
			}
	
			/* PARTS SAMENVOEGEN OF BORDER INVOEGEN */
	
			/* uitzondering: crop */
			if( (R.r.mode & R_BORDER) && (R.r.mode & R_MOVIECROP)) {
			}
			else {
				R.rectx= R.r.xsch; 
				R.recty= R.r.ysch;
		
				if(R.r.mode & R_PANORAMA) R.rectx*= R.r.xparts;
		
				if(parts>1 || (R.r.mode & R_BORDER)) {
					if(R.rectot) freeN(R.rectot);
					if(R.r.mode & R_BORDER) R.rectot=(ulong *)callocN(4*R.rectx*R.recty, "rectot");
					else R.rectot=(ulong *)mallocN(4*R.rectx*R.recty, "rectot");
		
					part= R.parts.first;
					for(pa=0; pa<parts; pa++) {
						if(allparts[pa][0]== -1) break;
						if(part==0) break;
						
						if(R.r.mode & R_PANORAMA) {
							if(pa) {
								allparts[pa][0] += pa*R.r.xsch;
								allparts[pa][2] += pa*R.r.xsch;
							}
						}
						addparttorect(pa, part);
						
						part= part->next;
					}
				
					part= R.parts.first;
					while(part) {
						freeN(part->rect);
						part= part->next;
					}
					freelistN(&R.parts);
				}
			}
	
			if(R.flag & R_HALO) {
				add_halo_flare();
			}
	
			if(R.r.scemode & R_HOTSPOT) {
				add_cmapcode();
			}
			
			if(R.r.mode & R_MBLUR) {
				add_to_blurbuf(blur);
			}

			/* EINDE (blurlus) */
			freeroteerscene();
	
			if(test_break()) break;
		}
	
		/* definitief vrijgeven */
		add_to_blurbuf(-1);

		/* FIELD AFHANDELEN */
		if(R.r.mode & R_FIELDS) {
			if(R.flag & R_SEC_FIELD) R.rectf2= R.rectot;
			else R.rectf1= R.rectot;
			R.rectot= 0;
		}
		
		if(test_break()) break;
	}

	/* FIELDS SAMENVOEGEN */
	if(R.r.mode & R_FIELDS) {
		R.r.ysch*= 2; 
		R.afmy*= 2;
		R.recty*= 2;
		R.r.yasp/=2;

		if(R.rectot) freeN(R.rectot);	/* komt voor bij afbreek */
		R.rectot=(ulong *)mallocN(4*R.rectx*R.recty, "rectot");
		
		if(test_break()==0) {
			rt= R.rectot;
			
			if(R.r.mode & R_ODDFIELD) {
				rt2= R.rectf1;
				rt1= R.rectf2;
			}
			else {
				rt1= R.rectf1;
				rt2= R.rectf2;
			}
			
			len= 4*R.rectx;
			
			for(a=0; a<R.recty; a+=2) {
				memcpy(rt, rt1, len);
				rt+= R.rectx;
				rt1+= R.rectx;
				memcpy(rt, rt2, len);
				rt+= R.rectx;
				rt2+= R.rectx;
			}
		}
	}

	/* R.rectx= R.r.xsch; */
	/* if(R.r.mode & R_PANORAMA) R.rectx*= R.r.xparts; */
	/* R.recty= R.r.ysch; */

	/* als border: wel de skybuf doen */
	if(R.r.mode & R_BORDER) {
		if( (R.r.mode & R_MOVIECROP)==0) {
			if(R.r.bufflag & 1) {
				R.xstart= -R.afmx;
				R.ystart= -R.afmy;
				rt= R.rectot;
				for(y=0; y<R.recty; y++, rt+= R.rectx) scanlinesky(rt, y); 
			}
		}
	}

	set_mblur_offs(0);

	/* VRIJGEVEN */
	
	/* zbuf test */


/*
	{
		ImBuf * ibuf;
		int i;
		
		ibuf = allocImBuf(R.rectx, R.recty, 32, 0, 0);
		ibuf->rect = R.rectz;
		
		saveiff(ibuf, "/tmp/z32", IB_rect);
		
		for (i = (ibuf->x * ibuf->y) - 1; i >=0 ;i--) ibuf->rect[i] >>= 20;
		ibuf->depth = 8;

		saveiff(ibuf, "/tmp/z8", IB_rect);
		
		freeImBuf(ibuf);
	}
*/	
	if (R.r.imtype == R_IRIZ) {
		if (R.rectz && parts == 1 && (R.r.mode & R_FIELDS) == 0) {
			izbuf = allocImBuf(R.rectx, R.recty, 32, IB_zbuf, 0);
			memcpy(izbuf->zbuf, R.rectz, R.rectx * R.recty * sizeof(ulong));
		}
	}
	
	if(R.rectz) freeN(R.rectz);
	R.rectz= 0;
	
	if(R.rectf1) freeN(R.rectf1);
	R.rectf1= 0;
	if(R.rectf2) freeN(R.rectf2);
	R.rectf2= 0;

}

/* ~~~~~~~~~~~~~~~~ timer ~~~~~~~~~~~~~~~~~~~~~~ */

struct rusage rus1, rus2;
struct timeval tval1, tval2;

void start_timer()
{
	struct timezone rt;
	
	getrusage(RUSAGE_SELF, &rus1);
	gettimeofday(&tval1, &rt);
}

void end_timer(long *real, long *cpu)
{
	/* in hondersten van seconde */
	int rt1, rt2;
	struct timezone rt;
	
	getrusage(RUSAGE_SELF, &rus2);
	gettimeofday(&tval2, &rt);	

	rt1= 100*rus1.ru_utime.tv_sec + rus1.ru_utime.tv_usec/10000;
	rt1+= 100*rus1.ru_stime.tv_sec + rus1.ru_stime.tv_usec/10000;
	
	rt2= 100*rus2.ru_utime.tv_sec + rus2.ru_utime.tv_usec/10000;
	rt2+= 100*rus2.ru_stime.tv_sec + rus2.ru_stime.tv_usec/10000;

	if(cpu) *cpu= rt2-rt1;

	rt1= 100*tval1.tv_sec + tval1.tv_usec/10000;
	rt2= 100*tval2.tv_sec + tval2.tv_usec/10000;
	
	if(real) *real= rt2-rt1;
	
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


void initrender()
{
	Image *bima;
	extern ushort usegamtab;
	char name[256];

	/* scenedata naar R */
	R.r= G.scene->r;
	/* voor zekerheid: bij voortijdige return */
	R.rectx= R.r.xsch; 
	R.recty= R.r.ysch;
	
	/* verboden combinatie */
	if((R.r.mode & R_BORDER) && (R.r.mode & R_PANORAMA)) R.r.mode-= R_BORDER;

	/* MAG ER WEL WORDEN GERENDERD */
	if(R.r.xparts*R.r.yparts>64) {
		error("No more than 64 parts");
		G.afbreek= 1;
		return;
	}

	/* BACKBUF TESTEN */
	if(R.r.bufflag & 1) {
		if(R.r.alphamode == R_ADDSKY) {
			strcpy(name, R.r.backbuf);
			convertstringcode(name);
		
			if(R.backbuf) {
				R.backbuf->id.us--;
				bima= R.backbuf;
			}
			else bima= 0;
			
			R.backbuf= add_image(name);
			
			if(bima && bima->id.us<1) {
				free_image(bima);
			}
			if(R.backbuf==0) {
				error("No backbuf there!");
				G.afbreek= 1;
				return;
			}
		}
	}
	
	if (R.r.ftype[0] == '*') do_script(R.r.ftype + 1); /* FRANK*/
	
	usegamtab= 0; /* zie hieronder */
	
	if(R.r.mode & (R_OSA|R_MBLUR)) {
		R.osa= R.r.osa;
		if(R.osa>16) R.osa= 16;
		
		init_render_jit(R.osa);
		init_filt_mask();
		
			/* wordt af en toe tijdelijk op nul gezet, o.a. in transp zbuf */
		if(R.r.mode & R_GAMMA) {
			if((R.r.mode & R_MBLUR)==0) usegamtab= 1;
		}
	}
	else R.osa= 0;
	
	/* WINDOW */
	R.r.xsch= (R.r.size*R.r.xsch)/100;
	R.r.ysch= (R.r.size*R.r.ysch)/100;
	
	R.afmx= R.r.xsch/2;
	R.afmy= R.r.ysch/2;
	
	if(R.afmx<1 || R.afmy<1) {
		error("Image too small");
		return;
	}
	R.ycor= ( (float)R.r.yasp)/( (float)R.r.xasp);

	start_timer();

	if(R.r.scemode & R_DOSEQ) {
		R.rectx= R.r.xsch; 
		R.recty= R.r.ysch;

		if(R.rectot) freeN(R.rectot);
		R.rectot= (ulong *)callocN(4*R.rectx*R.recty, "rectot");

		do_render_seq();

		/* displayen */
		if(R.rectot) render_display(0, R.recty-1);
	}
	else {
		
		if(G.scene->camera==0) G.scene->camera= find_camera();
		
		if(G.scene->camera==0) {
			error("No camera");
		}
		else {
		
			if(G.scene->camera->type==OB_CAMERA) {
				Camera *cam= G.scene->camera->data;
				if(cam->type==CAM_ORTHO) R.r.mode |= R_ORTHO;
			}
			
			render(); /* keert terug met complete rect xsch-ysch */
		}
	}

	/* nog eens displayen: seq/parts/pano etc */
	init_render_display();
	if(R.rectot) render_display(0, R.recty-1);
	
	/* TIJD berekenen */
	end_timer(&G.time, &G.cputime);
	
	printrenderinfo(-1);

	/* variabelen weer goed */
	R.osatex= 0;
	R.vlr= 0;	/* bij cubemap */
	R.flag= 0;
}

void exit_render_stuff()
{

	if(G.afbreek==1) return;

	/* in de gaten houden: backgroundrender !!! */
	if(G.background) {
		if(G.scene->r.scemode & R_NETWORK) init_render_camera_network();
	}

	if(G.scene->r.imtype==R_MDEC) make_mdec_movies();
	
}

void animrender()
{
	int cfrao;
	char name[256];
	
	if(G.scene==0) return;

	/* scenedata naar R: (voor backbuf, R.rectx enz) */
	R.r= G.scene->r;

	/* START ANIMLUS overal wordt NIET de cfra uit R.r gebruikt: ivm rest blender */
	cfrao= CFRA;
	
	if(R.r.imtype==R_MOVIE) {
		R.rectx= (R.r.size*R.r.xsch)/100;
		R.recty= (R.r.size*R.r.ysch)/100;
		
#ifdef __sgi		
		start_movie();
#endif
	}
	
	/* in de gaten houden: backgroundrender !!! */
	if(R.r.scemode & R_NETWORK)	init_render_camera_network();

	for(CFRA=SFRA; CFRA<=EFRA; CFRA++) {
		
		R.flag= R_ANIMRENDER;
		
		initrender();

		/* SCHRIJF PLAATJE */
		if(test_break()==0) {

			if(R.r.imtype==R_MOVIE) {
#ifdef __sgi	
				append_movie(CFRA);
#endif
			}
			else {
				makepicstring(name, CFRA);
				schrijfplaatje(name);
				if(test_break()==0) printf("Saved: %s", name);
			}
		
			timestr(G.cputime, name);
			printf(" Time: %s (%.2f)\n", name, ((float)(G.time-G.cputime))/100);
			fflush(stdout); /* nodig voor renderd !! */
		}
		
		if(G.afbreek==1) break;

	}
	
	CFRA= cfrao;
	
	/* restoren tijd */
	if(R.r.mode & (R_FIELDS|R_MBLUR)) {
		do_all_ipos();
		do_all_keys();
		do_all_ikas();
	}
	
	if(R.r.imtype==R_MOVIE) end_movie();
	if(G.background==0) exit_render_stuff();	/* voor end_camera_net */	
	if(R.r.scemode & R_NETWORK)	end_render_camera_network();
}



