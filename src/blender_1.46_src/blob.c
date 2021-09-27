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

cc -g -float -c blob.c ; ld blob.o -o blob.so -lc -lm ; cp blob.so /pics/textures/proc/

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "Blob";


/* 2. aantal sub types */

	#define NR_STYPES	4

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"blob", "tvtv", "magie", "drup"};


/* 4. aantal externe variabelen (voor buttons) */

	#define NR_VARS		4

/* 5. informatie over externe variabelen */

	VarStruct _varstr[NR_VARS]= {
	 /* butcode,	naam,       default,min, max */
	 
		NUM|FLO,	"val 1",	1.0,   -100.0, 100.0, 
		NUM|FLO,    "val 2",    1.0,   -100.0, 100.0,
		NUM|FLO,    "val 3",    1.0,   -100.0, 100.0,
		TOG|INT,    "rgb",      1.0,      0.0,   1.0, 
	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float  val1, val2, val3;
		int    tog;
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
	float x, y, z, val1, val2, val3, rv, gv, bv;
	
	float pr , pg , pb ;
	
	x = texvec[0] + .01 ; y = texvec[1]+ .01; z =  texvec[2];
	val1 = cast->val1;
	val2 = cast->val2;
	val3 = cast->val3;
	
	switch(stype){
		case 0:
			x+=.1;y+=.1;z+=.1;
			rv =fsin (x * val1) * fsin (y * val2) * fcos (z * val3);
			gv =fcos (x * val1) * fsin (y * val2) * fsin (z * val3);
			bv =fsin (x * val1) * fcos (y * val2) * fsin (z * val3);
		
/* 			while (rv > .8) {rv -= .7;gv += .1;bv += .2;} 
			while (rv < .2) {rv += .3;}
	
			while (gv > .8) {gv -= .7;bv -= .3;}
			while (gv < .2) {gv += .3;}
		
			while (bv > .8) {bv -= .7;rv += .3;}
			while (bv < .2) {bv += .3;}
			
			while (rv > .8) {rv -= .7;}
			while (rv < .2) {rv += .3;}
*/			
			pr = (.4 * bv) + (.4 * gv) + ( rv);
			pg = (.4 * gv) + (.4 * rv) + ( gv);
			pb = (.4 * rv) + (.4 * bv) + ( bv);
			
			rv =fsin (pr * val1) * fsin (pg * val2) * fcos (bv * val3);
			gv =fcos (pr * val1) * fsin (pg * val2) * fsin (bv * val3);
			bv =fsin (pr * val1) * fcos (pg * val2) * fsin (bv * val3);
		
		break;
		case 3:
			x+=.1;y+=.1;z+=.1;
			rv =fsin (x * val1) * fsin (y * val2) * fcos (z * val3);
			gv =fcos (x * val1) * fsin (y * val2) * fsin (z * val3);
			bv =fsin (x * val1) * fcos (y * val2) * fsin (z * val3);
		
			while (rv > .8) {rv -= .7;}
			while (rv < .2) {rv += .3;}
	
			while (gv > .8) {gv -= .7;}
			while (gv < .2) {gv += .3;}
		
			while (bv > .8) {bv -= .7;}
			while (bv < .2) {bv += .3;}
			
		break;
		case 2:
			x+=.1;y+=.1;z+=.1;
			
			pr =fsin (x * val1) * fsin (y * val2) * fcos (z * val3);
			pg =fcos (x * val1) * fsin (y * val2) * fsin (z * val3);
			pb =fsin (x * val1) * fcos (y * val2) * fsin (z * val3);
			
			x-=.1;y-=.1;z-=.1;
			rv =( pr + fsin (x * val1) * fsin (y * val2) * fcos (z * val3) ) / 2;
			gv =( pg + fcos (x * val1) * fsin (y * val2) * fsin (z * val3) ) / 2;
			bv =( pb + fsin (x * val1) * fcos (y * val2) * fsin (z * val3) ) / 2;
			
			pr = rv * 2.0; 
			while (pr > .8) {pr -= .7; pg += .3; }
			while (pr < .2) {pr += .3;}
			
			pg = gv * 2.0; 
			while (pg > .8) {pg -= .7; pb += .3;}
			while (pg < .2) {pg += .3;}
			
			
			pb = bv * 2.0; 
			while (pb > .8) {pb -= .7;}
			while (pb < .2) {pb += .3;}

			_result[4] = 1.0;
		break;
		case 1:
			z+=.1;
			
			rv = fsin (x * val1) * fsin (y * val2) * fcos (z * val3) ;
			gv = fcos (x * val1) * fsin (y * val2) * fsin (z * val3) ;
			bv = fsin (x * val1) * fcos (y * val2) * fsin (z * val3) ;
		break;
	}
	if ((rv > gv)&&(rv > bv)) { _result[0] = rv; }
	if ((gv > rv)&&(gv > bv)) { _result[0] = gv; }
	if ((bv > rv)&&(bv > gv)) { _result[0] = bv; }
	
	/* _result[0] = (rv*.8  + gv  + bv *.2); */
	
	_result[1] = rv;
	_result[2] = gv;
	_result[3] = bv;
	_result[4] = 1.0;
	
	if (cast->tog == 1) return 1;
	return(0);
}

