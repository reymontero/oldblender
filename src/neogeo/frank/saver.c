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

#include <signal.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h>
#include <math.h>

/*

zipfork "cc -O3 saver.c -o saver -lgl -lX11 -lm>/dev/console"                                             
zipfork "./test >/dev/console"                                                                              
*/


Device devs[] = {
	LEFTMOUSE, 
	MIDDLEMOUSE, 
	RIGHTMOUSE, 
	MOUSEX, 
	MOUSEY, 
	LEFTSHIFTKEY, 
	RIGHTSHIFTKEY, 
	LEFTALTKEY, 
	RIGHTALTKEY, 
	LEFTCTRLKEY, 
	RIGHTCTRLKEY, 
	SPACEKEY, 
	RETKEY, 
	EKEY
};

#define NUMDEVS (sizeof(devs) / sizeof(Device))
float error[256];

void dimcalc_err(short *gam2, short *gam1, float val)
{
	long i;
	/* flikkert te veel */

	for (i = 0; i <= 255; i++){
		error[i] += gam1[i] * val;
		gam2[i] = error[i];
		error[i] -= gam2[i];
	}
}

void dimcalc(short *gam2, short *gam1, float val)
{
	long i;
	if (val < 0) val = 0;
	for (i = 0; i <= 255; i++){
		gam2[i] = gam1[i] * val;
	}
}

/* we gaan een andere oplossing zoeken voor het sample probleem
 * we gaan nu 2 windows openen: allebei 1 pixel groot, maar onder elkaar
 * voorste window word gesloten bij einde blanktime, daarna wordt
 */

main(long argc, char ** argv)
{
	FILE * fd;
	float gamval;
	short gam1[256], gam2[256];
	short vals1[NUMDEVS], vals2[NUMDEVS];
	long i, win, event, otherwin;
	short val, go = TRUE;
	float dim, darker = 0.001, lighter = 0.01;
	ulong dum;

	/*noport();*/
	noborder();
	win = winopen(argv[0]);

	noise(TIMER0, getgdesc(GD_TIMERHZ));
	qdevice(WINSHUT); 
	qdevice(WINQUIT);
	qdevice(TIMER0);
	
	while (go){
		switch(event = qread(&val)){
		case WINSHUT:
		case WINQUIT:
			go = FALSE;
			break;
		case REDRAW:
			if (val == win) {
				/* start van blanking */
				
				winset(win);
				prefposition(1278, 1279, 0, 2);
				winconstraints();
				
				gflush();
				finish();
				
				if (otherwin) winclose(otherwin);
				noborder();
				prefposition(1278, 1279, 0, 1);
				otherwin = winopen("");

				color(RED); clear();
				gflush(); finish();

				winset(win);
				winpop();
				color(BLACK); clear();
				gflush(); finish();
							
				event = qread(&val);
				/*printf("%d %d\n", event, val);*/
				
				/* huidige gamma inlezen en array aanmaken*/
				fd = fopen("/etc/config/system.glGammaVal", "r");
				if (fd){
					fscanf(fd, "%f", &gamval);
					fclose(fd);
				} else{
					gamval = 1.2;
				}
				gamval = 1.0 / gamval;
				
				for (i = 255 ; i >= 0 ; i--) gam1[i] = (255.0 * pow(i / 255.0 , gamval)) + 0.5;
	
				/* origineel array inlezen */
				getdev(NUMDEVS, devs, vals1);
	
				for (dim = 1.0 ; dim > 0.5; dim -= darker){
					dimcalc(gam2, gam1, dim);
					gammaramp(gam2, gam2, gam2);
					getdev(NUMDEVS, devs, vals2);
					if (memcmp(vals1, vals2, sizeof(vals1))) break;
					if (event = qtest()){
						if (event == REDRAW) break;
						qread(&val);
					}
					sginap(1);
				}

				while (qtest() != REDRAW && memcmp(vals1, vals2, sizeof(vals1)) == 0){
					event = qread(&val);
					if (event == REDRAW) qenter(event, val);
					getdev(NUMDEVS, devs, vals2);
				}

				if (qtest() != REDRAW) {
					qenter(REDRAW, otherwin);
					printf("Screensaver: no REDRAW ?\n");
				}
			}else{
				winclose(otherwin);
				otherwin = 0;
				/*printf("blank end %f\n", dim);*/
				for (; dim < 1.0; dim += lighter){
					dimcalc(gam2, gam1, dim);
					gammaramp(gam2, gam2, gam2);
					sginap(1);
				}
				gammaramp(gam1, gam1, gam1);
			}
			break;
		}
	}
}

