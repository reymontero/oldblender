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

/* werking:

1 - zoek een overgang in een kolom
2 - kijk wat de relatie met links en rechts is, 
	
	Is pixel boven overgang links of rechts ervan gelijk aan bovenste kleur, 
	zoek dan naar beneden.
	
	Is pixel onder overgang links of rechts ervan gelijk aan onderste kleur, 
	zoek dan naar boven.
	
	
*/

/* er moet een functie * komen die aan kan geven of twee kleuren nu
 * wel of niet gelijk zijn.
 * Voor nu maar een define
 */


/*
	zipfork "cc -O scale.c util.o -lgl_s -limbuf -limage -lm -o scale "  > /dev/console                                                                
	zipfork "anti /data/rt > /dev/console"                                                                  
	zipfork "anti /pics/martin/03.01.ChambFinal/0001 > /dev/console"                                                                  
*/

#define nDEBUG

#ifdef DEBUG
#define ZM 4.0
#define RCT TRUE
#else
#define ZM 1.0
#define RCT FALSE
#endif

struct ImBuf * ibuf;

#define compare(x, y) (x != y)

typedef struct Edge
{
	struct Edge * next, * prev;
	short position;
	long col1, col2;
}Edge;



main(long argc, char ** argv)
{
	short val;
	long i;
	double zoom = ZM;
	
	if (argc == 1) exit(0);
	
	ibuf = loadiffname(argv[1], IB_rect | IB_cmap);
	if (ibuf == 0) exit(0);
	
	if (argc == 2) {
		foreground();
		prefsize(zoom * ibuf->x, zoom * ibuf->y);
		winopen("anti");
		RGBmode();
		gconfig();
		rectzoom(zoom, zoom);
		lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
	}
	
	clever_double (ibuf);
		
	if (argc == 2) {
		do {
			lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
		} while(qread(&val));
	} else {
		if (argc == 4) {
			freecmapImBuf(ibuf);
			ibuf->ftype = TGA;
			if (ibuf->depth != 24 && ibuf->depth != 8) ibuf->depth = 32;
			if (strcmp(argv[3], "24") == 0) ibuf->depth = 24;
		}
		saveiff(ibuf, argv[2], IB_rect | IB_cmap);
	}
}


main_rt(long argc, char ** argv)
{	
	short val;
	
	if (argc == 1) exit(0);
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) exit(0);
	
	antialias(ibuf);
		
	if (argc == 2) {
		do {
			lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
		} while(qread(&val));
	} else {
		saveiff(ibuf, argv[2], IB_rect);
	}
}

