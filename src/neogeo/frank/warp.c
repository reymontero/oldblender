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

#include <stdio.h>

#include <local/iff.h>
#include <gl/image.h>
#include <gl/device.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <local/Button.h>
#include <fmclient.h>
#include <local/gl_util.h>
#include <math.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>

/*

zipfork "cc -O -o warp warp.c -limbuf -limage -lm util.o > /dev/console"                                                                  
zipfork "chromakey /pics/ahsgrab/0251 /pics/ahsgrab/back_mask_start > /dev/console"                                                                  

*/

main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf *bbuf, *abuf, *fbuf;
	char back[512], alpha[512], front[512], out[512];
	long size;
	uchar *frect, *arect;
	extern rectalphaover();
	
	if (argc < 5) {
		printf("%s: back- alpha- front- out-image\n", argv[0]);
		exit(0);
	}
	
	strcpy(back, argv[1]);
	strcpy(alpha, argv[2]);
	strcpy(front, argv[3]);
	strcpy(out, argv[4]);
	
	while (1) {
		bbuf = loadiffname(back, IB_rect);
		if (bbuf == 0) break;
		abuf = loadiffname(alpha, IB_rect);
		if (abuf == 0) break;
		fbuf = loadiffname(front, IB_rect);
		if (fbuf == 0) break;
		
		size = abuf->x * abuf->y;
		if (size != fbuf->x * fbuf->y) {
			printf("sizes don't match\n");
			exit(1);
		}
		
		arect = (uchar *) abuf->rect;
		frect = (uchar *) fbuf->rect;
		
		if (abuf->depth == 32) {
			
		} else {
			if (abuf->depth == 8) {
				for (;size > 0; size--){
					frect[0] = arect[3];
					frect += 4; arect += 4;
				}
			} else {
				
			}
		}
		
		rectoptot(bbuf, fbuf, rectalphaover);
		
		if (saveiff(bbuf, out, IB_rect) == 0) {
			perror(out);
			exit(1);
		}
		
		printf("saved %s\n", out);
		
		freeImBuf(bbuf);
		freeImBuf(abuf);
		freeImBuf(fbuf);
		
		newname(back, +1);
		newname(alpha, +1);
		newname(front, +1);
		newname(out, +1);		
	}
}

