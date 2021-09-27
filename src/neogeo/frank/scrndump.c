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

/*

bindkey -r f1,'cc -s pal.c -lgl -o pal\n'
bindkey -r f2,'pal\n'

 */

#include <stdio.h>
#include <sys/types.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <gl/get.h>
/*#include <gl/gr2vid.h>*/

#include <local/iff.h>
#include <local/util.h>

/*

zipfork "cc -O3 scrndump.c -lgl_s -limbuf -limage -lm util.o -o scrndump"                                          
zipfork "scrndump >/dev/console"                                          

*/

main()
{
	char head[128], tail[128], name[128];
	ushort numlen;
	long pic, maxx, maxy;
	struct ImBuf *ibuf;
	
	printf("\nDruk beide Ctrl-toetsen tegelijkertijd in om een plaatje in de huidige\n");
	printf("directory weg te schrijven.\n");
	printf("Het eerste plaatje heet dump.0001, het volgende dump.0002, enz.\n");
	printf("Druk beide Alt-toetsen in om het programma te stoppen.\n\n");

	foreground();
	prefposition(0, 10, 0, 10);
	noport();
	winopen("PAL");

	maxx = getgdesc(GD_XPMAX);
	maxy = getgdesc(GD_YPMAX);
	
	ibuf = allocImBuf(maxx, maxy, 24, IB_rect, 0);
	ibuf->ftype = IMAGIC;
	
	pic = stringdec("dump.0001", head, tail, &numlen);
	while(1){
		if (getbutton(LEFTALTKEY) && getbutton(RIGHTALTKEY)) exit(0);
		
		if (getbutton(LEFTCTRLKEY) && getbutton(RIGHTCTRLKEY)) {
			stringenc(name, head, tail, numlen, pic);
			pic++;
			printf("reading %s\n", name);
			fflush(stdout);
			sginap(10);
			readdisplay(0, 0, maxx-1, maxy-1, ibuf->rect, RD_FREEZE);
			printf("writing %s\n", name);
			/* wegschrijven */
			saveiff(ibuf, name, IB_rect);
			printf("done\n\n");
			while (getbutton(LEFTCTRLKEY) & getbutton(RIGHTCTRLKEY)) sginap(4);
		}
		sginap(4);
	}
}

