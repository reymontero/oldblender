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

#include "imbuf.h"


#define OBJECTBLOK "allocimbuf"

ulong dfltcmap[16] = {
	0x00000000, 0xffffffff, 0xff777777, 0xffcccccc, 
	0xff4433cc, 0xff4488dd, 0xff44ddcc, 0xff338888, 
	0xff448833, 0xff44dd44, 0xffccdd44, 0xffcc8833, 
	0xffdd8888, 0xffcc3344, 0xffcc33cc, 0xffdd88cc
};

void freeplanesImBuf(struct ImBuf * ibuf)
{
	if (ibuf==0) return;
	if (ibuf->planes){
		if (ibuf->mall & IB_planes) free(ibuf->planes);
	}
	ibuf->planes = 0;
	ibuf->mall &= ~IB_planes;
}


void freerectImBuf(struct ImBuf * ibuf)
{
	if (ibuf==0) return;
	if (ibuf->rect){
		if (ibuf->mall & IB_rect) free(ibuf->rect);
	}
	ibuf->rect=0;
	ibuf->mall &= ~IB_rect;
}


void freezbufImBuf(struct ImBuf * ibuf)
{
	if (ibuf==0) return;
	if (ibuf->zbuf){
		if (ibuf->mall & IB_zbuf) free(ibuf->zbuf);
	}
	ibuf->zbuf=0;
	ibuf->mall &= ~IB_zbuf;
}


void freecharImBuf(struct ImBuf * ibuf)
{
	if (ibuf == 0) return;
	if (ibuf->chardata){
		if (ibuf->mall & IB_char) free(ibuf->chardata);
	}
	ibuf->chardata = 0;
	ibuf->mall &= ~IB_char;
}


void freecmapImBuf(struct ImBuf * ibuf)
{
	if (ibuf == 0) return;
	if (ibuf->cmap){
		if (ibuf->mall & IB_cmap) free(ibuf->cmap);
	}
	ibuf->cmap = 0;
	ibuf->mall &= ~IB_cmap;
}


void freeImBuf(struct ImBuf * ibuf)
{
	if (ibuf){
		freeplanesImBuf(ibuf);
		freerectImBuf(ibuf);
		freezbufImBuf(ibuf);
		freecharImBuf(ibuf);
		freecmapImBuf(ibuf);
		free(ibuf);
	}
}


short addcharImBuf(struct ImBuf * ibuf)
{
	long size;

	if (ibuf == 0) return(FALSE);
	freecharImBuf(ibuf);

	size = ibuf->x * ibuf->y * sizeof(uchar);
	if (ibuf->chardata = mallocN(size, "addcharImBuf")){
		ibuf->mall |= IB_char;
		return (TRUE);
	}

	return (FALSE);
}


short addzbufImBuf(struct ImBuf * ibuf)
{
	long size;

	if (ibuf==0) return(FALSE);
	freezbufImBuf(ibuf);

	size = ibuf->x * ibuf->y * sizeof(ulong);
	if (ibuf->zbuf = mallocN(size, "addzbufImBuf")){
		ibuf->mall |= IB_zbuf;
		return (TRUE);
	}

	return (FALSE);
}


short addrectImBuf(struct ImBuf * ibuf)
{
	long size;

	if (ibuf==0) return(FALSE);
	freerectImBuf(ibuf);

	size = ibuf->x * ibuf->y * sizeof(ulong);
	if (ibuf->rect = mallocN(size, "addrectImBuf")){
		ibuf->mall |= IB_rect;
		if (ibuf->depth > 32) return (addzbufImBuf(ibuf));
		else return (TRUE);
	}

	return (FALSE);
}


short addcmapImBuf(ibuf)
struct ImBuf *ibuf;
{
	long min;
	
	if (ibuf==0) return(FALSE);
	freecmapImBuf(ibuf);

	checkncols(ibuf);
	if (ibuf->maxcol == 0) return (TRUE);

	if (ibuf->cmap = callocN(sizeof(ulong) * ibuf->maxcol, "addcmapImBuf")){
		min = ibuf->maxcol * sizeof(ulong);
		if (min > sizeof(dfltcmap)) min = sizeof(dfltcmap);
		memcpy(ibuf->cmap, dfltcmap, min);
		ibuf->mall |= IB_cmap;
		return (TRUE);
	}

	return (FALSE);
}


short addplanesImBuf(ibuf)
struct ImBuf *ibuf;
{
	long size;
	short skipx,d,x,y;
	ulong **planes;
	ulong *point2;

	if (ibuf==0) return(FALSE);
	freeplanesImBuf(ibuf);

	skipx = ((ibuf->x+31) >> 5);
	ibuf->skipx=skipx;
	x=ibuf->x;
	y=ibuf->y;
	d=ibuf->depth;

	planes = mallocN((d*skipx*y + d) * sizeof(long), "addplanesImBuf");
	ibuf->planes = planes;
	if (planes==0) return (FALSE);

	point2 = (ulong *)planes;
	point2 += d;
	size = skipx*y;

	for (;d>0;d--){
		*(planes++) = point2;
		point2 += size;
	}
	ibuf->mall |= IB_planes;

	return (TRUE);
}


struct ImBuf *allocImBuf(short x,short y,uchar d,ulong flags,uchar bitmap)
{
	struct ImBuf *ibuf;

	ibuf = callocN(sizeof(struct ImBuf), "ImBuf_struct");
	if (bitmap) flags |= IB_planes;

	if (ibuf){
		ibuf->x=x;
		ibuf->y=y;
		ibuf->depth=d;
		ibuf->ftype=TGA;

		if (flags & IB_rect){
			if (addrectImBuf(ibuf)==FALSE){
				freeImBuf(ibuf);
				return (0);
			}
		}
		
		if (flags & IB_zbuf){
			if (addzbufImBuf(ibuf)==FALSE){
				freeImBuf(ibuf);
				return (0);
			}
		}
		
		if (flags & IB_planes){
			if (addplanesImBuf(ibuf)==FALSE){
				freeImBuf(ibuf);
				return (0);
			}
		}
		if (flags & IB_char){
			if (flags & IB_charonlong) ibuf->x = (x + 3) & ~3;

			if (addcharImBuf(ibuf)==FALSE){
				freeImBuf(ibuf);
				return (0);
			}

			ibuf->x = x;
		}
	}
	return (ibuf);
}


struct ImBuf *dupImBuf(ibuf1)
struct ImBuf *ibuf1;
{
	struct ImBuf *ibuf2, tbuf;
	long flags = 0;
	long x, y;
	
	if (ibuf1 == 0) return (0);

	if (ibuf1->rect) flags |= IB_rect;
	if (ibuf1->planes) flags |= IB_planes;
	if (ibuf1->chardata) flags |= IB_char;

	x = ibuf1->x;
	y = ibuf1->y;
	if (ibuf1->flags & IB_fields) y *= 2;
	
	ibuf2 = allocImBuf(x, y, ibuf1->depth, flags, 0);
	if (ibuf2 == 0) return (0);

	if (flags & IB_rect) memcpy(ibuf2->rect,ibuf1->rect,x * y * sizeof(long));
	if (flags & IB_planes) memcpy(*(ibuf2->planes),*(ibuf1->planes),ibuf1->depth * ibuf1->skipx * y * sizeof(long));
	if (flags & IB_char) memcpy(ibuf2->chardata, ibuf1->chardata, x * y * sizeof(uchar));

	tbuf = *ibuf1;
	
	/* pointers goedzetten */
	tbuf.rect		= ibuf2->rect;
	tbuf.planes		= ibuf2->planes;
	tbuf.chardata	= ibuf2->chardata;
	tbuf.cmap		= ibuf2->cmap;
	
	/* malloc flag goed zetten */
	tbuf.mall		= ibuf2->mall;
	
	*ibuf2 = tbuf;
	
	if (ibuf1->cmap){
		addcmapImBuf(ibuf2);
		if (ibuf2->cmap) memcpy(ibuf2->cmap,ibuf1->cmap,ibuf2->maxcol * sizeof(long));
	}

	return(ibuf2);
}

