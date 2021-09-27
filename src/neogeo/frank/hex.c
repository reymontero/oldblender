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

#include <ctype.h>

char binhex[16] = {
'0', '1', '2', '3',
'4', '5', '6', '7',
'8', '9', 'A', 'B',
'C', 'D', 'E', 'F', };

main__()
{
	int c, l, i;
	char str[4096];
	int bytesperline = 16;
	
	while ((c = getchar()) != EOF) {
		if (i == 0) { 
			printf(" %s\n%8x ", str, l);
			i = bytesperline;
		}
		printf("%c%c", binhex[c >> 4], binhex[c & 0x0f]);
		
		if (isprint(c) && c < 0x80) str[bytesperline - i] = c;
		else str[bytesperline - i] = '.';
		l++; i--;
	}
	str[bytesperline - i] = 0;
	printf("%s\n", str);
}


main(long argc, char **argv)
{
	int c, l = 0, i = 0;
	char str[4096];
	int bytesperline = 16;
	
	if (argc == 1) {
		while ((c = getchar()) != EOF) {
			if (i == 0) { 
				printf(" %s\n%8x ", str, l);
				i = bytesperline;
			}
			printf("%c%c", binhex[c >> 4], binhex[c & 0x0f]);
			
			if (isprint(c) && c < 0x80) str[bytesperline - i] = c;
			else str[bytesperline - i] = '.';
			l++; i--;
		}
		str[bytesperline - i] = 0;
		printf("%s\n", str);
	} else if (argc == 2) {
		while ((c = getchar()) != EOF) {
			if (c == 10 || ((i & 255) == 255)) {
				if (c == 10) printf("10");
				printf("\n");
				i = 0;
			}
			else printf("%c%c", binhex[c >> 4], binhex[c & 0x0f]);
			i++;
		}
	} else {
		i = 0;
		while ((c = getchar()) != EOF) {
			printf("%d, ", c);
			i++;
			if ((i & 0x1f) == 0x1f) printf("\n");
		}
	}
}

