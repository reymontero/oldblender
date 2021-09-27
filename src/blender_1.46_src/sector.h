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

/* sector.h    maart 96

 * 
 * 
 */

#ifndef SECTOR_H
#define SECTOR_H

typedef struct Event {
	short event, var, sfac, sfac1;	/* shortversies van fac en fac1 */
	short shiftval;
	char flag, pulse;
	short varnr, rt;
	float fac, fac1;
	void *poin;
	char name[16];
} Event;

typedef struct Action {
	short action, go, cur, var, flag, varnr;	/* varnr voor actvarar */
	short sta, end, butsta, butend;
	float force, fac, min, max;
	
	void *poin;
	char name[16];
} Action;

typedef struct Sensor {
	
	short totaction, totevent;
	short event, evento;		/* voor 'hold' test */
	short flag, rt;
	
	Event *events;
	Action *actions;
	
	/* hieronder is oud: laten staan tot '98 voor compatibility */
	/* daarna versions intelligent oplossen (met nepstruct?) */
	short event1, event2;
	int action;
	float force, sta, end, fac1, fac2, cur, dx;
} Sensor;

typedef struct NetLink {
	struct NetLink *next, *prev;
	Object *ob;
	short type, flag;
	short sfra, len;		/* sfra: offset in 'movie' */
	/* ipo-verwijzing? */
	
} NetLink;

#
#
typedef struct {
	Object *ob;
	float piramat[4][4];
	short axis[2];
	short visi, direction, dist, d_axis;	/* LOGICAL VALUES */
	short view, dura, flag, hold;	/* hold: minimum tijd, dura=teller */

	NetLink *nl;				/* geeft aan dat-e afspeelt */

	float lens, loc[3], rot[3];	/* oorspronkelijke */
	float sta, end;				/* net clip */
	float clipsta, clipend;		/* oorspronkelijke */
	
	short cfie, sfra;			/* cfie: tijdens nl afspelen van 0->tot, sfra: offset in 'movie' */
} CamPos;

#
#
typedef struct {
	float rot[3], loc[3];
	float lens;
	float clipsta, clipend;
} CamFrame;

typedef struct LBuf {
	short tot, max;
	Object **ob;
} LBuf;

typedef struct Sector {
	ID id;

	Object *ob;
	short type, flag, totport, actport;
	
		/* bbsize= boundbox size in wereld coords */
	float size[3], bbsize[3];
	float r, g, b;
	
	struct Portal *portals;
	LBuf lbuf;
	CamPos *campos;
	CamFrame *camframe;
	short totcam, cfra, totfra, sfra, depth, rt;	/* depth: van recursieve visible test */
	
	ListBase ipo;
	
	Mesh *dynamesh;
	Mesh *texmesh;
	
} Sector;


typedef struct Portal {

	Sector *sector;
	Object *ob;
	
	short type, flag;
	float ofs[2], max[2];
	
} Portal;

/* TFace is een aanvulling op MFace */

typedef struct TFace {
	short uv[4][2];		/* als je dit wijzigt: ook fie set_correct_uv editmesh.c, ook andere plekken maken gebruik van de lengte van dit blok */
	ulong col[4];
	short no[3], flag;
	short mode, tile;
	void *tpage, *clut;
	
} TFace;

#
#
typedef struct DFace {
		/* alleen gedurende simulatie */
	float *v1, *v2, *v3, *v4;
	float dist;
	Material *ma;
	float no[3];
	short proj;
	char flag, edcode;
	ushort ocx, ocy, ocz, rt;
	
} DFace;

typedef struct Life {
	ID id;
	Sector *sector;
	
	char type, lay;
	short flag;
	float oldloc[3], loc[3], speed[3];		/* loc ook in object zetten */
	float oldloc1[3], loc1[3], speed1[3];	/* lokale sector co's */
	float startloc[3], startrot[3];
	float rot[3], rotspeed[3];
	float oldimat[4][4];
	float mass, frict, rotfrict, axsize, frictfac;
	float r, g, b;
	
	short totsens, actsens;
	Sensor *sensors;
	
	short timer, sfra, cfra, dflag;		/* voor init/afhandeling ipoos, dflag: zit niet aan buttons */
	short state[4];						/* lokale variables */
	Material *contact;
	Object *collision, *from;
	float colloc[3];					/* collision loc */
	DFace *floor;
	float floorloc[3];
	
	LBuf links;							/* tijdens simul: de kinderen, in volgorde */
	
	ListBase ipo;
	
	Mesh *dynamesh;
	Mesh *texmesh, *oldmesh;
} Life;


#define TOLER 0.0000076

	/* life.c */
extern Life *add_life();
extern Life *copy_life(Life *lf);

	/* sector.c */
extern short simuldevs[32], simulvals[32];
extern Sector *add_sector();
extern Sector *copy_sector(Sector *se);
extern Sector *find_sector(float *loc, float *local);


#endif /* SECTOR_H */

