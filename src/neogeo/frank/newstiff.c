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

#include <local/util.h>
#include <printstiff.h>
#include <malloc.h>
#include <math.h>

float gamm = 1.0;
uchar gam[256];

void morecol(long sizex, uchar * buf, float fac) {
	long c, m, y, x;
	float avg;
	
	if (fac == 1.0) return;
	
	for (x = 0; x < sizex; x++) {
		c = buf[0];
		m = buf[1];
		y = buf[2];
		
		avg = (c + m + y) / 3.0;
		c = 0.5 + avg + fac * (c - avg);
		m = 0.5 + avg + fac * (m - avg);
		y = 0.5 + avg + fac * (y - avg);
		
		if (c & 0xffffff00) {
			if (c < 0) c = 0;
			else c = 255;
		}
		
		if (m & 0xffffff00) {
			if (m < 0) m = 0;
			else m = 255;
		}
		
		if (y & 0xffffff00) {
			if (y < 0) y = 0;
			else y = 255;
		}
		
		*buf++ = c;
		*buf++ = m;
		*buf++ = y;
	}
}

int main(int argc, char *argv[])
{
    STStream *iptr, *optr;
    PSTImageHeader _pheader, *pheader = &_pheader;
	int page = 0, count, x, y, last, i, invert = FALSE;
	float colscale = 1.0;
	char * buffer, * _buffer = NULL, * prog;
	
	prog = argv[0];
	
	while (argc > 1) {
		/*fprintf(stderr, "processing option: [%s]\n", argv[1]);*/
		if (argv[1][0] != '-') break;
		i = 1;
		while (argv[1][i]) {
			switch(argv[1][i]) {
			case 'g':
				if (argv[1][i + 1]) gamm = atof(argv[1] + i + 1);
				else {
					gamm = atof(argv[2]);
					argc--;
					argv++;
				}
				i = strlen(argv[1]) - 1;
				break;
			case 'i':
				invert = TRUE;
				break;
			case 'c':
				if (argv[1][i + 1]) colscale = atof(argv[1] + i + 1);
				else {
					colscale = atof(argv[2]);
					argc--;
					argv++;
				}
				
				i = strlen(argv[1]) - 1;
				break;
			default:
				printf("unknown option %c\n", argv[1][i]);
				argc = 0;
			}
			i++;
		}
		argc--;
		argv++;
	}
	
	if (invert) {
		for (x = 255 ; x >= 0 ; x--) gam[x] = 255.0 * pow((255.0 - x) / 255.0 , 1.0 / gamm) + 0.5;
	} else {
		for (x = 255 ; x >= 0 ; x--) gam[x] = 255.0 - (255.0 * pow(x / 255.0 , gamm)) + 0.5;
	}
	
	gam[0] = 255;
	gam[255] = 0;		
	
	if ((iptr = STOpen(fileno(stdin), ST_READ)) == NULL) {
		STPerror(prog);
		exit(1);
	}
	
	if ((optr = STOpen(fileno(stdout), ST_WRITE)) == NULL) {
		STPerror(prog);
		exit(1);
	}

	while (PSTReadImageHeader(iptr, pheader) != -1) {
		
		fprintf(stderr, "\tReading page: %ld\n", page);

		if (pheader->bitsPerSample != 8) {
			fprintf(stderr, "\tBits per sample: %d, not supported\n", pheader->bitsPerSample);
			exit(1);
		}
		if (pheader->type != ST_TYPE_RGB) {
			fprintf(stderr, "\tImage type: %d, not RGB\n", pheader->type);
		}
				
		fprintf(stderr, "\tImage width: %ld\n", pheader->width);
		fprintf(stderr, "\tImage height: %ld\n", pheader->height);
		fprintf(stderr, "\tBits per sample: %d\n", pheader->bitsPerSample);
		fprintf(stderr, "\tSamples per pixel: %d\n", pheader->samplesPerPixel);
		fprintf(stderr, "\tImage size (bytes): %ld\n", pheader->imgbytes);
		fprintf(stderr, "\tImage type: %d\n", pheader->type);
		fprintf(stderr, "\tData format: %d\n", pheader->plane);
		
		fprintf(stderr, "\tResolution unit: %d\n", pheader->resUnit);
		fprintf(stderr, "\tX Resolution: %d\n", pheader->xRes);
		fprintf(stderr, "\tY Resolution: %d\n", pheader->yRes);
		fprintf(stderr, "\tThresholding: %d\n", pheader->thresholding);
		fprintf(stderr, "\tCompression: %d\n", pheader->compression);
		fprintf(stderr, "\tDate/time: %s\n", pheader->dateTime);
		fprintf(stderr, "\tHost computer: %s\n", pheader->hostComputer);
		fprintf(stderr, "\tSoftware: %s\n", pheader->software);
		fprintf(stderr, "\tDocument name: %s\n", pheader->docName);
		fprintf(stderr, "\tCurrent page number: %d\n", pheader->pageNumbers[0]);
		fprintf(stderr, "\tTotal pages: %d\n", pheader->pageNumbers[1]);
		fprintf(stderr, "\tTarget printer: %s\n", pheader->targetPrinter);
		fprintf(stderr, "\tDriver options: %s\n", pheader->driverOptions);


		pheader->type = ST_TYPE_CMY;
		if (pheader->pageNumbers[0] == pheader->pageNumbers[1]) last = 1;
		else last = 0;
		
		if (PSTWriteImageHeader(optr, pheader, 0) < 0) {
			STPerror(prog);
			exit(1);
		}
		
		count = pheader->width * pheader->samplesPerPixel;
		
		if (_buffer != NULL) free(_buffer);
		buffer = _buffer = malloc(count);
		
		for (y = pheader->height; y > 0; y--) {
			if (STRead(iptr, _buffer, count) != count) {
				STPerror(prog);
				exit(1);
			}
			
			if (pheader->samplesPerPixel == 3) {
				morecol(pheader->width, buffer, colscale);
			}

			for (x = count - 1; x >= 0; x--) buffer[x] = gam[buffer[x]];
			
			if (STWrite(optr, _buffer, count) != count) {
				STPerror(prog);
				exit(1);
			}
		}
		page++;
		fprintf(stderr, "\t\tNext\n");
	}
}


