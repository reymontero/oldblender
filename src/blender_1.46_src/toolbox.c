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



/*		toolbox.c	GRPAHICS
 * 
 *		feb 93
 *		dec 93 voor blender
 */

#include "blender.h"
#include "graphics.h"

/* ZEER VREEMDE BUG:
 * 
 * 
 * Op de iris wil in de funktie tbox_embossbox() de setlinestyle()
 * niet werken. Na enige tijd blijft het systeem hangen
 * 
		Aug  5 16:58:28 2A:iris unix: GR1FIFOHandler: TIMEOUT (FIFO still > half full)!
		Aug  5 16:58:28 2A:iris unix: 
		Aug  5 16:58:28 4A:iris unix: WARNING: Graphics error: base = 0xbf000000  GE PC = 1222
		Aug  5 16:58:28 2A:iris unix: 
		Aug  5 16:58:28 3B:iris xdm[413]: Server for display :0 terminated unexpectedly: 2304
		Aug  5 16:58:28 6B:iris Xsession: ton: logout
		Aug  5 16:58:53 2A:iris unix: Killing the input queue owner
		Aug  5 16:58:53 3B:iris xdm[413]: Server for display :0 terminated unexpectedly: 2304
 * 
 * Dit is opgelost door een setpattern te gebruiken.
 * 
 */

/*   ********  NOTES  ***********	*****************************
	
	- Toolbox items zelf invullen
	- linestyle nr 65535 en pattern TBOXPAT (1234) worden hier gedefinieerd
	- de colormap kleuren staan met comments in de bgntoolbox()
	- de funktie extern_qread eventueel vervangen
	- let op de benaming van bijzondere toetsen (NumL etc) 
	- meelinken: Button.c,  ivm rawcodes
	
	*****************************	*****************************
*/	


	/* TBOXX: breedte in pixels */
#define TBOXXL  80
#define TBOXXR  170
#define TBOXX	(TBOXXL+TBOXXR)
	/* TBOXEL: aantal elementen onder elkaar */
#define TBOXEL	14
	/* TBOXH: hoogte van 1 element */
#define TBOXH	20
#define TBOXY	TBOXH*TBOXEL
#define TBOXBLACK	2
#define TBOXWHITE	3
#define TBOXGREY	1
#define TBOXPAT		1234

fmfonthandle helv10;
float tbwinmat[4][4], tbprojmat[4][4];
int tbx1, tbx2, tby1, tby2, tbfontyofs, confontyofs, tbmain=0;
int tbmemx=TBOXX/2, tbmemy=(TBOXEL-0.5)*TBOXH, tboldwin, addmode= 0;
ushort tbpat[16]; 
short oldcursor, oldmap[4][3];

extern void add_font_prim();

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
	tbstr= 0;
	tbstr1= 0;
	tbfunc= 0;
	tbval= 0;

/* HOOFDMENU ITEMS LINKS */
	if(x==0) {

		switch(y) {
			case 0:		
				if(addmode==OB_MESH) tbstr= "  MESH";
				else if(addmode==OB_CURVE) tbstr= "  CURVE";
				else if(addmode==OB_SURF) tbstr= "  SURF";
				else tbstr= "ADD";
				break;
			case 1:		tbstr= "VIEW";		break;
			case 2:		tbstr= "EDIT";		break;
			case 3:		tbstr= "OBJECT";	break;
			case 4:		tbstr= "OBJECT";	break;
			case 5:		tbstr= "MESH";		break;
			case 6:		tbstr= "CURVE";		break;
			case 7:		tbstr= "KEY";		break;
			case 8:		tbstr= "RENDER";	break;
			case 9:		tbstr= "DIV";		break;
		}
	}
	
/* TOPICS */
	else {
		
		if(G.obedit) {
			addmode= G.obedit->type;
		}
		
		
/* VIEW TOPICS */
		if(tbmain==1) {
			switch(y) {
				case 0: tbstr= "";		tbstr1= "";	break;
				case 1: tbstr= "";		tbstr1= "";	break;
				case 2: tbstr= "";		tbstr1= "";	break;
				case 3: tbstr= "";		tbstr1= "";	break;
				case 4: tbstr= "Centre";		tbstr1= "c";	break;
				case 5: tbstr= "Home";			tbstr1= "C";	break;
				case 6: tbstr= "";		tbstr1= "";	break;
				case 7: tbstr= "";		tbstr1= "";	break;
				case 8: tbstr= "";		tbstr1= "";	break;
				case 9: tbstr= "Zbuffer";		tbstr1= "z";	break;
				case 10: tbstr= "";		tbstr1= "";	break;
				case 11: tbstr= "";		tbstr1= "";	break;
			}
		}

/* EDIT TOPICS */
		else if(tbmain==2) {
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
		else if(tbmain==0) {

			if(addmode==0) {
				switch(y) {
					case 0: tbstr= "Mesh";		tbstr1= ">>";	tbval=OB_MESH; break;
					case 1: tbstr= "Curve";		tbstr1= ">>";	tbval=OB_CURVE; break;
					case 2: tbstr= "Surface";	tbstr1= ">>";	tbval=OB_SURF; break;
					case 3: tbstr= "Text";		tbstr1= "";		tbval=OB_FONT; tbfunc= add_font_prim; break;
					case 4: tbstr= "MetaBall";	tbstr1= "";		tbval=OB_MBALL; tbfunc= add_primitiveMball; break;
					case 5: tbstr= "Empty";		tbstr1= "A";	tbval=OB_EMPTY; break;
					case 6: tbstr= "";			tbstr1= "";		tbval=0; break;
					case 7: tbstr= "Camera";	tbstr1= "A";	tbval=OB_CAMERA; break;
					case 8: tbstr= "Lamp";		tbstr1= "A";	tbval=OB_LAMP; break;
					case 9: tbstr= "Ika";		tbstr1= "A";	tbval=OB_IKA; break;
#ifdef FREE
					case 10: tbstr= "";			tbstr1= "A";	tbval=0; break;
					case 11: tbstr= "";			tbstr1= "A";	tbval=0; break;
#else
					case 10: tbstr= "Sector";	tbstr1= "A";	tbval=OB_SECTOR; break;
					case 11: tbstr= "Life";		tbstr1= "A";	tbval=OB_LIFE; break;
#endif
					case 12: tbstr= "";			tbstr1= "";		tbval=0; break;
					case 13: tbstr= "Lattice";	tbstr1= "A";	tbval=OB_LATTICE; break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= (void (*)() )add_object_draw;
			}
			else if(addmode==OB_MESH || addmode==OB_SECTOR  || addmode==OB_LIFE) {		
				switch(y) {
					case 0: tbstr= ">Plane";	tbstr1= "A";	tbval=0; break;
					case 1: tbstr= ">Cube";		tbstr1= "A";	tbval=1; break;
					case 2: tbstr= ">Circle";	tbstr1= "A";	tbval=4; break;
					case 3: tbstr= ">UVsphere";	tbstr1= "A";	tbval=11; break;
					case 4: tbstr= ">Icosphere";tbstr1= "A";	tbval=12; break;
					case 5: tbstr= ">Cylinder";	tbstr1= "A";	tbval=5; break;
					case 6: tbstr= ">Tube";		tbstr1= "A";	tbval=6; break;
					case 7: tbstr= ">Cone";		tbstr1= "A";	tbval=7; break;
					case 8: tbstr= ">";			tbstr1= "";				break;
					case 9: tbstr= ">Grid";		tbstr1= "A";	tbval=10; break;
					case 10: tbstr= ">";		tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";		break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveMesh;
			}
			else if(addmode==OB_SURF) {
				switch(y) {
					case 0: tbstr= ">Curve";		tbstr1= "A";	tbval=0; break;
					case 1: tbstr= ">Circle";	tbstr1= "A";	tbval=1; break;
					case 2: tbstr= ">Surface";	tbstr1= "A";	tbval=2; break;
					case 3: tbstr= ">Tube";		tbstr1= "A";	tbval=3; break;
					case 4: tbstr= ">Sphere";	tbstr1= "A";	tbval=4; break;
					case 5: tbstr= ">Donut";		tbstr1= "A";	tbval=5; break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= "";			tbstr1= "";		break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= addprimitiveNurb;
			}
			else if(addmode==OB_CURVE) {
				switch(y) {
					case 0: tbstr= ">Bezier Curve";tbstr1= "A";	tbval=10; break;
					case 1: tbstr= ">Bezier Circle";tbstr1= "A";	tbval=11; break;
					case 2: tbstr= "";			tbstr1= "";		break;
					case 3: tbstr= "";			tbstr1= "";		break;
					case 4: tbstr= ">Nurbs Curve";tbstr1= "A";	tbval=40; break;
					case 5: tbstr= ">Nurbs Circle";tbstr1= "A";	tbval=41; break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= ">Path";		tbstr1= "A";	tbval=46; break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= addprimitiveCurve;
			}
			else if(addmode==OB_MBALL) {
				switch(y) {
					case 0: tbstr= "Ball";		tbstr1= "A";	tbval=1; break;
					case 1: tbstr= "";			tbstr1= "";		break;
					case 2: tbstr= "";			tbstr1= "";		break;
					case 3: tbstr= "";			tbstr1= "";		break;
					case 4: tbstr= "";			tbstr1= "";		break;
					case 5: tbstr= "";			tbstr1= "";		break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= "";			tbstr1= "";		break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= "Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveMball;
			}
		}
		
/* OB TOPICS 1 */
		else if(tbmain==3) {
			switch(y) {
				case 0: tbstr= "Clear size";	tbstr1= "a|s";	break;
				case 1: tbstr= "Clear rotation";	tbstr1= "a|r";	break;
				case 2: tbstr= "Clear location";	tbstr1= "a|l";	break;
				case 3: tbstr= "Clear origin";	tbstr1= "a|o";	break;
				case 4: tbstr= "Make Parent";	tbstr1= "c|p";	break;
				case 5: tbstr= "Clear Parent";	tbstr1= "a|p";	break;
				case 6: tbstr= "Make Track";	tbstr1= "c|t";	break;
				case 7: tbstr= "Clear Track";	tbstr1= "a|t";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "Image Displist";	tbstr1= "c|d";	break;
				case 11: tbstr= "Image Aspect";	tbstr1= "a|v";	break;
			}
		}
		
/* OB TOPICS 2 */
		else if(tbmain==4) {
			switch(y) {
				case 0: tbstr= "EditMode";		tbstr1= "Tab";	break;
				case 1: tbstr= "Move To Layer";	tbstr1= "m";	break;
				case 2: tbstr= "Delete";		tbstr1= "x";	break;
				case 3: tbstr= "Delete All";		tbstr1= "c|x";	break;
				case 4: tbstr= "Apply Size/Rot";	tbstr1= "c|a";	break;
				case 5: tbstr= "Apply Deform";		tbstr1= "c|A";	break;
				case 6: tbstr= "Join";		tbstr1= "c|j";	break;
				case 7: tbstr= "make local";	tbstr1= "l";	break;
				case 8: tbstr= "select Links";		tbstr1= "L";	break;
				case 9: tbstr= "make Links";	tbstr1= "c|l";	break;
				case 10: tbstr= "Copy menu";	tbstr1= "c|c";	break;
				case 11: tbstr= "Convert menu";	tbstr1= "a|c";	break;
			}
		}

/* mesh TOPICS */
		else if(tbmain==5) {
			switch(y) {
				case 0: tbstr= "Select Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Deselect Linked";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "Delete Menu";	tbstr1= "x";	break;
				case 4: tbstr= "Make edge/face";	tbstr1= "f";	break;
				case 5: tbstr= "Fill";			tbstr1= "F";	break;
				case 6: tbstr= "Split";			tbstr1= "y";	break;
				case 7: tbstr= "Undo/reload";	tbstr1= "u";	break;
				case 8: tbstr= "Calc Normals";	tbstr1= "c|n";	break;
				case 9: tbstr= "Separate";		tbstr1= "p";	break;
				case 10: tbstr= "Write Videosc";	tbstr1= "a|w";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	
/* CURVE TOPICS */
		else if(tbmain==6) {
			switch(y) {
				case 0: tbstr= "Select Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Deselect Link";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "Delete Menu";	tbstr1= "x";	break;
				case 4: tbstr= "Make Segment";	tbstr1= "f";	break;
				case 5: tbstr= "Cyclic";		tbstr1= "c";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
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
				case 1: tbstr= "Show";		tbstr1= "k";	break;
				case 2: tbstr= "Next";		tbstr1= "PageUp";	break;
				case 3: tbstr= "Prev";		tbstr1= "PageDn";	break;
				case 4: tbstr= "Show+sel";	tbstr1= "K";	break;
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
				case 0: tbstr= "Load File As";	tbstr1= "F1";	break;
				case 1: tbstr= "Load";			tbstr1= "c|o";		break;
				case 2: tbstr= "Save File As";	tbstr1= "F2";	break;
				case 3: tbstr= "Save";			tbstr1= "c|w";	break;
				case 4: tbstr= "Save Image";	tbstr1= "F3";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	}
}

/* ******************** INIT ************************** */

#undef getmatrix

void bgnpupdraw()	/* doet ook font, patterns, linestyle, colors */
{
	static long firsttime= 1;
	fmfonthandle helvfont;
	long x;
	
	pushattributes();
	linewidth(1);
	setlinestyle(0);
	zbuffer(0);
	
	tboldwin= mywinget();
	if(tboldwin) {
		mmode(MPROJECTION);
		getmatrix(tbprojmat);
		mmode(MVIEWING);
		getmatrix(tbwinmat);
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
	tbfontyofs= (TBOXH-7)/2;	/* toolbox, hier stond ooit getheigh */
	confontyofs= 9;				/* voor confirm */
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
		mywinset(tboldwin);
		mmode(MPROJECTION);
		loadmatrix(tbprojmat);
		mmode(MVIEWING);
		loadmatrix(tbwinmat);
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
			qenter(EXECUTE, 0);
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
	
	tbx1= mval[0]-tbmemx;
	tby1= mval[1]-tbmemy;
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
			tbmemx= mval[0]-(tbx1);
			tbmemy= mval[1]-(tby1);
	}
	
	color(0);
	sboxfs(tbx1-5, tby1-5, tbx2+5, tby2+5);
	
	endpupdraw();
}

void tbox_embossbox(x1, y1, x2, y2, type)	/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
short x1, y1, x2, y2;
{
	/* in deze fie wordt raar getekend, zie hiervoor notities bovenin */

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
		
		setpattern(TBOXPAT);
		
		color(TBOXWHITE);
		sboxfs(x1+2, y2-2, x2-2, y2-2);	/* boven */
		color(TBOXBLACK);
		sboxfs(x1+2, y1+1, x2-1, y1+1);	/* onder */
		sboxfs(x2-1, y1+2, x2-1, y2-1);	/* rechts */
		setpattern(0);
		
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
		
		setpattern(TBOXPAT);
		
		color(TBOXWHITE);
		if(type==1) {
			sboxfs(x1+2, y1+2, x2-2, y1+2);	/* onder */
			sboxfs(x2-2, y1+1, x2-2, y2-2);	/* rechts */
			setpattern(0);
			
			color(TBOXBLACK);
			sboxfs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			
			color(TBOXWHITE);	/* tekst */
		}
		else {
			sboxs(x1+2, y1+2, x2-2, y1+2);	/* onder */

			color(TBOXBLACK);
			sboxfs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			sboxfs(x1+1, y1+2, x1+1, y2-1);	/* links */
			setpattern(0);
		}
	}
}


void tbox_embossbox_OLD(x1, y1, x2, y2, type)	/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
short x1, y1, x2, y2;
{
	/* deze fie werkt niet op iris! */
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
		x1= tbx1; x2= tbx1+TBOXXL;
	}
	else if(x==1) {
		x1= tbx1+TBOXXL;
		x2= x1+ TBOXXR;
	}
	
	y1= tby1+ (TBOXEL-y-1)*TBOXH;
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
	if(mval[0]<TBOXXL) *x= 0;
	else *x= 1;
	
	*y= mval[1]-tby1;
	*y/= TBOXH;
	*y= TBOXEL- *y-1;
	if(*y<0) *y= 0;
	if(*y>TBOXEL-1) *y= TBOXEL-1;
	
}

void drawtoolbox()
{
	long x, y, actx, acty, type;
	
	tbox_getactive(&actx, &acty);

	for(x=0; x<2; x++) {
		for(y=0; y<TBOXEL; y++) {
			
			if(x==0) type= 0; 
			else type= 2;
			
			if(actx==x && acty==y) type++;
			if(type==0) {
				if(tbmain==y) type= 1;
			}
			
			tbox_drawelem(x, y, type);
			
		}
	}
}


void toolbox()
{
	long actx, acty;
	short event, val, mval[2], xo= -1, yo=0;
	
	bgntoolbox();
	if(helv10==0) return;
	
	drawtoolbox();
	
	/* 
	 *	De aktieve window wordt in queue terug gestopt.
	 */
	 
	while(1) {
		event= 0;
		if(qtest()) event= extern_qread(&val);
		
		if(event) {
			switch(event) {
				case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
					if(val==0 || event==RETKEY) {
						tbox_getactive(&actx, &acty);
						tbox_setinfo(actx, acty);
						
						if(event==RIGHTMOUSE) {
							if(addmode) {
								addmode= 0;
								drawtoolbox();
							}
						}
						else if(tbstr1 && tbstr1[0]=='>') {
							addmode= tbval;
							drawtoolbox();
						}
						else {
							endtoolbox();

							tbox_execute();
							return;
						}
					}
					break;
				case ESCKEY:
					/* altkeys: om conflicten met overdraw en stow/push/pop te voorkomen */
				case LEFTALTKEY:
				case RIGHTALTKEY:
					if(val) endtoolbox();
					return;
			}
		}
		else sginap(2);
		
		tbox_getmouse(mval);
		if(mval[0]<tbx1-10 || mval[0]>tbx2+10 || mval[1]<tby1-10 || mval[1]>tby2+10) break;
		
		tbox_getactive(&actx, &acty);
		
		/* muisafhandeling en redraw */
		if(xo!=actx || yo!=acty) {
			if(actx==0) {
				tbmain= acty;
				addmode= 0;
				drawtoolbox();
			}
			else if(xo> -1) {
				if(xo==0) tbox_drawelem(xo, yo, 1);
				else tbox_drawelem(xo, yo, 2);
				tbox_drawelem(actx, acty, 3);
			}
			xo= actx;
			yo= acty;
		}
	}
	
	endtoolbox();
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
	short mousedown= 0, val, value=0, event, len, mval[2], xmax, ymax, startx, starty, endx, endy;

	if(title==0 || item==0) return 0;
	if(G.background) return 0;
	
	bgnpupdraw();
	
	len= fmgetstrwidth(helv10, title);
	xmax= fmgetstrwidth(helv10, item);
	
	if( xmax>len ) len= xmax;
	if(len==0) len= 100;
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
		if(qtest()) event= extern_qread(&val);
		
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
				case ESCKEY:
					value= 1;
					break;
			}
		}
		else sginap(2);
		
		tbox_getmouse(mval);
		if(mval[0]<startx-10 || mval[0]>endx+10 || mval[1]<starty-10 || mval[1]>endy+10) value= 1;
		
	}

	color(0);
	sboxfs(startx-1, starty-5, endx+5, endy+1);
	endpupdraw();
	
	return (value-1);
}


long saveover(str)
char *str;
{
	long file;
	
	if(G.f & G_DISABLE_OK) return 1;
	
	file= open(str, O_RDONLY);
	close(file);
	if(file==-1) return 1;
	else if(confirm("SAVE OVER", str)) return 1;
	
	return 0;
}

short okee(char *str)
{
	if(G.f & G_DISABLE_OK) return 1;
	return confirm("OK?", str);
}

void error(char *str)
{
	char str1[100];
	
	if(G.background) {
		printf("ERROR %s\n", str);
		return;
	}
	
	if(strlen(str)>90) str[90]= 0;
	sprintf(str1, "ERROR: %s", str);
	confirm("", str1);
}

void errorstr(char *str1, char *str2, char *str3)
{
	char str[256];
	
	if(str1 && strlen(str1)>79) str1[79]= 0;
	if(str2 && strlen(str2)>79) str2[79]= 0;
	if(str3 && strlen(str3)>79) str3[79]= 0;

	strcpy(str, "ERROR ");
	if(str1) strcat(str, str1);
	strcat(str, " ");
	if(str2) strcat(str, str2);
	strcat(str, " ");
	if(str3) strcat(str, str3);

	if(G.background) {
		printf("ERROR %s\n", str);
		return;
	}
	
	confirm("", str);
	
}

void notice(char *str)
{
	confirm("", str);
}

void pupmenu_draw(startx, starty, endx, endy, items, title, spoin, sel)
int startx, starty, endx, endy, items;
char *title, **spoin;
short sel;
{
	int a=0;
	
	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	sboxfs(startx+5, starty-10, endx+5, endy-10);
	setpattern(0);

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
	static int lastselected= 1;
	static char laststring[100]="";
	int len, height, a, items, retval[100], xmax, ymax, startx, starty, endx, endy;
	int act, acto= -1, value=0, event;
	short val, mval[2];
	short mouseymove;
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
				if( astr[1] ) {
					items++;
					retval[items]= items;
					spoin[items]= astr+1;
				}
				*astr= 0;
				break;
			case '`':
				*astr='|';
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
	
	if(strcmp(laststring, spoin[0])!=0) lastselected= 1;
	strcpy(laststring, spoin[0]);
	
	startx= mval[0]-len/2;
	if(lastselected>=0 && lastselected<items) {
		starty= mval[1]-height+TBOXH/2+lastselected*TBOXH;
	}
	else starty= mval[1]-height/2;
	
	mouseymove= 0;
	
	if(startx<10) startx= 10;
	if(starty<10) {
		mouseymove= 10-starty;
		starty= 10;
	}
	
	endx= startx+len;
	endy= starty+height;
	if(endx>xmax) {
		endx= xmax-10;
		startx= endx-len;
	}
	if(endy>ymax) {
		mouseymove= ymax-endy-10;
		endy= ymax-10;
		starty= endy-height;
		
	}

	if(mouseymove) {
		setvaluator(MOUSEY, mouseymove+mval[1], 0, ymax);
		mouseymove= mval[1];	/* wer terug */
	}

	pupmenu_draw(startx, starty, endx, endy, items, title, spoin, 1);

	while(value==0) {
		event= 0;
		if(qtest()) event= extern_qread(&val);
		
		if(event) {
			switch(event) {
			
			case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
				if(val) {
					value= 2;
				}
				break;
			case ONEKEY: if(val) {value= 2; act= 1; break;}
			case TWOKEY: if(val) {value= 2; act= 2; break;}
			case THREEKEY: if(val) {value= 2; act= 3; break;}
			case FOURKEY: if(val) {value= 2; act= 4; break;}
			case FIVEKEY: if(val) {value= 2; act= 5; break;}
			case SIXKEY: if(val) {value= 2; act= 6; break;}
			case SEVENKEY: if(val) {value= 2; act= 7; break;}
			case EIGHTKEY: if(val) {value= 2; act= 8; break;}
			case NINEKEY: if(val) {value= 2; act= 9; break;}
			case ZEROKEY: if(val) {value= 2; act= 10; break;}
			
			case DOWNARROWKEY:
				if(val && endy<ymax-TBOXH) {
					color(0);
					sboxfs(startx-1, starty-10, endx+5, endy+10);
					starty+= TBOXH;
					endy+= TBOXH;
					pupmenu_draw(startx, starty, endx, endy, items, title, spoin, 1);
				}
				break;
			case UPARROWKEY:
				if(val && starty>TBOXH) {
					color(0);
					sboxfs(startx-1, starty-10, endx+5, endy+10);					starty-= TBOXH;
					endy-= TBOXH;
					pupmenu_draw(startx, starty, endx, endy, items, title, spoin, 1);
				}
				break;
			case ESCKEY:
			case LEFTALTKEY:
			case RIGHTALTKEY:
				if(val) value= 1;
				break;
			}
		}
		else sginap(2);
		
		/* anders verandert act (toetsen 1,2,3 enz) */
		if(value==2) break;
		
		tbox_getmouse(mval);
		if(mval[0]<startx-20 || mval[0]>endx+20 || mval[1]<starty-20 || mval[1]>endy+20) value= 1;
		
		act= items-1- (mval[1]-starty)/TBOXH;
		if(act!=acto && act<items) {
		
			a= endy-20;
			
			/* teken acto */
			if(title && acto==0) ;
			else if(acto>=0) {		/* vergelijken, acto is init op -1 */
				tbox_embossbox(startx, a-acto*TBOXH, endx, a-acto*TBOXH+TBOXH, 2);
				color(TBOXBLACK);
				cmov2i(startx+5, a-acto*TBOXH+tbfontyofs);
				fmprstr(spoin[acto]);
			}
			/* teken act */
			if(title && act==0) ;
			else if(act>=0) {
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

	if(mouseymove) {
		setvaluator(MOUSEY, mouseymove, 0, ymax);
	}

	if(value==2 && act>=0 && act<100) {
		lastselected= act;
		return retval[act];
	}
	return -1;
}

