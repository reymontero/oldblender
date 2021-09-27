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

#include <fcntl.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <local/gl_util.h>
#include "/usr/people/blend/plugin.h"
#include "/usr/people/include/Button.h"
#include "/usr/people/include/util.h"
#include <math.h>
#include <fmclient.h>

#define TRUE 1
#define FALSE 0

#define N 0x1000
#define B 0x100
#define BM 0xff
#define NP 12   /* 2^N */
#define NM 0xfff
#define lerp(t, a, b) ( a + t * (b - a) )
#define s_curve(t) ( t * t * (3. - 2. * t) )
#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;

static start = 1;
static p[B + B + 2];
static float g3[B + B + 2][3];
static float g2[B + B + 2][2];
static float g1[B + B + 2];


extern float hnoise(float noisesize, float x, float y, float z);

float Noise3(float vec[]);


/*
 * 

cc -g -float -c musgrave.c ; ld musgrave.o /usr/people/trace/mips1/util.o /usr/people/frank/source/gl_util.o /usr/people/trace/mips1/Button.o /usr/people/frank/source/storage.o /usr/people/blend/noise.o -o musgrave.so -lc -limbuf -lm -lfm -lgl;cp musgrave.so /pics/textures/proc

                ^^^^^^        ^^^^^^      ^^^^^^
 * 
 * 
 */

float x, y, z, x1;
/* ******************************************************************* */
/* 				ZELF INVULLEN: struct en funktienamen niet wijzigen							   */
/* ******************************************************************* */

/* 1. naam: */

	char _name[24]= "Musgra";


/* 2. aantal sub types */

	#define NR_STYPES	5

/* 3. namen van subtypes (maximaal 16 bytes per naam) */

	char _stnames[NR_STYPES][16]= {"Multi", "fBm", "Terra", "Hybrid", "Ridged"};


/* 4. aantal externe variabelen (voor buttons) */

/*	#define NR_VARS		11*/

/* 5. informatie over externe variabelen */

	VarStruct _varstr[/*NR_VARS*/]= {
	 /* butcode,	naam,       default,min, max */
		NUM|FLO,	"H",	    0.0,	-1.0, 1.0, 
		NUM|FLO,	"lacun",	3.6,    0.0, 10.0,  
		NUM|FLO,	"octav",	0.8,	0.0, 5.0, 
		NUM|FLO,	"offst",	0.6,	-1.0, 2.0, 
		NUM|FLO,	"scale",	2.9,	0.0,  10.0, 
		NUM|FLO,	"gain",	    0.0,	0.0,  10.0, 

/*		NUM|FLO,	"u",		0.0,	-2.0, 2.0, 
		NUM|FLO,	"v",		0.0,	-2.0, 2.0, 
		NUMSLI|FLO, "R ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "G ",		0.5,	0.0, 1.0, 	
		NUMSLI|FLO, "B ",		0.5,	0.0, 1.0,
		NUMSLI|FLO, "sx ",		3.8,	-10.0, 10.0, 	
		NUMSLI|FLO, "sy ",		3.8,	-10.0, 10.0, 	
		NUMSLI|FLO, "sz ",		3.8,	-10.0, 10.0,
*/	};

/* 6. hulpstruct om variabelen te casten */

	typedef struct Cast {
		float H, lacun, octav, offst, scale, gain;
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
      float            value, frequency, remainder, increment, Noise3(), point[3], octaves, Hh;
      float            result, signal, weight;
      int               i;
      static int        first = TRUE;
      static int        last = 0;
      static float     *exponent_array;

/*	fac= hnoise(cast->limit, texvec[0], texvec[1], texvec[2]);
	texvec[0]+= fac;
	texvec[1]+= fac;
	texvec[2]+= fac;
*/

	point[0] = texvec[0] * cast->scale;
	point[1] = texvec[1] * cast->scale;
	point[2] = texvec[2] * cast->scale;
	octaves = cast->octav;
	Hh = cast->H;
	
	if(last != stype){
		first = TRUE;
		last = stype;
	}
	
	last = stype;	
	if(stype == 0)
	{

      /* precompute and store spectral weights */
      if (first) {
            /* seize required memory for exponent_array */
            exponent_array = 
                        (float *)mallocN( (octaves+1.0) * sizeof(float), "test");
            frequency = 1.0;
            for (i=0; i<=cast->octav; i++) {
                  /* compute weight for each frequency */
                  exponent_array[i] = pow( frequency, -Hh );
                  frequency *= cast->lacun;
            }
            first = FALSE;
      }

      value = 1.0;            /* initialize vars to proper values */
      frequency = 1.0;

      /* inner loop of multifractal construction */
      for (i=0; i<cast->octav; i++) {
            value *= cast->offst * frequency * Noise3( point );
            point[0] *= cast->lacun;
            point[1] *= cast->lacun;
            point[2] *= cast->lacun;
      } /* for */

      remainder = cast->octav - (int)cast->octav;
      if ( remainder )      /* add in ``octaves''  remainder */
            /* ``i''  and spatial freq. are preset in loop above */
            value += remainder * Noise3( point ) * exponent_array[i];

		_result[0] = value;
/*      return value;
*/
	_result[4]= 1.0;

	return 0;
	} else {
		if (stype == 1){
			  if ( first ) {
					/* seize required memory for exponent_array */
					exponent_array = 
								(float *)mallocN( (octaves+1) * sizeof(float) , "test");
					frequency = 1.0;
					for (i=0; i<=octaves; i++) {
						  /* compute weight for each frequency */
						  exponent_array[i] = pow( frequency, -Hh );
		                  frequency *= cast->lacun;
					}
					first = FALSE;
			  }
		
			  value = 0.0;            /* initialize vars to proper values */
			  frequency = 1.0;
		
			  /* inner loop of spectral construction */
			  for (i=0; i<octaves; i++) {
					value += Noise3( point ) * exponent_array[i];
					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;
			  } /* for */
		
			  remainder = octaves - (int)octaves;
			  if ( remainder )      /* add in ``octaves''  remainder */
					/* ``i''  and spatial freq. are preset in loop above */
					value += remainder * Noise3( point ) * exponent_array[i];

			_result[0] = value;
			_result[4]= 1.0;
		
			return 0;
		} else {
			if (stype == 2){
				  if ( first ) {
						/* seize required memory for exponent_array */
						exponent_array = 
									(float *)mallocN( (octaves+1) * sizeof(float) , "test");
						frequency = 1.0;
						for (i=0; i<=octaves; i++) {
							  /* compute weight for each frequency */
							  exponent_array[i] = pow( frequency, -Hh );
							  frequency *= cast->lacun;
						}
						first = FALSE;
				  }
      /* first unscaled octave of function; later octaves are scaled */
      value = cast->offst + Noise3( point );
					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;

      /* spectral construction inner loop, where the fractal is built */
      for (i=1; i<octaves; i++) {
            /* obtain displaced noise value */
            increment = Noise3( point ) + cast->offst;
            /* scale amplitude appropriately for this frequency */
            increment *= exponent_array[i];
            /* scale increment by current `altitude' of function */
            increment *= value;
            /* add increment to ``value''  */
            value += increment;
            /* raise spatial frequency */
 					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;
     } /* for */

      /* take care of remainder in ``octaves''  */
      remainder = octaves - (int)octaves;
      if ( remainder ) {
            /* ``i''  and spatial freq. are preset in loop above */
            /* note that the main loop code is made shorter here */
            /* you may want to that loop more like this */
            increment = (Noise3( point ) + cast->offst) * exponent_array[i];
            value += remainder * increment * value;
      }
			_result[0] = value;
			_result[4]= 1.0;
		
			return 0;


		} else {
			if (stype == 3){
				  if ( first ) {
						/* seize required memory for exponent_array */
						exponent_array = 
									(float *)mallocN( (octaves+1) * sizeof(float) , "test");
						frequency = 1.0;
						for (i=0; i<=octaves; i++) {
							  /* compute weight for each frequency */
							  exponent_array[i] = pow( frequency, -Hh );
							  frequency *= cast->lacun;
						}
						first = FALSE;
				  }
     result = ( Noise3( point ) + cast->offst ) * exponent_array[0];
      weight = result;
      /* increase frequency */
 					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;

      /* spectral construction inner loop, where the fractal is built */
      for (i=1; i<octaves; i++) {
            /* prevent divergence */
            if ( weight > 1.0 )  weight = 1.0;

            /* get next higher frequency */
            signal = ( Noise3( point ) + cast->offst ) * exponent_array[i];
            /* add it in, weighted by previous freq's local value */
            result += weight * signal;
            /* update the (monotonically decreasing) weighting value */
            /* (this is why H must specify a high fractal dimension) */
            weight *= signal;

            /* increase frequency */
 					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;
      } /* for */

      /* take care of remainder in ``octaves''  */
      remainder = octaves - (int)octaves;
      if ( remainder )
            /* ``i''  and spatial freq. are preset in loop above */
            result += remainder * Noise3( point ) * exponent_array[i];
			_result[0] = result;
			_result[4]= 1.0;
		
			return 0;
		} else {
			if (stype == 4){
				  if ( first ) {
						/* seize required memory for exponent_array */
						exponent_array = 
									(float *)mallocN( (octaves+1) * sizeof(float) , "test");
						frequency = 1.0;
						for (i=0; i<=octaves; i++) {
							  /* compute weight for each frequency */
							  exponent_array[i] = pow( frequency, -Hh );
							  frequency *= cast->lacun;
						}
						first = FALSE;
				  }
      /* get first octave */
      signal = Noise3( point );
      /* get absolute value of signal (this creates the ridges) */
      if ( signal < 0.0 ) signal = -signal;
      /* invert and translate (note that "cast->offst" should be ~= 1.0) */
      signal = cast->offst - signal;
      /* square the signal, to increase "sharpness" of ridges */
      signal *= signal;
      /* assign initial values */
      result = signal;
      weight = 1.0;

      for( i=1; i<octaves; i++ ) {
            /* increase the frequency */
 					point[0] *= cast->lacun;
					point[1] *= cast->lacun;
					point[2] *= cast->lacun;

            /* weight successive contributions by previous signal */
            weight = signal * cast->gain;
            if ( weight > 1.0 ) weight = 1.0;
            if ( weight < 0.0 ) weight = 0.0;
            signal = Noise3( point );
            if ( signal < 0.0 ) signal = -signal;
            signal = cast->offst - signal;
            signal *= signal;
            /* weight the contribution */
            signal *= weight;
            result += signal * exponent_array[i];
      }
			_result[0] = result;
			_result[4]= 1.0;
		
			return 0;
}
	}
}
}}}

void plugin_tex_getinfo( int *stypes, int *vars)
{

	*stypes= NR_STYPES;
	/**vars= NR_VARS;*/
	*vars= sizeof(_varstr) / sizeof(VarStruct);
}

static void normalize2(float v[2])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

static void normalize3(float v[3])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}


static void init(void)
{
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		p[i] = i;

		g1[i] = (float)((random() % (B + B)) - B) / B;

		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((random() % (B + B)) - B) / B;
		normalize2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((random() % (B + B)) - B) / B;
		normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = random() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}
}

float Noise3(float vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register i, j;

	if (start) {
		start = 0;
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}


