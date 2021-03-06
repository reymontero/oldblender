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

 * vanillaRenderPipe_int.h
 *
 * Version: $Id: vanillaRenderPipe_int.h,v 1.4 2000/09/14 09:11:40 nzc Exp $
 */

#ifndef VANILLARENDERPIPE_INT_H
#define VANILLARENDERPIPE_INT_H "$Id: vanillaRenderPipe_int.h,v 1.4 2000/09/14 09:11:40 nzc Exp $"

#include "vanillaRenderPipe_types.h"
#include "zbufferdatastruct_types.h"

/**
 * Z buffer initializer, for new pipeline.
 * <LI>
 * <IT> AColourBuffer : colour buffer for one line
 * <IT> APixbufExt    : pixel data buffer for one line, depth RE_ZBUFLEN 
 * </LI>
 */
void initRenderBuffers(int width);
/*  void initRenderBuffers(void); */

/**
 * Z buffer destructor, frees stuff from initZBuffers().
 */
void freeRenderBuffers(void);

/** 
 * Fill the accumulation buffer APixbufExt with face and halo indices. 
 * Note: Uses globals.
 * @param y the line number to set
 */
void calcZBufLine(int y);

/** 
 * Shade and render the pixels in this line, into AColourBuffer
 * Note: Uses globals.
 * @param y the line number to set
 */
void renderZBufLine(int y);

/**
 * Count and sort the list behind ap into buf. Sorts on min. distance.
 * Low index <=> high z
 */
int countAndSortPixelFaces(int buf[RE_MAX_FACES_PER_PIXEL][5], 
                           RE_APixstrExt *ap);

/**
 * Oversample buffer with conflict resolution. Output to sampcol.
 * - proper antialiasing on faces
 * - add-blending on faces
 * - alpha thresholding on all faces, and on halos
 * - traced
 * @param zrow    the sorted list of faces and distances
 * @param totvlak the number of faces in this list
 * @param x       the x pixel coordinate in SCS
 * @param y       the y pixel coordinate in SCS
 * @param osaNr   the number times the picture is oversampled
 */
void oversamplePixel(int zrow[RE_MAX_FACES_PER_PIXEL][RE_PIXELFIELDSIZE],
					int totvlak, float x, float y, int osaNr);

/**
 * Calculate the view depth to this object on this location, with 
 * the current view parameters in R.
 */
int calcDepth(float x, float y, void *data, int type);



/**
 * Fills in distances of all faces in a z buffer, for given jitter settings.
 */
int fillZBufDistances(void);

/**
 * Fills in distances of faces in the z buffer.
 *
 * Halo z buffering ---------------------------------------------- 
 *
 * A halo is treated here as a billboard: no z-extension, always   
 * oriented perpendicular to the viewer. The rest of the z-buffer  
 * stores face-numbers first, then calculates colours as the       
 * final image is rendered. We'll use the same approach here,      
 * which differs from the original method (which was add halos per 
 * scan line). This means that the z-buffer now also needs to      
 * store info about what sort of 'thing' the index refers to.      
 *                                                                 
 * Halo extension:                                                 
 * h.maxy  ---------                                               
 *         |          h.xs + h.rad                                 
 *             |      h.xs                                         
 *                 |  h.xs - h.rad                                 
 * h.miny  ---------                                               
 *                                                                 
 * These coordinates must be clipped to picture size.              
 * I'm not quite certain about halo numbering.                     
 *                                                                 
 * Halos and jittering -------------------------------------------  
 *                                                                 
 * Halos were not jittered previously. Now they are. I wonder      
 * whether this may have some adverse effects here.                
 
 * @return 1 for succes, 0 if the operation was interrupted.
 */
int zBufferAllFaces(void);

/**
 * Fills in distances of halos in the z buffer.
 * @return 1 for succes, 0 if the operation was interrupted.
 */
int zBufferAllHalos(void);

/**
 * New fill function for z buffer, for edge-only rendering.
 */
void zBufferFillEdge(float *vec1, float *vec2);

/**
 * New fill function for z buffer.
 */
void zBufferFillFace(float *v1, float *v2, float *v3);

/**
 * One more filler: fill in halo data in z buffer.
 * Empty so far, but may receive content of halo loop.
 */
void zBufferFillHalo(void);

/**
 * Copy the colour buffer output to R.rectot, to line y.
 */
void transferColourBufferToOutput(int y);

/**
 * Set the colour buffer fields to zero.
 */
void eraseColBuf(RE_COLBUFTYPE *buf);

/**
 * Blend source over dest, and leave result in dest. 1 pixel.
 */
void blendOverFloat(int type, float* dest, float* source, void* data);

/**
 * Blend source over dest, and leave result in dest. 1 pixel into 
 * multiple bins.
 */
void blendOverFloatRow(int type, float* dest, float* source, 
                       void* data, int mask, int osaNr) ;

#endif /* VANILLARENDERPIPE_INT_H */

