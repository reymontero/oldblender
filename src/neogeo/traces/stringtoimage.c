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

/* makestring.c	feb 94 

 * 
 * Dit programma laadt "/usr/people/trace/util.scenes/drawstring.trace",
 * verandert de string en schrijft rt.trace.
 * Daarna wordt een renderopdracht gegeven.
 * 
 * cc stringtoimage.c -o stringtoimage
 */


#include <stdio.h>
#include <fcntl.h>
#include <string.h>


void main(argc, argv)
int argc;
char **argv;
{
	long version, file, filelen;
	char str[100], *traces, *cp;

	if(argc!=2) {
		printf("usage: stringtoimage 'string... '\n");
		exit(0);
	}

	strcpy(str, "/usr/people/trace/util.scenes/drawstring.trace");
	file=open(str,O_RDONLY);
	if(file== -1) {
		exit(0);
	}
	filelen=lseek(file,0,2);	/* seek end */
	lseek(file,0,0);		/* en weer terug */

	traces= (char *)malloc(filelen);
	read(file, traces, filelen);
	close(file);

	cp= traces+filelen-20;
	while(cp>traces) {
		if( *cp=='a' && *(cp+1)=='b' ) {
			if( *(cp+2)=='c' && *(cp+3)=='d' ) {
				if(strncmp(cp, "abcd    ", 8)==0) {
				
					strcpy(str, argv[1]);
					str[31]= 0;
					strcpy(cp, str);
					
					strcpy(str, "/usr/people/trace/util.scenes/rt.trace");
					file=open(str,O_WRONLY+O_CREAT+O_TRUNC);
					if(file== -1) {
						exit(0);
					}
					fchmod(file, 0664);
					write(file, traces, filelen);
					close(file);
					
					free(traces);
					
					system("traces -b /usr/people/trace/util.scenes/rt.trace -f 1");
					exit(0);
				}
			}
		}
		cp--;
	}
	exit(0);
}

