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

#include <signal.h>
#include <string.h>				/* strcpy etc.. */
#include <fcntl.h>
#include <stdio.h> 
#include <errno.h>
#include <local/util.h>
#include <local/network.h>
#include <stdlib.h>				/* getenv */
#include <signal.h>				/* kill */
#include <sys/param.h>			/* MAXHOSTNAMELEN */
#include <limits.h>				/* INT_MAX ed */
#include <sys/prctl.h>			/* sproc */
#include <sys/stat.h>			/* stat */
#include <ctype.h>				/* isspace */
#include <sys/wait.h>
#include <sys/file.h>			/* flock */
#include <time.h>
#include <dirent.h>

#include "renderd.h"

/*
 * osview informatie: /usr/include/sys/sysinfo.h en sysmp.h
 */

#include <fam.h>

long go = TRUE, active_kill = FALSE;
pid_t renderproc = 0;

char myname[MAXHOSTNAMELEN];
int verbose = 2;

char daemon_dir[256];
timestruc_t	dir_time = {0, 0};
long portnumber;

int status = IDLE, max_load = 32;

FAMConnection	_fc, * fc = &_fc;
FAMEvent		_fe, * fe = &_fe;
FAMRequest		_fr, * fr = &_fr;
FAMRequest		_fr2, * fr2 = &_fr2;

int famfd = -1, sock = -1;

/* pipes: 0 = read, 1 = write */

int Stdin[2] = {-1, -1};
int Stdout[2] = {-1, -1};
int Stderr[2] = {-1, -1};

int mcast_w = -1, mcast_r = -1;
struct		sockaddr_in	addr_w, addr_r;

/*

	zipfork "make renderd >/dev/console"                                          
	zipfork "echo must be typed >  /dev/console"                                                        

*/

/* mogelijke optimalisaties
 * 
 * - Dir alleen opnieuw uitlezen als er inderdaad iets in gewijzigd is.
 * - berichten versturen op het moment dat files verwijderd worden zodat
 *   je niet telkens hoeft te pollen.
 * 
 */


ListBase _renderlist = {0, 0}, *renderlist = &_renderlist;
ListBase _systemlist = {0, 0}, *systemlist = &_systemlist;
ListBase _connectlist = {0, 0}, *connectlist = &_connectlist;

typedef struct Connect {
	struct Connect * next, * prev;

	char name[MAXNAMELEN];
	char buffer[4096];
	int connection;
	int dead;
}Connect;


Render * active_render = 0;
System * this_system = 0;

Line _active_line, *active_line = &_active_line;

void perror2(char * s1, char * s2) {
	char buf[1024];
	int olderr;
	
	olderr = errno;
	sprintf(buf, "%s: %s", s1, s2);
	
	errno = olderr;
	perror(buf);
}


void multicast(int type, ino_t inode, char * str)
{
	Message	message;
	int		count;
	
	message.magic	= MULTI_MAGIC;
	message.type	= type;
	message.inode	= inode;
	strncpy(message.str, str, sizeof(message.str));
	count = sendto(mcast_w, &message, sizeof(message), 0, &addr_w, sizeof(addr_w));
	if (count < 0) {
		perror("multicast");
	}
}


Line * add_line(void * head) {
	Line * line;
	Header * header = head;
	
	if (header == 0) return (0);
	
	line = NEW(Line);
	addtail(&header->lines, line);
	
	return(line);
}


Line *  set_line(void * head, char * code, char * data) {
	Line * line, * last = 0;
	Header * header = head;
	int len;
	
	if (header == 0) return (0);
	line = header->lines.first;
	len = strlen(code);
	
	while (line) {
		if (strncmp(line->buf, code, len) == 0) {
			if (last != 0) freelinkN(&header->lines, last);
			last = line;
		}
		line = line->next;
	}
	
	if (last == 0) last = add_line(header);
	sprintf(last->buf, "%s\t%s\n", code, data);
	
	/* dubbele \n weghalen */
	
	len=strlen(last->buf);
	if (len > 2) {
		if (last->buf[len-2] == '\n') last->buf[len-1] = 0;
	}
	
	return(last);
}


Line *  set_line_val(void * head, char * code, long long val) {
	char data[MAXLINELEN];
	
	sprintf(data, "%lld", val);
	
	return(set_line(head, code, data));
}


timestruc_t get_mtime (char * name)
{
	struct stat statbuf;
	timestruc_t nulltime = {0, 0};
	
	if (stat(name, &statbuf) == -1) return (nulltime);
	
	return(statbuf.st_mtim);
}


void free_header(void * head)
{
	Header * header = head;
	
	if (header == 0) return;
	
	freelistN(&header->lines);
	if (header->type == RENDER) freelinkN(renderlist, header);
	else if (header->type == SYSTEM) freelinkN(systemlist, header);
	else {
		printf("Error: can't determine type (%d): %s\n", header->type, header->infofile);
	}
}


void check_frames(Render * render)
{
	Line * line;
	
	if (render == 0) return;
	render->firstfree = 0;
	
	render->total_int = render->todo_int = render->busy_int = render->error_int = 0;
	
	line = render->lines.last;
	while (line) {
		switch (line->buf[0]) {
			case ' ':
				render->firstfree = line;
				render->total_int++;
				render->todo_int++;
				break;
			case '.':
				render->total_int++;
				render->busy_int++;
				break;
			case 'E':
				render->total_int++;
				render->error_int++;
				break;
			case '*':
				render->total_int++;
				render->done_int++;
				break;
		}
		line = line->prev;
	}
}

void interpret_render(void * head)
{
	Render * render = head;
	Line * line;
	
	memset(&render->_start, 0, &render->_end - &render->_start);
		
	line = render->lines.first;
	while (line) {
		switch (line->buf[0]) {
			case 'C':
				render->command = line;
				break;
			case 'F':
				render->file = line;
				break;
			case 'S':
				render->scene = line;
				break;
			case 'T':
				render->time = line;
				break;
			case 'I':
				render->interface = line;
				break;
			case 'P':
				render->pri = line;
				break;
			case 'L':
				render->load = line;
				break;			
		}
		line = line->next;
	}
	
	/* alle andere variableen worden op 0 gezet door de memset */
	render->pri_int = PRI_DEF;
	
	if (render->pri) render->pri_int = atoi(render->pri->buf + 2);
	if (render->load) render->load_int = atoi(render->load->buf + 2);
	if (render->time) render->time_int = atoll(render->time->buf + 2);
	if (render->interface) render->interface_int = atoll(render->interface->buf + 2);

	check_frames(render);
}


void interpret_system(void * head)
{
	System * system = head;
	Line * line;
	
	memset(&system->_start, 0, &system->_end - &system->_start);
		
	line = system->lines.first;
	while (line) {
		switch (line->buf[0]) {
			case 'S':
				system->status = line;
				break;
			case 'L':
				system->max_load = line;
				break;
			case '.':
				system->render_frame = line;
				break;
			case 'F':
				system->render_infofile = line;
				break;	
			case 'T':
				system->render_time = line;
				break;	
		}
		line = line->next;
	}
		
	if (system->status) system->status_int = atoi(system->status->buf + 2);
	if (system->max_load) system->max_load_int = atoi(system->max_load->buf + 2);
}


int reread_infofile(int fd, void * head)
{
	char	* buf, * start, * end;
	int		size;
	Line	* line;
	Header	* header = head;
	
	if (fd == -1) return (-1);
	
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	
	buf = mallocN(size + 1, "Read_infofile");

	if (read(fd, buf, size) != size) {
		freeN(buf);
		return(-1);
	}
	buf[size] = '\n';
	
	freelistN(&header->lines);
	
	start = end = buf;
	
	for (;size > 0; size --) {
		if (*end == '\n') {
			if (start != end) {		/* empty line */
				line = add_line(header);
				memcpy(line->buf, start, end - start + 1);
				line->buf[end - start + 1] = 0;
				/*printf("Read: [%s]\n", line->buf);*/
			}
			start = end + 1;
		}
		end++;
	}
	
	freeN(buf);
		
	if (header->type == RENDER) interpret_render(header);
	else if (header->type == SYSTEM) interpret_system(header);
	
	header->mtime = get_mtime(header->infofile);
	header->changed = FALSE;
	
	return(0);
}

int update_infofile(void * head)
{
	int fd, error = 0;
	Header * header = head;
	
	fd =open_r(header->infofile);
	if (fd != -1) {
		error = reread_infofile(fd, header);
		close(fd);
	} error = -1;
	
	return(error);
}

void set_inode(Header * header)
{
	struct stat		Stat;
	
	if (stat(header->infofile, &Stat)) {
		printf("Can't stat %s\n", header->infofile);
	}
	header->inode = Stat.st_ino;
}

Header * read_infofile(char * name,  int type)
{
	Header			* header = 0;
	int				fd;
	
	fd = open_r(name);

	if (fd != -1) {
		if (type == RENDER) header = (Header *) CLN(Render);
		else if (type == SYSTEM) header = (Header *) CLN(System);
		
		strcpy(header->infofile, name);
		header->type = type;
		
		if (reread_infofile(fd, header) == -1) {
			free_header(header);
			header = 0;
		}

		if (header) set_inode(header);

		close(fd);
	}
	
	if (header != 0) {
		if (header->type == RENDER) addtail(renderlist, header);
		else if (header->type == SYSTEM) addtail(systemlist, header);
	}
	
	return (header);
}

int open_new(char * name)
{
	int fd;
	
	fd = open(name, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		perror2("open_new", name);
		return(fd);
	}
	
	if (flock(fd, LOCK_EX) == -1) {
		perror2("lock_new", name);
		close(fd);
		return(-1);
	}
	
	return(fd);
}


int open_rw(char * name)
{
	int fd;
	
	fd = open(name, O_RDWR);
	if (fd == -1) {
		perror2("open_rw", name);
		return(fd);
	}
	
	if (flock(fd, LOCK_EX) == -1) {
		perror2("lock_rw", name);
		close(fd);
		return(-1);
	}
	
	return(fd);
}

int open_r(char * name)
{
	int fd;
	
	fd = open(name, O_RDONLY);
	if (fd == -1) {
		perror2("open_r", name);
		return(fd);
	}
	
	if (flock(fd, LOCK_SH) == -1) {
		perror2("lock_r", name);
		close(fd);
		return(-1);
	}
	
	return(fd);
}

int check_infofile(void * head)
{
	timestruc_t mtime;
	Header * header = head;
	
	if (header) {		
		mtime = get_mtime(header->infofile);
		if (memcmp(&mtime, &header->mtime, sizeof(mtime))) {
			if (mtime.tv_sec == 0) {
				mtime = get_mtime(header->infofile);
				if (mtime.tv_sec == 0 && errno == ENOENT) {
					if (verbose > 2) printf("file %s was removed\n", header->infofile);
					dir_time.tv_sec = 0; /* patch */
					return(FILE_REMOVED);
				}
			} else {
				return(FILE_CHANGED);
			}
		}
	
		if (header->changed) return (FILE_CHANGED);

		return(FILE_OK);
	}
	
	/* ???? wat is wijsheid ???? */
	
	return (FILE_REMOVED);
}

int sync_dir()
{
	timestruc_t mtime;
	DIR * dirp;
	Header * header, *next;
	Render * render;
	struct dirent *direntp;
	char buf[MAXPATHLEN + MAXNAMELEN];
	int type, removed = FALSE;
	ListBase * list;
	time_t curtime;
	

	/* remove old files */
	
		curtime = time(0);
	
		render = renderlist->first;
		while (render) {
			if (render->done_int == render->total_int) {
				if (curtime - render->mtime.tv_sec > 3600) {
					remove(render->infofile);
					removed = TRUE;
				}
			}
			render = render->next;
		}
		
		if (removed) multicast('RDir', 0, "");
	
	mtime = get_mtime(daemon_dir);

	if (memcmp(&mtime, &dir_time, sizeof(mtime))) {
		dir_time = mtime;
		if (verbose > 2) printf("Dir changed\n");
		
		header = renderlist->first;
		while (header) {
			header->exists = FALSE;
			header = header->next;
		}

		header = systemlist->first;
		while (header) {
			header->exists = FALSE;
			header = header->next;
		}

		dirp = opendir(daemon_dir);
	
		while ((direntp = readdir( dirp )) != 0 ) {
			if (direntp->d_name[0] == '@') {
				if (verbose > 3) printf("found: %s\n", direntp->d_name);
				type = RENDER;
				list = renderlist;
			} else if (direntp->d_name[0] == ':') {
				if (verbose > 3) printf("found: %s\n", direntp->d_name);
				type = SYSTEM;
				list = systemlist;
			} else type = 0;
			
			if (type) {
				sprintf(buf, "%s/%s", daemon_dir, direntp->d_name);

				header = list->first;
				while (header) {
					if (strcmp(header->infofile, buf) == 0) break;
					header = header->next;
				}
				
				if (header == 0) header = read_infofile(buf, type);
				if (header) header->exists = TRUE;
			}
		}
	
		closedir( dirp );
		
		header = (Header *) renderlist->first;
		while (header) {
			next = header->next;
			if (header->exists == FALSE) {
				if (verbose > 3) printf("Removed: %s\n", header->infofile);
				free_header(header);
			}
			header = next;
		}
		
		header = (Header *) systemlist->first;
		while (header) {
			next = header->next;
			if (header->exists == FALSE) free_header(header);
			header = next;
		}
		
		return(TRUE);
	}
	return(FALSE);
}

int sync_files(Header * header)
{
	Header * next;
	int update = FALSE, fstat;
	
	while (header) {
		next = header->next;
		fstat = check_infofile(header);
		
		if (fstat != FILE_OK) {
			update = TRUE;
			if (fstat == FILE_REMOVED) {
				free_header(header);
			} else if (fstat == FILE_CHANGED) {
				update_infofile(header);
			}
		}
		header = next;
	}
	
	return(update);	
}


Render * find_best_render()
{
	Render * best = 0, * render;
	int fd, ok;
	
	sync_dir();
	
	/* check if all files are still up to date */
	
	sync_files(renderlist->first);
	
	/* find the best one */
	
	render = renderlist->first;
	
	while (render) {
		/* is er nog iets te renderen ?? */
		if (render->pri_int != PRI_STOP && render->firstfree != 0) {

			ok = TRUE;
			if (render->interface_int & EXIT_LINE) {
				/* als dit het laatste plaatje is (EXIT dus), mag dat alleen maar uitgevoerd worden
				 * als ALLES klaar is.
				 */
				 
				if (render->todo_int == 1 && (render->busy_int != 0 || render->error_int != 0)) ok = FALSE;
			}
			
			if (ok) {
				/* load moet wel kleiner of gelijk zijn */
				if (render->load_int <= max_load) {
					if (best == 0) {
						/* er is nog niets dus deze is het best */
						best = render;
					} else if (render->pri_int > best->pri_int) {
						/* prioriteit is hoger */
						best = render;
					} else if (render->pri_int == best->pri_int) {
						if (render->time_int < best->time_int) {
							/* prioriteit is gelijk maar file is ouder */
							best = render;
						}
					}
				}
			}
		}
		render = render->next;
	}
		
	return (best);
}


Header * inode_to_header(ino_t inode)
{
	Header * header;
	
	header = systemlist->first;
	while (header) {
		if (header->inode == inode) break;
		header = header->next;
	}
	
	if (header == 0) {
		header = renderlist->first;
		while (header) {
			if (header->inode == inode) break;
			header = header->next;
		}
	}
	
	if (header == 0) {
		 if (verbose > 2) printf("couldn't find entry for inode %d\n", inode);
	}
	
	return(header);
}

void handle_mcast_r()
{
	Message	message;
	int		count, addrlen, num;
	Header	* header;
	fd_set  readers;
	struct  timeval time_out;

	do {
		count = recvfrom(mcast_r, &message, sizeof(message), 0, &addr_r, &addrlen);
		if (count < 0) {
			perror("recvfrom");
		} else if (count != 0) {
			if (message.magic != MULTI_MAGIC) {
				printf("recieved unknown multicast message from %s : [%s] ???\n",
					inet_ntoa(addr_r.sin_addr),
					& message);
			} else {
				switch(message.type) {
					case 'UpDt':
						header = inode_to_header(message.inode);
						if (header) {
							header->changed = TRUE;
							if (verbose > 2) printf("Recieved UpDt for %s\n", header->infofile);
						} else { /* veranderd !!! */
							/* this is a new file I don't know about so read the dir */
							if (verbose > 2) printf("Recieved UpDt for new: %d\n", message.inode);
							dir_time.tv_sec = 0;
						}
						break;
					case 'RDir':
						if (verbose > 2) printf("Recieved RDir\n");
						dir_time.tv_sec = 0;
						break;
					default:
						printf("What's this: %s\n", &message.type);
						break;
				}
			}
		}
		
		/* check for more messages */
		
		FD_ZERO(&readers);
		FD_SET(mcast_r, &readers);
		
		time_out.tv_sec = 0;
		time_out.tv_usec = 0;
	} while ((num = select(FD_SETSIZE,&readers,0,0,&time_out)) > 0);
	
	if (num == -1 && errno != EINTR) perror("select handle_mcast_r");
}


int write_infofile(int fd,  void * head)
{
	Line * line;
	Header * header = head;
	int len;
	
	if (fd == -1) return (-1);
	if (header == 0) return (-1);
	
	lseek(fd, 0, SEEK_SET);
	
	line = header->lines.first;

	while (line) {
		len = strlen(line->buf);

		if (write(fd, line->buf, len) != len) {
			perror2("write: possibly corrupted", header->infofile);
			close(fd);
			return (-1);
		}
		line = line->next;
	}

	/* hier wordt geen error-checking gedaan, pas dus op */
	
	ftruncate(fd, lseek(fd, 0, SEEK_CUR));
	fsync(fd);
	
	while (check_infofile(header) == FILE_OK) {
		printf("HEY: file didn't change !???\n");
		sginap(10);
	}

	close(fd);
		
	set_inode(header);

	if (header->inode) {
		multicast('UpDt', header->inode, "");
	} else {
		printf("%s inode = 0???\n", header->infofile);
	}
	
	if (header->type == RENDER) check_frames((Render *) header);
	
	header->mtime = get_mtime(header->infofile);

	return(0);
}

Render * make_renderfile_old(char * command, char * name, char * scene, int sfra, int efra)
{
	Render * render;
	Line * line;
	char buf[MAXPATHLEN + MAXNAMELEN];
	int i, fd, error;
	
	render = CLN(Render);
	render->type = RENDER;
	
	sprintf(buf, "%s -b %s -S %s -f ", command, name, scene);
	set_line(render, "C", buf);

	set_line(render, "F", name);
	set_line(render, "S", scene);

	set_line_val(render, "I", ONE_SHOT);
	set_line_val(render, "T", time(0));
	
	set_line_val(render, "P", PRI_DEF);
	set_line_val(render, "L", 16);
	
	for (i = sfra; i <= efra; i++) {
		line = add_line(render);
		sprintf(line->buf, "  %04d\n", i);
	}
	
	strcpy(buf, name);
	
	for (i = 0; buf[i] != 0 ; i++) {
		if (buf[i] == '/') buf[i] = '@';
	}
	
	sprintf(render->infofile, "%s/%s~%s", daemon_dir, buf, scene);
		
	fd = open_new(render->infofile);
	
	if (fd != -1) error = write_infofile(fd, render);
	else error = TRUE;
	
	if (error) {
		remove(render->infofile);
		freelistN(&render->lines);
		freeN(render);
		render = 0;
	} else {
		addtail(renderlist, render);
		interpret_render(render);
	}
	
	return(render);
}


Render * make_partial_renderfile(char * command, char * name, char * scene, int sfra, int efra, int no, int last)
{
	Render * render;
	Line * line;
	char buf[MAXPATHLEN + MAXNAMELEN];
	char tmp[MAXPATHLEN + MAXNAMELEN];
	int i, fd, error;
	time_t	curtime;
	static time_t prevtime = 0;
	
	/* remove old entry if it exists */
	
		strcpy(buf, name);
		
		for (i = 0; buf[i] != 0 ; i++) {
			if (buf[i] == '/') buf[i] = '@';
		}

		if (no) sprintf(tmp, "%s/%s~%s.%02d", daemon_dir, buf, scene, no);
		else sprintf(tmp, "%s/%s~%s", daemon_dir, buf, scene);
		
		render = renderlist->first;
		while(render) {
			if (strcmp(render->infofile, tmp) == 0) break;
			render = render->next;
		}
		
		if (render) free_header(render);
	
	render = CLN(Render);
	render->type = RENDER;
	render->beep = TRUE;
	
	strcpy(render->infofile, tmp);
	
	
	sprintf(buf, "%s -b %s -S %s -I ", command, name, scene);
	set_line(render, "C", buf);

	set_line(render, "F", name);
	set_line(render, "S", scene);

	if (last) set_line_val(render, "I", READS_STDIN | EXIT_LINE);
	else set_line_val(render, "I", READS_STDIN);
	
	curtime = time(0);
	if (curtime <= prevtime) curtime = prevtime + 1;
	prevtime = curtime;
	
	set_line_val(render, "T", curtime);
	set_line_val(render, "P", PRI_DEF);
	set_line_val(render, "L", 16);
	
	/* add frames */

		for (i = sfra; i <= efra; i++) {
			line = add_line(render);
			sprintf(line->buf, "  %04d\n", i);
		}
	
	/* add exit line */
	
		if (last) {
			line = add_line(render);
			sprintf(line->buf, "  EXIT\n");
		}
	
	fd = open_new(render->infofile);
	
	if (fd != -1) error = write_infofile(fd, render);
	else error = TRUE;
	
	if (error) {
		remove(render->infofile);
		freelistN(&render->lines);
		freeN(render);
		render = 0;
	} else {
		addtail(renderlist, render);
		interpret_render(render);
	}
	
	return(render);
}


Render * make_renderfile(char * command, char * name, char * scene, int sfra, int efra)
{
	int number = 0;
	
	while ((efra - sfra) > 1000)
	{
		number++;
		make_partial_renderfile(command, name, scene, sfra, sfra + 999, number, FALSE);
		sfra += 1000;
	}
	
	if (number) number++;
	
	make_partial_renderfile(command, name, scene, sfra, efra, number, TRUE);
}

System * make_systemfile(char * name)
{
	System * system;
	Line * line;
	char buf[MAXPATHLEN + MAXNAMELEN];
	int i, fd, error;
	
	system = CLN(System);
	system->type = SYSTEM;
	
	strcpy(system->infofile, name);
	
	system->status = set_line_val(system, "S", status);
	system->max_load = set_line_val(system, "L", max_load);

	fd = open_new(system->infofile);
	
	if (fd != -1) error = write_infofile(fd, system);
	else error = TRUE;
	
	if (error) {
		remove(system->infofile);
		freelistN(&system->lines);
		freeN(system);
		system = 0;
	} else addtail(systemlist, system);
	
	return(system);
}


void update_active_render(char result)
{
	int fd, check;
	Line * line;
	
	/* gegevens van dit systeem goedzetten */

	status &= ~BUSY;
	status &= ~FINI;
	
	if (this_system->render_infofile) freelinkN(&this_system->lines, this_system->render_infofile);
	if (this_system->render_time) freelinkN(&this_system->lines, this_system->render_time);
	if (this_system->render_frame) freelinkN(&this_system->lines, this_system->render_frame);
	
	this_system->render_infofile = this_system->render_time = this_system->render_frame = NULL;
	this_system->need_update = TRUE;
	
	/* active_render = 0, update na crash */
	
	if (active_render == 0) return;
	
	if (active_render->busy == FALSE) {
		printf("Update_active_render(%s) Not Busy !!!\n", active_render->infofile);
		return;
	}
	
	fd = open_rw(active_render->infofile);
	if (fd != -1) {
		check = check_infofile(active_render);
		if (check == FILE_CHANGED) {
			if (reread_infofile(fd, active_render)) {
				printf("update_active_render: Reread failed, using old settings ???\n");
			}
		}
		
		line = active_render->lines.first;
		while (line) {
			if (strcmp(line->buf, active_line->buf) == 0) break;
			line = line->next;
		}
		
		if (line == 0) {
			printf("ERROR: couldn't find line: %s in %s\n",
					active_line->buf,
					active_render->infofile);
		} else {
			line->buf[0] = result;
			if (write_infofile(fd, active_render) == 0) {
				if (verbose > 2) printf("Update was OK\n");
			} else {
				printf("Update FAILED\n");
			}
		}
		close(fd);
	} else {
		if (check_infofile(active_render) != FILE_REMOVED) printf("update_active_render: couldn't open file %s???\n", active_render->infofile);
	}
	
	active_render->busy = FALSE;
}

void kill_renderer()
{
	if (renderproc) {
		active_kill = TRUE;
		
		/* zo snel mogelijk bestand updaten */
		if (active_render) {
			if (active_render->busy) update_active_render(' ');
		}
		
		kill(renderproc, SIGCONT);

		/* even ervoor zorgen dat signaal goed wordt afgehandeld
		 * Er kan anders alweer een nieuw proces opgestart worden
		 * voordat het signaal afgehandeld word. (?????)
		 */
		
		do {
			kill(renderproc, SIGINT);
			sigrelse(SIGCLD);
			sginap(10);
			sighold(SIGCLD);
		} while (renderproc);
	}
	
	status &= ~BUSY;
}



void sig_trap(signal)
{
	long error = FALSE;
	int stat;
	pid_t proc;
	
	/* vind lokale part */

	if (verbose > 2) printf("Recieved signal %d\n", signal);

	switch (signal) {
	case SIGCLD:
		/* in tegenstelling tot de handleiding hoeft de sig_trap niet opnieuw
		 * gezet to worden.
		 */
		proc = wait(&stat);
		if (proc == renderproc){
			if (active_render) {
				if (active_render->busy) {
					if (active_kill) {
						/* plaatje terug geven aan renderlijst */
						update_active_render(' ');
					} else {
						if (WIFEXITED(stat) == 0 || WEXITSTATUS(stat) != 0) {
							/* er is iets fout gegaan */
							if (WIFEXITED(stat) &&  WEXITSTATUS(stat) != 1) {
								printf("exit status != 1 ??? Is this an error or not ???\n");
							}
							update_active_render('E');
						} else{
							update_active_render('*');
						}
					}
				} else {
					if (verbose > 2) printf("OK: Renderproc for %s was killed\n", active_render->infofile);
				}
			} else {
				printf("sig_trap: active_render = 0\n");
			}
			
			active_render->renderproc = renderproc = 0;
			active_render = 0;
		} else error = FALSE;
		break;
	case SIGUSR1:
		if (active_render && renderproc) {
			if (verbose > 1) printf("received SIGUSR1\n");
			update_active_render('*');
		}
		break;
	case SIGINT:
	case SIGTERM:
		if (verbose > 2) printf("Stopping \n");
		error = go = FALSE;
		break;
	}

	if (error) if (verbose > 0) printf("Recieved signal while not rendering\n");
}


long saferead(Connect * connect, void * buf, long size)
{
	long count, connection;
	short len;
	
	if (connect == 0) return (0);
	
	count = net_read(connect->connection, buf, size);
	if (count == 0) {
		/* close on EOF */
		if (verbose > 2) printf("connection to %s closed\n", connect->name);
		close(connect->connection);
		remlink(connectlist, connect);
		freeN(connect);
	} else if (count != -1){
		((char *)buf)[count] = 0;
		if (verbose > 2) printf("recieved: [%s] from %s\n", buf, connect->name);
	}
	
	return (count);
}


void new_connection(struct in_addr addr, long connection)
{
	struct hostent * hostent;
	Connect * connect;
	
	if (connection == -1) return;

	connect = callocstructN(struct Connect, 1, "Connect");
	addtail(connectlist, connect);
	
	connect->connection = connection;
	strcpy(connect->name, "Unknown");

	hostent = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
	if (hostent) {
		strcpy(connect->name, hostent->h_name);
	} else perror("gethostbyaddr");
	
	if (verbose > 2) printf("new_connection to %s (%d)\n", connect->name, connection);
}


void close_in_pipes()
{
	if (Stdin[0] != -1) close(Stdin[0]);
	if (Stdin[1] != -1) close(Stdin[1]);
	
	Stdin[0] = Stdin[1] = -1;
}

void close_out_pipes()
{
	if (Stdout[0] != -1) close(Stdout[0]);
	if (Stdout[1] != -1) close(Stdout[1]);
	if (Stderr[0] != -1) close(Stderr[0]);
	if (Stderr[1] != -1) close(Stderr[1]);
	
	Stdout[0] = Stdout[1] = Stderr[0] = Stderr[1] = -1;
}

void close_all_pipes()
{
	close_in_pipes();
	close_out_pipes();
}


void open_in_pipes()
{
	if (pipe(Stdin)) {
		perror("pipe failed (stdin)");
		exit(1);
	}
}

void open_out_pipes()
{
	if (pipe(Stdout)) {
		perror("pipe failed (stdout)");
		exit(1);
	}
	if (pipe(Stderr)) {
		perror("pipe failed (stderr)");
		exit(1);
	}	
}


void open_pipes()
{
	open_in_pipes();
	open_out_pipes();
}


void execute(char * string)
{
	char * argv[20], cmd[1024];
	long i, index = 0, len, space = TRUE;

	/* even een locale kopie maken voor alle zekerheid */

	strcpy(cmd, string);
	len = strlen(cmd);

	if (verbose > 1) {
		for (i = 0; i < len ; i++){
			if (isspace(cmd[i])){
				cmd[i] = ' ';
			}
		}
		printf("      render: %s\n", cmd);
	}
	
	for (i = 0; i < len ; i++){
		if (isspace(cmd[i])){
			space = TRUE;
			cmd[i] = 0;
		} else{
			if (space){
				argv[index++] = cmd + i;
				space = FALSE;
				if (index >= 20){
					printf("To many vectors in %s\n", string);
					break;
				}
			}
		}
	}

	if (verbose > 2) printf("      render: %s %d\n", cmd, index);

	if (index == 0) exit(1);
	argv[index] = 0;

	if (verbose > 2) printf("      render: lets go\n");

	/* redirect in & output */
	dup2(Stdin[0], stdin->_file);
	dup2(Stdout[1], stdout->_file);
	dup2(Stderr[1], stderr->_file);
	
	close_all_pipes();
	
	if (execv(argv[0], argv) == -1) perror2("execv", argv[0]);
	
	sleep(1);
	
	exit(1);
}

void start_render(Render * render, char * sub)
{
	char    cmd[4096];

	active_kill = FALSE;
	
	if (render != active_render && active_render) {
		if (active_render->renderproc) {
			printf("start_render: renderproc still busy ???\n");
		}
	}
	
	active_render = render;
	renderproc = render->renderproc;
	
	/* do we have to start a new renderproc for each command or is this the first time ? */
	
	if (render->interface_int & ONE_SHOT || renderproc == 0) {
		/* open and close input pipes, to make sure that new renderproc
		 * doesn't read commands not processed by previous proces.....
		 */
		
		close_in_pipes();
		open_in_pipes();
		
		renderproc = fork();
		if (renderproc == 0) {
			/* this is the child */
			if (render->interface == NULL || render->interface_int & ONE_SHOT) {
				sprintf(cmd, "%s %s", render->command->buf + 2, sub);
			} else if (render->interface_int & READS_STDIN) {
				sprintf(cmd, "%s", render->command->buf + 2);
			}
	
			execute(cmd);
			exit(0);
		}
	}
	
	if (renderproc == -1) {
		perror("fork failed");
	} else {
		render->renderproc = renderproc;
		if (render->interface_int & READS_STDIN) {
			printf("Sending %s",  sub);
			write(Stdin[1], sub, strlen(sub));
		}
		render->busy = TRUE;
		status |= BUSY;
		set_line(this_system, "F", active_render->infofile);
		set_line(this_system, "T", active_render->time->buf + 2);
		set_line(this_system, ".", active_line->buf + 2);
		interpret_system(this_system);
		this_system->need_update		= TRUE;
	}
}

void process_input(Connect * connect, char * line)
{
	long code;
	
	code = (line[0] << 24) + (line[1] << 16) + (line[2] << 8) + line[3];
	switch(code){
	case 'WAIT':    /* ik moet stoppen */
		status ^= WAIT;
		if (status & WAIT) {
			if (renderproc) kill(renderproc, SIGSTOP);
		} else {
			status &= ~FINI;
			if (renderproc) kill(renderproc, SIGCONT);
		}
		break;
	case 'FINI':
		if (renderproc) {
			status |= FINI;
			kill(renderproc, SIGCONT); /* voor alle zekerheid */
		}
		status |= WAIT;
		break;
	case 'RETU':
		if (renderproc) kill_renderer();
		status |= WAIT;
		break;
	case 'ACPT':
		max_load = atoi(line + 4);
		break;
	default:
		printf("Unknown instruction: [%s] from %s\n", line, connect->name);
	}
}

void set_signals()
{
	sigset(SIGINT, sig_trap);
	sigset(SIGCLD, sig_trap);
	sigset(SIGTERM, sig_trap);
	sigset(SIGUSR1, sig_trap);
}

void release_signals()
{
	sigrelse(SIGINT);
	sigrelse(SIGCLD);
	sigrelse(SIGTERM);
	sigrelse(SIGUSR1);
}

void hold_signals()
{
	sighold(SIGCLD);
	sighold(SIGINT);
	sighold(SIGTERM);
	sighold(SIGUSR1);
}


void update_this_system()
{
	int fd;
	
	if (status != this_system->status_int || max_load != this_system->max_load_int || this_system->need_update) {
		/* open_new ! system_file MOET blijven bestaan */
		
		fd = open_new(this_system->infofile);

		if (fd != -1) {
			this_system->status_int = status;
			set_line_val(this_system, "S", status);
	
			this_system->max_load_int = max_load;
			set_line_val(this_system, "L", max_load);

			if (write_infofile(fd, this_system) == 0) {
				this_system->mtime = get_mtime(this_system->infofile);
			}
			close(fd);
			
			this_system->need_update = 0;
		}
	}
}

init_mcast()
{
	struct		in_addr	ifaddr;
	struct		in_addr	grpaddr;
	struct		ip_mreq	mreq;	
	char		*group = "239.0.0.239";
	u_char		ttl = 1;
	int			on = 1;
	
	grpaddr.s_addr = inet_addr(group);
	if (!IN_MULTICAST(grpaddr.s_addr)) {
		printf("Invalid multicast group address: %s\n",group);
		exit(1);
	}

	mcast_w = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_w < 0) {
		perror("socket");
		exit(1);
	}
	
	mcast_r = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_r < 0) {
		perror("socket");
		exit(1);
	}
	
	bzero(&addr_r, sizeof(addr_w));
	addr_r.sin_family = AF_INET;
	addr_r.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_r.sin_port = htons(portnumber);

	memcpy(&addr_w, &addr_r, sizeof(addr_w));
	
	ifaddr.s_addr = htonl(INADDR_ANY);

	/* mcast_r */
	
		/* 
		 * Allow multiple instances of this program to listen on the same
		 * port on the same host. By default, only 1 program can bind
		 * to the port on a host.
		 */
		
		on = 1;
		if (setsockopt(mcast_r, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0) {
			perror("setsockopt REUSEPORT");
			exit(1);
		}
	
		if (bind(mcast_r, &addr_r, sizeof(addr_r)) < 0) {
			perror("bind");
			exit(1);
		}
	
		mreq.imr_multiaddr = grpaddr;
		mreq.imr_interface = ifaddr;
		if (setsockopt(mcast_r, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt add membership");
			exit(1);
		}
	
	/* mcast_w */
	
		if (setsockopt(mcast_w, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))) {
			perror("setsockopt ttl");
			exit(1);
		}
		
		addr_w.sin_addr = grpaddr;

}


init_renderd()
{
	struct	servent *servent;
	char * dir;
	
	/* read portnumber */
		servent = getservbyname("renderd", "tcp"); /* that's me */
	
		if (servent == 0){
			printf("Assign a socket number in your /etc/services file for renderd\n");
			exit(-1);
		}
		
		portnumber = servent->s_port;
		
	/* check DAEMON_DIR */

		if (dir = getenv("DAEMONDIR")) {
			strcpy(daemon_dir, dir);
		} else if (dir = getenv("BLENDERDIR")) {
			strcpy(daemon_dir, dir);
			strcat(daemon_dir, "/daemon");
		} else strcpy(daemon_dir, "/usr/people/daemon");
				
		if (verbose > 2) printf("using %s\n", daemon_dir);
		
	/* read myname */
		if (gethostname(myname, sizeof(myname)) == -1){
			perror("Couldn't get hostname of local system");
			exit(-1);
		}
		
	/* fam */
		fc = 0;
		
		if (fc) {
			if (FAMOpen(fc)) {
				printf("FamError: %s\n", FamErrlist[FAMErrno]);
				exit(1);
			}
		
			famfd = FAMCONNECTION_GETFD(fc);
			FAMMonitorDirectory(fc, daemon_dir, fr, 0);
			FAMMonitorFile(fc, "/tmp/daemon/:mips.neogeo.nl", fr2, 0);
		}
	/* mcast */
		init_mcast();
}


int my_gets(char * pre, char * buf)
{
	int len, i;
	int saved = FALSE;
	
	len = strlen(buf);
	
	while (len) {
		for (i = 0; i < len ; i++) {
			if (buf[i] == 10) break;
		}
		buf[i] = 0;
		
		if (strncmp(buf, "Saved:", 6) == 0) saved = TRUE;
		
		printf("%s: %s\n", pre, buf);

		buf += i + 1;
		len = strlen(buf);
	}
	
	return (saved);
}

void handle_input()
{
	int		connection, num, count, addrlen;
	fd_set  readers;
	struct  timeval time_out;
	struct	in_addr address;
	Connect * connect;
	char    buf[1024];
	
	/* set a 10 second time out */
	time_out.tv_sec = 10;
	time_out.tv_usec = 0;

	release_signals();

	/* initialize the fd_set */
	FD_ZERO(&readers);
	FD_SET(sock, &readers);
	if (fc) FD_SET(famfd, &readers);
	FD_SET(Stdout[0], &readers);
	FD_SET(Stderr[0], &readers);
	FD_SET(mcast_r, &readers);
	
	connect = connectlist->first;
	while (connect) {
		connection = connect->connection;
		if (connection != -1) FD_SET(connection, &readers);
		connect = connect->next;
	}
	
	/* look for i/o events */
	
	num = select(FD_SETSIZE,&readers,0,0,&time_out);

	if (num == -1 && errno != EINTR) perror("select");

	hold_signals();
	
	/* process all events */
	if (num > 0) {
				
		/* is there something on the socket? */
		if (FD_ISSET(sock, &readers)){
			connection = accept_connection(sock, &address);
			if (connection != -1) new_connection(address, connection);
			num--;
		}
		
		if (FD_ISSET(Stdout[0], &readers)){
			count = read(Stdout[0], buf, sizeof(buf));
			if (count > 0) {
				buf[count] = 0;
				my_gets("stdout", buf);
			} else {
				if (count == 0) {
					printf("^D on stdout ???\n");
				}
			}
			num--;
		}
		
		if (FD_ISSET(Stderr[0], &readers)){
			count = read(Stderr[0], buf, sizeof(buf));
			if (count > 0) {
				buf[count] = 0;
				my_gets("stderr", buf);
			} else {
				if (count == 0) {
					printf("^D on stderr ???\n");
				}
			}
			num--;
		}

		if (FD_ISSET(mcast_r, &readers)){
			handle_mcast_r();
			num--;
		}
		
		if (fc) {
			/* fam? */
			if (FD_ISSET(famfd, &readers)){
				while (FAMPending(fc)) {
					FAMNextEvent(fc, fe);
					printf("Fam event: %d on (%d) %s %s\n", fe->code, fe->fr.reqnum, fe->hostname, fe->filename);
				}
				num--;
			}
		}
		
		connect = connectlist->first;
		while (connect) {
			connection = connect->connection;
			if (connection != -1){
				if (FD_ISSET(connection, &readers)){
					num --;
					do {
						count = saferead(connect, buf, sizeof(buf) - 1);
						if (count > 0) process_input(connect, buf);
					}while(count > 0);
				}
			}
			connect = connect->next;
		}

		if (num) printf("recieving messages on unknown file descriptor\n");
	}
}


renderd_main(int argc, char ** argv)
{
	long    i;
	char    buf[1024];
	long	check;
	int		fd;
	int		oldstatus;
	int		max_mem_in_use = 2000000;
	int		max_files_open = 15;
	Render	* new_render;
	
	/* process arg's */
		while (argc > 1) {
			if (argv[1][0] == '-') {
				i = 1;
				while (argv[1][i]) {
					switch(argv[1][i]) {
					case 'v':
						verbose++;
						break;
					default:
						printf("unknown option %c\n", argv[1][i]);
					}
					i++;
				}
				argc--;
				argv++;
			} else break;
		}

	/* common inits renderd / Render */
		init_renderd();
		
	/* create a socket */
		sock = init_recv_socket(portnumber);
		if (sock == -1) exit(1);
	
	/* create the pipes */
		open_pipes();
		
	/* read (or create) infofile */
		sprintf(buf, "%s/:%s", daemon_dir, myname);
		this_system = (System *) read_infofile(buf, SYSTEM);
		
		if (this_system == 0) this_system = make_systemfile(buf);
		oldstatus = status = this_system->status_int;
		max_load = this_system->max_load_int;
		
		status &= ~OFFLINE;
			
	/* rest of initialisation */
	
		set_signals();
		putenv("DISPLAY=:0.0");
		
		if (0) {
			Render * render;
			render = make_renderfile_old("/usr/people/frank/source/dummy", "/data/rt.blend", "S.first", 1, 100);
			if (render) free_header(render);
		}
		
	/* read all files */
	
		sync_dir();
		
	/* zijn we gecrashed ?? */
	
		if (status & BUSY) {
			printf("Recovering from crash\n");
			printf("FILE : %sFRAME: %s", this_system->render_infofile->buf + 2, this_system->render_frame->buf + 2);
			
			/* active_render opzoeken */
			active_render = renderlist->first;
			
			while (active_render) {
				if (strncmp(active_render->infofile, this_system->render_infofile->buf + 2, strlen(active_render->infofile)) == 0) break;
				active_render = active_render->next;
			}
			
			if (active_render) {
				if (strcmp(active_render->time->buf + 2,  this_system->render_time->buf + 2)) {
					printf("File exists, but has newer creation time\n");
					active_render = 0;
				} else {
					printf("File still active\n");
					sprintf(active_line->buf, ". %s", this_system->render_frame->buf + 2);
					active_render->busy = TRUE;
				}
			} else {
				printf("File not found\n");
			}
			
			update_active_render(' ');
		}

	/* main loop */
		do {
			fflush(stdout);

			while ((status & BUSY) == 0 && (status & WAIT) == 0) {
				new_render = find_best_render();

				if (new_render != active_render && active_render != NULL) {
					if (active_render->renderproc != NULL) kill_renderer();
				}
				
				if (new_render == 0) break;
				
				if (verbose > 2)  printf("found %s\n", new_render->infofile);
				
				/* open_rw locked de file meteen */
				
				fd = open_rw(new_render->infofile);
				
				if (fd != -1) {
					if (check_infofile(new_render)) {
						/* is tussentijds veranderd, try again */
						if (verbose > 2) printf("Oops %s file changed\n", active_render->infofile);
					} else {
						new_render->firstfree->buf[0] = '.';
						strcpy(active_line->buf, new_render->firstfree->buf);
						
						if (write_infofile(fd, new_render)) {
							new_render = 0;
						} else {
							new_render->mtime = get_mtime(new_render->infofile);
							start_render(new_render, active_line->buf + 2);
						}
					}
					close(fd);
				}
			}
			
			if ((status & BUSY) == 0) {
				if (active_render) {
					if (active_render->renderproc != NULL) kill_renderer();
				}
			}
			
			/* check if file still exists */
			
	
			if (active_render != 0) {
				check = check_infofile(active_render);
				
				switch (check) {
					case FILE_OK:
						break;
					case FILE_REMOVED:
						kill_renderer();
						printf("file was removed\n");
						break;
					case FILE_CHANGED:
						if (verbose > 2) printf("%s: file changed\n", active_render->infofile);
						update_infofile(active_render);

						/* checken if prioriteit niet op nul gezet is ? */
						
						if (active_render->pri_int == 0) kill_renderer();
						break;
				}
				
				/* controleren of new_render pri_max gezet heeft */
				
				/* werkt nog niet omdat hierboven FILE_CHANGED wordt afgevangen
					find_best_render() gooit dat in de war. 
				*/
				
				/*
				if (active_render && new_render) {
					if (active_render != new_render) {
						if (new_render->pri_int >= PRI_MAX && active_render->pri_int < PRI_MAX) {
							kill_renderer();
						}
					}
				}
				*/
			}
			
			if (status != this_system->status_int){
				if ((status & (BUSY | FINI)) == 0) printf("    IDLE\n");
			}
			
			update_this_system();
						
			/* sanity checks */
			
			if (getdtablehi() > max_files_open) {
				printf("WARNING %d files open\n", getdtablehi());
				max_files_open *= 1.5;
			}
			
			if (mem_in_use > max_mem_in_use) {
				printf("WARNING %d memory in use\n", mem_in_use);
				max_mem_in_use *= 1.5;
			}
			
			/* wait for anything to happen */
	
			handle_input();
			
		} while (go);
	
	/* clean_up */
		if (fc) FAMClose(fc);
		if (verbose > 2) printf("Cleanup\n");
		kill_renderer();
		release_signals();
		sginap(10);
		status |= OFFLINE;
		update_this_system();
		
	shutdown(sock, 2); /* werkt dit niet ?? */
	sginap(10);
	close(sock);
	sginap(10);

	if (verbose > 0) printf("Done\n");
}

