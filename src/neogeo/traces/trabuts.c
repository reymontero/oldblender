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

/*	trabuts.c	*/


/*	 
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

	shadesphere(cb)
	MinMaxRGB2(col)
	shadeshadesphere(cb,n,col,x,y)
	shadesky()

	donaambutton()
	dobasebutton()
	setbuttex()
	bladertex()

	workfieldbuttons()

	textubuts(event)
	wrldbuts(event)
	objbuts(event)
	movebuts(event)
	displaybuts(event)

	tekentextubuts(mode)	 0= met clearwindow, 1=alleen buts dus
	tekenviewbuts(mode)	 
	tekenlampbuts(mode)
	tekenobjbuts(mode)
	tekenwrldbuts(mode)
	tekendisplaybuts(mode)
	tekenmovebuts(mode)
	tekenmainbuts(mode)	 0= altijd, 1=eerste keer (initfont!) 

short	button(&var,min,max,str)

*/


#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <fmclient.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "/usr/people/include/Trace.h"
#include "/usr/people/include/Button.h"

#define SHLX (-.577349)
#define SHLY (-.577349)
#define SHLZ (0.577349)

#define LX 124*SHLX
#define LY 124*SHLY
#define LZ 124*SHLZ



/* ***************  globals **************** */

extern struct ListBase editNurb;
extern void setmass_ika(float mass);

struct Base **ibase;
struct ColBlck undocolb,*colb;
struct LaData *la;
struct But *butarcol,*buttexbut,*DefBut();

struct BodyPoint *bopsel; /* allevier uit editika.c */
struct Rod *rodsel;	
struct Joint *jointsel=0;
struct Skeleton *skelsel=0;

float editbutweight=1.0, setmassbut=1.0;
short texture=0,buttex=0,septex,actex,texco,zoom=0,lampdist,miststa;
short xof=10,butcol=0,baselay,turn=1,step=18,degr=360,doublimit=3,editbutflag=15,vfontbut=0;
short extr_offs= 1000;
short rodedit= 0, setwidthbut= 500;
short panoramaview= 0,panodegr=0, splineorder=4;
char naam[18],ibnaam[18];
char texstr[15][8]={
	"None","Color","Wood","Marble","Magic",
	"Image","Tiles","Blend","Stucci","Noise","","","","",""};
char textustr[15][10], *actexp,imaname[100], *vfontbutstr=0;

/* ***************          **************** */


void shadeshadesphere(n,col,x,y)
short x,y,*col;
float *n;
{
	float v1,inp, inprspec, isr=0.0, isb=0.0, isg=0.0, i;
	float v[3],Spec();
	long c1, c2;
	extern float Titot;
	short a,b,c, tracol;

	if(R.col.kspec || (texco & 1) ) {
		v[0]= x+LX; 
		v[1]= y+LY; 
		v[2]= LZ+128;
		Normalise(v);
	}

	if(R.col.tex[0]) {  /*texture afhandeling;*/

		if(texco & 60) {
			if(R.col.mode & 1) {
				i=(float)(2<<(14-zoom));
				R.lo[0]= i*n[0];
				R.lo[1]= i*n[2];
				R.lo[2]= -i*n[1];
			} else {
				R.lo[0]= x<<(9-zoom);
				R.lo[1]= y<<(9-zoom);
				R.lo[2]= 0;
			}
			if(texco & 56) {
				R.uv[0]=R.co[0]=R.gl[0]=R.lo[0];
				R.uv[1]=R.co[1]=R.gl[1]=R.lo[1];
				R.co[2]=R.gl[2]=R.lo[2];
			}
		}
		if(texco & 2) {
			R.orn[0]= 32767*n[0];
			R.orn[1]= 32767*n[1];
			R.orn[2]= 32767*n[2];
		}
		if(texco & 1) {
			i= -2*(n[0]*v[0]+n[1]*v[1]+n[2]*v[2]);
			R.ref[0]= 32767*(v[0]+i*n[0]);
			R.ref[1]= 32767*(v[1]+i*n[1]);
			R.ref[2]= 32767*(v[2]+i*n[2]);
		}
		Titot= 1.0;
		if(septex) {
			if(R.col.tex[buttex]) objectex(&R.col,buttex);
		}
		else {
			if(R.col.tex[0]) objectex(&R.col,0);
			if(R.col.tex[1]) objectex(&R.col,1);
			if(R.col.tex[2]) objectex(&R.col,2);
			if(R.col.tex[3]) objectex(&R.col,3);
		}
	}

	if (R.col.mode & 16) {
		col[0]= 255.0*R.col.r;
		col[1]= 255.0*R.col.g;
		col[2]= 255.0*R.col.b;
	}
	else {
		inp= n[0]*SHLX+n[1]*SHLY+n[2]*SHLZ;

		if(R.col.kspec)  {
			if(inp<=0) inp=0;
			else {
				v1=v[0]*n[0]+v[1]*n[1]+v[2]*n[2];
				if(v1>0) {
					v1= Spec(v1,R.col.spec);
					inprspec= (v1*R.col.kspec);
					isr= inprspec*R.col.specr;
					isg= inprspec*R.col.specg;
					isb= inprspec*R.col.specb;
				}
			}
		}
		inp= (R.col.kref*inp + R.col.emit);
		col[0]= 255.0*(R.col.ambr+inp*R.col.r + isr);
		col[1]= 255.0*(R.col.ambg+inp*R.col.g + isg);
		col[2]= 255.0*(R.col.ambb+inp*R.col.b + isb);
	}

	if(R.col.tra!=0.0) {
		if(R.col.mode & 128) {	/* ztra shade */
			if(R.col.ang!=0.0) {
				i= R.col.ang*(isr+isb+isg)/3.0;
				if(i>1.0) i= 1.0;
				R.col.tra*= (1.0-i);
			}
		}

		tracol=  64+100*(abs(x)>abs(y));
		tracol= (R.col.tra*tracol);
		col[2]= tracol+ (col[2]*(1.0-R.col.tra)) ;
		col[1]= tracol+ (col[1]*(1.0-R.col.tra)) ;
		col[0]= tracol+ (col[0]*(1.0-R.col.tra)) ;

	}

	if(col[0]<4) col[0]= 4;
	else if(col[0]>247) col[0]= 247;
	if(col[1]<4) col[1]= 4;
	else if(col[1]>247) col[1]= 247;
	if(col[2]<4) col[2]= 4;
	else if(col[2]>247) col[2]= 247;
}


void MinMaxRGB2(col)
register short *col;
{
	if(col[0]<4) col[0]=4;
	else if(col[0]>247) col[0]=247;
	if(col[1]<4) col[1]=4;
	else if(col[1]>247) col[1]=247;
	if(col[2]<4) col[2]=4;
	else if(col[2]>247) col[2]=247;
}

void shadesphere(c)
struct ColBlck *c;
{
	extern struct Osa O;
	static struct ColBlck *cc;
	float *n,z1;
	float rsq,xsq,ysq,zsq,tmat[4][4],imat[3][3];
	unsigned long line[2*SHADESPHEREX+10],dit[4],*temp;
	unsigned long eline[2*SHADESPHEREX+10],oline[2*SHADESPHEREX+10];
	short x1,y1,x,y,yof,xof2,col[3],toets,val, map, norm;
	char *s, copybuf[64];

	if(c==0 || c==cc) {
		if(G.zbuf && G.basact) {
			if(G.basact->soort & 1) {
				if( ((struct ObData *)G.basact->d)->dt==2) {
					shadeDispList(G.basact);
					tekenbase_ext(G.basact);
				}
			}
		}
	}

	if(c==0) c=cc;
	cc=c;
	if(cc==0) return;

	convertColBlck(&R.col, cc);
	initrendertexture();
	if(actex && G.adrtex[actex]->soort==5) {
		if(G.adrtex[actex]->ima) {
			strcpy(imaname,G.adrtex[actex]->ima->name);
			SetButs(307, 307);
		}
	}
	
	n= R.vn;
	/* dither */
	if(G.machine==IRIS) {
		dit[1]=0; 
		dit[0]=0x08080808;
		dit[2]=0xFCFCFCFC; 
		dit[3]=0x04040404;
	} else {
		dit[0]= dit[1]= dit[2]= dit[3]= 0;
	}
	temp= oline;
	for(x= -SHADESPHEREX;x<=SHADESPHEREX;x+=2) {
		*(temp++)=dit[0];
		*(temp++)=dit[1];
	}
	temp= eline;
	for(x= -SHADESPHEREX;x<=SHADESPHEREX;x+=2) {
		*(temp++)=dit[2];
		*(temp++)=dit[3];
	}

	z1= R.col.kref*R.col.amb;
	z1/= 255.0;
	R.col.ambr= z1*wrld.ambr;
	R.col.ambg= z1*wrld.ambg;
	R.col.ambb= z1*wrld.ambb;

	yof=SHADESPHEREY+10;
	xof2=xof+(2*SHADESPHEREX);
	winset(G.winar[0]);

	persp(0);

	frontbuffer(1);
	rsq= (SHADESPHEREX-1)*(SHADESPHEREX-1);

	map= cc->map[0] |cc->map[2] | cc->map[4] | cc->map[6];
	texco= (map>>9);

	R.col.copysize= 0;
	if(texco & 63) {
		if(map & 256) R.col.copysize= 16;    /* a r g b */
		if(map & 127) R.col.copysize= 64;
	}
	if(R.col.copysize<40) {
		if(R.col.mode & 128)		/* ztra shade en spec tra */
			if(R.col.ang>0.0) R.col.copysize= 40;
	}
	if(R.col.copysize) memcpy(copybuf, &R.col, R.col.copysize);

	/* imat goedzetten */
	for(y=0;y<4;y++) {
		if(cc->map[y] & 16384) {
			if(cc->base[y]==0) cc->base[y]= G.basact;
		}
		if(cc->base[y]) {
			Mat3One(cc->base[y]->imat);
			cc->base[y]->ivec[0]=cc->base[y]->ivec[1]=cc->base[y]->ivec[2]=0;
		}
	}
	/* texture frequentie */
	R.osatex= 0;
	R.vlr= 0;	/* wordt in textu.c cubemap() gebruikt */
	/* for(y=0; y<4; y++) if( G.adrtex[ cc->tex[y] ]->soort==5 ) R.osatex= 1; */
	if(R.osatex) {
		O.dxlo[0]= (32767>>zoom)/(float)SHADESPHEREX;
		O.dxlo[1]= 0.0;
		O.dxlo[2]= 0.0;
		O.dylo[0]= 0.0;
		O.dylo[1]= (32767>>zoom)/(float)SHADESPHEREY;
		O.dylo[2]= 0.0;
		VECCOPY(O.dxgl, O.dxlo);
		VECCOPY(O.dygl, O.dylo);
		if(texco & 2) {
			O.dxno[0]= 1.0/(float)SHADESPHEREX;
			O.dxlo[1]= 0.0;
			O.dxlo[2]= 0.0;
			O.dylo[0]= 0.0;
			O.dyno[1]= 1.0/(float)SHADESPHEREY;
			O.dyno[2]= 0.0;
		}
		VECCOPY(O.dxref, O.dxlo);
		VECCOPY(O.dyref, O.dylo);
	}

	/* begin met zwart */
	cpack(0); 
	sboxs(xof,-SHADESPHEREY-1+yof,xof2+2,-SHADESPHEREY+2+yof);

	for(y= -SHADESPHEREY;y<SHADESPHEREY;y++) {
		ysq=y*y;

		s= (char *)line;
		if(y & 1) memcpy(line,oline,8*SHADESPHEREX+4);
		else memcpy(line,eline,8*SHADESPHEREX+4);

		toets=qtest();  /* escapes */
		if(toets) {
			toets= traces_qread(&val);
			if(toets!=MOUSEX && toets!=MOUSEY) {
				if(val){
					frontbuffer(0);
					qenter(toets,val);
					persp(1);
					return;
				}
			}
		}
		if(cc->mode & 1) {
			for(x= -SHADESPHEREX;x<=SHADESPHEREX;x++) {
				s++;
				xsq=x*x;
				if(xsq+ysq<rsq) {
					n[2]=fsqrt(rsq-xsq-ysq);
					n[0]= -x; 
					n[1]= -y;
					Normalise(n);
					if(R.col.copysize) memcpy(&R.col, copybuf, R.col.copysize);
					shadeshadesphere(n,col,x,y);

					*(s++) +=(char)(col[2]);
					*(s++) +=(char)(col[1]);
					*(s++) +=(char)(col[0]);
				}
				else {
					col[0]= 64+100*(abs(x)>abs(y));

					*(s++) = col[0]; 
					*(s++)=col[0]; 
					*(s++)=col[0];
				}
			}
		}
		else {
			if(y< -SHADESPHEREY+6 || y> SHADESPHEREY-6) {
				for(x= -SHADESPHEREX;x<=SHADESPHEREX;x++) {
					col[0]= 64+100*(abs(x)>abs(y));
					s++;
					*(s++) = col[0]; 
					*(s++)=col[0]; 
					*(s++)=col[0];
				}
			}
			else {
				for(x= -SHADESPHEREX;x<=SHADESPHEREX;x++) {
					if(x< -SHADESPHEREX+6 || x> SHADESPHEREX-6) {
						col[0]= 64+100*(abs(x)>abs(y));
						s++;
						*(s++) = col[0]; 
						*(s++)=col[0]; 
						*(s++)=col[0];
					}
					else {
						/*
						if(y<0) {
							if(x<0) norm= 1;
							else norm= 2;
						}
						else {
							if(y>SHADESPHEREX/2) norm= 0;
							else {
								if(x<0) {
									if(x<-2*y) norm= 1; else norm= 0;
								}
								else {
									if(x>2*y) norm= 1; else norm= 0;
								}
							}
						}
						if(norm== 0) {
							n[0]=0.0; n[1]=0.0; n[2]=1.0;
						}
						else if(norm== 1) {
							n[0]= -1.0; n[1]=0.0; n[2]=0.0;
						}
						else {
							n[0]=0.0; n[1]= -1.0; n[2]=0.0;
						}
						*/
						n[0]=0.0; n[1]=0.0; n[2]=1.0;
						
						if(R.col.copysize) memcpy(&R.col, copybuf, R.col.copysize);
						shadeshadesphere(n,col,x,y);
						s++;
						*(s++) +=(char)(col[2]);
						*(s++) +=(char)(col[1]);
						*(s++) +=(char)(col[0]);

					}
				}
			}
		}

		lrectwrite(xof,y+yof,xof2,y+yof,line);
		cpack(0);
		sboxs(xof,y+2+yof,xof2+2,y+2+yof);
	}

	persp(1);
	frontbuffer(0);
	if(y==SHADESPHEREY) xof+=126;
	if(xof>S.vxw-30) xof=10;
	R.osatex= 0;
}

void shadesky()
{
	float bmat[4][4],imat[3][3], f1, viewy, papery;
	unsigned long line[2*SHADESPHEREX+10],dit[4],*temp;
	unsigned long eline[2*SHADESPHEREX+10],oline[2*SHADESPHEREX+10];
	short x1,y1,x,y,yof,xof2,toets,val;
	char *s,col[4];

	/* dither */
	dit[1]=0; 
	dit[0]=0x08080808;
	dit[2]=0xFCFCFCFC; 
	dit[3]=0x04040404;
	temp= oline;
	for(x= -SHADESPHEREX;x<=SHADESPHEREX;x+=2) {
		*(temp++)=dit[0];
		*(temp++)=dit[1];
	}
	temp= eline;
	for(x= -SHADESPHEREX;x<=SHADESPHEREX;x+=2) {
		*(temp++)=dit[2];
		*(temp++)=dit[3];
	}

	R.d= (SHADESPHEREX*view0.lens)/16;
	R.ycor= R.yasp;
	R.ycor= R.ycor/R.xasp;

	yof=SHADESPHEREY+10;
	xof2=xof+(2*SHADESPHEREX);
	winset(G.winar[0]);
	getmatrix(bmat);
	wrld.grvec[0]=bmat[2][0];
	wrld.grvec[1]=bmat[2][1];
	wrld.grvec[2]=bmat[2][2];
	Normalise(wrld.grvec);

	Mat3CpyMat4(G.workbase->imat,bmat);
	Mat3Inv(bmat,G.workbase->imat);
	Mat3CpyMat3(G.workbase->imat,bmat);

	wrld.fs &= ~4;
	if(wrld.stex[0]) wrld.fs|=4;
	initrendertexture();

	for(y=0;y<4;y++) {
		if(wrld.sbase[y]) {
			parentbase(wrld.sbase[y],bmat,imat);
			Mat3CpyMat4(imat,bmat);
			Mat3Inv(wrld.sbase[y]->imat,imat);
			wrld.sbase[y]->ivec[0]=wrld.sbase[y]->ivec[1]=wrld.sbase[y]->ivec[2]=0;
		}
	}

	persp(0);

	frontbuffer(1);
	for(y= -SHADESPHEREY;y<SHADESPHEREY;y++) {

		s= (char *)line;
		if(y & 1) memcpy(line,oline,8*SHADESPHEREX+4);
		else memcpy(line,eline,8*SHADESPHEREX+4);

		toets=qtest();  /* escapes */
		if(toets) {
			toets= traces_qread(&val);
			if(toets!=MOUSEX && toets!=MOUSEY) {
				if(val){
					frontbuffer(0);
					qenter(toets,val);
					persp(1);
					return;
				}
			}
		}
		viewy= R.ycor*y;
		papery= ((float)y)/(SHADESPHEREY); 
		for(x= -SHADESPHEREX;x<=SHADESPHEREX;x++) {
			if( (wrld.fs & 7)==0 ) {
				col[3]= wrld.horr;
				col[2]= wrld.horg;
				col[1]= wrld.horb;
			} else {
				R.view[0]= x;
				if(wrld.fs & 8) {	/* paper */
					R.view[0]/= SHADESPHEREX; 
					R.view[1]= papery;
					R.view[2]= 0.0;
				}
				else {
					R.view[1]= viewy;
					R.view[2]= -R.d;
					Normalise(R.view);
				}
				if(zoom) {
					f1= (2<<(zoom-1));
					R.view[0]/=f1;
					R.view[1]/=f1;
				}
				sky(col);
			}
			s++;
			if(col[1]<4) *(s++)+=4; 
			else if(col[1]>247) *(s++)+=247; 
			else *(s++)+=col[1];
			if(col[2]<4) *(s++)+=4; 
			else if(col[2]>247) *(s++)+=247; 
			else *(s++)+=col[2];
			if(col[3]<4) *(s++)+=4; 
			else if(col[3]>247) *(s++)+=247; 
			else *(s++)+=col[3];
		}

		lrectwrite(xof,y+yof,xof2,y+yof,line);
		cpack(0); 
		sboxs(xof,y+2+yof,xof2+2,y+2+yof);
	}
	persp(1);
	frontbuffer(0);
	if(y==SHADESPHEREY) xof+=126;
	if(xof>S.vxw-30) xof=10;

}

void shadenep()
{
	if(G.winar[0]) {

		if(G.mainb==6) shadesky();
		else if(G.mainb==5) shadesphere(0);
	}
}

/* *********************************************************************** */


void donaambutton()
{
	short a, len, w=0;
	char str[18];

	if(G.basact==0) return;
	strcpy(str, naam);
	len= strlen(str);
	for(a=len-2; a>=0; a--) {
		if(str[a]=='.') {
			if( isdigit(str[a+1]) ) {
				str[a]=0;
				w= atoi(str+a+1);
				break;
			}
		}
	}
	strcpy(G.basact->str,str);
	G.basact->nnr= w;
}


void dobasebutton()
{
	struct Base *b;
	short a, len, w=0, ok=0;
	char str[18];

	strcpy(str, ibnaam);
	len= strlen(str);
	for(a=0;a<len-1;a++) {		/* eerst naam splitsen */
		if(str[a]=='.' && isdigit(str[a+1])) {
			str[a]= 0;
			w= atoi(str+a+1);
			break;
		}
	}
	
	b= G.firstbase;
	while(b) {
		if(strncmp(b->str,str,14)==0) {
			if(w== b->nnr) {
				ok=1;
				ibase[buttex]=b;
			}
		}
		b=b->next;
	}
	if(ok==0) {
		ibase[buttex]=0;
		strcpy(ibnaam,"");
		SetButs(67,67);
	}
}

struct Base *dobasebutton2()	    /* werkt met string ibnaam, als ibnaam bestaat return base */
{
	struct Base *b;
	short a,len,w=0;
	char str[18];

	strcpy(str,ibnaam);
	len= strlen(str);
	for(a=0;a<len;a++) {		/* eerst naam splitsen */
		if(str[a]=='.') {
			str[a]=0;
			if(a<len) w= atoi(str+a+1);
			break;
		}
	}
	b=G.firstbase;
	while(b) {
		if(strncmp(b->str,str,14)==0) {
			if(w== b->nnr) {
				return (b);
			}
		}
		b=b->next;
	}
	strcpy(ibnaam,"");
	return 0;
}

short calcbaselay()
{
	short a;

	if(G.basact==0) return 0;
	if(G.basact->lay== 0xFFFF) return 0;
	for(a=0;a<16;a++) {
		if(BTST(G.basact->lay,a)) return a+1;
	}
	return 1;
}

void dobaselaybutton()
{

	if(G.basact==0) return;
	G.basact->lay=0;
	if(baselay==0) G.basact->lay= -1;
	else G.basact->lay= BSET(G.basact->lay,baselay-1);
	projektie();
}

void setbuttex()
{
	struct But *but;
	short a,b;
	char *t;

	if(G.mainb==6) t=wrld.stex;
	else if(G.mainb==5) t=colb->tex;
	else if(G.mainb==4) t=la->tex;
	else return;

	but= buttexbut;
	for(a=0;a<4;a++) {
		sprintf(but->str,"%d ",*t);
		if(*t==0) b=0; 
		else b=G.adrtex[*t]->soort;
		strcat(but->str,texstr[b]);
		t++;
		but++;
	}
	if(G.mainb==6) SetButs(610,610);
	if(G.mainb==5) SetButs(210,210);
	if(G.mainb==4) SetButs(410,410);
}

void bladertex()
{
	void tekentextubuts();
	Device mdev[2];
	short mval[2],xo,texo;
	float fac;

	mdev[0]=MOUSEX; 
	mdev[1]=MOUSEY;

	getdev(2,mdev,mval);
	xo=mval[0];
	texo=actex;
	while(getbutton(LEFTMOUSE)) {
		getdev(2,mdev,mval);
		if(mval[0]!=xo) {
			fac=mval[0]-xo;
			fac= fac*.1;
			actex= texo+ffloor(fac+.5);
			if(actex<0) actex=0;
			else if(actex>G.totex) actex=G.totex;
			if(actex!= *actexp) {
				*actexp=actex;
				setbuttex();
			}
		}
	}
	if(texo!=actex) {
		if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
		else inittexture(0,0);
		tekentextubuts(1);
	}
}

short give_vfontnr(vfont)
struct VectorFont *vfont;
{
	struct VectorFont *vf;
	short nr= 0;

	vf= (struct VectorFont *)G.vfbase.first;
	while(vf) {
		if(vf==vfont) return nr;
		nr++;
		vf= vf->next;
	}
	return -1;
}

struct VectorFont *give_vfontpointer(nr)	/* nr= button */
short nr;
{
	struct VectorFont *vf;
	short tel= 0;

	vf= G.vfbase.first;
	while(vf) {
		if(tel==nr) return vf;
		tel++;
		vf= vf->next;
	}
	vf= (struct VectorFont *)G.vfbase.first;
	return vf;
}

struct VectorFont *exist_vfont(str)
{
	struct VectorFont *vf;

	vf= G.vfbase.first;
	while(vf) {
		if(strcmp(vf->name,str)==0) return vf;
		vf= vf->next;
	}
	return 0;
}

void set_vfontbutstr()
{
	struct VectorFont *vf;
	int len= 0;
	char di[100], fi[100];

	if(vfontbutstr) freeN(vfontbutstr);	
	vf= G.vfbase.first;
	while(vf) {
		strcpy(di, vf->name);
		splitdirstring(di, fi);
		len+= strlen(fi)+3;
		vf= vf->next;
	}
	
	vfontbutstr= callocN(len+21, "vfontbutstr");
	strcpy(vfontbutstr, "FONTS %t");
	vf= G.vfbase.first;
	while(vf) {
		strcat(vfontbutstr, " | ");
		
		strcpy(di, vf->name);
		splitdirstring(di, fi);
		if( strlen(fi)>7 ) {
			if(strcmp( fi+strlen(fi)-7, ".pdfont")==0) {
				fi[strlen(fi)-7]= 0;
			}
		}
		
		strcat(vfontbutstr, fi);
		vf= vf->next;
	}
}

/* *********************************************************************** */


void viewbuts(event)
short event;
{
	extern struct BGpic bgpic;
	long val;
	char str[100];

	switch(event) {
	case 151:
		loadorfreeBGpic();
		break;
	case 152:
		bgpic.dproj= 0;
		projektie();
		break;
	case 153:
		strcpy(str,bgpic.name);
		convertstringcode(str);
		val= ffileselect("Load Background",str);
		if(val && strlen(str)<80 ) {
			if(bgpic.name[1]=='/') makestringcode(str);
			strcpy(bgpic.name,str);
			SetButs(event, event);
		}
		
	}
}

int count_tex_users(int actex)
{
	struct Base *base;
	struct ObData *ob;
	struct LaData *la;
	struct ColBlck *col;
	int c, users= 0;

	base= G.firstbase;
	while(base) {
		if(base->soort & 1) {
			ob= (struct ObData *)(base->d);
			col= (struct ColBlck *)(ob+1);
			for(c=0; c<ob->c; c++, col++) {
				if(col->tex[0]==actex) users++;
				if(col->tex[1]==actex) users++;
				if(col->tex[2]==actex) users++;
				if(col->tex[3]==actex) users++;
			}
		}
		else if(base->soort==2) {
			la= (struct LaData *)(base->d);
			if(la->tex[0]==actex) users++;
			if(la->tex[1]==actex) users++;
			if(la->tex[2]==actex) users++;
			if(la->tex[3]==actex) users++;
		}
		base= base->next;
	}
	if(wrld.stex[0]==actex) users++;
	if(wrld.stex[1]==actex) users++;
	if(wrld.stex[2]==actex) users++;
	if(wrld.stex[3]==actex) users++;
	
	return users;
}

void shuffle_char(char *adr, int val)
{
	/* uit array (textures) vervalt een nummer */
	
	if(*adr==0) return;
	
	if(*adr == val) *adr= 0;
	else if(*adr > val) (*adr)--;
}

void textubuts(event)
short event;
{
	void tekentextubuts();
	struct Base *base;
	struct ObData *ob;
	struct LaData *la;
	struct ColBlck *col;
	static int doitflag= 1;
	int *temp, c, erased= 0;
	short val;
	char str[100];

	switch(event) {
	case 301: 	/*texture */
		if(actex>0 && G.adrtex[actex]->soort==5) {
			delImap(G.adrtex[actex]);
		}
		inittexture(actex,texture);
		setbuttex();
		tekentextubuts(1);
		break;
	case 302: 	/* del unused */
		doitflag= 0;
		
		for(actex=1; actex<=G.totex; actex++) {
			if(count_tex_users(actex)==0) {
				erased++;
				textubuts(305);
				actex--;
			}
		}
		actex= *actexp;
		tekentextubuts(1);
		setbuttex();
		sprintf(str, "Erased %d", erased);
		okee(str);
		doitflag= 1;
		break;
	case 303: 	/*save */
		break;
	case 304: 	/*new */
		if(G.totex>= MAXTEX) {
			error("Too many textures");
			return;
		}
		memcpy(G.adrtex[0],G.adrtex[actex],sizeof(struct Tex));
		G.totex++;
		actex=G.totex;
		*actexp=actex;
		G.adrtex[G.totex]=(struct Tex *)mallocN(sizeof(struct Tex),"textublok");
		memcpy(G.adrtex[G.totex],G.adrtex[0],sizeof(struct Tex));
		if(G.adrtex[actex]->soort==5) {
			if(G.adrtex[actex]->ima) G.adrtex[actex]->ima->us++;
		}
		tekentextubuts(1);
		setbuttex();
		break;
	case 305: 	/*del */
		if(actex) {
			if(G.adrtex[actex]->soort==5) delImap(G.adrtex[actex]);
			
			/* kopieeren en vrijgeven */
			freeN(G.adrtex[actex]);
			for(c= actex; c<G.totex; c++) {
				G.adrtex[c]= G.adrtex[c+1];
			}
			
			base= G.firstbase;
			while(base) {
				if(base->soort & 1) {
					ob= (struct ObData *)(base->d);
					col= (struct ColBlck *)(ob+1);
					for(c=0; c<ob->c; c++, col++) {
						shuffle_char( &col->tex[0], actex);
						shuffle_char(&col->tex[1], actex);
						shuffle_char(&col->tex[2], actex);
						shuffle_char(&col->tex[3], actex);
					}
				}
				else if(base->soort==2) {
					la= (struct LaData *)(base->d);
					shuffle_char(la->tex, actex);
					shuffle_char(la->tex+1, actex);
					shuffle_char(la->tex+2, actex);
					shuffle_char(la->tex+3, actex);
				}
				base= base->next;
			}
			shuffle_char(wrld.stex, actex);
			shuffle_char(wrld.stex+1, actex);
			shuffle_char(wrld.stex+2, actex);
			shuffle_char(wrld.stex+3, actex);

			G.totex--;
			
			if(doitflag) {
				actex= 0;
				*actexp= 0;

				tekentextubuts(1);
				setbuttex();
			}
		}
		break;
	case 306: 	/*UNDO */
		if(actex!=0) {
			/* mogelijk imawrap vrijgeven */
			if(G.adrtex[actex]->soort==5) {
				if(G.adrtex[actex]->ima != G.adrtex[0]->ima)
					delImap(G.adrtex[actex]);
			}
			temp= (long *)mallocN(sizeof(struct Tex),"textubuts");
			memcpy(temp, G.adrtex[actex], sizeof(struct Tex));
			memcpy(G.adrtex[actex], G.adrtex[0], sizeof(struct Tex));
			memcpy(G.adrtex[0], temp, sizeof(struct Tex));
			freeN(temp);
		}
		setbuttex();
		tekentextubuts(1);
		break;
	case 307: 	/* imastring: ima opnieuw laden */
		if(actex>0 && G.adrtex[actex]->soort==5) {
			addImap(G.adrtex[actex],imaname);
		}
		break;
	case 308:	/* filesel met imastring */
		if(actex>0 && G.adrtex[actex]->soort==5) {
			strcpy(str,imaname);
			if(str[0]!='/') strcpy(str,"/pics/textures/");
			convertstringcode(str);
			val= ffileselect("Load Imagefile",str);
			if(val) {
				if(strlen(str)>=80) {
					error("string too long");
				}
				else {
					if(imaname[1]=='/') makestringcode(str);
					strcpy(imaname,str);
					addImap(G.adrtex[actex], str);
				}
			}
			return;	/* geen shadenep ivm afhandeling redraw en inputchanges */
		}
		break;
	case 309:	/* type */
		if(actex>0 && G.adrtex[actex]->soort==5) {
			if(G.adrtex[actex]->type==3) {	/* field-imap */
				if(G.adrtex[actex]->var[6]) {	/* mipmap */
					error("No field-imagewrap and mipmap");
					G.adrtex[actex]->var[6]= 0;
					SetButs(300, 300);
				}
			}
		}
		break;
	case 310:	/* div */
		if(actex>0) {
			if(G.adrtex[actex]->div[0]==0) G.adrtex[actex]->div[0]= 1;
			if(G.adrtex[actex]->div[1]==0) G.adrtex[actex]->div[1]= 1;
			if(G.adrtex[actex]->div[2]==0) G.adrtex[actex]->div[2]= 1;
		}
	}
	
	if(doitflag) shadenep();
}

void lampbuts(event)
short event;
{
	void tekenlampbuts();
	void tekentextubuts();
	short texo;

	switch(event) {
	case 401:
		la->ld= 1000*lampdist;
		if(G.zbuf) reshadeall();
		projektie();
		break;
	case 410:	/* buttex */
		actexp= &la->tex[buttex];
		actex= *actexp;
		if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
		else inittexture(0,0);
		tekenlampbuts(1); /* doet ook textu en shade! */
		break;
	case 411:
		texo= *actexp;
		bladertex();
		SetButs(411,411);
		break;
	case 415: 
	case 416: 
	case 417:
		(la->map[buttex])&=255;
		(la->map[buttex])=BSET((la->map[buttex]),event-407);
		SetButs(415,417);
		break;
	}
}

void objbuts(event)
short event;
{
	struct ObData *ob;
	struct ColBlck *cb;
	void tekenobjbuts();
	void tekentextubuts();
	long *temp;
	short texo,a;
	unsigned char *rgb;

	switch(event) {
	case 202:	/* modes, modes die niet samengaan veranderen */
		if(colb->mode1 & 256) {	/* Skyshade */
			colb->mode &= ~(16+32+64+128);
			SetButs(202,202);
		}
		break;
	case 203:	/* butcol */
		if(butcol==0) rgb= &colb->r;
		else if(butcol==1) rgb= &colb->specr;
		else rgb= &colb->mirr;
		butarcol->poin= rgb;
		butarcol->a1= (252-butcol);
		(butarcol+1)->poin= rgb+1;
		(butarcol+1)->a1= (252-butcol);
		(butarcol+2)->poin= rgb+2;
		(butarcol+2)->a1= (252-butcol);
		SetButs(206,206);
		return;
	case 204:	/* colact */
		tekenobjbuts(1);
		return;
	case 205:	/* drawt */
		projektie();
		return;
	case 207:	/* copycol */
		if( G.basact!=0 && (G.basact->soort & 1) && G.basact->soort!=3 ) {
			if(okee("Copycol")==0) break;
			ob= (struct ObData *)G.basact->d;
			cb= (struct ColBlck *)(ob+1);
			for(a=1;a<=ob->c;a++) {
				if(a!=G.colact) memcpy(cb,colb,sizeof(struct ColBlck));
				cb++;
			}
		}
		return;
	case 208:	/* undo */
		temp= (long *)mallocN(sizeof(struct ColBlck),"objbuts");
		memcpy(temp,colb,sizeof(struct ColBlck));
		memcpy(colb,&undocolb,sizeof(struct ColBlck));
		memcpy(&undocolb,temp,sizeof(struct ColBlck));
		freeN(temp);
		SetButs(200,299);
		setbuttex();
		break;

	case 210:	/* buttex */
		actexp= &colb->tex[buttex];
		actex= *actexp;
		if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
		else inittexture(0,0);
		tekenobjbuts(1); /* doet ook textu en shade! */
		return;
	case 211:
		texo= *actexp;
		bladertex();
		SetButs(211,211);
		break;
	case 215:	/* mapping */
	case 216: 
	case 217: 
	case 218: 
	case 219: 
	case 220:
		(colb->map[2*buttex])&=511;
		(colb->map[2*buttex])=BSET((colb->map[2*buttex]),229-event);
		SetButs(215,220);
		break;
	case 230:	/* halo */
		tekenobjbuts(1);
		return;
	}
	
	if(colb->hasize==0) shadesphere(0);
}


void wrldbuts(event)
short event;
{
	void tekenwrldbuts();
	void tekentextubuts();
	short texo;

	switch(event) {
	case 666:
		return;		/* geen shadenep */
	case 667:
		if(G.obslimits) projektie();
		return;		/* geen shadenep */
	case 603:
		wrld.mistdist=lampdist*1000;
		wrld.miststa=miststa*1000;
		return;		/* geen shadenep */
	case 604: 
	case 605:	/* mapping */
		wrld.smap[buttex]&=7;
		if(event== 604) wrld.smap[buttex]+=16;
		else wrld.smap[buttex]+=8;
		SetButs(604,605);
		break;
	case 610:		/* buttex */
		actexp= &wrld.stex[buttex];
		actex= *actexp;
		if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
		else inittexture(0,0);
		tekenwrldbuts(1); /* doet ook textu en shade! */
		return;
	case 611:
		texo= *actexp;
		bladertex();
		SetButs(611,611);
		break;
	}
	shadesky();
}

void movebuts(event)
short event;
{
	struct Base *base;
	struct ObData *ob;
	struct Key *key;
	struct MoData *mo;
	struct Bezier *bez=0;
	struct EditVert *eve;
	struct BPoint *bp;
	struct BezTriple *bezt;
	struct Nurb *nu;
	float *tfl,n[3], mat[4][4], len;
	long a, count, nr;
	char str[100];
	
	switch(event) {
	case 701:
		animfast();
		break;
	case 702:
		makeDispList(G.basact);		/* ook berekenpad */
		tekenspline2d(1);
		G.frs= G.basact->len;
		projektie();
		break;
	case 704:	/* loadvec */
		if(G.basact!=0 && G.basact->soort== -2) {
			mo=(struct MoData *)G.basact->d;
			n[0]= G.persinv[2][0];
			n[1]= G.persinv[2][1];
			n[2]= G.persinv[2][2];
			Normalise(n);
			n[0]*=32767; 
			n[1]*=32767; 
			n[2]*=32767;
			mo->n[0]= ffloor(n[0]);
			mo->n[1]= ffloor(n[1]);
			mo->n[2]= ffloor(n[2]);
			projektie();
		}
		break;
	case 705:	/* duplicator */
		if(G.basact!=0) {
			/* testen of global flag 4 wel aan moet */
			G.f&= ~4;
			base= G.firstbase;
			while(base) {
				if(base->soort== -2) {
					mo=(struct MoData *)base->d;
					if(mo->f & 4) G.f|=4;
				}
				else if(base->soort==1) {
					if(base->dupli) G.f|=4;
				}
				base= base->next;
			}
			projektie();
		}
		break;
	case 706:	/* frame-time verhouding */
		G.framelen= ((float)G.framapto)/((float)G.images);
		projektie();
		break;
	case 707:	/* set Vertex Parent */
		if(G.ebase) {
			if(G.ebase->soort & 1) {
				ob= (struct ObData *)G.ebase->d;
				ob->vl1= 1; ob->vl2= 2; ob->vl3= 3; ob->vl4= 0;
				if(G.ebase->soort==1) {
					eve= G.edve.first;
					nr= 0;
					count= 1;
					while(eve) {
						if(eve->f & 1) {
							if(nr==0) ob->vl1= count;
							else if(nr==1) ob->vl2= count;
							else if(nr==2) ob->vl3= count;
							else ob->vl4= count;
							nr++;
						}
						count++;
						eve= eve->next;
					}
				}
				else if ELEM(G.ebase->soort, 5, 11) {
					nr= 0;
					count= 1;
					nu= editNurb.first;
					while(nu) {
						if( (nu->type & 7)==1 ) {
							bezt= nu->bezt;
							a= nu->pntsu;
							while(a--) {
								if(bezt->f2 & 1) {
									if(nr==0) ob->vl1= count;
									else if(nr==1) ob->vl2= count;
									else if(nr==2) ob->vl3= count;
									else ob->vl4= count;
									nr++;
								}
								bezt++;
								count++;
							}
						}
						else {
							bp= nu->bp;
							a= nu->pntsu*nu->pntsv;
							while(a--) {
								if(bp->f1 & 1) {
									if(nr==0) ob->vl1= count;
									else if(nr==1) ob->vl2= count;
									else if(nr==2) ob->vl3= count;
									else ob->vl4= count;
									nr++;
								}
								bp++;
								count++;
							}
						}
						nu= nu->next;
					}
				}
			}
		}
		else error("Only in EditBase mode");
		break;
	case 708:	/* speed */
		if(G.basact) {
			VECCOPY(n, G.basact->r);
			G.cfra++;
			loadkeypostype(20, G.basact, G.basact);
			parentbase(G.basact, mat, mat);
			G.cfra--;
			loadkeypostype(20, G.basact, G.basact);
			n[0]-= G.basact->r[0];
			n[1]-= G.basact->r[1];
			n[2]-= G.basact->r[2];
			len= fsqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
			strcpy(str, "Speed %t|");
			sprintf(str+9, " %.2f per frame", len);
			pupmenu(str);
		}
		break;
	case 715:
		if(G.basact) {
			G.basact->lastipo= G.ipomode;
			tekenspline2d(1);
		}
		break;
	case 716:	/* lineair key */
	case 717:	/* Card1 key */
	case 718:	/* Card2 key */
	case 719:	/* B key */
		key= 0;
		if(G.basact==0) return;
		if(G.ipomode==20) key= G.basact->key;
		else if(G.ipomode==21) key= G.basact->pkey;
		else if(G.ipomode==22) {
			ob= (struct ObData *)G.basact->d;
			if(G.basact->soort==1) {
				key= ob->vv->key;
			}
			else if ELEM(G.basact->soort, 5, 11) {
				key= ob->cu->key;
			}
		}
		a= 1;
		while(key) {
			if(a==G.actkey) {
				if(event==716) key->f1= 1;
				else if(event==717) key->f1= 0;
				else if(event==718) key->f1= 2;
				else if(event==719) key->f1= 3;
			}
			a++;
			key= key->next;
		}
		tekenspline2d(1);
		break;
	case 720:
		copyIpo();
		break;
	case 721:
		pasteIpo();
		break;
	case 722:
		clearIpo();
		break;
	case 723:
		copyKey();
		break;
	case 724:
		pasteKey();
		break;
	}
}

void displaybuts(event)
short event;
{
	short val;
	char *adr, str[100], text[30];

	switch(event) {
	case 801:
		initrender(0);
		break;
	case 802:
		strcpy(str,"anim ");
		makepicstring(str+5,G.sfra);
		system(str);
		break;
	case 803:
		initrender(R.anim);
		break;
	case 805:
		if(view0.persp==2) projektie();
		break;
	case 806:
		if(G.holo==0)
			view0.obs[0]= view0.tar[0]= 0;
		projektie();
		break;
	case 807:
		SetButs(807,807);
		break;
	case 808:
		projektie();
		break;
	case 810:
		R.xsch=720;
		R.ysch=576;
		R.xasp=54;
		R.yasp=51;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 811:
		R.xsch=1280;
		R.ysch=1024;
		R.xasp=1;
		R.yasp=1;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 820:		/* preview */
		R.xsch=320;
		R.ysch=256;
		R.xasp=1;
		R.yasp=1;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 821:			/* AmiHilace */
		R.xsch=736;
		R.ysch=568;
		R.xasp=1;
		R.yasp=1;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 822:			/* CDI */
		R.xsch=384;
		R.ysch=280;
		R.xasp=1;
		R.yasp=1;
		R.size=0;
		R.safety= 125;	/* NTSC: 125 */ /* CODIM: 57 */
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 823:			/* PAL 16:9 */
		R.xsch=720;
		R.ysch=576;
		R.xasp=64;
		R.yasp=45;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 824:			/* D2MAC */
		R.xsch=1024;
		R.ysch=576;
		R.xasp=1;
		R.yasp=1;
		R.size=2;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;
	case 825:			/* MPEG */
		R.xsch=368;
		R.ysch=272;
		R.xasp=105;
		R.yasp=100;
		R.size=0;
		R.safety= 100;
		SetButs(804,805);
		if(view0.persp==2) projektie();
		break;

	case 830: 
	case 831: 
	case 832: 
	case 833: 
	case 834:	/* fileselects */
		switch(event) {
		case 830:
			adr= G.skb; 
			strcpy(text, "SELECT SKYBUFFER"); 
			break;
		case 831:
			adr= G.pic; 
			strcpy(text, "SELECT OUTPUT PICS"); 
			break;
		case 832:
			adr= G.scr; 
			strcpy(text, "SELECT SCRIPT"); 
			break;
		case 833:
			adr= G.ftype; 
			strcpy(text, "SELECT FTYPE"); 
			break;
		case 834:
			adr= G.bufscr; 
			strcpy(text, "SELECT BUFSCRIPT"); 
			break;
		}
		strcpy(str,adr);
		convertstringcode(str);
		val= ffileselect(text,str);
		if(val && strlen(str)<80 ) {
			if(adr[1]=='/') makestringcode(str);
			strcpy(adr,str);
			SetButs(event, event);
		}
		break;
	}
	if(event>=810 && event<830 && view0.persp==2) {
		winset(G.winar[0]);
		tekenoverdraw(0);
		winset(G.winar[1]);
	}
}

void delColBlck()
{
	struct ObData *ob;
	struct VV *vv;
	struct Base *base;
	struct Nurb *nu;
	struct ColBlck *col;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct EditVlak *evl;
	long len, a;
	short coln, button();

	if(G.basact==0) return;

	if(G.basact->soort==1) {
		ob= (struct ObData *)G.basact->d;
		if(ob->c==1) return;
		vv= ob->vv;

		coln= -1;
		if(G.ebase!=0 && G.ebase->soort==1) {
			evl= (struct EditVlak *)G.edvl.first;
			while(evl) {
				if(evl->f1==G.colact-1) {
					if(coln== -1) {
						coln=1;
						if(button(&coln, 1, ob->c-1,"Replace with:")==0) return;
					}
					evl->f1= coln-1;
				}
				else if(evl->f1>G.colact-1) evl->f1--;
				evl= evl->next;
			}
		}
		else {
			ob= (struct ObData *)G.basact->d;
			vv= ob->vv;
			adrve= (struct VertOb *)(vv+1);
			adrvl= (struct VlakOb *)(adrve+ vv->vert);
			for(a=0;a<vv->vlak;a++) {
				
				if( (adrvl->ec & 31)==G.colact-1 ) {
					if(coln== -1) {
						coln=1;
						if( button(&coln,1,ob->c-1,"Replace with:")==0) return;
					}
					adrvl->ec&= ~31;
					adrvl->ec+= coln-1;
				}
				else if( (adrvl->ec & 31) > G.colact-1 ) adrvl->ec--;
				adrvl++;
			}
		}
		
		/* ook testen op users */
		base= G.firstbase;
		while(base) {
			if(base->soort==1) {
				ob= (struct ObData *)base->d;
				if(ob->vv== vv) {
					col= (struct ColBlck *)(ob+1);
					col+= (G.colact-1);
					len= (ob->c- G.colact)*sizeof(struct ColBlck);
					if(len) memcpy(col,col+1,len);
					ob->c--;
				}
			}
			base= base->next;
		}
		
	}
	else if ELEM(G.basact->soort, 5, 11) {
		ob= (struct ObData *)G.basact->d;
		if(ob->c==1) return;

		col= (struct ColBlck *)(ob+1);
		col+= (G.colact-1);
		len= (ob->c- G.colact)*sizeof(struct ColBlck);
		if(len) memcpy(col,col+1,len);
		ob->c--;

		coln= -1;
		if(G.ebase) nu= editNurb.first;
		else nu= ob->cu->curve.first;

		while(nu) {
			if(nu->col==G.colact-1) {
				if(coln== -1) {
					coln=1;
					button(&coln,1,ob->c-1,"Replace with:");
				}
				nu->col= coln-1;
			}
			else if(nu->col>G.colact-1) nu->col--;
			nu= nu->next;
		}
	}
}

void editbuts(event)
short event;
{
	void tekeneditbuts();
	struct Base *base;
	struct ObData *ob;
	struct ColBlck *col;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct VectorFont *vf,*initvfont();
	struct VV *vv;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct EditVlak *evl;
	struct EditVert *eve;
	float fac;
	long a,len;
	short coln,button(), randfac, doit;
	char *new, str[100];

	switch(event) {
	case 901:  /* colact */
		ob= (struct ObData *)G.basact->d;
		colb=(struct ColBlck *)(ob+1);
		colb+=(G.colact-1);
		mapcolor(252,colb->r,colb->g,colb->b);
		break;
	case 902:  /* new colb */
		if(G.basact==0) return;
		if ELEM3(G.basact->soort, 1, 5, 11) {
			ob= (struct ObData *)G.basact->d;
			if(ob->c==15) {
				error("Too many");
				return;
			}
			G.colact = ob->c+1;
			if(G.basact->soort==1) {
				vv= ob->vv;
				/* ook testen op users */
				base= G.firstbase;
				while(base) {
					if(base->soort==1) {
						ob= (struct ObData *)base->d;
						if(ob->vv== vv) {
							ob->c++;
							len= sizeof(struct ObData)+ (ob->c)*sizeof(struct ColBlck);
							new= mallocN(len,"NewColblock");
							len-= sizeof(struct ColBlck);
							memcpy(new,ob,len);
							memcpy(new+len,colb,sizeof(struct ColBlck));
							base->d= (long *)new;
							freeN(ob);
						}
					}
					base= base->next;
				}
			}
			else {
				ob->c++;
				len= sizeof(struct ObData)+ (ob->c)*sizeof(struct ColBlck);
				new= mallocN(len,"NewColblock");
				len-= sizeof(struct ColBlck);
				memcpy(new,ob,len);
				memcpy(new+len,colb,sizeof(struct ColBlck));
				G.basact->d= (long *)new;
				freeN(ob);
			}
			tekeneditbuts(1);
			break;
		}
	case 903:  /* del colb */
		delColBlck();
		tekeneditbuts(1);
		break;
	case 904: 
	case 905:	/* select of deselect current col */
		if(G.ebase!=0) {
			if(G.ebase->soort==1) {
				evl= (struct EditVlak *)G.edvl.first;
				while(evl) {
					if(evl->f1==G.colact-1) {
						if(event==904) {
							if(evl->v1->h==0) evl->v1->f|=1; 
							if(evl->v2->h==0) evl->v2->f|=1; 
							if(evl->v3->h==0) evl->v3->f|=1;
						} else {
							if(evl->v1->h==0) evl->v1->f&= ~1; 
							if(evl->v2->h==0) evl->v2->f&= ~1; 
							if(evl->v3->h==0) evl->v3->f&= ~1;
						}
					}
					evl= evl->next;
				}
				countall();
				tekenoverdraw(0);
				tekenvertices_ext();
			}
			else if ELEM(G.ebase->soort, 5, 11) {
				nu= editNurb.first;
				while(nu) {
					if(nu->col==G.colact -1) {
						if((nu->type & 7)==1) {
							bezt= nu->bezt;
							a= nu->pntsu;
							while(a--) {
								if(event==904) {
									bezt->f1 |=1; 
									bezt->f2 |=1; 
									bezt->f3 |=1;
								} else {
									bezt->f1 &= ~1; 
									bezt->f2 &= ~1; 
									bezt->f3 &= ~1;
								}
								bezt++;
							}
						}
						else {
							bp= nu->bp;
							a= nu->pntsu*nu->pntsv;
							while(a--) {
								if(event==904) bp->f1 |= 1;
								else bp->f1 &= ~1;
								bp++;
							}
						}
					}
					nu= nu->next;
				}
				projektie();
			}
		}
		break;
	case 906:	/* assign */
		if(G.ebase!=0) {
			if(G.ebase->soort==1) {
				evl= (struct EditVlak *)G.edvl.first;
				while(evl) {
					if(evl->v1->f & 1)
						if(evl->v2->f & 1)
							if(evl->v3->f & 1)
								evl->f1= G.colact-1;
					evl= evl->next;
				}
			}
			else if ELEM(G.ebase->soort, 5, 11) {
				nu= editNurb.first;
				while(nu) {
					if(isNurbsel(nu)) nu->col= G.colact-1;
					nu= nu->next;
				}
			}
		}
		break;
	case 907:	/* '?' (=show) */
		if(G.ebase!=0) {
			if(G.ebase->soort==1) {
				evl= (struct EditVlak *)G.edvl.first;
				a= 0;
				while(evl) {
					if(evl->v1->f & 1)
						if(evl->v2->f & 1)
							if(evl->v3->f & 1)
								if(a==0) {
									G.colact= evl->f1+1;
									a=1;
								} 
								else {
									if(G.colact!= evl->f1+1) {
										error("Mixed colors");
										break;
									}
								}
					evl= evl->next;
				}
			}
			else if ELEM(G.ebase->soort, 5, 11) {
				nu= editNurb.first;
				a= 0;
				while(nu) {
					if(isNurbsel(nu)) {
						if(a==0) {
							G.colact= nu->col+1;
							a= 1;
						}
						else {
							if(G.colact!= nu->col+1) {
								error("Mixed colors");
								break;
							}
						}
					}
					nu= nu->next;
				}
			}
			tekeneditbuts(1);
		}
		break;
	case 910:	/* hide */
		if(G.ebase) {
			if(G.ebase->soort==1) hideVV();
			else if ELEM3(G.ebase->soort, 5, 11, -2) hideNurb();
			else if(G.ebase->soort== -4) hideIka();
		}
		break;
	case 911:	/* reveal */
		if(G.ebase) {
			if(G.ebase->soort==1) revealVV();
			else if ELEM3(G.ebase->soort, 5, 11, -2) revealNurb();
			else if(G.ebase->soort== -4) revealIka();
		}
		break;
	case 912:	/* select swap */
		if(G.ebase) {
			if(G.ebase->soort==1) selectswapVV();
			else if ELEM3(G.ebase->soort, 5, 11, -2) selectswapNurb();
			else if(G.ebase->soort== -4) selectswapIka();
		}
		break;
	case 913:	/* FasterDraw */
		fasterdraw();
		break;
	case 914:	/* flip normals */
		if(G.ebase && G.ebase->soort==1) {
			evl= G.edvl.first;
			while(evl) {
				if( (evl->v1->f & 1) && (evl->v2->f & 1) && (evl->v3->f & 1) ) {
					eve= evl->v2;
					evl->v2= evl->v3;
					evl->v3= eve;
				}
				evl= evl->next;
			}
			recalc_editnormals();
			projektie();
		}
		break;
	case 918:	/* fractal subd */
		if(G.ebase) if(G.ebase->soort==1) {
			randfac= 10;
			if(button(&randfac,1,100,"Rand fac:")==0) return;
			setcursorN(2);
			fac= -( (float)randfac )/100;
			subdivideflag(1,fac);
			countall();
			setcursorN(1);
			projektie();
		}
		break;
	case 919:	/* smooth */
		setcursorN(2);
		vertexsmooth();
		setcursorN(1);
		break;
	case 920:	/* extrude */
		G.f |= 1;	/* okee() uitgeschakeld */
		extrudeVV();
		G.f -= 1;
		break;
	case 921:	/* intersect */
		intersectVV();
		break;
	case 922:	/* spin */
		spinVV(step,degr,0,0);
		break;
	case 923:	/* screw */
		screwVV(step,turn);
		break;
	case 924:	/* split */
		G.f |= 1;
		splitVV();
		G.f -= 1;
		break;
	case 925:	/* rem doub */
		if(G.ebase) if(G.ebase->soort==1) {
			setcursorN(2);
			a= removedoublesflag(1,(float)doublimit);
			setcursorN(1);
			sprintf(str, "Removed: %d\n", a);
			notice(str);
		}
		break;
	case 926:	/* subdivide */
		if(G.ebase) if(G.ebase->soort==1) {
			setcursorN(2);
			subdivideflag(1,0.0);
			countall();
			setcursorN(1);
			projektie();
		}
		break;
	case 927:	/* noise */
		if(G.ebase) if(G.ebase->soort==1) vertexnoise();
		break;
	case 928:	/* spin dup */
		spinVV(step,degr,0,1);
		break;

	case 929:
		xsortvert_flag(1);
		break;
	case 934:
		extrude_repeatVV(step, extr_offs);
		break;
	case 938:
		vertices_to_sphere();
		break;
	case 939:
		hashvert_flag(1);
		break;
		
	case 930:	/* poly extrude */
		extrudepoly(G.basact);
		projektie();
		break;
	case 931:	/* globale bezier settings */
		base= G.firstbase;
		while(base) {
			if(base->soort==9) {
				makepolytext(base);
				extrudepoly(base);
			}
			base= base->next;
		}
		tekeneditbuts(1);
		countall();
		projektie();
		break;
	case 940:	/* herbereken font obj */
		if(G.basact && G.basact->soort==9) {
			makepolytext(G.basact);
			extrudepoly(G.basact);
			tekeneditbuts(1);
			projektie();
		}
		break;
	case 941:	/* nieuw lettertype van preset */
		if(G.basact && G.basact->soort==9) {
			ob= (struct ObData *)G.basact->d;
			if(ob->fo) {
				vf= give_vfontpointer(vfontbut);
				if(vf) {
					vf->us++;
					freevfont(ob->fo->vf);
					ob->fo->vf= vf;
					makepolytext(G.basact);
					extrudepoly(G.basact);
					projektie();
				}
				else {
					vfontbut= give_vfontnr(ob->fo->vf);
				}
				tekeneditbuts(1);	/* er kan een font zijn vrijgegeven, dan moet MENUbut opnieuw */
			}
		}
		break;
	case 942:	/* nieuw lettertype met filesel */
		vf= give_vfontpointer(vfontbut);
		strcpy(str,vf->name);
		if( ffileselect("LOAD FONT",str) ) {
			vf= exist_vfont(str);
			if(vf==0) vf= initvfont(str);
			if(vf) {
				ob= (struct ObData *)G.basact->d;
				if(ob->fo) {
					vf->us++;
					freevfont(ob->fo->vf);
					ob->fo->vf= vf;
					makepolytext(G.basact);
					extrudepoly(G.basact);
					tekeneditbuts(1);
					projektie();
				}
			}
		}
		break;
	case 943:	/* ToUpper */
		to_upper();
		break;
	case 950:	/* centre */
		docentre(G.basact, 0);
		break;
	case 951:	/* centrenew */
		docentre(G.basact, 1);
		break;

	case 960: 
	case 961: 
	case 962: 
	case 963: 
	case 964:	/* splinetype */
		if(G.ebase) {
			setsplinetype(event-960);
			tekeneditbuts(1);
			makeDispList(G.ebase);
			projektie();
		}
		break;
	case 965: 
	case 966: 
	case 967:	    /* u knots */
	case 968: 
	case 969: 
	case 970:	    /* v knots */
		if(G.ebase) {
			nu= editNurb.first;
			while(nu) {
				if(isNurbsel(nu)) {
					if((nu->type & 7)==4) {
						if(event<968) {
							nu->flagu &= 1;
							nu->flagu += ((event-965)<<1);
							makeknots(nu, 1, nu->flagu>>1);
						}
						else if(nu->pntsv>1) {
							nu->flagv &= 1;
							nu->flagv += ((event-968)<<1);
							makeknots(nu, 2, nu->flagv>>1);
						}
					}
				}
				nu= nu->next;
			}
			makeDispList(G.ebase);
			projektie();
		}
		break;
	case 971:		    /* set weight */
		if(G.ebase) {
			weightflagNurb(1, editbutweight, 0);
			makeDispList(G.ebase);
			projektie();
		}
		break;
	case 972:
		editbutweight= 1.0;
		SetButs(900, 900);
		break;
	case 973:
		editbutweight= fsqrt(2.0)/4.0;
		SetButs(900, 900);
		break;
	case 974:
		editbutweight= fsqrt(3.0)/9.0;
		SetButs(900, 900);
		break;
	case 980: 
	case 981:	    /* orderu en orderv */
		if(G.ebase) {
			extern struct Nurb *lastnu;
		
			nu= lastnu;
		
			if(nu && (nu->type & 7)==4 ) {
				if(event==980) {
					if(nu->orderu>nu->pntsu) {
						nu->orderu= nu->pntsu;
						SetButs(980, 980);
					}
					makeknots(nu, 1, nu->flagu>>1);
				}
				else {
					if(nu->orderv>nu->pntsv) {
						nu->orderv= nu->pntsv;
						SetButs(981, 981);
					}
					makeknots(nu, 2, nu->flagv>>1);
				}
			}
			makeDispList(G.ebase);
			projektie();
		}
		break;
	case 982:	    /* bevel toestanden en resol: herbereken displist  */
		makeDispList(G.basact);
		projektie();
		break;
	case 983:	    /* bevbase */
		if(G.basact && G.basact->soort==11) {
			ob= (struct ObData *)(G.basact->d);
			ob->cu->bevbase= dobasebutton2();
			if(ob->cu->bevbase==G.basact) {
				ob->cu->bevbase= 0;
				ibnaam[0]= 0;
			}
			SetButs(983, 983);
			makeDispList(G.basact);
			projektie();
		}
		break;
	case 984:	    /* subdivide */
		subdivideNurb();
		break;
	case 985:	    /* spin */
		if(G.ebase==0 || G.ebase->soort!=5 || (G.ebase->lay & view0.lay==0)) return;
		spinNurb(0, 0);
		countall();
		makeDispList(G.ebase);
		projektie();
		break;
	case 986:	    /* 3D curve */
		if(G.ebase) {
			ob= (struct ObData *)G.basact->d;
			nu= editNurb.first;
			while(nu) {
				nu->type &= ~8;
				if((ob->cu->flag & 256)==0) nu->type |= 8;
				test2DNurb(nu);
				nu= nu->next;
			}
		}
		if(G.basact && G.basact->soort==11) {
			ob= (struct ObData *)G.basact->d;
			nu= ob->cu->curve.first;
			while(nu) {
				nu->type &= ~8;
				if((ob->cu->flag & 256)==0) nu->type |= 8;
				test2DNurb(nu);
				nu= nu->next;
			}
		}
		makeDispList(G.basact);
		projektie();
		break;
	case 990:
		setmass_ika(setmassbut);
		break;
	case 991:
		setwidth_ika(setwidthbut);
		break;
	case 992:	/* constraints */
		if(jointsel) {
			doit= 0;
			if(jointsel->amax<jointsel->amin) {jointsel->amax= jointsel->amin; doit= 1;}
			if(jointsel->umax<jointsel->umin) {jointsel->umax= jointsel->umin; doit= 1;}
			if(jointsel->afac>jointsel->amax-jointsel->amin) {jointsel->afac=jointsel->amax-jointsel->amin; doit= 1;}
			if(jointsel->ufac>jointsel->umax-jointsel->umin) {jointsel->ufac=jointsel->umax-jointsel->umin; doit= 1;}
			if(doit) {
				SetButs(992, 992);
			}
			projektie();
		}
		break;
	case 993:
		re_init_deform();
		break;
	case 994:
		setlock_ika(1);
		break;
	case 995:
		setlock_ika(0);
		break;
	case 996:
		
		break;
	}
}


/* *********************************************************************** */


void deftextutypebut()
{
	struct Tex *tex;
	

	tex= G.adrtex[actex];

	switch(tex->soort) {
	case 0:
	
		break;
	case 1:
	
		break;
	case 2:
	
		break;
	case 3:
	
		break;
	case 4:
	
		break;
	case 5:
		DefBut(MENU|CHA, 309, "Norm%x0|Bump%x1|Field%x2|Anim5%x3|NoPremul%x4", 866,140,59,17, &tex->type);
		break;
	case 6:
	
		break;
	case 7:
		break;
	case 8:
	
		break;
	}
}

void tekentextubuts(mode)
short mode;
{
	struct Textudata *td,*inittextudata();
	struct Tex *tex;
	float min,max;
	int users;
	short x,a,x1,y1;
	char s[8];
	static char userstr[20], animstr[20];

	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(640,0,1280,199);
	}

	DefButBlock("Tex",G.winar[1],G.font,60,0,1);

	tex= G.adrtex[actex];
	texture= tex->soort;
	td= inittextudata(texture);

	imaname[0]= 0;
	if(texture==5 && tex->ima!=0 ) {
		strcpy(imaname,tex->ima->name);
	}
	/* 300 wordt niet afgehandeld */
	for(a=0; a<10; a++)
		DefBut(ROW|SHO,301,texstr[a],      647+52*a,172,50,17,&texture,1.0,(float)a);

	DefBut(BUT,	306,"UNDO",	647,140,88,17);
	SetButCol(4);
	DefBut(BUT,	304,"NEW",	738,140,65,17);
	DefBut(BUT,	305,"DEL",	805,140,58,17);
	SetButCol(0);

	if(tex->soort==5) deftextutypebut();
	else DefBut(NUM|CHA,	309,"Type",	866,140,59,17,&tex->type,0.0,10.0);
	
	DefBut(TOG|SHO,	300,"Sept",	929,140,42,17,&septex);
	DefBut(NUM|SHO,	300,"Zm: ",	974,140,60,17,&zoom,0.0,4.0);
	SetButCol(4);
	DefBut(BUT,	302,"Del Unused",	1036,140,96,17);
	SetButCol(0);
	/* DefBut(BUT,	302,"Load",	1036,140,44,17); */
	/* DefBut(BUT,	303,"Save",	1093,140,40,17); */
	DefBut(TEX,	300,"Name: ",	1136,140,138,17,tex->str,0.0,11.0);
	
	DefBut(TEX,	307,"Ima: ",	647,59,324,18,imaname,0.0,95.0);
	DefBut(BUT,	308,"Load",		974,59,107,18);

	DefBut(TOG|CHA|BIT|2,300,"Blend",	1093,59,88,18,&tex->f);
	DefBut(TOG|CHA|BIT|0,300,"Stencil",	1184,59,88,18,&tex->f);

	for(a=0;a<6;a++) {
		x1=116; 
		min= -20000;
		max=20000; 
		strcpy(s,"+");
		if(a<3) { 
			x1=7; 
			min= -400;
			max= 400; 
			strcpy(s,"/ "); 
		}
		y1=118- (a % 3)*18;
		DefBut(NUM|SHO,300,td->tekst[a],640+x1,y1,107,17,&tex->var[a],(float)td->min[a],(float)td->max[a]);
		DefBut(NUM|SHO,300,td->tekst[a+6],858+x1,y1,107,17,&tex->var[a+6],(float)td->min[a+6],(float)td->max[a+6]);
		x1= x1/1.2;
		DefBut(NUM|SHO,310,s,		1088+x1,y1,90,17,&tex->div[a], min, max);
	}
	DefBut(SLI|CHA,300,"R",	1154,40,90,6, tex->c1,0.0,255.0,255,0);
	DefBut(SLI|CHA,300,"G",	1154,25,90,6, tex->c1+1,0.0,255.0,255,1);
	DefBut(SLI|CHA,300,"B",	1154,10,90,6, tex->c1+2,0.0,255.0,255,2);
	
	DefBut(SLI|CHA,300,"Col",	680,40,90,6, &tex->cint,0.0,255.0,0);
	DefBut(SLI|CHA,300,"Nor",	680,25,90,6, &tex->bint,0.0,255.0,0);
	DefBut(SLI|CHA,300,"Int",	680,10,90,6, &tex->vint,0.0,255.0,0);
	
	DefBut(SLI|CHA,300,"Var",	901,10,90,6,tex->li,0.0,255.0,0);
	
	SetButCol(2);	/* 1068,7,33,42 */
	emboss2(255,1068,7,1101,50,1);

	/* tel aantal users */
	if(actex) {
		users= count_tex_users(actex);
	
		sprintf(userstr, "%d users  ", users);
	}
	else strcpy(userstr, "no users ");

	DefBut(LABEL,300,userstr,      1200,172,60,17);

	/* printen aantal frames anim */
	if(tex->ima && tex->ima->anim) {
		sprintf(animstr, "%d frames  ", tex->ima->anim->duration);
	}
	else strcpy(animstr, "  ");
	DefBut(LABEL,300, animstr,      887,39,60,17);
}


void tekenviewbuts(mode)
short mode;
{
	extern struct BGpic bgpic;
	extern short bgpicmode;

	if(G.winar[2]) {
		winclose(G.winar[2]);
		G.winar[2]=0;
	}
	FreeButBlock("Tex");
	FreeButBlock("WIN1");
	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,1279,199);	/* ortho zet dit goed neer */
	}

	DefButBlock("WIN1",G.winar[1],G.font,10,0,1);

	DefBut(TOG|SHO|BIT|0,151,"BackGroundPic",	1047,160,127,29 , &bgpicmode);
	DefBut(NUM|SHO,152,"Size:",		1178,160,82,29, &bgpic.size, 1.0, 500.0);
	DefBut(BUT,	    153,"",			1028,136,16,19);
	DefBut(TEX,	    151,"BGpic: ",	1047,136,211,19,bgpic.name,0.0,100.0);
	DefBut(SLI|SHO, 151,"Blend",	1079,118,131,8,&bgpic.blend,0.0,255.0, 0);

	DefBut(TOG|SHO,101,"Obs Limits",	1051,52,104,29 , &G.obslimits);
	DefBut(NUM|SHO,101,"Near:",		1157,52,101,29, &view0.near, 100.0, 20000.0);
	DefBut(NUM|SHO,101,"Far (x1000):",	1051,19,206,28, &view0.far, 10.0, 20000.0);

	DefBut(NUM|SHO,101,"ObsZoom",	941,52,104,29 , &G.obszoom, 0.0, 100.0);

}

void tekenlampbuts(mode)
short mode;
{
	struct Base *b;
	short isbase(),*t;
	char s[8];

	if(G.basact==0) return;
	if(G.basact->soort!=2) return;

	if(G.winar[2]) {
		winclose(G.winar[2]);
		G.winar[2]=0;
	}
	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,639,199);
	}

	DefButBlock("WIN1",G.winar[1],G.font,50,0,1);

	b=G.basact;
	la=(struct LaData *)b->d;
	/* textures */
	actexp= &(la->tex[buttex]);
	actex= *actexp;
	if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
	else inittexture(0,0);

	strcpy(naam,b->str);
	if(b->nnr) {
		sprintf(s,".%d",b->nnr);
		strcat(naam,s);
	}
	/* 101 doet projektie */
	DefBut(TEX,66,"Name: ", 18,140,164,17,naam,0.0,18.0);

	SetButCol(4);
	buttexbut=DefBut(ROW|SHO,410,"",243,140,72,17,&buttex,5.0,0.0);
	DefBut(ROW|SHO,410,"",318,140,72,17,&buttex,5.0,1.0);
	DefBut(ROW|SHO,410,"",392,140,72,17,&buttex,5.0,2.0);
	DefBut(ROW|SHO,410,"",466,140,72,17,&buttex,5.0,3.0);
	SetButCol(0);
	DefBut(BUTRET,411,"( )",	541,139,46,17);

	lampdist= la->ld/1000;
	DefBut(NUM|SHO,401,"Dist:",	17,120,84,17,&lampdist,1.0,2000.0);
	baselay= calcbaselay();
	DefBut(NUM|SHO,68,"Lay:",	104,120,77,17,&baselay,0.0,12.0);
	DefBut(TOG|CHA|BIT|0,101,"Shad",17,100,53,17,&la->f);
	DefBut(TOG|CHA|BIT|1,400,"Halo",74,99,46,17,&la->f);
	DefBut(TOG|CHA|BIT|4,400,"Star",122,99,47,17,&la->f);
	DefBut(TOG|CHA|BIT|2,400,"Lay", 173,98,48,17,&la->f);

	/*DefBut(ROW|SHO,400,"BufSi 512",	17,73,77,17,&la->bufsize,2.0,512.0);
	DefBut(ROW|SHO,400,"768",	96,73,40,17,&la->bufsize,2.0,768.0);
	DefBut(ROW|SHO,400,"1024",	138,73,40,17,&la->bufsize,2.0,1024.0);
	DefBut(ROW|SHO,400,"1536",	181,73,41,17,&la->bufsize,2.0,1536.0);*/

	DefBut(ROW|SHO,400,"BufSi 0.5",	17,73,77,17,&la->bufsize,2.0,512.0);
	DefBut(ROW|SHO,400,".75",		96,73,30,17,&la->bufsize,2.0,768.0);
	DefBut(ROW|SHO,400,"1.0",		128,73,30,17,&la->bufsize,2.0,1024.0);
	DefBut(ROW|SHO,400,"1.5",		160,73,30,17,&la->bufsize,2.0,1536.0);
	DefBut(ROW|SHO,400,"2.0",		193,73,30,17,&la->bufsize,2.0,2048.0);


	DefBut(NUM|SHO,101,"ClipSta:",	17,53,98,17,	&la->clipsta,1.0,500.0);
	DefBut(NUM|SHO,101,"ClipEnd:",	118,53,103,17,	&la->clipend,10.0,5000.0);
	DefBut(NUM|SHO,400,"Samp:",		17,33,98,17,&	la->samp,1.0,5.0);
	DefBut(NUM|SHO,400,"Bias/10:",	118,33,103,17,	&la->bias,1.0,1000.0);
	DefBut(NUM|FLO,400,"Soft:",		17,13,98,17,	&la->soft,1.0,100.0);

	DefBut(ROW|SHO,401,"Lamp",	244,108,61,25,&la->soort,1.0,0.0);
	DefBut(ROW|SHO,401,"Spot",	307,108,59,25,&la->soort,1.0,1.0);
	DefBut(ROW|SHO,401,"Sun",	368,108,58,25,&la->soort,1.0,2.0);
	DefBut(ROW|SHO,401,"Hemi",	427,108,55,25,&la->soort,1.0,3.0);
	
	DefBut(TOG|CHA|BIT|3,401,"Quad",	498,109,55,25,&la->f);
	DefBut(TOG|CHA|BIT|5,401,"SpotLin",	555,109,75,25,&la->f);

	DefBut(SLI|FLO,401,"Energy", 465,87,105,9,&(la->energy), 0.2, 5.0, 0, 0);

	DefBut(SLI|CHA,401,"R",		515,60,94,11,&la->r,0.0,255.0,250,0);
	DefBut(SLI|CHA,401,"G",		515,41,94,11,&la->g,0.0,255.0,250,1);
	DefBut(SLI|CHA,401,"B",		515,22,94,11,&la->b,0.0,255.0,250,2);

	DefBut(SLI|CHA,401,"SpotSiz",	266,88,85,6,&la->spsi,0.0,255.0,0);
	DefBut(SLI|CHA,401,"SpotBle",	266,68,85,6,&la->spbl,0.0,255.0,0);
	DefBut(SLI|CHA,401,"Quad1",  	266,48,85,6,&la->ld1,0.0,255.0,0);
	DefBut(SLI|CHA,401,"Quad2",  	266,28,85,6,&la->ld2,0.0,255.0,0);
	DefBut(SLI|CHA,402,"HaloInt",	266,8,85,6,&la->haint,0.0,255.0,0);

	strcpy(ibnaam,"");
	ibase= la->base;
	if(ibase[buttex]) {
		if(isbase(ibase[buttex])==0) ibase[buttex]=0;
		else {
			strcpy(ibnaam,ibase[buttex]->str);
			if(ibase[buttex]->nnr) {
				sprintf(s,".%d",ibase[buttex]->nnr);
				strcat(ibnaam,s);
			}
		}
	}
	SetButCol(3);
	DefBut(TEX,67,"",       17,172,128,17,ibnaam,0.0,18.0);

	t= &(la->map[buttex]);
	DefBut(TOG|SHO|BIT|10,417,"Base",	147,172,48,17,t);
	DefBut(TOG|SHO|BIT|9,416,"Glob",	198,172,38,17,t);
	DefBut(TOG|SHO|BIT|8,415,"Vec",		238,172,36,17,t);
	DefBut(TOG|SHO|BIT|2,402,"Col",		298,172,31,17,t);
	DefBut(TOG|SHO|BIT|1,402,"HaInt",	333,172,43,17,t);
	DefBut(TOG|SHO|BIT|0,402,"Vec",		380,172,40,17,t);

	SetButCol(2);
	emboss2(250,430,17,473,73,1);

	setbuttex();
	tekentextubuts(mode);
}


void tekenobjbuts(mode)
short mode;
{
	struct Base *b;
	struct ObData *ob;
	struct PerfSph *ps;
	short colt,isbase();
	ushort *t;
	char s[8],*rgb,*tt;


	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,639,199);
	}

	if(G.basact==0) return;
	if((G.basact->soort & 1)==0) return;

	DefButBlock("WIN1",G.winar[1],G.font,60,0,1);

	b=G.basact;
	if(b->soort==3) {
		ps=(struct PerfSph *)b->d;
		colb= &(ps->c);
		G.colact=1; 
		colt=1;
	} else {
		ob=(struct ObData *)b->d;
		colt=ob->c;
		if(colt <G.colact) G.colact=1;
		colb=(struct ColBlck *)(ob+1);
		colb+=(G.colact-1);
	}
	memcpy(&undocolb,colb,sizeof(struct ColBlck));

	/* textures */
	actexp= &(colb->tex[buttex]);
	actex= *actexp;
	if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
	else inittexture(0,0);

	strcpy(naam,b->str);
	if(b->nnr) {
		sprintf(s,".%d",b->nnr);
		strcat(naam,s);
	}
	/* 101 doet projektie */
	DefBut(TEX,66,"Name: ",		18,140,172,17,naam,0.0,18.0);
	SetButCol(4);
	buttexbut=DefBut(ROW|SHO,210,"",245,140,76,17,&buttex,5.0,0.0);
	DefBut(ROW|SHO,210,"",325,140,80,17,&buttex,5.0,1.0);
	DefBut(ROW|SHO,210,"",408,140,78,17,&buttex,5.0,2.0);
	DefBut(ROW|SHO,210,"",489,140,88,17,&buttex,5.0,3.0);
	SetButCol(0);
	DefBut(BUTRET,211,"( )",581,140,46,17);

	DefBut(NUM|SHO,230,"Halo:",	17,117,107,17,&(colb->hasize),0.0,20000.0);
	baselay= calcbaselay();
	DefBut(NUM|SHO,68,"Lay:",	18,83,97,17, &baselay,0.0,12.0);
	sprintf(s,"%d",colt);
	strcat(s," Col: ");
	DefBut(NUM|SHO,204,s,		243,81,80,31,&G.colact,1.0,(float)colt);

	DefBut(BUT,207,"CopyCol",	243,59,80,17);

	DefBut(BUT,208,"UNDO",	193,140,48,17);

	if(b->soort!=3) {
		DefBut(NUM|CHA,104,"Draw:",	116,83,76,17, &ob->dt,0.0,3.0);
		DefBut(NUM|CHA, 201, "Effect", 18,60,97,18, &ob->ef, 0.0, 5.0);
		DefBut(NUM|SHO,201,"EfLen:",	116,60,76,18, &ob->elen, 1.0, 1000.0);
	}

	if(colb->hasize) {
		DefBut(TOG|CHA|BIT|0,202,"Halotex",	127,117,63,17,&(colb->mode));
		DefBut(TOG|CHA|BIT|1,202,"Star",	193,117,48,17,&(colb->mode));
	}
	else {
		DefBut(TOG|CHA|BIT|0,202,"Smooth",	127,117,63,17,&(colb->mode));
		DefBut(TOG|CHA|BIT|1,202,"Zoffs",	193,117,48,17,&(colb->mode));
	}
	
	DefBut(TOG|CHA|BIT|2,202,"Traceble",243,117,68,17,&(colb->mode));
	DefBut(TOG|CHA|BIT|3,202,"Shadow",	313,117,66,17,&(colb->mode));
	DefBut(TOG|CHA|BIT|4,202,"Shless",	382,117,54,17,&(colb->mode));
	DefBut(TOG|CHA|BIT|5,202,"Env",		439,117,37,17,&(colb->mode));
	DefBut(TOG|SHO|BIT|9,202,"Z inv",	531,117,40,17,&(colb->mode1));
	DefBut(TOG|CHA|BIT|7,202,"ZtraSh",	573,117,54,17,&(colb->mode));
	DefBut(TOG|SHO|BIT|8,202,"SkySh",	479,117,48,17,&(colb->mode1));
	DefBut(TOG|SHO|BIT|10,202,"Wire",	193,98,48,17, &(colb->mode1));
	
	if(butcol==0) rgb= &colb->r;
	else if(butcol==1) rgb= &colb->specr;
	else rgb= &colb->mirr;
	/* global */
	butarcol= DefBut(SLI|CHA,206,"R",522,98,85,10,rgb,0.0,255.0,252-butcol,0);
	DefBut(SLI|CHA,206,"G",		522,79,85,10,rgb+1,0.0,255.0,252-butcol,1);
	DefBut(SLI|CHA,206,"B",		522,62,85,10,rgb+2,0.0,255.0,252-butcol,2);
	DefBut(ROW|SHO,203,"Col",	442,60,42,17,&butcol,1.0,0.0);
	DefBut(ROW|SHO,203,"Spec",	399,60,42,17,&butcol,1.0,1.0);
	DefBut(ROW|SHO,203,"Mir",	356,60,42,17,&butcol,1.0,2.0);

	DefBut(SLI|CHA,201,"Ref",	500,33,85,8,&(colb->kref),0.0,255.0,0);
	DefBut(SLI|CHA,201,"Emit",	500,15,85,8,&(colb->emit),0.0,255.0,0);
	if(colb->hasize)
		DefBut(SLI|CHA,201,"Star",	335,33,85,8,&(colb->kspec),0.0,255.0,0);
	else
		DefBut(SLI|CHA,201,"Spec",	335,33,85,8,&(colb->kspec),0.0,255.0,0);
	DefBut(SLI|CHA,201,"Har",	335,15,85,8,&(colb->spec),3.0,127.0,0);
	DefBut(SLI|CHA,201,"Mir",	190,33,78,8,&(colb->mir),0.0,255.0,0);
	DefBut(SLI|CHA,201,"Ang",	190,15,78,8,&(colb->ang),0.0,255.0,0);
	DefBut(SLI|CHA,201,"Tra",	38,33,85,8,&(colb->tra),0.0,255.0,0);
	DefBut(SLI|CHA,201,"Amb",	38,15,85,8,&(colb->amb),0.0,255.0,0);

	strcpy(ibnaam,"");
	ibase= colb->base;
	if(ibase[buttex]) {
		if(isbase(ibase[buttex])==0) ibase[buttex]=0;
		else {
			strcpy(ibnaam,ibase[buttex]->str);
			if(ibase[buttex]->nnr) {
				sprintf(s,".%d",ibase[buttex]->nnr);
				strcat(ibnaam,s);
			}
		}
	}
	SetButCol(3);
	DefBut(TEX,67,"",       17,172,128,17,ibnaam,0.0,18.0);

	t= &(colb->map[2*buttex]);
	DefBut(TOG|SHO|BIT|14,215,"Base",	146,172,36,17,t);
	DefBut(TOG|SHO|BIT|13,216,"UV",		182,172,28,17,t);
	DefBut(TOG|SHO|BIT|12,217,"Glob",	211,172,36,17,t);
	DefBut(TOG|SHO|BIT|11,218,"Orco",	248,172,38,17,t);
	DefBut(TOG|SHO|BIT|10,219,"Nor",	287,172,37,17,t);
	DefBut(TOG|SHO|BIT|9,220,"Refl",	325,172,33,17,t);
	DefBut(TOG|SHO|BIT|8,201,"Col",		363,172,31,17,t);
	DefBut(TOG|SHO|BIT|7,201,"Nor",		395,172,30,17,t);
	DefBut(TOG|SHO|BIT|6,201,"Csp",		426,172,30,17,t);
	DefBut(TOG|SHO|BIT|5,201,"Cmi",		457,172,33,17,t);
	tt= (char *)t;
	tt++;
	DefBut(TOG3|CHA|BIT|4,201,"Ref",	490,172,28,17,tt);
	DefBut(TOG3|CHA|BIT|3,201,"Spe",	519,172,28,17,tt);
	DefBut(TOG3|CHA|BIT|2,201,"Ang",	548,172,27,17,tt);
	DefBut(TOG3|CHA|BIT|1,201,"Tra",	576,172,25,17,tt);
	DefBut(TOG3|CHA|BIT|0,201,"Emi",	602,172,26,17,tt);

	mapcolor(252,colb->r,colb->g,colb->b);
	mapcolor(251,colb->specr,colb->specg,colb->specb);
	mapcolor(250,colb->mirr,colb->mirg,colb->mirb);
	SetButCol(2);
	emboss2(250,356,80,396,112,1);
	emboss2(251,399,80,439,112,1);
	emboss2(252,442,80,482,112,1);
	setbuttex();
	tekentextubuts(mode);

	shadesphere(colb);
}


void tekenwrldbuts(mode)
short mode;
{
	struct But *butar;
	short totbut,x,isbase();
	char s[8],*t;

	if(G.winar[2]) {
		winclose(G.winar[2]);
		G.winar[2]=0;
	}
	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,639,199);
	}

	DefButBlock("WIN1",G.winar[1],G.font,40,0,1);

	actexp= &(wrld.stex[buttex]);
	actex= *actexp;
	if(actex) memcpy(G.adrtex[0],G.adrtex[actex],152);
	else inittexture(0,0);

	x=0;
	SetButCol(4);
	buttexbut=DefBut(ROW|SHO,610,"",  	95,145,72,17,&buttex,5.0,0.0);
	DefBut(ROW|SHO,610,"",  	170,145,72,17,&buttex,5.0,1.0);
	DefBut(ROW|SHO,610,"",  	245,146,72,17,&buttex,5.0,2.0);
	DefBut(ROW|SHO,610,"",  	321,147,72,17,&buttex,5.0,3.0);
	SetButCol(0);
	DefBut(BUTRET,611,"( )",        417,146,46,17);

	DefBut(TOG|CHA|BIT|1,602,"Real",	19,118,71,19,&wrld.fs);
	DefBut(TOG|CHA|BIT|0,602,"Blend",	94,118,74,19,&wrld.fs);
	DefBut(TOG|CHA|BIT|3,602,"Paper",	171,118,71,19,&wrld.fs);
	
	DefBut(SLI|CHA,601,"HoR",       96,43,85,6,&(wrld.horr),0.0,255.0,252,0);
	DefBut(SLI|CHA,601,"HoG",       96,24,85,6,&(wrld.horg),0.0,255.0,252,1);
	DefBut(SLI|CHA,601,"HoB",       96,6,85,6,&(wrld.horb),0.0,255.0,252,2);
	DefBut(SLI|CHA,601,"ZeR",       96,100,85,6,&(wrld.zenr),0.0,255.0,251,0);
	DefBut(SLI|CHA,601,"ZeG",       96,84,85,6,&(wrld.zeng),0.0,255.0,251,1);
	DefBut(SLI|CHA,601,"ZeB",       96,65,85,6,&(wrld.zenb),0.0,255.0,251,2);
	DefBut(SLI|CHA,666,"AmbR",      505,45,81,6,&(wrld.ambr),0.0,255.0,250,0);
	DefBut(SLI|CHA,666,"AmbG",      505,26,81,6,&(wrld.ambg),0.0,255.0,250,1);
	DefBut(SLI|CHA,666,"AmbB",      505,9,81,6,&(wrld.ambb),0.0,255.0,250,2);

	DefBut(SLI|FLO,666,"Expos",      505,85,81,6,&(wrld.expos), 0.2, 5.0, 0, 0);

	lampdist=wrld.mistdist/1000;
	miststa=wrld.miststa/1000;
	
	DefBut(TOG|CHA,666,"Mist",      378,117,85,19,&wrld.misi);
	DefBut(NUM|SHO,603,"Sta:",      378,94,85,19,&miststa,0.0,10000.0);
	DefBut(NUM|SHO,603,"Di:",       378,72,85,19,&lampdist,0.0,10000.0);
	DefBut(NUM|SHO,603,"Hi:",       378,50,85,19,&wrld.misthi,0.0,1000.0);
	
	DefBut(TOG|CHA|BIT|4,600,"Stars",	259,117,112,19,&wrld.fs);
	DefBut(NUM|LON,600,"StarDist:",		259,94,112,18,&(wrld.rt5[0]), 0.0, 1000.0);
	DefBut(NUM|SHO,600,"MinDist:",		259,72,112,18,&(wrld.starmindist), 0.0, 5000.0);
	DefBut(NUM|LON,600,"Size:",			259,50,112,18,&(wrld.rt5[2]), 0.0, 1000.0);
	DefBut(NUM|LON,600,"Col:",			259,28,112,18,&(wrld.rt5[1]), 0.0, 1000.0);

	
				/* 666/667 doet geen shadesky */
	/* DefBut(NUM|CHA,666,"D.O.F:",     127, 45, 96, 17, &wrld.dofi, 0.0, 50.0); */
	/* DefBut(NUM|SHO,667,"Min:",       127, 25, 96, 17, &wrld.dofmin,0.0,10000.0); */
	/* DefBut(NUM|SHO,667,"Max:",       127, 5,  96, 17, &wrld.dofmax,0.0,10000.0); */
	/* DefBut(NUM|SHO,667,"Sta:",       227, 25, 96, 17, &wrld.dofsta,0.0,10000.0); */
	/* DefBut(NUM|SHO,667,"End:",       227, 5,  96, 17, &wrld.dofend,0.0,10000.0); */

	strcpy(ibnaam,"");
	ibase= wrld.sbase;
	if(ibase[buttex]) {
		if(isbase(ibase[buttex])==0) ibase[buttex]=0;
		else {
			strcpy(ibnaam,ibase[buttex]->str);
			if(ibase[buttex]->nnr) {
				sprintf(s,".%d",ibase[buttex]->nnr);
				strcat(ibnaam,s);
			}
		}
	}
	SetButCol(3);
	DefBut(TEX,67,"",       17,172,128,17,ibnaam,0.0,18.0);

	t= &wrld.smap[buttex];
	DefBut(TOG|CHA|BIT|4,604,"Base",	149,172,42,17,t);
	DefBut(TOG|CHA|BIT|3,605,"Vec",		194,172,36,17,t);
	DefBut(TOG|CHA|BIT|2,606,"ZenUp",	243,172,56,17,t);
	DefBut(TOG|CHA|BIT|5,606,"ZenDn",	301,172,55,17,t);
	DefBut(TOG|CHA|BIT|1,607,"Hor",		359,172,44,17,t);
	DefBut(TOG|CHA|BIT|0,608,"Blend",	404,172,60,17,t);

	SetButCol(2);
	emboss2(251,20,61,53,111, 1);
	emboss2(252,19,7,52,53,1);
	emboss2(250,433,6,466,46,1);
	setbuttex();
	tekentextubuts(mode);

	shadesky();
}

void tekenmovebuts(mode)
short mode;
{
	static int lastmoveipo=0, lastbaseipo= 20;
	struct Base *b;
	struct MoData *mo;
	struct Bezier *bez=0;
	short len;
	char s[8], *cp;

	if(G.basact==0) return;

	FreeButBlock("Tex");

	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,1279,199);
	}

	DefButBlock("WIN1", G.winar[1], G.font, 100, 0, 1);

	b=G.basact;
	if(b->soort== -2) {
		mo=(struct MoData *)b->d;
		mo->f = BCLR(mo->f,1);
	}
	strcpy(naam,b->str);
	if(b->nnr) {
		sprintf(s,".%d",b->nnr);
		strcat(naam,s);
	}

	/* 101 maakt projektie() */
	/* >700 wordt afgehandeld */
	SetButCol(4);
	DefBut(BUT,701,"Anim",			10,153,65,40);
	SetButCol(0);
	DefBut(NUM|SHO,700,"Sta:",      81,176,91,17,&G.sfra,1.0,2500.0);
	DefBut(NUM|SHO,700,"End",		177,176,101,17,&G.efra,1.0,2500.0);
	DefBut(NUM|SHO,706,"Images:",   81,154,91,17,&G.images,1.0,2500.0);
	DefBut(NUM|SHO,706,"MapTo:",	177,154,101,17,&G.framapto,1.0,2500.0);

	DefBut(TEX,66,"Name: ",			10,120,190,17,naam,0.0,18.0);
	baselay= calcbaselay();
	DefBut(NUM|SHO,68,"Lay:",		203,120,74,17, &baselay,0.0,12.0);
	
	SetButCol(3);
	DefBut(TOGN|CHA|BIT|4,101,"Vec",	10,100,52,17,&b->f);
	DefBut(TOGN|CHA|BIT|5,101,"Quat",	66,100,54,17,&b->f);
	DefBut(TOGN|CHA|BIT|6,101,"Mat",	122,100,53,17,&b->f);
	DefBut(TOG|CHA|BIT|0,101,"QuatFirst",   178,100,98,17,&b->f1);

	DefBut(ROW|CHA,101,"TrackX",	10,75,58,17, &b->tflag, 12.0, 0.0);
	DefBut(ROW|CHA,101,"Y",			71,75,19,17, &b->tflag, 12.0, 1.0);
	DefBut(ROW|CHA,101,"Z",			92,75,19,17, &b->tflag, 12.0, 2.0);
	DefBut(ROW|CHA,101,"-X",		113,75,24,17, &b->tflag, 12.0, 3.0);
	DefBut(ROW|CHA,101,"-Y",		139,75,24,17, &b->tflag, 12.0, 4.0);
	DefBut(ROW|CHA,101,"-Z",		165,75,24,17, &b->tflag, 12.0, 5.0);
	DefBut(ROW|CHA,101,"UpX",		191,75,40,17, &b->upflag, 13.0, 0.0);
	DefBut(ROW|CHA,101,"Y",			234,75,20,17, &b->upflag, 13.0, 1.0);
	DefBut(ROW|CHA,101,"Z",			257,75,19,17, &b->upflag, 13.0, 2.0);

	SetButCol(0);
	DefBut(NUM|SHO,101,"Sta:",				203,45,73,24,&b->sf,1.0,2500.0);
	DefBut(TOG|SHO|BIT|1,101,"SF local",	10,45,70,24,&b->f2);
	DefBut(TOG|SHO|BIT|2,101,"VertPar",		82,45,62,24,&b->f2);
	DefBut(BUT,707,"SetVP",					146,45,55,24);
	DefBut(TOG|SHO|BIT|3,101,"SF Key",		10,2,70,17,&b->f2);
	

	SetButCol(3);
	DefBut(TOG|CHA|BIT|1,101,"Q o", 10,20,43,17,&b->f1);
	DefBut(TOG|CHA|BIT|2,101,"lo",  57,20,38,17,&b->f1);
	DefBut(TOG|CHA|BIT|3,101,"gl",  99,20,39,17,&b->f1);
	DefBut(TOG|CHA|BIT|4,101,"M o", 143,20,39,17,&b->f1);
	DefBut(TOG|CHA|BIT|5,101,"lo",  188,20,41,17,&b->f1);
	DefBut(TOG|CHA|BIT|6,101,"gl",  232,20,43,17,&b->f1);

	SetButCol(0);
	DefBut(ROW|SHO,101,"ActKey",	292,175,55,17,&G.keydraw,2.0,0.0);
	DefBut(ROW|SHO,101,"BaseKey",	350,175,64,17,&G.keydraw,2.0,1.0);
	DefBut(ROW|SHO,101,"AllKey",	416,175,49,17,&G.keydraw,2.0,2.0);

	DefBut(ROW|SHO,700,"Act Base",	293,155,88,17,&G.keymode, 3.0, 0.0);
	DefBut(ROW|SHO,700,"Sel Base",	384,155,71,17,&G.keymode, 3.0, 1.0);

	DefBut(BUT,716,"Lin K",		473,175,40,17);
	DefBut(BUT,717,"Car1",		515,175,36,17);
	DefBut(BUT,718,"Car2",		553,175,36,17);
	DefBut(BUT,719,"B",			591,175,24,17);


	DefBut(NUM|SHO,702,"Len",	362,130,93,17,&b->len,1.0,2500.0);
	DefBut(NUM|SHO,101,"H2:",	362,110,93,17,&b->holo1,0.0,2500.0);
	DefBut(TOG|SHO|BIT|0,101,"Cyclic",293,130,66,17,&b->f2);

	DefBut(BUT,708,"Speed",		293,110,66,17);

	DefBut(ROW|SHO,715,"GlobTime",      472,150,93,17,&G.ipomode, 1.0, 10.0);

	if(b->soort== -2) {
		SetButCol(0);
		DefBut(NUM|SHO,101,"Q Fill:",   	293,89,91,20,&mo->qf,0.0,3600.0);
		DefBut(BUT,704,"LoadVec",       	387,89,67,20);
		DefBut(TOG|SHO,101,"Q Follow",  	379,62,76,24,&mo->qfo);
		SetButCol(4);
		DefBut(TOG|CHA|BIT|2,705,"Duplicate",	293,24,83,17,&mo->f);
		SetButCol(0);
		
		DefBut(NUM|SHO,705,"Lock:",	379,24,75,17,&mo->lock, 0.0, 1000.0);
		DefBut(NUM|SHO,705,"On:",		293,5,83,17,&mo->dupon, 0.0, 1000.0);
		DefBut(NUM|SHO,705,"Off:",		379,5,75,17,&mo->dupoff, 0.0, 1000.0);

		DefBut(ROW|SHO,715,"Trans",     472,90,93,17,&G.ipomode,1.0, 1.0);
		DefBut(ROW|SHO,715,"Qfill",     472,70,93,17,&G.ipomode,1.0, 0.0);
	}
	else {
		SetButCol(4);
		cp= 0;
		if(b->soort==1) cp= &b->dupli;
		DefBut(TOG|CHA, 705, "Duplicate",	293,24,83,17, cp);
		SetButCol(0);
		if(b->soort==1) cp= &b->duplirot;
		DefBut(TOG|CHA, 705, "Rot",			379,24,75,17, cp);
	}

	/* IPO BUTTONS */
	DefBut(ROW|SHO,715,"Base",      472,130,93,17,&G.ipomode,1.0, 20.0);
	DefBut(ROW|SHO,715,"Parent",    472,110,93,17,&G.ipomode,1.0, 21.0);

	if ELEM3(b->soort, 1, 5, 11) {
		DefBut(ROW|SHO,715,"Vertex",    472,90,93,17,&G.ipomode,1.0,22.0);
	}
	else if(b->soort==4) {
		DefBut(ROW|SHO,715,"Noise",     472, 90,93,17,&G.ipomode,1.0,23.0);
	}
	else if(b->soort== -2) {
		DefBut(ROW|SHO,715,"Vertex",    472,50,93,17, &G.ipomode, 1.0, 22.0);
	}
	else if(b->soort== -4) {
		DefBut(ROW|SHO,715,"Vertex",    472,90,93,17, &G.ipomode, 1.0, 22.0);
	}
	tekenspline2d(1);

	SetButCol(4);
	DefBut(BUT,720,"Copy",    528,4,44,18);
	DefBut(BUT,721,"Paste",   575,4,42,18);
	DefBut(BUT,722,"Ipo Clr", 471,4,54,18);

	DefBut(BUT,723,"Key Copy", 471,24,75,18);
	DefBut(BUT,724,"Paste",   548,24,70,18);
}

void tekendisplaybuts(mode)
short mode;
{
	short a,b;

	FreeButBlock("Tex");
	if(G.winar[2]) {
		winclose(G.winar[2]);
		G.winar[2]=0;
	}
	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,1279,199);
	}

	/* 101 maakt projektie() */
	/* 805 maakt projektie() als persp==2 */
	/* >800 wordt afgehandeld */
	DefButBlock("WIN1",G.winar[1],G.font,90,0,1);

	DefBut(TEX,800,"",		34,172,207,17,G.skb,0.0,79.0);
	DefBut(BUT,830," ",		14,172,17,17);
	DefBut(TEX,800,"",		34,149,207,17,G.pic,0.0,79.0);
	DefBut(BUT,831," ",		14,149,17,17);
	DefBut(TEX,800,"",		34,125,207,17,G.scr,0.0,79.0);
	DefBut(BUT,832," ",		14,125,17,17);
	DefBut(TEX,800,"",		34,102,207,17,G.ftype,0.0,79.0);
	DefBut(BUT,833," ",		14,102,17,17);
	DefBut(TEX,800,"",		34,79,207,17,G.bufscr,0.0,79.0);
	DefBut(BUT,834," ",		14,79,17,17);

	DefBut(TOG|SHO,800,"Skybuf",	245,172,63,17,&G.skybuf);
	DefBut(LABEL,800,"Pics",	245,149,63,17);
	DefBut(TOG|SHO,800,"Script",	245,125,63,17,&G.script);
	DefBut(LABEL,800,"Ftype",	245,102,63,17);
	DefBut(TOG|SHO,800,"BufScr",	245,79,63,17,&G.bufscript);

	for(b=0;b<3;b++) for(a=0;a<3;a++) {
		DefBut(TOG|SHO|BIT|(3*b+a),800,"",	34+18*a,11+12*b,16,10,&R.winpos);
	}

	SetButCol(4);
	DefBut(BUT,801,"RENDER",	319,142,192,47);
	SetButCol(0);
	DefBut(TOG|SHO|BIT|3,800,"Gour",515,169,60,20,&R.f1);
	DefBut(TOG|SHO|BIT|11,800,"Wire",577,169,60,20,&R.f1);
	DefBut(TOG|SHO|BIT|1,800,"Shad",515,144,60,20,&R.f1);
	DefBut(TOG|SHO|BIT|2,800,"Trace",577,144,60,20,&R.f1);
	DefBut(ROW|SHO,804,"100%",	515,114,121,20,&R.size,1.0,0.0);
	DefBut(ROW|SHO,804,"75%",	515,90,36,20,&R.size,1.0,1.0);
	DefBut(ROW|SHO,804,"50%",	554,90,40,20,&R.size,1.0,2.0);
	DefBut(ROW|SHO,804,"25%",	597,90,39,20,&R.size,1.0,3.0);
	DefBut(TOG|SHO,800,"OSA",	319,114,124,20,&G.ali);
	DefBut(NUM|SHO,807,"Nr:",	447,114,63,20,&G.osa,2.0,66.0);
	DefBut(ROW|SHO,807,"5",		319,90,27,20,&G.osa,2.0,5.0);
	DefBut(ROW|SHO,807,"8",		349,90,27,20,&G.osa,2.0,8.0);
	DefBut(ROW|SHO,807,"11",	379,90,31,20,&G.osa,2.0,11.0);
	DefBut(ROW|SHO,807,"16",	413,90,30,20,&G.osa,2.0,16.0);
		
	DefBut(NUM|SHO,800,"Xparts:",	319,42,99,31,&R.xparts,1.0,16.0);
	DefBut(NUM|SHO,800,"Yparts:",	422,42,86,31,&R.yparts,1.0,16.0);

	DefBut(TOG|SHO|BIT|5,800,"Fields",514,42,111,31,&R.f1);
	DefBut(TOG|SHO|BIT|9,800,"x",626,42,9,31,&R.f1);

	DefBut(ROW|SHO,800,"Sky",		319,11,38,24,&G.genlock,3.0,0.0);
	DefBut(ROW|SHO,800,"Premul",	360,11,54,24,&G.genlock,3.0,3.0);
	DefBut(ROW|SHO,800,"Genl",		417,11,44,24,&G.genlock,3.0,1.0);
	DefBut(ROW|SHO,800,"Key",		463,11,45,24,&G.genlock,3.0,2.0);

	DefBut(NUM|SHO,806,"Holo",		243,11,62,18,&G.holo, 0.0, 3.0);
	DefBut(NUM|SHO,806,"Hlen",		243,31,62,18,&G.holopadlen, 10.0, 500.0);

	DefBut(TOG|SHO,808,"Panora",	93,37,70,22,&panoramaview);
	DefBut(NUM|SHO,808,"Pofs:",		93,11,70,22,&panodegr,-360.0,360.0);
	DefBut(TOG|SHO|BIT|10,800,"Edge",168,37,70,22,&R.f1);
	DefBut(NUM|SHO,800,"Eint:",		168,11,70,22,&R.edgeint,0.0,255.0);

	DefBut(TOG|SHO,805,"Border",	515,11,58,24,&G.border);
	DefBut(TOG|SHO,800,"Gauss",		576,11,58,24,&G.filt);

	SetButCol(4);
	DefBut(BUT,803,"ANIM",		652,142,192,47);
	SetButCol(0);
	DefBut(ROW|SHO,800,"None",	652,101,47,35,&R.anim,4.0,1.0);
	DefBut(ROW|SHO,800,"HDISK",	704,101,73,35,&R.anim,4.0,6.0);
	DefBut(ROW|SHO,800,"TAPE",	781,101,63,35,&R.anim,4.0,5.0);
	DefBut(BUT,802,"PLAY",		652,54,94,34);
	DefBut(TOG|SHO,800,"Compr",	750,54,94,34,&G.disc);

	DefBut(ROW|SHO,800,"BW",	891,10,65,29,&R.planes,5.0,0.0);
	DefBut(ROW|SHO,800,"RGB",	961,10,61,29,&R.planes,5.0,1.0);
	DefBut(ROW|SHO,800,"RGBA",	1027,10,60,29,&R.planes,5.0,2.0);

	DefBut(ROW|SHO,800,"Iris",	892,87,62,29,&R.imtype,6.0,0.0);
	DefBut(ROW|SHO,800,"Targa",	958,87,59,29,&R.imtype,6.0,1.0);
	DefBut(ROW|SHO,800,"JPEG 90",1021,87,68,29,&R.imtype,6.0,5.0);
	DefBut(ROW|SHO,800,"HAMX",	893,56,102,25,&R.imtype,6.0,3.0);
	DefBut(ROW|SHO,800,"Ftype",	999,56,89,25,&R.imtype,6.0,4.0);

	DefBut(NUM|SHO,800,"Sta:",	652,11,93,31,&G.sfra,1.0,2500.0);
	DefBut(NUM|SHO,800,"End:",	750,11,95,31,&G.efra,1.0,2500.0);

	DefBut(NUM|SHO,805,"SizeX:",	892,158,100,31,&R.xsch,4.0,4096.0);
	DefBut(NUM|SHO,805,"SizeY:",	995,158,94,31,&R.ysch,4.0,4096.0);
	DefBut(NUM|SHO,805,"AspX:",	892,132,100,20,&R.xasp,1.0,200.0);
	DefBut(NUM|SHO,805,"AspY:",	995,132,94,20,&R.yasp,1.0,200.0);

	DefBut(BUT,810,"PAL",		1129,170,133,18);
	DefBut(BUT,811,"FULL",		1129,149,133,18);
	DefBut(BUT,820,"Preview",	1129,115,133,18);
	DefBut(BUT,821,"Amiga HiLace Os",1129,94,133,18);
	DefBut(BUT,822,"CDI",		1129,73,133,18);
	DefBut(BUT,823,"PAL 16:9",	1129,52,133,18);
	DefBut(BUT,824,"D2MAC",		1129,31,133,18);
	DefBut(BUT,825,"MPEG",		1129,10,133,18);

}


void tekeneditbuts(mode)
short mode;
{
	extern short bezres,bezmax;
	extern struct Nurb *lastnu;
	struct Base *base;
	struct VectorFont *vfont;
	struct ObData *ob;
	struct PolyData *po;
	struct FontData *fo;
	struct CurveData *cu;
	struct PerfSph *ps;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct IkaData *ika;
	float *fp;
	short colt,y, *sp;
	char s[24],di[100],fi[100];

	if(G.basact==0) return;

	/* 900: 	algemeen
	   901-909	colors
	   911-929	triface en 934-939
	   930-933	polydata
	   940-959	font
	   960-990	splines/nurbs
	   990-999  ika
	*/

	FreeButBlock("Tex");

	winset(G.winar[1]);
	if(mode==0) {
		color(100);
		sboxfs(0,0,1279,199);
	}

	DefButBlock("WIN1",G.winar[1],G.font,52,0,1);

	if(G.ebase) base=G.ebase; 
	else base=G.basact;

	strcpy(naam,base->str);
	if(base->nnr) {
		sprintf(s,".%d",base->nnr);
		strcat(naam,s);
	}

	/* 101 maakt projektie() */
	/* >900 wordt afgehandeld */
	DefBut(TEX,66,"Name: ",		15,151,180,17,naam,0.0,18.0);
	baselay= calcbaselay();
	DefBut(NUM|SHO,68,"Lay:",	14,118,80,17,&baselay,0.0,12.0);

	if(base->soort==1) {
		ob= (struct ObData *)base->d;
		DefBut(TOG|SHO,0,"KeepNorm",	14,98,80,17,&ob->rt3);
	}

	if ELEM3(base->soort, 1, 5, 11) {
		ob= (struct ObData *)base->d;
		colt=ob->c;
		if(colt <G.colact) G.colact=1;
		colb=(struct ColBlck *)(ob+1);
		colb+=(G.colact-1);

		DefBut(NUM|CHA,104,"Draw:",	127,118,65,17,&ob->dt,0.0,3.0);
		sprintf(s,"%d",colt); 
		strcat(s," Col: ");
		DefBut(NUM|SHO,901,s,		241,138,128,30,&G.colact,1.0,(float)colt);
		DefBut(BUT,907,"?",		373,139,31,29);
		SetButCol(4);
		DefBut(BUT,902,"New",		241,113,80,21);
		DefBut(BUT,903,"Delete",	324,113,80,21);
		DefBut(BUT,906,"Assign",	241,56,162,26);

		SetButCol(0);
		DefBut(BUT,904,"Select",	241,86,79,22);
		DefBut(BUT,905,"Deselect",	324,87,79,21);

		DefBut(BUT,910,"Hide",		1091,152,77,18);
		DefBut(BUT,911,"Reveal",	1171,152,86,18);
		DefBut(BUT,912,"Select Swap",	1091,129,166,18);

		mapcolor(252,colb->r,colb->g,colb->b);
		SetButCol(2);
		emboss2(252,409,56,435,169,1);

		SetButCol(4);
		DefBut(BUT,950,"Do Centre",		97,29,97,23);
		DefBut(BUT,951,"Centre New",	97,2,97,25);
		SetButCol(0);
	}
	if(base->soort== 1) {
		SetButCol(4);
		DefBut(BUT,913,"FasterDraw",	97,55,98,21);
		DefBut(BUT,914,"Flip Normals",	98,79,97,23);

		DefBut(BUT,928,"Spin Dup",	639,106,87,30);

		DefBut(BUT,920,"Extrude",	477,139,249,30);
		DefBut(BUT,921,"Intersect",	749,134,94,35);
		DefBut(BUT,922,"Spin",		558,106,78,30);
		DefBut(BUT,923,"Screw",		477,106,79,30);
		
		DefBut(BUT,934,"Extr Repeat",477,25,128,27);
		
		DefBut(BUT,924,"Split",		847,135,91,34);
		DefBut(BUT,925,"Rem Doubles",960,136,101,32);
		DefBut(BUT,926,"Subdivide",	749,94,93,35);
		DefBut(BUT,918,"Fract Subd",847,94,91,35);
		DefBut(BUT,927,"Noise",		749,54,93,34);
		DefBut(BUT,919,"Smooth",	847,54,91,34);
		DefBut(BUT,938,"Sphere",	939,54,91,34);
		DefBut(BUT,929,"Xsort",		749,14,93,34);
		DefBut(BUT,939,"Hash",		847,14,91,34);

		SetButCol(0);
		DefBut(NUM|SHO,900,"Degr:",	477,82,78,19,&degr,10.0,360.0);
		DefBut(NUM|SHO,900,"Steps:",	557,82,78,19,&step,1.0,180.0);
		DefBut(NUM|SHO,900,"Turns:",	639,82,86,19,&turn,1.0,360.0);

		DefBut(NUM|SHO,900,"Offset:",	608,25,117,27, &extr_offs, 10.0, 5000.0);

		DefBut(NUM|SHO,900,"Limit:",			961,110,100,19,&doublimit,1.0,250.0);
		DefBut(TOG|SHO|BIT|0,900,"Clockw",		639,55,86,23,&editbutflag);
		DefBut(TOG|SHO|BIT|1,900,"Keep Orig",	477,55,156,23,&editbutflag);

		DefBut(TOG|SHO|BIT|5,900,"Small Vertices",1090,85,164,19,&editbutflag);
		DefBut(TOG|SHO|BIT|5,900,"Draw Normals",1090,65,164,19,&G.f);
		DefBut(TOG|SHO|BIT|2,900,"Curve",		1090,45,164,19,&editbutflag);
		DefBut(TOG|SHO|BIT|3,900,"CleverEdge",	1090,25,164,19,&editbutflag);
		DefBut(TOGN|SHO|BIT|8,900,"DoubleSided",1090,5,164,19,&G.f);

	}
	if ELEM3(base->soort, 5, 11, -2) {
		SetButCol(4);
		DefBut(LABEL, 900, "Convert",	463,173,72,17);
		DefBut(BUT,960,"Poly",		467,152,72,17);
		DefBut(BUT,961,"Bezier",	467,132,72,17);
		DefBut(BUT,962,"Bspline",	467,112,72,17);
		DefBut(BUT,963,"Cardinal",	467,92,72,17);
		DefBut(BUT,964,"Nurb",		467,72,72,17);

		DefBut(LABEL, 900, "Make Knots",562,173,102,17);
		DefBut(BUT,965,"Uniform U",	565,152,102,17);
		DefBut(BUT,966,"Endpoint U",	565,132,102,17);
		DefBut(BUT,967,"Bezier U",	565,112,102,17);
		DefBut(BUT,968,"V",		670,152,50,17);
		DefBut(BUT,969,"V",		670,132,50,17);
		DefBut(BUT,970,"V",		670,112,50,17);

		DefBut(BUT,971,"Set Weight",	465,11,95,49);
		SetButCol(0);
		DefBut(NUM|FLO,900,"Weight:",	564,36,102,22, &editbutweight, 0.01, 10.0);
		DefBut(BUT,972,"1.0",		669,36,50,22);
		DefBut(BUT,973,"sqrt(2)/4",	564,11,78,20);
		DefBut(BUT,974,"sqrt(3)/9",	645,11,74,20);

		nu= lastnu;
		if(nu==0) nu= editNurb.first;
		if(nu) sp= &(nu->orderu); 
		else sp= 0;
		DefBut(NUM|SHO, 980, "Order U:", 565,91,102,17, sp, 2.0, 6.0);
		if(nu) sp= &(nu->orderv); 
		else sp= 0;
		DefBut(NUM|SHO, 981, "V:",	 670,91,50,17, sp, 2.0, 6.0);
		if(nu) sp= &(nu->resolu); 
		else sp= 0;
		DefBut(NUM|SHO, 982, "Resol U:", 565,70,102,17, sp, 1.0, 128.0);
		if(nu) sp= &(nu->resolv); 
		else sp= 0;
		DefBut(NUM|SHO, 982, "V:", 670,70,50,17, sp, 1.0, 128.0);

		SetButCol(4);

		DefBut(BUT, 984, "Subdivide",	808,132,101,36);

		if(base->soort==5) {
			DefBut(BUT, 985, "Spin",	808,92,101,36);
		}
		else if(base->soort==11) {

			ob= (struct ObData *)base->d;
			cu= ob->cu;
			if(isbase(cu->bevbase)) {
				strcpy(ibnaam, cu->bevbase->str);
				if(cu->bevbase->nnr) {
					sprintf(di, ".%d", cu->bevbase->nnr);
					strcat(ibnaam, di);
				}
			}
			else {
				cu->bevbase= 0;
				ibnaam[0]= 0;
			}

			SetButCol(0);

			DefBut(NUM|SHO, 982, "BevResol:",	905,49,102,17, &cu->presetbev, 0.0, 10.0);
			DefBut(TEX, 983, "BevBase:",	905,10,182,17, ibnaam, 0.0, 16.0);
			DefBut(NUM|SHO, 982, "Width:",	798,50,102,17, &cu->width, 0.0, 200.0);
			DefBut(NUM|SHO, 982, "Ext1:",	798,30,102,17, &cu->ext1, 0.0, 500.0);
			DefBut(NUM|SHO, 982, "Ext2:",	798,10,102,17, &cu->ext2, 0.0, 500.0);
			SetButCol(3);
			DefBut(TOG|SHO|BIT|8, 986, "3D",	733,50,62,17, &cu->flag);
			DefBut(TOG|SHO|BIT|0, 982, "Front",	733,30,62,17, &cu->flag);
			DefBut(TOG|SHO|BIT|1, 982, "Back",	733,10,62,17, &cu->flag);
			SetButCol(0);
		}

	}

	if(base->soort==7 || base->soort==9) {
		ob= (struct ObData *)base->d;
		po= ob->po;

		DefBut(NUM|CHA,104,"Draw:",	127,118,65,17,&ob->dt,0.0,3.0);
		DefBut(NUM|SHO,930,"Extrude:",	248,147,107,20,&po->ext,0.0,500.0);
		DefBut(NUM|SHO,930,"45Face+:",	248,124,107,20,&po->f45p,0.0,100.0);
		DefBut(NUM|SHO,930,"45Face-:",	248,101,107,20,&po->f45m,0.0,100.0);
		DefBut(NUM|SHO,931,"BezRes:",	248,78,107,20,&bezres,0.0,100.0);
		DefBut(NUM|SHO,931,"BezMax:",	248,55,107,20,&bezmax,1.0,64.0);
		SetButCol(3);
		DefBut(TOG|CHA|BIT|0,101,"Front",	372,147,49,20,&po->f1);
		DefBut(TOG|CHA|BIT|1,101,"Fr 45",	372,124,49,20,&po->f1);
		DefBut(TOG|CHA|BIT|2,101,"Side",	372,101,49,20,&po->f1);
		DefBut(TOG|CHA|BIT|3,101,"Ba 45",	372,78,49,20,&po->f1);
		DefBut(TOG|CHA|BIT|4,101,"Back",	372,55,49,20,&po->f1);
		SetButCol(0);
	}
	if(base->soort==9) {
		fo= ob->fo;
		DefBut(ROW|SHO,940,"Left",		467,150,52,17,fo->set,0.0,0.0);
		DefBut(ROW|SHO,940,"Right",		523,150,53,17,fo->set,0.0,1.0);
		DefBut(ROW|SHO,940,"Middle",	579,150,55,17,fo->set,0.0,2.0);
		DefBut(ROW|SHO,940,"Flush",		637,150,49,17,fo->set,0.0,3.0);
		DefBut(NUM|SHO,940,"Spacing:",	468,125,107,20,fo->set+1,-50.0,200.0);
		DefBut(TOG|SHO,940,"Kerning",	579,125,107,20,fo->set+2);
		DefBut(NUM|SHO,940,"Linedist:",	468,102,107,20,fo->set+3,0.0,200.0);
		DefBut(NUM|SHO,940,"Shear:",	578,102,107,20,fo->set+4,-100.0,100.0);
		DefBut(NUM|SHO,940,"Size:",		468,79,107,20,fo->set+5,1.0,100.0);

		DefBut(BUT,943,"ToUpper",		468,57,107,20);


		vfontbut= give_vfontnr(fo->vf);
		set_vfontbutstr();
		DefBut(MENU|SHO,941,vfontbutstr, 730,119,200,20, &vfontbut);
		
		SetButCol(4);
		DefBut(BUT,942,"Load Font",	730,147,200,23);

	}
	if(base->soort==4) {	/* camera */
		DefBut(TOG|SHO|BIT|0,900,"TraNoise",220,146,75,20,&base->noise);
		DefBut(TOG|SHO|BIT|1,900,"X",		301,146,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|2,900,"Y",		341,146,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|3,900,"Z",		381,146,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|8,900,"Sin",		421,146,36,20,&base->noise);
		
		DefBut(TOG|SHO|BIT|4,900,"RotNoise",220,120,75,20,&base->noise);
		DefBut(TOG|SHO|BIT|5,900,"X",		301,120,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|6,900,"Y",		341,120,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|7,900,"Z",		381,120,36,20,&base->noise);
		DefBut(TOG|SHO|BIT|9,900,"Sin",		421,120,36,20,&base->noise);

		DefBut(NUM|SHO,900,"NoiseFreq:",	220,74,195,20,&base->noisefreq, 1.0, 500.0);
		DefBut(NUM|SHO,900,"VecInt:",		220,50,195,20,&base->nvecint, 1.0, 10000.0);
		DefBut(NUM|SHO,900,"RotInt:",		220,26,195,20,&base->nrotint, 1.0, 500.0);
		
		DefBut(TOG|SHO|BIT|6,101, "Ortho",	14,88,70,17, &base->f2);
	}

	if(base->soort== -4) {	/* ika */
		ika= (struct IkaData *)base->d;
		
		DefBut(NUM|SHO,104,"Draw:",	127,118,65,17,&ika->dt,0.0,3.0);

		SetButCol(4);
		if(bopsel) {
			sprintf(s,"cur: %.2f",bopsel->m);
			DefBut(LABEL, 900, s, 565,175,102,17);
		}
		DefBut(NUM|FLO,100,"Mass:",	565,152,102,17, &setmassbut, 0.01, 20.0);
		DefBut(NUM|SHO,100,"Width:",565,132,102,17, &setwidthbut, 10.0, 10000.0);
		/* DefBut(BUT,967,"Bezier U",565,112,102,17); */
		DefBut(BUT,990,"Set",		670,152,50,17);
		DefBut(BUT,991,"Set",		670,132,50,17);
		/* DefBut(BUT,970,"Set",	670,112,50,17); */

		SetButCol(3);
		DefBut(TOG|SHO, 100, "Rod Edit",	808,132,101,36, &rodedit);
		SetButCol(0);
		DefBut(BUT, 993, "Init Deform",		808,100,101,26);
		DefBut(BUT, 994, "Lock Point",		808,80,101,17);
		DefBut(BUT, 995, "UnLock Point",	808,60,101,17);

		DefBut(LABEL, 900, "Alpha constraint",400,165,102,17);	
			
		if(jointsel) fp= &(jointsel->amin); else fp= 0;
		DefBut(SLI|FLO, 992, "min", 400, 150, 100, 8, fp, 0.0, 3.1415, 0.0);
		if(jointsel) fp= &(jointsel->amax); else fp= 0;
		DefBut(SLI|FLO, 992, "max", 400, 135, 100, 8, fp, 0.0, 3.1415, 0.0);
		if(jointsel) fp= &(jointsel->afac); else fp= 0;
		DefBut(SLI|FLO, 992, "soft", 400, 120, 100, 8, fp, 0.0, 3.1415, 0.0);

		DefBut(LABEL, 900, "Up constraint",400,100,102,17);	
		
		if(jointsel) fp= &(jointsel->umin); else fp= 0;
		DefBut(SLI|FLO, 992, "min", 400, 85, 100, 8, fp, 0.0, 3.1415, 0.0);
		if(jointsel) fp= &(jointsel->umax); else fp= 0;
		DefBut(SLI|FLO, 992, "max", 400, 70, 100, 8, fp, 0.0, 3.1415, 0.0);
		if(jointsel) fp= &(jointsel->ufac); else fp= 0;
		DefBut(SLI|FLO, 992, "soft", 400, 55, 100, 8, fp, 0.0, 3.1415, 0.0);

		DefBut(BUT,910,"Hide",		1091,152,77,18);
		DefBut(BUT,911,"Reveal",	1171,152,86,18);
		DefBut(BUT,912,"Select Swap",	1091,129,166,18);
		
	}
}

void projektie_and_keypos()
{
	loadkeyposall();
	projektie();
}

void tekenmainbuts(mode)
short mode;
{
	/* mode 0: volledige redraw
	   mode 1: init
	   mode 2: redraw zonder clear()
	*/
	short vec[2],a;
	char s[2];
	fmfonthandle helvfont,helv10;

	winset(G.winar[1]);
	frontbuffer(1);
	backbuffer(1);

	if(mode!=2) {
		s[1]=0;
		color(100); 
		clear();
		bgnpolygon();
		color(48);
		vec[0]=0; 
		vec[1]=200;
		v2s(vec);
		vec[0]=1279; 
		vec[1]=200;
		v2s(vec);
		color(32);
		vec[0]=1279; 
		vec[1]=222;
		v2s(vec);
		vec[0]=0; 
		vec[1]=222;
		v2s(vec);
		endpolygon();
	}
	if(mode!=1) {
		if(mode==0 && G.mainb>1) {	/* voorkom rare redraw als submenu verkeerd staat */
			if(G.basact==0) {
				if(G.mainb==6 || G.mainb==8);
				else G.mainb= G.mainbo= 1;
			}
			else {
				if( (G.basact->soort & 1)==0 && G.mainb==5) G.mainb= G.mainbo= 1;
				if( G.basact->soort!=2 && G.mainb==4) G.mainb= G.mainbo= 1;
			}
		}
		SetButs(100,149);
		if(G.mainb==1) tekenviewbuts(mode);
		if(G.mainb==4) tekenlampbuts(mode);
		if(G.mainb==5) tekenobjbuts(mode);
		if(G.mainb==6) tekenwrldbuts(mode);
		if(G.mainb==7) tekenmovebuts(mode);
		if(G.mainb==8) tekendisplaybuts(mode);
		if(G.mainb==9) tekeneditbuts(mode);
		return;
	}

	/* font openen */

	fminit();
	helvfont= fmfindfont("Helvetica-Bold");
	
	if( helvfont == 0) sluit(0);

	if(G.machine==ENTRY) helv10=fmscalefont(helvfont,8.0);
	else helv10=fmscalefont(helvfont,11.0);
	fmsetfont(helv10);
	G.font= helv10;

	/* mapkleuren worden in funktie setUserdefs() gezet */

	DefButBlock("main",G.winar[1],helv10,36,2,1);
	DefButCol(0,NORMALDRAW,100, 120,0,WHITE,BLACK, 121,122,0,0);
	DefButCol(1,PUPDRAW, 0,1,2,3,2, 2,3,0,0);
	DefButCol(2,NORMALDRAW,100, 124,0,WHITE,BLACK, 125,126,0,0);
	DefButCol(3,NORMALDRAW,100, 128,0,WHITE,BLACK, 129,130,0,0);
	DefButCol(4,NORMALDRAW,100, 132,0,WHITE,BLACK, 133,134,0,0);

	color(100);
	sboxfs(0,0,1279,199);

	DefBut(ROW|SHO,100,"VIEW",	250,203,50,16,&G.mainb,1.0,1.0);
	DefBut(ROW|SHO,100,"LAMP",	302,203,50,16,&G.mainb,1.0,4.0);
	DefBut(ROW|SHO,100,"OBJ",	355,203,50,16,&G.mainb,1.0,5.0);
	DefBut(ROW|SHO,100,"WRLD",	408,203,50,16,&G.mainb,1.0,6.0);
	DefBut(ROW|SHO,100,"MOVE",	461,203,50,16,&G.mainb,1.0,7.0);
	DefBut(ROW|SHO,100,"DISPL",	514,203,50,16,&G.mainb,1.0,8.0);
	DefBut(ROW|SHO,100,"EDIT",	567,203,50,16,&G.mainb,1.0,9.0);
	/* event 101 doet projektie() */
	SetButFunc(projektie_and_keypos);
	DefBut(NUM|SHO,103,"",		645,203,60,16,&G.cfra,1.0,2500.0);
	DefBut(NUM|SHO,101,"Lens",	935,203,80,16,&view0.lens,10.0,1000.0);
	DefBut(NUM|SHO,101,"Grid",	1020,203,100,16,&view0.grid,100.0,20000.0);
	DefBut(TOG|SHO,104,"Zbuf",	1130,203,60,16,&G.zbuf);
	DefBut(TOG|SHO,101,"Hiera",	1200,203,60,16,&G.hie);
	s[0]='1';
	for(a=1;a<13;a++) {
		DefBut(TOG|SHO|BIT|(a-1),110+a,s,16*a,203,14,16,&view0.lay);
		s[0]++;
		if(s[0]>'9') s[0]='0';
	}
	DefBut(ROW|SHO,102,"O",		730,203,20,16,&G.move1,2.0,0.0);
	DefBut(ROW|SHO,102,"+", 	752,203,20,16,&G.move1,2.0,1.0);
	DefBut(ROW|SHO,102,".", 	774,203,20,16,&G.move1,2.0,2.0);
	DefBut(ROW|SHO,102,"V", 	809,203,20,16,&G.move2,3.0,0.0);
	DefBut(ROW|SHO,102,"X", 	832,203,20,16,&G.move2,3.0,1.0);
	DefBut(ROW|SHO,102,"Y", 	854,203,20,16,&G.move2,3.0,2.0);
	DefBut(ROW|SHO,102,"Z", 	876,203,20,16,&G.move2,3.0,3.0);

	tekenviewbuts(1);
}

short button(var,min,max,str)
short *var,min,max;
char *str;
{
	struct But *butar;
	ushort pat[16];
	short x1,x2,y1,y2;
	short oldmap[4][3], toets,val,mval[2],ret=0,DoButtons();
	Device mdev[2];

	if(min>max) min= max;

	winset(G.winar[1]);

	for(x1=0;x1<16;x1+=2) pat[x1]=0x5555;
	for(x1=1;x1<16;x1+=2) pat[x1]=0xAAAA;
	defpattern(1,16,pat);

	mdev[0]=MOUSEX; 
	mdev[1]=MOUSEY;
	getdev(2, mdev, mval);
	if(mval[0]<150) mval[0]=150;
	if(mval[1]<30) mval[0]=30;
	if(mval[0]>1240) mval[0]=1240;
	if(mval[1]>980) mval[1]=980;

	fullscrn();
	drawmode(PUPDRAW);
	sginap(2);
	finish();

	if(G.machine==ENTRY) {
		getmcolor(1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor(2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor(3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		mapcolor(1, 170, 170, 170); 
		mapcolor(2, 0, 0, 0); 
		mapcolor(3, 240, 240, 240);
	}
	
	x1=mval[0]-150; 
	x2=mval[0]+20; 
	y1=mval[1]-20; 
	y2=mval[1]+20;
	color(1);
	setpattern(1);
	sboxfs(x1,y1+5,x2,y2-5);
	setpattern(0);
	sboxs(x1,y1+5,x2,y2-5);

	DefButBlock("button",G.winar[1],G.font,2,1,1);

	DefBut(NUM|SHO,101,str,	x1+5,y1+10,125,20,var,(float)min,(float)max);
	DefBut(BUT,102,"OK",	x1+136,y1+10,25,20);

	while(TRUE) {
		toets= traces_qread(&val);
		if(val) {
			if (toets == ESCKEY) break;
			if (toets == RETKEY) {
				ret = 1;
				break;
			}

			if(toets==LEFTMOUSE) {
				getdev(2, mdev, mval);
				if(mval[0]<x1 || mval[0]>x2 || mval[1]<y1 || mval[1]>y2) break;
				else {
					if(DoButtons()==102) {
						ret=1;
						break;
					}
				}
			}
			if(toets==INPUTCHANGE) G.winakt=val;
		}
	}
	while(getbutton(LEFTMOUSE));

	FreeButBlock("button");
	color(0);
	sboxfs(x1,y1,x2,y2);

	if(G.machine==ENTRY) {
		mapcolor(1, oldmap[1][0], oldmap[1][1], oldmap[1][2]);
		mapcolor(2, oldmap[2][0], oldmap[2][1], oldmap[2][2]);
		mapcolor(3, oldmap[3][0], oldmap[3][1], oldmap[3][2]);
	}

	drawmode(NORMALDRAW);
	endfullscrn();
	ortho2(0,1279,0,223-1);

	winset(G.winakt);
	return ret;
}

void workfieldbuttons()
{
	struct But *but;
	short run=1,dev,val,DoButtons();
	short a, r,g,b,mode=120,event;
	char col[3];

	if(G.mainb!=1) return;

	winset(G.winar[1]);
	
	getmcolor(mode,&r,&g,&b);
	col[0]= r; 
	col[1]= g; 
	col[2]= b;

	DefButBlock("wfield", G.winar[1], G.font, 26, 0, 1);

	but=DefBut(SLI|CHA,70,"R",	400,130,100,10,col,0.0,255.0,mode,0);
	DefBut(SLI|CHA,70,"G",		400,115,100,10,col+1,0.0,255.0,mode,1);
	DefBut(SLI|CHA,70,"B",		400,100,100,10,col+2,0.0,255.0,mode,2);

	DefBut(ROW|SHO,71,"Back",	200,20,96,20,&mode,0.0,(float)User.back[0]);

	DefBut(ROW|SHO,71,"But 0  0",	300,20,96,20,&mode,0.0,(float)User.but0[0][0]);
	DefBut(ROW|SHO,71,"But 0  1",	300,45,96,20,&mode,0.0,(float)User.but0[1][0]);
	DefBut(ROW|SHO,71,"But 0  2",	300,70,96,20,&mode,0.0,(float)User.but0[2][0]);
	SetButCol(2);
	DefBut(ROW|SHO,71,"But 1  0",	400,20,96,20,&mode,0.0,(float)User.but1[0][0]);
	DefBut(ROW|SHO,71,"But 1  1",	400,45,96,20,&mode,0.0,(float)User.but1[1][0]);
	DefBut(ROW|SHO,71,"But 1  2",	400,70,96,20,&mode,0.0,(float)User.but1[2][0]);
	SetButCol(3);
	DefBut(ROW|SHO,71,"But 2  0",	500,20,96,20,&mode,0.0,(float)User.but2[0][0]);
	DefBut(ROW|SHO,71,"But 2  1",	500,45,96,20,&mode,0.0,(float)User.but2[1][0]);
	DefBut(ROW|SHO,71,"But 2  2",	500,70,96,20,&mode,0.0,(float)User.but2[2][0]);
	SetButCol(4);
	DefBut(ROW|SHO,71,"But 3  0",	600,20,96,20,&mode,0.0,(float)User.but3[0][0]);
	DefBut(ROW|SHO,71,"But 3  1",	600,45,96,20,&mode,0.0,(float)User.but3[1][0]);
	DefBut(ROW|SHO,71,"But 3  2",	600,70,96,20,&mode,0.0,(float)User.but3[2][0]);

	DefBut(BUT, 73,"OK",	700, 45, 96, 45);

	while(run) {
		event= 0;
		switch(dev= traces_qread(&val)) {
		case LEFTMOUSE:
			if(val) {
				event=DoButtons();
				if(event==71) {
					getmcolor(mode,&r,&g,&b);
					col[0]=r; 
					col[1]=g; 
					col[2]=b;
					but->a1= mode;
					(but+1)->a1= mode;
					(but+2)->a1= mode;
					SetButs(70,70);
				}
				else if(event==73) dev= RETKEY;
			}
			break;
		}
		if(val) {
			if(dev==ESCKEY) break;
			else if(dev==RETKEY) break;
			else {
				;
			}
		}
		
	}
	FreeButBlock("wfield");

	if(dev==RETKEY) {
		getmcolor(User.back[0], User.back+1, User.back+2, User.back+3);
		
		for(a=0; a<3; a++) {
			getmcolor(User.but0[a][0], User.but0[a]+1, User.but0[a]+2, User.but0[a]+3);
			getmcolor(User.but1[a][0], User.but1[a]+1, User.but1[a]+2, User.but1[a]+3);
			getmcolor(User.but2[a][0], User.but2[a]+1, User.but2[a]+2, User.but2[a]+3);
			getmcolor(User.but3[a][0], User.but3[a]+1, User.but3[a]+2, User.but3[a]+3);
			/* getmcolor(User.but4[a][0], User.but4[a]+1, User.but4[a]+2, User.but4[a]+3); */
		}
	}
	setUserdefs();

	tekenmainbuts(0);
	
}

