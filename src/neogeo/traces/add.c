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

/*	add.c   */



/*	
short	isbase(base)
	delbase(base)
	deleteobj()
	wisalles

	wordruimte(ob)
	newbasename(base)
	maaksurf(ob,prim)
	*addbase(soort,prim)
	adduplicate()
*/


#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "/usr/people/include/Trace.h"

short isbase(base)
struct Base *base;
{
	struct Base *b;

	b=G.firstbase;
	while(b) {
		if(b==base) return 1;
		b=b->next;
	}
	return 0;
}

void delbase(base,lbase)
struct Base *base,*lbase;
{
	struct Base **tex;
	struct ObData *ob;
	struct ColBlck *col;
	struct LaData *la;
	struct VV *vv;
	struct PerfSph *ps;
	struct MoData *mo;
	struct PolyData *po;
	struct CurveData *cu;
	struct FontData *fo;
	struct IkaData *ika;
	struct Rod *rod;
	struct Bezier *bez;
	short colt,a,b;

	/* de imatusers zijn afgeschaft*/

	freekeys(base->key);
	freekeys(base->pkey);
	if(base->ipokey) freeN(base->ipokey);
	if(base->ipopkey) freeN(base->ipopkey);
	if(base->iponoise) freeN(base->iponoise);

	if(base->soort==1) {
		ob=(struct ObData *)base->d;
		vv=ob->vv;
		vv->us--;

		if(vv->us==0) {
			freekeys(vv->key);
			freeN(vv);
		}
		else if(vv->us<0) printf("error: users %s %d\n", base->str, vv->us);

		if(ob->ipovvkey) freeN(ob->ipovvkey);
		freelistN(&(ob->disp));
		if(ob->floatverts) freeN(ob->floatverts);
		freeN(ob);
		G.totobj--;
	}
	else if(base->soort==2) {
		la=(struct LaData *)base->d;
		freeN(la);
		G.totlamp--;
	}
	else if(base->soort==3) {
		ps=(struct PerfSph *)base->d;
		freeN(ps);
		G.totobj--;
	}
	else if(base->soort==7) {
		ob=(struct ObData *)base->d;
		po= ob->po;
		po->us--;
		if(po->us<1) {
			if(po->f45) freeN(po->f45);
			freeN(po);
		}
		freeN(ob);
		G.totobj--;
	}
	else if(base->soort==9) {
		/* po en fo hebben geen duplicates */
		ob=(struct ObData *)base->d;
		po= ob->po;
		if(po) {
			if(ob->po->f45) freeN(ob->po->f45);
			freeN(ob->po);
		}
		fo= ob->fo;
		if(fo) {
			freevfont(fo->vf);
			if(fo->str) freeN(fo->str);
			/*OUD! if(fo->bez) freeN(fo->bez); */
			freeN(fo);
		}
		freeN(ob);
		G.totobj--;
	}
	else if(base->soort==5 || base->soort==11) {
		ob=(struct ObData *)base->d;
		cu= ob->cu;
		freeNurblist(&(cu->curve));
		freelistN(&(cu->disp));
		if(base->soort==11) freelistN(&(cu->bev));
		if(cu->key) freekeys(cu->key);
		if(ob->ipovvkey) freeN(ob->ipovvkey);
		if(ob->floatverts) freeN(ob->floatverts);
		freeN(cu);
		freeN(ob);
		G.totobj--;
	}
	else if(base->soort== -2) {
		mo=(struct MoData *)base->d;
		freeNurblist(&(mo->curve));
		freelistN(&(mo->disp));
		if(mo->ipotra) freeN(mo->ipotra);
		if(mo->ipoqfi) freeN(mo->ipoqfi);
		if(mo->data) freeN(mo->data);
		if(mo->key) freekeys(mo->key);
		if(mo->ipovkey) freeN(mo->ipovkey);
		freeN(mo);
		G.totmove--;
	}
	else if(base->soort== -4) {
		ika= (struct IkaData *)base->d;
		
		freeBPlist(&(ika->bpbase));
		freeRodlist(&(ika->rodbase));
		freeJointlist(&(ika->jointbase));
		freeSkellist(&(ika->skelbase));
		
		if(ika->key) freekeys(ika->key);
		if(ika->ipokey) freeN(ika->ipokey);

		freeN(ika);
		
		G.totmove--;
	}
	
	if(base==view0.opar) view0.opar=0;
	if(base==view0.tpar) view0.tpar=0;

	if(base==G.ebase) {
		if(G.edve.first) freelist(&G.edve.first);
		if(G.eded.first) freelist(&G.eded.first);
		if(G.edvl.first) freelist(&G.edvl.first);
		G.ebase=0;
	}

	if(base!=G.firstbase) lbase->next = base->next;
	else G.firstbase= base->next;
	if(base==G.lastbase) {
		if(lbase) {
			lbase->next=0;
			G.lastbase=lbase;
		}
		else {
			G.lastbase=0;
			G.firstbase=0;
		}
	}
	if(G.firstbase==0) G.lastbase=0;
	freeN(base);
}

void deleteobj()
{
	struct Base **tex,*lbase,*base;
	struct View *v;
	struct Key *key;
	struct ObData *ob;
	struct ColBlck *col;
	struct LaData *la;
	struct PerfSph *ps;
	long *temp;
	short colt,a,b;

	if(okee("ERASE SELECTED")==0) return;

	base=G.firstbase;
	lbase=0;
	while(base) {
		if( (base->lay & view0.lay)!=0 && (base->f & 1) )
			delbase(base,lbase);
		else lbase=base;
		base=base->next;
	}

	base=G.firstbase;
	while(base) {
		if(base->p) if(isbase(base->p)==0) base->p=0;
		if(base->t) if(isbase(base->t)==0) base->t=0;
		if(base->pkey) {
			key= base->pkey;
			a= 1;
			while(key) {
				temp= (long *)(key+1);
				if(*temp && isbase(*temp)==0) {
					deletekeypos_spec(&(base->pkey), a);
				}
				a++;
				key= key->next;
			}
		}
		/* warp */
		if((base->soort & 1)!=0 || base->soort==2) {
			if ELEM5(base->soort, 1, 5, 7, 9, 11) {
				ob=(struct ObData *)base->d;
				if(base->soort==11) {
					if(ob->cu->bevbase) {
						if(isbase(ob->cu->bevbase)==0) ob->cu->bevbase= 0;
					}
				}
				col=(struct ColBlck *)(ob+1);
				tex=col->base;
				colt=ob->c;
			}
			else if(base->soort==2) {
				la=(struct LaData *)base->d;
				tex=la->base;
				colt=1;
			}
			else {
				ps=(struct PerfSph *)base->d;
				col= &(ps->c);
				tex=col->base;
				colt=1;
			}
			for(a=0;a<colt;a++) {
				for(b=0;b<4;b++) {
					if(tex[b]) if(isbase(tex[b])==0)
						tex[b]=0;
				}
				if(colt>1) {
					col++;
					tex=col->base;
				}
			}
		}
		base=base->next;
	}

	tex=wrld.sbase;
	for(b=0;b<4;b++) {
		if(tex[b]) if(isbase(tex[b])==0)
			tex[b]=0;
	}

	v= &view0;
	while(v) {
		if(isbase(v->opar)==0) v->opar= 0;
		if(isbase(v->tpar)==0) v->tpar= 0;
		if(isbase(v->cambase)==0) v->cambase= 0;
		v= v->next;
	}

	/* G.mainb=1; G.mainbo=1; */
	G.basact=0;
	tekenmainbuts(0);
	countall();
	projektie();
}

void wisalles(ok)
short ok;
{
	extern struct ListBase editNurb;
	struct View *v, *nextv;
	struct Base *lbase,*base;
	short okee(),a;

	if(ok) if(okee("ERASE ALL")==0) return;

	base=G.firstbase;
	lbase=0;
	while(base) {
		delbase(base,lbase);
		lbase=base;
		base=base->next;
	}
	G.basact=0;

	freeNurblist(&editNurb);

	for(a=1;a<=G.totex;a++) {
		if(G.adrtex[a]->soort==5) delImap(G.adrtex[a]);
		freeN(G.adrtex[a]);
		G.adrtex[a]=0;
	}
	G.totex=0;
	if(view0.next) {
		v=view0.next;
		while(v) {
			nextv= v->next;
			freeN(v);
			v= nextv;
		}
	}
	view0.next=0;
	view0.opar= 0;
	view0.tpar= 0;
	view0.cambase= 0;

	if(G.timeipo) freeN(G.timeipo);
	G.timeipo= 0;

	for(a=0;a<4;a++) {
		wrld.stex[a]=0;
		wrld.sbase[a]=0;
	}
	countall();

	if(G.winar[0]) {
		if(G.mainb!=1) {
			G.mainb=1; 
			G.mainbo=1;
			tekenmainbuts(0);
		}
		else SetButs(100, 199);
	}
}

void newbasename(base)
struct Base *base;
{
	struct Base *b;
	short nr=0;

	b=G.firstbase;
	while(b) {
		if(b!=base) if(strncmp(b->str,base->str,14)==0)
			if(nr<= b->nnr) nr= b->nnr+1;
		b=b->next;
	}
	base->nnr=nr;
}

void wordruimte(vv)
struct VV *vv;
{
	struct VertOb *adrve;
	long max, a, v, *temp;
	float fac;

	max=MAX3(vv->afm[0],vv->afm[1],vv->afm[2]);
	fac=32760.0/max;
	vv->ws= 1/fac;

	v=vv->vert;
	adrve=(struct VertOb *)(vv+1);
	for(a=0;a<v;a++) {
		temp=(long *)adrve;
		adrve->c[0] = fac* *(temp++);
		adrve->c[1] = fac* *(temp++);
		adrve->c[2] = fac* *(temp++);
		adrve++;
	}
}

void maaksurf(ob,prim)
struct ObData *ob;
short prim;
{
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	long *temp;
	short len;
	short x,a,c;

	x=view0.grid;

	if(prim==0) len=32+16*2+12*4;
	if(prim==1) len=32+16*12+12*8;
	ob->vv=(struct VV *)callocN(len,"maaksurf1");
	/* printf("vv: %d\n",ob->vv); */
	adrve=(struct VertOb *)(ob->vv +1);
	/* printf("adrve: %d\n",adrve); */
	if(prim==0) {
		ob->vv->vert=4;
		ob->vv->vlak=2;
		adrvl=(struct VlakOb *)(adrve+4);
	}
	else {
		ob->vv->vert=8;
		ob->vv->vlak=12;
		adrvl=(struct VlakOb *)(adrve+8);
	}
	/* printf("adrvl: %d\n",adrvl); */
	ob->vv->us=1;  /* users */
	ob->vv->afm[0]=x; 
	ob->vv->afm[1]=x;
	if(prim==1) {
		ob->vv->afm[2]=x;
	}
	/* de vertices  */
	c=4;
	if(prim==1) c=8;
	temp=(long *)adrve;

	for(a=1;a<=c;a++) {
		switch(a) {
		case 1: 
		case 2: 
		case 5: 
		case 6:
			*temp= -x; 
			break;
		case 3: 
		case 4: 
		case 7: 
		case 8:
			*temp=  x; 
			break;
		}
		temp++;
		switch(a) {
		case 2: 
		case 3: 
		case 6: 
		case 7:
			*temp= -x; 
			break;
		case 1: 
		case 4: 
		case 5: 
		case 8:
			*temp=  x; 
			break;
		}
		temp++;
		if(prim==1) {
			switch(a) {
			case 1: 
			case 2: 
			case 3: 
			case 4:
				*temp= -x; 
				break;
			case 5: 
			case 6: 
			case 7: 
			case 8:
				*temp=  x; 
				break;
			}
		}
		temp++;
	}
	wordruimte(ob->vv);

	/* de vlakken */
	c=0;
	for(a=0;a<=prim;a++) {
		adrvl->v1=1+4*a;
		adrvl->v2=2+4*a;
		adrvl->v3=3+4*a;
		adrvl->n[2]= -32767;
		if(prim && a==0) {
			adrvl->v1=3+4*a;
			adrvl->v3=1+4*a;
		}
		adrvl++;
		adrvl->v1=3+4*a;
		adrvl->v2=4*a;
		adrvl->v3=1+4*a;
		adrvl->n[2]= -32767;
		if(prim && a==0) {
			adrvl->v1=1+4*a;
			adrvl->v3=3+4*a;
		}
		adrvl++;
	}
	if(prim==1) {
		for(a=0;a<=2;a++) {
			adrvl->v1=a;
			adrvl->v2=a+1;
			adrvl->v3=a+4;
			adrvl++;
			adrvl->v1=a+1;
			adrvl->v2=a+5;
			adrvl->v3=a+4;
			adrvl++;
		}
		adrvl->v1=3;
		adrvl->v2=0;
		adrvl->v3=4;
		adrvl++;
		adrvl->v1=3;
		adrvl->v2=4;
		adrvl->v3=7;
		adrvl=(struct VlakOb *)(adrve+8);
		normalen(adrve,adrvl,8,12);
	}
}

void initcolblocks(col, colt)
struct ColBlck *col;
short colt;
{
	long *temp;
	short a;

	for(a=0;a<colt;a++) {
		temp=(long *)col;
		if(a!=0) *temp= *(temp-16);
		*(temp+1)=0xC8C83200;
		*(temp+2)=0xF0F0;
		*(temp+3)=0xF0F0F0F0;
		*(temp+4)=0xF0000000;
		*(temp+8)=0x9000000;
		*(temp+9)=0x9000000;
		*(temp+10)=0x9000000;
		*(temp+11)=0x9000000;
		col++;
	}
}

struct Base *addbase(soort)	/* wordt aangeroepen in toolbox.c */
short soort;
{
	extern short vfontbut;
	struct Base *base, *lbase=0;
	struct ObData *ob;
	struct LaData *la;
	struct PerfSph *ps;
	struct ColBlck *col;
	struct PolyData *po;
	struct CurveData *cu;
	struct IkaData *ika;
	struct Nurb *nu, *addNurbprim();
	struct MoData *mo;
	struct Bezier *bez;
	struct FontData *fo;
	struct VectorFont *initvfont(),*give_vfontpointer();
	long *temp, tempmuis[3];
	float *tfl, q1[4], q2[4], fac, co, si, hoek;
	short prim=0,a,colt;

	if(soort==9 && G.ebase!=0) {
		error("Not in editbase mode!");
		return 0;
	}

	if(soort==7) return;

	if(soort) {
		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				base->f -= 1;
			}
			base= base->next;
		}
	}

	if(G.lastbase) {
		base=(struct Base *)callocN(256,"addbase1");
		if(base) {
			G.lastbase->next=base;
			lbase= G.lastbase;
			G.lastbase=base;
		}
	}
	else {
		G.firstbase=(struct Base *)callocN(256,"addbase2");
		G.lastbase=G.firstbase;
		base=G.firstbase;
	}

	if(base==0) error("Not enough memory");
	else if(soort) {		/* soort=0 is alleen base aanmaken*/
		if(soort==91) {		/* komt uit verdit.c */
			soort=1;
			prim=1;
		}
		/* berekening quat */
		co= cosf(0.5*(view0.theta));
		si= sinf(0.5*(view0.theta));
		q1[0]= co; 
		q1[1]= q1[2]= 0.0; 
		q1[3]= si;
		co= cosf(0.5*(view0.phi));
		si= sinf(0.5*(view0.phi));
		q2[0]= co; 
		q2[2]= q2[3]= 0.0; 
		q2[1]= si;
		QuatMul(base->q, q1, q2);

		base->soort=soort;
		base->f= 9;
		base->f1= 2+8+16+64;	/* als dit verandert, ook clearworkbase doen (edit.c) */
		VECCOPY(base->v,view0.muis);
		base->lay= 1<<(G.layact-1);
		if(G.localview) base->lay |= 32768;
		base->sf= 1;
		base->len= G.frs;
		base->f2= 0;
		base->tflag= 1;
		base->upflag= 2;

		base->m[0][0]=1.0;
		base->m[1][1]=1.0;
		base->m[2][2]=1.0;
		VECCOPY(base->r,view0.muis);
		if(soort==1) {
			colt=1;
			base->d=(long *)callocN(80+sizeof(struct ColBlck),"addbase3");
			ob=(struct ObData *)base->d;
			ob->c=1; 
			ob->dt=2;
			col=(struct ColBlck *)(ob+1);
			col->r=240; 
			col->g=240; 
			col->b=240; 
			col->mode=12;
			maaksurf(ob,prim);
			if(prim==0) strcpy(base->str,"Plane");
			if(prim==1) strcpy(base->str,"Cube");
			newbasename(base);
			initcolblocks(col, colt);
		}
		else if(soort==2) {
			base->d=(long *)callocN(72,"addbase4");
			la=(struct LaData *)base->d;
			la->ld= 20*(long)view0.grid;
			la->r=la->g=la->b=la->ld2= 255;
			la->haint= 128;
			la->f=1;
			la->energy= 1.0;
			la->map[0]=0x104;
			la->map[1]=0x104;
			la->map[2]=0x104;
			la->map[3]=0x104;
			la->spsi= 150;
			la->bufsize= 512;
			la->clipsta= la->ld/10000;
			la->clipend= la->ld/500;
			la->samp= 3; 
			la->bias= 10;
			la->soft= 3.0;
			strcpy(base->str,"Lamp");
			newbasename(base);
			base->tflag= 5;
			base->upflag= 1;
			if(G.zbuf) reshadeall();
		}
		else if(soort==3) {
			colt=1;
			base->d=(long *)callocN(96,"addbase5");
			ps=(struct PerfSph *)base->d;
			ps->rad= view0.grid;
			ps->radf=view0.grid;
			col= &(ps->c);
			col->r=240; 
			col->g=50; 
			col->b=50; 
			col->mode=13;
			initcolblocks(col, colt);
			strcpy(base->str,"Sphere");
			newbasename(base);
		}
		else if(soort==4) {	/* cambase */
			strcpy(base->str,"Camera");
			newbasename(base);
			base->tflag= 5;
			base->upflag= 1;
		}
		else if(soort==5) {	/* nurbs surf */
			colt= 1;
			base->d= (long *)callocN(80+sizeof(struct ColBlck),"addbase9");
			ob=(struct ObData *)base->d;
			ob->c=1; 
			ob->dt=2;
			col=(struct ColBlck *)(ob+1);
			col->r=255; 
			col->g=255; 
			col->b=255; 
			col->mode=12;

			ob->cu= callocstructN(struct CurveData, 1, "addbase10");
			cu= ob->cu;
			cu->us= 1;
			cu->ws= 1.0;
			cu->width= 100;
			cu->flag= 255;

			G.ebase= base;			/* voor addNurb */
			nu= addNurbprim(4, 4);	/* 4= 3D nurb, 4= sphere */
			G.ebase= 0;
			addtail(&(cu->curve), nu);
			testWspace(base);

			strcpy(base->str,"Nurbs");
			newbasename(base);
			initcolblocks(col, colt);
		}
		else if(soort==7) {
		}
		else if(soort==9) {	/* font */
			colt=3;
			base->d=(long *)callocN(80+3*sizeof(struct ColBlck),"addbase6");
			ob=(struct ObData *)base->d;
			ob->c=3; 
			ob->dt=2;
			col=(struct ColBlck *)(ob+1);
			col->r=255; 
			col->g=255; 
			col->b=255; 
			col->mode=12;
			initcolblocks(col, colt);
			fo= ob->fo= callocstructN(struct FontData,1,"fontdata");

			fo->vf= give_vfontpointer(vfontbut);
			if(fo->vf==0) {
				error("Can't open font");
				delbase(base,lbase);
				return 0;
			}

			fo->vf->us++;
			fo->str= mallocN(8,"fontstring");
			strcpy(fo->str,"Text");
			fo->len= 8;
			fo->lines=  fo->set[2]= 1;
			fo->set[5]= fo->set[3]= 10;
			makepolytext(base);
			strcpy(base->str,"Text");
			newbasename(base);
			a= 0;
			if(G.basact!=0 && G.basact->soort==9) a= 1;
			G.ebase= G.basact= base;
			if(G.mainb==9) tekeneditbuts(a);
			else tekenmainbuts(2);
			textedit();
		}
		else if(soort== 11) {	/* curve */
			colt=3;
			base->d= (long *)callocN(80+3*sizeof(struct ColBlck),"addbase9");
			ob=(struct ObData *)base->d;
			ob->c=3; 
			ob->dt=2;
			col=(struct ColBlck *)(ob+1);
			col->r=255; 
			col->g=255; 
			col->b=255; 
			col->mode=12;
			ob->cu= callocstructN(struct CurveData, 1, "addbase10");
			cu= ob->cu;
			cu->ws= 1.0;
			cu->us= 1;
			cu->width= 100;
			cu->flag= 255;

			VECCOPY(tempmuis, view0.muis);	/* patch addNurbprim */
			view0.muis[0]= view0.muis[1]= view0.muis[2]= 0;

			nu= addNurbprim(9, 0);	/* 9= 2D bezier, 0= curve */
			addtail(&(cu->curve), nu);
			
			testWspace(base);

			VECCOPY(view0.muis, tempmuis);

			makeBevelList(base);

			strcpy(base->str,"Curve");
			newbasename(base);
			initcolblocks(col, colt);
		}
		else if(soort== -2) {
			base->d=(long *)callocN(sizeof(struct MoData),"addbase8");
			mo=(struct MoData *)base->d;
			mo->fr=G.frs;
			mo->f=1;
			mo->n[2]= 32767;

			VECCOPY(tempmuis, view0.muis);	/* patch voor addNurbprim */
			view0.muis[0]= view0.muis[1]= view0.muis[2]= 0;

			nu= addNurbprim(4, 6);	/* 4= 3D nurb, 6= 5 pnts pad */
			nu->resolu= 64;
			addtail(&(mo->curve), nu);
			makeDispList(base);  /* doet ook berekenpad */
			
			VECCOPY(view0.muis, tempmuis);

			strcpy(base->str,"Move");
			newbasename(base);
		}
		else if(soort== -4) {	
			base->d= callocN(sizeof(struct IkaData),"addbaseIka");
			ika= (struct IkaData *)base->d;

			default_ika(ika);
			
			strcpy(base->str,"Ika");
			newbasename(base);
		}

		lbase= G.basact;
		G.basact= base;

		makeDispList(base);

		countall();
		projektie();

		/* buttons */
		if(G.mainb==5) {
			if( (base->soort & 1)==0 ) {
				G.mainb= G.mainbo= 1;
				tekenmainbuts(0);
			}
			else tekenmainbuts(2);
		}
		else if(G.mainb==4) {
			if(base->soort!=2) {
				G.mainb= G.mainbo= 1;
				tekenmainbuts(0);
			}
			else tekenmainbuts(2);
		}
		else if(G.mainb==9) {
			if(lbase && lbase->soort!=G.basact->soort)
				tekenmainbuts(0);
			else tekenmainbuts(2);
		}
		else tekenmainbuts(2);

	}
	return base;
}


void adduplicate()
{
	struct Base *base,*b,*lbase,**ablock,**abl;
	struct ObData *ob, *obn;
	struct FontData *fo;
	struct LaData *la;
	struct Key *key, *prevk, *keyn;
	struct PerfSph *ps;
	struct MoData *mo,*mo1;
	struct PolyData *po;
	struct CurveData *cu;
	struct IkaData *ika, *ika1;
	struct Nurb *nu, *new, *duplicateNurb();
	struct Bezier *bez;
	short len,totdup=0,a;
	char *str;

	/* eerst aantal tellen voor ablock pointers */
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) totdup++;
		base= base->next;
	}

	if(totdup==0) return;
	ablock= abl= (struct Base **)callocN(totdup*2*4,"addupablock");

	b=G.firstbase;
	lbase=G.lastbase;
	while(b) {
		if(TESTBASE(b)) {
			*(abl++)= b;
			base= addbase(0);
			*(abl++)= base;
			if(base==0) break;

			/* de gedupliceerde wordt desel, dus basact= new base */
			G.basact= base;
			memcpy(base,b,sizeof(struct Base));
			base->next=0;
			b->f--;

			base->key=0; 
			base->ipokey=0;
			base->pkey=0; 
			base->ipopkey=0;
			base->iponoise=0;
			
			if(b->key) {
				len= sizeof(struct Key)+4*(3+3+4+9);

				key= b->key;
				prevk= 0;
				while(key) {
					keyn= (struct Key *)mallocN(len, "copymenuKey");
					memcpy(keyn, key, len);

					if(prevk==0) base->key= keyn;
					else prevk->next= keyn;
					prevk= keyn;

					key= key->next;
				}

			}
			if(b->ipokey) {
				bez= b->ipokey;
				len= sizeof(struct Bezier)+bez->cp*36;
				base->ipokey=(struct Bezier *)mallocN(len,"addupbasei");
				memcpy(base->ipokey, b->ipokey, len);
			}
			if(b->iponoise) {
				bez= b->iponoise;
				len= sizeof(struct Bezier)+bez->cp*36;
				base->iponoise= (struct Bezier *)mallocN(len,"addupbaseii");
				memcpy(base->iponoise, b->iponoise, len);
			}
			
			if(b->soort & 1) G.totobj++;
			else if(b->soort==2) G.totlamp++;
			else if(b->soort<0) G.totmove++;

			if(b->soort==1 || b->soort==7) {
				ob=(struct ObData *)b->d;
				len=sizeof(struct ObData) + ob->c * sizeof(struct ColBlck);
				base->d= (long *)mallocN(len,"addupbase1");
				memcpy(base->d,ob,len);
				if(b->soort==1) ob->vv->us++;
				if(b->soort==7) ob->po->us++;
				obn= (struct ObData *)base->d;
				if(ob->ipovvkey) {
					bez= ob->ipovvkey;
					len= sizeof(struct Bezier)+bez->cp*36;
					obn->ipovvkey= (struct Bezier *)mallocN(len,"addupipovvkey");
					memcpy(obn->ipovvkey,bez,len);
				}
				obn->floatverts= 0;
				obn->disp.first= obn->disp.last= 0;
			}
			else if(b->soort== 2) {
				la=(struct LaData *)b->d;
				len=sizeof(struct LaData);
				base->d=(long *)mallocN(len,"addupbase2");
				memcpy(base->d,la,len);
			}
			else if(b->soort== 3) {
				ps=(struct PerfSph *)b->d;
				len=sizeof(struct PerfSph);
				base->d=(long *)mallocN(len,"addupbase3");
				memcpy(base->d,ps,len);
			}
			else if(b->soort== 9) {
				ob=(struct ObData *)b->d;
				len=sizeof(struct ObData) + ob->c * sizeof(struct ColBlck);
				base->d=(long *)mallocN(len,"addupbase6");
				memcpy(base->d,ob,len);
				fo= callocstructN(struct FontData,1,"fontdata");
				memcpy(fo,ob->fo,sizeof(struct FontData));
				fo->vf->us++;
				str= callocN(fo->len+1,"addupstring");
				strcpy(str,ob->fo->str);
				fo->str= str;
				po= callocstructN(struct PolyData,1,"polydata");
				memcpy(po,ob->po,sizeof(struct PolyData));
				po->f45= 0;
				ob= (struct ObData *)base->d;
				ob->disp.first= ob->disp.last= 0;
				ob->po= po;
				ob->fo= fo;
				makepolytext(base);
				extrudepoly(base);
			}
			else if ELEM(b->soort, 5, 11) {
				ob= (struct ObData *)b->d;
				len= sizeof(struct ObData) + ob->c * sizeof(struct ColBlck);
				base->d= (long *)mallocN(len,"addupbase6");
				memcpy(base->d, ob, len);
				cu= callocstructN(struct CurveData,1,"curvedata");
				memcpy(cu, ob->cu, sizeof(struct CurveData));
				nu= cu->curve.first;
				cu->curve.first= cu->curve.last= 0;
				while(nu) {
					new= duplicateNurb(nu);
					addtail(&(cu->curve), new);
					nu= nu->next;
				}
				obn= (struct ObData *)base->d;
				obn->floatverts= 0;
				obn->cu= cu;
				cu->disp.first= cu->disp.last= 0;
				cu->bev.first= cu->bev.last= 0;
				
				if(ob->cu->key) {
					key= ob->cu->key;
					prevk= 0;
					len= sizeof(struct Key) +4 +12*ob->cu->keyverts;
					while(key) {
						keyn= (struct Key *)mallocN(len, "copymenuKey");
						memcpy(keyn, key, len);

						if(prevk==0) ob->cu->key= keyn;
						else prevk->next= keyn;
						prevk= keyn;

						key= key->next;
					}
				}

				if(ob->ipovvkey) {
					bez= ob->ipovvkey;
					len= sizeof(struct Bezier)+bez->cp*36;
					obn->ipovvkey= (struct Bezier *)mallocN(len,"addupipovvkey");
					memcpy(obn->ipovvkey, bez, len);
				}
				
				if(b->soort==11) makeBevelList(base);
			}
			else if(b->soort== -2) {
				mo= (struct MoData *)b->d;
				len= sizeof(struct MoData);
				base->d= (long *)mallocN(len,"addupbase-2");
				memcpy(base->d, mo, len);
				mo1= (struct MoData *)base->d;

				nu= mo->curve.first;
				mo1->curve.first= mo1->curve.last= 0;
				while(nu) {
					new= duplicateNurb(nu);
					addtail(&(mo1->curve), new);
					nu= nu->next;
				}
				mo1->disp.first= mo1->disp.last= 0;

				if(mo->ipotra) {
					bez=mo->ipotra;
					len=sizeof(struct Bezier)+bez->cp*36;
					mo1->ipotra=(struct Bezier *)mallocN(len,"addupipo1");
					memcpy(mo1->ipotra,mo->ipotra,len);
				}
				if(mo->ipoqfi) {
					bez=mo->ipoqfi;
					len=sizeof(struct Bezier)+bez->cp*36;
					mo1->ipoqfi=(struct Bezier *)mallocN(len,"addupipo2");
					memcpy(mo1->ipoqfi,mo->ipoqfi,len);
				}

				mo1->data= 0;

				if(mo->key) {
					key= mo->key;
					prevk= 0;
					len= sizeof(struct Key) +4 +12*mo->keyverts;
					while(key) {
						keyn= (struct Key *)mallocN(len, "copymenuKey");
						memcpy(keyn, key, len);

						if(prevk==0) mo->key= keyn;
						else prevk->next= keyn;
						prevk= keyn;

						key= key->next;
					}
				}

				if(mo->ipovkey) {
					bez= mo->ipovkey;
					len= sizeof(struct Bezier)+bez->cp*36;
					mo->ipovkey= (struct Bezier *)mallocN(len,"addupipovvkey");
					memcpy(mo->ipovkey, bez, len);
				}
			}
			else if(b->soort== -4) {
				ika= (struct IkaData *)b->d;
				ika1= mallocN(sizeof(struct IkaData),"addupbase-4");
				
				memcpy(ika1, ika, sizeof(struct IkaData));
				base->d= (long *)ika1;
				
				ika1->bpbase.first= ika1->bpbase.last= 0;
				ika1->rodbase.first= ika1->rodbase.last= 0;
				ika1->jointbase.first= ika1->jointbase.last= 0;
				ika1->skelbase.first= ika1->skelbase.last= 0;
				
				/* alles selecteren ivm dupli */
				setflags_bodypoints(ika->bpbase.first, 1);
				
				add_dupli_ika(&ika1->bpbase, &ika1->rodbase, &ika1->jointbase, &ika1->skelbase, 
					  &ika->bpbase, &ika->rodbase, &ika->jointbase, &ika->skelbase);

				if(ika->key) {
					key= ika->key;
					prevk= 0;
					len= sizeof(struct Key) +4*4*ika->keyverts+4*ika->keyrods;
					while(key) {
						keyn= (struct Key *)mallocN(len, "copymenuIkaKey");
						memcpy(keyn, key, len);

						if(prevk==0) ika->key= keyn;
						else prevk->next= keyn;
						prevk= keyn;

						key= key->next;
					}
				}

				if(ika->ipokey) {
					bez= ika->ipokey;
					len= sizeof(struct Bezier)+bez->cp*36;
					ika->ipokey= (struct Bezier *)mallocN(len,"addupipokey");
					memcpy(ika->ipokey, bez, len);
				}

				
			}
			newbasename(base);
			makeDispList(base);
		}
		if(b==lbase) break;
		b=b->next;
	}

	/* goedzetten van pointers met ablock */
	base= lbase->next;
	while(base) {
		abl= ablock;
		for(a=0;a<totdup;a++) {
			if(base->p== *abl) base->p= *(abl+1);
			if(base->t== *abl) base->t= *(abl+1);
			abl+=2;
		}
		base= base->next;
	}
	freeN(ablock);

	countall();
	if((G.f & 1)==0) {	/* grabber uitgeschakeld */
		grabber();
		if(G.mainb==4) if(b->soort==2) tekenlampbuts(1);
		if(G.mainb==5) if(b->soort & 1) tekenobjbuts(1);
		if(G.mainb==7) tekenmovebuts(1);
		if(G.mainb==9) tekeneditbuts(1);
	}
}

