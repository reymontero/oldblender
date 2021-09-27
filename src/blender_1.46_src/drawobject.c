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

/* drawobject.c  		GRAPHICS

 * 
 * jan 95
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "edit.h"
#include "effect.h"
#include "sector.h"

ulong rect_desel[16]= {0x707070,0x0,0x0,0x707070,0x407070,0x70cccc,0x407070,0x0,0xaaffff,0xffffff,0x70cccc,0x0,0x70cccc,0xaaffff,0x407070,0x707070};
ulong rect_sel[16]= {0x707070,0x0,0x0,0x707070,0x702070,0xcc50cc,0x702070,0x0,0xff80ff,0xffffff,0xcc50cc,0x0,0xcc50cc,0xff80ff,0x702070,0x707070};

ulong rectu_desel[16]= {0xff4e4e4e,0xff5c2309,0xff000000,0xff4e4f4d,0xff000000,0xffff9d72,0xffff601c,0xff000000,0xff5d2409,0xffffffff,0xffff9d72,0xff5b2209,0xff4e4e4e,0xff5c2309,0xff010100,0xff4f4f4f};
ulong rectu_sel[16]= {0xff4e4e4e,0xff403c00,0xff000000,0xff4e4e4d,0xff000000,0xfffff64c,0xffaaa100,0xff000000,0xff403c00,0xffffffff,0xfffff64c,0xff403c00,0xff4f4f4f,0xff403c00,0xff010100,0xff4e4e4e};

ulong rectl_desel[81]= {0x777777,0x777777,0xa9fefe,0xaaffff,0xaaffff,0xaaffff,0xaaffff,0x777777,0x777777,0x777777,0xa9fefe,0xaafefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0x777777,0xaaffff,0xa9fefe,0x4e4e4e,0x0,0x124040,0x0,0x4e4e4e,0xaafefe,0xaaffff,0xaaffff,0x777777,0x0,0x227777,0x55cccc,0x227777,0x0,0x777777,0xaaffff,0xaaffff,0x777777,0x124040,0x88ffff,0xffffff,0x55cccc,0x124040,0x777777,0xaaffff,0xaaffff,0x777777,0x0,0x55cccc,0x88ffff,0x227777,0x0,0x777777,0xaaffff,0xaafefe,0xaafefe,0x4f4f4f,0x0,0x124040,0x0,0x4e4e4e,0xa9fefe,0xaaffff,0x777777,0xa9fefe,0xa9fefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0xaaffff,0xaaffff,0xaaffff,0x777777,0x777777};
ulong rectl_sel[81]= {0x777777,0x777777,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0xffaaff,0xffaaff,0x4e4e4e,0x10101,0x402440,0x0,0x4e4e4e,0xffaaff,0xffaaff,0xffaaff,0x777777,0x0,0x774477,0xcc77cc,0x774477,0x0,0x777777,0xffaaff,0xffaaff,0x777777,0x402440,0xffaaff,0xffffff,0xcc77cc,0x412541,0x777777,0xffaaff,0xffaaff,0x777777,0x10101,0xcc77cc,0xffaaff,0x774477,0x0,0x777777,0xffaaff,0xffaaff,0xffaaff,0x4e4e4e,0x10101,0x402440,0x0,0x4e4e4e,0xffaaff,0xffaaff,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0x777777,0x777777};
ulong rectlus_desel[81]= {0x777777,0x777777,0xa9fefe,0xaaffff,0xaaffff,0xaaffff,0xaaffff,0x777777,0x777777,0x777777,0xa9fefe,0xaafefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0x777777,0xaaffff,0xa9fefe,0x4e4e4e,0x0,0x5c2309,0x0,0x4e4f4d,0xaafefe,0xaaffff,0xaaffff,0x777777,0x0,0xff601c,0xff9d72,0xff601c,0x0,0x777777,0xaaffff,0xaaffff,0x777777,0x5d2409,0xffceb8,0xff9d72,0xff9d72,0x5b2209,0x777777,0xaaffff,0xaaffff,0x777777,0x10100,0xffceb8,0xffceb8,0xff601c,0x0,0x777777,0xaaffff,0xaafefe,0xaafefe,0x4e4e4e,0x0,0x5c2309,0x10100,0x4f4f4f,0xa9fefe,0xaaffff,0x777777,0xa9fefe,0xa9fefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0x777777,0x777777,0x777777,0xa9fefe,0xa9fefe,0xaaffff,0xaaffff,0xaaffff,0x777777,0x777777};
ulong rectlus_sel[81]= {0x777777,0x777777,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0xffaaff,0xffaaff,0x4e4e4e,0x10100,0x403c00,0x0,0x4e4e4d,0xffaaff,0xffaaff,0xffaaff,0x777777,0x0,0xaaa100,0xfff64c,0xaaa100,0x0,0x777777,0xffaaff,0xffaaff,0x777777,0x403c00,0xfffde2,0xffffff,0xfff64c,0x403c00,0x777777,0xffaaff,0xffaaff,0x777777,0x10100,0xfff64c,0xfffde2,0xaaa100,0x0,0x777777,0xffaaff,0xffaaff,0xffaaff,0x4f4f4f,0x0,0x403c00,0x10100,0x4e4e4e,0xffaaff,0xffaaff,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0x777777,0x777777,0x777777,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0xffaaff,0x777777,0x777777};
ulong rectllib_desel[81]= {0xff777777,0xff777777,0xb9b237,0xb9b237,0xb9b237,0xb9b237,0xb9b237,0xff777777,0xff777777,0xff777777,0xb9b237,0xb9b237,0xff777777,0xff777777,0xff777777,0xb9b237,0xb9b237,0xff777777,0xb9b237,0xb9b237,0x4e4e4e,0x0,0x5c2309,0x0,0x4e4f4d,0xb9b237,0xb9b237,0xb9b237,0xff777777,0x0,0xff601c,0xff9d72,0xff601c,0x0,0xff777777,0xb9b237,0xb9b237,0xff777777,0x5d2409,0xffceb8,0xff9d72,0xff9d72,0x5b2209,0xff777777,0xb9b237,0xb9b237,0xff777777,0x10100,0xffceb8,0xffceb8,0xff601c,0x0,0xff777777,0xb9b237,0xb9b237,0xb9b237,0x4e4e4e,0x0,0x5c2309,0x10100,0x4f4f4f,0xb9b237,0xb9b237,0xff777777,0xb9b237,0xb9b237,0xff777777,0xff777777,0xff777777,0xb9b237,0xb9b237,0xff777777,0xff777777,0xff777777,0xb9b237,0xb9b237,0xb9b237,0xb9b237,0xb9b237,0xff777777,0xff777777};
ulong rectllib_sel[81]= {0xff777777,0xff777777,0xfff64c,0xfff64c,0xfff64c,0xfff64c,0xfff64c,0xff777777,0xff777777,0xff777777,0xfff64c,0xfff64c,0xff777777,0xff777777,0xff777777,0xfff64c,0xfff64c,0xff777777,0xfff64c,0xfff64c,0x4e4e4e,0x10100,0x403c00,0x0,0x4e4e4d,0xfff64c,0xfff64c,0xfff64c,0xff777777,0x0,0xaaa100,0xfff64c,0xaaa100,0x0,0xff777777,0xfff64c,0xfff64c,0xff777777,0x403c00,0xfffde2,0xffffff,0xfff64c,0x403c00,0xff777777,0xfff64c,0xfff64c,0xff777777,0x10100,0xfff64c,0xfffde2,0xaaa100,0x0,0xff777777,0xfff64c,0xfff64c,0xfff64c,0x4f4f4f,0x0,0x403c00,0x10100,0x4e4e4e,0xfff64c,0xfff64c,0xff777777,0xfff64c,0xfff64c,0xff777777,0xff777777,0xff777777,0xfff64c,0xfff64c,0xff777777,0xff777777,0xff777777,0xfff64c,0xfff64c,0xfff64c,0xfff64c,0xfff64c,0xff777777,0xff777777};

ulong rectl_set[81]= {0xff777777,0xff777777,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xff777777,0xff777777,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0xff777777,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0xaaaaaa,0xaaaaaa,0x4e4e4e,0x10100,0x202020,0x0,0x4e4e4d,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xff777777,0x0,0xaaa100,0xaaaaaa,0xaaa100,0x0,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0x202020,0xfffde2,0xffffff,0xaaaaaa,0x202020,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0x10100,0xaaaaaa,0xfffde2,0xaaa100,0x0,0xff777777,0xaaaaaa,0xaaaaaa,0xaaaaaa,0x4f4f4f,0x0,0x202020,0x10100,0x4e4e4e,0xaaaaaa,0xaaaaaa,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0xff777777,0xff777777,0xaaaaaa,0xaaaaaa,0xff777777,0xff777777,0xff777777,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xaaaaaa,0xff777777,0xff777777};

ulong rectblack[3][3]={{0,0,0},{0,0,0},{0,0,0}};
ulong rectwhite[3][3]={{0xFFFFFF,0xFFFFFF,0xFFFFFF},
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

void drawboundbox(Object *ob);
void drawmeshwire(Object *ob);

ulong selcol= 0xFF88FF;
ulong actselcol= 0xFFBBFF;

ulong colortab[14]=
	{0x0,		0xFF88FF, 0xFFBBFF, 
	 0x403000,	0xFFFF88, 0xFFFFBB, 
	 0x103040,	0x66CCCC, 0x77CCCC, 
	 0xFFFFFF
	};


float cube[8][3] = {
	{-1.0, -1.0, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.0,  1.0, -1.0},
	{ 1.0, -1.0, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.0,  1.0, -1.0},
};

float cube1[8][3] = {
	{-1.2, -1.2, -1.0},
	{-1.0, -1.0,  1.0},
	{-1.0,  1.0,  1.0},
	{-1.2,  1.2, -1.0},
	{ 1.2, -1.2, -1.0},
	{ 1.0, -1.0,  1.0},
	{ 1.0,  1.0,  1.0},
	{ 1.2,  1.2, -1.0},
};

void tekenrect9o(short sx, short sy, ulong *rect)
{
	ulong rectt[81], *r;
	int a;

	if(sx<curarea->winx && sx>0 && sy<curarea->winy && sy>0) {

		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		lrectread(sx-4, sy-4, sx+4, sy+4, rectt);
		r= rectt;
		a= 81;
		while(a--) {
			if(*rect!=0) *r= *rect;
			r++; rect++;
		}
		lrectwrite(sx-4, sy-4, sx+4, sy+4, rectt);
	}
}

void tekenrect9(short sx, short sy, ulong *rect)
{

	if(sx<curarea->winx && sx>0 && sy<curarea->winy && sy>0) {

		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		lrectwrite(sx-4, sy-4, sx+4, sy+4, rect);
	}
}

void tekenrect4(short sx, short sy, ulong *rect)
{

	if(sx<curarea->winx && sx>0 && sy<curarea->winy && sy>0) {
		
		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		lrectwrite(sx-1, sy-1, sx+2, sy+2, rect);
	}
		
}

void tekenrect3(short sx, short sy, ulong *rect)
{

	if(sx!=3200) {
		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		lrectwrite(sx-1, sy-1, sx+1, sy+1, rect);
	}
}

void tekenrect2(short sx, short sy, ulong *rect)
{

	if(sx!=3200) {
		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		lrectwrite(sx, sy, sx+1, sy+1, rect);
	}
}

void tekenrect_col(short size, short sx, short sy, ulong col)
{
	ulong rect[9];
	
	if(sx!=3200) {
		sx+= curarea->winrct.xmin;
		sy+= curarea->winrct.ymin;

		if(size==2) {
			size=3;
			while(size>=0) rect[size--]= col;
			lrectwrite(sx, sy, sx+1, sy+1, rect);
		}
		else if(size==3) {
			size=8;
			while(size>=0) rect[size--]= col;
			lrectwrite(sx-1, sy-1, sx+1, sy+1, rect);
		}
	}
}

void drawaxes(float size)
{
	float v1[3], v2[3];
	float f800, f125;

	f125= 0.125*size;
	f800= 0.8*size;
	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	
	/* X-AS: pijl */
	v2[0]= size;
	LINE3F(v1, v2);
	v1[0]= f800;
	v1[1]= -f125;
	LINE3F(v1, v2);
	v1[1]= f125;
	LINE3F(v1, v2);
	
	cmov(size+f125, 0.0, 0.0);
	charstr("x");
	
	/* Y-AS: pijl */
	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	v2[1]= size;
	LINE3F(v1, v2);
	v1[1]= f800;
	v1[0]= -f125;
	LINE3F(v1, v2);
	v1[0]= f125;
	LINE3F(v1, v2);
	
	cmov(0.0, size+f125, 0.0);
	charstr("y");
	
	/* Z-AS: pijl */
	v1[0]=v1[1]=v1[2]=v2[0]=v2[1]=v2[2]= 0.0;
	v2[2]= size;
	LINE3F(v1, v2);
	v1[2]= f800;
	v1[0]= -f125;
	LINE3F(v1, v2);
	v1[0]= f125;
	LINE3F(v1, v2);

	cmov(0.0, 0.0, size+f125);
	charstr("z");
	
}

void drawgourcube()
{
	float vec[3];
	float n[3];

	n[0]=0; n[1]=0; n[2]=0;
	bgnpolygon();
		n[0]= -1.0;
		n3f(n); 
		v3f(cube[0]); v3f(cube[1]); v3f(cube[2]); v3f(cube[3]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= -1.0;
		n3f(n); 
		v3f(cube[0]); v3f(cube[4]); v3f(cube[5]); v3f(cube[1]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[0]= 1.0;
		n3f(n); 
		v3f(cube[4]); v3f(cube[7]); v3f(cube[6]); v3f(cube[5]);
		n[0]=0;
	endpolygon();

	bgnpolygon();
		n[1]= 1.0;
		n3f(n); 
		v3f(cube[7]); v3f(cube[3]); v3f(cube[2]); v3f(cube[6]);
		n[1]=0;
	endpolygon();

	bgnpolygon();
		n[2]= 1.0;
		n3f(n); 
		v3f(cube[1]); v3f(cube[5]); v3f(cube[6]); v3f(cube[2]);
		n[2]=0;
	endpolygon();

	bgnpolygon();
		n[2]= -1.0;
		n3f(n); 
		v3f(cube[7]); v3f(cube[4]); v3f(cube[0]); v3f(cube[3]);
	endpolygon();
}

void drawcube()
{

	bgnline();
		v3f(cube[0]); v3f(cube[1]);v3f(cube[2]); v3f(cube[3]);
		v3f(cube[0]); v3f(cube[4]);v3f(cube[5]); v3f(cube[6]);
		v3f(cube[7]); v3f(cube[4]);
	endline();
	bgnline();
		v3f(cube[1]); v3f(cube[5]);
	endline();
	bgnline();
		v3f(cube[2]); v3f(cube[6]);
	endline();
	bgnline();
		v3f(cube[3]); v3f(cube[7]);
	endline();
}

void drawcube_size(float *size)
{

	pushmatrix();
	scale(size[0], size[1], size[2]);
	
	bgnline();
		v3f(cube[0]); v3f(cube[1]);v3f(cube[2]); v3f(cube[3]);
		v3f(cube[0]); v3f(cube[4]);v3f(cube[5]); v3f(cube[6]);
		v3f(cube[7]); v3f(cube[4]);
	endline();
	bgnline();
		v3f(cube[1]); v3f(cube[5]);
	endline();
	bgnline();
		v3f(cube[2]); v3f(cube[6]);
	endline();
	bgnline();
		v3f(cube[3]); v3f(cube[7]);
	endline();
	
	popmatrix();
}


void tekenshadbuflimits(Lamp *la, float mat[][4])
{
	float sta[3], end[3], lavec[3];
	short s[2];

	lavec[0]= -mat[2][0];
	lavec[1]= -mat[2][1];
	lavec[2]= -mat[2][2];
	Normalise(lavec);

	sta[0]= mat[3][0]+ la->clipsta*lavec[0];
	sta[1]= mat[3][1]+ la->clipsta*lavec[1];
	sta[2]= mat[3][2]+ la->clipsta*lavec[2];

	end[0]= mat[3][0]+ la->clipend*lavec[0];
	end[1]= mat[3][1]+ la->clipend*lavec[1];
	end[2]= mat[3][2]+ la->clipend*lavec[2];

	bgnline();
		v3f(sta);
		v3f(end);
	endline();

	project_short(sta, s);
	if(s[0]!=3200) tekenrect3(s[0], s[1], rectwhite[0]);
	project_short(end, s);
	if(s[0]!=3200) tekenrect3(s[0], s[1], rectwhite[0]);
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



void drawlamp(Object *ob)
{
	Lamp *la;
	float vec[3], lvec[3], vvec[3],x,y,z;
	
	la= ob->data;
	vec[0]=vec[1]=vec[2]= 0.0;
	
	setlinestyle(1);
	if(la->type==LA_SPOT) {
		
		lvec[0]=lvec[1]= 0.0; 
		lvec[2] = 1.0;
		x = G.vd->persmat[0][2];
		y = G.vd->persmat[1][2];
		z = G.vd->persmat[2][2];
		vvec[0]= x*ob->obmat[0][0] + y*ob->obmat[0][1] + z*ob->obmat[0][2];
		vvec[1]= x*ob->obmat[1][0] + y*ob->obmat[1][1] + z*ob->obmat[1][2];
		vvec[2]= x*ob->obmat[2][0] + y*ob->obmat[2][1] + z*ob->obmat[2][2];

		y = fcos( M_PI*la->spotsize/360.0 );
		spotvolume(lvec, vvec, y);
		x = -la->dist;
		lvec[0] *=  x ; 
		lvec[1] *=  x ; 
		lvec[2] *=  x;
		vvec[0] *= x ; 
		vvec[1] *= x ; 
		vvec[2] *= x;
		bgnline();
			v3f(vvec);
			v3f(vec);
			v3f(lvec);
		endline();
		z = x*fsqrt(1 - y*y);
		x *= y;
		translate(0.0 , 0.0 , x);
		circ(0.0 , 0.0 , z);
	}
	else if ELEM(la->type, LA_HEMI, LA_SUN) {
		bgnline();
			v3f(vec); 
			vec[2]= -la->dist; 
			v3f(vec);
		endline();
	}
	else {
		if(la->mode & LA_SPHERE) {

			float tmat[4][4], imat[4][4];
			
			vec[0]= vec[1]= vec[2]= 0.0;
			getmatrix(tmat);
			Mat4Invert(imat, tmat);
			
			drawcircball(vec, la->dist, imat);

		}
	}
	loadmatrix(G.vd->viewmat);
	
	VECCOPY(vec, ob->obmat[3]);
	bgnline();
		v3f(vec); 
		vec[2]= 0; 
		v3f(vec);
	endline();
	setlinestyle(0);
	
	if(la->type==LA_SPOT && (la->mode & LA_SHAD) ) {
		tekenshadbuflimits(la, ob->obmat);
	}
}

void draw_limit_line(float sta, float end, ulong *rect)
{
	float vec[2][3];
	short mval[2];
	
	vec[0][0]= 0.0;
	vec[0][1]= 0.0;
	vec[0][2]= -sta;

	vec[1][0]= 0.0;
	vec[1][1]= 0.0;
	vec[1][2]= -end;
	
	LINE3F(vec[0], vec[1]);

	project_short(vec[0], mval);
	if(mval[0]!=3200) tekenrect3(mval[0], mval[1], rect);
	project_short(vec[1], mval);
	if(mval[0]!=3200) tekenrect3(mval[0], mval[1], rect);
}		


void drawcamera(Object *ob)
{
	/* een staande piramide met (0,0,0) als top */
	Camera *cam;
	World *wrld;
	float vec[8][3], tmat[4][4], fac, facx, facy, depth;
	short mval[2];
	
	cam= ob->data;
	
	/* zo is ie altijd te zien */
	fac= cam->drawsize;
	if(G.vd->persp>=2) fac= cam->clipsta+0.1;
	
	depth= - fac*cam->lens/16.0;
	facx= fac*1.28;
	facy= fac*1.024;
	
	vec[0][0]= 0; vec[0][1]= 0; vec[0][2]= 0.001;	/* GLBUG: z niet op nul vanwege picking op entry */
	vec[1][0]= facx; vec[1][1]= facy; vec[1][2]= depth;
	vec[2][0]= facx; vec[2][1]= -facy; vec[2][2]= depth;
	vec[3][0]= -facx; vec[3][1]= -facy; vec[3][2]= depth;
	vec[4][0]= -facx; vec[4][1]= facy; vec[4][2]= depth;

	bgnclosedline();
		v3f(vec[0]); 
		v3f(vec[1]); 
		v3f(vec[2]); 
		v3f(vec[0]); 
		v3f(vec[3]); 
		v3f(vec[4]);
	endclosedline();

	bgnline();
		v3f(vec[2]); 
		v3f(vec[3]);
	endline();
	bgnline();
		v3f(vec[4]); 
		v3f(vec[1]);
	endline();

	/* pijl aan top */
	vec[0][2]= depth;

	if(G.vd->persp>=2) return;
	
	concave(1);
	bgnpolygon();

		vec[0][0]= -0.2*cam->drawsize; 
		vec[0][1]= cam->drawsize;
		v3f(vec[0]);
		vec[0][0]= 0.2*cam->drawsize;
		v3f(vec[0]);
		vec[0][1]= 1.6*cam->drawsize;
		v3f(vec[0]);
		vec[0][0]= 0.4*cam->drawsize;
		v3f(vec[0]);
		vec[0][0]= 0.0; 
		vec[0][1]= 2.0*cam->drawsize;
		v3f(vec[0]);
		vec[0][0]= -0.4*cam->drawsize; 
		vec[0][1]= 1.6*cam->drawsize;
		v3f(vec[0]);
		vec[0][0]= -0.2*cam->drawsize;
		v3f(vec[0]);
	
	endpolygon();
	concave(0);
	
	if(cam->flag & (CAM_SHOWLIMITS+CAM_SHOWMIST+CAM_SHOWNETCLIP)) {
		loadmatrix(G.vd->viewmat);
		Mat4CpyMat4(vec, ob->obmat);
		Mat4Ortho(vec);
		multmatrix(vec);

		Mat4SwapMat4(G.vd->persmat, tmat);
		mygetsingmatrix(G.vd->persmat);

		if(cam->flag & CAM_SHOWLIMITS) 
			draw_limit_line(cam->clipsta, cam->clipend, (ulong *)rectyellow);

		if(cam->flag & CAM_SHOWNETCLIP) 
			draw_limit_line(cam->netsta, cam->netend, (ulong *)rectblack);
		
		wrld= G.scene->world;
		if(cam->flag & CAM_SHOWMIST) 
			if(wrld) draw_limit_line(wrld->miststa, wrld->miststa+wrld->mistdist, (ulong *)rectwhite);
			
		Mat4SwapMat4(G.vd->persmat, tmat);
	}
}

void tekenvertslatt(sel)
short sel;
{
	Lattice *lt;
	BPoint *bp;
	ulong *rect;
	int a, uxt, u, vxt, v, wxt, w;

	if(sel) rect= (ulong *)rectyellow;
	else rect= (ulong *)rectpurple;

	bp= editLatt->def;
	lt= editLatt;
	
	if(lt->flag & LT_OUTSIDE) {
		
		for(w=0; w<lt->pntsw; w++) {
			if(w==0 || w==lt->pntsw-1) wxt= 1; else wxt= 0;
			for(v=0; v<lt->pntsv; v++) {
				if(v==0 || v==lt->pntsv-1) vxt= 1; else vxt= 0;
				
				for(u=0; u<lt->pntsu; u++, bp++) {
					if(u==0 || u==lt->pntsu-1) uxt= 1; else uxt= 0;
					if(uxt || vxt || wxt) {
						if(bp->hide==0) {
							if((bp->f1 & 1)==sel) tekenrect3(bp->s[0], bp->s[1], rect);
						}
					}
				}
			}
		}
	}
	else {

		a= editLatt->pntsu*editLatt->pntsv*editLatt->pntsw;
		while(a--) {
			if(bp->hide==0) {
				if((bp->f1 & 1)==sel) tekenrect3(bp->s[0], bp->s[1], rect);
			}
			bp++;
		}
	}
}

void calc_lattverts()
{
	BPoint *bp;
	float mat[4][4];
	int a, tot, b;
	short *sp;

	Mat4SwapMat4(G.vd->persmat, mat);
	mygetsingmatrix(G.vd->persmat);
	
	 bp= editLatt->def;
	
	a= editLatt->pntsu*editLatt->pntsv*editLatt->pntsw;
	while(a--) {
		project_short(bp->vec, bp->s);
		bp++;
	}

	Mat4SwapMat4(G.vd->persmat, mat);
}


void calc_lattverts_ext()
{

	areawinset(curarea->win);
	multmatrix(G.obedit->obmat);
	calc_lattverts();
	loadmatrix(G.vd->viewmat);
	
}


void drawlattice(Object *ob)
{
	Lattice *lt;
	BPoint *bp, *bpu;
	int u, v, w, dv, dw, uxt, vxt, wxt;

	lt= ob->data;
	if(ob==G.obedit) {
		bp= editLatt->def;
		
		cpack(0x004000);
	}
	else {
		bp= lt->def;
	}
	
	dv= lt->pntsu;
	dw= dv*lt->pntsv;
	
	if(lt->flag & LT_OUTSIDE) {
		
		for(w=0; w<lt->pntsw; w++) {
			
			if(w==0 || w==lt->pntsw-1) wxt= 1; else wxt= 0;
			
			for(v=0; v<lt->pntsv; v++) {
				
				if(v==0 || v==lt->pntsv-1) vxt= 1; else vxt= 0;
				
				for(u=0, bpu=0; u<lt->pntsu; u++, bp++) {
				
					if(u==0 || u==lt->pntsu-1) uxt= 1; else uxt= 0;
					
					if(uxt || vxt || wxt) {
					
						if(w && (uxt || vxt)) {
							bgnline();
							v3f( (bp-dw)->vec ); v3f(bp->vec);
							endline();
						}
						if(v && (uxt || wxt)) {
							bgnline();
							v3f( (bp-dv)->vec ); v3f(bp->vec);
							endline();
						}
						if(u && (vxt || wxt)) {
							bgnline();
							v3f(bpu->vec); v3f(bp->vec);
							endline();
						}
					}
					
					bpu= bp;
				}
			}
		}		
	}
	else {
		for(w=0; w<lt->pntsw; w++) {
			
			for(v=0; v<lt->pntsv; v++) {
				
				for(u=0, bpu=0; u<lt->pntsu; u++, bp++) {
				
					if(w) {
						bgnline();
						v3f( (bp-dw)->vec ); v3f(bp->vec);
						endline();
					}
					if(v) {
						bgnline();
						v3f( (bp-dv)->vec ); v3f(bp->vec);
						endline();
					}
					if(u) {
						bgnline();
						v3f(bpu->vec); v3f(bp->vec);
						endline();
					}
					bpu= bp;
				}
			}
		}
	}
	
	if(ob==G.obedit) {
		
		calc_lattverts();
		
		if(G.zbuf) zbuffer(0);
		
		tekenvertslatt(0);
		tekenvertslatt(1);
		
		if(G.zbuf) zbuffer(1);
	}
}

/* ***************** ******************** */

void calc_meshverts()
{
	EditVert *eve;
	float mat[4][4];

	if(G.edve.first==0) return;
	eve= G.edve.first;

	Mat4SwapMat4(G.vd->persmat, mat);
	mygetsingmatrix(G.vd->persmat);

	eve= G.edve.first;
	while(eve) {
		if(eve->h==0) {
			project_short(eve->co, &(eve->xs));
		}
		eve= eve->next;
	}
	Mat4SwapMat4(G.vd->persmat, mat);
}

void calc_meshverts_ext()
{

	areawinset(curarea->win);
	multmatrix(G.obedit->obmat);
	calc_meshverts();
	loadmatrix(G.vd->viewmat);
	
}

void calc_Nurbverts(nurb)
Nurb *nurb;
{
	Nurb *nu;
	BezTriple *bezt;
	BPoint *bp;
	float mat[4][4];
	int a, tot, b;
	short *sp;

	Mat4SwapMat4(G.vd->persmat, mat);
	mygetsingmatrix(G.vd->persmat);

	nu= nurb;
	while(nu) {
		if((nu->type & 7)==1) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				project_short(bezt->vec[0], bezt->s[0]);
				project_short(bezt->vec[1], bezt->s[1]);
				project_short(bezt->vec[2], bezt->s[2]);
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				project_short(bp->vec, bp->s);
				bp++;
			}
		}
		nu= nu->next;
	}

	Mat4SwapMat4(G.vd->persmat, mat);
}

void calc_nurbverts_ext()
{

	areawinset(curarea->win);
	multmatrix(G.obedit->obmat);
	calc_Nurbverts(editNurb.first);
	loadmatrix(G.vd->viewmat);
	
}

void tekenvertices(sel)
short sel;
{
	ulong *rect;
	EditVert *eve;
	short xs,ys;

	if(sel) rect= (ulong *)rectyellow;
	else rect= (ulong *)rectpurple;

	eve= (EditVert *)G.edve.first;
	while(eve) {
		if( (eve->f & 1)==sel ) {
			tekenrect2(eve->xs, eve->ys, rect);
		}
		eve= eve->next;
	}
}

void tekenvertices_ext(int mode)
{
	ScrArea *tempsa, *sa;
	View3D *vd;
	
	if(G.f & (G_FACESELECT+G_DRAWFACES)) {
		addqueue(curarea->win, REDRAW, 1);
		return;
	}
	
	if(G.zbuf) zbuffer(0);
	
	frontbuffer(1);
	backbuffer(0);

	/* alle views aflopen */
	tempsa= curarea;
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==SPACE_VIEW3D) {
			vd= sa->spacedata.first;
			if(G.obedit->lay & vd->lay) {
				areawinset(sa->win);
				multmatrix(G.obedit->obmat);

				calc_meshverts();
				if(mode==0 || mode==2) tekenvertices(0);
				if(mode==1 || mode==2) tekenvertices(1);
				sa->win_equal= 0;
				
				loadmatrix(G.vd->viewmat);
			}
		}
		sa= sa->next;
	}
	if(curarea!=tempsa) areawinset(tempsa->win);
	
	frontbuffer(0);
	backbuffer(1);
	if(G.zbuf) zbuffer(1);
}

/* ************** DRAW DISPLIST ****************** */

int draw_index_wire= 1;
int index3_nors_incr= 1;

void drawDispListwire(dlbase)
ListBase *dlbase;
{
	DispList *dl;
	int parts, nr, ofs, p1, p2, p3, p4, a, b, *index;
	float *data, *v1, *v2, *v3, *v4, side;

	if(dlbase==0) return;

	dl= dlbase->first;
	while(dl) {
		data= dl->verts;

		switch(dl->type) {
		case DL_SEGM:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				bgnline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				endline();
			}
			break;
		case DL_POLY:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				bgnclosedline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				endclosedline();
			}
			break;
		case DL_SURF:
			parts= dl->parts;
			while(parts--) {
				nr= dl->nr;
				if(dl->flag & 1) bgnclosedline();
				else bgnline();
				while(nr--) {
					v3f(data);
					data+=3;
				}
				if(dl->flag & 1) endclosedline();
				else endline();
			}
			ofs= 3*dl->nr;
			nr= dl->nr;
			while(nr--) {
				data= (  dl->verts )+3*nr;
				parts= dl->parts;
				if(dl->flag & 2) bgnclosedline();
				else bgnline();
				while(parts--) {
					v3f(data);
					data+=ofs;
				}
				if(dl->flag & 2) endclosedline();
				else endline();
			}
			break;
			
		case DL_INDEX3:
			if(draw_index_wire) {
				parts= dl->parts;
				data= dl->verts;
				index= dl->index;
				while(parts--) {
					bgnline();
						v3f(data+3*index[0]);
						v3f(data+3*index[1]);
						v3f(data+3*index[2]);
					endline();
					index+= 3;
				}
			}
			break;
			
		case DL_INDEX4:
			if(draw_index_wire) {
				parts= dl->parts;
				data= dl->verts;
				index= dl->index;
				while(parts--) {
					bgnline();
						v3f(data+3*index[0]);
						v3f(data+3*index[1]);
						v3f(data+3*index[2]);
						if(index[3]) v3f(data+3*index[3]);
					endline();
					index+= 4;
				}
			}
			break;
			
		}
		dl= dl->next;
	}
}

void drawDispListsolid(ListBase *lb, Object *ob)
{
	DispList *dl;
	int parts, nr, ofs, p1, p2, p3, p4, a, b, *index;
	float *data, *v1, *v2, *v3, *v4, side;
	float *ndata, *n1, *n2, *n3, *n4;

	short col[3];
	char *cp;

	if(lb==0) return;
	if(G.f & G_BACKBUFSEL); else shademodel(GOURAUD);

	dl= lb->first;
	while(dl) {
		data= dl->verts;
		ndata= dl->nors;

		switch(dl->type) {
		case DL_SURF:
			if(G.f & G_BACKBUFSEL) {
				for(a=0; a<dl->parts; a++) {
	
					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);
	
					v1= data+ 3*p1; 
					v2= data+ 3*p2;
					v3= data+ 3*p3; 
					v4= data+ 3*p4;
	
					for(; b<dl->nr; b++) {
						bgnpolygon();
							v3f(v1);v3f(v2);v3f(v4);v3f(v3);
						endpolygon();
	
						v2= v1; v1+= 3;
						v4= v3; v3+= 3;
					}
				}
			}
			else {
				lmbind(MATERIAL, dl->col+1);
				
				for(a=0; a<dl->parts; a++) {
	
					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);
	
					v1= data+ 3*p1; 
					v2= data+ 3*p2;
					v3= data+ 3*p3; 
					v4= data+ 3*p4;
					n1= ndata+ 3*p1; 
					n2= ndata+ 3*p2;
					n3= ndata+ 3*p3; 
					n4= ndata+ 3*p4;
	
					for(; b<dl->nr; b++) {
						bgnpolygon();
							n3f(n1); v3f(v1);
							n3f(n2); v3f(v2);
							n3f(n4); v3f(v4);
							n3f(n3); v3f(v3);
						endpolygon();
	
						v2= v1; v1+= 3;
						v4= v3; v3+= 3;
						n2= n1; n1+= 3;
						n4= n3; n3+= 3;
					}
				}
			}
			break;

		case DL_INDEX3:
		
			parts= dl->parts;
			data= dl->verts;
			ndata= dl->nors;
			index= dl->index;
			
			if(G.f & G_BACKBUFSEL) {
				while(parts--) {
					bgnpolygon();
						v3f(data+3*index[0]);
						v3f(data+3*index[1]);
						v3f(data+3*index[2]);
					endpolygon();
					index+= 3;
				}
			}
			else {
				lmbind(MATERIAL, dl->col+1);
				
				/* voor poly's is er maar 1 normaal nodig */
				if(index3_nors_incr==0) {
					while(parts--) {
						bgnpolygon();
							n3f(ndata);
							v3f(data+3*index[0]);
							v3f(data+3*index[1]);
							v3f(data+3*index[2]);
						endpolygon();
						index+= 3;
					}
				}
				else {
					while(parts--) {
						bgnpolygon();
							ofs= 3*index[0];
							n3f(ndata+ofs); v3f(data+ofs);
							ofs= 3*index[1];
							n3f(ndata+ofs); v3f(data+ofs);
							ofs= 3*index[2];
							n3f(ndata+ofs); v3f(data+ofs);
						endpolygon();
						index+= 3;
					}
				}
			}
			break;

		case DL_INDEX4:

			parts= dl->parts;
			data= dl->verts;
			ndata= dl->nors;
			index= dl->index;

			if(G.f & G_BACKBUFSEL) {
				while(parts--) {
					bgnpolygon();
						v3f(data+3*index[0]);
						v3f(data+3*index[1]);
						v3f(data+3*index[2]);
						if(index[3]) v3f(data+3*index[3]);
					endpolygon();
					index+= 4;
				}
			}
			else {
				
				lmbind(MATERIAL, dl->col+1);
			
				while(parts--) {
					bgnpolygon();
						ofs= 3*index[0];
						n3f(ndata+ofs); v3f(data+ofs);
						ofs= 3*index[1];
						n3f(ndata+ofs); v3f(data+ofs);
						ofs= 3*index[2];
						n3f(ndata+ofs); v3f(data+ofs);
						if(index[3]) {
							ofs= 3*index[3];
							n3f(ndata+ofs); v3f(data+ofs);
						}
					endpolygon();
					index+= 4;
				}
			}
			break;
			
		}
		dl= dl->next;
	}
	if(G.f & G_BACKBUFSEL);
	else {
		shademodel(FLAT);
		lmbind(MATERIAL, 0);
	}
}

void drawDispListshaded(ListBase *lb, Object *ob)
{
	DispList *dl, *dlob;
	int parts, nr, ofs, p1, p2, p3, p4, a, b, *index;
	float *data, *v1, *v2, *v3, *v4, side;
	ulong *cdata, *c1, *c2, *c3, *c4;
	short col[3];
	char *cp;

	if(lb==0) return;

	shademodel(GOURAUD);

	dl= lb->first;
	dlob= ob->disp.first;
	while(dl && dlob) {
		
		data= dl->verts;
		cdata= dlob->col1;
		if(cdata==0) break;
		
		switch(dl->type) {
		case DL_SURF:

			for(a=0; a<dl->parts; a++) {

				DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);

				v1= data+ 3*p1; 
				v2= data+ 3*p2;
				v3= data+ 3*p3; 
				v4= data+ 3*p4;
				c1= cdata+ p1; 
				c2= cdata+ p2;
				c3= cdata+ p3; 
				c4= cdata+ p4;

				for(; b<dl->nr; b++) {
					bgnpolygon();
						cpack(*c1); v3f(v1);
						cpack(*c2); v3f(v2);
						cpack(*c4); v3f(v4);
						cpack(*c3); v3f(v3);
					endpolygon();

					v2= v1; v1+= 3;
					v4= v3; v3+= 3;
					c2= c1; c1++;
					c4= c3; c3++;
				}
			}
			break;

		case DL_INDEX3:
			
			parts= dl->parts;
			index= dl->index;
			
			while(parts--) {
				bgnpolygon();
					cpack(*(cdata+index[0])); v3f(data+3*index[0]);
					cpack(*(cdata+index[1])); v3f(data+3*index[1]);
					cpack(*(cdata+index[2])); v3f(data+3*index[2]);
				endpolygon();
				index+= 3;
			}
			break;

		case DL_INDEX4:
			
			lmbind(MATERIAL, dl->col+1);
			
			parts= dl->parts;
			index= dl->index;
			while(parts--) {
				bgnpolygon();
					cpack(*(cdata+index[0])); v3f(data+3*index[0]);
					cpack(*(cdata+index[1])); v3f(data+3*index[1]);
					cpack(*(cdata+index[2])); v3f(data+3*index[2]);
					if(index[3]) {
						cpack(*(cdata+index[3])); v3f(data+3*index[3]);
					}
				endpolygon();
				index+= 4;
			}
			break;
			
		}
		dl= dl->next;
		dlob= dlob->next;
	}
	
	shademodel(FLAT);
}


void drawmeshsolid(Object *ob, float *nors)
{
	Mesh *me;
	DispList *dl;
	MVert *mvert;
	TFace *tface;
	MFace *mface;
	float *extverts=0, *v1, *v2, *v3, *v4;
	int a, start, end, matnr=0, vertexpaint, i;
	
	vertexpaint= (G.f & (G_VERTEXPAINT+G_FACESELECT)) && (ob==OBACT);

	me= get_mesh(ob);
	if(me==0) return;

	if( (G.f & G_BACKBUFSEL)==0 ) lmbind(MATERIAL, matnr+1);

	mface= me->mface;
	if( (G.f & G_FACESELECT) && ob==OBACT) tface= me->tface;
	else tface= 0;

	mvert= me->mvert;
	a= me->totface;

	if ELEM(ob->type, OB_SECTOR, OB_LIFE) {backface(TRUE);}

	start= 0; end= me->totface;
	set_buildvars(ob, &start, &end);
	mface+= start;
	if(tface) tface+= start;
	
	dl= test_displist(&ob->disp, DL_VERTS);
	if(dl) extverts= dl->verts;

	for(a=start; a<end; a++, mface++, nors+=3) {
		if(mface->v3) {
			if(tface && (tface->flag & TF_HIDE)) {
				if( (G.f & G_BACKBUFSEL)==0) {
					bgnclosedline();
					v3f( (mvert+mface->v1)->co);
					v3f( (mvert+mface->v2)->co);
					v3f( (mvert+mface->v3)->co);
					if(mface->v4) v3f( (mvert+mface->v1)->co);
					endclosedline();
					tface++;
				}
			}
			else {
				if(extverts) {
					v1= extverts+3*mface->v1;
					v2= extverts+3*mface->v2;
					v3= extverts+3*mface->v3;
					if(mface->v4) v4= extverts+3*mface->v4;
					else v4= 0;
				}
				else {
					v1= (mvert+mface->v1)->co;
					v2= (mvert+mface->v2)->co;
					v3= (mvert+mface->v3)->co;
					if(mface->v4) v4= (mvert+mface->v4)->co;
					else v4= 0;
				}
				
				
				if(tface) {
					if(tface->mode & TF_TWOSIDE) {backface(FALSE);}
					else {backface(TRUE);}
				}
				
				if(G.f & G_BACKBUFSEL) {
					if(vertexpaint) {
						i= a+1;
						cpack( ((i & 0xF00)<<12) + ((i & 0xF0)<<8) + ((i & 0xF)<<4) );
					}
					bgnpolygon();
					v3f( v1 );
					v3f( v2 );
					v3f( v3 );
					if(v4) v3f( v4 );
					endpolygon();
				}
				else {
					if(mface->mat_nr!=matnr) {
						matnr= mface->mat_nr;
						lmbind(MATERIAL, matnr+1);
					}
					
					bgnpolygon();
					n3f(nors);
					v3f( v1 );
					v3f( v2 );
					v3f( v3 );
					if(v4) v3f( v4 );
					endpolygon();
				}
			}
			if(tface) tface++;
		}
	}
	
	if ELEM(ob->type, OB_SECTOR, OB_LIFE) {backface(FALSE);}

	if(G.f & G_BACKBUFSEL) {
		backface(FALSE);
	}
	else lmbind(MATERIAL, 0);
	
}

void drawmeshshaded(Object *ob, ulong *col1, ulong *col2)
{
	Mesh *me;
	MVert *mvert;
	MFace *mface;
	TFace *tface;
	DispList *dl;
	float *extverts=0, *v1, *v2, *v3, *v4;
	int a, start, end, twoside;
	
	shademodel(GOURAUD);

	me= ob->data;
	mface= me->mface;
	
	/* tekent ie geen hide */
	if( (G.f & G_FACESELECT) && ob==OBACT) tface= me->tface;
	else tface= 0;
	
	mvert= me->mvert;
	a= me->totface;
	
	twoside= me->flag & ME_TWOSIDED;
	if(col2==0) twoside= 0;
	
	if(twoside) {backface(TRUE);}
	
	start= 0; end= me->totface;
	set_buildvars(ob, &start, &end);
	mface+= start;
	if(tface) tface+= start;
	col1+= 4*start;
	if(col2) col2+= 4*start;
	
	dl= test_displist(&ob->disp, DL_VERTS);
	if(dl) extverts= dl->verts;

	for(a=start; a<end; a++, mface++, col1+=4) {
		if(mface->v3) {
			if(tface && (tface->flag & TF_HIDE)) tface++;
			else {
				if(extverts) {
					v1= extverts+3*mface->v1;
					v2= extverts+3*mface->v2;
					v3= extverts+3*mface->v3;
					if(mface->v4) v4= extverts+3*mface->v4;
					else v4= 0;
				}
				else {
					v1= (mvert+mface->v1)->co;
					v2= (mvert+mface->v2)->co;
					v3= (mvert+mface->v3)->co;
					if(mface->v4) v4= (mvert+mface->v4)->co;
					else v4= 0;
				}

				if(tface) {
					if(tface->mode & TF_TWOSIDE) {backface(FALSE);}
					else {backface(TRUE);}
				}

				bgnpolygon();
				cpack( *(col1));
				v3f( v1 );
				cpack( *(col1+1));
				v3f( v2 );
				cpack( *(col1+2));
				v3f( v3 );
				if(v4) {
					cpack( *(col1+3));
					v3f( v4 );
				}
				endpolygon();
				
				if(twoside) {
					bgnpolygon();
					cpack( *(col2+2));
					v3f( v3 );
					cpack( *(col2+1));
					v3f( v2 );
					cpack( *(col2));
					v3f( v1 );
					if(mface->v4) {
						cpack( *(col2+3));
						v3f( v4 );
					}
					endpolygon();
						
				}
			}
		}
		if(col2) col2+= 4;
	}
	
	shademodel(FLAT);
	if(twoside) {backface(FALSE);}
	
}
void set_gl_materials(Object *ob)
{
	Material *ma;
	int a;
	
	if(ob->totcol==0) def_gl_material(0, 0);
	else {
		for(a=0; a<ob->totcol; a++) {
			ma= give_current_material(ob, a+1);
			def_gl_material(a+1, ma);
		}
	}
}

void drawDispList(Object *ob, int dt)
{
	ListBase *lb=0;
	DispList *dl;
	Mesh *me;
	Sector *se;
	int a, solid;

	solid= (dt > OB_WIRE);

	switch(ob->type) {
	case OB_MESH:
	case OB_SECTOR:
	case OB_LIFE:
		
		me= get_mesh(ob);
		if(me==0) return;
		
		if(me->bb==0) tex_space_mesh(me);
		if(me->totface>4) if(boundbox_clip(ob->obmat, me->bb)==0) return;
		
		if(dt==OB_SOLID ) {
			
			lb= &me->disp;
			if(lb->first==0) addnormalsDispList(ob, lb);
			
			dl= lb->first;
			if(dl==0) return;
			
			if(G.f & G_BACKBUFSEL); else {
				set_gl_materials(ob);
				two_sided( me->flag & ME_TWOSIDED );
			}
		
			drawmeshsolid(ob, dl->nors);
			
		}
		else if(dt==OB_SHADED) {
			if( G.f & G_VERTEXPAINT) {
				/* in deze volgorde: vertexpaint heeft soms al mcol gemaakt */
				if(me->mcol) 
					drawmeshshaded(ob, (ulong *)me->mcol, 0);
				else if(me->tface) {
					tface_to_mcol(me);
					drawmeshshaded(ob, (ulong *)me->mcol, 0);
					freeN(me->mcol); me->mcol= 0;
				}
				else 
					drawmeshwire(ob);
				
			}
			else {
				dl= ob->disp.first;
				
				if(dl==0 || dl->col1==0) {
					shadeDispList(ob);
					dl= ob->disp.first;
				}
				if(dl) drawmeshshaded(ob, dl->col1, dl->col2);
			}
		}
		
		if(ob==OBACT && (G.f & G_FACESELECT)) {
			draw_tfaces3D(ob, me);
		}
		
		break;
		
	case OB_FONT:
	case OB_CURVE:
	
		lb= &((Curve *)ob->data)->disp;
		if(lb->first==0) makeDispList(ob);
		
		if(solid && ob!=G.obedit) {
			dl= lb->first;
			if(dl==0) return;
			
			/* regel: dl->type INDEX3 altijd vooraan in lijst */
			if(dl->type!=DL_INDEX3) {
				curve_to_filledpoly(ob->data, lb);
				dl= lb->first;
			}
			if(dl->nors==0) addnormalsDispList(ob, lb);
			
			index3_nors_incr= 0;
			
			if(dt==OB_SHADED) {
				if(ob->disp.first==0) shadeDispList(ob);
				drawDispListshaded(lb, ob);
			}
			else {
				set_gl_materials(ob);
				two_sided(0);
				drawDispListsolid(lb, ob);
			}
			index3_nors_incr= 1;
		}
		else {
			draw_index_wire= 0;
			drawDispListwire(lb);
			draw_index_wire= 1;
		}
		break;
	case OB_SURF:
	
		lb= &((Curve *)ob->data)->disp;
		if(lb->first==0) makeDispList(ob);
		
		if(solid) {
			dl= lb->first;
			if(dl==0) return;
			
			if(dl->nors==0) addnormalsDispList(ob, lb);
			
			if(dt==OB_SHADED) {
				if(ob->disp.first==0) shadeDispList(ob);
				drawDispListshaded(lb, ob);
			}
			else {
				set_gl_materials(ob);
				two_sided(0);
			
				drawDispListsolid(lb, ob);
			}
		}
		else {
			drawDispListwire(lb);
		}
		break;
	case OB_MBALL:

		lb= &ob->disp;
		if(lb->first==0) makeDispList(ob);
		
		if(solid) {
			
			if(dt==OB_SHADED) {
				dl= lb->first;
				if(dl && dl->col1==0) shadeDispList(ob);
				drawDispListshaded(lb, ob);
			}
			else {
				set_gl_materials(ob);
				two_sided(0);
			
				drawDispListsolid(lb, ob);
			}
		}
		else drawDispListwire(lb);
		break;
	}
	
}

/* ******************************** */


void draw_particle_system(Object *ob, PartEff *paf)
{
	Particle *pa;
	float ptime, ctime, vec[3], vec1[3];
	int a;
	
	pa= paf->keys;
	if(pa==0) {
		build_particle_system(ob);
		pa= paf->keys;
		if(pa==0) return;
	}
	
	if(ob->ipoflag & OB_OFFS_PARTICLE) ptime= ob->sf;
	else ptime= 0.0;
	ctime= system_time(0, 0, (float)CFRA, ptime);

	if(paf->stype!=PAF_VECT) bgnpoint();

	for(a=0; a<paf->totpart; a++, pa+=paf->totkey) {
		
		if(ctime > pa->time) {
			if(ctime < pa->time+pa->lifetime) {
			
				if(paf->stype==PAF_VECT) {
					where_is_particle(paf, pa, ctime, vec);
					where_is_particle(paf, pa, ctime+1.0, vec1);
		
					bgnline();
						v3f(vec);
						v3f(vec1);
					endline();
					
				}
				else {
					where_is_particle(paf, pa, ctime, vec);
					
					/* O2 pakt geen nul lange lijnen */
					
					v3f(vec);
						
				}
			}
		}
	}
	if(paf->stype!=PAF_VECT) endpoint();
}

void drawmeshwire(Object *ob)
{
	extern float editbutsize;	/* buttons.c */
	Sector *se;
	Mesh *me;
	MVert *mvert;
	MFace *mface;
	DFace *dface;
	DispList *dl;
	Material *ma;
	EditEdge *eed;
	EditVlak *evl;
	float fvec[3], cent[3], *f1, *f2, *f3, *f4, *extverts=0;
	int a, start, end, test, colbcol=0, ok;

	if(ob==G.obedit || (G.obedit && ob->data==G.obedit->data)) {
		if(G.obedit==ob) {
			cpack(0);
			if(ob->type==OB_SECTOR) {
				Sector *se= ob->data;
				if(se->flag & SE_SHOW_TEXMESH); else cpack(0x401000);
			}
			else if(ob->type==OB_LIFE) {
				Life *lf= ob->data;
				if(lf->flag & LF_SHOW_TEXMESH); else cpack(0x401000);
			}
		}
		
		eed= G.eded.first;

		while(eed) {
			if(eed->h==0) {
				bgnline();
				v3f(eed->v1->co);
				v3f(eed->v2->co);
				endline();
			}
			eed= eed->next;
		}

		if(ob!=G.obedit) return;
		
		calc_meshverts();

		if(G.zbuf) zbuffer(0);
		tekenvertices(0);
		tekenvertices(1);
		if(G.zbuf) zbuffer(1);

		if(G.f & G_DRAWNORMALS) {	/* normals */
			cpack(0xDDDD22);
			evl= G.edvl.first;
			while(evl) {
				if(evl->v1->h==0 && evl->v2->h==0 && evl->v3->h==0) {
					CalcCent3f(fvec, evl->v1->co, evl->v2->co, evl->v3->co);
					bgnline();
					v3f(fvec);
					fvec[0]+= editbutsize*evl->n[0];
					fvec[1]+= editbutsize*evl->n[1];
					fvec[2]+= editbutsize*evl->n[2];
					v3f(fvec);
					endline();
				}
				evl= evl->next;
			}
		}
		if(G.f & (G_FACESELECT+G_DRAWFACES)) {	/* vlakken */
			
			evl= G.edvl.first;
			while(evl) {
				if(evl->v1->h==0 && evl->v2->h==0 && evl->v3->h==0) {
					
					if(vlakselectedAND(evl, 1)) cpack(0x559999);
					else cpack(0x664466);
				
					if(evl->v4 && evl->v4->h==0) {
					
						CalcCent4f(cent, evl->v1->co, evl->v2->co, evl->v3->co, evl->v4->co);
						bgnclosedline();
							VecMidf(fvec, cent, evl->v1->co); v3f(fvec);
							VecMidf(fvec, cent, evl->v2->co); v3f(fvec);
							VecMidf(fvec, cent, evl->v3->co); v3f(fvec);
							VecMidf(fvec, cent, evl->v4->co); v3f(fvec);
						endclosedline();
					}
					else {

						CalcCent3f(cent, evl->v1->co, evl->v2->co, evl->v3->co);
						bgnclosedline();
							VecMidf(fvec, cent, evl->v1->co); v3f(fvec);
							VecMidf(fvec, cent, evl->v2->co); v3f(fvec);
							VecMidf(fvec, cent, evl->v3->co); v3f(fvec);
						endclosedline();
					}
				}
				evl= evl->next;
			}
		}
	}
	else {
		
		me= get_mesh(ob);
		if(me==0) return;
		
		if(me->bb==0) tex_space_mesh(me);
		if(me->totface>4) if(boundbox_clip(ob->obmat, me->bb)==0) return;
		
		mvert= me->mvert;
		mface= me->mface;
		dface= me->dface;
		
		ok= 0;
		if(me->totface==0) ok= 1;
		else {
			ma= give_current_material(ob, 1);
			if(ma && (ma->mode & MA_HALO)) ok= 1;
		}
		
		dl= test_displist(&ob->disp, DL_VERTS);
		if(dl) extverts= dl->verts;
		
		if(ok) {
			
			start= 0; end= me->totvert;
			set_buildvars(ob, &start, &end);
			
			bgnpoint();
			
			if(extverts) {
				extverts+= 3*start;
				for(a= start; a<end; a++, extverts+=3) {
					v3f(extverts);
				}
			}
			else {
				mvert+= start;
				for(a= start; a<end; a++, mvert++) {
					v3f(mvert->co);
				}
			}
			
			endpoint();
		}
		else {
			
			start= 0; end= me->totface;
			set_buildvars(ob, &start, &end);
			mface+= start;
			
			for(a=start; a<end; a++, mface++) {
				test= mface->edcode;
				
if(dface ) {
	/* test= mface->edcode>>4; */
	if(dface->flag & DF_HILITE) cpack(0xFF00FF);
	else cpack(0);
	dface->flag &= ~DF_HILITE;
	dface++;
}
				
				if(test) {
					if(extverts) {
						f1= extverts+3*mface->v1;
						f2= extverts+3*mface->v2;
					}
					else {
						f1= (mvert+mface->v1)->co;
						f2= (mvert+mface->v2)->co;
					}
					
					if(mface->v4) {
						if(extverts) {
							f3= extverts+3*mface->v3;
							f4= extverts+3*mface->v4;
						}
						else {
							f3= (mvert+mface->v3)->co;
							f4= (mvert+mface->v4)->co;
						}
						if(test== ME_V1V2+ME_V2V3+ME_V3V4+ME_V4V1) {
							bgnclosedline();
								v3f(f1); v3f(f2); v3f(f3); v3f(f4);
							endclosedline();
						}
						else if(test== ME_V1V2+ME_V2V3+ME_V3V4) {
							bgnline();
								v3f(f1); v3f(f2); v3f(f3); v3f(f4);
							endline();
						}
						else if(test== ME_V2V3+ME_V3V4+ME_V4V1) {
							bgnline();
								v3f(f2); v3f(f3); v3f(f4); v3f(f1);
							endline();
						}
						else if(test== ME_V3V4+ME_V4V1+ME_V1V2) {
							bgnline();
								v3f(f3); v3f(f4); v3f(f1); v3f(f2);
							endline();
						}
						else if(test== ME_V4V1+ME_V1V2+ME_V2V3) {
							bgnline();
								v3f(f4); v3f(f1); v3f(f2); v3f(f3);
							endline();
						}
						else {
							if(test & ME_V1V2) {
								bgnline();
									v3f(f1); v3f(f2);
								endline();
							}
							if(test & ME_V2V3) {
								bgnline();
									v3f(f2); v3f(f3);
								endline();
							}
							if(test & ME_V3V4) {
								bgnline();
									v3f(f3); v3f(f4);
								endline();
							}
							if(test & ME_V4V1) {
								bgnline();
									v3f(f4); v3f(f1);
								endline();
							}
						}
					}
					else if(mface->v3) {
						if(extverts) f3= extverts+3*mface->v3;
						else f3= (mvert+mface->v3)->co;
	
						if(test== ME_V1V2+ME_V2V3+ME_V3V1) {
							bgnclosedline();
								v3f(f1); v3f(f2); v3f(f3);
							endclosedline();
						}
						else if(test== ME_V1V2+ME_V2V3) {
							bgnline();
								v3f(f1); v3f(f2); v3f(f3);
							endline();
						}
						else if(test== ME_V2V3+ME_V3V1) {
							bgnline();
								v3f(f2); v3f(f3); v3f(f1);
							endline();
						}
						else if(test== ME_V1V2+ME_V3V1) {
							bgnline();
								v3f(f3); v3f(f1); v3f(f2);
							endline();
						}
						else {
							if(test & ME_V1V2) {
								bgnline();
									v3f(f1); v3f(f2);
								endline();
							}
							if(test & ME_V2V3) {
								bgnline();
									v3f(f2); v3f(f3);
								endline();
							}
							if(test & ME_V3V1) {
								bgnline();
									v3f(f3); v3f(f1);
								endline();
							}
						}
					}
					else if(test == ME_V1V2) {
						bgnline();
							v3f(f1); v3f(f2);
						endline();
					}
				}
			}
		}
		
	}
}

ulong nurbcol[8]= {
	0, 0x9090, 0x409030, 0x603080, 0, 0x40fff0, 0x40c033, 0xA090F0 };

void tekenhandlesN(nu, sel)
Nurb *nu;
short sel;
{
	BezTriple *bezt;
	float *fp;
	ulong *col;
	long a;

	if(nu->hide) return;
	if( (nu->type & 7)==1) {
		if(sel) col= nurbcol+4;
		else col= nurbcol;

		bezt= nu->bezt;
		a= nu->pntsu;
		while(a--) {
			if(bezt->hide==0) {
				if( (bezt->f2 & 1)==sel) {
					fp= bezt->vec[0];
					cpack(col[bezt->h1]);
					bgnline(); 
					v3f(fp);
					v3f(fp+3); 
					endline();
					cpack(col[bezt->h2]);
					bgnline(); 
					v3f(fp+3); 
					v3f(fp+6); 
					endline();
				}
				else if( (bezt->f1 & 1)==sel) {
					fp= bezt->vec[0];
					cpack(col[bezt->h1]);
					bgnline(); 
					v3f(fp); 
					v3f(fp+3); 
					endline();
				}
				else if( (bezt->f3 & 1)==sel) {
					fp= bezt->vec[1];
					cpack(col[bezt->h2]);
					bgnline(); 
					v3f(fp); 
					v3f(fp+3); 
					endline();
				}
			}
			bezt++;
		}
	}
}

void tekenvertsN(nu, sel)
Nurb *nu;
short sel;
{
	BezTriple *bezt;
	BPoint *bp;
	ulong *rect;
	long a;

	if(nu->hide) return;

	if(sel) rect= (ulong *)rectyellow;
	else rect= (ulong *)rectpurple;

	if((nu->type & 7)==1) {

		bezt= nu->bezt;
		a= nu->pntsu;
		while(a--) {
			if(bezt->hide==0) {
				if((bezt->f1 & 1)==sel) tekenrect3(bezt->s[0][0], bezt->s[0][1], rect);
				if((bezt->f2 & 1)==sel) tekenrect3(bezt->s[1][0], bezt->s[1][1], rect);
				if((bezt->f3 & 1)==sel) tekenrect3(bezt->s[2][0], bezt->s[2][1], rect);
			}
			bezt++;
		}
	}
	else {
		bp= nu->bp;
		a= nu->pntsu*nu->pntsv;
		while(a--) {
			if(bp->hide==0) {
				if((bp->f1 & 1)==sel) tekenrect3(bp->s[0], bp->s[1], rect);
			}
			bp++;
		}
	}
}

void draw_editnurb(Object *ob, Nurb *nurb, int sel)
{
	Nurb *nu;
	BPoint *bp, *bp1;
	int a, b, ofs;
	
	nu= nurb;
	while(nu) {
		if(nu->hide==0) {
			switch(nu->type & 7) {
			case CU_POLY:
				cpack(nurbcol[3]);
				bp= nu->bp;
				for(b=0; b<nu->pntsv; b++) {
					if(nu->flagu & 1) bgnclosedline();
					else bgnline();

					for(a=0; a<nu->pntsu; a++, bp++) {
						v3f(bp->vec);
					}

					if(nu->flagu & 1) endclosedline();
					else endline();
				}
				break;
			case CU_NURBS:

				bp= nu->bp;
				for(b=0; b<nu->pntsv; b++) {
					bp1= bp;
					bp++;
					for(a=nu->pntsu-1; a>0; a--, bp++) {
						if(bp->hide==0 && bp1->hide==0) {
							if(sel) {
								if( (bp->f1 & 1) && ( bp1->f1 & 1) ) {
									cpack(nurbcol[5]);

									bgnline();
									v3f(bp->vec); 
									v3f(bp1->vec);
									endline();
								}
							}
							else {
								if( (bp->f1 & 1) && ( bp1->f1 & 1) );
								else {
									cpack(nurbcol[1]);
		
									bgnline();
									v3f(bp->vec); 
									v3f(bp1->vec);
									endline();
								}
							}
						}
						bp1= bp;
					}
				}
				if(nu->pntsv > 1) {
					ofs= nu->pntsu;
					for(b=0; b<nu->pntsu; b++) {
						bp1= nu->bp+b;
						bp= bp1+ofs;
						for(a=nu->pntsv-1; a>0; a--, bp+=ofs) {
								if(sel) {
									if( (bp->f1 & 1) && ( bp1->f1 & 1) ) {
										cpack(nurbcol[7]);
			
										bgnline();
										v3f(bp->vec); 
										v3f(bp1->vec);
										endline();
									}
								}
								else {
									if( (bp->f1 & 1) && ( bp1->f1 & 1) );
									else {
										cpack(nurbcol[3]);
			
										bgnline();
										v3f(bp->vec); 
										v3f(bp1->vec);
										endline();
									}
								}
							}
							bp1= bp;
						}
					}
				}
				break;
			}
		}
		nu= nu->next;
	}


void drawnurb(Object *ob, Nurb *nurb, int dt)
{
	extern float editbutsize;	/* buttons.c */
	Curve *cu;
	Nurb *nu;
	BezTriple *bezt;
	BPoint *bp, *bp1;
	BevPoint *bevp;
	BevList *bl;
	float vec[3], seg[12], *fp, *data;
	long a, b, ofs, nr, skip;

	/* eerst handles niet select */
	nu= nurb;
	while(nu) {
		if((nu->type & 7)==CU_BEZIER) {
			tekenhandlesN(nu, 0);
		}
		nu= nu->next;
	}
	
	/* dan DispList */
	
	cpack(0);
	cu= ob->data;
	drawDispList(ob, dt);

	draw_editnurb(ob, nurb, 0);
	draw_editnurb(ob, nurb, 1);

	if(cu->flag & CU_3D) {
	
		if(cu->bev.first==0) makeBevelList(ob);
		
		cpack(0x0);
		bl= cu->bev.first;
		nu= nurb;
		while(nu && bl) {
			bevp= (BevPoint *)(bl+1);		
			nr= bl->nr;
			
			skip= nu->resolu/16;
			
			while(nr-- > 0) {
				
				bgnline();
				vec[0]= bevp->x-editbutsize*bevp->mat[0][0];
				vec[1]= bevp->y-editbutsize*bevp->mat[0][1];
				vec[2]= bevp->z-editbutsize*bevp->mat[0][2];
				v3f(vec);
				vec[0]= bevp->x+editbutsize*bevp->mat[0][0];
				vec[1]= bevp->y+editbutsize*bevp->mat[0][1];
				vec[2]= bevp->z+editbutsize*bevp->mat[0][2];
				v3f(vec);

				endline();
				
				bevp++;
				
				a= skip;
				while(a--) {
					bevp++;
					nr--;
				}
			}

			bl= bl->next;
			nu= nu->next;
		}
	}

	calc_Nurbverts(nurb);

	if(G.zbuf) zbuffer(0);
	
	nu= nurb;
	while(nu) {
		if((nu->type & 7)==1) tekenhandlesN(nu, 1);
		tekenvertsN(nu, 0);
		tekenvertsN(nu, 1);
		nu= nu->next;
	}
	
	if(G.zbuf) zbuffer(1);
}

void tekentextcurs()
{
	cpack(0);
	bgnpolygon();
	v2f(G.textcurs[0]);
	v2f(G.textcurs[1]);
	v2f(G.textcurs[2]);
	v2f(G.textcurs[3]);
	endpolygon();
}

void drawcircball(float *cent, float rad, float tmat[][4])
{
	float si, co, phi, dphi, vec[3], vx[3], vy[3];
	int a, tot=32;
		
	VECCOPY(vx, tmat[0]);
	VECCOPY(vy, tmat[1]);
	VecMulf(vx, rad);
	VecMulf(vy, rad);
	
	dphi= 2.0*M_PI/tot;
	phi= 0.0;
	
	bgnclosedline();
	for(a=0; a<tot; a++, phi+= dphi) {
		si= sinf(phi);
		co= cosf(phi);
		vec[0]= cent[0]+si*vx[0]+co*vy[0];
		vec[1]= cent[1]+si*vx[1]+co*vy[1];
		vec[2]= cent[2]+si*vx[2]+co*vy[2];
		v3f(vec);
	}
	endclosedline();
}

void drawmball(Object *ob, int dt)
{
	MetaBall *mb;
	MetaElem *ml;
	float imat[4][4], tmat[4][4];
	int code= 1;
	
	mb= ob->data;

	if(ob==G.obedit) {
		cpack(0x0);
		if((G.f & G_PICKSEL)==0 ) drawDispList(ob, dt);
		ml= editelems.first;
	}
	else {
		drawDispList(ob, dt);
		ml= mb->elems.first;
	}

	getmatrix(tmat);
	Mat4Invert(imat, tmat);
	Normalise(imat[0]);
	Normalise(imat[1]);
	
	while(ml) {
		
		if(ob==G.obedit) {
			if(ml->flag & SELECT) cpack(0xA0A0F0);
			else cpack(0x3030A0);
			
			if(G.f & G_PICKSEL) {
				ml->selcol= code;
				loadname(code++);
			}
		}
		drawcircball(&(ml->x), ml->rad, imat);
		
		ml= ml->next;
	}

}

void drawboundbox(Object *ob)
{
	Mesh *me;
	BoundBox *bb=0;
	float *vec;
	
	if(ob->type==OB_MESH) {
		bb= ( (Mesh *)ob->data )->bb;
		if(bb==0) {
			tex_space_mesh(ob->data);
			bb= ( (Mesh *)ob->data )->bb;
		}
	}
	else if ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT) {
		bb= ( (Curve *)ob->data )->bb;
		if(bb==0) {
			makeDispList(ob);
			bb= ( (Curve *)ob->data )->bb;
		}
	}
	else if(ob->type==OB_MBALL) {
		bb= ob->bb;
		if(bb==0) {
			makeDispList(ob);
			bb= ob->bb;
		}
	}
	else if ELEM(ob->type, OB_SECTOR, OB_LIFE) {
		me= get_mesh(ob);
		if(me) bb= me->bb;
	}
	else {
		drawcube();
		return;
	}
	
	if(bb==0) return;
	
	vec= bb->vec[0];
	bgnline();
		v3f(vec); v3f(vec+3);v3f(vec+6); v3f(vec+9);
		v3f(vec); v3f(vec+12);v3f(vec+15); v3f(vec+18);
		v3f(vec+21); v3f(vec+12);
	endline();
	bgnline();
		v3f(vec+3); v3f(vec+15);
	endline();
	bgnline();
		v3f(vec+6); v3f(vec+18);
	endline();
	bgnline();
		v3f(vec+9); v3f(vec+21);
	endline();
}

void drawtexspace(Object *ob)
{
	Mesh *me;
	MetaBall *mb;
	Curve *cu;
	BoundBox bb;
	float *vec, *loc, *size;
	
	if(ob->type==OB_MESH) {
		me= ob->data;
		size= me->size;
		loc= me->loc;
	}
	else if ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT) {
		cu= ob->data;
		size= cu->size;
		loc= cu->loc;
	}
	else if(ob->type==OB_MBALL) {
		mb= ob->data;
		size= mb->size;
		loc= mb->loc;
	}
	else return;
	
	bb.vec[0][0]=bb.vec[1][0]=bb.vec[2][0]=bb.vec[3][0]= loc[0]-size[0];
	bb.vec[4][0]=bb.vec[5][0]=bb.vec[6][0]=bb.vec[7][0]= loc[0]+size[0];
	
	bb.vec[0][1]=bb.vec[1][1]=bb.vec[4][1]=bb.vec[5][1]= loc[1]-size[1];
	bb.vec[2][1]=bb.vec[3][1]=bb.vec[6][1]=bb.vec[7][1]= loc[1]+size[1];

	bb.vec[0][2]=bb.vec[3][2]=bb.vec[4][2]=bb.vec[7][2]= loc[2]-size[2];
	bb.vec[1][2]=bb.vec[2][2]=bb.vec[5][2]=bb.vec[6][2]= loc[2]+size[2];
	
	setlinestyle(5);
	
	vec= bb.vec[0];
	bgnline();
		v3f(vec); v3f(vec+3);v3f(vec+6); v3f(vec+9);
		v3f(vec); v3f(vec+12);v3f(vec+15); v3f(vec+18);
		v3f(vec+21); v3f(vec+12);
	endline();
	bgnline();
		v3f(vec+3); v3f(vec+15);
	endline();
	bgnline();
		v3f(vec+6); v3f(vec+18);
	endline();
	bgnline();
		v3f(vec+9); v3f(vec+21);
	endline();
	
	setlinestyle(0);
}


void draw_object(Base *base)
{
	PartEff *paf;
	Object *ob;
	Curve *cu;
	Life *lf;
	Mesh *me;
	ListBase elems;
	CfraElem *ce;
	float cfraont, axsize=1.0;
	ulong *rect, col=0;
	static int warning_recursive= 0;
	int sel, drawtype, colindex= 0, ipoflag;
	short dt, dtx, zbufoff= 0;
	char str[4];
	
	ob= base->object;
	
	/* keys tekenen? */
	if(base==BASACT || (base->flag & (SELECT+BA_WASSEL))) {
		if(warning_recursive==0 && ob!=G.obedit) {
			if(ob->ipo && ob->ipo->showkey && (ob->ipoflag & OB_DRAWKEY)) {
				float temp[7][3];

				warning_recursive= 1;
				
				elems.first= elems.last= 0;
				make_cfra_list(ob->ipo, &elems);
		
				cfraont= CFRA;
				drawtype= G.vd->drawtype;
				if(drawtype>OB_WIRE) G.vd->drawtype= OB_WIRE;
				sel= base->flag;
				memcpy(temp, &ob->loc, 7*3*4);
				
				ipoflag= ob->ipoflag;
				ob->ipoflag &= ~OB_OFFS_OB;
				
				set_no_parent_ipo(1);
				
				ce= elems.first;
				while(ce) {
					if( (ce->sel) || (ob->ipoflag & OB_DRAWKEYSEL)==0) {

						CFRA= ce->cfra/G.scene->r.framelen;

						if(ce->sel) base->flag= SELECT;
						else base->flag= 0;

						where_is_object_time(ob, ce->cfra/G.scene->r.framelen);
						draw_object(base);
					}
					
					ce= ce->next;
				}
				
				set_no_parent_ipo(0);
				
				base->flag= sel;
				ob->ipoflag= ipoflag;
				
				/* restore icu->curval */
				CFRA= cfraont;
				
				memcpy(&ob->loc, temp, 7*3*4);
				where_is_object(ob);
				G.vd->drawtype= drawtype;
				
				freelistN(&elems);

				warning_recursive= 0;
			}
		}
	}
	
	/* patch? kinderen met timeoffs verprutsen ouders. Hoe los je dat op! */
	/* if( ((int)ob->ctime) != F_CFRA) where_is_object(ob); */

	multmatrix(ob->obmat);

	if(G.machine==ENTRY || G.machine==VIDEO) linewidth(2);

	/* welke wire kleur */
	if((G.f & (G_BACKBUFSEL+G_PICKSEL)) == 0) {
		project_short(ob->obmat[3], &base->sx);
		
		if(G.moving==1 && (base->flag & (SELECT+BA_PARSEL))) colindex= 9;
		else {
			if(BASACT==base) {
				if(base->flag & (SELECT+BA_WASSEL)) colindex= 2;
			}
			else {
				if(base->flag & (SELECT+BA_WASSEL)) colindex= 1;
			}
			if(ob->id.lib) colindex+= 3;
			else if(warning_recursive==1) colindex+= 6;
		}	
		
		col= colortab[colindex];
		cpack(col);
		
	}
	
	/* maximum drawtype */
	dt= MIN2(G.vd->drawtype, ob->dt);
	if(G.zbuf==0 && dt>OB_WIRE) dt= OB_WIRE;
	dtx= 0;
	
	/* faceselect uitzondering: ook solid tekenen als dt==wire */
	if(ob==OBACT && (G.f & (G_FACESELECT+G_VERTEXPAINT))) {
		if(ob->type==OB_MESH) {
			
			if(G.f & G_BACKBUFSEL) dt= OB_SOLID;
			else dt= OB_SHADED;
	
			zclear();
			zbuffer(1);
			zbufoff= 1;
			
		}
		else {
			if(dt<OB_SOLID) {
				dt= OB_SOLID;
				zclear();
				zbuffer(1);
				zbufoff= 1;
			}
		}
	}
	else if(dt>=OB_WIRE ) {

		if(dt>OB_SOLID) if(G.f & G_BACKBUFSEL) dt= OB_SOLID;

		dtx= ob->dtx;
		if(G.obedit==ob) {
			if(dtx & OB_TEXSPACE) dtx= OB_TEXSPACE;
			else dtx= 0;
		}
		
		if(G.f & G_DRAW_EXT) {
			if(ob->type==OB_EMPTY || ob->type==OB_CAMERA || ob->type==OB_LAMP) dt= OB_WIRE;
		}
		
		if((base->flag & OB_FROMDUPLI)==0) if(ob->transflag & OB_DUPLI) dt= OB_WIRE;
	}
	
	if( (G.f & G_DRAW_EXT) && dt>OB_WIRE) {
		
		switch( ob->type) {
		case OB_MBALL:
			drawmball(ob, dt);
			break;
		}
	}
	else {
	
		switch( ob->type) {
			
		case OB_MESH:
			if(ob==G.obedit || (G.obedit && ob->data==G.obedit->data)) drawmeshwire(ob);
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else if(dt==OB_WIRE) drawmeshwire(ob);
			else if( ((Mesh *)ob->data)->tface ) {
				if((G.f & (G_BACKBUFSEL))) drawmeshsolid(ob, 0);
				else draw_tface_mesh(ob, ob->data, dt);
			}
			else  drawDispList(ob, dt);
			
			break;
		case OB_FONT:
			cu= ob->data;
			if(ob==G.obedit) {
				tekentextcurs();
				cpack(0xFFFF90);
				drawDispList(ob, OB_WIRE);
			}
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else if(boundbox_clip(ob->obmat, cu->bb)) drawDispList(ob, dt);
				
			break;
		case OB_CURVE:
		case OB_SURF:
			cu= ob->data;
			/* een pad niet solid tekenen: wel dus!!! */
			/* if(cu->flag & CU_PATH) if(dt>OB_WIRE) dt= OB_WIRE; */
			
			if(ob==G.obedit) drawnurb(ob, editNurb.first, dt);
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else if(boundbox_clip(ob->obmat, cu->bb)) drawDispList(ob, dt);
			
			break;
		case OB_MBALL:
			if(ob==G.obedit) drawmball(ob, dt);
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else drawmball(ob, dt);
			break;
		case OB_EMPTY:
			drawaxes(1.0);
			break;
		case OB_LAMP:
			drawlamp(ob);	/* doet loadmatrix */
			break;
		case OB_CAMERA:
			drawcamera(ob);
			break;
		case OB_LATTICE:
			drawlattice(ob);
			break;
		case OB_SECTOR:
			drawsector(ob, dt, col);
			break;
		case OB_LIFE:
			drawlife(ob, dt, col);
			break;
		case OB_IKA:
			draw_ika(ob, base->flag & SELECT);
			break;
		default:
			drawaxes(1.0);
		}
	}
	
	/* draw extra: na gewone draw ivm makeDispList */
	if(dtx) {
		if(dtx & OB_AXIS) {
			if(ob->type==OB_LIFE) {lf= ob->data; axsize= lf->axsize;}
			drawaxes(axsize);
		}
		if(dtx & OB_BOUNDBOX) drawboundbox(ob);
		if(dtx & OB_TEXSPACE) drawtexspace(ob);
		if(dtx & OB_DRAWNAME) {
			cmov(0.0, 0.0, 0.0);
			str[0]= ' '; str[1]= 0;
			fmsetfont(G.font);
			fmprstr(str);
			fmprstr(ob->id.name+2);
		}
		if(dtx & OB_DRAWIMAGE) drawDispListwire(&ob->disp);
	}

	loadmatrix(G.vd->viewmat);

	if(zbufoff) zbuffer(0);

	if(warning_recursive) return;
	if(base->flag & OB_FROMDUPLI) return;
	
	if((G.f & (G_BACKBUFSEL+G_PICKSEL))==0) {
		/* hulplijnen e.d. */

		if( paf=give_parteff(ob) ) {
			if(col) cpack(0xFFFFFF);	/* zichtbaarheid */
			draw_particle_system(ob, paf);
			cpack(col);
		}
		
		if(ob->parent && (ob->parent->lay & G.vd->lay)) {
			setlinestyle(1);
			LINE3F(ob->obmat[3], ob->orig);
			setlinestyle(0);
		}
		
		if(ob->type == OB_LAMP) {
			if(ob->id.lib) {
				if(base->flag & SELECT) rect= rectllib_sel;
				else rect= rectllib_desel;
			}
			else if(ob->id.us>1) {
				if(base->flag & SELECT) rect= rectlus_sel;
				else rect= rectlus_desel;
			}
			else {
				if(base->flag & SELECT) rect= rectl_sel;
				else rect= rectl_desel;
			}
			tekenrect9(base->sx, base->sy, rect);
		}
		else {
			if(ob->id.lib || ob->id.us>1) {
				if(base->flag & SELECT) rect= rectu_sel;
				else rect= rectu_desel;
			}
			else {
				if(base->flag & SELECT) rect= rect_sel;
				else rect= rect_desel;
			}
			tekenrect4(base->sx, base->sy, rect);
		}
		
		if(ob->network.first) {
			NetLink *nl= ob->network.first;
			setlinestyle(2);
			while(nl) {
				LINE3F(ob->obmat[3], nl->ob->obmat[3]);
				nl= nl->next;
			}
			setlinestyle(0);
		}
	}
	else if((G.f & (G_VERTEXPAINT|G_FACESELECT))==0) {
		linewidth(2);
		bgnline();
			v3f(ob->obmat[3]);
			v3f(ob->obmat[3]);
		endline();
		
		if(G.machine!=ENTRY || G.machine!=VIDEO) linewidth(1);
	}
}
void draw_object_ext(Base *base)
{
	ScrArea *tempsa, *sa;
	View3D *vd;
	
	if(G.vd==0) return;
	
	if(G.vd->drawtype > OB_WIRE) {
		G.zbuf= 1;
		zbuffer(1);
	}
	
	G.f |= G_DRAW_EXT;
	frontbuffer(1);
	backbuffer(0);

	/* alle views aflopen */
	tempsa= curarea;
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==SPACE_VIEW3D) {
			/* er wordt beperkt in beide buffers getekend: selectbuffer! */
			if(sa->win_equal) backbuffer(1);
			else backbuffer(0);
			
			vd= sa->spacedata.first;
			if(base->lay & vd->lay) {
				areawinset(sa->win);
				draw_object(base);
			}
		}
		sa= sa->next;
	}
	if(curarea!=tempsa) areawinset(tempsa->win);
	
	G.f &= ~G_DRAW_EXT;
	frontbuffer(0);
	backbuffer(1);
	
	if(G.zbuf) {
		G.zbuf= 0;
		zbuffer(0);
	}
}

