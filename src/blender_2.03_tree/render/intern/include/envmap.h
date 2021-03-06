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

 * envmap_ext.h
 *
 * Version: $Id: envmap.h,v 1.3 2000/08/28 12:23:44 nzc Exp $
 */

#ifndef ENVMAP_EXT_H
#define ENVMAP_EXT_H "$Id: envmap.h,v 1.3 2000/08/28 12:23:44 nzc Exp $"

/* no types!!! */
/*  #include "envmap_types.h" */

/**
 * Make environment maps for all objects in the scene that have an
 * environment map as texture. 
 * (initrender.c)
 */
void make_envmaps(void);

#endif /* ENVMAP_EXT_H */


