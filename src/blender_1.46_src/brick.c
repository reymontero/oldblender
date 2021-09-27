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

cc -g -float -c brick.c ; ld brick.o noise.o -o brick.so -lc -lm; cp brick.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

float x, y, z, x1;
/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "Brick";


/* 2. aantal sub types */

	#define NR_STYPES	2

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"rt1", "rt2"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		5

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		NUM|FLO,	"Width",	0.4,	0.01, 1.0,  
		NUM|FLO,	"Height",	0.2,	0.01, 1.0, 
		NUM|FLO,	"Mortar",	0.05,	0.01, 1.0, 
		NUM|FLO,	"NoiseSize",0.1,	0.0,  1.0, 
		NUM|FLO,	"Noise",	2.0,	0.0,  20.0, 
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float width, height, mortar;
		float noisesize, noise;
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

#define step(a, x)	((x)<(a) ? 0.0 : 1.0)

int plugin_tex_doit(int stype, Cast *cast, float *texvec, float *dxt, float *dyt)
{
	float ss, tt, w, h, u, sbrick, tbrick, ubrick;
	float mwf, mhf, uu;

	mwf= cast->mortar*0.5/(cast->width +cast->mortar);
	mhf= cast->mortar*0.5/(cast->height+cast->mortar);
	
	ss= texvec[0]/(cast->width +cast->mortar);
	tt= texvec[1]/(cast->width +cast->mortar);
	uu= texvec[2]/(cast->height+cast->mortar);

	ubrick = floor(uu+0.1);	/* which brick? */
	if( ( (int)ubrick) & 1) {
		ss+= 0.5;
		tt+= 0.5;
	}

	sbrick = floor(ss);	/* which brick? */
	tbrick = floor(tt);	/* which brick? */
	
	ss -= sbrick;
	tt -= tbrick;
	uu -= ubrick;

	uu+= cast->noise*hnoise(cast->noisesize, sbrick, tbrick, ubrick);
	
	w = step(mwf, ss) - step(1.0-mwf, ss);
	h = step(mwf, tt) - step(1.0-mwf, tt);
	u = step(mhf, uu) - step(1.0-mhf, uu);

	_result[0]= w*h*u;

	return 0;
}

void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	*vars= NR_VARS;

}


