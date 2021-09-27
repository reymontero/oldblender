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

#include <string.h>			/* strcpy etc.. */
#include <stdio.h> 
#include <errno.h>
#include <local/util.h>
#include <local/network.h>
#include <local/Button.h>
#include <stdlib.h>			/* getenv */
#include <signal.h>			/* kill */
#include <sys/param.h>			/* MAXHOSTNAMELEN */
#include <sys/prctl.h>			/* sproc */
#include <ctype.h>			/* isspace */
#include <fmclient.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <sys/mman.h>			/* mmap */


#define malloc(x) mallocN(x,"malloc")
#define free(x) freeN(x)
#define calloc(x,y) callocN((x)*(y),"calloc")

/*

zipfork "make render >/dev/console"                                          
zipfork "render >/dev/console"                                                                
zipfork "cc -g render.c -o render util.o network.o Button.o -lm -lfm_s -lgl_s >/dev/console"                                          

*/

/* struct voor blenderfile */

typedef struct BHead {
	int code, len;
	void *old;
	int SDNAnr, nr;
} BHead;


fmfonthandle helvfont,helv12=0;
long win, connection = -1;
short susp = 0;

#define BUTWHITE 7
#define BUTDARK 113
#define BUTLIGHT 114
#define BUTBLACK 0

#define BUTB1 115
#define BUTB2 116
#define BUTB3 117
#define BUTB4 118

#define ERROR -1
#define UNKNOWN 0
#define TRACES 1
#define BLENDER 2

#define CHUNK 10

void drawbuttons()
{
	if(helv12==0) {
		fminit();
		if( (helvfont=fmfindfont("Helvetica-Bold")) == 0) exit(1);
		helv12=fmscalefont(helvfont,11.0);
	}

	color(BUTBLACK);
	clear();

	mapcolor(BUTLIGHT,0,122,122);
	mapcolor(BUTDARK,0,81,81);
	mapcolor(BUTWHITE,255,255,255);
	mapcolor(BUTBLACK,0,0,0);

	mapcolor(BUTB1,0,30,30);
	mapcolor(BUTB2,120,190,190);
	mapcolor(BUTB3,0,60,60);
	mapcolor(BUTB4,160,240,240);

	DefButBlock("DisTape",win,helv12,50,2,0);
	DefButCol(2,NORMALDRAW,0,BUTDARK,BUTLIGHT,BUTWHITE,BUTWHITE,BUTB1,BUTB2,BUTB3,BUTB4);
	SetButCol(2);
	BGflush();
	BGadd(BUT,100,		"Add",    0,0,10,10);
	BGadd(BUT,101,		"Rem",    0,0,10,10);
	BGnewline();
	BGnewline();
	BGadd(TOG|SHO,102,	"Wait",	    0,0,10,10, &susp);
	BGnewline();
	BGadd(BUT,103,		"Finish",	    0,0,10,10);
	BGnewline();
	BGadd(BUT,104,		"Return",	    0,0,10,10);
	BGnewline();
	BGnewline();
	BGadd(BUT,105,		"Info",	    0,0,10,10);
	BGadd(BUT,106,		"Clear",	    0,0,10,10);

	BGposition(10,10,100,140);
	/*BGposition(30,20,220,120);*/

	BGspacing(4,4);
	BGdirection('d');
	BGdraw();
	gflush();
}


long safewrite(char *cmd)
{
	long len;
	
	len = strlen(cmd);
	if (len) {
		if (net_write(connection, cmd, len)){
			printf("error writing: %s \n", cmd);
			perror("write");
			exit(1);
		}
	}
	return(0);
}


long saferead(void * buf, long size)
{
	long count;

	count = net_read(connection, buf, size);
	if (count == 0) {
		/* close on EOF */
		exit(1);
	}
	return (count);
}


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
	
	while (sfra <= efra) {
		tfra = sfra + CHUNK - 1;
		if (tfra > efra) tfra = efra;

		sprintf(cmd, "QUEUE %s -b %s -s %d -e %d -a", 
		    traces, name, sfra, tfra);
		safewrite(cmd);
		sfra = tfra + 1;
	}
}


void add_blender(char *fd, int filelen, char *name)
{
	BHead *bhead;
	int tfra, sfra=0, efra=0, skipdata, *ip, found= 0;
	char cmd[1024];
	
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
			
			while (sfra <= efra) {
				tfra = sfra + CHUNK - 1;
				if (tfra > efra) tfra = efra;
				
				if(bhead->len==8) {		/* current scene */
					sprintf(cmd, "QUEUE /usr/people/bin/blender -b %s -s %d -e %d -a", name, sfra, tfra);
					safewrite(cmd);
				}
				else if(bhead->len==32) {	/* andere scene */
					sprintf(cmd, "QUEUE /usr/people/bin/blender -b %s -S %s -s %d -e %d -a", name, ip+2, sfra, tfra);
					safewrite(cmd);
				}
				
				sfra = tfra + 1;
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


void rem_from_que(char * name)
{
	char cmd[256];

	sprintf(cmd, "REMOVE %s ", name);
	safewrite(cmd);
}


main(argc,argv)
int argc;
char **argv;
{
	long go = 1, count, portnumber, event;
	short val;
	struct servent *servent;
	char myname[128];
	char file[128];

	/* create a socket */
	servent = getservbyname("traces", "tcp"); /* that's me */
	if (servent == 0){
		printf("Assign a socket number in your /etc/services file for traces\n");
		exit(-1);
	}
	if (gethostname(myname, sizeof(myname)) == -1){
		perror("Couldn't get hostname of local system");
		exit(-1);
	}
	portnumber = servent->s_port;

	prefsize(120, 160);
	
	win = winopen(myname);
	color(0);
	clear();
	drawbuttons();

	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(LEFTMOUSE);
	strcpy(file, "/data/");

	while(go){
		switch(qread(&val)){
		case LEFTMOUSE:
			if (val) {
				val = DoButtons();
				if (val) {
					if ((connection = connect_to_host(myname, portnumber)) != -1) {
						switch (val) {
						case 100:
							if (fileselect("Add File", file)){
								add_to_que(file);
							}
							break;
						case 101:
							if (fileselect("Stop File", file)){
								rem_from_que(file);
							}
							break;
						case 102:
							if (susp) safewrite("SUSPEND");
							else safewrite("CONTINUE");
							break;
						case 103:
							safewrite("FINISH");
							susp = TRUE;
							SetButs(102,102);
							break;
						case 104:
							safewrite("RET1");
							susp = TRUE;
							SetButs(102,102);
							break;
						case 105:
							safewrite("INFO");
							break;
						case 106:
							safewrite("CLEAR");
							break;
						}
						close(connection);
					}
				}
			}
			break;
		case REDRAW:
			color(0);
			clear();
			drawbuttons();
			break;
		}
	}
}

