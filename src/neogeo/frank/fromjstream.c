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

zipfork "cc -O -o test util.o test.c > /dev/console"                                                                  
zipfork "chromakey /pics/ahsgrab/0251 /pics/ahsgrab/back_mask_start > /dev/console"                                                                  

*/

uchar * find_or_die (uchar * mem, long len, long match)
{
	char str[5];
	/*printf("%d, %8x\n", len, match);*/
	
	while (len > 0) {
		if (mem[0] == ((match >> 24) & 0xff)) {
			if (mem[1] == ((match >> 16) & 0xff)) {
				if (mem[2] == ((match >> 8) & 0xff)) {
					if (mem[3] == (match & 0xff)) {
						/*printf("found\n");*/
						return(mem);
					}
				}
			}
		}
		len--;
		mem++;
	}
	
	printf("no (more) match found\n");
	exit(0);
}


long my_write(long file,  uchar * mem,  long size)
{
	uchar * mem2;
	long newsize, count;
	
	
	mem2 = malloc(2 * size);
	
	count = newsize = 0;
	
	while (count < size) {
		mem2[newsize] =  mem[count];
		if (mem[count] == 0xff) {
			newsize++;
			mem2[newsize] = 0;
		}
		count++; newsize++;
	}
	
	if (write(file,  mem2, newsize) != newsize) return(-1);
	free(mem2);
	
	return(size);
}

void main(long argc, char **argv)
{
	uchar * stream, *point1 = 0, *point2 = 0, *point3, *streamend;
	uchar * example;
	long streamsize, file, headersize, marker;
	uchar frame[256], field[260], end[3];
	
	if (argc < 4) {
		printf("usage: %s in.jst example.jf0 out\n", argv[0]);
		exit(1);
	}
	
	end[0] = 0;
	end[1] = 0xff;
	end[2] = 0xd9;
	
	file = open(argv[1],O_RDONLY);
	if (file == -1) {
		perror(argv[1]);
		exit(1);
	}
	
	streamsize = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	stream = (uchar *) mmap(0, streamsize, PROT_READ, MAP_SHARED, file, 0);
	streamend = stream + streamsize;
	
	point1 = find_or_die (stream, streamsize, 'DATA');
	point1 += 8;
	
	memcpy(&marker, point1, 4);

	printf("%d\n", marker);
	
	file = open(argv[2],O_RDONLY);
	if (file == -1) {
		perror(argv[2]);
		exit(1);
	}
	
	headersize = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	example = (uchar *) mmap(0, headersize, PROT_READ, MAP_SHARED, file, 0);
	point2 = find_or_die (example, headersize, marker);
	
	headersize = point2 - example;
	
	strcpy(frame, argv[3]);
	
	while (1) {
		point2 = find_or_die (point1 + 4, (streamend - point1) - 4, 0xffffffff);
				
		strcpy(field, frame);
		strcat(field, ".jf0");
		file = open(field, O_RDWR | O_CREAT | O_TRUNC);
		
		if (file == -1) {
			perror(field);
			exit(0);
		}
		
		fchmod(file,0664);
		write(file, example, headersize);
		if (my_write(file, point1, point2 - point1) != point2 - point1){
			perror(field);
			exit(0);
		}

		write(file, end, 3);
		close(file);
		
		while(point2[0] == 0xff) point2++;
		point1 = point2;
		
/* */

		point2 = find_or_die (point1 + 4, (streamend - point1) - 4, 0xffffffff);
		
		strcpy(field, frame);
		strcat(field, ".jf1");
		file = open(field, O_RDWR | O_CREAT | O_TRUNC);
		
		if (file == -1) {
			perror(field);
			exit(0);
		}
		
		fchmod(file,0664);
		write(file, example, headersize);
		if (my_write(file, point1, point2 - point1) != point2 - point1){
			perror(field);
			exit(0);
		}

		write(file, end, 3);
		close(file);
		
		while(point2[0] == 0xff) point2++;
		point1 = point2;
		
		printf("saved: %s\n", frame);
		
		newname(frame, +1);
		
	} 
}


