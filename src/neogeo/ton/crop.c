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

#include <sys/types.h>

#include <local/iff.h>
#include <local/storage.h>
#include <local/util.h>
/* #include <gl/gl.h> */
/* #include <gl/device.h> */
#include <string.h>
#include <stdio.h> 
#include <fcntl.h>

/*
cc -O2 crop.c ../trace/util.o -limbuf -limage -lm -o crop

*/

main(argc,argv)
int argc;
char **argv;
{
	extern void rectcpy();
	long win = 0, new, first;
	struct ImBuf *ibuf, *nbuf;
	ulong *rect;
	long newx=0, newy=0;	
	long a, i, srcx=0, srcy=0, destx=0, desty=0;
	char *inimage=0, *outimage=0;
	
	char fb[2] = {'a', 'a'};
	
	if(argc==2) {
		ibuf = loadiffname(argv[1], IB_test);
		if (ibuf == 0) {
			printf("can't load image\n");
			exit(0);
		}
		printf("size: x=%d y=%d\n", ibuf->x, ibuf->y);
		if (ibuf->cmap) {
			printf("cmap: min=%d max=%d\n", ibuf->mincol, ibuf->maxcol);
		}
		exit(0);
	}
	else if(argc==4 || argc==5) {
		newx= atoi(argv[1]);
		newy= atoi(argv[2]);
		inimage= argv[3];
		if(argc==4) outimage= inimage;
		else outimage= argv[4];
	}
	else {
		printf("usage: crop [+-]newx [+-]newy inimage [outimage]\n");
		printf("or: crop image : gives imagesize\n");
		exit(0);
	}

	if(newx==0 || newy==0) {
		printf("can't crop\n");
		exit(0);
	}
	
	ibuf = loadiffname(inimage, IB_rect | IB_cmap);
	if (ibuf == 0) {
		printf("can't load image\n");
		exit(0);
	}

	if (argv[1][0] == '-' || argv[1][0] == '+') newx += ibuf->x;
	if (argv[2][0] == '-' || argv[2][0] == '+') newy += ibuf->y;
	
	nbuf= dupImBuf(ibuf);
	freerectImBuf(nbuf);
	nbuf->x= newx;
	nbuf->y= newy;
	nbuf->rect= mallocN(4*newx*newy, "newrect");
	
	if(newx*newy > ibuf->x*ibuf->y) {
		rect = nbuf->rect;
		for(a= newx*newy; a>0; a--) {
			*rect++= 0;
		}
	}
	
	if(newx<ibuf->x) srcx= (ibuf->x-newx)/2;
	else destx= (newx-ibuf->x)/2;

	if(newy<ibuf->y) srcy= (ibuf->y-newy)/2;
	else desty= (newy-ibuf->y)/2;
		

	rectop(nbuf, ibuf, destx, desty, srcx, srcy, newx, newy, rectcpy);
	
	if (saveiff(nbuf, outimage, IB_rect | IB_cmap) == 0) exit(0);

	printf("Saved: %s\n", outimage);
}

