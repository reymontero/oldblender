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


/*

cc -O3 tab.c noise.o -lgl_s -lc_s -lfm_s -float -lm -o tab

*/

#include <gl/gl.h>
#include <gl/device.h>
#include "/usr/people/frank/source/iff.h"
#include "/usr/people/include/Trace.h"
#include "/usr/people/include/Button.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/syssgi.h>


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
,PADPLUSKEY};


void decodekeytab()
{
	extern unsigned char hash[512];
	char ihash[512];
	long a,b,file;
	char *c,hus1,hus2,*adr,sl[MAXSYSIDSIZE];

	file= open("/usr/people/trace/.Tcode",O_RDONLY);
	if(file== -1) exit(0);

	read(file,keycode,200);
	close(file);

	syssgi(SGI_SYSID,sl);

	/* inverse hashtab */
	for(a=0;a<256;a++) {
		for(b=0;b<256;b++) {
			if(a==hash[b]) break;
		}
		ihash[a]= b;
	}
	for(a=0;a<256;a++) ihash[256+a]= ihash[a];

	/* bereken hus1 en hus2 ahv sleutel */
	hus1= hash[ sl[0]+ hash[ sl[2]] ];
	hus2= hash[ sl[1]+ hash[ sl[3]] ];

	adr= (char *)decodekeytab;
	
	c= (char *)keycode;
	for(a=0;a<100;a++) {
		c[0]= ( ihash[ (ihash[ c[0] ] -hus1) % 256 ] /*-adr[0]*/ ) % 256 ;
		c[1]= ( ihash[ (ihash[ c[1] ] -hus2) % 256 ] /*-adr[0]*/ ) % 256 ;
		c+=2;
		adr++;
	}

}

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

	/* bereken hus1 en hus2 ahv sleutel */
	hus1= hash[ sl[0]+ hash[ sl[2]] ];
	hus2= hash[ sl[1]+ hash[ sl[3]] ];

	adr= (char *)decodekeytab;

	c= (char *)keycode;
	for(a=0;a<100;a++) {
		c[0]= hash[ hus1+hash[ /*adr[0]+*/ c[0] ] ];
		c[1]= hash[ hus2+hash[ /*adr[0]+*/ c[1] ] ];
		c+=2;
		adr++;
	}
	file= open("/usr/people/trace/.Tcode",O_WRONLY+O_CREAT+O_TRUNC);
	fchmod(file,0664);
	write(file,keycode,200);
	close(file);
}


main()
{

	makekeytab();

}

