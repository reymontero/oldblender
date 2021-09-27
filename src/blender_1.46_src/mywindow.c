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



/* screen.c  juli 94		GRAPHICS
 * 
 * vervanging voor een aantal fie's zoals swinopen, winset,  (zie onder)
 * dit alles omdat GL en X te traag zijn
 * 
 * 
 */

/*			  let op: winid's beginnen met 4, eerste 3 voor GL! */


#include "blender.h"
#include "graphics.h"


/* undefs in verband met eeuwige loops! */
#undef winset
#undef winget
#undef ortho
#undef ortho2
#undef window
#undef winclose
#undef loadmatrix
#undef getmatrix
#undef multmatrix
#undef getsize
#undef getorigin


int swincount= 4, curswin=0;
bWindow *swinarray [MAXWIN]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bWindow mainwindow;
ListBase swindowbase= {0, 0};

void init_my_mainwin(int win)
{
	long orx, ory, sizex, sizey, endx, endy;

	winset(win);
	
	reshapeviewport();
	
	getsize(&sizex, &sizey);
	getorigin(&orx, &ory);

	endx= orx+sizex-1;
	endy= ory+sizey-1;

	mainwindow.xmin= orx;
	mainwindow.ymin= ory;
	mainwindow.xmax= endx;
	mainwindow.ymax= endy;
	
	ortho2(-0.5, sizex-0.5, -0.5, sizey-0.5);
	loadmatrix(matone);
		
	mmode(MPROJECTION);
	getmatrix(mainwindow.winmat);
	mmode(MVIEWING);
	getmatrix(mainwindow.viewmat);
	
	swinarray[win]= &mainwindow;
	mainwindow.parent_id= win;
	
	curswin= win;
}


void mygetsize(int *x, int *y)
{
	bWindow *win;
	
	if(curswin<4) {
		getsize((long *)x, (long *)y);
		return;
	}
	
	win= swinarray[curswin];
	if(win==0) return;

	*x= win->xmax-win->xmin+1;
	*y= win->ymax-win->ymin+1;
}

void mygetorigin(int *x, int *y)
{
	bWindow *win;
	
	getorigin((long *)x, (long *)y);	/* hoofdwindow */

	if(curswin>=4) {
		win= swinarray[curswin];
		if(win) {
			*x+= win->xmin;
			*y+= win->ymin;
		}
	}
}

void mygetsuborigin(int *x, int *y)
{
	bWindow *win;
	
	win= swinarray[curswin];
	if(win==0) return;
	
	*x= win->xmin;
	*y= win->ymin;
}

void myloadmatrix(float mat[][4])
{
	bWindow *win;
	int mode;

	loadmatrix(mat);

	win= swinarray[curswin];
	if(win==0) return;
	
	mode= getmmode();
	if(mode==MVIEWING) {
		Mat4CpyMat4(win->viewmat, mat);
	}
	else {
		Mat4CpyMat4(win->winmat, mat);
	}
}

void loadmatrix_win(float mat[][4], int swin)
{
	bWindow *win;
	int mode;

	win= swinarray[swin];
	if(win==0) return;
	
	mode= getmmode();
	if(mode==MVIEWING) {
		Mat4CpyMat4(win->viewmat, mat);
	}
	else {
		Mat4CpyMat4(win->winmat, mat);
	}
}

void mygetmatrix(float mat[][4])
{
	bWindow *win;
	int mode;

	win= swinarray[curswin];
	if(win==0) return;
	
	mode= getmmode();
	if(mode==MVIEWING) {
		Mat4CpyMat4(mat, win->viewmat);
	}
	else {
		Mat4CpyMat4(mat, win->winmat);
	}
}

void mymultmatrix(float mat[][4])
{
	bWindow *win;
	float tmat[4][4];

	win= swinarray[curswin];
	if(win==0) return;
	
	Mat4MulMat4(tmat, mat, win->viewmat);
	Mat4CpyMat4(win->viewmat, tmat);
	loadmatrix(tmat);

}

void mygetsingmatrix(float mat[][4])
{
	bWindow *win;
	float matview[4][4], matproj[4][4];
	int mode;

	win= swinarray[curswin];
	if(win==0) return;
	
	Mat4MulMat4(mat, win->viewmat, win->winmat);
	
}

int give_new_winid()
{
	int id= 0, a;
	
	a= 4;
	while(id==0 && a<MAXWIN) {
		if(swinarray[a]==0) id= a;
		a++;
	}

	return id;
}

int mywinget()
{
	return curswin;
}

void mywinset(int id)
{
	bWindow *win;

	if(id>1 && id<4) {
		winset(id);
		curswin= id;
		return;
	}
	win= swinarray[id];
	if(win==0) {
		printf("winset %d: doesn't exist\n", id);
		return;
	}
	
	winset(win->parent_id);
	
	if(win->parent_id == id)
		viewport(0, win->xmax-win->xmin, 0, win->ymax-win->ymin);
	else
		viewport(win->xmin, win->xmax, win->ymin, win->ymax);
	
	mmode(MPROJECTION);
	loadmatrix(win->winmat);
	mmode(MVIEWING);
	loadmatrix(win->viewmat);
	
	curswin= id;
}

int mywinexist(int id)
{
	bWindow *win;
	
	win= swinarray[id];
	if(win==0) return 0;
	else return 1;
}

int myswinopen(int winid, int xmin, int xmax, int ymin, int ymax)
{
	static int firsttime= 1;
	bWindow *win;
	
	win= callocN(sizeof(bWindow), "winopen");
	addtail(&swindowbase, win);
	
	swincount++;
	
	if(swincount>=MAXWIN) {
		printf("too many window\n");
		return;
	}
	
	win->parent_id= winid;
	win->id= give_new_winid();
	win->xmin= xmin;
	win->ymin= ymin;
	win->xmax= xmax;
	win->ymax= ymax;
	
	swinarray[win->id]= win;
	
	Mat4One(win->viewmat);
	Mat4One(win->winmat);
	
	mywinset(win->id);

	return win->id;
}

void mywinclose(int winid)
{
	bWindow *win;
	
	if(winid && winid<4) {
		winclose(winid);
		return;
	}

	win= swinarray[winid];
	if(win==0) {
		printf("error: close window %d, doesn't exist\n", winid);
		return;
	}
	remlink(&swindowbase, win);
	freeN(win);
	swinarray[winid]= 0;
	
	if(curswin==winid) curswin= 0;

	swincount--;
}

void mywinposition(int swin, int xmin, int xmax, int ymin, int ymax) /* let op: andere syntax */
{
	bWindow *win;
	
	win= swinarray[swin];
	if(win==0) return;
	
	win->xmin= xmin;
	win->ymin= ymin;
	win->xmax= xmax;
	win->ymax= ymax;
}


void myortho2(float x1, float x2, float y1, float y2)
{
	bWindow *win;
	
	
	ortho2(x1, x2, y1, y2);

	win= swinarray[curswin];
	if(win==0) return;
	
	mmode(MPROJECTION);
	getmatrix(win->winmat);
	mmode(MVIEWING);
}

void myortho(float x1, float x2, float y1, float y2, float n, float f)
{
	bWindow *win;
	
	ortho(x1, x2, y1, y2, n, f);

	win= swinarray[curswin];
	if(win==0) return;
	
	mmode(MPROJECTION);
	getmatrix(win->winmat);
	mmode(MVIEWING);
}

void mywindow(float x1, float x2, float y1, float y2, float n, float f)
{
	bWindow *win;

	window(x1, x2, y1, y2, n, f);

	win= swinarray[curswin];
	if(win==0) return;	

	mmode(MPROJECTION);
	getmatrix(win->winmat);
	mmode(MVIEWING);
}

/* ********** END MY WINDOW ************** */

