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

/* plugin.c  		MIXED MODEL

 * 
 * dec 95
 * 
 */

#include "/usr/people/blend/plugin.h"
#include "/usr/people/include/Button.h"
#include <math.h>

extern float hnoise(float noisesize, float x, float y, float z);


/*
 * 

cc -g -float -c frt.c ; ld frt.o noise.o -o frt.so -lc -lm;cp frt.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

float x, y, z, x1;
/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "Rt";


/* 2. aantal sub types */

	#define NR_STYPES	2

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"rt1", "rt2"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		13

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		NUM|FLO,	"limit",	0.0,  -10.0, 10.0,  
		NUM|FLO,	"size1",		1.4,	-10.0, 10.0, 
		NUM|FLO,	"size2",	-0.3,	-10.0, 10.0, 
		NUMSLI|FLO, "R ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "G ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "B ",		0.5,	0.0, 1.0,
		NUMSLI|FLO, "sx ",		0.0,	-100.0, 100.0, 	
		NUMSLI|FLO, "sy ",		0.0,	-100.0, 100.0, 	
		NUMSLI|FLO, "sz ",		0.0,	-100.0, 100.0,
		NUMSLI|FLO, "x ",		0.0,	-2.0, 2.0, 	
		NUMSLI|FLO, "y ",		0.0,	-2.0, 2.0, 	
		NUMSLI|FLO, "z ",		0.0,	-2.0, 2.0,
		NUM|FLO,	"it",	7.0,  0.0, 1000,  
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float limit;
		float size, size2, r, g, b, sx, sy, sz,  x, y, z, it;
	} Cast;

/* ******************************************************************* */
/* ******************************************************************* */


/* het resultaat van de texture is Tin,Tr,Tg,Tb,Ta,  */

float _result[8];

/* deze globals worden door blender gezet */

float ctime;

/* allereerste keer */

void plugin_tex_init()
{
	
}


/* return 0: eenkanaals tex, 
   return 1: RGB texture */

void pffft(Cast *cast)
{
}

int plugin_tex_doit(int stype, Cast *cast, float *texvec, float *dxt, float *dyt)
{
	float fac, x1, y1, z1,  r;
	int i;

	fac= hnoise(cast->limit, texvec[0], texvec[1], texvec[2]);
	texvec[0]+= fac;
	texvec[1]+= fac;
	texvec[2]+= fac;

	x = (texvec[0] + cast->x) * cast->sx;
	y = (texvec[1] + cast->y) * cast->sy;
	z = (texvec[2] + cast->z) * cast->sz;

/*	for(i = 0; i < (int)cast->it; i++)
	{
		x = fabs(x) - (y * cast->size2) + cast->size;
		y = x;
	}
*/

	for(i = 0; i < (int)cast->it; i++)
	{
/*		x =x -sin(y) + cast->size2 + cast->size;*/
		r = sqrt(x*x+y+y);
		x = z=(cast->size*(cos(r)-cos(3*r)/3+cos(5*r)/5-cos(7*r)/7))/2;
		
		y = x;
	}
		
	_result[0] = x;
/*	if(fabs(x - y) <= cast->limit)
		_result[0] = 1.0;
	else
		_result[0] = 0.0;
*/
/*		_result[1]= (int)x % 2;
		_result[2]= (int)y % 2;
		_result[3]= (int)z % 2;
*/
	_result[4]= 1.0;

	return 0;
}

void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	*vars= NR_VARS;

}



