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


main(argc,argv)
int argc;
char **argv;
{
	FILE * file;
	long code, next, byte, last, pos;
	char name[256];
	
	strcpy(name, "/pics/martinnew/01_BLOED.PIM");
	if (argc > 1) strcpy(name, argv[1]);
	
	file = fopen(name, "rb");
	
	do{
		byte = getc(file);
		if (byte == EOF) break;
		code = (code << 8) | ((next >> 24) & 0xff);
		next = (next << 8) | (byte & 0xff);
		if ((code & 0xffffff00) == 0x0100) {
			pos = ftell(file) - 8;
			printf("%8x %8x %8x %8x\n", code,  next, pos, pos - last);
			last = pos;
		}
	}while(1);
}

