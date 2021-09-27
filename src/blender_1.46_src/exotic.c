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



/*  exotic.c   mei 95     MIXED MODEL
 * 
 *  
 *  eigen videoscape formaat:
 * 
 * lamp:
 *		3DG2
		aantal_lampen
		
		type
		spsi spbl
		r, g, b, energy
		locx, locy, locz
		vecx, vecy, vecz

		
	curve / nurbs:
		3DG3
		5 of 11 (curve of surf)
		aantal_nurbs
		extr1 extr2
		
		mat[0][0] mat[0][1] mat[0][2] mat[0][3]
		mat[1][0] mat[1][1] mat[1][2] mat[1][3]
		...		
		
		type
		pntsu, pntsv
		resolu, resolv
		orderu, orderv
		flagu, flagv
		
		(als type==nurb) x y z w
						 x y z w
						 ...
		(als type==bez)  xyz xyz xyz h1 h2 h3
						 xyz xyz xyz h1 h2 h3
						 ...
 *  
 * 
 */



#include "blender.h"
#include "file.h"
#include "edit.h"
#include "psx.h"

void read_videoscape_mesh(char *str)
{
	Object *ob;
	Mesh *me;
	MVert *mvert;
	MFace *mface;
	Material *ma;
	FILE *fp;
	float *vertdata, *vd, min[3], max[3], cent[3];
	ulong color[32], col;
	int totcol, a, b, verts, tottria=0, totquad=0, totedge=0, poly, nr0, nr, first;
	short end, event;
	char s[50];

	fp= fopen(str, "r");
	if(fp==NULL) {
		error("Can't read file");
		return;
	}

	fscanf(fp, "%40s", s);

	fscanf(fp, "%d\n", &verts);
	if(verts<=0) {
		fclose(fp);
		error("Read error");
		return;
	}

	if(verts>65000) {
		error("too many vertices");
		fclose(fp);
		return;
	}
	
	INIT_MINMAX(min, max);
	vd= vertdata= mallocN(4*3*verts, "videoscapelezer");

	for(a=0; a<verts; a++) {
		fscanf(fp, "%f %f %f", vd, vd+1, vd+2);
		DO_MINMAX(vd, min, max);
		vd+=3;
	}
	
	/* de vlakken en kleuren tellen */
	for(a=0; a<32; a++) color[a]= 0;
	totcol= 0;
	end= 1;
	while(end>0) {
		end= fscanf(fp,"%d", &poly);
		if(end<=0) break;

		if(poly==3) tottria++;
		else if(poly==4) totquad++;
		else totedge+= poly;

		for(a=0;a<poly;a++) {
			end= fscanf(fp,"%d", &nr);
			if(end<=0) break;
		}
		if(end<=0) break;
		
		end= fscanf(fp,"%i\n", &col);
		col &= 0xF0F0F0;
		for(a=0; a<totcol; a++) {
			if(color[a]==col) break;
		}
		if(a>=totcol && totcol<32) {
			color[totcol]= col;
			totcol++;
		}
	}

	if(totedge+tottria+totquad>65000) {
		PRINT3(d, d, d, totedge, tottria, totquad);
		error("too many faces");
		freeN(vertdata);
		fclose(fp);
		return;
	}

	/* nieuw object */
	ob= add_object(OB_MESH);
	me= ob->data;
	me->totvert= verts;
	me->totface= totedge+tottria+totquad;
	
	me->mvert= callocN(me->totvert*sizeof(MVert), "mverts");
	if(me->totface) me->mface= callocN(me->totface*sizeof(MFace), "mface");
	
	/* kleuren */
	if(totcol) {
		ob->mat= callocN(4*totcol, "ob->mat");
		me->mat= callocN(4*totcol, "me->mat");
		ob->totcol= me->totcol= totcol;
		ob->actcol= 1;
	}
	
	/* materialen */
	for(a=0; a<totcol; a++) {
		ma= G.main->mat.first;
		while(ma) {
			if(ma->mtex[0]==0) {
				col= rgb_to_cpack(ma->r, ma->g, ma->b);
				if(color[a]==col) {
					me->mat[a]= ma;
					ma->id.us++;
					break;
				}
			}
			ma= ma->id.next;
		}
		if(ma==0) {
			ma= add_material("ext");
			me->mat[a]= ma;
			cpack_to_rgb(color[a], cent, cent+1, cent+2);
			ma->r= cent[0];
			ma->g= cent[1];
			ma->b= cent[2];
			automatname(ma);
		}
	}
	
	/* verts */
	
	cent[0]= (min[0]+max[0])/2.0;
	cent[1]= (min[1]+max[1])/2.0;
	cent[2]= (min[2]+max[2])/2.0;
	VECCOPY(ob->loc, cent);
	ob->rot[0]= ob->rot[1]= ob->rot[2]= 0.0;
	
	a= me->totvert;
	vd= vertdata;
	mvert= me->mvert;
	while(a--) {
		VecSubf(mvert->co, vd, cent);
		mvert++;
		vd+= 3;
	}

	/* faces */
	if(me->totface) {
		rewind(fp);
	
		fscanf(fp, "%40s", s);
		fscanf(fp, "%d\n", &verts);
		for(a=0;a<verts;a++) {
			fscanf(fp, "%f %f %f", s, s+4, s+8);
		}
		
		a= me->totface;
		mface= me->mface;
		while(a--) {
			end= fscanf(fp,"%d", &poly);
			if(end<=0) break;
	
			if(poly==3 || poly==4) {
				fscanf(fp,"%d", &nr);
				mface->v1= nr;
				fscanf(fp,"%d", &nr);
				mface->v2= nr;
				fscanf(fp,"%d", &nr);
				mface->v3= nr;
				if(poly==4) {
					if( fscanf(fp,"%d", &nr) <=0 ) break;
					mface->v4= nr;
				}
				mface->edcode= 3;
				
				test_index_mface(mface, poly);
				
				mface++;
			}
			else {
				if( fscanf(fp,"%d", &nr0) <=0) break;
				first= nr0;
				for(b=1; b<poly; b++) {
					end= fscanf(fp,"%d", &nr);
					if(end<=0) break;
					mface->v1= nr;
					mface->v2= nr0;
					nr0= nr;
					mface++;
					a--;
				}
				mface->v1= first;
				mface->v2= nr;
				mface->edcode= 1;
				mface++;
				if(end<=0) break;
			}
			end= fscanf(fp,"%i", &col);
			col &= 0xF0F0F0;
			if(end<=0) break;
			
			for(b=0; b<totcol; b++) {
				if(color[b]==col) {
					(mface-1)->mat_nr= b;
					break;
				}
			}
		}
	}
	
	fclose(fp);
	freeN(vertdata);
	
	G.obedit= ob;
	make_editMesh();
	load_editMesh();
	G.obedit= 0;
	tex_space_mesh(me);
	waitcursor(1);
}

void read_radiogour(char *str)
{
	Object *ob;
	Mesh *me;
	MVert *mvert;
	MFace *mface;
	Material *ma;
	FILE *fp;
	float *vertdata, *vd, min[3], max[3], cent[3];
	ulong col, *col1, *coldata;
	int  a, b, verts, tottria=0, totquad=0, totedge=0, poly, nr0, nr, first;
	short end, event;
	char s[50];

	fp= fopen(str, "r");
	if(fp==NULL) {
		error("Can't read file");
		return;
	}

	fscanf(fp, "%40s", s);

	fscanf(fp, "%d\n", &verts);
	if(verts<=0) {
		fclose(fp);
		error("Read error");
		return;
	}

	if(verts>65000) {
		error("too many vertices");
		fclose(fp);
		return;
	}
	
	INIT_MINMAX(min, max);
	vd= vertdata= mallocN(4*3*verts, "videoscapelezer");
	col1= coldata= mallocN(verts*4, "coldata");
	
	for(a=0; a<verts; a++) {
		fscanf(fp, "%f %f %f %i", vd, vd+1, vd+2, col1);
		DO_MINMAX(vd, min, max);
		vd+=3;
		col1++;
	}
	
	/* de vlakken tellen */
	end= 1;
	while(end>0) {
		end= fscanf(fp,"%d", &poly);
		if(end<=0) break;

		if(poly==3) tottria++;
		else if(poly==4) totquad++;
		else totedge+= poly;

		for(a=0;a<poly;a++) {
			end= fscanf(fp,"%d", &nr);
			if(end<=0) break;
		}
		if(end<=0) break;
		
		end= fscanf(fp,"%i\n", &col);

	}

	if(totedge+tottria+totquad>65000) {
		PRINT3(d, d, d, totedge, tottria, totquad);
		error("too many faces");
		freeN(vertdata);
		freeN(coldata);
		fclose(fp);
		return;
	}

	/* nieuw object */
	ob= add_object(OB_MESH);
	me= ob->data;
	me->totvert= verts;
	me->totface= totedge+tottria+totquad;
	me->flag= 0;
	me->mvert= callocN(me->totvert*sizeof(MVert), "mverts");
	if(me->totface) me->mface= callocN(me->totface*sizeof(MFace), "mface");
	me->mcol= (MCol *)coldata;
	
	/* verts */
	
	cent[0]= (min[0]+max[0])/2.0;
	cent[1]= (min[1]+max[1])/2.0;
	cent[2]= (min[2]+max[2])/2.0;
	VECCOPY(ob->loc, cent);
	ob->rot[0]= ob->rot[1]= ob->rot[2]= 0.0;
	
	a= me->totvert;
	vd= vertdata;
	mvert= me->mvert;
	while(a--) {
		VecSubf(mvert->co, vd, cent);
		mvert++;
		vd+= 3;
	}

	/* faces */
	if(me->totface) {
		rewind(fp);
	
		fscanf(fp, "%40s", s);
		fscanf(fp, "%d\n", &verts);
		for(a=0;a<verts;a++) {
			fscanf(fp, "%f %f %f %i", s, s, s, s);
		}
		
		a= me->totface;
		mface= me->mface;
		while(a--) {
			end= fscanf(fp,"%d", &poly);
			if(end<=0) break;
	
			if(poly==3 || poly==4) {
				fscanf(fp,"%d", &nr);
				mface->v1= nr;
				fscanf(fp,"%d", &nr);
				mface->v2= nr;
				fscanf(fp,"%d", &nr);
				mface->v3= nr;
				if(poly==4) {
					if( fscanf(fp,"%d", &nr) <=0 ) break;
					mface->v4= nr;
				}
				mface->edcode= 3;
				
				test_index_mface(mface, poly);
				
				mface++;
			}
			else {
				if( fscanf(fp,"%d", &nr0) <=0) break;
				first= nr0;
				for(b=1; b<poly; b++) {
					end= fscanf(fp,"%d", &nr);
					if(end<=0) break;
					mface->v1= nr;
					mface->v2= nr0;
					nr0= nr;
					mface++;
					a--;
				}
				mface->v1= first;
				mface->v2= nr;
				mface->edcode= 1;
				mface->flag= ME_SMOOTH;
				
				mface++;
				if(end<=0) break;
			}
			end= fscanf(fp,"%i", &col);
		}
	}
	
	fclose(fp);
	freeN(vertdata);
	
	G.obedit= ob;
	make_editMesh();

	load_editMesh();
	G.obedit= 0;
	tex_space_mesh(me);

	waitcursor(1);
}


void read_videoscape_lamp(char *str)
{
	Object *ob;
	Lamp *la;
	FILE *fp;
	float vec[3], *q1;
	int tot, val;
	char s[50];
	
	fp= fopen(str, "r");
	if(fp==NULL) {
		error("Can't read file");
		return;
	}

	fscanf(fp, "%40s", s);
	fscanf(fp, "%d\n", &tot);
	
	while(tot--) {
		ob= add_object(OB_LAMP);
		la= ob->data;
		
		fscanf(fp, "%d\n", &val);
		la->type= val;
		if(la->type==1) la->type= LA_SPOT;
		else if(la->type==2) la->type= LA_SUN;
		
		fscanf(fp, "%f %f\n", &la->spotsize, &la->spotblend);
		
		fscanf(fp, "%f %f %f %f\n", &la->r, &la->g, &la->b, &la->energy);		
		
		fscanf(fp, "%f %f %f\n", ob->loc, ob->loc+1, ob->loc+2);
		val= fscanf(fp, "%f %f %f\n", vec, vec+1, vec+2);
		q1= vectoquat(vec, 5, 2);
		QuatToEul(q1, ob->rot);
		
		if(val<=0) break;
		
	}
	fclose(fp);
}

void read_videoscape_nurbs(char *str)
{
	Object *ob;
	Curve *cu;
	Nurb *nu;
	BezTriple *bezt;
	BPoint *bp;
	FILE *fp;
	float vec[3], tmat[4][4], omat[3][3], imat[3][3], mat[3][3];
	int a, tot, type, val;
	char s[50];

	fp= fopen(str, "r");
	if(fp==NULL) {
		error("Can't read file");
		return;
	}

	fscanf(fp, "%40s", s);
	fscanf(fp, "%d\n", &type);
	
	if(type==5) ob= add_object(OB_SURF);
	else ob= add_object(OB_CURVE);
	cu= ob->data;
	
	fscanf(fp, "%d\n", &tot);
	fscanf(fp, "%d %d\n", &type, &val);
	
	cu->ext1= 0.002*type;
	cu->ext2= 0.002*val;

	for(a=0; a<4; a++) fscanf(fp, "%e %e %e %e\n", tmat[a], tmat[a]+1, tmat[a]+2, tmat[a]+3);

	VECCOPY(ob->loc, tmat[3]);

	Mat3CpyMat4(omat, tmat);
	Mat3ToEul(omat, ob->rot);
	EulToMat3(ob->rot, mat);
	Mat3Inv(imat, mat);
	Mat3MulMat3(tmat, imat, omat);
	
	while(tot--) {
		nu= callocstructN(Nurb, 1, "nu from exotic");
		addtail(&cu->nurb, nu);
		
		fscanf(fp, "%d\n", &type);
		nu->type= type;

		fscanf(fp, "%d %d\n", &type, &val);
		nu->pntsu= type; nu->pntsv= val;
		fscanf(fp, "%d %d\n", &type, &val);
		nu->resolu= type; nu->resolv= val;
		fscanf(fp, "%d %d\n", &type, &val);
		nu->orderu= type; nu->orderv= val;
		fscanf(fp, "%d %d\n", &type, &val);
		nu->flagu= type; nu->flagv= val;
		
		if( (nu->type & 7)==CU_BEZIER) {
			a= nu->pntsu;
			nu->bezt= bezt= callocN(a*sizeof(BezTriple), "bezt from exotic");
			while(a--) {
				fscanf(fp, "%f %f %f ", bezt->vec[0], bezt->vec[0]+1, bezt->vec[0]+2);
				Mat3MulVecfl(tmat, bezt->vec[0]);
				fscanf(fp, "%f %f %f ", bezt->vec[1], bezt->vec[1]+1, bezt->vec[1]+2);
				Mat3MulVecfl(tmat, bezt->vec[1]);
				fscanf(fp, "%f %f %f ", bezt->vec[2], bezt->vec[2]+1, bezt->vec[2]+2);
				Mat3MulVecfl(tmat, bezt->vec[2]);
				fscanf(fp, "%d %d\n", &type, &val);
				bezt->h1= type;
				bezt->h2= val;
				bezt++;
			}
		}
		else {
			a= nu->pntsu*nu->pntsv;
			if(a) {
				nu->bp= bp= callocN(a*sizeof(BPoint), "bp from exotic");
				while(a--) {
					fscanf(fp, "%f %f %f %f\n", bp->vec, bp->vec+1, bp->vec+2, bp->vec+3);
					Mat3MulVecfl(tmat, bp->vec);
					bp++;
				}
				
				val= KNOTSU(nu);
				nu->knotsu= mallocN(4*val, "knots");
				for(a=0; a<val; a++) fscanf(fp, "%f\n", nu->knotsu+a);
				
				if(nu->pntsv>1) {
					val= KNOTSV(nu);
					nu->knotsv= mallocN(4*val, "knots");
					for(a=0; a<val; a++) fscanf(fp, "%f\n", nu->knotsv+a);
				}
			}
			else {
				remlink(&cu->nurb, nu);
				freeN(nu);
			}
		}
	}
	fclose(fp);
	makeDispList(ob);
}

void read_videoscape(char *str)
{
	int file, type;
	ushort val, numlen;
	char name[100], head[100], tail[100];
	
	strcpy(name, str);
	
	while( TRUE ) {
		file= open(name, O_RDONLY);
		if(file<=0) break;
		else {
			read(file, &type, 4);
			close(file);
			
			if(type==DDG1) read_videoscape_mesh(name);
			else if(type==DDG2) read_videoscape_lamp(name);
			else if(type==DDG3) read_videoscape_nurbs(name);
		}

		val = stringdec(name, head, tail, &numlen);
		stringenc(name, head, tail, numlen, val + 1);

	}
	allqueue(REDRAWVIEW3D, 0);	
}


/* ***************** INVENTOR ******************* */


#define IV_MAXSTACK 100000
#define IV_MAXFIELD 10
#define IV_MAXCOL 16

float *iv_data_stack;
float ivcolors[IV_MAXCOL][3];

ListBase ivbase, *nurbsbase;

struct IvNode {
	struct IvNode *next, *prev;
	char *nodename;
	char *fieldname[IV_MAXFIELD];
	int datalen[IV_MAXFIELD];
	float *data[IV_MAXFIELD];
};

int iv_curcol=0;

int iv_colornumber(struct IvNode *iv)
{
	float *fp, fr, fg, fb;
	int a;
	char *cp;
	
	/* terugzoeken naar laatste materiaal */
	while(iv) {
		if( strcmp(iv->nodename, "Material ")==0) {
			fp= iv->data[0];
			if(fp==0) fp= iv->data[1];
			if(fp) {
				fr= fp[0];
				fg= fp[1];
				fb= fp[2];
			}
			break;
		}
		else if( strcmp(iv->nodename, "BaseColor ")==0) {
			fp= iv->data[0];
			fr= fp[0];
			fg= fp[1];
			fb= fp[2];
			break;
		}
		else if( strcmp(iv->nodename, "PackedColor ")==0) {
			cp= (char *)iv->data[0];
			fr= cp[3]/255.0;
			fg= cp[2]/255.0;
			fb= cp[1]/255.0;
			break;
		}
		iv= iv->prev;
		
	}
	if(iv==0) return 0;
	if(iv->datalen[0]<3) return 0;
	
	for(a=0; a<iv_curcol; a++) {
	
		if(ivcolors[a][0]== fr)
			if(ivcolors[a][1]== fg)
				if(ivcolors[a][2]== fb) return a+1
				;
	}
	
	if(a>=IV_MAXCOL) a= IV_MAXCOL-1;
	iv_curcol= a+1;
	ivcolors[a][0]= fr;
	ivcolors[a][1]= fg;
	ivcolors[a][2]= fb;
	
	return iv_curcol;
}

int iv_finddata(struct IvNode *iv, char *field, int fieldnr)
{
	/* zoek naar "field", tel lengte data en maak datablok. return skipdata */
	float *fp;
	int len, stackcount, skipdata=0;
	char *cpa, terminator, str[32];
	
	len= strlen(field);
	
	cpa= iv->nodename+1;
	while( *cpa != '}' ) {
		
		if( *cpa == *field ) {
			if( strncmp(cpa, field, len)==0 ) {
				iv->fieldname[fieldnr]= cpa;
				
				/* lezen tot aan eerste karakter */
				cpa+= len;
				skipdata+= len;
				*cpa= 0;
				cpa++;
				skipdata++;
				
				while( *cpa==' ') cpa++;
				if( *cpa=='[' ) {
					terminator= ']';
					cpa++;
					skipdata++;
				}
				else terminator= 13;
				
				stackcount= 0;
				fp= iv_data_stack;
				while( *cpa!=terminator && *cpa != '}' ) {
					
					if( isspace(*cpa)==0 && (isspace(cpa[-1]) || cpa[-1]==0) ) {
						
						if(cpa[1]=='x') {
							memcpy(str, cpa, 16);
							str[16]= 0;
							sscanf(str, "%x", fp);
						}
						else *fp= atof(cpa);

						stackcount++;
						if(stackcount>=IV_MAXSTACK) {
							printf("stackoverflow in IV read\n");
							break;
						}
						fp++;
						
					}
					cpa++;
					skipdata++;
				}
				
				iv->datalen[fieldnr]= stackcount;
				if(stackcount) {
					iv->data[fieldnr]= mallocN(4*stackcount, "iv_finddata");
					memcpy(iv->data[fieldnr], iv_data_stack, 4*stackcount);
				}
				else iv->data[fieldnr]= 0;
				
				return skipdata;
			}
		}
		cpa++;
		skipdata++;
	}
	
	return skipdata;
}

void read_iv_index(float *data, float *baseadr, float *index, int nr, int coordtype)
{
	/* in data schrijven: baseadr met offset index (aantal nr)*/
	float *fp;
	int ofs;
	
	while(nr--) {
		ofs= *index;
		fp= baseadr+coordtype*ofs;
		VECCOPY(data, fp);
		data+= 3;
		index++;
	}
}



void read_inventor(char *str, struct ListBase *listb)
{
	struct IvNode *iv, *ivp, *ivn;
	char *maindata, *md, *cpa;
	float *index, *data, *fp;
	long file, filelen, count, face, nr;
	int skipdata, asnurbs=0, ok, a, b, tot, first, colnr, coordtype, polytype, *idata;
	struct DispList *dl, *dln;
	
	ivbase.first= ivbase.last= 0;
	iv_curcol= 0;
	
	file= open(str, O_RDONLY);
	if(file== -1) {
		error("Can't read file\n");
		return;
	}
	filelen= lseek(file, 0, 2);	/* seek end */
	lseek(file, 0, 0);		/* en weer terug */
	maindata= mallocN(filelen, "leesInventor");
	read(file, maindata, filelen);
	close(file);

	iv_data_stack= mallocN(4*IV_MAXSTACK, "ivstack");

	/* we gaan alles ordenen: welke zijn de nodes en de fields? */
	md= maindata;
	count= 0;
	while(count<filelen) {
		if( *md=='{' ) {	/* terug lezen */
		
			cpa= md-2;
			while( *cpa==32) cpa--;		/* spaties weg */
			while( *cpa>32 && *cpa<128) cpa--;
			cpa++;
			*md= 0;
			
			ok= 0;
			skipdata= 0;
			iv= callocN(sizeof(struct IvNode), "leesInventor");
			iv->nodename= cpa;

			if(strcmp(cpa, "Coordinate3 ")==0 || strcmp(cpa, "Coordinate4 ")==0) {
				skipdata= iv_finddata(iv, "point", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "VertexProperty ")==0) {
				skipdata= iv_finddata(iv, "vertex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedLineSet ")==0) {
				skipdata= iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedTriangleMesh ")==0) {
				skipdata= iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedFaceSet ")==0) {
				skipdata= iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "FaceSet ")==0) {
				skipdata= iv_finddata(iv, "numVertices", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "Material ")==0) {
				iv_finddata(iv, "diffuseColor", 0);
				iv_finddata(iv, "ambientColor", 1);
				ok= 1;
			}
			else if(strcmp(cpa, "BaseColor ")==0) {
				iv_finddata(iv, "rgb", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "PackedColor ")==0) {
				iv_finddata(iv, "rgba", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "QuadMesh ")==0) {
				iv_finddata(iv, "verticesPerColumn", 0);
				iv_finddata(iv, "verticesPerRow", 1);
				
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedTriangleStripSet ")==0) {
				skipdata= iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "TriangleStripSet ")==0) {
				skipdata= iv_finddata(iv, "numVertices", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedNurbsSurface ")==0 || strcmp(cpa, "NurbsSurface ")==0) {
				iv_finddata(iv, "numUControlPoints", 0);
				iv_finddata(iv, "numVControlPoints", 1);
				iv_finddata(iv, "uKnotVector", 2);
				iv_finddata(iv, "vKnotVector", 3);
				ok= 1;
			}
			else {
				/* naar 't einde */
				while( *md != '}') {
					md++;
					count++;
					if(count<filelen) break;
				}
			}
			
			
			if(ok) {
				addtail(&ivbase, iv);
				md+= skipdata;
				count+= skipdata;
			}
			else freeN(iv);
			
		}
		md++;
		count++;
	}
	
	/* nodes samenvoegen */
	iv= ivbase.first;
	while(iv) {
		ivn= iv->next;
		
		if( strncmp(iv->nodename, "Indexed", 7)==0) {
			/* terugzoeken: zelfde naam? */
			
			ivp= iv->prev;
			while(ivp) {
				if(strcmp(iv->nodename, ivp->nodename)==0) break;

				if(strcmp(ivp->nodename, "Coordinate3 ")==0 || 
				   strcmp(ivp->nodename, "Coordinate4 ")==0 ||
				   strcmp(ivp->nodename, "VertexProperty ")==0) {
					ivp= 0;
					break;
				}
				ivp= ivp->prev;
			}
			
			if(ivp) {
				/* iv bij ivp voegen */
				
				tot= iv->datalen[0] + ivp->datalen[0];
				data= mallocN(tot*4, "samenvoeg iv");
				memcpy(data, ivp->data[0], 4*ivp->datalen[0]);
				memcpy(data+ivp->datalen[0], iv->data[0], 4*iv->datalen[0]);
				
				ivp->datalen[0]+= iv->datalen[0];
				freeN(ivp->data[0]);
				ivp->data[0]= data;
				
				remlink(&ivbase, iv);
				freeN(iv->data[0]);
				freeN(iv);
			}
		}
		
		iv= ivn;
	}

	
	/* Nodes omzetten naar DispLists */
	iv= ivbase.first;
	while(iv) {
		
		/* printf(" Node: %s\n", iv->nodename); */
		/* if(iv->fieldname[0]) printf(" Field: %s len %d\n", iv->fieldname[0], iv->datalen[0]); */
		coordtype= 3;
		
		if( strcmp(iv->nodename, "IndexedLineSet ")==0 ) {
			
			colnr= iv_colornumber(iv);

			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			if(ivp) {
			
				/* tel het aantal lijnen */
				tot= 0;
				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-1; a++) {
					if(index[0]!= -1 && index[1]!= -1) tot++;
					index++;
				}
				
				tot*= 2;	/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor1");
				addtail(listb, dl);
				dl->type= DL_SEGM;
				dl->nr= 2;
				dl->parts= tot/2;
				dl->col= colnr;
				data= (float *)(dl+1);
				
				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-1; a++) {
					if(index[0]!= -1 && index[1]!= -1) {
						read_iv_index(data, ivp->data[0], index, 2, coordtype);
						data+= 6;
					}
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "FaceSet ")==0 ) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			
			if(ivp) {
				/* tel het aantal driehoeken */
				tot= 0;
				
				index= iv->data[0];
				polytype= index[0];
				
				for(a=0; a<iv->datalen[0]; a++) {
					if(index[0]== polytype) tot++;	/* een soort? */
					index++;
				}
				
				
				tot*= polytype;		/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor4");
				addtail(listb, dl);
				dl->type= DL_POLY;
				dl->nr= polytype;
				dl->parts= tot/polytype;
				dl->col= colnr;
				data= (float *)(dl+1);

				index= ivp->data[0];
				first= 1;
				for(a=0; a<iv->datalen[0]; a++) {
					
					VECCOPY(data, index);
					data+= 3;
					index+= 3;

					VECCOPY(data, index);
					data+= 3;
					index+= 3;

					VECCOPY(data, index);
					data+= 3;
					index+= 3;

					if(polytype==4) {
						VECCOPY(data, index);
						data+= 3;
						index+= 3;
					}
				}
			}
		}
		else if( strcmp(iv->nodename, "TriangleStripSet ")==0 ) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			
			if(ivp) {
				/* tel het aantal driehoeken */
				tot= 0;
				face= 0;
				
				index= iv->data[0];		/* afmeting strip */ 
				
				for(a=0; a<iv->datalen[0]; a++) {
					tot+= index[0];
					face+= index[0]-2;
					index++;
				}
				
				dl= callocN(sizeof(struct DispList), "leesInventor4");
				dl->verts= callocN( tot*3*4, "dl verts");
				dl->index= callocN( face*3*4, "dl index");
				
				dl->type= DL_INDEX3;
				dl->nr= tot;
				dl->parts= face;

				addtail(listb, dl);
				dl->col= colnr;

				index= iv->data[0];		/* afmeting strip */ 
				fp= ivp->data[0];		/* vertices */
				data= dl->verts;
				idata= dl->index;
				first= 0;
				
				for(a=0; a<iv->datalen[0]; a++) {
					
					/* vertices */
					for(b=0; b<index[0]; b++) {
						VECCOPY(data, fp);
						data+= 3; 
						fp+= coordtype;
					}
						
					/* indices */
					for(b=0; b<index[0]-2; b++) {
						idata[0]= first;
						idata[1]= first+1;
						idata[2]= first+2;
						first++;
						idata+= 3;
					}
					first+= 2;
					
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "IndexedFaceSet ")==0 ) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			if(ivp) {
			
				/* tel het aantal driehoeken */
				face= 0;
				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) face++;
					index++;
				}
				
				/* aantal vertices */
				tot= ivp->datalen[0]/coordtype;

				dl= callocN(sizeof(struct DispList), "leesInventor5");
				addtail(listb, dl);
				dl->type= DL_INDEX3;
				dl->nr= tot;
				dl->parts= face;
				dl->col= colnr;

				dl->verts= callocN( tot*3*4, "dl verts");
				dl->index= callocN(4*3*face, "dl index");

				/* vertices */
				fp= ivp->data[0];
				data= dl->verts;
				for(b=tot; b>0; b--) {
					VECCOPY(data, fp);
					data+= 3; 
					fp+= coordtype;
				}
				
				/* indices */
				index= iv->data[0];
				idata= dl->index;
				first= 1;
				
				for(a=0; a<iv->datalen[0]-2; a++) {
					
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) {

						/* deze truuk is om poly's met meer dan 3 vertices correct te vullen */
						if(first) {
							nr= index[0];
							first= 0;
						}
						idata[0]= nr;
						idata[1]= index[1];
						idata[2]= index[2];
						
						idata+= 3;
					}
					else first= 1;
					
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "IndexedTriangleMesh ")==0 || 
				 strcmp(iv->nodename, "IndexedTriangleStripSet ")==0 ) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			if(ivp) {
			
				/* tel het aantal driehoeken */
				face= 0;
				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) face++;
					index++;
				}
				
				/* aantal vertices */
				tot= ivp->datalen[0]/coordtype;
				
				dl= callocN(sizeof(struct DispList), "leesInventor6");
				addtail(listb, dl);
				dl->type= DL_INDEX3;
				dl->nr= tot;
				dl->parts= face;
				dl->col= colnr;
				
				dl->verts= callocN( tot*3*4, "dl verts");
				dl->index= callocN(4*3*face, "dl index");

				/* vertices */
				fp= ivp->data[0];
				data= dl->verts;
				for(b=tot; b>0; b--) {
					VECCOPY(data, fp);
					data+= 3; 
					fp+= coordtype;
				}
				
				/* indices */
				index= iv->data[0];
				idata= dl->index;
				
				for(a=iv->datalen[0]-2; a>0; a--) {
				
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) {
						idata[0]= index[0];
						idata[1]= index[1];
						idata[2]= index[2];
						idata+= 3;
					}
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "QuadMesh ")==0 ) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "VertexProperty ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			
			if(ivp) {
				tot= ffloor(*(iv->data[0])+0.5) * ffloor(*(iv->data[1])+0.5);

				if(tot>0) {
					dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor8");
					addtail(listb, dl);
					dl->type= DL_SURF;
					dl->parts= ffloor(*(iv->data[0])+0.5);
					dl->nr= ffloor(*(iv->data[1])+0.5);
					dl->col= colnr;
					data= (float *)(dl+1);
					memcpy(data, ivp->data[0], tot*3*4);
				}
			}
		}
		else if(strcmp(iv->nodename, "IndexedNurbsSurface ")==0 || strcmp(iv->nodename, "NurbsSurface ")==0) {
			
			colnr= iv_colornumber(iv);
		
			/* terugzoeken naar data */
			ivp= iv;
			while(ivp->prev) {
				ivp= ivp->prev;
				if( strcmp(ivp->nodename, "Coordinate3 ")==0 ) {
					coordtype= 3;
					break;
				}
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			if(ivp) {
				a= *(iv->data[0]);
				b= *(iv->data[1]);
				
				tot= a*b;
				
				asnurbs= 2;		/* tijdelijk */
				
				if(asnurbs==0) {
					if(okee("Read as Nurbs")) asnurbs= 1;
					else asnurbs= 2;
				}
				
				if( (a>4 || b>4) && tot>6 && asnurbs==1) {
					struct Nurb *nu;
					struct BPoint *bp;
					
					/* new_nubase(); */
					
					nu= callocstructN(struct Nurb, 1, "addNurbprim");
					addtail(nurbsbase, nu);
					nu->type= 4;

					nu->resolu=nu->pntsu= a;
					nu->resolv=nu->pntsv= b;

					nu->flagu= 0;
					nu->flagv= 0;
					
					nu->bp=bp= callocstructN(struct BPoint, tot, "addNurbprim3");
					
					a= tot;
					data= ivp->data[0];
					while(a--) {
						VECCOPY(bp->vec, data);
						bp->vec[3]= 1.0;
						data+= coordtype;
						bp++;
					}
					
					
					nu->orderu= iv->datalen[2] - nu->pntsu;
					nu->orderv= iv->datalen[3] - nu->pntsv;
					
					nu->knotsu= mallocN( 4*(iv->datalen[2]), "knots");
					memcpy(nu->knotsu, iv->data[2], 4*(iv->datalen[2]));
					nu->knotsv= mallocN( 4*(iv->datalen[3]), "knots");
					memcpy(nu->knotsv, iv->data[3], 4*(iv->datalen[3]));					
					
				}
				else {
					dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor3");
					addtail(listb, dl);
					dl->type= DL_SURF;
					dl->nr= *(iv->data[0]);
					dl->parts= *(iv->data[1]);
					dl->col= colnr;
					data= (float *)(dl+1);
					
					a= tot;
					fp= ivp->data[0];
					while(a--) {
						VECCOPY(data, fp);
						fp+= coordtype;
						data+= 3;
					}
				}
			}
		}
		iv= iv->next;
	}

	/* vrijgeven */
	iv= ivbase.first;
	while(iv) {
		for(a=0; a<IV_MAXFIELD; a++) {
			if(iv->data[a]) freeN(iv->data[a]);
		}
		iv= iv->next;
	}

	freelistN(&ivbase);
	freeN(maindata);
	freeN(iv_data_stack);
	
	if(nurbsbase) {
		nurbsbase= 0;
		/* makeDispList(nubase); */
	}
}

/* ************************************************************ */

void displist_to_mesh(DispList *dlfirst, char *name)
{
	Object *ob;
	Mesh *me;
	Material *ma;
	DispList *dl;
	MVert *mvert;
	MFace *mface;
	float *vertdata, *data, *vd, vec[3], min[3], max[3], afm[3], fac;
	int end, a, b, startve, *idata, totedge=0, tottria=0, totquad=0, totvert=0, totvlak, totcol=0, colnr;
	int p1, p2, p3, p4;
	char str[100];
		
	/* eerst tellen */
	INIT_MINMAX(min, max);

	dl= dlfirst;
	while(dl) {
	
		/* PATCH 1 (polyfill) kan hier niet, er wordt geen listbase meegegeven. eerder doen! */
		/* PATCH 2 */
		if(dl->type==DL_SEGM && dl->nr>2) {
			data= (float *)(dl+1);
			if(data[0]==data[3*(dl->nr-1)]) {
				if(data[1]==data[3*(dl->nr-1)+1]) {
					if(data[2]==data[3*(dl->nr-1)+2]) {
						dl->type= DL_POLY;
						dl->nr--;
					}
				}
			}
		}
		
		/* kleuren */
		if(dl->col > totcol) totcol= dl->col;
		
		/* afmeting en tellen */
		if(dl->type==DL_SURF) {
			a= dl->nr;
			b= dl->parts;
			if(dl->flag & 1) a++;
			if(dl->flag & 2) b++;
			
			totquad+= a*b;

			totvert+= dl->nr*dl->parts;

			data= (float *)(dl+1);
			for(a= dl->nr*dl->parts; a>0; a--) {
				DO_MINMAX(data, min, max);
				data+= 3;
			}
		}
		else if(dl->type==DL_POLY) {
			if(dl->nr==3 || dl->nr==4) {
				if(dl->nr==3) tottria+= dl->parts;
				else totquad+= dl->parts;
				
				totvert+= dl->nr*dl->parts;

				data= (float *)(dl+1);
				for(a= dl->nr*dl->parts; a>0; a--) {
					DO_MINMAX(data, min, max);
					data+= 3;
				}
			}
			else if(dl->nr>4) {
				
				tottria+= dl->nr*dl->parts;
				totvert+= dl->nr*dl->parts;
				
				data= (float *)(dl+1);
				for(a= dl->nr*dl->parts; a>0; a--) {
					DO_MINMAX(data, min, max);
					data+= 3;
				}
				
			}
		}
		else if(dl->type==DL_INDEX3) {
			tottria+= dl->parts;
			totvert+= dl->nr;
			
			data= dl->verts;
			for(a= dl->nr; a>0; a--) {
				DO_MINMAX(data, min, max);
				data+= 3;
			}
		}
		else if(dl->type==DL_SEGM) {
			
			tottria+= (dl->nr-1)*dl->parts;
			totvert+= dl->nr*dl->parts;
			
			data= (float *)(dl+1);
			for(a= dl->nr*dl->parts; a>0; a--) {
				DO_MINMAX(data, min, max);
				data+= 3;
			}
		}

		dl= dl->next;
	}

	if(totvert==0) {
		return;
	}
	if(totvert>=42760 || tottria>=42760) {
		sprintf(str, "Too many faces (%d) or vertices (%d)", tottria, totvert);
		error(str);

		return;
	}
	
	if(totcol>16) {
		error("Found more than 16 different colors");
		totcol= 16;
	}

	afm[0]= (max[0]-min[0])/2;
	afm[1]= (max[1]-min[1])/2;
	afm[2]= (max[2]-min[2])/2;
	vec[0]= (min[0]+max[0])/2;
	vec[1]= (min[1]+max[1])/2;
	vec[2]= (min[2]+max[2])/2;

	ob= add_object(OB_MESH);
	VECCOPY(ob->loc, vec);
	ob->rot[0]= ob->rot[1]= ob->rot[2]= 0.0;	/* is anders viewrot */
	where_is_object(ob);

	me= ob->data;
	
	/* kleuren */
	if(totcol) {
		ob->mat= callocN(4*totcol, "ob->mat");
		me->mat= callocN(4*totcol, "me->mat");
		ob->totcol= me->totcol= totcol;
		ob->actcol= 1;
	}
	
	/* materialen */
	for(a=0; a<totcol; a++) {
		ma= G.main->mat.first;
		while(ma) {
			if(ma->mtex[0]==0) {
				if(ivcolors[a][0]==ma->r && ivcolors[a][1]==ma->g && ivcolors[a][2]==ma->b) {
					me->mat[a]= ma;
					ma->id.us++;
					break;
				}
			}
			ma= ma->id.next;
		}
		if(ma==0) {
			ma= add_material("ext");
			me->mat[a]= ma;
			ma->r= ivcolors[a][0];
			ma->g= ivcolors[a][1];
			ma->b= ivcolors[a][2];
			automatname(ma);
		}
	}
	
	totvlak= totquad+tottria+totedge;

	printf("Import: %d vertices %d faces\n", totvert, totvlak);
	
	if(totvert) me->mvert= callocN(totvert*sizeof(MVert), "mvert");
	if(totvlak) me->mface= callocN(totvlak*sizeof(MFace), "mface");
	me->totvert= totvert;
	me->totface= totvlak;
	
	mvert= me->mvert;
	mface= me->mface;

	startve= 0;

	dl= dlfirst;
	while(dl) {
		
		colnr= (dl->col>15 ? 15: dl->col);
		if(colnr) colnr--;
		
		if(dl->type==DL_SURF) {
			data= (float *)(dl+1);

			for(a=dl->parts*dl->nr; a>0; a--) {
				mvert->co[0]= data[0] -vec[0];
				mvert->co[1]= data[1] -vec[1];
				mvert->co[2]= data[2] -vec[2];
				
				data+=3;
				mvert++;
			}

			for(a=0; a<dl->parts; a++) {

				DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);
				p1+= startve; 
				p2+= startve; 
				p3+= startve; 
				p4+= startve;

				for(; b<dl->nr; b++) {
				
					mface->v1= p1;
					mface->v2= p2;
					mface->v3= p4;
					mface->v4= p3;
					
					mface->mat_nr= colnr;
					test_index_mface(mface, 4);
					
					mface++;
					
					p4= p3; 
					p3++;
					p2= p1; 
					p1++;
				}
			}
			
			startve += dl->parts*dl->nr;

		}
		else if(dl->type==DL_POLY) {
		
			if(dl->nr==3 || dl->nr==4) {
				data= (float *)(dl+1);

				for(a=dl->parts*dl->nr; a>0; a--) {
					mvert->co[0]= data[0] -vec[0];
					mvert->co[1]= data[1] -vec[1];
					mvert->co[2]= data[2] -vec[2];
					data+=3;
					mvert++;
				}

				for(a=0; a<dl->parts; a++) {
					if(dl->nr==3) {
						mface->v1= startve+a*dl->nr;
						mface->v2= startve+a*dl->nr+1;
						mface->v3= startve+a*dl->nr+2;
						mface->mat_nr= colnr;
						test_index_mface(mface, 3);
						mface++;
					}
					else {
						mface->v1= startve+a*dl->nr;
						mface->v2= startve+a*dl->nr+1;
						mface->v3= startve+a*dl->nr+2;
						mface->v4= startve+a*dl->nr+3;
						mface->mat_nr= colnr;
						test_index_mface(mface, 4);
						mface++;
					}
				}
				startve += dl->parts*dl->nr;
			}
			else if(dl->nr>4) {
				data= (float *)(dl+1);

				for(a=dl->parts*dl->nr; a>0; a--) {
					mvert->co[0]= data[0] -vec[0];
					mvert->co[1]= data[1] -vec[1];
					mvert->co[2]= data[2] -vec[2];
					
					data+=3;
					mvert++;
				}

				for(b=0; b<dl->parts; b++) {
					for(a=0; a<dl->nr; a++) {
						mface->v1= startve+a;
						
						if(a==dl->nr-1) mface->v2= startve;
						else mface->v2= startve+a+1;
						
						mface->mat_nr= colnr;
						test_index_mface(mface, 2);

						mface++;
					}
					startve += dl->nr;
				}
			}
		}
		else if(dl->type==DL_INDEX3) {
			data= dl->verts;
			
			for(a=dl->nr; a>0; a--) {
				mvert->co[0]= data[0] -vec[0];
				mvert->co[1]= data[1] -vec[1];
				mvert->co[2]= data[2] -vec[2];
				data+=3;
				mvert++;
			}
			
			idata= dl->index;
			for(b=dl->parts; b>0; b--) {
				mface->v1= startve+idata[0];
				mface->v2= startve+idata[1];
				mface->v3= startve+idata[2];
				mface->mat_nr= colnr;
				test_index_mface(mface, 3);
				mface++;
				idata+= 3;
			}
			startve += dl->nr;
		}
		else if(dl->type==DL_SEGM) {
			data= (float *)(dl+1);

			for(a=dl->parts*dl->nr; a>0; a--) {
				mvert->co[0]= data[0] -vec[0];
				mvert->co[1]= data[1] -vec[1];
				mvert->co[2]= data[2] -vec[2];
				data+=3;
				mvert++;
			}

			for(b=0; b<dl->parts; b++) {
				for(a=0; a<dl->nr-1; a++) {
					mface->v1= startve+a;
					mface->v2= startve+a+1;
					mface->mat_nr= colnr;
					test_index_mface(mface, 2);
					mface++;
				}
				startve += dl->nr;
			}
		}
		dl= dl->next;
	}

	G.obedit= ob;
	make_editMesh();
	load_editMesh();
	G.obedit= 0;
	tex_space_mesh(me);

}

void displist_to_objects(char *name, ListBase *lbase)
{
	DispList *dl, *first, *prev, *next;
	ListBase tempbase, dltemp;
	int maxaantal, curcol, totvert=0, vert;
	char str1[64];
	
	dl= lbase->first;
	while(dl) {
		next= dl->next;
		
		/* PATCH 1: polyfill */
		if(dl->type==DL_POLY && dl->nr>4) {
			/* oplossing: bij elkaar in aparte listbase zetten */
			;
		}
		/* PATCH 2: poly's van 2 punten */
		if(dl->type==DL_POLY && dl->nr==2) dl->type= DL_SEGM;
		
		dl= next;
	}

	/* vertices tellen */

	dl= lbase->first;
	while(dl) {

		if(dl->type==DL_SURF) totvert+= dl->nr*dl->parts;
		else if(dl->type==DL_POLY) {
			if(dl->nr==3 || dl->nr==4) totvert+= dl->nr*dl->parts;
			else if(dl->nr>4) totvert+= dl->nr*dl->parts;
		}
		else if(dl->type==DL_INDEX3) totvert+= dl->nr;
		else if(dl->type==DL_SEGM) totvert+= dl->nr*dl->parts;

		dl= dl->next;
	}

	if(totvert==0) {
		
		error("Found no data");
		if(lbase->first) freelistN(lbase);
		
		return;
	}

	/* splitdirstring(str, name); */
	/* name[14]= 0; */
	/* for(a=0; a<14; a++) if(name[a]=='.') name[a] =0; */

	maxaantal= 32000;
	
	if(totvert>maxaantal) {
	
		/* probeer kleuren bij elkaar te zetten */
		curcol= 0;
		tempbase.first= tempbase.last= 0;

		while(lbase->first) {
			dl= lbase->first;
			while(dl) {
				next= dl->next;
				if(dl->col==curcol) {
					remlink(lbase, dl);
					addtail(&tempbase, dl);
					dl->col= 0;
				}
				
				dl= next;
			}
			
			/* in tempbase zitten alle kleuren 'curcol' */
			totvert= 0;
			dl= first= tempbase.first;
			while(dl) {
				vert= 0;
				
				if(dl->type==DL_SURF) vert= dl->nr*dl->parts;
				else if(dl->type==DL_POLY) {
					if(dl->nr==3 || dl->nr==4) vert= dl->nr*dl->parts;
					else if(dl->nr>4) vert= dl->nr*dl->parts;
				}
				else if(dl->type==DL_INDEX3) totvert+= dl->nr;
				else if(dl->type==DL_SEGM) vert= dl->nr*dl->parts;
				
				totvert+= vert;
				if(totvert > maxaantal || dl->next==0) {
					if(dl->next==0) {
						displist_to_mesh(first, name);
					}
					else if(dl->prev) {
						prev= dl->prev;
						prev->next= 0;
						displist_to_mesh(first, name);
						prev->next= dl;
						first= dl;
						totvert= 0;
					}
				}
				
				dl= dl->next;
			}
			
			freedisplist(&tempbase);
			
			curcol++;
		}
	}
	else displist_to_mesh(lbase->first, name);

	freedisplist(lbase);

}


void read_exotic(char *name)
{
	ListBase lbase={0, 0};
	int file;
	int s0;
	char str[32];

	file= open(name, O_RDONLY);
	if(file<=0) {
		error("Can't open file");
		return;
	}
	read(file, &s0, 4);
	if(s0!=FORM) {
		read(file, str, 24);
		close(file);

		waitcursor(1);
		
		if(s0==GOUR) {
			error("Sorry, under construction: use 1.0");
			/*
			if(G.obedit) {
				if(G.obedit->type==OB_MESH) insert_radiogour(name);
			}
			else read_radiogour(name);
			*/
			
		}
		else if ELEM4(s0, DDG1, DDG2, DDG3, DDG4) {
			if(G.obedit) error("Not in editmode");
			else read_videoscape(name);
			
			strcpy(G.sce, name);
		}
		else if(strncmp((char *)&s0, "#Inv", 4)==0) {

			if( strncmp(str, "entor V1.0 ascii", 16)==0) {
				read_inventor(name, &lbase);
				displist_to_objects(name, &lbase);				
			}
			else error("Can only read Inventor 1.0 ascii");

			strcpy(G.sce, name);
		}
		else {				/* DXF */
			FILE *fp;
			
			error("Unknown file type");
		}
		
		waitcursor(0);
	}
}


/* ************************ WRITE ************************** */



char videosc_dir[80]= {0, 0};

void write_videoscape(char *str)
{
	Object *ob;
	Mesh *me;
	Material *ma;
	MVert *mvert;
	MFace *mface;
	FILE *fp;
	EditVert *eve;
	EditVlak *evl;
	ulong kleur[32];
	float co[3];
	int a, tot, file;
	char *cp;
	
	ob= OBACT;
	if(ob && ob->type==OB_MESH);
	else {
		error("No active Mesh");
		return;
	}

	if(testextensie(str,".trace")) str[ strlen(str)-6]= 0;
	if(testextensie(str,".blend")) str[ strlen(str)-6]= 0;
	if(testextensie(str,".obj")==0) strcat(str, ".obj");

	file= open(str, O_RDONLY);
	close(file);
	if(file>-1) if(saveover(str)==0) return;
	
	cp= (char *)kleur;
	for(a=0; a<ob->totcol; a++, cp+=4) {
		
		ma= give_current_material(ob, a+1);
		if(ma) {
			cp[0]= 255.0*ma->emit;
			cp[1]= 255.0*ma->b;
			cp[2]= 255.0*ma->g;
			cp[3]= 255.0*ma->r;
		}
		else kleur[a]= 0x00707070;
		
		if(a>30) break;
	}
	
	fp= fopen(str, "w");
	if(fp==NULL) return;

	fprintf(fp,"3DG1\n");

	if(G.obedit) {

		fprintf(fp, "%d\n", G.totvert);
	
		tot= 0;
		eve= G.edve.first;
		while(eve) {
			VECCOPY(co, eve->co);
			Mat4MulVecfl(ob->obmat, co);
			fprintf(fp, "%f %f %f\n", co[0], co[1], co[2] );
			eve->vn= (struct EditVert *)tot;
			tot++;
			eve= eve->next;
		}
		evl= G.edvl.first;
		while(evl) {

			if(evl->v4==0) {
				fprintf(fp, "3 %d %d %d 0x%x\n", evl->v1->vn, evl->v2->vn, evl->v3->vn, kleur[evl->mat_nr]);
			}
			else {
				fprintf(fp, "4 %d %d %d %d 0x%x\n", evl->v1->vn, evl->v2->vn, evl->v3->vn, evl->v4->vn, kleur[evl->mat_nr]);
			}
			evl= evl->next;
		}
	}
	else {
		DispList *dl;
		float *extverts=0;
		
		dl= test_displist(&ob->disp, DL_VERTS);
		if(dl) extverts= dl->verts;

		me= ob->data;
		
		fprintf(fp, "%d\n", me->totvert);
		
		mvert= me->mvert;
		mface= me->mface;
		for(a=0; a<me->totvert; a++, mvert++) {
			if(extverts) {
				VECCOPY(co, extverts);
				extverts+= 3;
			}
			else {
				VECCOPY(co, mvert->co);
			}
			Mat4MulVecfl(ob->obmat, co);
			fprintf(fp, "%f %f %f\n", co[0], co[1], co[2] );
		}
		for(a=0; a<me->totface; a++, mface++) {
			if(mface->v3==0) {
				fprintf(fp, "2 %d %d 0x%x\n", mface->v1, mface->v2, kleur[mface->mat_nr]);
			}
			if(mface->v4==0) {
				fprintf(fp, "3 %d %d %d 0x%x\n", mface->v1, mface->v2, mface->v3, kleur[mface->mat_nr]);
			}
			else {
				fprintf(fp, "4 %d %d %d %d 0x%x\n", mface->v1, mface->v2, mface->v3, mface->v4, kleur[mface->mat_nr]);
			}
		}
	}
	
	fclose(fp);
	
	strcpy(videosc_dir, str);
}


