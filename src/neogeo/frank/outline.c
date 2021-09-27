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
#include <gl/gl.h>
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

zipfork "cc -g -o outline outline.c util.o -limbuf -limage -lm> /dev/console"                                                                  
zipfork "chromakey /pics/ahsgrab/0251 /pics/ahsgrab/back_mask_start > /dev/console"                                                                  

*/



void noalpha(long argc, char **argv)
{
	char inname[256], outname[256];
	long i, alpha, col;
	struct ImBuf * ibuf;
	uchar * rect;
	short val;
	extern void rectcpy();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage \n", argv[0]);
		exit(1);
	}
	
	strcpy(inname, argv[1]);
	strcpy(outname, argv[2]);
	
	do {
		ibuf = loadiffname(inname, IB_rect);
		if (ibuf == 0) break;
		
		printf("Loaded %s\n", inname);
		rect = (uchar * ) ibuf->rect;
		
		for (i = ibuf->x * ibuf->y; i > 0; i--){
			alpha = rect[0];
			if (alpha > 128) {
				col =  (rect[1] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[1] = col;

				col =  (rect[2] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[2] = col;

				col =  (rect[3] * 255 ) / alpha;
				if (col > 255) col = 255;
				rect[3] = col;
			} else {
				rect[0] = rect[1] = rect[2] = rect[3] = 0;
			}
			rect += 4;
		}
		
		if (ibuf->cmap) {
			freecmapImBuf(ibuf);
			ibuf->depth = 32;
		}

		if (saveiff(ibuf, outname, IB_rect) == 0){
			perror(outname);
			exit(4);
		}
		
		freeImBuf(ibuf);
		printf("Saved %s\n", outname);
		
		newname(inname, +1);
		newname(outname, +1);
	} while (ibuf);
}

void outline(long argc, char **argv)
{
	struct ImBuf * ibuf, * obuf, * wbuf;
	uchar * rect;
	long i, oln = 2;
	char out[128];
	extern void rectpremulalphaover();
	
	if (argc == 1) {
		printf("usage: %s inimage [outimage]\n", argv[0]);
		exit(0);
	}
	
	if (argc == 2) sprintf(out, "%s.oln", argv[1]);
	else strcpy(out, argv[2]);
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) exit(1);
	
	wbuf = dupImBuf(ibuf);
	
	rect = (uchar *) wbuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--){
		if (rect[0] > 0x20) rect[1] = rect[2] = rect[3] = rect[0];
		else rect[1] = rect[2] = rect[3] = rect[0] = 0;
		rect += 4;
	}
	obuf = dupImBuf(wbuf);
	
	/* eerst -3 +3 op & neer */
	
	for (i = -oln; i <= oln; i++) {
		if (i != 0) rectop(obuf, wbuf, 0, 0, 0, i, 32767, 32767, rectpremulalphaover, 0);
	}

	freeImBuf(wbuf);
	wbuf = dupImBuf(obuf);
	
	for (i = -oln; i <= oln; i++) {
		if (i != 0) rectop(obuf, wbuf, 0, 0, i, 0, 32767, 32767, rectpremulalphaover, 0);
	}
	
	rectoptot(obuf, ibuf, rectpremulalphaover, 0);
	
	saveiff(obuf, out, IB_rect);
	
}


void add_drop(struct ImBuf * ibuf)
{
	struct ImBuf * obuf, * wbuf;
	uchar * rect;
	long i, oln = 4;
	char out[128];
	extern void rectpremulalphaover();
	extern void rectpremulalphaunder();
		
	wbuf = dupImBuf(ibuf);
	
	rect = (uchar *) wbuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--){
		rect[0] >>= 5;
		rect[1] = rect[2] = rect[3] = 0;
		rect += 4;
	}
	
	obuf = dupImBuf(wbuf);
	for (i = oln; i <= 2 * oln; i++) {
		rectop(obuf, wbuf, 0, 0, 0, i, 32767, 32767, rectpremulalphaover, 0);
	}
	freeImBuf(wbuf);

	wbuf = dupImBuf(obuf);
	for (i = oln; i <= 2 * oln; i++) {
		rectop(obuf, wbuf, i, 0, 0, 0, 32767, 32767, rectpremulalphaover, 0);
	}
	freeImBuf(wbuf);
	
	rectoptot(ibuf, obuf, rectpremulalphaunder, 0);
	freeImBuf(obuf);
}


void drop(long argc, char **argv)
{
	struct ImBuf * ibuf;
	char out[256];
	
	if (argc == 1) {
		printf("usage: %s inimage [outimage]\n", argv[0]);
		exit(0);
	}
	
	if (argc == 2) sprintf(out, "%s.drp", argv[1]);
	else strcpy(out, argv[2]);
	
	ibuf = loadiffname(argv[1], IB_rect);
	if (ibuf == 0) exit(1);
	
	add_drop(ibuf);
	
	saveiff(ibuf, out, IB_rect);
}

void col_to_alpha(struct ImBuf *ibuf, long bot, long top)
{
	char alpha[256], *rect;
	long i, val;
	
	for (i = 0 ; i < 256; i++ ) alpha[i] = 255;
	for (i = 128 - bot; i <= 128 + bot; i++) alpha[i] = 0;
	if (top == bot) top++;
	for (i = bot; i <= top; i++) {
		val = (255 * (i - bot)) / (top - bot);
		alpha[128 - i] = val;
		alpha[128 + i] = val;
	}

	rect = (char *) ibuf->rect;
	for (i = 4 * ibuf->x * ibuf->y; i > 0; i--) *rect++ = alpha[*rect];

	rect = (char *) ibuf->rect;
	for (i = ibuf->x * ibuf->y; i > 0; i--) {
		if (rect[1] > rect[0]) rect[0] = rect[1];
		if (rect[2] > rect[0]) rect[0] = rect[2];
		if (rect[3] > rect[0]) rect[0] = rect[3];
		rect += 4;
	}
}

void noise_proces(struct ImBuf ** ibuf, long bot, long top)
{
	struct ImBuf * tbuf1, * tbuf2, * tbuf3;
	extern void rectsub2();
	extern void rectblend();
	extern void rector();
	float value = 0.5;
	uchar * rect1, * rect2, * alpha, a;
	long i;
	
	/* er zijn 2 criteria
	 * 
	 * verschil tussen plaatje 2 & 3 mag niet groter zijn dan
	 * de 2 * limit
	 *   EN
	 * het gemiddelde tussen 1 & 2 min het gem tussen 3 & 4 mag
	 * niet groter zijn dan de limit.
	 * 
	 */
		
	tbuf1 = dupImBuf(ibuf[1]);
	rectoptot(tbuf1, ibuf[2], rectsub2);
	col_to_alpha(tbuf1, 2 * bot, 2 * top);
	
/*	tbuf2 = dupImBuf(ibuf[0]);
	rectoptot(tbuf2, ibuf[1], rectblend, *((long *) &value));

	tbuf3 = dupImBuf(ibuf[2]);
	rectoptot(tbuf3, ibuf[3], rectblend, *((long *) &value));

	rectoptot(tbuf2, tbuf3, rectsub2);
	col_to_alpha(tbuf2, limit);
	
	rectoptot(tbuf1, tbuf2, rector);
	
	freeImBuf(tbuf2);
	freeImBuf(tbuf3);
*/	
	tbuf2 = dupImBuf(ibuf[1]);

	rect1 = (uchar *) tbuf2->rect;
	rect2 = (uchar *) ibuf[2]->rect;
	alpha = (uchar *) tbuf1->rect;
	
	for (i = tbuf1->x * tbuf1->y; i > 0; i--) {
		a = *alpha;
		
		if (a) {
			if (a == 255) {
				*((ulong *) rect1) = *((ulong *) rect2);
			} else {
				a++;
				rect1[0] += (a * (rect2[0] - rect1[0])) >> 8;
				rect1[1] += (a * (rect2[1] - rect1[1])) >> 8;
				rect1[2] += (a * (rect2[2] - rect1[2])) >> 8;
				rect1[3] += (a * (rect2[3] - rect1[3])) >> 8;
			}
		}
		alpha += 4; rect1 += 4; rect2 += 4;
	}
	
	freeImBuf(tbuf1);
	freeImBuf(ibuf[2]);
	ibuf[2] = tbuf2;
}

void rmnoise(long argc, char **argv)
{
	char in[256], out[256];
	long bot = 2, top = 4, tga = FALSE;
	struct ImBuf * ibuf[4];
	short last = FALSE;
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [bot [top]]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	if (argc > 3) {
		bot = atoi(argv[3]);
		printf("using bot %d\n", bot);
	}

	if (argc > 4) {
		top = atoi(argv[4]);
		printf("using top %d\n", top);
	}
	
	if (argc > 5) tga = TRUE;
	
	ibuf[0] = loadiffname(in, IB_rect);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect);
	if (ibuf[2] == 0) exit(0);
	
	if (saveiff(ibuf[1], out, IB_rect) == 0){
		perror(out);
		exit(3);
	}
	printf("saved %s\n", out);
	newname(out, +1);
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		noise_proces(ibuf, bot, top);
		
		if (tga) ibuf[2]->ftype = TGA;
		
		if (saveiff(ibuf[2], out, IB_rect) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


struct ImBuf * delta_proces(struct ImBuf *rgb1, struct ImBuf *rgb2, struct ImBuf *cmap1, struct ImBuf *cmap2, long val)
{
	struct ImBuf * tbuf1, * tbuf2;
	extern void rectsub2();
	extern void rectblend();
	extern void rector();
	float value = 0.5;
	ulong *rect1, *rect2, *alpha;
	long i;
			
	tbuf1 = dupImBuf(rgb1);
	rectoptot(tbuf1, rgb2, rectsub2);
	col_to_alpha(tbuf1, val, val);
	
	tbuf2 = dupImBuf(cmap1);

	rect1 = tbuf2->rect;
	rect2 = cmap2->rect;
	alpha = tbuf1->rect;
	
	for (i = tbuf1->x * tbuf1->y; i > 0; i--) {
		if (*alpha) *rect1 = *rect2;
		alpha++; rect1++; rect2++;
	}
	
	freeImBuf(tbuf1);
	return(tbuf2);
}


void maxdlta(long argc, char **argv)
{
	char in[256], out[256];
	long val, maxdelta = 35000, i, x, y, t, offset, scale, save = TRUE;
	double dx, dy, len, dia;
	struct ImBuf * ibuf[4], * tbuf, *rbuf;
	ulong * rect1, * rect2, * rrect;
	short last = FALSE;
	extern void rectcpy();
	extern void rectxor();
	extern long file_size();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [maxdelta]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);
	if (out[0] == '-') save = FALSE;
	
	if (argc > 3) {
		maxdelta = atoi(argv[3]);
		printf("using maxdelta %d\n", maxdelta);
	}

	ibuf[0] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[2] == 0) exit(0);
	
	if (save) {
		ibuf[1]->mincol = 0;	/* patch */
		if (saveiff(ibuf[1], out, IB_rect | IB_cmap) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
	}
	
	rbuf = dupImBuf(ibuf[0]);
	
	rrect = rbuf->rect;
	dia = sqrt((rbuf->x * rbuf->x) + (rbuf->y * rbuf->y));
	
	for (y = 0; y < rbuf->y; y++) {
		dy = (rbuf->y / 2.0) - y;
		dy /= dia;
		for (x = 0; x < rbuf->x; x++) {
			dx = (rbuf->x / 2.0) - x;
			dx /= dia;
			
			len = 1.0 - sqrt(4.0 * ((dx * dx) + (dy * dy)));
			/* len loopt van 0.0 aan de rand tot aan 1.0 in het midden */
			
			len = 0.5 * len * (drand48() + 1.0);
			*rrect++ = 256.0 * len;
		}
	}
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect | IB_cmap);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		tbuf = dupImBuf(ibuf[2]);
		rectoptot(tbuf, ibuf[1], rectxor);
		saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
		freeImBuf(tbuf);
		
		printf(" %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
		
		if (file_size("/tmp/rt") > maxdelta) {
			/* - bereken een masker
			 * - bereken nieuw plaatje
			 * - schrijf xor weg
			 * - kleiner -> ga door
			 */
			
			
			tbuf = dupImBuf(ibuf[2]);
			val = 1;
			
			while (file_size("/tmp/rt") > maxdelta) {
				rrect = rbuf->rect;
				rect1 = tbuf->rect;
				rect2 = ibuf[1]->rect;
				
				for (y = 0; y < tbuf->y; y++) {
					for (x = 0; x < tbuf->x; x++) {
						if (*rrect < val) {
							rect1[0] = rect2[0];
							if (y || x > 1) {
								rect1[-1] = rect2[-1];
								rect1[-2] = rect2[-2];
							}
						}
						rrect++; rect1++; rect2++;
					}
				}

				rectoptot(tbuf, ibuf[1], rectxor);
				saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
				rectoptot(tbuf, ibuf[1], rectxor);

				printf("  %s: %6d %d\n", ibuf[2]->name, file_size("/tmp/rt"), val);
				if (file_size("/tmp/rt") <= maxdelta) {
					freeImBuf(ibuf[2]);
					ibuf[2] = tbuf;
				}
				val++;
			}
		}
		
		if (save) {
			ibuf[2]->mincol = 0;	/* patch */
			if (saveiff(ibuf[2], out, IB_rect | IB_cmap) == 0){
				perror(out);
				exit(3);
			}
			printf("saved %s\n", out);
			newname(out, +1);
		}
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


void maxdlta_hyper(long argc, char **argv)
{
	char in[256], out[256];
	long val, maxdelta = 35000, i, x, y, t, offset, scale;
	double dx, dy, len;
	struct ImBuf * ibuf[4], *tbuf;
	short last = FALSE;
	extern void rectcpy();
	extern void rectxor();
	extern long file_size();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [maxdelta]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	if (argc > 3) {
		maxdelta = atoi(argv[3]);
		printf("using maxdelta %d\n", maxdelta);
	}

	ibuf[0] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[2] == 0) exit(0);
	
	if (saveiff(ibuf[1], out, IB_rect | IB_cmap) == 0){
		perror(out);
		exit(3);
	}
	printf("saved %s\n", out);
	newname(out, +1);
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect | IB_cmap);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		tbuf = dupImBuf(ibuf[2]);
		rectoptot(tbuf, ibuf[1], rectxor);
		saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
		freeImBuf(tbuf);
		
		printf(" %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
		
		if (file_size("/tmp/rt") > maxdelta) {
			/* - bereken een masker
			 * - bereken nieuw plaatje
			 * - schrijf xor weg
			 * - kleiner -> ga door
			 */
			
			
			tbuf = dupImBuf(ibuf[2]);
			val = 2;

			while (file_size("/tmp/rt") > maxdelta) {
				offset = tbuf->y - val;
				scale = tbuf->x - offset;
				
				for (i = 1000; i > 0; i--) {
					dx = drand48() - 0.5;
					dy = drand48() - 0.5;
					len = sqrt((dx * dx) + (dy * dy));
					
					dx /= len;
					dy /= len;
					len = drand48();
					
					dx *= len;
					dy *= len;
					
					if (dx < 0) {
						x = (tbuf->x - offset + (dx * scale)) / 2.0;
					} else {
						x = (tbuf->x + offset + (dx * scale)) / 2.0;
					}
					
					if (dy < 0) {
						y = (tbuf->y - offset + (dy * scale)) / 2.0;
					} else {
						y = (tbuf->y + offset + (dy * scale)) / 2.0;
					}
					
					if ((x >= 0) && (y >= 0) && (x < tbuf->x) && (y < tbuf->y)) {
						x += (y * tbuf->x);
						tbuf->rect[x] = ibuf[1]->rect[x];
						if (y || x > 1) {
							tbuf->rect[x - 1] = ibuf[1]->rect[x - 1];
							tbuf->rect[x - 2] = ibuf[1]->rect[x - 2];
						}
					}
				}

				rectoptot(tbuf, ibuf[1], rectxor);
				saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
				rectoptot(tbuf, ibuf[1], rectxor);

				printf("  %s: %6d %d\n", ibuf[2]->name, file_size("/tmp/rt"), val);
				if (file_size("/tmp/rt") <= maxdelta) {
					freeImBuf(ibuf[2]);
					ibuf[2] = tbuf;
				}
				val += 4;
				if (val > tbuf->y) val = tbuf->y;
			}
		}

		if (saveiff(ibuf[2], out, IB_rect | IB_cmap) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


void maxdlta_square(long argc, char **argv)
{
	char in[256], out[256];
	long val, maxdelta = 35000, i, x, y;
	struct ImBuf * ibuf[4], *tbuf;
	short last = FALSE;
	extern void rectcpy();
	extern void rectxor();
	extern long file_size();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [maxdelta]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	if (argc > 3) {
		maxdelta = atoi(argv[3]);
		printf("using maxdelta %d\n", maxdelta);
	}

	ibuf[0] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[2] == 0) exit(0);
	
	if (saveiff(ibuf[1], out, IB_rect | IB_cmap) == 0){
		perror(out);
		exit(3);
	}
	printf("saved %s\n", out);
	newname(out, +1);
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect | IB_cmap);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		tbuf = dupImBuf(ibuf[2]);
		rectoptot(tbuf, ibuf[1], rectxor);
		saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
		freeImBuf(tbuf);
		
		printf(" %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
		
		if (file_size("/tmp/rt") > maxdelta) {
			/* - bereken een masker
			 * - bereken nieuw plaatje
			 * - schrijf xor weg
			 * - kleiner -> ga door
			 */
			
			
			tbuf = dupImBuf(ibuf[2]);
			val = 2;

			while (file_size("/tmp/rt") > maxdelta) {
				
				rectop(tbuf, ibuf[1], 0, 0, 0, 0, val, 32767, rectcpy);
				rectop(tbuf, ibuf[1], 0, 0, 0, 0, 32767, val, rectcpy);
				rectop(tbuf, ibuf[1], tbuf->x - val, 0, tbuf->x - val, 0, 32767, 32767, rectcpy);
				rectop(tbuf, ibuf[1], 0, tbuf->y - val, 0, tbuf->y - val, 32767, 32767, rectcpy);
			
				
				rectoptot(tbuf, ibuf[1], rectxor);
				saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
				rectoptot(tbuf, ibuf[1], rectxor);

				printf("  %s: %6d %d\n", ibuf[2]->name, file_size("/tmp/rt"), val);
				if (file_size("/tmp/rt") <= maxdelta) {
					freeImBuf(ibuf[2]);
					ibuf[2] = tbuf;
				}
				val += 2;
			}
		}

		if (saveiff(ibuf[2], out, IB_rect | IB_cmap) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


void maxdlta_rand(long argc, char **argv)
{
	char in[256], out[256];
	long val, maxdelta = 35000, i, x, y;
	struct ImBuf * ibuf[4], *tbuf;
	short last = FALSE;
	extern void rectxor();
	extern long file_size();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [maxdelta]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	if (argc > 3) {
		maxdelta = atoi(argv[3]);
		printf("using maxdelta %d\n", maxdelta);
	}

	ibuf[0] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[2] == 0) exit(0);
	
	if (saveiff(ibuf[1], out, IB_rect | IB_cmap) == 0){
		perror(out);
		exit(3);
	}
	printf("saved %s\n", out);
	newname(out, +1);
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect | IB_cmap);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		tbuf = dupImBuf(ibuf[2]);
		rectoptot(tbuf, ibuf[1], rectxor);
		saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
		freeImBuf(tbuf);
		
		printf(" %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
		
		if (file_size("/tmp/rt") > maxdelta) {
			/* - bereken een masker
			 * - bereken nieuw plaatje
			 * - schrijf xor weg
			 * - kleiner -> ga door
			 */
			
			
			tbuf = dupImBuf(ibuf[2]);
			while (file_size("/tmp/rt") > maxdelta) {
				val = 1;
				
				for (i = (file_size("/tmp/rt") - maxdelta) + 10; i > 0; i--) {
					x = ((random() & 0xffff) * tbuf->x) >> 16;
					y = ((random() & 0xffff) * tbuf->y) >> 16;
					x += (y * tbuf->x);
					tbuf->rect[x] = ibuf[1]->rect[x];
					if (y) {
						tbuf->rect[x - 1] = ibuf[1]->rect[x - 1];
						tbuf->rect[x - 2] = ibuf[1]->rect[x - 2];
					}
				}

				rectoptot(tbuf, ibuf[1], rectxor);
				saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
				rectoptot(tbuf, ibuf[1], rectxor);

				printf("  %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
				if (file_size("/tmp/rt") <= maxdelta) {
					freeImBuf(ibuf[2]);
					ibuf[2] = tbuf;
				}
				val += 2;
			}
		}

		if (saveiff(ibuf[2], out, IB_rect | IB_cmap) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}

void maxdlta_oud(long argc, char **argv)
{
	char in[256], out[256];
	long val, maxdelta = 35000, i, x, y;
	struct ImBuf * ibuf[4], *tbuf, *rgb1, *rgb2;
	short last = FALSE;
	extern void rectxor();
	extern long file_size();
	
	if (argc < 3) {
		printf("usage: %s inimage outimage [maxdelta]\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	if (argc > 3) {
		maxdelta = atoi(argv[3]);
		printf("using maxdelta %d\n", maxdelta);
	}

	ibuf[0] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect | IB_cmap);
	if (ibuf[2] == 0) exit(0);
	
	if (saveiff(ibuf[1], out, IB_rect | IB_cmap) == 0){
		perror(out);
		exit(3);
	}
	printf("saved %s\n", out);
	newname(out, +1);
	
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect | IB_cmap);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		}
		
		tbuf = dupImBuf(ibuf[2]);
		rectoptot(tbuf, ibuf[1], rectxor);
		saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
		freeImBuf(tbuf);
		
		printf(" %s: %6d\n", ibuf[2]->name, file_size("/tmp/rt"));
		
		if (file_size("/tmp/rt") > maxdelta) {
			/* - bereken een masker
			 * - bereken nieuw plaatje
			 * - schrijf xor weg
			 * - kleiner -> ga door
			 */
			
			rgb1 = dupImBuf(ibuf[1]);
			rgb2 = dupImBuf(ibuf[2]);
			applycmap(rgb1);
			applycmap(rgb2);
			
			for (i = maxdelta / 4; i > 0; i--) {
				x = ((random() & 0xffff) * rgb1->x) >> 16;
				y = ((random() & 0xffff) * rgb1->y) >> 16;
				rgb1->rect[(y * rgb1->x) + x] = -1;
				rgb2->rect[(y * rgb2->x) + x] = 0;				
			}
			val = 100;
			
			while (file_size("/tmp/rt") > maxdelta) {
				tbuf = delta_proces(rgb1, rgb2, ibuf[1], ibuf[2], val);
				rectoptot(tbuf, ibuf[1], rectxor);
				saveiff(tbuf, "/tmp/rt", IB_rect | IB_cmap);
				
				printf("  %s: %6d %d\n", ibuf[2]->name, file_size("/tmp/rt"), val);

				if (file_size("/tmp/rt") < maxdelta) {
					rectoptot(tbuf, ibuf[1], rectxor);
					freeImBuf(ibuf[2]);
					ibuf[2] = tbuf;
				}else freeImBuf(tbuf);
				
				val += 2;
			}
			
			freeImBuf(rgb1);
			freeImBuf(rgb2);
		}

		if (saveiff(ibuf[2], out, IB_rect | IB_cmap) == 0){
			perror(out);
			exit(3);
		}
		printf("saved %s\n", out);
		newname(out, +1);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


void post_proces(struct ImBuf * ibuf, struct ImBuf * bbuf, char * name)
{
	struct ImBuf * fbuf, * obuf;
	extern void rectpremulalphaover();
	
	fbuf = dupImBuf(ibuf);
	obuf = dupImBuf(bbuf);

	add_drop(fbuf);
	gamwarp(fbuf, 1.2);
	
	rectop(obuf, fbuf, (bbuf->x - fbuf->x) / 2, (bbuf->y - fbuf->y) / 2, 0, 0, 32767, 32767, rectpremulalphaover, 0);
	
	if (saveiff(obuf, name, IB_rect) == 0){
		perror(name);
		exit(3);
	}
	
	printf("saved %s\n", name);
	newname(name, +1);
	
	freeImBuf(fbuf);
	freeImBuf(obuf);
}

void pre_proces(struct ImBuf * ibuf)
{
	scaleImBuf(ibuf, 286, 214);
}

void bitlink(long argc, char **argv)
{
	char in[256], out[256];
	long bot = 2, top = 4;
	struct ImBuf * ibuf[4], * bbuf;
	short last = FALSE;
	
	if (argc < 4) {
		printf("usage: %s inimage outimage backimage\n", argv[0]);
		exit(1);
	}
	
	strcpy(in, argv[1]);
	strcpy(out, argv[2]);

	bbuf =loadiffname(argv[3], IB_rect);
	if (bbuf == 0) {
		perror(argv[3]);
		exit(2);
	}

	ibuf[0] = loadiffname(in, IB_rect);
	if (ibuf[0] == 0) {
		perror(in);
		exit(2);
	}
	
	pre_proces(ibuf[0]);
	
	ibuf[1] = dupImBuf(ibuf[0]);
	newname(in, +1);

	ibuf[2] = loadiffname(in, IB_rect);
	if (ibuf[2] == 0) exit(0);

	pre_proces(ibuf[2]);
	post_proces(ibuf[1], bbuf, out);
		
	do {
		newname(in, +1);
		ibuf[3] = loadiffname(in, IB_rect);
		if (ibuf[3] == 0) {
			ibuf[3] = dupImBuf(ibuf[2]);
			last = TRUE;
		} else pre_proces(ibuf[3]);
		
		noise_proces(ibuf, bot, top);
		post_proces(ibuf[2], bbuf, out);
		
		freeImBuf(ibuf[0]);
		ibuf[0] = ibuf[1];
		ibuf[1] = ibuf[2];
		ibuf[2] = ibuf[3];
		ibuf[3] = 0;
	} while (last == FALSE);
}


void main(long argc, char **argv)
{
	if (strcmp(argv[0], "outline") == 0) outline(argc, argv);
	else if (strcmp(argv[0], "drop") == 0) drop(argc, argv);
	else if (strcmp(argv[0], "rmnoise") == 0) rmnoise(argc, argv);
	else if (strcmp(argv[0], "bitlink") == 0) bitlink(argc, argv);
	else if (strcmp(argv[0], "maxdlta") == 0) maxdlta(argc, argv);
	else if (strcmp(argv[0], "noalpha") == 0) noalpha(argc, argv);
}


