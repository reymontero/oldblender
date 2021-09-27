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
#include <sys/prctl.h>			/* sproc */
#include <ctype.h>				/* isspace */
#include <sys/wait.h>

#define malloc(x) mallocN(x,"malloc")
#define free(x) freeN(x)
#define calloc(x,y) callocN((x)*(y),"calloc")

#define R_WAIT 0
#define R_UNKNOWN -1
#define R_BUSY (1 << 0)
#define R_SUSP (1 << 1)
#define close(x) shutdown(x, 2);close(x)


/*
	- als entry 'entry' heet i.p.v. entry.neogeo.nl wordt
	  niet herkent dat het de zelfde computer is

	- automaties alle hosts aflopen om verbindingen te leggen
	
	- render intelligenter zodat actuele informatie getoond wordt.
	
	- Meer (minder) info in de info: filenamen / hostnames korter
	  maar wel actuele prints van traces onthouden en laten zien
	  totale rendertijd per blok onthouden, en natuurlijk de 
	  prints van errors onthouden
	
	- kan er niet een globale render lijst onthouden worden,  waarbij
	  onderhandelt wordt wie er *mag* renderen.
	  * Hiervoor moet onderzocht worden wat de reactie-snelheid is 
	  op een 'globaal' verzoek. Als er binnen deze tijd geen klacht
	  komt, begin dan.
	  * Zodra er alsnog een klacht komt, kun je nog altijd afbreken.

	  * Dit kan ook met file-locks opgelost worden. Locken van de
	  trace - file of van de output directory
	  	  
	- verbindingen lopen nog steeds niet lekker. Is het mogelijk
	  om telkens een verbinding te openen en te sluiten. Wat is
	  daarvan de overhead.
	
	- Broadcast moet ook echt broadcast worden. Onderzoek bz om te 
	  kijken hoe het daar gebeurt.
	  
	- Een host die uit de lucht is kan een tracesd voor vele secondes
	  platleggen. Kan bij het inloggen niet intelligenter gehandelt
	  worden.
		
	- Gegevens worden nu nog 2x onthouden: op de locale en op de 
	globale renderer. Wat is beter ??
	
	- Er moet ook een prioriteit aan animaties gegeven kunnen worden.
	Probleem daarbij is dat niet bekent is wat 'op het net' de
	hoogste prioriteit heeft. Elke renderer kan aangeven wat op dat
	moment de hoogste prioriteit bij hem heeft. Daarmee kan dan al 
	een vergelijk gemaakt worden tussen 'aanbiedingen' onderling. Mocht
	een aanbieding uitblijven (omdat je net te laat was), terwijl je
	wel een andere aanbieding hebt afgeslagen, dan kan er een
	timeout optreden (10 sec ?) waarna je weer een 'status 0' verstuurt.
	
	- Als er een leesfout optreed, dan wordt de verbinding wel gesloten, 
	maar dit wordt niet netjes afgehandelt. Er wordt geprobeert
	om toch nog op die lijn te schrijven. Kan de 'leesfout' niet 
	gecorrigeerd worden?

	er kan een core-dump onstaan in render(), deze fout onstaat consequent.

*/

/*
 * 
 * Nieuwe opzet tracesd
 * 
 * - Er komt een controle file voor elk te renderen animatie: zoiets als
 * foo.blend.render. In die file staat:
 * 
 * - prioriteit
 * - voor elk te renderen plaatje een nummer met een karakter ervoor.
 *  '.' = nog niet gerenderd
 *  '+' = wordt gerenederd
 *  '*' = is gerendered
 *  '-' = error
 * 
 * Er moet een interface komen die laat zien:
 * 
 * - Welke files er te renderen zijn.
 * - Welke prioriteit die files hebben
 * - Hoeveel ervan er al gerenderd is
 * 
 * 
 * - Er moet ook een gewicht aangegeven kunnen worden aan een file. (Aantallen MB
 * die nodig zijn om te kunnen renderen). Je kunt dan aangeven hoe zwaar een
 * background render mag zijn.
 * 
 * Voor elke nieuwe file wordt een nieuwe blender opgestart. Als er meerdere files
 * zijn met gelijke prioriteit wordt er telkens 5 (10) plaatjes van dezelfde file
 * gerenderd.
 * 
 */
 
/* ???????????????????? */
#define TRUE 1
#define FALSE 0

struct renderer{
    struct renderer * next, *prev;
    char name[MAXHOSTNAMELEN];
	char buffer[4 * 4096];
	struct	in_addr address;
	long bufindex;
    long status;
	long index;
    long connection;
	long dead;
};

/* 1 = oorsprong, 2 = plek waar gerenderd wordt */

#define LOLO 1
#define EXLO 2
#define LOEX 3

typedef struct part{
    struct part * next, *prev;
    char cmd[1024];
    struct renderer * renderer;
}Part;

struct ListBase _renderbase = {0, 0}, *renderbase = &_renderbase;

struct ListBase _donebase = {0, 0}, *donebase = &_donebase;
struct ListBase _todobase = {0, 0}, *todobase = &_todobase;
struct ListBase _busybase = {0, 0}, *busybase = &_busybase;
struct ListBase _errbase = {0, 0}, *errbase = &_errbase;

long status = R_WAIT, portnumber, go = TRUE;
pid_t renderproc = 0;
struct part * localpart;
FILE * infofile;
char * infoname = "/usr/tmp/render_info";
char * sortname = "/usr/tmp/render_sort";

char myname[MAXHOSTNAMELEN];
int verbose = 1;
int inforequest = 0;

/*

zipfork "cc -g tracesd.c -o tracesd util.o network.o -lmpc >/dev/console"                                          
zipfork "echo must be typed "                                                        

*/


/*
stap 1:
    Hou een lijst bij van alle beschikbare render-daemons met status.
    Moet het programma zelf ook een van de hosts zijn. Ik kan een verbinding
    met mezelf leggen, of moet dit telkens een uitzondering zijn ?
    
    voor alle beschikbare daemons:
	host (naam/adres)
	status	(niet aanspreekbaar / busy / waiting / suspended /...)
	connection
	
    houdt een lijst bij van files:
	namen.
	
    houdt een overzicht bij van huidige file
	Overzicht van frames (blokken)
	    klaar, (of gewist)
	    worden gerenderd
	    moeten nog gedaan worden.

    zolang er plaatjes te renderen zijn:
	is er een plekje vrij (hier of daar)
	    verstuur opdracht voor volgende pakket
	anders
	    wacht tot er een plek vrijkomt
	    
    is het niet gemakkelijker als de trace zelf bijhoudt voor 
    wie hij aan het renderen is i.v.m. foutmeldingen ?
*/


long safewrite(struct renderer * renderer, char *cmd)
{
	long len;

	if (renderer == 0) return(0);
	if (verbose > 1) printf("    sending: %s to: %s\n", cmd, renderer->name);

	len = strlen(cmd);
	if (len) {
		if (net_write(renderer->connection, cmd, len)){
			if (verbose > 0) printf("error writing: %s to :%s\n", cmd, renderer->name);
			perror("write");
			return(-1);
		}
	}
	return(0);
}


void sig_trap(signal)
{
	struct part * part;
	long error = FALSE;
	int stat;
	pid_t proc;

	/* vind lokale part */

	if (verbose > 1) printf("Recieved signal %d\n", signal);

	part = localpart;
	if (part == 0) error = TRUE;
	
	switch (signal) {
	case SIGCLD:
		/* in tegenstelling tot de handleiding hoeft de sig_trap niet opnieuw
		 * gezet to worden.
		 */
		proc = wait(&stat);
		if (proc == renderproc){
			if (part){
				remlink(busybase, part);
				/* zend bericht naar mogelijke externe daemon */
				if (WIFEXITED(stat) == 0 || WEXITSTATUS(stat) != 0) {
					/* er is iets fout gegaan */
					if (part->renderer) safewrite(part->renderer, "ERROR");
					else if (verbose > 0) printf("Recieved error while processing %s\n", part->cmd);
					addtail(errbase, part);
				} else{
					if (part->renderer) safewrite(part->renderer, "DONE");
					addtail(donebase, part);
				}
				renderproc = 0;
				localpart = 0;
				status &= ~R_BUSY;
			}
		} else error = FALSE;
		break;
	case SIGINT:
	case SIGTERM:
		error = go = FALSE;
		break;
	}

	if (error) if (verbose > 0) printf("Recieved signal while not rendering\n");
}


struct part * con_to_busy(long connection)
{
	struct part * part;

	part = busybase->first;
	while (part){
		if (part->renderer && part->renderer->connection == connection) break;
		part = part->next;
	}
	return (part);
}


long saferead_old(struct renderer * renderer, void * buf, long size)
{
	long count, connection;
	short len;
	
	if (renderer == 0) return (0);
	
	count = net_read(renderer->connection, buf, size);
	if (count == 0) {
		/* close on EOF */
		if (verbose > 1) printf("connection to %s closed\n", renderer->name);
		close(renderer->connection);
		remlink(renderbase, renderer);
		freeN(renderer);
	} else if (count != -1){
		((char *)buf)[count] = 0;
		if (verbose > 1) printf("recieved: [%s]\n", buf);
	}
	
	return (count);
}

long saferead(struct renderer * renderer, void * buf, long size)
{
	long count;
	short len;
	long index;
	char * buffer;
	
	if (renderer == 0) return (0);
	
	errno = 0;
	index = renderer->bufindex;
	buffer = renderer->buffer;
	
	size += sizeof(int);
	if (renderer->dead == FALSE) {
		if (index + size < sizeof(renderer->buffer)) {
			count = read(renderer->connection, buffer + index, size);
			if (count == 0) {
				/* close on EOF */
				renderer->dead = TRUE;
			} else if (count == -1){
				if (errno != EWOULDBLOCK) perror("saferead");
				/* return (-1); */
			} else {
				index += count;
			}
		} else if (index == 0) {
			printf("Buffer to small: %d requested\n", size);
			return(-1);
		}
	}
	
	if (renderer->dead && index == 0){
		if (verbose > 1) printf("connection to %s closed\n", renderer->name);
		close(renderer->connection);
		remlink(renderbase, renderer);
		freeN(renderer);
		return(0);
	}
	
	if (index > sizeof(int)) {
		count = * ((int *) buffer);
		if (index >= count + sizeof(int)) {
			if (count <= size) {
				memmove(buf, buffer + sizeof(int), count);
				memmove(buffer, buffer + sizeof(int) + count, sizeof(renderer->buffer) - count - sizeof(int));
				((char *)buf)[count] = 0;
				if (verbose > 1) printf("recieved: %6d [%s]\n", count, buf);
				index -= count + sizeof(int);
			} else {
				printf("line to long: %d > %d (%s) \n", count, size, buffer + sizeof(int));
				count = -1;
			}
		} else {
			buffer[index] = 0;
			/*printf("partial line; %s\n", buffer + sizeof(int));*/
			count = -1;
		}
	}
	
	renderer->bufindex = index;
	return (count);
}

void broadcast(char * data){
	long len;
	struct renderer * renderer;

	len = strlen(data);
	renderer = renderbase->first;
	while (renderer) {
		if (renderer->status != R_UNKNOWN) safewrite(renderer, data);
		renderer = renderer->next;
	}
}


struct renderer * new_connection(struct in_addr addr, long connection)
{
	struct renderer * renderer, *rend;
	struct hostent * hostent;
	
	if (connection == -1) return(0);

	renderer = callocstructN(struct renderer, 1, "renderer");
	renderer->connection = connection;
	strcpy(renderer->name, "Unknown");

	hostent = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
	if (hostent) {
		strcpy(renderer->name, hostent->h_name);
		renderer->address = *((struct in_addr *) hostent->h_addr_list[0]);

		if (verbose > 2) printf("  %s\n", hostent->h_name);
		if (hostent->h_addrtype && hostent->h_length == sizeof(struct in_addr)) {
			if (verbose > 2) printf("  %s\n", inet_ntoa(renderer->address));
		}
	} else perror("gethostbyaddr");
	
	addtail(renderbase, renderer);
	renderer->status = R_UNKNOWN;
	
	if (verbose > 1) printf("new connection to %s\n", renderer->name);
	return(renderer);
}


struct renderer * new_renderer(char * name, long connection)
{
	struct renderer * renderer;
	struct hostent * hostent;
	
	if (connection == -1) return(0);

	hostent = gethostbyname(name);
	if (hostent) {
		renderer = new_connection(*((struct in_addr *) hostent->h_addr_list[0]), connection);
		if (renderer) {
			if (strcmp(renderer->name, myname) == 0) {
				if (verbose > 1) printf("Ignoring %s: that's me\n", myname);
				remlink(renderbase, renderer);
				freeN(renderer);
				close(connection);
				return(0);
			}else if (verbose > 0) printf("added %s to renderlist\n", renderer->name);
			renderer->status = R_WAIT;
		}
	} else {
		perror("gethostbyname");
		close(connection);
	}
	
	return(renderer);
}


void render(char * string)
{
	char * argv[20], cmd[1024];
	long i, index = 0, len, space = TRUE;

	/* even een locale kopie maken voor alle zekerheid */

	strcpy(cmd, string);
	len = strlen(cmd);

	if (verbose > 1) printf("      render: %s\n", cmd);
	
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

	if (verbose > 1) printf("      render: %s %d\n", cmd, index);

	if (index == 0) exit(0);
	argv[index] = 0;

	if (verbose > 1) printf("      render: lets go\n");

	if (execv(argv[0], argv) == -1) perror(string);

	exit(0);
}


void start_part_sproc(struct part * part)
{
	if (part == 0) return;

	addtail(busybase, part);
	localpart = part;
	if (verbose > 0) printf("executing %s\n", part->cmd);
	renderproc = sproc(render, PR_SALL, part->cmd);
	if (renderproc == -1) {
		perror("sproc failed");
	}
	status |= R_BUSY;
}

void start_part(struct part * part)
{
	if (part == 0) return;

	addtail(busybase, part);
	localpart = part;
	if (verbose > 0) printf("executing %s\n", part->cmd);
	renderproc = fork();
	if (renderproc == 0) {
		render(part->cmd);
		exit(0);
	}
	if (renderproc == -1) {
		perror("fork failed");
	}
	status |= R_BUSY;
}

void remove_part(char * file, ListBase * list)
{
	Part * part, * next;
	
	part = list->first;
	while (part) {
		next = part->next;
		if (strstr(part->cmd, file)) {
			remlink(list, part);
			freeN(part);
		}
		part = next;
	}
}


struct renderer * find_renderer(struct renderer *renderer, char * name)
{
	if (renderer == 0) renderer = renderbase->first;
	else renderer = renderer->next;
	
	while (renderer) {
		if (strcmp(renderer->name, name) == 0) break;
		renderer = renderer->next;
	}
	
	return (renderer);
}

void list_other(struct renderer *renderer, ListBase * base, char * string)
{
	struct part *part;
	char cmd[1024];
	
/*	sprintf(cmd, string, myname, "");
	safewrite(renderer, cmd);
*/	
	part = base->first;
	while (part){
		sprintf(cmd, string, myname, part->cmd);
		safewrite(renderer, cmd);
		sginap(0);
		part = part->next;
	}
}

void list_local(ListBase * base, char * string)
{
	struct part *part;

/*	fprintf(infofile, string, myname, "");
*/	
	part = base->first;
	while (part){
		fprintf(infofile, string, myname, part->cmd);
		part = part->next;
	}
}


void kill_renderer()
{
	if (renderproc) {
		kill(renderproc, SIGCONT);
		kill(renderproc, SIGUSR1);
		wait(0);
		renderproc = 0;
		
		/* even ervoor zorgen dat signaal goed wordt afgehandeld
		 * Er kan anders alweer een niew proces opgestart worden
		 * voordat het signaal afgehandeld word.
		 */
		
		sigpause(SIGCLD);
		sighold(SIGCLD);		
	}
	
	status &= ~R_BUSY;
}


void process_input(struct renderer *renderer, char * line)
{
	struct renderer *rend;
	struct part *part, *next;
	char cmd[1024];
	long code, i;

	code = (line[0] << 24) + (line[1] << 16) + (line[2] << 8) + line[3];
	switch(code){
	case 'STAR':
		/* hier oude exemplaren wissen */
	case 'STAT':
		renderer->status = atoi(line + 7);
		break;
	case 'DONE':
		part = con_to_busy(renderer->connection);
		if (part == 0){
			if (verbose > 0) printf("Recieved done from unknown renderer\n");
		} else{
			remlink(busybase, part);
			addtail(donebase, part);
		}
		break;
	case 'ERRO':
		part = con_to_busy(renderer->connection);
		if (part == 0){
			if (verbose > 0) printf("Recieved error from unknown renderer\n");
		} else {
			if (verbose > 0) printf("Recieved error while processing %s from %s\n", part->cmd, part->renderer->name);
			remlink(busybase, part);
			addtail(errbase, part);
		}
		break;
	case 'RET2':
		part = con_to_busy(renderer->connection);
		if (part == 0){
			if (verbose > 0) printf("Recieved return from unknown renderer\n");
		} else {
			if (verbose > 0) printf("Recieved return while processing %s from %s\n", part->cmd, part->renderer->name);
			remlink(busybase, part);
			addhead(todobase, part);
		}
		break;
	case 'QUEU':
		part = callocstructN(struct part, 1, "part todo");
		strcpy(part->cmd, line + sizeof("QUEUE"));
		addtail(todobase, part);
		break;
	case 'REND':

		/* RENDER commando 
		 * Dit gebeurt op een nieuwe poort, zodat er geen spraakverwarring
		 * kan onstaan. Zoek eerst naar orginele (de andere) renderer.
		 */
		
		rend = find_renderer(0, renderer->name);
		while (rend) {
			if (rend != renderer && rend->status != R_UNKNOWN) break;
			rend = find_renderer(rend, renderer->name);
		}
		if (rend != 0 && status == R_WAIT && todobase->first == 0){
			if (safewrite(renderer, "OK")) break;
			part = callocstructN(struct part, 1, "part ext");
			strcpy(part->cmd, line + sizeof("RENDER"));
			part->renderer = rend;
			start_part(part);
		} else
			safewrite(renderer, "NO");
		break;
	case 'SUSP':    /* ik moet stoppen */
		if (renderproc) kill(renderproc, SIGSTOP);
		status |= R_SUSP;
		break;
	case 'FINI':
		if (renderproc) kill(renderproc, SIGCONT); /* voor alle zekerheid */
		status |= R_SUSP;
		break;
	case 'CONT':
		if (renderproc) kill(renderproc, SIGCONT);
		status &= ~R_SUSP;
		break;
	case 'RET1':
		part = localpart;
		if (part) {
			remlink(busybase, part);
			if (part->renderer) {
				safewrite(part->renderer, "RET2");
				freeN(part);
			} else {
				addhead(todobase, part);
			}
		}
		localpart = 0;
		status |= R_SUSP;
		kill_renderer();
		break;
	case 'INFO':
		if (infofile) fclose(infofile);
		infofile = fopen(infoname, "w");
		if (infofile) {
			if (localpart) fprintf(infofile,	"RENDER %s %s\n", myname, localpart->cmd);
			fprintf(infofile, "STATUS: %s = %d\n", myname, status);
			list_local(donebase,				"DONE   %s %s\n");
			list_local(todobase,				"QUEUED %s %s\n");
			list_local(errbase,					"ERROR  %s %s\n");
			broadcast("INF2");
			inforequest = 0;
			
			renderer = renderbase->first;
			while (renderer) {
				if (renderer->status != R_UNKNOWN) inforequest++;
				renderer = renderer->next;
			}
		}
		if (inforequest) break;
	case 'INFD':
		/* INFO-DONE */
		inforequest--;
		if (inforequest <= 0 && infofile != 0) {
			fclose(infofile);
			infofile = 0;
			sprintf(cmd, "sort < %s > %s", infoname, sortname);
			system(cmd);
			chmod(sortname, 0666);
			if (fork() == 0){
				stringselect("RenderInfo", sortname);
				exit(0);
			}
			remove(infoname);
		}
		break;
	case 'INF2':
		if (localpart) {
			sprintf(cmd, "INFR RENDER %s %s", myname, localpart->cmd);
			safewrite(renderer, cmd);
		}
		
		sprintf(cmd, "INFR STATUS: %s = %d", myname, status);
		safewrite(renderer, cmd);
		
		list_other(renderer, donebase,				"INFR DONE   %s %s");
		list_other(renderer, todobase,				"INFR QUEUED %s %s");
		list_other(renderer, errbase,				"INFR ERROR  %s %s");
		safewrite(renderer, "INFDONE");
		break;
	case 'INFR':
		/* INFO-REPLY */
		if (infofile) {
			fprintf(infofile,	"%s\n", line + 5);
		} else printf("Stray info: %s\n", line);
		
		break;
	case 'CLEA':
		broadcast("CLE2");
	case 'CLE2':
		freelistN(donebase);
		freelistN(errbase);		
		break;
	case 'REMO':
		/* 1 - staat nog in queue
		 * 2 - wordt lokaal gerenderd (= localpart)
		 * 3 - wordt extern gerenderd (!= localpart && part->renderer != 0)
		*/

		remove_part(line + 6, errbase);
		remove_part(line + 6, todobase);
		remove_part(line + 6, donebase);

		part = busybase->first;
		while (part) {
			next = part->next;
			if (strstr(part->cmd, line + 6)) {
				remlink(busybase, part);
				if (part == localpart) {
					/* dit ben ik zelf */
					kill_renderer();
					localpart = 0;
				} else safewrite(part->renderer, line);
				freeN(part);
			}
			part = next;
		}
		break;
	}
}


void try_to_execute()
{
	struct renderer * renderer, * newrenderer;
	struct part * part;
	char cmd[1024];
	long connection;
	
	part = todobase->first;
	if (part == 0) return;

	if (status == R_WAIT && renderproc == 0){
		/* doen we het zelf maar */
		remlink(todobase, part);
		part->renderer = 0;
		start_part(part);
	}

	if (todobase->first == 0) return;

	renderer = renderbase->first;
	while (renderer) {
		if (todobase->first == 0) break;

		if (renderer->status == R_WAIT){
			connection = connect_to_host(renderer->name, portnumber);
			if (connection != -1) {
				newrenderer = new_renderer(renderer->name, connection);
				if (newrenderer) {
					part = todobase->first;
					sprintf(cmd, "RENDER %s", part->cmd);
					if (safewrite(newrenderer, cmd) == 0){
						if (saferead(newrenderer, cmd, 2) == 2){
							if (strcmp(cmd, "OK") == 0){
								remlink(todobase, part);
								part->renderer = renderer;
								addtail(busybase, part);
							}
							renderer->status = R_BUSY;
						}
					}
					remlink(renderbase, newrenderer);
					freeN(newrenderer);
				}
				close(connection);
			}
		}

		renderer = renderer->next;
	}

}


main(argc,argv)
int argc;
char **argv;
{
	long    sock, connection, i, count, flags, num;
	char    line[1024];
	struct  timeval time_out;
	fd_set  readers;
	struct servent *servent;
	struct renderer *renderer;
	long oldstatus = status - 1;
	struct in_addr address;

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
	sock = init_recv_socket(portnumber);
	if (sock == -1) exit(1);

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

	for (i = 1; i < argc; i++){
		/* scan argc[] voor alle hosts waarmee een verbinding gelegd moet worden
		 * Alle connections zijn nog leeg, dus we doen het gewoon op volgorde
		 */

		connection = connect_to_host(argv[i], portnumber);
		if (connection == -1) {
			if (verbose > 0) printf("Connection to %s refused\n", argv[i]);
		} else {
			if (verbose > 0) printf("Connected to %s\n", argv[i]);
			renderer = new_renderer(argv[i], connection);
			if (renderer) {
				/*address[i] = renderer->address;*/
				safewrite(renderer, "STARTUP");
			}
		}
	}

	sigset(SIGINT, sig_trap);
	sigset(SIGCLD, sig_trap);
	sigset(SIGTERM, sig_trap);
	putenv("DISPLAY=:0.0");
	
	do {
		/* initialize the fd_set */
		FD_ZERO(&readers);
		FD_SET(sock, &readers);
		
		renderer = renderbase->first;
		while (renderer) {
			connection = renderer->connection;
			if (connection != -1) FD_SET(connection, &readers);
			renderer = renderer->next;
		}
		
		/* set a 10 second time out */
		time_out.tv_sec = 10;
		time_out.tv_usec = 0;

		/* look for i/o events */

		/* sginap(2); /* PATCH VOOR GROTE BUFFER PROBLEMEN ? */
		
		i = select(FD_SETSIZE,&readers,0,0,&time_out);

		/* hold the signals */
		sighold(SIGCLD);
		sighold(SIGINT);
		sighold(SIGTERM);

		/* process all events */
		if (i > 0) {
			num = i;
			
			/* is there something on the socket? */
			if (FD_ISSET(sock, &readers)){
				connection = accept_connection(sock, &address);
				if (connection != -1) new_connection(address, connection);
				num --;
			}

			/* is there something on one of the connections? */
			renderer = renderbase->first;
			while (renderer) {
				connection = renderer->connection;
				if (connection != -1){
					if (FD_ISSET(connection, &readers)){
						num --;
						/* set non_block */
						flags = fcntl(connection,F_GETFL);
						fcntl(connection,F_SETFL,flags | FNDELAY);

						do {
							count = saferead(renderer, line, sizeof(line) - 1);
							if (count > 0) process_input(renderer, line);
						}while(count > 0);

						/* restore flags */
						fcntl(connection,F_SETFL,flags);
					}
				}
				renderer = renderer->next;
			}
			if (num) fprintf(stderr, "recieving messages on unknown file descriptor\n");
/*		} else if (i == -1) {
			perror("select");
*/		}

		if (todobase->first) try_to_execute();

		if (oldstatus != status){
			sprintf(line, "STATUS %d", status);
			broadcast(line);
			oldstatus = status;
			if ((status & R_BUSY) == 0) printf("    IDLE\n");
		}

		sigrelse(SIGINT);
		sigrelse(SIGCLD);
		sigrelse(SIGTERM);

	} while (go);

	renderer = renderbase->first;
	while (renderer) {
		close(renderer->connection);
		renderer = renderer->next;
	}

	kill_renderer();
	sginap(10);
	
	shutdown(sock, 2); /* werkt dit niet ?? */
	sginap(10);
	close(sock);
	sginap(10);
	
	remove(infoname);
	remove(sortname);
	
	if (verbose > 0) printf("Done\n");
}

