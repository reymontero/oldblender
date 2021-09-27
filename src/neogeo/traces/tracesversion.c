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

/* tracesversion.c	nov. 92 

 * 
 * Dit programma laadt de file "lasttracesversion",  hoogt het eerst
 * daarin gevonden nummer op met 1 en schrijft het over.
 * 
 * Vervolgens wordt de traces ingelezen en daarin gezocht naar
 * de string "TracesVersion Develop".
 * Op de plek van "Develop" wordt het nummer geschreven.
 * De gewijzigde traces wordt daarna in de /usr/people/bin geschreven.
 * 
 * cc tracesversion.c -o tracesversion
 */


#include <stdio.h>
#include <fcntl.h>
#include <string.h>


void main()
{
	long version, file, filelen;
	char str[50], *traces, *cp;

	strcpy(str, "lasttracesversion");
	file=open(str,O_RDONLY);
	if(file== -1) {
		printf("Can't find file 'lasttracesversion'\n");
		exit(0);
	}
	filelen=lseek(file,0,2);	/* seek end */
	lseek(file,0,0);		/* en weer terug */
	cp= (char *)malloc(filelen);
	read(file, cp, filelen);
	close(file);

	sscanf(cp, "%d", &version);
	version++;
	sprintf(cp, "%d", version);
	cp[4]= 32;

	file=open(str,O_WRONLY+O_CREAT+O_TRUNC);
	if(file== -1) {
		printf("Can't write file 'lasttracesversion'\n");
		exit(0);
	}
	fchmod(file,0775);
	write(file, cp, filelen);
	close(file);
	free(cp);


	strcpy(str, "traces");
	file=open(str,O_RDONLY);
	if(file== -1) {
		printf("Can't find traces\n");
		exit(0);
	}
	filelen=lseek(file,0,2);	/* seek end */
	lseek(file,0,0);		/* en weer terug */
	traces= (char *)malloc(filelen);
	read(file, traces, filelen);
	close(file);

	cp= traces+filelen-20;
	while(cp>traces) {
		if( *cp=='T' && *(cp+1)=='r' ) {
			if( *(cp+2)=='a' && *(cp+3)=='c' ) {
				if(strncmp(cp, "Traces Version Develop", 21)==0) {
					cp+= 14;
					sprintf(str, "%d", version);
					cp[0]= str[0];
					cp[1]= '.';
					cp[2]= str[1];
					cp[3]= str[2];
					cp[4]= str[3];
					cp[5]= 0;

					strcpy(str, "/usr/people/bin/traces");
					file=open(str,O_WRONLY+O_CREAT+O_TRUNC);
					if(file== -1) {
						printf("Can't write bin/traces");
						free(traces);
						exit(0);
					}
					fchmod(file,0775);
					write(file, traces, filelen);
					close(file);
					printf("New version of traces: %s\n", cp);
					free(traces);
					exit(0);
				}
			}
		}
		cp--;
	}

	free(traces);
	printf("Can't find versionstring in tracesfile\n");

	exit(0);
}

