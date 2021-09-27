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

/*

 * glxhelper.h:
 *
 *   List of drawing modes supported by GLXCreateWindow (in glxhelper.c).
 * More than this are possible with mixed model, but this is just an 
 * example.  You can either expand this list (and the corresponding code in 
 * GLXCreateWindow) or call the mixed model calls yourself, using 
 * GLXCreateWindow as an example.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GLXcolorIndexSingleBuffer,
    GLXcolorIndexDoubleBuffer,
    GLXrgbSingleBuffer,
    GLXrgbDoubleBuffer
} GLXWindowType;

extern Window GLXCreateWindow( Display*, Window, 
                               int, int, int, int, int, 
                               unsigned long, XSetWindowAttributes *,
                               GLXWindowType);

#ifdef __cplusplus
}
#endif

