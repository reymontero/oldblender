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

/* 	font.c	 april 92 */


#include <sys/types.h>
#include <local/util.h>
#include <local/iff.h>
#include <local/objfnt.h>
#include <stdio.h> 
#include <fcntl.h>
#include <string.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include "/usr/people/include/Trace.h"

#define CLIP MAKE_ID('C','L','I','P')
#define CLNM MAKE_ID('C','L','N','M')
#define VBOX MAKE_ID('V','B','O','X')
#define OBJ  MAKE_ID('O','B','J',' ')
#define OFLG MAKE_ID('O','F','L','G')
#define FILL MAKE_ID('F','I','L','L')
#define DATA MAKE_ID('D','A','T','A')
#define OEND MAKE_ID('O','E','N','D')
#define OFIN MAKE_ID('O','F','I','N')
#define PDFF MAKE_ID('P','D','F','F')
#define CLFN MAKE_ID('C','L','F','N')
#define KRNP MAKE_ID('K','R','N','P')
#define BEZ4 MAKE_ID('B','E','Z','4')

#define MAX_VF_CHARS 256

/* bezres hangt aan button en is word, bezresF is float en moet ingesteld worden */
/* bezmax hangt aan button, bezmaxF wordt uitgevoerd */

float fscale = 0.5,bezresF,wordr_ofsx,wordr_ofsy,wordr_fac;
float AreaF2Dfl();
short textediting= 0,bezres=12,bezmax=6,bezmaxF;

/* Nieuwe opzet voor vectorfont:
 * 
 * Standaard worden de beziers onthouden, elk font wordt eerst naar dit formaat.
 * geconverteerd. Beziers 'draaien' al goed voor bijvoorbeeld beveling.
 * Dit is het formaat zoals in de curve gebruikt wordt.
 * 
 * Daarnaast is er een cache waarin geinterpoleerde versies bewaard worden.
 *	Nodig: resolutie, pointer naar punten.
 *	Dit is het nog oude formaat van de poly's.
 *
 * De psfont moet helemaal netjes gemaakt worden zodat er alleen nog maar
 * curves uit kunnen komen.
 *
 */

/*
 * Pdraw en postscript fonts worden een op een gecopieerd, pas op het moment
 * dat er kopieen gemaakt worden wordt een matrix toegepast (scale,  shear, rot?)
 */
struct bezpart {
	struct bezpart *next,*prev;
	long count;
	float points[1][3];
};

struct bezcurve {
	struct bezcurve *next,*prev;
	struct bezpart *first,*last;
	short count,hole;
};

struct chartrans {
	long xof,yof;
	float rot;
	short linenr,charnr;
};

struct VFont {
	struct VFont *next,*prev;
	char name[255], namenull;
	float scale;
	struct ListBase nurbsbase[MAX_VF_CHARS];
	float	    resol[MAX_VF_CHARS];
	float	    width[MAX_VF_CHARS];
	float	    *points[MAX_VF_CHARS];
	short kcount,us;
	short *kern;
};


void freevfontN(struct VFont *vf)
{
	long i;
	struct Nurb *nu;
	struct BezTriple *bezt;

	if (vf == 0) return;

	/* vf->us--; */
	/* if (vf->us < 0){ */
		/* patch */
		/* remlink(&G.vfbase,vf); */

		for (i = 0; i < MAX_VF_CHARS; i++){
			while (nu = vf->nurbsbase[i].first){
				bezt = nu->bezt;
				if (bezt) freeN(bezt);
				remlink(&vf->nurbsbase[i], nu);
				freeN(nu);
			}
		}

		if (vf->kern) freeN(vf->kern);
		freeN(vf);

	/* } */
}

void freevfont(vf)
struct VectorFont *vf;
{
	if(vf==0) return;

	vf->us--;
	if(vf->us<=0) {
		remlink(&G.vfbase,vf);
		freeN(vf->name);
		if (vf->type == FNT_PDRAW) freeN(vf->base);

		freevfontN(vf->objfnt);

		freeN(vf);
	}
}

struct VectorFont *initvfont(name)
char *name;
{
	/* vfont->us (users) zelf instellen, wordt hier op nul gezet */
	long file,len,totlen, i;
	uchar *mem, *p, ascii;
	struct VectorFont *vfont;
	struct VFont * loadvfont(), *Vfont;

	file = open(name,O_RDONLY);
	if (file<=0) return (0);

	len = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);

	mem = (uchar *) mallocN(len,"initvfontmem");
	if (mem == 0) {
		close(file) ;
		return(0);
	}

	if (read(file,mem,len) != len) {
		close(file) ;
		freeN(mem) ;
		return (0);
	}
	close(file);

	if (GL(mem) != PDFF){
		/* look for %!PS-AdobeFont-1 */

		for (i = 0; i < len; i++){
			if (mem[i] == '%'){
				if (memcmp(mem + i, "%!PS-AdobeFont-1", 16) == 0) break;
			}
		}
		freeN(mem);

		if (i > len) {
			printf("No Vectorfont or Postscript-1 file %s\n",name);
			return(0);
		}
		/*printf("Found PSfont\n");*/
		Vfont = loadvfont(name);
		if (Vfont == 0) return(0);

		vfont = callocstructN(struct VectorFont,1,"initvfontfont");
		if (vfont == 0) {
			freeN(mem) ;
			return(0);
		}
		vfont->name = mallocN(strlen(name)+1,"initvfontname");
		strcpy(vfont->name,name);
		vfont->type = FNT_HAEBERLI;
		vfont->objfnt = (struct objfnt *) Vfont;
		addtail(&G.vfbase,vfont);
		return(vfont);
	}

	vfont = callocstructN(struct VectorFont,1,"initvfontfont");
	if (vfont == 0) {
		freeN(mem) ;
		return(0);
	}

	vfont->name = mallocN(strlen(name)+1,"initvfontname");
	strcpy(vfont->name,name);
	vfont->base = mem;
	vfont->type = FNT_PDRAW;

	totlen = len - 8;
	mem += 8;
	while(totlen > 0){
		len = GL(mem + 4);
		switch(GL(mem)){
		case CLNM:
			ascii = mem[8];
			break;
		case VBOX:
			if (ascii) vfont->index[ascii] = mem;
			RMK(printf("found %d %x\n",ascii,mem);)
			    break;
		case OFIN:
			ascii = 0;
			break;
		case KRNP:
			vfont->kern = (short *) (mem + 10);
			vfont->kcount = vfont->kern[-1];
		}
		len= ((len + 1) & ~1) + 8;
		mem += len;
		totlen -= len;
	}

	addtail(&G.vfbase,vfont);
	/* patch */
	vfont->objfnt = (struct objfnt *) loadvfont(name);
	return(vfont);
}

struct VFont *loadpdrawfont(uchar * mem, long totlen)
{
	struct BezTriple *bezt;
	struct Nurb *nu;
	struct VFont *vfont = 0;
	struct ListBase *curve;
	long len, ascii = 0, ok = FALSE, hole, i;
	short *point;

	vfont = callocstructN(struct VFont,1,"initvfontfont");

	if (vfont) {
		vfont->scale = 0.5;
		totlen -= 8;
		mem += 8;
		while(totlen > 0){
			len = GL(mem + 4);
			switch(GL(mem)){
			case CLNM:
				ascii = mem[8];
				if (ascii >= 0 && ascii < MAX_VF_CHARS){
					curve = &vfont->nurbsbase[ascii];
				} else{
					curve = 0;
					ascii = 0;
				}
				break;
			case FILL:
				hole = mem[9];
				break;
			case OBJ :
				if (GL(mem+8) == BEZ4) ok = TRUE;
				else ok = FALSE;
				break;
			case VBOX:
				i = GL(mem + 4) + 8;
				vfont->width[ascii] = GL(mem + i - 8);
				break;
			case OFIN:
				ascii = 0;
				curve = 0;
				break;
			case KRNP:
				vfont->kcount = GS(mem + 8);
				if (vfont->kcount){
					vfont->kern = mallocstructN(short, 2 * vfont->kcount, "kerndata");
					if (vfont->kern) memcpy(vfont->kern, mem + 10, vfont->kcount * 2 * sizeof(short));
					else vfont->kcount = 0;
				}
				break;
			case DATA:
				point = (short *)(mem + 8);
				i = (*point++) - 1;

				if (ok == FALSE || curve == 0 || i < 1) break;

				nu = callocstructN(struct Nurb,1, "pdrawnurb");
				if (nu == 0) break;

				nu->type = 9;
				nu->pntsu = i;
				nu->resolu = 12;
				nu->flagu = 1;

				bezt = callocstructN(struct BezTriple, i, "pdrawbezt");
				if (bezt == 0) {
					freeN(nu);
					break;
				}
				nu->bezt = bezt;
				addtail(curve, nu);

				if (hole) {
					/* volgorde omdraaien */
					bezt += (i - 1);
					for (; i > 0 ; i--) {
						bezt->vec[2][0] = fscale * point[2];
						bezt->vec[2][1] = - fscale * point[3];
						bezt->vec[1][0] = fscale * point[0];
						bezt->vec[1][1] = - fscale * point[1];
						bezt->vec[0][0] = fscale * point[4];
						bezt->vec[0][1] = - fscale * point[5];
						bezt--;
						point += 6;
					}
				} else{
					for (; i > 0 ; i--) {
						bezt->vec[0][0] = fscale * point[2];
						bezt->vec[0][1] = - fscale * point[3];
						bezt->vec[1][0] = fscale * point[0];
						bezt->vec[1][1] = - fscale * point[1];
						bezt->vec[2][0] = fscale * point[4];
						bezt->vec[2][1] = - fscale * point[5];
						bezt++;
						point += 6;
					}
				}
				/*printf("Added %d for character '%c'\n", nu->pntsu, ascii);*/

				autocalchandlesNurb(nu, 0);	/* flag=0, alles doen */
				break;
			}
			len= ((len + 1) & ~1) + 8;
			mem += len;
			totlen -= len;
		}
	}
	return(vfont);
}

struct VFont *objfnt_to_vfont(struct objfnt *fnt)
{
	struct VFont * vfont = 0;
	chardesc *cd;
	short *_data, *data;
	long i, count, stop, ready, meet;
	short first[2], last[2];
	struct Nurb *nu;
	struct BezTriple *bezt, *bez2;
	float scale, dx, dy;


	if (fnt == 0) return (0);

	if (fnt->type != SP_TYPE){
		printf("objfnt is wrong type: need splines\n");
		return(0);
	}

	scale = 4 * fscale * fnt->scale / 10000.0;

	vfont = callocstructN(struct VFont,1,"initvfontfont");
	if (vfont == 0) return(0);

	vfont->scale = 2 * fnt->scale / 1000.0;

	for (i = 0; i < MAX_VF_CHARS; i++){
		cd = getchardesc(fnt, i);
		if (cd && cd->data && cd->datalen){
			vfont->width[i] = scale * cd->movex / fscale;

			_data = data = cd->data;

			do{
				/* eerst even tellen */
				_data = data;
				count = 0;
				ready = stop = FALSE;

				do{
					switch(*data++){
					case SP_MOVETO:
						first[0] = data[0];
						first[1] = data[1];
					case SP_LINETO:
						count++;
						last[0] = data[0];
						last[1] = data[1];
						data += 2;
						break;
					case SP_CURVETO:
						count++;
						last[0] = data[4];
						last[1] = data[5];
						data += 6;
						break;
					case SP_RET:
					case SP_RETCLOSEPATH:
						stop = TRUE;
						ready = TRUE;
						break;
					case SP_CLOSEPATH:
						stop = TRUE;
						break;
					}
				}while (stop == FALSE);

				if (last[0] == first[0] && last[1] == first[1]) meet = 1;
				else meet = FALSE;

				/* is er meer dan 1 uniek punt ?*/

				if (count - meet > 0) {
					data = _data;

					nu = callocstructN(struct Nurb,1, "objfnt_nurb");
					bezt = callocstructN(struct BezTriple, count, "objfnt_bezt");
					if (nu != 0 && bezt != 0){
						addtail(&vfont->nurbsbase[i], nu);
						nu->type= 9;
						nu->pntsu = count;
						nu->resolu= 12;
						nu->flagu= 1;
						nu->bezt = bezt;
						stop = FALSE;

						/* punten inlezen */
						do{
							switch(*data++){
							case SP_MOVETO:
								bezt->vec[1][0] = scale * *data++;
								bezt->vec[1][1] = scale * *data++;
								break;
							case SP_LINETO:
								bez2 = bezt++;
								bezt->vec[1][0] = scale * *data++;
								bezt->vec[1][1] = scale * *data++;
								/* vector handles */
								dx = (bezt->vec[1][0] - bez2->vec[1][0]) / 3.0;
								dy = (bezt->vec[1][1] - bez2->vec[1][1]) / 3.0;
								bezt->vec[0][0] = bezt->vec[1][0] - dx;
								bezt->vec[0][1] = bezt->vec[1][1] - dy;
								bez2->vec[2][0] = bez2->vec[1][0] + dx;
								bez2->vec[2][1] = bez2->vec[1][1] + dy;
								break;
							case SP_CURVETO:
								bezt->vec[2][0] = scale * *data++;
								bezt->vec[2][1] = scale * *data++;
								bezt++;
								bezt->vec[0][0] = scale * *data++;
								bezt->vec[0][1] = scale * *data++;
								bezt->vec[1][0] = scale * *data++;
								bezt->vec[1][1] = scale * *data++;
								break;
							case SP_RET:
							case SP_RETCLOSEPATH:
								stop = TRUE;
								ready = TRUE;
								break;
							case SP_CLOSEPATH:
								stop = TRUE;
								break;
							}
						}while (stop == FALSE);

						if (meet) {
							/* kopieer handles */
							nu->bezt->vec[0][0] = bezt->vec[0][0];
							nu->bezt->vec[0][1] = bezt->vec[0][1];
							/* en vergeet laatste punt */
							nu->pntsu--;
						} else{
							/* vector handles */
							bez2 = nu->bezt;
							dx = (bezt->vec[1][0] - bez2->vec[1][0]) / 3.0;
							dy = (bezt->vec[1][1] - bez2->vec[1][1]) / 3.0;
							bezt->vec[2][0] = bezt->vec[1][0] - dx;
							bezt->vec[2][1] = bezt->vec[1][1] - dy;
							bez2->vec[0][0] = bez2->vec[1][0] + dx;
							bez2->vec[0][1] = bez2->vec[1][1] + dy;
						}
						autocalchandlesNurb(nu, 0); 	/* flag=0, alles doen */
					} else{
						if (nu) freeN(nu);
						if (bezt) freeN(bezt);
					}
				}
				_data = data;
			}while (ready == FALSE);
		}
	}
	return(vfont);
}


struct VFont *loadpsfont(char * name)
{
	struct objfnt * fnt;
	struct VFont *vfont = 0;

	extern struct objfnt * loadpostcriptfont(char *);

	fnt = loadpostcriptfont(name);
	if (fnt == 0) return(0);

	vfont = objfnt_to_vfont(fnt);
	freeobjfnt(fnt);
	return(vfont);
}


struct VFont *loadvfont(char * name)
{
	long file,len, i;
	uchar *mem;
	struct VFont *vfont = 0;

	file = open(name,O_RDONLY);
	if (file != -1){
		len = lseek(file,0L,SEEK_END);
		lseek(file,0L,SEEK_SET);

		mem = (uchar *) mallocN(len,"initvfontmem");
		if (mem) {
			if (read(file,mem,len) == len) {
				switch (GL(mem)){
				case PDFF:
					vfont = loadpdrawfont(mem, len);
					break;
				case OFMAGIC:
					break;
				default:
					/* look for %!PS-AdobeFont-1 */
					for (i = 0; i < len; i++){
						if (mem[i] == '%'){
							if (memcmp(mem + i, "%!PS-AdobeFont-1", 16) == 0) break;
						}
					}
					if (i > len) {
						printf("No Vectorfont or Postscript-1 file %s\n",name);
					} else {
						vfont = loadpsfont(name);
					}
				}
			} else perror("read");
			freeN(mem);

		} else perror("malloc");
		close(file);

	} else perror("open");

	if (vfont){
		strncpy(vfont->name, name, sizeof(vfont->name));
		/*addtail(&G.vfbase,vfont);*/
	}
	return(vfont);
}

short *find_diff_array(start,place,ofs,len)
short *start,*place,ofs;
long len;
{
	/* - loopt array af en geeft adres terug van de eerste verschillende
	   - is cyclisch!
	   - ofs is negatief: achterwaarts
	*/
	short *end,*walk,a,absofs,around=0;

	absofs= abs(ofs);
	end= start+ absofs*(len-1);
	walk= place+ ofs;
	while(walk!=place) {
		if(walk>=end) {
			walk= start;
			around+= 1;
		}
		else if(walk<=start) {
			walk= end;
			around+=1;
		}
		for(a=0;a<absofs;a++) {
			if(walk[a]!=place[a]) return walk;
		}
		walk+=ofs;
		if(around>1) return place;
	}
	return place;
}


void extrudepoly(base)
struct Base *base;
{
	struct ObData *ob;
	struct PolyData *po;
	float t01,t02,cosh,x1,x2,x3,y1,y2,y3;
	short *a1,*a2,*a3,*adr,*adrp,*start,p,vorm,a=0,b,button();


	if(base==0) return;
	if(base->soort!=7 && base->soort!=9) return;

	ob=(struct ObData *)base->d;
	po=(struct PolyData *)ob->po;

	if(po->f45m!=0 || po->f45p!=0) {
		if(po->f45) return;
		if(po->vert==0) return;
		p= po->vert;
		vorm= po->poly;
		po->f45=adr= (short *)mallocN(6*p,"45 faces");
		adrp= (short *)(po+1);
		for(a=0;a<vorm;a++) {

			p= *(adrp++);
			start= adrp;
			for(b=0;b<p;b++) {
				if(b==0) a1=adrp+3*(p-1);
				else a1=adrp-3;
				a2= adrp;
				if(b==p-1) a3=adrp-3*(p-1);
				else a3=adrp+3;
				if(a1[0]==a2[0] && a1[1]==a2[1]) {
					a1= find_diff_array(start,a1,-3,p);
				}
				if(a2[0]==a3[0] && a2[1]==a3[1]) {
					a3= find_diff_array(start,a3,3,p);
				}
				x1=a2[0]-a1[0]; 
				x2=a2[0]-a3[0];
				y1=a2[1]-a1[1]; 
				y2=a2[1]-a3[1];
				t01= fsqrt(x1*x1+y1*y1);
				t02= fsqrt(x2*x2+y2*y2);
				/* if(t01<=0.1 || t02<=0.1) printf("deler nul \n"); */
				x1/=t01; 
				y1/=t01;
				x2/=t02; 
				y2/=t02;

				t02= x1*x2+y1*y2;
				if(fabs(t02)>=1.0) t02= .5*PI;
				else t02= (facos(t02))/2.0;

				t02= fsin(t02);


				if(t02==0) {
					/* 	printf("t02  nul \n"); */
					t02= 1;
				}
				{
					t02= 1000.0/t02;
					x3= x1-x2;
					y3= y1-y2;
					if(x3==0 && y3==0) {
						printf("x3 en y3  nul \n");
						x3= y1;
						y3= -x1;
					} else {
						t01= fsqrt(x3*x3+y3*y3);
						x3/=t01; 
						y3/=t01;
					}

					*(adr++)= (short)-t02*y3;
					*(adr++)= (short)t02*x3;
					*(adr++)= -1000;
				}
				adrp+=3;
			}
		}

	}
	else if(po->f45) {
		freeN(po->f45);
		po->f45=0;
	}
}




void makescreenpoints(points,x1,x2,x3,x4,num)
float *points;
double x1,x2,x3,x4;
long num;
{
	double t1,t2,t3,fnum1,fnum2;

	fnum1 = fnum2 = num;

	t1 = 3.0*(x2-x1) / fnum1;
	fnum1 *= fnum2;
	t2 = 3.0*(x1-2.0*x2+x3) / fnum1;
	fnum1 *= fnum2;
	t3 = (x4-x1+3.0*(x2-x3)) / fnum1;

	x2 = t1+t2+t3;
	x3 = 2.0*t2+6.0*t3;
	x4 = 6.0*t3;

	for(;num >0; num--){
		*points = x1;
		points += 3;
		x1 += x2;
		x2 += x3;
		x3 += x4;
	}
}


long screen_inter(points,bez,res,max)
float *points,bez[5][3],res;
long max;
{
	long num,i,xtra = 0;
	float fnum,buig,x1,y1,z1,x2,y2,z2,len,area;

	x1 = bez[1][0] - bez[0][0];
	y1 = bez[1][1] - bez[0][1];
	z1 = bez[1][2] - bez[0][2];

	x2 = bez[2][0] - bez[1][0];
	y2 = bez[2][1] - bez[1][1];
	z2 = bez[2][2] - bez[1][2];
	len = fsqrt((x2 * x2 + y2 * y2 + z2 * z2) * (x1 * x1 + y1 * y1 + z1 * z1));
	if (len == 0.0 ) len = 1.0;

	if ((x1 * x2 + y1 * y2 + z1 * z2) / len < 0.9999){     /* groter dan 0.6 graden */
		xtra = 1;
		*points++ = bez[1][0] ;
		*points++ = bez[1][1] ;
		*points++ = bez[1][2];
	}

	/* dit moet berekend worden aan de hand van de geprojecteerde coordinaten !!*/

	area = AreaF2Dfl(bez+1,bez+2,bez+3)+AreaF2Dfl(bez+1,bez+3,bez+4)
	    +AreaF2Dfl(bez+1,bez+2,bez+4)+AreaF2Dfl(bez+2,bez+3,bez+4);

	x1 = bez[1][0] - bez[4][0];
	y1 = bez[1][1] - bez[4][1];
	z1 = bez[1][2] - bez[4][2];
	len = fsqrt(x1*x1 + y1*y1 + z1*z1);

	fnum = res * fsqrt(area / 2.0);
	if (len != 0.0) fnum = res * area / len;
	else fnum = 0;
	num = fnum;
	if (num > max) num = max;

	RMK(
	    /* voor als we ooit nog perfecte interpolatie willen , afhankelijk van de buiging punten berekenen*/

	x1 = bez[1][0] - 2.0*bez[2][0] + bez[3][0];
	    x2 = (bez[4][0]-bez[1][0]+3.0*(bez[2][0]-bez[3][0]))/max;
	    y1 = bez[1][1] - 2.0*bez[2][1] + bez[3][1];
	    y2 = (bez[4][1]-bez[1][1]+3.0*(bez[2][1]-bez[3][1]))/max;
	    z1 = bez[1][2] - 2.0*bez[2][2] + bez[3][2];
	    z2 = (bez[4][2]-bez[1][2]+3.0*(bez[2][2]-bez[3][2]))/max;

	    for (i = max-1;i>0;i--){
		x1 += x2;
		    y1 += y2;
		    z1 += z2;
		    buig += fabs(x1) + fabs(y1) + fabs(z1);
	}
	)

	    if (num <= 1){
		*points++ = bez[1][0] ;
		*points++ = bez[1][1] ;
		*points++ = bez[1][2];
		num = 1;
	} else {
		makescreenpoints(points+0,bez[1][0],bez[2][0],bez[3][0],bez[4][0],num);
		makescreenpoints(points+1,bez[1][1],bez[2][1],bez[3][1],bez[4][1],num);
		makescreenpoints(points+2,bez[1][2],bez[2][2],bez[3][2],bez[4][2],num);
	}
	return (num + xtra);
}


void buildchar(bezbase,vfont,ascii,lofsx,lofsy,shear)
struct ListBase * bezbase;
struct VFont *vfont;
uchar ascii;
long lofsx,lofsy;
float shear;
{
	float bez[5][3], points[100][3];
	long i,pres;
	struct bezcurve * bezc;
	struct bezpart * bezp;
	struct Nurb * nu;
	struct BezTriple * bezt1, *bezt2;
	float ofsx, ofsy;

	if (vfont == 0) return;

	bez[0][2] = 0.0;
	bez[1][2] = 0.0;
	bez[2][2] = 0.0;
	bez[3][2] = 0.0;
	bez[4][2] = 0.0;

	ofsx = fscale * lofsx;
	ofsy = fscale * lofsy;
	shear /= fscale;

	nu = vfont->nurbsbase[ascii].first;
	while (nu) {
		bezt1 = nu->bezt;
		if (bezt1) {
			bezc = callocstructN(struct bezcurve,1, "buildchar");
			if (bezc == 0) return;
			i = bezc->count = nu->pntsu;

			bezt2 = bezt1 + 1;

			for (; i > 0 ; i--){
				if (i == 1) bezt2 = nu->bezt;

				bez[0][0] = bezt1->vec[0][0] + ofsx;
				bez[0][1] = bezt1->vec[0][1] + ofsy;
				bez[1][0] = bezt1->vec[1][0] + ofsx;
				bez[1][1] = bezt1->vec[1][1] + ofsy;
				bez[2][0] = bezt1->vec[2][0] + ofsx;
				bez[2][1] = bezt1->vec[2][1] + ofsy;

				bez[3][0] = bezt2->vec[0][0] + ofsx;
				bez[3][1] = bezt2->vec[0][1] + ofsy;
				bez[4][0] = bezt2->vec[1][0] + ofsx;
				bez[4][1] = bezt2->vec[1][1] + ofsy;

				if(shear!=0.0) {
					bez[0][0] -= shear * bezt1->vec[0][1];
					bez[1][0] -= shear * bezt1->vec[1][1];
					bez[2][0] -= shear * bezt1->vec[2][1];
					bez[3][0] -= shear * bezt2->vec[0][1];
					bez[4][0] -= shear * bezt2->vec[1][1];
				}

				pres = screen_inter(points,bez,1.0 /bezresF , bezmaxF);
				bezp = (struct bezpart *) callocN( (3 * sizeof(float)*pres) + sizeof(struct bezpart),"bezp");
				if (bezp == 0) return;

				addtail(&bezc->first,bezp);
				memcpy(bezp->points,points,3 * sizeof(float) * pres);
				bezp->count = pres;

				bezt1++; 
				bezt2++;
			}
			addtail(bezbase,bezc);
		}
		nu = nu->next;
	}
}


void buildcharN(curve,vfont,ascii,lofsx,lofsy,shear)
struct ListBase *curve;
struct VFont *vfont;
uchar ascii;
long lofsx,lofsy;
float shear;
{
	struct BezTriple *bezt1, *bezt2;
	struct Nurb *nu1, *nu2;
	float ofsx, ofsy;
	long i;

	if (vfont == 0) return;
	if (curve == 0) return;

	ofsx = fscale * lofsx;
	ofsy = fscale * lofsy;
	shear /= fscale;

	/* maak een kopie op afstand ofsx, ofsy met shear*/

	nu1 = vfont->nurbsbase[ascii].first;
	while(nu1)
	{
		bezt1 = nu1->bezt;
		if (bezt1){
			nu2 = mallocstructN(struct Nurb,1, "duplichar_nurb");
			if (nu2 == 0) break;
			memcpy(nu2, nu1, sizeof(struct Nurb));

			nu2->bp = 0;
			nu2->knotsu = nu2->knotsv = 0;
			nu2->trim.first = 0;
			nu2->trim.last = 0;
			i = nu2->pntsu;

			bezt2= mallocstructN(struct BezTriple, i, "duplichar_bezt2");
			if (bezt2 == 0){
				freeN(nu2);
				break;
			}
			memcpy(bezt2, bezt1, i * sizeof(struct BezTriple));

			nu2->bezt = bezt2;
			for (; i > 0; i--){
				bezt2->vec[0][0] += ofsx;
				bezt2->vec[0][1] += ofsy;
				bezt2->vec[1][0] += ofsx;
				bezt2->vec[1][1] += ofsy;
				bezt2->vec[2][0] += ofsx;
				bezt2->vec[2][1] += ofsy;
				bezt2++;
			}
			if (shear != 0.0){
				bezt1 = nu1->bezt;
				bezt2 = nu2->bezt;
				i = nu2->pntsu;
				for (; i > 0; i--){
					bezt2->vec[0][0] -= shear * bezt1->vec[0][1];
					bezt2->vec[1][0] -= shear * bezt1->vec[1][1];
					bezt2->vec[2][0] -= shear * bezt1->vec[2][1];
					bezt1++;
					bezt2++;
				}
			}
			addtail(curve, nu2);
		}
		nu1 = nu1->next;
	}
}


void buildtext(bezbase,fo,mode)
struct ListBase * bezbase;
struct FontData *fo;
short mode;	/* mode>0: alleen textcurs doen, mode==-1: Nurblist maken  */
{
	struct VectorFont *vectfont;
	struct VFont *vfont;

	struct chartrans *chartransdata,*ct;
	float *f,maxlen=0,shear;
	long len,i,slen,xof=0,yof=0,linedist,*ld,*linedata,*linedata2;
	short *kern,kcount,str,cnr=0,lnr=0,xtrax;
	uchar ascii,*mem;

	/* opmerking: berekeningen altijd tot en met de '\0' van de string omdat
	   de cursor op die plek moet kunnen staan */

	if(fo==0) return;
	vectfont= fo->vf;
	if (vectfont==0) return;

	/* patch */
	vfont = (struct VFont *) vectfont->objfnt;

	if (fo->str==0) return;

	mem= fo->str;
	slen = strlen(fo->str);
	fo->lines= 1;
	for (i= 0; i<=slen; i++,mem++) {
		ascii = *mem;
		if(ascii== '\n' || ascii== '\r') fo->lines++;
	}

	/* bereken ofset en rotatie van iedere letter */
	ct=chartransdata= callocstructN(struct chartrans, slen+1, "buildtext");
	linedata= (long *)mallocN(4*fo->lines,"buildtext2");
	linedata2= (long *)mallocN(4*fo->lines,"buildtext2");
	xof= fo->xof;
	yof= fo->yof;

	xtrax= 32*fo->set[1];
	linedist= 3200+64*fo->set[3];
	shear= -0.02*( (float)fo->set[4])*fscale;

	for (i = 0 ; i<=slen ; i++) {
		ascii = fo->str[i];
		if(ascii== '\n' || ascii== '\r' || ascii==0) {
			ct->xof= xof;
			ct->yof= yof;
			ct->linenr= lnr;
			ct->charnr= cnr;
			yof-= linedist;
			maxlen= MAX2(maxlen,xof);
			linedata[lnr]= xof;
			linedata2[lnr]= cnr;
			xof= fo->xof;
			lnr++;
			cnr= 0;
		}
		else {
			ct->xof= xof;
			ct->yof= yof;
			ct->linenr= lnr;
			ct->charnr= cnr++;

			xof += vfont->width[ascii] + xtrax;

			if(fo->set[2] && vfont->kern) {
				kern = vfont->kern;
				str = (ascii << 8) + fo->str[i+1];
				for (kcount = vfont->kcount ; kcount >0 ; kcount--) {
					if (*kern++ == str) {
						xof += *kern;
						break;
					}
					kern++;
				}
			}
		}
		ct++;
	}

	/* met alle fontsettings plekken letters berekenen */
	if(fo->set[0]>0 && lnr>1) {
		ct= chartransdata;

		if(fo->set[0]==1) {		/* right */
			for(i=0;i<lnr;i++) linedata[i]= maxlen-linedata[i];
			for (i=0; i<=slen; i++) {
				ct->xof+= linedata[ct->linenr];
				ct++;
			}
		} else if(fo->set[0]==2) {	/* middle */
			for(i=0;i<lnr;i++) linedata[i]= (maxlen-linedata[i])/2;
			for (i=0; i<=slen; i++) {
				ct->xof+= linedata[ct->linenr];
				ct++;
			}
		} else if(fo->set[0]==3) {	/* flush */
			for(i=0;i<lnr;i++)
				if(linedata2[i]>1)
					linedata[i]= (maxlen-linedata[i])/(linedata2[i]-1);
			for (i=0; i<=slen; i++) {
				ct->xof+= ct->charnr*linedata[ct->linenr];
				ct++;
			}
		}
	}

	if(mode>1) {
		/* 2: curs omhoog
		   3: curs omlaag */
		ct= chartransdata+fo->pos;
		if(mode==2 && ct->linenr==0);
		else if(mode==3 && ct->linenr==lnr);
		else {
			if(mode==2) lnr= ct->linenr-1;
			else lnr= ct->linenr+1;
			cnr= ct->charnr;
			/* zoek karakter met lnr en cnr */
			fo->pos= 0;
			ct= chartransdata;
			for (i= 0; i<slen; i++) {
				if(ct->linenr==lnr) {
					if(ct->charnr==cnr) break;
					if( (ct+1)->charnr==0) break;
				}
				else if(ct->linenr>lnr) break;
				fo->pos++;
				ct++;
			}
		}
	}
	/* eerst cursor */
	if(textediting) {
		ct= chartransdata+fo->pos;
		f= G.textcurs[0];
		f[0]= fscale * (-140.0 + ct->xof);
		f[1]= fscale * (0.0 + ct->yof);
		f[2]= fscale * (140.0 + ct->xof);
		f[3]= fscale * (0.0 + ct->yof);
		f[4]= fscale * (140.0 + ct->xof) -2200*shear;
		f[5]= fscale * (2200.0 + ct->yof);
		f[6]= fscale * (-100.0 + ct->xof) -2200*shear;
		f[7]= fscale * (2200.0 + ct->yof);
	}

	if(mode>0) {
		for(i=0; i<4; i++) {
			G.textcurs[i][0]= wordr_fac*(G.textcurs[i][0]-wordr_ofsx);
			G.textcurs[i][1]= wordr_fac*(G.textcurs[i][1]-wordr_ofsy);
		}
	}
	else if(mode== -1) {
		/* nurbdata */
		ct= chartransdata;
		for (i= 0; i<slen; i++) {
			ascii = fo->str[i];
			buildcharN(bezbase, vfont, ascii, ct->xof, ct->yof, shear);
			ct++;
		}
	}
	else {
		/* bereken letters */

		ct= chartransdata;
		for (i= 0; i<slen; i++) {
			ascii = fo->str[i];
			buildchar(bezbase,vfont,ascii,ct->xof,ct->yof,shear);
			ct++;
		}
	}

	freeN(chartransdata);
	freeN(linedata);
	freeN(linedata2);
}

void text_to_curve(base)
struct Base *base;
{
	struct ObData *ob;
	struct FontData *fo;

	ob= (struct ObData *)base->d;
	fo= ob->fo;
	ob->cu= callocstructN(struct CurveData, 1, "text_to_curve");
	ob->cu->ws= 1.0;
	ob->cu->width= 100;
	ob->cu->flag= 255;

	fscale= ((float)fo->set[5])/20.0;

	buildtext(&(ob->cu->curve), fo, -1);
	freevfont(fo->vf);
	if(fo->str) freeN(fo->str);
	/* OUD! if(fo->bez) freeN(fo->bez); */
	freeN(fo);
	if(ob->po->f45) freeN(ob->po->f45);
	freeN(ob->po);
	ob->po= 0;
	ob->fo= 0;

	base->soort= 11;
	makeBevelList(base);
	makeDispList(base);
}

void makepolytext(base)
struct Base *base;
{
	struct ObData *ob;
	struct PolyData *po,*pon;
	struct FontData *fo;
	struct ListBase _bezbase;
	struct ListBase *bezbase = &_bezbase;
	struct bezcurve * bezc;
	struct bezpart * bezp;
	float fac,min[3],max[3];
	long i,c,vert=0,poly=0;
	short *p,*pnr;
	char *tempstr= 0;
	
	bezbase->first = bezbase->last = 0;
	min[0]= min[1]= min[2]= 1.0e20;
	max[0]= max[1]= max[2]= -1.0e20;

	ob= (struct ObData *)base->d;
	po= ob->po;
	fo= ob->fo;

	fscale= ((float)fo->set[5])/20.0;
	bezresF= fscale*((float)bezres);
	bezmaxF= bezmax;

	if(textediting==0 && ob->ef==4) {	/* build effect */
		i= strlen(fo->str);
		if(i) {
			tempstr= mallocN(i+1, "makepolytext");
			strcpy(tempstr, fo->str);
			c= build_schiphol(base, i);
			if(c>=0 && c<=i) fo->str[c]= 0;
		}
	}

	buildtext(bezbase,fo,0);

	if(ob->ef==4 && tempstr) {
		strcpy(fo->str, tempstr);
		freeN(tempstr);
	}

	/* aantal punten en poly's tellen */
	bezc = bezbase->first;
	while (bezc){
		bezp = bezc->first;
		if (bezp) {
			poly++;
			vert+=1;
			while(bezp){
				for(i=0; i<bezp->count; i++) {
					min[0]= MIN2(min[0],bezp->points[i][0]);
					min[1]= MIN2(min[1],bezp->points[i][1]);
					max[0]= MAX2(max[0],bezp->points[i][0]);
					max[1]= MAX2(max[1],bezp->points[i][1]);
				}
				vert+= bezp->count;
				bezp = bezp->next;
			}
		}
		bezc = bezc->next;
	}

	pon= (struct PolyData *)callocN(sizeof(struct PolyData)+6*vert+2*poly, "makepolytext");
	if(po) {
		if(po->f45) {
			freeN(po->f45);
			po->f45= 0;
		}
		memcpy(pon,po,sizeof(struct PolyData));
		freeN(po);
	}
	else {	/* polydata initialiseren */
		pon->f1= 31;

	}

	po= pon;
	ob->po= po;
	po->vert= vert;
	po->poly= poly;
	po->us= 1;

	/* wordruimte */
	if(fo->set[0]==2) {	/* middle */
		po->afm[0]= (max[0]-min[0]+1)/2;
		po->afm[1]= (max[1]-min[1]+1)/2;
		wordr_ofsx= (min[0]+max[0])/2;
		wordr_ofsy= (min[1]+max[1])/2;
		fac= MAX2(max[0]-min[0]+1,max[1]-min[1]+1);
		fo->xof-= wordr_ofsx;
		fo->yof-= wordr_ofsy;
	}
	else {
		po->afm[0]= MAX2(fabs(max[0]),fabs(min[0]));
		po->afm[1]= MAX2(fabs(max[1]),fabs(min[1]));
		wordr_ofsx= 0;
		wordr_ofsy= 0;
		fac= MAX4(fabs(max[0]),fabs(min[0]),fabs(max[1]),fabs(min[1]));
	}
	if(fac>1.0e19) {
		fac= 32760.0;
		wordr_ofsx= 0;
		wordr_ofsy= 0;
	}
	fac= 32760.0/fac;
	po->ws= 1.0/fac;
	po->depth= fscale*8.0*fac;
	po->n[2]= -32767;

	/* word array maken */
	p= (short *)(po+1);
	bezc = bezbase->first;
	while (bezc) {
		if(bezc->hole==1) {
			bezp = bezc->first;
			if (bezp) {
				pnr= p;
				p++;
				while(bezp){
					*pnr+= bezp->count;
					for(i=0; i<bezp->count; i++) {
						p[0]= fac*(bezp->points[i][0]-wordr_ofsx);
						p[1]= fac*(bezp->points[i][1]-wordr_ofsy);
						p+=3;
					}
					bezp = bezp->next;

				}
				(*pnr)++;
				bezp = bezc->first;
				p[0]= fac*(bezp->points[0][0]-wordr_ofsx);
				p[1]= fac*(bezp->points[0][1]-wordr_ofsy);
				p+=3;
			}
		}
		else {
			bezp = bezc->last;
			if (bezp) {
				pnr= p;
				p++;
				while(bezp){
					*pnr+= bezp->count;
					for(i=bezp->count-1; i>=0; i--) {
						p[0]= fac*(bezp->points[i][0]-wordr_ofsx);
						p[1]= fac*(bezp->points[i][1]-wordr_ofsy);
						p+=3;
					}
					bezp = bezp->prev;

				}
				(*pnr)++;
				bezp = bezc->last;
				p[0]= fac*(bezp->points[bezp->count-1][0]-wordr_ofsx);
				p[1]= fac*(bezp->points[bezp->count-1][1]-wordr_ofsy);
				p+=3;
			}
		}
		bezc= bezc->next;
	}

	/* cursor? */
	if(textediting) {
		wordr_fac= fac;
		for(i=0; i<4; i++) {
			G.textcurs[i][0]= fac*(G.textcurs[i][0]-wordr_ofsx);
			G.textcurs[i][1]= fac*(G.textcurs[i][1]-wordr_ofsy);
		}
	}

	/* vrijgeven */
	bezc = bezbase->first;
	while (bezc){
		freelistN(&bezc->first);
		bezc = bezc->next;
	}

	freelistN(bezbase);

}

char findaccent(char char1, char code)
{
	char new= 0;
	
	if(char1=='a') {
		if(code=='`') new= 224;
		else if(code==39) new= 255;
		else if(code=='^') new= 226;
		else if(code=='~') new= 227;
		else if(code=='"') new= 228;
		else if(code=='o') new= 229;
		else if(code=='e') new= 230;
		else if(code=='-') new= 170;
	}
	else if(char1=='c') {
		if(code==',') new= 231;
		if(code=='|') new= 162;
	}
	else if(char1=='e') {
		if(code=='`') new= 232;
		else if(code==39) new= 233;
		else if(code=='^') new= 234;
		else if(code=='"') new= 235;
	}
	else if(char1=='i') {
		if(code=='`') new= 236;
		else if(code==39) new= 237;
		else if(code=='^') new= 238;
		else if(code=='"') new= 239;
	}
	else if(char1=='n') {
		if(code=='~') new= 241;
	}
	else if(char1=='o') {
		if(code=='`') new= 242;
		else if(code==39) new= 243;
		else if(code=='^') new= 244;
		else if(code=='~') new= 245;
		else if(code=='"') new= 246;
		else if(code=='/') new= 248;
		else if(code=='-') new= 186;
		else if(code=='e') new= 143;
	}
	else if(char1=='s') {
		if(code=='s') new= 167;
	}
	else if(char1=='u') {
		if(code=='`') new= 249;
		else if(code==39) new= 250;
		else if(code=='^') new= 251;
		else if(code=='"') new= 252;
	}
	else if(char1=='y') {
		if(code==39) new= 253;
		else if(code=='"') new= 255;
	}
	else if(char1=='A') {
		if(code=='`') new= 192;
		else if(code==39) new= 193;
		else if(code=='^') new= 194;
		else if(code=='~') new= 195;
		else if(code=='"') new= 196;
		else if(code=='o') new= 197;
		else if(code=='e') new= 198;
	}
	else if(char1=='C') {
		if(code==',') new= 199;
	}
	else if(char1=='E') {
		if(code=='`') new= 200;
		else if(code==39) new= 201;
		else if(code=='^') new= 202;
		else if(code=='"') new= 203;
	}
	else if(char1=='I') {
		if(code=='`') new= 204;
		else if(code==39) new= 205;
		else if(code=='^') new= 206;
		else if(code=='"') new= 207;
	}
	else if(char1=='N') {
		if(code=='~') new= 209;
	}
	else if(char1=='O') {
		if(code=='`') new= 210;
		else if(code==39) new= 211;
		else if(code=='^') new= 212;
		else if(code=='~') new= 213;
		else if(code=='"') new= 214;
		else if(code=='/') new= 216;
		else if(code=='e') new= 141;
	}
	else if(char1=='U') {
		if(code=='`') new= 217;
		else if(code==39) new= 218;
		else if(code=='^') new= 219;
		else if(code=='"') new= 220;
	}
	else if(char1=='Y') {
		if(code==39) new= 221;
	}
	else if(char1=='1') {
		if(code=='4') new= 188;
		if(code=='2') new= 189;
	}
	else if(char1=='3') {
		if(code=='4') new= 190;
	}
	else if(char1==':') {
		if(code=='-') new= 247;
	}
	else if(char1=='-') {
		if(code==':') new= 247;
		if(code=='|') new= 135;
		if(code=='+') new= 177;
	}
	else if(char1=='|') {
		if(code=='-') new= 135;
		if(code=='=') new= 136;
	}
	else if(char1=='=') {
		if(code=='|') new= 136;
	}
	else if(char1=='+') {
		if(code=='-') new= 177;
	}
	
	if(new) return new;
	else return char1;
}

void textedit()
{
	struct ObData *ob;
	struct FontData *fo;
	int accentcode= 0;
	short max=1000,dev,c,x,temp,doit,cursmove=0;
	char buf[1000],*str;

	if(G.ebase==0 || G.ebase->soort!=9) return;
	G.ebase->f&= ~1;
	ob= (struct ObData *)G.ebase->d;
	fo= ob->fo;
	strcpy(buf,fo->str);
	str= fo->str;
	fo->str= buf;

	fo->pos= fo->len= strlen(buf);
	setcursorN(1);
	G.f|= 8;	/* geen overdraw cursor tekenen */
	textediting= 1;
	makepolytext(G.ebase);
	projektie();

	qdevice(KEYBD);

	while(getbutton(RIGHTMOUSE));
	if(qtest()==KEYBD) dev= traces_qread(&c);  /* event 'A' weghalen */

	while(textediting) {
		dev = traces_qread(&c);
		if(c) {
			if(dev==INPUTCHANGE) {
				winset(c);
				G.winakt= c;
				continue;
			}
			else if(dev==LEFTMOUSE) break;
			else if(dev==RIGHTMOUSE) break;
			else if(dev==ESCKEY ) break;
			else if(dev==TABKEY ) break;
			
			else if(dev == KEYBD){
				if ((c > 31 && c < 256 && c != 127) || (c == 13)) {
					if(accentcode && fo->pos>1) {
						buf[fo->pos-1]= findaccent(buf[fo->pos-1], c);
					}
					else if(fo->len<max-1) {
						if(getbutton(LEFTALTKEY) || getbutton(RIGHTALTKEY) ) {
							if(c=='t') c= 137;
							else if(c=='c') c= 169;
							else if(c=='r') c= 174;
							else if(c=='f') c= 164;
							else if(c=='p') c= 163;
							else if(c=='y') c= 165;
							else if(c=='.') c= 138;
							else if(c=='g') c= 176;
							else if(c=='s') c= 223;
							else if(c=='1') c= 185;
							else if(c=='2') c= 178;
							else if(c=='3') c= 179;
							else if(c=='%') c= 139;
							else if(c=='?') c= 191;
							else if(c=='!') c= 161;
							else if(c=='x') c= 215;
							else if(c=='>') c= 187;
							else if(c=='<') c= 171;
							else if(c=='v') c= 1001;
						}
						if(c==1001) {
							int file, filelen;
							char *strp;
							
							file= open("/tmp/.cutbuffer", O_RDONLY);
							if(file>0) {
							
								filelen= lseek(file, 0, 2);	/* seek end */
								lseek(file, 0, 0);		/* en weer terug */
							
								strp= mallocN(filelen+1, "tempstr");
								read(file, strp, filelen);
								close(file);
								strp[filelen]= 0;
								if(fo->len+filelen<max) {
									strcat(buf, strp);
									fo->len= strlen(buf);
									fo->pos= fo->len;
								}
								freeN(strp);
							}
						}
						else {
							for(x=fo->len;x>fo->pos;x--) buf[x]= buf[x-1];
							buf[fo->pos]= c;
							fo->pos++;
							fo->len++;
							buf[fo->len]='\0';
						}
					}
					accentcode= 0;
				}
			} else {
				cursmove= 0;
				c= rawtoascii(dev);
				
				switch(c) {
				case 0:
					break;
				case '\b':	/* backspace */
					if(fo->len!=0) {
						if(getbutton(LEFTALTKEY) || getbutton(RIGHTALTKEY) ) {
							if(fo->pos>0) accentcode= 1;
						}
						else if(fo->pos>0) {
							for(x=fo->pos;x<=fo->len;x++) buf[x-1]= buf[x];
							fo->pos--;
							buf[--fo->len]='\0';
						}
					}
					break;
				case 255:	/* right */
					fo->pos++;
					cursmove= 1;
					break;
				case 254:	/* left */
					fo->pos--;
					cursmove= 1;
					break;
				case 253:	/* shift right */
					while(fo->pos<fo->len) {
						if( buf[fo->pos]==0) break;
						if( buf[fo->pos]=='\n') break;
						if( buf[fo->pos]=='\r') break;
						fo->pos++;
					}
					cursmove= 1;
					break;
				case 252:	/* shift left */
					while(fo->pos>0) {
						if( buf[fo->pos-1]=='\n') break;
						if( buf[fo->pos-1]=='\r') break;
						fo->pos--;
					}
					cursmove= 1;
					break;
				case 250:	/* up */
					if (getbutton(RIGHTALTKEY)){
						if (fo->pos && buf[fo->pos - 1] < 255) buf[fo->pos - 1]++;
					} else cursmove= 2;
					break;
				case 251:	/* down */
					if (getbutton(RIGHTALTKEY)){
						if (fo->pos && buf[fo->pos - 1] > 1) buf[fo->pos - 1]--;
					} else cursmove= 3;
					break;
				case 248:	/* shift up */
					fo->pos= 0;
					cursmove= 1;
					break;
				case 249:	/* shift down */
					fo->pos= fo->len;
					cursmove= 1;
					break;
				default:
					if (getbutton(RIGHTALTKEY) == 0 || getbutton(LEFTALTKEY)==0) break;
					/* alleen nog maar voor speciale toetsen */
					c += 128;
					if(fo->len<max-1) {
						for(x=fo->len;x>fo->pos;x--) buf[x]= buf[x-1];
						buf[fo->pos]=c;
						fo->pos++;
						fo->len++;
						buf[fo->len]='\0';
					}
				}
			}

			if(fo->pos>fo->len) fo->pos= fo->len;
			else if(fo->pos>=max) fo->pos= max;
			else if(fo->pos<0) fo->pos= 0;
			else doit= 1;
		}
		if(doit && qtest()==0) {
			if(cursmove) {
				buildtext(0,ob->fo,cursmove);
			}
			else makepolytext(G.ebase);

			projektie();
			doit= 0;
		}
	}

	unqdevice(KEYBD);

	textediting= 0;
	if(dev!=ESCKEY) {
		freeN(str);
		fo->str= mallocN(fo->len+1,"tekstedit");
		strcpy(fo->str,buf);
	}
	else fo->str= str;

	fo->len= strlen(fo->str);

	G.ebase->f|= 1;
	makepolytext(G.ebase);

	G.f &= ~8;
	setcursorN(0);
	extrudepoly(G.ebase);
	projektie();

	G.ebase= 0;

	tekenmainbuts(2);	/* omdat polydata-pointer veranderd is */
}

void to_upper()
{
	struct Base *base;
	struct ObData *ob;
	struct FontData *fo;
	int len, ok;
	char *str;
	
	if(G.ebase) {
		error("Not in editmode!");
		return;
	}
	
	base= G.firstbase;
	while(base) {
		if(base->f & 1) {
			if(base->lay & view0.lay) {
				if(base->soort==9) {
					ok= 0;
					ob= (struct ObData *)(base->d);
					fo= ob->fo;
					
					len= strlen(fo->str);
					str= fo->str;
					while(len) {
						if( *str>=97 && *str<=122) {
							ok= 1;
							*str-= 32;
						}
						len--;
						str++;
					}
					
					if(ok==0) {
						len= strlen(fo->str);
						str= fo->str;
						while(len) {
							if( *str>=65 && *str<=90) {
								*str+= 32;
							}
							len--;
							str++;
						}
					}
					makepolytext(base);
					extrudepoly(base);
				}
			}
		}
		base= base->next;
	}

	projektie();

}

