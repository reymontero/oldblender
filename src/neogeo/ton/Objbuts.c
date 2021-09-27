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


 
/*  
cc -O3 Objbuts.c -lgl_s -lc_s -float -lfastm -o Objbuts  
cc Objbuts.c -lgl_s -lc_s -float -lfastm -o Objbuts  

*/



#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include "Trace.h"
#include "Button.h"
#include <stdio.h>



void shadeshadesphere(cb,n,col,x,y)
struct ColBlck *cb;
short x,y,col[];
float n[];
{
	float v1,inp,inpspec=0;
	float v[3];
	short temp,temp2;
	
	if(cb->kspec) {  /* geen spec dus geen viewvector */
		v[0]=x+SHLX1; v[1]=y+SHLY1; v[2]=SHLZ1+256;
		NormalWrd(v);
	}

	col[0]=cb->r; col[1]=cb->g; col[2]=cb->b;

	if(cb->tex[0]);  /*texture afhandeling;*/

	inp=n[0]*SHLX+n[1]*SHLY+n[2]*SHLZ;

	if(cb->kspec)  {
		if(inp<=0) inp=0;
		else {
			v1=v[0]*n[0]+v[1]*n[1]+v[2]*n[2];
			if(v1>0) {  
				v1=Spec(v1,cb->spec);
				temp2=2*(v1*cb->kspec);
			}
		}
	}
	if (cb->mode & 16)  {  
		col[0]+= (cb->specr*temp2)>>8;
		col[1]+= (cb->specg*temp2)>>8;
		col[2]+= (cb->specb*temp2)>>8;
	}
	else {
		temp=(cb->kref*inp);
		col[0]=(temp*(cb->ambr+col[0]) + temp2*cb->specr)>>8; 
		col[1]=(temp*(cb->ambg+col[1]) + temp2*cb->specg)>>8;
		col[2]=(temp*(cb->ambb+col[2]) + temp2*cb->specb)>>8; 
	} 
}




void shadesphere(c)
struct ColBlck *c;
{
	short x1,y1,x,y,xof,yof,xof2,col[3];

	float n[3],z1;
	long rsq,xsq,ysq,zsq;
	unsigned long line[2*SHADESPHEREX+10];
	uchar *s;
		
	xof=10; yof=SHADESPHEREY+10;
	xof2=xof+(2*SHADESPHEREX);
	
	rsq=16*SHADESPHEREX-1;
	rsq*=rsq;
	
	for(y= -SHADESPHEREY;y<SHADESPHEREY;y++) {
		y1= -(y<<4);
		ysq=y1*y1;

		s= (uchar *)line; 

		for(x= -SHADESPHEREX;x<=SHADESPHEREX;x++) {
			s++;
			x1= -(x<<4);
			xsq=x1*x1;
			if(xsq+ysq<rsq) {
				zsq=rsq-xsq-ysq;
				z1=fsqrt((float)zsq);
				n[0]=x1; n[1]=y1; n[2]=z1;
				NormalWrd(n);
				shadeshadesphere(c,n,col,x,y);
				
				MinMaxRGB(col);
				*(s++)=(uchar)(col[2]);
				*(s++)=(uchar)(col[1]);
				*(s++)=(uchar)(col[0]);
			}
			else { 
				*(s++)=85; *(s++)=55; *(s++)=45;
			}
		}
		cpack(0); rectf(xof,y+yof,xof2,y+yof+1);
		lrectwrite(xof,y+yof,xof2,y+yof,line) ;	

	}
}
		


main()
{
    short mval[2], lastval[2],totbut,event;
    Device mdev[2];
    short x,y,trek,val;
    long org[2], size[2], tijd;
    unsigned char aa,string[40];
    Boolean run;
    struct But *butar;
    struct ColBlck c;
   
	c.kref=200; c.r=55; c.g=90; c.b=230;
	c.kspec=160;
	c.spec=40;
	c.specr=c.specg=c.specb=240;
	c.mode=1;
	c.ambr=c.ambg=c.ambb=10;

    prefposition(400,800,400,800);
    winar[0][2]=winopen("Shade");
    RGBmode(); 
    gconfig();
    prefposition(800,1200,400,800);
    winar[1][2]=winopen("Obj edit");
    gconfig();
 
    qdevice(REDRAW);
    qdevice(LEFTMOUSE);
    qdevice(ESCKEY);
    qdevice(INPUTCHANGE);

    color(48); clear(); winset(winar[0][2]); cpack(0); clear();

    mdev[0] = MOUSEX;
    mdev[1] = MOUSEY;

    run = TRUE; 

    totbut=9;
    butar=(struct But *) malloc(totbut*sizeof(struct But));
    winar[1][0]=(unsigned long)butar;
    winar[1][1]=totbut;  

    aa=0;
    x=0;
    DefBut(&butar[x++],1,1,4,80,40,150,80,6,0,0,&c.r,0,255,"R");
    DefBut(&butar[x++],1,1,4,80,40,130,80,6,0,0,&c.g,0,255,"G");
    DefBut(&butar[x++],1,1,4,80,40,110,80,6,0,0,&c.b,0,255,"B");
    DefBut(&butar[x++],1,1,4,80,190,150,80,6,0,0,&c.specr,0,255,"SpR");
    DefBut(&butar[x++],1,1,4,80,190,130,80,6,0,0,&c.specg,0,255,"SpG");
    DefBut(&butar[x++],1,1,4,80,190,110,80,6,0,0,&c.specb,0,255,"SpB");
    DefBut(&butar[x++],1,1,4,80,40,80,80,6,0,0,&c.kref,0,255,"Ref");
    DefBut(&butar[x++],1,1,4,80,40,60,80,6,0,0,&c.kspec,0,255,"Spe");
    DefBut(&butar[x++],1,1,4,80,40,40,80,6,0,0,&c.spec,3,128,"Har");

    /*trek=120;
    DefBut(&butar[x++],1,1,5,48,40,250,120,30,0,&trek,0,0,1000,"Getal");
    */

    winset(winar[0][2]);
    shadesphere(&c);
    while (run) {
	switch (qread(&val)) {
	case LEFTMOUSE:
	    if (val == 1) {
		event=DoButtons();
		if(event!=-1) {
			winset(winar[0][2]);
			shadesphere(&c);
			winset(winar[1][2]);
			qreset();
		}
		
	    }
	    break;
	case ESCKEY:
		if (val == 0)
		run = FALSE;
		break;
	case REDRAW:
		reshapeviewport();
		break;
	case INPUTCHANGE:
		if(val) winset(val);
		break;
	case (WINQUIT || WINSHUT):
		FreeButs();
	}
    }
    gexit();
    return 0;
}

