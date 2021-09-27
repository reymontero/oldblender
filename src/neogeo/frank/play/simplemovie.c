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

/******************************************************************************

 * File:        simplemovie.c
 *
 * Usage:       simplemovie <moviefile>
 *
 * Description: Plays a movie with the SGI movie library. Uses a keystroke
 *              interface with the following controls:
 *
 *                   KEY       ACTION           LIBMOVIE FUNCTIONS CALLED 
 *                   ---       ------           -------------------------
 *                 p or P    play the movie       mvSetPlaySpeed(), mvPlay() 
 *
 *                 s or S    stop playback        mvStop()   
 *
 *                 r or R    rewind the movie     mvSetCurrentFrame()
 *
 *                 m or M    toggle audio muting  mvGetEnableAudio(), 
 *                                                mvSetEnableAudio()
 *
 *                 l or L    toggle loop state    mvGetPlayLoopMode(),
 *                                                mvSetPlayLoopMode() 
 *
 *                 b or B    play backward        mvSetPlaySpeed(), mvPlay()
 *
 *                 f or F    play fast            mvGetPlaySpeed(),
 *                                                mvSetPlaySpeed(), mvPlay()
 *
 *                 h or H    play slow            mvGetPlaySpeed(), 
 *                                                mvSetPlaySpeed(), mvPlay()
 *
 *                 3         loop 3 times         mvSetPlayLoopLimit(),
 *                                                mvSetPlayLoopCount()
 *
 *                 e or E    play every frame     mvGetPlayEveryFrame(),
 *                                                mvSetPlayEveryFrame()
 *
 *                 q or Q    quit simplemovie     mvClose()
 * 
 * Functions:   SGI movie library functions used:
 *              
 *              mvOpenFile()
 *              mvOpenMem()
 *              mvFindTrackByMedium()
 *              mvGetImageWidth()
 *              mvGetImageHeight()
 *              mvBindWindow()
 *              mvGetEventFD()
 *              mvSetSelectEvents()
 *              mvPendingEvents()
 *              mvNextEvent()
 *              mvGetActualFrameRate()
 *              mvGetSlowThreshold()
 *              mvShowCurrentFrame()
 *              mvResizeWindow()
 *              mvQueryViewSize()
 *              mvSetViewSize()
 *
 ******************************************************************************/

#include <bstring.h>	/* for bzero(3C) -- used by FD_ZERO macro */
#include <sys/select.h>
#include <sys/types.h>
#include <sys/mman.h>   /* for mmap(2) */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#include <dmedia/moviefile.h>
#include <dmedia/movieplay.h>

#ifndef _SVR4_SOURCE  /* To pick up select(2), due to structure of unistd.h. */
#define _SVR4_SOURCE
#endif
#include <unistd.h>

/*
 * Helper functions for mixed-model GL window creation.  See the
 * directory /usr/people/4Dgifts/examples/GLX/gl-xlib on an IRIS
 * system for more information, including this header file and
 * the source code to GLXCreateWindow().
 */

#include "glxhelper.h"

/*
 * Forward declarations for functions used by main but defined later.
 */ 

static void	   usage( void );

static void	   setupFDSet( MVid theMovie, Display *dpy, int *fdM, int *fdX, 
                              fd_set *fdSet );
static void        initMovieWindow( MVid theMovie, Display **dpy, Window *win );
static DMstatus    createXWindow( Display **dpy, Window *win, 
                                 int width, int height );
static DMboolean   handleMovieEvents( MVid theMovie );
static DMboolean   handleXEvents( MVid theMovie, Display *dpy, Window win );
static DMboolean   handleKeyboard( MVid theMovie, XEvent* event );

/*
 * Global variable programName is used for error handling.
 */

static char *programName;

/******************************************************************************
 *
 * main
 *
 *****************************************************************************/

main ( int argc, char** argv )
{
    Display*  dpy;	           /* X display */
    Window    win;                 /* X window id for movie window. */
    MVid      theMovie;            /* MVid of the movie to be played. */
    char      *movieName;          /* Name of the movie to be played. */
    DMboolean memMovie = DM_FALSE; /* Use memory mapping if true. */
    int	      moviefd;             /* File descriptor for libmovie events. */
    int       xfd;                 /* File descriptor for X events. */
    fd_set    fdSet; 		   /* For use by select. */
    int       i;	

    programName = argv[0];

    if ( ( argc < 2 ) || ( argc > 3 ) ) {
        usage();
    }

    for ( i = 1; i < argc; i++ ) {
        if ( strcmp( argv[i], "-mem" ) == 0 ) {
            memMovie = DM_TRUE;
        }
        else {
            if ( movieName != NULL ) {
                fprintf( stderr, "%s: Only one movie at a time, please.\n\n",
                        programName, movieName );
		usage();
                exit( EXIT_FAILURE );
            }
            movieName = argv[i];
            if ( !( mvIsMovieFile( movieName ) ) ) {
                fprintf( stderr, "%s: %s is not a movie file.\n",
                        programName, movieName );
                exit( EXIT_FAILURE );
            }
        }
    }

    if ( memMovie ) {

        /*
         * Memory map the movie and open it from memory.
         */

    	{
            int fd;
            int len;
            void* addr;
            fd = open( movieName, O_RDONLY );
            if ( fd < 0 ) {
                fprintf( stderr, "%s: Could not open file: %s\n",
			programName, movieName );
                exit( EXIT_FAILURE );
            }
            len = lseek( fd, 0, SEEK_END );
            if ( len < 0 ) {
                fprintf( stderr, "%s: Could not seek to end of file: %s\n",
			programName, movieName );
                exit( EXIT_FAILURE );
            }
            addr = mmap( NULL, len, PROT_READ, MAP_PRIVATE, fd, 0 );
            if ( mvOpenMem( addr, len, &theMovie ) != DM_SUCCESS ) {
                fprintf( stderr, "%s: could not open movie in memory %s\n",
                        programName, movieName );
                exit( EXIT_FAILURE );
            }
    	}      
    }
    else {

        /*
         * Open movie from file.
         */

        if ( mvOpenFile( movieName, O_RDONLY, &theMovie ) != DM_SUCCESS ) {
	    fprintf( stderr, "%s: could not open movie file %s\n", 
                    programName, movieName );
	    exit( EXIT_FAILURE );
        }
    }

    /*
     * Create a window to display the movie.
     */

    initMovieWindow( theMovie, &dpy, &win );

    /*
     * Set up the file descriptor set for event handling by select().
     */

    setupFDSet( theMovie, dpy, &moviefd, &xfd, &fdSet );

    /*
     * decide which movie events we're interested in hearing.
     */

    mvSetSelectEvents( MV_EVENT_MASK_FRAME | MV_EVENT_MASK_STOP | 
                      MV_EVENT_MASK_ERROR | MV_EVENT_MASK_SLOW_PLAY );

    /*
     * Sit in select(), waiting for a message from the movie
     * player or from the X server.  Process events from both.
     */

    {
        DMboolean keepPlaying = DM_TRUE;   /* Flag to determine if we should 
                                              keep playing or stop. */
        int       maxfd;                   /* Number of file descriptors in
                                              set for select. */
        fd_set    loopfdSet;

        maxfd = ( ( moviefd > xfd ) ? moviefd : xfd ) + 1;

        while ( keepPlaying ) {

            loopfdSet = fdSet;
	    select( maxfd, &loopfdSet, NULL, NULL, NULL );

	    if ( FD_ISSET(moviefd, &loopfdSet ) ) { 	   /* movie event */
                keepPlaying = handleMovieEvents( theMovie );
	    } else if ( FD_ISSET( xfd, &loopfdSet ) ) {	   /* X event */
                keepPlaying = handleXEvents( theMovie, dpy, win );
	    }
        }
    }

    /*
     * Movie has finished playing.  Clean up and exit.
     */

    mvClose( theMovie );
    XDestroyWindow( dpy, win );
    exit( EXIT_SUCCESS );
}

/*********
 *
 * Print usage message and exit.
 *
 *********/

static void usage( void )
{
    fprintf( stderr, "usage: %s [ -mem ] <moviefile>\n", programName );
    fprintf( stderr, "       -mem causes the movie to be memory mapped ");
    fprintf( stderr, "upon opening.\n");
    exit( EXIT_FAILURE );
}

/*********
 *
 * Helper function to set up file descriptor set for select.
 *
 *********/

static void setupFDSet( MVid theMovie, Display *dpy, 
                       int *fdM, int *fdX, fd_set *thefdSet )
{
    /*
     * Get the file descriptor responsible for communicating the
     * events we'll get back from the movie library
     */

    if ( mvGetEventFD( fdM ) != DM_SUCCESS ) {
	fprintf( stderr, "%s: could not get movie event fd.\n", programName );
        mvClose( theMovie );
	exit( EXIT_FAILURE );
    }

    /*
     * Get the file descriptor for the X connection.
     */

    *fdX = ConnectionNumber( dpy );

    /*
     * Place the event file descriptors in the fd_set for select().
     */

    FD_ZERO( thefdSet );
    FD_SET( *fdM, thefdSet );
    FD_SET( *fdX, thefdSet );
}

/*********
*
* Create a GL/X window in which to display the movie.
*
*********/

static void initMovieWindow( MVid theMovie, Display **dpy, Window *win )
{
    MVid imageTrack;               /* The image track of theMovie. */
    int	 winWidth;                 /* The width of a movie image.  */
    int  winHeight;                /* The height of a movie image. */

    if ( mvFindTrackByMedium( theMovie, DM_IMAGE, &imageTrack ) 
        != DM_SUCCESS) {
        fprintf( stderr, "%s: Could not find image track.\n", programName );
	mvClose( theMovie );
        exit( EXIT_FAILURE );
    }
    winWidth  = mvGetImageWidth( imageTrack );
    winHeight = mvGetImageHeight( imageTrack );
    
    if ( createXWindow( dpy, win, winWidth, winHeight ) != DM_SUCCESS ) {
	fprintf( stderr, "%s: Could not create X window.\n", programName );
	mvClose( theMovie );
        exit( EXIT_FAILURE );
    }

    /*
     * Bind the window we just created to the movie.
     */

    if ( mvBindWindow( theMovie, *dpy, *win ) != DM_SUCCESS ) {
	fprintf( stderr, "%s: Could not bind movie to window.\n", programName );
	mvClose( theMovie );
        exit( EXIT_FAILURE );
    }

    XMapWindow( *dpy, *win );
    XFlush( *dpy );
}

/*********
*
* Interpret and act on movie events.
*
**********/

static DMboolean handleMovieEvents( MVid theMovie )
{
    MVevent event;

    /*
     * Loop until movie event queue is empty.
     */

    while ( mvPendingEvents() != 0 ) {
	mvNextEvent( &event );
	switch ( event.type ) {
	    case MV_EVENT_FRAME: 		/* a frame played */
	        break;

	    case MV_EVENT_STOP: 		/* end of movie */
	        fprintf( stderr, "%s: Playback stopped.\n", programName );
	        break;

	    case MV_EVENT_ERROR:		/* error */
	        fprintf( stderr, "%s: Error during playback: %s.\n",
                         programName, mvGetErrorStr( mvGetErrno() ) );
	        return DM_FALSE;
	        break;

	    case MV_EVENT_SLOW_PLAY:	/* playback too slow */
	        {
		    double speed;
		    mvGetActualFrameRate( theMovie, &speed );
		    fprintf( stderr, "%s: movie playing at %f frames/sec, "
		            "below threshold %f\n",
		            programName, speed, mvGetSlowThreshold() );
	        }
	        break;
	    }
        }
 return DM_TRUE;
}

/********
*
* Interpret and act on X events.
*
*********/

static DMboolean handleXEvents( MVid theMovie, Display *dpy, Window win )
{
    XEvent event;

    /*
     * Loop until X event queue is empty.
     */

    while ( XPending( dpy ) != 0 ) {

	XNextEvent( dpy, &event );
 
	switch ( event.type ) {
	    case Expose:			/* repaint display */
	        mvShowCurrentFrame( theMovie );
	        break;

	    case ConfigureNotify:		/* window was resized */
                {
                    XWindowAttributes winAttrs;
	            int actual_width, actual_height;

	            /*
	             * inform movie library window was resized.
	             */

	            mvResizeWindow( dpy, win );

	            /*
	             * Get the new window size
	             */

	            XGetWindowAttributes( dpy, win, &winAttrs );

	            /*
                     * Get and print the actual movie size.
	             */

	            mvQueryViewSize( theMovie, winAttrs.width, winAttrs.height, 
	                            DM_TRUE, &actual_width, &actual_height );
                    printf( "%s: actual width = %d, height = %d\n", 
	                   programName, actual_width, actual_height );

                    /*
                     * Make the width and height of the movie region the 
                     * same as the window.
                     */

	            mvSetViewSize( theMovie, winAttrs.width,
	                          winAttrs.height, DM_TRUE );
                }
	        break;

	    case KeyRelease:		/* keyboard event */
	        return( handleKeyboard( theMovie, &event) );
	        break;
	}
    }
 return DM_TRUE;
}

/********
*
* Interpret keyboard events and take appropriate action.
*
*********/

static DMboolean handleKeyboard( MVid theMovie, XEvent* event )
{
    char	c[2];

    XLookupString( (XKeyEvent*) event, c, 1, NULL, NULL);
    c[1] = NULL;

    printf( "%s: ", programName );

    switch ( c[0] ) {
	case 's':	    /* stop */
	case 'S':
 	    printf( "stop\n" );
	    mvStop( theMovie );
	    return DM_TRUE;

	case 'p':	    /* play */
	case 'P':
	    printf( "play\n" );
	    mvSetPlaySpeed( theMovie, 1.0 );
	    mvPlay( theMovie );
	    return DM_TRUE;

	case 'r':
	case 'R':	    /* rewind */
	    printf( "rewind\n" );
	    mvSetCurrentFrame( theMovie, 0 );
	    return DM_TRUE;

	case 'm':	    /* toggle audio muting */
	case 'M':
	    printf( "toggle mute to " );
	    if ( mvGetEnableAudio( theMovie ) ) {
	        printf( " Audio OFF\n" );
	        mvSetEnableAudio( theMovie, DM_FALSE );
	    } else {
	        printf(" Audio ON\n" );
	        mvSetEnableAudio( theMovie, DM_TRUE );
	    }
	    return DM_TRUE;

	case 'l':	    /* trip through loop states */
	case 'L':
	    printf( "toggle loop state to " );
	    switch ( mvGetPlayLoopMode( theMovie ) ) {
	        case MV_LOOP_NONE:
	            mvSetPlayLoopMode( theMovie, MV_LOOP_CONTINUOUSLY );
	            printf( "CONTINUOUS\n" );
	            break;
	        case MV_LOOP_CONTINUOUSLY:
	            mvSetPlayLoopMode( theMovie, MV_LOOP_SWINGING );
  
	            printf( "SWINGING\n" );
	            break;
    	        case MV_LOOP_SWINGING:
	            mvSetPlayLoopMode( theMovie, MV_LOOP_NONE );
	            printf( "NONE\n" );
	            break;
	     }
	     return DM_TRUE;

	case 'b':       /* backward playback */
	case 'B':
 
	    printf( "backward play\n" );
	    mvSetPlaySpeed( theMovie, -1.0 );
	    mvPlay( theMovie );
	    return DM_TRUE;

	case 'f':	    /* fast scan (4x) */
	case 'F':
	    printf( "fast play\n" );

            /*
             * Preserve direction of play, just play 4 times faster.
             */

	    if ( mvGetPlaySpeed( theMovie ) < 0) {
	        mvSetPlaySpeed( theMovie, -4.0 );
	    } else {
	        mvSetPlaySpeed( theMovie, 4.0 );
	    }
	    mvPlay( theMovie );
	    return DM_TRUE;

	case 'h':	    /* slow scan (1/4x) */
	case 'H':
	    printf( "slow play\n" );

            /*
             * Preserve direction of play, just play 1/4 as fast.
             */

	    if ( mvGetPlaySpeed( theMovie ) < 0 ) {
	        mvSetPlaySpeed( theMovie, -0.25 );
	    } else {
	        mvSetPlaySpeed( theMovie, 0.25 );
	    }
	    mvPlay( theMovie );
	    return DM_TRUE;

	case 'e':	    /* show every frame */
	case 'E':
	    printf( "play every frame set to " );
	    if ( mvGetPlayEveryFrame( theMovie ) ) {
	        mvSetPlayEveryFrame( theMovie, DM_FALSE );
	        printf( "FALSE - drop frames if necessary\n" );
	    } else {
	        mvSetPlayEveryFrame( theMovie, DM_TRUE );
	        printf( "TRUE - show every frame\n" );
	    }
	    return DM_TRUE;

	case '3':	    /* loop three times */
	    printf( "loop three times\n" );
	    mvSetPlayLoopLimit( theMovie, 3 );
	    mvSetPlayLoopCount( theMovie, 0 );
	    return DM_TRUE;

	case 'q':	    /* quit */
	case 'Q':
	    printf( "quit\n" );
	    return DM_FALSE;

        default:
	    printf( "unrecognized keycode %s\n", c );
        }
    return DM_TRUE;
}

/*********
*
* Open a connection to the X server, and create an X window
* suitable for GL rendering here.
*
*********/

static DMstatus createXWindow( Display **dpy, Window *win, 
                              int width, int height )
{
    XSetWindowAttributes childWinAttribs;

    /* 
     *Open a connection to the X server. 
     */

    *dpy = XOpenDisplay( 0 );
    if ( *dpy == NULL ) {
	fprintf( stderr, "%s: Cannot open display.\n", programName );
	return DM_FAILURE;
    }
    childWinAttribs.colormap = DefaultColormap( *dpy, DefaultScreen( *dpy ) );

    /* 
     * Even if we don't use it, childWinAttribs.border_pixel must be something.
     */

    childWinAttribs.border_pixel = 0;

    /* 
     * Create an X window configured for GL rendering, using
     * the helper functions defined in glxhelper.c
     */

    *win = GLXCreateWindow( *dpy, RootWindow( *dpy, DefaultScreen( *dpy ) ),
                           100, 100, width, height, 0, CWColormap|CWBorderPixel,
                           &childWinAttribs, GLXrgbSingleBuffer );
    XSelectInput( *dpy, *win, ExposureMask | StructureNotifyMask | 
                 KeyReleaseMask );
    return DM_SUCCESS;
}

