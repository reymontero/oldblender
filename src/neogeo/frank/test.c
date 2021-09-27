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

#include <stdlib.h>
	typedef struct Movie{
		ushort	magic, version;
		ushort	sizex;
		ushort	sizey;
		ushort	frames;
		ushort	flags;
	/*
				MV_video
				MV_horizontal
				MV_pingpong
				MV_loop
	
				MV_audio
				MV_stereo
				MV_38Khz
	*/
		ushort	current;
		ushort	audio_size;
		uchar	ytab[64];
		uchar	ctab[64];	
		int		fd;
		int		padint[16];
		uint 	offset[1];
	}Movie;

/*

zipfork "cc -g -o test test.c util.o -limbuf -lm> /dev/console"                                                                  
zipfork "test > /dev/console"                                                                  

*/


float dist(float * p, float x, float y)
{
	x -= p[0];
	y -= p[1];
	
	return(sqrt((x * x) + (y * y)));
}

void printfbuf(float * buf, long size)
{
	long x, y;
	
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			printf("%7.0f ", buf[0]);
			buf++;
		}
		printf("\n");
	}	
	printf("\n");
}

float * make_cluster(long size)
{
	float * buf, * _buf, halfsize, p1[2], p2[2], m1[2], m2[2], val, max;
	long x, y;
	
	_buf = buf = malloc(size * size * sizeof (float));
	
	halfsize = 0.5 * size;
	
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			/* vectoren vullen */
			p1[0] = p2[0] = m1[0] = m2[0] = 0.1;
			p1[1] = p2[1] = m1[1] = m2[1] = 1.0 / 3.0;
			
			p2[0] += halfsize;
			p2[1] += halfsize;
			m1[1] += halfsize;
			m2[0] += halfsize;
			
			if (x > halfsize) {
				p1[0] += size;
				m1[0] += size;
			}
			if (y > halfsize) {
				p1[1] += size;
				m2[1] += size;
			}
			
			val = size * size;		/* altijd positief ! */
			val += dist(p1, x, y);
			val += dist(p2, x, y);
			val -= dist(m1, x, y);
			val -= dist(m2, x, y);
			printf("%f\n", val);
			*buf++ = val;
		}
	}
	
	printfbuf(_buf, size);	

	/* op volgorde sorteren en lineair verloop aanbrengen */
	
	for (y = 0; y < size * size; y++) {
		max = -0.1;
		for (x = 0; x < size * size; x++) {
			if (_buf[x] > max) {
				buf = _buf + x;
				max = _buf[x];
			}
		}
		* buf = - (y + 1) * 255.0 / (size * size);
	}
	
	printfbuf(_buf, size);	

	/* alle waardes positief maken */
	
	for (x = 0; x < size * size; x++) {
		_buf[x] = -_buf[x];
	}

	printfbuf(_buf, size);	
	return(_buf);
}

void clustered_dots_(ibuf, size)
struct ImBuf *ibuf;
long size;
{
	extern float rgb_to_bw[];
	float fx, fy, fsize, halfsize;
	int a, b, x, y, tot;
	ulong *rect;
	char *cp, *rt;
	
	cspace(ibuf,rgb_to_bw);

	halfsize= 0.5*size;
	fsize= 256/(halfsize*halfsize);
	
	cp= (char *)ibuf->rect;
	for(y=0; y<ibuf->y; y++) {
		for(x=0; x<ibuf->x; x++, cp+=4) {
			
			fx= ((x+y) % size) -halfsize;
			fy= ((1000+y-x) % size) -halfsize;
			
			tot= cp[1]+ fsize*( (fx*fx+ fy*fy) ) -128;
			
			if(tot<0) tot= 0;
			else if(tot>255) tot= 255;
			cp[1]= cp[2]= cp[3]= tot;
			
		}
	}
}

void clustered_dots(ibuf, size)
struct ImBuf *ibuf;
long size;
{
	extern float rgb_to_bw[];
	int x, y, index;
	float * array, * row;
	uchar * cp;
	
	cspace(ibuf,rgb_to_bw);
	
	array = make_cluster(size);

	cp= (uchar *) ibuf->rect;
	for (y = 0; y < ibuf->y; y++) {
		row = array + size * (y % size);
		index = 0;
		for(x = 0; x < ibuf->x; x++) {
			if (cp[3] > row[index]) cp[1]= cp[2]= cp[3]= 255;
			else cp[1]= cp[2]= cp[3]= 0;

			index++; if (index >= size) index -= size;
			cp += 4;
		}
	}
}

void main_cluster(long argc, char **argv)
{
	struct ImBuf * ibuf;
	
	if (argc < 3) exit;
	
	ibuf = loadiffname(argv[1], IB_rect);
	
	clustered_dots(ibuf, 16);
	
	ibuf->ftype = TGA;
	ibuf->cmap = 0;
	ibuf->depth = 8;
	
	saveiff(ibuf, argv[2], IB_rect);
}

void main_make_cmap(long argc, char **argv)
{
	struct ImBuf * ibuf;
	
	if (argc < 3) exit;
	
	ibuf = loadiffname(argv[1], IB_rect);
	
	freecmapImBuf(ibuf);
	
	ibuf->depth = 8;
	ibuf->mincol = 0;
	ibuf->maxcol = 256;
	ibuf->cbits = 5;
	ibuf->ftype = TGA;
	
	makecmap(ibuf, 0, 0, 0, 0, 256, dit0);
	
	saveiff(ibuf, argv[2], IB_rect);
}

void main_sbrk(long argc, char **argv)
{
	long * buf, i, size = 1<< 24;
	
	/*readtype1("/usr/lib/DPS/outline/base/Futura", 0, 1);*/
	
	/* WERKT !! */
	
	buf = sbrk(size);
	printf("%d\n", buf);
	for(i = 0; i < size / sizeof(long); i+= 100) {
		buf[i] = 1;
	}
	
	sginap(100);
	buf = sbrk(-size);
	printf("%d\n", buf);
	while (1) sginap(100);
	
}


void main_watisdit(long argc, char **argv)
{
	int fd;
	int out;
	
	fd = open("/tmp/rt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd != -1) {
		if (fork() == 0) {
			out = stdout->_file;
			printf("%d %d\n", fd, out);

			dup2(fd, stdout->_file);
			dup2(fd, stderr->_file);
			
			fprintf(stdout, "standard out\n");
			fprintf(stderr, "standard err\n");
			
			exit(0);
		}
	} else {
		printf("fd = -1\n");
	}
}


void main_uid_gid(long argc, char **argv)
{
	printf("%d %d\n", geteuid(), getuid());
	printf("%d %d\n", getegid(), getgid());
	
	setreuid(geteuid(), getuid());
	setregid(getegid(), getgid());

	printf("%d %d\n", geteuid(), getuid());
	printf("%d %d\n", getegid(), getgid());
	
	setreuid(geteuid(), getuid());
	setregid(getegid(), getgid());

	printf("%d %d\n", geteuid(), getuid());
	printf("%d %d\n", getegid(), getgid());
}

void main_movie(long argc, char **argv)
{
	Movie * movie;
	int i, max = 0, size;
	
	if (argc == 1) exit (1);
	
	movie = load_to_mem(argv[1]);
	
	printf("frames: %d\n", SWAP_S(movie->frames));
	for (i = 0; i < SWAP_S(movie->frames); i++)
	{
		size = SWAP_L(movie->offset[i+1]) - SWAP_L(movie->offset[i]);
		if (size > max) max = size;
	}
	
	printf("max: %d\n",  max);
}



typedef struct Rt{
	unsigned quant :6;
	unsigned runl  :15;
	unsigned huff  :11;
}Rt;

void main_bitjes(long argc, char **argv)
{
	long * prt;
	Rt rt;
	extern void huffdecompress(ImBuf *,  char * , int, int);
	
	prt = (long *) &rt;
	
	*prt = 0;
	rt.quant = 63;
	printf("%08x\n", *prt);
	
	
}

void main_mdec(long argc, char **argv)
{
	extern void huffdecompress(ImBuf *,  char * , int, int);
	ImBuf * ibuf;
	
	long file, size;
	char * mem;

	if (argc < 2) exit(1);
	
	file = open(argv[1], O_RDONLY);
	if (file == -1) exit (1);

	size = lseek(file,0L,SEEK_END);
	if (size > 0) {
		lseek(file,0L,SEEK_SET);

		mem = mallocN(size, "load_to_mem");
		if (mem) {
			if (read(file, mem, size) != size){
				freeN(mem);
				mem = 0;
			}
		}
	}
	close(file);
	
	ibuf = allocImBuf(320, 256, 24, IB_rect, 0);
	huffdecompress(ibuf, mem + 8, size - 8, mem[4]);
	if (argc < 3) exit(0);
	
	cspace(ibuf, yuvrgb);
	flipy(ibuf);
	saveiff(ibuf, argv[2], IB_rect);
	
}
void main_chmod(long argc, char **argv)
{
	long file;
	
/*	file = open(argv[1], O_RDWR | O_CREAT | O_TRUNC);
	printf("%s : %d\n", argv[1], file);
	if (file <= 0) perror(0);
	
	fchmod(file,0664);
	perror(0);
*/
/*	file = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0664);
	printf("%s : %d\n", argv[1], file);
	if (file <= 0) perror(0);
	
	fchmod(file,0664);
	perror(0);
*/	
	file = creat(argv[1], 0664);
	printf("%s : %d\n", argv[1], file);
	if (file <= 0) perror(0);
}


void main(long argc, char **argv)
{
	printf("0x%08x 0x%08x %08x\n", -1, -1 ^ 15, -1 ^1);
}


