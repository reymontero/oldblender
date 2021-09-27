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



/*  editika.c     GRAPHICS
 * 
 *  april 96
 *  
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "ika.h"


void draw_limb(Limb *li, float small)
{
	float vec[2];
	
	rotate( li->alpha*1800.0/M_PI, 'z');
	
	arc(0.0, 0.0, small, 900.0, 2700.0);
	
	vec[0]= 0.0; vec[1]= small;
	bgnline();
	v2f(vec);
	vec[0]= li->len; vec[1]= 0.0;
	v2f(vec);
	vec[0]= 0.0; vec[1]= -small;
	v2f(vec);
	endline();
	
	small*= 0.25;
	
	if(li->next) circ(li->len, 0.0, small);
	else circf(li->len, 0.0, small);
	
	
	translate(li->len, 0.0, 0.0);
}

void draw_ika(Object *ob, int sel)
{
	Ika *ika;
	Limb *li;
	short col[3];
	float small= 0.15;
	
	ika= ob->data;
	li= ika->limbbase.first;
	if(li==0) return;
	
	/* we zijn al in objectspace */
	pushmatrix();
	
	gRGBcolor(col, col+1, col+2);
	
	if((ika->flag & IK_GRABEFF)==0) {
		if(sel) cpack(0xFFFF);
		circf(0.0, 0.0, 0.05*li->len);
		if(sel) RGBcolor(col[0], col[1], col[2]);
	}

	while(li) {
		
		small= 0.10*li->len;
	
		draw_limb(li, small);
		li= li->next;
	}	
	
	if(ika->flag & IK_GRABEFF) {
		if(sel) if(ika->def) cpack(0xFFFF00); else cpack(0xFFFF);
		circf(0.0, 0.0, 0.25*small);
		if(sel) RGBcolor(col[0], col[1], col[2]);
	}
	
	popmatrix();
}

/* type 0: verts, type 1: limbs */
void draw_ika_nrs(Object *ob, int type)
{
	Ika *ika;
	Limb *li;
	int nr=0;
	char str[12];
	
	if(curarea->spacetype!=SPACE_VIEW3D) return;
	winset(curarea->win);
	
	frontbuffer(1); backbuffer(0);
	loadmatrix(G.vd->viewmat);
	multmatrix(ob->obmat);
	
	ika= ob->data;
	li= ika->limbbase.first;
	
	/* we zijn al in objectspace */
	pushmatrix();
	cpack(0xFFFFFF);
	fmsetfont(G.font);
	
	if(type==0) {
		sprintf(str, " %d", nr++);
		cmov(0.0, 0.0, 0.0);
		fmprstr(str);

		while(li) {
			rotate( li->alpha*1800.0/M_PI, 'z');
			translate(li->len, 0.0, 0.0);
			sprintf(str, " %d", nr++);
			cmov(0.0, 0.0, 0.0);
			fmprstr(str);
			
			li= li->next;
		}	
	}
	else {
		while(li) {
			rotate( li->alpha*1800.0/M_PI, 'z');
			translate( 0.7*li->len, 0.0, 0.0);
			sprintf(str, " %d", nr++);
			cmov(0.0, 0.0, 0.0);
			fmprstr(str);
			translate( 0.3*li->len, 0.0, 0.0);
			
			li= li->next;
		}	
		
	}
	
	frontbuffer(0); backbuffer(1);
	popmatrix();
}



int extrude_ika(Object *ob, int add)
{
	Ika *ika;
	Limb *li;
	float dvec[3], dvecp[3], oldeul[3], mat[3][3], imat[3][3];
	int firsttime= 1;
	short event, val, afbreek=0, mval[2], xo, yo;
	
	/* init */
	VECCOPY(oldeul, ob->rot);
	initgrabz(ob->obmat[3][0], ob->obmat[3][1], ob->obmat[3][2]);

	Mat3CpyMat4(mat, ob->obmat);
	Mat3Inv(imat, mat);
	
	getmouseco_areawin(mval);
	xo= mval[0];
	yo= mval[1];

	/* het laatste punt van de ika */
	ika= ob->data;
	
	if(add) {
		/* een erbij: */
		li= callocN(sizeof(Limb), "limb");
		addtail(&ika->limbbase, li);
		if(li->prev) {
			li->eff[0]= li->prev->eff[0];
			li->eff[1]= li->prev->eff[1];
		}
		li->eff[0]+= 0.5;
	}
	li= ika->limbbase.last;
	if(li==0) return 0;
	
	while(TRUE) {

		getmouseco_areawin(mval);
		if(mval[0]!=xo || mval[1]!=yo || firsttime) {
			firsttime= 0;

			window_to_3d(dvec, mval[0]-xo, mval[1]-yo);
			VECCOPY(dvecp, dvec);
			
			/* apply */
			Mat3MulVecfl(imat, dvecp);
			li->eff[0]+= dvecp[0];
			li->eff[1]+= dvecp[1];

			calc_limb(li);
			
			if(li->prev==0) {
				VECCOPY(ob->rot, oldeul);
				euler_rot(ob->rot, li->alpha, 'z');
				li->alpha= li->alphao= 0.0;
			}
			
			xo= mval[0];
			yo= mval[1];
			
			force_draw();
		}
		
		while(qtest()) {
			event= extern_qread(&val);
			if(val) {
				switch(event) {
				case ESCKEY:
				case LEFTMOUSE:
				case MIDDLEMOUSE:
				case SPACEKEY:
				case RETKEY:
					afbreek= 1;
					break;
				}
			}
			if(afbreek) break;
		}
		
		if(afbreek) break;
	}
	
	if(event==ESCKEY) {
		if(ika->limbbase.first!=ika->limbbase.last) {
			li= ika->limbbase.last;
			remlink(&ika->limbbase, li);
			freeN(li);
		}
	}
	else if(add) init_defstate_ika(ob);
	
	allqueue(REDRAWVIEW3D, 0);
	
	if(event==LEFTMOUSE) return 0;
	return 1; 
}


void make_skeleton()
{
	Object *ob;
	Base *base;
	Ika *ika;
	Deform *def;
	Limb *li;
	int a, tot;
	
	ob= OBACT;
	if(ob==0 || ob->type!=OB_IKA || (ob->flag & SELECT)==0) return;
	
	if( okee("Make Skeleton")==0 ) return;
	
	ika= ob->data;
	if(ika->def) freeN(ika->def);
	ika->def= 0;
	ika->totdef= 0;
	
	/* per selected ob, per limb, de obmat en imat berekenen */
	
	base= FIRSTBASE;
	while(base) {
		if TESTBASE(base) {
			if(base->object->type==OB_IKA) ika->totdef+= count_limbs(base->object);
			else ika->totdef++;
		}
		base= base->next;
	}
	
	if(ika->totdef==0) {
		error("Nothing selected");
		return;
	}
	
	ika->def= def= callocN(ika->totdef*sizeof(Deform), "deform");
	
	base= FIRSTBASE;
	while(base) {
		if TESTBASE(base) {
			
			if(base->object->type==OB_IKA) {
				
				li= ( (Ika *)(base->object->data) )->limbbase.first;
				a= 0;
				while(li) {
					what_does_parent1(base->object, PARLIMB, a, 0, 0);
					def->ob= base->object;
					def->partype= PARLIMB;
					def->par1= a;
					
					Mat4Invert(def->imat, workob.obmat);
					def->vec[0]= li->len;
					def->fac= 1.0;
					
					def++;
					a++;
					li= li->next;
				}
			}
			else {
				what_does_parent1(base->object, PAROBJECT, 0, 0, 0);
				def->ob= base->object;
				def->partype= PAROBJECT;
				
				def->vec[0]= 0.0;
				def->fac= 1.0;
				
				Mat4Invert(def->imat, workob.obmat);
				def++;
			}
		}
		base= base->next;
	}
	allqueue(REDRAWVIEW3D, 0);
}

