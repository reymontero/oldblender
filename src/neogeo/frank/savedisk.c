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
#include <local/util.h>
#include <gl/image.h>
#include <unistd.h>
#include <sys/resource.h>


#include <limits.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>
#include <fcntl.h>
#include <ctype.h>

/*

zipfork "cc -g rt.c util.o -lm -lmpc -o rt > /dev/console"                                                                  
zipfork "rt > /dev/console"                                                                  

*/

void error(char * string)
{
	perror(string);
	exit(1);
}

#include <sys/types.h>
#include <time.h>

main_tijd(long argc, char **argv){
	time_t tijd;
	struct tm * tm;
	
	time(&tijd);
	tm = localtime(&tijd);
	
	printf("%d\n", tm->tm_hour);
	
}

long checktarga(tga,mem)
TARGA *tga;
char *mem;
{
	tga->numid = mem[0];
	tga->maptyp = mem[1];
	tga->imgtyp = mem[2];

	tga->maporig = GSS(mem+3);
	tga->mapsize = GSS(mem+5);
	tga->mapbits = mem[7];
	tga->xorig = GSS(mem+8);
	tga->yorig = GSS(mem+10);
	tga->xsize = GSS(mem+12);
	tga->ysize = GSS(mem+14);
	tga->pixsize = mem[16];
	tga->imgdes = mem[17];

	if (tga->maptyp > 1) return(0);
	switch (tga->imgtyp){
	case 1:			/* raw cmap */
	case 2:			/* raw rgb */
	case 3:			/* raw b&w */
	case 9:			/* cmap */
	case 10:			/* rgb */
	case 11:			/* b&w */
		break;
	default:
		return(0);
	}
	if (tga->mapsize && tga->mapbits > 32) return(0);
	if (tga->xsize <= 0) return(0);
	if (tga->ysize <= 0) return(0);
	if (tga->pixsize > 32) return(0);
	return(1);
}

long ispic2(long * buf)
{
	TARGA tga;
	long fp;
	long ofs = 0;
	
	if (buf[ofs] == CAT) ofs += 3;
	if (buf[ofs] == FORM){
		if (buf[ofs+2] == IMAG) return(CDI);
		if (buf[ofs+2] == ILBM) return(AMI);
		if (buf[ofs+2] == ANIM){
			if (buf[ofs+3] == FORM){
				return(ANIM);
			}else{
				return(Anim);
			}
		}
	}else{
		if (GS(buf) == IMAGIC) return(IMAGIC);
		if (GSS(buf) == IMAGIC) return(IMAGIC);
	}
	if (checktarga(&tga,buf)) return(TGA);
	return(FALSE);
}

main(long argc, char **argv)
{
	int buf[512];
	char trace[1024], image[1024], script[1024], targa[1024], *name, *cbuf;
	int file, ofile, yes, white;
	off_t offset;
	ulong len;
	long x, y;
	char * temp;
	IMAGE * ima;
	long lastscript;
	
	if (argc == 1) exit(0);
	
	file = open(argv[1], O_RDONLY);
	if (file == -1) error("open");
	
	
	offset = 0;
	strcpy(trace, "/render/mo/traces/0001");
	strcpy(image, "/render/mo/images/0001");
	strcpy(targa, "/render/mo/targas/0001");
	strcpy(script, "/render/mo/scripts/0001");
	
	cbuf = (char *) buf;
	ima = (IMAGE *) buf;
	
	while (read(file, buf, 512) == 512) {
		yes = FALSE;
		
		if (buf[0] == 'FORM') {
			
			if (buf[2] == 'TRI0') {
				len = buf[1] + 12;
				name = trace;
			} else {
				len = buf[1] + 8;
				name = image;
			}
			yes = TRUE;
		} else if (yes = ispic2((long *) buf)) {
			name = targa;
			if (yes == TGA) {
				/* grofweg de lengte bepalen */
				x = GSS(cbuf + 12);
				y = GSS(cbuf + 14);
				if (x <= 1280 && y <= 1280 && x > 8 && y > 8) {
					len = (x + 1) * (y + 1);
					len *= 4;
					len += 256 * 4;
					len += 1024;
				} else yes = FALSE;
			} else if (yes == IMAGIC) {
				x = ima->xsize;
				y = ima->ysize;
				if (x <= 1280 && y <= 1280 && x > 8 && y > 8) {
					len = (x + 1) * (y + 1) * ima->zsize;
					len += 1024;
				} else yes = FALSE;
			} else yes = 0;
		} else if (isprint(cbuf[0]) && offset >= lastscript) {
			name = script;
			len = white = 0;
			do {
				yes = 0;
				while (yes != 512) {
					if (isprint(cbuf[yes]) || isspace(cbuf[yes])) yes++;
					else break;
					
					if (isspace(cbuf[yes])) white = TRUE;
					
					len++;
				}
				if (yes != 512) break;
			} while (read(file, buf, 512) == 512);

			if ((len > 64) && white) yes = TRUE;
			else yes = FALSE;
			
			if (yes) lastscript = offset + len;
		}
		
		if (yes) {
			printf("%s: offset %8d: len %8d\n", name, offset, len);
			if (len < 10000000) {
				lseek (file, offset, SEEK_SET);
				temp = mallocN(len, "asd");
				if (read(file, temp, len) != len) error("read");
				ofile = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (write(ofile, temp, len) != len) error("write");
				close(ofile);
				freeN(temp);
				
				newname(name, +1);
			}
			lseek (file, offset + 512, SEEK_SET);
		}
		offset += 512;
	}
}

