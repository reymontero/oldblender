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
/*
 * 

cc -g -float -c frt.c ; ld frt.o -o frt.so -lc -lm;mv frt.so /pics/textures

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

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

	#define NR_VARS		5

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		NUM|INT,	"limit",	128.0,	0.0, 255.0, 
		NUM|FLO,	"size",		4.0,	-1.0, 1.0, 
		NUMSLI|FLO, "R ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "G ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "B ",		0.5,	0.0, 1.0,
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		int limit;
		float size, r, g, b;
	} Cast;

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
	float x, y, u, v, s, c;
	int i;

	_result[0]= (cfra*cast->size);

	_result[1]= _result[0]+cast->r;
	_result[2]= _result[0]+cast->g;
	_result[3]= _result[0]+cast->b;
	_result[4]= 1.0;

	return 0;
}





void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	*vars= NR_VARS;

}



