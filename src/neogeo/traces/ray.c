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

/* 	ray.c	 */


/*
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'


*/

#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <string.h>
#include "/usr/people/include/Trace.h"


#define EPSILON 1.0e-20

/* ************************* */
/* globals */

struct Branch *adroct[256];
struct VlakRen *vlrcontr,*snvlr;
float tra,rtu,rtv,rtlabda,ddalabda;	/* tra: vermfactor, is max afmeting octree */
float ocfacx,ocfacy,ocfacz;
ulong raycount;
long rx1,ry1,rz1,rx2,ry2,rz2,octcount,min[3],max[3];


/* ************************* */

struct snijpsort {
	float labda,u,v;
	struct VlakRen *vlr;
	long snijp;
};



short cliptest(p,q,u1,u2)	/* floatversie cliptestf() in zbuf.c */
long p,q;
float *u1,*u2;
{
	float r;

	if(p<0) {
		if(q<p) return 0;
		else if(q<0) {
			r=(float)q/p;
			if(r>*u2) return 0;
			else if(r>*u1) *u1=r;
		}
	}
	else {
		if(p>0) {
			if(q<0) return 0;
			else if(q<p) {
				r=(float)q/p;
				if(r<*u1) return 0;
				else if(r<*u2) *u2=r;
			}
		}
		else if(q<0) return 0;
	}
	return 1;
}

void clearocvlak(ocvlak)
register long *ocvlak;
{
	register tot;

	ocvlak+=799;

	for(tot=800;tot>0;tot--) *(ocvlak--)=0;
}

void d2dda(b1,b2,c1,c2,ocvlak,rts,rtf)
short b1,b2,c1,c2;
char *ocvlak;
short rts[][3];
float rtf[][3];
{
	short ocx1,ocx2,ocy1,ocy2;
	short x,y,dx=0,dy=0;
	float ox1,ox2,oy1,oy2;
	float labda,labdao,labdax,labday,ldx,ldy;

	/* deze versie is getest met 3DDALA.GFA */

	ocx1= rts[b1][c1];
	ocy1= rts[b1][c2];
	ocx2= rts[b2][c1];
	ocy2= rts[b2][c2];

	if(ocx1==ocx2 && ocy1==ocy2) {
		ocvlak[32*ocx1+ocy1]=1;
		return;
	}

	ox1= rtf[b1][c1];
	oy1= rtf[b1][c2];
	ox2= rtf[b2][c1];
	oy2= rtf[b2][c2];

	if(ox1!=ox2) {
		if(ox2-ox1>0.0) {
			labdax= (ox1-ocx1-1.0)/(ox1-ox2);
			ldx= -1.0/(ox1-ox2);
			dx= 1;
		} else {
			labdax= (ox1-ocx1)/(ox1-ox2);
			ldx= 1.0/(ox1-ox2);
			dx= -1;
		}
	} else {
		labdax=1.0;
		ldx=0;
	}

	if(oy1!=oy2) {
		if(oy2-oy1>0.0) {
			labday= (oy1-ocy1-1.0)/(oy1-oy2);
			ldy= -1.0/(oy1-oy2);
			dy= 1;
		} else {
			labday= (oy1-ocy1)/(oy1-oy2);
			ldy= 1.0/(oy1-oy2);
			dy= -1;
		}
	} else {
		labday=1.0;
		ldy=0;
	}
	
	x=ocx1; y=ocy1;
	labda=0;
	while(labda<1.0) {
		labdao=labda;
		if(labdax==labday) {
			labdax+=ldx;
			x+=dx;
			labday+=ldy;
			y+=dy;
		} else {
			if(labdax<labday) {
				labdax+=ldx;
				x+=dx;
			} else {
				labday+=ldy;
				y+=dy;
			}
		}
		labda=MIN2(labdax,labday);
		if(labda==labdao) break;
		ocvlak[32*x+y]=1;
	}

	ocvlak[32*ocx1+ocy1]=1;
	ocvlak[32*ocx2+ocy2]=1;
}

void vuldriehoek(c1,c2,ocvlak,ocmin)
short c1,c2;
char *ocvlak;
short *ocmin;
{
	short a,x,y,y1,y2,*ocmax;

	ocmax=ocmin+3;

	for(x=ocmin[c1];x<=ocmax[c1];x++) {
		a=32*x;
		for(y=ocmin[c2];y<=ocmax[c2];y++) {
			if(ocvlak[a+y]) {
				y++;
				while(ocvlak[a+y] && y!=ocmax[c2]) y++;
				for(y1=ocmax[c2];y1>y;y1--) {
					if(ocvlak[a+y1]) {
						for(y2=y;y2<=y1;y2++) ocvlak[a+y2]=1;
						y1=0;
					}
				}
				y=ocmax[c2];
			}
		}
	}
}

struct Branch *addbranch(br,oc)
struct Branch *br;
short oc;
{
	
	if(br->b[oc]) return br->b[oc];
	
	octcount++;
	if(adroct[octcount>>8]==0)
		adroct[octcount>>8]=(struct Branch *)callocN(256*sizeof(struct Branch),"addbranch");

	if(octcount> 65530) {
		error("octree vol");
		octcount=0;
	}
	
	return br->b[oc]=adroct[octcount>>8]+(octcount & 255);
}

void ocwrite(vlr,x,y,z)
struct VlakRen *vlr;
short x,y,z;
{
	struct Branch *br;
	struct Node *no;
	short a,oc1,oc2,oc3,oc4,oc5;

	x<<=2;
	y<<=1;
	oc1= ((x & 64)+(y & 32)+(z & 16))>>4;
	oc2= ((x & 32)+(y & 16)+(z & 8))>>3;
	oc3= ((x & 16)+(y & 8)+(z & 4))>>2;
	oc4= ((x & 8)+(y & 4)+(z & 2))>>1;
	oc5= ((x & 4)+(y & 2)+(z & 1));

	br= addbranch(adroct[0],oc1);
	br= addbranch(br,oc2);
	br= addbranch(br,oc3);
	br= addbranch(br,oc4);
	no= (struct Node *)addbranch(br,oc5);

	while(no->next) no=no->next;

	if(no->v[6]) {		/* node vol */
		no->next=(struct Node *)addbranch(no,7);
		no=no->next;
	}
	a=0;
	while(no->v[a]!=0) a++;
	no->v[a]=vlr;

}

void makeoctree()
{
	struct VlakRen *vlr;
	struct VertRen *v1,*v2,*v3;
	float ocfac[3],t00,t01,t02;
	float rtf[3][3];
	long v;
	short a,b,c,rts[3][3],oc1,oc2,oc3,ocmin[6],*ocmax,x,y,z;
	char ocvlak[3212];

	ocmax=ocmin+3;

	octcount=0;
	raycount=0;
	for(a=0;a<3;a++) {
		min[a]= 0x7FFFFFFF;
		max[a]= 0x80000000;
	}
	
	/* eerst min max octreeruimte */
	for(v=0;v<G.totvlak;v++) {
		if((v & 255)==0) vlr= R.blovl[v>>8];	
		else vlr++;
		if(vlr->col->mode & 4) {	/* traceble */
			v1= vlr->v1;
			v2= vlr->v2;
			v3= vlr->v3;
			for(c=0;c<3;c++) {
				if(min[c] > v1->co[c]) min[c]= v1->co[c];
				if(min[c] > v2->co[c]) min[c]= v2->co[c];
				if(min[c] > v3->co[c]) min[c]= v3->co[c];
				if(max[c] < v1->co[c]) max[c]= v1->co[c];
				if(max[c] < v2->co[c]) max[c]= v2->co[c];
				if(max[c] < v3->co[c]) max[c]= v3->co[c];
			}
		
		}
	}
	if(min[0]==0x7FFFFFFF) return;	/* lege octree */

	adroct[0]=(struct Branch *)callocN(256*sizeof(struct Branch),"makeoctree");

	for(c=0;c<3;c++) {	/* octree iets oprekken, nog nodig? */
		min[c]-=1000;
		max[c]+=1000;
	}
	t00=max[0]-min[0]+1;
	t01=max[1]-min[1]+1;
	t02=max[2]-min[2]+1;
	
	ocfacx=ocfac[0]=31.9/t00;
	ocfacy=ocfac[1]=31.9/t01;
	ocfacz=ocfac[2]=31.9/t02;
	
	tra= fsqrt(t00*t00+t01*t01+t02*t02);	/* globaal, max afm octree */
	
	for(v=0;v<G.totvlak;v++) {
		if((v & 255)==0) vlr= R.blovl[v>>8];	
		else vlr++;
		if(vlr->col->mode & 4) {	/* traceble */
			v1= vlr->v1;
			v2= vlr->v2;
			v3= vlr->v3;
			for(c=0;c<3;c++) {
				rtf[0][c]= (v1->co[c]-min[c])*ocfac[c] ;
				rts[0][c]= (short)rtf[0][c];
				rtf[1][c]= (v2->co[c]-min[c])*ocfac[c] ;
				rts[1][c]= (short)rtf[1][c];
				rtf[2][c]= (v3->co[c]-min[c])*ocfac[c] ;
				rts[2][c]= (short)rtf[2][c];
			}
			clearocvlak(ocvlak);
			for(c=0;c<3;c++) {
				oc1= rts[0][c];
				oc2= rts[1][c];
				oc3= rts[2][c];
				ocmin[c]=MIN3(oc1,oc2,oc3);
				ocmax[c]=MAX3(oc1,oc2,oc3);
			}
			if(ocmax[c]>31) ocmax[c]=31;
			if(ocmin[c]<0) ocmin[c]=0;

			d2dda(0,1,0,1,ocvlak+1024,rts,rtf);
			d2dda(0,1,0,2,ocvlak,rts,rtf);
			d2dda(0,1,1,2,ocvlak+2048,rts,rtf);
			d2dda(1,2,0,1,ocvlak+1024,rts,rtf);
			d2dda(1,2,0,2,ocvlak,rts,rtf);
			d2dda(1,2,1,2,ocvlak+2048,rts,rtf);
			d2dda(2,0,0,1,ocvlak+1024,rts,rtf);
			d2dda(2,0,0,2,ocvlak,rts,rtf);
			d2dda(2,0,1,2,ocvlak+2048,rts,rtf);
			
			vuldriehoek(0,1,ocvlak+1024,ocmin);
			vuldriehoek(0,2,ocvlak,ocmin);
			vuldriehoek(1,2,ocvlak+2048,ocmin);
			
			for(x=ocmin[0];x<=ocmax[0];x++) {
				a=32*x;
				for(y=ocmin[1];y<=ocmax[1];y++) {
					b=32*y;
					if(ocvlak[a+y+1024]) {
						for(z=ocmin[2];z<=ocmax[2];z++) {
							if(ocvlak[b+z+2048] && ocvlak[a+z]) ocwrite(vlr,x,y,z);
						}
					}
				}
			}
			/*
			winset(G.winar[1]);	
			for(x=0;x<32;x++) {
				for(y=0;y<32;y++) {
					color(ocvlak[32*x+y]);
					sboxfs(500+5*x,10+5*y,504+5*x,14+5*y);
					color(ocvlak[32*x+y+1024]);
					sboxfs(700+5*x,10+5*y,704+5*x,14+5*y);
					color(ocvlak[32*x+y+2048]);
					sboxfs(900+5*x,10+5*y,904+5*x,14+5*y);
				}
			}
			while(getbutton(LEFTMOUSE)==0);
			while(getbutton(LEFTMOUSE));
			*/
		}
	}
	/* winset(G.winar[9]); */

}

struct Node *ocread(x,y,z)
long x,y,z;
{
	static long mversch=0,xo=32,yo=32,zo=32;
	struct Branch *br;
	long oc1,versch;

	versch=(xo ^ x) | (yo ^ y) | (zo ^ z);
	if(versch>mversch) {
		xo=x; yo=y; zo=z;
		x<<=2;
		y<<=1;
		
		oc1= ((x & 64)+(y & 32)+(z & 16))>>4;
		br= adroct[0]->b[oc1];
		if(br) {
			oc1= ((x & 32)+(y & 16)+(z & 8))>>3;
			br= br->b[oc1];
			if(br) {
				oc1= ((x & 16)+(y & 8)+(z & 4))>>2;
				br= br->b[oc1];
				if(br) {
					oc1= ((x & 8)+(y & 4)+(z & 2))>>1;
					br= br->b[oc1];
					if(br) {
						mversch=0;
						oc1= ((x & 4)+(y & 2)+(z & 1));
						return (struct Node *)br->b[oc1];
					}
					else mversch=1;
				}
				else mversch=3;
			}
			else mversch=7;
		}
		else mversch=15;
	}
	return 0;
}

short snijpunt2()
{
	struct VertRen *v1,*v2,*v3;
	float x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	float u,v,m0,m1,m2,deeldet,det1,det2,det3,labda;
	
	v1=R.vlr->v1; v2=R.vlr->v2; v3=R.vlr->v3;

	t00= v3->co[0]-v1->co[0];
	t01= v3->co[1]-v1->co[1];
	t02= v3->co[2]-v1->co[2];
	t10= v3->co[0]-v2->co[0];
	t11= v3->co[1]-v2->co[1];
	t12= v3->co[2]-v2->co[2];
	t20= rx1-rx2;
	t21= ry1-ry2;
	t22= rz1-rz2;
	
	x0=t11*t22-t12*t21;
	x1=t12*t20-t10*t22;
	x2=t10*t21-t11*t20;

	deeldet=t00*x0+t01*x1+t02*x2;
	if(deeldet!=0.0) {
		m0=rx1-v3->co[0];
		m1=ry1-v3->co[1];
		m2=rz1-v3->co[2];
		det1=m0*x0+m1*x1+m2*x2;
		u= det1/deeldet;
	/* in de gaten houden: u en v<=0.0 geeft nog spikkels! */
		if(u<0.0) {
			det2=t00*(m1*t22-m2*t21);
			det2+=t01*(m2*t20-m0*t22);
			det2+=t02*(m0*t21-m1*t20);
			v= det2/deeldet;
			if(v<0.0) {
				if(u+v> -1.0) {
					return 1;
				}
			}
		}
	}
	return 0;
}


short snijpunt(vlr)
struct VlakRen *vlr;
{
	struct VertRen *v1,*v2,*v3;
	float x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	float Inpf(),m0,m1,m2,deeldet,det1,det2,det3;
	static short vlrsnij;
	short de=0;

	vlr->colg=raycount;
	
	v1=vlr->v1; v2=vlr->v2; v3=vlr->v3;

	t00= v3->co[0]-v1->co[0];
	t01= v3->co[1]-v1->co[1];
	t02= v3->co[2]-v1->co[2];
	t10= v3->co[0]-v2->co[0];
	t11= v3->co[1]-v2->co[1];
	t12= v3->co[2]-v2->co[2];
	t20= rx1-rx2;
	t21= ry1-ry2;
	t22= rz1-rz2;
	
	x0= t11*t22-t12*t21;
	x1= t12*t20-t10*t22;
	x2= t10*t21-t11*t20;

	deeldet= t00*x0+t01*x1+t02*x2;
	if(deeldet!=0.0) {
		m0=rx1-v3->co[0];
		m1=ry1-v3->co[1];
		m2=rz1-v3->co[2];
		det1=m0*x0+m1*x1+m2*x2;
		rtu= det1/deeldet;
		if(rtu<=0.0) {
			det2=t00*(m1*t22-m2*t21);
			det2+=t01*(m2*t20-m0*t22);
			det2+=t02*(m0*t21-m1*t20);
			rtv= det2/deeldet;
			if(rtv<=0.0) {
				if(rtu+rtv>= -1.0) {
					
					det3=  m0*(t12*t01-t11*t02);
					det3+= m1*(t10*t02-t12*t00);
					det3+= m2*(t11*t00-t10*t01);
					rtlabda= det3/deeldet;
					
					/* det3= m0*vlr->n[0]+m1*vlr->n[1]+m2*vlr->n[2]; */
					/* rtlabda= vlr->len*det3/deeldet; */
					
					if(rtlabda>=0.0 && rtlabda<=1.0) {
						if(R.schaduwvoeler==0) {
							/* parralle ray en vlak ? */
							if(rtlabda>ddalabda) {
								vlr->colg=0;
								return 0;
							}
						}
						/* deeledge controle */	
						if( rtlabda< .1) {
							if(R.vlr==0) return 1;
							if(v1==R.vlr->v1 || v2==R.vlr->v1 || v3==R.vlr->v1) de++;
							if(v1==R.vlr->v2 || v2==R.vlr->v2 || v3==R.vlr->v2) de++;
							if(v1==R.vlr->v3 || v2==R.vlr->v3 || v3==R.vlr->v3) de++;
							if(de) {
								
								if(vlrcontr==R.vlr) return vlrsnij;
								vlrcontr=R.vlr;
								return (vlrsnij=snijpunt2());
							}
							return 1;
						}
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

short snijpuntProj(vlr)
struct VlakRen *vlr;
{
	/* wonderbaarlijk genoeg is deze maar iets sneller, en dan moet
	   de deeledge vergelijk nog! 
	   Waarschijnlijk is de andere veel kritischer
	   en kan er dus eerder uitspringen */
	struct VertRen *v1,*v2,*v3;
	float x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	float deeldet,det1,det2,inp,inp2,len;
	static short vlrsnij;
	short de=0,*rt=0;

	vlr->colg=raycount;

	v1=vlr->v1; v2=vlr->v2; v3=vlr->v3;
	t20= rx2-rx1;
	t21= ry2-ry1;
	t22= rz2-rz1;

	inp= vlr->n[0]*t20+vlr->n[1]*t21+vlr->n[2]*t22;
	if (fabs(inp) < EPSILON) return 0;
	inp2= vlr->n[0]*v1->co[0]+vlr->n[1]*v1->co[1]+vlr->n[2]*v1->co[2];
	det1= vlr->n[0]*rx1+vlr->n[1]*ry1+vlr->n[2]*rz1;

	rtlabda= (inp2-det1)/inp;

	if(rtlabda<0.0 || rtlabda>1.0) return 0;

	if(vlr->snproj==0) {
		t20= v3->co[0]-(rx1+ t20*rtlabda);
		t21= v3->co[1]-(ry1+ t21*rtlabda);
		t00= v3->co[0]-v1->co[0];
		t01= v3->co[1]-v1->co[1];
		t10= v3->co[0]-v2->co[0];
		t11= v3->co[1]-v2->co[1];
	} else if(vlr->snproj==1) {
		t20= v3->co[0]-(rx1+ t20*rtlabda);
		t21= v3->co[2]-(rz1+ t22*rtlabda);
		t00= v3->co[0]-v1->co[0];
		t01= v3->co[2]-v1->co[2];
		t10= v3->co[0]-v2->co[0];
		t11= v3->co[2]-v2->co[2];
	} else {
		t20= v3->co[1]-(ry1+ t21*rtlabda);
		t21= v3->co[2]-(rz1+ t22*rtlabda);
		t00= v3->co[1]-v1->co[1];
		t01= v3->co[2]-v1->co[2];
		t10= v3->co[1]-v2->co[1];
		t11= v3->co[2]-v2->co[2];
	}
	

	deeldet= t00*t11-t01*t10;
	if(fabs(deeldet)<EPSILON) return 0;
	rtu= (t10*t21-t11*t20)/deeldet;
	if(rtu<-1.0 || rtu>0.0) return 0;
	rtv= (t01*t20-t00*t21)/deeldet;
	if(rtv<-1.0 || rtv>0.0) return 0;
	if(rtu+rtv<-1.0) return 0;


	return 1;
}


long vergsnijp(sn1,sn2)
struct snijpsort *sn1,*sn2;
{

	if(sn1->labda > sn2->labda ) return 1;
	else if(sn1->labda < sn2->labda) return -1;
	return 0;
}

short testnode(x,y,z)
long x,y,z;
{
	struct Node *no;
	struct VlakRen *vlr;
	struct snijpsort sn[100];
	short tel=0,snijp;
	
	if(x<0 || y<0 || z<0) return 0;
	if(x>31 || y>31 || z>31) return 0;
	
	no= ocread(x,y,z);
	if(no==0) return 0;
	
	if(R.schaduwvoeler) {
		vlr=no->v[0];
		while(vlr!=0) {
			if(raycount!=vlr->colg) {
				if(snijpunt(vlr)) return 1;
				
			}
			tel++;
			if(tel==7) {
				no=no->next;
				if(no==0) return 0;
				tel=0;
			}
			vlr=no->v[tel];
		}
	}
	/* spiegel en glas  */
	else {
		snijp=0;
		vlr=no->v[0];
		while(vlr!=0) {
			if(raycount!=vlr->colg) {
				if(snijpunt(vlr)) {
					sn[snijp].labda= rtlabda;
					sn[snijp].u= rtu;
					sn[snijp].v= rtv;
					sn[snijp].vlr= vlr;
					snijp++;
				}				
			}
			tel++;
			if(tel==7) {
				no=no->next;
				if(no==0) break;
				tel=0;
			}
			vlr=no->v[tel];
		}
		if(snijp) {
			if(snijp>1) {
				qsort(sn,snijp,sizeof(struct snijpsort),vergsnijp);
				/* printf("snijp %d\n",snijp); */
			}
			rtlabda= sn[0].labda;
			rtu= sn[0].u;
			rtv= sn[0].v;
			snvlr= sn[0].vlr;
			return 1;
		}
	}

	return 0;
}

short d3dda(x2,y2,z2)	/* was vroeger PROC trace */
long x2,y2,z2;
{
	float u1,u2,ox1,ox2,oy1,oy2,oz1,oz2;
	float labda,labdao,labdax,ldx,labday,ldy,labdaz,ldz;
	long dx,dy,dz;	
	long xo,yo,zo,c1=0;
	long ocx1,ocx2,ocy1,ocy2,ocz1,ocz2;
	short snij=0,snijp=0;
	
	/* clip met octree */

	rx1=R.co[0];ry1=R.co[1];rz1=R.co[2];
	rx2=x2;ry2=y2;rz2=z2;
	dx= rx2-rx1;
	u1=0;
	u2=1;
	if(cliptest(-dx,rx1-min[0]-1,&u1,&u2)) {
		if(cliptest(dx,max[0]-rx1+1,&u1,&u2)) {
			dy=ry2-ry1;
			if(cliptest(-dy,ry1-min[1]-1,&u1,&u2)) {
				if(cliptest(dy,max[1]-ry1+1,&u1,&u2)) {
				dz=rz2-rz1;	
					if(cliptest(-dz,rz1-min[2]-1,&u1,&u2)) {
						if(cliptest(dz,max[2]-rz1+1,&u1,&u2)) {
							c1=1;
							if(u2<1.0) {
								rx2= rx1+u2*dx;
								ry2= ry1+u2*dy;
								rz2= rz1+u2*dz;
							}
							if(u1>0.0) {
								rx1+=u1*dx;
								ry1+=u1*dy;
								rz1+=u1*dz;
							}
						}
					}
				}
			}
		}
	}
	if(c1==0) return 0;

	/* de 3dda */
	raycount++;
	if(R.vlr) R.vlr->colg=raycount;
	vlrcontr=0;	/* global om deeledge te testen */

	ox1= (rx1-min[0])*ocfacx;
	oy1= (ry1-min[1])*ocfacy;
	oz1= (rz1-min[2])*ocfacz;
	ox2= (rx2-min[0])*ocfacx;
	oy2= (ry2-min[1])*ocfacy;
	oz2= (rz2-min[2])*ocfacz;
	ocx1= (long)ox1;
	ocy1= (long)oy1;
	ocz1= (long)oz1;
	ocx2= (long)ox2;
	ocy2= (long)oy2;
	ocz2= (long)oz2;

	labda=1.0;
	
	if(ocx1==ocx2 && ocy1==ocy2 && ocz1==ocz2) {
		snij=testnode(ocx1,ocy1,ocz1);
	}
	else {
		/* het berekenen van de labda's en d's */
		if(ox1!=ox2) {
			if(ox2-ox1>0.0) {
				labdax= (ox1-ocx1-1.0)/(ox1-ox2);
				ldx= -1.0/(ox1-ox2);
				dx= 1;
			} else {
				labdax= (ox1-ocx1)/(ox1-ox2);
				ldx= 1.0/(ox1-ox2);
				dx= -1;
			}
		} else {
			labdax=1.0;
			ldx=0;
		}

		if(oy1!=oy2) {
			if(oy2-oy1>0.0) {
				labday= (oy1-ocy1-1.0)/(oy1-oy2);
				ldy= -1.0/(oy1-oy2);
				dy= 1;
			} else {
				labday= (oy1-ocy1)/(oy1-oy2);
				ldy= 1.0/(oy1-oy2);
				dy= -1;
			}
		} else {
			labday=1.0;
			ldy=0;
		}

		if(oz1!=oz2) {
			if(oz2-oz1>0.0) {
				labdaz= (oz1-ocz1-1.0)/(oz1-oz2);
				ldz= -1.0/(oz1-oz2);
				dz= 1;
			} else {
				labdaz= (oz1-ocz1)/(oz1-oz2);
				ldz= 1.0/(oz1-oz2);
				dz= -1;
			}
		} else {
			labdaz=1.0;
			ldz=0;
		}
		
		xo=ocx1; yo=ocy1; zo=ocz1;
		ddalabda= MIN3(labdax,labday,labdaz);
		snij= testnode(xo,yo,zo);
		
		if(snij==0) {
			while(ddalabda<1.0 && snij==0) {
				labdao=ddalabda;

				if(labdax<labday) {
					if(labday<labdaz) {
						xo+=dx;
						labdax+=ldx;
					} else if(labdax<labdaz) {
						xo+=dx;
						labdax+=ldx;
					} else {
						zo+=dz;
						labdaz+=ldz;
						if(labdax==labdaz) {
							xo+=dx;
							labdax+=ldx;
						}
					}
				} else if(labdax<labdaz) {
					yo+=dy;
					labday+=ldy;
					if(labday==labdax) {
						xo+=dx;
						labdax+=ldx;
					}
				} else if(labday<labdaz) {
					yo+=dy;
					labday+=ldy;
				} else if(labday<labdax) {
					zo+=dz;
					labdaz+=ldz;
					if(labdaz==labday) {
						yo+=dy;
						labday+=ldy;
					}
				} else {
					xo+=dx;
					labdax+=ldx;
					yo+=dy;
					labday+=ldy;
					zo+=dz;
					labdaz+=ldz;
				}

				ddalabda=MIN3(labdax,labday,labdaz);
				if(ddalabda==labdao) break;
				snij=testnode(xo,yo,zo);
			}
		}
	}


	return snij;
}		

void shade(vlr)
struct VlakRen *vlr;
{
	struct VertRen *v1,*v2,*v3;
	float l,n1[3],n2[3],n3[3];
	short *o1,*o2,*o3,omklap=0;

	if(vlr==0) return;	

	R.vlr=vlr;
	memcpy(&R.col,vlr->col,sizeof(struct ColBlckF));
	R.vno=vlr->n;
	R.svlako=0;

	R.co[0]= rx1+ rtlabda*(rx2-rx1);
	R.co[1]= ry1+ rtlabda*(ry2-ry1);
	R.co[2]= rz1+ rtlabda*(rz2-rz1);

	l= vlr->n[0]*R.view[0]+vlr->n[1]*R.view[1]+vlr->n[2]*R.view[2];
	if(l<0) {	
		omklap=1;
		vlr->n[0]= -vlr->n[0];
		vlr->n[1]= -vlr->n[1];
		vlr->n[2]= -vlr->n[2];
		vlr->puno= ~(vlr->puno);
	}

	if(R.col.texco & 64) {
		v1=vlr->v1;
		v2=vlr->v2;
		v3=vlr->v3;
		l=1.0+rtu+rtv;
			
		if(R.col.mode & 1) {
			 /* puno's goedzetten */
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

			R.vn[0]= l*n3[0]-rtu*n1[0]-rtv*n2[0];
			R.vn[1]= l*n3[1]-rtu*n1[1]-rtv*n2[1];
			R.vn[2]= l*n3[2]-rtu*n1[2]-rtv*n2[2];
			
			Normalise(R.vn);	
		}
		else {VECCOPY(R.vn,vlr->n);}

		if(R.col.texco & 4) {
			if(v2->v) {	/* is nul bij subdivide */
				o1= v1->v->c;o2= v2->v->c;o3= v3->v->c;
				R.lo[0]= l*o3[0]-rtu*o1[0]-rtv*o2[0];
				R.lo[1]= l*o3[1]-rtu*o1[1]-rtv*o2[1];
				R.lo[2]= l*o3[2]-rtu*o1[2]-rtv*o2[2];
			}
		}
		if(R.col.texco & 16) {
			R.uv[0]= 65535*(rtu+.5);
			R.uv[1]= 65535*(rtv+.5);
		}
		if(R.col.texco & 2) {
			R.orn[0]= 32767*R.vn[0];
			R.orn[1]= -32767*R.vn[1];
			R.orn[2]= 32767*R.vn[2];
				
		}
	}
	else {VECCOPY(R.vn,vlr->n);}

	shadelamplus();

	if(omklap) {
		vlr->n[0]= -vlr->n[0];
		vlr->n[1]= -vlr->n[1];
		vlr->n[2]= -vlr->n[2];
		vlr->puno= ~(vlr->puno);
	}
}

void calcrefvec(ref, view, norm, puno)
float *ref, *view, *norm, *puno;
{
	float i, side;
	
	if(puno) {
		i= -2*(view[0]*puno[0]+ view[1]*puno[1]+ view[2]*puno[2]);

		ref[0]= view[0]+ i*puno[0];
		ref[1]= view[1]+ i*puno[1];
		ref[2]= view[2]+ i*puno[2];
		
		/* test of ref aan dezelfde kant als view zit */
		i= (view[0]*norm[0]+ view[1]*norm[1]+ view[2]*norm[2]);
		side= (ref[0]*norm[0]+ ref[1]*norm[1]+ ref[2]*norm[2]);
		
		if(side<=0.0 && i<=0.0);
		if(side>=0.0 && i>=0.0);
		else {
			i= -2*i;
			ref[0]= view[0]+ i*norm[0];
			ref[1]= view[1]+ i*norm[1];
			ref[2]= view[2]+ i*norm[2];
		}
	}
	else {
		i= -2*(view[0]*norm[0]+ view[1]*norm[1]+ view[2]*norm[2]);
		ref[0]= view[0]+ i*norm[0];
		ref[1]= view[1]+ i*norm[1];
		ref[2]= view[2]+ i*norm[2];
	}
}

void traceray(f, depth, v1, v2, v3, col)
float f;
short depth;
float v1,v2,v3;
ushort *col;
{
	extern ushort shortcol[4];
	float f1,fr,fg,fb;
	long x2,y2,z2;
	char c[4], *charcol;

	if(depth>5) return;

	fr= R.col.mirr;
	fg= R.col.mirg;
	fb= R.col.mirb;

	x2= R.co[0]+tra*v1;
	y2= R.co[1]+tra*v2;
	z2= R.co[2]+tra*v3;

	R.schaduwvoeler= 0;
	if( d3dda(x2,y2,z2) ) {
		R.view[0]=v1;
		R.view[1]=v2;
		R.view[2]=v3;
		Normalise(R.view);

		shade(snvlr);
		f1= 1.0-f;

		col[3]= f*fr*shortcol[3]+ f1*col[3];
		col[2]= f*fg*shortcol[2]+ f1*col[2];
		col[1]= f*fb*shortcol[1]+ f1*col[1];
		
		if(R.col.mir) {
			f1= -2*(R.vn[0]*R.view[0]+R.vn[1]*R.view[1]+R.vn[2]*R.view[2]);
			if(f1> -0.2) f1= -0.2;
			
			v1= (R.view[0]+f1*R.vn[0]);
			v2= (R.view[1]+f1*R.vn[1]);
			v3= (R.view[2]+f1*R.vn[2]);
			f*= R.col.mir;
			
			traceray(f, depth+1, v1, v2, v3, col);
		}
	}
	else {	/* sky */
		if( (wrld.fs & 5)==0 ) {
			f1= 1.0-f;
			f*= 255;
			col[3]= f*fr*wrld.horr+ f1*col[3];
			col[2]= f*fg*wrld.horg+ f1*col[2];
			col[1]= f*fb*wrld.horb+ f1*col[1];
		} else {
			R.view[0]=v1;
			R.view[1]=v2;
			R.view[2]=v3;
			Normalise(R.view);
			sky(c);
			f1= 1.0-f;
			f*= 255;
			col[3]= f*fr*c[3]+ f1*col[3];
			col[2]= f*fg*c[2]+ f1*col[2];
			col[1]= f*fb*c[1]+ f1*col[1];
		}
	}
}








