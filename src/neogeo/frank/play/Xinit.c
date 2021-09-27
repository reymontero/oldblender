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


#include "forms.h"
#include "Play.h"

/*
 *Sproc include files
 */
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <Xm/MwmUtil.h>
#include <math.h>
#include <local/gl_util.h>

#include <sys/stat.h>			/* stat */

/*
 *Local Prototype definitions
 */

static void doIt ();
static int handleXEvents (FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d);
static void handleEvents ();
static void handleVlanEvents ();

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


#define SERVER_PORT	(5000 + 123)
char *host = "iris";
int sock = 0;
long modifiers = 0;
int record = FALSE, vlanabort = FALSE;
int startvl = FALSE;
int video_zoom = 1;
FD_Play *fd_Play;

timestruc_t file_time = {0, 0};

timestruc_t get_mtime (char * name)
{
	struct stat statbuf;
	timestruc_t nulltime = {0, 0};
	
	if (stat(name, &statbuf) == -1) return (nulltime);
	
	return(statbuf.st_mtim);
}

char * remote_vlancmd(char * cmd)
{
	long count, print_error = TRUE;
	char ret[512];
	
	if (sock == 0) sock = connect_to_host(host, SERVER_PORT);
	
	if (sock == -1) {
		if (print_error) {
			printf("\n\nNo video listening on port.\n\n");
			printf("Start a NEW video\n");
			printf("If that doesn't help do a:\n");
			printf("  rsh iris /etc/killall video\n");
			printf("and try again\n\n\n");
		}
			print_error = FALSE;
		sock = 0;
	} else {
		/*printf("cmd: %s", cmd);
		fflush(stdout);*/
		if (write(sock, cmd, strlen(cmd)) == strlen(cmd)){
			if (count = read(sock, ret, sizeof(ret) - 1)){
				ret[count] = 0;
				/*printf(" ret: %d -%s-\n", count, ret);*/
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
	
	if (count = read(sock, ret, sizeof(ret) - 1)){
		ret[count] = 0;
		printf("%d\n", count);
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
	VLControlValue val;
	pid_t ppid;
	

		val.intVal = videowin;
		vlSetControl(video.svr, video.path, video.drn, VL_WINDOW, &val);

		/*
		* Trim off top and bottom three lines
		*/
		
		vlGetControl(video.svr, video.path, video.drn, VL_SIZE, &val);
		val.xyVal.y -= 6;
		
		vlGetControl(video.svr, video.path, video.drn, VL_OFFSET, &val);
		val.xyVal.y -= 3;
		vlSetControl(video.svr, video.path, video.drn, VL_OFFSET, &val);

	if (first) {
		first = 0;
		
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
	extern int outputproc;

	if (outputproc) kill(outputproc, SIGINT);
	
	if (video.timingActive == 1) {
		video.timingActive = 0;
		vlDestroyPath(video.svr, video.timingPath);
	}
		
	exit (0);
}

int vid_expose(XEvent *ev, void *data) {
	printf("\nin vid_expose\n\n");
}

int vid_configure_notify1(FL_OBJECT *ob, int event, FL_Coord mx, FL_Coord my, int key, void *raw_event) {
	printf("\nin vid_configure_notify1 %d\n\n", event);
	return(0);
}

int vid_configure_notify2(FL_OBJECT *ob, Window win, int win_width, int win_height, XEvent *ev, void *data) {
	printf("\nin vid_configure_notify2 %d\n\n", ev->type);
	return(0);
}

int vidcanvas_key(FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d)
{

    printf("KeyPress: keysym=%ld\n", XKeycodeToKeysym(fl_display, ev->xkey.keycode, 0));
    return 0;
}

int vidcanvas_but(FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d)
{

    printf("Button%s: %d\n", ev->type==ButtonPress?"Press":"Release", ev->xbutton.button);
    return 0;
}

int idle_callback(XEvent *xev, void * user_data)
{
	static lastFrame = -1, lastSpeed = M_PI;
	static int first = TRUE;
	char txt[128];
	int wait = TRUE;
	int x, y;
	Edit * edit;
	Shot * shot;
	VLInfoPtr info;
	timestruc_t new_time;
	struct timeval currenttime;
	int offset = 0;
	
	extern int version;
	extern char imagename[1024], soundname[1024];
	extern int imageframe, soundframe, first_blender;

	fl_freeze_form(fd_Play->Play);
		
		if (version != 1) offset = 1;
	
		if (lastFrame != outputFrame) {
			if (fd_Play->MPos->pushed == FALSE) {
				fl_set_slider_value(fd_Play->MPos, outputFrame / (total_frames - 1.0));
				lastFrame = outputFrame;
			}
			wait = FALSE;
		}
		
		if (atof(fl_get_input(fd_Play->MFrame)) - offset != outputFrame) {
			fl_get_input_cursorpos(fd_Play->MFrame, &x, &y);
			if (x == -1) {
				sprintf(txt, "%d", (int) (outputFrame /*+ 0.5*/) + offset);
				fl_set_input(fd_Play->MFrame, txt);
			}
		}
		
		if (fd_Play->MSpeed_slider->pushed == FALSE) {
			if (fl_get_slider_value(fd_Play->MSpeed_slider) != 0.5) {
				fl_set_slider_value(fd_Play->MSpeed_slider, 0.5);
				playstate.advanceVideo = 0.0;
			}
			wait = FALSE;
		}
	
		if (playstate.advanceVideo != lastSpeed) {
			sprintf(txt, "%.3f", playstate.advanceVideo);
			fl_set_input(fd_Play->MSpeed_input, txt);
			
			
			fl_set_button(fd_Play->MFReverse, 0);
			fl_set_button(fd_Play->MReverse, 0);
			fl_set_button(fd_Play->MStop, 0);
			fl_set_button(fd_Play->MForward, 0);
			fl_set_button(fd_Play->MFForward, 0);
			
			if (playstate.advanceVideo == 0.0) {
				fl_set_button(fd_Play->MStop, 1);
			} else if (playstate.advanceVideo < 0.0) {
				if (playstate.advanceVideo <  - (FAST_SPEED / 2.0)) {
					fl_set_button(fd_Play->MFReverse, 1);
				} else {
					fl_set_button(fd_Play->MReverse, 1);
				}
			} else {
				if (playstate.advanceVideo > (FAST_SPEED / 2.0)) {
					fl_set_button(fd_Play->MFForward, 1);
				} else {
					fl_set_button(fd_Play->MForward, 1);
				}
			}
			
			wait = FALSE;
		}
			
		if (currentFrame == 0 && playstate.advanceVideo == 0.0) fl_set_button(fd_Play->MGoto_in, 1);
		else fl_set_button(fd_Play->MGoto_in, 0);
		if (currentFrame == total_frames - 1 && playstate.advanceVideo == 0.0) fl_set_button(fd_Play->MGoto_out, 1);		
		else fl_set_button(fd_Play->MGoto_out, 0);
		
		if (playstate.autoStop) fl_set_button(fd_Play->MLoop, 1);
		else fl_set_button(fd_Play->MLoop, 0);
		
		lastSpeed = playstate.advanceVideo;

		if (first) {
			shot = find_shot("Main");
			
			if (shot) {
				first = FALSE;
				strcpy(txt, "@i> ");
				strcat(txt, shot->name);
				
				fl_add_browser_line(fd_Play->MShots, txt);
				fl_add_browser_line(fd_Play->MShots, "@-");
				
				if (shot->type == HULPTAPE) {
					edit = ((Hulptape *) shot)->first;
					while (edit){
						if (edit->shot) {
							strcpy(txt, "");
							if (edit->shot->type == HULPTAPE) strcat(txt, "@i> ");
							else strcat(txt, "   ");
							strcat(txt, edit->shot->name);
							fl_add_browser_line(fd_Play->MShots, txt);								
						}
						edit = edit->next;
					}
					
				}
			}
		}

		if (strcmp(fl_get_input(fd_Play->Image_movie), imagename)) {
			fl_set_input(fd_Play->Image_movie, imagename);
		}

		if (strcmp(fl_get_input(fd_Play->Sound_movie), soundname)) {
			fl_set_input(fd_Play->Sound_movie, soundname);
		}

		if (atoi(fd_Play->Image_frame->label) - offset != imageframe) {
			if (first_blender) {
				sprintf(txt, "%d  (%d)", imageframe + offset, imageframe + offset + first_blender - 1);
			} else sprintf(txt, "%d", imageframe + offset);
			fl_set_object_label(fd_Play->Image_frame, txt);
		}

		if (atoi(fd_Play->Sound_frame->label) - offset != soundframe) {
			sprintf(txt, "%d", soundframe + offset);
			fl_set_object_label(fd_Play->Sound_frame, txt);
		}
		
		if (fl_get_button(fd_Play->Auto_reload)) {
			new_time = get_mtime(playstate.current_edl);
			if (memcmp(&new_time, &file_time, sizeof(new_time))){
				gettimeofday(&currenttime, 0);
				if ((currenttime.tv_sec - new_time.tv_sec) > 1) {
					load_edl(playstate.current_edl, APPEND);
					playstate.newmain= TRUE;
					flushAll = TRUE;
				} else if (currenttime.tv_sec - new_time.tv_sec) {
					if (((1000 * currenttime.tv_usec) - new_time.tv_nsec) > 0) {
						load_edl(playstate.current_edl, APPEND);
						playstate.newmain= TRUE;
						flushAll = TRUE;
					}
				}
			}
		}
		
		if (abs(fl_get_slider_value(fd_Play->Buffer) - inList) > 1) {
			fl_set_slider_value(fd_Play->Buffer, (double) inList);
		}
		
	fl_unfreeze_form(fd_Play->Play);
	
	if (record) {
		handleVlanEvents();
		wait = FALSE;
	}
	
	if (video.buffer) {
		info = vlGetLatestValid(video.svr, video.buffer);
		if (info) {
			wait = FALSE;
			vlPutFree(video.svr, video.buffer);
			printf("Read frame\n");
		}
	}
	
	if (wait) sginap(5);
	
	return(0);
}

int Xforms()
{
	char *argv[1];
	int argc = 1, i;
	static int first = TRUE;
	FL_OBJECT obj;
	VLControlValue val;
	char txt[256];
	
	argv[0] = "play";
	
	if (first) {
		fl_initialize(&argc, argv, 0, 0, 0);
		first = FALSE;
	
		fd_Play = create_form_Play();
	
		if (image.width == 768 | image.width == 384) {
			fl_set_form_size(fd_Play->Play, fd_Play->Play->w + 768 - 720, fd_Play->Play->h);
		}
	}
	topwin = fl_show_form(fd_Play->Play,FL_FREE_SIZE,FL_FULLBORDER,"Play");
	videowin = fl_get_canvas_id(fd_Play->video_canvas);

	fl_set_button(fd_Play->Auto_reload, 1);
	fl_deactivate_object(fd_Play->Buffer);
	fl_set_slider_bounds(fd_Play->Buffer, 0.0, (double) maxInList);
	sprintf(txt, "%d", maxInList);
	fl_set_input(fd_Play->Buffer_Size, txt);

	fl_add_canvas_handler(fd_Play->video_canvas, Expose, handleXEvents, 0);
	fl_add_canvas_handler(fd_Play->video_canvas, KeyPress, handleXEvents, 0);
	fl_add_canvas_handler(fd_Play->video_canvas, KeyRelease, handleXEvents, 0);
	fl_add_canvas_handler(fd_Play->video_canvas, ButtonPress, handleXEvents, 0);
	fl_add_canvas_handler(fd_Play->video_canvas, MotionNotify, handleXEvents, 0);

	fl_set_idle_callback(idle_callback, 0);
	
	XFlush(dpy);
	
	exposeWindow();
	motionWindow();
		
	return 0;
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
	int upAndRunning = 0;                  /* don't set this to true until
						      we get our first Expose event*/


	dpy = XOpenDisplay(0);              /* rule # 0: establish our display */
	if (!dpy) {
		fprintf(stderr, "cannot open display\n");
		exit(-1);
	}

	/*
	 * Trim off top and bottom three lines
	 */
	vlGetControl(video.svr, video.path, video.drn, VL_SIZE, &val);
	winHeight = val.xyVal.y-6;
	winWidth = val.xyVal.x;
		
	
	if ((inputthread = sproc ((void (*)(void *))handleEvents, PR_SALL | PR_BLOCK)) == -1) {
		fprintf (stderr, "Unable to sproc to handle events\n");
		perror ("sproc");
		stopAllThreads ();
	}
	
	sigset(SIGCLD, stopAllThreads);
	
	streamDecompress();
}


void abs_frame(double pos)
{
	if (pos < 0.0) pos = 0.0;
	if (pos > 1.0) pos = 1.0;
	pos *= total_frames -1.0;

	playstate.absframe = pos;
	playstate.advanceVideo = 0.0;
	flushAll = TRUE;
}

static int handleXEvents (FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *d)
{
	pid_t ppid = -1;
	static int startx, starty;
	CLimageInfo imageinfo;	
	VLControlValue value, origin, offset;
	Hulptape * hulptape;
	char name[1024];
	double frame;
	
	/* na opstarten:
	 * 
	 * ReparentNotify
	 * ConfigureNotify (begin / scalen / verplaatsen)
	 * MapNotify
	 * Expose (scalen / pop)
	 */
	

	printf("  Xevent %d\r", ev->type);
	fflush(stdout);
	switch(ev->type) {
	case ConfigureNotify:
		motionWindow();
		break;
	case Expose:
		Xforms();
		exposeWindow();
		break;
	case ButtonPress:
		{
			XButtonEvent *bev = (XButtonEvent *)ev;
			printf("button: %d\n", bev->button);
			if (bev->button == Button1) abs_frame(bev->x / (double) ob->w);
			if (bev->button == Button3) abs_frame(bev->x / (double) ob->w);
			if (bev->button == Button2) {
				startx = bev->x;
				starty = bev->y;
			}
			break;
		}
	case MotionNotify:
		{
			XMotionEvent *mev = (XMotionEvent *)ev;
			/*printf("motion: %d\n", mev->state);*/
			if (mev->state & Button1Mask) abs_frame(mev->x / (double) ob->w);
			if (mev->state & Button3Mask) abs_frame(mev->x / (double) ob->w);
			if (mev->state & Button2Mask) {
				vlGetControl(video.svr, video.path, video.drn, VL_ORIGIN, &origin);
				vlGetControl(video.svr, video.path, video.drn, VL_OFFSET, &offset);
				origin.xyVal.x += startx - mev->x;
				origin.xyVal.y += starty - mev->y;
				
				if (origin.xyVal.x < 0) {
					offset.xyVal.x -= origin.xyVal.x;
					origin.xyVal.x = 0;
				}
				if (origin.xyVal.y < 0) {
					offset.xyVal.y -= origin.xyVal.y;
					origin.xyVal.y = 0;
				}
				printf("%d %d\n", origin.xyVal.x, origin.xyVal.y);
				startx = mev->x;
				starty = mev->y;
				vlSetControl(video.svr, video.path, video.drn, VL_ORIGIN, &origin);
				if (vlSetControl(video.svr, video.path, video.drn, VL_OFFSET, &offset) == -1) {
					printf("vlSetControl VL_OFFSET: %s\n", vlStrError(vlGetErrno()));
				}
			}
			break;
		}
	case KeyPress:
	case KeyRelease:
		{
			XKeyEvent *kev = (XKeyEvent *)ev;
			KeySym keysym;
			int buf;
			XLookupString(kev, (char *)&buf, 1, &keysym, 0);

			/* tijdens opname mag alleen op escape gereageerd worden */
			if ((record != 0) && (keysym != XK_Escape)) break;
			
			if (ev->type == KeyPress) {
				switch (keysym) {
/* modifiers */
				case XK_Shift_L:
					modifiers |= LSHIFT;
					break;
				case XK_Shift_R:
					modifiers |= RSHIFT;
					break;
				case XK_Control_L:
					modifiers |= LCTRL;
					break;
				case XK_Control_R:
					modifiers |= RCTRL;
					break;
				case XK_Alt_L:
					modifiers |= LALT;
					break;
				case XK_Alt_R:
					modifiers |= RALT;
					break;
/* arrows */
				case XK_Left:
					if (modifiers & SHIFT) abs_frame(0.0);
					else playstate.relframe = -1;
					break;
				case XK_Right:
					if (modifiers & SHIFT) abs_frame(1.0);
					else playstate.relframe = 1;
					break;
				case XK_Up:
					playstate.relframe = 10;
					break;
				case XK_Down:
					playstate.relframe = -10;
					break;

/* keypad */
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
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 2.0;
					else playstate.defaultspeed = 2.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_3:
				case XK_KP_Page_Down:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 3.0;
					else playstate.defaultspeed = 3.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_4:
				case XK_KP_Left:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 4.0;
					else playstate.defaultspeed = 4.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_5:
				case XK_KP_Begin:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 5.0;
					else playstate.defaultspeed = 5.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_6:
				case XK_KP_Right:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 6.0;
					else playstate.defaultspeed = 6.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_7:
				case XK_KP_Home:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 7.0;
					else playstate.defaultspeed = 7.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_8:
				case XK_KP_Up:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 8.0;
					else playstate.defaultspeed = 8.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_9:
				case XK_KP_Page_Up:
					if (modifiers & SHIFT) playstate.defaultspeed = 1.0 / 9.0;
					else playstate.defaultspeed = 9.0;
					if (playstate.advanceVideo) playstate.advanceVideo = playstate.defaultspeed;
					break;
				case XK_KP_Delete:
				case XK_KP_Decimal:
					playstate.relframe = playstate.defaultspeed;
					break;
				case XK_KP_Enter:
				case XK_Return:
					playstate.advanceVideo = playstate.defaultspeed;
					break;
/* function_keys */
				case XK_F1:
					strcpy(name, playstate.current_edl);
					if (fileselect("Load EDL", name)) {
						load_edl(name, REPLACE);
						playstate.newmain= TRUE;
					}
					break;
/* ascii */
				case XK_g:	/* genlock on / of */
/*
					vlGetControl(video.svr, video.devicePath, video.devNode, VL_SYNC, &value);
					printf("%d\n", value.intVal);
					
					if (value.intVal == VL_SYNC_INTERNAL) value.intVal = VL_SYNC_GENLOCK;
					else value.intVal = VL_SYNC_INTERNAL;
					
					if (vlSetControl(video.svr, video.devicePath, video.devNode, VL_SYNC, &value))
						vlPerror("Warning: unable to set video timing -- ");
*/
					break;
				case XK_i:	/* interactive mode on / of */
				case XK_I:
/*					if (keysym == XK_I) {
						playstate.interactive = 0;
						maxinbuffer = LONG_MAX;
					} else {
						playstate.interactive = playstate.interactive == 0;
	
						if (playstate.interactive) maxinbuffer = IA_FIELDS;
						else maxinbuffer = NON_IA_FIELDS;
					}
					printf("Interactive: %d,  max delay: %d fields\n", playstate.interactive, maxinbuffer);
*/

					printf("Interactive can't be changed anymore...\n");
					break;
				case XK_s: /* Single Step */
					if (load_edl("edl", REPLACE) == SUCCESS) {
						playstate.newmain= TRUE;
					}
					break;
				case XK_n:
					break;
				case XK_r:
					record = TRUE;
					break;
				case XK_x:
					Xforms();
					break;
				case XK_space:
					frame = outputFrame - (nowinbuffer * playstate.advanceVideo) / 2.0;

					if (frame < 0.0) frame = 0.0;
					else if (frame >= total_frames) frame = total_frames - 1;
					
					playstate.absframe = frame;
					playstate.advanceVideo = 0.0;
					flushAll = TRUE;

					break;
				case XK_Q:
					fl_hide_form(fd_Play->Play);
					fl_free_form(fd_Play->Play);
					
					if ((ppid = getppid ()) == -1) {
						perror("Unable to find video thread");
					} else
						if (ppid != 1)
							kill ((pid_t)ppid, SIGTERM);

					stopAllThreads();
					break;
				case XK_Z:
					vlSetControl(video.svr, video.path, video.drn, VL_ORIGIN, &value);
					value.fractVal.numerator = 2;
					value.fractVal.denominator = 1;					
					vlSetControl(video.svr, video.path, video.drn, VL_ZOOM, &value);
					video_zoom = 2;
					
					vlEndTransfer(video.svr, video.path);
					sginap(100);
	vlGetControl(video.svr, video.path, video.src, VL_SIZE, &value);
	value.xyVal.y -= 6;
	
	printf("%d %d\n", value.xyVal.y, value.xyVal.x);
	value.xyVal.y *= video_zoom;
	value.xyVal.x *= video_zoom;
	vlSetControl(video.svr, video.path, video.src, VL_SIZE, &value);
	
					vlBeginTransfer(video.svr, video.path, 0, NULL);
					
					motionWindow();
					break;
				case XK_z:
					vlSetControl(video.svr, video.path, video.drn, VL_ORIGIN, &value);
					value.fractVal.numerator = 1;
					value.fractVal.denominator = 1;					
					vlSetControl(video.svr, video.path, video.drn, VL_ZOOM, &value);
					video_zoom = 1;
					motionWindow();						
					break;
				case XK_question:
					printf("\ncurrent frame: %.0f\n", currentFrame);
					break;
				case XK_Escape:
					if (record == FALSE) {
						printf("Press Q or close window to stop playback\n");
					} else {
						vlanabort = TRUE;
						printf("Aborting edit\n");
					}
					break;
				default:
					printf("key %d\n", keysym);
					break;
				}
				flushAll = TRUE;
			} else {
				switch (keysym) {
/* modifierss */
				case XK_Shift_L:
					modifiers &= ~LSHIFT;
					break;
				case XK_Shift_R:
					modifiers &= ~RSHIFT;
					break;
				case XK_Control_L:
					modifiers &= ~LCTRL;
					break;
				case XK_Control_R:
					modifiers &= ~RCTRL;
					break;
				case XK_Alt_L:
					modifiers &= ~LALT;
					break;
				case XK_Alt_R:
					modifiers &= ~RALT;
					break;
/* arrows */
				/*case XK_Left:
				case XK_Right:
					playstate.advanceVideo = 0;
					break;*/
				}
			}
		}
		break;
	default:
		break;
	}
	
	return(0);
}


static void handleVlanEvents ()
{
	static long inpoint, current, avgcount, oldinbuffer;
	static unsigned long long starttime, now, dlta, before, after;
	static CLimageInfo imageinfo;	
	char * ret, cmd[256];
	double fieldnumber, avg;

	if (vlanabort == TRUE) {
		record = vlanabort = FALSE;
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
					printf("Can't connect to vlan\n");
					record = 0;
				} else {
					strcpy(cmd, "SD ");
					strcat(cmd, tcode_to_string((int) total_frames + 0.5));
					remote_vlancmd(cmd);
					record++;
					sginap(10); /* even laatste handeling af laten maken */
					oldinbuffer = maxinbuffer;
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
						do {
							sginap(1);
							if ( clGetNextImageInfo( codec.Hdl, &imageinfo, sizeof(imageinfo) ) < SUCCESS ) {
									ERROR( "clGetNextImageInfo failed" );
							}
						} while(imageinfo.imagecount & 1);
						starttime = ULONGLONG_MAX;
						remote_vlancmd("LR+");
						avg = 0.0;
						avgcount = 0;
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
				current = string_to_tcode(remote_vlancmd("NOP"));
				dmGetUST(&after);
				if (current > (inpoint - 20)) {
					record++;
					remote_vlancmd("LR-");
				} else {
					printf(" delay: %.2f fields\n", (after - before) / NANOS_PER_FIELD);
					now = after;
					dlta = (inpoint - current) * NANOS_PER_FRAME;
					now += dlta;
					if (starttime > now) starttime = now;
					avg += now / NANOS_PER_FRAME;
					avgcount++;
					
					dlta = now - imageinfo.ustime;
					fieldnumber = (dlta / NANOS_PER_FIELD) + imageinfo.imagecount;
					printf("%.2f %.2f %.2f\n", (25.0 * starttime) / NANOS_PER_SEC, (25.0 * now) / NANOS_PER_SEC, fieldnumber);
				}
				
				break;
			case 5:
				avg /= avgcount;
				starttime = avg * NANOS_PER_FRAME;
				printf("avg: %.2f\n", avg);
				do {
					sginap(1);
					if ( clGetNextImageInfo( codec.Hdl, &imageinfo, sizeof(imageinfo) ) < SUCCESS ) {
							ERROR( "clGetNextImageInfo failed" );
					}
				} while(imageinfo.imagecount & 1);
				
				dlta = starttime - imageinfo.ustime;
				fieldnumber = ((dlta * image.frameRate * 2.0) / NANOS_PER_SEC) + imageinfo.imagecount;
				
				printf("%.2f + %.2f\n", (dlta * image.frameRate * 2.0) / NANOS_PER_SEC, fieldnumber);
				fieldnumber = 2.0 * ffloor((fieldnumber / 2.0) + 0.5);
				maxinbuffer = LONG_MAX;
				start_at_field = fieldnumber - (2 * prerollVideoFrames) - 4;
				record++;
				remote_vlancmd("LR-");	/* troep wissen in buffer */
				fflush(stdout);
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
					break;
				}
				sginap(10);
				break;
			default:
				remote_vlancmd("AB");
				record = 0;
		}
	}
	
	if (record == 0) {
		maxinbuffer = oldinbuffer;
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
	
/*	printf("video %d\n", first);
	if (first) {
		sginap(1);
		first ++;
		return;
	}
*/	
	while (vlPending(video.svr) != 0 ) {
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
			default:
				printf("%s\n", vlEventToName(event.reason));
		}
	}
}

void handleEvents( void )
{
	int	mvFD, xFD, vlFD, maxFD;
	fd_set	theFDSet, loopFDSet;
	struct timeval timeout;
	
	Xforms();

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
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

	FD_ZERO( &theFDSet );
	FD_SET( mvFD, &theFDSet );
	FD_SET( xFD, &theFDSet );
	
	maxFD = mvFD;
	maxFD = MAX2(maxFD, xFD);

	/* 
     * Sit in select(), waiting for a message from the movie
     * player or from the X server. Process events from both. 
     */
	
	while (1) {
		fl_do_forms();
	}
		
		loopFDSet = theFDSet;
		
		if (record) select( maxFD, &loopFDSet, NULL, NULL, &timeout);
		else select( maxFD, &loopFDSet, NULL, NULL, NULL);
		
		if ( FD_ISSET( mvFD, &loopFDSet ) ) {  	   /* movie event */
			handleMovieEvents( );
		} else if ( FD_ISSET( xFD, &loopFDSet ) ) {  	   /* X event */
			fl_check_forms();
		} else if ( FD_ISSET( vlFD, &loopFDSet ) ) {	   /* video event */
			handleVideoEvents();
		}
		
		if (startvl == TRUE) {
			printf("starting\n\n\n\n\n");
/*			vlSelectEvents(video.svr, video.devicePath, VLSyncLostMask | VLSequenceLostMask | VLFrameVerticalRetraceMask);
*/			vlFD = vlGetFD(video.svr);
			FD_SET( vlFD, &theFDSet );
			maxFD = MAX2(maxFD, vlFD);
			startvl++;
		}
}

