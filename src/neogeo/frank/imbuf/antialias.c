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


#define OBJECTBLOK "antialias"

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
	zipfork "cc -g anti.c util.o -lgl_s -limbuf -limage -lm -o anti > /dev/console"                                                                  
	zipfork "anti /data/rt > /dev/console"                                                                  
	zipfork "anti /pics/martin/03.01.ChambFinal/0001 > /dev/console"                                                                  
*/

uint anti_mask = 0xffffffff;
int anti_a, anti_b, anti_g, anti_r;

#define compare(x, y) ((x ^ y) & anti_mask)

typedef struct Edge
{
	struct Edge * next, * prev;
	short position;
	long col1, col2;
}Edge;

void anti_free_listarray(int count, ListBase * listarray)
{
	int i;
	
	if (listarray == 0) return;
	
	for (i = 0; i < count; i++) freelistN(listarray + i);
	freeN(listarray);	
}

ListBase * scanimage(struct ImBuf * ibuf, long dir)
{
	long step, pixels, lines, nextline, x, y, col1, col2;
	ulong * rect;
	ListBase * listarray, * curlist;
	Edge * edge;
	int count;
	
	switch (dir) {
	case 'h':
		step = 1; nextline = ibuf->x;
		pixels = ibuf->x; lines = ibuf->y;
		break;
	case 'v':
		step = ibuf->x; nextline = 1;
		pixels = ibuf->y; lines = ibuf->x;
	}
	
	listarray = callocstructN(ListBase, lines, "listarray");
	for (y = 0; y < lines; y++){
		rect = ibuf->rect;
		rect += y * nextline;
		curlist = listarray + y;
		
		col1 = rect[0];
		count = 0;
		
		for (x = 0; x < pixels; x++) {
			col2 = rect[0];
			if (compare(col1, col2)) {
				edge = NEW(Edge);
				edge->position = x;
				edge->col1 = col1;
				edge->col2 = col2;
				addtail(curlist, edge);
				col1 = col2;
				count++;
				if (count > 100) {
					printf("\n\n%s: Aborting antialias !\n", ibuf->name);
					printf("To many transitions.\nIs this a natural image ?\n\n"), 
					anti_free_listarray(lines, listarray);
					return(0);
				}
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
		if (anti_a) dst[0] += weight * (src[0] - dst[0]);
		if (anti_b) dst[1] += weight * (src[1] - dst[1]);
		if (anti_g) dst[2] += weight * (src[2] - dst[2]);
		if (anti_r) dst[3] += weight * (src[3] - dst[3]);
		dst += step;
		src += step;
		weight += add;
	}
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


void antialias(struct ImBuf * ibuf)
{
	struct ImBuf * cbuf;
	ListBase * listarray, * curlist;
	short val;
	long i;
	
	if (ibuf == 0) return;
	cbuf = dupImBuf(ibuf);
	
	anti_a = (anti_mask >> 24) & 0xff;
	anti_b = (anti_mask >> 16) & 0xff;
	anti_g = (anti_mask >>  8) & 0xff;
	anti_r = (anti_mask >>  0) & 0xff;
	
	listarray = scanimage(cbuf, 'h');
	if (listarray) {
		filterimage(ibuf, cbuf, listarray, 'h');
		anti_free_listarray(ibuf->y, listarray);
		
		listarray = scanimage(cbuf, 'v');
		if (listarray) {
			filterimage(ibuf, cbuf, listarray, 'v');
			anti_free_listarray(ibuf->x, listarray);
		}
	}
			
	freeImBuf(cbuf);
}


/* intelligente scaling */

void _intel_scale(struct ImBuf * ibuf, ListBase * listarray, long dir)
{
	long step, lines, nextline, x, y, pos, col;
	ulong * irect, * trect;
	long start, end;
	Edge * left, * right;
	struct ImBuf * tbuf;
	
	switch (dir) {
	case 'h':
		step = 1; nextline = ibuf->x;
		lines = ibuf->y;
		tbuf = double_fast_y(ibuf);
		break;
	case 'v':
		step = 2 * ibuf->x; nextline = 1;
		lines = ibuf->x;
		tbuf = double_fast_x(ibuf);
		break;
	default:
		return;
	}
	
	freerectImBuf(ibuf);
	ibuf->rect = tbuf->rect;
	ibuf->mall |= IB_rect;
	
	
	ibuf->x = tbuf->x;
	ibuf->y = tbuf->y;
	tbuf->rect = 0;
	freeImBuf(tbuf);
	
	for (y = 0; y < lines - 2; y++){
		irect = ibuf->rect;
		irect += ((2 * y) + 1) * nextline;
		
		left = listarray[y].first;
		while (left) {
			right = findmatch(listarray[y + 1].first, left);
			if (right) {
				if (left->col2 == right->col2) {
					if (left->next && right->next) {
						if (left->next->position >= right->position) {
							start = ((left->position + right->position) >> 1);
							end = ((left->next->position + right->next->position) >> 1);
							col = left->col2;
							trect = irect + (start * step);
							for (x = start; x < end; x++) {
								*trect = col;
								trect += step;
							}
						}
					}
				}

				if (left->col1 == right->col1) {
					if (left->prev && right->prev) {
						if (left->prev->position <= right->position) {
							end = ((left->position + right->position) >> 1);
							start = ((left->prev->position + right->prev->position) >> 1);
							col = left->col1;
							trect = irect + (start * step);
							for (x = start; x < end; x++) {
								*trect = col;
								trect += step;
							}
						}
					}
				}

			}
			left = left->next;
		}
	}
}


void clever_double (struct ImBuf * ibuf)
{
	ListBase * listarray, * curlist;
	Edge * new;
	long size;
	long i;
	
	if (ibuf == 0) return;
	
	size = ibuf->x;
	listarray = scanimage(ibuf, 'v');
	if (listarray) {
		for (i = 0; i < size; i++) {
			curlist = listarray + i;
			new = CLN(Edge);
			new->col2 = ibuf->rect[i]; /* bovenste pixel */
			new->col1 = new->col2 - 1;
			addhead(curlist, new);
			new = CLN(Edge);
			new->position = ibuf->y - 1;
			new->col1 = ibuf->rect[i + ((ibuf->y -1) * ibuf->x)]; /* onderste pixel */
			new->col2 = new->col1 - 1;
			addtail(curlist, new);
		}
		_intel_scale(ibuf, listarray, 'v');
		anti_free_listarray(size, listarray);

		size = ibuf->y;
		listarray = scanimage(ibuf, 'h');
		if (listarray) {
			for (i = 0; i < size; i++) {
				curlist = listarray + i;
				new = CLN(Edge);
				new->col2 = ibuf->rect[i * ibuf->x]; /* linkse pixel */
				new->col1 = new->col2 - 1;
				addhead(curlist, new);
				new = CLN(Edge);
				new->position = ibuf->x - 1;
				new->col1 = ibuf->rect[((i + 1) * ibuf->x) - 1]; /* rechtse pixel */
				new->col2 = new->col1 - 1;
				addtail(curlist, new);
			}
			_intel_scale(ibuf, listarray, 'h');
			anti_free_listarray(size, listarray);
		}
	}
}
