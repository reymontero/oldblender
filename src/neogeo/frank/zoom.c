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

#include <sys/types.h>

#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h> 
#include <local/iff.h>

/*

bindkey -r f1,'cc zoom.c -lgl_s -o zoom\n'
bindkey -r f2,'zoom\n'	

*/


void printcolor(mouse,ofsx,ofsy)
short *mouse;
long ofsx,ofsy;
{
	short val,u,v;
	char rgb[4],string[50];
	long yuv;

	screenspace();
	cpack(0);
	rectfi(10+ofsx,10+ofsy,290+ofsx,30+ofsy);
	cpack(0xffffff);
	recti(10+ofsx,10+ofsy,290+ofsx,30+ofsy);
	recti(263+ofsx,13+ofsy,287+ofsx,27+ofsy);

	/* cmov2i(mouse[0],mouse[1]);
	gl_readscreen(1,rgb+3,rgb+2,rgb+1); */
	readdisplay(mouse[0], mouse[1], mouse[0], mouse[1], (ulong *) rgb, 0);
	cpack(* ((ulong *) rgb));
	rectfi(265+ofsx,15+ofsy,285+ofsx,25+ofsy);

	cpack(0);
	rectfi(11+ofsx,11+ofsy,122+ofsx,29+ofsy);
	cmov2i(20+ofsx,15+ofsy);
	yuv = colcspace((rgb[1] << 16) + (rgb[2] << 8) + (rgb[3]),rgbbeta);

	u = abs(((yuv >> 16) & 0xff) - 128);
	v = abs((yuv & 0xff) - 128);
	if (u>v) v = u;

	if (v > 96) cpack(0x00ffff);
	else cpack(0xffffff);
	sprintf(string,"rgb: %2x %2x %2x y: %2x uv: %2x",rgb[3],rgb[2],rgb[1], (yuv >> 8) & 0xff, v);
	charstr(string);
}


main()
{
	short mouse[2],go,val, mousex =- 32768, mousey, cont=FALSE;
	Device dev[2];
	long ofsx,ofsy,sizex,sizey,readx,ready,event;
	float zoom;
	short bx1,bx2,by1,by2, all = TRUE;
	ulong *buf;
	long mainwin, subwin;

	sizex=720;
	sizey=568;
	zoom=6.0;

	prefsize(300,300);
	mainwin = winopen("Zoom");
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	minsize(100,100);
	maxsize(getgdesc(GD_XPMAX),getgdesc(GD_YPMAX));
	winconstraints();

	subwin = swinopen(mainwin);
	winposition(10, 290, 10, 30);
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	
	winset(mainwin);

	qdevice(KEYBD);
	qdevice(WINQUIT);
	qdevice(WINSHUT);  /* anders verdwijnt sluitsymbool */
	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);

	rectzoom(zoom,zoom);
	dev[0]=MOUSEX;
	dev[1]=MOUSEY;
	screenspace();

	go=TRUE;
	do{
		readx=(sizex/zoom)/2+1;
		ready=(sizey/zoom)/2+1;
		bx1=readx*zoom-zoom;
		by1=ready*zoom-zoom;
		bx2=readx*zoom-1;
		by2=ready*zoom-1;
		do{
			getdev(2,dev,mouse);
			if(mouse[0] != mousex || mouse[1] != mousey || cont){
				mousex = mouse[0]; 
				mousey = mouse[1];
				winset(subwin);
				printcolor(mouse,ofsx,ofsy);
				winset(mainwin);
				if (all){
					buf = (ulong *) malloc((2*readx + 1) * (2*ready + 1) * 4);
					readdisplay(mouse[0]-readx,mouse[1]-ready,mouse[0]+readx,mouse[1]+ready, buf, 0);
					lrectwrite(0, 0, 2*readx, 2*ready, buf);
					free(buf);
				} else{
					mouse[0] -= ofsx;
					mouse[1] -= ofsy;
					rectcopy(mouse[0]-readx,mouse[1]-ready,mouse[0]+readx,mouse[1]+ready,0,0);
				}
				finish();
			}
			if (qtest()==0) sginap(10);
	/* HOI FRANK: dit heb ik erin gezet om overdraw te kunnen screengrabben */
			if(getbutton(LEFTALTKEY) && getbutton(LEFTSHIFTKEY)) {
				while(getbutton(LEFTCTRLKEY)==0) sginap(10);
			}
				
		}while(qtest()==0);
		mousex = -32768;
		event=qread(&val);
		switch (event){
		case KEYBD:
			switch (val){
			case 27:
				go=FALSE;
				break;
			case '+':
			case '=':
				zoom++;
				rectzoom(zoom,zoom);
				break;
			case '-':
				if (zoom!=1.0){
					zoom--;
					rectzoom(zoom,zoom);
				}
				break;
			case 'a':
			case 'A':
				printf("all\n");
				all = TRUE;
				RGBmode();
				gconfig();
				break;
			case 'r':
			case 'R':
				printf("rgbmode\n");
				all = FALSE;
				RGBmode();
				gconfig();
				break;
			case 'c':
			case 'C':
				printf("cmode\n");
				all = FALSE;
				cmode();
				gconfig();
				break;
			case 'f':
			case 'F':
				all = FALSE;
				readsource(SRC_FRONT);
				break;
			case 'z':
			case 'Z':
				all = FALSE;
				readsource(SRC_ZBUFFER);
				break;
			case 13:
				cont = (cont == 0);
				break;
			default:
				printf("%d\n",val);
			}
			break;
		case REDRAW:
			getorigin(&ofsx,&ofsy);
			getsize(&sizex,&sizey);
			ortho2(-0.5,sizex-0.5,-0.5,sizey-0.5);
			viewport(0,sizex-1,0,sizey-1);
			
			winset(subwin);
			winposition(10, 290, 10, 30);
			winset(mainwin);
			
			break;
		case INPUTCHANGE:
			break;
		case WINSHUT:
		case WINQUIT:
			go = FALSE;
			break;
		default:
			printf("unknown event %d %d\n",event,val);
		}
		/*
		cmov2i(srcxorg,srcyorg+(i/factor));
		gl_readscreen(srcxsize,r0,g0,b0);
		leest blijkbaar r g b in *r0,*g0.. ook in cmode !
*/

	}while(go);
}

