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

#ifndef VFR_UTIL_H

#define VFR_UTIL_H
/*
#include <sys/types.h>
#include <vfr/vframer.h>
vframer.h mag niet twee keer geinclude worden
*/

#define SHIFTBIT    0
#define TOYUV	    7

extern VFR_DEV *vfr;
extern long vlan;
extern long barcolors[];

extern long escaped();
extern void vfr_rgb(struct ImBuf * ibuf,long flags);
extern void colorbars(long flags);
extern void black(long flags);
extern void inittape(long flags);
extern char *vlancmd(char * command);
extern long vlan_ready();
extern char * vlantime(char * code, long time);
#define VFR_CUSTOM_HPHASE (vfr_read(vfr,VFR_REG_HORIZONTAL_PHASE))
#endif

