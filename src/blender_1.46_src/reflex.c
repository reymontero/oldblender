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

cc -g -float -c reflex.c ; ld reflex.o -o reflex.so -lc -lm ; cp reflex.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "reflex";


/* 2. aantal sub types */

	#define NR_STYPES	1

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"reflex"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		24

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
		LABEL,      "Base",		0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.0,  0.0,  1.0, 
		NUMSLI|FLO, "G ",		1.0,  0.0,  1.0,
		NUMSLI|FLO, "B ",		1.0,  0.0,  1.0,
		NUMSLI|FLO, "o ",	   -0.5, -1.0,  1.0,
		NUMSLI|FLO, "s ",		1.0,  0.001, 2.0,
		
		LABEL,      "Border",	0.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.0,  0.0,  1.0, 
		NUMSLI|FLO, "G ", 		0.0,  0.0,  1.0,
		NUMSLI|FLO, "B ",		1.0,  0.0,  1.0,
		NUMSLI|FLO, "o ",		0.0, -1.0,  1.0,
		NUMSLI|FLO, "i ",		1.0,  0.0,  1.0,

		TOG|INT,    "Spec1",	1.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		1.0,  0.0,  1.0, 
		NUMSLI|FLO, "G ", 		1.0,  0.0,  1.0,
		NUMSLI|FLO, "B ",		0.0,  0.0,  1.0,
		NUMSLI|FLO,	"x ",		1.0, -1.0,  1.0, 
		NUMSLI|FLO, "y ", 		1.0, -1.0,  1.0,
		
		TOG|INT,    "Spec2",	1.0,  0.0,  0.0, 
		NUMSLI|FLO,	"R ",		0.0,  0.0,  1.0, 
		NUMSLI|FLO, "G ", 		0.0,  0.0,  1.0,
		NUMSLI|FLO, "B ",		0.0,  0.0,  1.0,
		NUMSLI|FLO,	"x ",		0.0, -1.0,  1.0, 
		NUMSLI|FLO, "y ", 		0.0, -1.0,  1.0,
		
		
		
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float  dum1, r1, g1, b1, i1, j1;
		
		float  dum2, r2, g2, b2, i2, j2;
		
		int    dum3; 
		float        r3, g3, b3, i3, j3;
		
		int    dum4;
		float        r4, g4, b4, i4, j4;
		
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
	float sx, sy;
	float center, nenter, overfl;
	float ro = 0.0, go = 0.0, bo = 0.0, ao = 1.0;
	int   s1, s2;
	
	r1 = cast->r1; g1 = cast->g1; b1 = cast->b1; i1 = cast->i1; j1 = cast->j1;
	r2 = cast->r2; g2 = cast->g2; b2 = cast->b2; i2 = cast->i2; j2 = cast->j2;
	r3 = cast->r3; g3 = cast->g3; b3 = cast->b3; i3 = cast->i3; j3 = cast->j3;
	r4 = cast->r4; g4 = cast->g4; b4 = cast->b4; i4 = cast->i4; j4 = cast->j4;
	s1 = cast->dum3;
	s2 = cast->dum4;
	
	
	x = texvec[0] / j1;
	y = texvec[1] / j1;
	z = texvec[2] / j1;
	
	
	switch(stype){
		case 0:
			if ((x*x)+(y*y) < 1.0) {	
				
				center = ( x * x ) + ( y * y )  ;
				nenter = 1.0 - center;
				
				i1 += 1; i1 *= 2;  
				i2 += 1; i2 *= 2;  
				
				r1 = ( i1 *  (r1 * nenter) ) + ( i2 * j2 * (r2 * center) ); 
				g1 = ( i1 *  (g1 * nenter) ) + ( i2 * j2 * (g2 * center) ); 
				b1 = ( i1 *  (b1 * nenter) ) + ( i2 * j2 * (b2 * center) ); 
			
				r2 = r1 ;
				g2 = g1 ;
				b2 = b1 ;
				
				i3 *= 3.15; sx = abs(i3) * fsin (x * i3);
				j3 *= 3.15; sy = abs(j3) * fsin (y * j3);
				
				r1 += sx*r3 ;g1 += sx*g3 ;b1 += sx*b3;
				r1 += sy*r3 ;g1 += sy*g3 ;b1 += sy*b3;
				
				r1 *=  (1.0 - ((x*x)+(y*y)) );
				g1 *=  (1.0 - ((x*x)+(y*y)) );
				b1 *=  (1.0 - ((x*x)+(y*y)) );
				
				i4 *=  3.15; sx = abs(i4) * fsin (x * i4);
				j4 *=  3.15; sy = abs(j4) * fsin (y * j4);
				
				r2 +=  sx*r4 ;g2 += sx*g4 ;b2 += sx*b4;
				r2 +=  sy*r4 ;g2 += sy*g4 ;b2 += sy*b4;
				
				r2 *=  (1.0 - ((x*x)+(y*y)) );
				g2 *=  (1.0 - ((x*x)+(y*y)) );
				b2 *=  (1.0 - ((x*x)+(y*y)) );
				
				if (( s1 == 1 ) && ( s2 == 1 )){
					ro = .5 * (r1+r2);
					go = .5 * (g1+g2);
					bo = .5 * (b1+b2);
				}else{
					if ( s1 == 1 ){
						ro = r1;go = g1;bo = b1;
					}else{
						ro = r2;go = g2;bo = b2;
					}
				}
				
				
				
			}
		break;
	}
	
	if (bo > 1.0) { bo = 1.0; ro *= 1.1; ro *= 1.1; }
	if (ro > 1.0) { ro = 1.0; go *= 1.1; bo *= 1.1; }
	if (go > 1.0) { go = 1.0; ro *= 1.1; bo *= 1.1; }
	
	if (ro > 1.0) { ro = 1.0; }
	if (go > 1.0) { go = 1.0; }
	if (bo > 1.0) { bo = 1.0; }
	
	
	_result[0] = 1.0;
	_result[1] = ro;
	_result[2] = go;
	_result[3] = bo;
	_result[4] = 1.0;

	return (1);
}

