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

#include <local/iff.h>

#include <local/util.h>
#include <fcntl.h>
#include <gl/gl.h>
#include <gl/device.h>


main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf * ibuf;
	long i, j, dev;
	short val;
	
	if (argc == 1) {
		printf("%s: cmap_image\n", argv[0]);
		exit(1);
	}
	
	ibuf = loadiffname(argv[1], IB_rect | IB_cmap);
	if (ibuf == 0) {
		perror(0);
		exit(0);
	}
	
	if (ibuf->cmap == 0) printf("Image has no cmap\n");
	
	prefsize(ibuf->x, ibuf->y);
	winopen(argv[1]);
	
	if (ibuf->cmap == 0) RGBmode();
	gconfig();
	
	qdevice(ESCKEY);
	qdevice(INPUTCHANGE);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	
	if (ibuf->maxcol < 4096) for (i = (ibuf->x * ibuf->y) - 1; i >=0; i--) ibuf->rect[i] += 512;
	
	while (1) {
		if (ibuf->cmap) {
			if (ibuf->maxcol < 4096) {
				for (i = 0; i < ibuf->maxcol; i++){
					j = ibuf->cmap[i];
					mapcolor(512 + i, j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff);
				}
			} else {
				for (i = ibuf->mincol; i < ibuf->maxcol; i++){
					j = ibuf->cmap[i];
					mapcolor(i, j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff);
				}
			}
		}
		lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
		dev = qread(&val);
		if (dev == ESCKEY && val != 0) break;
		if (dev == LEFTMOUSE) {
			cmode();
			gconfig();
		}
		if (dev == MIDDLEMOUSE) {
			RGBmode();
			gconfig();
		}
	}
}

