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
#include <sys/stat.h>
#include <sys/param.h>			/* MAXHOSTNAMELEN */

#define malloc(x) mallocN(x,"malloc")
#define free(x) freeN(x)
#define calloc(x,y) callocN((x)*(y),"calloc")

#define R_WAIT 0
#define R_UNKNOWN -1
#define R_BUSY (1 << 0)
#define R_SUSP (1 << 1)

#define TRUE 1
#define FALSE 0

#define PRI_MAX 40
#define PRI_HIGH 30
#define PRI_DEF 20
#define PRI_LOW 10
#define PRI_MIN 0
#define PRI_STOP PRI_MIN

#define FILE_OK 0
#define FILE_REMOVED -1
#define FILE_CHANGED 1

#define RENDER 1
#define SYSTEM 2

#define IDLE 0
#define BUSY 1
#define WAIT 2
#define SUSP BUSY + WAIT
#define FINI 4
#define FINISHING FINI + WAIT + BUSY
#define OFFLINE 8
#define SWAPSTOP 16
#define MEMLOW 32

#define ONE_SHOT 1
#define READS_STDIN 2
#define	EXIT_LINE 4

/* #define MAXLINELEN MAXPATHLEN + MAXNAMELEN */
#define MAXLINELEN 256

typedef struct Line {
	struct Line *next, *prev;
	char buf[MAXLINELEN];
}Line;


#define HEADER																	\
	char infofile[MAXNAMELEN];		/* absolute, system independant filename */	\
	timestruc_t	mtime;							/* date of last modification */	\
	ListBase lines;																\
	ino_t	inode;																\
	char	changed, selected, type, exists, largest;							\
	short	linenum;															\
	short	string_width;														\
	char	string[1024];														\

typedef struct Header {
	struct Header * next, * prev;
	
	HEADER
}Header;

typedef struct Render {
	struct Render * next, * prev;

	HEADER
	
		char _start;

	Line * command;
	Line * file;
	Line * scene;
	Line * interface;
	Line * time;
	
	Line * firstfree;
	
	Line * pri;
	Line * load;
	
	
	int pri_int, load_int, total_int, todo_int, done_int, busy_int, error_int, interface_int;
	long long time_int;
	
		char _end;

	pid_t	renderproc;
	int		busy;
	int		ready;
	int		beep;
}Render;

typedef struct System {
	struct System * next, * prev;

	HEADER

		char _start;

	Line * status;
	Line * max_load;

	/* lines duplicated from active render struct */
	
	Line * render_infofile;
	Line * render_frame;
	Line * render_time;
	
	int status_int, max_load_int, need_update;
	
		char _end;
	
	int wait;
}System;


typedef struct Message {
	int		magic;
	int		type;
	ino_t	inode;
	char	str[256];
}Message;

#define MULTI_MAGIC	'RNDD'

extern void addhead(ListBase *,  void *);
extern void addtail(ListBase *,  void *);
extern void remlink(ListBase *,  void *);
extern void freelinkN(ListBase *,  void *);
extern void freelist(ListBase *);
extern void freelistN(ListBase *);

extern ListBase * renderlist, * systemlist;
extern Line *  set_line(void * header, char * code, char * data);
extern Line *  set_line_val(void * header, char * code, long long);
extern timestruc_t	dir_time;
extern int mcast_w, mcast_r;
extern void handle_mcast_r();
extern int verbose;

