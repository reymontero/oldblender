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

/* *******************************************************************************

*
* Copyright 1991, Silicon Graphics, Inc.
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
* the contents of this file may not be disclosed to third parties, copied or
* duplicated in any form, in whole or in part, without the prior written
* permission of Silicon Graphics, Inc.
*
* RESTRICTED RIGHTS LEGEND:
* Use, duplication or disclosure by the Government is subject to restrictions
* as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
* and Computer Software clause at DFARS 252.227-7013, and/or in similar or
* successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
* rights reserved under the Copyright Laws of the United States.
*
******************************************************************************* */
#include <math.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdlib.h>

#ifndef _SGI_FFT_
#define	_SGI_FFT_

#define	    FACTOR_SPACE    15
/* *******************************************************
    Complex structures definitions
******************************************************* */

typedef struct {
    float re;
    float im;
} complex;

typedef struct {
    double re;
    double im;
} zomplex;


/* *******************************************************
    C Functions prototypes
******************************************************* */
/* *******************************************************
	complex <---> complex FFTs
******************************************************* */
complex *cfft1di( int n, complex *save);
int cfft1d( int job, int n, complex *array, int inc, complex *save);

complex *cfft2di( int n1, int n2, complex *save);
int cfft2d( int job, int n1, int n2, complex *array, int ld, complex *save);

complex *cfft3di( int n1, int n2, int n3, complex *save);
int cfft3d( int job, int n1, int n2, int n3, complex *array, int ld1, int ld2, complex *save);


/* *******************************************************
	zomplex <---> zomplex FFTs
******************************************************* */
zomplex *zfft1di( int n, zomplex *save);
int zfft1d( int job, int n, zomplex *array, int inc, zomplex *save);

zomplex *zfft2di( int n1, int n2, zomplex *save);
int zfft2d( int job, int n1, int n2, zomplex *array, int ld, zomplex *save);

zomplex *zfft3di( int n1, int n2, int n3, zomplex *save);
int zfft3d( int job, int n1, int n2, int n3, zomplex *array, int ld1, int ld2, zomplex *save);


/* *******************************************************
	real <---> complex FFTs
******************************************************* */
float *sfft1di( int n, float *save);
int sfft1d( int job, int n, float *array, int inc, float *save);

float *sfft2di( int n1, int n2, void *save);
int sfft2d( int job, int n1, int n2, void *array, int ld, void *save);

float *sfft3di( int n1, int n2, int n3, void *save);
int sfft3d( int job, int n1, int n2, int n3, void *array, int ld1, int ld2, void *save);


/* *******************************************************
	double <---> zomplex FFTs
******************************************************* */
double *dfft1di( int n, double *save);
int dfft1d( int job, int n, double *array, int inc, double *save);

double *dfft2di( int n1, int n2, void *save);
int dfft2d( int job, int n1, int n2, void *array, int ld, void *save);

double *dfft3di( int n1, int n2, int n3, void *save);
int dfft3d( int job, int n1, int n2, int n3, void *array, int ld1, int ld2, void *save);


/* *******************************************************
    Fortran Subroutines prototypes
******************************************************* */
/* *******************************************************
	complex <---> complex FFTs
******************************************************* */
void cfft1di_( int *n, complex *save);
int cfft1d_( int *job, int *n, complex *array, int *inc, complex *save);

void cfft2di_( int *n1, int *n2, complex *save);
void cfft2d_( int *job, int *n1, int *n2, complex *array, int *ld, complex *save);

void cfft3di_( int *n1, int *n2, int *n3, complex *save);
void cfft3d_( int *job, int *n1, int *n2, int *n3, complex *array, int *ld1, int *ld2, complex *save);

/* ****************************
	zomplex <---> zomplex FFTs
**************************** */
void zfft1di_( int *n, zomplex *save);
void zfft1d_( int *job, int *n, zomplex *array, int *inc, zomplex *save);

void zfft2di_( int *n1, int *n2, zomplex *save);
void zfft2d_( int *job, int *n1, int *n2, zomplex *array, int *ld, zomplex *save);

void zfft3di_( int *n1, int *n2, int *n3, zomplex *save);
void zfft3d_( int *job, int *n1, int *n2, int *n3, zomplex *array, int *ld1, int *ld2, zomplex *save);

/* ********************************************************
	real <---> complex FFTs
******************************************************** */
void *sfft1di_( int *n, float *save);
int *sfft1d_( int *job, int *n, float *array, int *inc, float *save);

void *sfft2di_( int *n1, int *n2, void *save);
int *sfft2d_( int *job, int *n1, int *n2, void *array, int *ld, void *save);

void *sfft3di_( int *n1, int *n2, int *n3, void *save);
int *sfft3d_( int *job, int *n1, int *n2, int *n3, void *array, int *ld1, int *ld2, void *save);

/* ********************************************************
	double <---> zomplex FFTs
******************************************************** */
void *dfft1di_( int *n, double *save);
int *dfft1d_( int *job, int *n, double *array, int *inc, double *save);

void *dfft2di_( int *n1, int *n2, void *save);
int *dfft2d_( int *job, int *n1, int *n2, void *array, int *ld, void *save);

void *dfft3di_( int *n1, int *n2, int *n3, void *save);
int *dfft3d_( int *job, int *n1, int *n2, int *n3, void *array, int *ld1, int *ld2, void *save);

#endif

