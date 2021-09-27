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

cc -g -float -c paul.c ; ld paul.o noise.o -o paul.so -lc -lm ; cp paul.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^                 ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "paul";


/* 2. aantal sub types */

	#define NR_STYPES	2

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"vlieg", "rond"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		3

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		NUMSLI|FLO,	"specie",	0.1,	0.0, .5,
		NUMSLI|FLO,	"# x",		1.0,	1.0, 50.0,
		NUMSLI|FLO,	"# y",		1.0,	1.0, 50.0,
		 
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float specie, x, y;
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
	int i, ix, iy, iz;
	float x, y, z, ox, oy, oz;
			
	x= texvec[0];
	y= texvec[1];
	z= texvec[2];
	
/*
*  ix is geheel getal ox is wat er overblijft
*  met ix kun je bv onevengetallen de x en de y flippen
*/
	ox= (x- (ix=ffloor(x)) );
	oy= (y- (iy=ffloor(y)) );
	oz= (z- (iz=ffloor(z)) );

	if((oz <= (.5/cast->x)+cast->specie && oz >= (.5/cast->x)) || (oz <= (.0/cast->x)+cast->specie && oz >= (.0/cast->x)))
	{
		_result[0]=0.0;
	}
	else
	{
		if((oz <= (1.0/cast->x) && oz >= (.5/cast->x)+cast->specie) && (ox < (.5/cast->x)+cast->specie && ox > (.5/cast->x)))
		{
			_result[0]=0.0;
		}
		else
		{
			if((oz <= (.5/cast->x) && oy >= (.0/cast->x)+cast->specie) && (ox < (.0/cast->x)+cast->specie && ox > (.0/cast->x)))
			{
				_result[0]=0.0;
			}
			else
			{
				if((oz <= (1.0/cast->x) && oz >= (.5/cast->x)+cast->specie) && (oy < (.0/cast->x)+cast->specie && oy > (.0/cast->x)))
				{
					_result[0]=0.0;
				}
				else
				{
					if((oz <= (.5/cast->x) && oz >= (.0/cast->x)+cast->specie) && (oy < (.5/cast->x)+cast->specie && oy > (.5/cast->x)))
					{
						_result[0]=0.0;
					}
					else
					{
						_result[0]=1.0;
					}
				}
			}			
		}
	}
	
	return 0;
	
}


/*	t = fatan2(y/cast->a, fabs(x)/cast->b) /(2*M_PI) + .5;*/

