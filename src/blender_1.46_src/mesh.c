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



/*  mesh.c      MIXED MODEL
 * 
 *  jan/maart 95
 *  
 * 
 */

#include "blender.h"
#include "sector.h"

void set_mesh(Object *ob, Mesh *me);

void unlink_mesh(Mesh *me)
{
	int a;
	
	if(me==0) return;
	
	for(a=0; a<me->totcol; a++) {
		if(me->mat[a]) me->mat[a]->id.us--;
		me->mat[a]= 0;
	}
	if(me->key) me->key->id.us--;
	me->key= 0;
	
	if(me->texcomesh) me->texcomesh= 0;
}


/* niet mesh zelf vrijgeven */
void free_mesh(Mesh *me)
{

	unlink_mesh(me);

	if(me->mat) freeN(me->mat);
	if(me->orco) freeN(me->orco);
	if(me->mface) freeN(me->mface);
	if(me->tface) freeN(me->tface);
	if(me->mvert) freeN(me->mvert);
	if(me->mcol) freeN(me->mcol);
	if(me->msticky) freeN(me->msticky);
	if(me->bb) freeN(me->bb);
	if(me->disp.first) freedisplist(&me->disp);
}

Mesh *add_mesh()
{
	Mesh *me;
	
	me= alloc_libblock(&G.main->mesh, ID_ME, "Mesh");
	
	me->size[0]= me->size[1]= me->size[2]= 1.0;
	me->smoothresh= 10;
	me->texflag= AUTOSPACE;
	me->flag= ME_TWOSIDED;
	
	me->bb= unit_boundbox();
	
	return me;
}

Mesh *copy_mesh(Mesh *me)
{
	Mesh *men, *p=0;
	int a;
	
	men= copy_libblock(me);
	
	men->mat= dupallocN(me->mat);
	for(a=0; a<men->totcol; a++) {
		id_us_plus(men->mat[a]);
	}
	id_us_plus(men->texcomesh);
	men->mface= dupallocN(me->mface);

	men->tface= dupallocN(me->tface);

	men->dface= 0;
	men->mvert= dupallocN(me->mvert);
	men->mcol= dupallocN(me->mcol);
	men->msticky= dupallocN(me->msticky);
	men->texcomesh= 0;
	men->orco= 0;
	men->bb= dupallocN(men->bb);
	men->disp.first= men->disp.last= 0;
	
	men->key= copy_key(me->key);
	if(men->key) men->key->from= (ID *)men;
	
	return men;
}

void make_local_tface(Mesh *me)
{
	TFace *tface;
	Image *ima;
	int a;
	
	if(me->tface==0) return;
	
	a= me->totface;
	tface= me->tface;
	while(a--) {
		
		/* speciaal geval: ima altijd meteen lokaal */
		if(tface->tpage) {
			ima= tface->tpage;
			if(ima->id.lib) {
				ima->id.lib= 0;
				ima->id.flag= LIB_LOCAL;
				new_id(0, ima, 0);
			}
		}
		tface++;
	}
	
}

void make_local_mesh(Mesh *me)
{
	Object *ob;
	Mesh *men;
	int local=0, lib=0;
	
	/* - zijn er alleen lib users: niet doen
	 * - zijn er alleen locale users: flag zetten
	 * - mixed: copy
	 */
	
	if(me->id.lib==0) return;
	if(me->id.us==1) {
		me->id.lib= 0;
		me->id.flag= LIB_LOCAL;
		new_id(0, me, 0);
		
		if(me->tface) make_local_tface(me);
		
		return;
	}
	
	ob= G.main->object.first;
	while(ob) {
		if( me==get_mesh(ob) ) {
			if(ob->id.lib) lib= 1;
			else local= 1;
		}
		else if( me==get_other_mesh(ob) ) {
			if(ob->id.lib) lib= 1;
			else local= 1;
		}
		
		ob= ob->id.next;
	}
	
	if(local && lib==0) {
		me->id.lib= 0;
		me->id.flag= LIB_LOCAL;
		new_id(0, me, 0);
		
		if(me->tface) make_local_tface(me);
		
	}
	else if(local && lib) {
		men= copy_mesh(me);
		men->id.us= 0;
		
		ob= G.main->object.first;
		while(ob) {
			if( me==get_mesh(ob) ) {				
				if(ob->id.lib==0) {
					set_mesh(ob, men);
				}
			}
			else if( me==get_other_mesh(ob) ) {				
				if(ob->id.lib==0) {
					set_mesh(ob, men);
				}
			}
			ob= ob->id.next;
		}
	}
}

void boundbox_mesh(Mesh *me, float *loc, float *size)
{
	MVert *mvert;
	BoundBox *bb;
	float min[3], max[3];
	int a;
	
	if(me->bb==0) me->bb= callocN(sizeof(BoundBox), "boundbox");
	bb= me->bb;
	
	INIT_MINMAX(min, max);

	mvert= me->mvert;
	for(a=0; a<me->totvert; a++, mvert++) {
		DO_MINMAX(mvert->co, min, max);
	}

	if(me->totvert) {
		loc[0]= (min[0]+max[0])/2.0;
		loc[1]= (min[1]+max[1])/2.0;
		loc[2]= (min[2]+max[2])/2.0;
		
		size[0]= (max[0]-min[0])/2.0;
		size[1]= (max[1]-min[1])/2.0;
		size[2]= (max[2]-min[2])/2.0;
	}
	else {
		loc[0]= loc[1]= loc[2]= 0.0;
		size[0]= size[1]= size[2]= 0.0;
	}
	
	bb->vec[0][0]=bb->vec[1][0]=bb->vec[2][0]=bb->vec[3][0]= loc[0]-size[0];
	bb->vec[4][0]=bb->vec[5][0]=bb->vec[6][0]=bb->vec[7][0]= loc[0]+size[0];
	
	bb->vec[0][1]=bb->vec[1][1]=bb->vec[4][1]=bb->vec[5][1]= loc[1]-size[1];
	bb->vec[2][1]=bb->vec[3][1]=bb->vec[6][1]=bb->vec[7][1]= loc[1]+size[1];

	bb->vec[0][2]=bb->vec[3][2]=bb->vec[4][2]=bb->vec[7][2]= loc[2]-size[2];
	bb->vec[1][2]=bb->vec[2][2]=bb->vec[5][2]=bb->vec[6][2]= loc[2]+size[2];
}


void tex_space_mesh(Mesh *me)
{
	KeyBlock *kb;
	float *fp, loc[3], size[3], min[3], max[3];
	int a;
	
	boundbox_mesh(me, loc, size);

	if(me->texflag & AUTOSPACE) {
		if(me->key) {
			kb= me->key->refkey;
			if (kb) {
				
				INIT_MINMAX(min, max);
				
				fp= kb->data;
				for(a=0; a<kb->totelem; a++, fp+=3) {	
					DO_MINMAX(fp, min, max);
				}
				if(kb->totelem) {
					loc[0]= (min[0]+max[0])/2.0; loc[1]= (min[1]+max[1])/2.0; loc[2]= (min[2]+max[2])/2.0;
					size[0]= (max[0]-min[0])/2.0; size[1]= (max[1]-min[1])/2.0; size[2]= (max[2]-min[2])/2.0;
				}
				else {
					loc[0]= loc[1]= loc[2]= 0.0;
					size[0]= size[1]= size[2]= 0.0;
				}
				
			}
		}

		VECCOPY(me->loc, loc);
		VECCOPY(me->size, size);
		me->rot[0]= me->rot[1]= me->rot[2]= 0.0;

		if(me->size[0]==0.0) me->size[0]= 1.0;
		else if(me->size[0]>0.0 && me->size[0]<0.00001) me->size[0]= 0.00001;
		else if(me->size[0]<0.0 && me->size[0]> -0.00001) me->size[0]= -0.00001;
	
		if(me->size[1]==0.0) me->size[1]= 1.0;
		else if(me->size[1]>0.0 && me->size[1]<0.00001) me->size[1]= 0.00001;
		else if(me->size[1]<0.0 && me->size[1]> -0.00001) me->size[1]= -0.00001;
	
		if(me->size[2]==0.0) me->size[2]= 1.0;
		else if(me->size[2]>0.0 && me->size[2]<0.00001) me->size[2]= 0.00001;
		else if(me->size[2]<0.0 && me->size[2]> -0.00001) me->size[2]= -0.00001;
	}
	
}

void make_orco_mesh(Mesh *me)
{
	MVert *mvert;
	KeyBlock *kb;
	float *orco, *fp;
	int a, totvert;
	
	totvert= me->totvert;
	if(totvert==0) return;
	orco= me->orco= mallocN(4*3*totvert, "orco mesh");

	if(me->key) {
		kb= me->key->refkey;
		if (kb) {		/***** BUG *****/
			fp= kb->data;
			
			for(a=0; a<totvert; a++, orco+=3) {
				orco[0]= (fp[0]-me->loc[0])/me->size[0];
				orco[1]= (fp[1]-me->loc[1])/me->size[1];
				orco[2]= (fp[2]-me->loc[2])/me->size[2];
				
				/* mvert alleen ophogen als totvert <= kb->totelem */
				if(a<kb->totelem) fp+=3;
			}
		}
	}
	else {
		if(me->texcomesh) {
			me= me->texcomesh;
		}	
	
		mvert= me->mvert;
		for(a=0; a<totvert; a++, orco+=3) {
			orco[0]= (mvert->co[0]-me->loc[0])/me->size[0];
			orco[1]= (mvert->co[1]-me->loc[1])/me->size[1];
			orco[2]= (mvert->co[2]-me->loc[2])/me->size[2];
			
			/* mvert alleen ophogen als totvert <= me->totvert */
			if(a<me->totvert) mvert++;
		}
	}
}

void test_index_mface(MFace *mface, int nr)
{
	int a;
	
	/* voorkom dat een nul op de verkeerde plek staat */
	if(nr==2) {
		if(mface->v2==0) SWAP(int, mface->v1, mface->v2);
	}
	else if(nr==3) {
		if(mface->v3==0) {
			SWAP(int, mface->v1, mface->v2);
			SWAP(int, mface->v2, mface->v3);
			
			a= mface->edcode;
			mface->edcode= 0;
			if(a & ME_V1V2) mface->edcode |= ME_V3V1;
			if(a & ME_V2V3) mface->edcode |= ME_V1V2;
			if(a & ME_V3V1) mface->edcode |= ME_V2V3;
			if(a & DF_V1V2) mface->edcode |= DF_V3V1;
			if(a & DF_V2V3) mface->edcode |= DF_V1V2;
			if(a & DF_V3V1) mface->edcode |= DF_V2V3;
			
			a= mface->puno;
			mface->puno &= ~15;
			if(a & ME_FLIPV1) mface->puno |= ME_FLIPV2;
			if(a & ME_FLIPV2) mface->puno |= ME_FLIPV3;
			if(a & ME_FLIPV3) mface->puno |= ME_FLIPV1;
		}
	}
	else if(nr==4) {
		if(mface->v3==0 || mface->v4==0) {
			SWAP(int, mface->v1, mface->v3);
			SWAP(int, mface->v2, mface->v4);
			a= mface->edcode;
			mface->edcode= 0;
			if(a & ME_V1V2) mface->edcode |= ME_V3V4;
			if(a & ME_V2V3) mface->edcode |= ME_V2V3;
			if(a & ME_V3V4) mface->edcode |= ME_V1V2;
			if(a & ME_V4V1) mface->edcode |= ME_V4V1;
			if(a & DF_V1V2) mface->edcode |= DF_V3V4;
			if(a & DF_V2V3) mface->edcode |= DF_V2V3;
			if(a & DF_V3V4) mface->edcode |= DF_V1V2;
			if(a & DF_V4V1) mface->edcode |= DF_V4V1;

			a= mface->puno;
			mface->puno &= ~15;
			if(a & ME_FLIPV1) mface->puno |= ME_FLIPV3;
			if(a & ME_FLIPV2) mface->puno |= ME_FLIPV4;
			if(a & ME_FLIPV3) mface->puno |= ME_FLIPV1;
			if(a & ME_FLIPV4) mface->puno |= ME_FLIPV2;
		}
	}
}


void flipnorm_mesh(Mesh *me)
{
	MFace *mface;
	MVert *mvert;
	DispList *dl;
	float *fp;
	int a, temp;
	
	mvert= me->mvert;
	a= me->totvert;
	while(a--) {
		mvert->no[0]= -mvert->no[0];
		mvert->no[1]= -mvert->no[1];
		mvert->no[2]= -mvert->no[2];
		mvert++;
	}
	
	mface= me->mface;
	a= me->totface;
	while(a--) {
		if(mface->v3) {
			if(mface->v4) {
				SWAP(int, mface->v4, mface->v1);
				SWAP(int, mface->v3, mface->v2);
				test_index_mface(mface, 4);
				temp= mface->puno;
				mface->puno &= ~15;
				if(temp & ME_FLIPV1) mface->puno |= ME_FLIPV4;
				if(temp & ME_FLIPV2) mface->puno |= ME_FLIPV3;
				if(temp & ME_FLIPV3) mface->puno |= ME_FLIPV2;
				if(temp & ME_FLIPV4) mface->puno |= ME_FLIPV1;
			}
			else {
				SWAP(int, mface->v3, mface->v1);
				test_index_mface(mface, 3);
				temp= mface->puno;
				mface->puno &= ~15;
				if(temp & ME_FLIPV1) mface->puno |= ME_FLIPV3;
				if(temp & ME_FLIPV2) mface->puno |= ME_FLIPV2;
				if(temp & ME_FLIPV3) mface->puno |= ME_FLIPV1;
			}
		}
		mface++;
	}

	if(me->disp.first) {
		dl= me->disp.first;
		fp= dl->nors;
		if(fp) {
			a= dl->nr;
			while(a--) {
				fp[0]= -fp[0];
				fp[1]= -fp[1];
				fp[2]= -fp[2];
				fp+= 3;
			}
		}
	}
}

Mesh *get_mesh(Object *ob)
{
	
	if(ob==0) return 0;
	if(ob->type==OB_MESH) return ob->data;
	else if(ob->type==OB_SECTOR) {
		Sector *se= ob->data;
		if(se->flag & SE_SHOW_TEXMESH) {
			if(se->texmesh==0) {
				se->texmesh= add_mesh();
				rename_id(se->texmesh, "TexMesh");
				se->texmesh->flag &= ~ME_TWOSIDED;
			}
			return se->texmesh;
		}
		return se->dynamesh;
	}
	else if(ob->type==OB_LIFE) {
		Life *lf= ob->data;
		if(lf->flag & LF_SHOW_TEXMESH) {
			if(lf->texmesh==0) {
				lf->texmesh= add_mesh();
				rename_id(lf->texmesh, "TexMesh");
				lf->texmesh->flag &= ~ME_TWOSIDED;
			}
			return lf->texmesh;
		}
		return lf->dynamesh;
	}
	else return 0;
}

Mesh *get_other_mesh(Object *ob)
{
	
	if(ob==0) return 0;
	
	if(ob->type==OB_SECTOR) {
		Sector *se= ob->data;
		if(se->flag & SE_SHOW_TEXMESH) {
			return se->dynamesh;
		}
		return se->texmesh;
	}
	else if(ob->type==OB_LIFE) {
		Life *lf= ob->data;
		if(lf->flag & LF_SHOW_TEXMESH) {
			return lf->dynamesh;
		}
		return lf->texmesh;
	}
	else return 0;
}


void set_mesh(Object *ob, Mesh *me)
{
	Mesh *old=0;
	
	if(ob==0) return;
	
	if(ob->type==OB_MESH) {
		old= ob->data;
		old->id.us--;
		ob->data= me;
		id_us_plus(me);
	}
	else if(ob->type==OB_SECTOR) {
		Sector *se= ob->data;
		
		if(se->flag & SE_SHOW_TEXMESH) {
			old= se->texmesh;
			se->texmesh= me;
			
			if(me->tface==0) make_tfaces(me);
		}
		else {
			old= se->dynamesh;
			se->dynamesh= me;
		}
		if(old) old->id.us--;
		id_us_plus(me);
	}
	else if(ob->type==OB_LIFE) {
		Life *lf= ob->data;

		if(lf->flag & LF_SHOW_TEXMESH) {
			old= lf->texmesh;
			lf->texmesh= me;
			
			if(me->tface==0) make_tfaces(me);
		}
		else {
			old= lf->dynamesh;
			lf->dynamesh= me;
		}
		if(old) old->id.us--;
		id_us_plus(me);
	}
	
	test_object_materials(me);
}

void mball_to_mesh(ListBase *lb, Mesh *me)
{
	DispList *dl;
	MVert *mvert;
	MFace *mface;
	float *nors, *verts, nor[3];
	int a, *index;
	
	dl= lb->first;
	if(dl==0) return;

	if(dl->type==DL_INDEX4) {
		me->flag= ME_NOPUNOFLIP;
		me->totvert= dl->nr;
		me->totface= dl->parts;
		
		me->mvert=mvert= callocN(dl->nr*sizeof(MVert), "mverts");
		a= dl->nr;
		nors= dl->nors;
		verts= dl->verts;
		while(a--) {
			VECCOPY(mvert->co, verts);
			VECCOPY(nor, nors);
			VecMulf(nor, 32767.0);
			VECCOPY(mvert->no, nor);
			mvert++;
			nors+= 3;
			verts+= 3;
		}
		
		me->mface=mface= callocN(dl->parts*sizeof(MFace), "mface");
		a= dl->parts;
		index= dl->index;
		while(a--) {
			mface->v1= index[0];
			mface->v2= index[1];
			mface->v3= index[2];
			mface->v4= index[3];

			mface->puno= 15;
			mface->edcode= ME_V1V2+ME_V2V3;
			
			mface++;
			index+= 4;
		}
	}	
}

void nurbs_to_mesh(Object *ob)
{
	Object *ob1;
	DispList *dl;
	Mesh *me;
	Curve *cu;
	MVert *mvert;
	MFace *mface;
	float *data, min[3], max[3];
	int a, b, len, ofs, vertcount, startvert, totvert=0, totvlak=0;
	int p1, p2, p3, p4, *index;

	min[0]= min[1]= min[2]= 1.0e10;
	max[0]= max[1]= max[2]= -1.0e10;

	cu= ob->data;

	if(ob->type==OB_CURVE) {
		/* regel: dl->type INDEX3 altijd vooraan in lijst */
		dl= cu->disp.first;
		if(dl->type!=DL_INDEX3) {
			curve_to_filledpoly(ob->data, &cu->disp);
		}
	}

	/* tellen */
	dl= cu->disp.first;
	while(dl) {
		if(dl->type==DL_SEGM) {
			totvert+= dl->parts*dl->nr;
			totvlak+= dl->parts*(dl->nr-1);
		}
		else if(dl->type==DL_POLY) {
			totvert+= dl->parts*dl->nr;
			totvlak+= dl->parts*dl->nr;
		}
		else if(dl->type==DL_SURF) {
			totvert+= dl->parts*dl->nr;
			totvlak+= (dl->parts-1+((dl->flag & 2)==2))*(dl->nr-1+(dl->flag & 1));
		}
		else if(dl->type==DL_INDEX3) {
			totvert+= dl->nr;
			totvlak+= dl->parts;
		}
		dl= dl->next;
	}
	if(totvert==0) {
		error("can't convert");
		return;
	}

	/* mesh maken */
	me= add_mesh("CuMesh");
	me->totvert= totvert;
	me->totface= totvlak;

	me->totcol= cu->totcol;
	me->mat= cu->mat;
	cu->mat= 0;
	cu->totcol= 0;

	mvert=me->mvert= callocN(me->totvert*sizeof(MVert), "cumesh1");
	mface=me->mface= callocN(me->totface*sizeof(MFace), "cumesh2");

	/* verts en vlakken */
	vertcount= 0;

	dl= cu->disp.first;
	while(dl) {
		if(dl->type==DL_SEGM) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= dl->verts;
			while(a--) {
				VECCOPY(mvert->co, data);
				data+=3;
				vertcount++;
				mvert++;
			}

			for(a=0; a<dl->parts; a++) {
				ofs= a*dl->nr;
				for(b=1; b<dl->nr; b++) {
					mface->v1= startvert+ofs+b-1;
					mface->v2= startvert+ofs+b;
					mface->edcode= ME_V1V2;
					test_index_mface(mface, 2);
					mface++;
				}
			}

		}
		else if(dl->type==DL_POLY) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= dl->verts;
			while(a--) {
				VECCOPY(mvert->co, data);
				data+=3;
				vertcount++;
				mvert++;
			}

			for(a=0; a<dl->parts; a++) {
				ofs= a*dl->nr;
				for(b=0; b<dl->nr; b++) {
					mface->v1= startvert+ofs+b;
					if(b==dl->nr-1) mface->v2= startvert+ofs;
					else mface->v2= startvert+ofs+b+1;
					mface->edcode= ME_V1V2;
					test_index_mface(mface, 2);
					mface++;
				}
			}
		}
		else if(dl->type==DL_INDEX3) {
			startvert= vertcount;
			a= dl->nr;
			data= dl->verts;
			while(a--) {
				VECCOPY(mvert->co, data);
				data+=3;
				vertcount++;
				mvert++;
			}

			a= dl->parts;
			index= dl->index;
			while(a--) {
				mface->v1= startvert+index[0];
				mface->v2= startvert+index[1];
				mface->v3= startvert+index[2];
				mface->v4= 0;
	
				mface->puno= 7;
				mface->edcode= ME_V1V2+ME_V2V3;
				test_index_mface(mface, 3);
				
				mface++;
				index+= 3;
			}
	
	
		}
		else if(dl->type==DL_SURF) {
			startvert= vertcount;
			a= dl->parts*dl->nr;
			data= dl->verts;
			while(a--) {
				VECCOPY(mvert->co, data);
				data+=3;
				vertcount++;
				mvert++;
			}

			for(a=0; a<dl->parts; a++) {

				if( (dl->flag & 2)==0 && a==dl->parts-1) break;

				if(dl->flag & 1) {				/* p2 -> p1 -> */
					p1= startvert+ dl->nr*a;	/* p4 -> p3 -> */
					p2= p1+ dl->nr-1;			/* -----> volgende rij */
					p3= p1+ dl->nr;
					p4= p2+ dl->nr;
					b= 0;
				}
				else {
					p2= startvert+ dl->nr*a;
					p1= p2+1;
					p4= p2+ dl->nr;
					p3= p1+ dl->nr;
					b= 1;
				}
				if( (dl->flag & 2) && a==dl->parts-1) {
					p3-= dl->parts*dl->nr;
					p4-= dl->parts*dl->nr;
				}

				for(; b<dl->nr; b++) {
					mface->v1= p1;
					mface->v2= p3;
					mface->v3= p4;
					mface->v4= p2;
					mface->mat_nr= dl->col;
					mface->edcode= ME_V1V2+ME_V2V3;
					test_index_mface(mface, 4);
					mface++;

					p4= p3; 
					p3++;
					p2= p1; 
					p1++;
				}
			}

		}

		dl= dl->next;
	}

	if(ob->data) {
		free_libblock(&G.main->curve, ob->data);
	}
	ob->data= me;
	ob->type= OB_MESH;
	
	tex_space_mesh(me);
	
	/* andere users */
	ob1= G.main->object.first;
	while(ob1) {
		if(ob1->data==cu) {
			ob1->type= OB_MESH;
		
			ob1->data= ob->data;
			id_us_plus(ob->data);
		}
		ob1= ob1->id.next;
	}

}



