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

  /*	textu.c	*/


/*	 
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

	struct Textudata

	inittexture(soort)
struct Textudata *inittextudata(soort)

	kleur()
	wood()
	marmer()
	tover()
	imagewrap()
	imagewraposa()
	tiles()
	blend()
	stucci()

	multitex()

	objectex(col,nr)
	skytex(nr)
*/

#include <math.h>
#include <string.h>
#include <gl/device.h>
#include <local/iff.h>
#include <stdlib.h>		/* rand */
#include "/usr/people/include/Button.h"	/* BTST */
#include "/usr/people/include/Trace.h"

struct Tex *rtex;
float t[3], dxt[3],dyt[3];
float Tin, Tinc, Titot, Tr, Tg, Tb, Ta;
short norm, rgb, Talpha, Tusealpha, imapXrepeat, imapYrepeat;

extern float hnoise(), turbulence();

/* onder in dit object: */
void clipx_rctf_swap();
void clipy_rctf_swap();

void converttopremul(ibuf)
struct ImBuf *ibuf;
{
	int x, y, val;
	char *cp;
	
	if(ibuf==0) return;
	if(ibuf->depth==24) {	/* alpha op 255 zetten */

		cp= (char *)(ibuf->rect);
		for(y=0; y<ibuf->y; y++) {
			for(x=0; x<ibuf->x; x++, cp+=4) {
				cp[0]= 255;
			}
		}
		return;
	}
	
	cp= (char *)(ibuf->rect);
	for(y=0; y<ibuf->y; y++) {
		for(x=0; x<ibuf->x; x++, cp+=4) {
			if(cp[0]==0) {
				cp[1]= cp[2]= cp[3]= 0;
			}
			else if(cp[0]!=255) {
				val= cp[0];
				cp[1]= (cp[1]*val)>>8;
				cp[2]= (cp[2]*val)>>8;
				cp[3]= (cp[3]*val)>>8;
			}
		}
	}
}

void delImap(tex)
struct Tex *tex;
{
	struct Imap *ima;
	struct MipMap *mm, *mmn;

	if(tex==0) return;
	ima= tex->ima;
	if(ima==0) return;
	ima->us--;
	if(ima->us<1) {
		if(ima->ibuf) freeImBuf(ima->ibuf);
		if(ima->anim) free_anim(ima->anim);
		mm= ima->mipmap;
		while(mm) {
			mmn= mm->next;
			freeImBuf(mm->ibuf);
			freeN(mm);
			mm= mmn;
		}
		freeN(ima);
		tex->ima= 0;
	}
}

void addImap(tex,str)
struct Tex *tex;
char *str;
{
	short a;

	if(tex->soort!=5) return;
	if(str==0 || str[0]!='/') return;

	if(tex->ima!=0) {
		if(tex->ima->ibuf==0) tex->ima->ok= 1;
		if(strcmp(tex->ima->name,str)==0  && tex->var[9] ) return;
		delImap(tex);
	}

	/* bestaat er een met dezelfde naam? */
	for(a=1;a<=G.totex;a++) {
		if(G.adrtex[a]->soort==5 && G.adrtex[a]->ima!=0) {
			if(strcmp(G.adrtex[a]->ima->name,str)==0) {
				tex->ima= G.adrtex[a]->ima;
				tex->ima->us++;
				return;
			}
		}
	}

	tex->ima= callocstructN(struct Imap,1,"addImap");
	tex->ima->us= 1;
	tex->ima->ok= 1;
	strcpy(tex->ima->name,str);

}


void cylindermap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float len;

	*adr2 = (z + 32768.0) / 65536.0;

	len= fsqrt(x*x+y*y);
	if(len>0) {
		*adr1 = (1.0 - (fatan2(x/len,y/len) / M_PI)) / 2.0;
	}
}

void spheremap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float len;

	len= fsqrt(x*x+y*y+z*z);
	if(len>0) {
		*adr1 = (1.0 - fatan2(x,y)/M_PI )/2.0;
		z/=len;
		*adr2 = 1.0- facos(z)/M_PI;
	}
}

long cubemap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float x1, y1, z1;
	long ret;
	short *nor;
	
	
	if(R.vlr && R.vlr->vl) {
		nor= R.vlr->vl->n;
		x1= abs(nor[0]);
		y1= abs(nor[1]);
		z1= abs(nor[2]);
	}
	else {
		x1 = fabs(x);
		y1 = fabs(y);
		z1 = fabs(z);
	}
	
	if(z1>=x1 && z1>=y1) {
		*adr1 = x;
		*adr2 = y;
		ret= 0;
	}
	else if(y1>=x1 && y1>=z1) {
		*adr1 = x;
		*adr2 = z;
		ret= 1;
	}
	else {
		*adr1 = y;
		*adr2 = z;
		ret= 2;
	}

	*adr1 = (*adr1 + 32768.0) / 65536.0;
	*adr2 = (*adr2 + 32768.0) / 65536.0;
	
	return ret;
}


extern struct ImBuf * zoomImBuf(struct ImBuf * ibuf, long newx, long newy, int filttype, double blur);

void makemipmap(ima)
struct Imap *ima;
{
	struct ImBuf *ibuf;
	struct MipMap *mm, *mmn;
	short minsize;

	minsize= MIN2(ima->ibuf->x, ima->ibuf->y);
	if(minsize<4) return;

	mm= callocstructN(struct MipMap, 1, "makemipmap");
	ima->mipmap= mm;
	
	ibuf= dupImBuf(ima->ibuf);
	filter(ibuf);
	mm->ibuf= (struct ImBuf *)onehalf(ibuf);
	freeImBuf(ibuf);

	/*mm->ibuf = zoomImBuf(ima->ibuf, ima->ibuf->x / 2, ima->ibuf->y / 2, 2, 1.0);*/ 

	minsize= MIN2(mm->ibuf->x, mm->ibuf->y);

	while(minsize>3) {
		mmn= callocstructN(struct MipMap, 1, "makemipmap1");

		ibuf= dupImBuf(mm->ibuf);
		filter(ibuf);
		mmn->ibuf= (struct ImBuf *)onehalf(ibuf);
		freeImBuf(ibuf);

		/*mmn->ibuf = zoomImBuf(mm->ibuf, mm->ibuf->x / 2, mm->ibuf->y / 2, 2, 1.0); */

		mm->next= mmn;
		mm= mmn;

		minsize= MIN2(mm->ibuf->x, mm->ibuf->y);
	}
}

struct anim *openanim(char * name)
{
	struct anim * anim;
	struct ImBuf * ibuf;
	
	anim = open_anim(name, ANIM_DFLT, IB_planes);
	if (anim == 0) return(0);

	 
	ibuf = anim_absolute(anim, 0);
	if (ibuf == 0) {
		free_anim(anim);
		return(0);
	}
	freeImBuf(ibuf);
	
	return(anim);
}

int calcimanr(cfra, animlen, sfra, fie_ima, cycl)
int cfra, animlen, sfra, fie_ima, cycl;
{
	int imanr, ofset;

	/* hier (+fie_ima/2-1) zorgt ervoor dat correct wordt gedeeld */
	
	if(R.f & 64) ofset= 1;
	else ofset= 0;
	
	if(fie_ima) imanr= (ofset+(cfra-sfra+1+fie_ima/2-1)*2)/fie_ima;
	else imanr= 1;
	if(fie_ima==1) imanr++;
		
	if(cycl && animlen) {
		while(imanr<=0) imanr+= animlen;
		while(imanr>animlen) imanr-= animlen;
	}
	else if(animlen) {
		if(imanr<1) imanr= 1;
		if(imanr>animlen) imanr= animlen;
	}
	return imanr;
}

void ima_ibuf_is_nul(ima)
struct Imap *ima;
{
	struct ImBuf *ibuf;
	long imanr;
	short curs;
	char str[100];
	
	curs= getcursorN();
	if(curs!=2) setcursorN(2);	/* pas op: doet winset */
	
	strcpy(str, ima->name);
	convertstringcode(str);

	if(rtex->type==2) ima->ibuf = loadiffname(str , IB_rect | IB_fields);
	else if(rtex->type==3) {
		if(ima->anim==0) ima->anim = openanim(str);
		if(ima->anim) {
			
			/*                cfra      animlen       sfra           fie/ima       cycl    */
			imanr= calcimanr(G.cfra, rtex->var[9], rtex->var[11], rtex->var[7], rtex->var[10]);

			ima->lastframe= imanr;
			ima->lastquality= G.ali;

			ima->ibuf = anim_absolute(ima->anim, imanr-1);

			if(ima->ibuf) {
				ibuf= ima->ibuf;
				/**** PATCH OM KLEUR 3 GOED TE KUNNEN ZETTEN MORKRAMIA */
				
				if (ibuf->planes != 0 && ibuf->cmap != 0 && ibuf->maxcol > 4) {
				/* 	if(view0.grid==5005) ibuf->cmap[8] = 0xFFD8472B; */
					ibuf->cmap[2] = 0x80000000;
				}
				
				/* convert_ibuf(ima->ibuf, IB_rect); bestaat niet meer */
				
				if(G.ali) {
					converttopremul(ima->ibuf);
					antialias(ima->ibuf);
				}
				else {
					if (ima->ibuf->x > 256 && ima->ibuf->y > 256) {
						ima->ibuf= (struct ImBuf *)onehalf(ibuf);
						freeImBuf(ibuf);
					}
					converttopremul(ima->ibuf);
				}
			}
		}

	}
	else {
		ima->ibuf = loadiffname(str , LI_rect);
		ima->lastquality= G.ali;
		if(ima->ibuf) {
			if(G.ali == 0 && ima->ibuf->x > 256 && ima->ibuf->y > 256) {
				ibuf= ima->ibuf;
				ima->ibuf= (struct ImBuf *)onehalf(ibuf);
				freeImBuf(ibuf);
			}
		}
		if(rtex->type!=4) converttopremul(ima->ibuf);
	}
	
	if (ima->ibuf==0) ima->ok= 0;
	ima->mipmap= 0;
	
	if(curs!=2) setcursorN(curs);
	
}

void imagewrap()
{
	struct Imap *ima;
	struct ImBuf *ibuf;
	float co[2],ft,fx,fy, fac1, fac2, fac3, fac4, val1, val2, val3;
	long ofs;
	int x, y;
	char *rect;

	ima= rtex->ima;

	if(ima==0 || ima->ok== 0) return;

	if(ima->ibuf==0) ima_ibuf_is_nul(ima);

	if (ima->ok) {

		if(rtex->var[6]) {	/* mipmap */
			if(ima->mipmap==0) makemipmap(ima);
		}

		ibuf = ima->ibuf;

		if( (R.f & 64) && (ibuf->flags & IB_fields) ) {
			/* PATCH */
			/* if(view0.grid==5001) { */
			/* 	if(ima->lastframe!=rtex->var[9]) ibuf->rect+= (ibuf->x*ibuf->y);; */
			/* } */
			/* else */
			ibuf->rect+= (ibuf->x*ibuf->y);
		}

		x= rtex->var[0];
		if(x==0 || x==3) ;
		else if(x==1 || x==4) {
			ft = t[1];
			t[1] = t[2];
			t[2] = ft;
		}
		else {
			ft = t[0];
			t[0] = t[1];
			t[1] = t[2];
			t[2] = ft;
		}

		x= rtex->var[3];
		if(x==0) {
			fx = (t[0] + 32768.0) / 65536.0;
			fy = (t[1] + 32768.0) / 65536.0;
		}
		else if(x==1) cylindermap(t[0], t[1], t[2], &fx, &fy);
		else if(x==2) spheremap(t[0], t[1], t[2], &fx, &fy);
		else cubemap(t[0], t[1], t[2], &fx, &fy);
		
		if(rtex->var[0] > 2) {
			ft= fx;
			fx= fy;
			fy= ft;
		}
		
		if(rtex->var[1]>1) {
			fx *= rtex->var[1];
			fx -= (long)(fx);
		}
		if(rtex->var[2]>1) {
			fy *= rtex->var[2];
			fy -= (long)(fy);
		}

		if(rtex->var[4]) {	/* x Crop */
			fac1= ((float)rtex->var[4])/100.0;
			fac2= 1.0-fac1;
			fx= fx*fac2+ 0.5*fac1;
		}
		if(rtex->var[5]) {	/* y Crop */
			fac1= ((float)rtex->var[5])/100.0;
			fac2= 1.0-fac1;
			fy= fy*fac2+ 0.5*fac1;
		}

		/* if(fx<0.0) fx= 0.0; else if(fx>1.0) fx= 1.0; */
		/* if(fy<0.0) fy= 0.0; else if(fy>1.0) fy= 1.0; */

		x = fx * ibuf->x;
		y = fy * ibuf->y;

		if(x<0) x= 0; else if(x>=ibuf->x) x= ibuf->x-1;
		if(y<0) y= 0; else if(y>=ibuf->y) y= ibuf->y-1;

		ofs = y * ibuf->x + x;
		rect = (char *)( ibuf->rect+ ofs);

		/* vanaf nu heeft elke image een alpha */
		if(Tusealpha) Talpha= 1;
		if(rtex->var[8]==0) Talpha= 0;

		Tb = ((float)rect[1])/255.0;
		Tg = ((float)rect[2])/255.0;
		Tr = ((float)rect[3])/255.0;
		Tin= (Tr+Tg+Tb)/3.0;

		if(Talpha) {			
			Ta= Tin= ((float)rect[0])/255.0;
		}

		if(rtex->type==1) {
			/* bump: drie samples nemen */
			val1= Tr+Tg+Tb;

			if(x<ibuf->x-1) {
				rect+=4;
				val2= ((float)(rect[1]+rect[2]+rect[3]))/255.0;
				rect--;
			}
			else val2= val1;

			if(y<ibuf->y-1) {
				rect+= 4*ibuf->x;
				val3= ((float)(rect[1]+rect[2]+rect[3]))/255.0;
			}
			else val3= val1;
			
			val3= rtex->bintf*(val1-val3);
			val2= rtex->bintf*(val1-val2);
			R.vn[0]+= val3;
			R.vn[1]+= val2;
			Normalise(R.vn);
		}


		if( (R.f & 64) && (ibuf->flags & IB_fields) ) {
			/* PATCH */
			/* if(view0.grid==5001) { */
			/* 	if(ima->lastframe!=rtex->var[9]) ibuf->rect-= (ibuf->x*ibuf->y);; */
			/* } */
			/* else  */
			ibuf->rect-= (ibuf->x*ibuf->y);
		}
	}

	rgb=1;
}

float square_rctf(rf)
rctf *rf;
{
	float x, y;

	x= rf->xmax- rf->xmin;
	y= rf->ymax- rf->ymin;
	return (x*y);
}

void clipx_rctf(rf, x1, x2)
rctf *rf;
float x1, x2;
{
	if(rf->xmin<x1) rf->xmin= x1;
	if(rf->xmax>x2) rf->xmax= x2;
	if(rf->xmin > rf->xmax) rf->xmin = rf->xmax;

}

void clipy_rctf(rf, y1, y2)
rctf *rf;
float y1, y2;
{
	if(rf->ymin<y1) rf->ymin= y1;
	if(rf->ymax>y2) rf->ymax= y2;
	if(rf->ymin > rf->ymax) rf->ymin = rf->ymax;

}

void boxsampleclip(ibuf,rf, rcol, gcol, bcol, acol)	/* return kleur 0.0-1.0 */
struct ImBuf *ibuf;
rctf *rf;
float *rcol, *gcol, *bcol, *acol;
{
	/* sample box, is reeds geclipt en minx enz zijn op ibuf size gezet.
     * Vergroot uit met antialiased edges van de pixels */

	float muly,mulx,div;
	long ofs;
	short x,y,startx,endx,starty,endy;
	char *rect;

	startx= ffloor(rf->xmin);
	endx= ffloor(rf->xmax);
	starty= ffloor(rf->ymin);
	endy= ffloor(rf->ymax);

	if(endx>=ibuf->x) endx= ibuf->x-1;
	if(endy>=ibuf->y) endy= ibuf->y-1;

	if(starty==endy && startx==endx) {

		ofs = starty*ibuf->x + startx;
		rect = (char *)(ibuf->rect +ofs);
		*bcol= ((float)rect[1])/255.0;
		*gcol= ((float)rect[2])/255.0;
		*rcol= ((float)rect[3])/255.0;
			/* alpha is globaal, reeds gezet in functie imagewraposa() */
		if(Talpha) {
			*acol= ((float)rect[0])/255.0;
		}
	}
	else {
		div= *rcol= *gcol= *bcol= *acol= 0.0;
		for(y=starty;y<=endy;y++) {
			ofs = y*ibuf->x +startx;
			rect = (char *)(ibuf->rect+ofs);

			muly= 1.0;

			if(starty==endy);
			else {
				if(y==starty) muly= 1.0-(rf->ymin - y);
				if(y==endy) muly= (rf->ymax - y);
			}
			if(startx==endx) {
				mulx= muly;
				if(Talpha) *acol+= mulx*rect[0];
				*bcol+= mulx*rect[1];
				*gcol+= mulx*rect[2];
				*rcol+= mulx*rect[3];
				div+= mulx;
			} else {
				for(x=startx;x<=endx;x++) {
					mulx= muly;
					if(x==startx) mulx*= 1.0-(rf->xmin - x);
					if(x==endx) mulx*= (rf->xmax - x);

					if(mulx==1.0) {
						if(Talpha) *acol+= rect[0];
						*bcol+= rect[1];
						*gcol+= rect[2];
						*rcol+= rect[3];
						div+= 1.0;
					}
					else {
						if(Talpha) *acol+= mulx*rect[0];
						*bcol+= mulx*rect[1];
						*gcol+= mulx*rect[2];
						*rcol+= mulx*rect[3];
						div+= mulx;
					}
					rect+=4;
				}
			}
		}
		/* if(div>0.0) { */
			div*= 255.0;
	
			*bcol/= div;
			*gcol/= div;
			*rcol/= div;
			
			if(Talpha) *acol/= div;
		/* } */
	}
}

void boxsample(ibuf, minx, miny, maxx, maxy, rcol, gcol, bcol, acol)	/* return kleur 0.0-1.0 */
struct ImBuf *ibuf;
float minx, miny, maxx, maxy;
float *rcol, *gcol, *bcol, *acol;
{
	/* Sample box, doet clip. minx enz lopen van 0.0 - 1.0 .
     * Vergroot uit met antialiased edges van de pixels.
     * Als globals imapXrepeat en/of imapYrepeat zijn gezet worden
     *  de weggeclipte stukken ook gesampled.
     */
	rctf *rf, stack[8];
	float opp, tot, r, g, b, a;
	short count=1;

	rf= stack;
	rf->xmin= minx*ibuf->x;
	rf->xmax= maxx*ibuf->x;
	rf->ymin= miny*ibuf->y;
	rf->ymax= maxy*ibuf->y;

	if(imapXrepeat) clipx_rctf_swap(stack, &count, 0.0, (float)(ibuf->x));
	else clipx_rctf(rf, 0.0, (float)(ibuf->x));

	if(imapYrepeat) clipy_rctf_swap(stack, &count, 0.0, (float)(ibuf->y));
	else clipy_rctf(rf, 0.0, (float)(ibuf->y));

	if(count>1) {
		tot= *rcol= *bcol= *gcol= *acol= 0.0;
		while(count--) {
			boxsampleclip(ibuf, rf, &r, &g, &b, &a);
			
			opp= square_rctf(rf);
			tot+= opp;

			*rcol+= opp*r;
			*gcol+= opp*g;
			*bcol+= opp*b;
		/* alpha is globaal, reeds gezet in functie imagewraposa() */
			if(Talpha) *acol+= opp*a;
			rf++;
		}
		if(tot!= 0.0) {
			*rcol/= tot;
			*gcol/= tot;
			*bcol/= tot;
			if(Talpha) *acol/= tot;
		}
	}
	else {
		boxsampleclip(ibuf, rf, rcol, gcol, bcol, acol);
	}
}

void filtersample(ibuf, fx, fy, rcol, gcol, bcol, acol)   /* return kleur 0.0-1.0 */
struct ImBuf *ibuf;										/* fx en fy tussen 0.0 en 1.0 */
float fx, fy;
float *rcol, *gcol, *bcol, *acol;
{
	/* met weighted filter 3x3
     * de linker of rechter kolom is altijd 0
     * en de bovenste of onderste rij is altijd 0
     */

	long fac, fac1, fac2, fracx, fracy, filt[4], *poin= 0;
	long ix, iy, x4;
	ulong r=0, g=0, b=0, a=0;
	char *rowcol, *rfilt[4];

	ix= (long)( 256.0*fx );
	fracx= (ix & 255);
	ix= (ix>>8);
	iy= (long)( 256.0*fy );
	fracy= (iy & 255);
	iy= (iy>>8);
	
	if(ix>=ibuf->x) ix= ibuf->x-1;
	if(iy>=ibuf->y) iy= ibuf->y-1;
	
	rowcol= (char *)(ibuf->rect+ iy*ibuf->x +ix);

	rfilt[0]= rfilt[1]= rfilt[2]= rfilt[3]= rowcol;
	x4= 4*ibuf->x;

	if(fracx<128) {
		if(ix>0) { 
			rfilt[0]-= 4; 
			rfilt[2]-=4; 
		}
		else if(imapXrepeat) { 
			rfilt[0]+= x4-4; 
			rfilt[2]+= x4-4; 
		}

		if(fracy<128) {
			/* geval linksonder */
			fac1= 128+fracy;
			fac2= 128-fracy;

			if(iy>0) { 
				rfilt[3]-= x4; 
				rfilt[2]-= x4; 
			}
			else if(imapYrepeat) {
				fac= x4*(ibuf->y-1) ;
				rfilt[3]+= fac; 
				rfilt[2]+= fac;
			}
		}
		else {
			/* geval linksboven */
			fac2= 384-fracy;
			fac1= fracy-128;

			if(iy<ibuf->y-1) { 
				rfilt[1]+= x4; 
				rfilt[0]+= x4; 
			}
			else if(imapYrepeat) {
				fac= x4*(ibuf->y-1) ;
				rfilt[1]-= fac; 
				rfilt[0]-= fac;
			}
		}

		filt[1]=filt[3]= 128+ fracx;
		filt[0]=filt[2]= 128- fracx;
		filt[0]*= fac1; 
		filt[1]*= fac1;
		filt[2]*= fac2; 
		filt[3]*= fac2;
	}
	else {
		if(fracy<128) {
			/* geval rechtsonder */
			fac1= 128+fracy;
			fac2= 128-fracy;

			if(iy>0) { 
				rfilt[3]-= x4; 
				rfilt[2]-= x4; 
			}
			else if(imapYrepeat) {
				fac= x4*(ibuf->y-1) ;
				rfilt[3]+= fac; 
				rfilt[2]+= fac;
			}
		}
		else {
			/* geval rechtsboven */
			fac2= 384-fracy;
			fac1= fracy-128;

			if(iy<ibuf->y-1) { 
				rfilt[1]+= x4; 
				rfilt[0]+= x4; 
			}
			else if(imapYrepeat) {
				fac= x4*(ibuf->y-1) ;
				rfilt[1]-= fac; 
				rfilt[0]-= fac;
			}
		}
		filt[0]=filt[2]= 384-fracx;
		filt[1]=filt[3]= fracx-128;
		filt[0]*= fac1; 
		filt[1]*= fac1;
		filt[2]*= fac2; 
		filt[3]*= fac2;

		if(ix<ibuf->x-1) { 
			rfilt[1]+= 4; 
			rfilt[3]+=4; 
		}
		else if(imapXrepeat) { 
			rfilt[1]-= x4-4; 
			rfilt[3]-= x4-4; 
		}
	}

	for(fac=3; fac>=0; fac--) {
		rowcol= rfilt[fac];
		r+= filt[fac]*rowcol[3];
		g+= filt[fac]*rowcol[2];
		b+= filt[fac]*rowcol[1];
		if(Talpha) a+= filt[fac]*rowcol[0];		/* alpha is globaal */
	}
	*rcol= ((float)r)/16777216.0;
	*gcol= ((float)g)/16777216.0;
	*bcol= ((float)b)/16777216.0;
	if(Talpha) *acol= ((float)a)/16777216.0;
	
}


void imagewraposa()
{
	struct Imap *ima;
	struct ImBuf *ibuf, *previbuf;
	struct MipMap *mm;
	float co[2],ft,fx,fy,fz, minx,maxx,miny,maxy, dx, dy, fac1, fac2, fac3, fac4;
	float area[8], maxd, pixsize, val1, val2, val3;
	long proj;
	short x, ok, areaflag= 0;
	
	ima= rtex->ima;

	if(ima==0 || ima->ok== 0) return;

	if(ima->ibuf==0) ima_ibuf_is_nul(ima);

	if (ima->ok) {
	
		if(rtex->var[6]) {	/* mipmap */
			if(ima->mipmap==0) makemipmap(ima);
		}
	
		ibuf = ima->ibuf;
		
		if( (R.f & 64) && (ibuf->flags & IB_fields) ) {
			/* PATCH */
			/* if(view0.grid==5001) { */
			/* 	if(ima->lastframe!=rtex->var[9]) ibuf->rect+= (ibuf->x*ibuf->y);; */
			/* } */
			/* else  */
			ibuf->rect+= (ibuf->x*ibuf->y);
		}
		
		/* vanaf nu heeft elke image een alpha */
		if(Tusealpha) Talpha= 1;
		if(rtex->var[8]==0) Talpha= 0;
		
		x= rtex->var[0];
		if(x==0 || x==3) ;
		else if(x==1 || x==4) {
			ft = t[1];
			t[1] = t[2];
			t[2] = ft;

			ft = dxt[1];
			dxt[1] = dxt[2];
			dxt[2] = ft;

			ft = dyt[1];
			dyt[1] = dyt[2];
			dyt[2] = ft;
		}
		else {
			ft = t[0];
			t[0] = t[1];
			t[1] = t[2];
			t[2] = ft;

			ft = dxt[0];
			dxt[0] = dxt[1];
			dxt[1] = dxt[2];
			dxt[2] = ft;

			ft = dyt[0];
			dyt[0] = dyt[1];
			dyt[1] = dyt[2];
			dyt[2] = ft;
		
		}
		
		x= rtex->var[3];
		if(x==0) {
			fx = (t[0] + 32768.0) / 65536.0;
			fy = (t[1] + 32768.0) / 65536.0;
			dxt[0]/=65536.0; 
			dxt[1]/=65536.0;
			dyt[0]/=65536.0; 
			dyt[1]/=65536.0;
		}
		else if(x==1 || x==2) {	/* buis en bol */
			/* uitzondering: de naad achter (y<0.0) */
			ok= 1;
			if(t[1]<=0.0) {
				fx= t[0]+dxt[0];
				fy= t[0]+dyt[0];
				if(fx>=0.0 && fy>=0.0 && t[0]>=0.0);
				else if(fx<=0.0 && fy<=0.0 && t[0]<=0.0);
				else ok= 0;
			}
			if(ok) {
				if(x==1) {
					cylindermap(t[0],t[1],t[2],area,area+1);
					cylindermap(t[0]+dxt[0],t[1]+dxt[1],t[2]+dxt[2],area+2,area+3);
					cylindermap(t[0]+dyt[0],t[1]+dyt[1],t[2]+dyt[2],area+4,area+5);
				}
				else { 
					spheremap(t[0],t[1],t[2],area,area+1);
					spheremap(t[0]+dxt[0],t[1]+dxt[1],t[2]+dxt[2],area+2,area+3);
					spheremap(t[0]+dyt[0],t[1]+dyt[1],t[2]+dyt[2],area+4,area+5);
				}
				areaflag= 1;
			}
			else {
				if(x==1) cylindermap(t[0], t[1], t[2], &fx, &fy);
				else spheremap(t[0], t[1], t[2], &fx, &fy);
				dxt[0]/=65536.0; 
				dxt[1]/=65536.0;
				dyt[0]/=65536.0; 
				dyt[1]/=65536.0;
			}
		}
		else {
			proj= cubemap(t[0], t[1], t[2], &fx, &fy);
			if(proj==1) {
				dxt[1]= dxt[2];
				dyt[1]= dyt[2];
			}
			else if(proj==2) {
				dxt[0]= dxt[1];
				dyt[0]= dyt[1];
				dxt[1]= dxt[2];
				dyt[1]= dyt[2];
			}
			dxt[0]/=65536.0; 
			dxt[1]/=65536.0;
			dyt[0]/=65536.0; 
			dyt[1]/=65536.0;
		}
		
		/* als area dan dxt[] en dyt[] opnieuw berekenen */
		if(areaflag) {
			fx= area[0]; 
			fy= area[1];
			dxt[0]= area[2]-fx;
			dxt[1]= area[3]-fy;
			dyt[0]= area[4]-fx;
			dyt[1]= area[5]-fy;
		}

		/* flip x en y */
		if(rtex->var[0]>2) {
			ft= fx;
			fx= fy;
			fy= ft;

			ft= dxt[0];
			dxt[0]= dxt[1];
			dxt[1]= ft;

			ft= dyt[0];
			dyt[0]= dyt[1];
			dyt[1]= ft;
		}

		/* repeat */
		imapXrepeat= (rtex->var[1]>1);
		imapYrepeat= (rtex->var[2]>1);
		if(imapXrepeat) {
			fx *= rtex->var[1];
			dxt[0]*= rtex->var[1];
			dyt[0]*= rtex->var[1];
			if(fx>1.0) fx -= (long)(fx);
		}
		if(imapYrepeat) {
			fy *= rtex->var[2];
			dxt[1]*= rtex->var[2];
			dyt[1]*= rtex->var[2];
			if(fy>1.0) fy -= (long)(fy);
		}

		if(rtex->var[4]) {	/* x Crop */
			fac1= ((float)rtex->var[4])/100.0;
			fac2= 1.0-fac1;
			fx= fx*fac2+ 0.5*fac1;
			dxt[0]*= fac2;
			dyt[0]*= fac2;
		}
		if(rtex->var[5]) {	/* y Crop */
			fac1= ((float)rtex->var[5])/100.0;
			fac2= 1.0-fac1;
			fy= fy*fac2+ 0.5*fac1;
			dxt[1]*= fac2;
			dyt[1]*= fac2;
		}

		
		if(ibuf->flags & IB_fields) {
			if(R.f & 32) {			/* field render */
				if(R.f & 64) {		/* correctie voor tweede field */
					/* fac1= 0.5/( (float)ibuf->y ); */
					/* fy-= fac1; */
				}
				else {				/* eerste field */
					fac1= 0.5/( (float)ibuf->y );
					fy+= fac1;
				}
			}
		}
		
		if(fx<0.0) fx= 0.0; else if(fx>1.0) fx= 1.0;
		if(fy<0.0) fy= 0.0; else if(fy>1.0) fy= 1.0;

		/* pixel coordinaten */

		minx= MIN3(dxt[0],dyt[0],dxt[0]+dyt[0] );
		maxx= MAX3(dxt[0],dyt[0],dxt[0]+dyt[0] );
		miny= MIN3(dxt[1],dyt[1],dxt[1]+dyt[1] );
		maxy= MAX3(dxt[1],dyt[1],dxt[1]+dyt[1] );
		/* keuze: filter (MipMap) of niet */
		if(rtex->var[6]==0) {
			/* geen MipMap */
			if(rtex->type==1) {
				/* bump: drie samples nemen */
				minx= (maxx-minx);	/* hier niet de helft i.v.m. drie boxsamples */
				miny= (maxy-miny);
				if(minx>0.25) minx= 0.25;
				if(miny>0.25) miny= 0.25;
				
				boxsample(ibuf, fx-minx, fy-miny, fx, fy, &Tr, &Tg, &Tb, &Ta);
				val1= Tr+Tg+Tb;

				boxsample(ibuf, fx, fy-miny, fx+minx, fy, &fac1, &fac2, &fac3, &fac4);
				val2= fac1+fac2+fac3;
				boxsample(ibuf, fx-minx, fy, fx, fy+miny, &fac1, &fac2, &fac3, &fac4);
				val3= fac1+fac2+fac3;

				val3= rtex->bintf*(val1-val3);
				val2= rtex->bintf*(val1-val2);
				R.vn[0]+= val3;
				R.vn[1]+= val2;
				Normalise(R.vn);
			}
			else {
				minx= (maxx-minx)/2;
				miny= (maxy-miny)/2;
				if(minx>0.25) minx= 0.25;
				if(miny>0.25) miny= 0.25;
				maxx= fx+minx;
				minx= fx-minx;
				maxy= fy+miny;
				miny= fy-miny;

				boxsample(ibuf, minx, miny, maxx, maxy, &Tr, &Tg, &Tb, &Ta);
				
			}
		}
		else if (rtex->var[6]==2 || rtex->type==1) { /* Oude MipMap of bump */
			dx= (maxx-minx);
			dy= (maxy-miny);
			
			if(dx>0.5) dx= 0.5;
			if(dy>0.5) dy= 0.5;

			maxd= MAX2(dx, dy);
			if(maxd>0.5) maxd= 0.5;
			
			pixsize= 1.0/(float)MIN2(ibuf->x, ibuf->y);
			mm= ima->mipmap;
			previbuf= ibuf;
			while(mm) {
				if(maxd<pixsize) break;
				previbuf= ibuf;
				ibuf= mm->ibuf;
				pixsize= 1.0/(float)MIN2(ibuf->x, ibuf->y);
				mm= mm->next;
			}

			if(rtex->type==1) {	    /* bump */
				filtersample(ibuf, (fx)*ibuf->x, (fy)*ibuf->y, &Tr, &Tg, &Tb, &Ta);
				val1= Tr+Tg+Tb;

				filtersample(ibuf, (fx+dx)*ibuf->x, (fy)*ibuf->y, &fac1, &fac2, &fac3, &fac4);
				val2= fac1+fac2+fac3;
				filtersample(ibuf, (fx)*ibuf->x, (fy+dy)*ibuf->y, &fac1, &fac2, &fac3, &fac4);
				val3= fac1+fac2+fac3;

				if(previbuf!=ibuf && mm!=0) {  /* interpoleren */

					fac1= 2.0*(pixsize-maxd)/pixsize;
					if(fac1>=1.0) fac1= 1.0;
					
					fac2= 1.0-fac1;

					filtersample(previbuf, (fx)*previbuf->x, (fy)*previbuf->y, &minx, &maxx, &miny, &fac4);
					Tr= fac2*Tr+ fac1*minx;
					Tg= fac2*Tg+ fac1*maxx;
					Tb= fac2*Tb+ fac1*miny;
					
					val1= fac2*val1+ fac1*(minx+maxx+miny);
					filtersample(previbuf, (fx+dx)*previbuf->x, (fy)*previbuf->y, &minx, &maxx, &miny, &fac4);
					val2= fac2*val2+ fac1*(minx+maxx+miny);
					filtersample(previbuf, (fx)*previbuf->x, (fy+dy)*previbuf->y, &minx, &maxx, &miny, &fac4);
					val3= fac2*val3+ fac1*(minx+maxx+miny);

				}
				val3= rtex->bintf*(val1-val3);
				val2= rtex->bintf*(val1-val2);
				R.vn[0]+= val3;
				R.vn[1]+= val2;
				Normalise(R.vn);

			}
			else {
				filtersample(ibuf, fx*ibuf->x, fy*ibuf->y, &Tr, &Tg, &Tb, &Ta);

				if(previbuf!=ibuf) {  /* interpoleren */

					filtersample(previbuf, fx*previbuf->x, fy*previbuf->y, &fac1, &fac2, &fac3, &fac4);

					fx= 2.0*(pixsize-maxd)/pixsize;
					if(fx>=1.0) {
						Ta= fac4;
						Tb= fac3;
						Tg= fac2;
						Tr= fac1;
					}
					else {
						fy= 1.0-fx;

						Tb= fy*Tb+ fx*fac3;
						Tg= fy*Tg+ fx*fac2;
						Tr= fy*Tr+ fx*fac1;
						if(Talpha) Ta= fy*Ta+ fx*fac4;
					}
				}
			}
		} else if (rtex->var[6] == 1) { /* MipMap: welke map moet je hebben: */
			dx= MAX2((maxx-minx), (1.0 / ibuf->x));
			dy= MAX2((maxy-miny), (1.0 / ibuf->y));
			
			maxd= MIN2(dx, dy);
			if(maxd>0.5) maxd= 0.5;

			pixsize = 1.0 / (float) MAX2(ibuf->x, ibuf->y);	/* hier stond 2.0 */
			
			mm= ima->mipmap;
			previbuf= ibuf;
			while(mm) {
				if(maxd < pixsize) break;
				previbuf= ibuf;
				ibuf= mm->ibuf;
				pixsize= 1.0 / (float)MAX2(ibuf->x, ibuf->y); /* hier stond 1.0 */		
				mm= mm->next;
			}

			minx= (maxx-minx)/2;
			miny= (maxy-miny)/2;
			if(minx>0.5) minx= 0.5;
			if(miny>0.5) miny= 0.5;
			
			/* minmaal 1 pixel sampelen */
			if (minx < 0.5 / ima->ibuf->x) minx = 0.5 / ima->ibuf->x;
			if (miny < 0.5 / ima->ibuf->y) miny = 0.5 / ima->ibuf->y;
			
			maxx= fx+minx;
			minx= fx-minx;
			maxy= fy+miny;
			miny= fy-miny;

			boxsample(ibuf, minx, miny, maxx, maxy, &Tr, &Tg, &Tb, &Ta);

			if(previbuf!=ibuf) {  /* interpoleren */
				boxsample(previbuf, minx, miny, maxx, maxy, &fac1, &fac2, &fac3, &fac4);
				fx= 2.0*(pixsize-maxd)/pixsize;
				if(fx>=1.0) {
					Ta= fac4; Tb= fac3;
					Tg= fac2; Tr= fac1;
				} else {
					fy= 1.0-fx;
					Tb= fy*Tb+ fx*fac3;
					Tg= fy*Tg+ fx*fac2;
					Tr= fy*Tr+ fx*fac1;
					if(Talpha) Ta= fy*Ta+ fx*fac4;
				}
			}
		}

		if(Talpha) {
			Tin= Ta;
			if(Ta==0.0) {
				Tr= Tg= Tb= 0.0;
			}
			else if(Talpha>0 && rtex->var[8] != 2) {		/* ontpremullen */
				Tr/=Ta;
				Tg/=Ta;
				Tb/=Ta;
			}
		}
		else Tin= (Tr+Tg+Tb)/3.0;
		
		rgb=1;

		if( (R.f & 64) && (ibuf->flags & IB_fields) ) {
			/* PATCH */
			/* if(view0.grid==5001) { */
			/* 	if(ima->lastframe!=rtex->var[9]) ibuf->rect-= (ibuf->x*ibuf->y);; */
			/* } */
			/* else  */
			ibuf->rect-= (ibuf->x*ibuf->y);
		}

	}
	else Tin= 0.0;
}


/* ************************** */

void kleur()
{

	if(rtex->var[2]) {
		Tin= turbulence(rtex->var[0],t[0],t[1],t[2], rtex->var[2]);
		if(Tin>1.0) Tin= 1.0;
	}
	else Tin= hnoise(rtex->var[0],t[0],t[1],t[2]);

	if(rtex->type==1) {

		Tr= rtex->c1f[0];
		Tg= rtex->c1f[1];
		Tb= rtex->c1f[2];

		Tr*= Tin;
		Tg= Tg*turbulence(rtex->var[0],t[2],t[0],t[1], rtex->var[2]);
		if(Tg>1.0) Tg= 1.0;
		Tb= Tb*turbulence(rtex->var[0],t[1],t[2],t[0], rtex->var[2]);
		if(Tb>1.0) Tb= 1.0;
		
		Tin=(Tr+Tg+Tb)/3.0;
		rgb= 1;
	} else if(rtex->type==2) {
		Tin*=Tin;
		Tin*=Tin;
	}
}

void wood()
{
	
	if(rtex->var[1]==0) t[2]=0;
	else if(rtex->var[1]==1) t[1]=0;
	else if(rtex->var[1]==2) t[0]=0;
	
	if(rtex->type==0) {
		Tin= 0.5+0.5*fsin( (t[0]+t[1]+t[2])/50.0 );
	}
	else if(rtex->type==1) {
		Tin= 0.5+0.5*fsin( fsqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2])/50.0 );
	}
	else if(rtex->type==2) {
		Tin= hnoise(rtex->var[0],t[0],t[1],t[2]);
		Tin= 0.5+ 0.5*fsin(rtex->var[2]*Tin+(t[0]+t[1]+t[2])/50.0);
	}
}

void marmer()
{
	float n, turb;

	n= t[0]/rtex->var[3]+t[1]/rtex->var[4]+t[2]/rtex->var[5];
	n/=50;
	turb= ((float)rtex->var[1])/10;
	Tin = 0.5+0.5*fsin(n+turb*turbulence(rtex->var[0],t[0],t[1],t[2],rtex->var[2]));

	switch (rtex->type){
	case 0:
		Tin= fsqrt(Tin);
		break;
	case 2:
		Tin= fsqrt(Tin);
		Tin= fsqrt(Tin);
		break;
	case 3:
		Tin= 1.0- fsqrt(Tin);
		break;
	case 4:
		Tin= fsqrt(Tin);
		Tin= fsqrt(Tin);
		Tin= fsqrt(Tin);
		Tin= fsqrt(Tin);
		break;
	case 5:
		Tin = Tin * Tin;
		Tin = Tin * Tin;
		Tin = Tin * Tin;
		Tin = Tin * Tin;
		break;
	}
}



void tover()
{
	short n;
	float tt,x,y,z;

	n= rtex->var[0];
	tt= (float)rtex->var[2];
	tt/=100;
	x=  fsin( ( t[0]+t[1]+t[2])/400.0 );
	y=  fcos( (-t[0]+t[1]-t[2])/400.0 );
	z= -fcos( (-t[0]-t[1]+t[2])/400.0 );
	if(n>0) {
		x*=tt; 
		y*=tt; 
		z*=tt;
		y= -fcos(x-y+z);
		y*=tt;
		if(n>1) {
			x= fcos(x-y-z);
			x*=tt;
			if(n>2) {
				z= fsin(-x-y-z);
				z*=tt;
				if(n>3) {
					x= -fcos(-x+y-z);
					x*=tt;
					if(n>4) {
						y= -fsin(-x+y+z);
						y*=tt;
						if(n>5) {
							y= -fcos(-x+y+z);
							y*=tt;
							if(n>6) {
								x= fcos(x+y+z);
								x*=tt;
								if(n>7) {
									z= fsin(x+y-z);
									z*=tt;
									if(n>8) {
										x= -fcos(-x-y+z);
										x*=tt;
										if(n>9) {
											y= -fsin(x-y+z);
											y*=tt;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(rtex->type==1) {
		tt= 0.004*(float)rtex->var[1];
		Tr= (0.5-tt*x); 
		Tr= ffloor(Tr)-Tr;
		Tg= (0.5-tt*y); 
		Tg= ffloor(Tg)-Tg;
		Tb= (0.5-tt*z); 
		Tb= ffloor(Tb)-Tb;
	} else {
		tt*= 2.0;
		x/=tt; 
		y/=tt; 
		z/=tt;
		Tr= 0.5-x;
		Tg= 0.5-y;
		Tb= 0.5-z;
	}
	if(Tr>1.0) Tr=1.0; 
	else if(Tr<0.0) Tr= 0.0;
	if(Tg>1.0) Tg=1.0; 
	else if(Tg<0.0) Tg= 0.0;
	if(Tb>1.0) Tb=1.0; 
	else if(Tb<0.0) Tb= 0.0;

	Tin= (Tr+Tg+Tb)/3.0;
	rgb=1;
}


void tiles()
{		    /* gooi deze eruit, maak ervan een imawarap */
	long rt[3];
	short x,y,r,tt;

	rt[0]=ffloor(t[0]);
	rt[1]=ffloor(t[1]);
	rt[2]=ffloor(t[2]);
	if(rtex->var[0]==0) rt[2]=0;
	else if(rtex->var[0]==1) rt[1]=0;
	else if(rtex->var[0]==2) rt[0]=0;
	rt[0]&= 255; 
	rt[1]&= 255; 
	rt[2]&= 255;


	r=rtex->var[1];
	if(rtex->type==0) {
		tt= 0;
		if(rt[0]>r) tt++;
		if(rt[1]>r) tt++;
		if(rt[2]>r) tt++;
		if(rtex->var[2]==0) {
			if(tt & 1) Tin= 1.0;
			else Tin= 0.0;
		}
		else Tin= 0.33333*tt;
	}
	else if(rtex->type==1) {
		if(rt[0]>r) Tin= 0.3333333;
		else if(rt[1]>r) Tin= 0.666666;
		else if(rt[2]>r) Tin= 1.0;
		if(Tin!=0.0 && rtex->var[2]==0) Tin= 1.0;
	}
	else if(rtex->type==2) {
		Tin= 0.0;
	}

}

void blend()
{

	if(rtex->type==1) {
		Tin= fsqrt(t[0]*t[0]+	t[1]*t[1]+	t[2]*t[2]);
		Tin= 1.0- Tin/32767.0;

		if (rtex->var[2] > 1) {
			Tin = 1.0 + (((1.0 + (rtex->var[2] - 1.0) / 2.0)) * (Tin - 1.0));
		}
		
		if(Tin<0.0) Tin= 0.0;
		
		if(rtex->var[2]) Tin*= Tin;
	}
	else {
		if(rtex->var[0]==1) t[0]=t[1];
		if(rtex->var[0]==2) t[0]=t[2];
	
		if(rtex->var[2]) Tin= ( t[0]*rtex->var[2]+16384.0 )/32767.0;
		else Tin= 0.5+t[0]/65536.0;
	}
	
	if(rtex->type==2) {
		if(Tin>0.0) Tin= fsqrt(Tin);
	}
	else if(rtex->type==3) {
		if(Tin>0.0) Tin*= (Tin);
	}
	
	if(rtex->var[1]) Tin= 1.0-Tin;
	if(Tin<0.0) Tin= 0.0; 
	else if(Tin>1.0) Tin= 1.0;
}

void stucci()
{
	float b2,vec[3];
	short ofs;

	ofs= rtex->var[1];
	if(rtex->type>=8) {
		;/* marble bump */
	}
	else {
		b2=hnoise(rtex->var[0],t[0],t[1],t[2]);
		if(rtex->type > 1) ofs*=(b2*b2);
		vec[0]= b2-hnoise(rtex->var[0],t[0]+ofs,t[1],t[2]);
		vec[1]= b2-hnoise(rtex->var[0],t[0],t[1]+ofs,t[2]);
		vec[2]= b2-hnoise(rtex->var[0],t[0],t[1],t[2]+ofs);

		if(rtex->type & 1) {
			R.vn[0]-= vec[0];
			R.vn[1]-= vec[1];
			R.vn[2]-= vec[2];
		}
		else {
			R.vn[0]+= vec[0];
			R.vn[1]+= vec[1];
			R.vn[2]+= vec[2];
		}
		Normalise(R.vn);
	}
	norm= 1;
	Tin= 0.0;
}

void texnoise()
{
	float div=3.0;
	long val, ran, loop;
	
	ran= random();
	val= (ran & 3);
	
	loop= rtex->var[0];
	while(loop--) {
		ran= (ran>>2);
		val*= (ran & 3);
		div*= 3.0;
	}
	
	Tin= ((float)val)/div;;

}

void multitex()
{
	extern struct Osa O;
	float temp, *fp;

	t[0]/= rtex->div[0];
	t[0]+= rtex->add[0];
	t[1]/= rtex->div[1];
	t[1]+= rtex->add[1];
	t[2]/= rtex->div[2];
	t[2]+= rtex->add[2];

	if(R.osatex) {
		dxt[0]/= rtex->div[0];
		dxt[1]/= rtex->div[1];
		dxt[2]/= rtex->div[2];

		dyt[0]/= rtex->div[0];
		dyt[1]/= rtex->div[1];
		dyt[2]/= rtex->div[2];
	}

	rgb=0; 
	norm=0;
	Talpha= 0;
	
	/* als Titot==0 moeten wel een aantal berekeningen nog */

	if(Titot!= 0.0) {

		switch(rtex->soort) {
		case 1:
			kleur(); 
			break;
		case 2:
			wood(); 
			break;
		case 3:
			marmer(); 
			break;
		case 4:
			tover(); 
			break;
		case 5:
			if(R.osatex) imagewraposa();
			else imagewrap();
			break;
		case 6:
			tiles(); 
			break;
		case 7:
			blend(); 
			break;
		case 8:
			stucci(); 
			break;
		case 9:
			texnoise(); 
			break;
		}
	}
	else Tin= 0.0;


	temp= rtex->vintf*Tin;	/* voor stencil */
	Tin= Tin*Titot;

	if(rgb) {
		if(rtex->f & 4) { 	/* blend */
			Tr= rtex->c1f[0];
			Tg= rtex->c1f[1];
			Tb= rtex->c1f[2];
			Tinc= Tin*rtex->cintf;
		} else {
			if(Talpha) Tinc= Tin*Titot*rtex->cintf;
			else Tinc= Titot*rtex->cintf;
		}
	} else {
		Tr= rtex->c1f[0];
		Tg= rtex->c1f[1];
		Tb= rtex->c1f[2];
		Tinc= Tin*rtex->cintf;
	}
	
	if(rtex->f & 1)			/* stencil */
		Titot= temp*Titot;
}


void objectex(col,nr)
struct ColBlckF *col;
short nr;
{
	extern struct Osa O;
	extern void VecMulf(float *v1, float f);
	struct Base *base;
	float fi,co,si,f1,f2,*mat,os[3];
	float b1, b2, i;
	long *vec,v[3];
	short map1,map2,x,y,osain;

	map1=col->map[2*nr];

	rtex=G.adrtex[col->tex[nr]];
	if(rtex==0 || rtex->soort==0) return;

	if(map1 & 2048) {
		t[0]=R.lo[0];	
		t[1]=R.lo[1]; 
		t[2]=R.lo[2];
		if(R.osatex) {
			VECCOPY(dxt,O.dxlo);
			VECCOPY(dyt,O.dylo);
			
			if(view0.grid==5002) {
				b1 = 1.0 + R.edgeint*(col->mir);
				VecMulf(dxt, b1);
				b1 = 1.0 + (R.edgeint*(col->mir) / 2.0);
				VecMulf(dyt, b1);
			}
		}
	}
	else if(map1 & 512) {
		t[0]=R.ref[0];	
		t[1]=R.ref[1]; 
		t[2]=R.ref[2];
		if(R.osatex) {
			VECCOPY(dxt,O.dxref);
			VECCOPY(dyt,O.dyref);
		}
	}
	else if(map1 & 1024) {
		t[0]=R.orn[0];	
		t[1]=R.orn[1]; 
		t[2]=R.orn[2];
		if(R.osatex) {
			VECCOPY(dxt,O.dxno);
			VECCOPY(dyt,O.dyno);
			dxt[0]*=32767; 
			dxt[1]*=32767; 
			dxt[2]*=32767;
			dyt[0]*=32767; 
			dyt[1]*=32767; 
			dyt[2]*=32767;
		}
	}
	else if(map1 & 4096) {
		t[0]= R.gl[0];	
		t[1]= R.gl[1]; 
		t[2]= R.gl[2]; 

		Mat4MulVecfl(G.viewinv, t);
	}
	else if(map1 & 8192) {
		t[0]=R.uv[0];	
		t[1]=R.uv[1]; 
		t[2]=0;
		if(R.osatex) {
			VECCOPY(dxt,O.dxuv);
			VECCOPY(dyt,O.dyuv);
		}
	}
	else if(map1 & (4096+16384) ) {	/* glob en imat */
		if(map1 & 16384) base= col->base[nr];
		else base= G.workbase;
		
		if(base==0) return;
		
		vec= base->ivec;
		v[0]= R.co[0]+vec[0];
		v[1]= R.co[1]+vec[1];
		v[2]= R.co[2]+vec[2];
		Mat3MulVec(base->imat, v);
		t[0]= v[0];
		t[1]= v[1];
		t[2]= v[2];
		if(R.osatex) {
			dxt[0]= O.dxco[0];
			dxt[1]= O.dxco[1];
			dxt[2]= O.dxco[2];
			Mat3MulVecfl(base->imat, dxt);
			dyt[0]= O.dyco[0];
			dyt[1]= O.dyco[1];
			dyt[2]= O.dyco[2];
			Mat3MulVecfl(base->imat, dyt);
		}
	}

	/* if(map1 & 2) Tusealpha= 1; */
	/* else Tusealpha= 0; */
	/* Tusealpha: onderzoek alpha in imagewrap */
	/* Talpha: er is een alpha gevonden */ 
	
	Tusealpha= 1;

	multitex();

	if(map1 & 352) {
		/* Talpha= 1; */
		if(Talpha && (map1 & 2) ) {
			Tinc= 1.0;	/* in verband met premul */
		}
		b1= 1.0-Tinc;
		if(map1 & 256) {
			col->r= (Tinc*Tr+b1*col->r);
			col->g= (Tinc*Tg+b1*col->g);
			col->b= (Tinc*Tb+b1*col->b);
		}
		if(map1 & 64) {
			col->specr= (Tinc*Tr+b1*col->specr);
			col->specg= (Tinc*Tg+b1*col->specg);
			col->specb= (Tinc*Tb+b1*col->specb);
		}
		if(map1 & 32) {
			col->mirr= (Tinc*Tr+b1*col->mirr);
			col->mirg= (Tinc*Tg+b1*col->mirg);
			col->mirb= (Tinc*Tb+b1*col->mirb);
		}
	}
	if(map1 & 128) {
		if(norm==0) {
			fi= Tin*rtex->bintf;
			co= fcos(fi);
			si= fsin(fi);
			f1= R.vn[0];
			f2= R.vn[1];
			R.vn[0]= f1*co+f2*si;
			R.vn[1]= f2*co-f1*si;
			f1= R.vn[1];
			f2= R.vn[2];
			R.vn[1]= f1*co+f2*si;
			R.vn[2]= f2*co-f1*si;
			Normalise(R.vn);
		}
	}
	if(map1 & 31) {
		map2=col->map[2*nr+1];
		
		if(map1 & 1) {
			if(map2 & 1) b1= 1.0-Tin*rtex->vintf;
			else b1= Tin*rtex->vintf;
			b2= 1.0-b1;
			col->emit= b1*rtex->lif[0]+ b2*col->emit;
		}
		if(map1 & 2) {
			if(Talpha) {
				Ta= 1.0-Ta;
				if(map2 & 2) b1= 1.0-Ta*rtex->vintf;
				else b1= Ta*rtex->vintf;
				b2= 1.0-b1;
				col->tra= b1*rtex->lif[0]+ b2*col->tra;
			}
			else {
				if(map2 & 2) b1= 1.0-Tin*rtex->vintf;
				else b1= Tin*rtex->vintf;
				b2= 1.0-b1;
				col->tra= b1*rtex->lif[0]+ b2*col->tra;
			}
		}
		if(map1 & 4) {
			if(map2 & 4) b1= 1.0-Tin*rtex->vintf;
			else b1= Tin*rtex->vintf;
			b2= 1.0-b1;
			col->ang= b1*rtex->lif[0]+ b2*col->ang;
		}
		if(map1 & 8) {
			if(map2 & 8) b1= 1.0-Tin*rtex->vintf;
			else b1= Tin*rtex->vintf;
			b2= 1.0-b1;
			col->kspec= b1*rtex->lif[0]+ b2*col->kspec;
		}
		if(map1 & 16) {			
			if(map2 & 16) b1= 1.0-Tin*rtex->vintf;
			else b1= Tin*rtex->vintf;
			b2= 1.0-b1;
			col->kref= b1*rtex->lif[0]+ b2*col->kref;
		}
	}
}

void skytex(nr)
short nr;
{
	float f1,f2;
	long b2,x,y,z,*vec,v[3];
	short map1,b1,i, ok;

	Tusealpha= 0;

	if(Titot==0.0) return;
	map1=wrld.smap[nr];

	dxt[0]=dxt[1]=dxt[2]= 0;
	dyt[0]=dyt[1]=dyt[2]= 0;

	rtex=G.adrtex[wrld.stex[nr]];
	if(rtex->soort==0) return;

	if(map1 & 16) {	/* imatbase */
		if(wrld.sbase[nr]==0) return;
		vec= wrld.sbase[nr]->ivec;
		v[0]= R.lo[0]+vec[0];
		v[1]= R.lo[1]+vec[1];
		v[2]= R.lo[2]+vec[2];
		Mat3MulVec(wrld.sbase[nr]->imat,v);
		t[0]= v[0];
		t[1]= v[1];
		t[2]= v[2];
	}
	else {
		t[0]=R.lo[0];
		t[1]=R.lo[1];
		t[2]=R.lo[2];
	}

	multitex();

	if(map1 & 1) {
		f1= Tin*rtex->vintf;
		f2=1.0-f1;
		wrld.inprz= f1+f2*wrld.inprz;
		wrld.inprh= 1.0-wrld.inprz;
	}
	if(map1 & 2) {
		f1= 1.0-Tinc;
		f2= Tinc*255.9;
		wrld.horr= (f2*Tr+f1*wrld.horr);
		wrld.horg= (f2*Tg+f1*wrld.horg);
		wrld.horb= (f2*Tb+f1*wrld.horb);
	}
	if(map1 & (4+32) ) {
		f1= 1.0-Tinc;
		f2= Tinc*255.9;
		
		if(wrld.fs & 2) {	/* real */
			ok= 0;
			if(wrld.fs & 128) {
				if(map1 & 4) ok= 1;
			}
			else if(map1 & 32) ok= 1;
			if(ok) {
				wrld.zenr= (f2*Tr+ f1*wrld.zenr);
				wrld.zeng= (f2*Tg+ f1*wrld.zeng);
				wrld.zenb= (f2*Tb+ f1*wrld.zenb);
			}
		}
		else {
			wrld.zenr= (f2*Tr+ f1*wrld.zenr);
			wrld.zeng= (f2*Tg+ f1*wrld.zeng);
			wrld.zenb= (f2*Tb+ f1*wrld.zenb);
		}
	}
}

void lamptex(lar,nr,lv)
struct LampRen *lar;
short nr;
float *lv;
{
	float fi,co,si,f1,f2,v[3];
	long b2,x,y,z,*vec;
	short map1,b1,i;

	Tusealpha= 0;
	
	if(Titot==0.0) return;
	map1= lar->map[nr];

	rtex=G.adrtex[lar->tex[nr]];
	if(rtex->soort==0) return;

	dxt[0]=dxt[1]=dxt[2]= 0;
	dyt[0]=dyt[1]=dyt[2]= 0;

	if(map1 & 256) {	/* lampvector */
		if(lar->soort==1) {
			f1= lar->spottexfac;
			VECCOPY(v, lv);
			Mat3MulVecfl(lar->imat, v);
			t[0]= f1*v[0];
			t[1]= f1*v[1];
			t[2]= f1*v[2];
		}
		else {
			t[0]= 32760*lv[0];
			t[1]= 32760*lv[1];
			t[2]= 32760*lv[2];
		}
	}
	else if(map1 & 512) {	/* global */
		t[0]= R.co[0];
		t[1]= R.co[1];
		t[2]= R.co[2];
	}
	else if(map1 & 1024) {	/* imatbase */
		if(lar->base[nr]==0) return;
		vec= lar->base[nr]->ivec;
		t[0]= R.co[0]+vec[0];
		t[1]= R.co[1]+vec[1];
		t[2]= R.co[2]+vec[2];
		Mat3MulVecfl(lar->base[nr]->imat,t);
	}

	multitex();

	if(map1 & 1) {
		if(norm==0) {
			fi= Tin*rtex->bintf;
			co= fcos(fi);
			si= fsin(fi);
			f1= lv[0];
			f2= lv[1];
			lv[0]= f1*co+f2*si;
			lv[1]= f2*co-f1*si;
			f1= lv[1];
			f2= lv[2];
			lv[1]= f1*co+f2*si;
			lv[2]= f2*co-f1*si;
			Normalise(lv);
		}
	}
	if(map1 & 4) {
		f1= 1.0-Tinc;
		lar->r= (Tinc*Tr+f1*lar->r);
		lar->g= (Tinc*Tg+f1*lar->g);
		lar->b= (Tinc*Tb+f1*lar->b);
	}
}

void externtex(nr, vecin, val)	/* schrijft (intensiteit) resultaat in val */
short nr;						/* voor externe aanroep textures */
float *vecin, *val;				/* Zonodig: lees (als extern) Tr,Tg en Tb uit */
{
	

	Tusealpha= 0;
	R.osatex= 0;
	Titot= 1.0;
	R.vlr= 0;
	
	dxt[0]=dxt[1]=dxt[2]= 0;
	dyt[0]=dyt[1]=dyt[2]= 0;

	rtex= G.adrtex[nr];
	if(rtex==0 || rtex->soort==0) return;
	
	VECCOPY(t, vecin);
	multitex();

	*val= rtex->vintf*Tin;
}


void externtexcol(nr, vecin, col)	/* schrijft kleur in col */
short nr;
float *vecin;
char *col;		/* let op: 0=r, 1=g, 2=b */
{
	int temp;
	float b1;

	Tusealpha= 0;
	R.osatex= 0;
	Titot= 1.0;
	R.vlr= 0;
	
	dxt[0]=dxt[1]=dxt[2]= 0;
	dyt[0]=dyt[1]=dyt[2]= 0;

	rtex= G.adrtex[nr];
	if(rtex==0 || rtex->soort==0) return;
	
	VECCOPY(t, vecin);
	multitex();

	b1= 1.0-Tinc;

	temp= 255*(Tinc*Tr)+b1*col[0];
	if(temp>255) col[0]= 255; else col[0]= temp;
	temp= 255*(Tinc*Tg)+b1*col[1];
	if(temp>255) col[1]= 255; else col[1]= temp;
	temp= 255*(Tinc*Tb)+b1*col[2];
	if(temp>255) col[2]= 255; else col[2]= temp;

}

void clipx_rctf_swap(stack, count, x1, x2)
rctf *stack;
short *count;
float x1, x2;
{
	rctf *rf, *new;
	short a;

	a= *count;
	rf= stack;
	for(;a>0;a--) {
		if(rf->xmin<x1) {
			if(rf->xmax<x1) {
				rf->xmin+= (x2-x1);
				rf->xmax+= (x2-x1);
			}
			else {
				if(rf->xmax>x2) rf->xmax= x2;
				new= stack+ *count;
				(*count)++;

				new->xmax= x2;
				new->xmin= rf->xmin+(x2-x1);
				new->ymin= rf->ymin;
				new->ymax= rf->ymax;
				
				if(new->xmin==new->xmax) (*count)--;
				
				rf->xmin= x1;
			}
		}
		else if(rf->xmax>x2) {
			if(rf->xmin>x2) {
				rf->xmin-= (x2-x1);
				rf->xmax-= (x2-x1);
			}
			else {
				if(rf->xmin<x1) rf->xmin= x1;
				new= stack+ *count;
				(*count)++;

				new->xmin= x1;
				new->xmax= rf->xmax-(x2-x1);
				new->ymin= rf->ymin;
				new->ymax= rf->ymax;

				if(new->xmin==new->xmax) (*count)--;

				rf->xmax= x2;
			}
		}
		rf++;
	}

}

void clipy_rctf_swap(stack, count, y1, y2)
rctf *stack;
short *count;
float y1, y2;
{
	rctf *rf, *new;
	short a;

	a= *count;
	rf= stack;
	for(;a>0;a--) {
		if(rf->ymin<y1) {
			if(rf->ymax<y1) {
				rf->ymin+= (y2-y1);
				rf->ymax+= (y2-y1);
			}
			else {
				if(rf->ymax>y2) rf->ymax= y2;
				new= stack+ *count;
				(*count)++;

				new->ymax= y2;
				new->ymin= rf->ymin+(y2-y1);
				new->xmin= rf->xmin;
				new->xmax= rf->xmax;

				if(new->ymin==new->ymax) (*count)--;

				rf->ymin= y1;
			}
		}
		else if(rf->ymax>y2) {
			if(rf->ymin>y2) {
				rf->ymin-= (y2-y1);
				rf->ymax-= (y2-y1);
			}
			else {
				if(rf->ymin<y1) rf->ymin= y1;
				new= stack+ *count;
				(*count)++;

				new->ymin= y1;
				new->ymax= rf->ymax-(y2-y1);
				new->xmin= rf->xmin;
				new->xmax= rf->xmax;

				if(new->ymin==new->ymax) (*count)--;

				rf->ymax= y2;
			}
		}
		rf++;
	}

}




/* INIT */

#undef None

struct Textudata None= {
	{"*","*","*","*","*","*","*","*","*","*","*","*"},
 	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};
struct Textudata Color= {
	{"Noise","*","Depth","*","*","*","*","*","*","*","*","*"},
 	{10,0,0,0,0,0,0,0,0,0,0,0},
	{20000,0,5,0,0,0,0,0,0,0,0,0},
	{4000,0,0,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};
struct Textudata Wood= {
	{"Noise ","Proj ","Turb ","X","Y","*","*","*","*","*","*","*"},
	{10,0,1,0,0,0,0,0,0,0,0,0},
	{2000,3,500,50,50,0,0,0,0,0,0,0},
	{300,3,10,0,0,0,0,0,0,0,0,0},
	0,0x95654E00,0xDF9C0000,0,0,
	{20,20,20},{0,0,0}
};
struct Textudata Marble= {
	{"Noise " ,"Turb ","Depth ","X rings ","Y rings ","Z rings ","*","*","*","*","*","*"},
	{10,1,0,1,1,1,0,0,0,0,0,0},
	{2000,500,5,200,200,200,0,0,0,0,0,0},
	{300,50,3,15,15,15,0,0,0,0,0,0},
	0,0x95654E00,0xDF9C0000,0,0,
	{20,20,20},{0,0,0}
};
struct Textudata Magic= {
	{"Depth ","Overfl ","Turbul ","*","*","*","*","*","*","*","*","*"},
	{0,1,10,0,0,0,0,0,0,0,0,0},
	{12,128,510,0,0,0,0,0,0,0,0,0},
	{5,10,120,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{20,20,20},{0,0,0}
};
struct Textudata Image= {
	{"Proj ","X Rep ","Y Rep ","Wrap ","X% Crop","Y% Crop",
	 "MipMap ","Fie/Im","ReadAlpha","Frames ","Cycl ","Sfra "},
	{0,	1,	1,	0,	-500,	-500,	    0,	1,		0, 0,	0,	-1999},
	{5,	100,	100,  6,	200,	200,		2,	100,	2, 1999, 1,	2500},
	{0,	1,	1,	0,	0,	0,				0,	2,	1, 0,	0,	1},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};
struct Textudata Tiles= {
	{"Proj ","Limit ","Blend ","*","*","*","*","*","*","*","*","*"},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{3,255,1,0,0,0,0,0,0,0,0,0},
	{0,127,0,0,0,0,0,0,0,0,0,0},
	0xFF000000,0xFFFF0000,0x0,0,2,
	{20,20,20},{0,0,0}
};
struct Textudata Blend= {
	{"Proj ","Neg ","Compr ","*","*","*","*","*","*","*","*","*"},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{2,1,8,0,10,0,0,0,0,0,0,4},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};
struct Textudata Stucci= {
	{"Noise ","Int ","Turbdiv ","*","*","*","*","*","*","*","*","*"},
	{10,0,1,0,0,0,0,0,0,0,0,0},
	{10000,2000,800,0,10,0,0,0,0,0,0,0},
	{2000,200,100,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};
struct Textudata Noise= {
	{"Depth","*","*","*","*","*","*","*","*","*","*","*"},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{7,0,0,0,10,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	0xFF00FF00,0xFFFF0000,0xFFFF000,0,0,
	{1,1,1},{0,0,0}
};

struct Textudata *inittextudata(soort)
short soort;
{
	struct Textudata *td;

	if(soort==1) td= &Color;
	else if(soort==2) td= &Wood;
	else if(soort==3) td= &Marble;
	else if(soort==4) td= &Magic;
	else if(soort==5) td= &Image;
	else if(soort==6) td= &Tiles;
	else if(soort==7) td= &Blend;
	else if(soort==8) td= &Stucci;
	else if(soort==9) td= &Noise;
	else td= &None;

	return td;
}

void inittexture(actex,soort)
short actex,soort;
{
	struct Textudata *td;

	td=inittextudata(soort);

	G.adrtex[actex]->soort= soort;
	memcpy(G.adrtex[actex]->var,td->def,24);
	memcpy(G.adrtex[actex]->c1,&(td->c1),3);
	memcpy(G.adrtex[actex]->c2,&(td->c2),3);
	memcpy(G.adrtex[actex]->c3,&(td->c3),3);
	memcpy(G.adrtex[actex]->li,&(td->li),3);
	G.adrtex[actex]->f= td->f;
	memcpy(G.adrtex[actex]->div,td->div,12);
	G.adrtex[actex]->type=0;
	G.adrtex[actex]->cint=255;
	G.adrtex[actex]->vint=255;
	G.adrtex[actex]->bint=128;
	G.adrtex[actex]->ima= 0;
}

