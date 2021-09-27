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

		sky()
		shadelamplus()
		shadepixel(x,y,rz,rp)

		scanlinehalo(rectz,rt,xs,ys,xpart)
		halovert(ver,rectp,rectz)

 		shadegour(vlr,rz)
		zbufpoly(type)
		zbufferall()

short 		keynatAli()
short 		zbufgourAli()
short 		zbufshadeAli()

short		keymat()
short 		zbufgour()
short 		zbufshade()

*/

/*#include <gl/feed.h>*/	/* werkt niet voor mips1 em is toch niet meer nodig */
#include "/usr/people/include/Trace.h"
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include "/usr/people/include/iff.h"

extern float Normalise();

extern float jit[64][2], jitweight[64];
extern float Zjitx,Zjity, Zmulx, Zmuly;
struct PixStrMain psmfirst;
short psmteller;

float holoofs, fmask[256], centLut[16];
ushort usegamtab=0, shortcol[4], *mask1[9], *mask2[9], *igamtab1, *igamtab2, *gamtab;
char cmask[256], *centmask;

struct Render R =
{
	{0,0,0},		/* long co[3]; */
	/* short lo[3],gl[3],uv[2],ref[3],orn[3]; */
	/* short itot,i,ic,rgb,norm; */
	/* float vn[3],view[3],xcor,ycor,zcor; */
	/* struct ColBlckF col; */
	/* short xsch,ysch,xasp,yasp,afmx,afmy,d,f,anim; */
	/* short xstart,xend,ystart,yend; */
	/* long svlako; */
	/* struct VertRen **blove; */
	/* struct VlakRen **blovl; */
	/* struct PolyRen **poly; */
	/* struct LampRen **la; */
	/* ulong *rectot,*rectz,*rectdaps,*rectaccu; */
	/* short xof,yof,rectx,recty;	 */
};

struct Osa O =
{
	{0,0,0},
};

void premulalpha(doel)   /* maakt premul 255 */
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
	register short c;
	register short mul;

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
}

void addalphaUnderGamma(doel,bron)   /* gamma-gecorr: vult bron onder doel in met alpha van doel */
register char *doel,*bron;
{
	register ulong tot;
	register ushort c, doe, bro;
	register ushort mul;

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
}


void addalphaOver(doel,bron)   /* vult bron over doel in met alpha van bron */
register char *doel,*bron;	/* controleert op overflow */
{
	register short c;
	register short mul;

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
	register short c;
	register short mul;

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

void clustered_dots(ibuf, size)
struct ImBuf *ibuf;
long size;
{
	extern float rgb_to_bw[];
	float fx, fy, fsize, halfsize;
	long a, b, x, y, tot;
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

void gamtabdit(in, out)
ushort *in;
char *out;
{
	static short rerr=0, gerr=0, berr=0;
	ulong col;
	char *cp;

	cp= (char *)&col;
	out[0]= in[0]>>8;

	col= gamtab[in[2]]+berr;
	if(col>65535) col= 65535;
	out[1]= cp[2];
	berr= cp[3];

	col= gamtab[in[4]]+gerr;
	if(col>65535) col= 65535;
	out[2]= cp[2];
	gerr= cp[3];

	col= gamtab[in[6]]+rerr;
	if(col>65535) col= 65535;
	out[3]= cp[2];
	rerr= cp[3];

}

float mistfactorN(float *co)	/* dist en hoogte, return alpha */
{
	static float misthi;
	float fac, hi;
	
	if(co==0) {
		misthi= 1000*wrld.misthi;
		return;
	}
	
	fac= -co[2]-wrld.miststa;
	if(fac>0.0) {
		if(fac< (float)wrld.mistdist) {
			fac= (fac/((float)wrld.mistdist));
			fac*= fac;
			/* u= fac*fac */
			/* alpha= (3.0*u- 2.0*fac*u); */
		}
		else fac= 1.0;
	}
	else fac= 0.0;
	
	/* de hoogte schakelt de mist af */
	if(wrld.misthi && fac!=0.0) {
		/* op hoogte misthi is mist volledig weg */

		hi= G.viewinv[0][2]*co[0]+G.viewinv[1][2]*co[1]+G.viewinv[2][2]*co[2]+G.viewinv[3][2];
		
		if(hi>misthi) fac= 0.0;
		else if(hi>0.0) {
			hi= (misthi-hi)/misthi;
			fac*= hi*hi;
		}
	}
	return 1.0-fac;
}

/* ************************************** */

void renderspothalo(ushort *col)
{
	struct LampRen *lar;
	float fx, fy, i;
	ulong *rt;
	int colt, a, x, y, ok= 0;
	ushort scol[4];
	
	
	for(a=0; a<G.totlamp; a++) {
		lar= R.la[a];
	
		if(lar->soort==1 && (lar->f & 2) && lar->haint>0) {
			spothalo(lar, R.view, &i);
			if(i>0.0) {
				i*= (512*lar->haint);	/* overflow toegstaan */
				colt= i;
				if(colt>65535) scol[0]= 65535; else scol[0]= colt;
				colt= i*lar->b;
				if(colt>65535) scol[1]= 65535; else scol[1]= colt;
				if(usegamtab)  scol[1]= igamtab2[scol[1]];
				colt= i*lar->g;
				if(colt>65535) scol[2]= 65535; else scol[2]= colt;
				if(usegamtab)  scol[2]= igamtab2[scol[2]];
				colt= i*lar->r;
				if(colt>65535) scol[3]= 65535; else scol[3]= colt;
				if(usegamtab) scol[3]= igamtab2[scol[3]];
				
				addalphaAddshort(col, scol);
			}
		}
	}
}

void shadehalo(har, col, zz, dist, spot, xn, yn)
struct HaloRen *har;
char *col;
ulong zz;
float dist;
struct HaloRen *spot;
float xn,yn;
{
	/* in col invullen */
	struct Tex *tex= 0;
	float t, hoek, zn, ster, alpha, si, co;
	int colt;

	if(har->type==127) {   /* bliks */
		float a, b, c, n, deler, turb, dist, x1, y1, turbulence();

		*( (long *)col)= 0;

		if(spot== 0) return;

		if(har->col->tex[0]) {
			tex= G.adrtex[har->col->tex[0]];

		}

		a= har->ys- spot->ys;
		b= spot->xs- har->xs;
		deler= fsqrt(a*a+b*b);
		if(deler>0.0) {

			if(tex) {		/* turbul */
				x1= 12*(har->xs+xn);
				y1= 12*(har->ys+yn);
				n= (24.0*G.cfra)/(float)(tex->div[0]);
				turb= ((float)tex->var[1])/10.0;

				c = fsqrt(fsin( turb*turbulence(tex->var[0],x1, y1,n,tex->var[2]) )+1.0);

				b*= c;
			}

			dist= fabs( (a*(xn)+ b*(yn) )/deler );
			if(dist<har->rad) {
				dist/=har->rad;
				dist= fsin(dist*PI/2.0);
				dist= 1.0- fsqrt(fsqrt(dist));
				dist*= har->alfa/255.0;

				col[0]= 255*dist;
				col[1]= dist*har->b;
				col[2]= dist*har->g;
				col[3]= dist*har->r;
			}
		}

		return;
	}

	if(wrld.misi) {
		alpha= mistfactorN(har->co);
		if(alpha==0.0) {
			*( (long *)col )=0;
			return;
		}
	}
	else alpha= 1.0;

	dist= fsqrt(dist/har->radsq);

	if(har->type>=30) {
		if(har->type>=40) {
			dist= fsin(dist*PI/2.0);
			if(har->type>=50) dist= fsqrt(dist);
		}
	}

	dist=((1.0-dist)* har->alfa)/255;

	if(har->starpoints) {
		/* rotatie */
		hoek= fatan2(yn, xn);
		hoek*= (1.0+0.25*har->starpoints);
		
		co= fcos(hoek);
		si= fsin(hoek);
		
		hoek= xn;
		xn= co*xn+si*yn;
		yn= -si*hoek+co*yn;
		
		ster= fabs(xn*yn);
		if(ster>1.0) {
			ster= (har->rad)/(ster);
			
			if(ster<1.0) dist*= fsqrt(ster);
		}
	}
	if( har->f & 1) {	/* spot */
		zn= fsqrt(xn*xn+yn*yn);
		if(zn!=0.0) {
			zn= (spot->xs*xn+spot->ys*yn)/zn;
		
			if(zn<=0.0) dist=0;
			else {
				t= spot->rad;
				if(zn<t) zn= 0;
				else {
					t= zn-t;
					if(t<spot->radsq && spot->radsq!=0) {
						zn*= t/spot->radsq;
					}
				}
				dist*= zn;
		
			}
		}
	}
	/* halo wordt doorsneden? */
	if(har->zs> zz-har->zd) {
		t= ((float)(zz-har->zs))/(float)har->zd;
		dist*= fsqrt(fsqrt(t));
	}

	dist*= alpha;
	
	if(dist<0.001) {
		*( (long *)col )=0;
		return;
	}

	if(har->col->tex[0]) {
		tex= G.adrtex[har->col->tex[0]];

	}

	colt= dist*256;
	if(colt>255) col[0]= 255; else col[0]= colt;
	colt= dist*har->b;
	if(colt>255) col[1]= 255; else col[1]= colt;
	colt= dist*har->g;
	if(colt>255) col[2]= 255; else col[2]= colt;
	colt= dist*har->r;
	if(colt>255) col[3]= 255; else col[3]= colt;
}


ulong calchalo_z(har, zz, gour)
struct HaloRen *har;
ulong zz;
long gour;
{
	if(har->f & 2) {
		if(gour) {
			if(zz!=0x7FFFFF) zz= 0;
		}
		else {
			if(zz!=0x7FFFFFFF) zz=0;
		}
	}
	else if(gour) {
		if(zz<0x800000) zz= (zz+0x7FFFFF);
		else zz= (zz-0x800000);
	}
	else {
		zz= (zz>>8);
		if(zz<0x800000) zz= (zz+0x7FFFFF);
		else zz= (zz-0x800000);
	}
	
	return zz;
}

void scanlinehaloPS(rectz,rectdelta,rectt,ys)
ulong *rectz,*rectdelta,*rectt;
short ys;
{
	struct HaloRen *har,*spot,*addhalo();
	struct PixStr *ps;
	float dist,xsq,ysq,ster,xn,yn,zn,t;
	ulong a,*rz,*rt,*rd,zz,ztot;
	long accol[4], count_mask();
	short minxr,minx,maxx,miny,maxy,x,nr,aantal, aantalm, behind;
	char col[4];

	ys+= R.ystart;
	for(a=0;a<G.tothalo;a++) {
		if((a & 255)==0) har= R.bloha[a>>8];
		else har++;

		if(G.afbreek) break;

		if(ys>har->maxy);
		else if(ys<har->miny);
		else {
			minx= har->xs-har->rad;
			maxx= har->xs+har->rad;
			if(R.xstart>maxx);
			else if(R.xend<minx);
			else {
				if(minx<R.xstart) minx= R.xstart;
				if(maxx>=R.xend) maxx= R.xend-1;

				minxr= minx-R.xstart;
				rt= rectt+minxr;
				rd= rectdelta+minxr;
				rz= rectz+minxr;

				spot= 0;
				if((har->f & 1) || har->type==127) {
					if(a<G.tothalo-1) spot= addhalo(a+1);
				}

				yn= (har->ys-ys)*R.ycor;
				ysq= yn*yn;
				for(x=minx;x<maxx;x++) {
				
					if(*rd & 0xFF000000) { /* is PS */
						xn= har->xs -x;
						xsq= xn*xn;
						dist= xsq+ysq;
						if(dist<har->radsq) {
							ps= (struct PixStr *)*rd;
							aantal= behind= 0;
							accol[0]=accol[1]=accol[2]=accol[3]= 0;
							while(ps) {
								aantalm= count_mask(ps->mask);
								aantal+= aantalm;

								zz= calchalo_z(har, ps->z, 0);
								if(zz> har->zs) {
									*( (long *)col )= 0;
									shadehalo(har,col,zz,dist,spot,xn,yn);
									accol[0]+= aantalm*col[0];
									accol[1]+= aantalm*col[1];
									accol[2]+= aantalm*col[2];
									accol[3]+= aantalm*col[3];
								}

								ps= ps->next;
							}
							ps= (struct PixStr *)*rd;
							aantal= G.osa-aantal;
							
							zz= calchalo_z(har, *rz, 0);
							if(zz> har->zs) {
								*( (long *)col )= 0;
								shadehalo(har,col,zz,dist,spot,xn,yn);
								accol[0]+= aantal*col[0];
								accol[1]+= aantal*col[1];
								accol[2]+= aantal*col[2];
								accol[3]+= aantal*col[3];
							}


							col[0]= accol[0]/G.osa;
							col[1]= accol[1]/G.osa;
							col[2]= accol[2]/G.osa;
							col[3]= accol[3]/G.osa;

							/* if(behind > (G.osa>>1)) addalphaUnder(rt,col); */
							addalphaAdd(rt,col);
						}
					}
					else {
						zz= calchalo_z(har, *rz, 0);
						if(zz> har->zs) {
							xn= har->xs -x;
							xsq= xn*xn;
							dist= xsq+ysq;
							if(dist<har->radsq) {
								shadehalo(har,col,zz,dist,spot,xn,yn);
								addalphaAdd(rt,col);
							}
						}
					}
					rt++;
					rz++;
					rd++;
				}
			}
		}
		if(har->f & 1) {
			a++;
			if((a & 255)==0) har= R.bloha[a>>8];
			else har++;
		}
	}

}

void halovert(gour)
short gour;
{
	struct HaloRen *har,*spot,*addhalo();
	float dist,xsq,ysq,ster,xn,yn,zn,t;
	ulong a,*rectz,*rz,*rectt,*rt,zz;
	short minxr,maxxr,minx,maxx,miny,maxy,x,y,yr;
	char col[4];

	if(gour) {
		readsource(SRC_ZBUFFER);
		rectz= (ulong *)mallocN(4*R.rectx+4,"rectzbuf1");
	}

	for(a=0;a<G.tothalo;a++) {
		if((a & 255)==0) har= R.bloha[a>>8];
		else har++;

		if(G.afbreek) break;

		if(R.ystart>har->maxy);
		else if(R.yend<har->miny);
		else {
			minx= ffloor(har->xs-har->rad);
			maxx= fceil(har->xs+har->rad);
			
			if(R.xstart>maxx);
			else if(R.xend<minx);
			else {
			
				miny= ffloor(har->ys-har->rad);
				maxy= fceil(har->ys+har->rad);

				if(minx<R.xstart) minx= R.xstart;
				if(maxx>=R.xend) maxx= R.xend-1;
				if(miny<R.ystart) miny= R.ystart;
				if(maxy>R.yend) maxy= R.yend;

				minxr= minx-R.xstart;
				maxxr= maxx-R.xstart;
				yr= miny-R.ystart;

				rectt= R.rectot+ R.rectx*yr;
				if(!gour) {
					rectz= R.rectz+ R.rectx*yr;
				}

				spot= 0;
				if((har->f & 1) || har->type==127) {
					if(a<G.tothalo-1) spot= addhalo(a+1);
				}

				for(y=miny;y<maxy;y++) {

					if(gour) {
						lrectread(minxr,yr,maxxr,yr,rectz);
						rz= rectz;
					}
					else rz= rectz+minxr;

					rt= (rectt+minxr);

					yn= (har->ys-y)*R.ycor;
					ysq= yn*yn;
					for(x=minx;x<maxx;x++) {
						
						zz= calchalo_z(har, *rz, gour);
						
						if(zz> har->zs) {

							xn= har->xs -x;
							xsq= xn*xn;
							dist= xsq+ysq;
							if(dist<har->radsq) {
								shadehalo(har,col,zz,dist,spot,xn,yn);
								addalphaAdd(rt,col);
							}
						}
						rt++;
						rz++;
					}
					yr++;
					rectt+= R.rectx;
					if(!gour) rectz+= R.rectx;
				}

			}
		}
		if(har->f & 1) {
			a++;
			if((a & 255)==0) har= R.bloha[a>>8];
			else har++;
		}
	}
	if(gour) {
		freeN(rectz);
		readsource(SRC_AUTO);
	}
}

void sky(col)
char *col;
{
	extern float Titot;
	float rf,gf,bf,view[3];
	long temp[2],*t;

	if((wrld.fs & 7)==0) {
		col[3]= wrld.horr;
		col[2]= wrld.horg;
		col[1]= wrld.horb;
		return;
	}

	wrld.fs |= 128;
	if(wrld.fs & 2) {
		wrld.inprz= R.view[0]*wrld.grvec[0]+ R.view[1]*wrld.grvec[1]+ R.view[2]*wrld.grvec[2];
		if(wrld.inprz<0.0) wrld.fs-= 128;
		wrld.inprz= fabs(wrld.inprz);
	}
	else if(wrld.fs & 8)
		wrld.inprz= 0.5*(R.view[1]+1.0);
	else
		wrld.inprz= fabs(.5+ R.view[1]);

	if(wrld.inprz>1.0) wrld.inprz=1.0;
	wrld.inprh= 1.0-wrld.inprz;

	if(wrld.fs & 4) {
		t= (long *)&wrld;
		temp[0]= t[0]; 
		temp[1]= t[1];
		if(wrld.fs & 2) {
			VECCOPY(view,R.view);
			Mat3MulVecfl(G.workbase->imat,view);
			R.lo[0]= 32767*view[0];
			R.lo[1]= 32767*view[1];
			R.lo[2]= 32767*view[2];
		}
		else {
			R.lo[0]= 32767*R.view[0];
			R.lo[1]= 32767*R.view[1];
			R.lo[2]= 32767*R.view[2];
		}
		Titot= 1.0;
		R.osatex= 0;
		if(wrld.stex[0]) skytex(0);
		if(wrld.stex[1]) skytex(1);
		if(wrld.stex[2]) skytex(2);
		if(wrld.stex[3]) skytex(3);
	}

	if(wrld.fs & 1) {
		rf= wrld.inprh*wrld.horr+wrld.inprz*wrld.zenr;
		gf= wrld.inprh*wrld.horg+wrld.inprz*wrld.zeng;
		bf= wrld.inprh*wrld.horb+wrld.inprz*wrld.zenb;

		if (rf>255.0) col[3]=255; 
		else col[3]= (char)rf;
		if (gf>255.0) col[2]=255; 
		else col[2]= (char)gf;
		if (bf>255.0) col[1]=255; 
		else col[1]= (char)bf;
	}
	else {
		col[3]= wrld.horr;
		col[2]= wrld.horg;
		col[1]= wrld.horb;
	}
	if(wrld.fs & 4) {
		t[0]=temp[0]; 
		t[1]=temp[1];
	}
}


float getMu(f0)
float f0;
{
	return (1.0+fsqrt(f0))/(1.0-fsqrt(f0));
}

float getF(phi, mu)
float phi, mu;
{
	float t1, t2, theta;
	
	if(phi<0.005) phi= 0.005;
	theta= fasin( fsin(phi)/mu);
	
	t1= fsin(phi-theta)/fsin(phi+theta);
	t2= ftan(phi-theta)/ftan(phi+theta);
	
	return 0.5*( (t1*t1)+(t2*t2) );
}

float CookTorro(n, l, v, colbf)
float *n, *l, *v;
struct ColBlckF *colbf;
{
	float nl, nv, nh, vh, lh, h[3], i, D, G, F;
	float beta, g, c, g1, g2, m;

	h[0]= v[0]+l[0];
	h[1]= v[1]+l[1];
	h[2]= v[2]+l[2];
	Normalise(h);
	nl= n[0]*l[0]+n[1]*l[1]+n[2]*l[2];
	nv= n[0]*v[0]+n[1]*v[1]+n[2]*v[2];
	nh= n[0]*h[0]+n[1]*h[1]+n[2]*h[2];
	vh= v[0]*h[0]+v[1]*h[1]+v[2]*h[2];
	lh= l[0]*h[0]+l[1]*h[1]+l[2]*h[2];
	
	/* berekening D */
	m= colbf->ang;
	if(m!=0.0) {
		/* beta= facos(nh)/m; */
		/* D= colbf->spec*fexp(-(beta*beta)); */
		beta= facos(nh);
		if(beta>(PI/2.0)) return 0.0;
		g1= nh*nh;
		g1*= g1;
		g2= (tanf(beta))/m;
		g2= fexp(-(g2*g2));
		D= g2/(4*m*m*g1);
	}
	else return 0.0;
	
	/* berekening G */
	g1= 2.0*nh*nv/vh;
	g2= 2.0*nh*nl/vh;
	if(g1<g2) G= g1; else G= g2;
	if(G<0.0) G= 0.0;
	if(G>1.0) G= 1.0;

	/* berekening F */
	
	g1= facos(nl);
	g2= getMu(colbf->mir);
	F= getF(g1, g2);
	
	/* c= lh; */
	/* g= colbf->mir+lh*lh-1.0; */
	/* if(g<0.0) g= 0.0; */
	/* else g= fsqrt(g); */
	/* F= 0.5*(g-c)*(g-c)/( (g+c)*(g+c) ); */
	/* g1= c*(g+c)-1.0; */
	/* g2= c*(g-c)+1.0; */
	/* F*= (1.0+ g1*g1/(g2*g2)); */
	
	i= 0.3*D*G/(nl*nv);

	if(i<0.0) return 0.0;
	return i;
}

float CookTorr(n, l, v, colbf)
float *n, *l, *v;
struct ColBlckF *colbf;
{
	float i, nh, nv, h[3], Spec();

	h[0]= v[0]+l[0];
	h[1]= v[1]+l[1];
	h[2]= v[2]+l[2];
	Normalise(h);

	nh= n[0]*h[0]+n[1]*h[1]+n[2]*h[2];
	if(nh<0.0) return 0.0;
	nv= n[0]*v[0]+n[1]*v[1]+n[2]*v[2];
	if(nv<0.0) nv= 0.0;
	i= Spec(nh, colbf->spec);

	i= i/(0.1+nv);
	if(i>1.0) return 1.0;
	return i;
}

void shadelamplus()
{
	extern float Titot;
	struct LampRen *lar;
	register struct ColBlckF *col;
	float i, inpr, t, lv[3], lampdist, ir, ig, ib, isr=0,isg=0,isb=0;
	float lampcol[3], Spec(),*vn,*view, shadfac;
	extern float testshadowbuf();
	long a, addcol, c1, c2, cooktorr;

	vn= R.vn;
	view= R.view;
	col= &R.col;

	if(col->mode & 256) {	/* sky + shadow */
		shadfac= ir= 0.0;
		for(a=0;a<G.totlamp;a++) {
			lar=R.la[a];
			if(lar->shb) {
				/* alleen testen binnen spotbundel */
				lv[0]= R.co[0]-lar->co[0];
				lv[1]= R.co[1]-lar->co[1];
				lv[2]= R.co[2]-lar->co[2];
				Normalise(lv);
				inpr= lv[0]*lar->vec[0]+lv[1]*lar->vec[1]+lv[2]*lar->vec[2];
				if(inpr>lar->spotsi) {
					i= testshadowbuf(lar->shb);

					t= inpr - lar->spotsi;
					if(t<lar->spotbl && lar->spotbl!=0.0) {
						t/= lar->spotbl;
						t*= t;
						i= t*i+(1.0-t);
					}
					
					shadfac+= i;
					ir+= 1.0;
				}
				else {
					shadfac+= 1.0;
					ir+= 1.0;
				}
			}
		}
		if(ir>0.0) shadfac/= ir;
		col->tra= 1.0- (1.0-col->tra)*(1.0-shadfac);
		
		shortcol[1]=shortcol[2]=shortcol[3]= 0;
		
		return;
	}

	if(col->texco & 63) {
		if(col->texco & 1) {
			i= -2*(vn[0]*view[0]+vn[1]*view[1]+vn[2]*view[2]);

			R.ref[0]= 32767*(view[0]+i*vn[0]);
			R.ref[1]= 32767*(view[1]+i*vn[1]);
			R.ref[2]= 32767*(view[2]+i*vn[2]);
			if(R.osatex) {
				if(col->mode & 1) {
					i= -2*( (vn[0]+O.dxno[0])*(view[0]+O.dxview) +
					    (vn[1]+O.dxno[1])*view[1]+ (vn[2]+O.dxno[2])*view[2] );

					O.dxref[0]= R.ref[0]- 32767*( view[0]+O.dxview+i*(vn[0]+O.dxno[0]));
					O.dxref[1]= R.ref[1]- 32767*(view[1]+ i*(vn[1]+O.dxno[1]));
					O.dxref[2]= R.ref[2]- 32767*(view[2]+ i*(vn[2]+O.dxno[2]));

					i= -2*( (vn[0]+O.dyno[0])*view[0]+
					    (vn[1]+O.dyno[1])*(view[1]+O.dyview)+ (vn[2]+O.dyno[2])*view[2] );

					O.dyref[0]= R.ref[0]- 32767*(view[0]+ i*(vn[0]+O.dyno[0]));
					O.dyref[1]= R.ref[1]- 32767*(view[1]+O.dyview+i*(vn[1]+O.dyno[1]));
					O.dyref[2]= R.ref[2]- 32767*(view[2]+ i*(vn[2]+O.dyno[2]));

				}
				else {

					i= -2*( vn[0]*(view[0]+O.dxview) +
					    vn[1]*view[1]+ vn[2]*view[2] );

					O.dxref[0]= R.ref[0]- 32767*(view[0]+O.dxview+i*vn[0]);
					O.dxref[1]= R.ref[1]- 32767*(view[1]+ i*vn[1]);
					O.dxref[2]= R.ref[2]- 32767*(view[2]+ i*vn[2]);

					i= -2*( vn[0]*view[0]+
					    vn[1]*(view[1]+O.dyview)+ vn[2]*view[2] );

					O.dyref[0]= R.ref[0]- 32767*(view[0]+ i*vn[0]);
					O.dyref[1]= R.ref[1]- 32767*(view[1]+O.dyview+i*vn[1]);
					O.dyref[2]= R.ref[2]- 32767*(view[2]+ i*vn[2]);
				}
			}
		}
		if(col->texco & 8) {
			R.gl[0]= R.co[0];
			R.gl[1]= R.co[1];
			R.gl[2]= R.co[2];
		}
		Titot= 1.0;
		if(col->tex[0]) objectex(col,0);
		if(col->tex[1]) objectex(col,1);
		if(col->tex[2]) objectex(col,2);
		if(col->tex[3]) objectex(col,3);
	}

	if(col->mode & 16) {	/* shless */
		if(usegamtab) {
			shortcol[3]= igamtab2[ (ushort)(65535*col->r) ];
			shortcol[2]= igamtab2[ (ushort)(65535*col->g) ];
			shortcol[1]= igamtab2[ (ushort)(65535*col->b) ];
		}
		else {
			shortcol[3]= (ushort)(65535*col->r);
			shortcol[2]= (ushort)(65535*col->g) ;
			shortcol[1]= (ushort)(65535*col->b) ;
		}
		return;
	}

	ir= ig= ib= col->emit;

	for(a=0;a<G.totlamp;a++) {
		lar=R.la[a];

		if(lar->tex[0]) {
			lampcol[0]= lar->r;
			lampcol[1]= lar->g;
			lampcol[2]= lar->b;
		}

		/* test op lamplayer */
		if(lar->f & 4) if((lar->lay & col->lay)==0) goto eindelamplus;

		/* lampdist berekening */
		if(lar->soort>=2) {			/* zon of hemi */
			VECCOPY(lv, lar->vec);
			lampdist= 1.0;
		}
		else {
			lv[0]= R.co[0]-lar->co[0];
			lv[1]= R.co[1]-lar->co[1];
			lv[2]= R.co[2]-lar->co[2];
			lampdist= fsqrt(lv[0]*lv[0]+lv[1]*lv[1]+lv[2]*lv[2]);
			lv[0]/=lampdist;
			lv[1]/=lampdist;
			lv[2]/=lampdist;
			if(lar->f & 8) {	/* quadlamp */
				t= 1.0;
				if(lar->ld1>0.0)
					t= lar->dist/(lar->dist+lar->ld1*lampdist);
				if(lar->ld2>0.0)
					t*= lar->distkw/(lar->distkw+lar->ld2*lampdist*lampdist);

				lampdist= t;
			}
			else {
				lampdist= (lar->dist/(lar->dist+lampdist));
			}
		}

		/* lampetex */
		if(lar->tex[0]) {
			Titot= 1.0;
			lamptex(lar,0,lv);
			if(lar->tex[1]) lamptex(lar,1,lv);
			if(lar->tex[2]) lamptex(lar,2,lv);
			if(lar->tex[3]) lamptex(lar,3,lv);
			if(lar->r==0.0 && lar->g==0.0 && lar->b==0.0) goto eindelamplus;
		}

		/* spot */
		if(lar->soort==1) {

			/* hier de fie Inp() vertaagt! */
			inpr= lv[0]*lar->vec[0]+lv[1]*lar->vec[1]+lv[2]*lar->vec[2];
			t= lar->spotsi;
			if(inpr<t) goto eindelamplus;
			else {
				t= inpr-t;
				if(t<lar->spotbl && lar->spotbl!=0) {
					/* zachte gebied */
					i= t/lar->spotbl;
					t= i*i;
					i= t*i;
					lampdist*=(3.0*t-2.0*i);
				}
				
				if((lar->f & 32)==0) lampdist*=inpr;
				
			}
		}

		/* inprodukt en reflectivity*/
		i= vn[0]*lv[0]+vn[1]*lv[1]+vn[2]*lv[2];
		if(lar->soort==3) {
			i= 0.5*i+0.5;
		}
		if(i>0) {
			i*=lampdist*col->kref;
		}

		/* schaduw en spec */
		if(i> -0.41) {			/* beetje willekeurig, beetje getest */
			shadfac= 1.0;
			if(lar->shb) {
				if(col->mode & 8) {
					shadfac= testshadowbuf(lar->shb);
					if(shadfac==0.0) goto eindelamplus;
					i*=shadfac;
				}
			}
			/* specularity */
			
			if(col->kspec!=0.0) {
			
				cooktorr= (lar->soort<2);
				
				if(cooktorr) {
					t= shadfac*col->kspec*lampdist*CookTorr(vn, lv, view, col);
					isr+= t*(lar->r * col->specr);
					isg+= t*(lar->g * col->specg);
					isb+= t*(lar->b * col->specb);
				}
				else {
					if(lar->soort==2) {
						lv[2]-= 1.0;
					}
					else {
						lv[0]+= view[0];
						lv[1]+= view[1];
						lv[2]+= view[2];
					}
					
					Normalise(lv);
					
					t= vn[0]*lv[0]+vn[1]*lv[1]+vn[2]*lv[2];
					
					if(lar->soort==3) {
						t= 0.5*t+0.5;
					}
					/* let op: shadfac en lampdist uit onderstaande */
					
					if(t>col->speclim) {
						t= col->kspec*Spec(t, col->spec);
						isr+= t*(lar->r * col->specr);
						isg+= t*(lar->g * col->specg);
						isb+= t*(lar->b * col->specb);
					}
				}
			}
		}
		if(i>0.0) {
			ir+= i*lar->r;
			ig+= i*lar->g;
			ib+= i*lar->b;
		}
eindelamplus:

		if(lar->tex[0]) {
			lar->r= lampcol[0];
			lar->g= lampcol[1];
			lar->b= lampcol[2];
		}
	}
	
	if(col->mode & 128) {	/* ztra shade */
		if(col->ang!=0.0) {
			/* t= col->ang*(isr+isb+isg)/3.0; */ /*oud*/
			t = MAX3(isr, isb, isg);
			t *= col->ang;
			if(t>1.0) t= 1.0;
			col->tra*= (1.0-t);
		}
	}
	if(usegamtab) {
		a= 65535.0*(col->r*ir +col->ambr +isr);
		if(a>65535) a=65535;
		shortcol[3]= igamtab2[a];
		a= 65535.0*(col->g*ig +col->ambg +isg);
		if(a>65535) a=65535;
		shortcol[2]= igamtab2[a];
		a= 65535*(col->b*ib +col->ambb +isb);
		if(a>65535) a=65535;
		shortcol[1]= igamtab2[a];

	}
	else {
		a= 65535.0*(col->r*ir +col->ambr +isr);
		/* addcol= 0; */
		if(a>65535) { 
		/* 	addcol= a-65535;  */
			a=65535; 
		}
		shortcol[3]= a;
		a= 65535.0*(col->g*ig +col->ambg +isg);
		if(a>65535) { 
		/* 	addcol+= a-65535;  */
			a=65535; 
		}
		shortcol[2]= a;
		a= 65535*(col->b*ib +col->ambb +isb);
		if(a>65535) { 
		/* 	addcol+= a-65535;  */
			a=65535; 
		}
		shortcol[1]= a;

		/* if(addcol) { */
		/* 	addcol/= 3; */
		/* 	a= shortcol[3]+addcol; */
		/* 	if(a>65535) shortcol[3]= 65535;  */
		/* 	else shortcol[3]= a; */
		/* 	a= shortcol[2]+addcol; */
		/* 	if(a>65535) shortcol[2]= 65535;  */
		/* 	else shortcol[2]= a; */
		/* 	a= shortcol[1]+addcol; */
		/* 	if(a>65535) shortcol[1]= 65535;  */
		/* 	else shortcol[1]= a; */
		/* } */
	}

}


void shadepixel(x,y,svlak)  /* x,y: windowcoordinaat van 0 tot rectx,y */
float x,y;
long svlak;
{
	static struct VlakRen *vlr;
	struct VlakRen *addvlak();
	static struct VertRen *v1,*v2,*v3;
	float u,v,l,dl,detsh,fac,deler, alpha;
	long a;
	static float t00,t01,t10,t11,dvlak,n1[3],n2[3],n3[3];
	short *o1,*o2,*o3;

	if(R.svlako== -1) {	/* doet initrender */
		vlr= R.vlr= 0;
	}

	if(svlak<=0) {	/* sky */
		R.svlako= 0;
		shortcol[0]= 0;
	}
	else if(svlak<=G.totvlak) {

		if(svlak!=R.svlako) {
			vlr= addvlak(svlak-1);

			if(R.vlr && R.vlr->col==vlr->col) {
				if(R.col.copysize) memcpy(&R.col, vlr->col, R.col.copysize);
			}
			else memcpy(&R.col, vlr->col, sizeof(struct ColBlckF));

			R.vlr=vlr;

			R.vno= vlr->n;
			R.osatex= (R.col.texco & 128);
			R.svlako= svlak;

			v1=vlr->v1;
			dvlak= v1->co[0]*vlr->n[0]+v1->co[1]*vlr->n[1]+v1->co[2]*vlr->n[2];

			if(R.col.texco & 64) {	/* uv nodig */

				v2=vlr->v2;
				v3=vlr->v3;
				if(vlr->snproj==0) {
					t00= v3->co[0]-v1->co[0]; t01= v3->co[1]-v1->co[1];
					t10= v3->co[0]-v2->co[0]; t11= v3->co[1]-v2->co[1];
				} else if(vlr->snproj==1) {
					t00= v3->co[0]-v1->co[0]; t01= v3->co[2]-v1->co[2];
					t10= v3->co[0]-v2->co[0]; t11= v3->co[2]-v2->co[2];
				} else {
					t00= v3->co[1]-v1->co[1]; t01= v3->co[2]-v1->co[2];
					t10= v3->co[1]-v2->co[1]; t11= v3->co[2]-v2->co[2];
				}
				detsh=t00*t11-t10*t01;
				t00/= detsh; t01/=detsh; 
				t10/=detsh; t11/=detsh;
				if(R.col.mode & 1) { /* puno's goedzetten */
					if(vlr->puno & 1) {
						n1[0]= -v1->n[0]; n1[1]= -v1->n[1]; n1[2]= -v1->n[2];
					} else {
						n1[0]= v1->n[0]; n1[1]= v1->n[1]; n1[2]= v1->n[2];
					}
					if(vlr->puno & 2) {
						n2[0]= -v2->n[0]; n2[1]= -v2->n[1]; n2[2]= -v2->n[2];
					} else {
						n2[0]= v2->n[0]; n2[1]= v2->n[1]; n2[2]= v2->n[2];
					}
					if(vlr->puno & 4) {
						n3[0]= -v3->n[0]; n3[1]= -v3->n[1]; n3[2]= -v3->n[2];
					} else {
						n3[0]= v3->n[0]; n3[1]= v3->n[1]; n3[2]= v3->n[2];
					}
					
				}
			}
		}
		else if(R.col.copysize) memcpy(&R.col, vlr->col, R.col.copysize);

		/* COXYZ nieuwe methode (de oude staat in oud.c) */
		if(G.holo==3) {
			R.view[0]= (x+(R.xstart)+1.0+holoofs);
		}
		else {
			R.view[0]= (x+(R.xstart)+1.0);
		}
		if(R.f & 64) R.view[1]= (y+R.ystart+1.5)*R.ycor;
		else R.view[1]= (y+R.ystart+1.0)*R.ycor;
		R.view[2]= -R.near;
		
		deler= vlr->n[0]*R.view[0] + vlr->n[1]*R.view[1] + vlr->n[2]*R.view[2];
		fac= dvlak/deler;
		R.co[0]= fac*R.view[0];
		R.co[1]= fac*R.view[1];
		R.co[2]= fac*R.view[2];

		if(R.osatex || (R.f & 2) ) {
			u= dvlak/(deler-vlr->n[0]);
			v= dvlak/(deler- R.ycor*vlr->n[1]);

			O.dxco[0]= R.co[0]- (R.view[0]-1.0)*u;
			O.dxco[1]= R.co[1]- (R.view[1])*u;
			O.dxco[2]= R.co[2]- (R.view[2])*u;

			O.dyco[0]= R.co[0]- (R.view[0])*v;
			O.dyco[1]= R.co[1]- (R.view[1]-1.0*R.ycor)*v;
			O.dyco[2]= R.co[2]- (R.view[2])*v;

		}

		fac= Normalise(R.view);
		if(R.osatex) {
			if( (R.col.texco & 1) ) {
				O.dxview= 1.0/fac;
				O.dyview= R.ycor/fac;
			}
		}

		/* UV en TEX*/
		if(R.col.texco & 64) {
			if(vlr->snproj==0) {
				u= (R.co[0]-v3->co[0])*t11-(R.co[1]-v3->co[1])*t10;
				v= (R.co[1]-v3->co[1])*t00-(R.co[0]-v3->co[0])*t01;
				if(R.osatex) {
					O.dxuv[0]=  O.dxco[0]*t11- O.dxco[1]*t10;
					O.dxuv[1]=  O.dxco[1]*t00- O.dxco[0]*t01;
					O.dyuv[0]=  O.dyco[0]*t11- O.dyco[1]*t10;
					O.dyuv[1]=  O.dyco[1]*t00- O.dyco[0]*t01;
				}
			} else if(vlr->snproj==1) {
				u= (R.co[0]-v3->co[0])*t11-(R.co[2]-v3->co[2])*t10;
				v= (R.co[2]-v3->co[2])*t00-(R.co[0]-v3->co[0])*t01;
				if(R.osatex) {
					O.dxuv[0]=  O.dxco[0]*t11- O.dxco[2]*t10;
					O.dxuv[1]=  O.dxco[2]*t00- O.dxco[0]*t01;
					O.dyuv[0]=  O.dyco[0]*t11- O.dyco[2]*t10;
					O.dyuv[1]=  O.dyco[2]*t00- O.dyco[0]*t01;
				}
			} else {
				u= (R.co[1]-v3->co[1])*t11-(R.co[2]-v3->co[2])*t10;
				v= (R.co[2]-v3->co[2])*t00-(R.co[1]-v3->co[1])*t01;
				if(R.osatex) {
					O.dxuv[0]=  O.dxco[1]*t11- O.dxco[2]*t10;
					O.dxuv[1]=  O.dxco[2]*t00- O.dxco[1]*t01;
					O.dyuv[0]=  O.dyco[1]*t11- O.dyco[2]*t10;
					O.dyuv[1]=  O.dyco[2]*t00- O.dyco[1]*t01;
				}
			}
			l= 1.0+u+v;

			if(R.col.mode & 1) {
				R.vn[0]= l*n3[0]-u*n1[0]-v*n2[0];
				R.vn[1]= l*n3[1]-u*n1[1]-v*n2[1];
				R.vn[2]= l*n3[2]-u*n1[2]-v*n2[2];

				/* hier zat vroeger de interphong */

				Normalise(R.vn);
				if(R.osatex && (R.col.texco & 3) ) {
					dl= O.dxuv[0]+O.dxuv[1];
					O.dxno[0]= dl*n3[0]-O.dxuv[0]*n1[0]-O.dxuv[1]*n2[0];
					O.dxno[1]= dl*n3[1]-O.dxuv[0]*n1[1]-O.dxuv[1]*n2[1];
					O.dxno[2]= dl*n3[2]-O.dxuv[0]*n1[2]-O.dxuv[1]*n2[2];
					dl= O.dyuv[0]+O.dyuv[1];
					O.dyno[0]= dl*n3[0]-O.dyuv[0]*n1[0]-O.dyuv[1]*n2[0];
					O.dyno[1]= dl*n3[1]-O.dyuv[0]*n1[1]-O.dyuv[1]*n2[1];
					O.dyno[2]= dl*n3[2]-O.dyuv[0]*n1[2]-O.dyuv[1]*n2[2];
				}
			}
			else {
				VECCOPY(R.vn, vlr->n);
			}

			if(R.col.mode & 512) {	/* z invert */
				/* R.vn[0]= -R.vn[0]; */
				/* R.vn[1]= -R.vn[1]; */
			}

			if(R.col.texco & 4) {
				if(v2->v) {	/* is nul o.a. bij nurbs en curves */
					o1= v1->v->c;
					o2= v2->v->c;
					o3= v3->v->c;
					
					R.lo[0]= l*o3[0]-u*o1[0]-v*o2[0];
					R.lo[1]= l*o3[1]-u*o1[1]-v*o2[1];
					R.lo[2]= l*o3[2]-u*o1[2]-v*o2[2];

					if(R.osatex) {
						dl= O.dxuv[0]+O.dxuv[1];
						O.dxlo[0]= dl*o3[0]-O.dxuv[0]*o1[0]-O.dxuv[1]*o2[0];
						O.dxlo[1]= dl*o3[1]-O.dxuv[0]*o1[1]-O.dxuv[1]*o2[1];
						O.dxlo[2]= dl*o3[2]-O.dxuv[0]*o1[2]-O.dxuv[1]*o2[2];
						dl= O.dyuv[0]+O.dyuv[1];
						O.dylo[0]= dl*o3[0]-O.dyuv[0]*o1[0]-O.dyuv[1]*o2[0];
						O.dylo[1]= dl*o3[1]-O.dyuv[0]*o1[1]-O.dyuv[1]*o2[1];
						O.dylo[2]= dl*o3[2]-O.dyuv[0]*o1[2]-O.dyuv[1]*o2[2];
						
					}

				}
			}
			if(R.col.texco & 16) {
				R.uv[0]= 65535*(u+.5);
				R.uv[1]= 65535*(v+.5);
			}
			if(R.col.texco & 2) {
				R.orn[0]= 32767*R.vn[0];
				R.orn[1]= -32767*R.vn[1];
				R.orn[2]= 32767*R.vn[2];
			}
		}
		else {
			VECCOPY(R.vn,vlr->n);
		}

		shadelamplus();

		/* MIST */
		if(wrld.misi) {
			alpha= mistfactorN(R.co);
		}
		else alpha= 1.0;

		/* RAYTRACE (tijdelijk?) UITGESCHAKELD */

		if( (R.f & 4) && (R.col.mir)!=0.0) {
			float m1,m2,m3,i, ref[3], view[3];
			float dxref[3], dyref[3], vno[3];
			long co[3];
			ushort a, tcol[4], col[4], memcol[4];

			if(G.ali) {
			    /* R.osatex= 0; */
				
				for(a=0; a<3; a++) {
					dxref[a]= (O.dxref[a]/16384.0);
					dyref[a]= (O.dyref[a]/16384.0);
				}
				
			    i= -2.0*(R.vn[0]*R.view[0]+R.vn[1]*R.view[1]+R.vn[2]*R.view[2]);
				
			    ref[0]= (R.view[0]+i*R.vn[0]);
			    ref[1]= (R.view[1]+i*R.vn[1]);
			    ref[2]= (R.view[2]+i*R.vn[2]);
				
				/* test ref als phong */
				if(R.col.mode & 1) {
					i= ref[0]*R.vno[0]+ ref[1]*R.vno[1]+ ref[2]*R.vno[2];
					if(i>0.0) {
						i+= .01;
						ref[0]-= i*R.vno[0];
						ref[1]-= i*R.vno[1];
						ref[2]-= i*R.vno[2];
					}
				}
				
			    VECCOPY(co, R.co);
			    VECCOPY(vno, R.vno);
				
			    tcol[0]= tcol[1]= tcol[2]= tcol[3]= 0;
				memcol[0]= shortcol[0]; memcol[1]= shortcol[1];
				memcol[2]= shortcol[2]; memcol[3]= shortcol[3]; 
				
			    for(a=0; a<G.osa; a++) {
				
					memcpy(&R.col,vlr->col,sizeof(struct ColBlckF));
					R.vlr= vlr;
					
					col[3]= memcol[3];
					col[2]= memcol[2];
					col[1]= memcol[1];
					
					m1= ref[0]+ ((jit[a][0]-.5)*dxref[0]+ (jit[a][1]-.5)*dyref[0]);
					m2= ref[1]+ ((jit[a][0]-.5)*dxref[1]+ (jit[a][1]-.5)*dyref[1]);
					m3= ref[2]+ ((jit[a][0]-.5)*dxref[2]+ (jit[a][1]-.5)*dyref[2]);
					
					/* test vec op correcte richting */
					i= m1*vno[0]+ m2*vno[1]+ m3*vno[2] ;
					if(i > 0.0) {
						m1= ref[0];
						m2= ref[1];
						m3= ref[2];
					}

					traceray(R.col.mir, 0,m1,m2,m3,col);
					
					tcol[3]+= col[3]/G.osa;
					tcol[2]+= col[2]/G.osa;
					tcol[1]+= col[1]/G.osa;
					
					VECCOPY(R.co, co);
			    }
			    shortcol[3]= (tcol[3]);
			    shortcol[2]= (tcol[2]);
			    shortcol[1]= (tcol[1]);
				shortcol[0]= 65535;
			}
			else {
			    col[3]= shortcol[3];
			    col[2]= shortcol[2];
			    col[1]= shortcol[1];
				
			    i= -2.0*(R.vn[0]*R.view[0]+R.vn[1]*R.view[1]+R.vn[2]*R.view[2]);
			    /* if(i>-0.2) i= -0.2; */
				
			    m1= (R.view[0]+i*R.vn[0]);
			    m2= (R.view[1]+i*R.vn[1]);
			    m3= (R.view[2]+i*R.vn[2]);

				/* test ref als phong */
				if(R.col.mode & 1) {
					i= m1*R.vno[0]+ m2*R.vno[1]+ m3*R.vno[2];
					if(i>0.0) {
						i+= .01;
						m1-= i*R.vno[0];
						m2-= i*R.vno[1];
						m3-= i*R.vno[2];
					}
				}

				traceray(R.col.mir, 0, m1, m2, m3, col);
			
				shortcol[3]= col[3];
				shortcol[2]= col[2];
				shortcol[1]= col[1];
				shortcol[0]= 65535;
			}			
		}
		else if(R.col.tra!=0.0 || alpha!=0.0) {
			fac= alpha*(1.0-R.col.tra);
			shortcol[0]= 65535.0*fac;
			shortcol[3]*= fac;
			shortcol[2]*= fac;
			shortcol[1]*= fac;
		}
		else {
			shortcol[0]= 65536;
		}
	}
	else {
		shortcol[0]= 65535;
		shortcol[1]= 65535;
		shortcol[2]= 0;
		shortcol[3]= 65535;
	}
	
	if(R.f & 4096) {
		if(svlak<=0) {	/* bereken viewvec en zet R.co op far */
		
			R.view[0]= (x+(R.xstart)+1.0);
			if(R.f & 64) R.view[1]= (y+R.ystart+1.5)*R.ycor;
			else R.view[1]= (y+R.ystart+1.0)*R.ycor;
			R.view[2]= -R.near;
			
			R.co[2]= 0;
			
		}
		renderspothalo(shortcol);
	}
}

struct PixStr *addpsmain()
{
	struct PixStrMain *psm;

	psm= &psmfirst;

	while(psm->next) {
		psm= psm->next;
	}

	psm->next= (struct PixStrMain *)mallocN(sizeof(struct PixStrMain),"pixstrMain");

	psm= psm->next;
	psm->next=0;
	psm->ps= (struct PixStr *)mallocN(4096*sizeof(struct PixStr),"pixstr");
	psmteller= 0;

	return psm->ps;
}

void freeps()
{
	struct PixStrMain *psm,*next;

	psm= &psmfirst;

	while(psm) {
		next= psm->next;
		if(psm->ps) {
			freeN(psm->ps);
			psm->ps= 0;
		}
		if(psm!= &psmfirst) freeN(psm);
		psm= next;
	}

	psmfirst.next= 0;
	psmfirst.ps= 0;
}

void addps(rd,vlak,z,ronde)
ulong *rd,vlak,z;
short ronde;
{
	static struct PixStr *prev;
	struct PixStr *ps,*last;

	if(*rd & 0xFF000000) { /* is al PS */
		ps= (struct PixStr *) *rd;
		if(ps->vlak0== vlak) return;
		while(ps) {
			if(ps->vlak== vlak) {
				ps->mask |= (1<<ronde);
				return;
			}
			last= ps;
			ps= ps->next;
		}

		if((psmteller & 4095)==0) prev= addpsmain();
		else prev++;
		psmteller++;

		last->next= prev;
		prev->next= 0;
		prev->vlak= vlak;
		prev->z= z;
		prev->mask = (1<<ronde);
		prev->ronde= ronde;
		return;
	}

	/* eerste PS maken */
	if((psmteller & 4095)==0) prev= addpsmain();
	else prev++;
	psmteller++;

	prev->next= 0;
	prev->vlak0= *rd;
	prev->vlak= vlak;
	prev->z= z;
	prev->mask = (1<<ronde);
	prev->ronde= ronde;
	*rd= (ulong) prev;
}


long count_mask(mask)
ushort mask;
{
	return (cmask[mask & 255]+cmask[mask>>8]);
}

float count_maskf(mask)
ushort mask;
{
	return (fmask[mask & 255]+fmask[mask>>8]);
}


void add_filt_mask(mask, col, rb1, rb2, rb3)
ulong mask;
ushort *col;
ulong *rb1, *rb2, *rb3;
{
	/* bereken de waarde van mask */
	ulong a, maskand, maskshift;
	long j;
	register ushort val, r, g, b, al;

	al= col[0];
	r= col[1];
	g= col[2];
	b= col[3];

	maskand= (mask & 255);
	maskshift= (mask >>8);

	for(j=2; j>=0; j--) {

		a= j;

		val= *(mask1[a] +maskand) + *(mask2[a] +maskshift);
		if(val) {
			rb1[0]+= val*al;
			rb1[1]+= val*r;
			rb1[2]+= val*g;
			rb1[3]+= val*b;
		}
		a+=3;

		val= *(mask1[a] +maskand) + *(mask2[a] +maskshift);
		if(val) {
			rb2[0]+= val*al;
			rb2[1]+= val*r;
			rb2[2]+= val*g;
			rb2[3]+= val*b;
		}
		a+=3;

		val= *(mask1[a] +maskand) + *(mask2[a] +maskshift);
		if(val) {
			rb3[0]+= val*al;
			rb3[1]+= val*r;
			rb3[2]+= val*g;
			rb3[3]+= val*b;
		}

		rb1+= 4;
		rb2+= 4;
		rb3+= 4;
	}
}

/* ********************* HOOFDLUSSEN ******************** */

void zbufshadeDA()	/* Delta Accum Pixel Struct */
{
	extern ushort *Acolrow;
	struct PixStr *ps;
	ulong coladr,*rz,*rp,*rd,*rt, mask, fullmask;
	ulong  *rowbuf1, *rowbuf2, *rowbuf3, *rb1, *rb2, *rb3;
	float xd, yd, xs, ys;
	long a, b;
	ushort *colrb, *acol;
	short tot,v,x,y,y1;
	short zbuffermetdehand();
	char *col,*colrt, tempcol[4];

	R.rectdaps= (ulong *)callocN(4*R.rectx*R.recty+4,"zbufDArectd");
	if(R.f & 16) bgnaccumbuf();

	tot= G.osa;

	/* psmfirst.ps= (struct PixStr *)mallocN(1024*sizeof(struct PixStr),"pixstr1"); */
	/* psmfirst.next= 0; */
	psmteller= 0;

	if(R.f & (1024)) {
		R.rectaccu= (ulong *)callocN(4*R.rectx*R.recty,"zbufshadeDA");
	}

	for(v=0;v<G.osa;v++) {

		xd= jit[v][0];
		yd= jit[v][1];
		Zjitx= -xd;
		Zjity= -yd;

		printrenderinfo(v);

		/* RECTDELTA  */
		if(v==0) {
			rz= R.rectot;
			R.rectot= R.rectdaps;
		}
		else if(v>0) {
			fillrect(R.rectot,R.rectx,R.recty,0);
		}

		zbufferall(0);

		if(v==0) {
			R.rectot= rz;
		}

		/* PIXSTRUCTEN MAKEN */
		if(v!=0) {
			rd= R.rectdaps;
			rp= R.rectot;
			rz= R.rectz;
			for(y=0;y<R.recty;y++) {
				for(x=0;x<R.rectx;x++,rp++,rd++) {
					if(*rd!= *rp) {
						addps(rd,*rp,*(rz+x),v);
					}
				}
				rz+= R.rectx;
			}
		}

		if(R.f & 1024) edge_enhance(1);	/* 1 is voor osa */
		
		if(G.afbreek) break;
	}
	if(R.f & (16+1) ) {		    /* om de juiste zbuffer Z voor transp en halo's terug te halen */
		xd= jit[0][0];
		yd= jit[0][1];
		Zjitx= -xd;
		Zjity= -yd;
		setwindowclip(0,0);
		printrenderinfo(v);
		zbufferall(0);
	}

	R.svlako= -1;
	rd= R.rectdaps;
	rz= R.rectz;
	colrt= (char *)R.rectot;
	col= (char *)&coladr;


	fullmask= (1<<G.osa)-1;
	/* de rowbuf is 4 pixels breder dan het plaatje! */
	rowbuf1= callocN(3*(R.rectx+4)*4*4, "ZbufshadeDA3");
	rowbuf2= callocN(3*(R.rectx+4)*4*4, "ZbufshadeDA3");
	rowbuf3= callocN(3*(R.rectx+4)*4*4, "ZbufshadeDA3");

	for(y=0;y<=R.recty;y++) {

		rb1= rowbuf1;
		rb2= rowbuf2;
		rb3= rowbuf3;

		if(y<R.recty) {
			for(x=0;x<R.rectx;x++,rd++) {

				if(*rd & 0xFF000000) { /* is PS */
					ps= (struct PixStr *)*rd;
					mask= 0;

					while(ps) {
						b= centmask[ps->mask];
						xs= (float)x+centLut[b & 15];
						ys= (float)y+centLut[b>>4];

						shadepixel(xs, ys, ps->vlak);

						if(shortcol[0]) {
							add_filt_mask(ps->mask, shortcol, rb1, rb2, rb3);
						}
						mask |= ps->mask;

						ps= ps->next;
					}
					ps= (struct PixStr *)*rd;
					mask= (~mask) & fullmask;

					b= centmask[mask];
					xs= (float)x+centLut[b & 15];
					ys= (float)y+centLut[b>>4];

					shadepixel(xs, ys, ps->vlak0);

					if(shortcol[0]) {
						add_filt_mask(mask, shortcol, rb1, rb2, rb3);
					}
				}
				else {
					shadepixel((float)x,(float)y,*rd);
					if(shortcol[0]) {
						add_filt_mask(fullmask, shortcol, rb1, rb2, rb3);
					}
				}

				rb1+=4; 
				rb2+=4; 
				rb3+=4;
			}
		}
		if(y>0) {
			colrb= (ushort *)(rowbuf3+4);
			for(x=0; x<R.rectx; x++,colrt+=4) {
				/* colrt[0]= colrb[0]>>8; */
				colrt[0]= *( (char *) (gamtab+colrb[0]) );
				colrt[1]= *( (char *) (gamtab+colrb[2]) );
				colrt[2]= *( (char *) (gamtab+colrb[4]) );
				colrt[3]= *( (char *) (gamtab+colrb[6]) );
				/* gamtabdit(colrb, colrt); */
				/* if(colrt[0]!=255 && colrt[0]!=0) if(getbutton(LEFTMOUSE)) */
				/* 	PRINT4(d, d, d, d, colrt[0], colrt[1], colrt[2], colrt[3]); */
				colrb+= 8;
			}

			if( (R.f & 1) && R.edgeint==211) {
				/* van deze pixels zijn de pixstr al 1 scanline oud */
				scanlinehaloPS(rz-R.rectx, rd-2*R.rectx, colrt-4*R.rectx, y-1);
			}

			if(R.f & 16) {
				abufsetrow(y-1);
				acol= Acolrow;
				colrt-= 4*R.rectx;
				
				for(x=0; x<R.rectx; x++, colrt+=4, acol+=4) {
					if(acol[0]) {
						tempcol[0]= (acol[0]>>8);
						tempcol[1]= (acol[1]>>8);
						tempcol[2]= (acol[2]>>8);
						tempcol[3]= (acol[3]>>8);
						addalphaOver(colrt, tempcol);
					}
				}
			}

			if( (R.f & 1) && R.edgeint!=211) {
				/* van deze pixels zijn de pixstr al 1 scanline oud */
				scanlinehaloPS(rz-R.rectx, rd-2*R.rectx, colrt-4*R.rectx, y-1);
			}
		}
		if(y<R.recty) {
			blkclr(rowbuf3, (R.rectx+4)*4*4);
			rb3= rowbuf3;
			rowbuf3= rowbuf2;
			rowbuf2= rowbuf1;
			rowbuf1= rb3;

			if( y>0 && G.background!=1) {
				y--;
				if(y & 1) {
					suspendtimer();
					lrectwrite(0,y-1,R.rectx-1,y,(ulong *) (colrt-8*R.rectx));
					restarttimer();
				}
				y++;
			}
			rz+= R.rectx;
		}
		if(G.afbreek) break;
	}

	if((R.f & (1024)) && G.afbreek==0) {
		rt= R.rectot;
		rp= R.rectaccu;
		for(a= R.rectx*R.recty; a>0; a--, rt++, rp++) {
			addalphaOver(rt,rp);
		}
	}
	if(wrld.dofi) {
		setzvalues_dof();	/* loopt pixstructen af en vult in zbuffer de minimumwaardes in */
	}
	freeN(R.rectdaps); 
	freeps();
	freeN(rowbuf1); 
	freeN(rowbuf2); 
	freeN(rowbuf3);
	R.rectdaps= 0;

	if(R.f & (1024)) if(R.rectaccu) freeN(R.rectaccu);
	R.rectaccu= 0;
	if(R.f & 16) endaccumbuf();
}

void zbufshade()
{
	extern ushort *Acolrow;
	struct VertRen *ver,*addvert();
	ulong a,*rz,*rp;
	float fy;
	long x,y;
	ushort *acol;
	char *charcol, *rt;

	Zjitx=Zjity= -.5;

	zbufferall(0);

	/* SHADE */
	rp= R.rectot;
	charcol= (char *)shortcol;

	if(R.f & 16) bgnaccumbuf();

	for(y=0;y<R.recty;y++) {
		R.svlako= -1;
		fy= y;
		
		if(R.f & 16) {		/* zbuf tra */
			abufsetrow(y);
			acol= Acolrow;
			
			for(x=0; x<R.rectx; x++, rp++, acol+= 4) {
				shadepixel((float)x, fy, *rp);
				
				if(acol[0]) addAlphaOverShort(shortcol, acol);
				
				if(shortcol[0]) {
					rt= (char *)rp;
					rt[0]= charcol[0];
					rt[1]= charcol[2];
					rt[2]= charcol[4];
					rt[3]= charcol[6];
				}
				else *rp= 0;
			}
		}
		else {
			for(x=0;x<R.rectx;x++,rp++) {
				shadepixel((float)x,fy,*rp);
				if(shortcol[0]) {
					rt= (char *)rp;
					rt[0]= charcol[0];
					rt[1]= charcol[2];
					rt[2]= charcol[4];
					rt[3]= charcol[6];
				}
				else *rp= 0;
			}
		}
		
		if((y & 1) && G.background!=1) {
			suspendtimer();
			lrectwrite(0,y-1,R.rectx-1,y,rp-2*R.rectx);
			restarttimer();
		}
		
		if(G.afbreek) break;
	}

	if(R.f & 16) endaccumbuf();

	if((R.f & 1) && G.afbreek==0) halovert(0);

}


void shadegour(vlr,rz)	/* stopt kleuren in vlaknormaal */
struct VlakRen *vlr;
char *rz;
{
	struct VertRen *v1;
	float n1[3];
	ulong *rectz,*xs,*ys;
	short a, tempgam;
	char *c1, *charcol;

	if(vlr->colg) {
		memcpy(rz,vlr->n,12);
		return;
	}

	charcol= (char *)shortcol;
	rectz=(ulong *)rz;
	memcpy(&R.col,vlr->col,sizeof(struct ColBlckF));

	if( (vlr->col->mode & 1)==0) { 
		VECCOPY(n1,vlr->n); 
	}

	tempgam= usegamtab;
	usegamtab= 0;
	R.osatex= 0;
	
	for(a=0;a<3;a++) {
		if(a==0) {
			v1=vlr->v1;
			c1=(char *)vlr->n;
		} else if(a==1) {
			memcpy(&R.col,vlr->col,60);
			v1=vlr->v2;
			c1+=4;
		} else {
			memcpy(&R.col,vlr->col,60);
			v1=vlr->v3;
			c1+=4;
		}

		VECCOPY(R.co,v1->co);
		VECCOPY(R.view,R.co);
		Normalise(R.view);
		if(R.col.texco & 4) {
			if(v1->v) {
				VECCOPY(R.lo, v1->v->c);
			}
		}
		if(R.col.texco & 8) {
			if(v1->v) {
				VECCOPY(R.gl, v1->v->c);
			}
		}

		if(R.col.mode & 1) {

			if(vlr->puno & (1<<a)) {
				R.vn[0]= -v1->n[0]; 
				R.vn[1]= -v1->n[1]; 
				R.vn[2]= -v1->n[2];
			} else {
				R.vn[0]= v1->n[0]; 
				R.vn[1]= v1->n[1]; 
				R.vn[2]= v1->n[2];
			}

			if(R.col.texco & 2) {
				VECCOPY(R.orn, R.vn);
				R.orn[0]*= 32767; 
				R.orn[1]*= 32767; 
				R.orn[2]*= 32767;
			}
		}
		else {
			VECCOPY(R.vn,n1);
			if(R.col.texco & 2) {
				R.orn[0]= vlr->vl->n[0];
				R.orn[1]= vlr->vl->n[1];
				R.orn[2]= vlr->vl->n[2];
			}
		}
		shadelamplus();

		c1[1]= charcol[2]; 
		c1[2]= charcol[4]; 
		c1[3]= charcol[6];

	}
	usegamtab= tempgam;
	
	memcpy(rz,vlr->n,12);
	vlr->colg= 1;
}


void drawgour(vlr, col)
struct VlakRen *vlr;
ulong *col;
{
	bgnpolygon();
	cpack(col[0]);
	v3f(vlr->v1->co);
	cpack(col[1]);
	v3f(vlr->v2->co);
	cpack(col[2]);
	v3f(vlr->v3->co);
	endpolygon();
}


void drawwire(vlr, col)
struct VlakRen *vlr;
ulong *col;
{
	long flags;
	
	if (vlr->vl) flags = vlr->vl->ec >> 5;
	else flags = vlr->ec & 7;
	
	switch (flags) {
	case 0:
		bgnclosedline();
		cpack(col[0]);
		v3f(vlr->v1->co);
		cpack(col[1]);
		v3f(vlr->v2->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		endclosedline();
		break;
	case 1:
		bgnline();
		cpack(col[0]);
		v3f(vlr->v1->co);
		cpack(col[1]);
		v3f(vlr->v2->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		endline();
		break;
	case 2:
		bgnline();
		cpack(col[1]);
		v3f(vlr->v2->co);
		cpack(col[0]);
		v3f(vlr->v1->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		endline();
		break;
	case 3:
		bgnline();
		cpack(col[0]);
		v3f(vlr->v1->co);
		cpack(col[1]);
		v3f(vlr->v2->co);
		endline();
		break;
	case 4:
		bgnline();
		cpack(col[1]);
		v3f(vlr->v2->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		cpack(col[0]);
		v3f(vlr->v1->co);
		endline();
		break;
	case 5:
		bgnline();
		cpack(col[1]);
		v3f(vlr->v2->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		endline();
		break;
	case 6:
		bgnline();
		cpack(col[0]);
		v3f(vlr->v1->co);
		cpack(col[2]);
		v3f(vlr->v3->co);
		endline();
		break;
	default:
		break;
	}
}


void hiddenline()	/* ZBUFFER EN GOUROUD WIREFRAME*/
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct ColBlckF *cb;
	ulong a,p,col[3], zero[3];
	short tra=0;

	subpixel(TRUE);
	linesmooth(SML_ON + SML_SMOOTHER + SML_END_CORRECT);
	swritemask(1);
	sclear(0);

	shademodel(GOURAUD);
	cb=0;
	stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
	zero[0] = zero[1] = zero[2] = 0;
	
	for(a=0;a<G.totvlak;a++) {
		if((a & 255)==0) {
			vlr= R.blovl[a>>8];
			if(G.afbreek) break;
		}
		else vlr++;

		if(vlr->rt1) {
			if(R.f & 16) {	/* transp */
				if(vlr->col!=cb) {
					cb= vlr->col;
					tra= (cb->mode & 192);
				}
			}
			if(tra==0) {
				shadegour(vlr,col);
				drawwire(vlr, col);
				
				stencil(TRUE,0,SF_EQUAL,1,ST_KEEP,ST_KEEP,ST_KEEP);
				drawgour(vlr, zero);

				stencil(TRUE,0,SF_ALWAYS,1,ST_INVERT,ST_INVERT,ST_INVERT);
				drawwire(vlr, col);
			}
		}
		if(G.afbreek) break;
	}

	shademodel(FLAT);
	subpixel(FALSE);
	linesmooth(FALSE);
	stencil(FALSE,0,0,0,0,0,0);
}


void zbufwire()	/* ZBUFFER EN GOUROUD WIREFRAME*/
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct ColBlckF *cb;
	ulong a,p,col[3];
	short tra=0;

	subpixel(TRUE);
	linesmooth(SML_ON + SML_SMOOTHER + SML_END_CORRECT);

	shademodel(GOURAUD);
	cb=0;
	for(a=0;a<G.totvlak;a++) {
		if((a & 255)==0) {
			vlr= R.blovl[a>>8];
			if(G.afbreek) break;
		}
		else vlr++;

		if(vlr->rt1) {
			if(R.f & 16) {	/* transp */
				if(vlr->col!=cb) {
					cb= vlr->col;
					tra= (cb->mode & 192);
				}
			}
			if(tra==0) {
				shadegour(vlr,col);
				drawwire(vlr,col);
			}
		}
		if(G.afbreek) break;
	}

	shademodel(FLAT);
	subpixel(FALSE);
	linesmooth(FALSE);
}


void zbufgour()	/* ZBUFFER EN GOUROUD */
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct ColBlckF *cb;
	ulong a,p,col[3];
	short tra=0;

	shademodel(GOURAUD);
	cb=0;
	for(a=0;a<G.totvlak;a++) {
		if((a & 255)==0) {
			vlr= R.blovl[a>>8];
			if(G.afbreek) break;
		}
		else vlr++;

		if(vlr->rt1) {
			if(R.f & 16) {	/* transp */
				if(vlr->col!=cb) {
					cb= vlr->col;
					tra= (cb->mode & 192);
				}
			}
			if(tra==0) {
				shadegour(vlr,col);
				drawgour(vlr,col);
			}
		}
		if(G.afbreek) break;
	}
}


void addaccu(z,t)
register char *z,*t;
{
	register short div,mul;

	mul= *t;
	div= mul+1;
	(*t)++;

	t[1]= (mul*t[1]+z[1])/div;
	t[2]= (mul*t[2]+z[2])/div;
	t[3]= (mul*t[3]+z[3])/div;

}

void accumkey()		/* enkel en osa */
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct ColBlckF *cb;
	float fac,lutf[66];
	ulong a,a1,*rt,*ra,*rz;
	short x,y,y1,v,tot=1;
	char luti[66],*ct,*ca;

	Zjitx= Zjity= -.5;

	if(G.ali) {
		tot= G.osa;
		R.rectaccu= (ulong *)callocN(4*R.rectx*R.recty+4,"accumkey");
	}

	for(v=0;v<tot;v++) {

		if(tot>1) setwindowclip(0,v);
		printrenderinfo(v);

		if(v>0) {		/* rectot is gewist in initrender */
			fillrect(R.rectot,R.rectx,R.recty,0);
		}
		if(G.ali) {
			Zjitx= -jit[v % tot][0];
			Zjity= -jit[v % tot][1];
		}

		zbufferall(1);

		if(G.afbreek || tot==1) break;

		/* accumuleren */
		rt= R.rectot;
		ra= R.rectaccu;
		for(y=0;y<R.recty;y++) {
			if(v==0) {  	/* alleen alfa zetten */
				for(x=R.rectx;x>0;x--,rt++,ra++) if(*rt) *ra= (*rt | 0x1000000);
			}
			else {
				for(x=R.rectx;x>0;x--,ra++,rt++) {
					if(*rt) {
						addaccu(rt,ra);
					}
				}
			}
			if(G.afbreek) break;
		}
		if(G.afbreek) break;
	}

	if(G.afbreek==0) {
		if(tot==1) {
			rt= R.rectot;
			for(y=0;y<R.recty;y++) {
				for(x=0;x<R.rectx;x++,rt++) {
					if(*rt) *rt|=0xFF000000;
				}
			}
		}
		else {
			/* LUT maken */
			for(y=0;y<=tot;y++) {
				fac= ((float)y)/tot;
				luti[y]= (255*fac);
				lutf[y]= fac;
			}
			/* alpha getallen maken */
			ct= (char *)R.rectot;
			ca= (char *)R.rectaccu;
			for(y=0;y<R.recty;y++) {
				for(x=0;x<R.rectx;x++,ct+=4,ca+=4) {
					if(ca[0]==0) *( (ulong *)ct)= 0;
					else if(ca[0]==tot) *( (ulong *)ct)= (*( (ulong *)ca) | 0xFF000000);
					else {
						fac= lutf[ca[0]];
						ct[0]= luti[ca[0]];
						ct[1]= ca[1]*fac;
						ct[2]= ca[2]*fac;
						ct[3]= ca[3]*fac;
					}
				}
			}
		}
	}

	if(R.rectaccu) {
		freeN(R.rectaccu);
		R.rectaccu= 0;
	}

	if((R.f & 1) && G.afbreek==0) halovert(0);

}

void accumgour()		/* enkel en osa */
{
	struct VlakRen *vlr;
	struct VertRen *ver;
	struct ColBlckF *cb;
	float fac,lutf[66];
	ulong a,a1,*rt,*rp,*rectp,*rectz,*rz;
	short x,y,y1,v,tot=1;
	char luti[66],*c;

	if(G.ali) tot= G.osa;

	rectp= (ulong *)mallocN(16*R.rectx+4,"rectaccumall1");
	rectz= (ulong *)mallocN(16*R.rectx+4,"rectaccumall2");

	for(v=0;v<tot;v++) {

		if(tot>1) setwindowclip(0,v);
		printrenderinfo(v);

		if(G.machine==ENTRY) czclear(0,0x7FFFFFFE);
		else czclear(0,0x7FFFFE);
		switch (R.f & (8 | 2048)) {
		case 8:
			zbufgour();
			break;
		case 2048:
			zbufwire();
			break;
		default:
			hiddenline();
			break;
		}
		
		if(G.afbreek) break;

		/* accumuleren */
		if(v==0) lrectread(0,0,R.rectx-1,R.recty-1,R.rectot);
		rt= R.rectot;
		for(y=0;y<R.recty;y+=4) {
			readsource(SRC_ZBUFFER);
			lrectread(0,y,R.rectx-1,y+3,rectz);
			readsource(SRC_AUTO);
			if(v) lrectread(0,y,R.rectx-1,y+3,rectp);
			rp= rectp;
			rz= rectz;
			if(v==0) {  /* alleen alfa zetten */
				for(y1=y;y1<y+4;y1++) {
					if(y1>=R.recty) break;
					if(G.machine==ENTRY) {
						for(x=R.rectx;x>0;x--,rt++,rz++) {
							if(*rz!=0x7FFFFFFE) (*rt)|=0x01000000;
						}
					}
					else {
						for(x=R.rectx;x>0;x--,rt++,rz++) {
							if(*rz!=0x7FFFFE) (*rt)|=0x01000000;
						}
					}
				}
			}
			else {
				for(y1=y;y1<y+4;y1++) {
					if(y1>=R.recty) break;
					if(G.machine==ENTRY) {
						for(x=R.rectx;x>0;x--,rp++,rt++,rz++) {
							if(*rz!=0x7FFFFFFE) {
								addaccu(rp,rt);
							}
						}
					}
					else {
						for(x=R.rectx;x>0;x--,rp++,rt++,rz++) {
							if(*rz!=0x7FFFFE) {
								addaccu(rp,rt);
							}
						}
					}
				}
			}
			if(G.afbreek) break;
		}
		if(G.afbreek) break;
	}

	if(G.afbreek==0) {
		/* LUT maken */
		for(y=0;y<=tot;y++) {
			luti[y]= (255*y)/tot;
			lutf[y]= ((float)y)/tot;
		}
		/* alpha getallen maken */
		c= (char *)R.rectot;
		for(y=0;y<R.recty;y++) {
			for(x=0;x<R.rectx;x++,c+=4) {
				if(c[0]==0);
				else if(c[0]==tot) c[0]=255;
				else {
					fac= lutf[c[0]];
					c[0]= luti[c[0]];
					c[1]*=fac;
					c[2]*=fac;
					c[3]*=fac;
				}
			}
		}
	}

	freeN(rectp);
	freeN(rectz);

	if((R.f & 1) && G.afbreek==0) halovert(1);

}



