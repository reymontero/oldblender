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

/* voor testen DNA */


struct Dub {
	float d1, d2;
};

struct Temp {
	float mat[4][4];
	float *edve;
	struct Dub dub;
	void (*func)();
};

struct Temp2 {
	struct Temp rt;
	rctf *rt2[6];
	char *rt3;
};

struct View2Dtemp
{
	rctf tot, cur;
	float min[2], max[2];
	int scroll;
};

struct VectorFont2 {
	struct VectorFont2 *next,*prev;
	char *name;
	unsigned char *base;
	unsigned char *index[256];
	short kcount,us;
	short *kern;
	struct Dub * objfnt;
	char type, rt1, rt2, rt3;
};


