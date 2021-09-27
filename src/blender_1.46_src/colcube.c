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
 *
 * made by J.Kassenaar 2 Jan 1996
 */

#include "/usr/people/blend/plugin.h"
#include "/usr/people/include/Button.h"
#include <math.h>
/*
 * 

cc -g -float -c colcube.c ; ld colcube.o -o colcube.so -lc -lm; cp colcube.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "colcube";


/* 2. aantal sub types */

	#define NR_STYPES	1

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"ColCube" };


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		4

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		
		NUM|FLO,	"blend",	10.0,	 0.0, 20.0, 
		NUM|FLO,	"x-move",	 0.0,	-1.0, 1.0,
		NUM|FLO,	"y-move",	 0.0,	-1.0, 1.0, 
		NUM|FLO,	"z-move",	 0.0,	-1.0, 1.0, 
		
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float size;
		float xmove, ymove, zmove;
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

float ctime;

/* allereerste keer */

void plugin_tex_init()
{
	
}


/* return 0: eenkanaals tex, 
   return 1: RGB texture */

int plugin_tex_doit(int stype, Cast *cast, float *texvec, float *dxt, float *dyt)
{
			_result[1]=  (21- cast->size) * ( texvec[0] - cast->xmove ) ;
			_result[2]=  (21- cast->size) * ( texvec[1] - cast->ymove );
			_result[3]=  (21- cast->size) * ( texvec[2] - cast->zmove ) ;
			_result[4]=  1.0;
			
	return 1;
}

