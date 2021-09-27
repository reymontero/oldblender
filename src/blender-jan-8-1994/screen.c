
/* screen.c  dec/jan 93/94		GRAPHICS
 * 
 * 
 * 
 * 
 * 
 */


#include "graphics.h"
#include "blender.h"
#include "screen.h"

/* TIPS:
 * 
 * - let op de edges,  vertices ervan moeten in volgorde
	 (laagste pointer eerst). Anders onvoorspelbare effecten!
 * - probleem: flags zijn nog niet echt netjes. Altijd na gebruik
	 op nul zetten.
 */

#define MAXWIN		128

/* Als je EDGEWIDTH verandert, ook globale array edcol[] goedzetten */
#define EDGEWIDTH	5
#define EDGEWIDTH2	((EDGEWIDTH-1)/2)
#define AREAGRID	4
#define AREAMINX	32
#define HEADERY		16
#define AREAMINY	(HEADERY+EDGEWIDTH)

#define NOHEADER	0
#define HEADERDOWN	1
#define HEADERTOP	2

#define EMPTY		0
#define VIEW3D		1


/* ********* Globals *********** */

struct ListBase screenbase= {0, 0};
Screen *curscreen= 0;
int displaysizex= 0, displaysizey= 0;
short mainwin=0;
ScrArea *areawinar[MAXWIN];

/* ulong edcol[EDGEWIDTH]= {0x0, 0x303030, 0x606060, 0x808080, 0x909090, 0xF0F0F0, 0x0}; */
ulong edcol[EDGEWIDTH]= {0x0, 0x505050, 0x909090, 0xF0F0F0, 0x0};

/* ********* Funkties *********** */

void getdisplaysize();
void setdisplaysize(int x, int y);
Screen *default_foursplit();
void setscreen(Screen *sc);
void drawscreen();
void initscreen();
void moveareas();

/* *********  IN/OUT  ************* */

void getmouseco_sc(short *mval)		/* screen coordinaten */
{
	static Device mdev[2]= {MOUSEX, MOUSEY};

	getdev(2, mdev, mval);
	mval[0]-= curscreen->startx;
	mval[1]-= curscreen->starty;
}

/* ********  WINDOW & QUEUE ********* */

void defheaddraw()
{
	cpack(0x606060);
	clear();
	swapbuffers();
}

void defwindraw()
{
	cpack(0x707070);
	/* cpack(rand()*7575); */
	clear();
	swapbuffers();
}

void defheadqread(ScrArea *sa)
{
	short event, val, do_redraw=0;
	
	winset(sa->headwin);
	
	while(sa->hq != sa->headqueue) {
		sa->hq-= 2;
		
		event= sa->hq[0];
		val= sa->hq[1];
		
		switch(event) {
		case REDRAW:
			do_redraw= 1;
			break;
		default:
			if(sa->headqread) sa->headqread(event, val);
		}
	}
	
	if(do_redraw) sa->headdraw();
}

void defwinqread(ScrArea *sa)
{
	short event, val, do_redraw=0;
	
	winset(sa->win);
	
	while(sa->wq != sa->winqueue) {
		sa->wq-= 2;
		
		event= sa->wq[0];
		val= sa->wq[1];
		
		switch(event) {
		case REDRAW:
			do_redraw= 1;
			break;
		default:
			if(sa->winqread) sa->winqread(event, val);
		}
	}
	
	if(do_redraw) sa->windraw();
}

void addqueue(short win, short event, short val)
{
	ScrArea *sa;
	short *end;
	
	sa= areawinar[win];
	if(sa) {
		if(win==sa->headwin) {
			end= sa->headqueue+MAXQUEUE;
			if( (long)sa->hq < (long)end) {
				sa->hq[0]= event;
				sa->hq[1]= val;
				sa->hq+= 2;
			}
		}
		else if(win==sa->win) {
			end= sa->winqueue+MAXQUEUE;
			if( (long)sa->wq < (long)end) {
				sa->wq[0]= event;
				sa->wq[1]= val;
				sa->wq+= 2;
			}
		}
	}
}

void screenmain()
{
	ScrArea *sa;
	long wx, wy, orx, ory;
	short event, val, towin;
	
	while(TRUE) {
		event= qread(&val);
		towin= 1;
		
		switch(event) {
		
		case LEFTMOUSE:
			if(curscreen->winakt==mainwin) {
				if(getbutton(LEFTALTKEY)) moveareas();
				towin= 0;
			}
			break;
		case INPUTCHANGE:
			curscreen->winakt= val;
			towin= 0;
			break;
		case REDRAW:
			towin= 0;
			if(val==mainwin) {
				winset(mainwin);
				getsize(&wx, &wy);
				getorigin(&orx, &ory);
				curscreen->startx= orx; curscreen->starty= ory;
				curscreen->endx= wx+orx-1; curscreen->endy= wy+ory-1;
				curscreen->sizex= wx;
				curscreen->sizey= wy;
				drawscreen();
			}
			else if(val) {
				addqueue(val, REDRAW, val);
			}
			break;
		}

		if(towin && curscreen->winakt) {
			addqueue(curscreen->winakt, event, val);
		}
		
		/* window queues */
		if(qtest()==0) {
			sa= curscreen->areabase.first;
			while(sa) {
				if(sa->headwin && sa->headqueue!=sa->hq) defheadqread(sa);
				if(sa->win && sa->winqueue!=sa->wq) defwinqread(sa);
				sa= sa->next;
			}
		}
	}
}

/* *********  AREAS  ************* */

void getdisplaysize()
{
	if(displaysizex==0) {	/* anders is setdisplaysize gebruikt */
		displaysizex= getgdesc(GD_XPMAX);
		displaysizey= getgdesc(GD_YPMAX);
	}
}


void setdisplaysize(int x, int y)
{
	displaysizex= x;
	displaysizey= y;
}


void setscreen(Screen *sc)
{
	curscreen= sc;
}


ScrArea *findcurarea()
{
	ScrArea *sa;
	short mval[2];
	
	getmouseco_sc(mval);
	sa= curscreen->areabase.first;
	while(sa) {
		if(sa->v1->vec.x<=mval[0] && sa->v3->vec.x>=mval[0]) {
			if(sa->v1->vec.y<=mval[1] && sa->v2->vec.y>=mval[1]) {
				return sa;
			}
		}
		sa= sa->next;
	}
	return 0;
}

ScrVert *addscrvert(struct ListBase *lb, short x, short y)
{
	ScrVert *sv;
	
	sv= callocN(sizeof(ScrVert), "addscrvert");
	sv->vec.x= x;
	sv->vec.y= y;
	
	if(lb) addtail(lb, sv);
	
	return sv;
}

ScrEdge *addscredge(struct ListBase *lb, ScrVert *v1, ScrVert *v2)
{
	ScrEdge *se;
	
	se= callocN(sizeof(ScrEdge), "addscredge");
	if( (long)v1 > (long)v2 ) {
		se->v1= v2;
		se->v2= v1;
	}
	else {
		se->v1= v1;
		se->v2= v2;
	}
	
	if(lb) addtail(lb, se);

	return se;
}

ScrEdge *findscredge(ScrVert *v1, ScrVert *v2)
{
	ScrVert *sv;
	ScrEdge *se;
	
	if( (long)v1 > (long)v2 ) {
		sv= v1;
		v1= v2;
		v2= sv;
	}
	se= curscreen->edgebase.first;
	while(se) {
		if(se->v1==v1 && se->v2==v2) return se;
		se= se->next;
	}
	return 0;
}

void removedouble_scrverts()
{
	ScrVert *v1, *verg;
	ScrEdge *se;
	ScrArea *sa;
	
	verg= curscreen->vertbase.first;
	while(verg) {
		if(verg->new==0) {	/* !!! */
			v1= verg->next;
			while(v1) {
				if(v1->new==0) {	/* !?! */
					if(v1->vec.x==verg->vec.x && v1->vec.y==verg->vec.y) {
						/* printf("doublevert\n"); */
						v1->new= verg;
					}
				}
				v1= v1->next;
			}
		}
		verg= verg->next;
	}
	
	/* vervang pointers in edges en vlakken */
	se= curscreen->edgebase.first;
	while(se) {
		if(se->v1->new) se->v1= se->v1->new;
		if(se->v2->new) se->v2= se->v2->new;
		/* edges zijn veranderd: dus.... */
		if( (long)se->v1 > (long)se->v2 ) {
			v1= se->v1;
			se->v1= se->v2;
			se->v2= v1;
		}
		se= se->next;
	}
	sa= curscreen->areabase.first;
	while(sa) {
		if(sa->v1->new) sa->v1= sa->v1->new;
		if(sa->v2->new) sa->v2= sa->v2->new;
		if(sa->v3->new) sa->v3= sa->v3->new;
		if(sa->v4->new) sa->v4= sa->v4->new;
		sa= sa->next;
	}
	
	/* verwijderen */
	verg= curscreen->vertbase.first;
	while(verg) {
		v1= verg->next;
		if(verg->new) {
			remlink(&curscreen->vertbase, verg);
			freeN(verg);
		}
		verg= v1;
	}
	
}

void removenotused_scrverts()
{
	ScrVert *sv, *svn;
	ScrEdge *se;
	ScrArea *sa;

	/* ga ervan uit dat de edges goed zijn */
	
	se= curscreen->edgebase.first;
	while(se) {
		se->v1->flag= 1;
		se->v2->flag= 1;
		se= se->next;
	}
	
	sv= curscreen->vertbase.first;
	while(sv) {
		svn= sv->next;
		if(sv->flag==0) {
			remlink(&curscreen->vertbase, sv);
			freeN(sv);
		}
		else sv->flag= 0;
		sv= svn;
	}
}

void removedouble_scredges()
{
	ScrVert *v1;
	ScrEdge *verg, *se, *sn;
	
	/* vergelijken */
	verg= curscreen->edgebase.first;
	while(verg) {
		se= verg->next;
		while(se) {
			sn= se->next;
			if(verg->v1==se->v1 && verg->v2==se->v2) {
				/* printf("edge removed \n"); */
				remlink(&curscreen->edgebase, se);
				freeN(se);
			}
			se= sn;
		}
		verg= verg->next;
	}
}

void removenotused_scredges()
{
	ScrVert *sv;
	ScrEdge *se, *sen;
	ScrArea *sa;
	int a=0;
	
	/* zet flag als edge gebruikt wordt in area */
	sa= curscreen->areabase.first;
	while(sa) {
		se= findscredge(sa->v1, sa->v2);
		if(se==0) printf("error: area %d edge 1 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v2, sa->v3);
		if(se==0) printf("error: area %d edge 2 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v3, sa->v4);
		if(se==0) printf("error: area %d edge 3 bestaat niet\n", a);
		else se->flag= 1;
		se= findscredge(sa->v4, sa->v1);
		if(se==0) printf("error: area %d edge 4 bestaat niet\n", a);
		else se->flag= 1;
		sa= sa->next;
		a++;
	}
	se= curscreen->edgebase.first;
	while(se) {
		sen= se->next;
		if(se->flag==0) {
			remlink(&curscreen->edgebase, se);
			freeN(se);
		}
		else se->flag= 0;
		se= sen;
	}
}

void calc_arearcts(ScrArea *sa)
{

	if(sa->v1->vec.x>0) sa->totrct.xmin= sa->v1->vec.x+EDGEWIDTH2+1;
	else sa->totrct.xmin= sa->v1->vec.x;
	if(sa->v4->vec.x<curscreen->sizex-1) sa->totrct.xmax= sa->v4->vec.x-EDGEWIDTH2-1;
	else sa->totrct.xmax= sa->v4->vec.x;
	
	if(sa->v1->vec.y>0) sa->totrct.ymin= sa->v1->vec.y+EDGEWIDTH2+1;
	else sa->totrct.ymin= sa->v1->vec.y;
	if(sa->v2->vec.y<curscreen->sizey-1) sa->totrct.ymax= sa->v2->vec.y-EDGEWIDTH2-1;
	else sa->totrct.ymax= sa->v2->vec.y;
		
	sa->winrct= sa->totrct;
	if(sa->headertype) {
		sa->headrct= sa->totrct;
		if(sa->headertype==HEADERDOWN) {
			sa->headrct.ymax= sa->headrct.ymin+HEADERY;
			sa->winrct.ymin= sa->headrct.ymax+1;
		}
		else if(sa->headertype==HEADERTOP) {
			sa->headrct.ymin= sa->headrct.ymax-HEADERY;
			sa->winrct.ymax= sa->headrct.ymin-1;
		}
	}
}

ScrArea *addscrarea(lb, v1, v2, v3, v4, headertype, areatype)
struct ListBase *lb;
ScrVert *v1, *v2, *v3, *v4;
short headertype, areatype;
{
	ScrArea *sa;
	
	sa= callocN(sizeof(ScrArea), "addscrarea");
	sa->v1= v1;
	sa->v2= v2;
	sa->v3= v3;
	sa->v4= v4;
	sa->headertype= headertype;
	sa->areatype= areatype;

	calc_arearcts(sa);
	
	if(headertype) {
		sa->headwin= swinopen(curscreen->mainwin);
		if(sa->headwin>=MAXWIN) {
			error("No more windows");
			winclose(sa->headwin);
			sa->headwin= 0;
		}
		else  {
			/* winposition() VOOR gconfig() geeft vieze effecten bij openen window */
			RGBmode();
			doublebuffer();
			gconfig();
			winposition(sa->headrct.xmin, sa->headrct.xmax, sa->headrct.ymin, sa->headrct.ymax);

			reshapeviewport();
			mmode(MVIEWING);
			ortho2(-0.5, sa->headrct.xmax-sa->headrct.xmin+0.5, -0.5, sa->headrct.ymax-sa->headrct.ymin+0.5);
			
			sa->headqueue= sa->hq= mallocN(2*MAXQUEUE, "addscrarea2");
			sa->headdraw= defheaddraw;
			sa->headqread= 0;
		}
	}
	if(sa->winrct.ymin<sa->winrct.ymax) {
		sa->win= swinopen(curscreen->mainwin);
		if(sa->win>=MAXWIN) {
			error("No more windows");
			winclose(sa->win);
			sa->win= 0;
		}
		else {
			/* winposition() VOOR gconfig() geeft vieze effecten bij openen window */
			RGBmode();
			doublebuffer();
			gconfig();
			winposition(sa->winrct.xmin, sa->winrct.xmax, sa->winrct.ymin, sa->winrct.ymax);
		
			reshapeviewport();
			mmode(MVIEWING);
			ortho2(-0.5, sa->winrct.xmax-sa->winrct.xmin+0.5, -0.5, sa->winrct.ymax-sa->winrct.ymin+0.5);

			sa->winqueue= sa->wq= mallocN(2*MAXQUEUE, "addscrarea2");
			sa->windraw= defwindraw;
			sa->winqread= 0;
		}
	}

	if(lb) addtail(lb, sa);

	return sa;
}

Screen *addscreen(short startx, short starty, short endx, short endy)
{
	Screen *sc;
	ScrVert *sv1, *sv2, *sv3, *sv4;
	ScrEdge *se;
	ScrArea *sa;
	
	sc= callocN(sizeof(Screen), "addscreen");
	addtail(&screenbase, sc);
	curscreen= sc;

	sc->startx= startx;	sc->starty= starty;
	sc->endx= endx;	sc->endy= endy;
	sc->sizex= sc->endx-sc->startx+1;
	sc->sizey= sc->endy-sc->starty+1;
	sc->verts_nr= 4;
	sc->edges_nr= 4;
	sc->areas_nr= 1;
	
	if(mainwin==0) {
		prefposition(sc->startx, sc->endx, sc->starty, sc->endy);
		noborder();
		sc->mainwin= mainwin= winopen("Blender");
		RGBmode();
		doublebuffer();
		gconfig();
		mmode(MSINGLE);	/* dan kan bij fullscrn() de matrix gepusht */
		ortho2(-0.5, curscreen->sizex-0.5, -0.5, curscreen->sizey-0.5);
	}
	
	sv1= addscrvert(&sc->vertbase, 0, 0);
	sv2= addscrvert(&sc->vertbase, 0, sc->endy-sc->starty);
	sv3= addscrvert(&sc->vertbase, sc->sizex-1, sc->sizey-1);
	sv4= addscrvert(&sc->vertbase, sc->sizex-1, 0);
	
	se= addscredge(&sc->edgebase, sv1, sv2);
	se= addscredge(&sc->edgebase, sv2, sv3);
	se= addscredge(&sc->edgebase, sv3, sv4);
	se= addscredge(&sc->edgebase, sv4, sv1);
	
	sa= addscrarea(&sc->areabase, sv1, sv2, sv3, sv4, HEADERDOWN, EMPTY);
	
	return sc;
}

void del_area(ScrArea *sa)
{
	if(sa->headwin) winclose(sa->headwin);
	if(sa->headqueue) {
		freeN(sa->headqueue);
		sa->headqueue= 0;
	}
	if(sa->win) winclose(sa->win);
	if(sa->winqueue) {
		freeN(sa->winqueue);
		sa->winqueue= 0;
	}
	
}

void deletescreen(Screen *sc)
{
	ScrArea *sa;
	
	freelistN(&sc->vertbase);
	freelistN(&sc->edgebase);
	sa= sc->areabase.first;
	while(sa) {
		del_area(sa);
		sa= sa->next;
	}
	freelistN(&sc->areabase);
	remlink(&screenbase, sc);
	freeN(sc);
}

void freescreens()
{
	Screen *sc, *scn;
	
	sc= screenbase.first;
	while(sc) {
		scn= sc->next;
		deletescreen(sc);
		sc= scn;
	}
}

void testareas()
{
	ScrArea *sa;
	rcti temp;
	
	/* testen of window nog klopt met screenverts */
	sa= curscreen->areabase.first;
	while(sa) {
		
		temp= sa->totrct;
		calc_arearcts(sa);

		if(temp.xmin!=sa->totrct.xmin || temp.xmax!=sa->totrct.xmax ||
			temp.ymin!=sa->totrct.ymin || temp.ymax!=sa->totrct.ymax) {
			
			if(sa->headwin) {
				winset(sa->headwin);
				winposition(sa->headrct.xmin, sa->headrct.xmax, sa->headrct.ymin, sa->headrct.ymax);
				reshapeviewport();
			}
		/* hier testen of window te klein is geworden: deleten, of andersom: een annmaken */
			if(sa->win) {
				winset(sa->win);
				winposition(sa->winrct.xmin, sa->winrct.xmax, sa->winrct.ymin, sa->winrct.ymax);
				reshapeviewport();
			}
		}
		sa= sa->next;
	}
	
	/* remake global windowarray */
	sa= curscreen->areabase.first;
	while(sa) {
		if(sa->headwin) areawinar[sa->headwin]= sa;
		if(sa->win) areawinar[sa->win]= sa;
		sa= sa->next;
	}
	
}


void joinarea(ScrArea *sa, char dir)
{
	ScrArea *sa2, *san;
	ScrEdge *se1, *se2;
	
	/* welke edge? */
	if(dir=='l') se1= findscredge(sa->v1, sa->v2); 		/* left */
	else if(dir=='u') se1= findscredge(sa->v2, sa->v3);	/* up */
	else if(dir=='r') se1= findscredge(sa->v3, sa->v4);	/* right */
	else se1= findscredge(sa->v4, sa->v1);				/* down */
	
	if(se1==0) {
		printf("couldn't find edge in joinarea\n");
		return;
	}
	
	/* vind area met zelfde edge */
	sa2= curscreen->areabase.first;
	while(sa2) {
		if(sa2 != sa) {
			if(dir=='r') se2= findscredge(sa2->v1, sa2->v2); 		/* left */
			else if(dir=='d') se2= findscredge(sa2->v2, sa2->v3);	/* top */
			else if(dir=='l') se2= findscredge(sa2->v3, sa2->v4);	/* right */
			else se2= findscredge(sa2->v4, sa2->v1);				/* down */
			if(se1==se2) break;
		}
		sa2= sa2->next;
	}
	
	if(sa2==0) error("Can't join areas");
	else {
		/* nieuwe area is oude sa */
		if(dir=='l') {
			sa->v1= sa2->v1;
			sa->v2= sa2->v2;
			addscredge(&curscreen->edgebase, sa->v2, sa->v3);
			addscredge(&curscreen->edgebase, sa->v1, sa->v4);
		}
		else if(dir=='u') {
			sa->v2= sa2->v2;
			sa->v3= sa2->v3;
			addscredge(&curscreen->edgebase, sa->v1, sa->v2);
			addscredge(&curscreen->edgebase, sa->v3, sa->v4);
		}
		else if(dir=='r') {
			sa->v3= sa2->v3;
			sa->v4= sa2->v4;
			addscredge(&curscreen->edgebase, sa->v2, sa->v3);
			addscredge(&curscreen->edgebase, sa->v1, sa->v4);
		}
		else {
			sa->v1= sa2->v1;
			sa->v4= sa2->v4;
			addscredge(&curscreen->edgebase, sa->v1, sa->v2);
			addscredge(&curscreen->edgebase, sa->v3, sa->v4);
		}
		
		/* edge en area weg */
		remlink(&curscreen->edgebase, se2);
		freeN(se2);
		del_area(sa2);
		remlink(&curscreen->areabase, sa2);
		freeN(sa2);
		
		removedouble_scredges();
		removenotused_scredges();
		removenotused_scrverts();	/* moet als laatste */
		
		testareas();
	}
}

short testsplitpoint(ScrArea *sa, char dir, float fac)
/* return 0: geen split mogelijk */
/* else return (integer) screencoordinaat splitpunt */
{
	short val, x, y;
	
	/* area groot genoeg? */
	val= 0;
	if(sa->v4->vec.x- sa->v1->vec.x <= 2*AREAMINX) return 0;
	if(sa->v2->vec.y- sa->v1->vec.y <= 2*AREAMINY) return 0;

	/* voor zekerheid */
	if(fac<0.0) fac= 0.0;
	if(fac>1.0) fac= 1.0;
	
	if(dir=='h') {
		y= sa->v1->vec.y+ fac*(sa->v2->vec.y- sa->v1->vec.y);
		y-= (y % AREAGRID);
		if(y- sa->v1->vec.y < AREAMINY) y= sa->v1->vec.y+ AREAMINY;
		if(sa->v2->vec.y- y < AREAMINY) y= sa->v2->vec.y- AREAMINY;
		return y;
	}
	else {
		x= sa->v1->vec.x+ fac*(sa->v4->vec.x- sa->v1->vec.x);
		x-= (x % AREAGRID);
		if(x- sa->v1->vec.x < AREAMINX) x= sa->v1->vec.x+ AREAMINX;
		if(sa->v4->vec.x- x < AREAMINX) x= sa->v4->vec.x- AREAMINX;
		return x;
	}
}

void splitarea(ScrArea *sa, char dir, float fac)
{
	Screen *sc;
	ScrVert *sv1, *sv2;
	short split;
	
	if(sa==0) return;
	
	split= testsplitpoint(sa, dir, fac);
	if(split==0) return;
	
	sc= curscreen;
	
	if(dir=='h') {
		/* nieuwe vertices */
		sv1= addscrvert(&sc->vertbase, sa->v1->vec.x, split);
		
		sv2= addscrvert(&sc->vertbase, sa->v4->vec.x, split);
		
		/* nieuwe edges */
		addscredge(&sc->edgebase, sa->v1, sv1);
		addscredge(&sc->edgebase, sv1, sa->v2);
		addscredge(&sc->edgebase, sa->v3, sv2);
		addscredge(&sc->edgebase, sv2, sa->v4);
		addscredge(&sc->edgebase, sv1, sv2);
		
		/* nieuwe areas */
		del_area(sa);
		addscrarea(&sc->areabase, sa->v1, sv1, sv2, sa->v4, sa->headertype, sa->areatype);
		addscrarea(&sc->areabase, sv1, sa->v2, sa->v3, sv2, sa->headertype, sa->areatype);

		remlink(&sc->areabase, sa);
		freeN(sa);
	}
	else {
		/* nieuwe vertices */
		sv1= addscrvert(&sc->vertbase, split, sa->v1->vec.y);
		
		sv2= addscrvert(&sc->vertbase, split, sa->v2->vec.y);
		
		/* nieuwe edges */
		addscredge(&sc->edgebase, sa->v1, sv1);
		addscredge(&sc->edgebase, sv1, sa->v4);
		addscredge(&sc->edgebase, sa->v2, sv2);
		addscredge(&sc->edgebase, sv2, sa->v3);
		addscredge(&sc->edgebase, sv1, sv2);
		
		/* nieuwe areas */
		del_area(sa);
		addscrarea(&sc->areabase, sa->v1, sa->v2, sv2, sv1, sa->headertype, sa->areatype);
		addscrarea(&sc->areabase, sv1, sv2, sa->v3, sa->v4, sa->headertype, sa->areatype);
	
		remlink(&sc->areabase, sa);
		freeN(sa);
	
	}
	
	/* dubbele vertices en edges verwijderen */
	removedouble_scrverts();
	removedouble_scredges();
	removenotused_scredges();
	
	testareas();
}

float editsplitpoint(ScrArea *sa, char dir)
{
	float fac;
	short event, val, split, mval[2], mvalo[2]={-1010, -1010};
	
	if(sa->win) winset(sa->win);
	else if(sa->headwin) winset(sa->headwin);
	else winset(curscreen->mainwin);
	
	drawmode(OVERDRAW);
	
	/* rekening houden met grid en minsize */
	while(1) {
		getmouseco_sc(mval);
		
		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];
			
			if(dir=='h') {
				fac= mval[1]- sa->v1->vec.y;
				fac/= sa->v2->vec.y- sa->v1->vec.y;
			} else {
				fac= mval[0]- sa->v1->vec.x;
				fac/= sa->v4->vec.x- sa->v1->vec.x;
			}
			
			split= testsplitpoint(sa, dir, fac);
			if(split==0) {
				error("Area too small");
				return 0.0;
			}
			color(0); clear(); color(1);

			if(dir=='h') sboxs(0, split-sa->totrct.ymin, sa->totrct.xmax - sa->totrct.xmin, split-sa->totrct.ymin);
			else sboxs(split-sa->totrct.xmin, 0, split-sa->totrct.xmin, sa->totrct.ymax - sa->totrct.ymin);
		}
		
		if(qtest()) {
			event= qread(&val);
			if(val && event==LEFTMOUSE) {
				if(dir=='h') {
					fac= split- sa->v1->vec.y;
					fac/= sa->v2->vec.y- sa->v1->vec.y;
				}
				else {
					fac= split- sa->v1->vec.x;
					fac/= sa->v4->vec.x- sa->v1->vec.x;
				}
				color(0); clear();
				drawmode(NORMALDRAW);
				return fac;
			}
			if(val && event==ESCKEY) {
				color(0); clear();
				drawmode(NORMALDRAW);
				return 0.0;
			}
		}
		sginap(1);
	}
	
}

void moveareas()
{
	ScrEdge *se, *firsted=0;
	ScrVert *v1;
	ScrArea *sa;
	vec2s addvec;
	float PdistVL2Dfl(), vec1[2], vec2[2], vec3[2];
	int dist, mindist= 1<<30, oneselected;
	short event=0, val, split, mval[2], mvalo[2], x1, x2, y1, y2, bigger, smaller;
	char dir;
	
	/* welke is de aangegeven edge? */
	getmouseco_sc(mvalo);
	vec1[0]= mvalo[0];
	vec1[1]= mvalo[1];
	
	se= curscreen->edgebase.first;
	while(se) {
		if(se->border==0) {
			vec2[0]= se->v1->vec.x;
			vec2[1]= se->v1->vec.y;
			vec3[0]= se->v2->vec.x;
			vec3[1]= se->v2->vec.y;
			dist= PdistVL2Dfl(vec1, vec2, vec3);
			
			if(dist<mindist) {
				mindist= dist;
				firsted= se;
			}
		}
		se= se->next;
	}
	if(mindist > 30) return;
	
	/* select connected, alleen in de juiste richting */
	/* 'dir' is de richting van de EDGE */
	firsted->v1->flag= 1;
	firsted->v2->flag= 1;
	if(firsted->v1->vec.x==firsted->v2->vec.x) dir= 'v';
	else dir= 'h';
	
	oneselected= 1;
	while(oneselected) {
		se= curscreen->edgebase.first;
		oneselected= 0;
		while(se) {
			if(se->v1->flag+ se->v2->flag==1) {
				if(dir=='h') if(se->v1->vec.y==se->v2->vec.y) {
					se->v1->flag= se->v2->flag= 1;
					oneselected= 1;
				}
				if(dir=='v') if(se->v1->vec.x==se->v2->vec.x) {
					se->v1->flag= se->v2->flag= 1;
					oneselected= 1;
				}
			}
			se= se->next;
		}
	}
	
	/* nu zijn alle vertices met 'flag==1' degene die verplaatst kunnen worden. */
	/* we lopen de areas af en testen vrije ruimte met MINSIZE */
	bigger= smaller= 10000;
	sa= curscreen->areabase.first;
	while(sa) {
		if(dir=='h') {	/* als top of down edge select, test hoogte */
			if(sa->v1->flag && sa->v4->flag) {
				y1= sa->v2->vec.y - sa->v1->vec.y-AREAMINY;
				if(y1<bigger) bigger= y1;
			}
			else if(sa->v2->flag && sa->v3->flag) {
				y1= sa->v2->vec.y - sa->v1->vec.y-AREAMINY;
				if(y1<smaller) smaller= y1;
			}
		}
		else {	/* als left of right edge select, test breedte */
			if(sa->v1->flag && sa->v2->flag) {
				x1= sa->v4->vec.x - sa->v1->vec.x-AREAMINX;
				if(x1<bigger) bigger= x1;
			}
			else if(sa->v3->flag && sa->v4->flag) {
				x1= sa->v4->vec.x - sa->v1->vec.x-AREAMINX;
				if(x1<smaller) smaller= x1;
			}
		}
		sa= sa->next;
	}
	winset(mainwin);
	pushmatrix(); pushviewport();
	
	fullscrn();
	drawmode(OVERDRAW);
	
	getmouseco_sc(mvalo);
	addvec.x= 0;
	addvec.y= 0;
	while(getbutton(LEFTMOUSE)) {
		getmouseco_sc(mval);
		
		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
			
			if(dir=='h') {
				addvec.y+= mval[1]-mvalo[1];
				if(addvec.y>bigger) addvec.y= bigger;
				if(addvec.y<-smaller) addvec.y= -smaller;
			}
			else {
				addvec.x+= mval[0]-mvalo[0];
				if(addvec.x>bigger) addvec.x= bigger;
				if(addvec.x<-smaller) addvec.x= -smaller;
			}
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];

			/* edges tekenen */
			color(0); clear();
			color(1);
			se= curscreen->edgebase.first;
			while(se) {
				if(se->v1->flag && se->v2->flag) {
					/* met areagrid even behelpen, verderop is OK! */
					x1= curscreen->startx+se->v1->vec.x+addvec.x-(addvec.x % AREAGRID);
					x2= curscreen->startx+se->v2->vec.x+addvec.x-(addvec.x % AREAGRID);
					y1= curscreen->starty+se->v1->vec.y+addvec.y-(addvec.y % AREAGRID);
					y2= curscreen->starty+se->v2->vec.y+addvec.y-(addvec.y % AREAGRID);
					sboxs(x1, y1, x2, y2);
				}
				se= se->next;
			}
		}
		
		if(qtest()) {
			event= qread(&val);
			if(val && event==ESCKEY) {
				break;
			}
		}
		sginap(1);
	}
	
	v1= curscreen->vertbase.first;
	while(v1) {
		if(v1->flag && event!=ESCKEY) {
		
			/* zo is AREAGRID netjes */
			if(addvec.x && v1->vec.x>0 && v1->vec.x<curscreen->sizex-1) {
				v1->vec.x+= addvec.x;
				if(addvec.x != bigger && addvec.x != -smaller) v1->vec.x-= (v1->vec.x % AREAGRID);
			}
			if(addvec.y && v1->vec.y>0 && v1->vec.y<curscreen->sizey-1) {
				v1->vec.y+= addvec.y;
				if(addvec.y != bigger && addvec.y != -smaller) v1->vec.y-= (v1->vec.y % AREAGRID);
			}
		}
		v1->flag= 0;
		v1= v1->next;
	}
	
	color(0); clear();
	drawmode(NORMALDRAW);
	endfullscrn();
	
	winset(mainwin);
	popmatrix(); popviewport();

	removedouble_scrverts();
	removedouble_scredges();

	testareas();

	drawscreen();
}

Screen *default_foursplit() 
{
	Screen *sc;
	ScrArea *sa;


	sc= addscreen(200, 200, displaysizex-1, displaysizey-1);

	splitarea( (ScrArea *)sc->areabase.first, 'h', 0.2);
	splitarea( (ScrArea *)sc->areabase.last, 'h', 0.5);
	splitarea( (ScrArea *)sc->areabase.last, 'h', 0.99);
			/* alle nieuwe areas komen op einde lijst! */
	sa= sc->areabase.first;
	splitarea( sa->next, 'v', 0.5);
	sa= sc->areabase.first;
	splitarea( sa->next, 'v', 0.5);
	
	sa= sc->areabase.first;
	sa->headertype= HEADERTOP;
	
	return sc;
}


void drawscredge(ScrEdge *se)
{
	Screen *sc;
	vec2s v1, v2;
	int a, dir;
	
	sc= curscreen;
	
	v1= se->v1->vec;
	v2= se->v2->vec;
	
	/* borders screen niet tekenen */
	se->border= 1;
	if(v1.x<=0 && v2.x<=0) return;
	if(v1.x>=sc->sizex-1 && v2.x>=sc->sizex-1) return;
	if(v1.y<=0 && v2.y<=0) return;
	if(v1.y>=sc->sizey-1 && v2.y>=sc->sizey-1) return;
	se->border= 0;

	if(v1.x== v2.x) {		/* vertikaal */
		dir= v1.y-v2.y;
		if(dir>0) dir= 1; else dir= -1;
		
		v1.y-= dir*EDGEWIDTH2;
		v2.y+= dir*EDGEWIDTH2;
		
		v1.x+= EDGEWIDTH2;
		v2.x+= EDGEWIDTH2;
		for(a=0; a<EDGEWIDTH; a++) {
			cpack(edcol[a]);
			LINE2S(&v1, &v2);
			v1.x--; v2.x--;
					/* en dit voor de afgeronde tuitjes */
			if(a<EDGEWIDTH2) { v1.y+= dir; v2.y-=dir;}
			else { v1.y-= dir; v2.y+=dir;}
		}
		
	}
	else {					/* horizontaal */
		dir= v1.x-v2.x;
		if(dir>0) dir= 1; else dir= -1;
		v1.x-= dir*EDGEWIDTH2;
		v2.x+= dir*EDGEWIDTH2;
		
		v1.y-= EDGEWIDTH2;
		v2.y-= EDGEWIDTH2;
		for(a=0; a<EDGEWIDTH; a++) {
			cpack(edcol[a]);
			LINE2S(&v1, &v2);
			v1.y++; v2.y++;
					/* en dit voor de afgeronde tuitjes */
			if(a<EDGEWIDTH2) { v1.x+= dir; v2.x-=dir;}
			else { v1.x-= dir; v2.x+=dir;}
		}
		
	}

}

void drawscreen()
{
	ScrEdge *se;

	winset(curscreen->mainwin);
	cpack(0x707070); clear();
	
	/* edges in mainwin */
	se= curscreen->edgebase.first;
	while(se) {
		drawscredge(se);
		se= se->next;
	}
	swapbuffers();
}

void initscreen()
{
	/* opent en initialiseert */
	/* eventueel defaultfile */
	Screen *sc;
	
	getdisplaysize();
	
	sc= default_foursplit();
	/* sc= curscreen= addscreen(0, 0, displaysizex-1, displaysizey-1); */
	
	
	qdevice(RAWKEYBD);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
	

	drawscreen();
	
}


maino()
{
	ScrArea *sa;
	ScrEdge *se;
	float fac;
	long wx, wy, orx, ory;
	short event, val;
	
	initscreen();
	
	screenmain();
	
	
	while(TRUE) {
		event= qread(&val);
		if(event) {
			if(val) {
				switch(event) {
				case LEFTMOUSE:
					if(getbutton(LEFTALTKEY)) {
						moveareas();
					}
					break;
				case JKEY:
					sa= findcurarea();
					if(sa) {
						event= pupmenu("Join %t|left %x1|right %x2|up %x3|down %x4");
						if(event>0) {
							if(event==1) joinarea( sa, 'l');
							else if(event==2) joinarea( sa, 'r');
							else if(event==3) joinarea( sa, 'u');
							else if(event==4) joinarea( sa, 'd');
							drawscreen();
						}
					}
					break;
				
				case HKEY:
					sa= findcurarea();
					if(sa) if(okee("Split horizontal")) {
						fac= editsplitpoint(sa, 'h');
						if(fac!=0.0) splitarea( sa, 'h', fac);
						drawscreen();
					}
					break;
				case VKEY:
					sa= findcurarea();
					if(sa) if(okee("Split vertical")) {
						fac= editsplitpoint(sa, 'v');
						if(fac!=0.0) splitarea( sa, 'v', fac);
						drawscreen();
					}
					break;
				case ESCKEY:
					exitblender();
				
				case REDRAW:
					if(val==mainwin) {
						winset(mainwin);
						getsize(&wx, &wy);
						getorigin(&orx, &ory);
						curscreen->startx= orx; curscreen->starty= ory;
						curscreen->endx= wx+orx-1; curscreen->endy= wy+ory-1;
						curscreen->sizex= wx;
						curscreen->sizey= wy;
						/* drawscreen(); */
					}
					break;
				case INPUTCHANGE:
					/* if(val) printf("INPUTCHANGE %d\n", val); */
					break;
				}
				/*
				se= curscreen->edgebase.first;
				val= 0;
				while(se) {
					if(se->border==0) val++;
					se= se->next;
				}
				*/
				
			}
		}
	}
}
