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

 * play.c: Plays back an SGI Movie JPEG file.
 *
 * 
 * Silicon Graphics Inc., June 1994
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <libgen.h>
#include <unistd.h>	/* geteuid(), getuid(), etc. */

/*
 * for schedctl
 */
#include <limits.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>
#include <sys/lock.h>


/*
 *X / gl include files
 *
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Sgm/GlxMDraw.h>

/*
 *dmdev include files
 */
#include <dmedia/cl.h>
#include <dmedia/cl_cosmo.h>
#include <movie.h>
#include <vl/vl.h>
#include <audio.h>
#include <audiofile.h>

#include "play.h"

/*
 *Global definitions
 */
_Movie movie;
_Image image;
_Codec  codec;
_Audio  audio;
_PlayState playstate;
_Video  video;
_Options options;

ListBase _shotlist = {0, 0}, * shotlist = &_shotlist;
int multiMovie = TRUE;

static void
initglobals( char * myname )
{
    audio.isTrack = 0;

    playstate.advanceAudio = 1;
    playstate.advanceVideo = 1;
    playstate.playMode = PLAY_CONT;
    playstate.waitforkey = 0;
    strcpy(playstate.current_edl, "");

    codec.singleFrame = 0;
    codec.OriginalFormat = CL_RGBX;
    codec.engine = CL_JPEG_COSMO;

    image.isTrack   = 0;
    image.interlacing = DM_IMAGE_INTERLACED_EVEN;
    image.orientation = DM_TOP_TO_BOTTOM;
    image.width           = 768;
    image.height          = 288;
    image.display = VIDEO_2_FIELD;
    image.numberFrames = -1;

    video.dataActive = 0;
    video.dataFrozen = 0;
    video.timingActive = 0;

    options.myname = strdup(basename(myname));
    options.use_default_ports = 1;
    options.audio_port_specified = 0;
    options.playAudioIfPresent = 1;
    options.display_port_specified = 0;
    options.ifilename = NULL;
    options.verbose    = FALSE;
    options.initialLoopMode= LOOP_NOTSPEC;
    options.autostop   = 0;
    options.initialPlayMode= PLAY_CONT;
    options.image_engine = strdup("cosmo");
    options.image_port_type = 0;
    options.vid_device = strdup("ev1");
	options.zoom = 1.0;
}

static void
initPlayState( void )
{
    if ((audio.isTrack) && (options.playAudioIfPresent))
        playstate.audioActive = 1;
    else 
        playstate.audioActive = 0;

    playstate.playMode = options.initialPlayMode;
    playstate.autoStop = options.autostop;

    if (options.initialLoopMode == LOOP_NOTSPEC) 
    {
        if (movie.loopMode == MV_LOOP_CONTINUOUSLY)
            playstate.loopMode = LOOP_REPEAT;
        else  /* XXX MV_LOOP_SWINGING not supported */
            playstate.loopMode = LOOP_NONE;
    }
    else 
    {
        playstate.loopMode = options.initialLoopMode;
    }
	playstate.absframe = -1;
	playstate.relframe = 0;
	playstate.defaultspeed = 1;
	playstate.interactive = TRUE;
	playstate.newmain = FALSE;
}

void
setscheduling()
{
    /*
     * set non-degrading priority
     * use high priority if running as root
     */
    /* 
     * swap permissions so that we become root for a moment. 
     * setreuid() should let us set real uid to effective uid = root
     * if the program is setuid root 
     * 
     */
    setreuid(geteuid(), getuid());
    setregid(getegid(), getgid());

    if (schedctl(NDPRI, 0, NDPHIMIN)< 0)
    {
         fprintf(stderr, "%s: run as root to enable real time scheduling\n",
             options.myname);
    }/* else if (schedctl(SLICE, 0, 1) == -1) printf("no slice \n");*/

    /* swap permissions back, now we're just "joe user" */
    setreuid(geteuid(), getuid());
    setregid(getegid(), getgid());

}

void init_framecount()
{
	Shot * shot;
	Diskmovie * diskmovie;
	
	shot = shotlist->first;
	while (shot) {
		if (shot->type == HULPTAPE) shot->flags = NOT_COUNTED;
		else {
			diskmovie = (Diskmovie *) shot;
			shot->videocount = diskmovie->imageframes;
			
			if (diskmovie->audio) shot->audiocount = diskmovie->audioframes;
			else shot->audiocount = 0;
			
			shot->flags = COUNTED;
		}
		shot = shot->next;
	}
}

int start_end_count(Edit * edit, int framecount)
{
	int start, end;
	
	if (edit->start != -1 || edit->end != -1) {
		start = edit->start;
		end = edit->end;
		if (start == FRAME_UNDEF) start = 0;
		if (end == FRAME_UNDEF) end = framecount - 1;
		framecount = (end - start) + 1;
	}
	
	framecount = (framecount / edit->speed) + 0.5;
	return(framecount);
}

int sub_calc_framecount(Hulptape * tape)
{
	Edit * edit;
	int videocount = 0, audiocount = 0, framecount;
	
	edit = tape->first;
	while (edit) {
		edit->videocount = 0;
		edit->audiocount = 0;

		if (edit->shot) {
			if (edit->shot->flags == NOT_COUNTED) sub_calc_framecount((Hulptape *) edit->shot);

			if (edit->videoactive) edit->videocount = start_end_count(edit, edit->shot->videocount);
			if (edit->audioactive) edit->audiocount = start_end_count(edit, edit->shot->audiocount);
			
			/*printf("%s: framecount: %5d at speed: %f\n", edit->shot->name, edit->videocount, edit->speed);*/

			audiocount += edit->audiocount;
			videocount += edit->videocount;
		}
		
		edit = edit->next;
	}
	
	tape->header.audiocount = audiocount;
	tape->header.videocount = videocount;
	tape->header.flags = COUNTED;	
}

int calc_framecount(Hulptape * tape)
{
	init_framecount();
	sub_calc_framecount(tape);
	
	printf("\n\ntotal duration: %s \n(%d audio / %d video)\n\n", tcode_to_string((int) tape->header.videocount + 0.5), tape->header.audiocount, tape->header.videocount);
}


main (int argc, char *argv[]) 
{
    char *myname = argv[0];
	Hulptape * maintape, * hulptape;
	Diskmovie * diskmovie;
	
	if (argc == 1) exit(0);
	if (fork()) exit(0);


	/*plock(TXTLOCK);*/	/* lock everything in memory */
	
    setreuid(geteuid(), getuid());
    setregid(getegid(), getgid());
	

	if (multiMovie) mvSetNumMoviesHint(40);
	
    initglobals(myname);
	
	if (load_edl(argv[1], REPLACE) != SUCCESS) exit(0);

	maintape = find_shot("Main");
	if (maintape == 0) {
		fprintf(stderr, "Can't find Main entry\n");
		exit(0);
	}
	/* find first movie */
	
	calc_framecount(maintape);
	/* BUG BUG BUG BUG */
	total_frames = maintape->header.videocount;
	
	diskmovie = shotlist->first;

	while (diskmovie->header.type != DISKMOVIE) {
		diskmovie = (Diskmovie *) diskmovie->header.next;
		if (diskmovie == 0) {
			fprintf(stderr, "No movie found\n");
			exit(0);
		}
	}
	
	movie.mv = diskmovie->movie;
	movie.imgTrack = diskmovie->image;
	movie.audTrack = diskmovie->audio;
	
	image.numberFrames = diskmovie->imageframes;
	audio.frameCount = diskmovie->audioframes;
	
    alInit ();
    vlInit ();
    /* 
     * set up initial play state variables based on information
     * from command-line options and movie file header
     */
    initPlayState();
    Xgo();
    exit(0);
}

