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


/* hoort bij button.c */

#ifndef BUTTON_H
#define BUTTON_H


#include <gl/device.h>
#include <fmclient.h>

/* dit is oud */
#define PAPER 100
#define PEN 7
#define BUT_IN 13
#define BUT_OUT 12


#define CHA	32
#define SHO	64
#define LON	96
#define INT	96
#define FLO	128
#define DOU	160
#define FUN	192
#define BIT	256

#define BUTPOIN	(128+64+32)

#define BUT	(1<<9)
#define ROW	(2<<9)
#define TOG	(3<<9)
#define SLI	(4<<9)
#define	NUM	(5<<9)
#define TEX	(6<<9)
#define TOG3	(7<<9)
#define BUTRET	(8<<9)
#define TOGN	(9<<9)
#define LABEL	(10<<9)
#define MENU	(11<<9)
#define ICONROW	(12<<9)
#define ICONTOG	(13<<9)
#define NUMSLI	(14<<9)
#define COL		(15<<9)
#define IDPOIN	(16<<9)
#define HSVSLI 	(17<<9)

#define BUTTYPE	(31<<9)

#define MAXBUTSTR	20

#define BTST(a,b)	( ( (a) & 1<<(b) )!=0 )
#define BCLR(a,b)	( (a) & ~(1<<(b)) )
#define BSET(a,b)	( (a) | 1<<(b) )

#define RGBDRAW		1

extern struct But *DefBut();
extern short DoButtons();


struct But
{
	short type,nr;
	char *str;
	char strdata[MAXBUTSTR];
	short x1,y1,x2,y2;
	char *poin;
	float min,max;
	short a1,a2,rt[4];
	void (*func)();

	fmfonthandle font, fonts;
	short lock, win;
	short col,drawtype;
};

struct ButCol
{
	short drawmode;
	short rgb;
	unsigned long back,paper_sel,paper_desel,pen_sel,pen_desel;
	unsigned long border1,border2,border3,border4;
};

struct ButBlock
{
	struct ButBlock *next;
	struct But *first;
	long aantal,window;
	char naam[20];
};

struct Bgrp{
    struct Bgrp *next,*prev;
    short type,nr;
    char *str;
    short x1,y1,x2,y2;
    char *poin;
    float min,max;
    short a1,a2;
	
	fmfonthandle font;
	short col,drawtype;
	void * func;
};

struct ButIcon {
	short xim, yim;
	unsigned long *rect;
	short xofs, yofs;
};

#endif /* BUTTON_H */

