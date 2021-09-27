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

#include <stdio.h>
#include <stdlib.h>			/* voorkomt dat je bij malloc type moet aangeven */
#include <dirent.h>
#include <sys/stat.h>
#ifdef __sgi
#include <sys/statfs.h>
#endif
#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif
#ifdef linux
#include <sys/vfs.h>
#endif
#include <fcntl.h>
#include <sys/mtio.h>			/* tape comando's */
#include <string.h>			/* strcpy etc.. */
#include <sys/ioctl.h>
#include <unistd.h>			/* lseek */
#include <pwd.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include "util.h"

#include "storage.h"

/*

bindkey -r f1,'cc -O -g -c storage.c\nmake fs\nfs\n'
bindkey -r f2,'make fs\n'

bindkey -r f3,'cc -O -c storage.c\n'

*/


long totnum,actnum,dirflags = 0;
struct direntry *files;

struct ListBase dirbase_={
	0,0};
struct ListBase *dirbase = &dirbase_;


char * getwdN(char * dir)
{
	char *pwd;

	if (dir){
		pwd = getenv("PWD");
		if (pwd){
			strcpy(dir, pwd);
			return(dir);
		}
		return(getwd(dir));
	}
	return(0);
}

void fastdir(on)
short on;
{
	if (on) dirflags |= FASTDIR;
	else dirflags &= ~FASTDIR;
}


int compare(entry1,entry2)
struct direntry *entry1,*entry2;
{
	/* type is gelijk aan stat.st_mode */

	if (S_ISDIR(entry1->type)){
		if (S_ISDIR(entry2->type)==0) return (-1);
	} else{
		if (S_ISDIR(entry2->type)) return (1);
	}
	if (S_ISREG(entry1->type)){
		if (S_ISREG(entry2->type)==0) return (-1);
	} else{
		if (S_ISREG(entry2->type)) return (1);
	}
	if ((entry1->type & S_IFMT) < (entry2->type & S_IFMT)) return (-1);
	if ((entry1->type & S_IFMT) > (entry2->type & S_IFMT)) return (1);
	return (strcasecmp(entry1->relname,entry2->relname));
}


double diskfree(dir)
char *dir;
{
	struct statfs disk;
	char name[100],*slash;


	strcpy(name,dir);

	if(strlen(name)){
		slash = strrchr(name,'/');
		if (slash) slash[1] = 0;
	} else strcpy(name,"/");

#if defined __FreeBSD__ || defined linux
	if (statfs(name, &disk)) return(-1);
#else
	if (statfs(name,&disk,sizeof(struct statfs),0)){
		printf("diskfree: Couldn't get information about %s.\n",dir);
		return(-1);
	}
#endif

	return ((double) disk.f_bsize * (double) disk.f_bfree);
}


void builddir(dirname,relname)
char *dirname,*relname;
{
	struct dirent *fname;
	struct dirlink *dlink;
	long rellen,num,newnum = 0, seen_ = FALSE, seen__ = FALSE;
	char buf[256];
	DIR *dir;


	strcpy(buf,relname);
	rellen=strlen(relname);

	if (rellen){
		buf[rellen]='/';
		rellen++;
	}

	if (chdir(dirname) == -1){
		perror(dirname);
		return;
	}
	if (dir = (DIR *)opendir(".")){
		while ((fname = readdir(dir)) != NULL){
			dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
			if (dlink){
				strcpy(buf+rellen,fname->d_name);
				dlink->name = strdup(buf);
				if (dlink->name[0] == '.') {
					if (dlink->name[1] == 0) seen_ = TRUE;
					else if (dlink->name[1] == '.') {
						if (dlink->name[2] == 0) seen__ = TRUE;
					}
				}
				addhead(dirbase,dlink);
				newnum++;
			}
		}
		
		if (newnum){
			if (seen_ == FALSE) {	/* Cachefs PATCH */
				dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
				strcpy(buf+rellen,"./.");
				dlink->name = strdup(buf);
				addhead(dirbase,dlink);
				newnum++;
			}
			if (seen__ == FALSE) {	/* MAC PATCH */
				dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
				strcpy(buf+rellen,"./..");
				dlink->name = strdup(buf);
				addhead(dirbase,dlink);
				newnum++;
			}
			if (files) files=(struct direntry *)realloc(files,(totnum+newnum) * sizeof(struct direntry));
			else files=(struct direntry *)malloc(newnum * sizeof(struct direntry));

			if (files){
				dlink = (struct dirlink *) dirbase->first;
				while(dlink){
					memset(&files[actnum], 0 , sizeof(struct direntry));
					files[actnum].relname = dlink->name;
					if ((dirflags & FASTDIR) == 0){
						stat(dlink->name,&files[actnum].s);
						files[actnum].type=files[actnum].s.st_mode;
					}
					files[actnum].flags = 0;
					totnum++;
					actnum++;
					dlink = dlink->next;
				}
			} else{
				printf("Couldn't get memory for dir");
				exit(1);
			}
			freelist(dirbase);
			if (files) qsort(files,actnum,sizeof(struct direntry),compare);
		} else {
			printf("%s empty directory\n",dirname);
		}

		closedir(dir);
	} else {
		printf("%s non-existend directory\n",dirname);
	}
}


void buildpwtable(pwtable)
struct passwd **pwtable;
{
	long count=0,slen;
	struct passwd *pw,*pwtab;

	do{
		pw=getpwent();
		if (pw){
			count++;
		}
	}while(pw);
	endpwent();

	pwtab = (struct passwd *)calloc(count+1,sizeof(struct passwd));
	count=0;
	do{
		pw=getpwent();
		if (pw){
			pwtab[count] = *pw;
			slen = strlen(pw->pw_name);
			pwtab[count].pw_name = malloc(slen+1);
			strcpy(pwtab[count].pw_name,pw->pw_name);
			count ++;
		}
	}while(pw);
	pwtab[count].pw_name = 0;
	endpwent();

	*(pwtable) = pwtab;
}


void freepwtable(pwtable)
struct passwd *pwtable;
{
	long count=0;

	do{
		if (pwtable[count].pw_name) free(pwtable[count].pw_name);
		else break;
		count++;
	}while (1);

	free(pwtable);
}


char *findpwtable(pwtable,user)
struct passwd *pwtable;
ushort user;
{
	static char string[32];
	
	while (pwtable->pw_name){
		if (pwtable->pw_uid == user){
			return (pwtable->pw_name);
		}
		pwtable++;
	}
	sprintf(string, "%d", user);
	return (string);
}


void adddirstrings()
{
	char *ftype="?pc?d?b?-?l?s??";
	char datum[100];
	char buf[250];
	char size[250];
	static char * types[8] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	long num,ret,mode;
	int num1, num2, num3, num4, num5;
	struct passwd *pwtable;
	struct direntry * file;
	struct tm *tm;
	
	buildpwtable(&pwtable);

	file = &files[0];
	
	for(num=0;num<actnum;num++){
		mode = file->s.st_mode;
		strcpy(file->mode1, types[(mode & 0700) >> 6]);
		strcpy(file->mode2, types[(mode & 0070) >> 3]);
		strcpy(file->mode3, types[(mode & 0007)]);
		
		if (((mode & S_ISGID) == S_ISGID) && (file->mode2[2]=='-'))file->mode2[2]='l';

		if (mode & (S_ISUID | S_ISGID)){
			if (file->mode1[2]=='x') file->mode1[2]='s';
			else file->mode1[2]='S';

			if (file->mode2[2]=='x')file->mode2[2]='s';
		}

		if (mode & S_ISVTX){
			if (file->mode3[2] == 'x') file->mode3[2] = 't';
			else file->mode3[2] = 'T';
		}

		strcpy(files[num].owner, findpwtable(pwtable,files[num].s.st_uid));
		tm= localtime(&files[num].s.st_mtime);
		strftime(files[num].time, 8, "%R", tm);
		strftime(files[num].date, 16, "%d-%b-%y", tm);
		
		num1= files[num].s.st_size % 1000;
		num2= files[num].s.st_size/1000;
		num2= num2 % 1000;
		num3= files[num].s.st_size/(1000*1000);
		num3= num3 % 1000;
		num4= files[num].s.st_size/(1000*1000*1000);
		num4= num4 % 1000;
		
		if(num4) sprintf(files[num].size, "%3d %03d %03d %03d", num4, num3, num2, num1);
		else if(num3) sprintf(files[num].size, "%7d %03d %03d", num3, num2, num1);
		else if(num2) sprintf(files[num].size, "%11d %03d", num2, num1);
		else sprintf(files[num].size, "%15d", num1);

		strftime(datum, 32, "%d-%b-%y %R", tm);
		if (files[num].s.st_size < 1000) {
			sprintf(size, "%10d", files[num].s.st_size);
		} else if (files[num].s.st_size < 1000 * 1000) {
			sprintf(size, "%6d %03d", files[num].s.st_size / 1000, files[num].s.st_size % 1000);
		} else if (files[num].s.st_size < 100 * 1000 * 1000) {
			sprintf(size, "%2d %03d %03d", files[num].s.st_size / (1000 * 1000), (files[num].s.st_size / 1000) % 1000, files[num].s.st_size % 1000);
		} else {
			sprintf(size, "> %4.1f M", files[num].s.st_size / (1024.0 * 1024.0));
			sprintf(size, "%10d", files[num].s.st_size);
		}
		sprintf(buf,"%s %s %s %7s %s %s %10s %s", file->mode1, file->mode2, file->mode3, files[num].owner, files[num].date, files[num].time, size,
			files[num].relname);
		files[num].string=malloc(strlen(buf)+1);
		if (files[num].string){
			strcpy(files[num].string,buf);
		}
		
		file++;
	}
	freepwtable(pwtable);
}

freedir()
{
	long num;

	num=actnum-1;
	actnum=totnum=0;

	if (files==0){
		return;
	}
	for(;num>=0;num--){
		free(files[num].relname);
		if (files[num].string) free(files[num].string);
	}
	free(files);
	files=0;
}

ulong getdir(dirname,filelist)
char *dirname;
struct direntry **filelist;
{
	if (actnum){
		freedir();
	}

	builddir(dirname,"");
	if ((dirflags & FASTDIR) == 0) adddirstrings();
	*(filelist)=files;
	return(actnum);
}


int tapectl(long file,short cmd,ulong num)
{
	struct mtop mtop;

	mtop.mt_op = cmd;
	mtop.mt_count = num;

	return(ioctl(file,MTIOCTOP,&mtop));
}


long filesize(file)
long file;
{
	long cpos,len;

	if (file <= 0) return (-1);

	cpos = lseek(file,0L,SEEK_CUR);
	if (cpos == -1) return (-1);

	len = lseek(file,0L,SEEK_END);
	lseek(file,cpos,SEEK_SET);

	return (len);
}



long calcchksum(point,size)
long *point,size;
{
	long chksum;

	chksum=0;
	for(;size>0;size--){
		chksum += *(point++);
	}
	return (chksum);
}

short copy(cpy,org)
char *cpy,*org;
{
	char *buf;
	long in,out,len;

	if ((buf = (char *) malloc(1024)) == 0) return(-1);
	if ((in  = open(org,O_RDONLY)) < 0) {
		free(buf) ;
		return(-1);
	}
	if ((out  = open(cpy,O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
		close(in) ;
		free(buf) ;
		return(-1);
	}

	while ((len = read(in,buf,1024)) > 0){
		if (write(out,buf,len) != len){
			len = -1;
			break;
		}
	}
	close(in);
	close(out);
	free(buf);
	return (len);
}


int exist(name)
char *name;
{
	struct stat st;

	if (stat(name,&st)) return(0);
	return(st.st_mode);
}


int recurdir(name)
char *name;
{
	struct stat finfo;
	char *slash;

	slash=0;
	while (1){
		if (slash){
			*slash =  '/';
			slash = strchr(slash+1,'/');
		} else{
			slash = strchr(name,'/');
		}
		if (slash==0) return (1);

		if (slash != name){
			*slash = 0;
			/*			printf("%s\n",name); */
			if (stat(name,&finfo)){
				/* pad bestaat niet ! */
				if (mkdir(name,0777)) return (0);
			} else{
				if (S_ISDIR(finfo.st_mode));
				else return (0);
			}
		}
	}
}


int existo(name)
char *name;
{
	long file;

	file = open(name,O_RDONLY);
	if (file<=0) return (FALSE);
	close(file);
	return(TRUE);
}

