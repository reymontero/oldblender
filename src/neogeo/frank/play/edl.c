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

#include <stdio.h>

int version;
char * comment;


#define STRING_SIZE 128

char * get_next_line(char * buf, long buf_size, FILE *fd) {
	char string[STRING_SIZE];
	
	while (fgets(buf, buf_size, fd)) {
		comment = strchr(buf, '#');
		if (comment) {
			comment[0] = 0;
			comment++;
		}
		/* staat er wel iets of is het een lege regel */
		
		if (sscanf(buf, "%1s", string) > 0) {
			/*printf("found :%s # %s\n", buf, comment);*/
			return(buf);
		}
	}
	
	return (0);
}


char * get_name(char * buf,  char * name) {
	char * pnt, * ret;
	int count;
	
	pnt = strchr(buf, '"');
	
	if (pnt) {
		strcpy(name, pnt+1);
		ret = strchr(pnt+1, '"');
		if (ret) ret++;
	} else {
		sscanf(buf, " %s%n", name, &count);
		ret = buf + count;
	}
	
	pnt = strchr(name, '"');
	if (pnt) pnt[0] = 0;
	
	pnt = strchr(name, '{');
	if (pnt) pnt[0] = 0;
	
	/* printf("get name: `%s` `%s`\n", name, ret); */
	return (ret);
}


void * find_first_shot(char * name) {
	Shot * shot;
	
	shot = shotlist->first;
	
	while (shot) {
		if (strcmp(shot->name, name) == 0) return(shot);
		shot = shot->next;
	}
	
	return (0);
}

void * find_shot(char * name) {
	Shot * shot;
	
	shot = shotlist->last;
	
	while (shot) {
		if (strcmp(shot->name, name) == 0) return(shot);
		shot = shot->prev;
	}
	
	return (0);
}


void * new_hulptape(char * name) {
	Hulptape * hulp;
	
	hulp = CLN(Hulptape);
	
	hulp->header.name = strdup(name);
	hulp->header.type = HULPTAPE;
	hulp->header.flags = NOT_COUNTED;
	addtail(shotlist, hulp);
	
	return(hulp);
}


void * new_diskmovie(char * name) {
	Diskmovie * diskmovie;
	
	diskmovie = CLN(Diskmovie);
	
	diskmovie->header.name = strdup(name);
	diskmovie->header.type = DISKMOVIE;
	diskmovie->header.flags = NOT_COUNTED;
	addtail(shotlist, diskmovie);
	
	return(diskmovie);
}


void * load_diskmovie(char * name) {
	Diskmovie * diskmovie;
	
	diskmovie = new_diskmovie(name);
	if (mvInit(diskmovie) == FAILURE) {
		if (diskmovie->movie) {
			mvClose(diskmovie->movie);
		}
		remlink(shotlist, diskmovie);
		freeN(diskmovie);
		return(0);
	}
	/*****/

	if (multiMovie == FALSE) {
		mvClose(diskmovie->movie);
		diskmovie->is_open = FALSE;
	}
	
	return (diskmovie);
}


long load_all(FILE * fd) {
	long error = FALSE;
	char buf[STRING_SIZE], * pnt;
	char name[STRING_SIZE];
	long offset, start, end;
	double speed;
	char * offsetp, * speedp, * endp, * durationp, *videop, *audiop;
	Hulptape * main, * hulp;
	Shot * shot;
	Diskmovie * diskmovie;
	Edit * edit;
	
	
	hulp = main = new_hulptape("Main");
	
	while(get_next_line(buf, sizeof(buf), fd)) {
		if (strchr(buf, '}')) {
			if (hulp == main) {
				printf("ERROR: encountered to many '}'\n");
				return(FAILURE);
			} else {
				printf("end of hulptape %s\n", hulp->header.name);
				hulp = main;
			}
		} else {
			pnt = get_name(buf, name);
			
			if (strchr(buf, '{')) {
				printf("start hulptape: %s\n", name);
				if (hulp != main) {
					printf("ERROR: Can't nest hulptapes %s in %s\n", name, hulp->header.name);
					return(FAILURE);
				}
				
				hulp = new_hulptape(name);
				if (comment) hulp->comment = strdup(comment);
				
			} else {
				offset = 0;
				start = end = FRAME_UNDEF;
				speed = 1.0;
	
				offsetp = strstr(pnt, "of");				
				speedp = strstr(pnt, "sp");
				durationp = strstr(pnt, "du");
				videop = strstr(pnt, "vi");
				audiop = strstr(pnt, "au");
				
				if (offsetp) {
					sscanf(offsetp, "%*s%ld", &offset);
					offsetp[0] = 0;
					/*printf("offset %d\n", offset);*/
				}
				
				if (speedp) {
					sscanf(speedp, "%*s%lf", &speed);
					speedp[0] = 0;
					/*printf("speed %f\n", speed);*/
				}
				
				if (durationp) {
					sscanf(durationp, "%*s%lf", &speed);
					speed = 1.0 / speed;
					durationp[0] = 0;
					/*printf("speed %f\n", speed);*/
				}
				
				/* wat er over blijft is: [#] - [#] */
				
				endp = strrchr(pnt, '-');
				if (endp) {
					endp[0] = 0;
					if (sscanf(pnt, "%ld", &start) == 1) {
						if (version > 1) start--;
					}
					if (sscanf(endp + 1, "%ld", &end) == 1) {
						if (version > 1) end--;
					}
				} else {
					if (sscanf(pnt, "%ld", &start) == 1) {
						if (version > 1) start--;
					}
					end = start;
				}

				/*printf("name: -%s- start: %d end: %d\n", name, start, end);*/
				
				edit = CLN(Edit);
				
				edit->start = start;
				edit->end = end;

				edit->offset = offset;
				edit->speed = speed;
				
				if (audiop || videop) {
					edit->audioactive = edit->videoactive = FALSE;
					if (audiop) edit->audioactive = TRUE;
					if (videop) edit->videoactive = TRUE;					
				} else {
					edit->audioactive = edit->videoactive = TRUE;
				}
				
				if (comment) edit->comment = strdup(comment);
				
				shot = find_shot(name);
				if (shot == 0) shot = load_diskmovie(name);
				edit->shot = shot;
				
				addtail(&hulp->first, edit);
			}
		}
	}
	
	return(SUCCESS);
}


long load_edl(char * name, long append) {
	FILE *fd;
	char buf[STRING_SIZE];
	long ret = SUCCESS;
	ListBase list;
	Diskmovie * diskmovie;
	Edit * edit;
	Hulptape * maintape;
	
	strcpy(playstate.current_edl, name);
	file_time = get_mtime(name);
	
	if (append == REPLACE) {
		list.first = shotlist->first;
		list.last = shotlist->last;
		shotlist->first = shotlist->last = 0;
	}

	if (mvIsMovieFile(name)) {
		diskmovie = load_diskmovie(name);
		if (diskmovie == 0) {
			fprintf(stderr, "Not a valid movie file: %s\n", name);
			ret = FAILURE;
		} else {
			maintape = new_hulptape("Main");
			
			edit = CLN(Edit);
			edit->start = edit->end = FRAME_UNDEF;
			edit->offset = 0;
			edit->speed = 1.0;
			edit->shot = (Shot *) diskmovie;
			edit->audioactive = edit->videoactive = TRUE;
			addtail(&maintape->first, edit);
			ret = SUCCESS;
		}
	} else {

		fd = fopen(name, "r");
		if (fd == 0){
			perror(name);
			return(FAILURE);
		}
			
		if (get_next_line(buf, sizeof(buf), fd)) {
			if (strncmp(buf, "EDL", 3) == 0) {
				version = atoi(buf + 3);
				if (version == 0) version = 2;				
				ret = load_all(fd);
			} else {
				printf("%s: not a valid EDL\n");
				ret = FAILURE;
			}
		} else {
			printf("%s: size zero ?\n");
			ret = FAILURE;
		}
		
		fclose(fd);
	}
	
	if (ret != SUCCESS && append == REPLACE) {
		shotlist->first = list.first;
		shotlist->last = list.last;
	}
	return(ret);
}

