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

cc -g -float -c hot.c ; ld hot.o -o hot.so -lc -lm ; cp hot.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "hot";


/* 2. aantal sub types */

	#define NR_STYPES	1

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"hot"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		24

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		LABEL,      "Base",		0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.7,  0.0,  1.0, 
		NUMSLI|FLO, "G ",		0.7,  0.0,  1.0,
		NUMSLI|FLO, "B ",		0.0,  0.0,  1.0,
		NUMSLI|FLO, "o ",		1.0, -1.0,  1.0,
		NUMSLI|FLO, "s ",		1.0,  0.001, 2.0,
		
		LABEL,      "Border",	0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.7,  0.0,  1.0, 
		NUMSLI|FLO, "G ", 		0.0,  0.0,  1.0,
		NUMSLI|FLO, "B ",		0.0,  0.0,  1.0,
		NUMSLI|FLO, "o ",		1.0, -1.0,  1.0,
		NUMSLI|FLO, "i ",		0.5,  0.0,  1.0,

		LABEL,      "Invader",	0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.7,  0.0,  1.0, 
		NUMSLI|FLO, "G ", 		0.2,  0.0,  1.0,
		NUMSLI|FLO, "B ",		0.0,  0.0,  1.0,
		LABEL,      "  ",	    0.0,  0.0,  0.0, 
		LABEL,      "  ",	    0.0,  0.0,  0.0, 
		
		LABEL,      "Light",	0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"x ",		0.0, -1.0,  1.0, 
		NUMSLI|FLO, "y ", 		0.0, -1.0,  1.0,
		NUMSLI|FLO, "z ",		0.0, -1.0,  1.0,
		NUMSLI|FLO, "s ",		0.0,  0.0,  1.0,
		NUMSLI|FLO, "e ",		0.5,  0.0,  1.0,
		
		
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float  dum1, r1, g1, b1, i1, j1;
		float  dum2, r2, g2, b2, i2, j2;
		float  dum4, r4, g4, b4, i4, j4;
		float  dum3, r3, g3, b3, i3, j3;
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
	float x, y, z; 
	float r1, g1, b1, i1, j1, nj1;
	float r2, g2, b2, i2, j2, nj2;
	float r3, g3, b3, i3, j3, nj3;
	float r4, g4, b4, i4, j4, nj4;
	float center, nenter, overfl;
	float ro = 0.0, go = 0.0, bo = 0.0, ao = 1.0;
	
	r1 = cast->r1; g1 = cast->g1; b1 = cast->b1; i1 = cast->i1; j1 = cast->j1;
	r2 = cast->r2; g2 = cast->g2; b2 = cast->b2; i2 = cast->i2; j2 = cast->j2;
	r3 = cast->r3; g3 = cast->g3; b3 = cast->b3; i3 = cast->i3; j3 = cast->j3;
	r4 = cast->r4; g4 = cast->g4; b4 = cast->b4; i4 = cast->i4; j4 = cast->j4;
	
	x = texvec[0] / j1;
	y = texvec[1] / j1;
	z = texvec[2] / j1;
	
	
	switch(stype){
		case 0:
			center = ((( x * x ) + ( y * y ) + ( z * z ) ));
			nenter = (1.0 - (( x * x ) + ( y * y )));
			
			i1 += 1; i1 *= 2;  
			i2 += 1; i2 *= 2;  
			
			ro = ( i1 *  (r1 * nenter) ) + ( i2 * j2 * (r2 * center) ); 
			go = ( i1 *  (g1 * nenter) ) + ( i2 * j2 * (g2 * center) ); 
			bo = ( i1 *  (b1 * nenter) ) + ( i2 * j2 * (b2 * center) ); 
		
			x-=r3;
			y-=g3;
			z-=b3;
			
			i3 *= .8;
			j3 *= .8;
		
			r1 = j3 * ro * (1.0 - i3 * ((x*x)+(y*y)+(z*z)) );
			g1 = j3 * go * (1.0 - i3 * ((x*x)+(y*y)+(z*z)) );
			b1 = j3 * bo * (1.0 - i3 * ((x*x)+(y*y)+(z*z)) );
			
			ro = .5 * (r1 + r4);
			go = .5 * (g1 + g4);
			bo = .5 * (b1 + b4);
			
			ao =  1.5 - ( (i3*2) * ((x*x)+(y*y)+(z*z)));
			if (ao > 1.0) ao = 1.0;
			if (ao < 0.0) ao = 0.0;
		break;
	}
	_result[0] = 1.0;
	_result[1] = ro;
	_result[2] = go;
	_result[3] = bo;
	_result[4] = ao;
	return (1);
}

