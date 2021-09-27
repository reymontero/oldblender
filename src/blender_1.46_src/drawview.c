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



/* drawview.c  april 94		GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "sector.h"


float defmatgl[]= {
	AMBIENT, 0.1, 0.1, 0.1,
	DIFFUSE, 0.9, 0.9, 0.9,
	SPECULAR, 0.7, 0.7, 0.7,
	EMISSION, 0.0, 0.0, 0.0, 
	SHININESS, 0.0,
	LMNULL
};

float defmattexgl[]= {
	AMBIENT, 0.0, 0.0, 0.0,
	DIFFUSE, 0.0, 0.0, 0.0,
	SPECULAR, 0.0, 0.0, 0.0,
	EMISSION, 2.0, 2.0, 2.0, 
	SHININESS, 0.0,
	LMNULL
};

float lmodel[]= {
	AMBIENT, 1.0, 1.0, 1.0,
	LOCALVIEWER, 0,
	TWOSIDE, 0, 
	LMNULL
};
float light[]= {
	LCOLOR, 1.0, 1.0, 1.0,
	POSITION, -0.3, 0.3, 0.90, 0.0,
	LMNULL
};
float blue_light[] =
    {LCOLOR,    0.5, 0.5, 0.8,
     POSITION,  0.2, 0.1, -1.0, 0.0, 
     LMNULL};

void deflighting()
{
	lmdef(DEFMATERIAL, 1, 0, defmatgl);
	lmdef(DEFMATERIAL, 2, 0, defmattexgl);
	lmdef(DEFLIGHT, 1, 10, light);
	lmdef(DEFLIGHT, 2, 10, blue_light);
	lmdef(DEFLMODEL, 1, 0, lmodel);
}

void bindlighting()
{
	lmcolor(LMC_COLOR);
	
	lmbind(MATERIAL, 1);
	lmbind(LIGHT0, 1);
	/* lmbind(LIGHT1, 2); */
	lmbind(LMODEL, 1);
	
}

void bindtexlighting()
{
	lmcolor(LMC_EMISSION);
	
	lmbind(MATERIAL, 2);
	lmbind(LIGHT0, 0);
	
}


void def_gl_material(int matnr, Material *ma)
{
	if(ma==0) {
		defmatgl[1]= defmatgl[2]= defmatgl[3]= 0.1;
		
		defmatgl[5]= defmatgl[6]= defmatgl[7]= 0.9;
	}
	else {
		if(ma->mode & MA_SHLESS) {
			defmatgl[5]= defmatgl[6]= defmatgl[7]= 0.0;
			defmatgl[1]= ma->r;
			defmatgl[2]= ma->g;
			defmatgl[3]= ma->b;
		}
		else {
			defmatgl[1]= defmatgl[2]= defmatgl[3]= 0.1;
		
			defmatgl[5]= (ma->ref+ma->emit)*ma->r;
			defmatgl[6]= (ma->ref+ma->emit)*ma->g;
			defmatgl[7]= (ma->ref+ma->emit)*ma->b;
		}
	}
	if(matnr==0) matnr= 1;
	lmdef(DEFMATERIAL, matnr, 0, defmatgl);
}

void two_sided(int val)
{
	
	if(val && lmodel[7]==1.0) return;
	if(val==0 && lmodel[7]==0.0) return;
	
	if(val) lmodel[7]= 1.0;
	else lmodel[7]= 0.0;
	
	lmdef(DEFLMODEL, 1, 0, lmodel);
}

void myczclear(ulong col)
{
	static ulong zmin= 12;
	
	if(zmin==12) zmin= getgdesc(GD_ZMAX);
	
	czclear(col, zmin);
}


/* ****************************************** */

void setalpha_bgpic(BGpic *bgpic)
{
	int x, y, alph;
	char *rect;
	
	alph= (int)(255.0*(1.0-bgpic->blend));
	
	rect= (char *)bgpic->rect;
	for(y=0; y< bgpic->yim; y++) {
		for(x= bgpic->xim; x>0; x--, rect+=4) {
			rect[0]= alph;
		}
	}
}

void draw_bgpic()
{
	BGpic *bgpic;
	Image *ima;
	float vec[3], fac, asp, zoom;
	ulong *rect;
	int x1, y1, x2, y2, cx, cy, xim, yim;
	short mval[2];
	char str[100];
	
	bgpic= G.vd->bgpic;
	if(bgpic==0) return;
	
	if(bgpic->tex) {
		init_render_texture(bgpic->tex);
		free_unused_animimages();
		ima= bgpic->tex->ima;
		end_render_texture(bgpic->tex);
	}
	else {
		ima= bgpic->ima;
	}
	
	if(ima==0) return;
	if(ima->ok==0) return;
	
	/* plaatje testen */
	if(ima->ibuf==0) {
		if(bgpic->tex) {
			ima_ibuf_is_nul(bgpic->tex);
		}
		else {
			strcpy(str, ima->name);
			convertstringcode(str);
			waitcursor(1);
			ima->ibuf = loadiffname(str , LI_rect);
			waitcursor(0);
		}
		if(ima->ibuf==0) {
			ima->ok= 0;
			return;
		}
		if(bgpic->rect) freeN(bgpic->rect);
		bgpic->rect= 0;
	}

	if(G.vd->persp==2) {
		x1= G.vd->pr_xmin;
		y1= G.vd->pr_ymin;
		x2= G.vd->pr_xmax;
		y2= G.vd->pr_ymax;
	}
	else {
		/* windowco berekenen */
		initgrabz(0.0, 0.0, 0.0);
		window_to_3d(vec, 1, 0);
		fac= MAX3( fabs(vec[0]), fabs(vec[1]), fabs(vec[1]) );
		fac= 1.0/fac;
	
		asp= ( (float)ima->ibuf->y)/(float)ima->ibuf->x;
	
		vec[0]= vec[1]= vec[2]= 0.0;
		project_short_noclip(vec, mval);
		cx= curarea->winrct.xmin+mval[0];
		cy= curarea->winrct.ymin+mval[1];
	
		x1=  cx+ fac*(bgpic->xof-bgpic->size);
		y1=  cy+ asp*fac*(bgpic->yof-bgpic->size);
		x2=  cx+ fac*(bgpic->xof+bgpic->size);
		y2=  cy+ asp*fac*(bgpic->yof+bgpic->size);
	}
	
	xim= x2-x1;
	yim= y2-y1;
	zoom= 1.0;
		
	if(xim<4 || yim<4) return;
	
	if(xim > ima->ibuf->x) {
		zoom= xim;
		zoom /= (float)ima->ibuf->x;
		zoom= fceil(zoom);
		
		xim= xim/zoom;
		yim= yim/zoom;
	}

	if(bgpic->rect==0 || zoom!=bgpic->zoom || xim!=bgpic->xim || yim!=bgpic->yim) {
		if(bgpic->rect) freeN(bgpic->rect);
		bgpic->rect= mallocN(xim*yim*4, "bgpicrect");
		
		scalefastrect(ima->ibuf->rect, bgpic->rect, ima->ibuf->x, ima->ibuf->y, xim, yim);

		bgpic->xim= xim;
		bgpic->yim= yim;
		bgpic->zoom= zoom;
		setalpha_bgpic(bgpic);
	}
	
	/* coordinaten hoe 't op scherm komt */
	x2= x1+ zoom*(bgpic->xim-1);
	y2= y1+ zoom*(bgpic->yim-1);

	/* volledige clip? */
	
	if(x2 < curarea->winrct.xmin ) return;
	if(y2 < curarea->winrct.ymin ) return;
	if(x1 > curarea->winrct.xmax ) return;
	if(y1 > curarea->winrct.ymax ) return;
	
	/* rectwrite coordinaten */
	xim= xim-1;
	yim= yim-1;
	rect= bgpic->rect;

	/* partiele clip */
	if(x1<curarea->winrct.xmin) {
		cx= curarea->winrct.xmin-x1;
		/* zorg ervoor dat de rect pixelnauwkeurig wordt neergezet */
		cx/= zoom;
		x1+= zoom*cx;
		xim-= cx;
		rect+= cx;
	}
	if(y1<curarea->winrct.ymin) {
		cy= curarea->winrct.ymin-y1;
		cy/= zoom;
		y1+= zoom*cy;
		rect+= cy*bgpic->xim;
		yim-= cy;
	}
	if(x2>curarea->winrct.xmax) {
		cx= x2-curarea->winrct.xmax;
		cx/= zoom;
		xim-= cx;
	}
	if(y2>curarea->winrct.ymax) {
		cy= y2-curarea->winrct.ymax;
		cy/= zoom;
		yim-= cy;
	}
	
	if(xim<=0) return;
	if(yim<=0) return;

	rectzoom(zoom, zoom);

	/* geeft aan dat per scanline rectwriten, ibuf->x in de rect verder gelezen wordt */
	pixmode(PM_STRIDE, bgpic->xim);

	blendfunction(BF_SA, BF_MSA);
	lrectwrite(x1, y1, x1+xim, y1+yim, rect);
	blendfunction(BF_ONE, BF_ZERO);
	rectzoom(1.0, 1.0);
	
	pixmode(PM_STRIDE, 0);
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


void drawgrid()
{
	/* extern short bgpicmode; */
	float wx, wy, x, y, fz, fw, fx, fy, dx, dy;
	float vec4[4];

	vec4[0]=vec4[1]=vec4[2]=0.0; 
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.vd->persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	wx= (curarea->winx/2.0);	/* ivm afrondfoutjes, grid op verkeerde plek */
	wy= (curarea->winy/2.0);

	x= (wx)*fx/fw;
	y= (wy)*fy/fw;

	vec4[0]=vec4[1]=G.vd->grid; 
	vec4[2]= 0.0;
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.vd->persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	dx= fabs(x-(wx)*fx/fw);
	if(dx==0) dx= fabs(y-(wy)*fy/fw);

	if(dx<6.0) {
		dx*= 10.0;
		setlinestyle(5);
		if(dx<6.0) {
			dx*= 10.0;
			if(dx<6.0) {
				setlinestyle(0);
				return;
			}
		}
	}
	
	persp(0);

	cpack(0x505050);
	
	x+= (wx); 
	y+= (wy);
	fx= x/dx;
	fx= x-dx*ffloor(fx);
	
	while(fx< curarea->winx) {
		sbox(fx, 0.0, fx, (float)curarea->winy);
		fx+= dx; 
	}

	fy= y/dx;
	fy= y-dx*ffloor(fy);
	

	while(fy< curarea->winy) {
		sbox(0.0, fy, (float)curarea->winx, fy);
		fy+= dx;
	}

	/* kruis in midden */
	if(G.vd->view==3) cpack(0xA0D0A0); /* y-as */
	else cpack(0xA0A0C0);	/* x-as */
	sbox(0, y, (float)curarea->winx, y);
	
	if(G.vd->view==7) cpack(0xA0D0A0);	/* y-as */
	else cpack(0xD0A0A0);	/* z-as */
	sbox(x, 0, x, (float)curarea->winy);
	
	persp(1);
	setlinestyle(0);
}


void drawfloor()
{
	View3D *vd;
	float vert[3], grid;
	int a, b, gridlines;
	
	vd= curarea->spacedata.first;

	vert[2]= 0.0;

	gridlines= vd->gridlines/2;
	grid= gridlines*vd->grid;
	
	cpack(0x404040);

	for(a= -gridlines;a<=gridlines;a++) {

		if(a==0) {
			if(vd->persp==0) cpack(0xA0D0A0);
			else cpack(0x402000);
		}
		else if(a==1) {
			if(G.machine==ENTRY) cpack(0x405040);
			else cpack(0x404040);
		}
		
		bgnline();
			vert[0]= a*vd->grid;
			vert[1]= grid;
			v3f(vert);
			vert[1]= -grid;
			v3f(vert);
		endline();
	}
	
	cpack(0x404040);
	
	for(a= -gridlines;a<=gridlines;a++) {
		if(a==0) {
			if(vd->persp==0) cpack(0xA0A0D0);
			else cpack(0);
		}
		else if(a==1) {
			if(G.machine==ENTRY) cpack(0x405040);
			else cpack(0x404040);
		}
		bgnline();
			vert[1]= a*vd->grid;
			vert[0]= grid;
			v3f(vert );
			vert[0]= -grid;
			v3f(vert);
		endline();
	}

}


void drawcursor()
{

	if(G.f & G_PLAYANIM) return;
	if(G.moving && G.vd->mxo==3200) return;

	drawmode(OVERDRAW);
	
	if(G.vd->mxo!=3200) {
		color(0);
		sboxfs(G.vd->mxo-21, G.vd->myo-21, G.vd->mxo+21, G.vd->myo+21);
	}
	
	if(G.moving) {
		G.vd->mxo= 3200;
		drawmode(NORMALDRAW);
		return;
	}
	
	project_short( give_cursor(), &G.vd->mx);

	G.vd->mxo= G.vd->mx;
	G.vd->myo= G.vd->my;

	if( G.vd->mx!=3200) {
		if(G.machine==ENTRY || G.machine==VIDEO) linewidth(1);
		
		if(G.machine==IRIS) {
			
			color(2);
			circs(G.vd->mx, G.vd->my,10);
			/* setpattern(1234); */
			/* color(3); */
			/* circs(G.vd->mx, G.vd->my,10); */
			/* setpattern(0); */
			color(1);
			
		}
		else {
			setlinestyle(1); 
			color(2);
			circs(G.vd->mx, G.vd->my,10);
			setlinestyle(2); 
			color(3);
			circs(G.vd->mx, G.vd->my,10);
			setlinestyle(0);
			color(1);
		}
		
		sboxs(G.vd->mx-20, G.vd->my, G.vd->mx-5, G.vd->my);
		sboxs(G.vd->mx+5, G.vd->my, G.vd->mx+20, G.vd->my);
		sboxs(G.vd->mx, G.vd->my-20, G.vd->mx, G.vd->my-5);
		sboxs(G.vd->mx, G.vd->my+5, G.vd->mx, G.vd->my+20);

		if(G.machine==ENTRY || G.machine==VIDEO) linewidth(2);
	}
	
	drawmode(NORMALDRAW);
}

void drawcursor_all()
{
	ScrArea *sa;
	
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==SPACE_VIEW3D) {
			areawinset(sa->win);
			persp(0);
			drawcursor();
			persp(1);
		}
		sa= sa->next;
	}
}

void calc_viewborder()
{
	float a, fac, wx, wy, x1, x2, y1, y2;
	float imax, imay, width;
	
	wx= curarea->winx;
	wy= curarea->winy;
	imax= G.scene->r.xsch;
	imay= G.scene->r.ysch;
	
	fac= ((float)imay*G.scene->r.yasp)/((float)imax*G.scene->r.xasp);
	imay= fac*imax;
	
		/* liggend plaatje */
	if(imax>imay) {
		if(wx<wy) width= wy;
		else width= wx;
		
		fac= width/(2.0*imax);
		
		x1= 0.5*wx-0.25*width;
		x2= 0.5*wx+0.25*width;
		y1= 0.5*wy - 0.5*fac*imay;
		y2= 0.5*wy + 0.5*fac*imay;
	}
	else {
		if(wx<wy) width= wy;
		else width= wx;

		fac= width/(2.0*imay);
		
		y1= 0.5*wy-0.25*width;
		y2= 0.5*wy+0.25*width;
		x1= 0.5*wx - 0.5*fac*imax;
		x2= 0.5*wx + 0.5*fac*imax;
	}


	/* zoom van viewborder */
	fac= (1.41421+( (float)G.vd->camzoom )/50.0);
	fac*= fac;
	
	width= (x2-x1)/4.0;
	a= (x2+x1)/2.0;
	x1= a-width*fac;
	x2= a+width*fac;

	width= (y2-y1)/4.0;
	a= (y2+y1)/2.0;
	y1= a-width*fac;
	y2= a+width*fac;
	
	/* deze getallen voor renderdisplay */
	G.vd->pr_xmin= fceil(x1)+curarea->winrct.xmin;
	G.vd->pr_ymin= fceil(y1)+curarea->winrct.ymin;
	G.vd->pr_xmax= fceil(x2)+curarea->winrct.xmin;
	G.vd->pr_ymax= fceil(y2)+curarea->winrct.ymin;
}

void drawviewborder()
{
	float fac, a;
	float x1, x2, y1, y2;
	float x3, y3, x4, y4;

	x1= G.vd->pr_xmin - curarea->winrct.xmin;
	y1= G.vd->pr_ymin - curarea->winrct.ymin;
	x2= G.vd->pr_xmax - curarea->winrct.xmin;
	y2= G.vd->pr_ymax - curarea->winrct.ymin;

	/* rand */
	setlinestyle(1);
	cpack(0);
	sbox(x1+1, y1-1, x2+1, y2-1);
	cpack(0xFFFFFF);
	sbox(x1, y1, x2, y2);

	/* border */
	if(G.scene->r.mode & R_BORDER) {
		
		cpack(0);
		x3= x1+ G.scene->r.border.xmin*(x2-x1);
		y3= y1+ G.scene->r.border.ymin*(y2-y1);
		x4= x1+ G.scene->r.border.xmax*(x2-x1);
		y4= y1+ G.scene->r.border.ymax*(y2-y1);
		
		sbox(x3+1, y3-1, x4+1, y4-1);
		cpack(0x4040FF);
		sbox(x3, y3, x4, y4);
	}

	/* safetykader */
/* 	fac= R.safety*.001; */
	fac= 0.1;
	
	a= fac*(x2-x1);
	x1+= a; 
	x2-= a;

	a= fac*(y2-y1);
	y1+= a;
	y2-= a;

	cpack(0);
	sbox(x1+1, y1-1, x2+1, y2-1);
	cpack(0xFFFFFF);
	sbox(x1, y1, x2, y2);
	setlinestyle(0);

}

void backdrawview3d(test)
int test;
{
	struct Base *base;
	long tel=1;
	ulong kleur;

	if(G.machine==ENTRY) return;
	if(G.vd->flag & V3D_NEEDBACKBUFDRAW); else return;
	
	if(G.obedit) {
		G.vd->flag &= ~V3D_NEEDBACKBUFDRAW;
		return;
	}
	
	if(test) {
		if(qtest()) {
			addafterqueue(curarea->win, BACKBUFDRAW, 1);
			return;
		}
	}
	
	/* O2: er zeker van zijn dat de kleur ongewijzigd blijft */
	dither(DT_OFF);	

	if(G.vd->drawtype > OB_WIRE) G.zbuf= TRUE;
	curarea->win_equal= 0;
	
	if(G.zbuf) {
		myczclear(0);
		zbuffer(1);
	}
	else {
		cpack(0); 
		clear();
		zbuffer(0);
	}
	G.f |= G_BACKBUFSEL;
	
	if(G.f & (G_VERTEXPAINT|G_FACESELECT)) {
		base= BASACT;
		if(base && (base->lay && G.vd->lay)) {
			draw_object(base);
		}
	}
	else {

		base= FIRSTBASE;
		while(base) {
			
			/* elke base ivm meerdere windows */
			base->selcol= ((tel & 0xF00)<<12) + ((tel & 0xF0)<<8) + ((tel & 0xF)<<4);
			tel++;
	
			if(base->lay & G.vd->lay) {
				
				if(test) {
					if(qtest()) {
						G.f &= ~G_BACKBUFSEL;
						G.zbuf= FALSE; zbuffer(FALSE);
	
						addafterqueue(curarea->win, BACKBUFDRAW, 1);
						dither(DT_ON);	
						
						return;
					}
				}
	
				cpack(base->selcol);
				draw_object(base);
				
			}
			base= base->next;
		}
	}
	
	G.vd->flag &= ~V3D_NEEDBACKBUFDRAW;

	G.f &= ~G_BACKBUFSEL;
	G.zbuf= FALSE; zbuffer(FALSE);

	dither(DT_ON);	

}

		
void drawname(Object *ob)
{
	char str[8];
	
	cpack(0x404040);
	cmov(0.0, 0.0, 0.0);
	str[0]= ' '; str[1]= 0;
	fmsetfont(G.font);
	fmprstr(str);
	fmprstr(ob->id.name+2);
}


void drawview3d_simul(int make_disp)
{
	Object *ob;
	Sector *se;
	Life *lf, *lfl;
	Material *ma;
	ulong col;
	int b, a, flag;
	
	/* hier doen: anders wordt in header getekend */
	areawinset(curarea->win);

	setwinmatrixview3d(0);	/* 0= geen pick rect */

	setviewmatrixview3d();
	
	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);
	Mat4Invert(G.vd->persinv, G.vd->persmat);
	Mat4Invert(G.vd->viewinv, G.vd->viewmat);

	if(G.vd->drawtype > OB_WIRE) {
		G.zbuf= TRUE;
		zbuffer(TRUE);
		if(G.machine==ENTRY) myczclear(0x708070);
		else if(G.f & G_SIMULATION) myczclear(0x0);
		else myczclear(0x606060);

		loadmatrix(matone);
		bindlighting();
	}
	else {
		if(G.machine==ENTRY) cpack(0x708070);
		else cpack(0x606060);
		clear();
	}

	loadmatrix(G.vd->viewmat);
	
	if(G.machine==ENTRY || G.machine==VIDEO) linewidth(2);

	/* extern: camera tekenen */
	if(G.vd->persp!=2) {
		ob= G.scene->camera;
		if(ob && (ob->lay & G.vd->lay)) {
			cpack(0x0);
			multmatrix(ob->obmat);
			drawcamera(ob);
			loadmatrix(G.vd->viewmat);
		}
	}
	
	a= G.totsect;
	while(a-- > 0) {
		ob= G.sectorbuf[a]->ob;

		multmatrix(ob->obmat);
		cpack(0x0);
		drawsector(ob, MIN2(ob->dt, G.vd->drawtype), 0);
		loadmatrix(G.vd->viewmat);
		
		se= ob->data;
		
		if(se->lbuf.tot && se->depth<4) {
			for(b=0; b<se->lbuf.tot; b++) {
				ob= se->lbuf.ob[b];
				if(ob->lay & G.vd->lay) {
					lf= ob->data;
					
					if(lf->dflag & LF_DONTDRAW);
					else {
					
						multmatrix(ob->obmat);
						
						col= 0;
						if( ma=lf->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
						if( lf->flag & LF_DRAWNEAR) col= (0xF0A020);
						drawlife(ob, MIN2(ob->dt, G.vd->drawtype), col);
						if(ob->dtx & OB_DRAWNAME) drawname(ob);
						if(ob->dtx & OB_AXIS) drawaxes( lf->axsize);
						loadmatrix(G.vd->viewmat);
					}
				}
			}
		}
	}

	a= G.totlife;
	
	while(a--) {
		ob= G.lifebuf[a];
		if(ob->type==OB_LIFE && (ob->lay & G.vd->lay)) {
			
			lf= ob->data;

			if(lf->dflag & LF_DONTDRAW);
			else {
				multmatrix(ob->obmat);
				col= 0;
				if( ma=lf->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
				if( lf->flag & LF_DRAWNEAR) col= (0xF0A020);
				drawlife(ob, MIN2(ob->dt, G.vd->drawtype), col);
				if(ob->dtx & OB_DRAWNAME) drawname(ob);
				if(ob->dtx & OB_AXIS) drawaxes( lf->axsize);	
				loadmatrix(G.vd->viewmat);
			}

			if(lf->type==LF_DYNAMIC) {				
				for(b=0; b<lf->links.tot; b++) {
					
					ob= lf->links.ob[b];
					
					if(ob->type==OB_LIFE) {
						if(ob->lay & G.vd->lay) {
						
							lfl= ob->data;
		
							if(lfl->dflag & LF_DONTDRAW);
							else {
								multmatrix(ob->obmat);
								col= 0;
								if( ma=lfl->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
								if( lfl->flag & LF_DRAWNEAR) col= (0xF0A020);
								drawlife(ob, MIN2(ob->dt, G.vd->drawtype), col);
								if(ob->dtx & OB_DRAWNAME) drawname(ob);
								if(ob->dtx & OB_AXIS) drawaxes( lfl->axsize);	
								loadmatrix(G.vd->viewmat);
							}
						}
					}
				}
			}
		}
	}

	if(G.zbuf) {
		G.zbuf= FALSE;
		zbuffer(FALSE);
	}
	
	if(G.vd->persp>1) {
		persp(0);
		drawviewborder();
		persp(1);
	}
	draw_gamedebug_info();	/* life.c */

	curarea->win_swap= 1;
}

void draw_smaller(int zoom, int mode)
{
	rcti *rt;
	int sx, sy;
	
	rt= &curarea->winrct;
	sx= curarea->winx/zoom;
	sy= curarea->winy/zoom;
	
	if(mode) {
		viewport(rt->xmin, rt->xmin+sx, rt->ymin, rt->ymin+sy);
	}
	else {
		viewport(rt->xmin, rt->xmax, rt->ymin, rt->ymax);
		rectzoom( (float)zoom, (float)zoom);
		rectcopy(rt->xmin, rt->ymin, rt->xmin+sx, rt->ymin+sx, rt->xmin, rt->ymin);
		rectzoom(1.0, 1.0);
	}	
}

void drawview3d()
{
	Base *base;
	Object *ob;
	int flag;
	
	setwinmatrixview3d(0);	/* 0= geen pick rect */

	setviewmatrixview3d();

	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);
	Mat4Invert(G.vd->persinv, G.vd->persmat);
	Mat4Invert(G.vd->viewinv, G.vd->viewmat);

	if(G.vd->drawtype > OB_WIRE) {
		G.zbuf= TRUE;
		zbuffer(TRUE);
		if(G.f & G_SIMULATION) {
			myczclear(0);
		}
		else {
			if(G.machine==ENTRY) myczclear(0x708070);
			else myczclear(0x707070);
		}
		
		loadmatrix(matone);
		bindlighting();
	}
	else {
		if(G.machine==ENTRY) cpack(0x708070);
		else cpack(0x707070);
		clear();
	}
	
	loadmatrix(G.vd->viewmat);
	
	linewidth(1);
		
	/* hele rare O2 patch: als je dit weglaat paktie de eerste cpack niet */
	bgnline(); endline();

	if(G.vd->view==0 || G.vd->persp!=0) {
		drawfloor();
		if(G.vd->persp==2) {
			if(G.scene->world) {
				if(G.scene->world->mode & WO_STARS) make_stars(1);
			}
			calc_viewborder();
			if(G.vd->flag & V3D_DISPBGPIC) draw_bgpic();
		}
	}
	else {
		drawgrid();
		if(G.vd->flag & V3D_DISPBGPIC) draw_bgpic();
	}
	
	if(G.machine==ENTRY || G.machine==VIDEO) linewidth(2);

	
	/* eerst set tekenen */
	if(G.scene->set) {
	
		/* patchje: kleur blijft constant */ 
		G.f |= G_PICKSEL;
		cpack(0x404040);
		
		base= G.scene->set->base.first;
		while(base) {
			if(G.vd->lay & base->lay) {
				where_is_object(base->object);
				draw_object(base);

				if(base->object->transflag & OB_DUPLI) {
					extern ListBase duplilist;
					Base tbase;
					
					tbase.flag= OB_FROMDUPLI;
					make_duplilist(G.scene->set, base->object);
					ob= duplilist.first;
					while(ob) {
						tbase.object= ob;
						draw_object(&tbase);
						ob= ob->id.next;
					}
					free_duplilist();
					
				}
			}
			base= base->next;
		}
		
		G.f &= ~G_PICKSEL;
	}
	
	
	/* eerst niet selected en dupli's */
	base= G.scene->base.first;
	while(base) {
		
		if(G.vd->lay & base->lay) {
		
			where_is_object(base->object);

			if(base->object->transflag & OB_DUPLI) {
				extern ListBase duplilist;
				Base tbase;
				
				/* altijd eerst original tekenen: displisten goedzetten */
				draw_object(base);
				
				/* patchje: kleur blijft constant */ 
				G.f |= G_PICKSEL;
				cpack(0x404040);
				
				tbase.flag= OB_FROMDUPLI;
				make_duplilist(G.scene, base->object);
				ob= duplilist.first;
				while(ob) {
					tbase.object= ob;
					draw_object(&tbase);
					ob= ob->id.next;
				}
				free_duplilist();
				
				G.f &= ~G_PICKSEL;				
			}
			else if((base->flag & SELECT)==0) {
				draw_object(base);
			}
			
		}
		
		base= base->next;
	}
	/*  selected */
	base= G.scene->base.first;
	while(base) {
		
		if TESTBASE(base) {
			draw_object(base);
		}
		
		base= base->next;
	}

	persp(0);
	
	/* if(G.vd->drawtype>=OB_SOLID) draw_smaller(2, 0); */

	if(G.vd->persp>1) drawviewborder();
	drawcursor();
	
	persp(1);

	G.vd->flag &= ~V3D_DISPIMAGE;
	curarea->win_swap= 1;
	
	if(G.zbuf) {
		G.zbuf= FALSE;
		zbuffer(FALSE);
	}
	
	G.vd->flag |= V3D_NEEDBACKBUFDRAW;
	addafterqueue(curarea->win, BACKBUFDRAW, 1);
	
}


double tottime = 0.0;

int update_time()
{
	struct tms voidbuf;
	static long ltime;
	long time;

	time = times(&voidbuf);
	tottime += (time - ltime) / 100.0;
	ltime = time;
	return (tottime < 0.0);
}

double speed_to_swaptime(int speed)
{
	switch(speed) {
	case 1:
		return 1.0/60.0;
	case 2:
		return 1.0/50.0;
	case 3:
		return 1.0/30.0;
	case 4:
		return 1.0/25.0;
	case 5:
		return 1.0/20.0;
	case 6:
		return 1.0/15.0;
	case 7:
		return 1.0/12.5;
	case 8:
		return 1.0/10.0;
	case 9:
		return 1.0/6.0;
	}
}

double key_to_swaptime(int key)
{
	switch(key) {
	case PAD1:
		G.animspeed= 1;
		tottime= 0;
		return speed_to_swaptime(1);
	case PAD2:
		G.animspeed= 2;
		tottime= 0;
		return speed_to_swaptime(2);
	case PAD3:
		G.animspeed= 3;
		tottime= 0;
		return speed_to_swaptime(3);
	case PAD4:
		G.animspeed= 4;
		tottime= 0;
		return speed_to_swaptime(4);
	case PAD5:
		G.animspeed= 5;
		tottime= 0;
		return speed_to_swaptime(5);
	case PAD6:
		G.animspeed= 6;
		tottime= 0;
		return speed_to_swaptime(6);
	case PAD7:
		G.animspeed= 7;
		tottime= 0;
		return speed_to_swaptime(7);
	case PAD8:
		G.animspeed= 8;
		tottime= 0;
		return speed_to_swaptime(8);
	case PAD9:
		G.animspeed= 9;
		tottime= 0;
		return speed_to_swaptime(9);
	}
	
	return speed_to_swaptime(G.animspeed);
}

void play_anim(int mode)
{
	ScrArea *sa, *oldsa;
	double swaptime;
	int cfra, cfraont;
	short event=0, val, mval[2];
	char str[12];
	
	/* patch voor zeer oude scenes */
	if(SFRA==0) SFRA= 1;
	if(EFRA==0) EFRA= 250;
	
	if(SFRA>EFRA) return;
	
	update_time();
	tottime= 0.0;
	
	swaptime= speed_to_swaptime(G.animspeed);

	waitcursor(1);
	G.f |= G_PLAYANIM;		/* in sequence.c en view.c wordt dit afgevangen */

	if(G.scene->r.scemode & R_NETWORK) {
		init_render_camera_network();
	}
	
	cfraont= CFRA;
	oldsa= curarea;
	
	while(TRUE) {
		
		if (tottime > 0.0) tottime = 0.0;
		
		for(; CFRA<=EFRA; CFRA++) {
			set_timecursor(CFRA);
			do_all_ipos();
			do_all_keys();
			do_all_ikas();
			test_all_displists();
			
			sa= G.curscreen->areabase.first;
			while(sa) {
				if(sa==oldsa) {
					if(sa->win && sa->windraw) {
						/* hier winget() gebruiken: anders wordt in header getekend */
						if(sa->win != winget()) areawinset(sa->win);
						sa->windraw();
					}
				}
				else if(mode) {
					if ELEM(sa->spacetype, SPACE_VIEW3D, SPACE_SEQ) {
						if(sa->win && sa->windraw) {
							/* hier winget() gebruiken: anders wordt in header getekend */
							if(sa->win != winget()) areawinset(sa->win);
							sa->windraw();
						}
					}
				}
				
				sa= sa->next;	
			}
			
			/* minimaal swaptime laten voorbijgaan */
			tottime -= swaptime;
			while (update_time()) sginap(1);

			screen_swapbuffers();
			
			tottime= 0.0;
			
			while(qtest()) {
				event= extern_qread(&val);
				if(event==ESCKEY) break;
				
				if(val) {
					swaptime= key_to_swaptime(event);
				}
			}
			if(event==ESCKEY || event==SPACEKEY) break;
		}
		if(event==ESCKEY || event==SPACEKEY) break;
		
		CFRA= SFRA;
	}

	if(event==SPACEKEY);
	else CFRA= cfraont;
	
	do_all_ipos();
	do_all_keys();
	do_all_ikas();

	if(oldsa!=curarea) areawinset(oldsa->win);
	
	/* restore all areas */
	sa= G.curscreen->areabase.first;
	while(sa) {
		if( (mode && sa->spacetype==SPACE_VIEW3D) || sa==curarea) addqueue(sa->win, REDRAW, 1);
		
		sa= sa->next;	
	}
	
	/* speed button */
	allqueue(REDRAWBUTSANIM, 0);
	
	/* vooropig */
	do_global_buttons(B_NEWFRAME);

	if(G.scene->r.scemode & R_NETWORK) {
		end_render_camera_network();
		allqueue(REDRAWBUTSALL, 0);		/* efra */
	}
	
	waitcursor(0);
	G.f &= ~G_PLAYANIM;
}

