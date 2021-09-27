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

/* 	files.c	*/


/*	
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'


short		addedge(a1,a2)
	   edges(adrvl,tot,vlak)
short		contrpuntnorm(n,puno,adrpu,v1,z)
		calcnorm(soort,v1,v2,v3,n)
		normalen(adrve,adrvl,tot,vlak)	

		sculptlezer(di,fi)
		pdrawlezer(di,fi)

		splitdirstring(di,fi)
		testextensie(s,ext)

		lees_view(file,len)
		lees_render(file,len)
		lees_base(file)
		lees_data(file,len)
long		*newadr(adr)
		leesscene(dir)

		schr_view(file)
		schr_render(file)
		schr_stri(file)
		schr_base(file,base)
		schr_data(file,data)
		schrijfscene(di,fi)

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

#define FORM MAKE_ID('F','O','R','M')
#define SC3D MAKE_ID('S','C','3','D')
#define VERT MAKE_ID('V','E','R','T')
#define FACE MAKE_ID('F','A','C','E')
#define PDPF MAKE_ID('P','D','P','F')
#define PDFF MAKE_ID('P','D','F','F')
#define OBJ  MAKE_ID('O','B','J',' ')
#define BEZ4 MAKE_ID('B','E','Z','4')
#define DBOX MAKE_ID('D','B','O','X')
#define FILL MAKE_ID('F','I','L','L')
#define DATA MAKE_ID('D','A','T','A')
#define USER MAKE_ID('U','S','E','R')

#define OB MAKE_ID(0,0,'O','B')
#define TA MAKE_ID(0,0,'T','A')
#define BE MAKE_ID(0,0,'B','E')
#define Z4 MAKE_ID(0,0,'Z','4')

#define TRI0 MAKE_ID('T','R','I','0')
#define TRa2 MAKE_ID('T','R','a','2')
#define TRa1 MAKE_ID('T','R','a','1')
#define BASE MAKE_ID('B','A','S','E')
#define DATA MAKE_ID('D','A','T','A')
#define GLOB MAKE_ID('G','L','O','B')
#define WRLD MAKE_ID('W','R','l','D')
#define VIEW MAKE_ID('V','I','E','W')
#define VIEN MAKE_ID('V','I','E','N')
#define REND MAKE_ID('R','E','N','D')
#define TEXT MAKE_ID('T','E','X','T')
#define TEXN MAKE_ID('T','E','X','N')
#define VFNT MAKE_ID('V','F','N','T')
/* nieuwe compiler vreet geen $ meer, hieronde is dus WEL goed */
#define SKBS MAKE_ID('S','K','B','$')
#define PICS MAKE_ID('P','I','C','$')
#define SCRS MAKE_ID('S','C','R','$')
#define IMAS MAKE_ID('I','M','A','$')
#define BSCS MAKE_ID('B','S','C','$')
#define ENDT MAKE_ID('E','N','D','T')

#define KNOTSU(nu)	    ( (nu)->orderu+ (nu)->pntsu+ (nu->orderu-1)*((nu)->flagu & 1) )
#define KNOTSV(nu)	    ( (nu)->orderv+ (nu)->pntsv+ (nu->orderv-1)*((nu)->flagv & 1) )

struct ColBlckA1 {
	char r, g, b;
	char mode, kref, tra, kspec, mir;
	short speclim;
	char spec, tex1, tex2, tex3;
	char ang, mirr, mirg, mirb;
	char ambr, ambb, ambg;
	char rt, met, rt2;
	short hasize;
};


/* globale variabelen */

extern short panoramaview,panodegr;
extern short bezres, bezmax;	/* font.c */
short leesfout;
long *ablock,*t00,tobl,ednietin;
char *ablockflag;

void convertstringcode(str)
char *str;
{
	char *slash,temp[100];

	if (str[0] == '/' && str[1] == '/') {
		strcpy(temp,G.sce);
		slash = strrchr(temp,'/');
		if (slash) {
			strcpy(slash+1,str+2);
			strcpy(str,temp);
		}
	}
}


void splitdirstring(char *di,char *fi)
{
	short a,b=0;

	a=strlen(di)-1;
	while(di[a]!='/' && (a>=0)) a--;

	a++;
	while(di[a]!=0) {
		fi[b++]=di[a];
		di[a]=0;
		a++;
	}
	fi[b]=0;
}

short testextensie(s,ext)
char *s,*ext;
{
	short a,b;

	a=strlen(s);
	b=strlen(ext);
	if(a==0 || b==0 || b>=a) return 0;
	while(b>0) {
		a--;
		b--;
		if(s[a]!=ext[b]) return 0;
	}
	return 1;
}




short testedge(a1,a2,edv)
short a1,a2,edv[][4];
{

	if(edv[a1][0]==a2) return 1;
	else if(edv[a2][0]==a1) return 1;
	else if(edv[a1][1]==a2) return 1;
	else if(edv[a2][1]==a1) return 1;
	else if(edv[a1][2]==a2) return 1;
	else if(edv[a2][2]==a1) return 1;
	else if(edv[a1][3]==a2) return 1;
	else if(edv[a2][3]==a1) return 1;
	return 0;
}

short addedge(a1,a2,edv)
short a1,a2,edv[][4];
{
	short li=0,re=0,pli,pre;

	if(testedge(a1,a2,edv)) return 1;

	if(edv[a1][0]== -2) {
		li++;
		pli=0;
	}
	if(edv[a1][1]== -2) {
		li++;
		pli=1;
	}
	if(edv[a1][2]== -2) {
		li++;
		pli=2;
	}
	if(edv[a1][3]== -2) {
		li++;
		pli=3;
	}
	if(edv[a2][0]== -2) {
		re++;
		pre=0;
	}
	if(edv[a2][1]== -2) {
		re++;
		pre=1;
	}
	if(edv[a2][2]== -2) {
		re++;
		pre=2;
	}
	if(edv[a2][3]== -2) {
		re++;
		pre=3;
	}

	if(li==0 && re==0) {
		for(li=0;li<4;li++) {
			for(re=0;re<4;re++) {
				if(edv[ edv[a1][li] ][re]== -2) {
					edv[ edv[a1][li] ][re]= a1;
					edv[a1][li]= a2;
					return 0;
				}
				if(edv[ edv[a2][li] ][re]== -2) {
					edv[ edv[a2][li] ][re]= a2;
					edv[a2][li]= a1;
					return 0;
				}
			}
		}
		ednietin++;
	}
	else {
		if(li==re) {
			if(a1 & 1) li++;
			else re++;
		}
		if(li>re) edv[a1][pli]= a2;
		else edv[a2][pre]= a1;
	}

	return 0;
}

void edges(adrvl,tot,vlak)
struct VlakOb *adrvl;
long tot,vlak;
{
	struct VlakOb *adrvlo;
	short *e,*edv;
	long t,leeg=0;

	if(adrvl==0 || tot==0 || vlak==0) return;

	edv = (short *)mallocN(8*tot,"edges");
	e= edv;
	for(t=0;t< 4*tot;t++,e++) *e= -2;

	ednietin=0;
	adrvlo= adrvl;
	for(t=0;t<vlak;t++) {
		adrvl->ec= (adrvl->ec & 31);
		if(addedge(adrvl->v1,adrvl->v2,edv)) adrvl->ec +=128;
		if(addedge(adrvl->v2,adrvl->v3,edv)) adrvl->ec +=64;
		if(addedge(adrvl->v3,adrvl->v1,edv)) adrvl->ec +=32;
		adrvl++;
	}

	leeg=0;
	adrvl= adrvlo;
	for(t=0;t<vlak;t++) {
		if(testedge(adrvl->v1,adrvl->v2,edv)==0) leeg++;
		if(testedge(adrvl->v2,adrvl->v3,edv)==0) leeg++;
		if(testedge(adrvl->v3,adrvl->v1,edv)==0) leeg++;
		adrvl++;
	}
	/*	if(leeg) printf("edges niet: %d\n",leeg); */
	freeN(edv);

}


int sortedge(edg1,edg2)
long *edg1,*edg2;
{
	if (edg1[0] < edg2[0]) return (-1);
	if (edg1[0] > edg2[0]) return (1);

	if (edg1[1] < edg2[1]) return (-1);
	if (edg1[1] > edg2[1]) return (1);

	if (edg1[2] < edg2[2]) return (-1);
	if (edg1[2] > edg2[2]) return (1);

	return(0);
}


void edgesnew(adrvl,tot,vlak)
struct VlakOb *adrvl;
long tot,vlak;
{
	struct VlakOb *adrvlo,*vlak1,*vlak2;
	long *e,*edv,*first;
	long t,temp,more,teken,count[4];

	if(adrvl==0 || tot==0 || vlak==0) return;

	e = edv = (long *)mallocN(4*3*sizeof(long)*vlak,"edgesnew");

	adrvlo = adrvl;
	for(t=vlak ; t> 0 ;t--) {
		adrvl->ec = adrvl->ec | (32 + 64 + 128);	/* alle vlaggen gewist */
		edv[0] = adrvl->v1;
		edv[1] = adrvl->v2;
		edv[2] = (long) adrvl;
		edv[3] = 128;

		edv[4] = adrvl->v3;
		edv[5] = adrvl->v2;
		edv[6] = (long) adrvl;
		edv[7] = 64;

		edv[8] = adrvl->v1;
		edv[9] = adrvl->v3;
		edv[10] = (long) adrvl;
		edv[11] = 32;

		edv += 12;
		adrvl++;
	}

	edv = e;
	for (t = 3 * vlak ; t > 0 ; t--){
		if (edv[0] > edv[1]){
			temp = edv[0];
			edv[0] = edv[1];
			edv[1] = temp;
		}
		edv += 4;
	}

	edv = e;
	qsort(edv,3*vlak,4*sizeof(long),sortedge);

	/* loop de gesorteerde edges af, en vergelijk normalen van bijbehorende vlakken */

	edv = e;
	for (t = 3 * vlak ; t > 0 ; t--){
		first = edv ;
		more = teken = 0 ;
		vlak1 = (struct VlakOb *) edv[2];
		edv += 4;
		while (edv[0] == first[0] && edv[1] == first[1]){
			more++;
			if (teken == 0){
				vlak2 = (struct VlakOb *) edv[2];
				temp = abs(vlak1->n[0] * vlak2->n[0] + vlak1->n[1] * vlak2->n[1] + vlak1->n[2] * vlak2->n[2]);
				if (temp < 0x3ff00000) teken = 1;
			}
			edv += 4;
			if (--t <= 0) break;
		}

		if (teken == 1 || more == 0) {
			/* deze edge moet getekend worden */
			vlak1->ec &= ~first[3];
		}
	}

	/* extra: edge-vlakken (v1==v2) afhandelen */
	adrvl = adrvlo;
	for(t=vlak ; t> 0 ;t--, adrvl++) {
		if(adrvl->v1==adrvl->v2) {
			adrvl->ec = adrvl->ec | (32 + 64 + 128);
			adrvl->ec &= ~64;
		}
	}

	/* defaults:
trap:	count: 52 799 1358 730
stoel: 	count: 28 794 1780 739
*/
	/*
	adrvl = adrvlo;
	count[0] = count[1] = count[2] = count[3] = 0;
	for(t=vlak ; t> 0 ;t--) {
		switch(adrvl->ec & 0xe0){
		case 0:
			count[0]++;
			break;
		case 32:
		case 64:
		case 128:
			count[1]++;
			break;
		case 32+64+128:
			count[3]++;
			break;
		default:
			count[2]++;
		}
		adrvl++;
	}
	printf("count: %d %d %d %d\n",count[0],count[1],count[2],count[3]);
*/
	freeN(e);
}

short contrpuntnorm(n,puno,adrpu,v1,z)
float *n,*puno;
long *adrpu;
short v1,*z;
{
	long *t;
	float inp;

	if(adrpu!=0 && z[v1]>0) {
		t=adrpu+3*v1;
		inp=n[0]*t[0]+n[1]*t[1]+n[2]*t[2];
		if(inp<0.0) return 1;
	}
	else {
		inp= n[0]*puno[0]+n[1]*puno[1]+n[2]*puno[2];
		if(inp< 0.0) return 1;
	}
	return 0;
}

short contrpuntnormr(n,puno)
float *n,*puno;
{
	float inp;

	inp=n[0]*puno[0]+n[1]*puno[1]+n[2]*puno[2];
	if(inp<0.0) return 1;
	return 0;
}

void normalen(adrve,adrvl,tot,vlak)
struct VertOb *adrve;
struct VlakOb *adrvl;
long tot,vlak;
{
	float n1[3],n2[3],n3[3],*adrco,*adrno,*tfl,f,fac,*f1,*temp;
	long *adrpu=0,*temp1,len, a;
	short pumanorm=0, x, y, z, *z1=0;
	struct VlakOb *adrvlo;
	struct VertOb *adrve1,*adrve2,*adrve3;

	if(adrvl==0 || adrve==0 || tot<=0 || vlak<=0) return;

	adrco=(float *)callocN(12+12*vlak,"normalen1");
	adrno=(float *)callocN(12+12*tot,"normalen2");

	if(pumanorm) {
		adrpu=(long *)callocN(12+12*tot,"normalen3");
		z1=(short *)callocN(2*tot,"normalen4");
	}
	adrvlo=adrvl;
	tfl=adrco;
	/* berekenen cos hoeken en puntmassa's */
	for(a=0;a<vlak;a++) {
		adrve1= adrve+adrvl->v1;
		adrve2= adrve+adrvl->v2;
		adrve3= adrve+adrvl->v3;
		n1[0]= adrve2->c[0]-adrve1->c[0];
		n1[1]= adrve2->c[1]-adrve1->c[1];
		n1[2]= adrve2->c[2]-adrve1->c[2];
		n2[0]= adrve3->c[0]-adrve2->c[0];
		n2[1]= adrve3->c[1]-adrve2->c[1];
		n2[2]= adrve3->c[2]-adrve2->c[2];
		n3[0]= adrve1->c[0]-adrve3->c[0];
		n3[1]= adrve1->c[1]-adrve3->c[1];
		n3[2]= adrve1->c[2]-adrve3->c[2];
		Normalise(n1);
		Normalise(n2);
		Normalise(n3);
		*(tfl++)= facos(-n1[0]*n3[0]-n1[1]*n3[1]-n1[2]*n3[2]);
		*(tfl++)= facos(-n1[0]*n2[0]-n1[1]*n2[1]-n1[2]*n2[2]);
		*(tfl++)= facos(-n2[0]*n3[0]-n2[1]*n3[1]-n2[2]*n3[2]);
		if(pumanorm) {
			x= (adrve1->c[0]+adrve2->c[0]+adrve3->c[0])/3;
			y= (adrve1->c[1]+adrve2->c[1]+adrve3->c[1])/3;
			z= (adrve1->c[2]+adrve2->c[2]+adrve3->c[2])/3;
			n1[0]= x-adrve1->c[0];
			n2[0]= x-adrve2->c[0];
			n3[0]= x-adrve3->c[0];
			n1[1]= y-adrve1->c[1];
			n2[1]= y-adrve2->c[1];
			n3[1]= y-adrve3->c[1];
			n1[2]= z-adrve1->c[2];
			n2[2]= z-adrve2->c[2];
			n3[2]= z-adrve3->c[2];
			Normalise(n1);
			Normalise(n2);
			Normalise(n3);
			tfl-=3;
			temp1=adrpu+3*adrvl->v1;
			*(temp1++) += *(tfl) * n1[0];
			*(temp1++) += *(tfl) * n1[1];
			*temp1     += *(tfl++) * n1[2];
			temp1=adrpu+3*adrvl->v2;
			*(temp1++) += *(tfl) * n2[0];
			*(temp1++) += *(tfl) * n2[1];
			*temp1     += *(tfl++) * n2[2];
			temp1=adrpu+3*adrvl->v3;
			*(temp1++) += *(tfl) * n3[0];
			*(temp1++) += *(tfl) * n3[1];
			*temp1     += *(tfl++) * n3[2];
			z1[adrvl->v1]++;
			z1[adrvl->v2]++;
			z1[adrvl->v3]++;
		}
		adrvl++;
	}
	/* puntmassa's testen */
	if(pumanorm) {
		temp1=adrpu;
		for(a=0;a<tot;a++) {
			if(z1[a]==1) {
				z1[a]=1;
				temp1+=3;
			}
			else {
				len= (*temp1) *  *(temp1++);
				len+=(*temp1) *  *(temp1++);
				len+=(*temp1) *  *(temp1++);
				if(len<124) z1[a]=1;
			}
		}
	}
	/* berekenen normalen en optellen bij puno's */
	adrvl= adrvlo;
	tfl= adrco;
	for(a=0;a<vlak;a++) {
		adrve1= adrve+adrvl->v1;
		adrve2= adrve+adrvl->v2;
		adrve3= adrve+adrvl->v3;
		CalcNormShort(adrve1->c, adrve2->c, adrve3->c, n3);
		adrvl->n[0]= ffloor(n3[0]*32767.0);
		adrvl->n[1]= ffloor(n3[1]*32767.0);
		adrvl->n[2]= ffloor(n3[2]*32767.0);

		temp= adrno+3*(adrvl->v1);
		fac= *(tfl++);
		if( contrpuntnorm(n3, temp, adrpu, adrvl->v1, z1) ) fac= -fac ;
		*(temp++) += fac*n3[0];
		*(temp++) += fac*n3[1];
		*(temp)   += fac*n3[2];
		temp= adrno+3*adrvl->v2;
		fac= *(tfl++);
		if( contrpuntnorm(n3, temp, adrpu, adrvl->v2, z1) ) fac= -fac ;
		*(temp++) += fac*n3[0];
		*(temp++) += fac*n3[1];
		*(temp)   += fac*n3[2];
		temp= adrno+3*adrvl->v3;
		fac= *(tfl++);
		if( contrpuntnorm(n3, temp, adrpu, adrvl->v3, z1) ) fac= -fac ;
		*(temp++) += fac*n3[0];
		*(temp++) += fac*n3[1];
		*(temp)   += fac*n3[2];
		adrvl++;
	}
	/* normaliseren puntnormalen */
	f1=adrno;
	adrve2=adrve;
	for(a=0; a<tot; a++) {
		Normalise(f1);
		adrve2->n[0]= ffloor( *(f1++)*32767.0 );
		adrve2->n[1]= ffloor( *(f1++)*32767.0 );
		adrve2->n[2]= ffloor( *(f1++)*32767.0 );
		adrve2++;
	}
	/* puntnormaal omklap-vlaggen voor bij shade */
	adrvl=adrvlo;
	for(a=0;a<vlak;a++) {
		adrve1=adrve+adrvl->v1;
		adrve2=adrve+adrvl->v2;
		adrve3=adrve+adrvl->v3;
		adrvl->f=0;
		len=adrvl->n[0]*adrve1->n[0]+adrvl->n[1]*adrve1->n[1]+adrvl->n[2]*adrve1->n[2];
		if(len<0) adrvl->f=1;
		len=adrvl->n[0]*adrve2->n[0]+adrvl->n[1]*adrve2->n[1]+adrvl->n[2]*adrve2->n[2];
		if(len<0) adrvl->f+=2;
		len=adrvl->n[0]*adrve3->n[0]+adrvl->n[1]*adrve3->n[1]+adrvl->n[2]*adrve3->n[2];
		if(len<0) adrvl->f+=4;
		adrvl++;
	}

	freeN(adrno);
	freeN(adrco);
	if(pumanorm) {
		freeN(adrpu);
		freeN(z1);
	}
}

void sculptlezer(di,fi)
char *di,*fi;
{
	struct VV *vv;
	struct Base *base,*addbase();
	struct ObData *ob;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct ColBlck *col;
	float fac;
	long file,*temp,*t,*tt,x,y,z,vec[3];
	long var,vlak=0,tot,len,min[3],max[3],rt[20];
	short a,c,colt=0,okee();
	char s[100],*b1;

	/* pas op: nog niet geschikt voor meer dan 32767 vlakken
		   en punten, ook de edges routine!	*/

	strcpy(s,di);
	strcat(s,fi);
	file=open(s,O_RDONLY);
	if(file== -1) return;

	read(file,&var,4);
	if(var==FORM){
		read(file,&len,4);
		read(file,&var,4);
		if(var!=SC3D) error("No Sculpt file");
		else {
			strcpy(G.sce,s);
			for(a=0;a<3;a++) {
				min[a]=0x7FFFFFFF;
				max[a]=0x80000000;
			}
			while(read(file,&var,4)) {
				read(file,&len,4);
				if(var==VERT) {
					tot=len/12;
					tt=t=(long *)callocN(len,"sculpt1");
					read(file,tt,len);
					for(a=0;a<tot;a++) {
						for(c=0;c<3;c++) {
							if(min[c]> *tt) min[c]= *tt;
							if(max[c]< *tt) max[c]= *tt;
							tt++;
						}
					}
				}
				else if(var==FACE) {
					vlak=len/16;
					vv=(struct VV *)callocN(32+12*tot+len,"sculptVV");
					vv->vert=tot;
					vv->vlak=vlak;
					vv->us=1;
					adrve=(struct VertOb *)(vv+1);
					adrvl=(struct VlakOb *)(adrve+tot);
					memcpy(adrve->c,t,12*tot);  /* to,from,n */
					freeN(t);
					rt[1]= -1;
					c=1;
					read(file,adrvl,len);
					temp=(long *)adrvl;
					for(a=0;a<vlak;a++) {
						adrvl->v1= (short) *(temp++);
						adrvl->v2= (short) *(temp++);
						adrvl->v3= (short) *(temp++);
						if(adrvl->v1==0) {
							adrvl->v1=adrvl->v2;	/* nog nodig of niet?? */
							adrvl->v2=adrvl->v3;	/* nog nodig of niet?? */
							adrvl->v3=0;
						}
						t= (temp++);
						for(c=1;c<=colt;c++) {
							if(*t==rt[c]) c=colt+1;
						}
						if(c==colt+1) {
							colt++;
							rt[colt]= *t;
						}
						adrvl++;
					}
				}
				else {
					if(lseek(file,len,SEEK_CUR)== -1) break;
				}
			}
			if(vlak==0) error("No faces!");
			else {
				base=addbase(0);
				base->soort=1;
				G.totobj++;
				base->f=12;
				base->f1=42;
				vec[0]=(min[0]+max[0])/2;
				vec[1]=(min[1]+max[1])/2;
				vec[2]=(min[2]+max[2])/2;
				if( okee("Original location?") ) {
					VECCOPY(base->v,vec);
				}
				else {
					VECCOPY(base->v,view0.muis);
				}
				base->lay=1<<(G.layact-1);
				base->sf=1;
				base->len= 80;
				base->q[0]=1.0;
				Mat3One(base->m);
				base->d=(long *)callocN(sizeof(struct ObData)+64*colt,"sculptOb");
				ob=(struct ObData *)base->d;
				ob->vv=vv;
				ob->c=colt;
				ob->dt=1;
				vv->afm[0]= (max[0]-min[0]+1)/2;
				vv->afm[1]= (max[1]-min[1]+1)/2;
				vv->afm[2]= (max[2]-min[2]+1)/2;
				strcpy(s,fi);
				s[14]=0;
				for(a=0;a<14;a++) if(s[a]=='.') s[a]=0;
				strcpy(base->str,s);
				newbasename(base);
				fac= MAX3(vv->afm[0],vv->afm[1],vv->afm[2]);
				fac=32760.0/fac;	/* groter geeft problemen */
				vv->ws=1/fac;
				adrve=(struct VertOb *)(vv+1);
				tt=(long *)adrve;
				for(a=0;a<tot;a++) {
					adrve->c[0]= (short)ffloor(fac*( *(tt++) -vec[0] ));
					adrve->c[1]= (short)ffloor(fac*( *(tt++) -vec[1] ));
					adrve->c[2]= (short)ffloor(fac*( *(tt++) -vec[2] ));
					adrve++;
				}

				adrvl=(struct VlakOb *)adrve;
				for(a=0;a<vlak;a++) {
					t= (long *)adrvl;
					t+=3;
					for(c=1;c<=colt;c++) {
						if(rt[c]== *t) {
							adrvl->ec= c-1;
							c=colt;
						}
					}
					adrvl++;
				}
				col=(struct ColBlck *)(ob+1);
				b1=(char *)&rt[1];
				for(a=1;a<=colt;a++) {
					col->r= *(b1++);
					col->g= *(b1++);
					col->b= *(b1++);
					if(*(b1++)!=0) col->mode=13;
					else col->mode=12;
					t=(long *)col;
					*(t+1)=0xC8C83200;
					*(t+2)=0xF0F0;
					*(t+3)=0xF0F0F0F0;
					*(t+4)=0xF0000000;
					*(t+8)=0x9000000;
					*(t+9)=0x9000000;
					*(t+10)=0x9000000;
					*(t+11)=0x9000000;
					col++;
				}
				adrve=(struct VertOb *)(vv+1);
				adrvl=(struct VlakOb *)(adrve+tot);

				edges(adrvl,tot,vlak);
				normalen(adrve,adrvl,tot,vlak);
				tekenbase_ext(base);
				countall();
				tekenoverdraw(0);
			}
		}
	}
	close(file);
}



long getlong(adr)
unsigned short *adr;
{
	return ((*adr)<<16)+*(adr+1);
}


test_li_re_oud(gat,liboy,adr,aantal)
short liboy,*adr,aantal;
{
	double pconst=0;
	float x,x1,x2,y,y1,y2;
	short t1,t2,drielijn=1,b2,b4,achtervl,*adr1,*adr2;

	b2=liboy-1;
	if(b2<0) b2=aantal-1;
	while(pconst<1.0 && pconst>-1.0) {
		b4= (liboy+drielijn) % aantal;
		x1= GL(adr+12*b2);
		x2= GL(adr+12*liboy);
		x=  GL(adr+12*b4);
		y1= GL(adr+12*b2+2);
		y2= GL(adr+12*liboy+2);
		y=  GL(adr+12*b4+2);
		pconst=(x2-x1)*(y1-y)+(y1-y2)*(x1-x);
		drielijn++;
		if(drielijn>aantal) break;
	}

	if(pconst<0) achtervl=0;
	else achtervl=1;
	if( (gat==1 && achtervl==0) || (gat!=1 && achtervl==1) ) {
		adr1= adr;
		adr2= adr+12*(aantal-1);
		while(adr1<adr2) {
			t1= adr1[0];
			t2= adr1[1];
			adr1[0]=adr2[0];
			adr1[1]=adr2[1];
			adr2[0]=t1;
			adr2[1]=t2;
			t1= adr1[2];
			t2= adr1[3];
			adr1[2]=adr2[2];
			adr1[3]=adr2[3];
			adr2[2]=t1;
			adr2[3]=t2;
			adr1+=12;
			adr2-=12;
		}

	}
}

test_li_re(gat,liboy,adr,aantal)
short liboy,aantal;
long *adr;
{
	double pconst=0;
	float x,x1,x2,y,y1,y2;
	short drielijn=1,b2,b4,achtervl;
	long *adr1,*adr2,t1,t2;

	b2=liboy-1;
	if (b2<0) b2=aantal-1;
	while(pconst<1.0 && pconst>-1.0) {
		b4 = (liboy+drielijn) % aantal;
		x1 = adr[2*b2];
		y1 = adr[2*b2+1];
		x2 = adr[2*liboy];
		y2 = adr[2*liboy+1];
		if (x1==x2 && y1==y2) printf("begin en eindpunt gelijk !\n");
		x  = adr[2*b4];
		y  = adr[2*b4+1];
		pconst=(x2-x1)*(y1-y)+(y1-y2)*(x1-x);
		drielijn++;
		if(drielijn>aantal) break;
	}

	if(pconst<0) achtervl=0;
	else achtervl=1;
	if( (gat==1 && achtervl==0) || (gat!=1 && achtervl==1) ) {
		adr1 = adr;
		adr2 = adr+2*(aantal-1);
		while(adr1<adr2) {
			t1= adr1[0];
			t2= adr1[1];
			adr1[0]=adr2[0];
			adr1[1]=adr2[1];
			adr2[0]=t1;
			adr2[1]=t2;
			adr1+=2;
			adr2-=2;
		}
	}
}

short verwijderdubbels(adr,aantal)
short *adr,aantal;
{			/* niet compleet: 2xzoveel kopieren! */
	short a,b;

	if(adr[0]==adr[12*aantal-12])
		if(adr[1]==adr[12*aantal-10]) aantal--;

	for(a=0;a<aantal-1;a++) {
		if(adr[0]== adr[12])
			if(adr[1]== adr[13]) {
				for(b=a;b<aantal;b++) {
					adr[0]= adr[12];
					adr[1]= adr[13];
				}
				aantal--;
			}
		adr+=12;
	}
	return aantal;
}

short *findiff(start,eind,code)
short *start,*eind;
long code;
{
	ulong len;
	/* zoekt naar chunk met naam=code. Daaropvolgend staat de lengte van die chunk */

	if (start < eind){
		while (GL(start) != code){
			start += (GL(start+2) + 8) >> 1;
			if (start >= eind) break;
		}
		if (start<eind) return (start);
	}
	return (0);
}

void makeclippoints(bez,points,off,num)
long *bez,*points,off,num;
{
	double x1,x2,x3,x4,y2,y3,y4;

	x1 = bez[0]+off;
	x2 = bez[4]+off;
	x3 = bez[8]+off;
	x4 = bez[6]+off;

	y2 = 3*(x2-x1)/num;
	y3 = 3*(x1-2*x2+x3)/(num*num);
	y4 = (x4-x1+3*(x2-x3))/(num*num*num);

	x2 = y2+y3+y4;
	x3 = 2*y3+6*y4;
	x4 = 6*y4;

	for(;num >0; num--){
		*(points) = x1;
		points += 2;
		x1 += x2;
		x2 += x3;
		x3 += x4;
	}
}


void makeclipbez(d,pp)
short *d;
long *pp;
{
	long parts,*bez,*bezstart,*new,*points,*pstart,max = 32,flag,xoff,yoff,i,col;
	double x1,x2,y1,y2,buig,m;

	pp[2]=pp[3]=0;

	d += 4;
	parts = *(d++)-1;

	bez = (long *) mallocN((parts+1)*6*sizeof(long),"bezcopy");
	if (bez==0) return;
	memcpy(bez,d,(parts+1)*6*sizeof(long));
	bezstart = bez;

	new = (long *) mallocN((max+1)*parts*2*sizeof(long),"newbezp");
	if (new==0){
		freeN(bez);
		return;
	}

	pstart = (long *) mallocN(max*2*sizeof(long),"tempbezp");
	if (pstart==0){
		freeN(bez);
		freeN(new);
		return;
	}

	pp[2] = (long)new;
	xoff = pp[0];
	yoff = pp[1];

	for(;parts>0;parts--){
		*(new++) = bez[0]+xoff;
		*(new++) = bez[1]+yoff;

		/* handles controleren ivm dubbele punten */
		x1=bez[2]-bez[0];
		x2=bez[4]-bez[0];
		y1=bez[3]-bez[1];
		y2=bez[5]-bez[1];

		m=sqrt((x1*x1+y1*y1)*(x2*x2+y2*y2));

		/* if(  m != 0.0 && (x1*x2+y1*y2)/m>-0.99 ) { */

		if(  m == 0 || (x1*x2+y1*y2)/m>-0.99 ) {
			/* liggen niet in elkaars verlengde */
			*(new++) = bez[0]+xoff;
			*(new++) = bez[1]+yoff;
		}
		flag = 1;

		if(bez[0]==bez[4] & bez[6]==bez[8]) {
			if (bez[1]==bez[5] & bez[7]==bez[9]) {
				flag=0;		/* geen handles */
			}
		}

		/* x4-x1+3*(x2-x3) */
		if(abs(bez[4]-bez[0]-bez[6]+bez[8]) <= 3) {
			if(abs(bez[5]-bez[1]-bez[7]+bez[9]) <= 3) {
				flag=0;		/* rechte lijn dus ook eindpunt invoegen */
			}
		}
		if(flag) {
			m=20000 ;
			buig=0;

			x1 = bez[0]-2*bez[4]+bez[8];
			x2 = (bez[6]-bez[0]+3*(bez[4]-bez[8]))/max;
			y1 = bez[1]-2*bez[5]+bez[9];
			y2 = (bez[7]-bez[1]+3*(bez[5]-bez[9]))/max;

			for (i = max-1;i>0;i--){
				x1 += x2;
				y1 += y2;

				buig += fabs(x1)+fabs(y1);
			}

			if (buig > m){
				x1 = bez[0]-2*bez[4]+bez[8];
				y1 = bez[1]-2*bez[5]+bez[9];
				m = buig/(floor(buig/m)+1)+0.01;
				buig = 0;

				points = pstart;
				makeclippoints(bez,points,xoff,max);
				makeclippoints(bez+1,points+1,yoff,max);
				points += 2;

				for (i = max-1;i>0;i--){
					x1 += x2;
					y1 += y2;

					buig += fabs(x1)+fabs(y1);
					if (buig > m){
						buig -= m;
						*(new++) = points[0];
						*(new++) = points[1];
					}
					points += 2;
				}
			}
		}
		bez += 6;
	}

	pp[3] = (new - (long *)pp[2]) >> 1;
	freeN(pstart);
	freeN(bezstart);
}

void pdrawlezer(di,fi)
char *di,*fi;
{
	struct VV *vv;
	struct Base *base,*addbase();
	struct ObData *ob;
	struct PolyData *po;
	struct ColBlck *col;
	float fac;
	long x,y,x2,y2,file,filelen,var,len,min[3],max[3],pp[250][4],*t;
	short gat,a,vorm=0,okee(),*data,*p,*d,*eind,tot=0,v;
	short liboy;
	char s[100];

	strcpy(s,di);
	strcat(s,fi);
	file=open(s,O_RDONLY);
	if(file== -1) {
		error("Can't read file");
		return;
	}

	read(file,&var,4);
	if(var!=PDPF && var!=PDFF) {
		error("No PDraw file!");
		close(file);
		return;
	}
	min[0]=min[1]=0x20202020;
	max[0]=max[1]= -0x20202020;
	filelen=lseek(file,0,2);	/* seek end */
	lseek(file,0,0);		/* en weer terug */
	d=data=(short *)mallocN(filelen,"Pdrawfile");
	eind=data+filelen/2;
	read(file,data,filelen);
	close(file);
	d+=4;
	while(d< eind) {
		if ((d=findiff(d,eind,OBJ))==0) break;

		if(GL(d+4)==BEZ4) {
			if ((d=findiff(d,eind,DBOX))==0) break;
			pp[vorm][0]= GL(d+4); /* DBOX */
			pp[vorm][1]= GL(d+6);

			if ((d=findiff(d,eind,FILL))==0) break;
			gat = d[4];

			if ((d=findiff(d,eind,DATA))==0){
				vorm=0;
				break;
			}

			if(d[4]>1) {		/* er zijn ook bezierstukken met maar een controle punt in omloop */
				/*			if(GL(d+2)>=24) {
		*/
				makeclipbez(d,pp[vorm]);
				liboy= -1;
				x2=y2=0x20202020;
				t = (long *)pp[vorm][2];
				for(a=0;a<pp[vorm][3];a++) {
					x = *(t++);
					y = *(t++);
					min[0]=MIN2(min[0],x);
					min[1]=MIN2(min[1],y);
					max[0]=MAX2(max[0],x);
					max[1]=MAX2(max[1],y);
					/* het bovenste meest linkse punt */
					if(y<=y2) {
						if( (y==y2 && x<x2) || y<y2 ) {
							x2=x;
							y2=y;
							liboy=a;
						}
					}
				}
				if(liboy!= -1) test_li_re(gat,liboy,pp[vorm][2],pp[vorm][3]);
				/* pp[vorm][3]=verwijderdubbels(pp[vorm][2],pp[vorm][3]); */
				vorm++;
			}
		} else{
			d += (GL(d+2)+8) >> 1;
		}
	}
	for(a=0;a<vorm;a++) tot+=pp[a][3];
	if(vorm>0) {
		base=addbase(0);
		base->soort=7;
		G.totobj++;
		base->f=12;
		base->f1=42;
		x=(min[0]+max[0])/2;
		y=(min[1]+max[1])/2;
		VECCOPY(base->v,view0.muis);
		base->lay=1<<(G.layact-1);
		base->sf=1;
		base->len=80;
		base->q[0]=1.0;
		Mat3One(base->m);
		Mat3One(base->imat);
		ob=(struct ObData *)callocN(sizeof(struct ObData)+3*sizeof(struct ColBlck));
		base->d=(long *)ob;
		ob->c=3;
		ob->dt=1;
		strcpy(s,fi);
		s[14]=0;
		for(a=0;a<14;a++) if(s[a]=='.') s[a]=0;
		strcpy(base->str,s);
		newbasename(base);
		po=(struct PolyData *)callocN(sizeof(struct PolyData)+6*tot+2*vorm);
		ob->po=po;
		po->vert=tot;
		po->poly=vorm;
		po->us=1;
		po->depth= 160;
		po->f1= 31;
		po->afm[0]=(max[0]-min[0]+1)/2;
		po->afm[1]=(max[1]-min[1]+1)/2;
		fac= MAX2(po->afm[0],po->afm[1]);
		fac= 32760/fac;
		po->ws= 1.0/fac;
		po->n[2]= -32767;
		p= (short *)(po+1);
		for(v=0;v<vorm;v++) {
			*p++ = pp[v][3];
			t=(long *)pp[v][2];
			if (t){
				for(a=pp[v][3];a>0;a--) {
					p[0] =  fac*(*(t++)-x);
					p[1] = -fac*(*(t++)-y);
					p += 3;
				}
				freeN(pp[v][2]);
			}
		}
		t=(long *)(ob+1);
		*t= 0xF0F0F00C;
		*(t+1)=0xC8C83200;
		*(t+2)=0xF0F0;
		*(t+3)=0xF0F0F0F0;
		*(t+4)=0xF0000000;
		*(t+8)=0x41000000;
		*(t+9)=0x41000000;
		*(t+10)=0x41000000;
		*(t+11)=0x41000000;
		memcpy(t+16,t,64);
		memcpy(t+32,t,64);
		*(t+16)= 0xF0F0F00B;
		*(t+32)= 0xF0F0F00B;
		tekenbase_ext(base);
		countall();
		tekenoverdraw(0);
	}
	freeN(data);
}

void pdrawlezer_oud(di,fi)
char *di,*fi;
{
	struct VV *vv;
	struct Base *base,*addbase();
	struct ObData *ob;
	struct PolyData *po;
	struct ColBlck *col;
	float fac;
	long x,y,x2,y2,file,filelen,var,len,min[3],max[3],pp[250][4],*t;
	short gat,a,vorm=0,okee(),*data,*p,*d,*eind,tot=0,v;
	short liboy;
	char s[100];

	strcpy(s,di);
	strcat(s,fi);
	file=open(s,O_RDONLY);
	if(file== -1) {
		error("Can't read file");
		return;
	}

	read(file,&var,4);
	if(var!=PDPF && var==PDFF) {
		error("No PDraw file!");
		close(file);
		return;
	}
	min[0]=min[1]=20202020;
	max[0]=max[1]= -20202020;
	filelen=lseek(file,0,2);	/* seek end */
	lseek(file,0,0);		/* en weer terug */
	d=data=(short *)mallocN(filelen,"Pdrawfile");
	eind=data+filelen/2;
	read(file,data,filelen);
	close(file);
	d++;
	printf("oud\n");
	while(d< eind) {
		d++;
		if(*d==OB) {
			d++;
			a= (*d)>>8;
			if(a=='J' && *(d+3)==BE && *(d+4)==Z4) {
				vorm++;
				d+=(9+12);
				pp[vorm][0]= GL(d); /* VBOX */
				pp[vorm][1]= GL(d+2);
				d+=4;
				while(*d!=TA && d<eind) d++;
				if(d>=eind) {
					vorm=0;
					break;
				}
				d++;
				if(*(d+1)<24) {
					vorm--;
				} else {
					d+=2;
					pp[vorm][3]= (*d)-1; /*aantal als closed curve*/
					gat= *(d-6);
					d++;
					pp[vorm][2]=(long)d;
					liboy= -1;
					x2=20202020;
					y2=20202020;
					for(a=0;a<pp[vorm][3];a++) {
						x=GL(d);
						y=GL(d+2);
						d+=12;
						min[0]=MIN2(min[0],x+pp[vorm][0]);
						min[1]=MIN2(min[1],y+pp[vorm][1]);
						max[0]=MAX2(max[0],x+pp[vorm][0]);
						max[1]=MAX2(max[1],y+pp[vorm][1]);

						/* het bovenste meest linkse punt */
						if(y<=y2) {
							if( (y==y2 && x<x2) || y<y2 ) {
								x2=x;
								y2=y;
								liboy=a;
							}
						}
					}
					if(liboy!= -1) test_li_re_oud(gat,liboy,pp[vorm][2],pp[vorm][3]);
					/* pp[vorm][3]=verwijderdubbels(pp[vorm][2],pp[vorm][3]); */
				}
			}
		}
	}
	for(a=1;a<=vorm;a++) tot+=pp[a][3];
	if(vorm>0) {
		base=addbase(0);
		base->soort=7;
		G.totobj++;
		base->f=12;
		base->f1=42;
		x=(min[0]+max[0])/2;
		y=(min[1]+max[1])/2;
		VECCOPY(base->v,view0.muis);
		base->lay=1<<(G.layact-1);
		base->sf=1;
		base->q[0]=1.0;
		Mat3One(base->m);
		Mat3One(base->imat);
		ob=(struct ObData *)callocN(sizeof(struct ObData)+3*sizeof(struct ColBlck));
		base->d=(long *)ob;
		ob->c=3;
		ob->dt=1;
		strcpy(s,fi);
		s[14]=0;
		for(a=0;a<14;a++) if(s[a]=='.') s[a]=0;
		strcpy(base->str,s);
		newbasename(base);
		po=(struct PolyData *)callocN(sizeof(struct PolyData)+6*tot+2*vorm);
		ob->po=po;
		po->vert=tot;
		po->poly=vorm;
		po->us=1;
		po->afm[0]=(max[0]-min[0]+1)/2;
		po->afm[1]=(max[1]-min[1]+1)/2;
		fac= MAX2(po->afm[0],po->afm[1]);
		fac= 32760/fac;
		po->ws= 1.0/fac;
		po->depth= 160;
		po->n[2]= -32767;
		p= (short *)(po+1);
		for(v=1;v<=vorm;v++) {
			*p++ = pp[v][3];
			d=(short *)pp[v][2];
			for(a=0;a<pp[v][3];a++) {
				p[0]= fac*(GL(d)+pp[v][0]-x);
				p[1]= -fac*(GL(d+2)+pp[v][1]-y);
				p+=3;
				d+=12;
			}
		}
		t=(long *)(ob+1);
		*t= 0xF0F0F00A;
		*(t+1)=0xC8C83200;
		*(t+2)=0xF0F0;
		*(t+3)=0xF0F0F0F0;
		*(t+4)=0xF0000000;
		*(t+8)=0x41000000;
		*(t+9)=0x41000000;
		*(t+10)=0x41000000;
		*(t+11)=0x41000000;
		memcpy(t+16,t,64);
		memcpy(t+32,t,64);
		*(t+16)= 0xF0F0F00B;
		*(t+32)= 0xF0F0F00B;
		tekenbase_ext(base);
	}
	freeN(data);
}

void videoscapelezer(str)
char *str;
{
	struct Nurb *nu;
	struct BPoint *bp;
	struct Base *base, *addbase();
	struct ObData *ob;
	struct CurveData *cu;
	struct ColBlck *col;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	FILE *fp;
	float *vertdata, *vd, min[3], max[3], afm[3], fac;
	long vec[3], end, a, b, verts, poly, nr, nr0, first, vlak, totedge=0, tottria=0, totquad=0;
	short cox, coy, event;
	char s[50];


	fp= fopen(str, "r");
	if(fp==NULL) {
		error("Can't read file");
		return;
	}

	sginap(10);	/* anders wordt queue leeggegooid, sginap(2) is te kort! */
	if(qtest());

	event= pupmenu("Load Videoscape as%t|Curve%x1|TriFace%x2");

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
	vd= vertdata= mallocN(4*3*verts, "videoscapelezer");
	min[0]= min[1]= min[2]= 1.0e20;
	max[0]= max[1]= max[2]= -1.0e20;
	for(a=0;a<verts;a++) {
		fscanf(fp, "%f %f %f", vd, vd+1, vd+2);
		for(b=0; b<3; b++) {
			vd[b]*= 5000.0;
			min[b]= MIN2(min[b], vd[b]);
			max[b]= MAX2(max[b], vd[b]);
		}
		vd+=3;
	}
	/* de vlakken */
	end= 1;
	while(end>0) {
		end= fscanf(fp,"%d", &poly);
		if(end<=0) break;

		if(poly==3) tottria++;
		if(poly==4) totquad++;
		else totedge+= poly;

		for(a=0;a<poly;a++) {
			end= fscanf(fp,"%d", &nr);
			if(end<=0) break;
		}
		if(end<=0) break;
		end= fscanf(fp,"%i\n", &poly);
	}
	vec[0]= (min[0]+max[0])/2;
	vec[1]= (min[1]+max[1])/2;
	vec[2]= (min[2]+max[2])/2;

if(totedge+tottria+2*totquad>65000) {
	error("too many faces");
	freeN(vertdata);
	fclose(fp);
	return;
}

	rewind(fp);

	fscanf(fp, "%40s", s);
	fscanf(fp, "%d\n", &verts);
	for(a=0;a<verts;a++) {
		fscanf(fp, "%f %f %f", s, s+4, s+8);
	}

	base= addbase(0);

	base->f= 12;
	base->f1= 42;
	VECCOPY(base->v,view0.muis);
	base->lay= 1<<(G.layact-1);
	base->sf=1;
	base->len= 80;
	base->q[0]=1.0;
	Mat3One(base->m);

	splitdirstring(str, s);
	s[14]= 0;
	for(a=0;a<14;a++) if(s[a]=='.') s[a] =0;
	strcpy(base->str,s);
	newbasename(base);

	if(event==1) {  /* curve */
		base->soort= 11;
		G.totpoly++;

		base->d= callocN(sizeof(struct ObData)+2*sizeof(struct ColBlck),"videoscape");
		ob= (struct ObData *)base->d;
		col= (struct ColBlck *)(ob+1);
		col->r= col->g= col->b= 240;
		initcolblocks(col, 2);
		cu= callocN(sizeof(struct CurveData), "videoscape");
		ob->cu= cu;
		cu->width= 100;

		ob->c= 1;
		ob->dt= 1;

		afm[0]= (max[0]-min[0]+1)/2;
		afm[1]= (max[1]-min[1]+1)/2;
		afm[2]= (max[2]-min[2]+1)/2;
		cox= 0;
		coy= 1;
		if(afm[0]<afm[1] && afm[0]<afm[2]) {
			cox= 1;
			coy= 2;
		}
		else if(afm[1]<afm[2] && afm[1]<afm[0]) {
			cox= 0;
			coy= 2;
		}

		fac= 1.0+MAX2(afm[cox], afm[coy]);
		fac= 32760.0/fac;	/* groter geeft problemen */
		cu->ws= 1.0/fac;
		cu->flag= 3;

		end= 1;
		while(end>0) {
			end= fscanf(fp,"%d", &poly);
			if(end<=0) break;

			nu= callocstructN(struct Nurb, 1, "videoscape2");
			nu->type= 8;
			nu->pntsu= poly;
			nu->resolu= 12;
			nu->flagu= 1;
			nu->pntsv= 1;
			addtail(&(cu->curve), nu);
			bp= nu->bp= callocstructN(struct BPoint, poly, "videoscape3");

			for(a=0;a<poly;a++) {
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				bp->vec[0]= fac*(vertdata[3*nr+cox]-vec[cox]);
				bp->vec[1]= fac*(vertdata[3*nr+coy]-vec[coy]);
				bp++;
			}

			if(end<=0) break;
			end= fscanf(fp,"%i", &poly);
		}

		makeBevelList(base);
		makeDispList(base);
	}
	else {	    /* TriFace */
		base->soort= 1;
		G.totobj++;

		vlak= tottria+totedge+2*totquad;
		base->d= callocN(sizeof(struct ObData)+sizeof(struct ColBlck),"videoscape");
		ob= (struct ObData *)base->d;
		vv= callocN(sizeof(struct VV)+ verts*sizeof(struct VertOb)+ 
		    vlak*sizeof(struct VlakOb));
		col= (struct ColBlck *)(ob+1);
		col->r= col->g= col->b= 240;
		col->mode= 12;
		initcolblocks(col, 1);

		ob->vv= vv;
		ob->c= 1;
		ob->dt= 1;
		vv->vert= verts;
		vv->vlak= vlak;
		vv->us= 1;
		vv->afm[0]= afm[0]= (max[0]-min[0])/2;
		vv->afm[1]= afm[1]= (max[1]-min[1])/2;
		vv->afm[2]= afm[2]= (max[2]-min[2])/2;

		fac= 1.0+MAX3(afm[0],afm[1],afm[2]);

		fac= 32760.0/fac;	/* groter geeft problemen */
		vv->ws= 1.0/fac;
		adrve= (struct VertOb *)(vv+1);
		vd= vertdata;

		for(a=0; a<verts; a++) {
			adrve->c[0]= (short)ffloor(fac*( vd[0] -vec[0] ));
			adrve->c[1]= (short)ffloor(fac*( vd[1] -vec[1] ));
			adrve->c[2]= (short)ffloor(fac*( vd[2] -vec[2] ));
			vd+= 3;
			adrve++;
		}

		adrvl=(struct VlakOb *)adrve;

		for(a=0;a<vlak;a++) {
			end= fscanf(fp,"%d", &poly);
			if(end<=0) break;

			if(poly==3) {
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v1= nr;
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v2= nr;
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v3= nr;
				adrvl++;
			}
			else if(poly==4) {
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v1= nr;
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v2= nr;
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v3= nr;
				adrvl++;
				adrvl->v1= (adrvl-1)->v1;
				adrvl->v2= (adrvl-1)->v3;
				end= fscanf(fp,"%d", &nr);
				if(end<=0) break;
				adrvl->v3= nr;
				adrvl++;
			}
			else {
				end= fscanf(fp,"%d", &nr0);
				if(end<=0) break;
				first= nr0;
				for(b=1; b<poly; b++) {
					end= fscanf(fp,"%d", &nr);
					if(end<=0) break;
					adrvl->v1= adrvl->v2= nr;	/* v1==v2 wordt door initrender afgevangen */
					adrvl->v3= nr0;
					nr0= nr;
					adrvl++;
					a++;
				}
				adrvl->v1= adrvl->v2= first;	/* v1==v2 wordt door initrender afgevangen */
				adrvl->v3= nr;
				adrvl++;
				if(end<=0) break;
			}
			end= fscanf(fp,"%i", &poly);
			if(end<=0) break;

		}
		adrve=(struct VertOb *)(vv+1);
		adrvl=(struct VlakOb *)(adrve+verts);

		edges(adrvl,verts,vlak);
		normalen(adrve,adrvl,verts,vlak);
	}

	projektie();

	freeN(vertdata);
	fclose(fp);
}


void displist_to_base(struct DispList *dlfirst, char *name)
{
	struct DispList *dl;
	struct Base *base, *addbase();
	struct ObData *ob;
	struct CurveData *cu;
	struct ColBlck *col;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	float *vertdata, *data, *vd, vec[3], min[3], max[3], afm[3], fac;
	long end, a, b, startve, totedge=0, tottria=0, totvert=0, totvlak, totcol=0, colnr;
	long p1, p2, p3, p4;
	char str[100];
	
	/* eerst tellen */
	min[0]= min[1]= min[2]= 1.0e20;
	max[0]= max[1]= max[2]= -1.0e20;

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
			tottria+= 2*a*b;

			totvert+= dl->nr*dl->parts;

			data= (float *)(dl+1);
			for(a= dl->nr*dl->parts; a>0; a--) {
				for(b=0; b<3; b++) {
				
					data[b]*= 100; /* ivm integerruimte */
					
					min[b]= MIN2(min[b], data[b]);
					max[b]= MAX2(max[b], data[b]);
				}
				data+= 3;
			}
		}
		else if(dl->type==DL_POLY) {
			if(dl->nr==3 || dl->nr==4) {
				if(dl->nr==3) tottria+= dl->parts;
				else tottria+= 2*dl->parts;
				totvert+= dl->nr*dl->parts;

				data= (float *)(dl+1);
				for(a= dl->nr*dl->parts; a>0; a--) {
					for(b=0; b<3; b++) {
				
						data[b]*= 100; /* ivm integerruimte */
					
						min[b]= MIN2(min[b], data[b]);
						max[b]= MAX2(max[b], data[b]);
					}
					data+= 3;
				}
			}
			else if(dl->nr>4) {
				data= (float *)(dl+1);
				
				tottria+= dl->nr*dl->parts;
				totvert+= dl->nr*dl->parts;
				
				for(a= dl->nr*dl->parts; a>0; a--) {
					for(b=0; b<3; b++) {
				
						data[b]*= 100; /* ivm integerruimte */
					
						min[b]= MIN2(min[b], data[b]);
						max[b]= MAX2(max[b], data[b]);
					}
					data+= 3;
				}
				
			}
		}
		else if(dl->type==DL_SEGM) {
			data= (float *)(dl+1);
			
			tottria+= (dl->nr-1)*dl->parts;
			totvert+= dl->nr*dl->parts;
			
			for(a= dl->nr*dl->parts; a>0; a--) {
				for(b=0; b<3; b++) {
				
					data[b]*= 100; /* ivm integerruimte */
					
					min[b]= MIN2(min[b], data[b]);
					max[b]= MAX2(max[b], data[b]);
				}
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
	
	totcol++;	/* nul doet mee */
	if(totcol>16) {
		error("Found more than 16 colors");
		totcol= 16;
	}
	else if(totcol<1) totcol= 1;

	afm[0]= (max[0]-min[0])/2;
	afm[1]= (max[1]-min[1])/2;
	afm[2]= (max[2]-min[2])/2;
	vec[0]= (min[0]+max[0])/2;
	vec[1]= (min[1]+max[1])/2;
	vec[2]= (min[2]+max[2])/2;

	base= addbase(0);

	base->f= 9;
	base->f1= 2+8+16+64;	/* was 42: 2+8+32 */
	VECCOPY(base->v, vec);
	base->lay= 1<<(G.layact-1);
	base->sf=1;
	base->len= 80;
	base->q[0]=1.0;
	Mat3One(base->m);

	strcpy(base->str, name);
	newbasename(base);

	if(base) {	/* hier nog Nurbs splitsen van Trifaces */
		base->soort= 1;
		G.totobj++;

		totvlak= tottria+totedge;
		base->d= callocN(sizeof(struct ObData)+totcol*sizeof(struct ColBlck),"autocad");
		ob= (struct ObData *)base->d;
		vv= callocN(sizeof(struct VV)+ totvert*sizeof(struct VertOb)+ 
		    totvlak*sizeof(struct VlakOb));
		
		col= (struct ColBlck *)(ob+1);
		for(a=0; a<totcol; a++, col++) {
			col->r= col->g= col->b= 240;
			col->mode= 12;
			initcolblocks(col, 1);
		}
		
		ob->vv= vv;
		ob->c= totcol;
		ob->dt= 1;
		vv->vert= totvert;
		vv->vlak= totvlak;
		vv->us= 1;
		vv->afm[0]= (max[0]-min[0]+1)/2;
		vv->afm[1]= (max[1]-min[1]+1)/2;
		vv->afm[2]= (max[2]-min[2]+1)/2;

		fac= 1.0+MAX3(afm[0],afm[1],afm[2]);
		fac= 32760.0/fac;	/* groter geeft problemen */
		vv->ws= 1.0/fac;
		adrve= (struct VertOb *)(vv+1);
		adrvl= (struct VlakOb *)(adrve+totvert);
		startve= 0;

		dl= dlfirst;
		while(dl) {
			
			colnr= (dl->col>15 ? 15: dl->col);

			if(dl->type==DL_SURF) {
				data= (float *)(dl+1);

				for(a=dl->parts*dl->nr; a>0; a--) {
					adrve->c[0]= (short)ffloor(fac*( data[0] -vec[0] ));
					adrve->c[1]= (short)ffloor(fac*( data[1] -vec[1] ));
					adrve->c[2]= (short)ffloor(fac*( data[2] -vec[2] ));
					data+=3;
					adrve++;
				}

				for(a=0; a<dl->parts; a++) {
	
					DL_SURFINDEX(dl->flag & 1, dl->flag & 2, dl->nr, dl->parts);
					p1+= startve; 
					p2+= startve; 
					p3+= startve; 
					p4+= startve;
	
					for(; b<dl->nr; b++) {
					
						adrvl->v1= p2;
						adrvl->v2= p3;
						adrvl->v3= p4;
						adrvl->ec= colnr;
						adrvl++;
						adrvl->v1= p2;
						adrvl->v2= p1;
						adrvl->v3= p3;
						adrvl->ec= colnr;
						adrvl++;
							
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
						adrve->c[0]= (short)ffloor(fac*( data[0] -vec[0] ));
						adrve->c[1]= (short)ffloor(fac*( data[1] -vec[1] ));
						adrve->c[2]= (short)ffloor(fac*( data[2] -vec[2] ));
						data+=3;
						adrve++;
					}

					for(a=0; a<dl->parts; a++) {
						adrvl->v1= startve+a*dl->nr;
						adrvl->v2= startve+a*dl->nr+1;
						adrvl->v3= startve+a*dl->nr+2;
						adrvl->ec= colnr;
						adrvl++;
						if(dl->nr==4) {
							adrvl->v1= startve+a*dl->nr;
							adrvl->v2= startve+a*dl->nr+2;
							adrvl->v3= startve+a*dl->nr+3;
							adrvl->ec= colnr;
							adrvl++;
						}
					}
					startve += dl->parts*dl->nr;
				}
				else if(dl->nr>4) {
					data= (float *)(dl+1);

					for(a=dl->parts*dl->nr; a>0; a--) {
						adrve->c[0]= (short)ffloor(fac*( data[0] -vec[0] ));
						adrve->c[1]= (short)ffloor(fac*( data[1] -vec[1] ));
						adrve->c[2]= (short)ffloor(fac*( data[2] -vec[2] ));
						data+=3;
						adrve++;
					}

					for(b=0; b<dl->parts; b++) {
						for(a=0; a<dl->nr; a++) {
							adrvl->v1= startve+a;
							adrvl->v2= startve+a;
							if(a==dl->nr-1) adrvl->v3= startve;
							else adrvl->v3= startve+a+1;
							adrvl->ec= colnr;
							adrvl++;
						}
						startve += dl->nr;
					}
				}
			}
			else if(dl->type==DL_SEGM) {
				data= (float *)(dl+1);

				for(a=dl->parts*dl->nr; a>0; a--) {
					adrve->c[0]= (short)ffloor(fac*( data[0] -vec[0] ));
					adrve->c[1]= (short)ffloor(fac*( data[1] -vec[1] ));
					adrve->c[2]= (short)ffloor(fac*( data[2] -vec[2] ));
					data+=3;
					adrve++;
				}

				for(b=0; b<dl->parts; b++) {
					for(a=0; a<dl->nr-1; a++) {
						adrvl->v1= startve+a;
						adrvl->v2= startve+a;
						adrvl->v3= startve+a+1;
						adrvl->ec= colnr;
						adrvl++;
					}
					startve += dl->nr;
				}
			}
			dl= dl->next;
		}
		adrve=(struct VertOb *)(vv+1);
		adrvl=(struct VlakOb *)(adrve+totvert);

		edges(adrvl,totvert,totvlak);
		normalen(adrve,adrvl,totvert,totvlak);
	}
}

void exoticlezer(char *str, short type)
{
	struct ListBase lbase, tempbase;
	struct DispList *dl, *prev, *first, *next, *dltemp;
	int a, totvert=0, vert;
	short maxaantal, curcol;
	char name[50], str1[50];
	
	
	lbase.first= lbase.last= 0;

	if (type == 0) printf("Iges NOT supported\n"); /*leesIGES(str, &lbase);*/
	else if(type==1) printf("Cyber NOT supported\n"); /*leesCYBER(str, 4, 4, &lbase);*/
	else if(type==2) leesDXF(str, &lbase);
	else if(type==3) leesInventor(str, &lbase);

	dl= lbase.first;
	while(dl) {
		next= dl->next;
		
		/* PATCH 1: polyfill */
		if(dl->type==DL_POLY && dl->nr>4) {
						
			filldisplist(dl, &dltemp);
			if(dltemp) {
				remlink(&lbase, dl);
				freeN(dl);
				addhead(&lbase, dltemp);
			}
		}
		/* PATCH 2: poly's van 2 punten */
		if(dl->type==DL_POLY && dl->nr==2) dl->type= DL_SEGM;
		
		dl= next;
	}

	/* alles tellen */

	dl= lbase.first;
	while(dl) {

		if(dl->type==DL_SURF) totvert+= dl->nr*dl->parts;
		else if(dl->type==DL_POLY) {
			if(dl->nr==3 || dl->nr==4) totvert+= dl->nr*dl->parts;
			else if(dl->nr>4) totvert+= dl->nr*dl->parts;
		}
		else if(dl->type==DL_SEGM) totvert+= dl->nr*dl->parts;

		dl= dl->next;
	}

	if(totvert==0) {
		if(type==0) {
			error("IGES: found no data");
			if(lbase.first) freelist(&lbase);
		}
		else if(type==1) {
			error("CYBER: found no data");
			if(lbase.first) freelist(&lbase);
		}
		else if(type==2) {
			error("DXF: found no data");
			if(lbase.first) freelistN(&lbase);
		}
		else if(type==3) {
			error("Inventor: found no data");
			if(lbase.first) freelistN(&lbase);
		}
		return;
	}

	deselectall_ex(0);

	splitdirstring(str, name);
	name[14]= 0;
	for(a=0; a<14; a++) if(name[a]=='.') name[a] =0;

	sprintf(str1, "Found %d vertices", totvert);
	pupmenu(str1);
	
	maxaantal= 32000;
	
	if(totvert>maxaantal) {
	
		/* probeer kleuren bij elkaar te zetten */
		curcol= 0;
		tempbase.first= tempbase.last= 0;

		while(lbase.first) {
			dl= lbase.first;
			while(dl) {
				next= dl->next;
				if(dl->col==curcol) {
					remlink(&lbase, dl);
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
				else if(dl->type==DL_SEGM) vert= dl->nr*dl->parts;
				
				totvert+= vert;
				if(totvert > maxaantal || dl->next==0) {
					if(dl->next==0) {
						displist_to_base(first, name);
					}
					else if(dl->prev) {
						prev= dl->prev;
						prev->next= 0;
						displist_to_base(first, name);
						prev->next= dl;
						first= dl;
						totvert= 0;
					}
				}
				
				dl= dl->next;
			}
			
			if(type>=2) freelistN(&tempbase);
			else freelist(&tempbase);
			
			curcol++;
		}
		
		
	}
	else displist_to_base(lbase.first, name);

	if(type>=2) freelistN(&lbase);
	else freelist(&lbase);
}



/* ************************* */


long gfa_to_float(f)
short *f;
{
	long d0;
	short d2, t;

	d0= *( (long *)f );
	d2= *(f+3);
	t= d2;

	if(d2<0) d2= -d2;

	d2-= 0x380;
	if(d2<0) return 0;
	else if(d2>0xFF) {
		d0= 0x7F800000;
	}
	else {
		d0= ((d0 & 0x7FFFFFFF)>>8) + (d2<<23);
	}

	if(t<0) d0 |= 0x80000000;

	return d0;
}



void amigabase(base)
struct Base *base;
{
	/* herstructureren */
	double *dp;
	float *fp;
	long temp, a;

	dp= (double *)base->q;
	for(a=0; a<4; a++) {
		temp= gfa_to_float(dp+a);
		base->q[a]= *( (float *)&temp );
	}

	dp= (double *)base;
	dp+= 10;
	fp= base->m[0];
	for(a=0; a<9; a++) {
		temp= gfa_to_float(dp+a);
		fp[a]= *( (float *)&temp );
	}

	if(base->lay>>12) base->lay= 1;

	base->w= 0;
	base->pkey= base->key= 0;
	base->ipopkey= base->ipokey= 0;
}

long *newadr(adr)
long *adr;
{
	static char *cp;
	static long *t00= 0;
	long a, len;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		len= ( ((long)t00) - ((long)ablock) )/8;
		if(len>0 && len<tobl-1) {
			t00+=2;
			cp++;
			if( adr== (long *)*t00 ) {
				adr=(long *)*(t00+1);
				(*cp)= 1;
				return adr;
			}
		}

		t00= ablock;
		cp= ablockflag;
		while(t00) {
			if(*t00==0) break;
			if( adr== (long *)*t00 ) {
				adr=(long *)*(t00+1);

				(*cp)= 1;
				return adr;

			}
			t00+= 2;
			cp++;
		}
		if(*t00==0) adr= 0;
	}
	return adr;
}


lees_viewoud(file,len)
long file,len;
{
	struct View *v;
	long t;

	if(view0.next) {
		v=view0.next;
		while(v) {
			freeN(v);
			v=v->next;
		}
	}
	read(file,&view0,100);
	view0.near= view0.far= 1000;
	if(len>100) {
		t=len/100 -1;
		v= &view0;
		while(t) {
			v=v->next=(struct View *)callocN(sizeof(struct View),"leesview");
			read(file,v,100);
			v->near= v->far= 1000;
			t--;
		}
	}
}

lees_view(file,len)
long file,len;
{
	struct View *v;
	long t, nr;

	if(view0.next) {
		v=view0.next;
		while(v) {
			freeN(v);
			v=v->next;
		}
	}

	read(file, &nr, 4);
	len= (len-4)/nr;
	read(file, &view0,len);
	/* cambase wordt later goed gezet */
	if(nr>1) {
		nr--;
		v= &view0;
		while(nr) {
			v=v->next=(struct View *)callocN(sizeof(struct View),"leesview");
			read(file,v,len);
			nr--;
		}
	}

}


lees_render(file,len)	    /* oude files */
long file,len;
{
	long t;

	read(file,&t,4);
	R.size=(t & 15);
	if(R.size>4) R.size=2;	/* zeer oude files */
	G.filt=BTST(t,5);
	G.schaduw=BTST(t,10);
	G.trace=BTST(t,11);
	G.border=BTST(t,12);
	G.skybuf=BTST(t,13);
	G.ali=BTST(t,14);
	G.script=BTST(t,15);
	G.holo=BTST(t,16);
	G.disc=BTST(t,17);
	/*G.osa=BTST(t,20);*/
	read(file,&G.osa,2);
	read(file,&R.anim,2);

	read(file,&t,4);    /* bcsrgb */
	read(file,&t,4);

	read(file,&G.sfra,4);	 /* 4 is oud foutje */
	read(file,&G.cfra,4);	 /* 4 is oud foutje */
	read(file,&G.efra,4);	 /* 4 is oud foutje */
	read(file,&G.xminb,8);	 /* alle vier */
	read(file,&view0.lay,2);
	read(file,&G.sview,6);	 /* en tracetype, tradep */
	read(file,&view0.opar,8);
	read(file,&G.layact,2);
	read(file,&R.f1,2);	 /*is ook short */
	read(file,&R.xsch,4);
	read(file,&R.xasp,4);
}

lees_global(file)
long file;
{
	extern short editbutflag;
	long t;

	read(file,&t,4);
	R.size=(t & 15);
	G.filt= BTST(t,5);
	G.schaduw= BTST(t,10);
	G.trace= BTST(t,11);
	G.border= BTST(t,12);
	G.skybuf =BTST(t,13);
	G.ali= BTST(t,14);
	G.script= BTST(t,15);
	G.holo= BTST(t,16);
	G.disc= BTST(t,17);
	G.genlock= BTST(t,18);
	if(BTST(t,19)) G.genlock+= 2;
	G.bufscript= BTST(t,20);
	G.zbuf= BTST(t, 21);
	if(BTST(t,22)) G.holo+= 2;
	
	read(file,&G.osa,2);
	read(file,&R.anim,2);

	read(file, &editbutflag, 2);
	read(file, &bezres, 2);
	read(file, &bezmax, 2);
	read(file, &G.holopadlen, 2);
	if(G.holopadlen==0) G.holopadlen= 100;

	if(bezres==0) bezres= 20;
	if(bezmax==0) bezmax= 12;

	read(file,&G.cfra,12);
	read(file,&G.xminb,8);	 /* alle vier */
	read(file,&t,2);	/* hier stond vroeger (voor 1.067) view0.lay */
	read(file,&G.sview,6);	 /* en obszoom, tradep */
	if(G.obszoom<0 || G.obszoom>100) G.obszoom= 0;
	
	read(file,&t,4);	/* hier stond vroeger (voor 1.067) view0.opar */
	read(file,&t,4);	/* hier stond vroeger (voor 1.067) view0.tpar */
	read(file,&G.layact,2);
	read(file,&R.f1,2);	 /*is ook short */
	read(file,&R.xsch,4);
	read(file,&R.xasp,4);

	read(file,&R.winpos,2);
	read(file,&R.planes,2);
	read(file,&R.imtype,2);
	read(file,&R.xparts,4);  /* en yparts */
	read(file,&panoramaview,2);
	read(file,&panodegr,2);
	read(file,&R.edgeint,2);

	read(file,&G.timeipo,4);

	t= 0;

	read(file,&t,4);
	read(file,&t,4);
	read(file,&t,4);
	read(file,&t,4);
}

lees_base(file)
long file;
{
	struct Base *b;

	read(file,t00,4);
	b=(struct Base *)mallocN(256,"leesbase");
	if(b==0) leesfout=1;
	else {
		*(t00+1)=(long)b;
		newadr(*t00);	/* om ablockflag te zetten */
		t00+=2;
		read(file,b,256);
		if(G.lastbase)
			G.lastbase->next=b;
		else G.firstbase=b;
		G.lastbase=b;
		b->next=0;

		if(b->soort & 1) G.totobj++;
		if(b->soort==2) G.totlamp++;
		if(b->soort== -2) G.totmove++;
	}
}

lees_data(file,len)
long file,len;
{
	long *t;

	read(file,t00,4);
	
	t=(long *)mallocN(len-4,"leesdata");
	if(t==0) leesfout=1;
	else {
		*(t00+1)=(long)t;
		t00+=2;
		read(file,t,len-4);
	}
}

lees_vectorfont(file,len)
long file,len;
{
	struct VectorFont *vf,*initvfont();
	char str[100];

	read(file,t00,4);
	read(file,str,len-4);

	str[len-4]=0;	/* voor zekerheid, als strlen is factor van 4! */

	/* bestaat deze font al */
	vf= (struct VectorFont *)G.vfbase.first;
	while(vf) {
		if(strcmp(str,vf->name)==0) break;
		vf= vf->next;
	}
	if(vf==0) { 	/* nieuwe font */
		vf= initvfont(str);
		if(vf==0) vf= (struct VectorFont *)G.vfbase.first;
	}

	*(t00+1)= (long)vf;
	newadr(*t00);	/* om ablockflag te zetten */
	t00+=2;
}

void link_list(ListBase *lb)
{
	Link *ln;
	

	if(lb->first==0) return;

	lb->first= newadr(lb->first);
	lb->last= newadr(lb->last);
	
	ln= lb->first;
	while(ln) {
		
		ln->prev= (Link *)newadr(ln->prev);
		ln->next= (Link *)newadr(ln->next);
		
		ln= ln->next;
	}
}


void lees_nurb(ListBase *lb)
{
	struct Nurb *nu;

	link_list(lb);

	nu= lb->first;
	while(nu) {
		nu->bezt= (struct BezTriple *)newadr(nu->bezt);
		nu->bp= (struct BPoint *)newadr(nu->bp);
		nu->knotsu= (float *)newadr(nu->knotsu);
		nu->knotsv= (float *)newadr(nu->knotsv);
		if(nu->trim.first) {
			lees_nurb(&nu->trim);
		}
		nu= nu->next;
	}
}

void lees_ika(ika)
struct IkaData *ika;
{
	struct Rod *rod, **rodpp;
	struct Joint *joint;
	struct Skeleton *skel;
	int a;
	
	link_list(&ika->bpbase);
	link_list(&ika->rodbase);
	link_list(&ika->jointbase);
	link_list(&ika->skelbase);

	rod= ika->rodbase.first;
	while(rod) {
		rod->rdef= (struct RodDeform *)newadr(rod->rdef);
		rod->p1= (struct BodyPoint *)newadr(rod->p1);
		rod->p2= (struct BodyPoint *)newadr(rod->p2);
		rod->p3= (struct BodyPoint *)newadr(rod->p3);
		rod->p4= (struct BodyPoint *)newadr(rod->p4);
		
		rod= rod->next;
	}
	
	joint= ika->jointbase.first;
	while(joint) {
		joint->p1= (struct BodyPoint *)newadr(joint->p1);
		joint->p2= (struct BodyPoint *)newadr(joint->p2);
		joint->p3= (struct BodyPoint *)newadr(joint->p3);
		joint->p1a= (struct BodyPoint *)newadr(joint->p1a);
		joint->p2a= (struct BodyPoint *)newadr(joint->p2a);
		joint->p3a= (struct BodyPoint *)newadr(joint->p3a);
		
		joint->r1= (struct Rod *)newadr(joint->r1);
		joint->r2= (struct Rod *)newadr(joint->r2);
		
		joint= joint->next;
	}
	
	skel= ika->skelbase.first;
	while(skel) {
		a= skel->nr;
		rodpp= (struct Rod **)(skel+1);
		while(a--) {
			*rodpp= (struct Rod *)newadr(*rodpp);
			rodpp++;
		}
		skel= skel->next;
	}
}

void leesscene(dir,app)
char *dir;
short app;
{
	struct View *v;
	struct Base *base,**tex;
	struct ObData *ob, *obn;
	struct LaData *la;
	struct PerfSph *ps, *psn;
	struct ColBlck *col;
	struct ColBlckA1 *cola1;
	struct MoData *mo;
	struct PolyData *po;
	struct FontData *fo;
	struct IkaData *ika;
	struct Bezier *bez;
	struct Nurb *nu;
	struct Key *key;
	long file, s, s1, s2, s3, s4, len,filelen,*t, *lp;
	int texofs=0, texlen;
	float *fp;
	short *sp, a,b,colt,app1= -1,basecount= 0, amiga=0, curs;
	char *cp, di[80], fi[50], *home, astr[100];

	len=strlen(dir);
	if(len==0) return;
	if(dir[len-1]=='/') return;


	strcpy(di,dir);
	splitdirstring(di,fi);

	if(testextensie(fi,".scene")) sculptlezer(di,fi);
	else if(testextensie(fi,".clip")) pdrawlezer(di,fi);
	else {
		file= open(dir,O_RDONLY);
		if(file<=0) {
			error("Can't open file");
			return;
		}
		read(file,&s,4);
		if(s!=FORM) {
			read(file, &s1, 4);
			read(file, &s2, 4);
			read(file, &s3, 4);
			read(file, &s4, 4);
			close(file);

			curs= getcursorN();
			if(curs!=2) setcursorN(2);	/* pas op: doet winset */

			if(s==PDPF || s==PDFF) {
				pdrawlezer(di,fi);
			}
			else if(strncmp(&s, "3DG1", 4)==0) {
				videoscapelezer(dir);
			}
/*			else if( strncmp(&s, "IGES", 4)==0) {
				exoticlezer(dir, 0);
			}
*/			else if( strncmp(&s, "Cybe", 4)==0) {
				exoticlezer(dir, 1);
			}
			else if( strncmp(&s, "#Inv", 4)==0) {
				if( strncmp(&s1, "ento", 4)==0) 
				 if( strncmp(&s2, "r V1", 4)==0 || strncmp(&s2, "r V2", 4)==0) 
					if( strncmp(&s3, ".0 a", 4)==0) {
						if( strncmp(&s4, "scii", 4)==0) exoticlezer(dir, 3);
					}
					else error("Can't read Inventor binary");
			
			}
			else {				/* DXF */
				FILE *fp;
		
				fp= fopen(dir, "r");
				fscanf(fp, "%d", &s);
				while(s==999) {		/* commentaar */
					fscanf(fp, "%c", &s);  /* lees 'return' */
					fscanf(fp, "%[^\n]", astr);  /* lees alles behalve 'return' */
					fscanf(fp, "%d", &s);
				}
				if(s==0) {
					fscanf(fp, "%s", astr);
					if( strcmp(astr, "SECTION")==0) {
						fclose(fp);
						exoticlezer(dir, 2);
						return;
					}
					else error("Unknown filetype");
				}
				fclose(fp);
				error("Unknown filetype");
			}
			if(curs!=2) setcursorN(curs);
			countall();
			tekenoverdraw(0);
			
			return;
		}
		read(file,&filelen,4);
		read(file,&s,4);
		if(s==TRa2 || s==TRa1) {
			if(app & 128) error("Can't read Amiga file");
			else if(s==TRa2) amiga= 2;
			else amiga= 1;
		}
		else if(s!=TRI0) {
			error("Can't read file");
			close(file);
			return;
		}
		G.basact=0;
		
		G.localview= 0;

		curs= getcursorN();
		if(curs!=2) setcursorN(2);	/* pas op: doet winset */

		/* appendflag */
		if(app & 128) app1=128;
		else {
			if (dir[0] == '/') strcpy(G.sce,dir);
			else {
				getwd(G.sce);
				strcat(G.sce,"/");
				strcat(G.sce,dir);
			}
			wisalles(0);
		}

		leesfout=0;
		texofs= G.totex;

		read(file,&s,4);	/* TOBL */
		read(file,&s,4);
		read(file,&tobl,4);
		t00=ablock=(long *)callocN(8*tobl+16,"leessceneAblock");
		ablockflag= callocN(tobl+16,"leessceneAblockflag");
		while( read(file,&s,4) ) {
			if(leesfout) break;
			read(file,&len,4);

			if(s==VIEW) {
				if( (len % 100)==0 && (app & 129)>app1) {
					if(amiga==0) lees_viewoud(file,len);
					else {
						cp= mallocN(len, "leesviewamiga");
						read(file, cp, len);
						memcpy(view0.obs, cp+6, 24);
						memcpy(&(view0.phi), cp+36, 12);
						freeN(cp);
					}
				}
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==VIEN) {
				if( (app & 129)>app1) lees_view(file,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==BASE) {
				if( (app & 128+28)>app1 ) {
					lees_base(file);
					basecount++;
				}
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==DATA) {
				if( (app & 128+28)>app1 ) lees_data(file,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==GLOB) {
				if( (app & 128+32)>app1 && amiga==0 ) lees_global(file);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==REND) {
				if( (app & 128+32)>app1 && amiga==0) lees_render(file,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==WRLD) {
				if( len==100 && (app & 130)>app1 && amiga==0) read(file,&wrld,100);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==TEXT || s==TEXN) {
				if( amiga ) {
					if( lseek(file,len,SEEK_CUR)== -1 ) break;
				}
				else if( (app & 128+28)>app1 ) {
					
					if(s==TEXN) texlen= 152;
					else texlen= 110;
					
					G.totex+= len/texlen;
					
					for(a= texofs+1; a<=G.totex; a++) {
						if(a<MAXTEX) {
							G.adrtex[a]=(struct Tex *)callocN(152,"leessceneAdrtex");
							read(file,G.adrtex[a], texlen);
						}
						else read(file, G.adrtex[0], texlen);
					}
				}
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==VFNT) {
				if( (app & 128+28)>app1 ) lees_vectorfont(file,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==SKBS) {
				if( (app & 128+32)>app1 && amiga==0 ) read(file,G.skb,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==PICS) {
				if( (app & 128+32)>app1 && amiga==0 ) read(file,G.pic,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==SCRS) {
				if( (app & 128+32)>app1 && amiga==0 ) read(file,G.scr,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==IMAS) {
				if( (app & 128+32)>app1 ) read(file,G.ftype,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==BSCS) {
				if( (app & 128+32)>app1 ) read(file,G.bufscr,len);
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if(s==USER) {
				astr[0]= 0;
				home = getenv("HOME");
				
				if (home) {
					strcpy(astr,home);
					strcat(astr,"/.T.trace");
					if(strcmp(astr, dir)!=0) astr[0]= 0;
				}
				if(astr[0]) {
					read(file, &User, len);
					setUserdefs();
				}
				else if( lseek(file,len,SEEK_CUR)== -1 ) break;
			}
			else if( lseek(file,len,SEEK_CUR)== -1 ) break;
		}
		close(file);

		/* foutafhandeling van inlezen */
		if(leesfout) {
			error("Error in file/memory");
			t00=ablock+1;
			base=G.firstbase;
			if(base) {
				if( base==(struct Base *) *t00) {
					G.firstbase=0;
					G.lastbase=0;
				} else {
					while(base) {
						if( base==(struct Base *) *t00) break;
					}
					G.lastbase=base;
					base->next=0;
				}
				while(*t00) {
					freeN(*t00);
					t00+=2;
				}
			}
			if(curs!=2) setcursorN(curs);
			return;
		}
		/* aan elkaar knopen */
		base=(struct Base *)(*(ablock+1));
		if(basecount==0) base= 0;		/* een base is altijd het eerste 'block' */

		while(base) {
			if(amiga) amigabase(base);

			base->borig= 0;
			base->d= newadr(base->d);
			base->p= (struct Base *)newadr(base->p);
			base->t= (struct Base *)newadr(base->t);
			base->w= (struct Base *)newadr(base->w);

			base->key= (struct Key *)newadr(base->key);
			key= base->key;
			while(key) {
				key->next= (struct Key *)newadr(key->next);
				key= key->next;
			}
			base->ipokey= (struct Bezier *)newadr(base->ipokey);

			base->pkey= (struct Key *)newadr(base->pkey);
			key= base->pkey;
			while(key) {
				key->next= (struct Key *)newadr(key->next);
				lp= (long *)(key+1);
				*lp= (long)newadr(*lp);
				key= key->next;
			}
			base->ipopkey= (struct Bezier *)newadr(base->ipopkey);

			base->iponoise= (struct Bezier *)newadr(base->iponoise);

			if(base->dupli) G.f|=4;

			if(base->soort==1) {
				ob=(struct ObData *)base->d;
				ob->ipovvkey= (struct Bezier *)newadr(ob->ipovvkey);
				ob->vv=(struct VV *)newadr(ob->vv);
				ob->disp.first= ob->disp.last= 0;
				ob->floatverts= 0;
				if(amiga) {
					sp= (short *)(ob->vv);
					ob->vv->us= sp[4];
				}
				else if( (ob->vv->f1 & 1)==0 ) {	/* maar een keer inlezen */
					ob->vv->key= (struct Key *)newadr(ob->vv->key);
					key= ob->vv->key;
					while(key) {
						key->next= (struct Key *)newadr(key->next);
						key= key->next;
					}
					ob->vv->f1++;
				}
			}
			else if(base->soort==7) {
				ob=(struct ObData *)base->d;
				ob->disp.first= ob->disp.last= 0;
				ob->po=(struct PolyData *)newadr(ob->po);
				po=ob->po;
				if((po->f & 1)==0) {
					po->f45=(short *)newadr(po->f45);
					po->f++;
				}
			}
			else if(base->soort==9) {
				ob= (struct ObData *)base->d;
				ob->disp.first= ob->disp.last= 0;
				col= (struct ColBlck *)(ob+1);
				ob->po= (struct PolyData *)newadr(ob->po);
				po= ob->po;
				po->f45= 0;
				ob->fo= (struct FontData *)newadr(ob->fo);
				if(ob->fo) {
					ob->fo->vf= (struct VectorFont *)newadr(ob->fo->vf);
					ob->fo->vf->us++;
					ob->fo->str= (char *)newadr(ob->fo->str);
					/* OUD! ob->fo->bez= (struct Bezier *)newadr(ob->fo->bez); */
				}
				makepolytext(base);
				extrudepoly(base,0);
			}
			else if ELEM(base->soort, 5, 11) {
				ob=(struct ObData *)base->d;
				ob->ipovvkey= (struct Bezier *)newadr(ob->ipovvkey);
				ob->disp.first= ob->disp.last= 0;
				ob->cu= (struct CurveData *)newadr(ob->cu);
				ob->floatverts= 0;
				if(ob->cu) {
					lees_nurb( &(ob->cu->curve) );
					ob->cu->disp.first=ob->cu->disp.last= 0;
					ob->cu->bev.first=ob->cu->bev.last= 0;
					ob->cu->bevbase= (struct Base *)newadr(ob->cu->bevbase);

					ob->cu->key= (struct Key *)newadr(ob->cu->key);
					key= ob->cu->key;
					while(key) {
						key->next= (struct Key *)newadr(key->next);
						key= key->next;
					}
				}
			}
			else if(base->soort== -2) {
				mo= (struct MoData *)base->d;
				mo->data= 0;	/* is niet weggeschreven */
				
					/* oude files */
				mo->beztra= (struct Bezier *)newadr(mo->beztra);
					/* nieuwe files */
				lees_nurb( &(mo->curve) );
				mo->disp.first= mo->disp.last= 0;

				mo->key= (struct Key *)newadr(mo->key);
				key= mo->key;
				while(key) {
					key->next= (struct Key *)newadr(key->next);
					key= key->next;
				}

				if(amiga==0) {
					mo->ipotra= (struct Bezier *)newadr(mo->ipotra);
					mo->ipoqfi= (struct Bezier *)newadr(mo->ipoqfi);
					mo->ipovkey= (struct Bezier *)newadr(mo->ipovkey);
				}
				else {
					mo->ipotra= mo->ipoqfi= 0;
				}
				if(mo->f & 4) G.f|=4;
			}
			else if(base->soort== -4) {
				ika= (struct IkaData *)base->d;
				
				lees_ika(ika);
				
				ika->key= (struct Key *)newadr(ika->key);
				key= ika->key;
				while(key) {
					key->next= (struct Key *)newadr(key->next);
					key= key->next;
				}
				ika->ipokey= (struct Bezier *)newadr(ika->ipokey);
			}

			/* patch voor amigatype==1 */
			if(amiga==1) {
				if(base->soort== 1) {
					ob= (struct ObData *)base->d;
					obn= callocN(sizeof(struct ObData)+ob->c*sizeof(struct ColBlck), "newcolblck");
					base->d= (long *)obn;
					memcpy(obn, ob, sizeof(struct ObData));
					cp= (char *)(ob+1);
					col= (struct ColBlck *)(obn+1);
					cp+= (ob->c-1)*32;
					col+= (ob->c-1);
					for(a=0; a<ob->c; a++) {
						cola1= (struct ColBlckA1 *)cp;
						col->r= cola1->r;
						col->g= cola1->g;
						col->b= cola1->b;
						col->mirr= cola1->mirr;
						col->mirg= cola1->mirg;
						col->mirb= cola1->mirb;
						col->ambr= cola1->ambr;
						col->ambg= cola1->ambg;
						col->ambb= cola1->ambb;
						col->mode= cola1->mode;
						col->kref= cola1->kref;
						col->kspec= cola1->kspec;
						col->spec= cola1->spec;
						col->tra= cola1->tra;
						col->specr= col->specb= col->specg= 255;
						col->hasize= cola1->hasize;
						col->mode &= 63;
						col--;
						cp-= 32;
					}
					freeN(ob);
				}
				else if(base->soort== 3) {
					ps= (struct PerfSph *)base->d;
					psn= callocN(sizeof(struct PerfSph)+sizeof(struct ColBlck), "newcolblckSphere");
					base->d= (long *)psn;
					memcpy(psn, ps, sizeof(struct PerfSph));
					cp= (char *)(ps+1);
					col= (struct ColBlck *)(psn+1);

					memcpy(col, cp, 32);

					freeN(ps);
				}
				else if(base->soort== 2) {
					la= (struct LaData *)base->d;
					base->d= callocN(sizeof(struct LaData), "newmemblockLamp");
					memcpy(base->d, la, 18);
					freeN(la);
				}
			}

			/* texture imatbases en patch oude files ( Ztra en Zinv en SkySh) */
			if( (base->soort & 1)!=0 || base->soort==2 ) {
				col= 0;
				if ELEM5(base->soort, 1, 5, 7, 9, 11) {
					ob=(struct ObData *)base->d;
					col= (struct ColBlck *)(ob+1);
					t= (long *)col->tex;
					tex= col->base;
					colt= ob->c;
				}
				else if(base->soort==2) {
					la=(struct LaData *)base->d;
					t= (long *)la->tex;
					tex= la->base;
					colt=1;
				}
				else {
					ps=(struct PerfSph *)base->d;
					col= &(ps->c);
					t=(long *)col->tex;
					tex=col->base;
					colt=1;
				}
				for(a=0; a<colt; a++) {
					if(amiga) *t=0;
					
					cp= (char *)t;
					for(b=0; b<4; b++) {
						if(amiga) tex[b]=0;
						else tex[b]= (struct Base *)newadr(tex[b]);
						
						if(cp[b]) cp[b]+= texofs;
						if(cp[b]>MAXTEX) cp[b]= 0;
					}
					if(amiga && col) if(col->mode & 2) col->mode= 1;
					if(col && col->mode1 & 255) {	/* niet gebruikt, was vroeger speclim */
						col->mode1= 0;
						col->mode &= ~2;	/* Zoffs */
					}

					if(colt>1) {
						col++;
						t= (long *)col->tex;
						tex= col->base;
					}
				}
			}
			base=base->next;
		}
		
		/* NIEUW */
		if(G.timeipo) {
			G.timeipo= (struct Bezier *)newadr(G.timeipo);
		}		
		
		/* obs en tar parents */
		if( (app & 129)>app1 && amiga==0) {
			view0.opar= (struct Base *)newadr(view0.opar);
			view0.tpar= (struct Base *)newadr(view0.tpar);
			view0.cambase= (struct Base *)newadr(view0.cambase);
			if(view0.next) {
				v= view0.next;
				while(v) {
					v->opar= (struct Base *)newadr(v->opar);
					v->tpar= (struct Base *)newadr(v->tpar);
					v->cambase= (struct Base *)newadr(v->cambase);
					v=v->next;
				}
			}
		}
		/* wereld tex en bases */
		if( (app & 130)>app1  && amiga==0) {
			for(a=0;a<4;a++) {
				if(app & 128) wrld.stex[a]=0;
				if(app & 128) wrld.sbase[a]=0;
				else wrld.sbase[a]=(struct Base *)newadr(wrld.sbase[a]);
			}
		}
		/* imap blokken, en oude files corrigeren voor crop */
		if( amiga==0 ) {
			for(b= texofs+1;b<=G.totex;b++) {
				if(G.adrtex[b]->soort==5) {
					G.adrtex[b]->ima= (struct Imap *)newadr(G.adrtex[b]->ima);
					if(G.adrtex[b]->ima) {
						G.adrtex[b]->ima->ok= 1;
						G.adrtex[b]->ima->mipmap= 0;
						G.adrtex[b]->ima->ibuf= 0;
						G.adrtex[b]->ima->anim= 0;
						G.adrtex[b]->ima->lastframe= 0;
					}
					if(G.adrtex[b]->var[5]==255) {	/* crop variabele */
						G.adrtex[b]->var[4]= 0;
						G.adrtex[b]->var[5]= 0;
					}
					/* fields per image */
					if(G.adrtex[b]->var[7]== 0) G.adrtex[b]->var[7]= 2;
				}
			}
		}
		/* displaylisten maken */
		/* zo mogelijk basact zetten */
		/* paden berekenen en imagewrap correctie */
		/* oude tracefiles en leesflaggen op nul */
		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) G.basact=base;
			if(base->len==0) { /* van voor keyframing */
				if(base->soort== -2) {
					mo=(struct MoData *)base->d;
					base->len= mo->fr;
					base->f2= (short)mo->f;
					if(amiga) {
						sp= &(mo->fr);
						base->len= sp[1];
					}
				}
				else base->len=80;
			}
			if(base->soort== -2) {
				mo=(struct MoData *)base->d;
				if(mo->beztra) {
					if(amiga) {
						sp= &(mo->fr);
						base->len= sp[1];
						mo->ipotra= 0;
						mo->beztra->cp-= 2;
						fp= (float *)(mo->beztra+1);
						for(a= mo->beztra->cp; a>0; a--) {
							fp[0]= *( (long *)fp );
							fp[1]= *( (long *)fp+1 );
							fp[2]= *( (long *)fp+2 );
							fp+= 3;
							fp[0]= *( (long *)fp );
							fp[1]= *( (long *)fp+1 );
							fp[2]= *( (long *)fp+2 );
							SWAP(float, fp[0], fp[-3]);
							SWAP(float, fp[1], fp[-2]);
							SWAP(float, fp[2], fp[-1]);

							fp+= 3;
							fp[0]= *( (long *)fp );
							fp[1]= *( (long *)fp+1 );
							fp[2]= *( (long *)fp+2 );

							fp[0]+= fp[-3];
							fp[1]+= fp[-2];
							fp[2]+= fp[-1];
							fp[-6]+= fp[-3];
							fp[-5]+= fp[-2];
							fp[-4]+= fp[-1];
							fp+= 3;
						}
					}
					nu= callocN(sizeof(struct Nurb), "leespad");
					oldbez_to_curve(mo->beztra, nu);
					freeN(mo->beztra);
					mo->beztra= 0;
					addtail(&mo->curve.first, nu);
				}
				if(mo->curve.first) {
					makeDispList(base);/* ook berekenpad */
				}
			}
			else if(base->soort==1) {
				ob= (struct ObData *)base->d;
				if(ob->vv->f1 & 1) ob->vv->f1--;
				if(base->skelnr) calc_deform(base);
			}
			else if(base->soort==2) {
				la= (struct LaData *)base->d;

				if(amiga) {
					memcpy(&(la->soort), &(la->rt), sizeof(struct LaData)-8);
					la->tex[0]= la->tex[1]= la->tex[2]= la->tex[3]= 0;
				}
				if(la->bufsize==0) {
					la->bufsize= 512;
					la->clipsta= la->ld/10000;
					la->clipend= 1000;
					la->samp= 3;
					la->bias= 40;
					la->soft= 5.0;
				}
				if(la->energy<0.2 || la->energy>5.0) la->energy= 1.0;
			}
			else if(base->soort==5) {
				if(base->skelnr) calc_deform(base);
			}
			else if(base->soort==7) {
				ob= (struct ObData *)base->d;
				if(ob->po->f & 1) ob->po->f--;
				if(ob->po->depth==0) ob->po->depth= 160;
				if(ob->po->f1==0) ob->po->f1= 31;
			}
			else if(base->soort==11) {
				ob= (struct ObData *)base->d;
				makeBevelList(base);
			}
			makeDispList(base);

			base= base->next;
		}

		if(wrld.expos<0.2 || wrld.expos>5.0) wrld.expos= 1.0;

		/* test of curves als bevelcurve wordt gebruikt */
		base= G.firstbase;
		while(base) {
			if(base->soort==11) {
				ob= (struct ObData *)base->d;
				if(ob->cu->bevbase) makeDispList(base);
			}
			base= base->next;
		}

		if(G.osa<=0 || G.osa>66) G.osa=5;
		if(G.xminb==G.xmaxb || G.yminb==G.ymaxb) {
			G.xminb= 320;
			G.xmaxb= 960;
			G.yminb= 200;
			G.xmaxb= 600;
		}
		G.framelen= ((float)G.framapto)/((float)G.images);
		if(R.xparts==0 || R.yparts==0 ||
		    (R.anim!=1 && R.anim!=5 && R.anim!=6) ) {
			R.anim= 6;
			R.winpos= 128;
			R.xparts=R.yparts=R.planes= 1;
			R.imtype= 2;
		}
		countall();

		/* testen of er nog blokken rondslingeren */
		cp= ablockflag;
		t00= ablock;

		while(tobl--) {
			if(*cp==0 && *(t00+1)) {
				freeN(*(t00+1));
				/* printf("rondslingerend datablok\n"); */
			}
			cp++;
			t00+=2;
		}
		freeN(ablock);
		freeN(ablockflag);

		if(G.winar[0]) SetButs(100,199);
		if(curs!=2) setcursorN(curs);
	}


}

/*  **************** */

char *writebuf;
long count, mywfile;

void mywrite(adr, len)
long *adr, len;
{
	if(len>50000) {
		if(count) {
			write(mywfile, writebuf, count);
			count= 0;
		}
		write(mywfile, adr, len);
		return;
	}
	if(len+count>99999) {
		write(mywfile, writebuf, count);
		count= 0;
	}
	memcpy(writebuf+count, adr, len);
	count+= len;
}

void bgnwrite(file)
long file;
{
	mywfile= file;
	writebuf= (char *)mallocN(100000,"bgnwrite");
	count= 0;
}

void endwrite()
{
	if(count) {
		write(mywfile, writebuf, count);
	}
	freeN(writebuf);

}

/* **************** */

void schr_view(file)
long file;
{
	struct View *v;
	long t,len, nr;

	nr= 1;
	len= sizeof(struct View);
	if(view0.next) {
		v= view0.next;
		while(v) {
			nr++;
			len+= sizeof(struct View);
			v= v->next;
		}
	}
	len+= 4;
	t= VIEN;
	mywrite(&t,4);
	mywrite(&len,4);
	mywrite(&nr,4);

	if(G.localview) gettempview(&v);
	else v= &view0;

	mywrite(v, sizeof(struct View));

	if(view0.next) {
		v= view0.next;
		while(v) {
			mywrite(v,sizeof(struct View));
			v= v->next;
		}
	}
}

void schr_render(file)	/* alleen oude files */
long file;
{
	long t;

	t=REND;
	mywrite(&t,4);
	t=64;
	mywrite(&t,4);

	t=R.size;
	if(G.filt) t=BSET(t,5);
	if(G.schaduw) t=BSET(t,10);
	if(G.trace) t=BSET(t,11);
	if(G.border) t=BSET(t,12);
	if(G.skybuf) t=BSET(t,13);
	if(G.ali) t=BSET(t,14);
	if(G.script) t=BSET(t,15);
	if(G.holo) t=BSET(t,16);
	if(G.disc) t=BSET(t,17);
	/* if(G.osa) t=BSET(t,20); */
	mywrite(&t,4);
	mywrite(&G.osa,2);
	mywrite(&R.anim,2);

	mywrite(&t,4);	/* bcsrgb */
	mywrite(&t,4);

	mywrite(&G.sfra,4);	 /* 4 is oud foutje */
	mywrite(&G.cfra,4);	 /* 4 is oud foutje */
	mywrite(&G.efra,4);	 /* 4 is oud foutje */
	mywrite(&G.xminb,8);	 /* alle vier */
	mywrite(&view0.lay,2);
	mywrite(&G.sview,6);	 /* en tracetype, tradep */
	mywrite(&view0.opar,8);
	mywrite(&G.layact,2);
	mywrite(&R.f1,2);	 /* flag is short */
	mywrite(&R.xsch,4);	/* dus ook ysch */
	mywrite(&R.xasp,4);
}

void schr_global(file)
long file;
{
	long t;
	extern short editbutflag;
	
	t=GLOB;
	mywrite(&t,4);
	t=100;
	mywrite(&t,4);

	t=R.size;
	if(G.filt) t=BSET(t,5);
	if(G.schaduw) t=BSET(t,10);
	if(G.trace) t=BSET(t,11);
	if(G.border) t=BSET(t,12);
	if(G.skybuf) t=BSET(t,13);
	if(G.ali) t=BSET(t,14);
	if(G.script) t=BSET(t,15);
	if(G.holo & 1) t=BSET(t,16);
	if(G.disc) t=BSET(t,17);
	if(G.genlock & 1) t=BSET(t,18);
	if(G.genlock & 2) t=BSET(t,19);
	if(G.bufscript) t=BSET(t,20);
	if(G.zbuf) t=BSET(t,21);
	if(G.holo & 2) t=BSET(t,22);

	mywrite(&t,4);
	mywrite(&G.osa,2);
	mywrite(&R.anim,2);

	t= 0;
	mywrite(&editbutflag, 2);
	mywrite(&bezres, 2);
	mywrite(&bezmax, 2);
	mywrite(&G.holopadlen, 2);
	
	mywrite(&G.cfra,12);	/* alle zes */

	mywrite(&G.xminb,8);	 /* alle vier */
	mywrite(&view0.lay,2);	/* oud: wordt niet meer gelezen */
	mywrite(&G.sview,6);	 /* en obszoom, tradep */
	mywrite(&view0.opar,8);	/* oud: wordt niet meer gelezen */
	mywrite(&G.layact,2);
	mywrite(&R.f1,2);	 /* flag is short */
	mywrite(&R.xsch,4);	/* dus ook ysch */
	mywrite(&R.xasp,4);

	mywrite(&R.winpos,2);
	mywrite(&R.planes,2);
	mywrite(&R.imtype,2);
	mywrite(&R.xparts,4);  /* ook yparts */
	mywrite(&panoramaview,2);
	mywrite(&panodegr,2);
	mywrite(&R.edgeint,2);

	mywrite(&G.timeipo,4);
	t= 0;
	mywrite(&t,4);
	mywrite(&t,4);
	mywrite(&t,4);
	mywrite(&t,4);
}

void schr_stri(file)
long file;
{
	long t;

	t= SKBS;
	mywrite(&t,4);
	t=80;
	mywrite(&t,4);
	mywrite(G.skb,80);
	t= PICS;
	mywrite(&t,4);
	t=80;
	mywrite(&t,4);
	mywrite(G.pic,80);
	t= SCRS;
	mywrite(&t,4);
	t=80;
	mywrite(&t,4);
	mywrite(G.scr,80);
	t= IMAS;
	mywrite(&t,4);
	t=80;
	mywrite(&t,4);
	mywrite(G.ftype,80);
	t= BSCS;
	mywrite(&t,4);
	t=80;
	mywrite(&t,4);
	mywrite(G.bufscr,80);
}

void schr_data(file,data,len)
long file,*data,len;
{
	long t;

	if(data) {
		t=DATA;
		mywrite(&t,4);
		t=len+4;
		mywrite(&t,4);
		t=(long)data;
		mywrite(&t,4);
		mywrite(data,len);
		tobl++;
	}
}

void schr_nurb(file, nu)
long file;
struct Nurb *nu;
{

	while(nu) {
		if(nu->bezt) schr_data(file, nu->bezt, nu->pntsu*sizeof(struct BezTriple));
		if(nu->bp) schr_data(file, nu->bp, nu->pntsu*nu->pntsv*sizeof(struct BPoint));
		if(nu->knotsu) schr_data(file, nu->knotsu, KNOTSU(nu)*4);
		if(nu->knotsv) schr_data(file, nu->knotsv, KNOTSV(nu)*4);
		if(nu->trim.first) schr_nurb(file, nu);

		schr_data(file, nu, sizeof(struct Nurb));

		nu= nu->next;
	}
}

void schr_ika(file, ika)
long file;
struct IkaData *ika;
{
	struct BodyPoint *bop;
	struct Rod *rod;
	struct Joint *joint;
	struct Skeleton *skel;
	
	bop= ika->bpbase.first;
	while(bop) {
		schr_data(file, bop, sizeof(struct BodyPoint));
		bop= bop->next;
	}
	rod= ika->rodbase.first;
	while(rod) {
		schr_data(file, rod, sizeof(struct Rod));
		if(rod->rdef) schr_data(file, rod->rdef, sizeof(struct RodDeform));
		rod= rod->next;
	}
	joint= ika->jointbase.first;
	while(joint) {
		schr_data(file, joint, sizeof(struct Joint));
		joint= joint->next;
	}
	skel= ika->skelbase.first;
	while(skel) {
		schr_data(file, skel, sizeof(struct Skeleton)+4*skel->nr);
		skel= skel->next;
	}
	
	
}

void schr_base(file,base)
long file;
struct Base *base;
{
	char a[20];
	long len;
	struct ObData *ob;
	struct VV *vv;
	struct LaData *la;
	struct MoData *mo;
	struct ColBlck *col;
	struct PolyData *po;
	struct FontData *fo;
	struct CurveData *cu;
	struct IkaData *ika;
	struct Bezier *bez;
	struct PerfSph *ps;
	struct Key *key;

	len=BASE;
	mywrite(&len,4);
	len=sizeof(struct Base)+4;
	mywrite(&len,4);
	len=(long)base;
	mywrite(&len,4);
	mywrite(base,sizeof(struct Base));
	tobl++;
	key=base->key;
	while(key) {
		len=sizeof(struct Key)+4*(3+3+4+9);
		schr_data(file,key,len);
		key= key->next;
	}
	if(base->ipokey) {
		bez=base->ipokey;
		len=sizeof(struct Bezier)+bez->cp*36;
		schr_data(file,bez,len);
	}
	key=base->pkey;
	while(key) {
		len=sizeof(struct Key)+4*25+4;
		schr_data(file,key,len);
		key= key->next;
	}
	if(base->ipopkey) {
		bez=base->ipopkey;
		len=sizeof(struct Bezier)+bez->cp*36;
		schr_data(file,bez,len);
	}
	if(base->iponoise) {
		bez= base->iponoise;
		len= sizeof(struct Bezier)+bez->cp*36;
		schr_data(file, bez, len);
	}

	if ELEM4(base->soort, 1, 5, 7, 11 ) {
		ob=(struct ObData *)base->d;
		len=sizeof(struct ObData) + ob->c * sizeof(struct ColBlck);
		schr_data(file,ob,len);
		if(ob->ipovvkey) {
			bez=ob->ipovvkey;
			len=sizeof(struct Bezier)+bez->cp*36;
			schr_data(file,bez,len);
		}
		vv=ob->vv;
		if(vv) if( (vv->f1 & 1)==0 ) {
			len= sizeof(struct VV)+vv->vert*12+vv->vlak*16;
			schr_data(file,vv,len);
			if(vv->key) {
				len= sizeof(struct Key)+ 2*6*vv->vert+4;
				key= vv->key;
				while(key) {
					schr_data(file,key,len);
					key= key->next;
				}
			}
			vv->f1++;
		}
		po=ob->po;
		if(po) if( (po->f & 1)==0 ) {
			len=sizeof(struct PolyData)+po->vert*6+po->poly*2;
			schr_data(file,po,len);
			if(po->f45) {
				len= 6*po->vert;
				schr_data(file,po->f45,len);
			}
			po->f++;
		}
		cu= ob->cu;
		if(cu) {
			schr_nurb(file, cu->curve.first);
			schr_data(file, cu, sizeof(struct CurveData));
			
			if(cu->key) {
				len= sizeof(struct Key)+ 12*cu->keyverts+4;
				key= cu->key;
				while(key) {
					schr_data(file, key, len);
					key= key->next;
				}
			}
		}
	}
	else if(base->soort==2) {
		la=(struct LaData *)base->d;
		len=sizeof(struct LaData);
		schr_data(file,la,len);
	}
	else if(base->soort==3) {
		ps=(struct PerfSph *)base->d;
		len=sizeof(struct PerfSph);
		schr_data(file,ps,len);
	}
	else if(base->soort==9) {
		ob=(struct ObData *)base->d;
		len=sizeof(struct ObData) + ob->c * sizeof(struct ColBlck);
		schr_data(file,ob,len);
		po=ob->po;
		if(po) schr_data(file,po,sizeof(struct PolyData));
		fo= ob->fo;
		if(fo) {
			schr_data(file,fo,sizeof(struct FontData));
			len= (fo->len+5) & 0xFFFC;
			schr_data(file,fo->str,len);
			/* OUD!! */
			/* if(fo->bez) { */
			/* 	len=sizeof(struct Bezier)+fo->bez->cp*36; */
			/* 	schr_data(file,fo->bez,len); */
			/* } */
		}
	}
	else if(base->soort== -2) {
		mo=(struct MoData *)base->d;
		len=sizeof(struct MoData);
		schr_data(file,mo,len);

		schr_nurb(file, mo->curve.first);

		if(mo->ipotra) {
			bez=mo->ipotra;
			len=sizeof(struct Bezier)+bez->cp*36;
			schr_data(file,bez,len);
		}
		if(mo->ipoqfi) {
			bez=mo->ipoqfi;
			len=sizeof(struct Bezier)+bez->cp*36;
			schr_data(file,bez,len);
		}
		if(mo->ipovkey) {
			bez= mo->ipovkey;
			len= sizeof(struct Bezier)+bez->cp*36;
			schr_data(file, bez, len);
		}
		if(mo->key) {
			len= sizeof(struct Key)+ 12*mo->keyverts+4;
			key= mo->key;
			while(key) {
				schr_data(file, key, len);
				key= key->next;
			}
		}
	}
	else if(base->soort== -4) {
		ika= (struct IkaData *)base->d;
		schr_data(file, ika, sizeof(struct IkaData));
		
		schr_ika(file, ika);
		
		if(ika->ipokey) {
			bez= ika->ipokey;
			len= sizeof(struct Bezier)+bez->cp*36;
			schr_data(file, bez, len);
		}
		if( ika->key ) {
			len= sizeof(struct Key)+ 16*ika->keyverts+4*ika->keyrods;
			key= ika->key;
			while(key) {
				schr_data(file, key, len);
				key= key->next;
			}
		}
	}
}

long convex(v1, v2, v3, v4)
float *v1, *v2, *v3, *v4;
{
	float cross[3], test[3];
	
	CalcNormFloat(v1, v2, v3, cross);
	CalcNormFloat(v2, v3, v4, test);
	if(cross[0]*test[0]+cross[1]*test[1]+cross[2]*test[2]>0.0) {
		CalcNormFloat(v3, v4, v1, test);
		if(cross[0]*test[0]+cross[1]*test[1]+cross[2]*test[2]>0.0) return 1;
	}
	return 0;
}

void givequadverts(evl, evl1, v1, v2, v3, v4)
struct EditVlak *evl, *evl1;
struct EditVert **v1, **v2, **v3, **v4;
{
				
	if(evl->v1!=evl1->v1 && evl->v1!=evl1->v2 && evl->v1!=evl1->v3) {
		*v1= evl->v1;
		*v2= evl->v2;
	}
	else if(evl->v2!=evl1->v1 && evl->v2!=evl1->v2 && evl->v2!=evl1->v3) {
		*v1= evl->v2;
		*v2= evl->v3;
	}
	else if(evl->v3!=evl1->v1 && evl->v3!=evl1->v2 && evl->v3!=evl1->v3) {
		*v1= evl->v3;
		*v2= evl->v1;
	}
	
	if(evl1->v1!=evl->v1 && evl1->v1!=evl->v2 && evl1->v1!=evl->v3) {
		*v3= evl1->v1;
		if(evl1->v2== *v2) *v4= evl1->v3;
		else *v4= evl1->v2;
	}
	else if(evl1->v2!=evl->v1 && evl1->v2!=evl->v2 && evl1->v2!=evl->v3) {
		*v3= evl1->v2;
		if(evl1->v3== *v2) *v4= evl1->v1;
		else *v4= evl1->v3;
	}
	else if(evl1->v3!=evl->v1 && evl1->v3!=evl->v2 && evl1->v3!=evl->v3) {
		*v3= evl1->v3;
		if(evl1->v1== *v2) *v4= evl1->v2;
		else *v4= evl1->v1;
	}
}


struct EditVlak4 {
	struct EditVlak *evl;
	struct EditEdge *ed1, *ed2, *ed3;
};



void schr_videoscape_doit(struct Base *base, char *str)
{
	struct EditVert **evlist, *eve, *v1, *v2, *v3, *v4;
	struct EditVlak4 *edvlak4block, *evl4;
	struct EditEdge *eed, *findedgelist();
	struct EditVlak *evl, *evl1, **evlpp;
	struct ObData *ob;
	struct VV *vv;
	struct VlakOb *adrvl;
	struct VertOb *adrve;
	struct ColBlck *col;
	FILE *fp;
	float fac, len, mat4[4][4], vec[3], omat[3][3];
	float *n1, *n2, inp, VecLenf();
	ulong kleur[16];
	long a, tot, totvlak, file, ok, lasttime, *edgedata, *edda;
	short *v, val;
	char *cp;

	if(base==0 || base->soort!=1) return;

	ob= (struct ObData *)base->d;
	col= (struct ColBlck *)(ob+1);
	tot= ob->c;
	cp= (char *)kleur;
	for(a=0;a<tot;a++, cp+=4) {
		cp[0]= col->emit;
		cp[1]= col->b;
		cp[2]= col->g;
		cp[3]= col->r;
		col++;
	}

	vv= ob->vv;

	parentbase(base, mat4, omat);
	fac= vv->ws;
	Mat4MulFloat3(mat4, fac);
	fac= 1.0/5000.0;
	Mat4MulFloat(mat4, fac);
	
	fp= fopen(str,"w");
	if(fp==NULL) return;

	fprintf(fp,"3DG1\n");

	/* zo mogelijk vierkanten maken */

	if(G.edve.first) freelist(&G.edve);
	if(G.eded.first) freelist(&G.eded);
	if(G.edvl.first) freelist(&G.edvl);
	free_hashedgetab();

	tot= vv->vert;
	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+tot);

	/* editverts aanmaken */

	evlist= (struct EditVert **)mallocN(tot*4,"evlist");
	for(a=0; a<tot; a++) {
		eve= addvertlist(0);
		evlist[a]= eve;

		VECCOPY(eve->co, adrve->c);
		adrve++;
	}
	totvlak= 0;
	
	if(vv->vlak) {
		/* edges en vlakken maken */

		for(a=0;a<vv->vlak;a++)	{
			v1= evlist[adrvl->v1];
			v2= evlist[adrvl->v2];
			v3= evlist[adrvl->v3];

			evl= addvlaklist(v1, v2, v3, (adrvl->ec & 31));
			if(evl) {
				VECCOPY(evl->n, adrvl->n);
				Normalise(evl->n);
				totvlak++;
			}
			adrvl++;
		}
	}
	freeN(evlist);

	fprintf(fp, "%d\n", vv->vert);
	tot= 0;
	eve= G.edve.first;
	while(eve) {
		VECCOPY(vec, eve->co);
		Mat4MulVecfl(mat4, vec);
		fprintf(fp, "%f %f %f\n", vec[0], vec[1], vec[2] );
		eve->vn= (struct EditVert *)tot;
		tot++;
		eve= eve->next;
	}

	if(totvlak) {

		/* Testen of driehoeken ook als vierhoeken geschreven kunnen worden.
			 * Nieuw vlakarray maken met als extra de pointers naar edges,  vlaggen (re)setten
			 * In de edges een teller bijhouden hoeveel vlakken en ze krijgen 
			 *  in ->vn een pointer naar data waarin staat welke vlakken ze delen.
			 * Voor alle edges met 2 vlakken: vergelijk vlaknormalen, indien gelijk: zet flag f
			 * Voor alle vlakken:
			 *    met 1 'goede' edge: zet vlag f1 in edge,  clear flag f in beide aanliggende vlakken
			 *    met 2 of meer 'goede edges: vind de langste edge,  doe als vorig.
			 *    zet vlakflag f als samengevoegd.
			 * Voor alle edges met flag f2: schrijf vierkant.
			 * Voor alle vlakken met flag f==0: schrijf driehoek.
			 */
	
		/* edges pointer geven naar vlakkenblock */
		eed= G.eded.first;
		tot= 0;
		while(eed) {
			eed->f= 0;
			tot++;
			eed= eed->next;
		}
		edda= edgedata= mallocN(8*tot, "schr_videosc3");
		eed= G.eded.first;
		while(eed) {
			eed->vn= (struct EditVert *)edda;
			edda+= 2;
			eed= eed->next;
		}
	
		/* init datablocks */
		evl4= edvlak4block= mallocN(totvlak*sizeof(struct EditVlak4), "schr_videosc4");
		evl= G.edvl.first;
		while(evl) {
			evl->f= 0;
			evl4->evl= evl;
	
			eed= findedgelist(evl->v1, evl->v2);
			eed->f++;
			edda= (long *)eed->vn;
			if(eed->f==1) *edda= (long)evl;
			else edda[1]= (long)evl;
			evl4->ed1= eed;
	
			eed= findedgelist(evl->v2, evl->v3);
			eed->f++;
			edda= (long *)eed->vn;
			if(eed->f==1) *edda= (long)evl;
			else edda[1]= (long)evl;
			evl4->ed2= eed;
	
			eed= findedgelist(evl->v3, evl->v1);
			eed->f++;
			edda= (long *)eed->vn;
			if(eed->f==1) *edda= (long)evl;
			else edda[1]= (long)evl;
			evl4->ed3= eed;
	
			evl4++;
			evl= evl->next;
		}
	
		/* alle edges met ->f==2: vergelijk kleur en vlaknormalen, zet flags */
		eed= G.eded.first;
		while(eed) {
			if(eed->f==2) {
				edda= (long *)eed->vn;
				evl= (struct EditVlak *)edda[0];
				n1= evl->n;
				evl1= (struct EditVlak *)edda[1];
				n2= evl1->n;
				
				if(evl->f1!=evl1->f1) eed->f= 0;	/* kleur */
				else {
					inp= n1[0]*n2[0]+ n1[1]*n2[1]+ n1[2]*n2[2];
					if(inp<0.999) eed->f= 0;
				}
			}
			else eed->f= 0;
			
			eed= eed->next;
		}
	
		/* loop vlakken af , test daarvan de edges en zet vlaggen */
		/* eerst alleen vlakken met 1 goede edge, daarna de rest */
		ok= 1;
		lasttime = 0;
		while(ok) {
			ok= 0;
	
			evl4= edvlak4block;
			for(a=0; a<totvlak; a++, evl4++) {
	
				if(evl4->evl->f==0) {
	
					tot= evl4->ed1->f + evl4->ed2->f + evl4->ed3->f;
					if(lasttime==0 && tot>2) tot= 0;
					
					if(tot) {
						eed= 0;
						len= 0.0;
						
						if(evl4->ed1->f) {
							/* beide vlakken moeten 'goed' zijn */
							evlpp= (struct EditVlak **)evl4->ed1->vn;
							if( evlpp[0]->f ) evlpp[1]->f= 0;
							else if( evlpp[1]->f ) evlpp[0]->f= 0;
							else {
								/* alleen goed gevormde convexe vierhoeken */
								givequadverts(evlpp[0], evlpp[1], &v1, &v2, &v3, &v4);
								if( convex(v1->co, v2->co, v3->co, v4->co)) {
									eed= evl4->ed1;
									len= VecLenf(eed->v1->co, eed->v2->co);
								}
							}	
						}
						if(evl4->ed2->f) {
							/* beide vlakken moeten 'goed' zijn */
							evlpp= (struct EditVlak **)evl4->ed2->vn;
							if( evlpp[0]->f ) evlpp[1]->f= 0;
							else if( evlpp[1]->f ) evlpp[0]->f= 0;
							else {
								givequadverts(evlpp[0], evlpp[1], &v1, &v2, &v3, &v4);
								if( convex(v1->co, v2->co, v3->co, v4->co)) {
									fac= VecLenf(evl4->ed2->v1->co, evl4->ed2->v2->co);
									if(len< fac ) {
										len= fac;
										eed= evl4->ed2;
									}
								}
							}
						}
						if(evl4->ed3->f) {
							/* beide vlakken moeten 'goed' zijn */
							evlpp= (struct EditVlak **)evl4->ed3->vn;
							if( evlpp[0]->f ) evlpp[1]->f= 0;
							else if( evlpp[1]->f ) evlpp[0]->f= 0;
							else {
								givequadverts(evlpp[0], evlpp[1], &v1, &v2, &v3, &v4);
								if( convex(v1->co, v2->co, v3->co, v4->co)) {
									fac= VecLenf(evl4->ed3->v1->co, evl4->ed3->v2->co);
									if(len< fac ) {
										eed= evl4->ed3;
									}
								}
							}
						}
						if(eed) {
							/* eed is de edge die mag samenvoegen, wis de andere edgeflags en zet vlakflag */
							ok= 1;
							eed->f1= 1;
							evl4->ed1->f= 0;
							evl4->ed2->f= 0;
							evl4->ed3->f= 0;
							evlpp= (struct EditVlak **)eed->vn;
							evlpp[0]->f= 1;
							evlpp[1]->f= 1;
						}
					}
				}
			}
			if(lasttime) break;
			if(ok==0 && lasttime==0) {
				ok= 1;
				lasttime= 1;
			}
		}
		
		/* vierhoeken schrijven */
		eed= G.eded.first;
		while(eed) {
			if(eed->f1) {
				edda= (long *)eed->vn;
				evl= (struct EditVlak *)edda[0];
				evl1= (struct EditVlak *)edda[1];
				
				/* vind de vier punten */
				givequadverts(evl, evl1, &v1, &v2, &v3, &v4);
				
				fprintf(fp, "4 %d %d %d %d 0x%x\n", v1->vn, v2->vn, v3->vn, v4->vn, kleur[evl->f1]);
			}
			eed= eed->next;
		}
		
		/* driehoeken schrijven */
		evl= G.edvl.first;
		while(evl) {
			if(evl->f==0) {
				fprintf(fp, "3 %d %d %d 0x%x\n", evl->v1->vn, evl->v2->vn, evl->v3->vn, kleur[evl->f1]);
			}
			evl= evl->next;
		}
	
		freeN(edvlak4block);
		freeN(edgedata);
	}
	
	/* edges schrijven */
	tot= vv->vert;
	adrve= (struct VertOb *)(vv+1);
	adrvl= (struct VlakOb *)(adrve+tot);
	for(a=0; a<vv->vlak; a++, adrvl++) {
		if(adrvl->v1 == adrvl->v2) fprintf(fp, "2 %d %d 0\n", adrvl->v1, adrvl->v3);
	}
	
	fclose(fp);

}

void schr_videoscape_lamps(char *str)
{
	struct Base *base;
	struct LaData *la;
	FILE *fp;
	float tmat[4][4], vec[3], omat[3][3], fac;
	int totlamp=0;
	
	/* tellen */
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort==2) totlamp++;
		}
		base= base->next;
	}

	
	fp= fopen(str,"w");
	if(fp==NULL) return;

	fprintf(fp, "3DG2\n");
	fprintf(fp, "%d\n", totlamp);
	
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort==2) {

				la= (struct LaData *)base->d;
				
				fprintf(fp, "%d\n", la->soort);
				
				vec[0]= la->spsi;
				vec[0]= 180.0 - vec[0]*180.0/255.0;
				vec[1]= la->spbl;
				vec[1]/= (255-la->spsi);
				fprintf(fp, "%f %f\n", vec[0], vec[1]);
				
				vec[0]= la->r/255.0;
				vec[1]= la->g/255.0;
				vec[2]= la->b/255.0;
				fprintf(fp, "%f %f %f %f\n", vec[0], vec[1], vec[2], la->energy);

				parentbase(base, tmat, omat);
				fac= 1.0/5000.0;
				
				fprintf(fp, "%f %f %f\n", fac*tmat[3][0], fac*tmat[3][1], fac*tmat[3][2]);	/* loc */
				fprintf(fp, "%f %f %f\n", tmat[2][0], tmat[2][1], tmat[2][2]);	/* rot */
				
			}
		}
		base= base->next;
	}
	fclose(fp);
}

#define KNOTSU(nu)	    ( (nu)->orderu+ (nu)->pntsu+ (nu->orderu-1)*((nu)->flagu & 1) )
#define KNOTSV(nu)	    ( (nu)->orderv+ (nu)->pntsv+ (nu->orderv-1)*((nu)->flagv & 1) )

void schr_videoscape_nurbs(struct Base *base, char *str)
{
	struct ObData *ob;
	struct CurveData *cu;
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	FILE *fp;
	float tmat[4][4], vec[3], omat[3][3], fac;
	int a, tot, totnurb=0;
	
	ob= (struct ObData *)base->d;
	cu= ob->cu;
	
	parentbase(base, tmat, omat);
	fac= cu->ws;
	Mat4MulFloat3(tmat, fac);
	fac= 1.0/5000.0;
	Mat4MulFloat(tmat, fac);

	/* tellen */
	nu= cu->curve.first;
	while(nu) {
		totnurb++;
		nu= nu->next;
	}

	
	fp= fopen(str,"w");
	if(fp==NULL) return;

	fprintf(fp, "3DG3\n");
	fprintf(fp, "%d\n", base->soort);
	fprintf(fp, "%d\n", totnurb);
	fprintf(fp, "%d %d\n", cu->ext1, cu->ext2);

	for(a=0; a<4; a++) fprintf(fp, "%e %e %e %e\n", tmat[a][0], tmat[a][1], tmat[a][2], tmat[a][3]);

	nu= cu->curve.first;
	while(nu) {
		
		fprintf(fp, "%d\n", nu->type);
		fprintf(fp, "%d %d\n", nu->pntsu, nu->pntsv);
		fprintf(fp, "%d %d\n", nu->resolu, nu->resolv);
		fprintf(fp, "%d %d\n", nu->orderu, nu->orderv);
		fprintf(fp, "%d %d\n", nu->flagu, nu->flagv);
		
		if(nu->bezt) {
			a= nu->pntsu;
			bezt= nu->bezt;
			while(a--) {
				fprintf(fp, "%f %f %f ", bezt->vec[0][0], bezt->vec[0][1], bezt->vec[0][2]);
				fprintf(fp, "%f %f %f ", bezt->vec[1][0], bezt->vec[1][1], bezt->vec[1][2]);
				fprintf(fp, "%f %f %f ", bezt->vec[2][0], bezt->vec[2][1], bezt->vec[2][2]);
				fprintf(fp, "%d %d\n", bezt->h1, bezt->h2);
				bezt++;
			}
		}
		else if(nu->bp) {
			a= nu->pntsu*nu->pntsv;
			bp= nu->bp;
			while(a--) {
				fprintf(fp, "%f %f %f %f\n", bp->vec[0], bp->vec[1], bp->vec[2], bp->vec[3]);
				bp++;
			}
			
			if(nu->knotsu) {
				tot= KNOTSU(nu);
				for(a=0; a<tot; a++) fprintf(fp, "%f\n", nu->knotsu[a]);
			}
			if(nu->knotsv) {
				tot= KNOTSV(nu);
				for(a=0; a<tot; a++) fprintf(fp, "%f\n", nu->knotsv[a]);
			}
		}
		
		nu= nu->next;
	}
	

	fclose(fp);
}


void schr_videoscape()
{
	struct Base *base;
	int file, val, lampdone=0;
	ushort numlen;
	char str[100], head[100], tail[100];
	
	if(G.ebase) return;

	strcpy(str, G.sce);
	val= ffileselect("VIDEOSCAPE",str);
	if(val==0) return;

	if(testextensie(str,".trace")) str[ strlen(str)-6]= 0;
	if(testextensie(str,".obj")==0) strcat(str, ".obj");
	
	file= open(str,O_RDONLY);
	close(file);
	if(file>-1) if(saveover(str)==0) return;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort==1) {
				schr_videoscape_doit(base, str);
				val = stringdec(str, head, tail, &numlen);
				stringenc(str, head, tail, numlen, val + 1);
			}
			else if(base->soort==11 || base->soort==5) {
				schr_videoscape_nurbs(base, str);
				val = stringdec(str, head, tail, &numlen);
				stringenc(str, head, tail, numlen, val + 1);
			}
			else if(lampdone==0 && base->soort==2) {
				lampdone= 1;
				schr_videoscape_lamps(str);
				val = stringdec(str, head, tail, &numlen);
				stringenc(str, head, tail, numlen, val + 1);
			}
		}
		base= base->next;
	}
	
	
	/* weggooien als nog hogere nummers bestaan */
	while(remove(str)==0) {
		
		val = stringdec(str, head, tail, &numlen);
		stringenc(str, head, tail, numlen, val + 1);
	}
}

void schrijfscene(dir)
char *dir;
{
	struct Base *base;
	struct ObData *ob;
	struct PolyData *po;
	struct VectorFont *vf;
	struct VV *vv;
	struct Bezier *bez;
	long file,len;
	short b, curs, fout=0;
	char di[100], astr[200], tempname[100], *home;
	
	tobl=0;
	len=strlen(dir);
	if(len==0) return;
	if(dir[len-1]=='/') return;

	strcpy(di,dir);
	if(testextensie(di,".trace")==0) strcat(di,".trace");

	if(strcmp(dir, "/usr/tmp/core.trace")!=0) {
		file=open(di,O_RDONLY);
		close(file);
		if(file>-1) if(saveover(di)==0) return;

		if(G.ebase) {
			if(G.ebase->soort==1) load_ebasedata();
			else if ELEM3(G.ebase->soort, 5, 11, -2) load_ebaseNurb();
			else if(G.ebase->soort== -4) {
				load_ebaseIka();
				make_ebaseIka();
			}
		}
	}
	
	/* BEVEILIGING */
	strcpy(tempname, di);
	strcat(tempname, "@"); 
	
	file=open(tempname,O_WRONLY+O_CREAT+O_TRUNC);
	if(file== -1) {
		error("Can't write file");
		return;
	}

	curs= getcursorN();
	if(curs!=2) setcursorN(2);	/* pas op: doet winset */

	fchmod(file,0664);
	bgnwrite(file);
	strcpy(G.sce,di);

	strcpy(astr,"FORM    TRI0TOBL");
	mywrite(astr,16);
	len=4;
	mywrite(&len,4);
	mywrite(&len,4); /* hier komt aantal baseblokken */

	schr_view(file);
	schr_global(file);
	schr_stri(file);

	len=WRLD;
	mywrite(&len,4);
	len=100;
	mywrite(&len,4);
	mywrite(&wrld,100);

	base=G.firstbase;
	while(base) {
		schr_base(file,base);
		base=base->next;
	}

	if(G.totex) {
		len=TEXN;
		mywrite(&len,4);
		len=(G.totex*152);
		mywrite(&len,4);
		for(b=1;b<=G.totex;b++) {
			mywrite(G.adrtex[b],152);

		}
		for(b=1;b<=G.totex;b++) {
			if(G.adrtex[b]->soort==5) {  /* elk blok maar 1 keer schrijven */
				if(G.adrtex[b]->ima && G.adrtex[b]->ima->us > 0) {
					schr_data(file,G.adrtex[b]->ima,sizeof(struct Imap));
					G.adrtex[b]->ima->us*= -1;
				}
			}
		}
		for(b=1;b<=G.totex;b++) {
			if(G.adrtex[b]->soort==5) {
				if(G.adrtex[b]->ima && G.adrtex[b]->ima->us<0)
					G.adrtex[b]->ima->us*= -1;
			}
		}
	}

	vf= (struct VectorFont *)G.vfbase.first;
	while(vf) {
		len= VFNT;
		mywrite(&len,4);
		strcpy(astr, vf->name);
		len= (strlen(astr)+3) & 0xFC;
		len+= 4;
		mywrite(&len, 4);
		mywrite(&vf, 4);
		len-= 4;
		mywrite(astr, len);
		tobl++;
		vf= vf->next;
	}

	base=G.firstbase;
	while(base) {
		if(base->soort==1) {
			ob=(struct ObData *)base->d;
			vv=ob->vv;
			if(vv) if(vv->f1 & 1) vv->f1--;
		}
		if(base->soort==7) {
			ob=(struct ObData *)base->d;
			po=ob->po;
			if(po) if(po->f & 1) po->f--;
		}
		base=base->next;
	}

	/* NIEUW */
	if(G.timeipo) {
		bez= G.timeipo;
		len= sizeof(struct Bezier)+bez->cp*36;
		schr_data(file, G.timeipo, len);
	}

	home = getenv("HOME");

	if(home) {
		strcpy(astr, home);
		strcat(astr, "/.T.trace");
		if(strcmp(astr, di)==0) {
			len= USER;
			mywrite(&len, 4);
			len= sizeof(struct UserDef)+4;
			mywrite(&len,4);
			mywrite(&User, len-4);
		}
	}
	
	endwrite();

	len=ENDT;
	write(file, &len,4);
	len=0;
	if(write(file, &len,4)!=4) {
		error("Not enough diskspace");
		fout= 1;
	}

	len=lseek(file,0L,2)-12;
	lseek(file,4L,0);
	write(file, &len,4);
	lseek(file,20L,0);
	write(file, &tobl,4);
	close(file);

	/* EINDE BEVEILIGING */
	if(!fout) {
		sprintf(astr, "mv -f %s %s\n", tempname, di);
		if(system(astr) < 0) {
			error("Can't change old file. File saved with @");
		}
	}
	else remove(tempname);

	if(curs!=2) setcursorN(curs);
}

void savecore(sig)
int sig;
{
	static int firsttime= 1;
	
	if(firsttime) {
	
		if(strcmp(G.sce, "/usr/tmp/core.trace")==0) {
			printf("Traces killed.\n");
			exit(1);
		}
		if(G.background) {
			printf("\nSegFault or BusError!! Traces killed.\n\n");
			exit(1);
		}
		printf("CORE!! Try to save file to /usr/tmp/core.trace....\n");
		
		strcpy(G.sce, "/usr/tmp/core.trace");
		G.f |= 3;	/* geen okee(), error() print in console */
		schrijfscene(G.sce);
		
		printf(".... saved!\n");
		firsttime= 0;
	}
	sigset(SIGBUS, SIG_DFL);		/* ongedaan maken van aanroepen savecore */
	sigset(SIGSEGV, SIG_DFL);

}

