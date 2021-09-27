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

/*

 * Xinit.c: Does X / gl initialization.
 *
 * 
 * Silicon Graphics Inc., June 1994
 */

#include <signal.h>	/* sigset(), kill() */
#include <unistd.h>	/* getppid() */

#include "play.h"
#include "glxh2.h"
#include <local/util.h>

/*
 *Sproc include files
 */
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <Xm/MwmUtil.h>
#include <local/network.h>
#include <dmedia/dmedia.h>

#include <math.h>

/*
 *Local Prototype definitions
 */

static void doIt ();
static void handleXEvents ();

static String fallback_resources[] = {
	"*frame*shadowType: SHADOW_IN",
	NULL
};


/*
 *Sproc'd processes pids'
 */
int inputthread = -1;

/*
 *X globals.
 */
static Widget glw, toplevel, form, frame;

Display *dpy;
int winWidth, winHeight;
Window topwin, videowin;                 /* the X & GL window identifiers */

#define BORDER 20


#define SERVER_PORT	(IPPORT_USERRESERVED + 123)
char *host = "iris";
int sock = 0;

char * remote_vlancmd(char * cmd)
{
	long count;
	char ret[512];
	static int print_error = TRUE;
	
	if (sock == 0) sock = connect_to_host(host, SERVER_PORT);
	
	if (sock == -1) {
		if (print_error) printf("Start video on iris\n");
		print_error = FALSE;
		sock = 0;
	} else {
		if (write(sock, cmd, strlen(cmd)) == strlen(cmd)){
			if (count = read(sock, ret, sizeof(ret) - 1)){
				ret[count] = 0;
				return (ret);
			}
		}
	}
	return (NULL);
}


char * remote_read(void)
{
	long count;
	char ret[512];
	static int print_error = TRUE;
	
	if (count = read(sock, ret, sizeof(ret) - 1)){
		ret[count] = 0;
		return (ret);
	}

	return (NULL);
}

/*
 *doIt:	Starts all the threads of the application, namely
 *			- video producer
 *			- video consumer (if display = GRAPHICS)
 *			- audio producer (only if Cosmo and display = VIDEO)
 *
 */

void
doIt ( void )
{
	playstate.advanceVideo = 1;
	playstate.advanceAudio = 1;
}


/*
 *exposeWindow: called during an expose event. Clears the window on expose.
 *      On the first expose event, it calls doIt() to start the 
 *		application. It also maps the widget into a video window and 
 *		sets up the transfer of data from the camera to the 
 *		Indy/Galileo video board.
 *
 *		NOTE: The first expose event is assumed to happen immediately 
 *		with the map notify event (when the window is mapped by the X
 *		server.), which is normally the case.
 */

void exposeWindow ()
{
	static int first = 1;
	Window dummyWin;
	int x,y;
	VLControlValue val;
	pid_t ppid;
	
	if (first) {
		first = 0;

		val.intVal = videowin;
		vlSetControl(video.svr, video.path, video.drn, VL_WINDOW, &val);

		/*
		* Trim off top and bottom three lines
		*/
		
		vlGetControl(video.svr, video.path, video.drn, VL_SIZE, &val);
		val.xyVal.y -= 6;
		vlSetControl(video.svr, video.path, video.drn, VL_SIZE, &val);

		vlGetControl(video.svr, video.path, video.drn, VL_OFFSET, &val);
		val.xyVal.y -= 3;
		vlSetControl(video.svr, video.path, video.drn, VL_OFFSET, &val);

		/*
		 *Once the window is mapped, vl/gl  can begin transfer, thus start the
		 *video application, i.e. streamDecompress()
		 */
		 
		if ((ppid = getppid ()) == -1) {
			perror("Unable to find video thread");
			exit (1);
		}

		if (unblockproc (ppid) == -1) {
			perror("Unable to start video thread");
			exit(1);
		}
	}
}

void motionWindow ()
{
	Window dummyWin;
	VLControlValue val;

	XTranslateCoordinates(dpy, videowin,
	    RootWindow(dpy, DefaultScreen(dpy)),
	    0, 0,
	    &val.xyVal.x, &val.xyVal.y,
	    &dummyWin);

	if (image.display == GRAPHICS)
		return;

	vlSetControl(video.svr, video.path, video.drn, VL_ORIGIN, &val);
}


/*
 *stopAllThreads (): Attempts to gracefully exit all the threads.
 */
void stopAllThreads ()
{
	if (video.timingActive == 1) {
		video.timingActive = 0;
		vlDestroyPath(video.svr, video.timingPath);
	}

	exit (0);
}


/*
 *Create a window for display
 */
void Xgo( void )
{
	int n;
	VLControlValue val;
	XSetWindowAttributes swa;    /* needed for XCreateColor(3X11)          */
	XColor gray;                 /* stores the color for X window backgrnd */
	GLXconfig*  config;          /* pointer to current GLX window created 
					    via GLXCreateWindow defined in glxh2.c */
	XSizeHints *Waspecthints;    /* used for min-size & aspect ratio manip */
	int w, h;                /* save new resized width/height for X parent */
	int wx[6], wy[6];      /* 6 x/y top-left origin points for the GL wins */
	float xunit, yunit;                   /* used to calc chnged win sizes */
	int myExpose, myConfigure, myKeyPress; /* indicates which events occur */
	int upAndRunning = 0;                  /* don't set this to true until
						      we get our first Expose event*/


	dpy = XOpenDisplay(0);              /* rule # 0: establish our display */
	if (!dpy) {
		fprintf(stderr, "cannot open display\n");
		exit(-1);
	}
	myExpose = myConfigure = myKeyPress = 0;     /* init our own notifiers */

	if (image.display != GRAPHICS) {
		/*
		 * Trim off top and bottom three lines
		 */
		vlGetControl(video.svr, video.path, video.drn, VL_SIZE, &val);
		winHeight = val.xyVal.y-6;
		winWidth = val.xyVal.x;
	} else {
		winWidth = image.width;
		if (image.interlacing == DM_IMAGE_NONINTERLACED) winHeight = 2*image.height;
		else winHeight = image.height;
	}
	
	/*
     * First create the top level X window.  The background will be gray,
     * and *this* (X parent) window is only interested in key press events.
     */
	gray.red = gray.green = gray.blue = 0x7fff;
	
	XAllocColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &gray);
	swa.background_pixel = gray.pixel;
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;
	topwin = XCreateWindow(dpy, RootWindow(dpy, DefaultScreen(dpy)),
	    100, 100, winWidth + 2 * BORDER, winHeight + 2 * BORDER,
	    0, CopyFromParent, InputOutput, CopyFromParent,
	    CWEventMask|CWBackPixel, &swa);

	XStoreName(dpy, topwin, "NeoGeo Cosmo play");


	/*
     *  Now specify the values for the Window Aspect Hints we want to 
     *  enforce so that the aspect ratio of 2 to 3 will always be 
     *  maintained when the parent X window is resized.  Also specify
     *  the minimum size (46x68) we will allow this window to shrink to.
	 * 
	 *  Leuk voor later
     */
/*
	Waspecthints = XAllocSizeHints();
	Waspecthints->min_aspect.x = 2;
	Waspecthints->max_aspect.x = 2;
	Waspecthints->min_aspect.y = 3;
	Waspecthints->max_aspect.y = 3;
	Waspecthints->min_width  = 230;
	Waspecthints->min_width  = 46;
	Waspecthints->min_height = 340;
	Waspecthints->min_height = 68;
	Waspecthints->flags = PAspect|PMinSize;
	XSetWMNormalHints(dpy, top, Waspecthints);
*/


	config = GLXCreateWindow(dpy, topwin, BORDER, BORDER, winWidth, winHeight, 0, 0);
	/* now scan through the config struct, looking for the proper argument
     *  value associated with the following buffer(arg2)-mode(arg3) pair.
     */
	videowin = GLXgetvalue(config, GLX_NORMAL, GLX_WINDOW);
	/* we're going to reuse config for each GL window so free it up now */
	free(config);


	/*
     * Now that all the windows exist, cause them to appear by requesting
     *  the Xserver report all Expose events that occur in these 6 windows.
     */
	XSelectInput(dpy, videowin, ExposureMask);

	/* now map (make visible) the GL window */
	XMapWindow(dpy, videowin);

	/* now install these window's colormaps so they'll show their true colors */
	XSetWMColormapWindows(dpy, topwin, &videowin, 1);

	/* and now, finally, map the big mama of 'em all, the X parent */
	XMapWindow(dpy, topwin);
    XFlush( dpy );

	if ((inputthread = 
		sproc ((void (*)(void *))handleXEvents, PR_SALL | PR_BLOCK)) == -1) {
		fprintf (stderr, "Unable to sproc to handle events\n");
		perror ("sproc");
		stopAllThreads ();
	}
	
	sigset(SIGCLD, stopAllThreads);
	streamDecompress();
}


static void handleXEvents ()
{
	XEvent ev;
	pid_t ppid = -1;
	long record = FALSE, abort = FALSE, inpoint, current;
	unsigned long long starttime, now, dlta, before, after;
	CLimageInfo imageinfo;	
	char * ret;
	double fieldnumber;
	
	/*
     * Tell kernel to send us SIGHUP when our parent goes
     * away.
     */
	sigset(SIGHUP, SIG_DFL);
	prctl(PR_TERMCHILD);

	/* na opstarten:
	 * 
	 * ReparentNotify
	 * ConfigureNotify (begin / scalen / verplaatsen)
	 * MapNotify
	 * Expose (scalen / pop)
	 */
	
	while (1) {
		while ((XPending(dpy) != 0) || (record == FALSE)) {
			XNextEvent(dpy, &ev);
			printf("event %d\n", ev.type);
			switch(ev.type) {
			case ConfigureNotify:
				motionWindow();
				break;
			case Expose:
				exposeWindow();
				break;
			case KeyPress:
			case KeyRelease:
				{
					XKeyEvent *kev = (XKeyEvent *)&ev;
					KeySym keysym;
					int buf;
					XLookupString(kev, (char *)&buf, 1, &keysym, 0);
	
					/* tijdens opname mag alleen op escape gereageerd worden */
					if ((record != 0) && (keysym != XK_Escape)) break;
					
					if (ev.type == KeyPress) {
						switch (keysym) {
						case XK_Left:
							playstate.advanceVideo = -1;
							break;
						case XK_Right:
							playstate.advanceVideo = 1;
							break;
						case XK_KP_0:
						case XK_KP_Insert:
							if (playstate.autoStop) {
								playstate.autoStop = FALSE;
								playstate.advanceVideo =  playstate.defaultspeed;
							} else {
								playstate.autoStop = TRUE;
								playstate.absframe = 0;
							}
							break;
						case XK_KP_1:
						case XK_KP_End:
							playstate.defaultspeed = 1;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_2:
						case XK_KP_Down:
							playstate.defaultspeed = 2;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_3:
						case XK_KP_Page_Down:
							playstate.defaultspeed = 3;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_4:
						case XK_KP_Left:
							playstate.defaultspeed = 4;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_5:
						case XK_KP_Begin:
							playstate.defaultspeed = 5;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_6:
						case XK_KP_Right:
							playstate.defaultspeed = 6;
							if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_KP_Delete:
						case XK_KP_Decimal:
							playstate.relframe = playstate.defaultspeed;
							break;
						case XK_i:	/* interactive mode on / of */
						case XK_I:
							playstate.interactive = playstate.interactive == 0;
		
							if (playstate.interactive) maxinbuffer = IA_FIELDS;
							else maxinbuffer = NON_IA_FIELDS;
							
							printf("Interactive: %d,  max delay: %d fields\n", playstate.interactive, maxinbuffer);
							break;
						case XK_s: /* Single Step */
							doIt ();
							break;
						case XK_r:
							record = TRUE;
							break;
						case XK_KP_Enter:
							playstate.advanceVideo = playstate.defaultspeed;
							break;
						case XK_Q:
							if (video.timingActive == 1) {
								video.timingActive = 0;
								vlDestroyPath(video.svr, video.timingPath);
							}
							if ((ppid = getppid ()) == -1) {
								perror("Unable to find video thread");
							} else
								if (ppid != 1)
									kill ((pid_t)ppid, SIGTERM);
							exit(0);
							break;
						case XK_Escape:
							if (record == FALSE) {
								printf("Press Q or close window to stop playback\n");
							} else {
								abort = TRUE;
							}
							break;
						default:
							printf("key %d\n", keysym);
							break;
						}
					} else {
						switch (keysym) {
						case XK_Left:
						case XK_Right:
							playstate.advanceVideo = 0;
							break;
						}
					}
				}
				break;
			default:
				break;
			}
		}
		
		if (abort == TRUE) {
			record = abort = FALSE;
			remote_vlancmd("AB");
			remote_vlancmd("SH0");
		}
		
		if (record) {
			switch (record) {
				case 1:
					/* test of er een verbindig is naar VLAN
					 * zo ja: bereid opname voor
					 */
					remote_vlancmd("GP");
					if (sock == 0) {
						printf("Can't connect to vlan: start video\n");
						record = 0;
					} else {
						record++;
						sginap(10); /* even laatste handeling af laten maken */
						maxinbuffer = IA_FIELDS;
						playstate.advanceVideo = 0;
						inpoint = string_to_tcode(remote_vlancmd("RI"));
					}
					break;
				case 2:
					/* wacht tot de buffer zowat leeg is */
					if (nowinbuffer <= maxinbuffer) {
						remote_vlancmd("PF");
						sginap(10);
						record++;
					} else {
						sginap(1);
					}
					break;
				case 3:
					/* wacht tot 30 frames voor inpunt */
					ret = remote_vlancmd("ES");
					switch(ret[0]){
					case 'A':			/* aborted */
					case 'D':			/* done */
						printf("something went wrong: aborting\n");
						record = 0;
						break;
					case 'C':			/*cueing */
						sginap(4);
						break;
					case 'E':			/*editing */
						if (string_to_tcode(remote_vlancmd("LR")) > inpoint - 40){
							starttime = ULONGLONG_MAX;
							record++;
						} else {
							sginap(1);
							/*printf("%s\n", remote_vlancmd("SE"));
							printf("%s\n", remote_vlancmd("EE"));*/
						}
						break;
					}
					break;
				case 4:
					/* bereken op welk moment de opname gestart moet worden */

					dmGetUST(&before);
					current = string_to_tcode(remote_vlancmd("LR"));
					dmGetUST(&after);
					if (current > (inpoint - 20)) record++;
					else {
						printf(" delay: %.2f fields\n", (after - before) / NANOS_PER_FIELD);
						now = after;
						dlta = (inpoint - current) * NANOS_PER_FRAME;
						now += dlta;
						if (starttime > now) starttime = now;
						printf("%llu %.2f %.2f\n", starttime, (25.0 * starttime) / NANOS_PER_SEC, (25.0 * now) / NANOS_PER_SEC);
					}
					
					break;
				case 5:
					if ( clGetNextImageInfo( codec.Hdl, &imageinfo, sizeof(imageinfo) ) < SUCCESS ) {
							ERROR( "clGetNextImageInfo failed" );
					}
					
					dlta = starttime - imageinfo.ustime;
					fieldnumber = ((dlta * image.frameRate * 2.0) / NANOS_PER_SEC) + imageinfo.imagecount;
					
					printf("%.2f + %.2f\n", (dlta * image.frameRate * 2.0) / NANOS_PER_SEC, fieldnumber);
					fieldnumber = 2.0 * ffloor((fieldnumber / 2.0) + 0.5);
					maxinbuffer = LONG_MAX;
					start_at_field = fieldnumber - (2 * prerollVideoFrames) - 4;
					record++;
					break;
				case 6:
					ret = remote_vlancmd("ES");
					switch(ret[0]){
					case 'A':			/* aborted */
						printf("something went wrong: aborting\n");
						record = 0;
						break;
					case 'D':			/* done */
						record = 0;
						break;
					case 'C':			/*cueing */
					case 'E':			/*editing */
						sginap(4);
						break;
					}
					break;
				default:
					remote_vlancmd("AB");
					record = 0;
			}
		}
		
		if (record == 0) {
			if (playstate.interactive) maxinbuffer = IA_FIELDS;
			else maxinbuffer = NON_IA_FIELDS;
		}
	}
}


