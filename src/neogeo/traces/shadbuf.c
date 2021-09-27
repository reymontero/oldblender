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

 

/*  		shadbuf.c

		mei/juni 92

*/

#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "storage.h"
#include "/usr/people/include/Trace.h"

extern struct Osa O;
float jitshad[32][2];
short filt3[9]= { 1,2,1,2,4,2,1,2,1 };
long bias= 0x00500000;



void lrectreadRectz(x1,y1,x2,y2,r1)	/* leest deel uit rectz in r1 */
ulong x1,y1,x2,y2;
char *r1;
{
	ulong len4,*rz;	

	if(x1>=R.rectx || x2>=R.rectx || y1>=R.recty || y2>=R.recty) return;
	if(x1>x2 || y1>y2) return;

	len4= 4*(x2- x1+1);
	rz= R.rectz+R.rectx*y1+x1;
	for(;y1<=y2;y1++) {
		memcpy(r1,rz,len4);
		rz+= R.rectx;
		r1+= len4;
	}
}

void initshadowbuf(lar, la, mat)
struct LampRen *lar;
struct LaData *la;
float mat[][4];		/* transf.matrix van lamp */
{
	struct ShadBuf *shb;
	float hoek, dist, temp;
	short v;

	if(la->spsi<16) return;

	/* geheugen reserveren */
	shb= (struct ShadBuf *)callocN( sizeof(struct ShadBuf),"initshadbuf");
	lar->shb= shb;

	if(shb==0) return;

	VECCOPY(shb->co,lar->co);
	shb->size= la->bufsize;
	shb->samp= la->samp;
	shb->soft= la->soft;
	
	shb->zbuf= (ulong *)mallocN( (shb->size*shb->size)/64,"initshadbuf2");
	shb->cbuf= (char *)callocN( (shb->size*shb->size)/256,"initshadbuf3");

	if(shb->zbuf==0 || shb->cbuf==0) {
		if(shb->zbuf) freeN(shb->zbuf);
		freeN(lar->shb);
		lar->shb= 0;
		return;
	}

	Mat4Ortho(mat);

	Mat4Invert(shb->winmat, mat);	/* winmat is hier temp */

	/* matrix: combinatie van inverse view en lampmat */
	Mat4MulMat4(shb->persmat, G.viewinv, shb->winmat);

	/* projektie */
	hoek= facos(lar->spotsi);
	temp= 0.5*shb->size*fcos(hoek)/fsin(hoek);
	shb->d= 1000*((float)la->clipsta);

	shb->pixsize= (shb->d)/temp;

	shb->far= 1000*((float)la->clipend);

	dist= shb->far- shb->d;
	dist= (100*la->bias)/dist;

	shb->bias= dist*0x7FFFFFFF;
}

long sizeoflampbuf(shb)
struct ShadBuf *shb;
{
	long num,count=0;
	char *cp;
	
	cp= shb->cbuf;
	num= (shb->size*shb->size)/256;

	while(num--) count+= *(cp++);
	
	return 256*count;
}

void makeshadowbuf(shb)
struct ShadBuf *shb;
{
	void projectvert();
	struct VlakRen *vlr;
	float temp,zcor,wsize,wd,dist;
	float winmat[4][4], vec[3];
	ulong *rz,*rz1,verg,*ztile,verg1;
	long a,x,y,xof,yof,win,v,minx,miny,byt1,byt2;
	short temprx,tempry;
	char *rc,*rcline,*ctile,*zt;

	/* viewvars onthouden */
	temprx= R.rectx; tempry= R.recty;
	R.rectx= R.recty= shb->size;

	initjit(shb->jit,shb->samp*shb->samp);

	/* matrices en window: in G.winmat komt transformatie
		van obsview naar lampview,  inclusief lampwinmat */
	
	wsize= shb->pixsize*(shb->size/2.0);

	i_window(-wsize, wsize, -wsize, wsize, shb->d, shb->far, winmat);

	Mat4SwapMat4(shb->winmat, G.winmat);
	Mat4MulMat4(G.winmat, shb->persmat, winmat);

	/* zbufferen */
	if(R.rectz) freeN(R.rectz);
 	R.rectz= (ulong *)mallocN(4*shb->size*shb->size,"makeshadbuf");
	rcline= mallocN(256*4+4,"makeshadbuf2");

	setzbufvlaggen(projectvert);
	zbuffershad();

				    /* noborder();prefposition(0, R.rectx-1, 0, R.rectx-1); */
				    /* win= winopen("test"); RGBmode(); singlebuffer(); */
				    /* gconfig(); */
				    /* lrectwrite(0, 0, R.rectx-1, R.rectx-1, R.rectz); */
				    /* while(getbutton(LEFTMOUSE)); */
				    /* while(getbutton(LEFTMOUSE)==0); */
				    /* winclose(win); */

	/* alle pixels 1 x ingevuld verwijderen (oneven) */
	/* probleem hierbij kan geven dat er abrupte overgangen van zacht gebied
	 * naar geen zacht gebied is: bijv als eronder een klein vlakje zit
	 * DAAROM (voorlopig?) ER WEER UIT
	 * 
		a= shb->size*shb->size;
		rz= R.rectz;
		while(a--) {
		    if(*rz & 1) *rz= 0x7FFFFFFF;
		    rz++;
		}
	 */
		
	/* Z tiles aanmaken */
	
	ztile= shb->zbuf;
	ctile= shb->cbuf;
	for(y=0;y<shb->size;y+=16) {
		for(x=0;x<shb->size;x+=16) {

			/* ligt rechthoek binnen spotbundel? */
			a= shb->size/2;
			if(x< a) minx= x+15-a;
			else minx= x-a;	
			if(y< a) miny= y+15-a;
			else miny= y-a;			
			dist= fsqrt( (float)(minx*minx+miny*miny) );

			if(dist>(float)(a+1)) {	/* randjes willen wel 'ns mis gaan */
				a= 256; verg= 0x80000000; /* 0x7FFFFFFF; */
				rz1= (&verg)+1;
			} else {
				lrectreadRectz(x,y,MIN2(shb->size-1,x+15),MIN2(shb->size-1,y+15),rcline);
				rz1= (ulong *)rcline;
				verg= (*rz1 & 0xFFFFFF00);

				for(a=0;a<256;a++,rz1++) {
					if( (*rz1 & 0xFFFFFF00) !=verg) break;
				}
			}
			if(a==256) { /* compleet leeg vakje */
				*ctile= 0;
				*ztile= *(rz1-1);
			}
			else {
				rc= rcline;
				rz1= (ulong *)rcline;
				verg=  rc[0];
				verg1= rc[1];
				rc+=4;
				byt1= 1; byt2= 1;
				for(a=1;a<256;a++,rc+=4) {
					byt1&= (verg==rc[0]);
					byt2&= (verg1==rc[1]);

					if(byt1==0) break;
				}
				if(byt1 && byt2) {	/* alleen byte opslaan */
					*ctile= 1;
					*ztile= (ulong)mallocN(256+4,"tile1");
					rz= (ulong *)*ztile;
					*rz= *rz1;

					zt= (char *)(rz+1);
					rc= rcline+2;
					for(a=0;a<256;a++,zt++,rc+=4) *zt= *rc;	
				}
				else if(byt1) {		/* short opslaan */
					*ctile= 2;
					*ztile= (ulong)mallocN(2*256+4,"Tile2");
					rz= (ulong *)*ztile;
					*rz= *rz1;

					zt= (char *)(rz+1);
					rc= rcline+1;
					for(a=0;a<256;a++,zt+=2,rc+=4) {
						zt[0]= rc[0];
						zt[1]= rc[1];
					}
				}
				else {			/* triple opslaan */
					*ctile= 3;
					*ztile= (ulong)mallocN(3*256,"Tile3");

					zt= (char *)*ztile;
					rc= rcline;
					for(a=0;a<256;a++,zt+=3,rc+=4) {
						zt[0]= rc[0];
						zt[1]= rc[1];
						zt[2]= rc[2];
					}
				}
			}
			ztile++;
			ctile++;
		}
	}

	freeN(rcline);
	freeN(R.rectz); R.rectz= 0;

	R.rectx= temprx; R.recty= tempry;
	Mat4SwapMat4(shb->winmat, G.winmat);

	/* printf("lampbuf %d\n", sizeoflampbuf(shb)); */
}

short firstreadshadbuf(shb,xs,ys,nr)
struct ShadBuf *shb;
long xs,ys,nr;
{
	/* return 1 als volledig gecomprimeerde shadbuftile && z==const */
	static long zsamp;
	long ofs;
	register char *ct;

	if(xs<0 || ys<0) return 0;
	if(xs>=shb->size || ys>=shb->size) return 0;
    
	/* z berekenen */
	ofs= (ys>>4)*(shb->size>>4) + (xs>>4);
	ct= shb->cbuf+ofs;
	if(*ct==0) {
	    if(nr==0) {
		zsamp= *( (long *)(shb->zbuf+ofs) );
		return 1;
	    }
	    else if(zsamp!= *( (long *)(shb->zbuf+ofs) )) return 0;
	    return 1;
	}
	
	return 0;
}

float readshadowbuf(shb,xs,ys,zs)	/* return 1.0 : volledig schaduw */
struct ShadBuf *shb;
long xs,ys,zs;
{
	float temp;
	long *rz,ofs,zsamp;
	register char *ct,*cz;

	/* simpleclip */
	if(xs<0 || ys<0) return 1.0;
	if(xs>=shb->size || ys>=shb->size) return 1.0;

	/* z berekenen */
	ofs= (ys>>4)*(shb->size>>4) + (xs>>4);
	ct= shb->cbuf+ofs;
	zsamp= *( (long *)(shb->zbuf+ofs) );

	if(*ct==3) {
		ct= ((char *)zsamp)+3*16*(ys & 15)+3*(xs & 15);
		cz= (char *)&zsamp;
		cz[0]= ct[0];
		cz[1]= ct[1];
		cz[2]= ct[2];
	}
	else if(*ct==2) {
		ct= ((char *)zsamp);
		zsamp= *(long *)ct;
		ct+= 4+2*16*(ys & 15)+2*(xs & 15);

		cz= (char *)&zsamp;
		cz[1]= ct[0];
		cz[2]= ct[1];
	}
	else if(*ct==1) {
		ct= ((char *)zsamp);
		zsamp= *(long *)ct;
		ct+= 4+16*(ys & 15)+(xs & 15);

		cz= (char *)&zsamp;
		cz[2]= ct[0];

	}

	/* if(zsamp >= 0x7FFFFE00) return 0.0; */	/* geen schaduw als op oneindig wordt gesampeld*/

	if(zsamp > zs) return 0.0; 		/* absoluut geen schaduw */
	else if( zsamp < zs-bias) return 1.0 ;	/* absoluut wel schaduw */
	else {					/* zacht gebied */

		temp=  ( (float)(zs- zsamp) )/(float)bias;
		return temp*temp;
			
	}
}


float testshadowbuf(shb)	/* return 1.0: geen schaduw */
struct ShadBuf *shb;
{
	float fac, co[4], dx[3], dy[3], vec[3], aantal=0;
	float xs1,ys1, siz,*j,xres,yres;
	long xs,ys, zs;
	short a,num;

	/* renderco en osaco roteren */
	siz= 0.5*(float)shb->size;
	VECCOPY(co, R.co);
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat, co);	/* rationele hom co */
	xs1= siz*(1.0+co[0]/co[3]);
	ys1= siz*(1.0+co[1]/co[3]);
	
	/* clip op z */
	fac= (co[2]/co[3]);
	if(fac>=1.0) return 0.0;
	else if(fac<= -1.0) return 1.0;

	zs= ((float)0x7FFFFFFF)*fac;

	/* num*num samples nemen, gebied met fac vergroten */
	num= shb->samp*shb->samp;
	fac= shb->soft;
	bias= shb->bias;

	if(num==1) return 1.0- readshadowbuf(shb,(long)xs1,(long)ys1,zs);

	co[0]= R.co[0]+O.dxco[0];
	co[1]= R.co[1]+O.dxco[1];
	co[2]= R.co[2]+O.dxco[2];
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat,co);	/* rationele hom co */
	dx[0]= xs1- siz*(1.0+co[0]/co[3]);
	dx[1]= ys1- siz*(1.0+co[1]/co[3]);

	co[0]= R.co[0]+O.dyco[0];
	co[1]= R.co[1]+O.dyco[1];
	co[2]= R.co[2]+O.dyco[2];
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat,co);	/* rationele hom co */
	dy[0]= xs1- siz*(1.0+co[0]/co[3]);
	dy[1]= ys1- siz*(1.0+co[1]/co[3]);

	xres= fac*( fabs(dx[0])+fabs(dy[0]) );
	yres= fac*( fabs(dx[1])+fabs(dy[1]) );

	if(xres<fac) xres= fac;
	if(yres<fac) yres= fac;
	
	xs1-= (xres)/2;
	ys1-= (yres)/2;

	j= shb->jit[0];

	if(xres<16.0 && yres<16.0) {
	    if(firstreadshadbuf(shb, (long)xs1, (long)ys1, 0)) {
		if(firstreadshadbuf(shb, (long)(xs1+xres), (long)ys1, 1)) {
		    if(firstreadshadbuf(shb, (long)xs1, (long)(ys1+yres), 1)) {
			if(firstreadshadbuf(shb, (long)(xs1+xres), (long)(ys1+yres), 1)) {
			    return 1.0- readshadowbuf(shb,(long)xs1,(long)ys1,zs);
			}
		    }
		}
	    }
	}

	for(a=num;a>0;a--) {	
		    /* i.p.v. jit ook met random geprobeerd: lelijk! */
		xs= xs1 + xres*j[0];
		ys= ys1 + yres*j[1];
		j+=2;
		
		aantal+= readshadowbuf(shb,xs,ys,zs);
	}

	return 1.0- aantal/( (float)(num) );
}



float testshadowbufo(shb)
struct ShadBuf *shb;
{
	float fac,co[4],dx[3],dy[3],vec[3],aantal=0;
	float xs1,ys1,siz,*j,xres,yres;
	long xs,ys,zs,zs1;
	short a,b,n,tel=0;

	/* renderco en osaco roteren */
	VECCOPY(co,R.co);
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat,co);	/* rationele hom co */

	xs1= ((0.5*(float)shb->size)*(1.0+co[0]/co[3]));
	ys1= ((0.5*(float)shb->size)*(1.0+co[1]/co[3]));
		/* clip op z */
	fac= (co[2]/co[3]);
	if(fac>=1.0) return 0.0;
	else if(fac<=0.0) return 1.0;

	zs1= 0x7FFFFFFF*fac;

	/* n*n samples nemen, gebied met fac vergroten */
	n= shb->samp;
	fac= shb->soft;
	bias= shb->bias;

	if(n==1) return 1.0- readshadowbuf(shb,(long)xs1,(long)ys1,zs1);

	co[0]= R.co[0]+O.dxco[0];
	co[1]= R.co[1]+O.dxco[1];
	co[2]= R.co[2]+O.dxco[2];
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat,co);	/* rationele hom co */
	dx[0]= xs1-((0.5*(float)shb->size)*(1.0+co[0]/co[3]));
	dx[1]= ys1-((0.5*(float)shb->size)*(1.0+co[1]/co[3]));
	dx[2]= zs1 -0x7FFFFFFF*(co[2]/co[3]);

	co[0]= R.co[0]+O.dyco[0];
	co[1]= R.co[1]+O.dyco[1];
	co[2]= R.co[2]+O.dyco[2];
	co[3]= 1.0;
	Mat4MulVec4fl(shb->winmat,co);	/* rationele hom co */
	dy[0]= xs1-((0.5*(float)shb->size)*(1.0+co[0]/co[3]));
	dy[1]= ys1-((0.5*(float)shb->size)*(1.0+co[1]/co[3]));
	dy[2]= zs1 -0x7FFFFFFF*(co[2]/co[3]);

	siz= ( fabs(dx[0])+fabs(dy[0]) )*( fabs(dx[1])+fabs(dy[1]) );
	if( siz< fac) fac/= siz;

	fac/= 2.0;
	xs1-= (dx[0]+dy[0])*fac;
	ys1-= (dx[1]+dy[1])*fac;
	zs1-= (dx[2]+dy[2])*fac;
	
	fac*= 2.0;

	dx[0]*=fac; dx[1]*=fac; dx[2]*=fac;
	dy[0]*=fac; dy[1]*=fac; dy[2]*=fac;

	for(a=0;a<n;a++) {	
		for(b=0;b<n;b++) {
			j= jitshad[tel++];

			xs= xs1+ dy[0]*j[1] +dx[0]*j[0];
			ys= ys1+ dy[1]*j[1] +dx[1]*j[0];
			zs= zs1+ dy[2]*j[1] +dx[2]*j[0];

			aantal+= readshadowbuf(shb,xs,ys,zs);
		}
	}
	
	return 1.0- aantal/( (float)(n*n) );
}


/* sampelen met filter
	xstart= xs-1;
	ystart= ys-1;
	if(xstart<0) xstart= 0;
	if(ystart<0) ystart= 0;
	xend= xstart+2;
	yend= ystart+2;
	if(xend>=shb->size) { xstart= shb->size-3; xend= shb->size-1;}
	if(yend>=shb->size) { ystart= shb->size-3; yend= shb->size-1;}

	fid= filt3;
	for(ys=ystart;ys<=yend;ys++) {
		rz= shb->buf+ ys*shb->size+ xstart;
		for(xs= xstart;xs<=xend;xs++,rz++) {
			if( *rz+0x100000<zs) aantal+= *fid;
			fid++;
		}
	}
	

	return 1.0-((float)aantal)/16.0;
*/











