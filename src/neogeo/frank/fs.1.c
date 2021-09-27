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

/*


zipfork "make fs >/dev/console"                                                     
zipfork "fs >/dev/console"                                       
	DefBut(MENU|CHA, REDRAWVIEW3D, "Drawtype%t|Bbox %x1|Wire %x2|Solid %x3|Shaded %x4",	28,180,100,18, &ob->dt);

*/

/* aan te brengen verbeteringen:
 * regel (y) moet met window geclipt worden (ofs,  max)
 * 
 */

/*
 * De effective UID van de fs is al direct ROOT.
 * er moet daarom een 'niet-root' fs opgestart worden om ervoor te zorgen
 * dat gewoonlijk de normale UID geldt.
 */


#include <sys/types.h>	    /* voor fork */
#include <unistd.h>

#include <string.h>	    /* voor buts!! */
#include <stdlib.h> 	 
#include <fcntl.h>
#include <sys/stat.h>
#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <local/Button.h>
#include <stdio.h>
#include <fmclient.h>
#include <local/storage.h>
#include <local/util.h>
#include <local/iff.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>	/* sproc */

#include <sys/times.h>	/* times */
#include <sys/param.h>

#include <local/gl_util.h>

#define TOPFREE  (40+16)
#define FIRSTLINE  (TOPFREE + 12)
#define BOTTOMY 12
#define SetButs drawbuttons
#define HILITE 1
#define ACTIVE 2

#define NOTACTIVE 0
#define ACTIVATE 1
#define INACTIVATE 2
#define GETDIR (USERPSEUDOFFSET+1)
#define PARENT (GETDIR+1)
#define STARTSWITH(x, y) (strncmp(x, y, sizeof(x) - 1) == 0)

struct But *dirbut, *filebut;
long win = 0,maxlen,numfiles;
long long totsize;

fmfonthandle fsfont = 0;

char dir[100],title[100],file[100],name[100], otherdir[100];
char fstmp[] = "/usr/tmp/fsdir";
char *fsmenu=0;
/*char fslog[] = "/usr/tmp/fslog";*/
char fslog[256];
long otherfs;
long fromfile = FALSE;
short su = FALSE;
short super = TRUE;
long outfile = -1;
long subut = FALSE;

void FileSelect(long orx,long ory,long wx,long wy);

void FS_addfilename_to_fsmenu(char *name)
{
	char *temp;
	int len;

	len= strlen(name);
	if(len==0) return;

	if(fsmenu) len+= strlen(fsmenu);

	if(fsmenu) {
	
		/* komtie voor? */
		if(strstr(fsmenu, name)) return;
		temp= mallocN(len+2, "fsmenu");
		
		strcpy(temp, fsmenu);
		strcat(temp, "|");
		strcat(temp, name);
		freeN(fsmenu);
		fsmenu= temp;
	}
	else {
		fsmenu= mallocN(len+2, "fsmenu");
		strcpy(fsmenu, name);
	}
}

void FS_readBlog()
{
	FILE *fp;
	long file, len;
	int end;
	char name[256], *home;

	home = getenv("HOME");
	if (home) {
/*		strcpy(name, home);
		strcat(name, "/.Blog");
		file= open(name, O_RDONLY);
		if (file >= 0) {
			len = read(file, G.sce, sizeof(G.sce));
			close(file);
			if (len > 0) G.sce[len] = 0;
		}
		
*/		
		strcpy(name, home);
		strcat(name, "/.Bfs");
		fp= fopen(name, "r");
		if(fp==NULL) {
			return;
		}
		
		end= 1;
		while(end>0) {
			end= fscanf(fp, "%s", name);
			if(end<=0) break;
			FS_addfilename_to_fsmenu(name);
		}
		
		fclose(fp);
	}
}



void copytoZ()
{
	long sizex,sizey;

	getsize(&sizex,&sizey);
	readsource(SRC_AUTO);
	zdraw(1);
	rectcopy(0,0,sizex-1,sizey-1,0,0);
	zdraw(0);

}

void copytoScreen()
{
	long sizex,sizey;

	getsize(&sizex,&sizey);
	readsource(SRC_ZBUFFER);
	rectcopy(0,0,sizex-1,sizey-1,0,0);
	readsource(SRC_AUTO);
}


short pup_it(char * str, long error)
{
	char a[250];
	long temp;
	short event, val;

	if (error) strcpy(a,"ERROR !%t |");
	else strcpy(a,"OK ?%t |");
	
	strcat(a,str);
	temp=defpup(a);

	sginap(2);
	event=dopup(temp);
	freepup(temp);
	if(event== -1) event=0;

	/*even een 1/2 seconde de tijd gunnen om op ESC te drukken */
	if (event){
		for(temp = 5; temp >0; temp--){
			while(qtest()){
				if (qread(&val) == ESCKEY){
					if (val){
						event = 0;
						break;
					}
				}
			}
			sginap(10);
		}
	}

	while(getbutton(RIGHTMOUSE)) sginap(10);
	return event;
}


short okee(char * str)
{
	long ret;
	char string[512];
	
	ret =  pup_it(str, 0);

	if (ret) {
		sprintf(string, "echo '  'fs_ok: %s >> $HOME/console", str);
		system(string);
	}
	
	return (ret);
	
}


short Error(char * str)
{
	return pup_it(str, 1);
}


void parent()
{
	short a;


	if(a = strlen(dir)){				/* eerst alle '/' weghalen aan het eind */
		while(dir[a-1] == '/'){
			a--;
			dir[a] = 0;
			if (a<=0) break;
		}
	}
	if(a = strlen(dir)){				/* daarna alles weghalen tot aan '/' */
		while(dir[a-1] != '/'){
			a--;
			dir[a] = 0;
			if (a<=0) break;
		}
	}
	if (a = strlen(dir)) {
		if (dir[a-1] != '/') strcat(dir,"/");
	} else strcpy(dir,"/");
}


void checkdir()
{
	short a;
	char *start,*eind;

	if (fromfile) return;

	while (start = strstr(dir,"/./")){
		eind = start + strlen("/./") - 1;
		strcpy(start,eind);
	}

	while (start = strstr(dir,"/../")){
		eind = start + strlen("/../") - 1;
		a = start-dir-1;
		while (a>0) {
			if (dir[a] == '/') break;
			a--;
		}
		strcpy(dir+a,eind);
	}

	while (start = strstr(dir,"//")){
		eind = start + strlen("//") - 1;
		strcpy(start,eind);
	}

	if(a = strlen(dir)){				/* eerst alle '/' weghalen aan het eind */
		while(dir[a-1] == '/'){
			a--;
			dir[a] = 0;
			if (a<=0) break;
		}
	}

	strcat(dir,"/");
}


void dir_file(s1,s2)
char *s1,*s2;
{
	short a;
	char string[100];

	strcpy(string,s1);
	strcat(string,s2);

	if (strlen(string)) {
		if (string[0] != '/') {
			getwdN(dir);
			strcat(dir,"/");
			strcat(dir,string);
			strcpy(string,dir);
		} else strcpy(dir,string);
	} else {
		getwdN(dir);
		file[0] = 0;
		return;
	}

	a = strlen(dir);
	while(exist(dir) == 0){
		a --;
		while(dir[a] != '/'){
			a--;
			if (a <= 0) break;
		}
		if (a >= 0) dir[a+1] = 0;
		else {
			strcpy(dir,"/");
			break;
		}
	}

	if (S_ISDIR(exist(dir))){
		strcpy(file,string + strlen(dir));
	} else {
		a = strlen(dir) - 1;
		while(dir[a] != '/') a--;
		dir[a + 1] = 0;
		strcpy(file,string + strlen(dir));
		return;
	}
	if (strrchr(file,'/')) strcpy(file,strrchr(file,'/')+1);

	if (a = strlen(dir)) {
		if (dir[a-1] != '/') strcat(dir,"/");
	}
}

void settitle()
{
	char s[100];
	double df;

	if (fromfile) return;

	df = diskfree(dir) / 1000.0;
	sprintf(s,"%s  Free: %.3f Mb  Files:%4d - %.3f kb\n",title,df/1000.0,numfiles,totsize/1000.0);
	if (df<1000.0) {
		strcat(s,"  ***********");
	}
	wintitle(s);
}


void printregel(files,x,y)
struct direntry *files;
short x,y;
{
	long wx,wy;

	if (y<BOTTOMY) return;

	getsize(&wx,&wy);

	switch(files->flags & (HILITE + ACTIVE)){
	case HILITE+ACTIVE:
		color(141);
		break;
	case HILITE:
		color(14);
		break;
	case ACTIVE:
		color(12);
		break;
	default:
		color(40);
		break;
	}

	sboxfs(30,y-4,wx-6,y+11);
	if(S_ISDIR(files->type)) color(WHITE);
	else color(BLACK);

	x += 30; cmov2i(x,y);
	fmprstr(files->relname);
	x += maxlen + 90;

	cmov2i(x - fmgetstrwidth(fsfont, files->size),y);
	fmprstr(files->size);

	/* rwx rwx rwx */
		x += 20; cmov2i(x,y); 
		fmprstr(files->mode1); 
	
		x += 30; cmov2i(x,y); 
		fmprstr(files->mode2); 
	
		x += 30; cmov2i(x,y); 
		fmprstr(files->mode3); 

	x += 30; cmov2i(x,y); 
	fmprstr(files->owner); 

	x += 60; cmov2i(x,y); 
	fmprstr(files->time); 

	x += 50; cmov2i(x,y); 
	fmprstr(files->date); 

	x += 80;
	color(40);
	sboxfs(wx-10,y-4,wx,y+11);
}


short printdir(dir,files,num,ofs,mode)
char *dir;
struct direntry *files;
short num,ofs,mode;
{
	extern struct ButCol BGbutcol[20];
	struct ButCol *bc;
	short y,x,a,h, sel, col;
	long wx,wy;
	float fac;

	getsize(&wx,&wy);

	if(getbutton(LEFTMOUSE)) {
		sel= 1;
		col= BGbutcol[0].paper_sel;
	}
	else {
		sel= 0;
		col= BGbutcol[0].paper_desel;
	}

	if(num< (wy-TOPFREE)/16) {
		ofs=0;
		color(BLACK);
		sboxfs(5,8,25,wy-TOPFREE);
		emboss(col, 9, 12, 21, wy-TOPFREE-2, sel);
	} else {
		if(ofs<0) ofs=0;
		if(ofs>(num-(wy-TOPFREE)/16)) ofs=num-(wy-TOPFREE)/16;

		fac=16*num;
		fac= (wy-FIRSTLINE)/fac;
		h=fac*(wy-FIRSTLINE)-2;
		a=2+fac*(16*num-(wy-FIRSTLINE+16*ofs));

		color(BLACK);
		sboxfs(5,8,25,wy-TOPFREE);

		emboss(col, 9, 10+a, 21, 10+a+h, sel);
	}

	if (mode) return(ofs);

	x=20;
	y = wy - FIRSTLINE;

	files += ofs;
	for(a=ofs;a<num;a++) {    /* printen */
		color(40);
		printregel(files,x,y);
		files++;
		y-=16;
		if(y<BOTTOMY) break;
	}
	color(40);
	sboxfs(30,2,wx,y+11);

	return(ofs);
}


void drawbuttons()
{
	long wx,wy;
	
	getsize(&wx,&wy);

	DefButBlock("FS",win,fsfont,10,0,0);
	if (fromfile){
		filebut = DefBut(TEX,3,"",		6,wy-32,wx-12,22,file,0.0,100.0);
	} else{
		DefBut(BUT,1,"Par",	5,wy-26,40,18);
		dirbut = DefBut(TEX,2,"",		50,wy-26,wx-56,18,dir,0.0,100.0);
		filebut = DefBut(TEX,3,"",		50,wy-26-24,wx-56,18,file,0.0,100.0);
	if(fsmenu) DefBut(MENU|SHO,	5, fsmenu, 5,wy-26-24,40,18, &super);
/*		if (subut) DefBut(TOG|SHO,4,"SU",	5,wy-26-24,40,18, &su);
*/
	}
}


long getfromfile(char * name, struct direntry **_files)
{
	FILE *fd;
	char string[1001], *text;
	struct direntry * files = 0;
	long count = 0;
	extern int compare();

	fd = fopen(name, "r");
	if (fd){
		/* lees alles behalve \n */
		while (fscanf(fd, "%1000[^\n]", string) != EOF){
			count++;
			/* lees nu \n */
			if (fscanf(fd, "\n") == EOF) break;
		}
		if (count){
			files = callocstructN(struct direntry, count, "fromfile");
			if (files){
				rewind(fd);
				count = 0;
				while (fscanf(fd, "%1000[^\n]", string) != EOF){
					text = callocN(strlen(string) + 49, "text");
					if (text == 0) break;
					strcpy(text+48, string);
					files[count].relname = text+48;
					files[count].string = text;
					count++;
					/* lees nu \n */
					if (fscanf(fd, "\n") == EOF) break;
				}
				if (count){
					if (fromfile == -TRUE){
						qsort(files,count,sizeof(struct direntry),compare);
					}
				}
			}
		}
	}

	*_files = files;
	return(count);
}


void procesdir(files,num)
struct direntry *files;
short num;
{
	char *s;
	long len;

	maxlen = totsize = numfiles = 0;
	for (num--; num>=0 ; num--){
		len = fmgetstrwidth(fsfont,files[num].relname);
		if (len > maxlen) maxlen = len;
		
		if ((S_ISDIR(files[num].type)) == 0){
			numfiles ++;
			totsize += files[num].s.st_size;
		}
	}
	drawbuttons();
	settitle();
}


void writetmp()
{
	FILE *fp;
	if (fp = fopen(fstmp, "w")){
		fprintf(fp, "%s\n", dir);
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
		chmod(fstmp, 0777);
	}
}


short checktmp()
{
	struct stat st;
	short ok=TRUE;
	FILE *fp;

	strcpy(otherdir, dir);
	otherfs = getpid();

	if (ok){
		if (fp = fopen(fstmp, "r")){
			if (fscanf(fp, "%s", otherdir) == EOF) ok = FALSE;
			if (fscanf(fp, "%d", &otherfs) == EOF) ok = FALSE;
			fclose(fp);
		} else ok = FALSE;
	}

	if (ok){
		/* file bestaat en is jonger dan 2 uur? */
		if (stat(fstmp, &st) != -1){
			if ((time(0) - st.st_mtime) > 7200){
				otherfs = getpid();
			} else{
				if (kill(otherfs, 0)){
					/* otherfs bestaat niet meer */
					otherfs = getpid();
				}
			}
		} else ok = FALSE;
	}

	/* dit kan vastlopen bij niet meer bestaande hosts */
/*
	if (stat(otherdir, &st) == -1){
		strcpy(otherdir, dir);
		ok = FALSE;
	}
*/
	if (ok == FALSE) writetmp();
	return(ok);
}


long countselect(files, num)
struct direntry *files;
long num;
{
	long i, count = 0;

	for(i = 0; i < num; i++){
		if (files[i].flags & ACTIVE) count++;
	}

	return(count);
}


long docmd(cmd)
char *cmd;
{
	char string[200];
	long file, error, size;
	short val;

	while (qtest()){
		if (qread(&val) == ESCKEY){
			if (val) return(-1);
		}
	}
		
	strcpy(string, cmd);
	strcat(string, " 1>/dev/null 2>>");
	strcat(string, fslog);

	remove(fslog);
	
	if (error = system(string)){
		if (file = open(fslog, O_RDONLY)){
			if ((size = read(file, string, sizeof(string)-1)) > 0){
				string[size] = 0;
				Error(string);
			} else Error(cmd);
			close(file);
		} else Error(cmd);
	}
	remove(fslog);
	
	return(error);
}


long docmd__(cmd)
char *cmd;
{
	char string[200];
	long file, error, size;
	short val;
	static FILE * shell = 0;
	
	if (shell == 0) shell = popen("sh -s ", "w");
	if (shell == 0) exit(0);

	while (qtest()){
		if (qread(&val) == ESCKEY){
			return(-1);
		}
	}
	
	remove(fslog);
	if (error = (fprintf(shell, "%s  1>/dev/null 2>> %s ; rm %s\n", cmd, fslog, fslog) == EOF)){
		exit(0);
	} else {
		fflush(shell);
		if (file = open(fslog, O_RDONLY)){
			if ((size = read(file, string, sizeof(string)-1)) > 0){
				string[size] = 0;
				Error(string);
			}
			close(file);
		}
	}
	remove(fslog);
	return(error);
}


void sigtrap(sig)
long sig;
{
	if (sig == SIGUSR1){
		qenter(GETDIR, 0);
		signal(SIGUSR1, sigtrap);
	}
}

void toterminal(cmd)
char *cmd;
{
	long len, i;

	len = strlen(cmd);
	for(i = 0 ; i< len; i++) ioctl(0,TIOCSTI,&cmd[i]);
}

void loadfont(){
	fmfonthandle fmfont;
	char *fontname;
	double fontsize;
	long maxx;
	
	maxx = getgdesc(GD_XPMAX) - 1;
	fminit();
	fontname = "Helvetica-Narrow-Bold";
	fontsize = 13.0 * 1279.0 / maxx;
	if (maxx > 1200) {
		fontsize = 13.0;
	} else if (maxx > 900) {
		fontsize = 11.0;
	} else {
		fontsize = 21.0;
	}
	if(fmfont=fmfindfont(fontname)) fsfont=fmscalefont(fmfont,fontsize);
}


void workspacecommand()
{
	FILE * pipein, * pipeout;
	short pipestart = FALSE;
	char cmd[512];
	
	if (file[0] != 0) {
		percentdone(10);
		
		if (ispic(file) == TGA) { /* patch */
			sprintf(cmd, "paint %s", file);
			system(cmd);
		} else {
			sprintf(cmd, "wstype -v %s/%s", dir, file);
			
			pipein = popen(cmd, "r");
			pipeout = popen("sh -s ", "w");
			pipestart = FALSE;
			percentdone(20);
			
			if (pipein != NULL && pipeout != 0) {
				fprintf(pipeout, "cd %s\n", dir);
				fprintf(pipeout, "LEADER=%s/%s\n", dir, file);
				fprintf(pipeout, "SELECTED=%s/%s\n", dir, file);
				fprintf(pipeout, "ARGC=1\n");
				percentdone(50);
				while(fscanf(pipein, " %511[^\n]\n", cmd) != EOF) {
					/*fprintf(stderr, "%s\n", cmd);*/
					if (STARTSWITH("TYPE", cmd)) {
						fprintf(pipeout, "LEADERTYPE=%s\n", cmd + 5);
					} else if (STARTSWITH("CMD OPEN", cmd)) {
						pipestart = TRUE;
						fprintf(pipeout, "%s\n", cmd + 8);
					} else if (STARTSWITH("CMD", cmd)) {
						pipestart = FALSE;
					} else if (pipestart) {
						fprintf(pipeout, "%s\n", cmd);
					}
				}
				percentdone(90);
			}
			if (pipein != NULL) pclose(pipein);
			if (pipeout != NULL) pclose(pipeout);					
		}
		percentdone(100);
	}
}


void FileSelect(orx,ory,wx,wy)
long orx,ory,wx,wy;
{
	short y,yo=376,x,xo=0,num=0,val,mval[2],ofs=0,aa,event,q_event;
	char s[512], head[100], tail[100];
	char supername[100];
	ushort numlen;
	Device mdev[2];
	long maxx,maxy,ready, number;
	float fac;
	struct direntry *files;
	short redraw = FALSE, selecting = NOTACTIVE;
	struct tms voidbuf;
	long prevtime, newtime, prevsel;
	int exi;
	FILE * pipein;
	
	prevtime = times(&voidbuf);
	
	if (orx != 'SP') {
		noport();
		winopen("");
	
		mdev[0]=MOUSEX;
		mdev[1]=MOUSEY;
		getdev(2, mdev, mval);
		maxx = getgdesc(GD_XPMAX) -1;
		maxy = getgdesc(GD_YPMAX) -1;
	
		if (orx == 32767){
			orx = mval[0] - (wx/2);
			ory = mval[1] - (wy/2);
	
			if (orx < 20) orx = 20;
			if (ory < 20) ory = 20;
			if (orx  > maxx - wx - 20) orx = maxx - wx - 20;
			if (ory  > maxy - wy - 40) ory = maxy - wy - 40;
			setvaluator(MOUSEX,orx+wx/2,0,maxx);
			setvaluator(MOUSEY,ory+wy/2,0,maxy);
		} else{
			/* als 'ie te ver naar rechts dreigt te komen spring dan naar links */
			if (abs(orx - (maxx / 2)) > abs(orx - wx - (maxx / 2))) orx -= 2 * (wx + 16);
		}
	
		prefposition(orx,orx+wx-1,ory,ory+wy-1);
		win = winopen(title);
	
		gconfig();
		copytoZ();
		color(40);
		clear();
		color(BLACK);
		minsize(wx,wy);
		if (fromfile == 0) maxsize(maxx/2,maxy-16);
		stepunit(1,16);
		winconstraints();
	
		if (fsfont == 0) loadfont();
		
		qreset();
		qdevice(REDRAW);	    
		qdevice(INPUTCHANGE);
		qdevice(WINSHUT);	    
		qdevice(WINQUIT);
		qdevice(LEFTMOUSE);	    
		qdevice(MIDDLEMOUSE);	
		qdevice(RIGHTMOUSE);
		qdevice(RAWKEYBD);	    
		qdevice(KEYBD);
		qdevice(ESCKEY);
		qdevice(RIGHTARROWKEY); 
		qdevice(LEFTARROWKEY);
		qdevice(MOUSEY);
		qdevice(RETKEY);	    
		qdevice(PADENTER);
		qdevice(PAGEDOWNKEY);   
		qdevice(PAGEUPKEY);
		qdevice(HOMEKEY);	    
		qdevice(ENDKEY);
		fmsetfont(fsfont);
	
		aa=0; 
		x=0;

		if (fromfile){
			num = getfromfile(dir, &files);
		} else{
			num = getdir(dir,&files);
			procesdir(files,num);
		}
		
		if (file[0]) {
			for (ofs = 2; ofs < num; ofs++) {
				if (S_ISREG(files[ofs].type)) {
					if (strcasecmp(file, files[ofs].relname) <= 0) {
						break;
					}
				}
			}
			ofs -= 9;
		}
		
		if (ofs > num) ofs = num;
		else if (ofs < 0) ofs = 0;
		
		drawbuttons();
		printdir(s,files,num,0,0);  /* eerst alleen slider */
	
		ready=FALSE;
		qenter(REDRAW, win);
	}
	
	
	while (ready==FALSE) {
/*		if (qtest()==0) sginap(1);
		while (qtest() == 0) sginap(5);
*/
		switch (q_event= qread(&val)) {
		case LEFTMOUSE:
			if (val) {
				event=DoButtons();
				if(event) {
					if (event==1) {  			/* parent */
						qenter(PARENT, 0);
					} else if (event==2) {  	/* dirbut */
						strcat(dir,"/");
						if (exist(dir) == 0){
							if (okee("Makedir")) recurdir(dir);
						}
						checkdir();
						ofs = 0;
						qenter(GETDIR, 0);
					} else if (event==3) {  	/* filebut */
						if (filebut->min==1){
							if (outfile != -1) ready=TRUE;
						}
					} else if (event==4) {  	/* subut */
						setreuid(geteuid(), getuid());
						setregid(getegid(), getgid());
					} else if (event==5) {  	/* super */
						getname_menu_but(supername, fsmenu, super);
						strcpy(dir, supername);
						checkdir();
						qenter(GETDIR, 0);
					}
					break;
				}
			}
		case MIDDLEMOUSE:
		case RIGHTMOUSE:
			if (val){
				getdev(2, mdev, mval);
				if(mval[0]<30+orx && q_event == LEFTMOUSE) {     /* slider */
					if ((mval[1] > 5 + ory) && (mval[1] <  ory+wy-TOPFREE)){
						if(yo+ofs>=0 && yo+ofs<num) files[yo+ofs].flags &= ~HILITE;
						fac=16*num;
						fac= (wy-TOPFREE)/fac;
						xo=ofs;
						yo=mval[1];
						if(fac<1.0) {
							ofs = printdir(dir,files,num,ofs,1);
							while (getbutton(LEFTMOUSE) == TRUE) {
								getdev(2, mdev, mval);
								x = xo+(yo-mval[1])/(16*fac);
								if(x<0) x=0;
								if(x>(num-(wy-TOPFREE)/16)) x=num-(wy-TOPFREE)/16;
								if(x!=ofs) {
									ofs=x;
									ofs = printdir(dir,files,num,ofs,0);
								}
								do{
									qread(&val);
								}while (qtest());
							}
							ofs = printdir(dir,files,num,ofs,1);
						}
					}
				} else {		/* file of dir  geselecteerd */
					if ((mval[1]-ory) < 7) break;
					y=(wy-FIRSTLINE+10-(mval[1]-ory) )/16;

					if(y>=0 && y+ofs<num) {
						if (q_event == RIGHTMOUSE){
							if (fromfile) break;
							if (files[y+ofs].flags & ACTIVE){
								files[y+ofs].flags &= ~ACTIVE;
								selecting = INACTIVATE;
							} else{
								if ((y + ofs) >= 2) files[y+ofs].flags |= ACTIVE;
								selecting = ACTIVATE;
							}
							printregel(&files[y+ofs],20,wy-FIRSTLINE-16*y);
							yo = y;
						} else if(S_ISDIR(files[y+ofs].type)) {
							strcat(dir,files[y+ofs].relname);
							strcat(dir,"/");
							checkdir();
							num=getdir(dir,&files);
							procesdir(files,num);
							while (getbutton(LEFTMOUSE) == TRUE) sginap(5);
							ofs = printdir(dir,files,num,0,0);

							y=yo;
							if(y>=0 && y+ofs<num) {
								files[y+ofs].flags |= HILITE;
								printregel(&files[y+ofs],20,wy-FIRSTLINE-16*y);
							}
						} else{
							if (q_event == LEFTMOUSE) {
								newtime = times(&voidbuf);
								if ((fromfile == FALSE) && (prevsel == y + ofs) && (newtime - prevtime <= HZ / 3)) {
									workspacecommand();
									newtime -= HZ;
								}
								prevsel = y + ofs;
								prevtime = newtime;
							}
							strcpy(file,files[y+ofs].relname);
							drawbuttons();
							if(q_event==MIDDLEMOUSE && outfile != -1) ready=TRUE;
						}
					}
				}
			} else{
				if (q_event == RIGHTMOUSE) selecting = NOTACTIVE;
			}
			while (getbutton(LEFTMOUSE) == TRUE) sginap(5);
			while (getbutton(MIDDLEMOUSE) == TRUE) sginap(5);
			break;
		case PARENT:
			parent();
			ofs = 0;
		case GETDIR:
			/* doet meteen een REDRAW */
			num=getdir(dir,&files);
			procesdir(files,num);
			/*ofs = printdir(dir,files,num,ofs,0);*/
		case DEPTHCHANGE:
		case REDRAW:
			getsize(&wx,&wy);
			getorigin(&orx,&ory);
			ortho2(-0.5,wx-0.5,-0.5,wy-0.5);
			viewport(0,wx-1,0,wy-1);
			color(40);
			clear();
			drawbuttons();
			ofs = printdir(dir,files,num,ofs,0);
			redraw = TRUE;
			break;
		case INPUTCHANGE:
			if(val) checktmp();
			else writetmp();
			break;
		case RETKEY:
		case PADENTER:
			if (val) {
				if (outfile != -1) ready=TRUE;
				else workspacecommand();
			}
			break;
		case PAGEUPKEY:
			if (val){
				yo += ofs;
				y = (wy - BOTTOMY - FIRSTLINE) / 16;
				ofs -= y;
				ofs = printdir(dir,files,num,ofs,0);
				yo -= ofs;
			}
			break;
		case PAGEDOWNKEY:
			if (val){
				yo += ofs;
				y = (wy - BOTTOMY - FIRSTLINE) / 16;
				ofs += y;
				ofs = printdir(dir,files,num,ofs,0);
				yo -= ofs;
			}
			break;
		case HOMEKEY:
			if (val){
				yo += ofs;
				ofs = printdir(dir,files,num,0,0);
				yo -= ofs;
			}
			break;
		case ENDKEY:
			if (val){
				yo += ofs;
				ofs = printdir(dir,files,num,32767,0);
				yo -= ofs;
			}
			break;
		case KEYBD:
			if (fromfile) break;

			switch(val){
			case 32:
				sprintf(s, "cd %s\n", dir);
				toterminal(s);
				break;
			case '.':
				qenter(GETDIR, 0);
				break;
			case '=':
			case '+':
				newname(file, +1);
				drawbuttons();
				break;
			case '-':
				newname(file, -1);
				drawbuttons();
				break;
			case 'v':
			case 'V':
				sprintf(s, "dirview %s", dir);
				system(s);
				break;
			case 'i':
			case 'I':
				sprintf(s, "imp %s%s", dir, file);
				system(s);
				break;
			case 'a':
			case 'A':
				if (countselect(files, num)){
					long i;
					for (i = num-1 ; i >= 0 ; i--){
						files[i].flags &= ~ACTIVE;
					}
				} else{
					long i;
					for (i = num-1 ; i >= 2 ; i--){
						files[i].flags |= ACTIVE;
					}
				}
				ofs = printdir(dir,files,num,ofs,0);
				break;
			case 'e':
			case 'E':
				strcpy(s,dir);
				strcat(s,file);
				exi = exist(s);
				if (S_ISREG(exi) || exi == 0){
					strcpy(s,"$WINEDITOR ");
					strcat(s,dir);
					strcat(s,file);
					system(s);
				}
				break;
			case 'z':
			case 'Z':	/* open a file viewer */
				strcpy(s,dir);
				strcat(s,file);
				if (S_ISREG(exist(s))){
					getdev(2, mdev, mval);
					sprintf(s, "it %s%s %d %d %d %d", dir, file, 
					    mval[0], mval[1], mval[0]+600, mval[1]+700);
					system(s);
				}
				break;
			case 'c':
			case 'C':
				if (checktmp() && countselect(files, num) != 0){
					if (strcmp(dir, otherdir)){
						sprintf(s, "Copy to %s", otherdir);
						if (okee(s)){
							long i;
							for (i = 0; i <num; i++){
								if (files[i].flags & ACTIVE){
									sprintf(s, "cp -r \"%s%s\" %s", dir, files[i].relname,
										otherdir);
									if (docmd(s)) break;
									else files[i].flags &= ~ACTIVE;
								}
							}
							/*remove(fslog);*/
							ofs = printdir(dir,files,num,ofs,0);
							if (otherfs != getpid()) kill(otherfs, SIGUSR1);
						}
					}
				}
				break;
			case 'b':
			case 'B':
				if (checktmp() && countselect(files, num) != 0){
					if (strcmp(dir, otherdir)){
						sprintf(s, "BackUp to %s", otherdir);
						if (okee(s)){
							long i;
							for (i = 0; i <num; i++){
								if (files[i].flags & ACTIVE){
									sprintf(s, "su root -c 'cd %s; tar cf - \"%s\" | (cd %s; tar xf -)'", dir, files[i].relname,
										otherdir);
									if (docmd(s)) break;
									else files[i].flags &= ~ACTIVE;
								}
							}
							/*remove(fslog);*/
							ofs = printdir(dir,files,num,ofs,0);
							if (otherfs != getpid()) kill(otherfs, SIGUSR1);
						}
					}
				}
				break;
			case 'l':
			case 'L':
				if (checktmp() && countselect(files, num) != 0){
					if (strcmp(dir, otherdir)){
						sprintf(s, "Make linked copy to %s", otherdir);
						if (okee(s)){
							long i;
							for (i = 0; i <num; i++){
								if (files[i].flags & ACTIVE){
									sprintf(s, "ln -s %s%s %s", dir, files[i].relname,
										otherdir);
									if (docmd(s)) break;
									else files[i].flags &= ~ACTIVE;
								}
							}
							/*remove(fslog);*/
							ofs = printdir(dir,files,num,ofs,0);
							if (otherfs != getpid()) kill(otherfs, SIGUSR1);
						}
					}
				}
				break;
			case 'm':
			case 'M':
				if (checktmp() && countselect(files, num) != 0){
					if (strcmp(dir, otherdir)){
						sprintf(s, "Move to %s", otherdir);
						if (okee(s)){
							long i, error = 0;
							for (i = 0; i <num; i++){
								if (files[i].flags & ACTIVE){
									sprintf(s, "mv %s%s %s", dir, files[i].relname, otherdir);
									if (error = docmd(s)) break;
									else files[i].flags &= ~ACTIVE;
								}
							}
							/*remove(fslog);*/
							if (otherfs != getpid()) kill(otherfs, SIGUSR1);
							if (error == 0) qenter(GETDIR, 0);
							else ofs = printdir(dir,files,num,ofs,0);
						}
					}
				}
				break;
			case 't':
			case 'T':
				if (checktmp() && countselect(files, num) != 0){
					sprintf(s, "Touch");
					if (okee(s)){
						long i, error = 0;
						for (i = 0; i <num; i++){
							if (files[i].flags & ACTIVE){
								sprintf(s, "touch %s%s", dir, files[i].relname);
								if (error = docmd(s)) break;
								else files[i].flags &= ~ACTIVE;
							}
						}
						qenter(GETDIR, 0);
					}
				}
				break;
			case 'r':
				if (num == 2){
					sprintf(s, "rmdir \"%s\"", dir);
					if (okee(s)){
						parent();
						chdir(dir);
						docmd(s);
						qenter(GETDIR, 0);
					}
				} else if (countselect(files, num) != 0){
					sprintf(s, "Remove from %s", dir);
					if (okee(s)){
						long i, error = 0;
						for (i = 0; i <num; i++){
							if (files[i].flags & ACTIVE){
								if(S_ISDIR(files[i].type)) sprintf(s, "rmdir \"%s%s\"", dir,
								    files[i].relname);
								else sprintf(s, "rm -f \"%s%s\"", dir, files[i].relname);
								if (error = docmd(s)) break;
								else files[i].flags &= ~ACTIVE;
							}
						}
						remove(fslog);
						if (error == 0) qenter(GETDIR, 0);
						else ofs = printdir(dir,files,num,ofs,0);
					}
				}
				break;
			case 'R':
				if (countselect(files, num) != 0){
					sprintf(s, "Remove from %s", dir);
					if (okee(s)){
						long i, error = 0;
						for (i = 0; i <num; i++){
							if (files[i].flags & ACTIVE){
								sprintf(s, "rm -rf %s%s", dir, files[i].relname);
								if (error = docmd(s)) break;
								else files[i].flags &= ~ACTIVE;
							}
						}
						remove(fslog);
						if (error == 0) qenter(GETDIR, 0);
						else ofs = printdir(dir,files,num,ofs,0);
					}
				}
				break;
			case 'f':
			case 'F':
				sprintf(s, "fs -w %d %d %d %d -d %s", orx + wx + 16, ory, wx, wy, dir);
				system(s);
				break;
			case 'p':
				qenter(PARENT, 0);
				break;
			case 'w':
			case 'W':
				FS_addfilename_to_fsmenu(dir);
						qenter(GETDIR, 0);
				break;
			case 'x':
			case 'X':
				strcpy(s,dir);
				strcat(s,file);
				if (S_ISREG(exist(s))) system(s);
				break;
			case '?':
				if (file[0]) {
					sprintf(s, "wstype %s", file);
					pipein = popen(s, "r");
					if (pipein) {
						if(fscanf(pipein, " %511[^\n]\n", s) != EOF) okee(s);
						pclose(pipein);
					}
				}
				break;
			case '/':
				strcpy(dir, "/");
				qenter(GETDIR, 0);
				break;
			}
			break;
		case ESCKEY:
			if (val == 0) break;
		case WINQUIT:
		case WINSHUT:
			dir[0]=file[0]=0;
			ready=TRUE;
			break;
		case MOUSEY:
			y=(wy-FIRSTLINE+10-(val-ory) )/16;
			if (y < 0) y = 0;
			if (y + ofs >= num) y = num - ofs - 1;
			if(y!=yo && qtest() != MOUSEY && num != 0){
				if(yo+ofs>=0 && yo+ofs<num) {
					files[yo+ofs].flags &= ~HILITE;
					if (yo >= 0) printregel(&files[yo+ofs],20,wy-FIRSTLINE-16*yo);
				}

				files[y+ofs].flags |= HILITE;
				if (selecting == NOTACTIVE) yo = y;
				if (yo + ofs >= num) yo = num - ofs - 1;
				if (yo + ofs < 0) yo = -ofs;
				do{
					if ((yo + ofs) >= 2){
						if (selecting == ACTIVATE) files[yo+ofs].flags |= ACTIVE;
						else if (selecting == INACTIVATE) files[yo+ofs].flags &= ~ACTIVE;
					}
					printregel(&files[yo+ofs],20,wy-FIRSTLINE-16*yo);
					if (yo < y) yo++;
					else if (yo > y) yo--;
					else break;
				}while(1);

				yo=y;
			}
			break;
		}
	}

	if (redraw == FALSE) copytoScreen();
	winclose(win);
	freedir();
	if (fsfont) fmfreefont(fsfont);
	if(fsmenu) freeN(fsmenu); /*ROB*/
}



main(argc,argv)
int argc;
char **argv;
{
	short val;
	long len, c;
	long orx=32767,ory,wx=450,wy=400;
	
	/* if we run setuid root, first switch back */
	
	if (geteuid() != getuid()) {
		subut = FALSE;
		setreuid(geteuid(), getuid());
		setregid(getegid(), getgid());
	}
	
	strcpy(title,"FileSelect");

	getwdN(dir);

	if (dir[1]) strcat(dir,"/");

	sprintf(fslog, "/usr/tmp/fslog%d", getpid());
	
	switch(argc){
	case 2:
		dir_file("",argv[1]);
	case 1:
		loadfont(); /* veroorzaakt korte pauze */
		break;
	default:
		while ((c = getopt(argc, argv, "t:p:d:w:f:s")) != -1){
			switch(c){
			case 't':
				strcpy(title, optarg);
				break;
			case 'd':
				fromfile = FALSE;
				dir_file("", optarg);
				break;
			case 'f':
				fromfile = TRUE;
				strcpy(dir, optarg);
				break;
			case 'p':
				sscanf(optarg,"%ld",&outfile);
				foreground();
				break;
			case 's':
				fromfile = -fromfile;
				break;
			case 'w':
				/*printf("%d %d \n", optind, argc);*/
				if (argc >= optind + 3){
					sscanf(optarg,"%ld",&orx);
					sscanf(argv[optind++],"%ld",&ory);
					sscanf(argv[optind++],"%ld",&wx);
					sscanf(argv[optind++],"%ld",&wy);
					/*printf("%d %d %d %d\n", orx, ory, wx, wy);*/
					break;
				}
			case '?':
				printf("usage: %s [dir] | [-t title] [-p pipe] [-d directory] [-w x y width heigth]\n", argv[0]);
				if (outfile) {
					dir[0] = 0;
					len = 1;
					write(outfile,&len,4);
					write(outfile,dir,len);
				}
				exit(0);
				break;
			}
		}
	}

	if (fromfile == FALSE){
		checkdir();
		checktmp();
		signal(SIGUSR1, sigtrap);
		signal(SIGTTOU, SIG_IGN); /* voor comando's naar terminal*/
	}

	FS_readBlog();

	FileSelect(orx, ory, wx, wy);
	if (fromfile) strcpy(dir,file);
	else strcat(dir,file);

	if (outfile == -1){
		/*printf("%s\n",dir);*/
	} else{
		len=strlen(dir)+1;
		write(outfile,&len,4);
		write(outfile,dir,len);
	}
	if (win) gexit();
	wait(0);
	exit(0);
}

