/* screen.h    dec 93 jan 94 */

/*
 * 
 *	LET OP: util.h (ListBase) en graphics.h (vec2s) ook nodig
 * 
 */
 
#define MAXQUEUE 256

typedef struct Screen {
	struct Screen *next, *prev;
	short startx, endx, starty, endy;	/* framebuffer coords */
	short sizex, sizey;
	short verts_nr, edges_nr, areas_nr;
	struct ListBase vertbase, edgebase, areabase;
	short mainwin, winakt;
} Screen;

typedef struct ScrVert {
	struct ScrVert *next, *prev, *new;
	vec2s vec;
	int flag;
} ScrVert;

typedef struct ScrEdge {
	struct ScrEdge *next, *prev;
	ScrVert *v1, *v2;
	short border;		/* 1 als op rand screen */
	short flag;
} ScrEdge;

typedef struct ScrArea {
	struct ScrArea *next, *prev;
	ScrVert *v1, *v2, *v3, *v4;
	short headwin, win;
	short headertype;	/* 0=niets, 1= down, 2= up */
	short areatype;		/* 0= niets */
	rcti totrct, headrct, winrct;
	short *headqueue, *hq, *winqueue, *wq;
	
	void (*headdraw)(), (*windraw)();
	void (*headqread)(), (*winqread)();
} ScrArea;
