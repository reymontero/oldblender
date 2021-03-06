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

/* 	zbuf.c	 */


/*
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

void snijpz(v1,v2,v3,xs,dx,f1,omsl,min,max)
void projectvert(v1, adr)
void zbuf(vlr)
short zbuffermetdehand()
short zbuffermetdehandAli(xw,yw,xpart,ypart)


*/

#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <string.h>
#include "/usr/people/include/Trace.h"

#define ABUFPART 64

struct APixstr {
	short mask[4];
	long z[4];
	long p[4];
	struct APixstr *next;
};

struct APixstrMain
{
	struct APixstr *ps;
	struct APixstrMain *next;
};

extern long count_mask();
extern float centLut[16];
extern char *centmask;

float *vlzp[32][3], labda[3][2], vez[400], *p[40], Zmulx, Zmuly;
float Zjitx, Zjity;
ulong Zvlnr, Zsample;
struct VlakRen *Zvlr;
void (*zbuffunc)();

struct APixstr *APixbuf;
ushort *Acolrow;
long *Arectz;
long Aminy, Amaxy, Azvoordeel;
struct APixstrMain apsmfirst;
short apsmteller= 0;


void fillrect(rect,x,y,val)
ulong *rect,x,y,val;
{
	ulong len,len4,*drect;

	len= x*y;
	drect= rect;
	while(len>0) {
		len--;
		*drect= val;
		drect++;
	}
}

/* *************  ACCU ZBUF ************ */

struct APixstr *addpsmainA()
{
	struct APixstrMain *psm;

	psm= &apsmfirst;

	while(psm->next) {
		psm= psm->next;
	}

	psm->next= mallocN(sizeof(struct APixstrMain), "addpsmainA");

	psm= psm->next;
	psm->next=0;
	psm->ps= callocN(4096*sizeof(struct APixstr),"pixstr");
	apsmteller= 0;

	return psm->ps;
}

void freepsA()
{
	struct APixstrMain *psm, *next;

	psm= &apsmfirst;

	while(psm) {
		next= psm->next;
		if(psm->ps) {
			freeN(psm->ps);
			psm->ps= 0;
		}
		if(psm!= &apsmfirst) freeN(psm);
		psm= next;
	}

	apsmfirst.next= 0;
	apsmfirst.ps= 0;
	apsmteller= 0;
}

struct APixstr *addpsA()
{
	static struct APixstr *prev;

	/* eerste PS maken */
	if((apsmteller & 4095)==0) prev= addpsmainA();
	else prev++;
	apsmteller++;
	
	return prev;
}

void zbufinvulAc(v1,v2,v3)  /* vult kleur in, met windowcoordinaat, van Aminy->Amaxy */
float *v1,*v2,*v3;
{
	struct APixstr *ap, *apofs, *apn;
	double x0,y0,z0,x1,y1,z1,x2,y2,z2,xx1;
	double zxd,zyd,zy0;
	float *minv,*maxv,*midv;
	register long *rz,zverg,zvlak,x;
	long my0,my2,sn1,sn2,rectx,zd,*rectzofs;
	long y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2, mask;
	
	/* MIN MAX */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1]) 	{
			minv=v1; midv=v2; maxv=v3;
		}
		else if(v1[1]<v3[1]) 	{
			minv=v1; midv=v3; maxv=v2;
		}
		else	{
			minv=v3; midv=v1; maxv=v2;
		}
	}
	else {
		if(v1[1]<v3[1]) 	{
			minv=v2; midv=v1; maxv=v3;
		}
		else if(v2[1]<v3[1]) 	{
			minv=v2; midv=v3; maxv=v1;
		}
		else	{
			minv=v3; midv=v2; maxv=v1;
		}
	}

	if(minv[1] == maxv[1]) return;	/* beveiliging 'nul' grote vlakken */

	my0= fceil(minv[1]);
	my2= ffloor(maxv[1]);
	omsl= ffloor(midv[1]);

	if(my2<Aminy || my0> Amaxy) return;

	if(my0<Aminy) my0= Aminy;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1!=0) {
		z0= (maxv[0]-minv[0])/xx1;
		dx0= (-65536.0*z0);
		xs0= 65536.0*(z0*(my2-minv[1])+minv[0]);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1!=0) {
		z0= (maxv[0]-midv[0])/xx1;
		dx1= (-65536.0*z0);
		xs1= 65536.0*(z0*(my2-midv[1])+midv[0]);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1!=0) {
		z0= (midv[0]-minv[0])/xx1;
		dx2= (-65536.0*z0);
		xs2= 65536.0*(z0*(omsl-minv[1])+minv[0]);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	x1= v1[0]- v2[0];
	x2= v2[0]- v3[0];
	y1= v1[1]- v2[1];
	y2= v2[1]- v3[1];
	z1= v1[2]- v2[2];
	z2= v2[2]- v3[2];
	x0= y1*z2-z1*y2;
	y0= z1*x2-x1*z2;
	z0= x1*y2-y1*x2;
	if(z0==0.0) return;

	if(midv[1]==maxv[1]) omsl= my2;
	if(omsl<Aminy) omsl= Aminy-1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 > Amaxy) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (x0*v1[0]+y0*v1[1])/z0+v1[2];

	zxd= -x0/z0;
	zyd= -y0/z0;
	zy0= my2*zyd+xx1;
	zd= (long)zxd;

	/* start-ofset in rect */
	rectx= R.rectx;
	rectzofs= (long *)(Arectz+rectx*(my2-Aminy));
	apofs= (APixbuf+ rectx*(my2-Aminy));
	mask= 1<<Zsample;
	zvlak= Zvlnr;

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		xs3= xs0;
		xs0= xs1;
		xs1= xs3;
		xs3= dx0;
		dx0= dx1;
		dx1= xs3;
		xs3= 1;	/* flag */

	}

	for(y=my2;y>omsl;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs1);
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		ap= apofs+sn1;
		x= sn2-sn1;
		
		zverg-= Azvoordeel;
		
		while(x>=0) {
			if(zverg< *rz) {
				apn= ap;
				while(apn) {	/* loopunrolled */
					if(apn->p[0]==0) {apn->p[0]= Zvlnr; apn->z[0]= zverg; apn->mask[0]= mask; break; }
					if(apn->p[0]==Zvlnr) {apn->mask[0]|= mask; break; }
					if(apn->p[1]==0) {apn->p[1]= Zvlnr; apn->z[1]= zverg; apn->mask[1]= mask; break; }
					if(apn->p[1]==Zvlnr) {apn->mask[1]|= mask; break; }
					if(apn->p[2]==0) {apn->p[2]= Zvlnr; apn->z[2]= zverg; apn->mask[2]= mask; break; }
					if(apn->p[2]==Zvlnr) {apn->mask[2]|= mask; break; }
					if(apn->p[3]==0) {apn->p[3]= Zvlnr; apn->z[3]= zverg; apn->mask[3]= mask; break; }
					if(apn->p[3]==Zvlnr) {apn->mask[3]|= mask; break; }
					if(apn->next==0) apn->next= addpsA();
					apn= apn->next;
				}				
			}
			zverg+= zd;
			rz++; 
			ap++; 
			x--;
		}
		zy0-= zyd;
		rectzofs-= rectx;
		apofs-= rectx;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(; y>=my0; y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs2);
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		ap= apofs+sn1;
		x= sn2-sn1;

		zverg-= Azvoordeel;

		while(x>=0) {
			if(zverg< *rz) {
				apn= ap;
				while(apn) {	/* loopunrolled */
					if(apn->p[0]==0) {apn->p[0]= Zvlnr; apn->z[0]= zverg; apn->mask[0]= mask; break; }
					if(apn->p[0]==Zvlnr) {apn->mask[0]|= mask; break; }
					if(apn->p[1]==0) {apn->p[1]= Zvlnr; apn->z[1]= zverg; apn->mask[1]= mask; break; }
					if(apn->p[1]==Zvlnr) {apn->mask[1]|= mask; break; }
					if(apn->p[2]==0) {apn->p[2]= Zvlnr; apn->z[2]= zverg; apn->mask[2]= mask; break; }
					if(apn->p[2]==Zvlnr) {apn->mask[2]|= mask; break; }
					if(apn->p[3]==0) {apn->p[3]= Zvlnr; apn->z[3]= zverg; apn->mask[3]= mask; break; }
					if(apn->p[3]==Zvlnr) {apn->mask[3]|= mask; break; }
					if(apn->next==0) apn->next= addpsA();
					apn= apn->next;
				}
			}
			zverg+= zd;
			rz++; 
			ap++; 
			x--;
		}

		zy0-=zyd;
		rectzofs-= rectx;
		apofs-= rectx;
	}
}

/* *************  NORMAL ZBUFFER ************ */

void hoco_to_zco(float *zco, float *hoco)
{
	float deler;

	deler= hoco[3];
	zco[0]= Zmulx*(1.0+hoco[0]/deler)+ Zjitx;
	zco[1]= Zmuly*(1.0+hoco[1]/deler)+ Zjity;
	zco[2]= 0x7FFFFFFF *(hoco[2]/deler);
}

void zbufline(vec1, vec2)
float *vec1, *vec2;
{
	ulong *rectz, *rectp;
	int start, end, x, y, oldx, oldy, ofs;
	int dz, vergz;
	float dx, dy;
	float v1[3], v2[3];
	
	dx= vec2[0]-vec1[0];
	dy= vec2[1]-vec1[1];

	if(fabs(dx) > fabs(dy)) {

		/* alle lijnen van links naar rechts */
		if(vec1[0]<vec2[0]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[0]);
		end= start+ffloor(dx);
		if(end>=R.rectx) end= R.rectx-1;
		
		oldy= ffloor(v1[1]);
		dy/= dx;
		
		vergz= v1[2];
		dz= (v2[2]-v1[2])/dx;
		
		rectz= R.rectz+ oldy*R.rectx+ start;
		rectp= R.rectot+ oldy*R.rectx+ start;
		
		if(dy<0) ofs= -R.rectx;
		else ofs= R.rectx;
		
		for(x= start; x<=end; x++, rectz++, rectp++) {
			
			y= ffloor(v1[1]);
			if(y!=oldy) {
				oldy= y;
				rectz+= ofs;
				rectp+= ofs;
			}
			
			if(x>=0 && y>=0 && y<R.recty) {
				if(vergz<*rectz) {
					*rectz= vergz;
					*rectp= Zvlnr;
				}
			}
			
			v1[1]+= dy;
			vergz+= dz;
		}
	}
	else {
		/* alle lijnen van onder naar boven */
		if(vec1[1]<vec2[1]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[1]);
		end= start+ffloor(dy);
		
		if(end>=R.recty) end= R.recty-1;
		
		oldx= ffloor(v1[0]);
		dx/= dy;
		
		vergz= v1[2];
		dz= (v2[2]-v1[2])/dy;

		rectz= R.rectz+ start*R.rectx+ oldx;
		rectp= R.rectot+ start*R.rectx+ oldx;
		
		if(dx<0) ofs= -1;
		else ofs= 1;

		for(y= start; y<=end; y++, rectz+=R.rectx, rectp+=R.rectx) {
			
			x= ffloor(v1[0]);
			if(x!=oldx) {
				oldx= x;
				rectz+= ofs;
				rectp+= ofs;
			}
			
			if(x>=0 && y>=0 && x<R.rectx) {
				if(vergz<*rectz) {
					*rectz= vergz;
					*rectp= Zvlnr;
				}
			}
			
			v1[0]+= dx;
			vergz+= dz;
		}
	}
}


void zbufclipwire(struct VlakRen *vlr)
{
	int clipline();
	float *f1,*f2,*f3, deler, *vzp;
	int c1, c2, c3, ec;

	c1= vlr->v1->clip;
	c2= vlr->v2->clip;
	c3= vlr->v3->clip;
	f1= vlr->v1->ho;
	f2= vlr->v2->ho;
	f3= vlr->v3->ho;

	/* edgecode: 1=niet tekenen */
	if (vlr->vl) ec = vlr->vl->ec >> 5;
	else ec = vlr->ec & 7;

	if(ec==7) return;
	
	if(c1 | c2 | c3) {	/* niet in midden */
		if(c1 & c2 & c3) {	/* helemaal eruit */
			return;
		} else {	/* clippen */

			vez[0]= f1[0]; vez[1]= f1[1]; vez[2]= f1[2]; vez[3]= f1[3];
			vez[4]= f2[0]; vez[5]= f2[1]; vez[6]= f2[2]; vez[7]= f2[3];
			if((ec & 4)==0) {
				if( clipline(vez, vez+4)==0) ec |= 4;
			}
			vzp= vez+ 8;
			vzp[0]= f2[0]; vzp[1]= f2[1]; vzp[2]= f2[2]; vzp[3]= f2[3];
			vzp[4]= f3[0]; vzp[5]= f3[1]; vzp[6]= f3[2]; vzp[7]= f3[3];
			if((ec & 2)==0) {
				if( clipline(vzp, vzp+4)==0) ec |= 2;
			}
			vzp= vez+16;
			vzp[0]= f3[0]; vzp[1]= f3[1]; vzp[2]= f3[2]; vzp[3]= f3[3];
			vzp[4]= f1[0]; vzp[5]= f1[1]; vzp[6]= f1[2]; vzp[7]= f1[3];
			if((ec & 1)==0) {
				if( clipline(vzp, vzp+4)==0) ec |= 1;
			}
			
			/* tekenen */
			if((ec & 4)==0) {
				hoco_to_zco(vez, vez);
				hoco_to_zco(vez+4, vez+4);
				zbufline(vez, vez+4);
			}
			if((ec & 2)==0) {
				hoco_to_zco(vez+8, vez+8);
				hoco_to_zco(vez+12, vez+12);
				zbufline(vez+8, vez+12);
			}
			if((ec & 1)==0) {
				hoco_to_zco(vez+16, vez+16);
				hoco_to_zco(vez+20, vez+20);
				zbufline(vez+16, vez+20);
			}
			
			return;
		}
	}

	deler= f1[3];
	vez[0]= Zmulx*(1.0+f1[0]/deler)+ Zjitx;
	vez[1]= Zmuly*(1.0+f1[1]/deler)+ Zjity;
	vez[2]= 0x7FFFFFFF *(f1[2]/deler);

	deler= f2[3];
	vez[4]= Zmulx*(1.0+f2[0]/deler)+ Zjitx;
	vez[5]= Zmuly*(1.0+f2[1]/deler)+ Zjity;
	vez[6]= 0x7FFFFFFF *(f2[2]/deler);

	deler= f3[3];
	vez[8]= Zmulx*(1.0+f3[0]/deler)+ Zjitx;
	vez[9]= Zmuly*(1.0+f3[1]/deler)+ Zjity;
	vez[10]= 0x7FFFFFFF *(f3[2]/deler);

	if((ec & 4)==0) zbufline(vez, vez+4);
	if((ec & 2)==0) zbufline(vez+4, vez+8);
	if((ec & 1)==0) zbufline(vez+8, vez);

}

void zbufinvulGLinv(v1,v2,v3) 
float *v1,*v2,*v3;
/* vult in R.rectot de waarde Zvlnr in met R.rectz */
/* KEERT Z-VERGELIJKING OM: ALLES ACHTER IS ZICHTBAAR */
{
	double x0,y0,z0,x1,y1,z1,x2,y2,z2,xx1;
	double zxd,zyd,zy0;
	float *minv,*maxv,*midv;
	ulong *rectpofs,*rp;
	register long *rz,zverg,zvlak,x;
	long my0,my2,sn1,sn2,rectx,zd,*rectzofs;
	long y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2;
	
	/* MIN MAX */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1]) 	{
			minv=v1;  midv=v2;  maxv=v3;
		}
		else if(v1[1]<v3[1]) 	{
			minv=v1;  midv=v3;  maxv=v2;
		}
		else	{
			minv=v3;  midv=v1;  maxv=v2;
		}
	}
	else {
		if(v1[1]<v3[1]) 	{
			minv=v2;  midv=v1; maxv=v3;
		}
		else if(v2[1]<v3[1]) 	{
			minv=v2;  midv=v3;  maxv=v1;
		}
		else	{
			minv=v3;  midv=v2;  maxv=v1;
		}
	}

	my0= fceil(minv[1]);
	my2= ffloor(maxv[1]);
	omsl= ffloor(midv[1]);

	if(my2<0 || my0> R.recty) return;

	if(my0<0) my0=0;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1!=0) {
		z0= (maxv[0]-minv[0])/xx1;
		dx0= (-65536.0*z0);
		xs0= 65536.0*(z0*(my2-minv[1])+minv[0]);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1!=0) {
		z0= (maxv[0]-midv[0])/xx1;
		dx1= (-65536.0*z0);
		xs1= 65536.0*(z0*(my2-midv[1])+midv[0]);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1!=0) {
		z0= (midv[0]-minv[0])/xx1;
		dx2= (-65536.0*z0);
		xs2= 65536.0*(z0*(omsl-minv[1])+minv[0]);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	x1= v1[0]- v2[0];
	x2= v2[0]- v3[0];
	y1= v1[1]- v2[1];
	y2= v2[1]- v3[1];
	z1= v1[2]- v2[2];
	z2= v2[2]- v3[2];
	x0= y1*z2-z1*y2;
	y0= z1*x2-x1*z2;
	z0= x1*y2-y1*x2;
	/* printf("x0 %f y0 %f z0 %f\n", x0, y0, z0); */
	if(z0==0.0) return;

	if(midv[1]==maxv[1]) omsl= my2;
	if(omsl<0) omsl= -1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 >= R.recty) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (x0*v1[0]+y0*v1[1])/z0+v1[2];

	zxd= -x0/z0;
	zyd= -y0/z0;
	zy0= my2*zyd+xx1;
	zd= (long)zxd;

	/* start-ofset in rect */
	rectx= R.rectx;
	rectzofs= (long *)(R.rectz+rectx*my2);
	rectpofs= (R.rectot+rectx*my2);
	zvlak= Zvlnr;

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		xs3= xs0;
		xs0= xs1;
		xs1= xs3;
		xs3= dx0;
		dx0= dx1;
		dx1= xs3;
		xs3= 1;	/* flag */

	}

	for(y=my2;y>omsl;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs1);
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		rp= rectpofs+sn1;
		x= sn2-sn1;
		while(x>=0) {
			if(zverg> *rz || *rz==0x7FFFFFFF) {
				*rz= zverg;
				*rp= zvlak;
			}
			zverg+= zd;
			rz++; 
			rp++; 
			x--;
		}
		zy0-=zyd;
		rectzofs-= rectx;
		rectpofs-= rectx;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(;y>=my0;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs2);
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		rp= rectpofs+sn1;
		x= sn2-sn1;
		while(x>=0) {
			if(zverg> *rz || *rz==0x7FFFFFFF) {
				*rz= zverg;
				*rp= zvlak;
			}
			zverg+= zd;
			rz++; 
			rp++; 
			x--;
		}

		zy0-=zyd;
		rectzofs-= rectx;
		rectpofs-= rectx;
	}
}

void zbufinvulGL(v1,v2,v3)  /* vult in R.rectot de waarde Zvlnr in met R.rectz */
float *v1,*v2,*v3;
{
	double x0,y0,z0,x1,y1,z1,x2,y2,z2,xx1;
	double zxd,zyd,zy0;
	float *minv,*maxv,*midv;
	ulong *rectpofs,*rp;
	register long *rz,zverg,zvlak,x;
	long my0,my2,sn1,sn2,rectx,zd,*rectzofs;
	long y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2;
	
	/* MIN MAX */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1]) 	{
			minv=v1;  midv=v2;  maxv=v3;
		}
		else if(v1[1]<v3[1]) 	{
			minv=v1;  midv=v3;  maxv=v2;
		}
		else	{
			minv=v3;  midv=v1;  maxv=v2;
		}
	}
	else {
		if(v1[1]<v3[1]) 	{
			minv=v2;  midv=v1; maxv=v3;
		}
		else if(v2[1]<v3[1]) 	{
			minv=v2;  midv=v3;  maxv=v1;
		}
		else	{
			minv=v3;  midv=v2;  maxv=v1;
		}
	}

	if(minv[1] == maxv[1]) return;	/* beveiliging 'nul' grote vlakken */

	my0= fceil(minv[1]);
	my2= ffloor(maxv[1]);
	omsl= ffloor(midv[1]);

	if(my2<0 || my0> R.recty) return;

	if(my0<0) my0=0;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1!=0) {
		z0= (maxv[0]-minv[0])/xx1;
		dx0= (-65536.0*z0);
		xs0= 65536.0*(z0*(my2-minv[1])+minv[0]);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1!=0) {
		z0= (maxv[0]-midv[0])/xx1;
		dx1= (-65536.0*z0);
		xs1= 65536.0*(z0*(my2-midv[1])+midv[0]);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1!=0) {
		z0= (midv[0]-minv[0])/xx1;
		dx2= (-65536.0*z0);
		xs2= 65536.0*(z0*(omsl-minv[1])+minv[0]);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	x1= v1[0]- v2[0];
	x2= v2[0]- v3[0];
	y1= v1[1]- v2[1];
	y2= v2[1]- v3[1];
	z1= v1[2]- v2[2];
	z2= v2[2]- v3[2];
	x0= y1*z2-z1*y2;
	y0= z1*x2-x1*z2;
	z0= x1*y2-y1*x2;
	/* printf("x0 %f y0 %f z0 %f\n", x0, y0, z0); */
	if(z0==0.0) return;

	if(midv[1]==maxv[1]) omsl= my2;
	if(omsl<0) omsl= -1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 >= R.recty) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (x0*v1[0]+y0*v1[1])/z0+v1[2];

	zxd= -x0/z0;
	zyd= -y0/z0;
	zy0= my2*zyd+xx1;
	zd= (long)zxd;

	/* start-ofset in rect */
	rectx= R.rectx;
	rectzofs= (long *)(R.rectz+rectx*my2);
	rectpofs= (R.rectot+rectx*my2);
	zvlak= Zvlnr;

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		xs3= xs0;
		xs0= xs1;
		xs1= xs3;
		xs3= dx0;
		dx0= dx1;
		dx1= xs3;
		xs3= 1;	/* flag */

	}

	for(y=my2;y>omsl;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs1);
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		rp= rectpofs+sn1;
		x= sn2-sn1;
		while(x>=0) {
			if(zverg< *rz) {
				*rz= zverg;
				*rp= zvlak;
			}
			zverg+= zd;
			rz++; 
			rp++; 
			x--;
		}
		zy0-=zyd;
		rectzofs-= rectx;
		rectpofs-= rectx;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(;y>=my0;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs2);
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;
		rp= rectpofs+sn1;
		x= sn2-sn1;
		while(x>=0) {
			if(zverg< *rz) {
				*rz= zverg;
				*rp= zvlak;
			}
			zverg+= zd;
			rz++; 
			rp++; 
			x--;
		}

		zy0-=zyd;
		rectzofs-= rectx;
		rectpofs-= rectx;
	}
}


void zbufinvulGL_onlyZ(v1,v2,v3)   /* vult alleen R.rectz. oneven z= 1x ingevuld */
float *v1,*v2,*v3;
{
	double x0,y0,z0,x1,y1,z1,x2,y2,z2,xx1;
	double zxd,zyd,zy0;
	float *minv,*maxv,*midv;
	register long *rz,zverg,x;
	long my0,my2,sn1,sn2,rectx,zd,*rectzofs;
	long y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2;
	
	/* MIN MAX */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1]) 	{
			minv=v1; 
			midv=v2; 
			maxv=v3;
		}
		else if(v1[1]<v3[1]) 	{
			minv=v1; 
			midv=v3; 
			maxv=v2;
		}
		else	{
			minv=v3; 
			midv=v1; 
			maxv=v2;
		}
	}
	else {
		if(v1[1]<v3[1]) 	{
			minv=v2; 
			midv=v1; 
			maxv=v3;
		}
		else if(v2[1]<v3[1]) 	{
			minv=v2; 
			midv=v3; 
			maxv=v1;
		}
		else	{
			minv=v3; 
			midv=v2; 
			maxv=v1;
		}
	}

	my0= fceil(minv[1]);
	my2= ffloor(maxv[1]);
	omsl= ffloor(midv[1]);

	if(my2<0 || my0> R.recty) return;

	if(my0<0) my0=0;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1!=0) {
		z0= (maxv[0]-minv[0])/xx1;
		dx0= (-65536.0*z0);
		xs0= 65536.0*(z0*(my2-minv[1])+minv[0]);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1!=0) {
		z0= (maxv[0]-midv[0])/xx1;
		dx1= (-65536.0*z0);
		xs1= 65536.0*(z0*(my2-midv[1])+midv[0]);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1!=0) {
		z0= (midv[0]-minv[0])/xx1;
		dx2= (-65536.0*z0);
		xs2= 65536.0*(z0*(omsl-minv[1])+minv[0]);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	x1= v1[0]- v2[0];
	x2= v2[0]- v3[0];
	y1= v1[1]- v2[1];
	y2= v2[1]- v3[1];
	z1= v1[2]- v2[2];
	z2= v2[2]- v3[2];
	x0= y1*z2-z1*y2;
	y0= z1*x2-x1*z2;
	z0= x1*y2-y1*x2;
	/* printf("x0 %f y0 %f z0 %f\n", x0, y0, z0); */
	if(z0==0.0) return;

	if(midv[1]==maxv[1]) omsl= my2;
	if(omsl<0) omsl= -1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 >= R.recty) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (x0*v1[0]+y0*v1[1])/z0+v1[2];

	zxd= -x0/z0;
	zyd= -y0/z0;
	zy0= my2*zyd+xx1;
	zd= (long)zxd;

	/* start-ofset in rect */
	rectx= R.rectx;
	rectzofs= (long *)(R.rectz+rectx*my2);

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		xs3= xs0;
		xs0= xs1;
		xs1= xs3;
		xs3= dx0;
		dx0= dx1;
		dx1= xs3;
		xs3= 1;	/* flag */

	}

	for(y=my2;y>omsl;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs1);
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;

		x= sn2-sn1;
		while(x>=0) {
			if(zverg< *rz) {
				*rz= zverg;
			}
			zverg+= zd;
			rz++; 
			x--;
		}
		zy0-=zyd;
		rectzofs-= rectx;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(;y>=my0;y--) {

		sn1= *((short *)&xs0);
		xs0+= dx0;

		sn2= *((short *)&xs2);
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (long) (sn1*zxd+zy0);
		rz= rectzofs+sn1;

		x= sn2-sn1;
		while(x>=0) {
			if(zverg< *rz) {
				*rz= zverg;
			}
			zverg+= zd;
			rz++; 
			x--;
		}

		zy0-=zyd;
		rectzofs-= rectx;
	}
}

void print3floats(v1,v2,v3)
float *v1,*v2,*v3;
{
	printf("1  %f %f %f %f\n", v1[0], v1[1], v1[2], v1[3]);
	printf("2  %f %f %f %f\n", v2[0], v2[1], v2[2], v2[3]);
	printf("3  %f %f %f %f\n", v3[0], v3[1], v3[2], v3[3]);
}

short cliptestf(p,q,u1,u2)
float p,q,*u1,*u2;
{
	float r;

	if(p<0.0) {
		if(q<p) return 0;
		else if(q<0.0) {
			r= q/p;
			if(r>*u2) return 0;
			else if(r>*u1) *u1=r;
		}
	}
	else {
		if(p>0.0) {
			if(q<0.0) return 0;
			else if(q<p) {
				r= q/p;
				if(r<*u1) return 0;
				else if(r<*u2) *u2=r;
			}
		}
		else if(q<0.0) return 0;
	}
	return 1;
}

short testclip(v)
float *v;
{
	float abs4;	/* LET OP: deze func moet zelfde doen als cliptestf, anders problemen in zbufclip()*/
	short c=0;

	abs4= fabs(v[3]);

	if(v[2]< -abs4) c=16;			/* hier stond vroeger " if(v[2]<0) ", zie clippz() */
	else if(v[2]> abs4) c+= 32;

	if( v[0]>abs4) c+=2;
	else if( v[0]< -abs4) c+=1;

	if( v[1]>abs4) c+=4;
	else if( v[1]< -abs4) c+=8;

	return c;
}


void clippz(v1, v2, b1, b2, b3)
float *v1,*v2;
long b1,*b2,*b3;
{
	float dz,dw,u1=0.0,u2=1.0;

	labda[b1][0]= -1.0;
	labda[b1][1]= -1.0;

	dz= v2[2]-v1[2];
	dw= v2[3]-v1[3];

	/* volgens het oorspronkelijke artikel van Liang Barsky, wordt bij het clippen van
	 * hoco's met het viewplane, het getal "0" gebruikt ipv "-w" .
	 * Dit wijkt af van de andere clipping (links en onder) en vond ik ook niet erg
	 * 'homogeen'. Het is dus een fout,  wie had dat gedacht van L&B!
	 */

	if(cliptestf(-dz-dw, v1[3]+v1[2], &u1,&u2)) {
		if(cliptestf(dz-dw, v1[3]-v1[2], &u1,&u2)) {
			*b3=1;
			if(u2<1.0) {
				labda[b1][1]= u2;
				*b2=1;
			}
			else labda[b1][1]=1.0;  /* u2 */
			if(u1>0.0) {
				labda[b1][0]= u1;
				*b2=1;
			} else labda[b1][0]=0.0;
		}
	}
}

void clippx(v1,v2,b1,b2,b3)
float *v1,*v2;
long b1,*b2,*b3;
{
	float dx,dz,u1=0.0,u2=1.0;

	labda[b1][0]= -1.0;
	labda[b1][1]= -1.0;

	dx= v2[0]-v1[0];
	dz= v2[3]-v1[3];

	if(cliptestf(-dx-dz, v1[0]+v1[3], &u1,&u2)) {
		if(cliptestf(dx-dz, v1[3]-v1[0], &u1,&u2)) {
			*b3=1;
			if(u2<1.0) {
				labda[b1][1]=u2;
				*b2=1;
			} else labda[b1][1]=1.0;  /* u2 */
			if(u1>0.0) {
				labda[b1][0]=u1;
				*b2=1;
			} else labda[b1][0]=0.0;
		}
	}
}

void clippy(v1,v2,b1,b2,b3)
float *v1,*v2;
long b1,*b2,*b3;
{
	float dy,dz,u1=0.0,u2=1.0;

	labda[b1][0]= -1.0;
	labda[b1][1]= -1.0;

	dy= v2[1]-v1[1];
	dz= v2[3]-v1[3];

	if(cliptestf(-dy-dz,v1[1]+v1[3],&u1,&u2)) {
		if(cliptestf(dy-dz,v1[3]-v1[1],&u1,&u2)) {
			*b3=1;
			if(u2<1.0) {
				labda[b1][1]=u2;
				*b2=1;
			} else labda[b1][1]=1.0;  /* u2 */
			if(u1>0.0) {
				labda[b1][0]=u1;
				*b2=1;
			} else labda[b1][0]=0.0;
		}
	}
}

int clipline(v1, v2)	/* return 0: niet tekenen */
float *v1,*v2;
{
	float dz,dw, u1=0.0, u2=1.0;
	float dx, dy;

	dz= v2[2]-v1[2];
	dw= v2[3]-v1[3];

	if(cliptestf(-dz-dw, v1[3]+v1[2], &u1,&u2)) {
		if(cliptestf(dz-dw, v1[3]-v1[2], &u1,&u2)) {

			dx= v2[0]-v1[0];
			/* dz= v2[3]-v1[3]; */
		
			if(cliptestf(-dx-dw, v1[0]+v1[3], &u1,&u2)) {
				if(cliptestf(dx-dw, v1[3]-v1[0], &u1,&u2)) {

					dy= v2[1]-v1[1];
				
					if(cliptestf(-dy-dw,v1[1]+v1[3],&u1,&u2)) {
						if(cliptestf(dy-dw,v1[3]-v1[1],&u1,&u2)) {
						
							if(u2<1.0) {
								v2[0]= v1[0]+u2*dx;
								v2[1]= v1[1]+u2*dy;
								v2[2]= v1[2]+u2*dz;
								v2[3]= v1[3]+u2*dw;
							}
							if(u1>0.0) {
								v1[0]= v1[0]+u1*dx;
								v1[1]= v1[1]+u1*dy;
								v1[2]= v1[2]+u1*dz;
								v1[3]= v1[3]+u1*dw;
							}
							return 1;
						}
					}
				}
			}
		}
	}
	
	return 0;
}


void maakvertpira(v1,v2,b1,b2,clve)
float *v1,*v2;
long *b1,b2,*clve;
{
	float l1,l2,*adr,min1= -1.0,nul=0.0;

	l1= labda[b2][0];
	l2= labda[b2][1];

	if(l1!= -1.0) {
		if(l1!= 0.0) {
			adr= vez+4*(*clve);
			p[*b1]=adr;
			(*clve)++;
			adr[0]= v1[0]+l1*(v2[0]-v1[0]);
			adr[1]= v1[1]+l1*(v2[1]-v1[1]);
			adr[2]= v1[2]+l1*(v2[2]-v1[2]);
			adr[3]= v1[3]+l1*(v2[3]-v1[3]);
		} else p[*b1]= v1;
		(*b1)++;
	}
	if(l2!= -1.0) {
		if(l2!= 1.0) {
			adr= vez+4*(*clve);
			p[*b1]=adr;
			(*clve)++;
			adr[0]= v1[0]+l2*(v2[0]-v1[0]);
			adr[1]= v1[1]+l2*(v2[1]-v1[1]);
			adr[2]= v1[2]+l2*(v2[2]-v1[2]);
			adr[3]= v1[3]+l2*(v2[3]-v1[3]);
			(*b1)++;
		}
	}
}

void projectholo(v1, adr)
float *v1;
float *adr;
{
	static float d, zd, zy;
	float x,y,z;

	if(v1==0) {
		/* init */
		if(view0.cambase) {
			d= view0.near;
			zd= d*0.01*(float)view0.cambase->len;
			
			if(view0.cambase->holo1) zy= zd+ (1.0+0.1*view0.cambase->holo1)*d;
			else zy= 0.0;
		}
		return;
	}

	x= v1[0]; 
	y= v1[1]; 
	z= v1[2];
	
	if(G.holo==2 && view0.cambase) {
		if(z== -(zd+d)) z+= 0.001;
		x= (-z/view0.near)*(zd*x)/((zd+d) +z);
		
		if(zy!=0.0) {
			if(z== -(zy+d)) z+=0.001;
			y= (-z/view0.near)*(zy*y)/((zy+d) +z);
		}
	}
	
	adr[0]= x*G.winmat[0][0]		     +z*G.winmat[2][0];
	adr[1]= 		     y*G.winmat[1][1]+z*G.winmat[2][1];
	adr[2]= 		     		      z*G.winmat[2][2]+G.winmat[3][2];
	adr[3]=		  		      -z;
}

void projectverto(v1,adr)
float *v1;
float *adr;
{
	/* berekent homogene co van vertex v1 */
	float x,y,z;

	if(G.holo==2) {
		projectholo(v1, adr);
		return;
	}
		
	x= v1[0]; 
	y= v1[1]; 
	z= v1[2];
	adr[0]= x*G.winmat[0][0]		     +z*G.winmat[2][0];
	adr[1]= 		     y*G.winmat[1][1]+z*G.winmat[2][1];
	adr[2]= 		     				  z*G.winmat[2][2]+G.winmat[3][2];
	adr[3]=		  					      z*G.winmat[2][3]+G.winmat[3][3];
}



void projectvert(v1,adr)
float *v1;
float *adr;
{
	/* berekent homogene co van vertex v1 */
	register float x,y,z;

	x= v1[0]; 
	y= v1[1]; 
	z= v1[2];
	adr[0]= x*G.winmat[0][0]+ y*G.winmat[1][0]+ z*G.winmat[2][0]+ G.winmat[3][0];
	adr[1]= x*G.winmat[0][1]+ y*G.winmat[1][1]+ z*G.winmat[2][1]+ G.winmat[3][1];
	adr[2]= x*G.winmat[0][2]+ y*G.winmat[1][2]+ z*G.winmat[2][2]+ G.winmat[3][2];
	adr[3]= x*G.winmat[0][3]+ y*G.winmat[1][3]+ z*G.winmat[2][3]+ G.winmat[3][3];
}


void zbufclip(f1, f2, f3, c1, c2, c3)
float *f1, *f2, *f3;
long c1, c2, c3;
{
	float deler;

	if(c1 | c2 | c3) {	/* niet in midden */
		if(c1 & c2 & c3) {	/* helemaal eruit */
			return;
		} else {	/* clippen */
			long v, b, clipflag[3], b1, b2, b3, c4, clve=3, clvlo, clvl=1;

			vez[0]= f1[0]; vez[1]= f1[1]; vez[2]= f1[2]; vez[3]= f1[3];
			vez[4]= f2[0]; vez[5]= f2[1]; vez[6]= f2[2]; vez[7]= f2[3];
			vez[8]= f3[0]; vez[9]= f3[1]; vez[10]= f3[2];vez[11]= f3[3];

/* PRINT4(f, f, f, f, vez[0], vez[1], vez[2], vez[3]); */
/* PRINT4(f, f, f, f, vez[4], vez[5], vez[6], vez[7]); */
/* PRINT4(f, f, f, f, vez[8], vez[9], vez[10], vez[11]); */


			vlzp[0][0]= vez;
			vlzp[0][1]= vez+4;
			vlzp[0][2]= vez+8;

			clipflag[0]= ( (c1 & 48) | (c2 & 48) | (c3 & 48) );
			if(clipflag[0]==0) {	/* anders opnieuw berekend verderop, na de eerste (z) clip */
				clipflag[1]= ( (c1 & 3) | (c2 & 3) | (c3 & 3) );
				clipflag[2]= ( (c1 & 12) | (c2 & 12) | (c3 & 12) );
			}
			
			for(b=0;b<3;b++) {
				
				if(clipflag[b]) {
				
					clvlo= clvl;
					
					for(v=0; v<clvlo; v++) {
					
						if(vlzp[v][0]!=0) {	/* vlak is er nog */
							b2= b3 =0;	/* clipvlaggen */
							if(b==0) {
								clippz(vlzp[v][0],vlzp[v][1],0,&b2,&b3);
								clippz(vlzp[v][1],vlzp[v][2],1,&b2,&b3);
								clippz(vlzp[v][2],vlzp[v][0],2,&b2,&b3);
							} else if(b==1) {
								clippx(vlzp[v][0],vlzp[v][1],0,&b2,&b3);
								clippx(vlzp[v][1],vlzp[v][2],1,&b2,&b3);
								clippx(vlzp[v][2],vlzp[v][0],2,&b2,&b3);
							} else {
								clippy(vlzp[v][0],vlzp[v][1],0,&b2,&b3);
								clippy(vlzp[v][1],vlzp[v][2],1,&b2,&b3);
								clippy(vlzp[v][2],vlzp[v][0],2,&b2,&b3);
							}
							if(b2==0 && b3==1) {
								/* er helemaal in */;
							} else if(b3==0) {
								vlzp[v][0]=0;
								/* er helemaal uit */;
							} else {
								b1=0;
								maakvertpira(vlzp[v][0],vlzp[v][1],&b1,0,&clve);
								maakvertpira(vlzp[v][1],vlzp[v][2],&b1,1,&clve);
								maakvertpira(vlzp[v][2],vlzp[v][0],&b1,2,&clve);

								/* als frontclip is gedaan: nu pas clipvlaggen zetten */
								if(b==0) {
									clipflag[1]= clipflag[2]= 0;
									f1= vez;
									for(b3=0; b3<clve; b3++) {
										c4= testclip(f1);
										clipflag[1] |= (c4 & 3);
										clipflag[2] |= (c4 & 12);
										f1+= 4;
									}
								}
								
								vlzp[v][0]=0;
								if(b1>2) {
									for(b3=3; b3<=b1; b3++) {
										vlzp[clvl][0]= p[0];
										vlzp[clvl][1]= p[b3-2];
										vlzp[clvl][2]= p[b3-1];
										clvl++;
									}
								}
							}
						}
					}
				}
/* printf("ronde %d clve %d clvl %d\n",b,clve,clvl); */
			}

			/* persp deling */
			/* valt hier nog te optimaliseren door alleen te gebruiken vertices te doen? */
			if(clve>38) printf("clip overflow: clve clvl %d %d\n",clve,clvl);
			f1=vez;
			for(c1=0;c1<clve;c1++) {
				deler= f1[3];
				f1[0]= Zmulx*(1.0+f1[0]/deler)+ Zjitx;
				f1[1]= Zmuly*(1.0+f1[1]/deler)+ Zjity;
				f1[2]= 0x7FFFFFFF *(f1[2]/deler);
				f1+=4;
			}
/* printf("clvl %d\n",clvl); */
			for(b=1;b<clvl;b++) {
				if(vlzp[b][0]) {
					zbuffunc(vlzp[b][0],vlzp[b][1],vlzp[b][2]);
				}
			}
			return;
		}
	}

	/* persp deling */
	
	deler= f1[3];
	vez[0]= Zmulx*(1.0+f1[0]/deler)+ Zjitx;
	vez[1]= Zmuly*(1.0+f1[1]/deler)+ Zjity;
	vez[2]= 0x7FFFFFFF *(f1[2]/deler);

	deler= f2[3];
	vez[4]= Zmulx*(1.0+f2[0]/deler)+ Zjitx;
	vez[5]= Zmuly*(1.0+f2[1]/deler)+ Zjity;
	vez[6]= 0x7FFFFFFF *(f2[2]/deler);

	deler= f3[3];
	vez[8]= Zmulx*(1.0+f3[0]/deler)+ Zjitx;
	vez[9]= Zmuly*(1.0+f3[1]/deler)+ Zjity;
	vez[10]= 0x7FFFFFFF *(f3[2]/deler);

	zbuffunc(vez,vez+4,vez+8);
}

/* ***************** ZBUFFER MAINROUTINES **************** */

void zbufqstrip(qs)
struct QStrip *qs;
{
	/* als Zvlnr==0 : gewoon tellen */
	struct VertStrip *vs1, *vs2;
	long u, v, count= 0;

	if(Zvlnr==0) count= 1;

	if(count) Zvlnr= qs->firstface;

	for(v= 0; v<qs->pntsv-1; v++) {
		vs1= qs->verts+ v*qs->pntsu;
		vs2= vs1+ qs->pntsu;
		for(u= 0; u<qs->pntsu-1; u++) {
			zbufclip(vs1->ho, (vs1+1)->ho, (vs2+1)->ho, vs1->clip, (vs1+1)->clip, (vs2+1)->clip);
			if(count) Zvlnr++;
			zbufclip(vs1->ho, (vs2+1)->ho, vs2->ho, vs1->clip, (vs2+1)->clip, vs2->clip);
			if(count) Zvlnr++;
			vs1++;
			vs2++;
		}
	}

}

void zbufferall(key)
short key;
{
	struct VlakRen *vlr;
	struct QStrip *qs;
	struct ColBlckF *cb=0;
	float *f1, *f2, *f3;
	long v,len;
	short transp=0, env=0, wire=0;

	Zmulx= ((float)R.rectx)/2.0;
	Zmuly= ((float)R.recty)/2.0;

	len= 4*R.rectx*R.recty;

	fillrect(R.rectz, R.rectx, R.recty, 0x7FFFFFFF);

	Zvlnr= 0;

	zbuffunc= zbufinvulGL;

	for(v=0;v<G.totvlak;v++) {

		if((v & 255)==0) vlr= R.blovl[v>>8];
		else vlr++;
		Zvlnr= v+1;

		if(vlr->rt1) {
			if(vlr->col!=cb) {
				cb= vlr->col;
				transp= cb->mode & (64+128);
				env= (cb->mode & 32);
				wire= (cb->mode & 1024);
				
				if(cb->mode & 512) zbuffunc= zbufinvulGLinv;
				else zbuffunc= zbufinvulGL;
			}
				
			if(key) {
				if(env) Zvlnr= 0;
				else if(transp) Zvlnr= 0x010101*( (long) (255*(1.0-cb->tra)) );
				else Zvlnr= 0xFFFFFF;
				
				if(wire) zbufclipwire(vlr);
				else zbufclip(vlr->v1->ho, vlr->v2->ho, vlr->v3->ho, 
					   vlr->v1->clip, vlr->v2->clip, vlr->v3->clip);
				
			}
			else if(transp==0) {
				if(env) Zvlnr= 0;
				
				if(wire) zbufclipwire(vlr);
				else zbufclip(vlr->v1->ho, vlr->v2->ho, vlr->v3->ho, 
					vlr->v1->clip, vlr->v2->clip, vlr->v3->clip);
			}
		}
	}

	/* de QStrips */
	/*	qs= R.qstrip.first;
	while(qs) {
	    Zvlnr= 0;
	    if(R.f & (16+128) ) { 
			transp= qs->col->mode & (64+128);
			env= qs->col->mode & 32;
			if(transp==0 || key) {
				if(env) Zvlnr= 0;
				else if(key) Zvlnr= (0x010101*(255- qs->col->tra));
				
				zbufqstrip(qs);
			}
			else {
				if(key) Zvlnr= 0xFFFFFF;
			
				zbufqstrip(qs);
			}
		}
	    qs= qs->next;
	}
*/
}

void zbuffershad()
{
	struct VlakRen *vlr;
	struct ColBlckF *col;
	long a,len, ok=1;

	Zmulx= ((float)R.rectx)/2.0;
	Zmuly= ((float)R.recty)/2.0;
	Zjitx= Zjity= -.5;

	len= 4*R.rectx*R.recty;

	fillrect(R.rectz,R.rectx,R.recty,0x7FFFFFFE);

	zbuffunc= zbufinvulGL_onlyZ;

	for(a=0;a<G.totvlak;a++) {

		if((a & 255)==0) vlr= R.blovl[a>>8];
		else vlr++;

		if(vlr->col!= col) {
			col= vlr->col;
			ok= 1;
			if((col->mode & 4)==0) ok= 0;

		}
		/* traceble er weer in: automatische verwijdering niet geweldig */

		if(vlr->rt1 && ok) {
			zbufclip(vlr->v1->ho, vlr->v2->ho, vlr->v3->ho, 
			    vlr->v1->clip, vlr->v2->clip, vlr->v3->clip);
		}
	}
}



/* ******************** ABUF ************************* */


void bgnaccumbuf()
{
	
	Acolrow= mallocN(4*2*R.rectx, "Acolrow");
	Arectz= mallocN(4*ABUFPART*R.rectx, "Arectz");
	APixbuf= mallocN(ABUFPART*R.rectx*sizeof(struct APixstr), "APixbuf");

	Aminy= -1000;
	Amaxy= -1000;
	
	apsmteller= 0;
	apsmfirst.next= 0;
	apsmfirst.ps= 0;
}

void endaccumbuf()
{
	
	freeN(Acolrow);
	freeN(Arectz);
	freeN(APixbuf);
	freepsA();
}

void copyto_abufz(sample)
long sample;
{
	struct PixStr *ps;
	long x, y, *rza;
	ulong *rd;
	
	memcpy(Arectz, R.rectz+ R.rectx*Aminy, 4*R.rectx*(Amaxy-Aminy+1));

	if(G.ali==0 || sample==0) return;
		
	rza= Arectz;
	rd= (R.rectdaps+ R.rectx*Aminy);

	sample= (1<<sample);
	
	for(y=Aminy; y<=Amaxy; y++) {
		for(x=0; x<R.rectx; x++) {
			
			if(*rd & 0xFF000000) {	/* is pixstr */
				ps= (struct PixStr *)*rd;

				while(ps) {
					if(sample & ps->mask) {
						*rza= ps->z;
						break;
					}
					ps= ps->next;
				}
			}
			
			rd++; rza++;
		}
	}
}

void zbuffer_abuf()
{
	/* gejitterd zbufferen met abufpixstructen */
	extern float jit[64][2];
	float vec[3], hoco[4], mul, zval, fval;
	struct ColBlckF *cb=0;
	long v, len;
	
	Zmulx= ((float)R.rectx)/2.0;
	Zmuly= ((float)R.recty)/2.0;

	/* clear APixstructen */
	len= sizeof(struct APixstr)*R.rectx*ABUFPART;
	bzero(APixbuf, len);
	
	zbuffunc= zbufinvulAc;
	
	for(Zsample=0; Zsample<G.osa; Zsample++) {
		
		copyto_abufz(Zsample);	/* init zbuffer */

		if(G.ali) {
			Zjitx= -jit[Zsample][0];
			Zjity= -jit[Zsample][1];
		}
		
		for(v=0; v<G.totvlak; v++) {
			if((v & 255)==0) {
				Zvlr= R.blovl[v>>8];
			}
			else Zvlr++;
			
			cb= Zvlr->col;
			
			if(cb->mode & (64+128)) {
				if( (cb->mode & 2) && cb->ang>0.0) {		/* voordeeltje voor transp renderen */
					mul= 0x7FFFFFFF;
					zval= mul*(1.0+Zvlr->v1->ho[2]/Zvlr->v1->ho[3]);

					VECCOPY(vec, Zvlr->v1->co);
					vec[2]-= 255.0*100.0*cb->ang;	/* z is negatief, wordt anders geclipt */
					projectverto(vec, hoco);
					fval= mul*(1.0+hoco[2]/hoco[3]);

					Azvoordeel= (long) fabs(zval - fval );
		/* PATCH: als transp en niet transp dicht op elkaar liggen: Zbuf aliasing */
					Azvoordeel+= 0xFFFF;
				}
				else Azvoordeel= 0xFFFF;

				Zvlnr= v+1;
		
				if(Zvlr->rt1) zbufclip(Zvlr->v1->ho, Zvlr->v2->ho, Zvlr->v3->ho, 
					Zvlr->v1->clip, Zvlr->v2->clip, Zvlr->v3->clip);
			}
			if(G.afbreek) break;
		}
		
		if(G.ali==0) break;
		if(G.afbreek) break;
	}
	
}

void addAlphaOverShort(doel,bron)   /* vult bron over doel in met alpha van bron */
register ushort *doel,*bron;	/* controleert op overflow */
{
	register ulong c;
	register ulong mul;

	if( doel[0]==0 || bron[0]>=0xFFF0) {	/* is getest, scheelt veel */
		*((ulong *)doel)= *((ulong *)bron);
		*((ulong *)(doel+2))= *((ulong *)(bron+2));
		return;
	}

	mul= 0xFFFF-bron[0];

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

void addAlphaUnderShort(doel, bron)   /* vult bron onder doel in met alpha van doel */
register ushort *doel, *bron;	/* controleert op overflow */
{
	register ulong c;
	register ulong mul;

	if(doel[0]>=0xFFF0) return;
	if( doel[0]==0 ) {	/* is getest, scheelt veel */
		*((ulong *)doel)= *((ulong *)bron);
		*((ulong *)(doel+2))= *((ulong *)(bron+2));
		return;
	}

	mul= 0xFFFF-doel[0];

	c= ((mul*bron[3])>>16)+doel[3];
	if(c>=0xFFF0) doel[3]=0xFFF0; 
	else doel[3]= c;
	c= ((mul*bron[2])>>16)+doel[2];
	if(c>=0xFFF0) doel[2]=0xFFF0; 
	else doel[2]= c;
	c= ((mul*bron[1])>>16)+doel[1];
	if(c>=0xFFF0) doel[1]=0xFFF0;
	else doel[1]= c;
	c= ((mul*bron[0])>>16)+doel[0];
	if(c>=0xFFF0) doel[0]=0xFFF0;
	else doel[0]= c;

}


int addtosampcol(sampcol, shortcol, mask)
ushort *sampcol, *shortcol;
long mask;
{
	int a, retval=G.osa;
	
	for(a=0; a<G.osa; a++) {
		if(mask & (1<<a)) addAlphaUnderShort(sampcol, shortcol);
		if(sampcol[0]>0xFFF0) retval--;
		sampcol+= 4;
	}
	return retval;
}

long vergzvlak(x1,x2)
long *x1,*x2;
{

	if( x1[0] < x2[0] ) return 1;
	else if( x1[0] > x2[0]) return -1;
	return 0;
}

void shadetrapixel(x, y, vlak)
float x, y;
long vlak;
{
	extern ushort shortcol[4];
	struct VlakRen *addvlak();
	long alpha;
	char charcol[12];

	if(vlak<0 || vlak>G.totvlak) {
		printf("error in shadetrapixel nr: %d\n", vlak);
		return;
	}
	Zvlr= addvlak(vlak-1);
	if(Zvlr->col->mode & 64) {
		shadegour(Zvlr, charcol);
		shortcol[0]= 65535*(1.0-Zvlr->col->tra);
		alpha= shortcol[0]/(768);
		shortcol[1]= alpha*(charcol[1]+charcol[5]+charcol[9]);
		shortcol[2]= alpha*(charcol[2]+charcol[6]+charcol[10]);
		shortcol[3]= alpha*(charcol[3]+charcol[7]+charcol[11]);
	}
	else {
		shadepixel(x, y, vlak);
	}
	
}

void abufsetrow(y)
long y;
{
	extern ushort usegamtab;
	extern ushort shortcol[4];
	struct APixstr *ap, *apn;
	float xs, ys;
	long x, part, a, b, zrow[100][3], totvlak, alpha[32], tempgam, nr, mask, longcol[4];
	int tempRf, sval;
	ushort *col, tempcol[4], sampcol[16*4], *scol;
	
	if(y<0) return;

	tempRf= R.f;
	R.f &= ~4096;

	/* alpha LUT */
	x= (65536/G.osa);
	for(a=0; a<=G.osa; a++) {
		alpha[a]= a*x;
	}
	
	/* moet pixbuf worden gemaakt? */
	if(y<Aminy || y>Amaxy) {
		part= (y/ABUFPART);
		Aminy= part*ABUFPART;
		Amaxy= Aminy+ABUFPART-1;
		if(Amaxy>=R.recty) Amaxy= R.recty-1;
		
		freepsA();
		zbuffer_abuf();
	}
	
	/* render rij */
	col= Acolrow;
	bzero(col, 2*4*R.rectx);
	ap= APixbuf+R.rectx*(y-Aminy);
	ys= y;
	tempgam= usegamtab;
	usegamtab= 0;
	
	for(x=0; x<R.rectx; x++, col+=4, ap++) {
		if(ap->p[0]) {
		
			/* sorteer op z */
			totvlak= 0;
			apn= ap;
			while(apn) {
				for(a=0; a<4; a++) {
					if(apn->p[a]) {
						zrow[totvlak][0]= apn->z[a];
						zrow[totvlak][1]= apn->p[a];
						zrow[totvlak][2]= apn->mask[a];
						totvlak++;
						if(totvlak>99) totvlak= 99;
					}
					else break;
				}
				apn= apn->next;
			}
			if(totvlak==1) {
				
				
				if(G.ali) {
					b= centmask[ ap->mask[0] ];
					xs= (float)x+centLut[b & 15];
					ys= (float)y+centLut[b>>4];
				}
				else {
					xs= x; ys= y;
				}
				shadetrapixel(xs, ys, ap->p[0]);
	
				nr= count_mask(ap->mask[0]);
				if(G.ali && nr<G.osa) {
					a= alpha[ nr ];
					col[0]= (shortcol[0]*a)>>16;
					col[1]= (shortcol[1]*a)>>16;
					col[2]= (shortcol[2]*a)>>16;
					col[3]= (shortcol[3]*a)>>16;
				}
				else {
					col[0]= shortcol[0];
					col[1]= shortcol[1];
					col[2]= shortcol[2];
					col[3]= shortcol[3];
				}
			}
			else {

				if(totvlak==2) {
					if(zrow[0][0] < zrow[1][0]) {
						a= zrow[0][0]; zrow[0][0]= zrow[1][0]; zrow[1][0]= a;
						a= zrow[0][1]; zrow[0][1]= zrow[1][1]; zrow[1][1]= a;
						a= zrow[0][2]; zrow[0][2]= zrow[1][2]; zrow[1][2]= a;
					}

				}
				else {	/* totvlak>2 */
					qsort(zrow, totvlak, 12, vergzvlak);
				}
				
				/* samenvoegen als pixels aanvullen */
				
				while(totvlak>0) {
					totvlak--;
					
					if(G.ali) {
						b= centmask[ zrow[totvlak][2] ];
						xs= (float)x+centLut[b & 15];
						ys= (float)y+centLut[b>>4];
					}
					else {
						xs= x; ys= y;
					}
					shadetrapixel(xs, ys, zrow[totvlak][1]);
					
					a= count_mask(zrow[totvlak][2]);
					if(G.ali && a<G.osa) {
						if(totvlak>0) {
							bzero(sampcol, 4*2*G.osa);
							sval= addtosampcol(sampcol, shortcol, zrow[totvlak][2]);
								/* sval==0: alpha helemaal vol */
							if(sval) {
							
								while(totvlak>0) {
									a= count_mask(zrow[totvlak-1][2]);
									if(a==G.osa) break;
									totvlak--;
									
									b= centmask[ zrow[totvlak][2] ];
									xs= (float)x+centLut[b & 15];
									ys= (float)y+centLut[b>>4];
	
									shadetrapixel(xs, ys, zrow[totvlak][1]);
									sval= addtosampcol(sampcol, shortcol, zrow[totvlak][2]);
									if(sval==0) break;
								}
							}
							
							scol= sampcol;
							longcol[0]= scol[0]; longcol[1]= scol[1];
							longcol[2]= scol[2]; longcol[3]= scol[3];
							scol+= 4;
							for(a=1; a<G.osa; a++, scol+=4) {
								longcol[0]+= scol[0]; longcol[1]+= scol[1];
								longcol[2]+= scol[2]; longcol[3]+= scol[3];
							}
							tempcol[0]= longcol[0]/G.osa;
							tempcol[1]= longcol[1]/G.osa;
							tempcol[2]= longcol[2]/G.osa;
							tempcol[3]= longcol[3]/G.osa;
							
							addAlphaUnderShort(col, tempcol);
							
						}
						else {
							a= alpha[a];
							shortcol[0]= (shortcol[0]*a)>>16;
							shortcol[1]= (shortcol[1]*a)>>16;
							shortcol[2]= (shortcol[2]*a)>>16;
							shortcol[3]= (shortcol[3]*a)>>16;
							addAlphaUnderShort(col, shortcol);
						}
					}	
					else addAlphaUnderShort(col, shortcol);

					if(col[0]>=0xFFF0) break;

				}
			}
		}
	}

	usegamtab= tempgam;
	R.f= tempRf;
}

