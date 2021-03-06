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

/*  readfile.c   juni 94     MIXED MODEL

 * 
 *  jan 95
 *  maart 95
 * 
 *   ZWAK PUNT: newadres berekening en meerdere files: oplossen
 *   bijvoorbeeld door per file aparte newadresarrays te maken? 
 *   
 * 
 * 
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
#include "graphics.h"
#include "file.h"
#include "screen.h"
#include "render.h"
#include "sequence.h"
#include "effect.h"
#include "sector.h"
#include "ika.h"
#include "oops.h"
#include "imasel.h"

#include <sys/param.h>
#include <fcntl.h>

typedef struct OldNew {
	void *old, *new;
	int nr;
} OldNew;

extern char *compflags;		/* genfile.c */
extern struct SDNA cur_sdna;
void read_libraries();

OldNew *datablocks=0;
OldNew *libblocks=0;
int datacount= 0, maxdatacount=1024;
int libcount= 0, maxlibcount=1024;

extern int switch_endian;	/* genfile.c */

void *disable_newlibadr;

/* ************ DIV ****************** */

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

int convertstringcode(char *str)
{
	int len, ret= 0;
	char *slash, temp[256];

	if (str[0] == '/' && str[1] == '/') {
		strcpy(temp, G.sce);
		slash = strrchr(temp, '/');
		if (slash) {
			strcpy(slash+1, str+2);
			strcpy(str, temp);
		}
		else {
			/* pad zonder slash! */
			strcpy(temp, str+2);
			strcpy(str, temp);
		}
		ret= 1;
	}
	len= strlen(str);
	if(str[len-1]=='#') {
		sprintf(str+len-1, "%04d", CFRA);
	}
	
	return ret;
}

void makestringcode(char *str)
{
	char *slash, len, temp[100];

	strcpy(temp, G.sce);
	slash = strrchr(temp,'/');
	if(slash) {
		*(slash+1)= 0;
		len= strlen(temp);
		if(len) {
			if(strncmp(str, temp, len)==0) {
				temp[0]= '/';
				temp[1]= '/';
				strcpy(temp+2, str+len);
				strcpy(str, temp);
			}
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


void *mallocNN(int len, char *str)
{
			PRINT2(d, s, len, str);
	return mallocN(len, str);
}
/* #define mallocN(a, b)	mallocNN(a, b) */

/* ************** AFHANDELING OUDE POINTERS ******************* */

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
			if(temp->nr==0 && temp->new) freeN(temp->new);
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
	OldNew *temp=0;
	int a;
	
	/* onderscheid tussen twee gevallen:
	 * 
	 * 1. lib obj's in locale scene, ob->parent		(old is uit library)
	 * 2. lib obj's in locale scene: base->object   (old is uit locale scene)
	 * 
	 */
	
	if(disable_newlibadr) {
		disable_newlibadr= new;
		
	}
	
	if(old==0) {	/* alles vrijgeven */
		if(libblocks) freeN(libblocks);
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

void *newlibadr(void *lib, void *adr)		/* alleen Lib datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;
	ID *id;
	
	/* ook testen op lib: lib==id->lib) */
	
	if(adr) {
	
		/* op goed geluk: eerst het volgende blok doen */
		/* als je dit weglaat gaat newcarou.blend mis */
		if(lastone<libcount-1) {
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->new;
				if(id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->new;
				}
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				id= onew->new;
				
				if(id && id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->new;
				}
			}
			onew++;
			lastone++;
		}

	}

	return 0;
}

void *newlibadr_us(void *lib, void *adr)		/* hoogt usernummer op */
{
	ID *id;
	
	id= newlibadr(lib, adr);
	if(id) {
		id->us++;
	}
	
	return id;
}

void *newlibadr_us_type(short type, void *adr)		/* alleen Lib datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;
	ID *id;
	
	if(adr) {
	
		/* op goed geluk: eerst het volgende blok doen */
		/* als je dit weglaat gaat newcarou.blend mis */

		if(lastone<libcount-1) {
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->new;
				if( GS(id->name) == type ) {
					id->us++;
					onew->nr++;
					return id;
				}
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				id= onew->new;
				
				if( GS(id->name) == type ) {
					id->us++;
					onew->nr++;
					return id;
				}
			}
			onew++;
			lastone++;
		}

	}

	return 0;
}

void change_libadr(void *old, void *new)
{
	struct OldNew *onew;
	ID *id;
	int lastone= 0;
	
	onew= libblocks;
	while(lastone<libcount) {
		
		id= onew->new;
		if(id && id->lib) {
			if(onew->new==old) {
				onew->new= new;
				
				/* geen return; blijkbaar kunnen er meer zijn? */
			}
		}
		
		onew++;
		lastone++;
	}
	
}



/* ********** END OUDE POINTERS ****************** */
/* ********** READ FILE ****************** */

void switch_endian_structs(BHead *bhead)
{
	int oldlen, blocks;
	short *spo;
	char *data;
	
	blocks= bhead->nr;
	data= (char *)(bhead+1);
	
	spo= old_sdna.structs[bhead->SDNAnr];
	oldlen= old_sdna.typelens[ spo[0] ];

	while(blocks--) {
		switch_endian_struct(bhead->SDNAnr, data);
		data+= oldlen;
	}
}

void read_struct(BHead *bh)
{
	void *temp= 0;
	
	if(bh->len) {
		
		if(switch_endian) switch_endian_structs(bh);
		
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			/* dit hele ingewikkelde spul is de naam van de struct, let op: old_sdna! */
			temp= mallocN(bh->len, old_sdna.types[ *old_sdna.structs[bh->SDNAnr] ]);
			
			/* temp= mallocN(bh->len, "read_struct"); */
			memcpy(temp, (bh+1), bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, &temp);
		}
	}
	
	add_data_adr(bh->old, temp);
}

void newstructname(BHead *bh)
{
	/* rijtje hardcoded uitzonderingen */
	short *sp;
	char *name;
	
	sp= old_sdna.structs[bh->SDNAnr];
	if( strcmp("Screen", old_sdna.types[sp[0]])==0 ) {
		bh->SDNAnr= findstruct_nr(&old_sdna, "bScreen");
	}
}


void *read_libstruct(BHead *bh)
{
	void *temp= 0;

	if(bh->len) {
	
		if(switch_endian) switch_endian_structs(bh);
		
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */

			temp= mallocN(bh->len, "read_libstruct");
			memcpy(temp, (bh+1), bh->len);

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

		if(switch_endian) switch_endian_structs(bh);
		
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			*data= mallocN(bh->len, "read_struct_expl");
			memcpy(*data, (bh+1), bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, data);
		}
	}
}

void link_list(ListBase *lb)		/* alleen direkte data */
{
	Link *ln, *prev;
	
	if(lb->first==0) return;

	lb->first= newadr(lb->first);
	
	ln= lb->first;
	prev= 0;
	while(ln) {
		
		ln->next= newadr(ln->next);
		ln->prev= prev;
		
		prev= ln;
		ln= ln->next;
	}
	
	lb->last= prev;
}

/* ************ READ voorbeeld ***************** */

void lib_link_vb(Main *main)
{
	Mesh *me;
	
	me= main->mesh.first;
	while(me) {
		if(me->id.flag & LIB_NEEDLINK) {
			
			me->id.flag -= LIB_NEEDLINK;
		}
		me= me->id.next;
	}
	
}

void direct_link_vb(me)
Mesh *me;
{

}

/* ************ READ IKA ***************** */

void lib_link_ika(Main *main)
{
	Ika *ika;
	int a;
	Deform *def;
	
	ika= main->ika.first;
	while(ika) {
		if(ika->id.flag & LIB_NEEDLINK) {
			
			ika->parent= newlibadr(ika->id.lib, ika->parent);
			
			a= ika->totdef;
			def= ika->def;
			while(a--) {
				def->ob=  newlibadr(ika->id.lib, def->ob);
				def++;
			}
			
			ika->id.flag -= LIB_NEEDLINK;
		}
		ika= ika->id.next;
	}
	
}

void direct_link_ika(ika)
Ika *ika;
{
	
	link_list(&ika->limbbase);

	ika->def= newadr(ika->def);
	
	/* afvangen fout uit V.138 en ouder */
	if(ika->def==0) ika->totdef= 0;
}

/* ************ READ Life ***************** */

void lib_link_life(Main *main)
{
	Life *lf;
	
	lf= main->life.first;
	while(lf) {
		if(lf->id.flag & LIB_NEEDLINK) {
			
			lf->sector= newlibadr(lf->id.lib, lf->sector);
			lf->dynamesh= newlibadr_us(lf->id.lib, lf->dynamesh);
			lf->texmesh= newlibadr_us(lf->id.lib, lf->texmesh);

			lf->id.flag -= LIB_NEEDLINK;
		}
		lf= lf->id.next;
	}
	
}

void direct_link_life(lf)
Life *lf;
{
	Sensor *sn;
	int a;
	
	lf->sensors= newadr(lf->sensors);
	a= lf->totsens;
	sn= lf->sensors;
	while(a--) {
		sn->events= newadr(sn->events);
		sn->actions= newadr(sn->actions);
		sn++;
	}
}

/* ************ READ Sector ***************** */

void lib_link_sector(Main *main)
{
	Sector *se;
	Portal *po;
	int a;
	
	se= main->sector.first;
	while(se) {
		if(se->id.flag & LIB_NEEDLINK) {
		
			se->ob= newlibadr(se->id.lib, se->ob);
			se->dynamesh= newlibadr_us(se->id.lib, se->dynamesh);
			se->texmesh= newlibadr_us(se->id.lib, se->texmesh);
			
			po= se->portals;
			a= se->totport;
			while(a--) {
				po->sector= newlibadr(se->id.lib, po->sector);
				po++;
			}
			
			se->id.flag -= LIB_NEEDLINK;
		}
		se= se->id.next;
	}
	
}

void direct_link_sector(se)
Sector *se;
{

	/* portals */
	se->portals= newadr(se->portals);
	
}

/* ************ READ LATTICE ***************** */

void lib_link_latt(Main *main)
{
	Lattice *lt;
	
	lt= main->latt.first;
	while(lt) {
		if(lt->id.flag & LIB_NEEDLINK) {
			
			lt->ipo= newlibadr_us(lt->id.lib, lt->ipo);
			lt->key= newlibadr_us(lt->id.lib, lt->key);
			
			lt->id.flag -= LIB_NEEDLINK;
		}
		lt= lt->id.next;
	}
	
}

void direct_link_latt(lt)
Lattice *lt;
{
	lt->def= newadr(lt->def);

}

/* ************ READ LAMP ***************** */

void lib_link_lamp(Main *main)
{
	Lamp *la;
	MTex *mtex;
	int a;
	
	la= main->lamp.first;
	while(la) {
		if(la->id.flag & LIB_NEEDLINK) {

			for(a=0; a<8; a++) {
				mtex= la->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(la->id.lib, mtex->tex);
					mtex->object= newlibadr(la->id.lib, mtex->object);
				}
			}
		
			la->ipo= newlibadr_us(la->id.lib, la->ipo);
			la->id.flag -= LIB_NEEDLINK;
		}
		la= la->id.next;
	}
}

void direct_link_lamp(la)
Lamp *la;
{
	int a;
	
	for(a=0; a<8; a++) {
		la->mtex[a]= newadr(la->mtex[a]);
	}
}

/* ************ READ keys ***************** */

void lib_link_key(Main *main)
{
	Key *key;
	
	key= main->key.first;
	while(key) {
		if(key->id.flag & LIB_NEEDLINK) {
			
			key->ipo= newlibadr_us(key->id.lib, key->ipo);
			key->from= newlibadr(key->id.lib, key->from);
			
			key->id.flag -= LIB_NEEDLINK;
		}
		key= key->id.next;
	}
	
}

void direct_link_key(key)
Key *key;
{
	KeyBlock *kb;
	
	link_list(&(key->block));
	
	key->refkey= newadr(key->refkey);
	
	kb= key->block.first;
	while(kb) {
		
		kb->data= newadr(kb->data);
		
		if(switch_endian) switch_endian_keyblock(key, kb);
			
		kb= kb->next;
	}
}

/* ************ READ mball ***************** */

void lib_link_mball(Main *main)
{
	MetaBall *mb;
	int a;
	
	mb= main->mball.first;
	while(mb) {
		if(mb->id.flag & LIB_NEEDLINK) {
			
			for(a=0; a<mb->totcol; a++) mb->mat[a]= newlibadr_us(mb->id.lib, mb->mat[a]);

			mb->ipo= newlibadr_us(mb->id.lib, mb->ipo);

			mb->id.flag -= LIB_NEEDLINK;
		}
		mb= mb->id.next;
	}
	
}

void direct_link_mball(mb)
MetaBall *mb;
{

	mb->mat= newadr(mb->mat);

	link_list(&(mb->elems));
	
	mb->disp.first= mb->disp.last= 0;
	
	mb->bb= 0;

}

/* ************ READ WORLD ***************** */

void lib_link_world(Main *main)
{
	World *wrld;
	MTex *mtex;
	int a;
	
	wrld= main->world.first;
	while(wrld) {
		if(wrld->id.flag & LIB_NEEDLINK) {
		
			wrld->ipo= newlibadr_us(wrld->id.lib, wrld->ipo);
			
			for(a=0; a<8; a++) {
				mtex= wrld->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(wrld->id.lib, mtex->tex);
					mtex->object= newlibadr(wrld->id.lib, mtex->object);
				}
			}
			
			wrld->id.flag -= LIB_NEEDLINK;
		}
		wrld= wrld->id.next;
	}
	
}

void direct_link_world(wrld)
World *wrld;
{
	int a;
	
	for(a=0; a<8; a++) {
		wrld->mtex[a]= newadr(wrld->mtex[a]);
	}
}


/* ************ READ IPO ***************** */

void lib_link_ipo(Main *main)
{
	Ipo *ipo;
	
	ipo= main->ipo.first;
	while(ipo) {
		if(ipo->id.flag & LIB_NEEDLINK) {
			
			ipo->id.flag -= LIB_NEEDLINK;
		}
		ipo= ipo->id.next;
	}
	
}

void direct_link_ipo(ipo)
Ipo *ipo;
{
	IpoCurve *icu;
	
	link_list(&(ipo->curve));
	icu= ipo->curve.first;
	while(icu) {
		icu->bezt= newadr(icu->bezt);
		icu->bp= newadr(icu->bp);
		icu= icu->next;
	}
}

/* ************ READ VFONT ***************** */

void direct_link_vfont(vf)
VFont *vf;
{
	int a;

	vf->data= callocN(sizeof(VFontData), "vfontdata");
	vf->flag= 1;	/* NEEDLOAD */
}

/* ************ READ IMAGE ***************** */

void lib_link_image(Main *main)
{
	Image *ima;
	
	ima= main->image.first;
	while(ima) {
		if(ima->id.flag & LIB_NEEDLINK) {
			
			ima->id.flag -= LIB_NEEDLINK;
		}
		ima= ima->id.next;
	}
	
}

void direct_link_image(ima)
Image *ima;
{
	int a;
	
	ima->ibuf= 0;
	ima->anim= 0;
	for(a=0; a<MAXMIPMAP; a++) ima->mipmap[a]= 0;
	ima->reprect= 0;
	
	ima->ok= 1;
	
}


/* ************ READ CURVE ***************** */

void lib_link_curve(Main *main)
{
	Curve *cu;
	int a;
	
	cu= main->curve.first;
	while(cu) {
		if(cu->id.flag & LIB_NEEDLINK) {
		
			for(a=0; a<cu->totcol; a++) cu->mat[a]= newlibadr_us(cu->id.lib, cu->mat[a]);

			cu->bevobj= newlibadr(cu->id.lib, cu->bevobj);
			cu->textoncurve= newlibadr(cu->id.lib, cu->textoncurve);
			cu->vfont= newlibadr_us(cu->id.lib, cu->vfont);

			cu->ipo= newlibadr_us(cu->id.lib, cu->ipo);
			cu->key= newlibadr_us(cu->id.lib, cu->key);
			
			cu->id.flag -= LIB_NEEDLINK;
		}
		cu= cu->id.next;
	}
	
}

void direct_link_curve(cu)
Curve *cu;
{
	Nurb *nu;
	
	cu->mat= newadr(cu->mat);
	cu->str= newadr(cu->str);

	if(cu->vfont==0) link_list(&(cu->nurb));
	else {
		cu->nurb.first=cu->nurb.last= 0;
	}
	
	cu->bev.first=cu->bev.last= 0;
	cu->disp.first=cu->disp.last= 0;
	cu->path= 0;
	
	nu= cu->nurb.first;
	while(nu) {
		nu->bezt= newadr(nu->bezt);
		nu->bp= newadr(nu->bp);
		nu->knotsu= newadr(nu->knotsu);
		nu->knotsv= newadr(nu->knotsv);

		if(switch_endian) {
			switch_endian_knots(nu);
		}
		
		nu= nu->next;
	}
	cu->bb= 0;
}

/* ************ READ TEX ***************** */

void lib_link_texture(Main *main)
{
	Tex *tex;
	
	tex= main->tex.first;
	while(tex) {
		if(tex->id.flag & LIB_NEEDLINK) {
		
			tex->ima= newlibadr_us(tex->id.lib, tex->ima);
			tex->ipo= newlibadr_us(tex->id.lib, tex->ipo);
			
			tex->id.flag -= LIB_NEEDLINK;
		}
		tex= tex->id.next;
	}
	
}

void direct_link_texture(tex)
Tex *tex;
{
	tex->plugin= newadr(tex->plugin);
	if(tex->plugin) {
		tex->plugin->handle= 0;
		open_plugin_tex(tex->plugin);
	}
	tex->coba= newadr(tex->coba);
}



/* ************ READ MATERIAL ***************** */

void lib_link_material(Main *main)
{
	Material *ma;
	MTex *mtex;
	int a;
	
	ma= main->mat.first;
	while(ma) {
		if(ma->id.flag & LIB_NEEDLINK) {
		
			ma->ipo= newlibadr_us(ma->id.lib, ma->ipo);
			
			for(a=0; a<8; a++) {
				mtex= ma->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(ma->id.lib, mtex->tex);
					mtex->object= newlibadr(ma->id.lib, mtex->object);
				}
			}
			
			ma->id.flag -= LIB_NEEDLINK;
		}
		ma= ma->id.next;
	}
	
}

void direct_link_material(ma)
Material *ma;
{
	int a;
	
	for(a=0; a<8; a++) {
		ma->mtex[a]= newadr(ma->mtex[a]);
	}
	ma->ren= 0;	/* mag niet blijven hangen, maarja */
}

/* ************ READ MESH ***************** */

void lib_link_mesh(Main *main)
{
	Mesh *me;
	TFace *tface;
	Image *ima;
	int a;
	
	me= main->mesh.first;
	while(me) {
		if(me->id.flag & LIB_NEEDLINK) {
		
			for(a=0; a<me->totcol; a++) me->mat[a]= newlibadr_us(me->id.lib, me->mat[a]);
			me->ipo= newlibadr_us(me->id.lib, me->ipo);
			me->key= newlibadr_us(me->id.lib, me->key);
			me->texcomesh= newlibadr_us(me->id.lib, me->texcomesh);
			
			if(me->tface) {
				a= me->totface;
				tface= me->tface;
				while(a--) {
					tface->tpage= newlibadr(me->id.lib, tface->tpage);

					if(tface->tpage) {
						ima= tface->tpage;
						if(ima->id.us==0) ima->id.us= 1;
					}
					tface++;
				}
			}
			me->id.flag -= LIB_NEEDLINK;
		}
		me= me->id.next;
	}
}

void direct_link_mesh(mesh)
Mesh *mesh;
{
	mesh->mat= newadr(mesh->mat);
	mesh->mvert= newadr(mesh->mvert);
	mesh->mface= newadr(mesh->mface);
	mesh->tface= newadr(mesh->tface);
	mesh->mcol= newadr(mesh->mcol);
	mesh->msticky= newadr(mesh->msticky);
	
	mesh->disp.first= mesh->disp.last= 0;
	mesh->bb= 0;
	mesh->oc= 0;
	mesh->dface= 0;
	mesh->orco= 0;
}

/* ************ READ OBJECT ***************** */

void lib_link_object(Main *main)
{
	Object *ob;
	NetLink *nl;
	void *poin;
	int warn=0, a;
	
	ob= main->object.first;
	while(ob) {
		if(ob->id.flag & LIB_NEEDLINK) {
	
			ob->parent= newlibadr(ob->id.lib, ob->parent);
			ob->track= newlibadr(ob->id.lib, ob->track);
			ob->ipo= newlibadr_us(ob->id.lib, ob->ipo);
			
			poin= ob->data;
		
			ob->data= newlibadr_us(ob->id.lib, ob->data);
			if(ob->data==0 && poin!=0) {
				ob->type= OB_EMPTY;
				warn= 1;
				if(ob->id.lib) printf("Can't find obdata of %s lib %s\n", ob->id.name+2, ob->id.lib->name);
				else printf("Object %s lost data. Lib:%x\n", ob->id.name+2, ob->id.lib);
			}
			for(a=0; a<ob->totcol; a++) ob->mat[a]= newlibadr_us(ob->id.lib, ob->mat[a]);
			
			ob->id.flag -= LIB_NEEDLINK;
			/* dit stond er eerst: weggehaald omdat de fie give_base_to... er niet meer is */
			/* if(ob->id.us) ob->id.flag -= LIB_NEEDLINK; */
			/* als us==0 wordt verderop nog een base gemaakt */
			
			nl= ob->network.first;
			while(nl) {
				nl->ob= newlibadr(ob->id.lib, nl->ob);
				nl= nl->next;
			}

		}
		ob= ob->id.next;
	}
	
	if(warn) error("WARNING IN CONSOLE");
}

void direct_link_object(ob)
Object *ob;
{
	PartEff *paf;
	
	ob->disp.first=ob->disp.last= 0;
	ob->mat= newadr(ob->mat);
	link_list(&ob->effect);
	paf= ob->effect.first;
	while(paf) {
		if(paf->type==EFF_PARTICLE) {
			paf->keys= 0;
		}
		paf= paf->next;
	}

	link_list(&ob->network);

	ob->bb= 0;
}

/* ************ READ SCENE ***************** */

void lib_link_scene(Main *main)
{
	Scene *sce;
	Base *base, *next;
	Editing *ed;
	Sequence *seq;
	
	sce= main->scene.first;
	while(sce) {
		if(sce->id.flag & LIB_NEEDLINK) {
			sce->id.us= 1;
			sce->camera= newlibadr(sce->id.lib, sce->camera);
			sce->world= newlibadr_us(sce->id.lib, sce->world);
			sce->set= newlibadr(sce->id.lib, sce->set);
			sce->ima= newlibadr_us(sce->id.lib, sce->ima);
			
			base= sce->base.first;
			while(base) {
				next= base->next;
				
				/* base->object= newlibadr_us(sce->id.lib, base->object); */

				base->object= newlibadr_us_type(ID_OB, base->object);
				
				if(base->object==0) {
					printf("LIB ERROR: base removed\n");
					remlink(&sce->base, base);
					if(base==sce->basact) sce->basact= 0;
					freeN(base);
				}
				base= next;
			}
			
			ed= sce->ed;
			if(ed) {
				WHILE_SEQ(ed->seqbasep) {
					if(seq->ipo) seq->ipo= newlibadr_us(sce->id.lib, seq->ipo);
					if(seq->scene) seq->scene= newlibadr(sce->id.lib, seq->scene);
				}
				END_SEQ
			}
			sce->id.flag -= LIB_NEEDLINK;
		}
		sce= sce->id.next;
	}
}

void link_recurs_seq(ListBase *lb)
{
	Sequence *seq;

	link_list( lb );
	seq= lb->first;
	while(seq) {
		if(seq->seqbase.first) link_recurs_seq( &seq->seqbase );
		seq= seq->next;
	}
}

void direct_link_scene(sce)
Scene *sce;
{
	Editing *ed;
	Sequence *seq;
	Strip *strip;
	StripElem *se;
	MetaStack *ms;
	int a;
	
	link_list( &(sce->base) );

	sce->basact= newadr(sce->basact);
	
	if(sce->ed) {
		ed= sce->ed= newadr(sce->ed);
		
		ed->metastack.first= ed->metastack.last= 0;
		
		/* recursief sequenties linken, ook lb wordt goedgezet */
		link_recurs_seq(&ed->seqbase);
		
		ed->seqbasep= &ed->seqbase;
		
		WHILE_SEQ(ed->seqbasep) {
			
			seq->seq1= newadr(seq->seq1);
			seq->seq2= newadr(seq->seq2);
			seq->seq3= newadr(seq->seq3);
			/* eigenlijk een patch: na invoering drie-seq effects */
			if(seq->seq3==0) seq->seq3= seq->seq2;
			
			seq->curelem= 0;
			
			seq->plugin= newadr(seq->plugin);
			if(seq->plugin) open_plugin_seq(seq->plugin, seq->name+2);
	
			seq->strip= newadr(seq->strip);
			if(seq->strip && seq->strip->done==0) {
				seq->strip->done= 1;
				
				/* standaard: strips van effecten/meta's worden niet weggeschreven, wel malloccen */
				
				if(seq->type==SEQ_IMAGE) {
					seq->strip->stripdata= newadr(seq->strip->stripdata);
					se= seq->strip->stripdata;
					if(se) {
						for(a=0; a<seq->strip->len; a++, se++) {
							se->ok= 1;
							se->ibuf= 0;
						}
					}
				}
				else if(seq->type==SEQ_MOVIE) {
					/* alleen eerste stripelem zit in file */
					se= newadr(seq->strip->stripdata);
					
					if(se) {
						seq->strip->stripdata= callocN(seq->len*sizeof(StripElem), "stripelem");
						*seq->strip->stripdata= *se;
						freeN(se);
						
						se= seq->strip->stripdata;
					
						for(a=0; a<seq->strip->len; a++, se++) {
							se->ok= 1;
							se->ibuf= 0;
							se->nr= a + 1;
						}
					}
				}
				else if(seq->len>0) 
					seq->strip->stripdata= callocN(seq->len*sizeof(StripElem), "stripelem");

			}
		}
		END_SEQ
	}
}

/* ************ READ SCREEN ***************** */

void lib_link_screen(Main *main)
{
	bScreen *sc;
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	View2D *v2d;
	SpaceIpo *si;
	SpaceButs *buts;
	SpaceFile *sfile;
	Oops *oops;
	
	sc= main->screen.first;
	while(sc) {
		if(sc->id.flag & LIB_NEEDLINK) {
			sc->id.us= 1;
			sc->scene= newlibadr(sc->id.lib, sc->scene);
			
			sa= sc->areabase.first;
			while(sa) {
				
				sa->full= newlibadr(sc->id.lib, sa->full);
	
				v3d= sa->spacedata.first;	/* v3d als voorbeeld */
				while(v3d) {
				
					if(v3d->spacetype==SPACE_VIEW3D) {
						v3d->camera= newlibadr(sc->id.lib, v3d->camera);
						if(v3d->bgpic) {
							v3d->bgpic->ima= newlibadr_us(sc->id.lib, v3d->bgpic->ima);
							v3d->bgpic->tex= newlibadr_us(sc->id.lib, v3d->bgpic->tex);
							v3d->bgpic->rect= 0;
						}
						if(v3d->localvd) {
							v3d->localvd->camera= newlibadr(sc->id.lib, v3d->localvd->camera);
						}
					}
					else if(v3d->spacetype==SPACE_IPO) {
						si= (SpaceIpo *)v3d;
						si->editipo= 0;
						si->from= 0;
						si->ipokey.first= si->ipokey.last= 0;
						si->ipo= newlibadr(sc->id.lib, si->ipo);
					}
					else if(v3d->spacetype==SPACE_BUTS) {
						buts= (SpaceButs *)v3d;
						buts->rect= 0;
						buts->lockpoin= 0;
						set_rects_butspace(buts);
					}
					else if(v3d->spacetype==SPACE_FILE) {
						sfile= (SpaceFile *)v3d;
						
						sfile->filelist= 0;
						sfile->libfiledata= 0;
						sfile->returnfunc= 0;
					}
					else if(v3d->spacetype==SPACE_IMASEL) {
						check_imasel_copy(v3d);
					}
					else if(v3d->spacetype==SPACE_PAINT) {
						check_paint_copy(v3d);
					}
					else if(v3d->spacetype==SPACE_IMAGE) {
						SpaceImage *sima= (SpaceImage *)v3d;
						
						sima->image= newlibadr(sc->id.lib, sima->image);
					}
					else if(v3d->spacetype==SPACE_OOPS) {
						SpaceOops *so= (SpaceOops *)v3d;
						
						/* patch als deze in oude files zit */
						if(so->v2d.cur.xmin==so->v2d.cur.xmax) {
							init_v2d_oops(&so->v2d);		
						}
						oops= so->oops.first;
						while(oops) {
							oops->id= newlibadr(0, oops->id);
							oops= oops->next;
						}
						so->lockpoin= 0;
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

void direct_link_screen(sc)
bScreen *sc;
{
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	SpaceOops *soops;
	Oops *oops;

	link_list( &(sc->vertbase) );
	link_list( &(sc->edgebase) );
	link_list( &(sc->areabase) );
	sc->winakt= 0;

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

		v3d= sa->spacedata.first;
		while(v3d) {
			if(v3d->spacetype==SPACE_VIEW3D) {
				
				v3d->bgpic= newadr(v3d->bgpic);
				v3d->localvd= newadr(v3d->localvd);
				if(v3d->gridlines==0) v3d->gridlines= 20;
			}
			else if(v3d->spacetype==SPACE_OOPS) {
				soops= (SpaceOops *)v3d;
				link_list( &(soops->oops) );
				oops= soops->oops.first;
				while(oops) {
					oops->link.first= oops->link.last= 0;
					oops= oops->next;
				}
			}
			v3d= v3d->next;
		}
		
		sa->v1= newadr(sa->v1);
		sa->v2= newadr(sa->v2);
		sa->v3= newadr(sa->v3);
		sa->v4= newadr(sa->v4);
		
		sa->win= sa->headwin= 0;
		sa->cursor= 0;
		sa->headqueue= sa->hq= sa->winqueue= sa->wq= 0;

		set_func_space(sa);	/* space.c */
		
		sa= sa->next;
	}
} 

/* ********** READ LIBRARY *************** */


void direct_link_library(lib)
Library *lib;
{
	Main *newmain;
	
	/* nieuwe main */
	newmain= callocN(sizeof(Main), "directlink");
	addtail(&G.mainbase, newmain);
	newmain->curlib= lib;

}

void lib_link_library(main)
Main *main;
{
	Library *lib;
	
	lib= main->library.first;
	while(lib) {
		lib->id.us= 1;
		lib= lib->id.next;
	}

}

/* ************** ALG & MAIN ******************** */

int read_libblock(Main *main, BHead *bhead, int flag)
{
	/* deze routine leest libblock en direkte data. Met linkfunkties
	 * alles aan elkaar hangen.
	 */
	
	ID *id;
	ListBase *lb;
	int skipdata;
	char *fd;

	if(bhead->code==ID_ID) {
		id= (ID *)(bhead + 1);
		lb= wich_libbase(main, GS(id->name));
		/* printf("idid %s oldp %x\n", id->name, bhead->old); */
	}
	else {
		lb= wich_libbase(main, bhead->code);
	}

	if(lb==0) {
		char str[5];
		str[4]= 0;
		
		COPY_4(str, &(bhead->code));
		PRINT2(s, d, str, bhead->len);
		PRINT(d, ID_OB);
	
		return bhead->len+sizeof(BHead);
	}
	
	fd= (char *)bhead;
	
	/* libblock inlezen */
	id= read_libstruct(bhead);
	addtail(lb, id);
	/* eerste acht bits wissen */
	id->flag= (id->flag & 0xFF00) | flag | LIB_NEEDLINK;
	id->lib= main->curlib;
	if(id->flag & LIB_FAKEUSER) id->us= 1;
	else id->us= 0;
	
	/* deze mag niet door de direct_link molen: is alleen het ID deel */
	if(bhead->code==ID_ID) {
		skipdata= bhead->len+sizeof(BHead);
		return skipdata;	
	}

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
			direct_link_screen(id);
			break;
		case ID_SCE:
			direct_link_scene(id);
			break;
		case ID_OB:
			direct_link_object(id);
			break;
		case ID_ME:
			direct_link_mesh(id);
			break;
		case ID_CU:
			direct_link_curve(id);
			break;
		case ID_MB:
			direct_link_mball(id);
			break;
		case ID_MA:
			direct_link_material(id);
			break;
		case ID_TE:
			direct_link_texture(id);
			break;
		case ID_IM:
			direct_link_image(id);
			break;
		case ID_LA:
			direct_link_lamp(id);
			break;
		case ID_VF:
			direct_link_vfont(id);
			break;
		case ID_IP:
			direct_link_ipo(id);
			break;
		case ID_KE:
			direct_link_key(id);
			break;
		case ID_LT:
			direct_link_latt(id);
			break;
		case ID_IK:
			direct_link_ika(id);
			break;
		case ID_SE:
			direct_link_sector(id);
			break;
		case ID_LF:
			direct_link_life(id);
			break;
		case ID_WO:
			direct_link_world(id);
			break;
		case ID_LI:
			direct_link_library(id);
			break;
	}
	
	/* vrijgeven, herinitialiseren */
	add_data_adr(0, (void *)1);	
	
	return skipdata;
}

void link_global(FileGlobal *fg)
{

	R.displaymode= fg->displaymode;
	R.winpos= fg->winpos;

	strcpy(G.psx, fg->psx);
	
	G.curscreen= newlibadr(0, fg->curscreen);
	if(G.curscreen==0) G.curscreen= G.main->screen.first;
	G.scene= G.curscreen->scene;
	G.obedit= 0;
	G.buts=0;
	G.v2d= 0;
	G.vd= 0;
	G.soops= 0;
	G.sima= 0;
	G.sipo= 0;
	
}

void vcol_to_fcol(Mesh *me)
{
	MFace *mface;
	ulong *mcol, *mcoln, *mcolmain;
	int a;

	if(me->totface==0 || me->mcol==0) return;
	
	mcoln= mcolmain= mallocN(4*4*me->totface, "mcoln");
	mcol = (ulong *)me->mcol;
	mface= me->mface;
	for(a=me->totface; a>0; a--, mface++) {
		mcoln[0]= mcol[mface->v1];
		mcoln[1]= mcol[mface->v2];
		mcoln[2]= mcol[mface->v3];
		mcoln[3]= mcol[mface->v4];
		mcoln+= 4;
	}
	freeN(me->mcol);
	me->mcol= (MCol *)mcolmain;
}

void oldstyle_events(Life *lf)
{
	Sensor *sn;
	int a;
	
	sn= lf->sensors;
	a= lf->totsens;
	while(a--) {
		
		switch(sn->action) {
		case 1:
			sn->force= -sn->force;
			sn->action= SN_X_TRANS;
			break;
		case 3:
			sn->force= -sn->force;
			sn->action= SN_Y_TRANS;
			break;
		case 5:
			sn->force= -sn->force;
			sn->action= SN_Z_TRANS;
			break;
	
		case 7:
			sn->force= -sn->force;
			sn->action= SN_X_ROT;
			break;
		case 9:
			sn->force= -sn->force;
			sn->action= SN_Y_ROT;
			break;
		case 11:
			sn->force= -sn->force;
			sn->action= SN_Z_ROT;
			break;
		}
		sn++;
	}
}



void do_versions(Main *main)
{
	/* PAS OP: pointers van libdata zijn nog niet omgezet */
	Tex *tex;
	Object *ob;
	Material *ma;
	Scene *sce;
	int a, b;

	if(main->versionfile == 100) {

		/* tex->extend en tex->imageflag veranderd: */
		
		tex= main->tex.first;
		while(tex) {
			if(tex->id.flag & LIB_NEEDLINK) {
				
				if(tex->extend==0) {
					if(tex->xrepeat || tex->yrepeat) tex->extend= TEX_REPEAT;
					else {
						tex->extend= TEX_EXTEND;
						tex->xrepeat= tex->yrepeat= 1;
					}
				}
	
				if(tex->imaflag & TEX_ANIM5) {
					tex->imaflag |= TEX_MORKPATCH;
					tex->imaflag |= TEX_ANTIALI;
				}
			}
			tex= tex->id.next;
		}
	}
	if(main->versionfile <= 101) {
		/* frame mapping */
		sce= main->scene.first;
		while(sce) {
			sce->r.framapto= 100;
			sce->r.images= 100;
			sce->r.framelen= 1.0;
			sce= sce->id.next;
		}
	}
	if(main->versionfile <= 102) {
		/* init halo's op 1.0 */
		ma= main->mat.first;
		while(ma) {
			ma->add= 1.0;
			ma= ma->id.next;
		}
	}
	if(main->versionfile <= 103) {
		/* nieuwe variabele in object: colbits */
		ob= main->object.first;
		while(ob) {
			ob->colbits= 0;
			if(ob->totcol) {
				for(a=0; a<ob->totcol; a++) {
					if(ob->mat[a]) ob->colbits |= (1<<a);
				}
			}
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 104) {
		/* de timeoffs zit op betere plek */
		ob= main->object.first;
		while(ob) {
			if(ob->transflag & 1) {
				ob->transflag -= 1;
				ob->ipoflag |= OB_OFFS_OB;
			}
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 105) {
		ob= main->object.first;
		while(ob) {
			ob->dupon= 1; ob->dupoff= 0;
			ob->dupsta= 1; ob->dupend= 100;
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 106) {
		Mesh *me;
		/* mcol is veranderd */

		me= main->mesh.first;
		while(me) {
			if(me->mcol) vcol_to_fcol(me);
			me= me->id.next;
		}
		
	}
	if(main->versionfile <= 107) {
		/* tot dec 96 ofzo */
		Life *lf= main->life.first;
		while(lf) {
			oldstyle_events(lf);
			lf= lf->id.next;
		}
		sce= main->scene.first;
		while(sce) {
			sce->r.mode |= R_GAMMA;
			sce= sce->id.next;
		}		
		ob= main->object.first;
		while(ob) {
			ob->ipoflag |= OB_OFFS_PARENT;
			ob= ob->id.next;
		}
		
	}
	if(main->versionfile <= 108) {
		Sector *se= main->sector.first;
		while(se) {
			se->size[0]= se->size[1]= se->size[2]= 1.0;
			se= se->id.next;
		}
	}	
	if(main->versionfile <= 109) {
		bScreen *sc;
		ScrArea *sa;
		View3D *vd;
		Life *lf;
		Sensor *sn;
		
		/* nieuwe variabele: gridlines */
		
		sc= main->screen.first;
		while(sc) {
			
			sa= sc->areabase.first;
			while(sa) {
	
				vd= sa->spacedata.first;
				while(vd) {
					if(vd->spacetype==SPACE_VIEW3D) {
						if(vd->gridlines==0) vd->gridlines= 20;
					}
					vd= vd->next;
				}
				sa= sa->next;
			}
			sc= sc->id.next;
		}
		
		/* sensors rename: */
		lf= main->life.first;
		while(lf) {
			sn= lf->sensors;
			a= lf->totsens;
			while(a--) {
				sn->sta= sn->fac1;
				sn->end= sn->fac2;
				sn->fac1= sn->fac2= 0.0;
				sn++;
			}
			lf= lf->id.next;
		}
	}
	if(main->versionfile <= 110) {
		Life *lf;
		Sensor *sn;
		
		/* sensor omgebouwd */

		lf= main->life.first;
		while(lf) {
			sn= lf->sensors;
			a= lf->totsens;
			while(a--) {
				sn->totevent= 1;
				sn->events= callocN(sizeof(Event), "event");
				sn->events->event= sn->event1;
				
				if(sn->event1==19) {
					sn->events->event= 31;	/* always */
					if(sn->action==SN_PLAY_OBIPO) sn->action= SN_LOOPSTOP_OBIPO;
				}
				
				sn->totaction= 1;
				sn->actions= callocN(sizeof(Action), "action");
				sn->actions->action= sn->action;
				sn->actions->force= sn->force;
				sn->actions->butsta= ffloor(sn->sta+0.5);
				sn->actions->butend= ffloor(sn->end+0.5);
				
				sn++;
			}
			lf= lf->id.next;
		}
	}
	if(main->versionfile <= 111) {
		Life *lf;
		Camera *cam;
		NetLink *nl;
		
		/* nieuwe variabele */
		cam= main->camera.first;
		while(cam) {
			if(cam->netsta==0.0) cam->netsta= 1.0;
			if(cam->netend==0.0) cam->netend= 20.0;
			cam= cam->id.next;
		}
		
		/* vergeten DNA weg te schrijven in BHead */
		ob= main->object.first;
		while(ob) {
			freelistN(&ob->network);
			ob= ob->id.next;
		}
		
		/* nieuwe default optie */
		lf= main->life.first;
		while(lf) {
			lf->flag |= LF_DO_FH;
			lf= lf->id.next;
		}
	}
	if(main->versionfile <= 112) {
		Mesh *me= main->mesh.first;
		while(me) {
			me->cubemapsize= 1.0;
			me= me->id.next;
		}
	}
	if(main->versionfile <= 113) {
		ma= main->mat.first;
		while(ma) {
			if(ma->flaresize==0.0) ma->flaresize= 1.0;
			ma->subsize= 1.0;
			ma->flareboost= 1.0;
			ma= ma->id.next;
		}
	}
	if(main->versionfile <= 114) {
		Mesh *me= main->mesh.first;
		MVert *mvert;
		MFace *mface;
		int a;
		short edcode;
		
		/* edge drawflags veranderd */
		
		while(me) {
			a= me->totface;
			mface= me->mface;
			while(a--) {
				if(mface->edcode & 16) {
					mface->edcode -= 16;
					mface->edcode |= ME_V3V1;
					/* cylinder */
					mface->edcode |= DF_V1V2|DF_V2V3|DF_V3V4|DF_V4V1;
				}
				mface++;
			}
			a= me->totvert;
			mvert= me->mvert;
			while(a--) {
				mvert->flag= ME_SPHERETEST;
				mvert++;
			}
			me= me->id.next;
		}
	}
	if(main->versionfile <= 115) {
		Life *lf;
		Sensor *sn;
		Action *ac;
		
		/* frictfac in action op 1.0 zetten */
		
		lf= main->life.first;
		while(lf) {
			
			a= lf->totsens;
			sn= lf->sensors;
			while(a--) {
				ac= sn->actions;
				b= sn->totaction;
				while(b--) {
					ac->fac= 1.0;
					ac++;
				}
				sn++;
			}
			lf= lf->id.next;
		}
		
	}
	if(main->versionfile <= 116) {
		Life *lf;
		Sensor *sn;
		Event *ev;
		Action *ac;
		int sethold;
		
		/* flag in action zetten, holdflags in events */
		
		lf= main->life.first;
		while(lf) {
			
			a= lf->totsens;
			sn= lf->sensors;
			while(a--) {
				
				sethold= 0;
				ac= sn->actions;
				b= sn->totaction;
				while(b--) {
					if(ac->action<100) {
						sethold= 1;
					}
					else if(ac->action<200) {
						ac->flag |= SN_IPOFORCE;
					}
					else if(ac->action==SN_ADDLIFE) sethold= -1;
					
					ac++;
				}
				
				ev= sn->events;
				b= sn->totevent;
				while(b--) {
					if(sethold== -1);	/* niks doen: add life geen hold */
					else {
						if(sethold || (sn->flag & SN_HOLD) ) ev->flag= SN_HOLD;
						if ELEM(ev->event, SN_CONTACT, SN_NEAR) ev->flag= SN_HOLD;
						if ELEM3(ev->event, SN_VAR_INTERVAL, SN_VAR_EQUAL, SN_VAR_CHANGED) ev->flag= SN_HOLD;
					}
					ev++;
				}
				
				sn->flag= 0;
				
				sn++;
			}
			lf= lf->id.next;
		}
		
	}
	if(main->versionfile <= 117) {
		Life *lf;
		Sensor *sn;
		Event *ev;
		Action *ac;
		
		/* near naam wissen */
		
		lf= main->life.first;
		while(lf) {
			
			/* stond in buttons.c */
			if(lf->axsize==0.0) lf->axsize= 0.5;
			
			a= lf->totsens;
			sn= lf->sensors;
			while(a--) {
				
				ev= sn->events;
				b= sn->totevent;
				while(b--) {
					if(ev->event==SN_NEAR) ev->name[0]= 0;
					ev++;
				}
				sn++;
			}
			lf= lf->id.next;
		}
	}
	if(main->versionfile <= 119) {		/* incl 118 */
		Life *lf;
		Action *ac;
		Sensor *sn;
		Material *ma;
		
		lf= main->life.first;
		while(lf) {
			if(lf->lay==0) lf->lay= 1;
			
			/* goto naam wissen */
			a= lf->totsens;
			sn= lf->sensors;
			while(a--) {
				
				ac= sn->actions;
				b= sn->totaction;
				while(b--) {
					if(ac->action==SN_GOTO) ac->name[0]= 0;
					ac++;
				}
				sn++;
			}
			
			lf= lf->id.next;
		}
		
		ma= main->mat.first;
		while(ma) {
			if(ma->lay==0) ma->lay= 1;
			ma= ma->id.next;
		}
	}

	/* eentje overgeslagen voor bug in freeware versie */
	
	if(main->versionfile <= 121) {
		/* O2 versie gemaakt. */
	}
	if(main->versionfile <= 122) {
		/* dithering gaat soms af (backbuf, pas sinds 121) */
		/* relatieve paden hersteld */
		/* sequences: endframe van seq wordt op betere plek geprint */
	}
	if(main->versionfile <= 123) {
		/* nog een paar O2 foutjes: keylines in ipo window */
		/* vertices halo object (O2) nu ook goed */
		/* zoomwin: ook op O2 */
		/* bug eruit: schaduw render in ortho */
	}
	if(main->versionfile <= 124) {
		/* inventor lezer */
		/* key kleur 24 bits beveiligd */
		/* schrijf plaatje: je kun niet naderhand 24bits naar 32 omzetten */
	}
	if(main->versionfile <= 125) {
		/* bug vanwege compileer fout */
	}
	if(main->versionfile <= 126) {
		Tex *tex;
		/* overdraw text beter (clever numbuts) */
		/* bug uit inventor lezer: node ambientColor werd niet herkend */
		/* bugje uit toolbox: clear loc= alt-g */
		/* Indy XL zbuffer OK. (net als O2) */
		/* r-g-b-fac in texure */
		/* genlock alpha in imap */
		
		tex= main->tex.first;
		while(tex) {
			if(tex->rfac==0.0 && tex->gfac==0.0 && tex->bfac==0.0) {
				tex->rfac= 1.0;
				tex->gfac= 1.0;
				tex->bfac= 1.0;
				tex->filtersize= 1.0;
			}
			tex= tex->id.next;
		}
	}
	if(main->versionfile <= 127) {
		/* bug uit imbuf: iris non-rle images */
		
	}
	if(main->versionfile <= 128) {
		/* nog een bug uit imbuf: grote IRIS plaatjes */
		/* bug uit relatieve paden: tutor werkt nu beter */
		/* serieuze bug: new_id verprutste user nrs */
		/* link menu, materials: betere implementatie */
		/* deform werkt nu ook op vertex parent */
	}
	if(main->versionfile <= 129) {
		/* mapping van images aangepast: scale en translate werkt bij spheremap */
		/* motion blur */
		/* sterren betere formule */
		/* effect: met wave uitgebreid */
		/* lamp tex: image projectie met OSA */
		sce= main->scene.first;
		while(sce) {
			if(sce->r.blurfac==0.0) sce->r.blurfac= 1.0;
			sce= sce->id.next;
		}
	}

	/* onder in blender.c de nummers wijzigen! */

}

void lib_link_all(Main *main)
{


	lib_link_screen(main);
	lib_link_scene(main);
	lib_link_object(main);
	lib_link_curve(main);
	lib_link_mball(main);
	lib_link_material(main);
	lib_link_texture(main);
	lib_link_image(main);
	lib_link_ipo(main);
	lib_link_key(main);
	lib_link_world(main);
	lib_link_lamp(main);
	lib_link_latt(main);
	lib_link_ika(main);
	lib_link_sector(main);
	lib_link_life(main);

	lib_link_mesh(main);	/* als laatste: tpage images met users op nul */

	lib_link_library(main);	/* alleen users goedzetten */
}


void read_file_dna(char *filedata, int filelen)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	freestructDNA(&old_sdna);

	fd= filedata;
	while(filelen>0 && afbreek==0) {
		
		bhead= (BHead *)fd;

		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code==DNA1) {

			old_sdna.data= mallocN(bhead->len, "sdna");
			memcpy(old_sdna.data, fd+sizeof(BHead), bhead->len);
			afbreek= 1;
		}
		
		fd+= bhead->len+sizeof(BHead);
		filelen -= bhead->len+sizeof(BHead);
	}

	
	init_structDNA(&old_sdna);
	set_compareflags_structDNA();
	
}

void switch_endian_bheads(char *filedata, int filelen)
{
	BHead *bhead;
	char *fd, str[5];
	
	str[4]= 0;
	
	fd= filedata;
	while(filelen>0) {
		
		bhead= (BHead *)fd;

		SWITCH_INT(bhead->len);
		SWITCH_INT(bhead->SDNAnr);
		SWITCH_INT(bhead->nr);
		
		/* de ID_.. codes */
		if((bhead->code & 0xFFFF)==0) bhead->code >>=16;

		if(bhead->code==ENDB) break;
		
		fd+= bhead->len+sizeof(BHead);
		filelen -= bhead->len+sizeof(BHead);
	}
}


char *openblenderfile(char *name, int *filelen)
{
	int len, file, version, temp, temp1;	
	char *filedata, str[16];
	
	len= strlen(name);
	if(len<6) return 0;
	if( name[len-1]=='/' ) return 0;

	if( strstr(name, ".blend") ) {
		file= open(name, O_RDONLY);
		if(file<=0) {
			error("Can't find file");
			return 0;
		}
		
		*filelen= lseek(file, 0, 2);	/* seek end */
		lseek(file, 0, 0);		/* en weer terug */

		read(file, str, 12);
		
		if(strncmp(str, "BLENDER_", 8)!=0) {
			close(file);
			error("Idiot file!");
			return 0;
		}
		
		/* endian test */
		switch_endian= 0;
		
		if( str[8]=='V' && G.order==L_ENDIAN) {
			switch_endian= 1;
			/* printf("file is big endian\n"); */
		}
		else if( str[8]=='v' && G.order==B_ENDIAN) {
			switch_endian= 1;
			/* printf("file is little endian\n"); */
		}
		
		G.versionfile= atoi(str+9);
		
		/* hele file inlezen */
		*filelen-= 12;
		filedata= mallocN(*filelen + 12, "filedata");	/* + 12: anders errors bij Little endina files. waarom??? */
		read(file, filedata, *filelen);
		close(file);
		
		if(switch_endian) switch_endian_bheads(filedata, *filelen);
		
		/* op zoek naar SDNA */
		read_file_dna(filedata, *filelen);

		return filedata;
	}
	else {
		read_exotic(name);
	}
	
	return 0;
}

void read_file(char *dir)
{
	BHead *bhead;
	Main *main;
	Object *ob;
	Curve *cu;
	FileGlobal *fg;
	UserDef *user;
	VFont *vf;
	int ok, len, filelen, skipdata, temp, temp1, version;
	char *filedata, *fd;
	
	waitcursor(1);

	filedata= openblenderfile(dir, &filelen);
	
	if (filedata) {
		G.save_over = TRUE;
		
		strcpy(G.sce, dir);
		strcpy(G.main->name, dir);	/* is gegarandeerd current file */
		
		/* er is maar 1 Main, dus alleen inhoud vrijgeven */

		free_main(0, G.main);
		FreeButs('i');	/* behalve icons */
		freefastshade();	/* anders oude lampgegevens */
		G.main->versionfile= G.versionfile;	/* kan per main verschillen */
		if(G.obedit) {
			freeNurblist(&editNurb);
			free_editmesh();
		}
		G.f &= ~(G_VERTEXPAINT + G_FACESELECT);

		/* alle data inlezen: */
		fd= filedata;
		ok= 0;
		main= G.main;
		
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
				break;
			case USER:
				read_struct_expl(bhead, &user);
				U= *user;
				freeN(user);
				skipdata= bhead->len+sizeof(BHead);
				break;
			case TEST:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case REND:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case ENDB:
				ok= 1;
				skipdata= 8;
				break;
			case ID_LI:
				skipdata= read_libblock(G.main, bhead, LIB_LOCAL);
				main= G.mainbase.last;
				break;
			case ID_ID:
				skipdata= read_libblock(main, bhead, LIB_READ+LIB_EXTERN);
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
		
		do_versions(G.main);	/* voor read_libraries */
		read_libraries();
		
		/* LibData linken */
		
		lib_link_all(G.main);
		link_global(fg);	/* als laatste */


		/* losslingerende blokken vrijgeven */
		add_data_adr(0, 0);
		add_lib_adr(0, 0);
		
		/* VECTORFONTS */
		vf= G.main->vfont.first;
		
		/* afvangen: Bfont van ander systeem */
		if(vf) {
			len= strlen(vf->name);

			if(len>5 && strcmp(vf->name+len-5, "Bfont")==0) {
				sprintf(vf->name, "%s/Bfont", getenv("BLENDERDIR"));
			}
		}
		while(vf) {
			reload_vfont(vf);
			vf= vf->id.next;
		}
		
		/* weinig DispListen, wel text_to_curve */
		ob= G.main->object.first;
		while(ob) {
			if(ob->type==OB_FONT) {
				cu= ob->data;
				if(cu->nurb.first==0) text_to_curve(ob, 0);
			}
			else if(ob->type==OB_MESH) {
				makeDispList(ob);
				if(ob->effect.first) object_wave(ob);
			}

			ob= ob->id.next;
		}
		
		freeN(fg);
		if(G.background==0) {
			setscreen(G.curscreen);
			countall();
		}
		set_scene_bg(G.scene);	/* baseflags */
		
		init_autosave();
		clear_obact_names();	/* voor add object */
		set_obact_names(OBACT);
		do_realtimelight(0, 0, 0);	/* re-init */
	}
	
	if(G.background==0) waitcursor(0);
}

int
inst_file(char *fname)
/*
 * Desc: installeer het bestand <fname> in de 
 *	 home directory van de user, ga er vanuit
 *	 dat het te installeren bestand in de BLENDERDIR 
 *	 te vinden is.
 * Post: return 0 als fout, anders 1
 */
{
#define BUFSIZE 1024
	FILE *f_in, *f_out;
	int n_in, n_out;
	char buf[BUFSIZE];
	char src_path[MAXPATHLEN];
	char dest_path[MAXPATHLEN];

	sprintf(src_path, "%s/%s", getenv("BLENDERDIR"), fname);
	f_in = fopen(src_path, "r");
	if (!f_in) {
	    return(0);
	}
	sprintf(dest_path, "%s/%s", getenv("HOME"), fname);
	f_out = fopen(dest_path, "w");
	if (!f_out) {
	    return(0);
	}
	n_in = fread(buf, 1, BUFSIZE, f_in);
	while (n_in == BUFSIZE) {
	    n_out = fwrite(buf, 1, n_in, f_out);
	    if (n_out < n_in) {
		/* Oops couldn't write the complete buffer */
		fclose(f_in);
		fclose(f_out);
		return(0);
	    }
	    n_in = fread(buf, 1, BUFSIZE, f_in);
	}
	/* schrijf het laatste stukje */
	if (n_in > 0) {
	    n_out = fwrite(buf, 1, n_in, f_out);
	    if (n_out < n_in) {
	    	/* Oops couldn't write the rest */
	    	fclose(f_in);
	    	fclose(f_out);
	    	return(0);
	    }
	}
	fclose(f_in);
	fclose(f_out);
	
	return(1);
} /* inst_file() */

int read_homefile()
{
	int file, ret;
	char *home, tstr[120], scestr[120];
	
	home = getenv("HOME");
	if (home) {

		strcpy(tstr, home);
		strcat(tstr, "/.B.blend");
		file= open(tstr, O_RDONLY);
		
		if (file < 0) {
			/* installatie doen */
	
			ret = inst_file(".B.blend");
			if (ret == 0) {
				printf("Could not install .B.Blend\n");
				printf("Has the software been properly installed?\n");
				exit(0);
			}
			ret = inst_file(".Bfs");
			if (ret == 0) {
				printf("Could not install .Bfs\n");
				printf("Has the software been properly installed?\n");
				exit(0);
			}
			file = open(tstr, O_RDONLY);
		} 
	
		if (file >= 0) {
			/* bestand gevonden */
			close(file);
			
			strcpy(scestr, G.sce);	/* even bewaren */
			read_file(tstr);
			strcpy(G.sce, scestr);
			
			G.save_over = FALSE;
			
			/* nog wat afhandelen? */
			disable_capslock(U.flag & NO_CAPSLOCK);
			
			return 1;
		}
	}
	return 0;
}

void read_autosavefile()
{
	int file;
	char tstr[120], scestr[120];
	int save_over;
	

	strcpy(scestr, G.sce);	/* even bewaren */
	sprintf(tstr, "%s%d.blend", U.tempdir, getpid());
	save_over = G.save_over;
	read_file(tstr);
	G.save_over = save_over;
	strcpy(G.sce, scestr);
			
}


/* ************* APPEND LIBRARY ************** */

BHead *bheadlib;

BHead *find_bhead(void *old, char *filedata)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	bheadlib= 0;
	if(old==0) return 0;
	
	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;

		if(bhead->code==ID_LI) {
			bheadlib= bhead;
		}
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->old==old) return bhead;
		
		fd+= bhead->len+sizeof(BHead);
	}
	return 0;
}

ID *is_yet_read(Main *main, BHead *bhead)
{
	ListBase *lb;
	ID *idtest, *id;
	
	idtest= (ID *)(bhead +1);
	lb= wich_libbase(main, GS(idtest->name));
	if(lb) {
		id= lb->first;
		while(id) {
			if( strcmp(id->name, idtest->name)==0 ) return id;
			id= id->next;
		}
	}
	return 0;
}

void expand_doit(Main *main, char *filedata, void *old)
{
	BHead *bhead;
	Library *lib;
	ID *id;

	bhead= find_bhead(old, filedata);
	if(bhead) {

		/* andere library? */
		
		if(bhead->code==ID_ID) {
			if(bheadlib) {
				lib= (Library *)(bheadlib+1);
				main= find_main(lib->name);

				id= is_yet_read(main, bhead);
		
				if(id==0) {
					read_libblock(main, bhead, LIB_READ+LIB_INDIRECT);
					printf("expand: andere lib %s\n", lib->name);
				}
				else {
					printf("expand: reeds ingelezen: %s lib: %s\n", id->name, lib->name);
					
					/* if(id->lib==0) add_lib_adr(bhead->old, id); */
					/* bovenstaand is niet nodig! Kan toch niet misgaan? */
					add_lib_adr(bhead->old, id);
					
				}
			}
		}
		else {
			id= is_yet_read(main, bhead);
			if(id==0) {
				id= (ID *)(bhead+1);
				read_libblock(main, bhead, LIB_TESTIND);
			}
			else {
				/* printf("expand: al ingelezen %s\n", id->name); */
				add_lib_adr(bhead->old, id);
			}
		}
	}
}

void expand_key(Main *main, char *filedata, Key *key)
{
	int a;
	
	expand_doit(main, filedata, key->ipo);
}


void expand_texture(Main *main, char *filedata, Tex *tex)
{
	int a;
	
	expand_doit(main, filedata, tex->ima);
}

void expand_material(Main *main, char *filedata, Material *ma)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(ma->mtex[a]) {
			expand_doit(main, filedata, ma->mtex[a]->tex);
			expand_doit(main, filedata, ma->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, ma->ipo);
}

void expand_lamp(Main *main, char *filedata, Lamp *la)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(la->mtex[a]) {
			expand_doit(main, filedata, la->mtex[a]->tex);
			expand_doit(main, filedata, la->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, la->ipo);
}

void expand_lattice(Main *main, char *filedata, Lattice *lt)
{
	int a;
	
	expand_doit(main, filedata, lt->ipo);
	expand_doit(main, filedata, lt->key);
}

void expand_sector(Main *main, char *filedata, Sector *se)
{
	int a;
	
	expand_doit(main, filedata, se->dynamesh);
	expand_doit(main, filedata, se->texmesh);
}

void expand_life(Main *main, char *filedata, Life *lf)
{
	int a;
	
	expand_doit(main, filedata, lf->dynamesh);
	expand_doit(main, filedata, lf->texmesh);
}



void expand_world(Main *main, char *filedata, World *wrld)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(wrld->mtex[a]) {
			expand_doit(main, filedata, wrld->mtex[a]->tex);
			expand_doit(main, filedata, wrld->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, wrld->ipo);
}


void expand_mball(Main *main, char *filedata, MetaBall *mb)
{
	int a;
	
	for(a=0; a<mb->totcol; a++) {
		expand_doit(main, filedata, mb->mat[a]);
	}
}

void expand_curve(Main *main, char *filedata, Curve *cu)
{
	int a;
	
	for(a=0; a<cu->totcol; a++) {
		expand_doit(main, filedata, cu->mat[a]);
	}
	expand_doit(main, filedata, cu->vfont);
	expand_doit(main, filedata, cu->key);
	expand_doit(main, filedata, cu->ipo);
	expand_doit(main, filedata, cu->bevobj);
	expand_doit(main, filedata, cu->textoncurve);
}

void expand_mesh(Main *main, char *filedata, Mesh *me)
{
	int a;
	TFace *tface;
	
	for(a=0; a<me->totcol; a++) {
		expand_doit(main, filedata, me->mat[a]);
	}
	
	expand_doit(main, filedata, me->key);
	expand_doit(main, filedata, me->texcomesh);
	
	if(me->tface) {
		tface= me->tface;
		a= me->totface;
		while(a--) {
			if(tface->tpage) expand_doit(main, filedata, tface->tpage);
			tface++;
		}
	}
}

void expand_object(Main *main, char *filedata, Object *ob)
{
	int a;
	
	expand_doit(main, filedata, ob->data);
	expand_doit(main, filedata, ob->ipo);
	for(a=0; a<ob->totcol; a++) {
		expand_doit(main, filedata, ob->mat[a]);
	}
}

void expand_scene(Main *main, char *filedata, Scene *sce)
{
	Base *base;
	
	base= sce->base.first;
	while(base) {
		expand_doit(main, filedata, base->object);
		base= base->next;
	}
	expand_doit(main, filedata, sce->camera);
	expand_doit(main, filedata, sce->world);
	
}

void expand_main(Main *main, char *filedata)
{
	ListBase *lbarray[30];
	ID *id;
	int a, doit= 1;
	
	if(filedata==0) return;
	
	while(doit) {
		doit= 0;
		
		a= set_listbasepointers(main, lbarray);
		while(a--) {
			id= lbarray[a]->first;

			while(id) {
				if(id->flag & LIB_TEST) {
					
					switch(GS(id->name)) {
						
					case ID_OB:
						expand_object(main, filedata, (Object *)id);
						break;
					case ID_ME:
						expand_mesh(main, filedata, (Mesh *)id);
						break;
					case ID_CU:
						expand_curve(main, filedata, (Curve *)id);
						break;
					case ID_MB:
						expand_mball(main, filedata, (MetaBall *)id);
						break;
					case ID_SCE:
						expand_scene(main, filedata, (Scene *)id);
						break;
					case ID_MA:
						expand_material(main, filedata, (Material *)id);
						break;
					case ID_TE:
						expand_texture(main, filedata, (Tex *)id);
						break;
					case ID_WO:
						expand_world(main, filedata, (World *)id);
						break;
					case ID_LT:
						expand_lattice(main, filedata, (Lattice *)id);
						break;
					case ID_LF:
						expand_life(main, filedata, (Life *)id);
						break;
					case ID_SE:
						expand_sector(main, filedata, (Sector *)id);
						break;
					case ID_LA:
						expand_lamp(main, filedata, (Lamp *)id);
						break;
					case ID_KE:
						expand_key(main, filedata, (Key *)id);
						break;
					}

					doit= 1;
					id->flag -= LIB_TEST;
					
				}
				id= id->next;
			}
		}
	}
}

void give_base_to_objects(Scene *sce, ListBase *lb)
{
	Object *ob;
	Base *base;
	int ok;
	
	/* alle objects die LIB_EXTERN en LIB_NEEDLINK zijn, een base geven */
	ob= lb->first;
	while(ob) {

		if(ob->id.us==0) {

			if(ob->id.flag & LIB_NEEDLINK) {
			
				ob->id.flag -= LIB_NEEDLINK;
				
				if( ob->id.flag & LIB_INDIRECT ) {
					
					base= callocN( sizeof(Base), "add_ext_base");
					addtail(&(sce->base), base);
					base->lay= ob->lay;
					base->object= ob;
					ob->id.us= 1;
					
					ob->id.flag -= LIB_INDIRECT;
					ob->id.flag |= LIB_EXTERN;

				}
			}
		}
		ob= ob->id.next;
	}
}


void append_named_part(SpaceFile *sfile, Main *main, char *name, int idcode)
{
	Object *ob;
	Base *base;
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
				
				id= is_yet_read(main, bhead);
				if(id==0) {
					read_libblock(main, bhead, LIB_TESTEXT);
				}
				else {
					printf("append: al ingelezen\n");
					add_lib_adr(bhead->old, id);
					if(id->flag & LIB_INDIRECT) {
						id->flag -= LIB_INDIRECT;
						id->flag |= LIB_EXTERN;
					}
				}
				
				if(idcode==ID_OB) {	/* los object: base geven */
					base= callocN( sizeof(Base), "app_nam_part");
					addtail(&(G.scene->base), base);
					
					if(id==0) ob= main->object.last;
					else ob= (Object *)id;
					
					base->lay= ob->lay;
					base->object= ob;
					ob->id.us++;
				}
				afbreek= 1;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
}

void append_id_part(char *filedata, Main *main, ID *id)
{
	BHead *bhead;
	ID *idread;
	int afbreek=0;
	char *fd;
	
	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code== GS(id->name)) {
			idread= (ID *)(bhead+1);
			
			if(strcmp(id->name, idread->name)==0) {
				id->flag -= LIB_READ;
				id->flag |= LIB_TEST;
				read_libblock(main, bhead, id->flag);
				afbreek= 1;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
}


void library_append(SpaceFile *sfile)	/* append aan G.scene */
{
	Main *main;
	VFont *vf;
	Object *ob;
	Curve *cu;
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
	if(strcmp(G.main->name, dir)==0) {
		error("Cannot use own file as library");
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
	
	/* mains maken */
	split_main();
	
	/* welke moeten wij hebben? */
	main= find_main(dir);

	idcode= groupname_to_code(group);
	
	if(totsel==0) {
		append_named_part(sfile, main, sfile->file, idcode);
	}
	else {
		for(a=0; a<sfile->totfile; a++) {
			if(sfile->filelist[a].flags & ACTIVE) {
				append_named_part(sfile, main, sfile->filelist[a].relname, idcode);
			}
		}
	}
	
	/* de main consistent maken */
	expand_main(main, sfile->libfiledata);
	
	/* als expand nog andere libs gevonden heeft: */
	read_libraries();
	
	lib_link_all(G.main);	

	/* losse objects aan G.scene hangen deze hebben nog een linkflag
	   moet na lib_link ivm gelinkte scenes (ob->us==0) */

	/* indirecte objects kunnen geen kwaad */
	/* als je deze terugzet, denk aan de 'need_link' flag: doe een find naar 'give_base_to' */
	/* give_base_to_objects(G.scene, &(G.main->object)); */



	/* VECTORFONTS */
	vf= G.main->vfont.first;
	while(vf) {
		if(vf->id.lib && vf->flag==1) reload_vfont(vf);
		vf= vf->id.next;
	}
	
	/* DISPLISTEN */
	ob= G.main->object.first;
	set_diplist_onlyzero(1);
	while(ob) {
		if(ob->id.lib) {
			if(ob->type==OB_FONT) {
				cu= ob->data;
				if(cu->nurb.first==0) text_to_curve(ob, 0);
			}
			makeDispList(ob);
		}
		else if(ob->type==OB_MESH && ob->parent && ob->parent->type==OB_LATTICE ) makeDispList(ob);
		
		ob= ob->id.next;
	}
	set_diplist_onlyzero(0);

	/* losslingerende blokken vrijgeven */
	add_data_adr(0, 0);
	add_lib_adr(0, 0);
	
	/* in sfile->dir staat de HELE libnaam */
	strcpy(G.lib, sfile->dir);
}

/* ************* READ LIBRARY ************** */

void read_libraries()
{
	Main *main;
	Library *lib;
	ID *id, *idn;
	ListBase *lbarray[30];
	int a, doit=1, tot, filelen;
	char *filedata;
	
	while(doit) {
		doit= 0;
		
		/* test 1: inlezen libdata */
		main= G.main->next;
		
		while(main) {

			disable_newlibadr= (void *)1;
		
			tot= count_mainblocks_flag(main, LIB_READ);
			if(tot) {
				filedata= main->curlib->filedata;
				if(filedata==0) {
					printf("read lib %s\n", main->curlib->name);
					filedata= openblenderfile(main->curlib->name, &filelen);
					main->curlib->filedata= filedata;
				}
				if(filedata) {
					doit= 1;
					a= set_listbasepointers(main, lbarray);
					while(a--) {
						id= lbarray[a]->first;
						while(id) {
							idn= id->next;
							if(id->flag & LIB_READ) {
								remlink(lbarray[a], id);
								
								append_id_part(filedata, main, id);
								/* printf("change adr: %s %x\n", id->name, disable_newlibadr); */
								if(disable_newlibadr==(void *)1) {
									printf("LIB ERROR: can't find %s\n", id->name);
									change_libadr(id, 0);
								}
								else change_libadr(id, disable_newlibadr);
								
								freeN(id);
							}
							id= idn;
						}
					}
				}
				else printf("ERROR: can't find lib %s \n", main->curlib->name);
			}
			disable_newlibadr= 0;

			expand_main(main, main->curlib->filedata);
			
			main= main->next;
		}
	}
	main= G.main->next;
	while(main) {
		if(main->curlib->filedata) freeN(main->curlib->filedata);
		main->curlib->filedata= 0;
		
		/* test of er libblocken niet zijn gelezen */
		a= set_listbasepointers(main, lbarray);
		while(a--) {
			id= lbarray[a]->first;
			while(id) {
				idn= id->next;
				if(id->flag & LIB_READ) {
					remlink(lbarray[a], id);
										
					printf("LIB ERROR: can't find %s\n", id->name);
					change_libadr(id, 0);
					
					freeN(id);
				}
				id= idn;
			}
		}
		
		/* sommige mains moeten nog worden ingelezen, dan is versionfile nog nul! */
		if(main->versionfile) do_versions(main);

		main= main->next;
	}
	
	join_main();
}


