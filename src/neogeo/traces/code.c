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



#include <fcntl.h>
#include <sys/syssgi.h>
#include <string.h>


void decodekeytab()
{
	extern short keycode[100];
	extern unsigned char hash[512];
	char ihash[512];
	long a,b,file;
	char *c,hus1,hus2,*adr,sl[MAXSYSIDSIZE],str[30];

	file= open("/.Tcode",O_RDONLY);
	if(file== -1) exit(0);

	read(file,keycode,200);
	close(file);

	syssgi(SGI_SYSID,sl);

	/* inverse hashtab */
	for(a=0;a<256;a++) {
		for(b=0;b<256;b++) {
			if(a==hash[b]) break;
		}
		ihash[a]= b;
	}
	for(a=0;a<256;a++) ihash[256+a]= ihash[a];

	/* bereken hus1 en hus2 ahv sleutel */
	hus1= hash[ sl[0]+ hash[ sl[2]] ];
	hus2= hash[ sl[1]+ hash[ sl[3]] ];

	adr= (char *)decodekeytab;

	c= (char *)keycode;
	for(a=0;a<100;a++) {
		c[0]= ( ihash[ (ihash[ c[0] ] -hus1) & 255 ]  ) & 255 ;
		c[1]= ( ihash[ (ihash[ c[1] ] -hus2) & 255 ]  ) & 255 ;
		c+=2;
		if((a & 3)==3) adr++;
	}

}


