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

 * Single frame decompression / display module.
 *
 * This module does the work for the following playback modes:
 * 
 * play -P cont -p video,engine=cosmoSF  
 * play -P step -p video,engine=cosmoSF 
 * 
 * play -P cont -p graphics,engine=swSF 
 * play -P step -p graphics,engine=swSF 
 * 
 * play -P cont -p graphics,engine=cosmoSF 
 * play -P step -p graphics,engine=cosmoSF 
 *
 * engine=cosmoSF causes the Cosmo codec to operate in 
 *   "single-frame decompression mode"
 * engine=swSF causes the software codec to operate in
 *   "single-frame decompression mode"
 *
 * -P cont   continuous frame advance
 * -P step   manual frame advance triggered by user input (mouse click 
 *           or key press)
 *
 * This module is included to provide example/test code for the
 * "single-frame" decompressor interface. The "SF" engines aren't
 * documented for end users.
 *
 * 
 * Silicon Graphics Inc., June 1994
 */

#include <unistd.h>	/* sginap() */

#include "play.h"

/*
 *frame and data buffers.
 */
static char *compressedBuffer;
static unsigned long *frameBuffer;
static unsigned char *tempBuffer;

/*
 *singleFrameDecompress: Plays back the video using single frame decompression.
 *
 */
void singleFrameDecompress () 
{
    static int i = 0;
    static int first = 1;
    int j, size;

    if (first) 
    {
        clInit(1); /* decompressor */
        
        if ((compressedBuffer = 
                    (char *) malloc(image.width * image.height * sizeof(int))) 
            == NULL) {
            mvClose(movie.mv);
            fprintf (stderr, "Unable to malloc\n");
            stopAllThreads ();
        }
    
        if (image.display == GRAPHICS) 
        {
            if ((frameBuffer = 
                    (unsigned long *) malloc(image.width * image.height * 
                    ((image.interlacing != DM_IMAGE_NONINTERLACED) ? 2:1) *
                                sizeof(int))) == NULL) 
            {
                mvClose(movie.mv);
                fprintf (stderr, "Unable to malloc\n");
                stopAllThreads ();
            }
            if ((tempBuffer = 
                    (unsigned char *)malloc(image.width * image.height * 
                    ((image.interlacing != DM_IMAGE_NONINTERLACED) ? 2:1) *
                                sizeof(int))) == NULL) 
            {
                mvClose(movie.mv);
                fprintf (stderr, "Unable to malloc\n");
                stopAllThreads ();
            }
        } 
        else 
        {
            frameBuffer = CL_EXTERNAL_DEVICE;
        }
    }
    
    
    for (;;)
    {
        if (playstate.advanceVideo) 
        {
            do  /* while playstate.loopMode == LOOP_REPEAT */
            {
                if (i >= image.numberFrames)  
                {
                    i = (playstate.loopMode == LOOP_REPEAT) 
                                        ? 0 : image.numberFrames - 1;
                }
                do 
                {
                    /*Read in a video field from the movie file to buffer.*/
                    size = mvGetCompressedImageSize(movie.imgTrack, i);
                    
                    if ((j = mvReadCompressedImage (movie.imgTrack, i, size,
                                    compressedBuffer)) < 0) 
                    {
                        fprintf(stderr, "Can't read image = %d\n", j);
                        stopAllThreads ();
                    }
                    
                    
                    /*Single frame decompression*/
                    if (clDecompress (codec.Hdl, 
                         ((image.interlacing != DM_IMAGE_NONINTERLACED)? 2:1), 
                         size, 
                         compressedBuffer, 
                         frameBuffer) < SUCCESS) 
                    {
                        fprintf (stderr, 
                             "Error Decompressing frame: %d\n\t Exiting",i);
                        stopAllThreads ();
                    }
                    
                    if (options.verbose)
                    {
                        printf("frame %d\n",i);
                    }
                    i++;
                    if (image.display == GRAPHICS) 
                    {
                        if (image.interlacing == DM_IMAGE_NONINTERLACED) 
                        {
                            lrectwrite (0, 0, image.width-1, image.height-1, 
                                                  frameBuffer);
                        }
                        else 
                        {
			    deinterlaceImage((unsigned char *)frameBuffer, 
                                             (unsigned char *)tempBuffer); 
                            lrectwrite(0, 0, image.width-1, image.height-1,
                                                (unsigned long*)tempBuffer);
                        }
                    } 
                    else  /* VIDEO */
                    {
                        VLControlValue val;
        
                        if (first) 
                        {
                            first = 0;
        
			    if (vlBeginTransfer(video.svr, video.path,
						0, NULL)) {
				vlPerror("screenPath couldn't transfer");
				stopAllThreads ();
			    }
                        }
                    }
                } while ( playstate.playMode==PLAY_CONT && 
						      i < image.numberFrames);
                
                if (playstate.playMode==PLAY_STEP) 
                {
                    goto GETOUT;
                }
            } while (playstate.loopMode == LOOP_REPEAT);
GETOUT:     playstate.advanceVideo = 0;

        }  /* if playstate.advanceVideo */
        else 
        {
            sginap (1);
        }
    } /* while */
}    

