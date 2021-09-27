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

#include "/usr/people/frank/source/iff.h"

#include "/usr/people/include/util.h"
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <sys/syssgi.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/statfs.h>

/*
cc -O3 noise.o code.o jit.c -lgl_s -lfm_s -lm -o jit

*/

extern decodekeytab();
float rad,rad1;

short keycode[100]= {
	0,LEFTCTRLKEY,RIGHTSHIFTKEY,LEFTSHIFTKEY,TABKEY,RETKEY
,LEFTARROWKEY,DOWNARROWKEY,RIGHTARROWKEY,UPARROWKEY,LEFTMOUSE
,MIDDLEMOUSE,F1KEY,F2KEY,F3KEY,F4KEY
,F5KEY,F6KEY,F7KEY,F8KEY,F9KEY,F10KEY
,F11KEY,F12KEY,AKEY,BKEY,CKEY
,DKEY,EKEY,FKEY,GKEY,HKEY
,IKEY,JKEY,KKEY,LKEY,MKEY
,NKEY,OKEY,PKEY,QKEY,RKEY
,SKEY,TKEY,UKEY,VKEY,WKEY
,XKEY,YKEY,ZKEY,ZEROKEY,ONEKEY
,TWOKEY,THREEKEY,FOURKEY,FIVEKEY,SIXKEY
,SEVENKEY,EIGHTKEY,NINEKEY,PAD0,PAD1
,PAD2,PAD3,PAD4,PAD5,PAD6
,PAD7,PAD8,PAD9,LEFTALTKEY,RIGHTALTKEY
,RIGHTCTRLKEY,PAUSEKEY,INSERTKEY,HOMEKEY,PAGEUPKEY
,ENDKEY,PAGEDOWNKEY,NUMLOCKKEY,PADVIRGULEKEY,PADASTERKEY
,PADPLUSKEY, SPACEKEY};


/*

#define LEFTCTRLKEY	keycode[1]
#define RIGHTSHIFTKEY	keycode[2]
#define LEFTSHIFTKEY	keycode[3]
#define TABKEY		keycode[4]
#define RETKEY		keycode[5]
#define LEFTARROWKEY	keycode[6]
#define DOWNARROWKEY	keycode[7]
#define RIGHTARROWKEY	keycode[8]
#define UPARROWKEY	keycode[9]
#define LEFTMOUSE	keycode[10]
#define MIDDLEMOUSE	keycode[11]
#define	F1KEY 		keycode[12]
#define	F2KEY 		keycode[13]
#define	F3KEY 		keycode[14]
#define	F4KEY 		keycode[15]
#define	F5KEY 		keycode[16]
#define	F6KEY 		keycode[17]
#define	F7KEY 		keycode[18]
#define	F8KEY 		keycode[19]
#define	F9KEY 		keycode[20]
#define	F10KEY		keycode[21]
#define	F11KEY		keycode[22]
#define	F12KEY		keycode[23]

#define AKEY		keycode[24]
#define BKEY		keycode[25]	
#define CKEY		keycode[26]	
#define DKEY		keycode[27]	
#define EKEY		keycode[28]	
#define FKEY		keycode[29]
#define GKEY		keycode[30]
#define HKEY		keycode[31]
#define IKEY		keycode[32]
#define JKEY		keycode[33]
#define KKEY		keycode[34]
#define LKEY		keycode[35]
#define MKEY		keycode[36]
#define NKEY		keycode[37]
#define OKEY		keycode[38]
#define PKEY		keycode[39]
#define QKEY		keycode[40]
#define RKEY		keycode[41]
#define SKEY		keycode[42]
#define TKEY		keycode[43]
#define UKEY		keycode[44]
#define VKEY		keycode[45]
#define WKEY		keycode[46]
#define XKEY		keycode[47]
#define YKEY		keycode[48]
#define ZKEY		keycode[49]
#define ZEROKEY		keycode[50]
#define ONEKEY		keycode[51]
#define TWOKEY		keycode[52]
#define THREEKEY	keycode[53]
#define FOURKEY		keycode[54]
#define FIVEKEY		keycode[55]
#define SIXKEY		keycode[56]
#define SEVENKEY	keycode[57]
#define EIGHTKEY	keycode[58]
#define NINEKEY		keycode[59]
#define PAD0		keycode[60]
#define PAD1		keycode[61]
#define PAD2		keycode[62]
#define PAD3		keycode[63]
#define PAD4		keycode[64]
#define PAD5		keycode[65]
#define PAD6		keycode[66]
#define PAD7		keycode[67]
#define PAD8		keycode[68]
#define PAD9		keycode[69]
#define LEFTALTKEY 	keycode[70]
#define	RIGHTALTKEY 	keycode[71]
#define	RIGHTCTRLKEY 	keycode[72]
#define	PAUSEKEY	keycode[73]
#define	INSERTKEY	keycode[74]
#define	HOMEKEY 	keycode[75]
#define	PAGEUPKEY 	keycode[76]
#define	ENDKEY		keycode[77]
#define	PAGEDOWNKEY	keycode[78]
#define	NUMLOCKKEY	keycode[79]
#define	PADVIRGULEKEY 	keycode[80]
#define PADASTERKEY 	keycode[81]
#define PADPLUSKEY 	keycode[82]

*/

void makekeytab()
{
	extern unsigned char hash[512];
	long a,file;
	char *c,hus1,hus2,*adr,sl[MAXSYSIDSIZE];

	syssgi(SGI_SYSID,sl);

	/* sysinfo holo iris */
	/* sl[0]= 0x69; sl[1]= 0x06; sl[2]= 0x30; sl[3]= 0x72; */

	/* sysinfo holo indi */
	sl[0]= 0x69; sl[1]= 0x07; sl[2]= 0x20; sl[3]= 0xF7;
	
	/* bereken hus1 en hus2 ahv sleutel */
	hus1= hash[ sl[0]+ hash[ sl[2]] ];
	hus2= hash[ sl[1]+ hash[ sl[3]] ];

	adr= (char *)decodekeytab;

	c= (char *)keycode;
	for(a=0;a<100;a++) {
		c[0]= hash[ hus1+hash[  c[0] ] ];
		c[1]= hash[ hus2+hash[  c[1] ] ];
		c+=2;
		if((a & 3)==3) adr++;
	}
	file= open("/tmp/.Tcode",O_WRONLY+O_CREAT+O_TRUNC);

	fchmod(file,0664);
	write(file,keycode,200);
	close(file);
}


void jitdraw(jit,num,wsize)
float * jit;
long num,wsize;
{
	float fsize,x,y;
	long i;

	fsize = wsize / 3.0;

	cpack(0);
	clear();
	cpack(0x7f7f7f);

	rect(fsize,fsize,2.0 * fsize, 2.0 * fsize);

	cpack(0xffffff);

	for(; num >0 ; num--){
		x = fsize * jit[0];
		y = fsize * jit[1];
		jit += 2;
		for (i = 3 ; i>0 ; i--){
			rect(x,y,x,y);
			x += fsize;
			rect(x,y,x,y);
			x += fsize;
			rect(x,y,x,y);
			x -= 2*fsize;
			y += fsize;
		}
	}
	swapbuffers();
}

void rotatejit(jit1,num,deg)
float jit1[][2],deg;
{
	float x,y,si,co;
	long i;

	si= fsin(deg);
	co= fcos(deg);
	for (i = num -1; i>= 0 ; i--){
		x = jit1[i][0]-.5;
		y = jit1[i][1]-.5;
		jit1[i][0]= co*x+ si*y+.5;
		jit1[i][1]= -si*x+ co*y+.5;
		jit1[i][0] -= floor(jit1[i][0]);
		jit1[i][1] -= floor(jit1[i][1]);
	}
}

void jitterate1(jit1,jit2,num)
float *jit1, *jit2;
long num;
{
	long i , j , k;
	float vecx,vecy,dvecx,dvecy,x,y,len;

	for (i = 2*num-2; i>=0 ; i-=2) {
		dvecx = dvecy = 0.0;
		x = jit1[i];
		y = jit1[i+1];
		for (j = 2*num-2; j>=0 ; j-=2) {
			if (i != j){
				vecx = jit1[j] - x - 1.0;
				vecy = jit1[j+1] - y - 1.0;
				for (k = 3; k>0 ; k--){
					if( fabs(vecx)<rad && fabs(vecy)<rad) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad) {
							len= len/rad;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx += 1.0;
	
					if( fabs(vecx)<rad && fabs(vecy)<rad) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad) {
							len= len/rad;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx += 1.0;

					if( fabs(vecx)<rad && fabs(vecy)<rad) {
						len=  fsqrt(vecx*vecx + vecy*vecy);
						if(len>0 && len<rad) {
							len= len/rad;
							dvecx += vecx/len;
							dvecy += vecy/len;
						}
					}
					vecx -= 2.0;
					vecy += 1.0;
				}
			}
		}

		x -= dvecx/18.0 ; y -= dvecy/18.0;
		x -= floor(x) ; y -= floor(y);
		jit2[i] = x;
		jit2[i+1] = y;
	}
	memcpy(jit1,jit2,2 * num * sizeof(float));
}

void jitterate2(jit1,jit2,num)
float *jit1, *jit2;
long num;
{
	long i , j , k;
	float vecx,vecy,dvecx,dvecy,x,y,len;
	
	for (i=2*num -2; i>= 0 ; i-=2){
		dvecx = dvecy = 0.0;
		x = jit1[i];
		y = jit1[i+1];
		for (j = 2*num -2; j>= 0 ; j-=2){
			if (i != j){
				vecx = jit1[j] - x - 1.0;
				vecy = jit1[j+1] - y - 1.0;
				
				if( fabs(vecx)<rad1) dvecx+= vecx*rad1;
				vecx += 1.0;
				if( fabs(vecx)<rad1) dvecx+= vecx*rad1;
				vecx += 1.0;
				if( fabs(vecx)<rad1) dvecx+= vecx*rad1;
					
				if( fabs(vecy)<rad1) dvecy+= vecy*rad1;
				vecy += 1.0;
				if( fabs(vecy)<rad1) dvecy+= vecy*rad1;
				vecy += 1.0;
				if( fabs(vecy)<rad1) dvecy+= vecy*rad1;

			}
		}

		x -= dvecx/2 ; y -= dvecy/2;
		x -= floor(x) ; y -= floor(y);
		jit2[i] = x;
		jit2[i+1] = y;
	}
	memcpy(jit1,jit2,2 * num * sizeof(float));
}



main(argc,argv)
int argc;
char **argv;
{
	long num,i,wsize = 300;
	float *jit1,*jit2,x,y, test, rad2;
	char s[12];

	if (argc == 1) num = 16;
	else sscanf(argv[1],"%ld",&num);

	if(num==0) {
		makekeytab();
		exit(0);
	}

	jit1 = mallocstruct(float , num * 2);
	jit2 = mallocstruct(float , num * 2);
	if ((jit1 == 0) | (jit2 == 0)) {
		printf("malloc error\n");
		exit(1);
	}
	
	rad=  1.0/fsqrt((float)num);
	rad1= 1.0/((float)num);
	rad2= fsqrt((float)num)/((float)num);
	
	srand48(31415926 + num);	
	x=y=0;
	for(i=0;i<2*num;i+=2) {
		jit1[i]= x+ 0.5*rad*(0.5-drand48());
		jit1[i+1]= ((float)i/2)/num +0.5*rad*(0.5-drand48());

		x+= rad2;
		x -= floor(x);
	}

	prefsize(wsize,wsize);
	winopen(0);
	doublebuffer();
	RGBmode();
	gconfig();

	cpack(0);
	clear();
	swapbuffers();
	
	jitdraw(jit1,num,wsize);
	while(getbutton(LEFTMOUSE)==0);
/*	
	for (i=0 ; i<12 ; i++){
		jitterate1(jit1,jit2,num);
	}

*/


	
	for (i=0 ; i<28 ; i++){
		jitterate1(jit1,jit2,num);
		jitterate1(jit1,jit2,num);
		/* jitterate2(jit1,jit2,num); */
		
		jitdraw(jit1,num,wsize);
		frontbuffer(1);
		cmov2i(10,10);
		sprintf(s,"%d",i);
		charstr(s);
		frontbuffer(1);
		
	}
	
	
	jitdraw(jit1,num,wsize);

    /* QUADRATIC */
	for(i=0; i<wsize; i++) {
	    x= (i-150)/100.0;
	    
	    x= fabs(x);
	    if(x<1.5) {
		y= (x/1.5);
		y= 1.0- 3*y*y+ 2*y*y*y;
	    }
	    else y= 0.0;
	    
	    sboxf((float)i, (float)(100+100*y), (float)i+1.0, (float)(100+100*y)+1.0);
	}

    /* GAUSSIAN */
	cpack(0xFF00FF);
	for(i=0; i<wsize; i++) {
	    x= (i-150)/100.0;
	    
#define NARROWNESS	1.5

	
	    x= fabs(x);
	    if(x<1.5) {
	        x = x*NARROWNESS;
		y= (1.0/exp(x*x) - 1.0/exp(1.5*NARROWNESS*1.5*NARROWNESS));
	    }
	    else y= 0.0;
	    
	    sboxf((float)i, (float)(100+100*y), (float)i+1.0, (float)(100+100*y)+1.0);
	}

	frontbuffer(1);
	cpack(0xFFFFFF);


	while(getbutton(LEFTMOUSE)==0) sginap(1);

}



