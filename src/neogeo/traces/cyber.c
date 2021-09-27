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
#include "people:include/util.h"
#include "people:arjan/c/arjan.h"
#include <gl/device.h>
#define facos acos
#define fsqrt sqrt

#endif

#define PI M_PI/180
#define nil 0


/* ****************************************************************************** */
/* *************************** leesCYBER **************************************** */
/* ****************************************************************************** */

/*
 * sx aantal samples in x richting (1 op de sx punten)
 * sy aantal samples in y richting (1 op de sy punten)
 */

void leesCYBER(char *name, int sx, int sy, struct ListBase *dispbase)
{
	long fp;
	char *fil, *rgl, tmp;
	struct DispList *dl;
	int x, y, ofs;
	short z;
	int max_x, max_y, r_min, r_max, form=0;
	long size;
	float *data, angle, r_dif, xy_dif;
	
/* ************ inlezen bestand ************ */
	
	fp=open(name,O_RDONLY);
    if (fp==-1)
    { 
		printf ("CYBER : error opening file\n"); 
		return;
    }
    size=lseek(fp, 0, SEEK_END);
    lseek(fp, 0, SEEK_SET);
    fil=(char *)malloc (size+1);
    read(fp,fil,size);
    close (fp);
    rgl=fil;

/* ************ ophalen belangrijke parameters ************/

													/* Cyberware Digitizer Data					*/
													/* NAME=									*/
													/* DATE=Wed Dec 31 16:00:00 1969			*/
													/* SPACE=CYLINDRICAL						*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "SPAC")!=0) 
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=6;
	tmp= *(rgl+11);
	*(rgl+11)=0;
	if (strcmp(rgl, "CYLINDRICAL")==0) form=1;
	*(rgl+11)=tmp;
													/* NLG=256									*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "NLG=")!=0) 
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=4;
	max_x=atoi(rgl);
													/* LGINCR=24544								*/
													/* LGMIN=0									*/
													/* LGMAX=255								*/
													/* NLT=256									*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "NLT=")!=0)
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=4;
	max_y=atoi(rgl);
													/* LTINCR=1559								*/
													/* LTMIN=0									*/
													/* LTMAX=255								*/
													/* RMIN=28096								*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "RMIN")!=0)
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=5;
	r_min=atoi(rgl);
													/* RMAX=124352								*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "RMAX")!=0)
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=5;
	r_max=atoi(rgl);
													/* RSHIFT=5									*/
													/* LGSHIFT=8								*/
													/* SCALE=100.00								*/
													/* RPROP=100.00								*/
													/* SMOOTHED=1								*/
													/* DATA=									*/
	tmp= *(rgl+4);
	*(rgl+4)=0;
	while (strcmp(rgl, "DATA")!=0) 
	{
		*(rgl+4)=tmp;
		rgl++;
		tmp= *(rgl+4);
		*(rgl+4)=0;
	}
	*(rgl+4)=tmp;
	rgl+=6;

/* ************ berekenen variabelen *********** */
						
	angle=(360.0/max_x)*PI;
	r_dif=(r_max-r_min)/32768;
	xy_dif=((float)max_y/(float)max_x)*((float)r_max/(float)max_y);
	ofs=0;
	
/* ************ opzetten DispList ********** */

	dl=(struct DispList *)malloc (sizeof(struct DispList)+((max_x/sx)*(max_y/sy)*3*sizeof(float)));
	addtail(dispbase, dl);
	dl->type= DL_SURF;
	dl->parts=max_x/sx;
	dl->nr=max_y/sy;
	dl->col=6;
	data=(float *)(dl+1);

/* ************ invullen DispList ************ */	
    
	switch (form)
	{
		case 0 :
		
			for (x=0;x<max_x/sx;x++)
			{
				for (y=0;y<max_y/sy;y++)
				{
					z=rgl[ofs+(y*sy*2)]<<8|rgl[ofs+(y*sy*2)+1];
					if (z<0) z=0;
					*(data)=x<<1;
					*(data+1)=y;
					*(data+2)=(z*r_dif)+r_min; 
					data+=3;				
				}
				ofs+=(sx)*max_y*2;
			}
			break;
		case 1 :	
			for (x=0;x<max_x/sx;x++)
			{
				for (y=0;y<max_y/sy;y++)
				{
					z=(rgl[ofs+(y*sy*2)]<<8) |rgl[ofs+(y*sy*2)+1];
					if (z<0) z=0;
					*(data)=cos(angle*(x*sx))*((z*r_dif)+r_min);
					*(data+1)= y*xy_dif;
					*(data+2)=sin(angle*(x*sx))*((z*r_dif)+r_min); 
					data+=3;
				}
				ofs+=(sx)*max_y*2;
			}
			break;
	}
	free (fil);
}

