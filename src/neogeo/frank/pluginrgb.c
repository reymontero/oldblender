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

/* plugintex.c  		MIXED MODEL

 * 
 * dec 95
 * 
 */

#include "/usr/people/blend/plugin.h"
#include <math.h>
/*
 * 

cc -g -float -c pluginrgb.c ; ld pluginrgb.o -o rgb.so -lc -lm ; mv rgb.so /pics/textures/proc/

                ^^^^^^^^^        ^^^^^^^^^      ^^^^^^^^^                 ^^^^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct- en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "RGB";


/* 2. aantal sub types */

	#define NR_STYPES	1

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"RGB"};


/* 4. informatie over externe variabelen */

	VarStruct _varstr[]= {
	 /* butcode,	naam,       default,min, max */
		NUMSLI|FLO, "R ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "G ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "B ",		0.5,	0.0, 1.0,
		NUMSLI|FLO, "A ",		1.0,	0.0, 1.0,
	};

/* 5. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float r, g, b, a;
	} Cast;


/* deze funktie laten staan! */

void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	*vars= sizeof(_varstr) / sizeof(VarStruct);

}




/* ******************************************************************* */
/* ******************************************************************* */


/* het resultaat van de texture is Tin,Tr,Tg,Tb,Ta, nor[0],nor[1],nor[2]  */

float _result[8];

/* deze globals worden door blender gezet */

float cfra;

/* allereerste keer */

void plugin_tex_init()
{
	
}


/* return 0: eenkanaals tex, 
   return 1: RGB texture
   return 2: normalen texture */

int plugin_tex_doit(int stype, Cast *cast, float *texvec, float *dxt, float *dyt)
{
	if (stype == 0) {
		_result[1] = cast->r;
		_result[2] = cast->g;
		_result[3] = cast->b;
		_result[4] = cast->a;
	}

	return 1;
}

