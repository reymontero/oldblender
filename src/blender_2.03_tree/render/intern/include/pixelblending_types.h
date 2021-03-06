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

 * pixelblending_types.h
 * types pixelblending 
 *
 * Version: $Id: pixelblending_types.h,v 1.4 2000/09/14 09:11:40 nzc Exp $
 */

#ifndef PIXELBLENDING_TYPES_H
#define PIXELBLENDING_TYPES_H "$Id: pixelblending_types.h,v 1.4 2000/09/14 09:11:40 nzc Exp $"

#include "blender.h"

/* Threshold for a 'full' pixel: pixels with alpha above this level are      */
/* considered opaque This is the decimal value for 0xFFF0 / 0xFFFF           */
#define RE_FULL_COLOUR_FLOAT 0.9998
/* Threshold for an 'empty' pixel: pixels with alpha above this level are    */
/* considered completely transparent. This is the decimal value              */
/* for 0x000F / 0xFFFF                                                       */
#define RE_EMPTY_COLOUR_FLOAT 0.0002
/* A 100% pixel. Sometimes, seems to be too little.... Hm.......             */
#define RE_UNITY_COLOUR_FLOAT 1.0
/* A 0% pixel. I wonder how 0 the 0.0 is...                                  */
#define RE_ZERO_COLOUR_FLOAT 0.0

/* threshold for alpha                                                       */
#define RE_FULL_ALPHA_FLOAT 0.9998

/* Same set of defines for shorts                                            */
#define RE_FULL_COLOUR_SHORT 0xFFF0
#define RE_EMPTY_COLOUR_SHORT 0x0000

/* Default gamma. For most CRTs, gamma ranges from 2.2 to 2.5 (Foley), so    */
/* 2.35 seems appropriate enough. Experience teaches a different number      */
/* though. Old blender: 2.0. It  might be nice to make this a slider         */
#define RE_DEFAULT_GAMMA 2.0
/* This 400 is sort of based on the number of intensity levels needed for    */
/* the typical dynamic range of a medium, in this case CRTs. (Foley)         */
/* (Actually, it says the number should be between 400 and 535.)             */
#define RE_GAMMA_TABLE_SIZE 400

#endif /* PIXELBLENDING_EXT_H */


