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

#include "play.h"

#include "forms.h"
#include "Play.h"

extern FD_Play *fd_Play;
extern void abs_frame(double);
extern int winWidth, winHeight;
MVid _outputmovie, dst_image, src_image, dst_audio, src_audio, outputmovie = 0;
DMparams	*movie_params, *image_params = 0, *audio_params = 0;

/* callbacks for form Play */
void canvas_callback(FL_OBJECT *ob, long data)
{
  printf("in canvas callback\n");
}

void movie_pos(FL_OBJECT *ob, long data)
{
	double f;
	
	switch (data) {
		case 0:
			abs_frame(fl_get_slider_value(ob));
			break;
		case 1:
			abs_frame(0.0);
			break;
		case 2:
			if (playstate.autoStop) {
				playstate.autoStop = FALSE;
				playstate.advanceVideo =  playstate.defaultspeed;
			} else {
				playstate.autoStop = TRUE;
				playstate.absframe = 0;
			}
			break;
		case 3:
			abs_frame(1.0);
			break;
		case 4:
			f = (atof(fl_get_input(ob)) - 0.5) / (total_frames - 1);
			abs_frame(f);
			break;
	}
	flushAll = TRUE;
}

void buffer_size(FL_OBJECT *ob, long data)
{
	int maxinlist;
	
	maxinlist = atoi(fl_get_input(ob));
	if (maxinlist < 10) {
		maxinlist = 10;
		fl_set_input(ob, "10");
	}
	if (maxinlist > 100) {
		maxinlist = 100;
		fl_set_input(ob, "100");
	}
	
	maxInList = maxinlist;
	fl_set_slider_bounds(fd_Play->Buffer, 0.0, maxInList);
	flushAll = TRUE;
}

void movie_speed(FL_OBJECT *ob, long data)
{
	char speedc[128];
	double speedf;
	
	switch (data) {
		case 0:
		case 1:
		case -1:
			/* stop, play, reverse */
			playstate.advanceVideo = data;
			break;
		case 2:
		case -2:
			/* fastFwd, fastRev */
			playstate.advanceVideo = data * FAST_SPEED / 2.0;
			break;
		case 3:
			/* slider */
			playstate.advanceVideo = 2.0 * (fl_get_slider_value(ob) - 0.5);
			break;
		case 4:
			/* numval */
			playstate.advanceVideo = atof(fl_get_input(ob));
			if (playstate.advanceVideo) playstate.defaultspeed = playstate.advanceVideo;
			break;
	}
	flushAll = TRUE;
}

void video_speed(FL_OBJECT *ob, long data)
{

}

void timecode(FL_OBJECT *ob, long data)
{

}

void set_in_out(FL_OBJECT *ob, long data)
{
 
}

void video_pos(FL_OBJECT *ob, long data)
{

}

void video_init(FL_OBJECT *ob, long data)
{

}

void add_controls(FL_OBJECT *ob, long data)
{

}

void new_file(FL_OBJECT *ob, long data)
{
	
}

void nop(FL_OBJECT *ob, long data)
{
	
}

void render(FL_OBJECT *ob, long data)
{
	char name[1024];
	
	if (outputmovie) return;
	
	strcpy(name, "/render/rt.mv");
	/*if (fileselect("Select output Movie", name)) {*/
	if (fileselect("Select output Movie", name)) {
		/*printf("\nstart %d\n", imagemovie); fflush(stdout);*/
		movie_params = mvGetParams(imagemovie);
		/*printf("\nget_params\n"); fflush(stdout);*/
		if (mvCreateFile(name, movie_params, NULL, &_outputmovie) != DM_SUCCESS) {
			perror("mvCreateFile");
		} else {
			chmod(name, 0666);
			/*printf("\nmovie_opened %d\n", &_outputmovie); fflush(stdout);*/
			if (mvFindTrackByMedium(imagemovie, DM_IMAGE, &src_image) == DM_SUCCESS) {
				image_params = mvGetParams(src_image);
				/*printf("\nget_params %d %d\n", src_image, image_params); fflush(stdout);*/
				if (mvAddTrack(_outputmovie, DM_IMAGE, image_params, NULL, &dst_image)) image_params = 0;
				/*printf("\nimage_opened %d\n", dst_image); fflush(stdout);*/
			}
			
			if (audiomovie) {
				if (mvFindTrackByMedium(audiomovie, DM_AUDIO, &src_audio) == DM_SUCCESS) {
					audio_params = mvGetParams(src_audio);
					if (mvAddTrack(_outputmovie, DM_AUDIO, audio_params, NULL, &dst_audio)) audio_params = 0;
					/*printf("\naudio_opened %d %d\n", &dst_audio, audio_params); fflush(stdout);*/
				}
			}
			/*printf("\nmovie_end\n"); fflush(stdout);*/
			if (_outputmovie) {
				abs_frame(0.0);
				while (flushAll) sginap(1);
				outputmovie = _outputmovie;
			}
		}
	}
}

void stop_render(FL_OBJECT *ob, long data)
{
	if (outputmovie) {
		outputmovie = 0;
		playstate.advanceVideo = 0.0;
		flushAll = TRUE;
		while (flushAll) sginap(1);
		mvClose(_outputmovie);
	}
}

