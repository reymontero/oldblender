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

/*   	view.c         */


/*  
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

	deflighting()
	bindlighting()

	persp(a)

	getmouseco(mdev)	
	initgrabz(x,y,z)
	tekendinges(vec,type)
	berekenschermco(vec,adr)
	berekenschermcofl(vec,adr)

	timestr(time,str)
	countall()
	
	tekenoverdraw(extra)
	tekengrid()
	tekengrond()
	spotvolume(lampvec,viewvec,inp)
	holoview()
	bepaalphitheta()

	setmatriceswork()
	projektie()

	viewmove()
	animfast()
	movedinges(vec,type)
	

******************************************************** */
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "/usr/people/include/Trace.h"

extern void VecMulf(float *v1, float f);
extern short panoramaview,panodegr;

short backbufok= 0;
float obsviewXfac,obsviewYfac;
char versionstr[32]= "Traces Version Develop";

float mat1[4][4]={
	{1,0,0,0},
	{0,1,0,0},
	{0,0,1,0},
	{0,0,0,1}
};


float tex[]= {
	AMBIENT, .1, .1, .1,
	DIFFUSE, .8, .9, 1.0,
	SPECULAR, 0.0, 0.0, 0.0,
	SHININESS, 0.0,
	LMNULL,
};
float lm[]= {
	AMBIENT, .1, .1, .1,
	LOCALVIEWER, 1,
	LMNULL,
};
float lt[]= {
	LCOLOR, 1.0, 1.0, 1.0,
	POSITION, -1.2, -1.0, 1.0, 0.0,
	LMNULL,
};
float blue_light[] =
    {LCOLOR,    0.3,0.3,0.9,   /* blue light */
     POSITION,  0.2,0.1,0.0,0.0,  /* Y axis at infinity */
     LMNULL};

void deflighting()
{
	lmdef(DEFMATERIAL, 1, 0, tex);
	lmdef(DEFLIGHT, 1, 10, lt);
	lmdef(DEFLIGHT, 2, 10, blue_light);
	lmdef(DEFLMODEL, 1, 0, lm);

	lmbind(LIGHT0, 1);
	lmbind(LIGHT1, 2);
	lmbind(LMODEL, 1);
}

void startlighting()
{
	lmbind(MATERIAL, 1);
	lmbind(LIGHT0, 1);
	lmbind(LIGHT1, 2);
	lmbind(LMODEL, 1);
	shademodel(GOURAUD);
}

void stoplighting()
{
	lmbind(MATERIAL, 0);
	shademodel(FLAT);
}

void persp(a)
short a;
{
	static short old_a=1;

	/* hier een push en popmatrix gaat fout.
		   na verandering van winmat in persp(0) en persp(1)
		   is de popmatrix ietsjes anders, GLbug? */

	if(old_a==a) return;  /* dat scheelt weer !? */

	winset(G.winar[0]);
	
	if(a== 0) {
		ortho2(0.0, (float)(S.vxw-1), 0.0, (float)(S.vyw-1));
		loadmatrix(mat1);
	}
	else if(a== 1) {
		mmode(MPROJECTION);
		loadmatrix(G.winmat);
		mmode(MVIEWING);
		loadmatrix(G.viewmat);
	}

	old_a= a;
}


void getmouseco(mval)
short *mval;
{
	static short mdev[2]= {MOUSEX,MOUSEY	};

	getdev(2,mdev,mval);
	mval[1]-= S.vys;
}

float initgrabz(x,y,z)
long x,y,z;
{
	long co[3],z1,d;
	float ho[4];
	float fx,fy,fz,fw;

	return G.persmat[0][3]*(float)x+G.persmat[1][3]*(float)y+G.persmat[2][3]*(float)z+G.persmat[3][3];
}


void berekenschermco(vec, adr)
long *vec;
short *adr;
{
	float fx, fy, vec4[4];

	adr[0]= 3200;
	VECCOPY(vec4, vec);
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	if( vec4[3]>0.0 ) {
		fx= (S.vxw/2)+(S.vxw/2)*vec4[0]/vec4[3];
		if( fx>0 && fx<S.vxw) {
			fy= (S.vyw/2)+(S.vyw/2)*vec4[1]/vec4[3];
			if(fy>0 && fy<S.vyw) {
				adr[0]= fx; 
				adr[1]= fy;
			}
		}
	}
}

void berekenschermcofl(vec, adr)
float *vec;
short *adr;
{
	float fx, fy, vec4[4];

	adr[0]= 3200;
	VECCOPY(vec4, vec);
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	if( vec4[3]>0.1 ) {
		fx= (S.vxw/2)+(S.vxw/2)*vec4[0]/vec4[3];
		if( fx>0 && fx<S.vxw) {
			fy= (S.vyw/2)+(S.vyw/2)*vec4[1]/vec4[3];
			if(fy>0 && fy<S.vyw) {
				adr[0]= fx; 
				adr[1]= fy;
			}
		}
	}
}

void berekenschermcofl_noclip(vec, adr)
float *vec;
short *adr;
{
	float fx, fy, vec4[4];

	adr[0]= 3200;
	VECCOPY(vec4, vec);
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	if( vec4[3]!=0.0 ) {
		fx= (S.vxw/2)+(S.vxw/2)*vec4[0]/vec4[3];
		if( fabs(fx)<30000) {
			fy= (S.vyw/2)+(S.vyw/2)*vec4[1]/vec4[3];
			if(fabs(fy)<30000) {
				adr[0]= fx; 
				adr[1]= fy;
			}
		}
	}

}

void berekenschermco_noclip(vec, adr)
long *vec;
short *adr;
{
	float fx, fy, vec4[4];

	adr[0]= 3200;
	VECCOPY(vec4, vec);
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	if( vec4[3]!=0  ) {
		fx= (S.vxw/2)+(S.vxw/2)*vec4[0]/vec4[3];
		if( fabs(fx)<30000) {
			fy= (S.vyw/2)+(S.vyw/2)*vec4[1]/vec4[3];
			if(fabs(fy)<30000) {
				adr[0]= fx; 
				adr[1]= fy;
			}
		}
	}

}

void tekendinges(vec,type)  /* 1=obs 2=tar */
long *vec;
short type;
{
	float fx,fy,fz,fw,mat[4][4];
	short x,y, sco[2];
	long vert[3];

	VECCOPY(vert,vec);

	if(type<0) color(3); 
	else cpack(0xA0FFFF);

	setlinestyle(1);
	bgnline();
	v3i(vert);
	vert[2]= 0;
	v3i(vert);
	endline();
	setlinestyle(0);

	berekenschermco(vec, sco);
	if(sco[0]==3200) return;
	x= sco[0];
	y= sco[1];

	persp(0);

	switch( abs(type) ) {
	case 1:
		arcs(x,y+2,8,2000,-200);
		arcs(x,y+2,7,2000,-200);
		circfs(x,y,2);
		break;
	case 2:
		linewidth(2);
		bgnline();
		vert[0]=x-3; 
		vert[1]=y-3;
		v2i(vert);
		vert[0]=x+3; 
		vert[1]=y+3;
		v2i(vert);
		endline();
		bgnline();
		vert[0]=x-3; 
		vert[1]=y+3;
		v2i(vert);
		vert[0]=x+3; 
		vert[1]=y-3;
		v2i(vert);
		endline();
		if(G.machine==ENTRY);
		else linewidth(1);
		break;
	}
	persp(1);

}


void timestr(time,str)
long time;
char *str;
{
	/* formaat 00:00:00.00 (hr:min:sec) string moet 12 lang */

	float temp;
	short hr,min,sec,hun;

	temp= ((float)time)/(100.0);
	min= ffloor(temp/60.0);
	hr= min/60;
	min-= 60*hr;
	temp-= (float)60*min;
	sec= ffloor(temp);
	temp-= (float)sec;
	hun= ffloor(100*temp);

	if(hr) sprintf(str,"%.2d:%.2d:%.2d.%.2d",hr,min,sec,hun);
	else sprintf(str,"%.2d:%.2d.%.2d",min,sec,hun);
	str[11]=0;
}

void countall()
{
	extern ListBase editNurb;
	extern ListBase bpbase;
	struct Base *base;
	struct ObData *ob;
	struct VV *vv;
	struct PerfSph *ps;
	struct PolyData *po;
	struct CurveData *cu;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct BodyPoint *bop;
	struct EditVert *eve;
	struct EditVlak *evl;
	long tot,a;

	if(G.ebase) {
		G.totvertsel=G.totvert=0;
		G.totvlaksel=G.totvlak=0;
		if(G.ebase->soort== 1) {
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				G.totvert++;
				if(eve->f & 1) G.totvertsel++;
				eve= eve->next;
			}
			evl= (struct EditVlak *)G.edvl.first;
			while(evl) {
				G.totvlak++;
				if(evl->v1->f & 1)
					if(evl->v2->f & 1)
						if(evl->v3->f & 1)
							G.totvlaksel++;
				evl= evl->next;
			}
		}
		else if ELEM3(G.ebase->soort, 5, 11, -2) {
			nu= editNurb.first;
			while(nu) {
				if((nu->type & 7)==1) {
					bezt= nu->bezt;
					a= nu->pntsu;
					while(a--) {
						G.totvert+=3;
						if(bezt->f1) G.totvertsel++;
						if(bezt->f2) G.totvertsel++;
						if(bezt->f3) G.totvertsel++;
						bezt++;
					}
				}
				else {
					bp= nu->bp;
					a= nu->pntsu*nu->pntsv;
					while(a--) {
						G.totvert++;
						if(bp->f1 & 1) G.totvertsel++;
						bp++;
					}
				}
				nu= nu->next;
			}
		}
		else if(G.ebase->soort== -4) {
			bop= bpbase.first;
			while(bop) {
				G.totvert++;
				if(bop->f & 1) G.totvertsel++;
				bop= bop->next;
			}
		}

		return;
	}

	G.totvert= G.totvertsel= G.totvlaksel= G.totvlak= G.totobj= 
	    G.totlamp= G.totpoly= G.totmove= 0;

	base= G.firstbase;
	while(base) {
		if(view0.lay & base->lay) {
			switch(base->soort) {
			case 1:	/* object */
				ob=(struct ObData *)base->d;
				vv=ob->vv;
				G.totobj++;
				G.totvert+= vv->vert;
				G.totvlak+= vv->vlak;
				if(base->f & 1) {
					G.totvertsel+= vv->vert;
					G.totvlaksel+= vv->vlak;
				}
				break;
			case 2: /* lamp */
				G.totlamp++;
				break;
			case 3: /* sphere */
				ps=(struct PerfSph *)base->d;
				G.totobj++;
				G.totvert+= 2*20*20-20;
				G.totvlak+= 4*20*20;
				if(base->f & 1) {
					G.totvertsel+= 2*20*20-20;
					G.totvlaksel+= 4*20*20;
				}
				break;
			case 4: /* camera */
				G.totobj++; /* voor backbufselectie */
				break;
			case 5:	/* nurbs */
				G.totobj++;
				ob=(struct ObData *)base->d;
				tot= 0;
				nu= ob->cu->curve.first;
				while(nu) {
					tot+= nu->resolu*nu->resolv;
					nu= nu->next;
				}
				G.totvert+= tot;
				G.totvlak+= 2*tot;
				if(base->f & 1) {
					G.totvertsel+= tot;
					G.totvlaksel+= 2*tot;
				}
				break;
			case 7: 
			case 9:
				ob=(struct ObData *)base->d;
				po= ob->po;
				G.totpoly++;
				tot= po->vert;
				if(po->f45p!=0 || po->f45m!=0) tot*=2;
				if(po->ext) tot*=2;
				G.totvert+= tot;
				if(tot> po->vert) {
					G.totvlak+= tot;
					if(base->f & 1) G.totvlaksel+= tot;
				}
				if(base->f & 1) G.totvertsel+= tot;

				break;
			case 11:
				G.totpoly++;
				ob=(struct ObData *)base->d;
				tot= 0;
				nu= ob->cu->curve.first;
				while(nu) {
					tot+= nu->pntsu*nu->resolu;
					nu= nu->next;
				}
				G.totvert+= tot;
				G.totvlak+= tot;
				if(base->f & 1) {
					G.totvertsel+= tot;
					G.totvlaksel+= tot;
				}
				break;
			case -2:
			case -4:
				G.totmove++;
				break;
			}
		}
		base= base->next;
	}
}


void tekenoverdraw(extra)
char *extra;
{
	extern long mem_in_use;
	static short oldcursx=3200,oldcursy,len=0;
	short a,hr,min;
	char str[200],tstr[32];

	if(G.winar[0]==0) return;
	if(G.font==0) return;
	if(G.moving) return;
	
	winset(G.winar[0]);

	persp(0);

	drawmode(OVERDRAW);

	if(extra==0) {
		if(oldcursx!=3200) {
			color(0);
			sboxfs(oldcursx-21,oldcursy-21,oldcursx+21,oldcursy+21);
		}
		oldcursx= view0.mx;
		oldcursy= view0.my;

		if( (G.f & 8)==0 && view0.mx!=3200) {
			if(G.machine==ENTRY) linewidth(1);

			setlinestyle(1); 
			color(2);
			circs(view0.mx,view0.my,10);
			setlinestyle(2); 
			color(3);
			circs(view0.mx,view0.my,10);
			setlinestyle(0);
			color(1);

			sboxs(view0.mx-20,view0.my,view0.mx-5,view0.my);
			sboxs(view0.mx+5,view0.my,view0.mx+20,view0.my);
			sboxs(view0.mx,view0.my-20,view0.mx,view0.my-5);
			sboxs(view0.mx,view0.my+5,view0.mx,view0.my+20);

			if(G.machine==ENTRY) linewidth(2);
		}
	}

	fullscrn();
	if(extra>(char *)1) {
		sprintf(str,"Traces %s Fra:%d   Ve:%d-%d Fa:%d-%d Ob:%d Po:%d La:%d Mo:%d Mem:%dk \
			%s \n",
			versionstr+14, G.cfra,G.totvertsel,G.totvert,G.totvlaksel,G.totvlak,
			G.totobj,G.totpoly,G.totlamp,G.totmove,(mem_in_use>>10),
			extra);
	}
	else {
		timestr(G.cputime,tstr);
		sprintf(str,"Traces %s Fra:%d   Ve:%d-%d Fa:%d-%d Ob:%d Po:%d La:%d Mo:%d Mem:%dk\
			Time:%s (%.2f) \n",
			versionstr+14, G.cfra,G.totvertsel,G.totvert,G.totvlaksel,G.totvlak,
			G.totobj,G.totpoly,G.totlamp,G.totmove,(mem_in_use>>10),
			tstr,((float)(G.time-G.cputime))/100);
	}

	color(0);
	
	sboxfs(0, S.y-23, S.x, S.y);
	if(versionstr[15]=='D') {
		color(2);
		sboxfs(3, S.y-16, 140, S.y-1);
	}
	color(1);
	cmov2i(12, S.y-14);
	charstr(str);
	color(3);
	cmov2i(10, S.y-12);
	charstr(str);

	endfullscrn();

	drawmode(NORMALDRAW);
	persp(1);
}

void tekenpixelgrid()
{
	float a,fac, zoom,dx2,dy2,dx,dy,startx,starty,endx,endy;
	short x,y,xsch,ysch;

	xsch= R.xsch;
	ysch= R.ysch;
	xsch-= (R.size*R.xsch)/4;
	ysch-= (R.size*R.ysch)/4;

	zoom= 0.5+0.5*(float)(2<<view0.rt1);

	fac= ((float)ysch*R.yasp)/((float)xsch*R.xasp);
	if(fac>1.0) {
		fac= 1.0/fac;
		fac*= zoom;
		fac*= (S.vxw/4);
		startx= (S.vxw/2)- fac -zoom*view0.rt2;
		endx= (S.vxw/2)+ fac -zoom*view0.rt2;
		starty= (S.vyw/2)-zoom*((S.vxw/4)+view0.rt3);
		endy= (S.vyw/2)+zoom*((S.vxw/4)-view0.rt3);

		dx= 2*fac/(float)xsch;
		dy= zoom*(S.vxw/2)/(float)ysch;
	}
	else {
		fac*= zoom;
		fac*=(S.vxw/4);
		startx= (S.vxw/2)-zoom*((S.vxw/4)+view0.rt2);
		endx= (S.vxw/2)+zoom*((S.vxw/4)-view0.rt2);
		starty= (S.vyw/2)-fac -zoom*view0.rt3;
		endy= (S.vyw/2)+fac -zoom*view0.rt3;

		dx= zoom*(S.vxw/2)/(float)xsch;
		dy= 2*fac/(float)ysch;
	}
	dx2= 2*dx;
	dy2= 2*dy;

	obsviewXfac= dx;	/* deze getallen worden gebruikt om de afmeting van pixels te berekenen */
	obsviewYfac= dy;	

	cpack(0x636363);
	if(dx<3.5 || dy<3.5) {
		sboxf(startx,starty,endx,endy);
	}
	else {
		a= startx ;/*onthoud*/
		for(x=0;x<xsch;x+=2) {
			if(startx>=0)
				sbox(startx,starty,startx+dx,endy);
			startx+=dx2;
			if(startx>S.x) break;
		}

		startx= a;
		a= starty;
		for(y=0;y<ysch;y+=2) {
			if(starty>=0)
				sbox(startx,starty,endx,starty+dy);
			starty+=dy2;
			if(starty>S.y) break;
		}
		starty= a;
	}
	/* safety */
	cpack(0);
	sbox(startx,starty,endx,endy);
	fac= R.safety*.001*(endx-startx);
	startx+=fac;
	endx-=fac;
	fac= R.safety*.001*(endy-starty);
	starty+=fac;
	endy-=fac;
	cpack(0xFFFFFF);
	sbox(startx,starty,endx,endy);
}

void tekengrid()
{
	extern short bgpicmode;
	float x,y,dx,fx,fy,fz,fw, vec4[4];

	vec4[0]=vec4[1]=vec4[2]=0.0; 
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	x= (S.vxw/2)*fx/fw;
	y= (S.vyw/2)*fy/fw;

	vec4[0]=vec4[1]=view0.grid; 
	vec4[2]= 0.0;
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	dx=fabs(x-(S.vxw/2)*fx/fw);
	if(dx==0) dx=fabs(y-(S.vyw/2)*fy/fw);

	if(dx<5.0) return;

	persp(0);

	x+= (S.vxw/2); 
	y+= (S.vyw/2);
	cpack(0xA0C0C0);
	sbox(x,0,x,S.vyw);
	sbox(0,y,S.vxw,y);
	
	if(G.machine==ENTRY) cpack(0x505050);
	else cpack(0x505050);
	
	fx=x;
	while(fx>0) {
		fx-=dx; 
		if(fx<S.vxw) sbox(fx, 0.0, fx, (float)S.vyw);
	}
	fx=x;
	while(fx< S.vxw) {
		fx+=dx; 
		if(fx>0) sbox(fx, 0.0, fx, (float)S.vyw);
	}
	fy= y;
	while(fy>0) {
		fy-=dx; 
		if(fy<800) sbox(0.0, fy, (float)S.vxw, fy);
	}
	fy=y;
	while(fy< S.vyw) {
		fy+=dx; 
		if(fy>0) sbox(0.0, fy, (float)S.vxw, fy);
	}

	persp(1);
}

void tekengrond()
{
	float fx,fy,fz,fw,x1,y1,d;
	long vert[3],grid;
	short a,b;
	
	if(view0.persp==3) {
		persp(0);
		tekenpixelgrid();
		persp(1);
	}

	cpack(0x404040);
	vert[2]=0;
	grid=10*view0.grid;


	for(a= -10;a<=10;a++) {
		if(a==0) {
			if(view0.persp==0) cpack(0xA0C0C0);
			else cpack(0x402000);
		}
		if(a==1) {
			if(G.machine==ENTRY) cpack(0x405040);
			else cpack(0x404040);
		}
		bgnline();
		vert[0]=a*view0.grid;
		vert[1]=grid;
		v3i(vert);
		vert[1]= -grid;
		v3i(vert);
		endline();
	}
	for(a= -10;a<=10;a++) {
		if(a==0) {
			if(view0.persp==0) cpack(0xA0C0C0);
			else cpack(0);
		}
		if(a==1) {
			if(G.machine==ENTRY) cpack(0x405040);
			else cpack(0x404040);
		}
		bgnline();
		vert[1]=a*view0.grid;
		vert[0]=grid;
		v3i(vert);
		vert[0]= -grid;
		v3i(vert);
		endline();
	}

	if (view0.persp == 2 && wrld.fs & 16) make_stars(1); 

}

void spotvolume(lvec,vvec,inp)
float *lvec,*vvec,inp;
{
	/* camera staat op 0,0,0 */
	float temp[3],plane[3],mat1[3][3],mat2[3][3],mat3[3][3],mat4[3][3],q[4],co,si,hoek,*rt;

	Normalise(lvec);
	Normalise(vvec);				/* is dit de goede vector ? */

	Crossf(temp,vvec,lvec);		/* vergelijking van vlak door vvec en lvec */
	Crossf(plane,lvec,temp);		/* en dan het vlak loodrecht daarop en evenwijdig aan lvec */

	Normalise(plane);

	/* nu hebben we twee vergelijkingen: die van de kegel en die van het vlak, maar we hebben
	drie onbekenden We halen nu een onbekende weg door het vlak naar z=0 te roteren */
	/* Ik heb geen flauw idee of dat mag, we moeten tenslotte twee oplossingen krijgen, maar we
	proberen het gewoon: vlak vector moet (0,0,1) worden*/

	/* roteer om uitproduct vector van (0,0,1) en vlakvector, inproduct graden */
	/* volgens defenitie volgt dat uitproduct is (plane[1],-plane[0],0), en cos() = plane[2]);*/

	q[1] = plane[1] ; 
	q[2] = -plane[0] ; 
	q[3] = 0 ;
	Normalise(&q[1]);

	hoek = facos(plane[2])/2.0;
	co = fcos(hoek);
	si = fsqrt(1-co*co);

	q[0] =  co;
	q[1] *= si;
	q[2] *= si;
	q[3] =  0;

	QuatToMat3(q,mat1);

	/* lampvector nu over acos(inp) graden roteren */

	vvec[0] = lvec[0] ; 
	vvec[1] = lvec[1] ; 
	vvec[2] = lvec[2] ;

	Mat3One(mat2);
	co = inp;
	si = fsqrt(1-inp*inp);

	mat2[0][0] =  co;
	mat2[1][0] = -si;
	mat2[0][1] =  si;
	mat2[1][1] =  co;
	Mat3MulMat3(mat3,mat2,mat1);

	mat2[1][0] =  si;
	mat2[0][1] = -si;
	Mat3MulMat3(mat4,mat2,mat1);
	Mat3Transp(mat1);

	Mat3MulMat3(mat2,mat1,mat3);
	Mat3MulVecfl(mat2,lvec);
	Mat3MulMat3(mat2,mat1,mat4);
	Mat3MulVecfl(mat2,vvec);

	return;
}

void panorview()
{
	float hoek,fx,fy,si,co,deler;
	long *t1,*t;

	t= view0.obs;
	t1= view0.tar;
	VECCOPY(t1,t);

	if(R.xsch<2 || R.ysch<2) {
		error("Image too small");
		return;
	}
	if(R.xasp*R.xsch>=R.yasp*R.ysch) {
		deler= ((float)R.xsch*view0.lens)/32.0;
	}
	else {
		deler= ((float)R.ysch*view0.lens)/32.0;
	}
	hoek= ((float)R.xsch)/deler;
	hoek= 2.0*fatan(hoek/2.0);

	fx= G.efra-G.sfra;
	fx= -fx*hoek/2;
	hoek= fx+ ((float)panodegr)*PI/180.0+hoek*(G.cfra-G.sfra);

	si= fsin(hoek);
	co= fcos(hoek);
	fx= 10*view0.grid*si;
	fy= 10*view0.grid*co;
	t1[0]+= fx;
	t1[1]+= fy;

}

void holoview()
{
	float dist,fac,fy,fz;
	long *t1,*t;

	t= view0.obs;
	t1= view0.tar;
	fy= t[1]-t1[1];
	fz= t[2]-t1[2];
	dist= fsqrt( fy*fy+ fz*fz );
	dist*= ((float)R.afmx)/(float)R.d;

	dist*= ( (float)G.holopadlen )/100.0;

	fac= (G.cfra-G.sfra)/((float)(G.efra-G.sfra));

	t[0]= t1[0]= -dist+ 2*fac*dist;
}


float bepaalphitheta()  /* geeft afstand obs-tar terug */
{
	float hoek,fx,fy,fz,dist,mat[4][4],omat[3][3];
	long *t,*t1,temp;
	short teller=0;

	if(view0.opar) {
		VECCOPY(G.workbase->v,view0.obs);
		G.workbase->sf= view0.opar->sf;
		G.workbase->p= view0.opar;
		parentbase(G.workbase,mat,omat);
		VECCOPY(view0.obs,G.workbase->r);
	}
	if(view0.tpar) {
		VECCOPY(G.workbase->v,view0.tar);
		G.workbase->sf= view0.tpar->sf;
		G.workbase->p= view0.tpar;
		parentbase(G.workbase,mat,omat);
		VECCOPY(view0.tar,G.workbase->r);
	}

	t= view0.obs;
	t1= view0.tar;

	fx=t[0]-t1[0]; 
	fy=t[1]-t1[1]; 
	fz=t[2]-t1[2];
	dist=fsqrt(fx*fx+fy*fy+fz*fz);

	view0.theta = fatan2(fy,fx);
	view0.phi = facos(fz / dist);

	view0.theta+=.5*PI;  /* de afwijking tov amigatraces */

	fz /= dist;
	fz *=fz;
	fz *=fz;
	fz *=fz;
	fz *=fz;
	fz *=fz;

	view0.theta *=  (1.0 - fz);

	return dist;
}

/* ************************************************ */
short proj_in_queue()
{
	short event,event1,val;

	event=  traces_qread(&val);

	if(val==0) {
		event1= qtest();
		if(event1!=event) return 0;

	}
	qenter(event,val);

	switch(event) {
	case PAD0: 
	case PAD1: 
	case PAD2: 
	case PAD3: 
	case PAD4: 
	case PAD5:
	case PAD6: 
	case PAD7: 
	case PAD8: 
	case PAD9: 
	case PADMINUS:
	case PADPLUSKEY: 
	case PADENTER:
	case LEFTARROWKEY: 
	case DOWNARROWKEY:
	case RIGHTARROWKEY: 
	case UPARROWKEY:
		return 1;
		break;
	}

	return 0;

}

void setwinmatrixwork(rect)		/* rect: voor picking */
rctf *rect;
{
	float zoom, d, near, far;
	float lens, dfac, tfac, fac, x1, y1, x2, y2;
	short orth;
	
	lens= view0.lens;
	near= view0.near;
	far = 1000.0*view0.far;
	
	if(view0.persp>=2 && view0.cambase) {
		if(view0.cambase->f2 & 64) {
			lens*= 100.0;
			near*= 100.0;
			far*= 100.0;
		}
	
		if(view0.cambase->soort==2) {
			struct LaData *la;
			
			la= (struct LaData *)view0.cambase->d;
			fac= fcos( PI*((float)(256- la->spsi))/512.0 );
			
			x1= facos(fac);
			lens= 16.0*fac/fsin(x1);
	
			near= 1000*la->clipsta;
			far= 1000*la->clipend;
		}
	}
	
	d= 0.015625*S.vxw*lens;
	R.d= 2*d;	/* R.d en R.afmx nodig voor holo mode */
	dfac= near/d;
	R.afmx= (S.vxw/2);
	if(view0.persp==1 && view0.dproj>1.0) far= view0.dproj*far;

	if(view0.persp==0) {
		x1= -32000*view0.dproj;
		x2= 32000*view0.dproj;
		y1= -20000*view0.dproj;
		y2= 20000*view0.dproj;
		orth= 1;
	}
	else if(view0.persp==3) {
		zoom= 0.5+0.5*(float)(2<<view0.rt1);
		x1= dfac*(-(S.vxw/2)/zoom +view0.rt2);
		x2= dfac*((S.vxw/2)/zoom +view0.rt2);
		y1= dfac*(-(S.vyw/2)/zoom +view0.rt3);
		y2= dfac*((S.vyw/2)/zoom +view0.rt3);
		orth= 0;
	}
	else {
		if(view0.persp==2) fac= (2.0+( (float)G.obszoom )/50.0);
		else fac= 2.0;
		x1= -dfac*(S.vxw/fac);
		x2= dfac*(S.vxw/fac);
		y1= -dfac*(S.vyw/fac);
		y2= dfac*(S.vyw/fac);
		
		if(view0.persp==2 && G.holo==3) {
			tfac= fac/4.0;	/* de fac is 1280/640 gecorr voor obszoom */
			fac= (G.cfra-G.sfra)/((float)(G.efra-G.sfra))-0.5;
			
			fac*= tfac*(x2-x1);
			fac*= ( (float)G.holopadlen )/100.0;
			x1-= fac;
			x2-= fac;
		}
		if(view0.cambase->f2 & 64) {
			/* x1/= 10.0; */
			/* y1/= 10.0; */
			/* x2/= 10.0; */
			/* y2/= 10.0; */
		}
		orth= 0;
	}

	if(rect) {		/* picking */
		rect->xmin/= (float)S.vxw;
		rect->xmin= x1+rect->xmin*(x2-x1);
		rect->ymin/= (float)S.vyw;
		rect->ymin= y1+rect->ymin*(y2-y1);
		rect->xmax/= (float)S.vxw;
		rect->xmax= x1+rect->xmax*(x2-x1);
		rect->ymax/= (float)S.vyw;
		rect->ymax= y1+rect->ymax*(y2-y1);

		if(orth) ortho(rect->xmin, rect->xmax, rect->ymin, rect->ymax, -far, far);
		else window(rect->xmin, rect->xmax, rect->ymin, rect->ymax, d*dfac, far);

	}
	else {
		if(orth) ortho(x1, x2, y1, y2, -far, far);
		else window(x1, x2, y1, y2, d*dfac, far);
	}
}

void setmatriceswork()
{
	float tempf, bmat[4][4], cmat[4][4];
	float view[3];
	float twist, bepaalphitheta();

	winset(G.winar[0]);

	setwinmatrixwork(0);

	if(G.holo) holoview();
	else if(panoramaview) panorview();

	if(view0.persp>=2) {	    /* obs/camera */
		if(view0.cambase) {
			parentbase(view0.cambase, bmat, cmat);


			VECCOPY(view0.obs, bmat[3]);	/* moet omdat bepaalphitheta hiermee rekent */
			view0.tar[0]= bmat[3][0]-10*view0.grid*bmat[2][0];
			view0.tar[1]= bmat[3][1]-10*view0.grid*bmat[2][1];
			view0.tar[2]= bmat[3][2]-10*view0.grid*bmat[2][2];

			Mat4Ortho(bmat);
			Mat4Invert(G.viewmat, bmat);

			/* Mat4InvGG(G.viewmat, bmat); */
								/* ONDRZOEKEN, WAAROM DIE ANDERE INVERT 
								NIET WERKT: weet je nog: camera adden in topview,
								180 graden draaien, zijview, 90 graden draaien,
								iets omhoog zetten */
			
			if(view0.cambase->f2 & 64) {
				G.viewmat[3][2]*= 100.0;
			}

			bepaalphitheta();
		}
		else {
			tempf=bepaalphitheta();
			i_polarview(tempf, (180/PI)*view0.theta, (180/PI)*view0.phi, 0.0, G.viewmat);
			i_translate(-(float)view0.tar[0],-(float)view0.tar[1],-(float)view0.tar[2], G.viewmat);
		}
	}
	else {
		if(view0.cambase && G.obslimits) {
			parentbase(view0.cambase, bmat, cmat);

			VECCOPY(view0.obs, bmat[3]);
			view0.tar[0]= bmat[3][0]-10*view0.grid*bmat[2][0];
			view0.tar[1]= bmat[3][1]-10*view0.grid*bmat[2][1];
			view0.tar[2]= bmat[3][2]-10*view0.grid*bmat[2][2];
		}
		if(view0.persp==0) {
			Mat4One(G.viewmat);

			i_rotate((-180/PI)*view0.phi, 'x', G.viewmat); 
			i_rotate((-180/PI)*view0.theta, 'z', G.viewmat);
			i_translate((float)view0.ofs[0], (float)view0.ofs[1], (float)view0.ofs[2], G.viewmat);
		}
		else {
			i_polarview(view0.dproj*30000, (180/PI)*view0.theta, (180/PI)*view0.phi, 0.0, G.viewmat);
			i_translate((float)view0.ofs[0], (float)view0.ofs[1], (float)view0.ofs[2], G.viewmat);
		}
	}
	
	loadmatrix(G.viewmat);

	mmode(MPROJECTION);
	getmatrix(G.winmat);
	Mat4MulMat4(G.persmat,G.viewmat,G.winmat);
	mmode(MVIEWING);

	Mat4Inv(G.persinv, G.persmat);

}

short selectprojektie(buffer, x1, y1, x2, y2)
short *buffer, x1, y1, x2, y2;
{
	rctf rect;
	struct Base *base;
	short mval[2], code, hits;
	
	G.f |= 16;
	winset(G.winar[0]);
	
	if(x1+x2+y1+y2==0) {
		getmouseco(mval);
		rect.xmin= mval[0]-7;
		rect.xmax= mval[0]+7;
		rect.ymin= mval[1]-7;
		rect.ymax= mval[1]+7;
	}
	else {
		rect.xmin= x1;
		rect.xmax= x2;
		rect.ymin= y1;
		rect.ymax= y2;
	}
	setwinmatrixwork(&rect);
	
	gselect(buffer, 2000);
	
	code= 1;
	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			base->selcol= code;
			loadname(code);
			tekenbase(base,1);
			code++;
		}
		base= base->next;
	}
	
	hits= endselect(buffer);
	if(hits<0) error("Too many objects in selectbuf");
	setmatriceswork();
	
	G.f &= ~16;
	
	return hits;
}

void backbufprojektie(test)
short test;
{
	struct Base *base;
	long tel=1;
	ulong kleur;

	if(G.machine==ENTRY) return;

	winset(G.winar[0]);
	frontbuffer(0);
	backbuffer(1);

	if(G.zbuf) {
		czclear(0,0x7FFFFF);
		zbuffer(1);
	}
	else {
		cpack(0); 
		clear();
		zbuffer(0);
	}
	G.f |= 16;
	backbufok= 0;
	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			kleur= ((tel & 0xF00)<<12) + ((tel & 0xF0)<<8) + ((tel & 0xF)<<4);
			base->selcol= (long)kleur;
			tekenbase(base,kleur);
			if(test) {
				if(qtest()) {
					G.f &= ~16;
					return;
				}
			}
			tel++;
		}
		base= base->next;
	}
	backbufok= 1;
	G.f &= ~16;
}

void projektie()
{
	extern short bgpicmode;
	struct Base *base,*duplibase();
	float bmat[4][4],cmat[3][3],fac;
	short a,x1,x2,y1,y2;
	char s[20];

	winset(G.winar[0]);
		frontbuffer(0);
	
	if( qtest() ) {
		if( proj_in_queue() ) return;
	}
	/* zbufferen: 
		- hierboven czclear
		- tekenbase lmbind lamp, tekenbase_ext zclear
		- ook in de tekenoverdraw kijken 
	*/
	backbufok= 0;

	if(G.zbuf) {
		zbuffer(1);
		zfunction(ZF_LEQUAL);
		if(G.machine==ENTRY) czclear(0x708070,0x7FFFFFFF);
		else czclear(0x707070, 0x7FFFFF);
		freefastshade();
	}
	else {
		if(G.machine==ENTRY) cpack(0x708070);
		else cpack(0x707070);
		clear();
		zbuffer(0);
	}
	setmatriceswork();

	if(view0.view==0 || view0.persp!=0) tekengrond();
	else {
		if(bgpicmode & 1) drawBGpic();
		tekengrid();
	}

	if (view0.persp==2 && (bgpicmode & 1)) drawBGpic();
	
	base=G.firstbase;

	if(G.f & 4) if(base) {
		base= duplibase();
		while(base) {
			tekenbase(base,0);
			base= base->next;
		}
		base=G.firstbase;
	}
	while(base) {
		if(base->f & 1); 
		else tekenbase(base,0);
		base= base->next;
	}
	base=G.firstbase;
	while(base) {
		if(base->f & 1) tekenbase(base,0);
		base= base->next;
	}

	if(view0.cambase==0 && G.localview==0) {
		if(view0.persp<2) {
			if(view0.opar) {
				VECCOPY(G.workbase->v,view0.obs);
				G.workbase->sf= view0.opar->sf;
				G.workbase->p= view0.opar;
				parentbase(G.workbase,bmat,cmat);
				VECCOPY(view0.obs,G.workbase->r);
			}
			if(view0.tpar) {
				VECCOPY(G.workbase->v,view0.tar);
				G.workbase->sf= view0.tpar->sf;
				G.workbase->p= view0.tpar;
				parentbase(G.workbase,bmat,cmat);
				VECCOPY(view0.tar,G.workbase->r);
			}
			tekendinges(view0.obs,1);
		}
		tekendinges(view0.tar,2);
	}
	berekenschermco(view0.muis,&view0.mx);

	if(view0.persp<2 && G.obslimits && G.localview==0) tekenobslimits();


	if(G.zbuf) zbuffer(0);

	/* viewborder geen overdraw meer, is mooier */
	if(view0.persp==2) {
		persp(0);
		
		fac= ((float)R.ysch*R.yasp)/((float)R.xsch*R.xasp);
		
		if(fac>1.0) {
			fac= 1.0/fac;
			fac*= (S.vxw/4);
			obsviewXfac= 2*fac/(float)R.xsch;
			obsviewYfac= (S.vxw/2)/(float)R.ysch;
			x1= (S.vxw/2)-fac;
			x2= (S.vxw/2)+fac;
			y1= (S.vyw/2)-(S.vxw/4); 
			y2= (S.vyw/2)+(S.vxw/4);
		}
		else {
			fac*= (S.vxw/4);
			obsviewXfac= (S.vxw/2)/(float)R.xsch;
			obsviewYfac= 2*fac/(float)R.ysch;
			x1= (S.vxw/4); 
			x2= (3*S.vxw/4);
			y1= (S.vyw/2)-fac;
			y2= (S.vyw/2)+fac;
		}
		fac= (4-R.size)*0.25;
		obsviewXfac/=fac;
		obsviewYfac/=fac;

		obsviewXfac*= (1.0+ ( (float)G.obszoom )/100.0 );
		obsviewYfac*= (1.0+ ( (float)G.obszoom )/100.0 );

		/* zoom van viewborder */
		fac= ( (float)G.obszoom )/200.0;
		a= fac*(x2-x1);
		x1-= a; x2+= a;
		a= fac*(y2-y1);
		y1-= a; y2+= a;

		setlinestyle(1);
		cpack(0);
		sboxs(x1+1,y1-1,x2+1,y2-1);
		cpack(0xFFFFFF);
		sboxs(x1,y1,x2,y2);
		
		/* safetykader */
		fac= R.safety*.001;
		a= fac*(x2-x1);
		x1+= a; 
		x2-= a;
		a= fac*(y2-y1);
		y1+= a; 
		y2-= a;
		cpack(0);
		sboxs(x1+1,y1-1,x2+1,y2-1);
		cpack(0xFFFFFF);
		sboxs(x1,y1,x2,y2);
		setlinestyle(0);
		
		/* border */
		if(G.border) {
			/* dit moet scherm-onafhankelijk geschreven worden, incl obszoom! */
			cpack(0);
			sboxs(G.xminb+1,G.yminb-1,G.xmaxb+1,G.ymaxb-1);
			cpack(0x4040FF);
			sboxs(G.xminb,G.yminb,G.xmaxb,G.ymaxb);
		}
	}
	
	if(G.f & 128) {	/* ipodraw */
		struct Key *key;
		
		give_curkey(&key);
		persp(0);
		cmat[0][1]= 0;
		cmat[1][1]= S.vyw;
		a= 1;
		while(key) {
			if(a==G.actkey) cpack(0xFFFF00);
			else cpack(0xFF00FF);
			cmat[0][0]= cmat[1][0]= S.vxw*key->pos;
			bgnline();
				v2f(cmat[0]); v2f(cmat[1]);
			endline();
			a++;
			key= key->next;
		}
	}
	
	if(G.hie!=2) {
		tekenoverdraw(0);
	}

	persp(1); /* wordt alleen uitgevoerd als laatste persp(0) was */
	
	swapbuffers();
}


void viewmove()
{
	float z,dx=0, dy=0, zoom;
	Device mdev[2];
	short mval[2],xo,yo,mode=0;

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	z= initgrabz(0,0,0);

	getdev(2, mdev, mval);
	xo=mval[0]; 
	yo=mval[1];

	if(getbutton(LEFTSHIFTKEY)) mode= 1;
	else if(getbutton(RIGHTSHIFTKEY)) mode= 1;
	else if(getbutton(LEFTCTRLKEY)) mode= 2;
	else if(getbutton(RIGHTCTRLKEY)) mode= 2;
	else view0.view=0; /* bij viewroteer klein grid */

	dx= view0.rt2;	/* als persp==3: met floats rekenen werkt beter bij kleine getallen */
	dy= view0.rt3;

	while(getbutton(MIDDLEMOUSE)){
		getdev(2, mdev, mval);
		if(view0.persp==2 || (view0.persp==3 && mode!=1) ) {
			view0.persp=1;

			view0.dproj= bepaalphitheta()/30000.0;
			view0.ofs[0]= -view0.tar[0];
			view0.ofs[1]= -view0.tar[1];
			view0.ofs[2]= -view0.tar[2];
		}
		if(xo!=mval[0] || yo!=mval[1] || G.hie==2) {
			if(mode==0) {
				view0.phi-= (float)(yo-mval[1])/300.0;
				view0.theta+= (float)(xo-mval[0])/300.0;
			} else if(mode==1) {
				if(view0.persp==3) {
					zoom= 0.5+0.5*(float)(2<<view0.rt1);
					dx-= (mval[0]-xo)/zoom;
					dy-= (mval[1]-yo)/zoom;
					view0.rt2= dx;
					view0.rt3= dy;
					if(view0.rt2<-320) view0.rt2= -320;
					if(view0.rt2> 320) view0.rt2=  320;
					if(view0.rt3<-250) view0.rt3= -250;
					if(view0.rt3> 250) view0.rt3=  250;
				}
				else {
					dx= 2.0*(mval[0]-xo)*z/S.vxw;
					dy= 2.0*(mval[1]-yo)*z/S.vyw;
					view0.ofs[0]+= (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
					view0.ofs[1]+= (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
					view0.ofs[2]+= (G.persinv[0][2]*dx + G.persinv[1][2]*dy);
				}
			} else if(mode==2) {
				view0.dproj*= 1.0+(float)(yo-mval[1])/1000.0;
				if(view0.dproj<.0001) view0.dproj= .0001;
				if(view0.dproj>500) view0.dproj=500;
				mval[1]= yo; /* blijft ie zoomen */
			}
			if(G.hie==2) {
				G.cfra++;
				if(G.cfra==G.efra+1) G.cfra=G.sfra;
				loadkeyposall();
				SetButs(103,103);
			}
			projektie();
			xo=mval[0]; 
			yo=mval[1];
		}
	}
}

void animfast()
{
	long tijd;
	short temp1,temp2,toets,val;

	temp1=G.cfra; 
	temp2=G.hie;
	G.hie=2;
	swapinterval(2);
	tijd= clock();
	for(G.cfra=G.sfra;G.cfra<=G.efra;G.cfra++) {
		SetButs(103,103);
		loadkeyposall();
		if(G.script) doscript();
		projektie();
		if(getbutton(MIDDLEMOUSE)) {
			if(view0.persp>=2) view0.persp=1;
			viewmove();
		}
		if(qtest()) {
			toets=  traces_qread(&val);
			if(val) {
				if(toets==ESCKEY) break;
				else if(toets==INPUTCHANGE) G.winakt=val;
			}
		}
		if(G.cfra==G.efra) {
			G.time= G.cputime= (clock()-tijd)/10000;
			tekenoverdraw(0);
			tijd= clock();
			G.cfra=G.sfra-1;
		}

	}
	G.hie=temp2;
	G.cfra=temp1;
	SetButs(103,103);
	while(getbutton(ESCKEY));
	loadkeyposall();
	swapinterval(1);
	projektie();
}


void movedinges(vec,type)
long *vec;
short type;
{
	float z,dx,dy,bepaalphitheta(),mat[4][4];
	long temp[3];
	Device mdev[2];
	short mval[2],xo,yo,val,a=1,ipuc=0;

	temp[0]=vec[0]; 
	temp[1]=vec[1]; 
	temp[2]=vec[2];

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	winset(G.winar[0]);
	getdev(2, mdev, mval);
	mval[1]-S.vys;
	xo=mval[0]; 
	yo=mval[1];
	drawmode(OVERDRAW);
	z=initgrabz(vec[0],vec[1],vec[2]);

	while(TRUE){
		getdev(2, mdev, mval);
		mval[1]-S.vys;
		if(mval[0]!=xo || mval[1]!=yo || a!=0) {
			color(0);
			clear();

			dx= (xo-mval[0])*z/640.0;
			dy= (yo-mval[1])*z/400.0;
			xo=mval[0]; 
			yo=mval[1];

			vec[0]-= (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
			vec[1]-= (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
			vec[2]-= (G.persinv[0][2]*dx + G.persinv[1][2]*dy);

			tekendinges(vec,-type);
		}
		a=qtest();
		if(a) {
			a= traces_qread(&val);
			if(a==ESCKEY || a==LEFTMOUSE || a==SPACEKEY) break;
			if(a==INPUTCHANGE) ipuc= val;
			a=0;
		}
	}
	if(a==ESCKEY) { 
		vec[0]=temp[0]; 
		vec[1]=temp[1]; 
		vec[2]=temp[2];
	}
	if(ipuc) qenter(INPUTCHANGE,ipuc);

	color(0);
	clear();
	drawmode(NORMALDRAW);

	if(type<3) {
		dx=view0.phi; 
		dy=view0.theta;
		z=bepaalphitheta();
		if(view0.persp<2) {
			view0.phi=dx; 
			view0.theta=dy;
		}
	}
	projektie();
}

/* ************************* FLY ROUTINES ************************* */


extern float *vectoquat();

void tekenviewcross(speed, cent)
float speed;
short *cent;
{
	short val;

	frontbuffer(1);
	persp(0);
	cpack(0x0);
	sbox(cent[0]-20, cent[1], cent[0]+20, cent[1]);
	sbox(cent[0], cent[1]-20, cent[0], cent[1]+20);
	cpack(0xFFFF);
	sbox(cent[0]-19, cent[1]-1, cent[0]+21, cent[1]-1);
	sbox(cent[0]+1, cent[1]-21, cent[0]+1, cent[1]+19);
	cpack(0xFF);
	val= speed/10.0;
	sbox(cent[0]-19, cent[1]+val, cent[0]+21, cent[1]+val);
	frontbuffer(0);
	persp(1);
	
}

void fly()
{
	struct Base *tbase;
	float speed=0.0, speedo=1.0, zspeed=0.0, dvec[3], *quat, mat[3][3], oldquat[4];
	long oldvec[3];
	short toets, val, cent[2];
	short mval[2];
	
	if(view0.cambase== 0) return;
	
	VECCOPY(oldvec, view0.cambase->v);
	QUATCOPY(oldquat, view0.cambase->q);
	
	cent[0]= S.vxw/2;
	cent[1]= S.vyw/2;
	setvaluator(MOUSEX, cent[0], 0, S.vxw);
	setvaluator(MOUSEY, cent[1]+S.vys, 0, S.vyw+S.vys);
	
	tekenviewcross(speed, cent);
	
	while(TRUE) {
		getmouseco(mval);

		if(qtest()) {
			toets=  traces_qread(&val);
			if(val) {
				if(toets==ESCKEY) {
					VECCOPY(view0.cambase->v, oldvec);
					QUATCOPY(view0.cambase->q, oldquat);
					break;
				}
				else if(toets==SPACEKEY) break;
				else if(toets==INPUTCHANGE) G.winakt=val;
				else if(toets==LEFTMOUSE) {
					speed+= view0.grid/75.0;
					if(getbutton(MIDDLEMOUSE)) speed= 0.0;
				}
				else if(toets==MIDDLEMOUSE) {
					speed-= view0.grid/75.0;
					if(getbutton(LEFTMOUSE)) speed= 0.0;
				}
			}
		}
		
		/* dvec bepalen */
		val= mval[0]-cent[0];
		if(val>20) val-= 20; else if(val< -20) val+= 20; else val= 0;
		dvec[0]= -0.0007*val;

		val= mval[1]-cent[1];
		if(val>20) val-= 20; else if(val< -20) val+= 20; else val= 0;
		dvec[1]= -0.0007*val;
		
		dvec[2]= -1.0;
		
		zspeed= 0.0;
		if(getbutton(LEFTCTRLKEY)) zspeed= -view0.grid/25.0;
		if(getbutton(LEFTALTKEY)) zspeed= view0.grid/25.0;
		
		if(speedo!=0.0 || zspeed!=0.0 || dvec[0]!=0.0 || dvec[1]!=0.0) {
		
			Normalise(dvec);
			
			Mat3CpyMat4(mat, G.persinv);
			Mat3MulVecfl(mat, dvec);
			quat= vectoquat(dvec, 5, 1);	/* track en upflag, niet die van de base: cameraview-berekening gebruikt ze niet */
			QUATCOPY(view0.cambase->q, quat);
			
			VecMulf(dvec, (float)speed);
			view0.cambase->v[0]-= dvec[0];
			view0.cambase->v[1]-= dvec[1];
			view0.cambase->v[2]-= (dvec[2]-zspeed);

			tbase= view0.cambase->t;
			view0.cambase->t= 0;
			projektie();
			tekenviewcross(speed, cent);
			view0.cambase->t= tbase;
		}
		speedo= speed;
	}
	
	if(view0.cambase->t) {	/* zodat ie weer mooi trackt */
		QUATCOPY(view0.cambase->q, oldquat);
	}
	projektie();
}


/* ********************** */

struct View tempview;

void gettempview(v)
struct View **v;
{
	*v= &tempview;
}

void initlocalview()
{
	struct Base *base;
	struct ObData *ob;
	float ws, min[3], max[3], afm[3], bmat[4][4], omat[3][3];
	long a, c, ok=0;

	/* maak een local viewblock */
	/* zet van alle selected bases (of alleen de ebase) de 15e layerflag */

	G.localview= 0;
	min[0]= min[1]= min[2]= 1.0e10;
	max[0]= max[1]= max[2]= -1.0e10;

	base= G.firstbase;
	if(G.ebase) base= G.ebase;
	while(base) {
		if(TESTBASE(base) || G.ebase) {
			minmaxbase(base, min, max);
			base->lay |= 32768;
			ok= 1;
		}
		if(G.ebase) break;
		base= base->next;
	}
	afm[0]= (max[0]-min[0]);
	afm[1]= (max[1]-min[1]);
	afm[2]= (max[2]-min[2]);
	ws= MAX3(afm[0], afm[1], afm[2]);
	if(ws==0.0) ok= 0;
	
	if(ok) {
		G.localview= 1;
		memcpy(&tempview, &view0, sizeof(struct View));

		view0.ofs[0]= -(min[0]+max[0])/2.0;
		view0.ofs[1]= -(min[1]+max[1])/2.0;
		view0.ofs[2]= -(min[2]+max[2])/2.0;
		afm[0]= MAX3(afm[0], afm[1], afm[2]);
		view0.dproj= afm[0]/22000.0;

		if(view0.persp>1) {
			view0.persp= 1;
			
		}
		view0.near= 100;
		view0.muis[0]= -view0.ofs[0];
		view0.muis[1]= -view0.ofs[1];
		view0.muis[2]= -view0.ofs[2];
		berekenschermco(view0.muis,&view0.mx);
		view0.lay= 32768;
		SetButs(100,126);
		countall();
		projektie();
	}
	else {
		/* flags wissen */
		
		base= G.firstbase;
		while(base) {
			if( (base->lay!= 65535) && (base->lay & 32768) ) {
				base->lay-= 32768;
				if(base->lay==0) base->lay=view0.lay;
				if(base!=G.ebase) base->f |= 1;
			}
			base= base->next;
		}
	}
}

void endlocalview()
{
	struct Base *base;
	/* restore oude view */
	/* clear alle 15e layerflags, maak weer selected */

	if(G.localview) {
		G.localview= 0;
		memcpy(&view0, &tempview, sizeof(struct View));
		base= G.firstbase;
		while(base) {
			if( (base->lay!= 65535) && (base->lay & 32768) ) {
				base->lay-= 32768;
				if(base->lay==0) base->lay=view0.lay;
				if(base!=G.ebase) base->f |= 1;
			}
			base= base->next;
		}
		SetButs(100,126);
		countall();
		if(G.script) doscript();
		projektie();
	}

}

