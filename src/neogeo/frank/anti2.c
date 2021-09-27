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
	zipfork "cc -g anti2.c util.o -lgl_s -limbuf -limage -lm -o anti2 > /dev/console"                                                                  
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


ListBase * scanimage(struct ImBuf * ibuf, long dir)
{
	long step, pixels, lines, nextline, x, y, col1, col2;
	ulong * rect;
	ListBase * listarray, * curlist;
	Edge * edge;
	
	switch (dir) {
	case 'h':
		step = 1; nextline = ibuf->x;
		pixels = ibuf->x; lines = ibuf->y;
		break;
	case 'v':
		step = ibuf->x; nextline = 1;
		pixels = ibuf->y; lines = ibuf->x;
	}
	
	listarray = callocstruct(ListBase, lines);
	for (y = 0; y < lines; y++){
		rect = ibuf->rect;
		rect += y * nextline;
		curlist = listarray + y;
		
		col1 = rect[0];
		for (x = 0; x < pixels; x++) {
			col2 = rect[0];
			if (compare(col1, col2)) {
				edge = NEW(Edge);
				edge->position = x;
				edge->col1 = col1;
				edge->col2 = col2;
				addtail(curlist, edge);
				col1 = col2;
			}
			rect += step;
		}
	}
	
	return(listarray);
}

Edge * findmatch(Edge * first, Edge * edge)
{
	Edge * match = 0;
	long in = 0, out = 65535;
	long last = 0;
	
	if (edge->prev) in = edge->prev->position;
	if (edge->next) out = edge->next->position;
	
	while (first) {
		if (first->position < edge->position) {
			if (first->col1 == edge->col1) {
				if (first->position >= in) match = first;
			} else if (first->col2 == edge->col2) {
				if (first->next == 0) match = first;
				else if (first->next->position >= edge->position) match = first;
			} else if (first->col2 == edge->col1) {
				match = 0; /* bij zigzagjes kan deze al 'ns foutief gezet zijn */
			}
		} else if (first->position == edge->position) {
			if (first->col1 == edge->col1 || first->col2 == edge->col2) match = first;
		} else {
			if (match) break;	/* er is er al een */
			
			if (first->col1 == edge->col1) {
				if (first->prev == 0) match = first;
				else if (first->prev->position <= edge->position) match = first;
			} else if (first->col2 == edge->col2) {
				if (first->position <= out) match = first;
			}
		}
		
		first = first->next;
	}
	
	return(match);
}


void filterdraw(ulong * ldest, ulong * lsrce, long zero, long half, long step)
{
	uchar * src, * dst;
	long count;
	double weight, add;
	
	/* we filteren de pixels op ldest tussen in en out met pixels van lsrce
	 * Het gewicht loopt ondertussen van 0 naar 1
	 */
	

	count = half - zero;
	if (count < 0) count = -count;
	if (count <= 1) return;
	
	if (zero < half) {
		src = (uchar *) (lsrce + (step * zero));
		dst = (uchar *) (ldest + (step * zero));
	} else {
		zero--;
		src = (uchar *) (lsrce + (step * zero));
		dst = (uchar *) (ldest + (step * zero));
		step = -step;
	}
	
	step = 4 * step;
	
	dst += step * (count >> 1);
	src += step * (count >> 1);
	
	count = (count + 1) >> 1;
	add = 0.5 / count;
	weight = 0.5 * add;
	
	/* dit moet natuurlijk gamma gecorrigeerd */
	
	for(; count > 0; count --) {
		dst[0] += weight * (src[0] - dst[0]);
		dst[1] += weight * (src[1] - dst[1]);
		dst[2] += weight * (src[2] - dst[2]);
		dst[3] += weight * (src[3] - dst[3]);
		dst += step;
		src += step;
		weight += add;
	}
	if (RCT) lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
}

void filterimage(struct ImBuf * ibuf, struct ImBuf * cbuf, ListBase * listarray, long dir)
{
	long step, pixels, lines, nextline, x, y, pos, drawboth;
	ulong * irect, * crect;
	Edge * left, * middle, * right, temp, * any;
	
	switch (dir) {
	case 'h':
		step = 1; nextline = ibuf->x;
		pixels = ibuf->x; lines = ibuf->y;
		break;
	case 'v':
		step = ibuf->x; nextline = 1;
		pixels = ibuf->y; lines = ibuf->x;
	}
	
	for (y = 1; y < lines - 1; y++){
		irect = ibuf->rect;
		irect += y * nextline;
		crect = cbuf->rect;
		crect += y * nextline;
		
		middle = listarray[y].first;
		while (middle) {
			left = findmatch(listarray[y - 1].first, middle);
			right = findmatch(listarray[y + 1].first, middle);
			drawboth = FALSE;
			
			if (left == 0 || right == 0) {
				/* rand */
				any = left;
				if (right) any = right;
				if (any) {
					/* spiegelen */
					pos = 2 * middle->position - any->position;

					if (any->position < middle->position) {
						if (pos > pixels - 1) pos = pixels - 1;
						if (middle->next) {
							if (pos > middle->next->position) pos = middle->next->position;
						}
/*						if (any->next) {
							if (pos > any->next->position) pos = any->next->position;
						}
*/					} else {
						if (pos < 0) pos = 0;
						if (middle->prev) {
							if (pos < middle->prev->position) pos = middle->prev->position;
						}
/*						if (any->prev) {
							if (pos < any->prev->position) pos = any->prev->position;
						}
*/					}
					temp.position = pos;
					if (left) right = &temp;
					else left = &temp;
					drawboth = TRUE;
				}
			} else if (left->position == middle->position || right->position == middle->position) {
				/* recht stuk */
				/* klein hoekje, met een van de twee op afstand 2 (ander is toch op afstand 0) ? */
				
				if (abs(left->position - right->position) == 2) drawboth = TRUE;
			} else if (left->position < middle->position && right->position > middle->position){
				/* trap 1 */
				drawboth = TRUE;
			} else if (left->position > middle->position && right->position < middle->position){
				/* trap 2 */
				drawboth = TRUE;
			} else {
				/* piek */
				drawboth = TRUE;
			}
			
			if (drawboth) {
				filterdraw(irect, crect - nextline, left->position, middle->position, step);
				filterdraw(irect, crect + nextline, right->position, middle->position, step);
			}

			middle = middle->next;
		}
	}
}

main(long argc, char ** argv)
{
	struct ImBuf * cbuf;
	ListBase * listarray1, * listarray2, * curlist;
	short val;
	double zoom = ZM;
	
	if (argc == 1) exit(0);
	
	ibuf = loadiffname(argv[1], IB_rect);
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
	
	cbuf = dupImBuf(ibuf);
	
	listarray1 = scanimage(ibuf, 'h');
	listarray2 = scanimage(ibuf, 'v');
	
	filterimage(ibuf, cbuf, listarray1, 'h');
	if (argc == 2) lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
	filterimage(ibuf, cbuf, listarray2, 'v');
	if (argc == 2) {
		do {
			lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
		} while(qread(&val));
	} else {
		saveiff(ibuf, argv[2], IB_rect);
	}
}

