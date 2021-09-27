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

/* 	spline.c	*/


/* 
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

	maakbez(q0,q1,q2,q3,p)
	berekenpad(base)

	tekenspline(bez)

	calchandle(a)
	calchandles()

	clearGspline()
	schrijfipospline(mode)
	tekenspline2d(mode)
	muisspline2d()
	insertsplinepunt2d()
	delsplinepunt2d()
	berekeny(f,o,b)
short	findzero(x,q0,q1,q2,q3,o)
float	ipo(bez,fac)

*/


#include <malloc.h>
#include <string.h>
#include <fmclient.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/device.h>
#include "/usr/people/include/Trace.h"

#define SMALL -1.0e-10

long rectblack[3][3]={{0,0,0},{0,0,0},{0,0,0}};
long rectwhite[3][3]={{0xFFFFFF,0xFFFFFF,0xFFFFFF},
	{0xFFFFFF,0xFFFFFF,0xFFFFFF},
	{0xFFFFFF,0xFFFFFF,0xFFFFFF}};
ulong rectyellow[5][5]={{0x77FFFF,0x77FFFF,0x77FFFF, 0x77FFFF, 0x77FFFF},
	{0x77FFFF,0x77FFFF,0x77FFFF, 0x77FFFF, 0x77FFFF},
	{0x77FFFF,0x77FFFF,0x77FFFF, 0x77FFFF, 0x77FFFF},
	{0x77FFFF,0x77FFFF,0x77FFFF, 0x77FFFF, 0x77FFFF},
	{0x77FFFF,0x77FFFF,0x77FFFF, 0x77FFFF, 0x77FFFF}};
ulong rectpurple[5][5]={{0xFF70FF,0xFF70FF,0xFF70FF, 0xFF70FF, 0xFF70FF},
	{0xFF70FF,0xFF70FF,0xFF70FF, 0xFF70FF, 0xFF70FF},
	{0xFF70FF,0xFF70FF,0xFF70FF, 0xFF70FF, 0xFF70FF},
	{0xFF70FF,0xFF70FF,0xFF70FF, 0xFF70FF, 0xFF70FF},
	{0xFF70FF,0xFF70FF,0xFF70FF, 0xFF70FF, 0xFF70FF}};

float spv_x1=IPOMINX,spv_x2=IPOMAXX;
float spv_y1=0,spv_y2=0;
struct Bezier *copybez=0;


void maakbez(q0,q1,q2,q3,p,it)
float q0,q1,q2,q3,*p;
short it;
{
	float rt0,rt1,rt2,rt3,f;
	short a;

	f=(float)it;
	rt0=q0;
	rt1=3.0*(q1-q0)/f;
	f*=f;
	rt2=3.0*(q0-2*q1+q2)/f;
	f*=it;
	rt3=(q3-q0+3.0*(q1-q2))/f;
 	
  	q0=rt0;
	q1=rt1+rt2+rt3;
	q2=2*rt2+6*rt3;
	q3=6*rt3;
  
  	for(a=0;a<=it;a++) {
		*p=q0;
		p+=3;
		q0+=q1;
 		q1+=q2;
 		q2+=q3;
 	}
}	

void oldbez_to_curve(struct Bezier *bez, struct Nurb *nu)	/* nu bestaat al */
{
	struct BezTriple *bezt;
	float *fp;
	int a;
	
	if(bez==0) return;
	if(nu==0) return;
	
	nu->pntsu= bez->cp;
	nu->type= 1;
	nu->resolu= 32;
	nu->bezt= callocstructN(struct BezTriple, nu->pntsu, "oldbez_to");
	nu->flagu= (bez->f & 1);
	
	bezt= nu->bezt;
	fp= (float *)(bez+1);
	for(a=0; a<nu->pntsu; a++, bezt++) {
		VECCOPY(bezt->vec[0], fp); fp+= 3;
		VECCOPY(bezt->vec[1], fp); fp+= 3;
		VECCOPY(bezt->vec[2], fp); fp+= 3;
	}
	
}

void berekenpad(base)
struct Base *base;
{
	void tekenspline2d();
	struct MoData *mo;
	struct Bezier *bez;
	struct DispList *dl;
	float *f,*f1,*bezpt,*maxbezpt,*dist,*maxdist,x,y,z;
	float totdist=0,fac,dfac,d=0,fac1,fac2,fac3,fac4,ipo();
	float *temp,*temp0,*temp1,*temp2,t1,t2,t3;
	short a,tot,it=32,s[2],cycl=0;
	float *t,*maxdata;
	
	if(base->soort != -2) return;
	mo= (struct MoData *)base->d;

	if(mo->data) freeN(mo->data);
	mo->data= 0;
	
	dl= mo->disp.first;
	if(dl==0) return;

	cycl= (dl->type==DL_POLY);
	
	tot= dl->nr-1;

	bezpt= (float *)(dl+1);
	
	dist= (float *)mallocN(16+tot*4,"berekenpaddist");

		/* alle lengtes in *dist */
	temp=bezpt;
	f=dist;
	*f=0;
	for(a=0; a<tot; a++) {
		f++;
		if(cycl && a==tot-1) {
			x= bezpt[0]-temp[0];
			y= bezpt[1]-temp[1];
			z= bezpt[2]-temp[2];
		}
		else {
			x= temp[3]-temp[0];
			y= temp[4]-temp[1];
			z= temp[5]-temp[2];
		}
		*f= *(f-1)+ fsqrt(x*x+y*y+z*z);
		temp+= 3;
	}
	totdist= *f;
	
		/* de padpunten in mo->data */
	
	t= mo->data = (float *)callocN(12*base->len,"ber.pad.punten");
	maxdata= t+3*base->len;
	maxbezpt= bezpt+3*tot+3;
	maxdist= dist+tot;
	f= dist+1;
	temp= bezpt;
	tot= 0;
	bez= mo->ipotra;

	dfac= 1.0/(base->len-1+cycl);
	fac= 0;
	d= ipo(bez,0.0)*totdist;

	while( tot<base->len) {
		while( (d>= *f) && f<maxdist) {
			f++;
			temp+=3;
		}
		while( d<*(f-1) && f>dist) {
			f--;
			temp-=3;
		}
		if(temp >maxbezpt ) {
			error("voorbij maxbezpt");
			break;
		}
		if(temp <bezpt ) {
			error("voor maxbezpt");
			/* printf("base->len %d\n",base->len); */
			break;
		}
		
		fac1= *(f)- *(f-1);
		fac2= *(f)-d;	
		fac1= fac2/fac1;
		fac2= 1.0-fac1;
		t[0]= fac1*temp[0]+ fac2*temp[3];
		t[1]= fac1*temp[1]+ fac2*temp[4];
		t[2]= fac1*temp[2]+ fac2*temp[5];
		tot++;
		t+=3;
		fac+=dfac;
		d= ipo(bez,fac)*totdist;
	}
	
	freeN(dist);
	if(G.mainb==7) tekenspline2d(0);
}


void bcrv(f)
float f[][3];
{
    double x2, x3, x4;
    double y2, y3, y4;
    double z2, z3, z4;
    double t1, t2, t3;
    double v[3];
    long i;
    
    t1 = 3.0*(f[1][0]-f[0][0]) / 32.0;
    t2 = 3.0*(f[0][0]-2.0*f[1][0]+f[2][0]) / (32.0 * 32.0);
    t3 = (f[3][0]-f[0][0]+3.0*(f[1][0]-f[2][0])) / (32.0 * 32.0 * 32.0);

    x2 = t1 + t2 + t3;
    x4 = 6.0 * t3;
    x3 = 2.0 * t2 + x4;

    t1 = 3.0*(f[1][1]-f[0][1]) / 32.0;
    t2 = 3.0*(f[0][1]-2.0*f[1][1]+f[2][1]) / (32.0 * 32.0);
    t3 = (f[3][1]-f[0][1]+3.0*(f[1][1]-f[2][1])) / (32.0 * 32.0 * 32.0);

    y2 = t1 + t2 + t3;
    y4 = 6.0 * t3;
    y3 = 2.0 * t2 + y4;

    t1 = 3.0*(f[1][2]-f[0][2]) / 32.0;
    t2 = 3.0*(f[0][2]-2.0*f[1][2]+f[2][2]) / (32.0 * 32.0);
    t3 = (f[3][2]-f[0][2]+3.0*(f[1][2]-f[2][2])) / (32.0 * 32.0 * 32.0);

    z2 = t1 + t2 + t3;
    z4 = 6.0 * t3;
    z3 = 2.0 * t2 + z4;
    
    v[0] = f[0][0];
    v[1] = f[0][1];
    v[2] = f[0][2];

    bgnline();
    for(i = 32; i>=0; i--){
	v3d(v);
	v[0] += x2; v[1] += y2; v[2] += z2;
	x2 += x3; y2 += y3; z2 += z3;
	x3 += x4; y3 += y4; z3 += z4;
    }
    endline();   
}


void tekenspline(bez,mode)
struct Bezier *bez;
{
	float *f,ftp[12], mat[4][4];
	short it=32,a,b,s[2];

	curvebasis(1);
	curveprecision(32);

	getsingmatrix(mat);
	Mat4SwapMat4(G.persmat, mat);

	f=(float *)(bez+1);
	f+=3;

	for(a=0;a<bez->cp-1;a++,f+=9) crv(f);

	if(bez->f & 1) {
		memcpy(ftp,f,24);
		f=(float *)(bez+1);
		memcpy(ftp+6,f,24);
		crv(ftp);
	}
	setlinestyle(1);
	f=(float *)(bez+1);
	for(a=0;a<bez->cp;a++,f+=9) {
		if(mode) {
			if(G.actp3==a) setlinestyle(0);
			bgnline();
				v3f(f);
				v3f(f+3);
				v3f(f+6);
			endline();
			if(G.actp3==a) setlinestyle(1);
			berekenschermcofl(f,s);
			tekenrect3(s,rectblack);
			berekenschermcofl(f+6,s);
			tekenrect3(s,rectblack);
		}
		berekenschermcofl(f+3,s);
		if(mode!=0 && G.actp3==a) tekenrect3(s,rectwhite);
		else tekenrect3(s,rectyellow);
	}
	setlinestyle(0);

	Mat4SwapMat4(G.persmat, mat);
}



void calchandle(bez,a,mode)
struct Bezier *bez;
short a,mode;		/* mode==1 : vector */
{
	float *p,*p1,*p2,*p3,pt[3];
	float dx1,dy1,dz1,dx,dy,dz,vx,vy,vz,len,len1,len2;
	
	if(a<0 || a>bez->cp) return;

	p=(float *)(bez+1);
	
	p2=p+9*a+3;
	if(bez->f & 1) {	/* cyclic bezier */
		if(a>0) p1=p2-9;
		else p1=p+(9*(bez->cp-1)+3);
		if(a<bez->cp-1) p3=p2+9;
		else p3=p+3;
	}
	else {
		if(a>0) p1=p2-9;
		else {
			pt[0]= 2*p2[0]-p2[9];
			pt[1]= 2*p2[1]-p2[10];
			pt[2]= 2*p2[2]-p2[11];
			p1=pt;
		}
		if(a<bez->cp-1) p3=p2+9;
		else {
			pt[0]= 2*p2[0]-p2[-9];
			pt[1]= 2*p2[1]-p2[-8];
			pt[2]= 2*p2[2]-p2[-7];
			p3=pt;
		}
	}
	dx= *p2 - *p1;
	dy= *(p2+1)- *(p1+1);
	dz= *(p2+2)- *(p1+2);
	len1=fsqrt(dx*dx+dy*dy+dz*dz);
	dx1= *p3 - *p2;
	dy1= *(p3+1)- *(p2+1);
	dz1= *(p3+2)- *(p2+2);
	len2=fsqrt(dx1*dx1+dy1*dy1+dz1*dz1);

	if(len1==0) len1=1;
	if(len2==0) len2=1;

	if( (bez->f & 1)==0 ) {
		if(a==bez->cp-1) len2=len1;
		if(a==0) len1=len2;
	}

	if(mode==0) {
		vx=dx1/len2+dx/len1;
		vy=dy1/len2+dy/len1;
		vz=dz1/len2+dz/len1;
		len=2.71*fsqrt(vx*vx+vy*vy+vz*vz);
		if(len!=0) {
			len1/=len;
			len2/=len;
			*(p2-3)= *p2-vx*len1;
			*(p2-2)= *(p2+1)-vy*len1;
			*(p2-1)= *(p2+2)-vz*len1;
			*(p2+3)= *p2+vx*len2;
			*(p2+4)= *(p2+1)+vy*len2;
			*(p2+5)= *(p2+2)+vz*len2;
		}
	}
	else {
		dx/=3.0; dy/=3.0; dz/=3.0;
		dx1/=3.0; dy1/=3.0; dz1/=3.0;
		*(p2-3)= *p2-dx;
		*(p2-2)= *(p2+1)-dy;
		*(p2-1)= *(p2+2)-dz;
		*(p2+3)= *p2+dx1;
		*(p2+4)= *(p2+1)+dy1;
		*(p2+5)= *(p2+2)+dz1;
	}
}

void calchandles(bez,mode)
struct Bezier *bez;
short mode;
{
	short a;
	
	for(a=0;a<bez->cp;a++) calchandle(bez,a,mode);
}

/* ********************************** */



void clearGspline()
{
	float *fp;

	G.spline->cp=2;
	G.spline->f=0;
	/* G.spline->f1=0; dit doet niets meer! (nul is lege spline) */
	fp= (float *)(G.spline+1);
	fp[3]= IPOMINX;
	fp[4]= IPOMINY;
	fp[5]= 0.0;

	fp[6]= fp[3]+ (IPOMAXX-IPOMINX)/3.0;
	fp[7]= fp[4]+ (IPOMAXY-IPOMINY)/3.0;
	fp[8]= 0.0;
	
	fp[12]= IPOMAXX;
	fp[13]= IPOMAXY;
	fp[14]= 0.0;

	fp[9]= fp[12]- (IPOMAXX-IPOMINX)/3.0;
	fp[10]= fp[13]- (IPOMAXY-IPOMINY)/3.0;
	fp[11]= 0.0;
	
	
	/* calchandles(G.spline,0); */
	
	G.actp2=0;
}

void tekenrect3ipo(s,col)
float *s;
short col;
{

	if(s[0]<S.x && s[0]>=IPOMINX && s[1]<S.vys && s[1]>=0) {
		color(col);
		rectf(s[0]-1,s[1]-1,s[0]+1,s[1]+1);
	}
}

void schrijfipospline(mode)	/* mode 1 controleert op eenheids spline */
short mode;			/* doet loadkeypos en fastproj */
{
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	float *fp;
	short len;
	
	if(mode==1) {
		mode=0;
		if(G.spline->cp==2) {
			fp= (float *)(G.spline+1);
			if( fp[4]==IPOMINY && fp[13]==IPOMAXY) {
				if( fp[7]== fp[4]+ (IPOMAXY-IPOMINY)/3.0 ) {
					if( fp[10]== fp[13]- (IPOMAXY-IPOMINY)/3.0 ) {
						mode= 1;
					}
				}
			}
		}
	}

	len=sizeof(struct Bezier)+36*G.spline->cp;

	if(G.ipomode==10) {
		if(G.timeipo) freeN(G.timeipo);
		G.timeipo= 0;
		if(mode==0) {
			G.timeipo= (struct Bezier *)mallocN(len,"schripospline");
			memcpy(G.timeipo, G.spline, len);
		}
	}

	if(G.basact==0) return;

	if(G.basact->soort== -2) {
		mo=(struct MoData *)G.basact->d;
		
		if(G.ipomode==0) {
			if(mo->ipoqfi) freeN(mo->ipoqfi);
			mo->ipoqfi=0;
			if(mode==0) {
				mo->ipoqfi=(struct Bezier *)mallocN(len,"schripospline");
				memcpy(mo->ipoqfi,G.spline,len);
			}
		}
		if(G.ipomode==1) {
			if(mo->ipotra) freeN(mo->ipotra);
			mo->ipotra=0;
			if(mode==0) {
				mo->ipotra=(struct Bezier *)mallocN(len,"schripospline2");
				memcpy(mo->ipotra,G.spline,len);
			}
			makeDispList(G.basact);		/* ook berekenpad */
		}
	}
	if(G.ipomode==20) {
		if(G.basact->ipokey) freeN(G.basact->ipokey);
		G.basact->ipokey=0;
		if(mode==0) {
			G.basact->ipokey=(struct Bezier *)mallocN(len,"schripospline3");
			memcpy(G.basact->ipokey,G.spline,len);
		}
	}
	else if(G.ipomode==21) {
		if(G.basact->ipopkey) freeN(G.basact->ipopkey);
		G.basact->ipopkey=0;
		if(mode==0) {
			G.basact->ipopkey=(struct Bezier *)mallocN(len,"schripospline4");
			memcpy(G.basact->ipopkey,G.spline,len);
		}
	}
	else if(G.ipomode==22) {
		if ELEM3(G.basact->soort, 1, 5, 11) {
			ob= (struct ObData *)G.basact->d;
			if(ob->ipovvkey) freeN(ob->ipovvkey);
			ob->ipovvkey= 0;
			if(mode==0) {
				ob->ipovvkey= (struct Bezier *)mallocN(len,"schripospline4");
				memcpy(ob->ipovvkey,G.spline,len);
			}
		}
		else if(G.basact->soort== -2) {
			mo= (struct MoData *)G.basact->d;
			
			if(mo->ipovkey) freeN(mo->ipovkey);
			mo->ipovkey= 0;
			if(mode==0) {
				mo->ipovkey= (struct Bezier *)mallocN(len,"schripospline4");
				memcpy(mo->ipovkey, G.spline, len);
			}
		}
		else if(G.basact->soort== -4) {
			ika= (struct IkaData *)G.basact->d;
			
			if(ika->ipokey) freeN(ika->ipokey);
			ika->ipokey= 0;
			if(mode==0) {
				ika->ipokey= (struct Bezier *)mallocN(len,"schripospline4");
				memcpy(ika->ipokey, G.spline, len);
			}
		}
	}
	else if(G.ipomode==23) {
		if(G.basact->soort==4) {
			if(G.basact->iponoise) freeN(G.basact->iponoise);
			G.basact->iponoise= 0;
			if(mode==0) {
				G.basact->iponoise= (struct Bezier *)mallocN(len,"schripospline4");
				memcpy(G.basact->iponoise, G.spline, len);
			}
		}
	}
	
	if(G.ipomode>=20) {
		loadkeypos(G.basact, G.basact);
	}
	projektie();
}



void maakorthospline2d(toets)
short toets;
{
	if(toets==0) ;

}

float *bezlenparts(bez)
struct Bezier *bez;
{
	float *f,*f1,*f2,*lendata,*verts,totlen,x,y,z;
	short a,b,it= 16,cycl,tot;

	cycl= (bez->f & 1);
	tot= (bez->cp+cycl);
	verts= (float *)mallocN(12+it*3*4,"bezlenparts");
	lendata= (float *)mallocN(tot*4,"bezlenparts1");
	lendata[0]= 0;

		/* alle splinepunten in *verts */	
	f=(float *)(bez+1);
	f+=3;
	for(a=0;a<bez->cp-1+cycl;a++,f+=9) {
		f1=f+6;
		if(cycl==1 && a==bez->cp-1) f1=(float *)(bez+1);
		maakbez(*(f),*(f+3),*(f1),*(f1+3),verts,it);
		maakbez(*(f+1),*(f+4),*(f1+1),*(f1+4),verts+1,it);
		maakbez(*(f+2),*(f+5),*(f1+2),*(f1+5),verts+2,it);

		f2= verts;
		totlen= 0;
		for(b=0;b<it;b++) {
			x= f2[3]-f2[0];
			y= f2[4]-f2[1];
			z= f2[5]-f2[2];
			totlen+= fsqrt(x*x+y*y+z*z);
			f2+=3;
		}
		lendata[a+1]= lendata[a]+totlen;
	}

	totlen= lendata[tot-1];
	for(a=0;a<tot;a++) lendata[a]/= totlen;
	
	freeN(verts);

	return lendata;
}

void tekenspline2d(mode)
short mode;		/* mode=1 laadt nieuwe bez in G.spline */
{
	extern float mat1[4][4];
	float frametotime();
	struct MoData *mo;
	struct IkaData *ika;
	struct Bezier *bez=0;
	struct Key *k=0;
	float *f,*lendata=0,startx,x,y,x1,x2,y1,y2,dx,dx2,facx,facy,ofs,ipoinv();
	short it=32,a,b,s[2],frs,len,fra,dfra;
	char str[6];

	if(G.mainb!=7) return;

	if(G.ipomode==10) frs= G.images;
	else {
		if(G.basact==0) return;
		frs= G.basact->len;
		if(G.basact->soort== -2) mo= (struct MoData *)G.basact->d;
	}
	
	
	curvebasis(1);
	curveprecision(32);

    /* de juiste Bezier vinden */
	if(mode) {
		if(G.ipomode==10) bez= G.timeipo;
		if(G.basact->soort== -2) {
			if(G.ipomode==0) bez= mo->ipoqfi;
			else if(G.ipomode==1) bez= mo->ipotra;
		}
		if(G.ipomode==20) bez=G.basact->ipokey;
		else if(G.ipomode==21) bez=G.basact->ipopkey;
		else if(G.ipomode==23) bez=G.basact->iponoise;
		else if(G.ipomode==22) {
		    if ELEM3(G.basact->soort, 1, 5, 11) 
				bez= ((struct ObData *)G.basact->d)->ipovvkey;
			else if(G.basact->soort== -2)
				bez= ((struct MoData *)G.basact->d)->ipovkey;
			else if(G.basact->soort== -4)
				bez= ((struct IkaData *)G.basact->d)->ipokey;
		}
		if(bez) {
			len=sizeof(struct Bezier)+(36*bez->cp);
			memcpy(G.spline,bez,len);
		}
		if(bez==0) {
			clearGspline();
			bez=G.spline;
		}
	}
	else bez= G.spline;

	winset(G.winar[1]);
	frontbuffer(0);
	
  /* de matrix */
	if(spv_y1==0) {	/* initialiseren, want IPOMAXY is geen constante */
		spv_y1=IPOMINY;
		spv_y2=IPOMAXY;
	}	
	ofs= (IPOMAXX*spv_x1-IPOMINX*spv_x2)/(IPOMAXX-IPOMINX);
	facx= IPOMINX/(spv_x1-ofs);
	x1= ofs;
	x2= (1279)/facx+ofs;
	ofs= (IPOMAXY*spv_y1-IPOMINY*spv_y2)/(IPOMAXY-IPOMINY);
	facy= IPOMINY/(spv_y1-ofs);
	y1= ofs;
	y2= (223)/facy+ofs;

   /* delta berekenen teken frames */
	startx=IPOMINX;
	dx= (facx*(IPOMAXX-IPOMINX))/((float)frs-1);
	/* minimale afstand print frames: 32 pixels */
	dfra= (short)(1.0+ 32.0/dx);
	if(dfra<=2);
	else if(dfra<=5) dfra=5;
	else if(dfra<=10) dfra= 10;
	else if(dfra<=20) dfra= 20;
	else if(dfra<=50) dfra= 50;
	else if(dfra<=100) dfra= 100;
	else if(dfra<=200) dfra= 200;
	else if(dfra<=500) dfra= 500;
	else if(dfra<=1000) dfra= 1000;
	else dfra= 2000;

	if(frs>1)
	    dx= (dfra*(IPOMAXX-IPOMINX))/((float)frs-1);
	else
	    dx= (dfra*(IPOMAXX-IPOMINX));
	startx-= dx/dfra;

  /* sliders */
	color(0);
	sboxf(IPOMINX,0,IPOMAXX,IPOMINY-1);
	color(38);
	sboxfs(spv_x1,5,spv_x2,IPOMINY-5);

	color(0);
	sboxf(IPOMINX-20,0,IPOMINX,IPOMAXY);
	color(38);
	sboxfs(IPOMINX-16,spv_y1,IPOMINX-4,spv_y2);

   /* windowmatrix zetten */
	
	ortho2(x1, x2, y1, y2);

   /* achtergrond */
	countkeys();	/* controleert G.actkey */
	if(G.ipomode==20) k=G.basact->key;
	else if(G.ipomode==21) k=G.basact->pkey;
	else if(G.ipomode==22) {
	    if(G.basact->soort==1) k= ((struct ObData *)G.basact->d)->vv->key;
	    else if ELEM(G.basact->soort, 5, 11) k= ((struct ObData *)G.basact->d)->cu->key;
	    else if(G.basact->soort== -2) k= ( (struct MoData *)G.basact->d )->key;
	    else if(G.basact->soort== -4) k= ( (struct IkaData *)G.basact->d )->key;
	}
	
	scrmask((IPOMINX*S.bxw)/1280, (IPOMAXX*S.bxw)/1280 +1, (IPOMINY*S.byw)/223, (IPOMAXY*S.byw)/223+1);

	x= startx;
	a= 1;
	while(x<=IPOMAXX) {
		if(a & 1) color(35);
		else color(38);
		sboxf(x,IPOMINY,fceil(x+dx),IPOMAXY+2);
		x+= dx;
		a++;
	}

   /* keypos lijnen */
	scrmask((IPOMINX*S.bxw)/1280, (IPOMAXX*S.bxw)/1280 +1, 4, (IPOMAXY*S.byw)/223+1);
	a=1;
	while(k) {
		if(a==G.actkey) color(6);
		else color(5);
		
		if(k->f1==1) setlinestyle(2);
		else if(k->f1==2) setlinestyle(3);
		else if(k->f1==3) setlinestyle(4);
		
		a++;
		y= k->pos*(IPOMAXY-IPOMINY)+IPOMINY;
		if( facy*(y-y1)>=IPOMINY ) sbox(IPOMINX,y,IPOMAXX,y);

		setlinestyle(0);
		
		x= ipoinv(bez,k->pos)*(IPOMAXX-IPOMINX)+IPOMINX;
		color(RED);
		sbox(x,10,x,y);
		k= k->next;
	}
	if(G.ipomode==1 && mo->beztra!=0) {	/* controlepunt lijnen */
		/*
		lendata= bezlenparts(mo->beztra);
		b= mo->beztra->cp;
		if(lendata) {
			for(a=0;a<b;a++) {
				if(a==G.actp3) color(6);
				else color(BLACK);

				y= lendata[a]*(IPOMAXY-IPOMINY)+IPOMINY;
				if( facy*(y-y1)>=IPOMINY ) 
				sbox(IPOMINX,y,IPOMAXX,y);
				x= ipoinv(bez,lendata[a])*(IPOMAXX-IPOMINX)+IPOMINX;
				color(RED);
				sbox(x,10,x,y);
			}
			freeN(lendata);
		}
		*/
	}
   /* groene cfra lijn */
	fra= G.cfra;
	if(G.ipomode>=20) fra-= (G.basact->sf-1);

	fra= (int) frametotime(fra);
	
	if(fra<=frs && fra>0) {
		color(GREEN);
		x= (IPOMAXX-IPOMINX)/((float)frs-1);
		x= IPOMINX+ x*(fra-1)+0.1;	/* +0.1 anders soms net weggeclipt */
		sbox(x,IPOMINY,x,IPOMAXY);
	}

   /* teksten */
	color(WHITE);
	x= startx- 8.0/facx;    /* een cijfer naar links */
	y= 6.0/facy+y1;
	fra= 0;
	while(x<IPOMAXX) {
		sprintf(str,"%d",fra);
		cmov2(x,y);
		fmprstr(str);
		x+= dx;
		fra+= dfra;
	}

   /* curves */
	scrmask((IPOMINX*S.bxw)/1280, (IPOMAXX*S.bxw)/1280, (IPOMINY*S.byw)/223, (IPOMAXY*S.byw)/223+1);

	f=(float *)(bez+1);
	for(a=0;a<bez->cp;a++,f+=9) {
		color(0);
		if(a==G.actp2) color(3);
		bgnline();		/* handles */
			if(a!=0) v3f(f);
			v3f(f+3);
			if(a!=bez->cp-1) v3f(f+6);
		endline();
		if(a!=0) tekenrect3ipo(f,0);
		if(a!=bez->cp-1) tekenrect3ipo(f+6,0);
		tekenrect3ipo(f+3,7);
	}

	color(7);
	f=(float *)(bez+1);
	f+=3;
	for(a=0;a<bez->cp-1;a++,f+=9) crv(f);

	scrmask(0, S.bxw, 0, S.byw-1);
	ortho2(0,1279,0,223-1);
	if(G.machine!=ENTRY) {
		swapbuffers();
		frontbuffer(1);
	}
}

void moveipokeypos()
{
	struct Key *key=0,*basekey,*sortkeylist();
	float dy,ofsy,facy;
	short mval[2],mdev[2],xo,yo,a;

	/* is hier sprake van keyframing? */
	if(G.ipomode==20) key= G.basact->key;
	else if(G.ipomode==21) key= G.basact->pkey;
	else if(G.ipomode==22) {
	    if(G.basact->soort==1) key= ((struct ObData *)G.basact->d)->vv->key;
	    else if ELEM(G.basact->soort, 5, 11) key= ((struct ObData *)G.basact->d)->cu->key;
	    else if(G.basact->soort== -2) key= ((struct MoData *)G.basact->d)->key;
	    else if(G.basact->soort== -4) key= ((struct IkaData *)G.basact->d)->key;
	}
	
	basekey= key;
	
	a= 1;
	while(key) {
		if(a==G.actkey) break;
		a++;
		key= key->next;
	}
	if(key== 0) return;

	/* verplaats aktieve key, geen dubbels! */

	ofsy= (IPOMAXY*spv_y1-IPOMINY*spv_y2)/(IPOMAXY-IPOMINY);
	facy= IPOMINY/(spv_y1-ofsy);

	dy= facy*(key->pos*(IPOMAXY-IPOMINY)+IPOMINY-ofsy);

	/* if( dy<0.0) return; */
	/* if( dy>IPOMAXY) return; */

	ButtonsGetmouse(0);
	ButtonsGetmouse(mval);
	xo= mval[0]; yo= mval[1];
	
	while( getbutton(LEFTMOUSE) || getbutton(MIDDLEMOUSE) ) {
		ButtonsGetmouse(mval);
		if(mval[0]!=xo || mval[1]!=yo) {
			dy= mval[1]-yo;
			dy/= (facy*(IPOMAXY-IPOMINY));

			key->pos+= dy;
			if(key->pos<0.0) key->pos= 0.0;
			else if(key->pos> 1.0) key->pos= 1.0;

			xo= mval[0]; yo= mval[1];
			
			tekenspline2d(0);
		}
	}

	basekey= sortkeylist(basekey);
	if(G.ipomode==20) G.basact->key= basekey;
	else if(G.ipomode==21) G.basact->pkey= basekey;
	else if(G.ipomode==22) {
	    if(G.basact->soort==1) ((struct ObData *)G.basact->d)->vv->key= basekey;
		else if ELEM(G.basact->soort, 5, 11) ((struct ObData *)G.basact->d)->cu->key= basekey;
	    else if(G.basact->soort== -2) ((struct MoData *)G.basact->d)->key= basekey;
	    else if(G.basact->soort== -4) ((struct IkaData *)G.basact->d)->key= basekey;
	}
	
	tekenspline2d(0);	/* omdat actkey van plek veranderd kan zijn */
}

void minmaxIpoSliders()
{
	/* test op overflows */
	float dx,dy;

	dx= spv_x2-spv_x1;
	if(dx>IPOMAXX-IPOMINX) {
		spv_x1= IPOMINX;
		spv_x2= IPOMAXX;
	}
	else if(spv_x1>IPOMAXX || spv_x2>IPOMAXX) {
		spv_x2= IPOMAXX;
		spv_x1= IPOMAXX-dx;
	}
	else if(spv_x1<IPOMINX || spv_x2<IPOMINX) {
		spv_x1= IPOMINX;
		spv_x2= IPOMINX+dx;
	}

	dy= spv_y2-spv_y1;
	if(dy>IPOMAXY-IPOMINY) {
		spv_y1= IPOMINY;
		spv_y2= IPOMAXY;
	}
	else if(spv_y1>IPOMAXY || spv_y2>IPOMAXY) {
		spv_y2= IPOMAXY;
		spv_y1= IPOMAXY-dy;
	}
	else if(spv_y1<IPOMINY || spv_y2<IPOMINY) {
		spv_y1= IPOMINY;
		spv_y2= IPOMINY+dy;
	}
	
}

void clearIpo()
{
    clearGspline();
    schrijfipospline(1);
    tekenspline2d(0);
}

void copyIpo()
{
    long len;
    
    len= sizeof(struct Bezier)+36*G.spline->cp;
    if(copybez) freeN(copybez);
    copybez= (struct Bezier *)mallocN(len, "copybez");
    memcpy(copybez, G.spline, len);
}

void pasteIpo()
{
    long len;
    
    if(copybez==0) return;
    len= sizeof(struct Bezier)+36*copybez->cp;
    memcpy(G.spline, copybez, len);
    schrijfipospline(1);
    tekenspline2d(0);
}


void ipozoom(event)
short event;
{
	float dx;
	short dox=1,doy=1,mdev[2],mval[2];

	ButtonsGetmouse(0);
	ButtonsGetmouse(mval);

	if(mval[1]<IPOMINY) doy= 0;
	else if(mval[0]<IPOMINX) dox= 0;
	
	if(dox) {
		dx= (spv_x2-spv_x1)/10.0;
		if(event) dx= -dx;
		spv_x1-= dx;
		spv_x2+= dx;
	}
	if(doy) {
		dx= (spv_y2-spv_y1)/10.0;
		if(event) dx= -dx;
		spv_y1-= dx;
		spv_y2+= dx;
	}
	minmaxIpoSliders();
	tekenspline2d(0);
}

void ipomove()
{
	/* if LEFTMOUSE zoom.
	   if mousey<IPOMINY: move x,   if mousex<IPOMINX: move y,
	   else move both */

	float dx,dy;
	short dox=1,doy=1,zoomx=0,zoomy=0,mdev[2],mval[2],xo,yo;

	ButtonsGetmouse(0);		/* is init */

	ButtonsGetmouse(mval);
	xo= mval[0]; yo= mval[1];

	if(xo<IPOMINX-20 ) return;

	if(getbutton(LEFTMOUSE)) {
		if(yo>IPOMINY && xo>IPOMINX) {
			moveipokeypos();
			return;
		}
		/* welke variabele is dichtst bij */
		if( abs(xo-(long)spv_x1) < abs(xo-(long)spv_x2) ) zoomx= 1;
		else zoomx= 2;
		if( abs(yo-(long)spv_y1) < abs(yo-(long)spv_y2) ) zoomy= 1;
		else zoomy= 2;
	}
	if(yo<IPOMINY) doy= 0;
	else if(xo<IPOMINX) dox= 0;

	while( getbutton(LEFTMOUSE) || getbutton(MIDDLEMOUSE) ) {
		ButtonsGetmouse(mval);
		if(mval[0]!=xo || mval[1]!=yo) {
			if(zoomx || zoomy) {
				if(dox) {
					dx= mval[0]-xo;
					if(zoomx==1) spv_x1+=dx;
					else if(zoomx==2) spv_x2+=dx;
					if(spv_x2-spv_x1<10.0) {
						if(zoomx==1) spv_x2= spv_x1+10.0;
						else spv_x1= spv_x2-10.0;
					}
				}
				if(doy) {
					dy= mval[1]-yo;
					if(zoomy==1) spv_y1+=dy;
					else if(zoomy==2) spv_y2+=dy;
					if(spv_y2-spv_y1<10.0) {
						if(zoomy==1) spv_y2= spv_y1+10.0;
						else spv_y1= spv_y2-10.0;
					}
				}
			} else {
				if(dox) {
					dx= (mval[0]-xo);
					spv_x1+=dx;
					spv_x2+=dx;
				}
				if(doy) {
					dy= (mval[1]-yo);
					spv_y1+=dy;
					spv_y2+=dy;
				}
			}
			minmaxIpoSliders();
			xo= mval[0]; yo= mval[1];
			tekenspline2d(0);
		}
	}
}

void muisspline2d()
{
	struct MoData *mo;
	struct Bezier *bez;
	Device mdev[2];
	long vec[3];
	short a,act,b,dist=100,val,mval[2],xn,yn,xm,ym,hand,s[2];
	float *f,*fact=0,ofsx,ofsy,facx,facy;
	float f1[3],f2[3],f3[3],dx,dy,lenfac,VecLenf();

	if(G.spline->cp==0) return;	

	ofsx= (IPOMAXX*spv_x1-IPOMINX*spv_x2)/(IPOMAXX-IPOMINX);
	facx= IPOMINX/(spv_x1-ofsx);
	ofsy= (IPOMAXY*spv_y1-IPOMINY*spv_y2)/(IPOMAXY-IPOMINY);
	facy= IPOMINY/(spv_y1-ofsy);

	ButtonsGetmouse(0);
	ButtonsGetmouse(mval);

	xn=xm= (mval[0])/facx+ofsx;
	yn=ym= (mval[1])/facy+ofsy;

	bez=G.spline;
	if(bez==0) return;

	f=(float *)(bez+1);
	for(a=0;a<bez->cp;a++) {
		for(b= -1;b<2;b++) {
			val=fabs(f[0]-xm)+fabs(f[1]-ym);
			if(val<dist) {
				dist=val;
				hand= b; fact=f; act=a;
			}
			f+=3;
		}
	}
	xn=xm= (mval[0]);
	yn=ym= (mval[1]);

	if(fact== 0) return;
	
	if(act!=G.actp2) {
		G.actp2=act;
		tekenspline2d(0);
	}
	G.actp2=act;
	VECCOPY(f1,fact);
	if(hand== -1) {
		VECCOPY(f2,fact+6);
		VECCOPY(f3,fact+3);
	}
	if(hand== 1) {
		VECCOPY(f2,fact-6);
		VECCOPY(f3,fact-3);
	}
	if(hand== 0) {
		VECCOPY(f2,fact-3);
		VECCOPY(f3,fact+3);
	}
	if(hand) {
		dx=VecLenf(f3,f2);
		dy=VecLenf(f3,f1);
		if(dx==0) dx=1;
		if(dy==0) dy=1;
		lenfac=dx/dy;
		act= -1; /* dan geen beperking in x bij uiteinden */
	}

	while(getbutton(RIGHTMOUSE)) {
		ButtonsGetmouse(mval);
		if(mval[0]!=xm || mval[1]!=ym) {
			xm= (mval[0]);
			ym= (mval[1]);
			dx= (xn-xm)/facx;
			dy= (yn-ym)/facy;
			if(act==0 || act==bez->cp-1) dx=0;
			fact[0]= f1[0]-dx; 
			fact[1]= f1[1]-dy;
			if(hand==0 || G.actp2==0 || G.actp2==bez->cp-1) {
				if(fact[0]<IPOMINX) fact[0]=IPOMINX;
				else if(fact[0]>IPOMAXX) fact[0]=IPOMAXX;
				if(fact[1]<IPOMINY) fact[1]=IPOMINY;
				else if(fact[1]>IPOMAXY) fact[1]=IPOMAXY;
				dx=f1[0]-fact[0];
				dy=f1[1]-fact[1];
			}
			if(hand== -1) if((G.qual & 3)==0) {
				*(fact+6)= f2[0]+lenfac*dx;
				*(fact+7)= f2[1]+lenfac*dy;
			}
			if(hand== 1) if((G.qual & 3)==0) {
				*(fact-6)= f2[0]+lenfac*dx;
				*(fact-5)= f2[1]+lenfac*dy;
			}
			if(hand== 0) {
				*(fact+3)= f3[0]-dx;
				*(fact+4)= f3[1]-dy;
				*(fact-3)= f2[0]-dx;
				*(fact-2)= f2[1]-dy;
			}
			tekenspline2d(0);
		}
		else qualafhandeling(&G.qual);
	}
	schrijfipospline(0);
}

void insertsplinepunt2d()
{
	struct Bezier *bez;
	float *f,ofsx,facx,ofsy,facy;
	short a,act,dist=1000,len,mdev[2],mval[2];

	ButtonsGetmouse(0);
	ButtonsGetmouse(mval);
	
	if(okee("Insert point")==0) return;
	bez=G.spline;
	if(bez->cp >399) return;

	ofsx= (IPOMAXX*spv_x1-IPOMINX*spv_x2)/(IPOMAXX-IPOMINX);
	facx= IPOMINX/(spv_x1-ofsx);
	ofsy= (IPOMAXY*spv_y1-IPOMINY*spv_y2)/(IPOMAXY-IPOMINY);
	facy= IPOMINY/(spv_y1-ofsy);

	mval[0]= (mval[0])/facx+ofsx;
	mval[1]= (mval[1])/facy+ofsy;

	act=G.actp2;
	if(act>=bez->cp) act=bez->cp-1;
	if(act==0) act=1;
	f=(float *)(bez+1);
	f+= 9*act;
	memcpy(f+9,f,36*(bez->cp-act));
	f[3]=mval[0];
	f[4]=mval[1];
	bez->cp++;
	calchandle(bez,act,0);
	tekenspline2d(0);
	schrijfipospline(0);
}

void delsplinepunt2d()
{
	struct Bezier *bez;
	float *f;
	short a,act,dist=1000,len,mdev[2],mval[2];

	ButtonsGetmouse(0);
	ButtonsGetmouse(mval);
	
	if(okee("Delete point")==0) return;
	bez=G.spline;
	if(G.actp2==0 || G.actp2==bez->cp-1 || bez->cp<=2) return;

	act=G.actp2;
	f=(float *)(bez+1);
	f+= 9*act;
	memcpy(f,f+9,36*(bez->cp-act));
	bez->cp--;
	tekenspline2d(0);
	schrijfipospline(0);
}

void bezier_snapmenu()
{
	struct Bezier *bez;
	struct Key *key=0, *tkey, *snapkey= 0;
	float *fp, ykey, yipo, mindist;
	short event;
	
	bez= G.spline;
	if(G.actp2<0 || G.actp2>=bez->cp) return;

	event= pupmenu("SNAP Bezier%t|Horizontal%x1|To Key%x2|To Next%x3");
	
	if(event>0) {
		fp= (float *)(bez+1);
		fp+= 9*G.actp2+3;	/* middelste punt van de triple */
	
		if(event==1) {
			fp[-2]= fp[1];
			fp[4]= fp[1];
		}
		else if(event==2) {
			/* dichtstbijzijnde key */
			
			if(G.ipomode==20) key= G.basact->key;
			else if(G.ipomode==21) key= G.basact->pkey;
			else if(G.ipomode==22) {
				if(G.basact->soort==1) key= ((struct ObData *)G.basact->d)->vv->key;
				else if ELEM(G.basact->soort, 5, 11) key= ((struct ObData *)G.basact->d)->cu->key;
			    else if(G.basact->soort== -2) key= ((struct MoData *)G.basact->d)->key;
			    else if(G.basact->soort== -4) key= ((struct IkaData *)G.basact->d)->key;
			}
			if(key) {
				yipo= fp[1];
				
				tkey= key;
				mindist= 100.0;
				while(tkey) {
					ykey= tkey->pos*(IPOMAXY-IPOMINY)+IPOMINY;
					if( fabs(yipo-ykey) < mindist ) {
						snapkey= tkey;
						mindist= fabs(yipo-ykey);
					}
					tkey= tkey->next;
				}
				if(snapkey) {
					fp[-2]= snapkey->pos*(IPOMAXY-IPOMINY)+IPOMINY;
					fp[1]= fp[-2];
					fp[4]= fp[-2];
				}
			}
			
		}
		else if(event==3) {
			if(G.actp2<bez->cp-1 && G.actp2>0) {
				fp[-2]= fp[10];
				fp[1]= fp[10];
				fp[4]= fp[10];
			}
		}
		schrijfipospline(1);
		tekenspline2d(0);
	}
}



short findzero(x,q0,q1,q2,q3,o)
float x,q0,q1,q2,q3,*o;
{
	double c0,c1,c2,c3,a,b,c,p,q,d,t,phi,Sqrt3d();
	short nr=0;

	c0=q0-x;
	c1=3*(q1-q0);
	c2=3*(q0-2*q1+q2);
	c3=q3-q0+3*(q1-q2);
	
	if(c3!=0.0) {
		a=c2/c3;
		b=c1/c3;
		c=c0/c3;
		a=a/3;

		p=b/3-a*a;
		q=(2*a*a*a-a*b+c)/2;
		d=q*q+p*p*p;

		if(d>0.0) {
			t= sqrt(d);
			o[0]= Sqrt3d(-q+t)+Sqrt3d(-q-t)-a;
			if(o[0]>=SMALL && o[0]<=1.000001) return 1;
			else return 0;
		} else if(d==0.0) {
			t= Sqrt3d(-q);
			o[0]= 2*t-a;
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= -t-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		} else {
			phi= acos(-q/sqrt(-(p*p*p)));
			t= sqrt(-p);
			p= cos(phi/3);
			q= sqrt(3-3*p*p);
			o[0]= 2*t*p-a;
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= -t*(p+q)-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) nr++;
			o[nr]= -t*(p-q)-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		}
	} else {
		a=c2;
		b=c1;
		c=c0;
		
		if(a!=0.0) {
			p=b*b-4*a*c;
			if(p>0) {
				p= sqrt(p);
				o[0]= (-b-p)/(2*a);
				if(o[0]>=SMALL && o[0]<=1.000001) nr++;
				o[nr]= (-b+p)/(2*a);
				if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
				else return nr;
			} else if(p==0) {
				o[0]= -b/(2*a);
				if(o[0]>=SMALL && o[0]<=1.000001) return 1;
				else return 0;
			}
		} else if(b!=0.0) {
			o[0]= -c/b;
			if(o[0]>=SMALL && o[0]<=1.000001) return 1;
			else return 0;
		} else if(c==0.0) {
			o[0]=0;
			return 1;
		}
		return 0;	
	}
}

void berekeny(f,o,b)
float *f,*o;
short b;
{
	float t,c0,c1,c2,c3;
	short a;

	c0=f[1];
	c1=3*(f[4]-f[1]);
	c2=3*(f[1]-2*f[4]+f[7]);
	c3=f[10]-f[1]+3*(f[4]-f[7]);
	for(a=0;a<b;a++) {
		t=o[a];
		o[a]=c0+t*c1+t*t*c2+t*t*t*c3;
	}
}
void berekenx(f,o,b)
float *f,*o;
short b;
{
	float t,c0,c1,c2,c3;
	short a;

	c0=f[0];
	c1=3*(f[3]-f[0]);
	c2=3*(f[0]-2*f[3]+f[6]);
	c3=f[9]-f[0]+3*(f[3]-f[6]);
	for(a=0;a<b;a++) {
		t=o[a];
		o[a]=c0+t*c1+t*t*c2+t*t*t*c3;
	}
}

float ipo(bez,fac)
struct Bezier *bez;
float fac;
{
	float *f,min,max,opl[32],*o,x,t;
	short a,b,aantal=0;

	if(bez==0) return fac;
	if(fac<0.0) fac=0.0;
	else if(fac>1.0) fac= 1.0;

	x=IPOMINX+fac*(IPOMAXX-IPOMINX);
	o= opl;
	f=(float *)(bez+1);
	f+=3;
	o[0]= -12; o[1]= -12;
	for(a=0;a<bez->cp-1;a++,f+=9) {
		min= MIN4(f[0],f[3],f[6],f[9]);
		max= MAX4(f[0],f[3],f[6],f[9]);
		if(min<=x && max>=x) {
			b=findzero(x,f[0],f[3],f[6],f[9],o);
			if(b) {
				berekeny(f,o,b);
				aantal+=b;
				o+=b;
				if(aantal>30) break;
			}
		}
	}
	if(aantal==0) return fac;
	x=opl[0];
	if(aantal>1) {
		for(a=1;a<aantal;a++) {
			if(x>opl[a]) x=opl[a];
		}
	}
	t=(x-IPOMINY)/(IPOMAXY-IPOMINY);
	if(t<0.0) t=0; else if(t>1.0) t=1.0;
	return t;
}

float ipoinv(bez,fac)
struct Bezier *bez;
float fac;
{
	float *f,min,max,opl[32],*o,x,t;
	short a,b,aantal=0;

	if(bez==0) return fac;
	x=IPOMINY+fac*(IPOMAXY-IPOMINY);
	o= opl;
	f=(float *)(bez+1);
	f+=3;
	o[0]= -12; o[1]= -12;
	
	if(bez->cp<1 || bez->cp>1000) {
		error("error in ipoinv");
		printf("error in ipoinv: bez: %x bez->cp: %d\n", bez, bez->cp);
	}
	
	for(a=0;a<bez->cp-1;a++,f+=9) {
		min= MIN4(f[1],f[4],f[7],f[10]);
		max= MAX4(f[1],f[4],f[7],f[10]);
		if(min<=x && max>=x) {
			b=findzero(x,f[1],f[4],f[7],f[10],o);
			if(b) {
				berekenx(f,o,b);
				aantal+=b;
				o+=b;
				if(aantal>30) break;
			}
		}
	}
	if(aantal==0) return fac;
	x=opl[0];
	if(aantal>1) {
		for(a=1;a<aantal;a++) {
			if(x>opl[a]) x=opl[a];
		}
	}
	t=(x-IPOMINX)/(IPOMAXX-IPOMINX);
	if(t<0.0) t=0; else if(t>1) t=1;
	return t;
}



