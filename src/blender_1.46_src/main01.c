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
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/*

 *					 main01.c
 *					 MY second SONY!
 *
 */

#include "psxdef.h"
#include "psxgraph.h"


void main()
{
	int a, padd;
	

	init_display();
	init_psxutil();	/* malloc */
	
	/* tekst kunnen printen? */
	FntLoad(960, 256);	
	SetDumpFnt(FntOpen(16, 16, 256, 200, 0, 512));	

	initmatrices();
	swapbuffers();

	/* main loop */
	while (1) {
		
		padd= pad_read();
			
		lookat_up(&camloc, &camvec, &viewmat);

		draw_all();
		
 		FntPrint("view vec %d %d %d \n", camvec.vx, camvec.vy, camvec.vz);
 		FntPrint("view loc %d %d %d \n", camloc.vx, camloc.vy, camloc.vz);

		if (padd == -1) FntPrint("EXIT");
		FntFlush(-1);

		swapbuffers();

		if (padd == -1) {		/* if END key is pressed */
			end_display();
			end_psxutil();
			return;
		}		
	}
}
 


