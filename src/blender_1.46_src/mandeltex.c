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

cc -g -float -c mandeltex.c ; ld mandeltex.o noise.o -o mandeltex.so -lc -lm;cp mandeltex.so /pics/textures/proc

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

/*	#define NR_VARS		11*/

/* 5. informatie over externe variabelen */

	VarStruct _varstr[/*NR_VARS*/]= {
	 /* butcode,	naam,       default,min, max */
		NUM|FLO,	"limit",	128.0,	0.0, 255.0, 
		NUM|FLO,	"it",		7.0,  0.0, 1000.0,  
		NUM|FLO,	"x",		0.0,	-2.0, 2.0, 
		NUM|FLO,	"y",		0.0,	-2.0, 2.0, 
		NUM|FLO,	"u",		0.0,	-2.0, 2.0, 
		NUM|FLO,	"v",		0.0,	-2.0, 2.0, 
		NUMSLI|FLO, "R ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "G ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "B ",		0.5,	0.0, 1.0,
		NUMSLI|FLO, "sx ",		3.8,	-10.0, 10.0, 	
		NUMSLI|FLO, "sy ",		3.8,	-10.0, 10.0, 	
		NUMSLI|FLO, "sz ",		3.8,	-10.0, 10.0,
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float limit, it, x, y, u, v, R, G, B, sx, sy, sz;
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
	float fac, x, y, u, v, z, s;
	int i;

/*	fac= hnoise(cast->limit, texvec[0], texvec[1], texvec[2]);
	texvec[0]+= fac;
	texvec[1]+= fac;
	texvec[2]+= fac;
*/

	x = (texvec[0] + cast->x) * cast->sx;
	y = (texvec[1] + cast->y) * cast->sy;
	u = (texvec[2] + cast->u) * cast->sz;
	v = cast->v * cast->sz;
	
/*	printf("%f, %f, %f\n", texvec[0], texvec[1], texvec[2]);
*/
	for(i = 0; i < (int)cast->it; i++)
	{
		z = x;
		x = x * x - y * y + u;
		y = 2.0 * z * y + v;
		s = x * x + y * y;
		if(s > 255)
			break;
	}
	_result[0] = 1.0 - (i / cast->it);
	
/*	printf("%f\n", _result[0]);
*/		
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
	/**vars= NR_VARS;*/
	*vars= sizeof(_varstr) / sizeof(VarStruct);
}



