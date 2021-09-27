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


#include <local/util.h>

#include <sys/types.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>

/*

zipfork "cc sound.c -o sound -lm  >/dev/console"                                             
zipfork "./sound  >/dev/console"                                                                              
*/

main(int argc, char ** argv)
{
	long file;
	short j;
	long i;
	
	double freq, samp = 44100, dur = 0.04, step, amp = 1.0;
	long count;
	short * buffer;
	char cmd[256], *soundtmp = "/usr/tmp/tmpsound";
	
	if (argc < 3) {
		printf("usage: %s frequency file [duration] [samplefreq] [amplitude] \n", argv[0]);
		exit(1);
	}
	
	sscanf(argv[1], "%lf", &freq);
	
	file = open(soundtmp, O_WRONLY |O_CREAT | O_TRUNC);
	fchmod(file,0664);
	
	if (file == -1) {
		perror(argv[2]);
		exit(1);
	}
	
	if (argc > 3) sscanf(argv[3], "%lf", &dur);
	if (argc > 4) sscanf(argv[4], "%lf", &samp);
	if (argc > 5) sscanf(argv[5], "%lf", &amp);
	
	printf("Generating tone of %.2f Hz, duration %.2f s with samplefreq %.2f Hz, amplitude %.2f\n", freq, dur, samp, amp);

	count = (dur * samp) + 0.5;
	step = 2 * M_PI * freq / samp;
	
	buffer = malloc(count * sizeof(short));
	
	for (i = 0 ; i < count; i++) buffer[i] = (amp * 32767.0 * sin(i * step));
	
	write(file, buffer, count* sizeof(short));
	close(file);
	
	sprintf(cmd, "sfconvert %s %s -i int 16 2scomp chan 1 rate %f end format aiff", soundtmp, argv[2], samp);
	printf("converting...\n");
	system(cmd);
	printf("done.\n");
	/*remove(soundtmp);*/
}



