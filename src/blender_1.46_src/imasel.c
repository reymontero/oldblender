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
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/*  imasel.c      MIXED MODEL

 * 
 *  maart 97
 *  
 *  algemene functies
 *  
 */

#include "blender.h"
#include "screen.h"
#include "imasel.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


int  bitset(long l,  long bit)
{	return (( l & bit) == bit);  }

void longtochar(char *des, ulong *src, int size)
{	int i;for (i = 0; i<size; i++){ des[i] = src[i] & 0xFF; }}

void chartolong(ulong *des, char *src, int size)
{	int i;for (i = 0; i<size; i++){ des[i] = src[i]; }}

int dir_compare(in1, in2)
ImaDir **in1, **in2;
{
	ImaDir *use1, *use2;

	use1 = *in1;
	use2 = *in2;
	
	return strcasecmp(use1->name,  use2->name);
}

void issort( int te, ImaDir **firstentry)
{
	ImaDir **sort;
	ImaDir *use;
	int i = 0;
	
	sort = mallocN(te * 4,  "dir Sorteer temp");
	use = *firstentry;
	
	while (use){
		sort[i++] = use;
		use = use->next;
	}
	
	qsort (sort, te, 4, (void *)dir_compare);
	
	*firstentry = sort[0];
	use = *firstentry;
	
	
	for (i=0; i<te; i++){
		if (i != 0)    use->prev = sort[i-1]; else use->prev = 0;
		if (i != te-1) use->next = sort[i+1]; else use->next = 0;
	
		use = use->next;
	}
	
	freeN(sort);
}


int ima_compare(in1, in2)
OneSelectableIma **in1, **in2;
{
	OneSelectableIma *use1, *use2;
	use1 = *in1; use2 = *in2;
	return strcasecmp(use1->file_name,  use2->file_name);
}

void imsort(OneSelectableIma **firstentry)
{
	OneSelectableIma **sort;
	OneSelectableIma *use;
	int tot = 0, i = 0;
	
	use = *firstentry;
	while (use){
		tot++;
		use = use->next;
	}
	
	if (tot){
		sort = mallocN(tot * 4,  "Sorteer imsort temp");
		use = *firstentry;
		while (use){
			sort[i++] = use;
			use = use->next;
		}
		
		qsort (sort, tot, 4, (void *)ima_compare);
		
		*firstentry = sort[0];
		use = *firstentry;
		for (i=0; i<tot; i++){
			if (i != 0)     use->prev = sort[i-1]; else use->prev = 0;
			if (i != tot-1) use->next = sort[i+1]; else use->next = 0;
		
			use = use->next;
		}
		freeN(sort);
	}
}

void str_image_type(long ftype, char *name)
{
	strcpy(name, "");
	
	if((ftype & JPG_MSK) == JPG_STD) strcat(name, "std ");
	if((ftype & JPG_MSK) == JPG_VID) strcat(name, "video ");
	if((ftype & JPG_MSK) == JPG_JST) strcat(name, "amiga ");
	if((ftype & JPG_MSK) == JPG_MAX) strcat(name, "max ");
	
	
	if( ftype == AN_hamx)   { strcat(name, "hamx "); return; }
	if( ftype == AN_yuvx)   { strcat(name, "yuvx "); return; }
	if( ftype == AN_tanx)   { strcat(name, "tanx "); return; }
	
	if( ftype == IMAGIC )   { strcat(name, "sgi ");  return; }
	if( ftype & JPG )       { strcat(name, "jpeg ");  }
	if( ftype & TGA )       { strcat(name, "targa "); }
	if( ftype & TIM )       { strcat(name, "tim ");   }
	if( ftype & AMI )       { strcat(name, "iff ");	  }
	 
	if(( ftype & AM_ham )   == AM_ham)    strcat(name, "ham ");
	if(( ftype & AM_hbrite )== AM_hbrite) strcat(name, "halfbrite ");
	if(( ftype & AM_lace )  == AM_lace)   strcat(name, "lace ");
	if(( ftype & AM_hires ) == AM_hires)  strcat(name, "hires ");
	
	if( ftype & CDI )        strcat(name, "cdi ");
	if( ftype == CD_rgb8 )   strcat(name, "rgb8 ");
	if( ftype == CD_rgb5 )   strcat(name, "rgb5 ");
	if( ftype == CD_dyuv )   strcat(name, "dyuv ");
	if( ftype == CD_clut8 )  strcat(name, "cmap 8 bit ");	
	if( ftype == CD_clut7 )  strcat(name, "cmap 7 bit ");	
	if( ftype == CD_clut4 )  strcat(name, "cmap 4 bit ");	
	if( ftype == CD_clut3 )  strcat(name, "cmap 3 bit ");	
	if( ftype == CD_rl7 )    strcat(name, "rle 7 bit ");	
	if( ftype == CD_rl3 )    strcat(name, "rle 3 bit ");	
	if( ftype == CD_mplte )  strcat(name, "mplte ");	
	
}

void append_pib(SpaceImaSel *simasel, OneSelectableIma *ima)
{
	long flen;
	int  file, wr;
	char name[256];
	OneSelectableIma *big_ima;
	
	if ( bitset (simasel->fase, IMS_WRITE_NO_BIP)) return;
	
	strcpy(name, simasel->dir);
	strcat(name, ".Bpib");
	
	file = open(name, O_APPEND | O_RDWR | O_CREAT, 0666);
	if (file == -1) {
		/* printf("Could not write .Bpib file in dir %s\n", simasel->dir); */
		simasel->fase |= IMS_WRITE_NO_BIP;
		return;
	}
	fchmod(file, 0666);

	flen = lseek(file, 0, SEEK_END);
	
	big_ima = mallocN(sizeof(OneSelectableIma), "BigIma");
		
	big_ima->header		= MAKE_ID('B', 'I', 'P', '2');
	big_ima->cmap		= BIG_SHORT(ima->cmap);
	big_ima->image		= BIG_SHORT(ima->image);
	big_ima->draw_me	= BIG_SHORT(ima->draw_me);
	big_ima->rt			= BIG_SHORT(ima->rt);
	big_ima->sx			= BIG_SHORT(ima->sx); 
	big_ima->sy			= BIG_SHORT(ima->sy); 
	big_ima->ex			= BIG_SHORT(ima->ex); 
	big_ima->ey			= BIG_SHORT(ima->ey); 
	big_ima->dw			= BIG_SHORT(ima->dw); 
	big_ima->dh			= BIG_SHORT(ima->dh);				
	big_ima->selectable	= BIG_SHORT(ima->selectable); 
	big_ima->selected	= BIG_SHORT(ima->selected);		
	big_ima->orgx		= BIG_SHORT(ima->orgx); 
	big_ima->orgy		= BIG_SHORT(ima->orgy); 
	big_ima->orgd		= BIG_SHORT(ima->orgd); 
	big_ima->anim		= BIG_SHORT(ima->anim);
	
	big_ima->mtime		= BIG_LONG(ima->mtime); 
	big_ima->disksize	= BIG_LONG(ima->disksize); 				
	big_ima->ibuf_type	= BIG_LONG(ima->ibuf_type); 

	memcpy(big_ima->pict_rect, ima->pict_rect, 3968);
	memcpy(big_ima->file_name, ima->file_name, 64);
	
	wr = write(file, (char*)big_ima, 4096);

	freeN(big_ima);
	close(file);
}

void write_new_pib(SpaceImaSel *simasel)
{
	OneSelectableIma *ima;
	char name[256];
	int dl;
	
	strcpy(name, simasel->dir);
	strcat(name, ".Bpib");
	dl = remove(name);
	
	ima = simasel->first_sel_ima;
	while (ima){
		append_pib(simasel, ima);
		ima = ima->next;
	}
}

void free_ima_dir(ImaDir *firstdir)
{
	ImaDir *n;
	
	while(firstdir){
		n = firstdir->next;
		freeN(firstdir);
		firstdir = n;
	}
}

void free_sel_ima(OneSelectableIma *firstima)
{
	OneSelectableIma *n;
	
	while(firstima){
		
		if (firstima->pict) {
			freeImBuf(firstima->pict);
		}
		n = firstima->next;
		freeN(firstima);
		firstima = n;
	}
}

check_for_pib(SpaceImaSel *simasel)
{
	ImaDir  *direntry;
	
	direntry = simasel->firstfile;
	while(direntry){
		if ((strlen(direntry->name) > 4) && (0==strcmp(direntry->name, ".Bpib")) ){
			simasel->fase |= IMS_FOUND_BIP;
			direntry = 0;
		}else{
			direntry = direntry->next;
		}
	}
}

void clear_ima_dir(SpaceImaSel *simasel)
{
	if(simasel->first_sel_ima)	free_sel_ima(simasel->first_sel_ima);
	if(simasel->firstdir)		free_ima_dir(simasel->firstdir);
	if(simasel->firstfile) 		free_ima_dir(simasel->firstfile);

	simasel->first_sel_ima	=  0;
	simasel->firstdir		=  0;
	simasel->firstfile		=  0;
	
	simasel->totaldirs		=  0;
	simasel->totalfiles		=  0;
	simasel->totalima		=  0;
	simasel->topdir			= -1;
	simasel->topfile		= -1;
	simasel->topima			=  0;
	simasel->image_slider	=  0.0;
	simasel->slider_height	=  0.0;
	simasel->slider_space	=  0.0;
	simasel->hilite			= -1;
	simasel->curimax		=  0;
	simasel->curimay		=  0;
	
	simasel->total_selected =  0;
	simasel->fase			=  0;
	simasel->subfase		=  0;
	simasel->imafase		=  0;
	simasel->ima_redraw     =  0;
}

void check_ima_dir_name(char *dir)
{
	short a;
	char *start, *eind;

	
	if(dir[0]=='.') {	/* komt voor, o.a. bij FILE_MAIN */
		dir[0]= '/';
		dir[1]= 0;
		return;
	}
	
	while (start = strstr(dir, "/../")) {
		eind = start + strlen("/../") - 1;
		a = start-dir-1;
		while (a>0) {
			if (dir[a] == '/') break;
			a--;
		}
		strcpy(dir+a,eind);
	}

	while (start = strstr(dir,"/./")){
		eind = start + strlen("/./") - 1;
		strcpy(start,eind);
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

	strcat(dir, "/");
}

int get_ima_dir(char *dirname, int dtype, int *td, ImaDir **first)
{
	DIR *dirp;
	struct dirent *dep;
	struct ImaDir *temp;
	struct ImaDir *dnext, *fnext;
	struct stat status;

	char getdirname[256];
	int  i=0, tot=0; 
	long isdir;
	
	strcpy(getdirname, dirname);
	strcat(getdirname, "/.");
	
	dirp = opendir(getdirname);
	if (dirp == NULL) return (-1);

	waitcursor(1);

	while((dep = readdir(dirp)) != NULL){
		
		strcpy(getdirname, dirname);
		strcat(getdirname,dep->d_name);
		
		stat(getdirname, &status);
		isdir = S_ISDIR(status.st_mode);
		
		
		if ( ((dtype == IMS_DIR)  && isdir) || ((dtype == IMS_FILE)  && !isdir)){			
			/* yes, searching for this type */
			tot++;
			if (tot == 1){
				dnext   = callocN(sizeof(struct ImaDir), "get first");
				*first  = dnext;
				
				dnext->prev     = 0;
				dnext->next     = 0;
			}else{
				fnext		    = callocN(sizeof(struct ImaDir), "get nextdir");
				dnext->next     = fnext;
				
				temp  = dnext;
				dnext = fnext;
				
				dnext ->prev     = temp;
				dnext ->next     = 0;
			}
			
			dnext->type     = dtype;
			dnext->selected = 0;
			dnext->hilite   = 0;
			
			dnext->mtime    = status.st_ctime;
			dnext->size     = (int)status.st_size;
			strcpy(dnext->name, dep->d_name);
			for(i=0; i<strlen(dnext->name); i++){ 
				if (dnext->name[i] == ' ') dnext->name[i] = '_'; 
			}
			
		}
	}
	closedir(dirp);
	
	if (tot) issort(tot, first);
	
	waitcursor(0);
	
	*td = tot;
	return (tot);
}

void imadir_parent(SpaceImaSel *simasel)
{
	if (strlen(simasel->dir) > 1){
		simasel->dir[strlen(simasel->dir)-1] = 0;
		while(simasel->dir[strlen(simasel->dir)-1] != '/'){
			simasel->dir[strlen(simasel->dir)-1] = 0;	
		}
	}
}


void get_next_image(SpaceImaSel *simasel)
{
	OneSelectableIma * ima;
	ImBuf            * ibuf;
	struct anim      * anim;
	int     i = 0, size;
	char    str[256], name[256];
	
	ima = simasel->first_sel_ima;
	if (ima == 0){
		simasel->imafase = 0;
		simasel->fase |=  IMS_KNOW_IMA;
		simasel->fase &= ~IMS_DOTHE_IMA;
		return;	
	}
	if (simasel->imafase > simasel->totalima){
		simasel->imafase = 0;
		simasel->fase &= ~IMS_DOTHE_IMA;
		simasel->fase |=  IMS_KNOW_IMA;
	}
	
	ima = simasel->first_sel_ima;
	i = 0;
	while(i < simasel->imafase){
		if ((ima) && (ima->next)) ima = ima->next;
		i++;
	}
	
	if (ima->image == 0) {
		if (ima->anim == 1) {
			/* open movie, get len, get middle picture */
			
			/* anim = open_anim(name, ANIM_MOVIE | ANIM_ANIM5 | ANIM_ONCE, IB_planes); */
			
			strcpy(name, simasel->dir);
			strcat(name, ima->file_name);
			anim = open_anim(name, ANIM_DFLT, IB_rect);
			
			if (anim == 0) {
				sprintf(str, "%s/cmovie.tga", getenv("BLENDERDIR"));
				ibuf = loadiffname(str, IB_rect);
			}else{
				ibuf = anim_nextpic(anim);
				freeImBuf(ibuf);
				
				ibuf = anim_absolute(anim, anim->duration / 2);
				
				free_anim(anim);
			}
		}else{
			strcpy(name, simasel->dir);
			strcat(name, ima->file_name);
			
			ibuf = loadiffname(name, IB_rect);			
		}
		
		if (ibuf){
			if (ima->dw < 4) ima->dw = 4;
			if (ima->dh < 4) ima->dh = 4;
			
			scaleImBuf(ibuf, ima->dw, ima->dh);
			
			ibuf->mincol =   0;
			ibuf->maxcol = 256;
			ibuf->cbits  =   5;
			ibuf->depth  =   8;
			setdither('F');
			
			freecmapImBuf(ibuf);
			ibuf->cmap = simasel->cmap->cmap;
			
			converttocmap(ibuf);
			
			/* copy ibuf->rect naar ima->pict_rect */ 
			size = ima->dw * ima->dh; if (size > 3968) size = 3968;
			longtochar(ima->pict_rect, ibuf->rect, size); 
			
			applycmap(ibuf);
			
			if (ima->pict) freeImBuf(ima->pict);
			ima->pict = ibuf;
			ibuf = 0;
			ima->cmap  = 1;
			ima->image = 1;
			
			append_pib(simasel, ima);
		}
	}
	simasel->ima_redraw++;
	simasel->imafase ++;
	if (simasel->imafase == simasel->totalima){
		simasel->imafase = 0;
		simasel->fase &= ~IMS_DOTHE_IMA;
		simasel->fase |= IMS_KNOW_IMA;
	}
}

void add_ima(int who, SpaceImaSel *simasel, ImaDir *direntry)
{
	OneSelectableIma *ima, *prev_ima;
	ImBuf   *ibuf;
	char    name[256];
	int     i = 0;
	
	strcpy(name ,  simasel->dir);
	strcat(name ,  direntry->name);
	
	prev_ima = simasel->first_sel_ima;
	i = 0;
	while((prev_ima)&&(prev_ima->next)){
		prev_ima = prev_ima->next;
	}
	
	ima = callocN(sizeof(OneSelectableIma), "OSIbip");
	if (direntry->type == IMS_IMA){
		/* Picture is an Image */
		
		ibuf = loadiffname(name, IB_test);
		
		if (ibuf){
			ima->anim	  = 0;
			ima->pict     = ibuf;
			ima->ibuf_type= ibuf->ftype;
			ima->orgx     = ibuf->x;
			ima->orgy     = ibuf->y;
			ima->orgd     = ibuf->depth;
			
			ima->dw    = 64;
			ima->dh    = 51;
			ima->cmap  =  0;
			ima->image =  0;  
			if (ima->orgx > ima->orgy){
				ima->dw = 64;
				ima->dh = 62 * ((float)ima->orgy / (float)ima->orgx);
			}else{
				ima->dw = 64 * ((float)ima->orgx / (float)ima->orgy);
				ima->dh = 62;
			}
		}else{
			printf("%s image with no imbuf ???\n", name);
		}
		ibuf = 0;	
	}else{
		/* Picture is an Animation */
	
		ima->pict     =  0;
		ima->anim	  =  1;
		ima->ibuf_type=  0;
		ima->orgx	  = 64;
		ima->orgy     = 51;
		ima->orgd     = 24;
		
		ima->dw    = 64;
		ima->dh    = 51;
		ima->cmap  =  0;
		ima->image =  0;  
	}
		
	strcpy(name, direntry->name); name[31] = 0;
	strcpy(ima->file_name, name);
	ima->disksize = (long)direntry->size;
	ima->mtime    = (long)direntry->mtime;
	
	ima->next = 0;
	ima->prev = prev_ima;

	if (prev_ima)	{	
		prev_ima->next = ima;
	}else{   
		simasel->first_sel_ima =  ima;	
	}
	
	simasel->ima_redraw++;
	simasel->totalima++;
}


void get_file_info(SpaceImaSel *simasel)
{
	OneSelectableIma *ima, *prev_ima;
	ImaDir  *direntry;
	char    name[256];
	int     i = 0;
	
	if (!simasel->firstfile){
		simasel->subfase = 0;
		simasel->fase |= IMS_KNOW_INF;
		simasel->fase &= ~IMS_DOTHE_INF;
		return;	
	}
	if (simasel->subfase > simasel->totalfiles){
		simasel->subfase = 0;
		simasel->fase |= IMS_KNOW_INF;
		simasel->fase &= ~IMS_DOTHE_INF;
	}

	direntry = simasel->firstfile;
	while(i < simasel->subfase){
		direntry = direntry->next;
		i++;
	}
	
	prev_ima = simasel->first_sel_ima;
	while((prev_ima)&&(prev_ima->next)){
		prev_ima = prev_ima->next;
	}
	
	strcpy(name ,  simasel->dir);
	strcat(name ,  direntry->name);
	
	if (ispic(name)) {
		direntry->type = IMS_IMA;
	}else{
		if (isanim(name)) {
			direntry->type = IMS_ANIM;
		}else{
			direntry->type = IMS_NOIMA;
		}
	}
	
	if (direntry->type != IMS_NOIMA){	
		add_ima(1, simasel, direntry);
	}
	
	simasel->subfase++;
	
	if (simasel->subfase == simasel->totalfiles){
		simasel->subfase = 0;
		simasel->fase |= IMS_KNOW_INF;
		simasel->fase &= ~IMS_DOTHE_INF;	
	}
}

void get_pib_file(SpaceImaSel *simasel)
{
	ImaDir           *direntry, *prev_dir, *next_dir;
	OneSelectableIma *ima, *prev_ima;
	ImBuf            *ibuf;
	long flen;
	int  i, dl, file, first, trd=0, rd, size, found, ima_added = 0;
	char name[256], dname[256], iname[256]; 
	
	if (bitset(simasel->fase , IMS_KNOW_BIP)) return;
	
	strcpy(name,  simasel->dir);
	strcat(name,  ".Bpib");
		
	file = open(name, O_RDONLY);
	
	flen = lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);

	simasel->totalima = 0;
	prev_ima = 0;
	first = 1;
	trd = 0;
	
	while(trd < flen){
		ima = mallocN(sizeof(OneSelectableIma), "Ima");
		
		rd = read(file, ima, 4096);
		found = 0;

		if (rd != 4096)	{ 
			printf("Error in Bpib file\n");
			strcpy(name, simasel->dir);
			strcat(name, ".Bpib");
			dl = remove(name);
			if (dl == 0) printf("corrupt Bpib file removed\n");
			trd = flen;
		}else{
		
			ima->cmap		= BIG_SHORT(ima->cmap);
			ima->image		= BIG_SHORT(ima->image);
			ima->draw_me	= BIG_SHORT(ima->draw_me);
			ima->rt			= BIG_SHORT(ima->rt);
			ima->sx			= BIG_SHORT(ima->sx); 
			ima->sy			= BIG_SHORT(ima->sy); 
			ima->ex			= BIG_SHORT(ima->ex); 
			ima->ey			= BIG_SHORT(ima->ey); 
			ima->dw			= BIG_SHORT(ima->dw); 
			ima->dh			= BIG_SHORT(ima->dh);				
			ima->selectable	= BIG_SHORT(ima->selectable); 
			ima->selected	= BIG_SHORT(ima->selected);		
			ima->orgx		= BIG_SHORT(ima->orgx); 
			ima->orgy		= BIG_SHORT(ima->orgy); 
			ima->orgd		= BIG_SHORT(ima->orgd); 
			ima->anim		= BIG_SHORT(ima->anim);
	
			ima->mtime		= BIG_LONG(ima->mtime); 
			ima->disksize	= BIG_LONG(ima->disksize); 				
			ima->ibuf_type	= BIG_LONG(ima->ibuf_type); 
			
			direntry = simasel->firstfile;
			while (direntry){
				if (MAKE_ID('B', 'I', 'P', '2') == ima->header){
					if (0 == strcmp(direntry->name, ima->file_name)){
						if (direntry->mtime == ima->mtime){
							
							/*  ima found and same > remove direntry */
							prev_dir = direntry->prev; 
							next_dir = direntry->next;
							
							if(prev_dir) prev_dir->next = next_dir; 
							if(next_dir) next_dir->prev = prev_dir;
							
							freeN(direntry);direntry = 0;
						
							size = ima->dw * ima->dh; if (size > 3968) size = 3968;
							if(size) {
								ima->pict = allocImBuf(ima->dw, ima->dh, 24, IB_rect | IB_cmap, 0);
								chartolong(ima->pict->rect, ima->pict_rect, size);
								ima->pict->cmap = simasel->cmap->cmap;
								applycmap(ima->pict);
							}
							ima->selected   = 0;
							ima->selectable = 0;
							
							if(prev_ima) prev_ima->next = ima;
							ima->next      = 0;
							ima->prev      = prev_ima;
							
							prev_ima = ima;
				
							if (first){ first = 0;simasel->first_sel_ima = ima; }
							simasel->totalima++;
							found = 1;
						}
					}
				}
				if(direntry) direntry = direntry->next;
			}
		}
		if (!found) freeN(ima);
		
		trd+=rd;
	}
	close(file);
	
	direntry = simasel->firstfile;
	
	while(direntry){
		
		strcpy(name ,  simasel->dir);
		strcat(name ,  direntry->name);
		
		if (ispic(name)) {
			direntry->type = IMS_IMA;
		}else{
			if (isanim(name)) {
				direntry->type = IMS_ANIM;
			}else{
				direntry->type = IMS_NOIMA;
			}
		}
		
		if (direntry->type != IMS_NOIMA){
			prev_ima = simasel->first_sel_ima;
			while((prev_ima)&&(prev_ima->next)){
				prev_ima = prev_ima->next;
			}
			add_ima(2, simasel, direntry);
			ima_added = 1;
		}
		direntry = direntry->next;
	}
	
	imsort(&simasel->first_sel_ima);
	
	simasel->fase |= IMS_KNOW_BIP;
	simasel->fase |= IMS_KNOW_INF;
	simasel->fase |= IMS_KNOW_IMA;
	
	if (ima_added){
		simasel->fase |= IMS_DOTHE_IMA;
		simasel->fase &= ~IMS_KNOW_IMA;
		addafterqueue(curarea->win, AFTERIMASELGET, 1);
	}else{
		write_new_pib(simasel);
	}		
}

void change_imadir(SpaceImaSel *simasel)
{
	ImaDir  *direntry;
	int i;
	char name[256];
	
	direntry = simasel->firstdir; 
	for (i=0; i<simasel->hilite; i++){
		direntry = direntry->next;	
	}
	
	if (direntry->name[0] != '.'){
		strcat(simasel->dir, direntry->name);
		strcat(simasel->dir, "/");
	}else{
		if (direntry->name[1] == '.'){
			imadir_parent(simasel);	
		}
	}
	clear_ima_dir(simasel);
}

void init_imaselspace(ScrArea *sa)
{
	SpaceImaSel *simasel;
	char str[256];
	
	simasel= callocN(sizeof(SpaceImaSel), "initimaselspace");
	addhead(&sa->spacedata, simasel);

	set_func_space(sa);

	simasel->spacetype= SPACE_IMASEL;
	
	simasel->mode = 7;
	strcpy (simasel->dir,  U.textudir);	/* TON */
	strcpy (simasel->file, "");

	simasel->first_sel_ima	=  0;
	simasel->hilite_ima	    =  0;
	simasel->firstdir		=  0;
	simasel->firstfile		=  0;
	simasel->cmap           =  0;
	simasel->returnfunc     =  0;
	
	simasel->title[0]       =  0;
	
	clear_ima_dir(simasel);
	
	sprintf(str, "%s/cmap.tga", getenv("BLENDERDIR"));
	simasel->cmap = loadiffname(str, IB_rect | IB_cmap);

	if (!simasel->cmap) {
		error("in console");
		printf("Image select cmap file not found \n");
	}
}

void check_imasel_copy(SpaceImaSel *simasel)
{
	char str[256];

	/* LET OP: wordt ook gebruikt bij inlezen blender file */
	/* dingen op nul zetten, opnieuw malloccen etc */
	simasel->first_sel_ima	=  0;
	simasel->hilite_ima	    =  0;
	simasel->firstdir		=  0;
	simasel->firstfile		=  0;
	simasel->cmap           =  0;
	clear_ima_dir(simasel);
	
	sprintf(str, "%s/cmap.tga", getenv("BLENDERDIR"));
	simasel->cmap = loadiffname(str, IB_rect | IB_cmap);
}

void free_imasel(SpaceImaSel *simasel)
{
	/* NIET de imasel zelf vrijgeven */
	
	clear_ima_dir(simasel);
	freeImBuf(simasel->cmap);
}


