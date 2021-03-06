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

/**

 * zbufferdatastruct.c
 *
 * Version: $Id: zbufferdatastruct.c,v 1.3 2000/09/14 16:32:49 nzc Exp $
 *
 * The z buffer consists of an array of lists. Each list holds the objects 
 * behind a pixel. These can be sorted for closest distance. Per object,
 * we store: 
 * - object type
 * - object index
 * - minimum distance
 * - maximum distance
 * - oversample flags
 *
 * The buffer was created to fit the new unified renderpipeline. We might 
 * turn it into an object later on.
 *
 * The z buffer has an unlimited depth. The oversampling code chops at a 
 * certain number of faces. This number is defined in 
 * vanillaRenderPipe_types.h
 *
 * Version 1 of the z buffer inserted objects by means of linear
 * search: we walk along the list until we find the right object or
 * until we have to insert a new one. This is terribly inefficient
 * when we are dealing with large numbers of objects. Can we find a
 * better solution here?
 *
 * Because we treat halos as billboards, we optimize halo
 * insertion. For this purpose the fillFlatObject() functions have
 * been implemented.  */

#include "blender.h"
#include "zbufferdatastruct.h"

/* if defined: all jittersamples are stored individually. _very_ serious     */
/* performance hit ! also gives some buffer size problems in big scenes      */
/* #define RE_INDIVIDUAL_SUBPIXELS */

/* ------------------------------------------------------------------------- */

static RE_APixstrExtMain RE_apsemfirst; /* pixstr bookkeeping var            */
static short RE_apsemteller = 0;        /* pixstr bookkeeping var            */
static int RE_zbufferwidth;             /* width of the z-buffer (pixels)    */
RE_APixstrExt *APixbufExt;     /* Zbuffer: linked list of face, halo indices */



/*-RE_APixstrExt------------------------------------------------------------ */ 

void initZbuffer(int width)
{
    APixbufExt         = callocN(RE_ZBUFLEN * width * sizeof(RE_APixstrExt), 
								 "APixbufExt");
	RE_zbufferwidth    = width;
    RE_apsemteller     = 0;
    RE_apsemfirst.next = NULL;
    RE_apsemfirst.ps   = NULL;
} /* end of RE_APixstrExt *initZbufferDataStruct() */

/* ------------------------------------------------------------------------- */

void freeZbuffer(void)
{
	if (APixbufExt) freeN(APixbufExt); 
	freepseA(); 
} /* end of void freeZbuffer(void) */

/* ------------------------------------------------------------------------- */

void resetZbuffer(void)
{
	int len;

	freepseA(); 
	len = sizeof(RE_APixstrExt) * RE_zbufferwidth * RE_ZBUFLEN;
	bzero(APixbufExt, len);
} /* end of void resetZbuffer(void) */

/* ------------------------------------------------------------------------- */

RE_APixstrExt *addpsemainA()
{
	RE_APixstrExtMain *psm;

	psm= &RE_apsemfirst;

	while(psm->next) {
		psm= psm->next;
	}

	psm->next= callocN(sizeof(RE_APixstrExtMain), "addpsemainA");

	psm= psm->next;

   /* Initialise the new structure to safe values. Memory that is newly */
   /* allocated must be zero... Not sure if that happens everywhere now.*/
	psm->next=0;
	psm->ps= callocN(4096*sizeof(RE_APixstrExt),"pixstrext");
	RE_apsemteller= 0;

	return psm->ps;
} /* End of RE_APixstrExt *addpsemainA() */

/* ------------------------------------------------------------------------- */

void freepseA()
{
	RE_APixstrExtMain *psm, *next;

	psm= &RE_apsemfirst;

	while(psm) {
		next= psm->next;
		if(psm->ps) {
			freeN(psm->ps);
			psm->ps= 0;
		}
		if(psm!= &RE_apsemfirst) freeN(psm);
		psm= next;
	}

	RE_apsemfirst.next= 0;
	RE_apsemfirst.ps= 0;
	RE_apsemteller= 0;
} /* End of void freepseA() */

/* ------------------------------------------------------------------------- */

RE_APixstrExt *addpseA(void)
{
	static RE_APixstrExt *prev;

	/* eerste PS maken */
	if((RE_apsemteller & 4095)==0) prev= addpsemainA();
	else prev++;
	RE_apsemteller++;
	
	return prev;
} /* End of RE_APixstrExt *addpseA(void) */

/* ------------------------------------------------------------------------- */

void insertObject(RE_APixstrExt* apn, 
				  int obindex,
				  int obtype, 
				  int dist, 
				  int mask)
{
	/* Guard the insertion if needed? */
	while(apn) {
		if(apn->t[0] == RE_NONE) {
			apn->p[0] = obindex; apn->t[0] = obtype;
			apn->zmin[0] = dist; apn->zmax[0] = dist;
			apn->mask[0] = mask;   
			break; 
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if((apn->p[0] == obindex) && (apn->t[0] & obtype)) {
			if(dist < apn->zmin[0]) apn->zmin[0] = dist;
			else if(dist > apn->zmax[0]) apn->zmax[0] = dist;
			apn->mask[0]|= mask; 
			break; 
		} 
#endif
		if(apn->t[1] == RE_NONE) {
			apn->p[1] = obindex; apn->t[1] = obtype;
			apn->zmin[1] = dist; apn->zmax[1] = dist;
			apn->mask[1] = mask;   
			break; 
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if((apn->p[1] == obindex) && (apn->t[1] & obtype)) {
			if(dist < apn->zmin[1]) apn->zmin[1] = dist;
			else if(dist > apn->zmax[1]) apn->zmax[1] = dist;
			apn->mask[1]|= mask; 
			break; 
		} 
#endif
		if(apn->t[2] == RE_NONE) {
			apn->p[2] = obindex; apn->t[2] = obtype;
			apn->zmin[2] = dist; apn->zmax[2] = dist;
			apn->mask[2] = mask;   
			break; 
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if((apn->p[2] == obindex) && (apn->t[2] & obtype)) {
			if(dist < apn->zmin[2]) apn->zmin[2] = dist;
			else if(dist > apn->zmax[2]) apn->zmax[2] = dist;
			apn->mask[2]|= mask; 
			break; 
		} 
#endif
		if(apn->t[3] == RE_NONE) {
			apn->p[3] = obindex; apn->t[3] = obtype;
			apn->zmin[3] = dist; apn->zmax[3] = dist;
			apn->mask[3] = mask;   
			break; 
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if((apn->p[3] == obindex) && (apn->t[3] & obtype)) {
			if(dist < apn->zmin[3]) apn->zmin[3] = dist;
			else if(dist > apn->zmax[3]) apn->zmax[3] = dist;
			apn->mask[3]|= mask; 
			break; 
		} 
#endif
		if(apn->next==0) apn->next= addpseA();
		apn= apn->next;
	}				
} /* end of insertObject(RE_APixstrExt*, int, int, int, int) */

/* ------------------------------------------------------------------------- */

void insertFlatObject(RE_APixstrExt *apn, int obindex, int obtype, int dist, int mask)
{
	while(apn) {
		if(apn->t[0] == RE_NONE) {
			apn->p[0] = obindex; apn->zmin[0] = dist; 
			apn->zmax[0] = dist; apn->mask[0] = mask; 
			apn->t[0] = obtype;  
			break; 
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if( (apn->t[0] & obtype) && (apn->p[0] == obindex)) {
			apn->mask[0]|= mask; break; 
		}
#endif
		if(apn->t[1] == RE_NONE) {
			apn->p[1] = obindex; apn->zmin[1] = dist; 
			apn->zmax[1] = dist; apn->mask[1] = mask; 
			apn->t[1] = obtype;  
			break;
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if( (apn->t[1] & obtype) && (apn->p[1] == obindex)) {
			apn->mask[1]|= mask; break; 
		}
#endif
		if(apn->t[2] == RE_NONE) {
			apn->p[2] = obindex; apn->zmin[2] = dist; 
			apn->zmax[2] = dist; apn->mask[2] = mask; 
			apn->t[2] = obtype;  
			break;
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if( (apn->t[2] & obtype) && (apn->p[2] == obindex)) {
			apn->mask[2]|= mask; break; 
		}
#endif
		if(apn->t[3] == RE_NONE) {
			apn->p[3] = obindex; apn->zmin[3] = dist; 
			apn->zmax[3] = dist; apn->mask[3] = mask; 
			apn->t[3] = obtype;  
			break;
		}
#ifndef RE_INDIVIDUAL_SUBPIXELS
		if( (apn->t[3] & obtype) && (apn->p[3] == obindex)) {
			apn->mask[3]|= mask; break; 
		}
#endif
		if(apn->next==0) apn->next= addpseA();
		apn= apn->next;
	};                    
} /* end of void insertFlatObject(RE_APixstrExt, int, int, int, int)*/

/* ------------------------------------------------------------------------- */
/* This function might be helped by an end-of-list marker                    */
void insertFlatObjectNoOsa(RE_APixstrExt *ap, 
						   int obindex, 
						   int obtype, 
						   int dist, 
						   int mask)
{
	while(ap) {
		if(ap->t[0] == RE_NONE) {
			ap->p[0] = obindex; ap->zmin[0] = dist; 
			ap->zmax[0] = dist; ap->mask[0] = mask; 
			ap->t[0] = obtype;  
			break; 
		}

		if(ap->t[1] == RE_NONE) {
			ap->p[1] = obindex; ap->zmin[1] = dist; 
			ap->zmax[1] = dist; ap->mask[1] = mask; 
			ap->t[1] = obtype;  
			break;
		}

		if(ap->t[2] == RE_NONE) {
			ap->p[2] = obindex; ap->zmin[2] = dist; 
			ap->zmax[2] = dist; ap->mask[2] = mask; 
			ap->t[2] = obtype;  
			break;
		}

		if(ap->t[3] == RE_NONE) {
			ap->p[3] = obindex; ap->zmin[3] = dist; 
			ap->zmax[3] = dist; ap->mask[3] = mask; 
			ap->t[3] = obtype;  
			break;
		}

		if(ap->next==0) ap->next= addpseA();
		ap= ap->next;
	};                    
} /* end of void insertFlatObjectNoOsa(RE_APixstrExt, int, int, int, int)*/

/* ------------------------------------------------------------------------- */

/* EOF */

