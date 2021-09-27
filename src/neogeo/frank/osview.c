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

#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysmp.h>

main()
{
	long offset;
	int fd,ioffs;
	long loadav[3];

	fd = open("/dev/kmem", O_RDONLY);
	if(fd == -1) {
		perror("open /dev/kmem failed");
		exit (1);
	}

	ioffs = sysmp(MP_KERNADDR, MPKA_AVENRUN);
	if (ioffs == -1){
		perror("sysmp failed");
		close(fd);
		exit(1);
	}
	offset = (long)ioffs & 0x7fffffff;

	while (1) {
		if(lseek (fd, offset, 0) == -1L) {
			perror("lseek failed");
			close(fd);
			exit(1);
		}
	
		if(read(fd,(char *)loadav, sizeof(loadav)) != sizeof(loadav)) {
			perror("read failed");
			close(fd);
			exit(1);
		}
	
		printf("load array: %.3f %.3f %.3f\n",(double)loadav[0]/1000.,
			(double)loadav[1]/1000.,
			(double)loadav[2]/1000.);
		
		sginap(500);
	}
}

