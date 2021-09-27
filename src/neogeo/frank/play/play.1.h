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

#ifndef __INC_DMPLAY_H__

#define __INC_DMPLAY_H__ 1

/*
 *dmplay.h: header file for dmplay.h
 *
 *
 *Original version by Rajal Shah. (5/24/94)
 *
 */

#include <stdio.h>

/*
 *X / gl include files
 */
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Sgm/GlxMDraw.h>


#include <sys/types.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/signal.h>

/*
 * dmedia headers
 */
#include <dmedia/dm_image.h>
#include <dmedia/cl.h>
#include <dmedia/cl_cosmo.h>
#include <dmedia/dmedia.h>
#include <dmedia/moviefile.h>
#include <vl/vl.h>
#include <audio.h>
#include <movie.h>

/*
 * local headers
 */

#include <local/util.h>
#include <local/network.h>


#define GRAPHICS               10
#define VIDEO_1_FIELD          11
#define VIDEO_2_FIELD          12

#define PLAY_STEP 0
#define PLAY_CONT 1

#define LOOP_NOTSPEC -1 /* not specified */
#define LOOP_NONE 0     /* no looping */
#define LOOP_REPEAT 1   /* repeat looping */

#define IA_FIELDS		8 		/* Number of fields in buffer when playing interactive */
#define NON_IA_FIELDS	25 		/* Number of fields in buffer when playing non_interactive */

#define NANOS_PER_SEC	1000000000
#define NANOS_PER_FRAME	(NANOS_PER_SEC / image.frameRate)
#define NANOS_PER_FIELD	(NANOS_PER_FRAME / 2)

#define FAST_SPEED 5.0

typedef struct {
    char *filename;
    MVloopmode loopMode;
    MVid mv;           /*Movie*/
    MVid imgTrack;     /*ImgTrack*/
    MVid audTrack;     /*AudTrack*/
} _Movie;

typedef struct {
    int isTrack;  			/*is there a video ('image') track?*/
    int height;
    int width;
    DMinterlacing interlacing;
    DMorientation orientation;
    int display;
    int numberFrames;
    double frameRate;
    int cliptop;
    int clipbottom;
    int clipleft;
    int clipright;
} _Image;

typedef struct {
    int loopMode; /* LOOP_NONE or LOOP_REPEAT */
    int audioActive;
    int playMode; /* PLAY_STEP or PLAY_CONT */
    int advanceAudio; 
    double advanceVideo;
	double defaultspeed;
    int autoStop; /* 0 if wait for user input, 1 if stop automatically */
	int waitforkey;
	double absframe;
	double relframe;
	int interactive;
	int newmain;
	char current_edl[1024];
} _PlayState;

typedef struct {
    CLcompressorHdl Hdl;
    int OriginalFormat;
    int engine;
    int singleFrame;
} _Codec;

typedef struct {
    int        isTrack;					/* is there an audio track ?*/
    ALport     outPort;					/* audio output port */
    ALconfig   config;					/* audio output port configuration */
    int        frameCount;				/* audio track length */
    int        sampleWidth;				/* audio sample width, in bits */
    int	       frameSize;				/* bytes per audio frame */
    int        blockSize;				/* ?? */
    int        channelCount;			/* 1=mono, 2=stereo, etc */
    int        queueSize;				/* audio output port queue */
    double     frameRate;				/* audio sample rate */
} _Audio;

typedef struct {
    VLServer svr;
    VLPath path;		/* video data path from Cosmo */
    VLNode src;			/* from Cosmo */
    VLNode drn;			/* screen out */
    VLNode voutdrn;		/* video out */
    VLNode dataTiming;		/* timing portion of data path */
    VLNode devNode;
    int    dataActive;		/* vlBeginTransfer on data path? */
    int    dataFrozen;		/* has VL_FREEZE been called? */

    VLNode timingSrc;		/* Timing path */
    VLNode timingDrn;
    VLPath timingPath;
    int    timingActive;	/* is the timing path currently active? */
    /*VLPath devicePath;*/
    int    height;
    int    width;
    VLNode memdrn;
	VLDevice	* dev;
	VLBuffer	buffer;
    VLPath otherpath;
} _Video;


/* 
 * Command line options
 */

typedef struct {
    char *myname;                 /* name of this program */
    int use_default_ports;        /* if no -p options specified by user */
    int display_port_specified;   /* if -p graphics or -p video flag seen */
    int audio_port_specified ;    /* if -p audio flag seen */
    int playAudioIfPresent;       /* set when -p audio flag seen */
    int verbose;		  /* -v flag */
    int initialLoopMode;          /* LOOP_NOTSPEC, LOOP_NONE or LOOP_REPEAT */
    int autostop;                 /* 1 if -E auto, 0 if -E key */
    int initialPlayMode;          /* PLAY_CONT if -P cont,PLAY_STEP if -P step*/
    char *ifilename;              /* input file name */
    char *image_engine;	          /* image track decompression engine */
    int image_port_type;          /* image track display: graphics or video*/
    char *vid_device;	          /* video device name: 'ev1' for now */
    int  vid_portnum;             /* video device port num: not used yet */
    float  zoom;                  /* zoom */
} _Options;


#define DISKMOVIE	0
#define HULPTAPE	1

#define REPLACE		0
#define APPEND		1

#define NOT_COUNTED	0
#define COUNTED		1

#define FRAME_UNDEF INT_MIN

typedef struct Shot {
	struct Shot * next, * prev;
	char * name;
	long videocount;			/* voor hoeveel frames is er data */
	long audiocount;			/* voor hoeveel frames is er data */
	long flags;
	long type;
	long OK;
} Shot;

typedef struct Edit{
	struct Edit * next, * prev;
	struct Edit * parent;		/* waar komen we vandaan */
	Shot * shot;				/* wijst naar movie, of naar hulptape */
	char * comment;
	int start, end, offset;
	double speed;
	char audioactive;
	char videoactive;
	long videocount;
	long audiocount;
} Edit;

typedef struct Hulptape {
	Shot header;
	char * comment;
	Edit * first, * last;
} Hulptape;

typedef struct Diskmovie {
	Shot header;
	MVid movie;
	MVid image;
	MVid audio;
	long imageframes;			/* voor hoeveel frames is er video */
	long audioframes;			/* voor hoeveel frames is er audio */
	long interlacing;
	long is_open;
	long first_blender;
} Diskmovie;

extern _Codec codec;
extern _PlayState playstate;
extern _Image image;
extern _Movie movie;
extern _Audio audio;
extern _Video video;
extern _Options options;
extern long putinbuffer, maxinbuffer, nowinbuffer, start_at_field;
extern double currentFrame, total_frames;
extern volatile double outputFrame;
extern int prerollVideoFrames;
extern volatile int inList, maxInList, flushAll;

extern ListBase * shotlist;
extern timestruc_t file_time;
extern timestruc_t get_mtime (char * name);
extern int multiMovie;

extern void setscheduling( void );
extern void stopAllThreads( void );
extern void streamDecompress( void );
extern void Xinit( int* argcp, char* argv[] );
extern void clInit( int );
extern int mvInit (Diskmovie * diskmovie);
extern void alInit( void );
extern void vlInit( void );
extern void Xgo( void );
extern void deinterlaceImage( void* from, void* to );
extern char * remote_vlan_cmd(char * cmd);
extern long load_edl(char * name, long append);
extern long save_edl(char * name);
extern void * new_hulptape(char * name);
extern void * load_diskmovie(char * name);
extern void * find_shot(char * name);
extern MVid dst_image, dst_audio, outputmovie;
extern MVid imagemovie, audiomovie;

/********
*
* ERROR - Report an error and exit the program.
*
* This example program has a very simple error handling mechanism.
* When something goes wrong in a call that was not expected to fail, 
* an error message is printed, all executing threads are stopped, and
* the program quits.
*
********/

#define ERROR(message)							      \
	{								      \
	    fprintf( stderr, "%s: %s\n", options.myname, message );	      \
	    fprintf( stderr, "%s, line %d\n", __FILE__, __LINE__); 	      \
	    stopAllThreads();						      \
	}

#endif /* __INC_DMPLAY_H__ */


