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
#include <stdlib.h>

/*
 * 

cc -g -float -c joeri.c ; ld joeri.o -o joeri.so -lc -lm ; cp joeri.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "Joeri";


/* 2. aantal sub types */

	#define NR_STYPES	4

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"test", "z blend", "point", "time"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		5

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
	 
		NUM|FLO,	"val",		0.5,	0.0, 1.0, 
		NUM|FLO,    "random",   0.0,    0.0, 1.0, 
		NUM|FLO,    "min",		0.0,    0.0, 1.0, 
		NUM|FLO,    "max",		1.0,    0.0, 1.0,
		NUM|FLO,    "time",		1.0,    0.0, 1000.0, 
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float  val, rnd, min, max, tim;
	} Cast;


/* deze funktie laten staan! */

void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	*vars= NR_VARS;

}




/* ******************************************************************* */
/* ******************************************************************* */


/* het resultaat van de texture is Tin,Tr,Tg,Tb,Ta,  */

float _result[8];

/* deze globals worden door blender gezet */

	float cfra;

/* allereerste keer */

void plugin_tex_init()
{
	
}


/* return 0: eenkanaals tex, 
   return 1: RGB texture */

int plugin_tex_doit(int stype, Cast *cast, float *texvec, float *dxt, float *dyt)
{
	float x, y, z, val, rvl, intv, min, max, tim;
	int ret;
	x = texvec[0]; y = texvec[1]; z = texvec[2];
	val = cast->val;
	min = cast->min;
	max = cast->max;
	rvl = (( rand() % 100) / 100.0) * cast->rnd;
	tim = cast->tim;
	
	switch (stype){
	case 0:
		intv = fsin ( x * 3.1415);
		ret = 0; 
		break;
	case 1:
		intv =  (x + y + z + (val - 0.5) + rvl ) / 3.0;
		ret = 0;
		break;
	case 2:
		intv = (x * y * z * val*3 + rvl)  ;
		ret = 0;
		break;
	case 3:
		intv = cfra / tim;
		ret = 0; 
		break;
	}
	if (intv > max) intv = max;
	if (intv < min) intv = min;
	
	_result[0] = intv;
	return (ret);
}

