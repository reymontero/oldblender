
/*		toolbox.c	GRPAHICS
 * 
 *		feb 93
 *		dec 93 voor blender
 */

#include "graphics.h"
#include "blender.h"
#include <fcntl.h>



/*   ********  NOTES  ***********	*****************************
	
	- Toolbox items zelf invullen
	- linestyle nr 65535 en pattern TBOXPAT (1234) worden hier gedefinieerd
	- de colormap kleuren staan met comments in de bgntoolbox()
	- de funktie qread eventueel vervangen
	- let op de benaming van bijzondere toetsen (NumL etc) 
	- meelinken: Button.c,  ivm rawcodes
	
	*****************************	*****************************
*/	


	/* TBOXX: breedte in pixels */
#define TBOXX	400
	/* TBOXEL: aantal elementen onder elkaar */
#define TBOXEL	6
	/* TBOXH: hoogte van 1 element */
#define TBOXH	20
#define TBOXY	TBOXH*TBOXEL
#define TBOXBLACK	2
#define TBOXWHITE	3
#define TBOXGREY	1
#define TBOXPAT		1234

fmfonthandle helv10;
float tbwinmat[4][4], tbprojmat[4][4];
long tbx1, tbx2, tby1, tby2, tbfontyofs, confontyofs, tbmain=0;
long tbmemx=0, tbmemy=0, tboldwin;
ushort tbpat[16]; 
short oldcursor, oldmap[4][3];

	/* variabelen per item */
char *tbstr, *tbstr1;		
void (*tbfunc)();
long tbval;


/* *********** ANSI COMPATIBLE **************** */

void mygetcursor(short *index)
{
	Colorindex color, wtm;
	Boolean b;
	
	getcursor( index, &color, &wtm, &b);
}

/* ********************* TOOLBOX ITEMS ************************* */

void tbox_setinfo(x, y)
long x, y;
{
	/* afhankelijk van tbmain worden vars gezet */
	extern struct Base *addbase();
	extern void addprimitiveVV(), addprimitiveCurve(), addprimitiveNurb();

	tbstr= 0;
	tbstr1= 0;
	tbfunc= 0;
	tbval= 0;

/* HOOFDMENU ITEMS LINKS */
	if(x==0) {

		switch(y) {
			case 0:		tbstr= "VIEW";		break;
			case 1:		tbstr= "EDIT";		break;
			case 2:		tbstr= "ADD";		break;
			case 3:		tbstr= "BASE";		break;
			case 4:		tbstr= "BASE";		break;
			case 5:		tbstr= "TRIFACE";	break;
		}
	}
	
/* HOOFDMENU ITEMS RECHTS */
	else if(x==3) {

		switch(y) {
			case 0:		tbstr= "NURBS";		break;
			case 1:		tbstr= "KEY";		break;
			case 2:		tbstr= "RENDER";	break;
			case 3:		tbstr= "DIV";		break;
		}
	}
	
/* TOPICS */
	else {
	
		if(x==1) y*= 2;
		else y= y*2+1;
		
/* VIEW TOPICS */
		if(tbmain==0) {
			switch(y) {
				case 0: tbstr= "StoreView";		tbstr1= "c|v";	break;
				case 1: tbstr= "LoadView";		tbstr1= "c|1,2..";	break;
				case 2: tbstr= "MoveCamera";	tbstr1= "o";	break;
				case 3: tbstr= "MoveTarget";	tbstr1= "t";	break;
				case 4: tbstr= "Centre";		tbstr1= "c";	break;
				case 5: tbstr= "Home";			tbstr1= "C";	break;
				case 6: tbstr= "Hierarchy";		tbstr1= "h";	break;
				case 7: tbstr= "Names";			tbstr1= "n";	break;
				case 8: tbstr= "Push/Pop";		tbstr1= "a|n";	break;
				case 9: tbstr= "Zbuffer";		tbstr1= "z";	break;
				case 10: tbstr= "LocalView";	tbstr1= "NumL";	break;
				case 11: tbstr= "";		tbstr1= "";		break;
			}
		}

/* EDIT TOPICS */
		else if(tbmain==1) {
			switch(y) {
				case 0: tbstr= "Grabber";	tbstr1= "g";	break;
				case 1: tbstr= "Rotate";	tbstr1= "r";	break;
				case 2: tbstr= "Size";		tbstr1= "s";	break;
				case 3: tbstr= "SizeView";	tbstr1= "a|s";	break;
				case 4: tbstr= "Shear";		tbstr1= "c|s";	break;
				case 5: tbstr= "Warp/bend";	tbstr1= "W";	break;
				case 6: tbstr= "Snapmenu";	tbstr1= "S";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "(De)sel All";	tbstr1= "a";	break;
				case 9: tbstr= "BorderSelect";	tbstr1= "b";	break;
				case 10: tbstr= "";				tbstr1= "";	break;
				case 11: tbstr= "Quit";	tbstr1= "q";	break;
			}
		}

/* ADD TOPICS */
		else if(tbmain==2) {

			switch(y) {
				case 0: tbstr= "Bez Curve";	tbstr1= "A";	tbval=10; break;
				case 1: tbstr= "Bez Circle";tbstr1= "A";	tbval=11; break;
				case 2: tbstr= "";			tbstr1= "";		break;
				case 3: tbstr= "";			tbstr1= "";		break;
				case 4: tbstr= "NurbCurve";	tbstr1= "A";	tbval=40; break;
				case 5: tbstr= "NurbCircl";	tbstr1= "A";	tbval=41; break;
				case 6: tbstr= "";			tbstr1= "";		break;
				case 7: tbstr= "";			tbstr1= "";		break;
				case 8: tbstr= "";			tbstr1= "";		break;
				case 9: tbstr= "";			tbstr1= "";		break;
				case 10: tbstr= "Duplicate";tbstr1= "D";	break;
				case 11: tbstr= "";			tbstr1= "";		break;
			}
		}
		
/* BASE TOPICS 1 */
		else if(tbmain==3) {
			switch(y) {
				case 0: tbstr= "Clr mtrx";	tbstr1= "a|m";	break;
				case 1: tbstr= "Clr rot";	tbstr1= "a|r";	break;
				case 2: tbstr= "Clr orig";	tbstr1= "a|o";	break;
				case 3: tbstr= "";			tbstr1= "";		break;
				case 4: tbstr= "Mk Parent";	tbstr1= "c|p";	break;
				case 5: tbstr= "ClrParent";	tbstr1= "a|p";	break;
				case 6: tbstr= "Mk Track";	tbstr1= "c|t";	break;
				case 7: tbstr= "ClrTrack";	tbstr1= "a|t";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "ImaDisplist";	tbstr1= "c|d";	break;
				case 11: tbstr= "ImaAspect";	tbstr1= "a|v";	break;
			}
		}
		
/* BASE TOPICS 2 */
		else if(tbmain==4) {
			switch(y) {
				case 0: tbstr= "EditMode";		tbstr1= "Tab";	break;
				case 1: tbstr= "MoveToLayer";	tbstr1= "m";	break;
				case 2: tbstr= "Delete";		tbstr1= "x";	break;
				case 3: tbstr= "DeleteAll";		tbstr1= "X";	break;
				case 4: tbstr= "Apply Size/Rot";	tbstr1= "c|a";	break;
				case 5: tbstr= "Join";			tbstr1= "c|j";		break;
				case 6: tbstr= "Fit Space";		tbstr1= "c|f";	break;
				case 7: tbstr= "Rename";		tbstr1= "c|r";	break;
				case 8: tbstr= "";				tbstr1= "";	break;
				case 9: tbstr= "Startframes";	tbstr1= "c|R";	break;
				case 10: tbstr= "Copy menu";	tbstr1= "c|c";	break;
				case 11: tbstr= "Conv menu";	tbstr1= "a|c";	break;
			}
		}

/* TRIFACE TOPICS */
		else if(tbmain==5) {
			switch(y) {
				case 0: tbstr= "Sel Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Desel Link";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "DeleteMenu";	tbstr1= "x";	break;
				case 4: tbstr= "Mk edge/face";	tbstr1= "f";	break;
				case 5: tbstr= "Fill";			tbstr1= "F";	break;
				case 6: tbstr= "Split";			tbstr1= "y";	break;
				case 7: tbstr= "Undo/reload";	tbstr1= "u";	break;
				case 8: tbstr= "CalcNormals";	tbstr1= "c|n";	break;
				case 9: tbstr= "Separate";		tbstr1= "p";	break;
				case 10: tbstr= "Wr Videosc";	tbstr1= "a|w";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	
/* NURBS TOPICS */
		else if(tbmain==6) {
			switch(y) {
				case 0: tbstr= "Sel Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Desel Link";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "DeleteMenu";	tbstr1= "x";	break;
				case 4: tbstr= "Make Segment";	tbstr1= "f";	break;
				case 5: tbstr= "Cyclic";		tbstr1= "c";	break;
				case 6: tbstr= "InsertPoint";	tbstr1= "i";	break;
				case 7: tbstr= "SelectRow";		tbstr1= "R";	break;
				case 8: tbstr= "Calc Handle";	tbstr1= "h";	break;
				case 9: tbstr= "Auto Handle";	tbstr1= "H";	break;
				case 10: tbstr= "Vect Handle";	tbstr1= "v";	break;
				case 11: tbstr= "PrintWeights";	tbstr1= "w";	break;
			}
		}
	
/* KEY TOPICS */
		else if(tbmain==7) {
			switch(y) {
				case 0: tbstr= "Insert";	tbstr1= "i";	break;
				case 1: tbstr= "Delete";	tbstr1= "d";	break;
				case 2: tbstr= "Next";		tbstr1= "PageUp";	break;
				case 3: tbstr= "Prev";		tbstr1= "PageDn";	break;
				case 4: tbstr= "";	tbstr1= "";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}

/* RENDER TOPICS */
		else if(tbmain==8) {
			switch(y) {
				case 0: tbstr= "RenderWindow";	tbstr1= "F11";	break;
				case 1: tbstr= "Render";		tbstr1= "F12";	break;
				case 2: tbstr= "Set Border";	tbstr1= "B";	break;
				case 3: tbstr= "Image Zoom";	tbstr1= "z";	break;
				case 4: tbstr= "";	tbstr1= "";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	
/* DIV TOPICS */
		else if(tbmain==9) {
			switch(y) {
				case 0: tbstr= "Load Scene";	tbstr1= "F1";	break;
				case 1: tbstr= "Append Sce";	tbstr1= "";		break;
				case 2: tbstr= "SaveAs Sce";	tbstr1= "F2";	break;
				case 3: tbstr= "Save Scene";	tbstr1= "c|w";	break;
				case 4: tbstr= "Save Image";	tbstr1= "F3";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "Exec Script";	tbstr1= "c|x";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	}
}

/* ******************** INIT ************************** */

void bgnpupdraw()	/* doet ook font, patterns, linestyle, colors */
{
	static long firsttime= 1;
	fmfonthandle helvfont;
	long x;
	
	pushattributes();
	linewidth(1);
	setlinestyle(0);
	zbuffer(0);
	
	tboldwin= winget();
	if(tboldwin) {
		getmatrix(tbwinmat);
		x= getmmode();
		if(x!=MSINGLE) {
			mmode(MPROJECTION);
			getmatrix(tbprojmat);
			mmode(x);
		}
	}

	mygetcursor(&oldcursor);
	setcursor(0, 0, 0);

	fullscrn();
	drawmode(PUPDRAW);
	sginap(2);
	finish();
	
	if(firsttime) {
		
		getmcolor((Colorindex)1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor((Colorindex)2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor((Colorindex)3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		
		for(x=0; x<16; x+=2) tbpat[x]=0x5555;
		for(x=1; x<16; x+=2) tbpat[x]=0xAAAA;

		deflinestyle(65535, 0x5555);	/* zelfde als Button.c */
		
		fminit();
		if( (helvfont=fmfindfont("Helvetica-Bold")) == 0) {
			helv10= 0;
			return;
		}
		helv10= fmscalefont(helvfont,11.0);
		
		initrawcodes();		/* Button.c */
		
		firsttime= 0;
	}
	
	mapcolor(1, 170, 170, 170); 
	mapcolor(2, 0, 0, 0); 
	mapcolor(3, 240, 240, 240);

	defpattern(TBOXPAT, 16, tbpat);
	fmsetfont(helv10);
	tbfontyofs= (TBOXH-getheight()/2-1)/2;
	confontyofs= (25-getheight()/2-1)/2;
}

void endpupdraw()
{
	long x;

	mapcolor(1, oldmap[1][0], oldmap[1][1], oldmap[1][2]);
	mapcolor(2, oldmap[2][0], oldmap[2][1], oldmap[2][2]);
	mapcolor(3, oldmap[3][0], oldmap[3][1], oldmap[3][2]);
	
	endfullscrn();
	drawmode(NORMALDRAW);
	
	if(tboldwin) {
		winset(tboldwin);
		loadmatrix(tbwinmat);
		x= getmmode();
		if(x!=MSINGLE) {
			mmode(MPROJECTION);
			loadmatrix(tbprojmat);
			mmode(x);
		}
	}

	popattributes();
	
	setcursor(oldcursor, 0, 0);
}

/* ********************************************** */

void tbox_execute()
{
	/* als tbfunc: functie aanroepen */
	/* als tbstr1 is een string: value tbval in queue stopen */
	short event=0, qual1=0, qual2=0, asciitoraw();
	
	
	if(tbfunc) tbfunc(tbval);
	else if(tbstr1) {
		if(strcmp(tbstr1, "Tab")==0) {
			event= TABKEY;
		}
		else if(strcmp(tbstr1, "PageUp")==0) {
			event= PAGEUPKEY;
		}
		else if(strcmp(tbstr1, "PageDn")==0) {
			event= PAGEDOWNKEY;
		}
		else if(strlen(tbstr1)<4) {
			if(tbstr1[1]=='|') {
				if(tbstr1[0]=='c') qual1= LEFTCTRLKEY;
				else if(tbstr1[0]=='a') qual1= LEFTALTKEY;
				event= asciitoraw(tbstr1[2]);
				if(event>100) {
					event-= 100;
					qual2= LEFTSHIFTKEY;
				}
			}
			else if(tbstr1[1]==0) {
				event= asciitoraw(tbstr1[0]);
				if(event>100) {
					event-= 100;
					qual2= LEFTSHIFTKEY;
				}
			}
			else if(tbstr1[0]=='F') {
				event= atoi(tbstr1+1);
				switch(event) {
					case 1: event= F1KEY; break;
					case 2: event= F2KEY; break;
					case 3: event= F3KEY; break;
					case 4: event= F4KEY; break;
					case 5: event= F5KEY; break;
					case 6: event= F6KEY; break;
					case 7: event= F7KEY; break;
					case 8: event= F8KEY; break;
					case 9: event= F9KEY; break;
					case 10: event= F10KEY; break;
					case 11: event= F11KEY; break;
					case 12: event= F12KEY; break;
				}
			}
		}
		
		if(event) {
			if(qual1) qenter(qual1, 1);
			if(qual2) qenter(qual2, 1);
			qenter(event, 1);
			qenter(event, 0);
			if(qual1) qenter(qual1, 0);
			if(qual2) qenter(qual2, 0);
		}
	}
}

void tbox_getmouse(mval)
short *mval;
{
	static Device mdev[2]= {MOUSEX,MOUSEY};

	getdev(2,mdev,mval);

}

void tbox_setmain(val)
{
	tbmain= val;
}

void bgntoolbox()
{
	short xmax, ymax, mval[2];
	
	bgnpupdraw();
	
	xmax = getgdesc(GD_XPMAX);
	ymax = getgdesc(GD_YPMAX);

	tbox_getmouse(mval);
	if(mval[0]<95) mval[0]= 95;
	if(mval[0]>xmax-95) mval[0]= xmax-95;

	setvaluator(MOUSEX, mval[0], 0, xmax);
	setvaluator(MOUSEY, mval[1], 0, ymax);
	
	tbx1= mval[0]-TBOXX/2-tbmemx;
	tby1= mval[1]-TBOXY/2-tbmemy;
	if(tbx1<10) tbx1= 10;
	if(tby1<10) tby1= 10;
	
	tbx2= tbx1+TBOXX;
	tby2= tby1+TBOXY;
	if(tbx2>xmax) {
		tbx2= xmax-10;
		tbx1= tbx2-TBOXX;
	}
	if(tby2>ymax) {
		tby2= ymax-10;
		tby1= tby2-TBOXY;
	}

	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	sboxfs(tbx1+5, tby1-5, tbx2+5, tby2-5);
	setpattern(0);

}

void endtoolbox()
{
	short mval[2];
	
	tbox_getmouse(mval);
	if(mval[0]>tbx1 && mval[0]<tbx2)
		if(mval[1]>tby1 && mval[1]<tby2) {
			tbmemx= mval[0]-(tbx1+tbx2)/2;
			tbmemy= mval[1]-(tby1+tby2)/2;
	}
	
	color(0);
	sboxfs(tbx1-5, tby1-5, tbx2+5, tby2+5);
	
	endpupdraw();
}

void tbox_embossbox(x1, y1, x2, y2, type)	/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
short x1, y1, x2, y2;
{

	color(TBOXBLACK);
	sboxs(x1, y1, x2, y2);

	if(type==0 || type==2) {
		color(TBOXGREY);
		sboxfs(x1+1, y1+1, x2-1, y2-1);
		if(type==2) {
			setpattern(TBOXPAT);
			color(TBOXWHITE);
			sboxfs(x1+1, y1+1, x2-1, y2-1);
			setpattern(0);
		
		}
		color(TBOXWHITE);
		sboxs(x1+1, y2-1, x2-1, y2-1);	/* boven */
		color(TBOXBLACK);
		sboxs(x1+2, y1+2, x2-2, y1+2);	/* onder */
		sboxs(x2-2, y1+2, x2-2, y2-2);	/* rechts */
		
		setlinestyle(65535);
		color(TBOXWHITE);
		sboxs(x1+2, y1+2, x2-2, y2-2);	/* kader */
		color(TBOXBLACK);
		sboxs(x1+2, y1+1, x2-1, y1+1);	/* onder */
		sboxs(x2-1, y1+2, x2-1, y2-1);	/* rechts */
		setlinestyle(0);
		
		color(TBOXBLACK);	/* tekst */
	}
	else {
		color(TBOXGREY);
		sboxfs(x1+1, y1+1, x2-1, y2-1);
		if(type==1) {
			setpattern(TBOXPAT);
			color(TBOXBLACK);
			sboxfs(x1+1, y1+1, x2-1, y2-1);
			setpattern(0);
			sboxfs(x1+1, y1+1, x1+2, y2-1);
		}
		color(TBOXWHITE);
		sboxs(x1+1, y1+1, x2-1, y1+1);	/* onder */
		color(TBOXBLACK);
		sboxs(x1+2, y2-2, x2-2, y2-2);	/* boven */
		sboxs(x1+2, y1+2, x1+2, y2-2);	/* links */
		
		setlinestyle(65535);
		color(TBOXWHITE);
		if(type==1) {
			sboxs(x1+2, y1+2, x2-2, y1+2);	/* onder */
			sboxs(x2-2, y1+1, x2-2, y2-2);	/* rechts */
			setlinestyle(0);
			
			color(TBOXBLACK);
			sboxs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			
			color(TBOXWHITE);	/* tekst */
		}
		else {
			sboxs(x1+2, y1+2, x2-2, y2-2);	/* kader */

			color(TBOXBLACK);
			sboxs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			sboxs(x1+1, y1+2, x1+1, y2-1);	/* links */
			setlinestyle(0);
		}
	}
}

void tbox_drawelem(x, y, type)
long x, y, type;		/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
{
	long x1, y1, x2, y2, len1, len2;
	
	if(x==0) {
		x1= tbx1; x2= tbx1+(2*TBOXX)/10;
	}
	else if(x==1) {
		x1= tbx1+(2*TBOXX)/10;
		x2= x1+ (3*TBOXX)/10;
	}
	else if(x==2) {
		x1= tbx1+TBOXX/2;
		x2= x1+ (3*TBOXX)/10;
	}
	else  {
		x1= tbx1+(8*TBOXX)/10;
		x2= tbx2;
	}
	
	y1= tby1+ y*TBOXH;
	y2= y1+TBOXH;
	
	tbox_embossbox(x1, y1, x2, y2, type);
	
	/* tekst */
	tbox_setinfo(x, y);
	if(tbstr && tbstr[0]) {
		len1= 5+fmgetstrwidth(helv10, tbstr);
		if(tbstr1) len2= 5+fmgetstrwidth(helv10, tbstr1); else len2= 0;
		
		while(len1>0 && (len1+len2+5>x2-x1) ) {
			tbstr[strlen(tbstr)-1]= 0;
			len1= fmgetstrwidth(helv10, tbstr);
		}
		cmov2i(x1+5,y1+tbfontyofs);
		fmprstr(tbstr);
		if(tbstr1 && tbstr1[0]) {
			if(type & 1) {
				color(TBOXBLACK);
				sboxfi(x2-len2-2, y1+2, x2-3, y2-2);
				color(TBOXWHITE);
				cmov2i(x2-len2, y1+tbfontyofs);
				fmprstr(tbstr1);
			}
			else {
				color(TBOXBLACK);
				cmov2i(x2-len2, y1+tbfontyofs);
				fmprstr(tbstr1);
			}
		}
	}
	
}

void tbox_getactive(x, y)
long *x, *y;
{
	short mval[2];
	
	tbox_getmouse(mval);
	
	mval[0]-=tbx1;
	if(mval[0]<(2*TBOXX)/10) *x= 0;
	else if(mval[0]<TBOXX/2) *x= 1;
	else if(mval[0]<(8*TBOXX)/10) *x= 2;
	else *x= 3;
	
	*y= mval[1]-tby1;
	*y/= TBOXH;
	if(*y<0) *y= 0;
	if(*y>TBOXEL-1) *y= TBOXEL-1;
	
}

void drawtoolbox()
{
	long x, y, actx, acty, type;
	
	tbox_getactive(&actx, &acty);

	for(x=0; x<4; x++) {
		for(y=0; y<TBOXEL; y++) {
			
			if(x==0 || x==3) type= 0; 
			else type= 2;
			
			if(actx==x && acty==y) type++;
			if(type==0) {
				if(x==0 && tbmain==y) type= 1;
				else if(x==3 && tbmain==y+TBOXEL) type= 1;
			}
			
			tbox_drawelem(x, y, type);
			
		}
	}
}


void toolbox()
{
	long actx, acty;
	short event, val, mval[2], xo= -1, yo=0;
	short redrawq[10], redrawcount= 0;
	long winakt= 0;
	
	bgntoolbox();
	if(helv10==0) return;
	
	drawtoolbox();
	
	/* 
	 *	De aktieve window wordt in queue terug gestopt.
	 */
	 
	while(1) {
		event= 0;
		if(qtest()) event= qread(&val);
		
		if(event) {
			switch(event) {
				case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
					if(val==0 || event==RETKEY) {
						tbox_getactive(&actx, &acty);
						tbox_setinfo(actx, acty);
						endtoolbox();
						if(winakt) qenter(INPUTCHANGE, winakt);

						tbox_execute();
						return;
					}
					break;
				case INPUTCHANGE:
					winakt= val;
					break;
				case ESCKEY:
					endtoolbox();
					if(winakt) qenter(INPUTCHANGE, winakt);
					return;
				case REDRAW:
					if(val && redrawcount<10) {
						redrawq[redrawcount]= val;
						redrawcount++;
					}
			}
		}
		else sginap(2);
		
		tbox_getmouse(mval);
		if(mval[0]<tbx1-10 || mval[0]>tbx2+10 || mval[1]<tby1-10 || mval[1]>tby2+10) break;
		
		tbox_getactive(&actx, &acty);
		
		/* muisafhandeling en redraw */
		if(xo!=actx || yo!=acty) {
			if(actx==0) tbmain= acty;
			else if(actx==3) tbmain= acty+TBOXEL;
			
			if(actx==0 || actx==3) drawtoolbox();
			else if(xo> -1) {
				if(xo==0 || xo==3) tbox_drawelem(xo, yo, 1);
				else tbox_drawelem(xo, yo, 2);
				tbox_drawelem(actx, acty, 3);
			}
			xo= actx;
			yo= acty;
		}
	}
	
	endtoolbox();
	if(winakt) qenter(INPUTCHANGE, winakt);
	
	for(val=0; val<redrawcount; val++) qenter(REDRAW, redrawq[val]);

}

/* ************************************  */

void confirm_draw(startx, starty, endx, title, item, sel)
short startx, starty, endx;
char *title, *item;
short sel;
{

	if(title[0]!=0) tbox_embossbox(startx, starty+30, endx, starty+55, 0);
	tbox_embossbox(startx, starty, endx, starty+25, 2+sel);
	
	color(TBOXBLACK);
	cmov2i(startx+5, starty+confontyofs);
	fmprstr(item);
	cmov2i(startx+5, starty+30+confontyofs);
	if(title[0]!=0) fmprstr(title);
}

short confirm(title, item)
char *title, *item;
{
	long winakt= 0;
	short mousedown= 0, val, value=0, event, len, mval[2], xmax, ymax, startx, starty, endx, endy;
	short redrawq[10], redrawcount= 0;

	if(title==0 || item==0) return 0;
	
	bgnpupdraw();
	
	len= fmgetstrwidth(helv10, title);
	xmax= fmgetstrwidth(helv10, item);
	if( xmax>len ) len= xmax;
	len+= 10;
	xmax = getgdesc(GD_XPMAX);
	ymax = getgdesc(GD_YPMAX);

	tbox_getmouse(mval);
	
	startx= mval[0]-len/2;
	starty= mval[1]-10;
	if(startx<10) startx= 10;
	if(starty<10) starty= 10;
	
	endx= startx+len;
	endy= starty+60;
	if(endx>xmax) {
		endx= xmax-10;
		startx= endx-len;
	}
	if(endy>ymax) {
		endy= ymax-10;
		starty= endy-60;
	}

	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	if(title[0]!=0) sboxfs(startx+5, starty+30-5, endx+5, starty+55-5);
	sboxfs(startx+5, starty-5, endx+5, starty+25-5);
	setpattern(0);
	
	confirm_draw(startx, starty, endx, title, item, 0);
	
	while(value==0) {
		event= 0;
		if(qtest()) event= qread(&val);
		
		if(event) {
			switch(event) {
				case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
					if(val) {
						mousedown= 1;
						tbox_getmouse(mval);
						if(mval[1]<starty+27)
							confirm_draw(startx, starty, endx, title, item, 1);
					}
					else if(mousedown) {
						if(mval[1]<starty+27) value= 2;
						else value= 1;
						if(event==RETKEY) sginap(10);
					}
					break;
				case INPUTCHANGE:
					winakt= val;
					break;
				case ESCKEY:
					value= 1;
					break;
				case REDRAW:
					if(val && redrawcount<10) {
						redrawq[redrawcount]= val;
						redrawcount++;
					}
			}
		}
		else sginap(2);
		
		tbox_getmouse(mval);
		if(mval[0]<startx-10 || mval[0]>endx+10 || mval[1]<starty-10 || mval[1]>endy+10) value= 1;
		
	}

	color(0);
	sboxfs(startx-1, starty-5, endx+5, endy+1);
	endpupdraw();
	
	if(winakt) qenter(INPUTCHANGE, winakt);
	
	for(val=0; val<redrawcount; val++) qenter(REDRAW, redrawq[val]);
	
	return (value-1);
}


long saveover(str)
char *str;
{
	long file;
	
	file= open(str, O_RDONLY);
	close(file);
	if(file==-1) return 1;
	else if(confirm("SAVE OVER", str)) return 1;
	
	return 0;
}

short okee(char *str)
{
	return confirm("OK?", str);
}

void error(char *str)
{
	confirm("", str);
}

void pupmenu_draw(startx, starty, endx, items, title, spoin, sel)
int startx, starty, endx, items;
char *title, **spoin;
short sel;
{
	int a=0;
	
	starty+= (items-1)*TBOXH;
	if(title) {
		tbox_embossbox(startx, starty+5, endx, starty+TBOXH+5, 0);
		color(TBOXBLACK);
		cmov2i(startx+5, starty+tbfontyofs+5);
		fmprstr(title);
		starty-= TBOXH;
		a= 1;
	}
	
	for(; a<items; a++) {
		tbox_embossbox(startx, starty, endx, starty+TBOXH, 2);
		color(TBOXBLACK);
		cmov2i(startx+5, starty+tbfontyofs);
		fmprstr(spoin[a]);

		starty-= TBOXH;
	}
	
	
}


short pupmenu(char *instr)	/* zelfde syntax als dopup */
{
	static int lastselected= -1;
	int len, height, a, items, retval[100], xmax, ymax, startx, starty, endx, endy;
	int act, acto=0, value=0, event;
	short val, winakt=0, mval[2];
	short redrawq[10], redrawcount= 0;
	char *str, *astr, *title=0, *spoin[100];
	
	if(instr==0) return 0;
	
	bgnpupdraw();

	/* kopie string maken */
	str= mallocN(strlen(instr)+1, "pupmenu");
	memcpy(str, instr, strlen(instr)+1);
	bzero(retval, 400);
	
	/* eerst string uitelkaar pulken, tellen hoeveel elementen, return values */
	astr= str;
	spoin[0]= astr;
	items= 0;
	while( *astr) {
		switch( *astr ) {
			case '%':
				if(astr[1]=='x') {
					retval[items]= atoi(astr+2);
					*astr= 0;
				}
				else if(astr[1]=='t') {
					title= spoin[0];
					*astr= 0;
				}
				break;
			case '|':
				items++;
				spoin[items]= astr+1;
				*astr= 0;
				break;
		}
		astr++;
	}
	
	items++;	/* het begon bij 0 te tellen */
	
	/* afmetingen en plaats */
	len= 0;
	for(a=0; a<items; a++) {
		xmax= fmgetstrwidth(helv10, spoin[a]);
		if(xmax>len) len= xmax;
	}
	len+= 10;
	
	height= TBOXH*items;
	
	xmax = getgdesc(GD_XPMAX);
	ymax = getgdesc(GD_YPMAX);

	tbox_getmouse(mval);
	
	startx= mval[0]-len/2;
	if(lastselected>=0 && lastselected<items) {
		starty= mval[1]-height+TBOXH/2+lastselected*TBOXH;
	}
	else starty= mval[1]-height/2;
	if(startx<10) startx= 10;
	if(starty<10) starty= 10;
	
	endx= startx+len;
	endy= starty+height;
	if(endx>xmax) {
		endx= xmax-10;
		startx= endx-len;
	}
	if(endy>ymax) {
		endy= ymax-10;
		starty= endy-height;
	}


	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	sboxfs(startx+5, starty-10, endx+5, endy-10);
	setpattern(0);

	pupmenu_draw(startx, starty, endx, items, title, spoin, 1);

	while(value==0) {
		event= 0;
		if(qtest()) event= qread(&val);
		
		if(event) {
			switch(event) {
			case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
				if(val) {
					value= 2;
				}
				break;
			case INPUTCHANGE:
				winakt= val;
				break;
			case ESCKEY:
				value= 1;
				break;
			case REDRAW:
				if(val && redrawcount<10) {
					redrawq[redrawcount]= val;
					redrawcount++;
				}
			}
		}
		else sginap(2);
		
		tbox_getmouse(mval);
		if(mval[0]<startx-10 || mval[0]>endx+10 || mval[1]<starty-10 || mval[1]>endy+10) value= 1;
		
		act= items-1- (mval[1]-starty)/TBOXH;
		if(act!=acto && act<items && act>=0) {
		
			if(title) a= endy-20; else a= endy;
			/* teken acto */
			if(title && acto==0) ;
			else {
				tbox_embossbox(startx, a-acto*TBOXH, endx, a-acto*TBOXH+TBOXH, 2);
				color(TBOXBLACK);
				cmov2i(startx+5, a-acto*TBOXH+tbfontyofs);
				fmprstr(spoin[acto]);
			}
			/* teken act */
			if(title && act==0) ;
			else {
				tbox_embossbox(startx, a-act*TBOXH, endx, a-act*TBOXH+TBOXH, 3);
				color(TBOXBLACK);
				cmov2i(startx+5, a-act*TBOXH+tbfontyofs);
				fmprstr(spoin[act]);
			}
			acto= act;
		}
	}

	color(0);
	sboxfs(startx-1, starty-10, endx+5, endy+10);
	endpupdraw();
	freeN(str);

	if(winakt) qenter(INPUTCHANGE, winakt);

	for(val=0; val<redrawcount; val++) qenter(REDRAW, redrawq[val]);
	
	if(value==2) {
		lastselected= act;
		return retval[act];
	}
	return -1;
}
