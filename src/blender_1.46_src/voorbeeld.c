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



convert(ImBuf * sbuf)
{
	ImBuf * vbuf;
	
	vbuf = loadiffname("rt", IB_test);
	
	vbuf->rect = sbuf->rect;
	vbuf->x = sbuf->x;
	vbuf->y = sbuf->y;
	vbuf->cbits = 6;	/* diepte van tabel */
	

	alpha_to_col0(2);	/* 1: alleen alpha 0,  2: alpha<128 */
	
	/* functie doorgeven OF 2,4,'f'); */
	setdither(2, 4, 'f', o, dit2, dit4);
	
	converttocmap(vbuf);
	
}

