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

#include <movie.h>

#include <gl/gl.h>
#include <gl/device.h>
#include <local/iff.h>
/*
zipfork "make animsplit >/dev/console"                                                                           
zipfork "animtest /mo/pics/martin/Custan1B.ani >/dev/console"                                       
*/


main(int argc, char ** argv)
{
	struct anim * anim;
	struct ImBuf * ibuf;
	short val;
	long position;
	long ofsx, ofsy, ibufx;
	char head[128], tail[128], name[256];
	long pic, first = 0, last = 2000000;
	ushort digits;
	

	initmoviepointers();
	
	if (argc == 5) {
		first = atoi(argv[1]) - 1;
		last = atoi(argv[2]) - 1;
		argv += 2;
	} else if (argc == 4) {
		first = atoi(argv[1]) - 1;
		argv++;
	}
	
	if (argc > 2){
		anim = open_anim(argv[1], ANIM_MOVIE | ANIM_ANIM5 | ANIM_ONCE | ANIM_MDEC, IB_planes);
	} else {
		printf("\n\nUsage: animsplit [first [last]] inanim5|inmovie|inmdec outputimage\n");
		printf("first image = 1\n");
		printf("Example:\n");
		printf("    > animspit 1 3 foo.mv 0001\n");
		printf("    will generate frames 0001, 0002 and 0003\n\n");
		exit (0);
	}
	if (anim == 0) perror(0);
	
	pic = stringdec(argv[2], head, tail, &digits);
	pic -= first;
	
	ibuf = anim_nextpic(anim);
	
	if (anim->curtype != ANIM_ANIM5 && anim->curtype != ANIM_MOVIE && anim->curtype != ANIM_MDEC){
		printf("no anim5, movie or mdec\n");
		exit(0);
	}
	
	printf("frames: %d\n", anim->duration);
	
	if (last > anim->duration) last = anim->duration;
	
	for (position = first; position <= last; position++) {
		ibuf = anim_absolute(anim, position);
		if (ibuf) {
			stringenc(name, head, tail, digits, pic + position);
			if (ibuf->planes) {
				if (saveiff(ibuf, name, IB_planes) == 0) {
					perror(name);
					exit(1);
				}
			} else {
				if (saveiff(ibuf, name, IB_rect) == 0) {
					perror(name);
					exit(1);
				}
			}
			printf("saved: %s\n", name);
			freeImBuf(ibuf);
		}
	}
}

