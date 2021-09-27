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


/*           calcika.c    juni 93    */
/*						   aug 94	 */
/*									 */
/*									 */

#include "/usr/people/include/Trace.h"
#include <math.h>
#include <gl/gl.h>
#include <gl/device.h>

/* DOEN :
 * 
 * - als alles rechtopstaat (obj 2) klappert de quaternion om als je er 1 keer aan trekt.
 *   tijdelijk opgelost door y coordinaat van bovenste punt iets te veranderen
 * 
 * 
 * 
 */



ListBase bpbase={0, 0}, rodbase={0, 0}, jointbase={0, 0}, skelbase={0, 0};
extern short rodedit;


struct BodyPoint *addbodypoint()
{
	struct BodyPoint *p1;

	p1= callocN(sizeof(struct BodyPoint), "addbodypoint");
	addtail(&bpbase, p1);
	
	return p1;
}

void freebodypoint(struct BodyPoint *p1)
{

	remlink(&bpbase, p1);
	freeN(p1);
	
}

void freeBPlist(ListBase *lb)
{
	struct BodyPoint *bop, *next;
	
	bop= lb->first;
	while(bop) {
		next= bop->next;
		freebodypoint(bop);
		bop= next;
	}
}

struct Rod *addrod()
{
	struct Rod *rod;

	rod= callocN(sizeof(struct Rod), "addrod");
	addtail(&rodbase, rod);
	
	rod->drawwidth= 0.1*view0.grid;
	
	return rod;
}

void freerod(struct Rod *rod)
{

	if(rod->rdef) freeN(rod->rdef);
	
	remlink(&rodbase, rod);
	freeN(rod);
	
}

void freeRodlist(ListBase *lb)
{
	struct Rod *rod, *next;
	
	rod= lb->first;
	while(rod) {
		next= rod->next;
		freerod(rod);
		rod= next;
	}
}

struct Joint *addjoint()
{
	struct Joint *joint;

	joint= callocN(sizeof(struct Joint), "addjoint");
	addtail(&jointbase, joint);
	
	return joint;
}

void freejoint(joint)
struct Joint *joint;
{

	remlink(&jointbase, joint);
	freeN(joint);
	
}

void freeJointlist(ListBase *lb)
{
	struct Joint *joint, *next;
	
	joint= lb->first;
	while(joint) {
		next= joint->next;
		freejoint(joint);
		joint= next;
	}
}

struct Skeleton *addskeleton(int nr)
{
	struct Skeleton *skel;
	
	skel= callocN( sizeof(struct Skeleton)+4*nr, "addskel");
	skel->nr= nr;
	addtail(&skelbase, skel);
	return skel;
}

void freeskeleton(struct Skeleton *skel)
{
	remlink(&skelbase, skel);
	freeN(skel);
}

void freeSkellist(ListBase *lb)
{
	struct Skeleton *skel, *next;
	
	skel= lb->first;
	while(skel) {
		next= skel->next;
		freeskeleton(skel);
		skel= next;
	}
}


void addIka1()
{
	struct BodyPoint *p1, *p2, *p3, *p4;
	struct Rod *rod;
	struct Joint *joint;

	p1= addbodypoint();
	p1->r[0]= 0.0;
	p1->r[1]= -2.0;
	p1->r[2]= 1.0;
	p1->m= 1.0;
	
	p2= addbodypoint();
	p2->r[0]= 0.20*view0.grid;
	p2->r[1]= 0.0;
	p2->r[2]= 1.3*view0.grid;
	p2->m= 1.0;

	p3= addbodypoint();
	p3->r[0]= -1.01*view0.grid;
	p3->r[1]= -1.0;
	p3->r[2]= 2.0*view0.grid;
	p3->m= 1.0;

	p4= addbodypoint();
	p4->r[0]= -2.1*view0.grid;
	p4->r[1]= 1.0;
	p4->r[2]= 1.01*view0.grid;
	p4->m= 1.0;


	rod= addrod();
	rod->p1= p1;
	rod->p2= p2;
	init_rod(rod, 0.0);
	
	rod= addrod();
	rod->p1= p2;
	rod->p2= p3;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p3;
	rod->p2= p4;
	init_rod(rod, 0.0);

	joint= addjoint();
	joint->r1= rod->prev->prev;
	joint->r2= rod->prev;
	joint->p1= p1;
	joint->p2= p2;
	joint->p3= p3;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);

	joint= addjoint();
	joint->r1= rod->prev;
	joint->r2= rod;
	joint->p1= p2;
	joint->p2= p3;
	joint->p3= p4;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);

}


void deselectallpoints()
{
	struct BodyPoint *bop;
	
	bop= bpbase.first;
	while(bop) {
		bop->f= 0;
		bop= bop->next;
	}
}

struct BodyPoint *nearest_bodypoint(mval)
short *mval;
{
	struct BodyPoint *bop, *sel= 0;
	int mindist= 10000, x, y, test;

	bop= bpbase.first;
	while(bop) {
		x= bop->sx-mval[0];
		y= bop->sy-mval[1];
		test= x*x+y*y;
		if(test<mindist) {
			sel= bop;
			mindist= test;
		}
		bop= bop->next;
	}
	return sel;
}

struct BodyPoint *nearest_sel_bodypoint_ex(mval, bopnot)
short *mval;
struct BodyPoint *bopnot;
{
	struct BodyPoint *bop, *sel= 0;
	int mindist= 10000, x, y, test;

	bop= bpbase.first;
	while(bop) {
		if(bopnot!=bop) {
			if(bop->f & 1) {
				x= bop->sx-mval[0];
				y= bop->sy-mval[1];
				test= x*x+y*y;
				if(test<mindist) {
					sel= bop;
					mindist= test;
				}
			}
		}
		bop= bop->next;
	}
	return sel;
}

struct Rod *rod_has_bodypoints(p1, p2, p3, p4)
struct BodyPoint *p1, *p2, *p3, *p4;
{
	/* return rod die de aangegeven punten heeft. punten mogen '0' zijn */
	struct Rod *rod;
	int nr=0, verg;
	
	if(p1) nr++;	/* 'nr' moet de score zijn om rod te returnen */
	if(p2) nr++;
	if(p3) nr++;
	if(p4) nr++;
	
	rod= rodbase.first;
	while(rod) {
		verg= 0;
		if(p1) if(rod->p1==p1 || rod->p2==p1 || rod->p3==p1 || rod->p4==p1 ) verg++;
		if(p2) if(rod->p1==p2 || rod->p2==p2 || rod->p3==p2 || rod->p4==p2 ) verg++;
		if(p3) if(rod->p1==p3 || rod->p2==p3 || rod->p3==p3 || rod->p4==p3 ) verg++;
		if(p4) if(rod->p1==p4 || rod->p2==p4 || rod->p3==p4 || rod->p4==p4 ) verg++;
		
		if(verg==nr) return rod;
		
		rod= rod->next;
	}
	return 0;
}

void setflags_bodypoints(bop, flag)
struct BodyPoint *bop;
{
	while(bop) {
		bop->f= flag;
		bop= bop->next;
	}
}

int is_rod_selected(struct Rod *rod)
{
	if(rod->type==ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) return 1;
	}
	else if(rod->type==T_ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) if(rod->p3->f &1) return 1;
	}
	if(rod->type==H_ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) if(rod->p3->f &1) if(rod->p4->f &1) return 1;
	}
	
	return 0;
}

int is_joint_selected(struct Joint *joint)
{
	if( is_rod_selected(joint->r1) && is_rod_selected(joint->r2) )
		return 1;
	return 0;
}

int is_skel_selected(struct Skeleton *skel)
{
	struct Rod **rod;
	int nr;
	
	nr= skel->nr;
	rod= (struct Rod **)(skel+1);
	while(nr--) {
		if( is_rod_selected(*rod) == 0 ) return 0;
		rod++;
	}
	return 1;
}

int is_rod_OR_selected(struct Rod *rod)	/* min 1 point */
{
	if(rod->type==ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) return 1;
	}
	else if(rod->type==T_ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) if(rod->p3->f &1) return 1;
	}
	if(rod->type==H_ROD) {
		if(rod->p1->f &1) if(rod->p2->f &1) if(rod->p3->f &1) if(rod->p4->f &1) return 1;
	}
	
	return 0;
}


void vars_for_buttons()
{
	extern struct BodyPoint *bopsel;
	extern struct Rod *rodsel;
	extern struct Joint *jointsel;
	extern struct Skeleton *skelsel;
	struct BodyPoint *bop, *b1;
	struct Rod *rod, *r1;
	struct Joint *joint, *j1;
	
	/* onthoud */
	b1= bopsel;
	r1= rodsel;
	j1= jointsel;
	
	/* is er maar 1 bp selected: ok */
	bop= bpbase.first;
	bopsel= 0;
	while(bop) {
		if(bop->f & 1) {
			if(bopsel) {
				bopsel= 0;
				break;
			}
			else bopsel= bop;
		}
		bop= bop->next;
	}

	/* is er maar 1 rod selected: ok */
	rod= rodbase.first;
	rodsel= 0;
	while(rod) {
		if(is_rod_selected(rod)) {
			if(rodsel) {
				rodsel= 0;
				break;
			}
			else rodsel= rod;
		}
		rod= rod->next;
	}
	
	/* is er maar 1 joint selected: ok */
	joint= jointbase.first;
	jointsel= 0;
	while(joint) {
		if(is_joint_selected(joint)) {
			if(jointsel) {
				jointsel= 0;
				break;
			}
			else jointsel= joint;
		}
		joint= joint->next;
	}
	
	skelsel= 0;
	
	if(G.mainb==9) {
		if(b1!=bopsel || r1!=rodsel || j1!=jointsel) {
			tekeneditbuts(1);
		}
	}
	
}

void freeAllika()
{
	struct Rod *rod;
	
	freeBPlist(&bpbase);
	freeRodlist(&rodbase);
	freeJointlist(&jointbase);
	freeSkellist(&skelbase);
		
}

void delete_bodypoints_flag(int flag)
{
	struct Skeleton *skel, *nexts;
	struct BodyPoint *bop, *nextb;
	struct Rod *rod, *nextr, **rodpp;
	struct Joint *joint, *nextj;
	int a, ok;

	skel= skelbase.first;
	while(skel) {
		nexts= skel->next;
		
		/* is er minimaal 1 select? */
		ok= 0;

		a= skel->nr;
		rodpp= (struct Rod **)(skel+1);
		while(a--) {
			if( (*rodpp)->p1->f & flag) ok= 1;
			else if((*rodpp)->p2->f & flag) ok= 1;
			else if((*rodpp)->p3 && (*rodpp)->p3->f & flag) ok= 1;
			else if((*rodpp)->p4 && (*rodpp)->p4->f & flag) ok= 1;
			if(ok) break;
			rodpp++;
		}
		
		if(ok) freeskeleton(skel);
		
		skel= nexts;
	}

	joint= jointbase.first;
	while(joint) {
		nextj= joint->next;
		
		/* is er minimaal 1 select? */
		ok= 0;
		if(joint->p1->f & flag) ok= 1;
		else if(joint->p2->f & flag) ok= 1;
		else if(joint->p3->f & flag) ok= 1;
		else if(joint->p1a && joint->p1a->f & flag) ok= 1;
		else if(joint->p2a && joint->p2a->f & flag) ok= 1;
		else if(joint->p3a && joint->p3a->f & flag) ok= 1;
		
		if(ok) freejoint(joint);
		
		joint= nextj;
	}


	rod= rodbase.first;
	while(rod) {
		nextr= rod->next;
		
		/* is er minimaal 1 select? */
		ok= 0;
		if(rod->p1->f & flag) ok= 1;
		else if(rod->p2->f & flag) ok= 1;
		else if(rod->p3 && rod->p3->f & flag) ok= 1;
		else if(rod->p4 && rod->p4->f & flag) ok= 1;
		
		if(ok) freerod(rod);
		
		rod= nextr;
	}

	bop= bpbase.first;
	while(bop) {
		nextb= bop->next;
		
		if(bop->f & flag) freebodypoint(bop);
		
		bop= nextb;
	}
}

void delete_rods_flag(int flag)
{
	struct BodyPoint *bop, *nextb;
	struct Rod *rod, *nextr, **rodpp;
	struct Joint *joint, *nextj;
	struct Skeleton *skel, *nexts;
	int a, ok;
	
	skel= skelbase.first;
	while(skel) {
		nexts= skel->next;
		
		a= skel->nr;
		ok= 0;
		rodpp= (struct Rod **)(skel+1);
		while(a--) {
			if( is_rod_selected(*rodpp) ) {
				ok= 1;
				break;
			}
			rod++;
		}
		
		if(ok) freeskeleton(skel);
		
		skel= nexts;
	}
	
	joint= jointbase.first;
	while(joint) {
		nextj= joint->next;
		if( is_rod_selected(joint->r1) || is_rod_selected(joint->r2) ) freejoint(joint);
		joint= nextj;
	}
	
	rod= rodbase.first;
	while(rod) {
		nextr= rod->next;
		if( is_rod_selected(rod) ) freerod(rod);
		rod= nextr;
	}
	
	/* losslingerende punten weg */
	
	rod= rodbase.first;
	while(rod) {
		if(rod->p1->f & flag) rod->p1->f &= ~flag;
		if(rod->p2->f & flag) rod->p2->f &= ~flag;
		if(rod->p3 && (rod->p3->f & flag)) rod->p3->f &= ~flag;
		if(rod->p4 && (rod->p4->f & flag)) rod->p4->f &= ~flag;
		
		rod= rod->next;
	}
	
	bop= bpbase.first;
	while(bop) {
		nextb= bop->next;
		if(bop->f & flag) freebodypoint(bop);
		bop= nextb;
	}
}


void delete_joints_flag(int flag)
{
	struct Joint *joint, *nextj;
	
	joint= jointbase.first;
	while(joint) {
		nextj= joint->next;
		if( is_rod_selected(joint->r1) && is_rod_selected(joint->r2) ) freejoint(joint);
		joint= nextj;
	}
}

void delete_skels_flag(int flag)
{
	struct Skeleton *skel, *nexts;
	struct Rod **rodpp;
	int a, sel;
	
	skel= skelbase.first;
	while(skel) {
		nexts= skel->next;
		rodpp= (struct Rod **)(skel+1);
		sel==0;
		while(a--) {
			if(is_rod_selected(*rodpp)) sel++;
			else break;
			rodpp++;
		}
		if(sel==skel->nr) freeskeleton(skel);
		skel= nexts;
	}
}


void default_ika(struct IkaData *ika)
{
	/* om Base te initialiseren, niet in editmode */
	
	addIka1();
	
	calc_ika();
	
	ika->bpbase= bpbase;
	ika->rodbase= rodbase;
	ika->jointbase= jointbase;
	ika->skelbase= skelbase;
	
	bpbase.first= bpbase.last= 0;
	rodbase.first= rodbase.last= 0;
	jointbase.first= jointbase.last= 0;
	skelbase.first= skelbase.last= 0;
	
	ika->dt= 2;
}

int dupli_split_mode= 0;	/* 2 soorten duplicate! */

void add_dupli_ika(bpnew, rodnew, jointnew, skelnew, bpold, rodold, jointold, skelold)
ListBase *bpnew, *rodnew, *jointnew, *skelnew, *bpold, *rodold, *jointold, *skelold;
{
	/* dupliceerst al het geselecteerde */
	struct BodyPoint *bop, *lbop;
	struct Rod *rod, **rodpp, *lrod;
	struct Joint *joint, *jointn, *ljoint;
	struct Skeleton *skel, *skeln, *lskel;
	int a;
	
	/* van old pointers op nul */
	bop= bpold->first;
	while(bop) {
		bop->new= 0;
		bop->f &= ~2;	/* voor temp */
		bop= bop->next;
	}
	rod= rodold->first;
	while(rod) {
		rod->new= 0;
		rod= rod->next;
	}

	/* nieuwe punten */
	bop= bpold->first;
	lbop= bpold->last;
	while(bop) {
		if(bop->f & 1) {
			bop->new= mallocN(sizeof(struct BodyPoint), "duplika");
			memcpy(bop->new, bop, sizeof(struct BodyPoint));
			bop->new->new= 0;
			bop->new->f= 2;
			
			addtail(bpnew, bop->new);
			
		}
		if(bop==lbop) break;
		bop= bop->next;
	}
	
	/* nieuwe rods */
	rod= rodold->first;
	lrod= rodold->last;
	while(rod) {
		if( is_rod_selected(rod) ) {
			rod->new= mallocN(sizeof(struct Rod), "duplika");
			memcpy(rod->new, rod, sizeof(struct Rod));
			rod->new->new= 0;
			
			addtail(rodnew, rod->new);
			
			rod->new->p1= rod->p1->new;
			rod->new->p2= rod->p2->new;
			if(rod->p3) rod->new->p3= rod->p3->new;
			if(rod->p4) rod->new->p4= rod->p4->new;
			
			if(rod->rdef) {
				rod->new->rdef= mallocN(sizeof(struct RodDeform), "duplika");
				memcpy(rod->new->rdef, rod->rdef, sizeof(struct RodDeform));
			}
		}
		if(rod==lrod) break;
		rod= rod->next;
	}
	
	/* nieuwe joints */
	joint= jointold->first;
	ljoint= jointold->last;
	while(joint) {
		if( is_joint_selected(joint) ) {
			jointn= mallocN(sizeof(struct Joint), "duplika");
			memcpy(jointn, joint, sizeof(struct Joint));
			
			addtail(jointnew, jointn);
			
			jointn->r1= joint->r1->new;
			jointn->r2= joint->r2->new;
			jointn->p1= joint->p1->new;
			jointn->p2= joint->p2->new;
			jointn->p3= joint->p3->new;
			if(joint->p1a) jointn->p1a= joint->p1a->new;
			if(joint->p2a) jointn->p2a= joint->p2a->new;
			if(joint->p3a) jointn->p3a= joint->p3a->new;
			
		}
		if(joint==ljoint) break;
		joint= joint->next;
	}
	
	/* nieuwe skeletons */
	skel= skelold->first;
	lskel= skelold->last;
	while(skel) {
		if( is_skel_selected(skel) ) {
			skeln= mallocN( sizeof(struct Skeleton)+4*skel->nr, "dupliskel");
			memcpy(skeln, skel, sizeof(struct Skeleton)+4*skel->nr);

			addtail(skelnew, skeln);
	
			rodpp= (struct Rod **)(skeln+1);
			a= skeln->nr;
			while(a--) {
				*rodpp= (*rodpp)->new;
				rodpp++;
			}
		}
		if(skel==lskel) break;
		skel= skel->next;
	}
	
	/* niet netjes, werkt alleen als listbases op editmode staan */
	if(dupli_split_mode) {
		delete_rods_flag(1);
	}
	
	/* nieuwe selecteren */
	bop= bpold->first;
	while(bop) {
		if(bop->f & 1) bop->f= 0;
		if(bop->f == 2) bop->f= 1;
		bop= bop->next;
	}
	if(bpold!=bpnew) {
		bop= bpnew->first;
		while(bop) {
			bop->f= 1;
			bop= bop->prev;
		}
	}
	
}

void adduplicateIka()	/* extern in editmode, vanuit toets */
{

	add_dupli_ika(&bpbase, &rodbase, &jointbase, &skelbase, &bpbase, &rodbase, &jointbase, &skelbase);
	vertgrabber();
		
}


void muis_ika()
{
	struct BodyPoint *bop;
	short mval[2], xs, ys;
	
	/* welk punt wordt geselecteerd */
	getmouseco(mval);
	bop= nearest_bodypoint(mval);
	if(bop==0) return;
	
	if(G.qual & 3) {
		if(bop->f & 1) bop->f=0;
		else bop->f= 1;
	}
	else {
		setflags_bodypoints(bpbase.first, 0);
		bop->f= 1;
		
	}
	
	projektie();

	getmouseco(mval);
	xs= mval[0]; 
	ys= mval[1];
	while(getbutton(RIGHTMOUSE)) {
		gsync();
		getmouseco(mval);
		if(abs(mval[0]-xs)+abs(mval[1]-ys) > 4) {
			vertgrabber();
			while(getbutton(RIGHTMOUSE))  gsync();
		}
	}
	vars_for_buttons();
}


void make_ebaseIka()
{
	/* kopieer base naar edit data */
	struct IkaData *ika;
	struct Key *key;
	struct Rod *rod;
	int a;
	
	freeAllika();	/* voor zekerheid en reload! */

	ika= (struct IkaData *)G.ebase->d;
	key= ika->key;
	
	if(key && G.ipomode==22 && G.actkey) {
		a= 1;
		while(key) {
			if(a== G.actkey) break;
			a++;
			key= key->next;
		}
		if(key) {
			key_to_ika(key, ika);
		}
	}

	setflags_bodypoints(ika->bpbase.first, 1);
	add_dupli_ika(&bpbase, &rodbase, &jointbase, &skelbase, 
					  &ika->bpbase, &ika->rodbase, &ika->jointbase, &ika->skelbase);
	setflags_bodypoints(bpbase.first, 0);
	
	calc_ika();
	vars_for_buttons();
}

void load_ebaseIka()
{
	/* kopieer edit data naar base */
	struct IkaData *ika;
	struct Base *base;
	struct Key *key, *newk, *prevk;
	struct BodyPoint *bop;
	struct Rod *rod;
	int a, count, diffvert, len;
	
	ika= (struct IkaData *)G.ebase->d;
	key= ika->key;
	
	/* eerst tellen */
	count= diffvert= 0;
	bop= bpbase.first;
	while(bop) {
		count++;
		bop= bop->next;
	}
	if(ika->keyverts!=count) diffvert= 1;
	ika->keyverts= count;

	count= 0;
	rod= rodbase.first;
	while(rod) {
		count++;
		rod= rod->next;
	}
	if(ika->keyrods!=count) diffvert= 1;
	ika->keyrods= count;

	freeBPlist(&ika->bpbase);
	freeRodlist(&ika->rodbase);
	freeJointlist(&ika->jointbase);
	freeSkellist(&ika->skelbase);

	deselectallpoints();

	ika->bpbase= bpbase;
	ika->rodbase= rodbase;
	ika->jointbase= jointbase;
	ika->skelbase= skelbase;


	if(G.ipomode==22 && key && G.actkey) {
		if(diffvert) {
			/* allemaal nieuwe keyblokken maken */
			
			prevk= 0;
			len= sizeof(struct Key)+4*4*ika->keyverts+4*ika->keyrods;
			while(key) {
				newk= (struct Key *)mallocN(len, "loadebaseNurb");
				memcpy(newk, key, sizeof(struct Key));
				
				ika_to_key(ika, newk);
				
				freeN(key);

				if(prevk==0) {
					ika->key= newk;
				}
				else prevk->next= newk;
				prevk= newk;

				key= newk->next;
			}
		}
		else {

			a= 1;
			while(key) {
				if(a== G.actkey) break;
				a++;
				key= key->next;
			}
			if(key) {
				ika_to_key(ika, key);
			}
		}
		
		/* nu zit waarschijnlijk verkeerde key in ikadata: dus... */
		base= G.ebase;
		G.ebase= 0;	
		loadkeypos(base);
		G.ebase= base;
	}

	bpbase.first= bpbase.last= 0;
	rodbase.first= rodbase.last= 0;
	jointbase.first= jointbase.last= 0;
	skelbase.first= skelbase.last= 0;
	
	vars_for_buttons();
}

void addprimitiveIka(int prim)
{
	struct BodyPoint *p1, *p2, *p3, *p4;
	struct Rod *rod;
	float cent[3], pmat[4][4], tmat[4][4], cmat[3][3], mat[3][3], imat[3][3];
	
	parentbase(G.ebase,tmat,mat);
	Mat3CpyMat4(mat,tmat);

	VECCOPY(cent,view0.muis);
	cent[0]-= tmat[3][0];
	cent[1]-= tmat[3][1];
	cent[2]-= tmat[3][2];

	winset(G.winar[0]);
	getmatrix(pmat);
	Mat3CpyMat4(imat,pmat);
	Mat3MulVecfl(imat, cent);
	Mat3MulMat3(cmat, imat, mat);
	Mat3Inv(imat, cmat);

	setflags_bodypoints(bpbase.first, 0);

	if(prim==1) {	/* rod */
		p1= addbodypoint();
		VECCOPY(p1->r, cent);
		p1->r[0]-= 0.5*view0.grid;
		Mat3MulVecfl(imat, p1->r);
		p1->m= 1.0;
		p1->f= 1;
		
		p2= addbodypoint();
		VECCOPY(p2->r, cent);
		p2->r[0]+= 0.5*view0.grid;
		p2->m= 1.0;
		Mat3MulVecfl(imat, p2->r);
		p2->f= 1;
		
		rod= addrod();
		rod->p1= p1;
		rod->p2= p2;
		init_rod(rod, 0.0);
		
	}
	else if(prim==2) {	/* T rod */
		p1= addbodypoint();
		VECCOPY(p1->r, cent);
		p1->r[0]-= 0.5*view0.grid;
		Mat3MulVecfl(imat, p1->r);
		p1->m= 1.0;
		p1->f= 1;
		
		p2= addbodypoint();
		VECCOPY(p2->r, cent);
		p2->r[0]+= 0.5*view0.grid;
		p2->r[1]+= 0.2*view0.grid;
		p2->m= 1.0;
		Mat3MulVecfl(imat, p2->r);
		p2->f= 1;
		
		p3= addbodypoint();
		VECCOPY(p3->r, cent);
		p3->r[0]+= 0.5*view0.grid;
		p3->r[1]-= 0.2*view0.grid;
		p3->m= 1.0;
		Mat3MulVecfl(imat, p3->r);
		p3->f= 1;
		
		rod= addrod();
		rod->p1= p1;
		rod->p2= p2;
		rod->p3= p3;
		init_rod(rod, 0.0);
	}
	else if(prim==3) {	/* H rod */
		p1= addbodypoint();
		VECCOPY(p1->r, cent);
		p1->r[0]-= 0.5*view0.grid;
		p1->r[1]+= 0.2*view0.grid;
		Mat3MulVecfl(imat, p1->r);
		p1->m= 1.0;
		p1->f= 1;
		
		p2= addbodypoint();
		VECCOPY(p2->r, cent);
		p2->r[0]-= 0.5*view0.grid;
		p2->r[1]-= 0.2*view0.grid;
		p2->m= 1.0;
		Mat3MulVecfl(imat, p2->r);
		p2->f= 1;
		
		p3= addbodypoint();
		VECCOPY(p3->r, cent);
		p3->r[0]+= 0.5*view0.grid;
		p3->r[1]+= 0.2*view0.grid;
		p3->m= 1.0;
		Mat3MulVecfl(imat, p3->r);
		p3->f= 1;
		
		p4= addbodypoint();
		VECCOPY(p4->r, cent);
		p4->r[0]+= 0.5*view0.grid;
		p4->r[1]-= 0.2*view0.grid;
		p4->m= 1.0;
		Mat3MulVecfl(imat, p4->r);
		p4->f= 1;
		
		rod= addrod();
		rod->p1= p1;
		rod->p2= p2;
		rod->p3= p3;
		rod->p4= p4;
		init_rod(rod, 0.0);
		
	}
	else if(prim==4) {	/* Q rod */
		
	}
	else if(prim==5) {	/* chain */
		
	}
	
	projektie();
	vars_for_buttons();
}

void delIka()
{
	struct BodyPoint *bop;
	struct Rod *rod;
	struct Joint *joint;
	int event;
	
	event= pupmenu("ERASE %t|Points%x1|Rods%x2|Joints%x3|Skeletons%x4|All%x5");
	if(event== -1) return;

	if(event==1) {
		delete_bodypoints_flag(1);
	}
	else if(event==2) {
		delete_rods_flag(1);
	}
	else if(event==3) {
		delete_joints_flag(1);
	}
	else if(event==4) {
		delete_skels_flag(1);
	}
	else if(event==5) {
		freeAllika();
	}
	
	projektie();
	vars_for_buttons();
}

void splitIka()
{
	if(okee(" Split ")==0) return;

	dupli_split_mode= 1;
	add_dupli_ika(&bpbase, &rodbase, &jointbase, &skelbase, &bpbase, &rodbase, &jointbase, &skelbase);
	dupli_split_mode= 0;
	
	vars_for_buttons();
}

void connect_ika()
{
	struct BodyPoint *bop, *p1=0;
	struct Rod *rod;
	struct Joint *joint;
	
	if( okee("Connect")==0 ) return;

	/* pointers op nul */
	bop= bpbase.first;
	while(bop) {
		bop->new= 0;
		bop= bop->next;
	}

	/* zoek tweetallen punten bij elkaar */
	bop= bpbase.first;
	while(bop) {
		if(bop->f & 1) {
			p1= nearest_sel_bodypoint_ex(&(bop->sx), bop);
			if(p1) {
				if( rod_has_bodypoints(bop, p1, 0, 0) );	/* beide punten in 1 rod */
				else {
					VecMidf(bop->r, bop->r, p1->r);
					p1->new= bop;
					p1->f= 0;
				}
			}
			bop->f= 0;
		}
		bop= bop->next;
	}
	
	/* pointers weer goedzetten */
	rod= rodbase.first;
	while(rod) {
		if(rod->p1->new) rod->p1= rod->p1->new;
		if(rod->p2->new) rod->p2= rod->p2->new;
		if(rod->p3 && rod->p3->new) rod->p3= rod->p3->new;
		if(rod->p4 && rod->p4->new) rod->p4= rod->p4->new;
		
		rod= rod->next;
	}
	
	joint= jointbase.first;
	while(joint) {
		if(joint->p1->new) joint->p1= joint->p1->new;
		if(joint->p2->new) joint->p2= joint->p2->new;
		if(joint->p3->new) joint->p3= joint->p3->new;
		if(joint->p1a && joint->p1a->new) joint->p1a= joint->p1a->new;
		if(joint->p2a && joint->p2a->new) joint->p2a= joint->p2a->new;
		if(joint->p3a && joint->p3a->new) joint->p3a= joint->p3a->new;
		
		joint= joint->next;
	}
	
	/* verwijderen */
	bop= bpbase.first;
	while(bop) {
		p1= bop->next;
		if(bop->new) freebodypoint(bop);
		
		bop= p1;
	}
	
	calc_ika();
	if(rodedit==0) {
		rodedit= 1;
		calc_ika();	/* interne spanningen op nul */
		rodedit= 0;
	}
	projektie();
	vars_for_buttons();
}

void makejoint()
{
	struct Joint *joint;
	struct Rod *r1=0, *r2=0, *rod;
	struct BodyPoint *p1=0, *p2=0;
	
	/* testen of joint al bestaat */
	vars_for_buttons();
	if(jointsel) {
		if( okee("Init Joint")==0 ) return;
		
		init_joint(jointsel, 0.0, 0.0, 0.0, 0.0, 0);
		jointsel= 0;	/* forceer buttonredraw */
	}
	else {
		if( okee("Make Joint")==0 ) return;
		/* we gaan op zoek naar 2 rods met geselecteerde punten */
		rod= rodbase.first;
		while(rod) {
			if(is_rod_OR_selected(rod)) {
				if(r1==0) r1= rod;
				else if(r2==0) r2= rod;
				else {
					error("Indicate 2 connecting rods");
					return;
				}
			}
			rod= rod->next;
		}
		if(r2==0) {
			error("Indicate 2 connecting rods");
			return;
		}
		/* r1 en r2 moeten aansluiten */
		if(r1->p1==r2->p1 || r1->p1==r2->p2 || r1->p1==r2->p3 || r1->p1==r2->p4) p1= r1->p1;
		if(r1->p2==r2->p1 || r1->p2==r2->p2 || r1->p2==r2->p3 || r1->p2==r2->p4) {
			if(p1) p2= r1->p2; else p1= r1->p2;
		}
		if(r1->p3) if(r1->p3==r2->p1 || r1->p3==r2->p2 || r1->p3==r2->p3 || r1->p3==r2->p4) {
			if(p1) p2= r1->p3; else p1= r1->p3;
		}
		if(r1->p4) if(r1->p4==r2->p1 || r1->p4==r2->p2 || r1->p4==r2->p3 || r1->p4==r2->p4) {
			if(p1) p2= r1->p4; else p1= r1->p4;
		}
		if(p1==0) {
			error("Indicate 2 connecting rods");
			return;
		}
		joint= addjoint();
		joint->r1= r1;
		joint->r2= r2;
		joint->p2= p1;
		joint->p2a= p2;
		
		if(p1 && p2) {	/* dubbel scharnier, testen op illegale joints */
		
			/* eerst rod 1 */
			if(r1->type==T_ROD) {
				if(r1->p2==p1 || r1->p3==p1) {
					if(r1->p2==p2 || r1->p3==p2) {
						joint->p1= r1->p1;
					}
				}
			}
			else if(r1->type==H_ROD) {
				if(r1->p1==p1 || r1->p2==p1) {
					if(r1->p1==p2 || r1->p2==p2) {
						joint->p1= r1->p3;
						joint->p1a= r1->p4;
					}
				}
				else if(r1->p3==p1 || r1->p4==p1) {
					if(r1->p3==p2 || r1->p4==p2) {
						joint->p1= r1->p1;
						joint->p1a= r1->p2;
					}
				}
			}
			
			/* daarna rod 2 */
			if(r2->type==T_ROD) {
				if(r2->p2==p1 || r2->p3==p1) {
					if(r2->p2==p2 || r2->p3==p2) {
						joint->p3= r2->p1;
					}
				}
			}
			else if(r2->type==H_ROD) {
				if(r2->p1==p1 || r2->p2==p1) {
					if(r2->p1==p2 || r2->p2==p2) {
						joint->p3= r2->p3;
						joint->p3a= r2->p4;
					}
				}
				else if(r2->p3==p1 || r2->p4==p1) {
					if(r2->p3==p2 || r2->p4==p2) {
						joint->p3= r2->p1;
						joint->p3a= r2->p2;
					}
				}
			}
			
		}
		else {	/* enkel scharnier */
			
			/* eerst rod 1 */
			if(r1->type==ROD) {
				if(p1==r1->p1) joint->p1= r1->p2;
				else joint->p1= r1->p1;
			}
			else if(r1->type==T_ROD) {
				if(p1==r1->p1) {	/* aan de staart */
					joint->p1= r1->p2;
					joint->p1a= r1->p3;
				}
				else if(p1==r1->p2 || p1==r1->p3) {	/* aan de andere zijde */
					joint->p1= r1->p1;
				}
			}
			else if(r1->type==H_ROD) {
				if(p1==r1->p1 || p1==r1->p2) {
					joint->p1= r1->p3;
					joint->p1a= r1->p4;
				}
				else if(p1==r1->p3 || p1==r1->p4) {	
					joint->p1= r1->p1;
					joint->p1a= r1->p2;
				}
			}
			
			/* daarna rod 2 */
			if(r2->type==ROD) {
				if(p1==r2->p1) joint->p3= r2->p2;
				else joint->p3= r2->p1;
			}
			else if(r2->type==T_ROD) {
				if(p1==r2->p1) {	/* aan de staart */
					joint->p3= r2->p2;
					joint->p3a= r2->p3;
				}
				else if(p1==r2->p2 || p1==r2->p3) {	/* aan de andere zijde */
					joint->p3= r2->p1;
				}
			}
			else if(r2->type==H_ROD) {
				if(p1==r2->p1 || p1==r2->p2) {
					joint->p3= r2->p3;
					joint->p3a= r2->p4;
				}
				else if(p1==r2->p3 || p1==r2->p4) {	
					joint->p3= r2->p1;
					joint->p3a= r2->p2;
				}
			}
		}
		if(joint->p1==0 || joint->p3==0) {
			error("Illegal joint");
			freejoint(joint);
			return;
		}
		init_joint(joint, 0.0, 0.0, 0.0, 0.0, 0);
		
	}
	vars_for_buttons();
	projektie();
}

void makeskeleton()
{
	struct Rod *rod, **rodpp;
	struct Skeleton *skel;
	int nr=0;
	
	if( okee("Make Skeleton")==0 ) return;
	
	/* tellen welke rods geselecteerd */
	rod= rodbase.first;
	while(rod) {
		if(is_rod_selected(rod)) nr++;
		rod= rod->next;
	}
	if(nr>1) {
		skel= addskeleton(nr);
		rodpp= (struct Rod **)(skel+1);

		rod= rodbase.first;
		while(rod) {
			if(is_rod_selected(rod)) {
				*rodpp= rod;
				make_orig_rod(rod);
				rodpp++;
			}
			rod= rod->next;
		}
		
	}
	else error("Select min 2 rods");
}

void re_init_deform()
{
	struct Rod *rod;
	struct Joint *joint;
	
	rod= rodbase.first;
	while(rod) {
		if(is_rod_selected(rod)) make_orig_rod(rod);
		rod= rod->next;
	}

	
	/* zeer tijdelijk: alpha en def alpha */
	joint= jointbase.first;
	while(joint) {
		if(is_joint_selected(joint)) {
			joint->rt[1]= joint->rt[0];
		}
		joint= joint->next;
	}
		
}

void revealIka()
{
	struct BodyPoint *bop;
	
	bop= bpbase.first;
	while(bop) {
		if(bop->h) {
			bop->h= 0;
			bop->f= 1;
		}
		bop= bop->next;
	}

	vars_for_buttons();
	projektie();
}

void hideIka()
{
	struct BodyPoint *bop;
	
	bop= bpbase.first;
	while(bop) {
		if(bop->f & 1) {
			bop->h= 1;
		}
		bop= bop->next;
	}

	vars_for_buttons();
	projektie();
}

void selectswapIka()
{
	struct BodyPoint *bop;
	
	bop= bpbase.first;
	while(bop) {
		if(bop->h==0) {
			if(bop->f & 1) bop->f= 0;
			else bop->f= 1;
		}
		bop= bop->next;
	}
	
	vars_for_buttons();
	projektie();
}

void selectconnectedIka()
{
	struct Rod *rod;
	struct BodyPoint *bop;
	int cont=1, ok;
	short mval[2];
	
	/* alle hulpflags resetten */
	bop= bpbase.first;
	while(bop) {
		bop->f & ~1;
		bop= bop->next;
	}
	
	getmouseco(mval);
	bop= nearest_bodypoint(mval);
	if(bop==0) return;
	bop->f |= 2;
	
	/* floodfill */
	while(cont) {
		cont= 0;
		rod= rodbase.first;
		while(rod) {
			ok= 0;
			if(rod->p1->f & 2) ok++;
			if(rod->p2->f & 2) ok++;
			if(rod->p3 && rod->p3->f & 2) ok++;
			if(rod->p4 && rod->p4->f & 2) ok++;
			if(ok) {
				if( (rod->p1->f & 2)==0) {rod->p1->f |= 2; cont= 1;}
				if( (rod->p2->f & 2)==0) {rod->p2->f |= 2; cont= 1;}
				if( rod->p3 && (rod->p3->f & 2)==0) {rod->p3->f |= 2; cont= 1;}
				if( rod->p4 && (rod->p4->f & 2)==0) {rod->p4->f |= 2; cont= 1;}
			}
			rod= rod->next;
		}
	}
	
	/* flaggen */
	bop= bpbase.first;
	while(bop) {
		if(bop->f & 2) {
			bop->f -= 2;
			if(G.qual & 3) bop->f= 0;
			else bop->f |= 1;
		}
		bop= bop->next;
	}
	
	projektie();
}

void setlock_ika(int lock)
{
	struct BodyPoint *bop;

	bop= bpbase.first;
	while(bop) {
		if(bop->f & 1) bop->f1= lock;
		bop= bop->next;
	}
	
}

void setmass_ika(float mass)
{
	struct BodyPoint *bop;

	bop= bpbase.first;
	while(bop) {
		if(bop->f & 1) bop->m= mass;
		bop= bop->next;
	}
}

void setwidth_ika(short width)
{
	struct Rod *rod;
	
	rod= rodbase.first;
	while(rod) {
		if(is_rod_selected(rod)) rod->drawwidth= width;
		rod= rod->next;
	}
	projektie();
}

