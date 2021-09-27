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

 *  SoftFocus sequence-plugin by Enji / TNCCI Inc. mrt 1997
 *
set PLUG=softfocus ; cc -O2 -float -mips2 -c $PLUG.c ; ld -shared $PLUG.o -o $PLUG.so ; cp $PLUG.so /pics/blender/plugin/seq/
 *
 */

#include <sys/types.h>
#include "/usr/people/blend/plugin.h"
#include "/usr/people/include/util.h"
#include "/usr/people/frank/source/imbuf/imbuf.h"
#include <math.h>
#include <stdio.h>
#include <local/iff.h>

#define OBJECTBLOK "filter"

float cfra;
char seqname[24];
extern void rectcpy();

/* ******************************************************************* */
	char _name[24]= "Soft Focus";			/* 1. naam: */
	#define NR_STYPES	1					/* 2. nr sub types */
	char _stnames[NR_STYPES][16]= {"--"};	/* 3. namen subtypes (max. 16 bytes per naam) */

	VarStruct _varstr[]= {					/* 4. informatie over externe variabelen */
	 /* butcode,	naam,       default,min, max */
		NUMSLI|FLO,	"gam",    1.0,  0.0, 10.0, 
	};

	typedef struct Cast {					/* 5. hulpstruct om variabelen te casten */
		float gamma;
	} Cast;

	void plugin_seq_getinfo( int *stypes, int *vars)
	{
		*stypes= NR_STYPES;
		*vars= sizeof(_varstr) / sizeof(VarStruct);
	}
/* ******************************************************************* */

void plugin_seq_init()
{
}


void blurbuf(struct ImBuf *ibuf, int nr, Cast *cast)
{
	/* nr = aantal keer verkleinen en vergroten */
	/* vervangt de rect van ibuf */
	struct ImBuf *tbuf, *ttbuf;
	int i, x4;
	
	tbuf= dupImBuf(ibuf);
	x4= ibuf->x/4;
	
	if(cast->gamma != 1.0) gamwarp(tbuf, cast->gamma);

	/* verkleinen */
	for(i=0; i<nr; i++) {
		ttbuf = onehalf(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		if(tbuf->x<4 || tbuf->y<4) break;
	}
	
	/*filter kleine plaatje */
/*	for(i=1; i<nr; i++) {
		filter(tbuf);
	}
*/
	/* vergroten */
	for(i=0; i<nr; i++) {
		ttbuf = double_x(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		ttbuf = double_y(tbuf);
		if (ttbuf) {
			freeImBuf(tbuf);
			tbuf = ttbuf;
		}
		
		if(tbuf->x > x4) {
			scaleImBuf(tbuf, ibuf->x, ibuf->y);
			break;
		}
	}
	
	if(cast->gamma != 1.0) gamwarp(tbuf, 1.0 / cast->gamma);

	freeN(ibuf->rect);
	ibuf->rect= tbuf->rect;
	freeN(tbuf);
}

void doblur(struct ImBuf *mbuf, float fac, Cast *cast)
{
	/* maak mipmap structuur aan
	 * fac is aantal pixels
	 */
	struct ImBuf *ibuf, *pbuf;
	float ifac, pfac;
	int n, b1, b2;
	char *irect, *prect, *mrect;
	
	/* welke twee buffers zijn nodig? */
				
	if(fac>7.0) fac= 7.0;
	if(fac<=1.0) return;
	
	pfac= 2.0;
	pbuf= dupImBuf(mbuf);
	n= 1;
	while(pfac < fac) {
		blurbuf(pbuf, n, cast);
		blurbuf(pbuf, n, cast);
		
		n++;
		pfac+= 1.0;
	}

	ifac= pfac;
	pfac-= 1.0;
	ibuf= dupImBuf(pbuf);
	blurbuf(ibuf, n, cast);
	blurbuf(ibuf, n, cast);
	
	fac= 255.0*(fac-pfac)/(ifac-pfac);
	b1= fac;
	if(b1>255) b1= 255;
	b2= 255-b1;
	
	if(b1==255) {
		rectop(mbuf, ibuf, 0, 0, 0, 0, 32767, 32767, rectcpy);
		freeN(ibuf);
		freeImBuf(pbuf);
	}
	else if(b1==0) {
		rectop(mbuf, pbuf, 0, 0, 0, 0, 32767, 32767, rectcpy);
		freeN(pbuf);
		freeImBuf(ibuf);
	}
	else {	/* interpoleren */
		n= ibuf->x*ibuf->y;
		irect= (char *)ibuf->rect;
		prect= (char *)pbuf->rect;
		mrect= (char *)mbuf->rect;
		while(n--) {
			mrect[0]= (irect[0]*b1+ prect[0]*b2)>>8;
			mrect[1]= (irect[1]*b1+ prect[1]*b2)>>8;
			mrect[2]= (irect[2]*b1+ prect[2]*b2)>>8;
			mrect[3]= (irect[3]*b1+ prect[3]*b2)>>8;
			mrect+= 4;
			irect+= 4;
			prect+= 4;
		}
		freeImBuf(ibuf);
		freeImBuf(pbuf);
	}
}


void plugin_seq_doit(Cast *cast,
					float facf0, float facf1,
					int x, int y,
					ImBuf *ibuf1, ImBuf *ibuf2,
					ImBuf *out, ImBuf *use)
{
	float  bfacf0, bfacf1;
	
	bfacf0 = (facf0 * 6.0) + 1.0;
	bfacf1 = (facf1 * 6.0) + 1.0;

	rectop(out, ibuf1, 0, 0, 0, 0, 32767, 32767, rectcpy);

	de_interlace(out);
	
	doblur(out, bfacf0, cast); /*fieldA*/

	out->rect += out->x * out->y;

	doblur(out, bfacf1, cast); /*fieldB*/

	out->rect -= out->x * out->y;

	interlace(out);
}


