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

 * vanillaRenderPipe_ext.h
 *
 * Version: $Id: vanillaRenderPipe.h,v 1.1 2000/08/24 16:58:57 nzc Exp $
 */

#ifndef VANILLARENDERPIPE_EXT_H
#define VANILLARENDERPIPE_EXT_H "$Id: vanillaRenderPipe.h,v 1.1 2000/08/24 16:58:57 nzc Exp $"

#include "vanillaRenderPipe_types.h"

/**
 * Render pipeline with all kinds of extras.
 *    status-------------------------\/
 * - integrated z buffering          ok
 * - integrated halo rendering       ok
 */
void zBufShadeAdvanced(void);


#endif /* VANILLARENDERPIPE_EXT_H */

