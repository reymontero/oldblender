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

#include <local/iff.h>


main()
{
	ImBuf * cmap, * ibuf;
	
	cmap = loadiffname("/hosts/quad/usr/people/blend/cmap.tga", IB_rect);
	if (cmap == 0) exit(0);
	printf("cmap OK\n");
	
	ibuf = loadiffname("/hosts/quad/pics/textures/plants1", IB_rect);
	if (ibuf == 0) exit(0);
	printf("ibuf OK\n");
	
			ibuf->mincol =   0;
			ibuf->maxcol = 256;
			ibuf->cbits  =   3;
			ibuf->depth  =   8;
			setdither('F');
			
			ibuf->cmap = cmap->cmap;
			
			converttocmap(ibuf);
/*			applycmap(ibuf);
			ibuf->depth = 24;
			ibuf->cmap = 0;
*/			
	saveiff(ibuf, "/hosts/quad/data/rt", IB_rect | IB_cmap);
}

