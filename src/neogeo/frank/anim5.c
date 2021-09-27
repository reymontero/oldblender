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

void anim5decode(ibuf,dlta,skiptab)
struct ImBuf *ibuf;
uchar *dlta;
long *skiptab;
{
	uchar depth;
	long skip;
	long *ofspoint;
	uchar **planes;

	/*	samenstelling delta:
		lijst met ofsets voor delta's per bitplane (ofspoint)
		per kolom in delta (point)
			aantal handelingen (noops)
				code
					bijbehorende data
				...
			...
	*/

	ofspoint = (long *)dlta;
	skip = ibuf->skipx * sizeof(long);
	planes = (uchar **)ibuf->planes;

	for(depth=ibuf->depth ; depth>0 ; depth--){

		if (ofspoint[0]){
			uchar *planestart;
			uchar *point;
			uchar x;

			point = dlta + ofspoint[0];
			planestart = planes[0];
			x = (ibuf->x + 7) >> 3;

			do{
				uchar noop;

				if (noop = *(point++)){
					uchar *plane;
					uchar code;

					plane = planestart;
					do{
						if ((code = *(point++))==0){
							uchar val;

							code = *(point++);
							val = *(point++);
							do{
								plane[0] = val;
								plane += skip;
							}while(--code);

						} else if (code & 128){

							code &= 0x7f;
							do{
								plane[0] = *(point++);
								plane += skip;
							}while(--code);

						} else plane += skiptab[code];

					}while(--noop);
				}
				planestart++;
			}while(--x);
		}
		ofspoint++;
		planes++;
	}
}



void anim5xordecode(ibuf,dlta,skiptab)
struct ImBuf *ibuf;
uchar *dlta;
long *skiptab;
{
	uchar depth;
	long skip;
	long *ofspoint;
	uchar **planes;

	/*	samenstelling delta:
		lijst met ofsets voor delta's per bitplane (ofspoint)
		per kolom in delta (point)
			aantal handelingen (noops)
				code
					bijbehorende data
				...
			...
	*/

	ofspoint = (long *)dlta;
	skip = ibuf->skipx * sizeof(long);
	planes = (uchar **)ibuf->planes;

	for(depth=ibuf->depth ; depth>0 ; depth--){

		if (ofspoint[0]){
			uchar *planestart;
			uchar *point;
			uchar x;

			point = dlta + ofspoint[0];
			planestart = planes[0];
			x = (ibuf->x + 7) >> 3;

			do{
				uchar noop;

				if (noop = *(point++)){
					uchar *plane;
					uchar code;

					plane = planestart;
					do{
						if ((code = *(point++))==0){
							uchar val;

							code = *(point++);
							val = *(point++);
							do{
								plane[0] ^= val;
								plane += skip;
							}while(--code);

						} else if (code & 128){

							code &= 0x7f;
							do{
								plane[0] ^= *(point++);
								plane += skip;
							}while(--code);

						} else plane += skiptab[code];

					}while(--noop);
				}
				planestart++;
			}while(--x);
		}
		ofspoint++;
		planes++;
	}
}


main(argc,argv)
long argc;
char **argv;
{
	struct ImBuf *ibuf1,*ibuf2,*ibuf;
	struct ListBase _animbase,*animbase;
	long buf[20],file,*ldlta,win,go = TRUE,*skiptab,i,j;
	struct Link * dlta;
	short val, doflip = TRUE, wplanes = FALSE, stopped = FALSE, wait = FALSE;
	char version[16];
	long dbbuffer = TRUE, xor = FALSE, cyclic = TRUE;
	Anhd anhd;
	
	animbase = &_animbase;
	animbase->first = animbase->last = 0;

	if (argc == 1){
		printf("usage: %s anim5\n",argv[0]);
		exit(0);
	}

	file = open(argv[1],O_RDONLY);
	if (file<0){
		printf("couldn't open %s\n",argv[1]);
		exit(0);
	}

	if (read(file,buf,24) != 24){
		printf("error while reading %s\n",argv[1]);
		exit(0);
	}

	if ((buf[0] != FORM) || (buf[2] != ANIM) || (buf[3] != FORM) || (buf[5] != ILBM)){
		printf("No anim5 file %s\n",argv[1]);
		exit(0);
	}

	ldlta = (long *) malloc(buf[4] + 20);
	memcpy(ldlta+2,buf+3,12);
	if (read(file,ldlta+5,buf[4]-4) != buf[4]-4){
		perror("read error");
		exit(0);
	}
	addtail(animbase,ldlta);

	version[4] = 0;
	while (read(file,buf,8) == 8){
		buf[1] = (buf[1] + 1) & ~1;
		switch(buf[0]){
		case FORM:
			lseek(file,4,SEEK_CUR);
			break;
		case DLTA:
			ldlta = (long *) malloc(buf[1] + 8 + sizeof(struct Link));
			ldlta[2] = buf[0];
			ldlta[3] = buf[1];
			if (read(file,ldlta+4,buf[1]) != buf[1]){
				perror("read error");
				free(ldlta);
			} else addtail(animbase,ldlta);
			break;
		case ANHD:
			read(file, &anhd, sizeof(Anhd));
			if (buf[1] != sizeof(Anhd)) lseek(file,buf[1] - sizeof(Anhd),SEEK_CUR);
			/*printf("%d : %d : %d\n", anhd.type, anhd.interleave, anhd.bits);*/
			if (anhd.interleave == 1) dbbuffer = FALSE;
			else dbbuffer = TRUE;
			if (anhd.bits & 2) xor = TRUE;
			else xor = FALSE;
			
			break;
		default:
			i = buf[1];
			lseek(file,buf[1],SEEK_CUR);
			buf[1] = 0;
			/*printf("skipping %s size %d\n",buf, i);*/
			break;
		}
	}

	ibuf1 = loadiffmem((long *)((char *) (animbase->first)+sizeof(struct Link)),0);
	ibuf2 = loadiffmem((long *)((char *) (animbase->first)+sizeof(struct Link)),0);
	if (ibuf2 == 0){
		printf("couldn't load picture\n");
		exit(0);
	}

	/*foreground();*/
	prefsize(ibuf1->x-1, ibuf1->y-1);
	noborder();
	win = winopen(argv[1]);

	gversion(version);
	/*printf("vers: %s\n", version);*/
	dither(DT_OFF);

	if (strncmp(version, "GL4DXG-", sizeof("GL4DXG")) == 0){
		pixmode(PM_TTOB, 1);
		doflip = FALSE;
		if (IS_ham(ibuf1) == FALSE){
			pixmode(PM_C0, 0);
			pixmode(PM_C1, -1);
			pixmode(PM_SIZE, 1);
			pixmode(PM_EXPAND, TRUE);			
			wplanes = TRUE;
		}
	}

	if (IS_ham(ibuf1)){
		RGBmode();
		doublebuffer();
		gconfig();
		if (ibuf1->depth > 6) dither(DT_ON);
	} else {
		doublebuffer();
		gconfig();
		color(512); clear();
		swapbuffers(); clear();
		for(i = 0; i< ibuf1->maxcol; i++){
			j = ibuf1->cmap[i];
			mapcolor(512 + i, j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff);
		}
		writemask(~512);
	}

	qdevice(ESCKEY);
	qdevice(PAD1); qdevice(PAD2);
	qdevice(PAD3); qdevice(PAD4);
	qdevice(PAD5); qdevice(PAD6);
	qdevice(PAD7); qdevice(PAD8);
	qdevice(PAD9); qdevice(PADPERIOD);
	qdevice(PAD0); qdevice(PADENTER);

	skiptab = mallocstruct(long,ibuf1->y+1);
	for (i = ibuf1->y ; i>= 0; i--){
		skiptab[i] = i*ibuf1->skipx*sizeof(long);
	}

	dlta = animbase->first;
	if (dlta) dlta = dlta->next;

	while (go){
		if (IS_ham(ibuf1)) {
			addrectImBuf(ibuf1);
			bptolong(ibuf1);
			applycmap(ibuf1);
			if (doflip) flipy(ibuf1);
			lrectwrite(0,0,ibuf1->x-1,ibuf1->y-1,ibuf1->rect);
		} else {
			if (wplanes) {
				for (i = 0; i < ibuf1->depth; i++){
					writemask(1 << i);
					lrectwrite(0, 0, ibuf1->x-1, ibuf1->y-1, ibuf1->planes[i]);
				}
			} else {
				bptolong(ibuf1);
				if (doflip) flipy(ibuf1);
				lrectwrite(0,0,ibuf1->x-1,ibuf1->y-1,ibuf1->rect);
			}
		}
		swapbuffers();
		
		if (dlta == 0) {
			dlta = animbase->first;
			if (dlta) dlta = dlta->next;
			if (dbbuffer) if (dlta) dlta = dlta->next;
		}

		if (dlta){
			if (dbbuffer) ibuf = ibuf2;
			else ibuf = ibuf1;
			if (xor) anim5xordecode(ibuf,((char *) (dlta)) + sizeof(struct Link) + 8,skiptab);
			else anim5decode(ibuf,((char *) (dlta)) + sizeof(struct Link) + 8,skiptab);
			if (dbbuffer) {
				ibuf = ibuf1;
				ibuf1 = ibuf2;
				ibuf2 = ibuf;
			}
			dlta = dlta->next;
		} else go = FALSE;
		
		wait = stopped;
		while (qtest() || wait){
			switch(qread(&val)){
			case ESCKEY:
				if (val) {
					go = wait = FALSE;
				}
				break;
			case PAD1:
				swapinterval(1); break;
			case PAD2:
				swapinterval(2); break;
			case PAD3:
				swapinterval(3); break;
			case PAD4:
				swapinterval(4); break;
			case PAD5:
				swapinterval(5); break;
			case PAD6:
				swapinterval(6); break;
			case PAD7:
				swapinterval(7); break;
			case PAD8:
				swapinterval(8); break;
			case PAD9:
				swapinterval(9); break;
			case PAD0:
				if (val) {
					freeImBuf(ibuf1);
					freeImBuf(ibuf2);
					ibuf1 = loadiffmem((long *)((char *) (animbase->first)+sizeof(struct Link)),0);
					ibuf2 = loadiffmem((long *)((char *) (animbase->first)+sizeof(struct Link)),0);
					stopped = TRUE;
					wait = FALSE;
					dlta = animbase->first;
					if (dlta) dlta = dlta->next;
				}
				break;
			case PADPERIOD:
				if (val){
					if (stopped) {
						wait = FALSE;
					}else{
						stopped = wait = TRUE;
					}
				}
				break;
			case PADENTER:
				if (val) wait = stopped = FALSE;
				break;
			case REDRAW:
				if (IS_ham(ibuf1) == FALSE){
					writemask(-1);
					color(512);
					clear();
					swapbuffers();
					clear();
					swapbuffers();
					writemask(~512);
				}
				break;
			}
		}
	}
}

