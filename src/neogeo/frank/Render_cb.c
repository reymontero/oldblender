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

#include "forms.h"

#include "Render.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>			/* mmap */
#include <stdlib.h>				/* getenv */

#include <local/util.h>
#include "renderd.h"

#include <local/network.h>
#include <X11/Xlib.h>

extern char lastfile[];
extern Render * select_render;
extern System * select_system;
extern long portnumber;

#define ERROR -1
#define UNKNOWN 0
#define TRACES 1
#define BLENDER 2

#define CHUNK 10

typedef struct BHead {
	int code, len;
	void *old;
	int SDNAnr, nr;
} BHead;

extern void update_all();
extern FD_Render *fd_Render;
extern int autoselect, ignore_next_select;

void add_traces(char * mem, long size, char *name)
{
	long len, chunk = 0;
	long sfra, efra, tfra;
	char cmd[1024], *traces;

	mem += 12;
	size -= 12;

	while (size > 0){
		chunk = GL(mem);
		len = GL(mem+4);
		if (chunk == 'REND' || chunk == 'GLOB') break;
		mem += 8 + len;
		size -= 8 + len;
	}

	if (size < 0){
		printf("coudn't find REND or GLOB chunk\n");
		return;
	}

	if (chunk == 'REND') {
		sfra = GL(mem + 8 + 16);
		efra = GL(mem + 8 + 24);
	} else {
		sfra = *(short *)(mem + 8 + 18);
		efra = *(short *)(mem + 8 + 20);
	}
	
	traces = getenv("TRACES_EX");
	if (traces == 0) {
		traces = "/usr/people/bin/traces";
	}
	
/*	while (sfra <= efra) {
		tfra = sfra + CHUNK - 1;
		if (tfra > efra) tfra = efra;

		sprintf(cmd, "QUEUE %s -b %s -s %d -e %d -a", 
		    traces, name, sfra, tfra);
		safewrite(cmd);
		sfra = tfra + 1;
	}
*/
}


void add_blender(char *fd, int filelen, char *name)
{
	BHead *bhead;
	int sfra=0, efra=0, skipdata, *ip, found= 0;
	
	/* in een "REND" blokje zit :
		de sfra en efra (len==8) 
		daarachter een scenenaam (len==32) 
	*/

	fd+= 12;
	filelen-= 12;

	while(filelen>0) {
		
		bhead= (BHead *)fd;
		
		switch(bhead->code) {
		case 'REND':
			skipdata= bhead->len+sizeof(BHead);
			ip= (int *)(fd+sizeof(BHead));
			
			sfra= ip[0];
			efra= ip[1];
			
			if(bhead->len==8) {		/* current scene */
				/*sprintf(cmd, "QUEUE /usr/people/bin/blender -b %s -s %d -e %d -a", name, sfra, tfra);*/
				printf("Can't execute current scene! %s\n", name);
			}else if(bhead->len==32) {	/* andere scene */
				/*sprintf(cmd, "QUEUE /usr/people/bin/blender -b %s -S %s -s %d -e %d -a", name, ip+2, sfra, tfra);*/
				make_renderfile("/usr/people/bin/blender", name, ip+2, sfra, efra);
			}

			found= 1;
			
			break;
		default:
			skipdata= bhead->len+sizeof(BHead);	
			if(found) return;
		}
		
		fd+= skipdata;
		filelen-= skipdata;
	}

}


long typeoffile(char * name, long * psize, char ** pmem)
{
	long file, size;
	char *mem;
	long type = UNKNOWN;
	
	/* bepaal fileformaat */

	file = open(name,O_RDONLY);
	if (file < 0) return(ERROR);
	size = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	mem=(char *)mmap(0,size,PROT_READ,MAP_SHARED,file,0);
	close(file);
	if (mem == (char *)-1){
		printf("Couldn't get mapping\n");
		return (ERROR);
	}

	switch(GL(mem)){
	case 'FORM':
		switch (GL(mem+8)){
		case 'TRI0':
			type = TRACES;
		}
		break;
	case 'BLEN':
		type = BLENDER;
		break;
	}
	
	*psize = size;
	*pmem = mem;
	
	return(type);
}


void add_to_que(char * name)
{
	long size;
	char * mem = 0;
	
	switch(typeoffile(name, &size, &mem)){
		case ERROR:
			perror(name);
			break;
		case UNKNOWN:
			printf("unrecognized filetype %s\n", name);
			break;
		case TRACES:
			add_traces(mem, size, name);
			break;
		case BLENDER:
			add_blender(mem, size, name);
			break;
		
	}
	if (mem) if (munmap(mem,size)) printf("Couldn't unmap file.\n");
}


/* callbacks for form Render */
void File_browse(FL_OBJECT *ob, long data)
{
	FL_Coord x,y;
	unsigned int keymask;
	int line;
	
	fl_get_mouse(&x,&y,&keymask);
	if ((keymask & ShiftMask) == 0) {
		line = fl_get_browser(ob);
		if (line) {
			fl_deselect_browser(ob);
			fl_select_browser_line(ob, abs(line));
		}
	}
	
	if (ignore_next_select == FALSE && line > 0) autoselect = FALSE;
	else ignore_next_select = FALSE;
	
	update_all();
}


void Sys_browse(FL_OBJECT *ob, long data)
{
	FL_Coord x,y;
	unsigned int keymask;
	int line;
	System * system;
	
	fl_get_mouse(&x,&y,&keymask);
	if ((keymask & 1) == 0) {
		line = fl_get_browser(ob);
		if (line) {
			fl_deselect_browser(ob);
			fl_select_browser_line(ob, abs(line));
		}
	}
	/*printf(". %d %x %d\n", data, keymask, line);*/
	update_all();
}

void Priority(FL_OBJECT *ob, long data)
{
	int fd;
	Render * render;
	
	if (select_render) {
	
		render = select_render;
		
		fd = open_rw(render->infofile);
		if (fd != -1) {

			if (check_infofile(render) == FILE_CHANGED) {
				if (reread_infofile(fd, render)) {
					printf("Reread failed\n");
				}
			}
			
			if (render->pri_int != data) {
				render->pri_int = data;
				render->pri = set_line_val(render, "P", render->pri_int);
				if (write_infofile(fd, render)) {
					printf("Render SetPriority: Update FAILED\n");
					select_render = 0;
				}
			}
			close (fd);
		}
	} else fl_set_button(ob, 0);
}


long open_connection(System * system)
{
	char * hostname;
	long connection;
	
	/* fill-in code for callback */
	if (system) {
		if ((system->status_int & OFFLINE) == 0){
			hostname = strrchr(system->infofile, ':');
			if (hostname != NULL) {
				connection = connect_to_host(hostname + 1, portnumber);
				return(connection);
			}
		}
	}
	return(-1);	
}

void Status_cb(FL_OBJECT *ob, long data)
{
	long connection;
	System * system;
	
	
	system = systemlist->first;
	
	while (system) {
		if (system->selected) {
			if ((connection = open_connection(system)) != -1) {
				switch (data) {
					case 0:
						/* wait */
						net_write(connection, "WAIT", 4);
						break;
					case 1:
						/* finish */
						net_write(connection, "FINI", 4);
						break;
					case 2:
						/* return */
						net_write(connection, "RETU", 4);
						break;
					default:
						printf("Status_cb: function not implemented\n");
				}
				close(connection);
			}
		}
		
		system = system->next;
	}
	/* zorg ervoor dat alles zover mogelijk
	 * geupdate is voordat we weer files gaan lezen
	 */
	sginap(10);
}

void Add_file(FL_OBJECT *ob, long data)
{
	if (fileselect("Add File", lastfile)) {
		add_to_que(lastfile);
	}
}


void Rem_file(FL_OBJECT *ob, long data)
{
	extern void set_select();
	Header * header;
	int fd;
	
	set_select();
	header = renderlist->first;
	while (header) {
		if (header->selected) break;
		header = header->next;
	}
	
	if (header) {
		if (fl_show_question("Do you really want to remove\nall selected files\n?", 1)) {
			while (header) {
				if (header->selected) {
					fd = open_rw(header->infofile);
					remove(header->infofile);
					close(fd);
				}
				header = header->next;
			}
			multicast('RDir', 0, "");
		}
	}
}

void Retry_file(FL_OBJECT *ob, long data)
{
	int fd;
	char buf[32];
	Render * render;
	Line * line;
	
	if (select_render) {
	
		render = select_render;
		
		if (render->error_int) {
			fd = open_rw(render->infofile);
			if (fd != -1) {
	
				if (check_infofile(render) == FILE_CHANGED) {
					if (reread_infofile(fd, render)) {
						printf("Reread failed\n");
					}
				}
				
				line = render->lines.first;
				
				while (line) {
					if (line->buf[0] == 'E') line->buf[0] = ' ';
					line = line->next;
				}
				
				if (write_infofile(fd, render)) {
					printf("Render Retry_file: Update FAILED\n");
					select_render = 0;
				}
				close (fd);
			}
		}
	}
}


void Load(FL_OBJECT *ob, long data)
{
	int fd;
	Render * render;
	
	if (select_render) {
	
		render = select_render;
		
		fd = open_rw(render->infofile);
		if (fd != -1) {

			if (check_infofile(render) == FILE_CHANGED) {
				if (reread_infofile(fd, render)) {
					printf("Reread failed\n");
				}
			}
			
			if (render->load_int != data) {
				render->load_int = data;
				render->pri = set_line_val(render, "L", render->load_int);
				if (write_infofile(fd, render)) {
					printf("Render SetLoad: Update FAILED\n");
					select_render = 0;
				}
			}
			close (fd);
		}
	} else fl_set_button(ob, 0);
}

void Beep_file(FL_OBJECT *ob, long data)
{
	int fd;
	Render * render;
	XKeyboardState xkbstate;
	XKeyboardControl xkbcontr;
	
	if (select_render) {
		select_render->beep = fl_get_button(ob);
		
	} else fl_set_button(ob, 0);
	
/*
	XGetKeyboardControl(fl_get_display(), &xkbstate);
	
	printf("%08x\n", xkbstate.led_mask);
	xkbcontr.led_mode = LedModeOn;
	
	XChangeKeyboardControl(fl_get_display(), KBLedMode, &xkbcontr);

	XGetKeyboardControl(fl_get_display(), &xkbstate);
	
	printf("%08x\n", xkbstate.led_mask);
	xkbcontr.led_mode = LedModeOff;
	
	XChangeKeyboardControl(fl_get_display(), KBLedMode, &xkbcontr);
*/	
}


void Accept(FL_OBJECT *ob, long data)
{
	long connection;
	char buf[256];
	System * system;
	
	system = systemlist->first;
	
	while (system) {
		if (system->selected) {
			if ((connection = open_connection(system)) != -1) {
					sprintf(buf, "ACPT %d", data);
					net_write(connection, buf, strlen(buf));
					close(connection);
			}
	
		}
		system = system->next;
	}
	
	/* zorg ervoor dat alles zover mogelijk
	 * geupdate is voordat we weer files gaan lezen
	 */
	sginap(10);
}


void Do_nothing(FL_OBJECT *ob, long data)
{
  extern long mem_in_use;
  extern char *check_memlist(void *);
  
  /*printf("%d %d %d %d\n", ob->x, ob->y, ob->w, ob->h);*/

  printf("mem_in_use:%d %s\n", mem_in_use, check_memlist(0));
  if (0) printmemlist();
}

