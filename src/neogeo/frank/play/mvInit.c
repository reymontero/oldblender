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

 * mvInit.c: Does libmovie (movie library) initialization
 *
 * 
 * Silicon Graphics Inc., June 1994
 */

#include <bstring.h>	/* bcopy() */
#include <libgen.h>	/* basename() */

#include "play.h"

char lastimagename[1024], lastsoundname[1024];
MVid imagemovie = 0, audiomovie = 0;

int mvCloseAll ()
{	
	lastimagename[0] = lastsoundname[0] = 0;
	
/*	if (imagemovie) mvClose(imagemovie);
	if (audiomovie) mvClose(audiomovie);
*/	
	imagemovie = audiomovie = 0;
	
	return(SUCCESS);
}


int mvReInit (Diskmovie * diskmovie, int video)
{
	Shot * shot;
	char * last;
	static int first = TRUE;
	MVid * movie;
	

	if (multiMovie) {
		if (diskmovie->is_open) {
			if (video) imagemovie = diskmovie->movie;
			else audiomovie = diskmovie->movie;
			return (SUCCESS);
		}
	}
	
	if (first) {
		lastimagename[0] = lastsoundname[0] = 0;
		first = FALSE;
	}
	
	shot = (Shot *) diskmovie;
	if (shot->OK == FALSE) return;
	
	if (video) {
		last = lastimagename;
		movie = &imagemovie;
	} else {
		last = lastsoundname;
		movie = &audiomovie;
	}
	
	if (strcmp(last, shot->name) == 0) return (SUCCESS);
	
	
	mvClose(*movie);
	
	
	if (mvOpenFile(shot->name, O_RDONLY, movie) != DM_SUCCESS ) {
		return (FAILURE);
	}
	
	diskmovie->movie = *movie;
	strcpy(last, shot->name);
	
	if (mvFindTrackByMedium(diskmovie->movie, DM_IMAGE, &diskmovie->image) != DM_SUCCESS) return(FAILURE);

	/* en audio voor audio */
	if (video == FALSE) {
		if (mvFindTrackByMedium(diskmovie->movie, DM_AUDIO, &diskmovie->audio) != DM_SUCCESS) return( FAILURE);
	}
	
	diskmovie->is_open = TRUE;
	
	return(SUCCESS);
}

int mvInit (Diskmovie * diskmovie)
{
	Shot * shot;
	const char *compressionScheme;
	int err;
	DMparams *image_params;
	const char	*string;
	
	shot = (Shot *) diskmovie;
	shot->OK = FALSE;
	
	/*
     * Open movie file.
     */

	if ( mvOpenFile(shot->name, O_RDONLY, &diskmovie->movie) != DM_SUCCESS ) {
		fprintf( stderr, "%s: could not open movie file\n%s\n", shot->name, mvGetErrorStr(mvGetErrno()));
		return (FAILURE);
	}

	diskmovie->is_open = TRUE;
	
	/*
     * Find the image track.
     */

	if ( mvFindTrackByMedium(diskmovie->movie, DM_IMAGE, &diskmovie->image) == DM_SUCCESS ) {
		/*
		 * Put information about the image track into the "image"
		 * structure.
		 */
		
		diskmovie->imageframes = mvGetTrackLength(diskmovie->image);
		diskmovie->interlacing = mvGetImageInterlacing (diskmovie->image);
		
		image_params = mvGetParams(diskmovie->image);

		diskmovie->first_blender = 0;

		string = dmParamsGetString(image_params, "FIRST_IMAGE");
		if (string) diskmovie->first_blender = atoi(string);
		
		string = dmParamsGetString(image_params, "BLENDER_1ST_IMG");
		if (string) diskmovie->first_blender = atoi(string);
		
				
		if (image.isTrack) {	/* is er al een movie geopend */
			int orgfields = 2, newfields = 2;
			
			if (image.interlacing == DM_IMAGE_NONINTERLACED) orgfields = 1;
			if (diskmovie->interlacing == DM_IMAGE_NONINTERLACED) newfields = 1;
			
			/* image.height en image.interlacing hier goed zetten 
			 * zodat de eerste movie ook niet interlaced kan zijn
			 */
			
			err = FALSE;			
			
			if (image.width					!= mvGetImageWidth (diskmovie->image)) err = TRUE;
			if (image.height / orgfields	!= mvGetImageHeight (diskmovie->image) / newfields) err = TRUE;

			/*if (image.interlacing  != mvGetImageInterlacing (diskmovie->image)) err = TRUE;*/
			/*if (image.orientation  != mvGetImageOrientation (diskmovie->image)) err = TRUE;*/
			if (err) {
				fprintf(stderr, "Movie image paramaters differ: skipping %s\n", shot->name);
				return (FAILURE);
			}
		} else {
			image.isTrack      = 1;
			image.frameRate    = mvGetImageRate (diskmovie->image);
			image.width        = mvGetImageWidth (diskmovie->image);
			image.height       = mvGetImageHeight (diskmovie->image);
			image.orientation  = mvGetImageOrientation (diskmovie->image);
			image.interlacing  = mvGetImageInterlacing (diskmovie->image);
		}
	}

	/*
     * This program only supports JPEG movies.
     */

	compressionScheme = mvGetImageCompression(diskmovie->image);
	if ( strcmp(compressionScheme, DM_IMAGE_JPEG) != 0 ) {
		fprintf(stderr, "%s: only JPEG movies are supported.", shot->name);
		return(FAILURE);
	}


	/*
     * Find the audio track.
     */

	if (mvFindTrackByMedium(diskmovie->movie, DM_AUDIO, &diskmovie->audio) != DM_SUCCESS) {
		shot->OK = TRUE;
		return (SUCCESS);
	}

	/* audio wordt altijd ongecomprimeerd gelezen voor 1 seconde geldt:
	 * 
	 * bytes / seconde = audio.frameRate * audio.frameSize
	 * 
	 */
	
	/*
     * Put information about the audio track into the "audio"
     * structure. 
     */


	if (audio.isTrack) {
		if (audio.sampleWidth  != mvGetAudioWidth (diskmovie->audio)) err = TRUE;
		if (audio.frameRate    != mvGetAudioRate (diskmovie->audio)) err = TRUE;
		if (audio.channelCount != mvGetAudioChannels (diskmovie->audio)) err = TRUE;
		if (audio.frameSize    != dmAudioFrameSize (mvGetParams (diskmovie->audio))) err = TRUE;
		if (err) {
			fprintf(stderr, "Movie audio paramaters differ: skipping %s\n", shot->name);
			return (FAILURE);
		}
	} else {
		audio.isTrack      = 1;
		audio.frameCount   = mvGetTrackLength (diskmovie->audio);
		audio.sampleWidth  = mvGetAudioWidth (diskmovie->audio);
		audio.frameRate    = mvGetAudioRate (diskmovie->audio);
		audio.channelCount = mvGetAudioChannels (diskmovie->audio);
		audio.frameSize    = dmAudioFrameSize (mvGetParams (diskmovie->audio));
		audio.blockSize    = audio.frameRate / image.frameRate;
	}

	
	diskmovie->audioframes = (image.frameRate * mvGetTrackLength (diskmovie->audio)) / audio.frameRate;

	/*
     * Audio queue is 2 seconds long
     */

	audio.queueSize = audio.frameRate * audio.channelCount * 2;

	shot->OK = TRUE;
	return(SUCCESS);
}

void deinterlaceImage( void* vfrom, void* vto)
{
	char* from = vfrom;
	char* to   = vto;

	int replicate,odd,flip;
	int height, width;
	int i;

	/*
     * Flip and de-interlace
     */
	odd       = (image.interlacing == DM_IMAGE_INTERLACED_EVEN ? 0 : 1);
	height    = image.height;
	width     = image.width * 4;
	replicate = 0;  /* turned off */
	flip      = 0;  /* turned off */

	if (replicate)
		for ( i = 0; i < height/2; i++)
			if (flip) {
				bcopy(from+width*i, to+width*(height-1-2*i), width);
				bcopy(from+width*i, to+width*(height-1-2*i+1), width);
			} 
			else {
				bcopy(from+width*i, to+width*2*i, width);
				bcopy(from+width*i, to+width*(2*i+1), width);
			}
	else /* interleave the two fields */ {
		/* First field */
		for ( i = 0; i < height/2; i++)
			if (flip)
				bcopy(from+width*i, to+width*(height-1-(2*i+odd)), width);
			else
				bcopy(from+width*i, to+width*(2*i+odd), width);
		odd = !odd;

		/* Second field */
		for ( i = 0; i < height/2; i++)
			if (flip)
				bcopy(from+width*(i+(height/2)),
				    to+width*(height-1-(2*i+odd)), width);
			else
				bcopy(from+width*(i+(height/2)), to+width*(2*i+odd), width);
	}
}

