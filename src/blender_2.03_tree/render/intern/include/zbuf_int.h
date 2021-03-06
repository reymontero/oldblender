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

/*

 * zbuf_int.h
 * internal interface for zbuf.h (ie. functions that are not used
 * anywhere else) 
 *
 * Version: $Id: zbuf_int.h,v 1.2 2000/08/28 12:23:44 nzc Exp $
 */

#ifndef ZBUF_INT_H
#define ZBUF_INT_H "$Id: zbuf_int.h,v 1.2 2000/08/28 12:23:44 nzc Exp $"

#include "render_types.h"
#include "zbuf_types.h"



/**
 * Convert a homogenous coordinate to a z buffer coordinate. The
 * function makes use of Zmulx, Zmuly, the x and y scale factors for
 * the screen, and Zjitx, Zjity, the pixel offset. (These are declared
 * in render.c) The normalised z coordinate must fall on [0, 1]. 
 * @param zco  [3, 4 floats] pointer to the resulting z buffer coordinate
 * @param hoco [4 floats] pointer to the homogenous coordinate of the
 * vertex in world space.
 */
void hoco_to_zco(float *zco, float *hoco);

/**
 * Fill the z buffer for alpha?
 *
 * This is one of the z buffer fill functions called in zbufclip() and
 * zbufwireclip(). 
 *
 * @param v1 [4 floats, world coordinates] first vertex
 * @param v2 [4 floats, world coordinates] second vertex
 * @param v3 [4 floats, world coordinates] third vertex
 */
void zbufinvulAc(float *v1, float *v2, float *v3);

/**
 * Fill the z buffer, but invert z order, and add the face index to
 * the corresponing face buffer.
 *
 * This is one of the z buffer fill functions called in zbufclip() and
 * zbufwireclip(). 
 *
 * @param v1 [4 floats, world coordinates] first vertex
 * @param v2 [4 floats, world coordinates] second vertex
 * @param v3 [4 floats, world coordinates] third vertex
 */
void zbufinvulGLinv(float *v1, float *v2, float *v3);

/**
 * Fill the z buffer, and add the face index to
 * the corresponing face buffer.  Writes into R.rectz and R.rectot. It
 * assumes that Zvlnr is set to the face index of the face under
 * consideration. Zvlnr is written into R.rectot. R.rectz
 *
 * This is one of the z buffer fill functions called in zbufclip() and
 * zbufwireclip(). 
 *
 * @param v1 [4 floats, world coordinates] first vertex
 * @param v2 [4 floats, world coordinates] second vertex
 * @param v3 [4 floats, world coordinates] third vertex
 */
void zbufinvulGL(float *v1, float *v2, float *v3);

/**
 * Fill the z buffer. The face buffer is not operated on!
 *
 * This is one of the z buffer fill functions called in zbufclip() and
 * zbufwireclip(). 
 *
 * @param v1 [4 floats, world coordinates] first vertex
 * @param v2 [4 floats, world coordinates] second vertex
 * @param v3 [4 floats, world coordinates] third vertex
 */
void zbufinvulGL_onlyZ(float *v1, float *v2, float *v3);

/**
 * Prints 3 unlabelled floating point values to stdout. Used for diagnostics.
 * @param v1 any float
 * @param v2 any float
 * @param v3 any float
 */
void print3floats(float *v1, float *v2, float *v3);

/**
 * Checks labda and uses this to make decision about clipping the line
 * segment from v1 to v2. labda is the factor by which the vector is
 * cut. ( calculate s + l * ( t - s )). The result is appended to the
 * vertex list of this face.
 * Note: uses globals.
 * (arguments: one int, one pointer to int... why?)
 * @param v1 start coordinate s
 * @param v2 target coordinate t
 * @param b1 
 * @param b2 
 * @param clve vertex vector.
 */
static void  maakvertpira(float *v1, float *v2, int *b1, int b2, int *clve);

/**
 * Sets labda: flag, and parametrize the clipping of vertices in
 * viewspace coordinates. labda = -1 means no clipping, labda in [0,
 * 1] means a clipping.
 * Note: uses globals.
 * @param v1 start coordinate s
 * @param v2 target coordinate t
 * @param b1 
 * @param b2 
 * @param b3
 * @param a index for coordinate (x, y, or z)
 */
static void  clipp(float *v1, float *v2, int b1, int *b2, int *b3, int a);

/**
 * Tests whether this coordinate is 'inside' or 'outside' of the view
 * volume? By definition, this is in [0, 1]. 
 * @param p vertex z difference plus coordinate difference?
 * @param q origin z plus r minus some coordinate?
 * @param u1 [in/out] clip fraction for ?
 * @param u2 [in/out]
 * @return 0 if point is outside, or 1 if the point lies on the clip
 *         boundary 
 */
static short cliptestf(float p, float q, float *u1, float *u2);


/* not documented yet */
/* not sure if these should stay static... */

static int   clipline(float *v1, float *v2);

/**
 * Provide book-keeping for the z buffer data lists.
 */
APixstr     *addpsmainA(void);
void         freepsA(void);
APixstr     *addpsA();

/**
 * Fill function for the z buffer (fills lines)
 */
void         zbuflineAc(float *vec1, float *vec2);
void         zbufline(float *vec1, float *vec2);


int          hashlist_projectvert(float *v1, float *hoco);

/**
 * Copy results from the solid face z buffering to the transparent
 * buffer.
 */
void         copyto_abufz(int sample);

/**
 * Do accumulation z buffering.
 */
void         zbuffer_abuf(void);

/**
 * Shade this face at this location in SCS.
 */
void         shadetrapixel(float x, float y, int vlak);

/**
 * Determine the distance to the camera of this halo, in ZCS.
 */
uint         calcHaloDist(HaloRen *har);

#endif /* ZBUF_INT_H */






