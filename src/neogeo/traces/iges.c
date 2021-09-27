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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define REMOVE 0xAD

#ifndef AMIGA

#include <unistd.h>
#include <device.h>
#include "/usr/people/arjan/c/arjan.h"
#include "/usr/people/include/util.h"

#else

#define SEEK_SET 0
#define SEEK_END 2
#define M_PI 3.1415926535
#define BLACK 0
#define WHITE 7
#include "people:include/util.h"
#include "people:arjan/c/arjan.h"
#include <gl/device.h>
#define facos acos
#define fsqrt sqrt

#endif

#define nil 0
#define maxread 82
#define ISAPOINTER(a)	( ((long)a)>0x10000000 && ((long)a)<0x11000000)

struct subfiglst {
	struct DispList *inh;
	struct subfiglst *next;
};

char pardelim=',',recdelim=';';
float matrix[12];
int currentmat=0;

void fout(int nummer,int regel)
{
	printf ("IGES-");
	if (nummer>100)
	{
		printf ("WARNING : Undefined object  %d (",nummer);
		switch (nummer)
		{
		case 104 : 
			printf ("CONIC ARC");				
			break;
		case 108 : 
			printf ("PLANE");				
			break;
		case 118 : 
			printf ("RULED SURFACE");			
			break;
		case 120 : 
			printf ("SURFACE OF REVOLUTION");		
			break;
		case 125 : 
			printf ("FLASH");				
			break;
		case 126 : 
			printf ("RATIONAL B SPLINE CURVE");		
			break;
		case 128 : 
			printf ("RATIONAL B SPLINE SURFACE");		
			break;
		case 130 : 
			printf ("OFFSET CURVE");				
			break;
		case 132 : 
			printf ("CONNECT POINT");			
			break;

		case 134 : 
			printf ("NODE");					
			break;
		case 136 : 
			printf ("FINITE ELEMENT");			
			break;
		case 138 : 
			printf ("NODAL DISPLACEMENT AND ROTATION");	
			break;

		case 140 : 
			printf ("OFFSET SURFACE");			
			break;
		case 142 : 
			printf ("CURVE ON PARAMETRIC SURFACE");		
			break;
		case 144 : 
			printf ("TRIMMED parametric SURFACE");		
			break;

		case 202 : 
			printf ("ANGULAR DIMENSION");			
			break;
		case 206 : 
			printf ("DIAMETER DIMENSION");			
			break;
		case 208 : 
			printf ("FLAG NOTE");				
			break;
		case 210 : 
			printf ("GENERAL LABEL");			
			break;
		case 212 : 
			printf ("GENERAL NOTE");				
			break;
		case 214 : 
			printf ("LEADER arrow");				
			break;
		case 216 : 
			printf ("LINEAR DIMENSION");			
			break;
		case 218 : 
			printf ("ORDINATE DIMENSION");			
			break;
		case 220 : 
			printf ("POINT DIMENSION");			
			break;
		case 222 : 
			printf ("RADIUS DIMENSION");			
			break;
		case 228 : 
			printf ("GENERAL SYMBOL");			
			break;
		case 230 : 
			printf ("SECTIONED AREA");			
			break;

		case 302 : 
			printf ("ASSOCIATIVITY DEFINITION");		
			break;
		case 402 : 
			printf ("ASSOCIATIVITY INSTANCE");		
			break;
		case 404 : 
			printf ("DRAWING");				
			break;
		case 304 : 
			printf ("LINE FONT DEFINITION");			
			break;
		case 306 : 
			printf ("MACRO");				
			break;
		case 406 : 
			printf ("PROPERTY");				
			break;
		case 320 : 
			printf ("NETWORK SUBFIGURE DEFINITION");		
			break;
		case 408 : 
			printf ("SUBFIGURE INSTANCE");			
			break;
		case 412 : 
			printf ("RECTANGULAR ARRAY SUBFIGURE INSTANCE");  
			break;
		case 414 : 
			printf ("CIRCULAR ARRAY SUBFIGURE INSTANCE");	
			break;
		case 420 : 
			printf ("NETWORK SUBFIGURE INSTANCE");		
			break;
		case 310 : 
			printf ("TEXT FONT DEFINITION");			
			break;
		case 410 : 
			printf ("VIEW");					
			break;
		case 416 : 
			printf ("EXTERNAL REFERENCE");			
			break;
		case 418 : 
			printf ("NODAL LOAD/CONSTRAINT");		
			break;
		case 314 : 
			printf ("COLOR DEFINITION");			
			break;
		case 312 : 
			printf ("TEXT DISPLAY TEMPLATE");		
			break;
		default : 
			printf ("Unknown");
		}
		printf (") in line %d\n", regel);
	}
	else
	{
		if (nummer>0) printf ("ERROR : ");
		switch (nummer)
		{
		case -1 : 
			printf ("READY : %d lines\n",regel);					    
			break;
		case  1 : 
			printf ("Can't open file\n");						    
			break;
		case  2 : 
			printf ("Unexpected code in line %d\n",regel);				    
			break;
		case  3 : 
			printf ("Line number incorrect in line %d\n",regel);			    
			break;
		case  4 : 
			printf ("Total number of lines incorrect\n");				    
			break;
		case  6 : 
			printf ("File  in compressed format.  Use uncompressed files only\n");	    
			break;
		case  7 : 
			printf ("Unexpected end of file\n");					    
			break;
		case  8 : 
			printf ("Pointer to matrix expected on line %d\n", regel);		    
			break;
		case  9 : 
			printf ("Pointer to subfigure definition expected on line %d\n", regel);	    
			break;
		default : 
			printf ("%d in line %d\n",nummer,regel);
		}
	}
}

float atofN(char *str)
{
	int len;

	if(str) {
		len= strlen(str)-1;
		while(len>0) {
			if(str[len]=='D') {
				str[len]='E';
				break;
			}
			len--;
		}
		return atof(str);
	}
	return 0;
}

int copystring (char **in, char *uit, int *pos, int *rnr, int *p)
{
	int t=0, end, nr;

	while (((*in)[*pos]!=pardelim) && ((*in)[*pos]!=recdelim))
	{
		uit[t++]=(*in)[(*pos)++];
		if (((*pos)>63))
		{
			*in+=maxread;
			(*pos)=0;
			(*rnr)++;
			t=0;

			(*in)[80]= 0;
			sscanf((*in)+73,"%7d",&nr);
			if (nr==1)
			{
				fout(7, *rnr);
				return(0);
			}
			(*p)++;
			if (*p!=nr)
			{
				fout(3,*rnr);
			}
		}
	}
	uit[t]='\0';
	if ((*in)[*pos]==recdelim) end=0; 
	else end=1;
	(*pos)++;
	return(end);
}


void joindisplist (struct ListBase *dispbase, struct DispList *DL, struct DispList *NW, char *Dpointer, char *rgl)
{
	long Mpointer, offset;
	struct DispList *TO;
	
	if(DL==0 || NW==0) return;
	
	offset= (long)DL;
	if( offset< 0x1000000 || offset>0x11000000) return;
	offset= (long)NW;
	if( offset< 0x1000000 || offset>0x11000000) return;
	
	DL->flag=REMOVE;
	NW->flag=REMOVE;
	TO=(struct DispList *)malloc(sizeof (struct DispList)+((DL->nr+NW->nr)*3*sizeof(float)));
	offset = sizeof (struct DispList) + (DL->nr*3*sizeof(float));
	memcpy(TO, DL, offset);
	memcpy(((char *) TO) + offset, NW + 1, NW->nr * 3 * sizeof(float));
	TO->nr+=NW->nr;
	TO->flag=0;
	addtail (dispbase, TO);

	Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
	*((long *)Mpointer)=(long)TO;
}

void tabsurf (struct ListBase *dispbase, struct DispList *DL, float x, float y, float z, char *Dpointer, char *rgl)
{
	int t;
	long Mpointer, offset;
	struct DispList *TO;
	float *data;
	float ix, iy, iz;

	DL->flag=REMOVE;

	TO=(struct DispList *)malloc(sizeof (struct DispList)+(DL->nr*6*sizeof(float)));
	offset = sizeof (struct DispList) + (DL->nr * 3 * sizeof(float));
	memcpy(TO, DL, offset);
	memcpy(((char *) TO) + offset, DL+1, DL->nr * 3 * sizeof(float));
	TO->flag=0;
	TO->parts=2;
	data=(float *)(((char *) TO) + offset);
	for (t=0;t<DL->nr;t++)
	{
		data[0]+=x;
		data++;
		data[0]+=y;
		data++;
		data[0]+=z;
		data++;
	}
	addtail (dispbase, TO);
	TO->type=DL_SURF;
	Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
	*((long *)Mpointer)=(long)TO;
}

int mtrx (int n, char *DP,  char *PP, int rnr)
{
	char *rgl, temp[40];
	int t, end, i, nr, pos=0;

	rgl=DP+(n-1)*maxread;    /* object regel in D */
	rgl[56]=0;
	t=atoi(rgl+48);          /* pointer naar matrix in D */
	if (!t) return (0);      /* geen matrix voor object */
	rgl=DP+((t-1)*maxread);  /* matrix regel in D */
	rgl[16]=0;
	t=atoi(rgl+8);           /* pointer naar matrix in P */
	if (t==currentmat) return 1;    /* reeds ingelezen matrix */
	rgl=PP+((t-1)*maxread);  /* matrix regel in P */
	nr=t;
	end=copystring(&rgl, temp, &pos, &nr, &t);
	if (strcmp(temp,"124")!=0) fout(8, rnr);
	else
	{
		currentmat=t;
		for (i=0;i<12;i++)
		{
			end=copystring (&rgl, temp, &pos, &nr, &t);				/* matriks waarden */
			matrix[i]=atofN(temp);
		}
	}
	return 1;
}

void applymatrix(float *data, int max)
{
	float x, y, z, t;

	for (t=0;t<max;t++)
	{
		x=(data[0]*matrix[0])+(data[1]*matrix[1])+(data[2]*matrix[2])+matrix[3];
		y=(data[0]*matrix[4])+(data[1]*matrix[5])+(data[2]*matrix[6])+matrix[7];
		z=(data[0]*matrix[8])+(data[1]*matrix[9])+(data[2]*matrix[10])+matrix[11];
		data[0]=x;
		data[1]=y;
		data[2]=z;
		data+=3;
	}
}

void subsubfig (struct ListBase *dispbase, char *rgl, int rnr, float x, float y, float z,  float s, char *DP, char *PP, struct
subfiglst **sub)
{
	long Mpointer, offset;
	struct DispList *dl, *TO;
	struct subfiglst *old;
	float *data;
	int a;

	Mpointer=((long)DP+(rnr*maxread)+17)+3&~3;				/* Pointer naar item list */
	old=(struct subfiglst *)*((long *)Mpointer);

	while (old!=nil)
	{
		if(old==(struct subfiglst *)0x20202020)
		{
			fout(9, rnr);
			return;
		}
		dl=old->inh;
		TO=(struct DispList *)malloc(sizeof (struct DispList)+(dl->nr*dl->parts*3*sizeof(float)));	/* Copieren display-list-blokje */
		offset = sizeof (struct DispList) + (dl->nr*dl->parts*3*sizeof(float));
		memcpy(TO, dl, offset);
		addtail (dispbase, TO);
		dl->flag=REMOVE;
		TO->flag=0;
		if ((*sub)==nil)
		{
			(*sub)=(struct subfiglst *)malloc (sizeof(struct subfiglst));
			(*sub)->next=nil;
			(*sub)->inh=TO;
			Mpointer=((long)DP+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)(*sub);
		}
		else
		{
			(*sub)->next=(struct subfiglst *)malloc (sizeof(struct subfiglst));
			(*sub)=(*sub)->next;
			(*sub)->next=nil;
			(*sub)->inh=TO;
		}
		data=(float *)(TO+1);						/* Pointer naar data gedeelte */
		for (a=0;a<dl->nr*dl->parts;a++)
		{
			data[0]*=s;
			data[0]+=x;
			data++;
			data[0]*=s;
			data[0]+=y;
			data++;
			data[0]*=s;
			data[0]+=z;
			data++;
		}
		if (mtrx(atoi(rgl+64), DP, PP, rnr)) applymatrix ((float *)(TO+1), dl->nr*dl->parts);
		old=old->next;

	}
}

void subfig (struct ListBase *dispbase, char *rgl, char *DP, char *PP, int NR, float x, float y, float z, float s, int *rnr,
int p)
{
	char temp[80];
	char *rg, *rl;
	int end, t, a, pos=0, max, rr;
	float *data;
	struct DispList *TO, *dl;
	long Mpointer, offset;
	struct subfiglst *sub;

	rg=DP+(NR-1)*maxread;								/* Pointer naar definition in D */
	rg[16]=0;
	rr=atoi(rg+8);
	rl=PP+(rr-1)*maxread;
	t=2;
	sub=nil;
	end=copystring (&rl, temp, &pos, &t, &rr);					/* Controle op type 308 (subfigure definition) */
	if (strcmp(temp,"308")!=0) fout(9, *rnr);
	else
	{
		end=copystring (&rl, temp, &pos, &t, &rr);				/* Depth */

		end=copystring (&rl, temp, &pos, &t, &rr);				/* Name subfigure */
		end=copystring (&rl, temp, &pos, &t, &rr);				/* Number of items */
		max=atoi(temp);
		for (t=0;t<max;t++)
		{
			end=copystring (&rl, temp, &pos, &t, &rr);				/* Item t */
			NR=atoi(temp);
			rg=DP+(NR-1)*maxread;								/* Pointer naar definition in D */
			rg[8]=0;
			end=atoi(rg);
			if (end!=408)
			{
				Mpointer=((long)DP+(NR*maxread)+17)+3&~3;				/* Pointer naar item */
				dl=(struct DispList *)*((long *)Mpointer);
				if (dl!=nil && ISAPOINTER(dl))
				{
					TO =(struct DispList *)malloc(sizeof (struct DispList)+(dl->nr*dl->parts*3*sizeof(float)));	/* Copieren display-list-blokje */
					offset = sizeof (struct DispList) + (dl->nr*dl->parts*3*sizeof(float));
					memcpy(TO, dl, offset);
					addtail (dispbase, TO);
					dl->flag=REMOVE;
					TO->flag=0;
					if (sub==nil)
					{
						sub=(struct subfiglst *)malloc (sizeof(struct subfiglst));
						sub->next=nil;
						sub->inh=TO;
						Mpointer=((long)DP+(atol(rgl+64)*maxread)+17)+3&~3;
						*((long *)Mpointer)=(long)sub;
					}
					else
					{
						sub->next=(struct subfiglst *)malloc (sizeof(struct subfiglst));
						sub=sub->next;
						sub->next=nil;
						sub->inh=TO;
					}
					data=(float *)(TO+1);						/* Pointer naar data gedeelte */
					for (a=0;a<dl->nr*dl->parts;a++)
					{
						data[0]*=s;
						data[0]+=x;
						data++;
						data[0]*=s;
						data[0]+=y;
						data++;
						data[0]*=s;
						data[0]+=z;
						data++;
					}
					if (mtrx(atoi(&rl[64]), DP, PP, NR)) applymatrix ((float *)(TO+1), dl->nr*dl->parts);
					if (mtrx(atoi(PP+(p-1)*maxread+64), DP, PP, NR)) applymatrix ((float *)(TO+1), dl->nr*dl->parts);
				}
				else
				{
					Mpointer=((long)DP+(atol(rgl+64)*maxread)+17)+3&~3;
					*((long *)Mpointer)=(long)nil;
				}
			}

			else subsubfig (dispbase, rgl, NR, x, y, z, s, DP, PP, &sub);
		}
	}
}

void cleandisplist(struct ListBase *dispbase)
{
	struct DispList *dl, *next;

	dl=dispbase->first;

	while (dl) {
		next= dl->next;
		if (dl->flag==REMOVE) {
			remlink(dispbase, dl);
			free(dl);
		}
		dl= next;
	}
}

void leesIGES(char *str, struct ListBase *dispbase)
{
	float *data;
	int end, nr, rnr=0, t, pos;
	int s=0, g=0, d=0, p=0;
	char temp[80], *rgl, *fil;
	struct DispList *dl;
	char *Ppointer, *Dpointer;
	int ia, ib, ic, id, ie;
	float fl, x, y, z, a, sa, ea, r;
	float fa[16];
	long size;
	long fp;
	long Mpointer;

	fp=open(str,O_RDONLY);
	if (fp==-1)
	{
		fout(1,rnr);
		return;
	}
	size=lseek(fp, 0, SEEK_END);
	t=lseek(fp, 0, SEEK_SET);
	fil=(char *)malloc (size);
	end=read(fp,fil,size);
	close (fp);
	rgl=fil;

	/* C C C C C C C C C C C C C C C C C C C C C C C C C C C C C C C C C */

	if (rgl[72]=='C')
	{
		fout(6, ++rnr);
		return;
	}

	/* S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S */

	while ((rgl[72]=='S') && end)
	{
		rgl[80]=0;
		sscanf(rgl+73,"%7d",&nr);
		s++;
		rnr++;
		if (s!=nr) fout(3,rnr);
		rgl+=maxread;
		printf("found S\n");
	}

	/* G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G */

	if (rgl[0]==',')
	{
		pardelim=',';
		if (rgl[1]==',') recdelim=';'; 
		else recdelim=rgl[3];
	}
	else
	{
		pardelim=rgl[2];
		if (rgl[4]==',') recdelim=';'; 
		else recdelim=rgl[6];
	}

	while ((rgl[72]=='G') && end)
	{
		rgl[80]=0;
		sscanf(rgl+73,"%7d",&nr);
		g++;
		rnr++;
		if (g!=nr)
		{
			fout(3,rnr);
			pardelim=',';
			recdelim=';';
		}
		rgl+=maxread;
		printf("found G\n");
	}

	/* D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D */

	Dpointer=rgl;
	while ((rgl[72]=='D') && end)
	{
		rgl[80]=0;
		sscanf(rgl+73,"%7d",&nr);
		d++;
		rnr++;
		if (d!=nr)
		{
			fout(3,rnr);
			return;
		}
		t=0;
		rgl+=maxread;
	}

	/* P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P */

	Ppointer=rgl;
	while ((rgl[72]=='P') && end)
	{
		rgl[80]=0;
		sscanf(rgl+73,"%7d",&nr);
		p++;
		rnr++;
		if (p!=nr)
		{
			fout(3,rnr);
			return;
		}
		pos=0;
		copystring (&rgl, temp, &pos, &rnr, &p);
		t=atoi(temp);
		switch (t)
		{

			/* cirkel(boog) */
		case 100 : 
			dl=(struct DispList *)malloc (sizeof(struct DispList)+(32*3*sizeof(float)));
			addtail(dispbase, dl);
			dl->type= DL_SEGM;
			dl->parts= 1;
			dl->nr= 32;
			dl->col=1;
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)dl;
			data=(float *)(dl+1);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z */
			z=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			x=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			y=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			fa[0]=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			fa[1]=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			fa[2]=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			fa[3]=atofN(temp);
			r=fsqrt(((fa[0]-x)*(fa[0]-x))+((fa[1]-y)*(fa[1]-y)));
			sa=facos((fa[0]-x)/r);
			r=fsqrt(((fa[2]-x)*(fa[2]-x))+((fa[3]-y)*(fa[3]-y)));
			ea=facos((fa[2]-x)/r);
			while (ea<=sa) ea+=2;
			fa[0]=(ea-sa)/31;
			for (a=sa, t=0;t<32;a+=fa[0], t++)
			{
				fa[1]=x+(r*cos(a*M_PI));
				data[0]=fa[1];
				data++;
				fa[2]=y+(r*sin(a*M_PI));
				data[0]=fa[2];
				data++;
				data[0]=z;
				data++;
			}
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), 32);
			break;

			/* composit curve */
		case 102 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Aantal onderdelen */
			ia=atoi(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Eerste onderdeel */
			Mpointer=((long)Dpointer+(atol(temp)*maxread)+17)+3&~3;
			dl=(struct DispList *)*((long *)Mpointer);
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;		/* Alvast opbergen */
			*((long *)Mpointer)=(long)dl;
			while (end)
			{
				end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Onderdeel n */
				Mpointer=((long)Dpointer+(atol(temp)*maxread)+17)+3&~3;
				joindisplist (dispbase, dl,(struct DispList *)*((long *)Mpointer), Dpointer, rgl);

				Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;		    /* resultaat ophalen */
				dl=(struct DispList *)*((long *)Mpointer);
			}
			if( ISAPOINTER(dl)) {
				dl->col=2;
				if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), dl->nr*dl->parts);
			}
			break;

			/* copius data */
		case 106 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);    /* Soort punten */
			ia=atoi(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Aantal punten */
			ib=atoi(temp);
			t=atoi(Dpointer+(atoi(rgl+64)*maxread)+32);
			if ((t==11) || (t==12) || (t==13) || (t==63))
			{
				dl=(struct DispList *)malloc (sizeof(struct DispList)+(ib*3*sizeof(float)));
				addtail(dispbase, dl);
				dl->type= DL_SEGM;
				dl->parts= 1;
				dl->nr= ib;
				switch (t)
				{
				case 11 : 
					dl->col=252;	
					break;
				case 12 : 
					dl->col=150;	
					break;
				case 13 : 
					dl->col=187;	
					break;
				case 63 : 
					dl->col=3;
				}
				Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
				*((long *)Mpointer)=(long)dl;
				data= (float *)(dl+1);
				switch (ia)
				{
				case 1 :    
					end=copystring (&rgl, temp, &pos, &rnr, &p);          /* coordinaten */
					z=atofN(temp);
					for (t=0;t<ib;t++)
					{
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* X-coordinaten */
						data[0]=atofN(temp);
						data++;
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Y-coordinaten */
						data[0]=atofN(temp);
						data++;
						data[0]=z;						/* Z-coordinaten */
						data++;
					}
					break;
				case 2 :    
					for (t=0;t<ib;t++)
					{
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* X-coordinaten */
						data[0]=atofN(temp);
						data++;
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Y-coordinaten */
						data[0]=atofN(temp);
						data++;
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Z-coordinaten */
						data[0]=atofN(temp);
						data++;
					}
					break;
				case 3 :    
					for (t=0;t<ib;t++)
					{
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* X-coordinaten */
						data[0]=atofN(temp);
						data++;
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Y-coordinaten */
						data[0]=atofN(temp);
						data++;
						end=copystring (&rgl, temp, &pos, &rnr, &p);          /* Z-coordinaten */
						data[0]=atofN(temp);
						data++;
						for (ic=0;ic<3;ic++)
						{
							end=copystring (&rgl, temp, &pos, &rnr, &p);          /* skip vector */
						}
					}
				}
				if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), ib);
			}
			else
			{
				while (copystring(&rgl, temp, &pos, &rnr, &p));
				Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
				*((long *)Mpointer)=(long)nil;
			}
			break;

			/* lijn */
		case 110 : 
			dl=(struct DispList *)malloc (sizeof(struct DispList)+(2*3*sizeof(float)));
			addtail(dispbase, dl);
			dl->type= DL_SEGM;
			dl->parts= 1;
			dl->nr= 2;
			dl->col=4;
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)dl;
			data= (float *)(dl+1);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			fl=atofN(temp);
			data[0]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			fl=atofN(temp);
			data[1]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z */
			data[2]=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			fl=atofN(temp);
			data[3]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			fl=atofN(temp);
			data[4]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z */
			data[5]=atofN(temp);
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), 2);
			break;

			/* parametric spline */
		case 112 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* spline type */
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* degree of continuity */
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* planar (true/false) */
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* number of segments */
			ia=atoi(temp);
			for (t=0;t<=ia;t++) end=copystring (&rgl, temp, &pos, &rnr, &p);		/* breakpoints */
			dl=(struct DispList *)malloc (sizeof(struct DispList)+((ia+1)*3*sizeof(float)));
			addtail(dispbase, dl);
			dl->type= DL_SURF;
			dl->parts= 1;
			dl->nr= ia;
			dl->col=5;
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)dl;
			data= (float *)(dl+1);
			for (ic=0;ic<ia;ic++)
			{
				for (ie=0;ie<3;ie++)
				{
					for (t=0;t<4;t++)
					{
						if (end) end=copystring (&rgl, temp, &pos, &rnr, &p);				/* coefficient (a..d) */
						if (end) fa[t]=atofN(temp); 
						else fa[t]=0;
					}
					data[0]=fa[0];
					data[3]=fa[0]+fa[1];
					fa[0]=data[0];
					data++;
				}
			}
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), ia);
			break;

			/* parametric spline surface */
		case 114 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* boundary type */
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* patch type */
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* number of U segments */
			ia=atoi(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* number of V segments */
			ib=atoi(temp);
			for (t=0;t<=ia;t++) end=copystring (&rgl, temp, &pos, &rnr, &p);		/* breakpoints U */
			for (t=0;t<=ib;t++) end=copystring (&rgl, temp, &pos, &rnr, &p);		/* breakpoints V */
			dl=(struct DispList *)malloc (sizeof(struct DispList)+((ia+1)*(ib+1)*3*sizeof(float)));
			addtail(dispbase, dl);
			dl->type= DL_SURF;
			dl->parts= ia+1;
			dl->nr= ib+1;
			dl->col=6;
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)dl;
			data= (float *)(dl+1);
			for (ic=0;ic<ia;ic++)
			{
				for (id=0;id<ib;id++)
				{
					for (ie=0;ie<3;ie++)
					{
						for (t=0;t<16;t++)
						{
							if (end) end=copystring(&rgl, temp, &pos, &rnr, &p);
							if (end) fa[t]=atofN(temp); 
							else fa[t]=0;
						}

						data[(((ic  )*(ib+1))+id  )*3]=fa[0];
						/* if ((ic==ia-1) || (id==ib-1))
				    { */
						data[(((ic+1)*(ib+1))+id  )*3]=fa[0]+fa[1]+fa[2]+fa[3];
						data[(((ic  )*(ib+1))+id+1)*3]=fa[0]+fa[4]+fa[8]+fa[12];
						data[(((ic+1)*(ib+1))+id+1)*3]=fa[0]+fa[1]+fa[2]+fa[3]+fa[4]+fa[5]+fa[6]+
						    fa[7]+fa[8]+fa[9]+fa[10]+fa[11]+fa[12]+fa[13]+fa[14]+fa[15];
						/* } */
						fl=data[0];
						data++;
					}
					data-=3;
				}
				if (ic<ia-1) for (t=0;t<48;t++) if (end) end=copystring(&rgl, temp, &pos, &rnr, &p); /* skip empty space */

			}
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), ia*ib);
			break;

			/* point */
		case 116 : 
			dl=(struct DispList *)malloc (sizeof(struct DispList)+(1*3*sizeof(float)));
			addtail(dispbase, dl);
			dl->type= DL_POINT;
			dl->parts= 1;
			dl->nr= 1;
			dl->col=7;
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)dl;
			data= (float *)(dl+1);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			fl=atofN(temp);
			data[0]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			fl=atofN(temp);
			data[1]=fl;
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z */
			data[2]=atofN(temp);
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), 1);
			break;

			/* tabulated cylinder */
		case 122 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Pointer to directrix */
			Mpointer=((long)Dpointer+(atol(temp)*maxread)+17)+3&~3;
			dl=(struct DispList *)*((long *)Mpointer);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X */
			x=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y */
			y=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z */
			z=atofN(temp);
			data=(float *)(dl+1);
			x-=data[0];
			y-=data[1];
			z-=data[2];
			if (dl->nr==2) z*=0.5;
			tabsurf (dispbase, dl, x, y, z, Dpointer, rgl);
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;		    /* resultaat ophalen */
			dl=(struct DispList *)*((long *)Mpointer);
			dl->col=9;
			if (mtrx(atoi(&rgl[64]), Dpointer, Ppointer, rnr)) applymatrix ((float *)(dl+1), dl->nr*dl->parts);
			break;
			/* transformatie matrix */
		case 124 : 
			currentmat=p;
			for (ie=0;ie<12;ie++)
			{
				end=copystring (&rgl, temp, &pos, &rnr, &p);				/* matriks waarden */
				matrix[ie]=atofN(temp);
			}
			break;

			/* subfigure definition */
		case 308 : 
			if (end) while (copystring (&rgl, temp, &pos, &rnr, &p));		/* skip to end of data */
			break;

			/* subfigure instance */
		case 408 : 
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Pointer to subfigure definition (308) */
			ia=atoi(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* X translation */
			x=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Y translation */
			y=atofN(temp);
			end=copystring (&rgl, temp, &pos, &rnr, &p);				/* Z translation */
			z=atofN(temp);
			if (end)
			{
				end=copystring (&rgl, temp, &pos, &rnr, &p);			/* Scale factor */
				fa[0]=atofN(temp);
			}
			else fa[0]=1;
			subfig (dispbase, rgl, Dpointer, Ppointer, ia, x, y, z, fa[0], &rnr, p);
			break;

		default  : 
			fout (t, rnr);
			if (end) while (copystring (&rgl, temp, &pos, &rnr, &p)); /* skip to end of data */
			Mpointer=((long)Dpointer+(atol(rgl+64)*maxread)+17)+3&~3;
			*((long *)Mpointer)=(long)nil;
		}
		rgl+=maxread;
		end=1;
	}
	cleandisplist (dispbase);

	/* T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T */

	if ((rgl[72]=='T') && end)
	{
		rnr++;
		for (t=0;rgl[t]!=' ';t+=8)
		{
			rgl[80]=0;
			sscanf(rgl+t+1,"%7d",&nr);
			switch (rgl[t])
			{
			case 'G' : 
				if (nr=g) g=0;
			case 'D' : 
				if (nr=d) d=0;
			case 'P' : 
				if (nr=p) p=0;
			}
		}
		if ((g+d+p)!=0)
		{
			fout(4,rnr);
			return;
		}
		else fout(-1,rnr);
		return;
	}
	else fout(2,rnr+1);
	free (fil);
}

