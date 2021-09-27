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

#include <sys/stat.h>


#ifndef	NULL
#define NULL			0
#endif

#ifndef	FALSE
#define FALSE			0
#endif

#ifndef	TRUE
#define TRUE			1
#endif

#define HDRSIZE 512
#define NAMSIZE 200
#define RECURDIR 1
#define FASTDIR 2

struct header{
	char	name[NAMSIZE];
	ulong	size;
	ulong	chksum;
	char	fill[HDRSIZE-NAMSIZE-2*sizeof(ulong)];
};


struct direntry{
	char	*string;
	mode_t	type;
	char	*relname;
	struct	stat s;
	ulong	flags;
	char	size[16];
	char	mode[16];
	char	owner[16];
	char	time[8];
	char	date[16];
};

struct dirlink
{
	struct dirlink *next,*prev;
	char *name;
};

extern int tapectl(long file,short cmd,ulong num);
extern long filesize(long file);
extern long calcchksum(long *point,long size);
extern short appendtape(char *name);
extern short copy(char *cpy,char *org);
extern int exist(char *);
extern ulong getdir(char *dirname,struct direntry **filelist);
extern double diskfree(char *dir);

