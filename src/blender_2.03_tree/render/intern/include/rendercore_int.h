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

 * render_int.h
 *
 * Version: $Id: rendercore_int.h,v 1.1 2000/08/24 16:58:57 nzc Exp $
 */

#ifndef RENDER_INT_H
#define RENDER_INT_H "$Id: rendercore_int.h,v 1.1 2000/08/24 16:58:57 nzc Exp $"

#include "zbuf_types.h"
#include "render_types.h"

float   CookTorr(float *n, float *l, float *v, int hard);
void    do_lamphalo_tex(LampRen *lar, float *p1, float *p2, float *intens);
void    spothalo(struct LampRen *lar, float *view, float *intens);
void    add_filt_mask(uint mask, ushort *col, uint *rb1, uint *rb2, uint *rb3);
void    addps(long *rd, int vlak, uint z, short ronde);
PixStr *addpsmain(void);
float   count_maskf(ushort mask);
void    freeps(void);
void    halovert(void);
void    renderhalo(HaloRen *har);	/* postprocess versie */
void scanlinehaloPS(uint *rectz, long *rectdelta, uint *rectt, short ys);


#endif /* RENDER_INT_H */


