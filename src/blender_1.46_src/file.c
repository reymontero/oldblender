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

/*  file.c   juni 94     MIXED MODEL

 * 
 *	FILEFORMAAT: IFF-achtige structuur  (niet meer IFF compatible!)
 
	start file:
		BLENDER_V100	12 bytes  (versie 1.00)
		
	datablokken:		zie ook struct BHead
		<bh.code>			4 chars
		<bh.len>			int,  len data achter BHead
		<bh.old>			void,  oude pointer
		<bh.SDNAnr>			int
		<bh.nr>				int, bij array: aantal structs

		data			
		...
		...
	
	Vrijwel alle data in blender zijn structs. Elke struct krijgt een BHead header mee.
	Met BHead kan de struktuur weer worden gelinkt en wordt met StructDNA vergeleken.
	
	SCHRIJVEN
	
	Voorkeur volgorde schrijven: (waarschijnlijk mag ook door elkaar, maar waarom zou je? )
	In ieder geval MOET indirekte data ACHTER LibBlock
	
	(Locale data)
	- voor elk LibBlock
		- schrijf LibBlock
		- schrijf bijhorende direkte data
	(Externe data)
	- per library
		- schrijf library block
		- per LibBlock
			- schrijf ID LibBlock
	- schrijf FileGlobal (een selectie uit globale data )
	- schrijf SDNA
	- schrijf USER als aangegeven (~/.B.blend)
		
	LEZEN
	
	- Bestaande Library (Main) pushen of vrijgeven
	- Nieuwe Main alloceren
	- file inlezen
	- lees SDNA
	- per LibBlock
		- lees LibBlock
		- als Library
			- nieuwe Main maken
			- ID's eraan hangen
		- else 
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
	- lees FileGlobal
	- lees USER data, als aangegeven (~/.B.blend)
	- file vrijgeven
	- per Library met Scene (per Main)
		- file inlezen
		- lees SDNA
		- alle LibBlocks uit Scene opzoeken en ID's aan Main hagen
			- als extern LibBlock
				- zoek Main's af
					- is al ingelezen:
					- nog niet ingelezen
					- of nieuwe Main maken
		- per LibBlock
			- recursief dieper lezen
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
		- file vrijgeven
	- per Library met nog niet gelezen LibBlocks
		- file inlezen
		- lees SDNA
		- per LibBlock
			- recursief dieper lezen
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
		- file vrijgeven
	- alle Main's samenvoegen
	- alle LibBlocks linken en indirekte pointers naar libblocks
	- FileGlobal goedzetten en pointers naar Global kopieeren
	
 *
 *
 *
 */



#include "blender.h"
#include "file.h"
#include "screen.h"
#include <local/storage.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>


typedef struct BHead {
	int code, len;
	void *old;
	int SDNAnr, nr;
} BHead;

typedef struct OldNew {
	void *old, *new;
	int nr;
} OldNew;

extern char *compflags;		/* genfile.c */
extern int compare();		/* storage.c */

OldNew *datablocks=0;
OldNew *libblocks=0;
int datacount= 0, maxdatacount=1024;
int libcount= 0, maxlibcount=1024;





/* *******  MYWRITE ********* */

char *writebuf;
long mywcount, mywfile;

void mywrite(adr, len)
long *adr, len;
{

	if(len<=0) return;
	if(len>50000) {
		if(mywcount) {
			write(mywfile, writebuf, mywcount);
			mywcount= 0;
		}
		write(mywfile, adr, len);
		return;
	}
	if(len+mywcount>99999) {
		write(mywfile, writebuf, mywcount);
		mywcount= 0;
	}
	memcpy(writebuf+mywcount, adr, len);
	mywcount+= len;
}

void bgnwrite(file)
long file;
{
	mywfile= file;
	writebuf= (char *)mallocN(100000,"bgnwrite");
	mywcount= 0;
}

void endwrite()
{
	if(mywcount) {
		write(mywfile, writebuf, mywcount);
	}
	freeN(writebuf);

}

/* ********  END MYWRITE ******** */
/* ********  DIV ******** */

void writeBlog()
{
	long file;
	char name[100], *home;

	home = getenv("HOME");
	if (home) {
		strcpy(name, home);
		strcat(name, "/.Blog");
		file= open(name, O_WRONLY | O_CREAT | O_TRUNC);
		if (file >= 0){
			fchmod(file, 0664);
			write(file, G.sce, strlen(G.sce));
			close(file);
		}
	}
}

void readBlog()
{
	long file,len;
	char name[100],*home;

	home = getenv("HOME");
	if (home) {
		strcpy(name, home);
		strcat(name, "/.Blog");
		file= open(name, O_RDONLY);
		if (file >= 0){
			len = read(file, G.sce, sizeof(G.sce));
			close(file);
			if (len > 0) G.sce[len] = 0;
		}
	}
}

int testextensie(char *str, char *ext)
{
	short a, b;

	a= strlen(str);
	b= strlen(ext);
	if(a==0 || b==0 || b>=a) return 0;
	while(b>0) {
		a--;
		b--;
		if(str[a]!=ext[b]) return 0;
	}
	return 1;
}

void convertstringcode(str)
char *str;
{
	char *slash, temp[100];

	if (str[0] == '/' && str[1] == '/') {
		strcpy(temp, G.sce);
		slash = strrchr(temp, '/');
		if (slash) {
			strcpy(slash+1, str+2);
			strcpy(str, temp);
		}
	}
}


void splitdirstring(char *di,char *fi)
{
	short a, b=0;

	a= strlen(di)-1;
	while(di[a]!='/' && (a>=0)) a--;

	a++;
	while(di[a]!=0) {
		fi[b++]= di[a];
		di[a]= 0;
		a++;
	}
	fi[b]= 0;
}

void add_data_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 1) doorgeven herinitialiseert en geeft ongebruikte blokken vrij
 * (0, 0) doorgeven geeft alles vrij 
 */
{
	OldNew *temp;
	int a;
	
	if(old==0) {	/* ongebruikte vrijgeven */
		temp= datablocks;
		for(a=0; a<datacount; a++, temp++) {
			if(temp->nr==0) freeN(temp->new);
		}
		if(new==0 && datablocks) {
			freeN(datablocks);
			datablocks= 0;
			maxdatacount= 1024;
		}
		datacount= 0;
	}
	else {
		if(datablocks==0) {
			datablocks= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
		}
		else if(datacount==maxdatacount) {
			maxdatacount*= 2;
			temp= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
			memcpy(temp, datablocks, maxdatacount*sizeof(OldNew)/2);
			freeN(datablocks);
			datablocks= temp;
		}
		
		temp= datablocks+datacount;
		temp->old= old;
		temp->new= new;
		temp->nr= 0;
		
		datacount++;
	}
}

void add_lib_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 0) doorgeven geeft alles vrij
 * Zet aantal users al op 1!!!
 */
{
	OldNew *temp;
	int a;
	
	if(old==0) {	/* alles vrijgeven */
		freeN(libblocks);
		libblocks= 0;
		maxlibcount= 1024;
		libcount= 0;
	}
	else {
		if(libblocks==0) {
			libblocks= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
		}
		else if(libcount==maxlibcount) {
			maxlibcount*= 2;
			temp= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
			memcpy(temp, libblocks, maxlibcount*sizeof(OldNew)/2);
			freeN(libblocks);
			libblocks= temp;
		}
		
		temp= libblocks+libcount;
		temp->old= old;
		temp->new= new;
		temp->nr= 1;
		
		libcount++;
	}
}

void *newadr(void *adr)		/* alleen direkte datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		if(lastone<datacount-1) {
			lastone++;
			onew= datablocks+lastone;
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
		}
		
		lastone= 0;
		onew= datablocks;
		while(lastone<datacount) {
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
			onew++;
			lastone++;
		}
	}
	
	return 0;
}

void *newlibadr(void *adr)		/* alleen Lib datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		if(lastone<libcount-1) {
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
			onew++;
			lastone++;
		}
	}
	
	return 0;
}



/* ********** END DIV ****************** */
/* ********** READ FILE ****************** */

void read_struct(BHead *bh)
{
	void *temp= 0;

	if(bh->len) {
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			temp= mallocN(bh->nr*bh->len, "read_struct");
			memcpy(temp, (bh+1), bh->nr*bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, &temp);
		}
	}
	
	add_data_adr(bh->old, temp);
}

void *read_libstruct(BHead *bh)
{
	void *temp= 0;

	if(bh->len) {
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			temp= mallocN(bh->nr*bh->len, "read_libstruct");
			memcpy(temp, (bh+1), bh->nr*bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, &temp);
		}
	}
	
	add_lib_adr(bh->old, temp);
	
	return temp;
}

void read_struct_expl(bh, data)	
BHead *bh;
void **data;	/* dubbele pointer ivm reconstruct */
{

	if(bh->len) {
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			*data= mallocN(bh->nr*bh->len, "read_struct_expl");
			memcpy(*data, (bh+1), bh->nr*bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, data);
		}
	}
}

void link_list(ListBase *lb)		/* alleen direkte data */
{
	Link *ln, *prev;
	
	if(lb->first==0) return;

	lb->first= newadr(lb->first);
	lb->last= newadr(lb->last);
	
	ln= lb->first;
	prev= 0;
	while(ln) {
		
		ln->next= newadr(ln->next);
		ln->prev= prev;
		
		prev= ln;
		ln= ln->next;
	}
}

/* ************ READ ALG ***************** */

void lib_link_ALG()
{
	
}

void direct_link_ALG(main, sc)
Main *main;
Screen *sc;
{

}

/* ************ READ OBJECT ***************** */

void lib_link_object(Main *main)
{
	Object *ob;
	int a;
	
	ob= main->object.first;
	while(ob) {
		if(ob->id.flag & LIB_NEEDLINK) {
			ob->parent= newlibadr(ob->parent);
			ob->track= newlibadr(ob->track);
			ob->ipo= newlibadr(ob->ipo);
			ob->data= newlibadr(ob->data);
			if(ob->data==0) ob->type= OB_EMPTY;
			
			for(a=0; a<ob->totcol; a++) ob->mat[a]= newlibadr(ob->mat[a]);
			
			ob->id.flag -= LIB_NEEDLINK;
		}
		ob= ob->id.next;
	}
}

void direct_link_object(main, ob)
Main *main;
Object *ob;
{
	
	ob->mat= newadr(ob->mat);
}

/* ************ READ SCENE ***************** */

void lib_link_scene(Main *main)
{
	Scene *sce;
	Base *base;
	
	sce= main->scene.first;
	while(sce) {
		if(sce->id.flag & LIB_NEEDLINK) {
			sce->camera= newlibadr(sce->camera);
			sce->world= newlibadr(sce->world);
			sce->set= newlibadr(sce->set);
			sce->ima= newlibadr(sce->ima);
			
			base= sce->base.first;
			while(base) {
				base->object= newlibadr(base->object);
				base= base->next;
			}
			sce->id.flag -= LIB_NEEDLINK;
		}
		sce= sce->id.next;
	}
}

void direct_link_scene(main, sce)
Main *main;
Scene *sce;
{

	link_list( &(sce->base) );

	sce->basact= newadr(sce->basact);
	
}

/* ************ READ SCREEN ***************** */

void lib_link_screen(Main *main)
{
	Screen *sc;
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	View2D *v2d;
	SpaceIpo *ipo;
	SpaceButs *buts;
	SpaceFile *sfile;

	sc= main->screen.first;
	while(sc) {
		if(sc->id.flag & LIB_NEEDLINK) {
			sc->scene= newlibadr(sc->scene);
			
			sa= sc->areabase.first;
			while(sa) {
				
				sa->full= newlibadr(sa->full);
	
				v3d= sa->spacedata.first;	/* v3d als voorbeeld */
				while(v3d) {
				
					if(v3d->spacetype==SPACE_VIEW3D) {
						v3d->camera= newlibadr(v3d->camera);
						
					}
					else if(v3d->spacetype==SPACE_IPO) {
						ipo= (SpaceIpo *)v3d;
	
					}
					else if(v3d->spacetype==SPACE_BUTS) {
						buts= (SpaceButs *)v3d;
	
					}
					else if(v3d->spacetype==SPACE_FILE) {
						sfile= (SpaceFile *)v3d;
						
						sfile->filelist= 0;
						sfile->returnfunc= 0;
					}
					v3d= v3d->next;
				}
				sa= sa->next;
			}
			sc->id.flag -= LIB_NEEDLINK;
		}
		sc= sc->id.next;
	}
}

void direct_link_screen(main, sc)
Main *main;
Screen *sc;
{
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;

	link_list( &(sc->vertbase) );
	link_list( &(sc->edgebase) );
	link_list( &(sc->areabase) );
	sc->winakt= 0;
	sc->qual= 0;
	
	/* vertices en offset */
	test_scale_screen(sc);

	/* edges */
	se= sc->edgebase.first;
	while(se) {
		se->v1= newadr(se->v1);
		se->v2= newadr(se->v2);
		if( (long)se->v1 > (long)se->v2) {
			sv= se->v1;
			se->v1= se->v2;
			se->v2= sv;
		}
		se= se->next;
	}

	/* areas */
	sa= sc->areabase.first;
	while(sa) {
		link_list( &(sa->spacedata) );

		sa->v1= newadr(sa->v1);
		sa->v2= newadr(sa->v2);
		sa->v3= newadr(sa->v3);
		sa->v4= newadr(sa->v4);
		
		sa->win= sa->headwin= 0;
		sa->headqueue= sa->hq= sa->winqueue= sa->wq= 0;

		set_func_space(sa);	/* space.c */
		
		sa= sa->next;
	}
} 

/* ************** ALG ******************** */

int read_libblock(Main *main, BHead *bhead, int flag)
{
	/* deze routine leest libblock en direkte data. Met linkfunkties
	 * alles aan elkaar hangen.
	 */
	
	ID *id;
	ListBase *lb;
	int skipdata;
	char *fd;

	lb= wich_libbase(main, bhead->code);
	if(lb==0) {
		return bhead->len+sizeof(BHead);
	}
	
	fd= (char *)bhead;
	
	/* libblock inlezen */
	id= read_libstruct(bhead);
	addtail(lb, id);
	id->flag= flag+LIB_NEEDLINK;
	id->lib= main->curlib;
	
	skipdata= bhead->len+sizeof(BHead);
	fd+= bhead->len+sizeof(BHead);
	bhead= (BHead *)fd;
	
	/* alle data inlezen */
	while(bhead->code==DATA) {
		
		read_struct(bhead);
		
		skipdata+= bhead->len+sizeof(BHead);
		fd+= bhead->len+sizeof(BHead);
		bhead= (BHead *)fd;		
	}

	/* pointers directe data goedzetten */
	switch( GS(id->name) ) {
		case ID_SCR:
			direct_link_screen(main, id);
			break;
		case ID_SCE:
			direct_link_scene(main, id);
			break;
		case ID_OB:
			direct_link_object(main, id);
			break;
	}
	
	/* vrijgeven, herinitialiseren */
	add_data_adr(0, (void *)1);	
	
	return skipdata;
}

void link_global(FileGlobal *fg)
{

	G.curscreen= newlibadr(fg->curscreen);
	G.scene= G.curscreen->scene;
	G.obedit= 0;
	
	
}

void lib_link_all(Main *main)
{
	lib_link_screen(main);
	lib_link_scene(main);
	lib_link_object(main);
}


void read_file_dna(char *filedata)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	freestructDNA(&old_sdna);

	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code==DNA1) {

			old_sdna.data= mallocN(bhead->len, "sdna");
			memcpy(old_sdna.data, fd+sizeof(BHead), bhead->len);
			afbreek= 1;
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
	init_structDNA(&old_sdna);
	set_compareflags_structDNA();
	
}


void read_file(char *dir)
{
	BHead *bhead;
	FileGlobal *fg;
	int ok, len, file, filelen, filelent, skipdata, temp, temp1, version;
	short curs;
	char *cp, di[100], fi[100], *home, astr[100];
	char *filedata, *fd;
	
	/* appenden: main wordt gepusht en later met nieuwe main samengevoegd,
	 * anders: main vrijgeven.
	 */

	len=strlen(dir);
	if(len==0) return;
	if(dir[len-1]=='/') return;

	strcpy(di,dir);
	splitdirstring(di, fi);

	/* fileformaattest doen */

	if(testextensie(fi,".blend")) {
		file= open(dir,O_RDONLY);
		if(file<=0) {
			error("Can't open file");
			return;
		}
		
		filelen= lseek(file, 0, 2);	/* seek end */
		lseek(file, 0, 0);		/* en weer terug */

		read(file, &temp, 4);
		read(file, &temp1, 4);
		if(temp!=BLEN || temp1!=DER_) {
			close(file);
			error("Idiot file!");
			return;
		}

		strcpy(G.sce, dir);
		read(file, &version, 4);
		
		curs= getcursorN();
		if(curs!=2) setcursorN(2);	/* alleen als er een screen is */

		/* er is maar 1 Main, dus alleen inhoud vrijgeven */

		free_main(0, G.main);
		
		/* hele file inlezen */
		filelen-= 12;
		filelent= filelen;
		filedata= mallocN(filelen, "filedata");
		read(file, filedata, filelen);
		close(file);

		/* eerst op zoek naar SDNA */
		read_file_dna(filedata);

		/* alle data inlezen: */
		filelen= filelent;
		fd= filedata;
		ok= 0;

		while(filelen>0) {
			
			bhead= (BHead *)fd;
			
			switch(bhead->code) {
			case GLOB:
				read_struct_expl(bhead, &fg);
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DATA:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DNA1:
				skipdata= bhead->len+sizeof(BHead);
			case USER:
				skipdata= bhead->len+sizeof(BHead);
			case TEST:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case ENDB:
				ok= 1;
				break;
			default:
				skipdata= read_libblock(G.main, bhead, LIB_LOCAL);
			}
			
			fd+= skipdata;
			filelen-= skipdata;
		}

		freeN(filedata);

		if(ok==0) {
			error("Warning: file not complete");
		}

		/* LibData linken */
		
		lib_link_all(G.main);
		link_global(fg);	/* als laatste */

		/* losslingerende blokken vrijgeven */
		add_data_adr(0, 0);
		add_lib_adr(0, 0);
		
		freeN(fg);
		
		setscreen(G.curscreen);
		if(curs!=2) setcursorN(curs);
	}
}

/* ************* READ LIBRARY ************** */

int libdir[100];

int groupname_to_code(char *group)
{

	if(strcmp(group, "Scene")==0) return ID_SCE;
	if(strcmp(group, "Object")==0) return ID_OB;
	if(strcmp(group, "Mesh")==0) return ID_ME;
	if(strcmp(group, "Curve")==0) return ID_CU;
	if(strcmp(group, "Metaball")==0) return ID_MB;
	if(strcmp(group, "Material")==0) return ID_MA;
	if(strcmp(group, "Texture")==0) return ID_TE;
	if(strcmp(group, "Ipo")==0) return ID_IP;
	if(strcmp(group, "World")==0) return ID_WO;
	return 0;	
}

int count_libfiles(SpaceFile *sfile, int idcode)
{
	BHead *bhead;
	int tot=0, afbreek=0;
	char *fd;
	
	fd= sfile->libfiledata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code==idcode) tot++;
		
		fd+= bhead->len+sizeof(BHead);
	}
	return tot;
}


int count_libdirs(SpaceFile *sfile)
{
	BHead *bhead;
	int tot=0, a, afbreek=0;
	char *fd;
	
	libdir[0]= 0;
	
	fd= sfile->libfiledata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		switch(bhead->code) {
		case GLOB:
		case DATA:
		case USER:
		case TEST:
		case DNA1:
		case ID_SCR:
		case ID_LA:
		case ID_CA:
			break;
		case ENDB:
			afbreek= 1;
			break;
		default:
			for(a=0; a<tot; a++) {
				if(libdir[a]==bhead->code) break;
			}
			if(a==tot) {
				libdir[tot]= bhead->code;
				tot++;
				if(tot>99) tot= 99;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
	return tot;
}

int is_a_library(SpaceFile *sfile, char *dir, char *group)
{
	/* return ok als een blenderfile, in dir staat de filenaam,
	 * in group het type libdata
	 */
	int len;
	char *fd;
	
	strcpy(dir, sfile->dir);
	len= strlen(dir);
	if(len<7) return 0;
	if( dir[len-1] != '/') return 0;
	
	group[0]= 0;
	dir[len-1]= 0;

	fd= strrchr(dir, '/');
	if(fd==0) return 0;
	*fd= 0;
	if(testextensie(fd+1, ".blend")) {
		*fd= '/';
	}
	else {
		strcpy(group, fd+1);
		fd= strrchr(dir, '/');
		if(fd==0 || testextensie(fd+1, ".blend")==0) return 0;
	}
	return 1;
}

void library_to_filelist(SpaceFile *sfile)
{
	BHead *bhead;
	ID *id;
	int file, filelen, temp, temp1, a, actual, ok, idcode;
	char dir[120], group[24], *fd, *str;

	/* naam testen */
	ok= is_a_library(sfile, dir, group);
	
	if(ok==0) {
		/* vrijgeven */
		if(sfile->libfiledata) freeN(sfile->libfiledata);
		sfile->libfiledata= 0;
		return;
	}
	
	/* en daar gaat ie */
	/* voorlopig alleen filedata inlezen als libfiledata==0 */
	if(sfile->libfiledata==0) {
	
		file= open(dir,O_RDONLY);
		if(file<=0) {
			error("Can't open library");
			return;
		}
			
		filelen= lseek(file, 0, 2);	/* seek end */
		lseek(file, 0, 0);		/* en weer terug */
	
		read(file, &temp, 4);
		read(file, &temp1, 4);
		if(temp!=BLEN || temp1!=DER_) {
			error("Idiot file!");
			close(file);
			return;
		}
		read(file, &temp, 4);
		
		filelen-= 12;
		sfile->libfiledata= mallocN(filelen, "filedata");
		read(file, sfile->libfiledata, filelen);
		close(file);
	}
	
	if(group[0]==0) {
		
		/* directories maken */
		sfile->totfile= count_libdirs(sfile);

		sfile->totfile+= 2;
		sfile->filelist= (struct direntry *)malloc(sfile->totfile * sizeof(struct direntry));
		actual= 2;
		
		for(a=0; a<sfile->totfile; a++) {
			memset( &(sfile->filelist[a]), 0 , sizeof(struct direntry));
			if(a==0) {
				sfile->filelist[a].relname= strdup(".");
				sfile->filelist[a].type |= S_IFDIR;
			}
			else if(a==1) {
				sfile->filelist[a].relname= strdup("..");
				sfile->filelist[a].type |= S_IFDIR;
			}
			else {
				switch( libdir[a-2] ) {
					case ID_SCE: str= "Scene"; break;
					case ID_OB: str= "Object"; break;
					case ID_ME: str= "Mesh"; break;
					case ID_CU: str= "Curve"; break;
					case ID_MB: str= "Metaball"; break;
					case ID_MA: str= "Material"; break;
					case ID_TE: str= "Texture"; break;
					case ID_IP: str= "Ipo"; break;
					case ID_WO: str= "World"; break;
					default: str=0; actual--;
						*( (short *)group)= libdir[a-2];
						group[3]= 0;
						printf("%s\n", group);
				}
				if(str) {
					sfile->filelist[actual].relname= strdup(str);
					sfile->filelist[actual].type |= S_IFDIR;
				}
				actual++;
			}
		}
		sfile->totfile= actual;
		qsort(sfile->filelist, actual, sizeof(struct direntry), compare);
	}
	else {

		/* files maken */
		idcode= groupname_to_code(group);
		sfile->totfile= count_libfiles(sfile, idcode);
		
		sfile->totfile+= 2;
		sfile->filelist= (struct direntry *)malloc(sfile->totfile * sizeof(struct direntry));
		
		memset( &(sfile->filelist[0]), 0 , sizeof(struct direntry));
		sfile->filelist[0].relname= strdup(".");
		sfile->filelist[0].type |= S_IFDIR;
		memset( &(sfile->filelist[1]), 0 , sizeof(struct direntry));
		sfile->filelist[1].relname= strdup("..");
		sfile->filelist[1].type |= S_IFDIR;
		
		actual= 2;
		fd= sfile->libfiledata;
		while(TRUE) {
			bhead= (BHead *)fd;
			
			if(bhead->code==ENDB) break;
			else if(bhead->code==idcode) {
				memset( &(sfile->filelist[actual]), 0 , sizeof(struct direntry));

				id= (ID *)(bhead+1);
				
				sfile->filelist[actual].relname= strdup(id->name+2);
				actual++;
			}
			
			fd+= bhead->len+sizeof(BHead);

		}
		
		qsort(sfile->filelist, sfile->totfile, sizeof(struct direntry), compare);
	}
}

void append_named_part(SpaceFile *sfile, Main *main, char *name, int idcode)
{
	BHead *bhead;
	ID *id;
	int afbreek=0;
	char *fd;
	
	fd= sfile->libfiledata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code==idcode) {
			id= (ID *)(bhead+1);
			if(strcmp(id->name+2, name)==0) {
				
				read_libblock(main, bhead, LIB_TESTEXT);
				
				afbreek= 1;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
	
}

BHead *find_bhead(void *old, char *filedata)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	if(old==0) return 0;
	
	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->old==old) return bhead;
		
		fd+= bhead->len+sizeof(BHead);
	}
	return 0;
}

int not_yet_read(Main *main, BHead *bhead)
{
	ListBase *lb;
	ID *idtest, *id;
	
	lb= wich_libbase(main, bhead->code);
	if(lb) {
		idtest= (ID *)(bhead+1);
		id= lb->first;
		while(id) {
			if( strcmp(id->name, idtest->name)==0 ) return 0;
			id= id->next;
		}
	}
	return 1;
}

void expand_test(Main *main, char *filedata, void *old)
{
	BHead *bhead;
	ID *id;
	
	bhead= find_bhead(old, filedata);
	if(bhead) {
		if(not_yet_read(main, bhead)) read_libblock(main, bhead, LIB_TESTIND);
	}
}

void expand_ob(Main *main, char *filedata, Object *ob)
{
	int a;
	
	expand_test(main, filedata, ob->data);
	for(a=0; a<ob->totcol; a++) {
		expand_test(main, filedata, ob->mat[a]);
	}
	
	ob->id.flag-= LIB_TEST;
}

void expand_sce(Main *main, char *filedata, Scene *sce)
{
	Base *base;
	
	base= sce->base.first;
	while(base) {
		expand_test(main, filedata, base->object);
		base= base->next;
	}
	expand_test(main, filedata, sce->camera);
	
	sce->id.flag-= LIB_TEST;
}

void expand_main(Main *main, char *filedata)
{
	Object *ob;
	Lamp *la;
	Camera *ca;
	Scene *sce;
	int doit= 1;
	
	while(doit) {
		doit= 0;
		
		ob= main->object.first;
		while(ob) {
			if(ob->id.flag & LIB_TEST) {
				expand_ob(main, filedata, ob);
				doit= 1;
			}
			ob= ob->id.next;
		}
		sce= main->scene.first;
		while(sce) {
			if(sce->id.flag & LIB_TEST) {
				expand_sce(main, filedata, sce);
				doit= 1;
			}
			sce= sce->id.next;
		}
	}
}

void library_append(SpaceFile *sfile)	/* append aan G.scene */
{
	Main main;
	int a, totsel=0, idcode;
	char dir[120], group[32];
	
	/* is er sprake van een library? */
	if( is_a_library(sfile, dir, group)==0 ) {
		error("Not a library");
		return;
	}
	if(sfile->libfiledata==0) {
		error("library not loaded");
		return;
	}
	if(group[0]==0) {
		error("Nothing indicated");
		return;
	}
	
	/* zijn er geselecteerde files? */
	for(a=0; a<sfile->totfile; a++) {
		if(sfile->filelist[a].flags & ACTIVE) {
			totsel++;
		}
	}
	
	if(totsel==0) {
		/* is de aangegeven file in de filelist? */
		if(sfile->file[0]) {
			for(a=0; a<sfile->totfile; a++) {
				if( strcmp(sfile->filelist[a].relname, sfile->file)==0) break;
			}
			if(a==sfile->totfile) {
				error("Wrong indicated name");
				return;
			}
		}
		else {
			error("Nothing indicated");
			return;
		}
	}
	
	/* nu hebben OF geselecteerde, OF 1 aangegeven file */
	
	read_file_dna(sfile->libfiledata);
	
	bzero(&main, sizeof(Main));
	strcpy(main.name, dir);
	
	idcode= groupname_to_code(group);
	
	if(totsel==0) {
		append_named_part(sfile, &main, sfile->file, idcode);
	}
	else {
		for(a=0; a<sfile->totfile; a++) {
			if(sfile->filelist[a].flags & ACTIVE) {
				append_named_part(sfile, &main, sfile->filelist[a].relname, idcode);
			}
		}
	}
	
	/* de main consistent maken */
	expand_main(&main, sfile->libfiledata);
	lib_link_all(&main);
	
	add_main_to_main(G.main, &main, G.scene);
	
	/* losslingerende blokken vrijgeven */
	add_data_adr(0, 0);
	add_lib_adr(0, 0);
}

/* ********** END READ FILE ****************** */
/* ********** WRITE FILE ****************** */

void writestruct(int filecode, char *structname, int nr, void *adr)
{
	BHead bh;
	short *sp;
	
	if(adr==0) return;

	/* BHead vullen met data */
	bh.code= filecode;
	bh.old= adr;
	bh.nr= nr;

	bh.SDNAnr= findstruct_nr(&cur_sdna, structname);
	if(bh.SDNAnr== -1) {
		printf("error: can't find SDNA code %s\n", structname);
		return;
	}
	sp= cur_sdna.structs[bh.SDNAnr];
	
	bh.len= cur_sdna.typelens[sp[0]];

	if(bh.len==0) return;
		
	mywrite(&bh, sizeof(BHead));
	mywrite(adr, nr*bh.len);
	
}

void writedata(int filecode, int len, void *adr)	/* geen struct */
{
	BHead bh;
	short *sp;
	
	if(adr==0) return;
	if(len==0) return;

	/* BHead vullen met data */
	bh.code= filecode;
	bh.old= adr;
	bh.nr= 1;
	bh.SDNAnr= 0;	
	bh.len= len;
	
	mywrite(&bh, sizeof(BHead));
	if(len) mywrite(adr, len);
	
}

void write_objects(ListBase *idbase)
{
	Object *ob;
	
	ob= idbase->first;
	while(ob) {
		/* schrijf LibData */
		writestruct(ID_OB, "Object", 1, ob);
		
		/* alle direkte data */
		writedata(DATA, 4*ob->totcol, ob->mat);
		
		ob= ob->id.next;
	}
}

void write_cameras(ListBase *idbase)
{
	Camera *cam;
	
	cam= idbase->first;
	while(cam) {
		/* schrijf LibData */
		writestruct(ID_CA, "Camera", 1, cam);
		
		/* alle direkte data */

		
		cam= cam->id.next;
	}
}

void write_lamps(ListBase *idbase)
{
	Lamp *la;
	
	la= idbase->first;
	while(la) {
		/* schrijf LibData */
		writestruct(ID_LA, "Lamp", 1, la);
		
		/* alle direkte data */

		
		la= la->id.next;
	}
}



void write_scenes(ListBase *scebase)
{
	Scene *sce;
	Base *base;
	
	sce= scebase->first;
	while(sce) {
		/* schrijf LibData */
		writestruct(ID_SCE, "Scene", 1, sce);
		
		/* alle direkte data */
		base= sce->base.first;
		while(base) {
			writestruct(DATA, "Base", 1, base);
			base= base->next;
		}
		sce= sce->id.next;
	}
}

void write_screens(ListBase *scrbase)
{
	Screen *sc;
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	View2D *v2d;
	SpaceIpo *ipo;
	SpaceButs *buts;
	int len;
	
	sc= scrbase->first;
	while(sc) {
		/* schrijf LibData */
		writestruct(ID_SCR, "Screen", 1, sc);
		
		/* alle direkte data */
		sv= sc->vertbase.first;
		while(sv) {
			writestruct(DATA, "ScrVert", 1, sv);
			sv= sv->next;
		}

		se= sc->edgebase.first;
		while(se) {
			writestruct(DATA, "ScrEdge", 1, se);
			se= se->next;
		}

		sa= sc->areabase.first;
		while(sa) {
			writestruct(DATA, "ScrArea", 1, sa);
			
			v3d= sa->spacedata.first; /* v3d als algemeen voorbeeld */
			while(v3d) {
				if(v3d->spacetype==SPACE_VIEW3D) {
					writestruct(DATA, "View3D", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_IPO) {
					writestruct(DATA, "SpaceIpo", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_BUTS) {
					writestruct(DATA, "SpaceButs", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_FILE) {
					writestruct(DATA, "SpaceFile", 1, v3d);
				}
				v3d= v3d->next;
			}
			
			sa= sa->next;
		}

		sc= sc->id.next;
	}
}

void write_global()
{
	FileGlobal fg;
	Scene *sc;
	
	fg.curscreen= G.curscreen;
	
	writestruct(GLOB, "FileGlobal", 1, &fg);

}

void write_file(char *dir)
{
	/* struct Temp2 rt; */
	int file, len, fout=0;
	short curs;
	char di[100], astr[200], tempname[100], *home;
	
	
	len= strlen(dir);
	if(len==0) return;
	if(dir[len-1]=='/') return;

	strcpy(di, dir);
	if(testextensie(di,".blend")==0) strcat(di,".blend");

	if(strcmp(dir, "/usr/tmp/core.blend")!=0) {
		file= open(di,O_RDONLY);
		close(file);
		if(file>-1) if(saveover(di)==0) return;

		/* if(G.ebase) load_ebasedata(); */
	}
	
	/* BEVEILIGING */
	strcpy(tempname, di);
	strcat(tempname, "@"); 
	
	file= open(tempname,O_WRONLY+O_CREAT+O_TRUNC);
	if(file== -1) {
		error("Can't write file");
		return;
	}

	curs= getcursorN();
	if(curs!=2) setcursorN(2);	/* pas op: doet winset */

	fchmod(file, 0664);
	bgnwrite(file);
	strcpy(G.sce, di);

	strcpy(astr, "BLENDER_V100    ");	/* blender 1.00 */
	mywrite(astr, 12);
/*----------------*/						
	
	write_screens(&G.main->screen);
	write_scenes(&G.main->scene);
	write_objects(&G.main->object);
	write_cameras(&G.main->camera);
	write_lamps(&G.main->lamp);

	
	write_global();

	writedata(DNA1, cur_sdna.datalen, cur_sdna.data);
	
	endwrite();
/*----------------*/						


	/* testen of alles goed is gelopen */
	len= ENDB;
	write(file, &len, 4);
	len= 0;
	if(write(file, &len,4)!=4) {
		error("Not enough diskspace");
		fout= 1;
	}

	close(file);

	/* EINDE BEVEILIGING */
	if(!fout) {
		sprintf(astr, "mv -f %s %s\n", tempname, di);
		if(system(astr) < 0) {
			error("Can't change old file. File saved with @");
		}
		writeBlog();	
	}
	else remove(tempname);
	
	if(curs!=2) setcursorN(curs);
}

