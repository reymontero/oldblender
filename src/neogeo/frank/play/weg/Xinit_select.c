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
#include <sys/select.h> /* select() */

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

/*
 *Local Prototype definitions
 */

static void doIt ();
static void handleEvents ();

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

int	mvFD;
int	xFD;
int	vlFD;
int maxFD;
fd_set	theFDSet, loopFDset;

#define BORDER 20

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
	swa.event_mask = KeyPressMask|StructureNotifyMask;
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
	    /*sproc ((void (*)(void *))handleEvents, PR_SADDR | PR_SFDS | PR_BLOCK)) == -1) {*/
	    sproc ((void (*)(void *))handleEvents, PR_SALL | PR_BLOCK)) == -1) {
		fprintf (stderr, "Unable to sproc to handle events\n");
		perror ("sproc");
		stopAllThreads ();
	}
	
	/*sigset(SIGCLD, stopAllThreads);*/
	streamDecompress();
}


static void handleXEvents ()
{
	XEvent ev;
	pid_t ppid = -1;

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
	
	while (XPending(dpy) != 0) {
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
			{
				XKeyEvent *kev = (XKeyEvent *)&ev;
				KeySym keysym;
				int buf;
				XLookupString(kev, (char *)&buf, 1, &keysym, 0);

				/*printf("  0x%x\n", keysym);*/

				switch (keysym) {
				case XK_Left:
					playstate.relframe = -1;
					break;
				case XK_Right:
					playstate.relframe = 1;
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
					printf("Interactive: %d\n", playstate.interactive);
					break;
				case XK_s: /* Single Step */
					doIt ();
					break;
				case XK_r:
					remote_vlan_cmd("LR");
					remote_vlan_cmd("LR");
					break;
				case XK_KP_Enter:
					playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_Escape:
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
				default:
					printf("key %d\n", keysym);
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}


/*********
 *
 * Interpret and act on movie events.
 *
 *********/

static void handleMovieEvents( )
{
	MVevent event;

	while ( mvPendingEvents() != 0 ) {
		mvNextEvent( &event );
		switch ( event.type ) {
			case MV_EVENT_FRAME: 		/* a frame played */
				printf( "%s: Played frame %d of movie %d.\n", 
				options.myname, event.mvframe.frame, 
				event.mvframe.id );
				break;
		    case MV_EVENT_STOP: 		/* end of movie */
				printf( "%s: Playback of movie %d stopped.\n",
			    options.myname, event.mvstop.id );
			    break;
		    case MV_EVENT_ERROR:		/* error */
				fprintf( stderr, "%s: Error during playback: %s.\n",
			    options.myname, mvGetErrorStr( mvGetErrno() ) );
			    break;
		}
	}
}

/*********
 *
 * Interpret and act on video events.
 *
 *********/

static void handleVideoEvents( )
{
	VLEvent event;
	static first = TRUE;
	
	/* blijkbaar mag dit niet
	 * 
	 */
	
	printf("video %d\n", first);
	if (first) {
		sginap(1);
		first ++;
		return;
	}
	
	while ( vlPending(video.svr) != 0 ) {
		vlNextEvent(video.svr, &event);
		switch(event.reason){
			case VLFrameVerticalRetrace:
				printf(".");
				fflush(stdout);
				break;
			case VLSyncLost:
				printf("Lost Sync\n");
				break;
			case VLSequenceLostMask:
				printf("Oops\n");
				break;
		}
	}
}

void
rtrt(VLServer svr, VLEvent *ev, void *data)
{

	printf("%d\n", ev->reason);
}



void handleEvents( void )
{

	/*
     * Tell kernel to send us SIGHUP when our parent goes
     * away.
     */
	 
	
	/* blijkbaar is het niet toegestaan om movie of video events te ontvangen,
	 * maar worden deze allemaal afgehandeld door clDecompress
	 */
	
	sigset(SIGHUP, SIG_DFL);
	prctl(PR_TERMCHILD);


	/*
     * Get the file descriptor responsible for communicating the
     * events we'll get back from the movie library
     */

	if ( mvGetEventFD(&mvFD) != DM_SUCCESS ) {
		fprintf( stderr, "%s: Could not get movie event FD.\n", options.myname);
	}

	/* 
     * Decide which movie events we're interested in hearing.
     */

	mvSetSelectEvents( MV_EVENT_MASK_FRAME | MV_EVENT_MASK_STOP | MV_EVENT_MASK_ERROR );

	xFD = ConnectionNumber( dpy );

	vlFD = vlGetFD(video.svr);
	
	/*vlSelectEvents(video.svr, video.path, VLSyncLostMask | VLSequenceLostMask | VLFrameVerticalRetraceMask);*/
	/*vlAddCallback(video.svr, video.path, VLSyncLostMask | VLSequenceLostMask | VLFrameVerticalRetraceMask, rtrt, NULL);*/
	
	FD_ZERO( &theFDSet );
	FD_SET( mvFD, &theFDSet );
	FD_SET( xFD, &theFDSet );
	/*FD_SET( vlFD, &theFDSet );*/


	maxFD = mvFD;
	maxFD = MAX2(maxFD, xFD);
	/*maxFD = MAX2(maxFD, vlFD);*/

	    /* 
     * Sit in select(), waiting for a message from the movie
     * player or from the X server. Process events from both. 
     */

	{
		DMboolean	keepPlaying = DM_TRUE;
		    fd_set	loopFDSet;

		while (1) {
			loopFDSet = theFDSet;
			select( maxFD, &loopFDSet, NULL, NULL, NULL );
			if ( FD_ISSET( mvFD, &loopFDSet ) ) {  	   /* movie event */
				handleMovieEvents( );
			} else if ( FD_ISSET( xFD, &loopFDSet ) ) {	   /* X event */
				handleXEvents();
			} else if ( FD_ISSET( vlFD, &loopFDSet ) ) {	   /* video event */
				handleVideoEvents();
			}
		}
	}
}

void handle__Events( void )
{
	fd_set	theFDSet, loopFDset;

	/*
     * Tell kernel to send us SIGHUP when our parent goes
     * away.
     */
	sigset(SIGHUP, SIG_DFL);
	prctl(PR_TERMCHILD);


	/*
     * Get the file descriptor responsible for communicating the
     * events we'll get back from the movie library
     */

	if ( mvGetEventFD(&mvFD) != DM_SUCCESS ) {
		fprintf( stderr, "%s: Could not get movie event FD.\n", options.myname);
	}

	/* 
     * Decide which movie events we're interested in hearing.
     */

	mvSetSelectEvents( MV_EVENT_MASK_FRAME | MV_EVENT_MASK_STOP | MV_EVENT_MASK_ERROR );

	xFD = ConnectionNumber( dpy );
	vlFD = vlGetFD(video.svr);

	vlAddCallback(video.svr, video.path, VLFrameVerticalRetraceMask, rtrt, NULL);

	vlRegisterHandler(video.svr, xFD, handleXEvents, NULL, NULL);
	vlRegisterHandler(video.svr, mvFD, handleMovieEvents, NULL, NULL);
	vlRegisterHandler(video.svr, vlFD, handleVideoEvents, NULL, NULL);
	
	vlMainLoop();
}


