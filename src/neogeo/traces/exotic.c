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

#include <stdio.h>

#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include "/usr/people/include/Trace.h"
#include "/usr/people/include/Button.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

extern void VecMulf(float *v1, float f);


/* ********* DXF tempstructen en globals */

struct DxfLayer {
	struct DxfLayer *next,  *prev;
	char name[32];
	int nr;
};

struct DxfBlock {
	struct DxfBlock *next,  *prev;
	char name[32];
	struct ListBase disp;
};

struct ListBase DxfLaybase = {0, 0};
struct ListBase DxfBlockbase = {0, 0};

/* ********** DXF hulproutines ********* */

struct ListBase *addDxfBlock(char *name)
{
	struct DxfBlock *dxfblock;
	
	dxfblock= callocN(sizeof(struct DxfBlock), "addDxfBlock");
	strncpy(dxfblock->name, name, 31);

	addtail(&DxfBlockbase, dxfblock);

	return &(dxfblock->disp);
}

struct ListBase *findDxfBlock(char *name)
{
	struct DxfBlock *dxfblock;
	
	dxfblock= DxfBlockbase.first;
	while(dxfblock) {
		if( strncmp(dxfblock->name, name, 31)==0) break;
		dxfblock= dxfblock->next;
	}
		
	if(dxfblock) return &(dxfblock->disp);
	else return 0;
}

void addDxfLayer(char *name)
{
	struct DxfLayer *dxflay;
	int nr=1;	/* eerste layer is nummer 1, nummer nul zijn de naamloze layers */
	
	if(name[0]=='0' && name[1]==0) return;
	
	dxflay= DxfLaybase.last;
	if(dxflay) nr= dxflay->nr+1;
	
	dxflay= callocN(sizeof(struct DxfLayer), "addDxfLayer");
	dxflay->nr= nr;
	strncpy(dxflay->name, name, 31);
	
	addtail(&DxfLaybase, dxflay);

}

int findDxfLayer(char *name)
{
	struct DxfLayer *dxflay;
	int nr=0;
	
	dxflay= DxfLaybase.first;
	while(dxflay) {
		if(strncmp(dxflay->name, name, 31)==0) return dxflay->nr;
		dxflay= dxflay->next;
	}
	return 0;
}

void strange_DXF_transform(dl, trans)
struct DispList *dl;
float *trans;
{
	float *data;
	float q1[4], q2[4], mat[3][3], vec[3];
	float co,si,n[3],x2,y2,z2,x1,y1,z1,len1,len,b1;
	float fac;
	int totvert;

	/* dominante as */
	if( fabs(trans[0])>fabs(trans[1]) ) fac= 1.0;
	else fac= -1.0;

	x2 = trans[0] ; y2 = trans[1] ; z2 = trans[2];

	/* vector roteren naar negatieve y-as */

	len1=fsqrt(x2*x2+y2*y2+z2*z2);
	if(len1 == 0.0) return;

	len= fsqrt(x2*x2+y2*y2);
	
	if(len>0.0) {
		b1= fatan2(x2, -y2);
		
		co= fcos(b1/2);
		si= fsin(b1/2);

	} else {
		co= 1.0; 
		si= 0.0; 
	}

	q1[0]= co;
	q1[1]= 0.0;
	q1[2]= 0.0;
	q1[3]= si;

	QuatToMat3(q1, mat);
	VECCOPY(vec, trans);
	Mat3MulVecfl(mat, vec);
	
	/* vector roteren naar positieve z-as */

	y2= vec[1];
	z2= vec[2];
	len= fsqrt(y2*y2+z2*z2);
	
	if(len>0.0) {
		b1= fatan2(fac*y2, z2);

		co= fcos(b1/2);
		si= fsin(b1/2);

	} else {
		co= 1.0; 
		si= 0.0; 
	}

	q2[0]= co;
	q2[1]= si;
	q2[2]= 0.0;
	q2[3]= 0.0;

	QuatMul(q1, q1, q2);
	QuatToMat3(q1, mat);

	/* data roteren */

	totvert= dl->nr*dl->parts;
	data= (float *)(dl+1);
	
	while(totvert) {
		
		Mat3MulVecfl(mat, data);

		totvert--;
		data+= 3;
	}
}


void makeDxfInsert(listb, lb, transfound, vec)	/* lb wordt gekopieerd en toegevoegd aan listb */
struct ListBase *listb, *lb;
int transfound;
float *vec;
{
	struct DispList *dl, *dlnew;
	float *data, *datanew;
	float mat[3][3], rmat[3][3], hoek;
	int len;
		
	Mat3One(mat);
	hoek= PI*vec[6]/180.0;
	
	mat[0][0]= fcos(hoek)*vec[3];
	mat[0][1]= fsin(hoek)*vec[3];
	
	mat[1][0]= -fsin(hoek)*vec[4];
	mat[1][1]= fcos(hoek)*vec[4];
	
	mat[2][2]= vec[5];
	
	dl= lb->first;
	while(dl) {
		len= dl->nr*dl->parts;
		dlnew= callocN(sizeof(struct DispList)+4*3*len, "makeDxfInsert");
		*dlnew= *dl;
		addtail(listb, dlnew);
		data= (float *)(dl+1);
		datanew= (float *)(dlnew+1);
		while(len--) {
			VECCOPY(datanew, data);
			Mat3MulVecfl(mat, datanew);
			VecAddf(datanew, datanew, vec);
			data+= 3;
			datanew+= 3;
		}
		
		if(transfound) strange_DXF_transform(dlnew, vec+7);
		
		dl= dl->next;
	}
}


void disp_poly_to_cubes(lb, dl, width, extrude, transfound, trans)
struct ListBase *lb;
struct DispList *dl;
float width, extrude;
int transfound;
float *trans;
{
	struct DispList *dln;
	float *data, *data3, *datan, axis[3], widthvec[3], extrudevec[3], vec[3], cent[3], cube[8][3];
	int a;
	
	/* neem setjes van 2 vertices, width==loodrecht op axis en lijn,, extrude==axis */
	if(dl->type!=DL_POLY) return;
	
	axis[0]= axis[1]= 0.0; axis[2]= 1.0;
	
	a= dl->nr;

	data= (float *)(dl+1);
	data3= data+3;
	while(a>0) {
		VecSubf(vec, data, data3);
		VecMulf(vec, 0.5);
		Crossf(widthvec, axis, vec);
		Normalise(widthvec);
		VecMulf(widthvec, 0.5*width);
		
		VECCOPY(extrudevec, axis);
		VecMulf(extrudevec, extrude);
		
		cent[0]= (data[0]+data3[0])/2.0;
		cent[1]= (data[1]+data3[1])/2.0;
		cent[2]= (data[2]+data3[2])/2.0;
		
		dln= callocN(sizeof(struct DispList)+6*4*3*4, "disppolytocube");
		addtail(lb, dln);
		dln->type= DL_POLY;
		dln->nr= 4;
		dln->parts= 6;
		datan= (float *)(dln+1);
		
		/* cube */
		cube[0][0]= cent[0]-vec[0]-widthvec[0];
		cube[0][1]= cent[1]-vec[1]-widthvec[1];
		cube[0][2]= cent[2]-vec[2]-widthvec[2];
		
		cube[1][0]= cent[0]+vec[0]-widthvec[0];
		cube[1][1]= cent[1]+vec[1]-widthvec[1];
		cube[1][2]= cent[2]+vec[2]-widthvec[2];
		
		cube[2][0]= cent[0]+vec[0]+widthvec[0];
		cube[2][1]= cent[1]+vec[1]+widthvec[1];
		cube[2][2]= cent[2]+vec[2]+widthvec[2];
		
		cube[3][0]= cent[0]-vec[0]+widthvec[0];
		cube[3][1]= cent[1]-vec[1]+widthvec[1];
		cube[3][2]= cent[2]-vec[2]+widthvec[2];
		
		cube[4][0]= cube[0][0]+extrudevec[0];
		cube[4][1]= cube[0][1]+extrudevec[1];
		cube[4][2]= cube[0][2]+extrudevec[2];
		cube[5][0]= cube[1][0]+extrudevec[0];
		cube[5][1]= cube[1][1]+extrudevec[1];
		cube[5][2]= cube[1][2]+extrudevec[2];
		cube[6][0]= cube[2][0]+extrudevec[0];
		cube[6][1]= cube[2][1]+extrudevec[1];
		cube[6][2]= cube[2][2]+extrudevec[2];
		cube[7][0]= cube[3][0]+extrudevec[0];
		cube[7][1]= cube[3][1]+extrudevec[1];
		cube[7][2]= cube[3][2]+extrudevec[2];
		
		/* onder */
		VECCOPY(datan, cube[0]); datan+=3;
		VECCOPY(datan, cube[1]); datan+=3;
		VECCOPY(datan, cube[2]); datan+=3;
		VECCOPY(datan, cube[3]); datan+=3;
		/* boven */
		VECCOPY(datan, cube[4]); datan+=3;
		VECCOPY(datan, cube[5]); datan+=3;
		VECCOPY(datan, cube[6]); datan+=3;
		VECCOPY(datan, cube[7]); datan+=3;
		/* voor */
		VECCOPY(datan, cube[0]); datan+=3;
		VECCOPY(datan, cube[1]); datan+=3;
		VECCOPY(datan, cube[5]); datan+=3;
		VECCOPY(datan, cube[4]); datan+=3;
		/* achter */
		VECCOPY(datan, cube[3]); datan+=3;
		VECCOPY(datan, cube[2]); datan+=3;
		VECCOPY(datan, cube[6]); datan+=3;
		VECCOPY(datan, cube[7]); datan+=3;
		/* links */
		VECCOPY(datan, cube[0]); datan+=3;
		VECCOPY(datan, cube[3]); datan+=3;
		VECCOPY(datan, cube[7]); datan+=3;
		VECCOPY(datan, cube[4]); datan+=3;
		/* rechts */
		VECCOPY(datan, cube[1]); datan+=3;
		VECCOPY(datan, cube[2]); datan+=3;
		VECCOPY(datan, cube[6]); datan+=3;
		VECCOPY(datan, cube[5]); datan+=3;
		
		if(transfound) strange_DXF_transform(dln, trans);

		a--;
		if(a==1) {
			if(dl->nr==2) break;
			data+= 3;
			data3= (float *)(dl+1);
		}
		else {
			data+= 3;
			data3+= 3;
		}
	}
	
	remlink(lb, dl);
	freeN(dl);
}

void disp_poly_to_faces(lb, dl, width, transfound, trans)
struct ListBase *lb;
struct DispList *dl;
float width;
int transfound;
float *trans;
{
	/* printf("Het komt voor \n"); */
}

/* ******************* */

void leesDXF(char *str, struct ListBase *listb)
{
	FILE *fp;
	struct DispList *dl, *dln;
	struct ListBase *lb, *lbinsert;
	struct DxfBlock *dxfblock;
	float vec[12], *data, *hulpdata, *hd, *hd2;
	int code, readcode=0, a, s, tot, afmx, afmy, ok=0, unknownsize, flag, p1, p2, p3, p4;
	int curlayer=0, totvlak, extrude, width, transfound;
	char astr[100];

/* test 
	dl= callocN(sizeof(struct DispList)+3*4, "test");
	dl->type= DL_POLY;
	dl->nr= 1;
	dl->parts= 1;
	data= (float *)(dl+1);

	vec[0]= 1.0; vec[1]= 0.0; vec[2]= 0.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf(" 1 0 0 %f %f %f\n", data[0], data[1], data[2]);
	vec[0]= -1.0; vec[1]= 0.0; vec[2]= 0.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf("-1 0 0 %f %f %f\n", data[0], data[1], data[2]);
	vec[0]= 0.0; vec[1]= 1.0; vec[2]= 0.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf(" 0 1 0 %f %f %f\n", data[0], data[1], data[2]);
	vec[0]= 0.0; vec[1]= -1.0; vec[2]= 0.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf(" 0-1 0 %f %f %f\n", data[0], data[1], data[2]);
	vec[0]= 0.0; vec[1]= 0.0; vec[2]= 1.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf(" 0 0 1 %f %f %f\n", data[0], data[1], data[2]);
	vec[0]= 0.0; vec[1]= 0.0; vec[2]= -1.0;
	data[0]= 1; data[1]= 10; data[2]= 100;
	strange_DXF_transform(dl, vec);
	printf(" 0 0-1 %f %f %f\n", data[0], data[1], data[2]);

	freeN(dl);
*/
	 
	lb= listb;	/* variabele wordt gebruikt om BLOCKS te kunnen maken */
	
	fp= fopen(str, "r");
	while(TRUE) {
		if( fscanf(fp, "%d", &code)==EOF ) break;
		if( fscanf(fp, "%99s", astr)==EOF ) break;
		
		if(code==0) {
			if(strcmp(astr, "SECTION")==0) {
				if( fscanf(fp, "%d", &code)==EOF ) break;
				if( fscanf(fp, "%99s", astr)==EOF ) break;
	
				if(code==2) {
					if(strcmp(astr, "ENTITIES")==0) {
						ok= 1;
						break;
					}
					else if(strcmp(astr, "BLOCKS")==0) {
						ok= 1;
						break;
					}
				}
			}
			else if(strcmp(astr, "LAYER")==0) {	/* kleurinformatie */
				if( fscanf(fp, "%d", &code)==EOF ) break;
				if( fscanf(fp, "%99s", astr)==EOF ) break;
				if(code==2) {
					addDxfLayer(astr);
				}
			}
		}
	}
	while(ok) {

		if(readcode==0) {		/* readcode==1 als een van de delen hieronder te ver doorleest */
			if( fscanf(fp, "%d", &code)==EOF ) break;
			if( fscanf(fp, "%99s", astr)==EOF ) break;
		}
		readcode= 0;

		if(strcmp(astr, "EOF")==0) {
			ok= 0;
		}
		if(strcmp(astr, "BLOCK")==0) {
			fscanf(fp, "%d", &code);
			fscanf(fp, "%99s", astr);
			fscanf(fp, "%d", &code);
			fscanf(fp, "%99s", astr);
			
			fscanf(fp, "%d", &code);
			fscanf(fp, "%99s", astr);
			if(code==2) {
				lb= addDxfBlock(astr);
			}
		}
		else if(strcmp(astr, "ENDBLK")==0) {
			/* hoofd listbase weer terug */
			lb= listb;
		}
		else if(strcmp(astr, "INSERT")==0) {
			code= 1;
			transfound= 0;
			vec[0]= vec[1]= vec[2]= 0.0;
			vec[3]= vec[4]= vec[5]= 1.0;
			vec[6]= 0.0;
			lbinsert= 0;
			while(code!=0) {
				if( fscanf(fp, "%d", &code)==EOF ) break;
				switch(code) {
					case 2: fscanf(fp, "%99s", astr); lbinsert= findDxfBlock(astr); break;
					case 10: fscanf(fp, "%f", vec); break;		/* translatie */
					case 20: fscanf(fp, "%f", vec+1); break;
					case 30: fscanf(fp, "%f", vec+2); break;
					case 41: fscanf(fp, "%f", vec+3); break;	/* scaling */
					case 42: fscanf(fp, "%f", vec+4); break;
					case 43: fscanf(fp, "%f", vec+5); break;
					case 50: fscanf(fp, "%f", vec+6); break;	/* xy vlak rotatie */
					case 210: fscanf(fp, "%f", vec+7); transfound= 1; break;
					case 220: fscanf(fp, "%f", vec+8); break;
					case 230: fscanf(fp, "%f", vec+9); break;
					default: fscanf(fp, "%99s", astr); break;
				}
			}
			readcode= 1;	/* te ver doorgelezen */
			if(lbinsert) {
				makeDxfInsert(lb, lbinsert, transfound, vec);
			}
		}
		else if(strcmp(astr, "LINE")==0) {	/* doet alleen 3D lijnen */
			code= 1;
			while(code!=0) {
				if( fscanf(fp, "%d", &code)==EOF ) break;
				switch(code) {
					case 8: fscanf(fp, "%99s", astr); curlayer= findDxfLayer(astr); break;
					case 10: fscanf(fp, "%f", vec); break;
					case 20: fscanf(fp, "%f", vec+1); break;
					case 30: fscanf(fp, "%f", vec+2); break;
					case 11: fscanf(fp, "%f", vec+3); break;
					case 21: fscanf(fp, "%f", vec+4); break;
					case 31: fscanf(fp, "%f", vec+5); break;
					default: fscanf(fp, "%99s", astr); break;
				}
				if(code==31) break;
			}
			if(code==31) {
				dl= callocN(sizeof(struct DispList)+4*3*2, "leesDXF2");
				addtail(lb, dl);
				dl->nr= 2;
				dl->parts= 1;
				dl->type= DL_SEGM;
				dl->col= curlayer;
				data= (float *)(dl+1);
				VECCOPY(data, vec);
				VECCOPY(data+3, vec+3);
			}
		}
		else if(strcmp(astr, "3DFACE")==0) {	/* 4 vertices */
			code= 1;
			while(code!=0) {
				if( fscanf(fp, "%d", &code)==EOF ) break;
				switch(code) {
					case 8: fscanf(fp, "%99s", astr); curlayer= findDxfLayer(astr); break;
					case 10: fscanf(fp, "%f", vec); break;
					case 20: fscanf(fp, "%f", vec+1); break;
					case 30: fscanf(fp, "%f", vec+2); break;
					case 11: fscanf(fp, "%f", vec+3); break;
					case 21: fscanf(fp, "%f", vec+4); break;
					case 31: fscanf(fp, "%f", vec+5); break;
					case 12: fscanf(fp, "%f", vec+6); break;
					case 22: fscanf(fp, "%f", vec+7); break;
					case 32: fscanf(fp, "%f", vec+8); break;
					case 13: fscanf(fp, "%f", vec+9); break;
					case 23: fscanf(fp, "%f", vec+10); break;
					case 33: fscanf(fp, "%f", vec+11); break;
					default: fscanf(fp, "%99s", astr); break;
				}
				if(code==33) break;
			}
			if(code==33) {
				dl= callocN(sizeof(struct DispList)+4*3*4, "leesDXF2");
				addtail(lb, dl);
				dl->nr= 4;
				dl->parts= 1;
				dl->type= DL_POLY;
				dl->col= curlayer;
				data= (float *)(dl+1);
				VECCOPY(data, vec);
				VECCOPY(data+3, vec+3);
				VECCOPY(data+6, vec+6);
				VECCOPY(data+9, vec+9);
			}
			
		}
		else if(strcmp(astr, "POLYLINE")==0) {
			afmx= afmy= unknownsize= flag= totvlak= extrude= width= transfound= 0;
			vec[3]= 0.0;	/* width */
			vec[4]= 0.0;	/* extrude */
			vec[6]= vec[7]= 0.0; vec[8]= 1.0;	/* extrude richting */
			code= 1;
			while(code!=0) {	/* wachten op VERTEX */
				if( fscanf(fp, "%d", &code)==EOF ) break;
				
				if(code==8) {fscanf(fp, "%99s", astr); curlayer= findDxfLayer(astr); }
				else if(code==39) { fscanf(fp, "%f", vec+3); extrude= 1; }
				else if(code==40) { fscanf(fp, "%f", vec+4); width= 1; }
				else if(code==70) fscanf(fp, "%d", &flag);
				else if(code==71) fscanf(fp, "%d", &afmy);
				else if(code==72) fscanf(fp, "%d", &afmx);
				else if(code==210) { fscanf(fp, "%f", vec+6); transfound=1;}
				else if(code==220) fscanf(fp, "%f", vec+7);
				else if(code==230) fscanf(fp, "%f", vec+8);
				else
					if( fscanf(fp, "%99s", astr)==EOF ) break;
			}
			
			if(flag & 64) {		/* nieuw type: beetje videoscape-achtig */
								/* afmy= aantal vertices */
								/* afmx= aantal vlakken (vierhoeken) */
				if(afmx>0 && afmy>0) {
					hulpdata= callocN(4*3*(afmy+afmx), "leesDXF6");
					
					tot= 4*afmx;
					dl= callocN(sizeof(struct DispList)+4*3*tot, "leesDXF7");
					addtail(lb, dl);
					dl->nr= 4;
					dl->parts= afmx;
					dl->col= curlayer;
					dl->type= DL_POLY;
					
					hd= hulpdata;
					data= (float *)(dl+1);
					
					tot= afmx+afmy;
					/* hoogstwaarschijnlijk zit in astr nu VERTEX */
					if(code==0 && strcmp(astr, "VERTEX")==0 ) {
						tot--;
						hd+= 3;
					}
					/* alle vertexen inlezen tot SEQUEND */
					while(tot>=0) {
					
						if( fscanf(fp, "%d", &code)==EOF ) break;
						
						switch(code) {
							case 10: fscanf(fp, "%f", hd-3); break;
							case 20: fscanf(fp, "%f", hd-2); break;
							case 30: fscanf(fp, "%f", hd-1); break;
							case 71: fscanf(fp, "%d", &p1); break;
							case 72: fscanf(fp, "%d", &p2); break;
							case 73: fscanf(fp, "%d", &p3); break;
							case 74: fscanf(fp, "%d", &p4); break;
							default:  fscanf(fp, "%99s", astr); break;
						}
						
						if(code==0 && strcmp(astr, "SEQEND")==0 ) break;
						if(code==0 && strcmp(astr, "VERTEX")==0 ) {
							tot--;
							hd+= 3;
						}
						if(code==74) {	/* nieuw vlak */
							p1= 3*(p1-1);
							p2= 3*(p2-1);
							p3= 3*(p3-1);
							p4= 3*(p4-1);
							data[0]= hulpdata[p1]; data[1]= hulpdata[p1+1]; data[2]= hulpdata[p1+2];
							data+= 3;
							data[0]= hulpdata[p2]; data[1]= hulpdata[p2+1]; data[2]= hulpdata[p2+2];
							data+= 3;
							data[0]= hulpdata[p3]; data[1]= hulpdata[p3+1]; data[2]= hulpdata[p3+2];
							data+= 3;
							data[0]= hulpdata[p4]; data[1]= hulpdata[p4+1]; data[2]= hulpdata[p4+2];
							data+= 3;
							totvlak++;
							if(totvlak>afmx) break;
						}
					}
					freeN(hulpdata);
					
					if(transfound) strange_DXF_transform(dl, vec+6);
				}
			}
			else {

				if(afmx==0 && afmy==0) {	/* zelf tellen */
					unknownsize= 1;
					afmx= 256;
					afmy= 1;
				}
	
				tot= afmx*afmy;
				dl= callocN(sizeof(struct DispList)+4*3*tot, "leesDXF1");
				addtail(lb, dl);
				dl->nr= afmx;
				dl->parts= afmy;
				dl->col= curlayer;
				dl->type= DL_SURF;
				if(flag & 1) dl->flag= 2;
				if(flag & 32) dl->flag+= 1;
				if(afmy<2) dl->type= DL_POLY;
	
				data= (float *)(dl+1);
				
				/* hoogstwaarschijnlijk zit in astr nu VERTEX */
				if(code==0 && strcmp(astr, "VERTEX")==0 ) {
					tot--;
					data+= 3;
				}
				/* alle vertexen inlezen tot SEQUEND */
				while(tot>=0) {
				
					if( fscanf(fp, "%d", &code)==EOF ) break;
					
					if(code==10) {
						fscanf(fp, "%f", data-3);
					}
					else if(code==20) {
						fscanf(fp, "%f", data-2);
					}
					else if(code==30) {
						fscanf(fp, "%f", data-1);
					}
					else if( fscanf(fp, "%99s", astr)==EOF ) break;
					
					if(code==0 && strcmp(astr, "SEQEND")==0 ) break;
					if(code==0 && strcmp(astr, "VERTEX")==0 ) {
						
						tot--;
						data+= 3;
					}
				}
				
				if(unknownsize) {
					dl->nr= 256-tot;
					if(dl->nr==0) {
						remlink(lb, dl);
					}
					if(dl->nr<250) {	/* realloc */
						tot= sizeof(struct DispList)+4*3*dl->nr;
						dln= mallocN(tot, "leesDXF12");
						memcpy(dln, dl, tot);
						remlink(lb, dl);
						freeN(dl);
						addtail(lb, dln);
						dl= dln;
					}
				}

				if(width && dl->type==DL_POLY) {
					if(extrude) {	/* kubussen */
						disp_poly_to_cubes(lb, dl, vec[4], vec[3], transfound, vec+6);
					}
					else {			/* vlakken */
						disp_poly_to_faces(lb, dl, vec[4], transfound, vec+6);
					}
				}
				else if(extrude && dl->type==DL_POLY) {
					tot= 2*dl->nr;
					dln= callocN(sizeof(struct DispList)+4*3*tot, "leesDXF1");
					addtail(lb, dln);
					dln->nr= dl->nr;
					dln->parts= 2;
					dln->col= curlayer;
					dln->type= DL_SURF;
					dln->flag= dl->flag;
					data= (float *)(dl+1);
					hd= (float *)(dln+1);
					hd2= hd+ 3*dl->nr;
					while(tot) {
						hd[0]= data[0]; hd[1]= data[1]; hd[2]= data[2];
						hd2[0]= data[0];
						hd2[1]= data[1];
						hd2[2]= data[2]+vec[3];
						
						tot-= 2; data+= 3; hd+=3; hd2+= 3;
					}
					remlink(lb, dl);
					freeN(dl);
					
					if(transfound) strange_DXF_transform(dln, vec+6);
				}
				else if(transfound) strange_DXF_transform(dl, vec+6);
			}
		}
	}
	fclose(fp);
	
	freelistN(&DxfLaybase);
	
	dxfblock= DxfBlockbase.first;
	while(dxfblock) {
		freelistN(&(dxfblock->disp));
		dxfblock= dxfblock->next;
	}
	freelistN(&DxfBlockbase);
	
}

/* ***************** INVENTOR ******************* */

#define IV_MAXSTACK 100000
#define IV_MAXFIELD 10
#define IV_MAXCOL 16

float *iv_data_stack;

struct ListBase ivbase;

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
	static float colors[IV_MAXCOL][3];
	float *fp;
	int a;
	
	/* terugzoeken naar laatste materiaal */
	while(iv && strcmp(iv->nodename, "Material ")!=0) iv= iv->prev;
	if(iv==0) return 0;
	if(iv->datalen[0]<3) return 0;
	
	fp= iv->data[0];
	for(a=0; a<iv_curcol; a++) {
	
		if(colors[a][0]== fp[0])
			if(colors[a][1]== fp[1])
				if(colors[a][2]== fp[2]) return a;
		
	}
	if(a>=IV_MAXCOL) a= IV_MAXCOL-1;
	
	iv_curcol= a+1;
	
	fp= iv->data[0];
	colors[a][0]= fp[0];
	colors[a][1]= fp[1];
	colors[a][2]= fp[2];
	return a;
	
}

void iv_finddata(struct IvNode *iv, char *field, int fieldnr)
{
	/* zoek naar "field", tel lengte data en maak datablok */
	float *fp;
	int len, stackcount;
	char *cpa, terminator;
	
	len= strlen(field);
	
	cpa= iv->nodename+1;
	while( *cpa != '}' ) {
		if( *cpa == *field ) {
			if( strncmp(cpa, field, len)==0 ) {
				iv->fieldname[fieldnr]= cpa;
				
				/* lezen tot aan eerste karakter */
				cpa+= len;
				*cpa= 0;
				cpa++;
				
				while( *cpa==' ') cpa++;
				if( *cpa=='[' ) {
					terminator= ']';
					cpa++;
				}
				else terminator= 13;
				
				stackcount= 0;
				fp= iv_data_stack;
				while( *cpa!=terminator && *cpa != '}' ) {
					
					if( isspace(*cpa)==0 && (isspace(cpa[-1]) || cpa[-1]==0) ) {
						/* sscanf(cpa, "%f", fp); */
						*fp= atof(cpa);
						
						stackcount++;
						if(stackcount>=IV_MAXSTACK) {
							printf("stackoverflow in IV read\n");
							break;
						}
						fp++;
						
					}
					cpa++;
				}
				
				iv->datalen[fieldnr]= stackcount;
				iv->data[fieldnr]= mallocN(4*stackcount, "iv_finddata");
				memcpy(iv->data[fieldnr], iv_data_stack, 4*stackcount);
				
				return;
			}
		}
		cpa++;
	}
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


ListBase *nurbsbase=0;
struct Base *nubase;

void new_nubase()
{
	static int count= 0;
	struct ObData *ob;
	struct CurveData *cu;
	
	count++;

	if(nurbsbase) {
		if(count<10) return;

		count= 0;
		
		makeDispList(nubase);
	}
	
	nubase= addbase(0);
	nubase->f= 9;
	nubase->f1= 2+8+16+64;	/* was 42: 2+8+32 */
	nubase->lay= 1<<(G.layact-1);
	nubase->sf=1;
	nubase->len= 80;
	nubase->q[0]=1.0;
	Mat3One(nubase->m);

	strcpy(nubase->str, "nurbs");
	newbasename(nubase);

	nubase->soort= 5;
	G.totobj++;

	nubase->d= callocN(sizeof(struct ObData) + sizeof(struct ColBlck),"autocad");
	ob= (struct ObData *)nubase->d;
	ob->dt=2;
	ob->c= 1;
	initcolblocks( (ob+1), 1);
	
	ob->cu= callocstructN(struct CurveData, 1, "addbase10");
	cu= ob->cu;
	cu->ws= 100.0;
	cu->us= 1;
	cu->width= 100;
	cu->flag= 255;
	
	nurbsbase= &cu->curve;
}

void leesInventor(char *str, struct ListBase *listb)
{
	struct IvNode *iv, *ivp;
	char *maindata, *md, *cpa;
	float *index, *data, *fp, tempindex[3];
	long file, filelen, count;
	int asnurbs=0, ok, a, b, tot, first, colnr, coordtype;
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
			iv= callocN(sizeof(struct IvNode), "leesInventor");
			iv->nodename= cpa;

			if(strcmp(cpa, "Coordinate3 ")==0 || strcmp(cpa, "Coordinate4 ")==0) {
				iv_finddata(iv, "point", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedLineSet ")==0) {
				iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedTriangleMesh ")==0) {
				iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedFaceSet ")==0) {
				iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "FaceSet ")==0) {
				iv_finddata(iv, "numVertices", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "Material ")==0) {
				iv_finddata(iv, "diffuseColor", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "QuadMesh ")==0) {
				iv_finddata(iv, "verticesPerColumn", 0);
				iv_finddata(iv, "verticesPerRow", 1);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedTriangleStripSet ")==0) {
				iv_finddata(iv, "coordIndex", 0);
				ok= 1;
			}
			else if(strcmp(cpa, "IndexedNurbsSurface ")==0) {
				iv_finddata(iv, "numUControlPoints", 0);
				iv_finddata(iv, "numVControlPoints", 1);
				iv_finddata(iv, "uKnotVector", 2);
				iv_finddata(iv, "vKnotVector", 3);
				ok= 1;
			}
			
			
			if(ok) {
				addtail(&ivbase, iv);
			}
			else freeN(iv);
			
		}
		md++;
		count++;
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
				
				for(a=0; a<iv->datalen[0]; a++) {
					if(index[0]== 3) tot++;
					index++;
				}
				tot*= 3;	/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
				addtail(listb, dl);
				dl->type= DL_POLY;
				dl->nr= 3;
				dl->parts= tot/3;
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
				tot= 0;
				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) tot++;
					index++;
				}
				tot*= 3;	/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
				addtail(listb, dl);
				dl->type= DL_POLY;
				dl->nr= 3;
				dl->parts= tot/3;
				dl->col= colnr;
				data= (float *)(dl+1);

				index= iv->data[0];
				first= 1;
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) {
						/* deze truuk is om poly's met meer dan 3 vertices correct te vullen */
						if(first) {
							VECCOPY(tempindex, index);
							first= 0;
						}
						else {
							tempindex[1]= index[1];
							tempindex[2]= index[2];
						}
						read_iv_index(data, ivp->data[0], tempindex, 3, coordtype);
						data+= 9;
					}
					else first= 1;
					
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "IndexedTriangleMesh ")==0 ) {
			
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
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) tot++;
					index++;
				}
				tot*= 3;	/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
				addtail(listb, dl);
				dl->type= DL_POLY;
				dl->nr= 3;
				dl->parts= tot/3;
				dl->col= colnr;
				data= (float *)(dl+1);

				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) {
						read_iv_index(data, ivp->data[0], index, 3, coordtype);
						data+= 9;
					}
					index++;
				}
			}
		}
		else if( strcmp(iv->nodename, "IndexedTriangleStripSet ")==0 ) {
			
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
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) tot++;
					index++;
				}
				tot*= 3;	/* aantal vertices */
				dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
				addtail(listb, dl);
				dl->type= DL_POLY;
				dl->nr= 3;
				dl->parts= tot/3;
				dl->col= colnr;
				data= (float *)(dl+1);

				index= iv->data[0];
				for(a=0; a<iv->datalen[0]-2; a++) {
					if(index[0]!= -1 && index[1]!= -1 && index[2]!= -1) {
						read_iv_index(data, ivp->data[0], index, 3, coordtype);
						data+= 9;
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
				if( strcmp(ivp->nodename, "Coordinate4 ")==0 ) {
					coordtype= 4;
					break;
				}
			}
			if(ivp) {
				tot= *(iv->data[0]) * *(iv->data[1]);
				
				if(tot>0) {
					dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
					addtail(listb, dl);
					dl->type= DL_SURF;
					dl->nr= *(iv->data[0]);
					dl->parts= *(iv->data[1]);
					dl->col= colnr;
					data= (float *)(dl+1);
					memcpy(data, ivp->data[0], tot*3*4);
				}
			}
		}
		else if( strcmp(iv->nodename, "IndexedNurbsSurface ")==0 ) {
			
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
				
				if(asnurbs==0) {
					if(okee("Read as Nurbs")) asnurbs= 1;
					else asnurbs= 2;
				}
				
				if( (a>4 || b>4) && tot>6 && asnurbs==1) {
					struct Nurb *nu;
					struct BPoint *bp;
					
					new_nubase();
					
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
					
					nu->knotsu= mallocN( 4*(iv->datalen[2]));
					memcpy(nu->knotsu, iv->data[2], 4*(iv->datalen[2]));
					nu->knotsv= mallocN( 4*(iv->datalen[3]));
					memcpy(nu->knotsv, iv->data[3], 4*(iv->datalen[3]));					
					
				}
				else {
					dl= callocN(sizeof(struct DispList)+tot*3*4, "leesInventor2");
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
		makeDispList(nubase);
	}
}

char byteswap(char byte)
{
	char i;
	
	i= 0;
	if(byte & 1) i |= 128;
	if(byte & 2) i |= 64;
	if(byte & 4) i |= 32;
	if(byte & 8) i |= 16;
	if(byte & 16) i |= 8;
	if(byte & 32) i |= 4;
	if(byte & 64) i |= 2;
	if(byte & 128) i |= 1;
	
	return i;
}

/* ********************************************** */

int finddoublevert(struct VertOb *adrve, int totvert)
{
	struct VertOb *testve;
	int a, tot;

	tot= totvert-128;
	if(tot<0) tot= 0;
	
	testve= adrve-1;
	for(a=totvert-1; a>tot; a--, testve--) {
		if( adrve->c[0]==testve->c[0]) {
			if( adrve->c[1]==testve->c[1]) {
				if( adrve->c[2]==testve->c[2]) {
					return a;
				}
			}
		}
	}
	return -1;
}

void leesCurry(char *name)
{
	struct Base *base;
	struct ObData *ob;
	struct VV *vv;
	struct ColBlck *col;
	struct VlakOb *adrvl, *adrvlo;
	struct VertOb *adrve, *adrveo, *adrve1, *adrve2, *adrve3;
	float n[3];
	int v1, v2, v3;
	int len, a, file, filelen, totvlak, totvert, vcount=0;
	char *filedata, *fd, *end;
	
	if(G.ebase) {
		leesCurryEdit();
		return;
	}
	
	file= open(name, O_RDONLY);
	if(file<=0) {
		error("Can't open file");
		return;
	}
	
	filelen= lseek(file, 0, 2);	/* seek end */
	lseek(file, 0, 0);		/* en weer terug */

	if(filelen<20) {
		close(file);
		return;
	}
	filedata= mallocN(filelen, "curry");
	read(file, filedata, filelen);
	close(file);
	
	end= filedata+filelen;
	fd= filedata+20;
	
	totvert= (filelen-20)/6;
	totvlak= totvert/3;
	
	if(totvert>65535) totvert= 65535;
	if(totvlak>65535) totvlak= 65535;
	
	base= addbase(0);

	base->f= 9;
	base->f1= 2+8+16+64;	/* was 42: 2+8+32 */
	base->lay= 1<<(G.layact-1);
	base->sf=1;
	base->len= 80;
	base->q[0]=1.0;
	Mat3One(base->m);

	strcpy(base->str, "curry");
	newbasename(base);

	base->soort= 1;
	G.totobj++;

	base->d= callocN(sizeof(struct ObData)+sizeof(struct ColBlck),"curry");
	ob= (struct ObData *)base->d;
	vv= callocN(sizeof(struct VV)+ totvert*sizeof(struct VertOb)+ 
		totvlak*sizeof(struct VlakOb));
	
	col= (struct ColBlck *)(ob+1);
	col->r= col->g= col->b= 240;
	col->mode= 12;
	initcolblocks(col, 1);
	
	ob->vv= vv;
	ob->c= 1;
	ob->dt= 1;
	vv->vert= totvert;
	vv->vlak= totvlak;
	vv->us= 1;
	vv->afm[0]= 32767;
	vv->afm[1]= 32767;
	vv->afm[2]= 32767;

	vv->ws= 1.0;
	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+totvert);
	
	totvert=0;
	totvlak=0;
	
	while(fd<end-17) {

		adrve->c[0]= 2*((fd[0]<<7)-16384);
		adrve->c[1]= 2*((fd[1]<<7)-16384);
		adrve->c[2]= 2*((fd[2]<<7)-16384);
		adrve->n[0]= -2*((fd[3]<<7)-16384);
		adrve->n[1]= -2*((fd[4]<<7)-16384);
		adrve->n[2]= -2*((fd[5]<<7)-16384);
		
		v1= finddoublevert(adrve, totvert);
		if(v1== -1) {
			v1= totvert;
			adrve++;
			totvert++;
		}
		fd+= 6;
		
		adrve->c[0]= 2*((fd[0]<<7)-16384);
		adrve->c[1]= 2*((fd[1]<<7)-16384);
		adrve->c[2]= 2*((fd[2]<<7)-16384);
		adrve->n[0]= -2*((fd[3]<<7)-16384);
		adrve->n[1]= -2*((fd[4]<<7)-16384);
		adrve->n[2]= -2*((fd[5]<<7)-16384);

		v2= finddoublevert(adrve, totvert);
		if(v2== -1) {
			v2= totvert;
			adrve++;
			totvert++;
		}
		fd+= 6;
		
		adrve->c[0]= 2*((fd[0]<<7)-16384);
		adrve->c[1]= 2*((fd[1]<<7)-16384);
		adrve->c[2]= 2*((fd[2]<<7)-16384);
		adrve->n[0]= -2*((fd[3]<<7)-16384);
		adrve->n[1]= -2*((fd[4]<<7)-16384);
		adrve->n[2]= -2*((fd[5]<<7)-16384);

		v3= finddoublevert(adrve, totvert);
		if(v3== -1) {
			v3= totvert;
			adrve++;
			totvert++;
		}
		fd+= 6;
		
		adrvl->v1= v1;
		adrvl->v2= v2;
		adrvl->v3= v3;
		
		totvlak++;
		adrvl++;		

		if(totvert>65535 || totvlak>65535) {
			error("warning: file too big");
			break;
		}
	}
	
	freeN(filedata);

	vv= callocN(sizeof(struct VV)+ totvert*sizeof(struct VertOb)+ 
	totvlak*sizeof(struct VlakOb));

	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+totvert);

	adrveo= (struct VertOb *)(ob->vv+1);
	adrvlo= (struct VlakOb *)(adrveo+ob->vv->vert);
	
	memcpy(adrve, adrveo, totvert*sizeof(struct VertOb));
	memcpy(adrvl, adrvlo, totvlak*sizeof(struct VlakOb));
	
	*vv= *(ob->vv);
	vv->vert= totvert;
	vv->vlak= totvlak;
	
	freeN(ob->vv);
	ob->vv= vv;
	
	/* puno omklapvlaggen */
	
	adrvl= (struct VlakOb *)(adrve+totvert);
	for(a=0; a<totvlak; a++) {
		adrve1= adrve+adrvl->v1;
		adrve2= adrve+adrvl->v2;
		adrve3= adrve+adrvl->v3;

		CalcNormShort(adrve1->c, adrve2->c, adrve3->c, n);
		adrvl->n[0]= ffloor(n[0]*32767.0);
		adrvl->n[1]= ffloor(n[1]*32767.0);
		adrvl->n[2]= ffloor(n[2]*32767.0);

		adrvl->f= 0;
		len= adrvl->n[0]*adrve1->n[0]+adrvl->n[1]*adrve1->n[1]+adrvl->n[2]*adrve1->n[2];
		if(len<0) adrvl->f= 1;
		len= adrvl->n[0]*adrve2->n[0]+adrvl->n[1]*adrve2->n[1]+adrvl->n[2]*adrve2->n[2];
		if(len<0) adrvl->f+= 2;
		len= adrvl->n[0]*adrve3->n[0]+adrvl->n[1]*adrve3->n[1]+adrvl->n[2]*adrve3->n[2];
		if(len<0) adrvl->f+= 4;
		
		adrvl++;
	}
	
}


/* ************************** BLENDER ************************ */

void saveblender()
{
	
}

/* *********************************************************** */


void schr_inventor()
{
	struct Base *base;
	struct ObData *ob;
	struct VV *vv;
	struct VlakOb *adrvl;
	struct VertOb *adrve;
	FILE *fp;
	float fac, len, mat4[4][4], vec[3], omat[3][3];
	float *n1, *n2, inp, VecLenf();
	int a, tot, file, ok, lasttime, *edgedata, *edda;
	short *v, val;
	char *cp, str[128];

	strcpy(str, G.sce);
	val= ffileselect("WRITE INVENTOR", str);
	if(val==0) return;

	if(saveover(str)==0) return;
 
 	fp= fopen(str, "w");
	if(fp==NULL) return;

	fprintf(fp,"#Inventor V2.0 ascii\n");

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort==1) {

				fprintf(fp,"Separator {\n");
				
				ob= (struct ObData *)base->d;
				vv= ob->vv;
				adrve= (struct VertOb *)(vv+1);
				adrvl= (struct VlakOb *)(adrve+vv->vert);
				
				parentbase(base, mat4, omat);
				fac= vv->ws;
				Mat4MulFloat3(mat4, fac);
				fac= 1.0/5000.0;
				Mat4MulFloat(mat4, fac);
				
				if(vv->vert) {
					fprintf(fp, "Coordinate3 {\n");
					fprintf(fp, "point [ \n");
					
					for(a=0; a<vv->vert; a++, adrve++) {
						VECCOPY(vec, adrve->c);
						Mat4MulVecfl(mat4, vec);
						
						fprintf(fp, "%f %f %f, \n", vec[0], vec[1], vec[2]);
					}
					fprintf(fp, "]\n}\n");
				}
				
				if(vv->vlak) {
					fprintf(fp, "IndexedTriangleStripSet {\n");
					fprintf(fp, "coordIndex [ \n");
					
					/* driehoeken schrijven */
					for(a=0; a<vv->vlak; a++, adrvl++) {
						fprintf(fp, "%d, %d, %d, -1,\n", adrvl->v1, adrvl->v2, adrvl->v3);
					}
					fprintf(fp, "]\n}\n");
				}
				
				fprintf(fp, "}\n");
			}
		}
		
		base= base->next;
	}

	fclose(fp);

}

