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

/* 	buttons!	  		         			*/


/* 

 - Include:	

	#include "/usr/people/include/Button.h"
	#include "/usr/people/include/util.h"
	#include <fmclient.h>

- Denk aan qdevices in hoofdprog:

	qdevice(INPUTCHANGE);
    	qdevice(LEFTMOUSE);
    	qdevice(RAWKEYBD);

- Compileren met ... Button.o util.o -lgl_s -lc_s -lfm_s -lm ... -o ...

- En dit als initialisatie
	1. Window openen, of windownrs onthouden:
		win= winopen("Test");

	2. Font(s) aanmaken:
		fmfonthandle helvfont,helv12;	 (pointer naar long)

		fminit();
		if( (helvfont=fmfindfont("Helvetica-Bold")) == 0) exit(1);
		helv12=fmscalefont(helvfont,12.0);

	3. Butblock definieren:
		DefButBlock("Naam",win,helv12,max,col,shape);

		- "Naam" is een charpointer, blokken met dezelfde naam
		   worden automatisch vrijgegeven en weer gereserveerd
		- win is een long, het GL windownummer
		- helv12 is een fmfonthandle (=long *)
		- max is een short, het maximaal aantal buttons
		- col is een short, het nummer van de met DefButCol gemaakte
		  kleuren. al aanwezig zijn 0 (cmap) en 1 (overdraw)
		- shape is een short, de vorm van de button, 0 is emboss.1 iets mooier, 
			2 alleen geschikt voor RGBmode.

		Alle DefButs na aanroep van DefButBlock horen bij dit blok en
		hebben als default de met DefButBlock bepaalde eigenschappen.
		Veranderd kunnen daarna nog:
			- SetButFont(fmfonthandle)
			- SetButCol(nr)
			- SetButShape(nr)
	4. Kleuren:
		Bij eerste aanroep worden kleuren van ButCol 0 uitgezocht,  zie
		de funktie initbutcolors().
		- Bij colormapbuttons:
			Andere cmapkleuren en overdraw zelf instellen met mapcolor() en 
			initialiseren met DefButCol().
			Mapcolors eventueel bij einde programma weer terugzetten.
			De drawmode() bljft staan zoals button is getekend.
		- Bij RGB buttons:
			alleen initialiseren DefButCol noodzakelijk.
			
	5. Buttons aanmaken:
		[ b= ] DefBut(type,nr,"naam",x,y,b,h [,adr,min,max,a1,a2,a3...] )

		- b: DefBut geeft (struct But *) terug.
		- type: aangeven met #defines uit de include. Het bestaat uit 
		  maximaal vier gORde getallen:
		  soort:
			BUT		activeringsbutton, in en meteen weer uit.
			TOG		toggle, of aan of uit
			ROW		rijbutton, altijd maar 1 van de rij ingedrukt
			SLI		slider
			NUM		met muisbeweging een getal veranderen
			TEX		tekstbutton
			TOG3	driestand toggle
			BUTRET	variatie op BUT, keert na indrukken meteen terug
			TOGN	(tog NOT) toggle, of aan of uit. Hier zijn 0 en 1 omgekeerd
			LABEL	alleen tekst in kleur van select
			MENU	werkt als pupmenu. De aktieve optie wordt weergegeven.
			ICONROW	er wordt 1 button getekend, met muis kan gescrold worden
		  variabele:
			CHA	char
			SHO	short
			LON	long
			FLO	float
			DOU	double
			FUN	functie
		  bitcodes:
			BIT	activeert bitvariabele
		  bitnummer:
			0-31	bitnummer uit variabele (van rechts af)

		  voorbeeld voor type:
			BUT		(zonder variabele)
			SLI|CHA		(slider die char verandert)		  
			TOG|SHO|BIT|0	(van de short toggelt bitje 0)
		- nr: is de returncode van DoButtons(), of wordt aan SetButtons()
                  doorgegeven. Verschillende buttons kunnen dezelfde nrs hebben.
		- "naam": is charpointer van constante of variabele.
		- x,y,b,h: startco (x,y) en breedte,hoogte.
		- adr: is pointer naar char,short,etc. Als de pointer nul is,  wordt geen
		  button aangemaakt,  wel wordt op de plek in kleur 'back' een rechthoek getekend.
		  Op deze wijze kan een button gewist worden.
		- min,max (float) bij sliders en numbuts
		- a1,a2,a3... alleen bij bepaalde types buttons (zie verderop) 

	5. Controle functies:
		- afhandeling buttons:
			nr=DoButtons();	(in hoofdlus)
			Deze functie kijkt alleen in de aktieve window: 
			win= winget().
			als nr= 0 is er geen button geselecteerd. Anders is
			nr het returnnr van de button.
		- alles vrijgeven:
			FreeButs();
			Alleen op eind gebruiken!!!
		- butblock vrijgeven:
			FreeButBlock("naam");
		- alle buts van nr 'min' tot 'max' opnieuw tekenen:
			SetButs(min,max);
			Na deze funktie is de aktieve window ongewijzigd.
		- nieuw kleurblok aanmaken:
			DefButCol(nr,DRAWMODE,back,pap_sel,pap_dsel,pen_sel,pen_dsel,b1,b2,b3,b4);
			Dit is blok 0:
			DefButCol(0,NORMALDRAW,100, 101,102,103,104, 104,105,106,103);
			nr= kleurbloknummer.
			DRAWMODE is OVERDRAW, NORMALDRAW (=cmap). of RGBDRAW.
			De rest zijn ulongs, cmap of rgb kleuren.
		- kleur aangeven:
			SetButCol(nr)
			nr is short van 2-20 (0 en 1 reeds aanwezig) of een met
			DefButCol aangemaakt kleurblok

	6. Ikonen
		- aanmaken iconstruct
			DefButIcon(nr, rect, xim, yim, xofs, yofs);
			nr is nummer van de iconstruct (max 10).
			rect is het 32 bits plaatje.
			xim yim is de totale afmeting.
			xofs,  yofs is het grid waarin de buttons staan
		- Button met ikoon maken: 
			de naam van de button = "ICON 1 12 4" betekent
				iconstruct 1,  icon '12' in de x, '4' in de y

- De Button types:
	1. BUT
		DefBut(BUT[|VAR],nr,"naam",x,y,b,h,[poin]);
		Een variabele doorgeven hoeft niet.
		Als variabele wordt doorgegeven :
			- FUN: funktie wordt aangeroepen bij indrukken
			- alle andere: als variabele=0 button is uit, anders in.
			  kan gebruikt worden om status van variabelen aan te geven.
		Type BIT kan hier niet.
	2. TOG / TOGN
		DefBut(TOG|VAR[|BIT|bitnr],nr,"naam",x,y,b,h,poin);
		De variabele wordt afhankelijk van de stand op 0 of 1 gezet.
		Een funktie wordt met 0 of 1 aangeroepen.
		Toevoegen van |BIT|bitnr zet bitnr van de VAR.
		Type TOGN zet button op select bij NOT TRUE.
		
	3. ROW
		DefBut(ROW|VAR,nr,"naam",x,y,b,h,poin,rijnr,const);
		'rijnr' en 'const' altijd als float doorgeven!
		'rijnr' bepaalt welke buttons bij elkaar horen. 'const' wordt de 
		inhoud van 'poin'.
		Vartypes FUN en BIT kunnen hier niet.
	4. SLI
		DefBut(SLI|VAR,nr,"naam",x,y,b,h,poin,min,max,cmapnr,ofs);
		'min' en 'max' altijd als float doorgeven.
		Als 'cmapnr'!=0 is het een kleurslider. 'ofs' bepaalt hoeveel
		de 'poin' terug moet om de pointer naar rood te vinden.
		Vartypes FUN en BIT kunnen hier niet.
	5. NUM
		DefBut(NUM|SHO,nr,"Naam",x,y,b,h,poin,min,max);
		min en max altijd als float doorgeven!.
		Vartypes FUN en BIT kunnen hier niet.
	6. TEX
		DefBut(TEX,nr,"Naam",x,y,b,h,poin,min,max);
		VARtype is standaard CHA. max is lengte string (als float doorgeven)
		als texbut met enter wordt afgesloten wordt but->min op 1.0
		gezet, anders 0. (voor o.a. OK mededelingen)
	7. TOG3
		DefBut(TOG3|CHA|BIT|bitnr,nr,"naam",x,y,b,h,poin);
		Een vreselijke button!
		Dit is ALTIJD een CHA|BIT button. 'poin' kan CHA, SHO en LON zijn,
		MAAR: wordt als char geinterpreteerd.
		Bij de derde stand wordt 'bitnr' van *((char*)poin+2) gezet.
		Als je een long doorgeeft als TOG3|CHA|BIT|0  krijg je resp de
		waarden 0, 0x01000000, 0x01000100
	8. LABEL
		DefBut(LABEL,nr,"naam",x,y,b,h);
		Alleen tekst. Deze wordt gecentreerd rond x+b en y+h.
	9. MENU
		DefBut(MENU|VAR, nr, string, x, y, b, h, poin);
		In string staat een aantal menuopties,  met syntax zoals pupmenu:
			- menu onderdelen scheiden met '|'
			- returnval aangeven met %x[nr]	(bijv: %x12). Zonder returnval wordt
				automatisch geteld: beginnend bij 0.
		In poin wordt de waarde geschreven.
	10. ICONROW
		DefBut(ICONROW|VAR, nr, "ICON a b c", x, y, b, h, poin, min, max);
		De waarde in poin bepaalt welke icon getekend wordt, van
		icon c tot icon (c+max-min).
		
	
******************************************************			*/

/* Omschrijving ButGroups
 * 
 * 1. ButGroups vormen een extra laag tussen gebruiker en button structuur
 *    die het mogelijk maakt om een relatieve positie / grootte van een
 *    groep buttons binnen een rechthoekig kader aan te geven.
 * 2. Alle functienamen beginnen met BG.
 * 
 * Functies:
 * 
 * BGflush()
 *	Wist alle nog onverwerkte buttons. Bij voorkeur aanroepen
 *	voordat je begint.
 * BGadd(, , , , ...)
 *	Voegt een Button aan de lijst toe. Zelfde definite als DefBut, 
 *	met dit verschil dat x en y genegeerd worden, en dat breedte
 *	en hoogte relatieve waardes zijn. Een button met breedte 20
 *	wordt twee keer zo breed als een button met breedte 10. Als y
 *	hoogte voor een regel wordt de maximale y van die regel aan-
 *	gehouden. Buttons houden hun eigen hoogte en lijnen afhankelijk
 *	van de richting waarin de buttons getekend worden met de boven of
 *	onderkant van de regel (niet getest).
 * BGnewline()
 *	Aanroepen om aan te geven dat volgende buttons op een nieuwe 
 *	regel moeten komen. Meerdere keren aanroepen om extra spatiering
 *	tussen regels te krijgen is toegestaan. Niet aanroepen bij begin
 *	en eind van BG defintie, tenzij je daar ook spatiering wilt.
 * BGposition(xofs, yofs, width, height)
 *	Om kader aan te geven waarbinnen buttons getekent moeten worden.
 *	Kan op elk moment binnen BG definite aangeroepen worden, maar in
 *	ieder geval voor BGdraw. Start waardes (0, 0, 100, 100).
 * BGspacing(x, y)
 *	Om absolute globale spatiering tussen buttons aan te geven.
 *	Kan op elk moment binnen BG definite aangeroepen worden, maar in
 *	ieder geval voor BGdraw. Start waardes (2, 2).
 * BGdirection(dir)
 *	Of buttons van onder naar boven, of van boven naar onder getekend
 *	moeten worden. Mogelijke waardes: 'u' (up), of 'd' (down).
 *	Kan op elk moment binnen BG definite aangeroepen worden, maar in
 *	ieder geval voor BGdraw. Start waarde ('u').
 * BGdraw()
 *	Tekent gedefinieerde buttons binnen rechthoek BGposition(), met
 *	spatiering BGspacing() in de richting BGdirection(). Hierna wordt
 *	de lijst geflushed.
 */
 
 
#include <string.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include "/usr/people/include/Button.h"
#include <stdio.h>
#include <local/util.h>

#define TBOXPAT		1234

/* ************ GLOBALS ************* */


struct ButBlock *BGfirstbutblock= 0;
char raw[255];

struct But *BGfirst;
short BGaantal;
short BGteller;
short BGwin;
fmfonthandle BGfont;
short BGcol;
short BGdrawtype;
void (*ButDrawFunc)();
void (*SliderDrawFunc)();
void * BGbutfunc = 0;
void (*colorfunc)();

float Bxasp=1;

struct ButCol BGbutcol[20]= {
	{ NORMALDRAW,0,
	100,
	101,102,103,104,
	104,105,106,103},
	{ OVERDRAW,0,
	0,
	3,1,1,3,
	1,3,1,3 }
};

struct ButIcon BGicon[10];

/* ************ BUTGRP GLOBALS ************* */

struct ListBase _butbase = {
	0,0};
struct ListBase *butbase = &_butbase;
ushort BG_xsp = 2 , BG_ysp = 2,BG_w=100, BG_h = 100;
short BG_x = 0, BG_y = 0, BG_dir = 'u';

/* ************ FUNC ************* */

void qualafhandeling(var)
short *var;
{
	*var=0;
	if(getbutton(RIGHTSHIFTKEY)) (*var) +=1;
	if(getbutton(LEFTSHIFTKEY)) (*var) +=2;
	if(getbutton(RIGHTCTRLKEY)) (*var) +=16;
	if(getbutton(LEFTCTRLKEY)) (*var) +=32;
	if(getbutton(RIGHTALTKEY)) (*var) +=4;
	if(getbutton(LEFTALTKEY)) (*var) +=8;
}


void initrawcodes()
{
	short a;

	for(a=0;a<255;a++) raw[a]=0;

	raw[AKEY]='a';	
	raw[BKEY]='b';
	raw[CKEY]='c';	
	raw[DKEY]='d';
	raw[EKEY]='e';	
	raw[FKEY]='f';
	raw[GKEY]='g';	
	raw[HKEY]='h';
	raw[IKEY]='i';	
	raw[JKEY]='j';
	raw[KKEY]='k';	
	raw[LKEY]='l';
	raw[MKEY]='m';	
	raw[NKEY]='n';
	raw[OKEY]='o';	
	raw[PKEY]='p';
	raw[QKEY]='q';	
	raw[RKEY]='r';
	raw[SKEY]='s';	
	raw[TKEY]='t';
	raw[UKEY]='u';	
	raw[VKEY]='v';
	raw[WKEY]='w';	
	raw[XKEY]='x';
	raw[YKEY]='y';	
	raw[ZKEY]='z';
	raw[SPACEKEY]=' ';

	raw[AKEY+100]='A';	
	raw[BKEY+100]='B';
	raw[CKEY+100]='C';	
	raw[DKEY+100]='D';
	raw[EKEY+100]='E';	
	raw[FKEY+100]='F';
	raw[GKEY+100]='G';	
	raw[HKEY+100]='H';
	raw[IKEY+100]='I';	
	raw[JKEY+100]='J';
	raw[KKEY+100]='K';	
	raw[LKEY+100]='L';
	raw[MKEY+100]='M';	
	raw[NKEY+100]='N';
	raw[OKEY+100]='O';	
	raw[PKEY+100]='P';
	raw[QKEY+100]='Q';	
	raw[RKEY+100]='R';
	raw[SKEY+100]='S';	
	raw[TKEY+100]='T';
	raw[UKEY+100]='U';	
	raw[VKEY+100]='V';
	raw[WKEY+100]='W';	
	raw[XKEY+100]='X';
	raw[YKEY+100]='Y';	
	raw[ZKEY+100]='Z';

	raw[PAD1]='1';	
	raw[PAD2]='2';
	raw[PAD3]='3';	
	raw[PAD4]='4';
	raw[PAD5]='5';	
	raw[PAD6]='6';
	raw[PAD7]='7';	
	raw[PAD8]='8';
	raw[PAD9]='9';	
	raw[PAD0]='0';
	raw[PAD1+100]='1';	
	raw[PAD2+100]='2';
	raw[PAD3+100]='3';	
	raw[PAD4+100]='4';
	raw[PAD5+100]='5';	
	raw[PAD6+100]='6';
	raw[PAD7+100]='7';	
	raw[PAD8+100]='8';
	raw[PAD9+100]='9';	
	raw[PAD0+100]='0';

	raw[ONEKEY]='1';	
	raw[TWOKEY]='2';
	raw[THREEKEY]='3';	
	raw[FOURKEY]='4';
	raw[FIVEKEY]='5';	
	raw[SIXKEY]='6';
	raw[SEVENKEY]='7';	
	raw[EIGHTKEY]='8';
	raw[NINEKEY]='9';	
	raw[ZEROKEY]='0';

	raw[ONEKEY+100]='!';	
	raw[TWOKEY+100]='@';
	raw[THREEKEY+100]='#';	
	raw[FOURKEY+100]='$';
	raw[FIVEKEY+100]='%';	
	raw[SIXKEY+100]='^';
	raw[SEVENKEY+100]='&';	
	raw[EIGHTKEY+100]='*';
	raw[NINEKEY+100]='(';	
	raw[ZEROKEY+100]=')';

	raw[55]='`';	
	raw[45]=',';
	raw[52]='.';	
	raw[53]='/';
	raw[43]=';';	
	raw[50]= '\'';
	raw[49]='[';	
	raw[56]=']';
	raw[57]= '\\';	
	raw[47]='-';
	raw[54]='=';

	raw[55+100]='~';	
	raw[45+100]='<';
	raw[52+100]='>';	
	raw[53+100]='?';
	raw[43+100]=':';	
	raw[50+100]= '"';
	raw[49+100]='{';	
	raw[56+100]='}';
	raw[57+100]= '|';	
	raw[47+100]='_';
	raw[54+100]='+';

	raw[51]='\n';	/* enter */
	raw[PADENTER]='\n';
	raw[61]='\b';	/* backsp */
	raw[LEFTARROWKEY]= 254;
	raw[RIGHTARROWKEY]= 255;
	raw[LEFTARROWKEY+100]= 252;
	raw[RIGHTARROWKEY+100]= 253;
	raw[UPARROWKEY]= 250;
	raw[DOWNARROWKEY]= 251;
	raw[UPARROWKEY+100]= 248;
	raw[DOWNARROWKEY+100]= 249;
}

short rawtoascii(a)
short a;
{
	short shift=0;

	if(a<=0 || a>100) return 0;
	if(getbutton(RIGHTSHIFTKEY)) shift=100;
	else if(getbutton(LEFTSHIFTKEY)) shift=100;
	else if(getvaluator(CAPSLOCKKEY)) shift=100;

	if(raw[a]>249 && raw[a]<256);
	else if(raw[a]<32 || raw[a]>127) shift= 0;

	return (short)raw[a+shift];
}

short asciitoraw(a)
short a;
{
	/* return >100: met SHIFT */
	short nr;
	
	if(a==0) return 0;
	
	for(nr=1; nr<256; nr++) {
		if(raw[nr]==a) return nr;
	}
	return 0;
}

float Bwinmat[4][4];
long getsizex, getsizey;

void graphics_to_window(x, y)	/* voor rectwrite b.v. */
float *x, *y;
{
	float gx, gy;
	int sx, sy;
	
	gx= *x;
	gy= *y;
	*x= getsizex*(0.5+ 0.5*(gx*Bwinmat[0][0]+ gy*Bwinmat[1][0]+ Bwinmat[3][0]));
	*y= getsizey*(0.5+ 0.5*(gx*Bwinmat[0][1]+ gy*Bwinmat[1][1]+ Bwinmat[3][1]));
	
	/* getorigin(&sx, &sy); */
	/* *x+= sx; */
	/* *y+= sy; */
}



void window_to_graphics(x, y)	/* voor muiscursor b.v. */
float *x, *y;
{
	float a, b, c, d, e, f, px, py;
	
	a= .5*getsizex*Bwinmat[0][0];
	b= .5*getsizex*Bwinmat[1][0];
	c= .5*getsizex*(1.0+Bwinmat[3][0]);

	d= .5*getsizey*Bwinmat[0][1];
	e= .5*getsizey*Bwinmat[1][1];
	f= .5*getsizey*(1.0+Bwinmat[3][1]);
	
	px= *x;
	py= *y;
	
	*y=  (a*(py-f) + d*(c-px))/(a*e-d*b);
	*x= (px- b*(*y)- c)/a;
	
}

void ButtonsGetmouse(adr)
short *adr;
{
	/* map muiscoordinaat invers naar geprojecteerde coordinaat */
	static Device mdev[2]= {MOUSEX, MOUSEY};
	float xwin, ywin, xt, yt;
	long x, y;

	if(adr==0) {
		getmatrix(Bwinmat);
		getsize(&getsizex, &getsizey);
		Bxasp= 2.0/(getsizex*Bwinmat[0][0]);
		
		return;
	}

	getdev(2, mdev, adr);

	/* coordinaat binnen window */
	getorigin(&x, &y);

	adr[0]-=x; 
	adr[1]-=y;

	xwin= adr[0];
	ywin= adr[1];
	window_to_graphics(&xwin, &ywin);
	adr[0]= xwin;
	adr[1]= ywin;
}


void DefButIcon(nr, rect, xim, yim, xofs, yofs)
short nr;
ulong *rect;
short xim, yim, xofs, yofs;
{

	BGicon[nr].rect= rect;
	BGicon[nr].xim= xim;
	BGicon[nr].yim= yim;
	BGicon[nr].xofs= xofs;
	BGicon[nr].yofs= yofs;
}

ulong temprect[40*40*4];


void DrawIcon(nr, paper, xnr, ynr, x, y)
short nr;
ulong paper;
short xnr, ynr, x, y;		/* xnr ynr is iconnr; x,y is tekencentrum */
{
	ulong *rii;		/* recticon */
	float xs, ys;
	int a, b, sizea, sizeb, rfac=256, gfac=256, bfac=256, fac;
	char *rd, *ri, *col;	/* rectdraw, recticon */
	
	rd= (char *)temprect;
	rii= BGicon[nr].rect;
	/* eerste pixels zijn zwart: grid, en daarbij rand: 3 pixels totaal offset*/
	rii+= (3+ynr*BGicon[nr].yofs)*BGicon[nr].xim+ xnr*BGicon[nr].xofs+3;
	/* en natuurlijk de andere rand eraf: */
	sizea= BGicon[nr].xofs-5;
	sizeb= BGicon[nr].yofs-5;
	
	if(*rii) {
		col= (char *)&paper;
		ri= (char *)rii;	/* eerste kleur icon==paperkleur */
		if(ri[3] && ri[2] && ri[1]) {
			rfac= (col[3]<<8)/ri[3];
			gfac= (col[2]<<8)/ri[2];
			bfac= (col[1]<<8)/ri[1];
		}
	}
	
	for(b=sizeb; b>0; b--) {
		ri= (char *)rii;
		for(a=sizea; a>0; a--, ri+=4, rd+=4) {
			fac= (rfac*ri[3])>>8;
			if(fac>255) rd[3]= 255; else rd[3]= fac;
			fac= (gfac*ri[2])>>8;
			if(fac>255) rd[2]= 255; else rd[2]= fac;
			fac= (bfac*ri[1])>>8;
			if(fac>255) rd[1]= 255; else rd[1]= fac;
		}
		rii+= BGicon[nr].xim;
	}
	
	x-= sizea/2;
	y-= sizeb/2;

	/* x*= Bxasp; */
	xs= x; ys= y;
	graphics_to_window(&xs, &ys);
	x= (xs+0.49);	
	y= (ys+0.51);
	lrectwrite(x, y, x+sizea-1, y+sizeb-1, temprect);
	
}


void sdrawline(x1,y1,x2,y2)
short x1,y1,x2,y2;
{
	short v[2];

	bgnline();
	v[0] = x1;
	v[1] = y1;
	v2s(v);
	v[0] = x2;
	v[1] = y2;
	v2s(v);
	endline();
}

short findMapColor(r, g, b)
short r, g, b;
{
	static short initdone = FALSE, col[256][3];
	short i, best, dr, dg, db;
	long dist, mdist;
	
	if (initdone == FALSE) {
		for (i = 0; i < 256; i++){
			getmcolor(i, &col[i][0], &col[i][1], &col[i][2]);
		}
		initdone = TRUE;
	}
	
	best = 0;
	mdist = 0x800000;
	
	for (i = 0; i < 256; i++){
		dr = r - col[i][0];
		dg = g - col[i][1];
		db = b - col[i][2];
		
		dist = 2*(dr * dr) + 5*(dg * dg) + (db * db);
		if (dist < mdist) {
			mdist = dist;
			best = i;
		}
	}
	/* mapcolor aanroepen om X systeem te laten werken */
	mapcolor(best, col[best][0], col[best][1], col[best][2]);
	return(best);
}


void initbutcolors()
{
	struct ButCol *bc;

	bc= &BGbutcol[0];
	bc->back= findMapColor(78, 78, 78);
	bc->pen_sel =findMapColor(255,255,255);
	bc->pen_desel =findMapColor(0,0,0);
	bc->paper_sel =findMapColor(31,31,40);
	bc->paper_desel =findMapColor(104, 138, 167);
	bc->border1 =bc->pen_desel;
	bc->border2 =findMapColor(200,200,200);
	bc->border3 =findMapColor(50,50,50);
	bc->border4 =bc->pen_sel;
}

void FreeButBlockData(struct ButBlock *bb)
{
	struct But *but;
	int a;
	
	but= bb->first;
	for(a=0; a<bb->aantal; a++, but++) {
		if(but->str) if(but->str != but->strdata) freeN(but->str);
	}
	
	if(bb->first) freeN(bb->first);
}

void FreeButs()
{
	struct ButBlock *bb,*bn;
	short a;

	bb= BGfirstbutblock;
	while(bb) {
		bn= bb->next;
		FreeButBlockData(bb);
		freeN(bb);
		bb= bn;
	}
	for(a=0; a<10; a++) {
		if(BGicon[a].rect) freeN(BGicon[a].rect);
	}
}


short GetButVal(but,value)
struct But *but;
float *value;
{
	short type;		/* geeft 0 als geen val, 1 is int, 2 is float */
	char *poin;

	type= but->type;
	poin= but->poin;

	if( (type & BUTPOIN)==CHA ) {
		*value= *poin;
		return 1;
	} else if( (type & BUTPOIN)==SHO ) {
		*value= *(short *)poin;
		return 1;
	} else if( (type & BUTPOIN)==LON ) {
		*value= *(long *)poin;
		return 1;
	} else if( (type & BUTPOIN)==FLO ) {
		*value= *(float *)poin;
		return 2;
	} else if( (type & BUTPOIN)==DOU ) {
		*value= *(double *)poin;
		return 2;
	}

	return 0;
}

void SetButVal(but, value, lvalue)
struct But *but;
float value;
long lvalue;
{
	short type;
	char *poin;

	type= but->type;
	poin= but->poin;

	if( (type & BUTPOIN)==CHA ) {
		*poin= (char)lvalue;
	}
	else if( (type & BUTPOIN)==SHO )
		*((short *)poin)= (short)lvalue;
	else if( (type & BUTPOIN)==LON )
		*((long *)poin)= lvalue;
	else if( (type & BUTPOIN)==FLO )
		*((float *)poin)= value;
	else if( (type & BUTPOIN)==DOU )
		*((double *)poin)= value;

}

void SetButFont(font)
fmfonthandle font;
{
	BGfont= font;
}

void SetButCol(nr)
short nr;
{
	BGcol= nr;
}


void DefButCol(nr,draw,back,pap_sel,pap_dsel,pen_sel,pen_dsel,b1,b2,b3,b4)
short nr,draw;
unsigned long back,pap_sel,pap_dsel,pen_sel,pen_dsel,b1,b2,b3,b4;
{
	struct ButCol *bc;

	bc= &BGbutcol[nr];
	bc->back= back;
	bc->drawmode= draw;
	if(draw==RGBDRAW) {
		bc->drawmode= NORMALDRAW;
		bc->rgb= 1;
	}
	else bc->rgb= 0;
	bc->paper_sel= pap_sel;
	bc->paper_desel= pap_dsel;
	bc->pen_sel= pen_sel;
	bc->pen_desel= pen_dsel;
	bc->border1= b1;
	bc->border2= b2;
	bc->border3= b3;
	bc->border4= b4;
}

short IsButSel(b)
struct But *b;
{
	float value;
	long lvalue;
	short a,push=0,true=1;

	a=GetButVal(b,&value);
	if(a==0) return 0;
	lvalue= (long)value;
	if( (b->type & BUTTYPE)==TOGN ) true= 0;

	if( (b->type & BIT) ) {
		if( BTST(lvalue,(b->type & 31)) ) return true;
		return !true;
	}

	switch(b->type & BUTTYPE) {
	case BUT:
		push= 0;
		break;
	case TOG:
		if(value!=0.0) push=1;
		break;
	case TOGN:
		if(value==0.0) push=1;
		break;
	case ROW:
		if(value == b->max) push=1;
		break;
	}

	return push;
}

/* ************** DRAW ******************* */

void EmbossSlider(str1,str2,f,x1,y1,x2,y2,sel,col,oud)
char *str1,*str2;
float f;
short x1,y1,x2,y2,sel,col,*oud;
{
	/* s1: getal, s2: naam, f: van 0.0 tot 1.0.
	   '*oud' zit in butstruct, is oude lengte getalstring */
	struct ButCol *bc;
	short s,h;

	bc= &BGbutcol[col];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;

	h=(y2-y1);
	colorfunc(BLACK);
	sboxs(x1-2, y1-2, x2+2, y2+2);

	sel? colorfunc(bc->paper_desel): colorfunc(bc->paper_sel);
	rectf(x1,y1,x2,y2);

	colorfunc(bc->border1);
	rect(x1-1,y2+1,x2+1,y2+1);
	colorfunc(bc->border2);
	rect(x1-1,y1,x1-1,y2);
	colorfunc(bc->border3);
	rect(x2+1,y1,x2+1,y2);
	colorfunc(bc->border4);
	rect(x1-1,y1-1,x2+1,y1-1);

	/* het blokje */
	sel? colorfunc(bc->paper_sel): colorfunc(bc->paper_desel);
	rectf(x1+f,y1+1,x1+h+f,y2-1);
	colorfunc(bc->border4);
	rect(x1+f,y2-1,x1+f+h,y2-1);
	colorfunc(bc->border1);
	rect(x1+f,y1+1,x1+f+h,y1+1);

	/* tekst rechts */
	h=getheight();
	s=Bxasp*fmgetstrwidth(BGfont,str2);
	colorfunc(bc->back);
	rectf(x2+8,y1-3,x2+8+s,y1+h-5);
	colorfunc(bc->pen_sel);
	cmov2i(x2+8,y1-2);
	fmprstr(str2);

	/* tekst links */
	s=Bxasp*fmgetstrwidth(BGfont,str1);
	colorfunc(bc->back);
	if( *oud==0) *oud=s;
	rectf(x1- *oud-8,y1-3,x1-8,y1+h-4);
	*oud= s;
	colorfunc(bc->pen_sel);
	cmov2i(x1-s-8,y1-2);
	fmprstr(str1);
}

void EmbossSlider2(str1,str2,f,x1,y1,x2,y2,sel,col,oud)
char *str1,*str2;
float f;
short x1,y1,x2,y2,sel,col,*oud;
{
	/* s1: getal, s2: naam, f: van 0.0 tot 1.0.
	   '*oud' zit in butstruct, is oude lengte getalstring */
	struct ButCol *bc;
	short s,h;

	h=(y2-y1);
	bc= &BGbutcol[col];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;

	colorfunc(BLACK);
	sboxs(x1-2, y1-2, x2+2, y2+2);

	sel? colorfunc(bc->paper_sel): colorfunc(bc->border1);
	rectf(x1,y1,x2,y2);

	colorfunc(bc->border1);
	rect(x1-1,y2+1,x2+1,y2+1);
	colorfunc(bc->border2);
	rect(x1-1,y1,x1-1,y2);
	colorfunc(bc->border1);
	rect(x2+1,y1,x2+1,y2);
	colorfunc(bc->border2);
	rect(x1-1,y1-1,x2+1,y1-1);

	/* het blokje */
	sel? colorfunc(bc->border1): colorfunc(bc->paper_sel);
	rectf(x1+f,y1+1,x1+h+f,y2-1);
	colorfunc(bc->border2);
	rect(x1+f,y2-1,x1+f+h,y2-1);
	colorfunc(BLACK);
	rect(x1+f,y1+1,x1+f+h,y1+1);

	/* tekst rechts */
	h=getheight();
	s=Bxasp*fmgetstrwidth(BGfont,str2);
	colorfunc(bc->back);
	rectf(x2+8,y1-3,x2+8+s,y1+h-5);
	colorfunc(bc->pen_sel);
	cmov2i(x2+8,y1-2);
	fmprstr(str2);

	/* tekst links */
	s=Bxasp*fmgetstrwidth(BGfont,str1);
	colorfunc(bc->back);
	if( *oud==0) *oud=s;
	rectf(x1- *oud-8,y1-3,x1-8,y1+h-4);
	*oud= s;
	colorfunc(bc->pen_sel);
	cmov2i(x1-s-8,y1-2);
	fmprstr(str1);
}


void EmbossSlider3(str1,str2,f,x1,y1,x2,y2,sel,col,oud)
char *str1,*str2;
float f;
short x1,y1,x2,y2,sel,col,*oud;
{
	/* s1: getal, s2: naam, f: van 0.0 tot 1.0.
	   '*oud' zit in butstruct, is oude lengte getalstring */
	struct ButCol *bc;
	ulong paper, dark, light;
	short s,h;
	char *cp1, *cp2;

	/* van de butcol wordt alleen back gebruikt en als kleur paper_sel */
	/* kleur pap_sel wordt iets lichter en donkerder gemaakt voor emboss */
	
	bc= &BGbutcol[col];
	paper= bc->paper_sel;
	cp1= (char *)&paper;
	cp2= (char *)&light;
	if(cp1[1]+50>255) cp2[1]= 255; else cp2[1]= cp1[1]+50;
	if(cp1[2]+50>255) cp2[2]= 255; else cp2[2]= cp1[2]+50;
	if(cp1[3]+50>255) cp2[3]= 255; else cp2[3]= cp1[3]+50;
	cp2= (char *)&dark;
	if(cp1[1]-50<0) cp2[1]= 0; else cp2[1]= cp1[1]-50;
	if(cp1[2]-50<0) cp2[2]= 0; else cp2[2]= cp1[2]-50;
	if(cp1[3]-50<0) cp2[3]= 0; else cp2[3]= cp1[3]-50;
	
	cpack(0x0);
	sboxs(x1-2, y1-2, x2+2, y2+2);			/* kader */

	h=(y2-y1);

	sel? cpack(paper): cpack(dark);
	rectf(x1,y1,x2,y2);

	cpack(dark);
	rect(x1-1,y2+1,x2+1,y2+1);
	rect(x1-1,y1,x1-1,y2);
	cpack(light);
	rect(x2+1,y1,x2+1,y2);
	rect(x1-1,y1-1,x2+1,y1-1);

	/* het blokje */
	sel? cpack(dark): cpack(light);
	rectf(x1+f,y1+1,x1+h+f,y2-1);
	cpack(0xFFFFFF);
	rect(x1+f,y2-1,x1+f+h,y2-1);
	cpack(0x0);
	rect(x1+f,y1+1,x1+f+h,y1+1);

	/* tekst rechts */
	h=getheight();
	s=Bxasp*fmgetstrwidth(BGfont,str2);
	cpack(bc->back);
	rectf(x2+8,y1-3,x2+8+s,y1+h-5);
	cpack(0xFFFFFF);
	cmov2i(x2+8,y1-2);
	fmprstr(str2);

	/* tekst links */
	s=Bxasp*fmgetstrwidth(BGfont,str1);
	cpack(bc->back);
	if( *oud==0) *oud=s;
	rectf(x1- *oud-8,y1-3,x1-8,y1+h-4);
	*oud= s;
	cpack(0xFFFFFF);
	cmov2i(x1-s-8,y1-2);
	fmprstr(str1);
}


void EmbossBut(str,x1,y1,x2,y2,sel,col)
char *str;
short x1,y1,x2,y2,sel,col;
{
	struct ButCol *bc;
	short s;

	bc= &BGbutcol[col];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;

	if(sel) {
		colorfunc(bc->paper_sel);
		sboxfs(x1+2,y1+2,x2-2,y2-2);
		colorfunc(bc->border4);
		sboxs(x1+1,y1+1,x2-1,y1+1);
		sboxs(x1,y1,x2,y1);
		sboxs(x2,y1,x2,y2);
		colorfunc(bc->border1);
		sboxs(x1,y2,x2,y2);
		sboxs(x1+1,y2-1,x2-1,y2-1);
		sboxs(x1,y1,x1,y2);
		colorfunc(bc->border3);
		sboxs(x1+1,y1+1,x1+1,y2-1);
		colorfunc(bc->border2);
		sboxs(x2-1,y1+1,x2-1,y2-1);
		colorfunc(bc->pen_sel);
	} else {
		colorfunc(bc->paper_desel);
		sboxfs(x1+2,y1+2,x2-2,y2-2);
		colorfunc(bc->border1);
		sboxs(x1+1,y1+1,x2-1,y1+1);
		sboxs(x1,y1,x2,y1);
		sboxs(x2,y1,x2,y2);
		colorfunc(bc->border4);
		sboxs(x1,y2,x2,y2);
		sboxs(x1+1,y2-1,x2-1,y2-1);
		sboxs(x1,y1,x1,y2);
		colorfunc(bc->border2);
		sboxs(x1+1,y1+1,x1+1,y2-1);
		colorfunc(bc->border3);
		sboxs(x2-1,y1+1,x2-1,y2-1);
		colorfunc(bc->pen_desel);
	}
	if(str[0]==0) return;
	s=Bxasp*fmgetstrwidth(BGfont,str);
	x1=(x1+x2-s)/2;
	y1=(y1+y2-getheight()/2-1)/2;
	cmov2i(x1,y1);
	fmprstr(str);
}

void EmbossBut2(str,x1,y1,x2,y2,sel,col)	/* met ditherlijntjes en zwart kader */
char *str;
short x1,y1,x2,y2,sel,col;
{
	struct ButCol *bc;
	short s;

	bc= &BGbutcol[col];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;

	if(bc->drawmode==NORMALDRAW) colorfunc(BLACK);
	else colorfunc(2);
	sboxs(x1, y1, x2, y2);

	if(sel) colorfunc(bc->border1); 
	else colorfunc(bc->paper_sel);
	sboxfs(x1+2,y1+2,x2-2,y2-2);

	if(sel) colorfunc(0); 
	else colorfunc(bc->border2);	/* linksboven */
	sdrawline(x1+1,y1+1,x1+1,y2-1);
	sdrawline(x1+1,y2-1,x2-1,y2-1);
	setpattern(TBOXPAT);
	
	sdrawline(x1+2,y1+2,x1+2,y2-2);
	sdrawline(x1+2,y2-2,x2-2,y2-2);
	setpattern(0);

	if(sel) colorfunc(bc->paper_sel); 
	else colorfunc(bc->border1);	/* rechtsonder */
	sdrawline(x2-1,y1+1,x2-1,y2-1);
	sdrawline(x1+1,y1+1,x2-1,y1+1);
	setpattern(TBOXPAT);
	
	sdrawline(x2-2,y1+2,x2-2,y2-2);
	sdrawline(x1+2,y1+2,x2-2,y1+2);
	setpattern(0);
	if(sel) colorfunc(bc->pen_sel); 
	else colorfunc(bc->pen_desel);

	if(str[0]==0) return;
	s=Bxasp*fmgetstrwidth(BGfont,str);
	x1=(x1+x2-s+1)/2;
	y1=(y1+y2-getheight()/2-1)/2;
	cmov2i(x1,y1);
	fmprstr(str);
}

void EmbossBut3(str,x1,y1,x2,y2,sel,col)	/* alleen RGBmode! */
char *str;
short x1,y1,x2,y2,sel,col;
{
	struct ButCol *bc;
	ulong paper, light=0, dark=0;
	int iconr, icox, icoy;
	short s, x, y;
	char *cp1, *cp2;
	
	/* van de butcol wordt alleen back gebruikt en als kleur paper_sel */
	/* kleur pap_sel wordt iets lichter en donkerder gemaakt voor emboss */
	
	paper= BGbutcol[col].paper_sel;
	cp1= (char *)&paper;
	cp2= (char *)&light;
	if(cp1[1]+50>255) cp2[1]= 255; else cp2[1]= cp1[1]+50;
	if(cp1[2]+50>255) cp2[2]= 255; else cp2[2]= cp1[2]+50;
	if(cp1[3]+50>255) cp2[3]= 255; else cp2[3]= cp1[3]+50;
	cp2= (char *)&dark;
	if(cp1[1]-50<0) cp2[1]= 0; else cp2[1]= cp1[1]-50;
	if(cp1[2]-50<0) cp2[2]= 0; else cp2[2]= cp1[2]-50;
	if(cp1[3]-50<0) cp2[3]= 0; else cp2[3]= cp1[3]-50;
	
	cpack(0x0);
	sboxs(x1, y1, x2, y2);			/* kader */

	cpack(paper);
	sboxfs(x1+2,y1+2,x2-2,y2-2);	/* centrum */
	
	if( strncmp(str, "ICON", 4)==0 ) {
		sscanf(str+4, "%d %d %d\n", &iconr, &icox, &icoy);
		DrawIcon(iconr, paper, icox, icoy, (x1+x2)/2, (y1+y2)/2);
	}
	else if(str[0]!=0) {
		if(sel) cpack(0xFFFFFF);
		else cpack(0x0);
		
		while( (s= Bxasp*fmgetstrwidth(BGfont,str)) > x2-x1) {
			
			if(s < 10) break;
			if(str[1]==0) break;
			
			str++;
		}

		x= (x1+x2-s+1)/2;
		y= (y1+y2-getheight()/2-1)/2;
		cmov2i(x, y);
		fmprstr(str);
	}

	if(sel) cpack(dark); 
	else cpack(light);	
	sboxs(x1+1, y2-1, x2-1, y2-2);	/* boven */
	sboxs(x1+1, y1+1, x1+2, y2-2);	/* links */
	
	if(sel) cpack(light); 
	else cpack(dark);	
	sboxs(x1+1, y1+1, x2-1, y1+2);	/* onder */
	sboxs(x2-1, y1+1, x2-2, y2-2);	/* rechts */
	
}


void emboss(paper,x1,y1,x2,y2,sel)	/* voor kleurblokjes en zo */
short paper,x1,y1,x2,y2,sel;
{
	struct ButCol *bc;

	bc= &BGbutcol[BGcol];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;
	
	colorfunc(paper);
	sboxfs(x1+2,y1+2,x2-2,y2-2);

	sel? colorfunc(bc->border1): colorfunc(bc->border4);
	sboxs(x1,y2,x2,y2);
	sboxs(x1+1,y2-1,x2-1,y2-1);
	sel? colorfunc(bc->border4): colorfunc(bc->border1);
	sboxs(x1+1,y1+1,x2-1,y1+1);
	sboxs(x1,y1,x2,y1);
	sel? colorfunc(bc->border3): colorfunc(bc->border2);
	sboxs(x1,y1,x1,y2);
	sboxs(x1+1,y1+1,x1+1,y2-1);
	sel? colorfunc(bc->border2): colorfunc(bc->border3);
	sboxs(x2-1,y2-1,x2-1,y1+1);
	sboxs(x2,y1,x2,y2);
}

void emboss2(paper,x1,y1,x2,y2,sel)	/* voor kleurblokjes en zo */
short paper,x1,y1,x2,y2,sel;
{
	struct ButCol *bc;

	bc= &BGbutcol[BGcol];
	drawmode(bc->drawmode);
	if(bc->rgb) colorfunc= cpack; else colorfunc= color;

	colorfunc(BLACK);
	sboxs(x1, y1, x2, y2);

	colorfunc(paper);
	sboxfs(x1+2,y1+2,x2-2,y2-2);

	if(sel) colorfunc(0); 
	else colorfunc(bc->border2);	/* linksboven */
	sdrawline(x1+1,y1+1,x1+1,y2-1);
	sdrawline(x1+1,y2-1,x2-1,y2-1);
	setpattern(TBOXPAT);
	sdrawline(x1+2,y1+2,x1+2,y2-2);
	sdrawline(x1+2,y2-2,x2-2,y2-2);
	setpattern(0);

	if(sel) colorfunc(bc->paper_sel); 
	else colorfunc(bc->border1);	/* rechtsonder */
	sdrawline(x2-1,y1+1,x2-1,y2-1);
	sdrawline(x1+1,y1+1,x2-1,y1+1);
	setpattern(TBOXPAT);
	sdrawline(x2-2,y1+2,x2-2,y2-2);
	sdrawline(x1+2,y1+2,x2-2,y1+2);
	setpattern(0);
}



short tekentekstbut(b,s,pos)
struct But *b;
char *s;
short *pos;
{
	short temp,h,t,texcol,ofs;
	char s1[150],ch;

	if(BGbutcol[b->col].rgb) colorfunc= cpack; else colorfunc= color;

	h=( b->y1 + b->y2 - getheight()/2 )/2;
	s1[0]=0;

	if(*pos== -1) {		/* niet aktieve tekstbut */
		ButDrawFunc(s1,b->x1,b->y1,b->x2,b->y2,0,b->col);
		strcpy(s1,b->str);
		strcat(s1,s);
		ofs=0;
		while(Bxasp*fmgetstrwidth(BGfont,s1+ofs)>(b->x2 - b->x1 -10)) ofs++;
		cmov2i(b->x1+4,h);
		colorfunc(BGbutcol[b->col].pen_desel);
		fmprstr(s1+ofs);
	}
	else {			/*  aktieve tekstbut */
		ButDrawFunc(s1,b->x1,b->y1,b->x2,b->y2,1,b->col);
		strcpy(s1,b->str);
		strcat(s1,s);
		if(*pos < strlen(b->str)) *pos = strlen(b->str);
		if(*pos > strlen(s1)) *pos = strlen(s1);
		ch=s1[*pos];
		s1[*pos]= 0;
		ofs=0;
		while(Bxasp*fmgetstrwidth(BGfont,s1+ofs)>(b->x2 - b->x1 -10)) ofs++;

		/* de cursor */
		t=Bxasp*fmgetstrwidth(BGfont,s1+ofs)+3;
		if(colorfunc == color) color(RED);
		else cpack(0xFF);
		sboxfs(b->x1+t,b->y1+2,b->x1+t+3,b->y2-2);

		texcol= BGbutcol[b->col].pen_sel;
		if(ofs) {
			colorfunc(texcol);
			cmov2i(b->x1+4,h);
			fmprstr(s1+ofs);
		}
		if(ofs==0) {
			s1[*pos]=ch;
			temp = 0;
			while(Bxasp*fmgetstrwidth(BGfont,s1+temp)>(b->x2 - b->x1 -10)) temp++;
			colorfunc(texcol);
			cmov2i(b->x1+4,h);
			s1[strlen(s1)-temp]=0;
			fmprstr(s1);
		}

	}
	return (ofs);
}

/* ****** MENUBUT ****** */

#ifndef TBOXPAT
#define TBOXPAT		1234
#endif
#ifndef TBOXH
#define TBOXH		20
#endif
#ifndef TBOXBLACK
#define TBOXBLACK	2
#endif
#ifndef TBOXWHITE
#define TBOXWHITE	3
#endif
#ifndef TBOXGREY
#define TBOXGREY	1
#endif

float tbwinmat[4][4], tbprojmat[4][4];
short tbpat[16], oldmap[4][3], tboldwin;
int tbfontyofs;

void bgnbutpupdraw()	/* doet ook patterns, linestyle, colors */
{
	static long firsttime= 1;
	long x;
	
	linewidth(1);
	setlinestyle(0);

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

	fullscrn();
	drawmode(PUPDRAW);
	sginap(2);
	finish();
	
	if(firsttime) {
		
		getmcolor(1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor(2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor(3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		
		for(x=0; x<16; x+=2) tbpat[x]=0x5555;
		for(x=1; x<16; x+=2) tbpat[x]=0xAAAA;
				
		firsttime= 0;
	}
	
	mapcolor(1, 170, 170, 170); 
	mapcolor(2, 0, 0, 0); 
	mapcolor(3, 240, 240, 240);

	defpattern(TBOXPAT, 16, tbpat);

	tbfontyofs= (TBOXH-getheight()/2-1)/2;
}

void endbutpupdraw()
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
}

void menu_embossbox(x1, y1, x2, y2, type)	/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
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
		
		setpattern(TBOXPAT);
		
		color(TBOXWHITE);
		sboxs(x1+2, y1+2, x2-2, y2-2);	/* kader */
		color(TBOXBLACK);
		sboxs(x1+2, y1+1, x2-1, y1+1);	/* onder */
		sboxs(x2-1, y1+2, x2-1, y2-1);	/* rechts */
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
			sboxs(x1+2, y1+2, x2-2, y1+2);	/* onder */
			sboxs(x2-2, y1+1, x2-2, y2-2);	/* rechts */
			setpattern(0);
			
			color(TBOXBLACK);
			sboxs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			
			color(TBOXWHITE);	/* tekst */
		}
		else {
			sboxs(x1+2, y1+2, x2-2, y2-2);	/* kader */

			color(TBOXBLACK);
			sboxs(x1+2, y2-1, x2-1, y2-1);	/* boven */
			sboxs(x1+1, y1+2, x1+1, y2-1);	/* links */
			setpattern(0);
		}
	}
}

void butmenu_draw(startx, starty, endx, items, title, spoin, sel)
int startx, starty, endx, items;
char *title, **spoin;
short sel;
{
	int a=0;
	
	starty+= (items-1)*TBOXH;
	if(title) {
		menu_embossbox(startx, starty+5, endx, starty+TBOXH+5, 0);
		color(TBOXBLACK);
		cmov2i(startx+5, starty+tbfontyofs+5);
		fmprstr(title);
		starty-= TBOXH;
		a= 1;
	}
	
	for(; a<items; a++) {
		menu_embossbox(startx, starty, endx, starty+TBOXH, 2);
		color(TBOXBLACK);
		cmov2i(startx+5, starty+tbfontyofs);
		fmprstr(spoin[a]);

		starty-= TBOXH;
	}
	
	
}



void domenubuto(struct But *but)
{
	float fvalue;
	int len, height, a, retval[100], items, xmax, ymax, startx, starty, endx, endy;
	int act, acto= -1, value=0, event;
	short val, winakt=0, mval[2];
	short redrawq[10], redrawcount= 0, mouseymove;
	static short mdev[2]={ MOUSEX,MOUSEY};
	char *instr, *str, *astr, *title=0, *spoin[100];
	

	instr= but->str;
	
	bgnbutpupdraw();

	/* kopie string maken */
	str= mallocN(strlen(instr)+1, "pupmenu");
	memcpy(str, instr, strlen(instr)+1);
	
	/* eerst string uitelkaar pulken, tellen hoeveel elementen, return values */
	astr= str;
	spoin[0]= astr;
	items= 0;
	retval[0]= -1;
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

				if(title) retval[items]= items-1;
				else retval[items]= items;
				
				*astr= 0;
				break;
		}
		astr++;
	}
	
	items++;	/* het begon bij 0 te tellen */

	/* afmetingen en plaats */
	len= 0;
	for(a=0; a<items; a++) {
		xmax= fmgetstrwidth(but->font, spoin[a]);
		if(xmax>len) len= xmax;
	}
	len+= 10;
	
	height= TBOXH*items;
	
	xmax = getgdesc(GD_XPMAX);
	ymax = getgdesc(GD_YPMAX);

	getdev(2,mdev,mval);
	
	startx= mval[0]-len/2;
	GetButVal(but, &fvalue);
	value= (int)fvalue;
	
	for(a=0; a<items; a++) {
		if( retval[a]==value ) break;
	}
	if(a==items) a= 0;
	
	starty= mval[1]-height+TBOXH/2+ a*TBOXH;
	
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
		setvaluator(MOUSEY, mval[1]+mouseymove, 0, ymax);
	}

	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	sboxfs(startx+5, starty-10, endx+5, endy-10);
	setpattern(0);

	butmenu_draw(startx, starty, endx, items, title, spoin, 1);

	value= 0;
	while(value==0) {
		event= 0;
		if(qtest()) event= qread(&val);
		
		if(event) {
			switch(event) {
			case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
				if(val==0) {
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
		
		getdev(2,mdev,mval);

		if(mval[0]<startx-20 || mval[0]>endx+20 || mval[1]<starty-20 || mval[1]>endy+20) value= 1;
		
		act= items-1- (mval[1]-starty)/TBOXH;
		if(act!=acto && act<items && act>=0) {
		
			a= endy-20;
			
			/* teken acto */
			if(title && acto==0) ;
			else if(acto>=0) {		/* vergelijken, acto is init op -1 */
				menu_embossbox(startx, a-acto*TBOXH, endx, a-acto*TBOXH+TBOXH, 2);
				color(TBOXBLACK);
				cmov2i(startx+5, a-acto*TBOXH+tbfontyofs);
				fmprstr(spoin[acto]);
			}
			/* teken act */
			if(title && act==0) ;
			else {
				menu_embossbox(startx, a-act*TBOXH, endx, a-act*TBOXH+TBOXH, 3);
				color(TBOXBLACK);
				cmov2i(startx+5, a-act*TBOXH+tbfontyofs);
				fmprstr(spoin[act]);
			}
			acto= act;
		}
	}

	color(0);
	sboxfs(startx-1, starty-10, endx+5, endy+10);
	endbutpupdraw();
	freeN(str);

	if(winakt) qenter(INPUTCHANGE, winakt);

	for(val=0; val<redrawcount; val++) qenter(REDRAW, redrawq[val]);
	
	if(value==2 && act>=0 && act<items) {
		if(act==0 && title);
		else {
			value= retval[act];
			fvalue= (float)value;
			SetButVal(but, fvalue, value);
		}
	}
}
int domenubut(struct But *but)
{
	float fvalue;
	int len, height, a, retval[100], items, xmax, ymax, startx, starty, endx, endy;
	int act, acto= -1, value=0, event;
	short val, winakt=0, mval[2];
	short redrawq[10], redrawcount= 0, mouseymove;
	static Device mdev[2]={ MOUSEX,MOUSEY};
	char *instr, *str, *astr, *title=0, *spoin[100];
	

	instr= but->str;
	
	bgnbutpupdraw();

	/* kopie string maken */
	str= mallocN(strlen(instr)+1, "pupmenu");
	memcpy(str, instr, strlen(instr)+1);
	
	/* eerst string uitelkaar pulken, tellen hoeveel elementen, return values */
	astr= str;
	spoin[0]= astr;
	items= 0;
	retval[0]= 0;
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
				if(astr[1]) {
					items++;
					spoin[items]= astr+1;

					if(title) retval[items]= items-1;
					else retval[items]= items;
				}
				*astr= 0;
				break;
		}
		astr++;
	}
	
	items++;	/* het begon bij 0 te tellen */

	/* afmetingen en plaats */
	len= 0;
	for(a=0; a<items; a++) {
		xmax= fmgetstrwidth(but->font, spoin[a]);
		if(xmax>len) len= xmax;
	}
	len+= 10;
	
	height= TBOXH*items;
	
	xmax = getgdesc(GD_XPMAX);
	ymax = getgdesc(GD_YPMAX);

	getdev(2,mdev,mval);
	
	startx= mval[0]-len/2;
	GetButVal(but, &fvalue, &value);

	if(title) a= 1;
	else a= 0;
	for(; a<items; a++) {
		if( retval[a]==value ) break;
	}
	if(a==items) a= 0;
	starty= mval[1]-height+TBOXH/2+ a*TBOXH;
	
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
		setvaluator(MOUSEY, mval[1]+mouseymove, 0, ymax);
	}
	/* wel altijd weer terug op button */
	mouseymove= mval[1];

	/* schaduw */
	color(TBOXBLACK);
	setpattern(TBOXPAT);
	sboxfs(startx+5, starty-10, endx+5, endy-10);
	setpattern(0);

	butmenu_draw(startx, starty, endx, items, title, spoin, 1);

	value= 0;
	while(value==0) {
		event= 0;
		if(qtest()) event= qread(&val);
		
		if(event) {
			switch(event) {
			case LEFTMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
				if(val==0) {
					value= 2;
				}
				break;
			/* case MIDDLEMOUSE: */
			/* 	if(val) value= 3; */
			/* 	break; */
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
		
		getdev(2,mdev,mval);

		if(mval[0]<startx-20 || mval[0]>endx+20 || mval[1]<starty-30 || mval[1]>endy+40) value= 1;
		
		act= items-1- (mval[1]-starty)/TBOXH;
		if(act!=acto) {
		
			a= endy-20;
			
			/* teken acto */
			if(title && acto==0) ;
			else if(acto>=0 && acto<items) {	/* vergelijken, acto is init op -1 */
				menu_embossbox(startx, a-acto*TBOXH, endx, a-acto*TBOXH+TBOXH, 2);
				color(TBOXBLACK);
				cmov2i(startx+5, a-acto*TBOXH+tbfontyofs);
				fmprstr(spoin[acto]);
			}
			/* teken act */
			if(title && act==0) ;
			else if(act>=0 && act<items) {
				menu_embossbox(startx, a-act*TBOXH, endx, a-act*TBOXH+TBOXH, 3);
				color(TBOXBLACK);
				cmov2i(startx+5, a-act*TBOXH+tbfontyofs);
				fmprstr(spoin[act]);
			}
			acto= act;
		}
	}

	color(0);
	sboxfs(startx-1, starty-10, endx+5, endy+10);
	endbutpupdraw();
	freeN(str);

	if(winakt) qenter(INPUTCHANGE, winakt);

	for(val=0; val<redrawcount; val++) qenter(REDRAW, redrawq[val]);
	
	if(mouseymove) {
		setvaluator(MOUSEY, mouseymove, 0, ymax);
	}
	
	if(value==2 && act>=0 && act<items) {
		if(act==0 && title);
		else {
			value= retval[act];
		
			fvalue= (float)value;
			SetButVal(but, fvalue, value);
		}
	}
	else return 0;
	
	return 1;
}

void getname_menu_but(butname, str, item)
char *butname, *str;
int item;
{
	int itemcount=0;
	char *bstr;
	
	bstr= butname;
	
	while( *str) {
		
		switch( *str ) {
		case '%':
			if( str[1]=='t' ) {
				itemcount--;
			}
			else if(str[1]=='x') {
				if( atoi(str+2)==item ) {
					bstr[0]= 0;
					return;
				}
			}
			break;
		case '|':
			if(itemcount==item) {
				bstr[0]= 0;
				return;
			}
			itemcount++;
			bstr= butname;
			break;
		default:
			*bstr= *str;
			bstr++;
		}
		
		str++;
	}
	
	bstr[0]= 0;		/* als *str==0, terminaten! */
	
}



/* *********** END MENUBUT ********************** */

void SetButShape(nr)
short nr;
{
	BGdrawtype= nr;

	if(nr==0) {
		ButDrawFunc= EmbossBut;
		SliderDrawFunc= EmbossSlider;
	}
	else if(nr==1) {
		ButDrawFunc= EmbossBut2;
		SliderDrawFunc= EmbossSlider2;
	}
	else if(nr==2) {	/* alleen RGB */
		ButDrawFunc= EmbossBut3;
		SliderDrawFunc= EmbossSlider3;
	}
}


void DrawBut(but,sel)
struct But *but;
short sel;
{
	float f,value;
	long x1, y1, x2, y2, s, lvalue;
	short a,h,w,b3,b4;
	char r,g,b,s1[20], butname[40];
	long olddraw;
	int iconr, icox, icoy;

	if(but==0) return;

	olddraw = getdrawmode();
	fmsetfont(but->font);
	BGfont= but->font;
	if(but->drawtype!=BGdrawtype) SetButShape(but->drawtype);

	if(BGbutcol[but->col].rgb) colorfunc= cpack; else colorfunc= color;

	switch (but->type & BUTTYPE) {

	case BUT: 
	case ROW: 
	case TOG: 
	case TOGN:
	case BUTRET:
		ButDrawFunc(but->str,but->x1,but->y1,but->x2,but->y2,sel,but->col);
		break;
	case ICONROW:
		/* tijdelijk veranderen van icoon, daarna weer terug! */
		strcpy(s1, but->str);
		if( strncmp(but->str, "ICON", 4)==0 ) {
			
			sscanf(but->str+4, "%d %d %d\n", &iconr, &icox, &icoy);
			GetButVal(but,&value);
			lvalue= (long)value;
			icox+= lvalue- (int)(but->min);
			
			sprintf(but->str, "ICON %d %d %d", iconr, icox, icoy);
			
		}
		ButDrawFunc(but->str,but->x1,but->y1,but->x2,but->y2,sel,but->col);

		strcpy(but->str, s1);

		break;
	case MENU:
		if(but->x2-but->x1 > 40) {
			GetButVal(but, &value, &lvalue);
			getname_menu_but(butname, but->str, lvalue);
		}
		else {
			butname[0]= 0;
			but->rt[0]= 0;
		}

		ButDrawFunc(butname,but->x1,but->y1,but->x2,but->y2,sel,but->col);
		a= strlen(butname);
		butname[a]= ' ';
		butname[a+1]= 0;
		h= but->y2- but->y1;
		x1= but->x2-0.66*h; x2= x1+.33*h;
		y1= but->y1+.42*h; y2= y1+.16*h;
		
		colorfunc(BGbutcol[but->col].pen_desel);
		sboxfi(x1, y1, x2, y2);
		colorfunc(BGbutcol[but->col].pen_sel);
		sboxfi(x1-1, y1+1, x2-1, y2+1);
		
		break;
	case NUM:
		a=GetButVal(but,&value);
		s1[0]=0;
		if(a==1) {
			lvalue= (long)value;
			sprintf(s1,"%s%d",but->str,lvalue);
		} else if(a==2) {
			sprintf(s1,"%s%.2f",but->str,value);
		}
		ButDrawFunc(s1,but->x1,but->y1,but->x2,but->y2,sel,but->col);
		break;
	case TOG3:
		colorfunc(BGbutcol[but->col].pen_desel);
		b3=(but->x1+but->x2)/2; 
		b4=4+but->y2;
		sboxfs(b3-3,b4-2,b3+3,b4+2);
		if(sel) {
			if( BTST( *(but->poin+2),(but->type & 31) ) ) {
				colorfunc(BGbutcol[but->col].pen_sel);
				sboxfs(b3-1,b4-1,b3+1,b4+1);
			}
		}
		ButDrawFunc(but->str,but->x1,but->y1,but->x2,but->y2,sel,but->col);
		break;
	case LABEL:
		/* eerst clear */
		colorfunc(BGbutcol[but->col].back);
		sboxfs(but->x1-1,but->y1-1,but->x2+1,but->y2+1);
		
		colorfunc(BGbutcol[but->col].pen_sel);
		s= Bxasp*fmgetstrwidth(BGfont,but->str);
		x1= (but->x1+but->x2-s)/2;
		y1= (but->y1+but->y2-getheight()/2-1)/2;
		cmov2i(x1,y1);
		fmprstr(but->str);

		break;
	case SLI:
		a=GetButVal(but,&value);
		f= (value-but->min)*(but->x2-but->x1-but->y2+but->y1)/(but->max - but->min);
		s1[0]=0;
		if(a==1) {
			lvalue= (long)value;
			sprintf(s1,"%d",lvalue);
		} else if(a==2) {
			sprintf(s1,"%.2f",value);
		}
		SliderDrawFunc(s1,but->str,f,but->x1,but->y1,but->x2,but->y2,sel,but->col,but->rt);

		/* kleurschuif */
		if(but->a1) {	/* cmapnummer */
			if(but->a1>3) drawmode(NORMALDRAW);
			f= but->max-but->min;
			but->poin-=but->a2;
			GetButVal(but,&value); 
			r= 255*value/f;
			but->poin++;
			GetButVal(but,&value); 
			g= 255*value/f;
			but->poin++;
			GetButVal(but,&value); 
			b= 255*value/f;
			but->poin+=(but->a2-2);
			mapcolor(but->a1,r,g,b);
		}
		break;
	case TEX:
		a= -1;
		tekentekstbut(but,but->poin,&a);
	}
	drawmode(olddraw);
}


void SetButs(min,max)
short min,max;
{
	struct ButBlock *bb;
	struct But *but;
	long oldwin, actwin;
	short push,nr;

	bb= BGfirstbutblock;
	but=0;
	oldwin = actwin = winget();
	while(bb) {
		if(bb->window!=actwin) {
			actwin = bb->window;
			winset(actwin);
		}

		but= bb->first;
		for(nr=0;nr<bb->aantal;nr++) {
			if(but->nr>=min && but->nr<=max) {
				push= IsButSel(but);
				DrawBut(but,push);
			}
			but++;
		}
		bb=bb->next;
	}

	if(oldwin != actwin) winset(oldwin);
}


void FreeButBlock(str)
char *str;
{
	struct ButBlock *lastb=0,*nextb,*b1,*bb;

	bb= BGfirstbutblock;
	while(bb) {
		if(strcmp(str,bb->naam)==0) break;
		bb=bb->next;
	}

	if(bb==0) return;

	nextb= bb->next;

	b1= BGfirstbutblock;

	if(bb==b1) BGfirstbutblock=nextb;
	else {
		while(b1) {
			if(b1->next==bb) {
				b1->next=nextb;
				break;
			}
			b1=b1->next;
		}
	}
	FreeButBlockData(bb);
	freeN(bb);
}


void DefButBlock(str,win,font,aantal,col,drawtype)
char *str;
long win;
fmfonthandle font;
short aantal,col,drawtype;
{
	struct ButBlock *bb;
	static short firsttime=1;

	if(firsttime) {
		initrawcodes();
		initbutcolors();
		firsttime= 0;
		BGfirstbutblock=0;
		ButtonsGetmouse(0);
		deflinestyle(65535, 0x5555);
	}

	winset(win);
	fmsetfont(font);
	SetButShape( drawtype );

	FreeButBlock(str);

	bb= callocN(sizeof(struct ButBlock),"DefButBlock");
	if(bb==0) {
		printf("Calloc error in DefButBlock\n");
		BGfirst= 0;
		return;
	}

	bb->next= BGfirstbutblock;
	BGfirstbutblock=bb;
	bb->first= callocN( aantal*sizeof(struct But), "DefButBlock2");
	bb->aantal=aantal;
	bb->window=win;
	strncpy(bb->naam,str,19);
	bb->naam[19]=0;

	/* globals */
	BGfirst= bb->first;
	BGaantal= bb->aantal;
	BGteller=0;
	BGwin= win;
	BGfont= font;
	BGcol= col;
	BGdrawtype= drawtype;
	
	/* voor zekerheid: als winmat veranderd is, tekenen ikonen goed */
	ButtonsGetmouse(0);
}

void SetButFunc(func)
void (*func)();
{
	struct But *b;
	
	b= BGfirst+BGteller;
	b->func= func;
	BGbutfunc = func;
}

struct But *DefBut(type,nr,str,x1,y1,x2,y2,poin,min,max,a1,a2)
short type,nr;
char *str;
short x1,y1,x2,y2;
char *poin;
float min,max;
short a1,a2;
{
	struct But *b;
	long maxl;
	short push,a;
	char s[20];

	if( type & BUTPOIN ) {		/* er is pointer nodig */
		if(poin< (char *)30000) {
			/* als pointer nul is wordt button gewist en niet gedefinieerd */

			if(BGbutcol[BGcol].rgb) colorfunc= cpack; else colorfunc= color;

			colorfunc(BGbutcol[BGcol].back);
			sboxfs(x1, y1, x1+x2, y1+y2);
			return 0;
		}
	}

	b= BGfirst+BGteller;
	BGteller++;
	if(BGteller>BGaantal) {
		printf("Button beyond max: %s\n",str);
		return 0;
	}

	/* ga er van uit dat de font en window goed staan */

	b->type= type;
	b->nr=nr;
	if( strlen(str)>=MAXBUTSTR-1 ) {
		b->str= callocN( strlen(str)+2, "DefBut");
		strcpy(b->str, str);
	}
	else {
		b->str= b->strdata;
		strcpy(b->str, str);
	}
	b->x1= x1; 
	b->y1= y1; 
	b->x2= (x1+x2); 
	b->y2= (y1+y2);
	b->poin= poin;
	b->min= min; 
	b->max= max;
	b->a1= a1; 
	b->a2= a2;
	b->font= BGfont;
	b->col= BGcol;
	b->drawtype= BGdrawtype;

	if((type & BUTTYPE)==NUM) {	/* spatie toevoegen achter naam */
		a= strlen(b->str);
		if(a>0 && a<MAXBUTSTR-2) {
			if(b->str[a-1]!=' ') {
				b->str[a]= ' ';
				b->str[a+1]= 0;
			}
		}
	}
	if( (type & BUTTYPE)==SLI ) {
		/* maxlen berekenen van getalstring */
		if( (type & BUTPOIN)==FLO || (type & BUTPOIN)==DOU ) {
			sprintf(s,"%.2f",max);
		} else {
			maxl= max;
			sprintf(s,"%d",maxl);
		}
		b->rt[0]=Bxasp*fmgetstrwidth(BGfont,s);
	}
	push= IsButSel(b);
	DrawBut(b,push);
	return b;
}


void SetupFile(bb)
struct ButBlock *bb;
{
	struct But *b;
	short totbut,nr;
	FILE *fp;

	fp=fopen("butsetup","w");
	if(fp==NULL);
	else {
		b= bb->first;
		totbut= bb->aantal;
		for(nr=0;nr<totbut;nr++) {
			fprintf(fp,"%d,%d,%d,%d   %s\n",b->x1,b->y1,b->x2-b->x1,b->y2-b->y1,b->str);
			b++;
		}
		fclose(fp);
	}
}


void EditBut(but)
struct But *but;
{
	short mval[2],mx,my,dx,dy,w,h,s,push;
	long x,y,maxx,maxy;
	Device mdev[2];
	char string[20];

	if(BGbutcol[but->col].rgb) colorfunc= cpack; else colorfunc= color;

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;
	getdev(2, mdev, mval);
	getorigin(&x, &y);
	getsize(&maxx, &maxy);
	mval[0]-=x; 
	mval[1]-=y;
	mx=mval[0]; 
	my=mval[1];
	while(getbutton(LEFTMOUSE)) {
		getdev(2, mdev, mval);
		mval[0]-=x; 
		mval[1]-=y;
		dx=mval[0]-mx; 
		dy=mval[1]-my;
		if(getbutton(RIGHTSHIFTKEY)) {
			dx/=4; 
			dy/=4;
		}
		if(dx!=0 || dy!=0) {
			if(mval[0]<maxx && mval[1]<maxy) {
				mx=mval[0]; 
				my=mval[1];
				
				colorfunc(BGbutcol[but->col].back);
				
				if( (but->type & BUTTYPE)==SLI) {

					h=getheight();
					s=Bxasp*fmgetstrwidth(BGfont,but->str);
					colorfunc(PAPER);
					rectf(but->x2+6,but->y1-4,but->x2+9+s,but->y1+h);

					s= but->rt[0];
					rectf(but->x1-10-s,but->y1-4,but->x1-7,but->y1+h);
				}
				sboxfs(but->x1-2,but->y1-2,but->x2+2,but->y2+2);
				if(getbutton(RIGHTALTKEY)==0) {
					but->x1+=dx;
					but->y1+=dy;
				}
				but->x2+=dx;
				but->y2+=dy;
				push= IsButSel(but);
				DrawBut(but,push);
			}
		}
	}
}


void dotextbut(b)
struct But *b;
{
	Device dev,mdev[2];
	short temp, x, mval[2], c, len=0,pos=0,ofs,qual;
	char s[150];

			
	ButtonsGetmouse(mval);

	/* doen: eerst pos berekenen ahv muisco */
	DrawBut(b,1);	/* ivm met globals zoals tekenmode */
	strcpy(s,b->poin);
	pos= 150;
	ofs = tekentekstbut(b,s,&pos);

	while((Bxasp*fmgetstrwidth(BGfont,s+ofs) + b->x1 + 4) > mval[0]) {
		if (pos <= ofs) break;
		pos--;
		s[pos] = 0;
	}

	strcpy(s,b->poin);
	tekentekstbut(b,s,&pos);

	while (getbutton(LEFTMOUSE)) sginap(1);
	len=strlen(s);
	b->min= 0.0;
	while(dev = qread(&c)) {
		if(dev==INPUTCHANGE) break;
		else if(getbutton(LEFTMOUSE)) break;
		else if(getbutton(RIGHTMOUSE)) break;
		else if(dev==ESCKEY && c!=0) break;

		if(c) {
			c= rawtoascii(dev);
			switch(c) {
			case 0:
				break;
			case '\b':	case 'b'+100: /* backspace */
				if(len!=0) {
					temp=pos-strlen(b->str);
					if(temp>0) {
						for(x=temp;x<=strlen(s);x++)
							s[x-1]=s[x];
						pos--;
						s[--len]='\0';
					}
				} 
				break;
			case 255: case 255+100:
				pos++;
				break;
			case 254: case 254+100:
				if(pos>0) pos--;
				break;
			case 253: case 253+100:
				pos= 150;
				break;
			case 252: case 252+100:
				pos=0;
				break;
			case 250: case 251:
				break;
			case '\n':
			case '\r':
				b->min= 1.0;
				goto klaartekstbut;
			default:
				if(len<b->max) {
					temp=pos-strlen(b->str);
					for(x=b->max;x>temp;x--)
						s[x]=s[x-1];
					s[temp]=c;
					pos++; 
					len++;
					s[len]='\0';
				}
			}
			tekentekstbut(b,s,&pos);
		}
		/* qreset() */
	}
klaartekstbut:
	if(dev!=ESCKEY) strcpy(b->poin,s);
	pos= -1;
	tekentekstbut(b,b->poin,&pos);
}


short DoButtons()
{
	struct But *b,*bt;
	struct ButBlock *bb;
	float f,fstart,deler,value,tempf, min, max;
	long x,y,lvalue,winakt;
	Device dev,mdev[2];
	short a,nr=0,mval[2],push=0,w,temp,totbut,sx,sy,h,c;
	short len=0,pos=0,ofs,qual, retval=1;
	char s[150], *point;

	/* aktieve window */
	winakt=winget();
	if(winakt==0) return;

	bb= BGfirstbutblock;

DoButaktievewindow:
	b=0;
	while(bb) {
		if(bb->window==winakt) {
			b= bb->first;
			totbut= bb->aantal;
			break;
		}
		bb=bb->next;
	}

	if(b==0) return 0;  /*  geen actieve window */

	ButtonsGetmouse(0);

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;
	ButtonsGetmouse(mval);

	for(nr=0;nr<totbut;nr++) {
		if(mval[0]>b->x1) {
			if(mval[0]<b->x2) {
				if( (b->type & BUTTYPE)==SLI ) temp=4;
				else temp= 1;
				if(mval[1]>=b->y1-temp) {
					if(mval[1]<=b->y2+temp) {

						if( b->type & BUTPOIN ) {		/* er is pointer nodig */
							if(b->poin< (char *)30000 ) {
								printf("DoButton pointer error: %s\n",b->str);
								return 0;
							}
						}

						if(getbutton(RIGHTCTRLKEY)) {
							EditBut(b);
							SetupFile(bb);
							return 0;
						}
						switch(b->type & BUTTYPE) {
						case BUT:
							DrawBut(b,1);
							push= 1;
							if(b->func) {
								if(b->poin) {
									GetButVal(b,&value);
									lvalue= value;
									b->func(lvalue);
								}
								else b->func();
								winset(winakt);
							}
							while (getbutton(LEFTMOUSE)==TRUE) {
								ButtonsGetmouse(mval);
								a=0;
								if(mval[0]>b->x1)
									if(mval[0]<b->x2)
										if(mval[1]>=b->y1-1)
											if(mval[1]<=b->y2+1) a=1;

								if(a!=push) {
									push=a;
									DrawBut(b,push);
								
								}
								sginap(1);
							}
							DrawBut(b,0);
							if(push==0) return 0;
							break;
						case TOG: 
						case TOGN:
							GetButVal(b,&value);
							if(b->type & BIT) {
								lvalue= value;
								w= BTST(lvalue,b->type & 31);
								if(w) lvalue = BCLR(lvalue,b->type & 31);
								else lvalue = BSET(lvalue,b->type & 31);
								SetButVal(b,value,lvalue);
								if(w) push=0; 
								else push=1;
								if((b->type & BUTTYPE)==TOGN) push= !push;
								DrawBut(b,push);
							}
							else {
								if(value==0.0) push=1; 
								else push=0;
								if((b->type & BUTTYPE)==TOGN) push= !push;
								SetButVal(b,(float)push,push);
								DrawBut(b,push);
							}
							while (getbutton(LEFTMOUSE)==TRUE) sginap(1);
							break;
						case ROW:
							bt= bb->first;
							for(temp=0;temp<totbut;temp++) {
								if( nr!=temp && (bt->type & BUTTYPE)==ROW ) {
									if(bt->min==b->min) {
										DrawBut(bt,0);
									}

								}
								bt++;
							}
							SetButVal(b,b->max,(long)b->max);
							DrawBut(b,1);
							while (getbutton(LEFTMOUSE)==TRUE) sginap(1);
							break;
						case SLI:
							a=GetButVal(b,&value);
							ButtonsGetmouse(mval);
							sx= mval[0];
							h= b->y2-b->y1;
							fstart= b->max-b->min;
							fstart= (value - b->min)/fstart;
							temp= 32767;
							DrawBut(b,1);
							deler= (b->x2-b->x1-h);
							while (getbutton(LEFTMOUSE)==TRUE) {
								ButtonsGetmouse(mval);
								f= (float)(mval[0]-sx)/deler +fstart;
								if(f>1.0) f=1.0;
								if(f<0.0) f=0.0;
								tempf= b->min+f*(b->max-b->min);
								if(a==1) {

									temp= ffloor(tempf+.5);
									a=GetButVal(b,&value);
									if(temp != (long)value ) {
										if( temp>=b->min && temp<=b->max) {
											pos=1;
											SetButVal(b,tempf,temp);
										}
										DrawBut(b,1);
										if(b->func) b->func();
									}

								} else {

									temp= ffloor(tempf+.5); /* ivm +1 of -1 */
									a=GetButVal(b,&value);
									if(tempf != value ) {
										if( tempf>=b->min && tempf<=b->max) {
											pos=1;
											SetButVal(b,tempf,temp);
										}
										DrawBut(b,1);
									}

								}
								sginap(1);
							}
							if(temp!=32767 && pos==0) {  /* plus 1 of min 1 */
								f= (float)(mval[0]-b->x1)/(b->x2-b->x1-h);
								sx= b->min+f*(b->max-b->min);
								if(a==1) {

									if(sx<temp) temp--;
									else temp++;
									if( temp>=b->min && temp<=b->max)
										SetButVal(b,tempf,temp);
								
								} else {

									if(sx<tempf) tempf-=.01;
									else tempf+=.01;
									if( tempf>=b->min && tempf<=b->max)
										SetButVal(b,tempf,temp);

								}
							}
							DrawBut(b,0);
							break;
						case NUM:		/* trekbut */
							DrawBut(b,1);
							ButtonsGetmouse(mval);
							a= GetButVal(b,&value);
							sx= mval[0];
							fstart=(float)(b->max-b->min);
							fstart= (value - b->min)/fstart;
							f=fstart;
							temp= 32767;
							while (getbutton(LEFTMOUSE)==TRUE) {
								qualafhandeling(&qual);
								if(getbutton(MIDDLEMOUSE)) {	/* maak er textbut van */
									GetButVal(b,&value);
									if( (b->type & BUTPOIN)==FLO ) {
										sprintf(s, "%.4f", value);
									}
									else {
										lvalue= value;
										sprintf(s, "%d", lvalue);
									}
									point= b->poin;
									b->poin= s;
									min= b->min;
									max= b->max;
									b->min= 0.0;
									b->max= 15.0;
									temp= b->type;
									b->type= TEX;
									
									dotextbut(b);
									
									b->type= temp;
									b->poin= point;
									if( (b->type & BUTPOIN)==FLO ) {
										value= atof(s);
										if(value<min) value= min;
										if(value>max) value= max;
										lvalue= value;
									}
									else {
										lvalue= atoi(s);;
										if(lvalue<min) lvalue= min;
										if(lvalue>max) lvalue= max;
										value= lvalue;
									}
									SetButVal(b, value, lvalue);
									b->min= min;
									b->max= max;
									DrawBut(b,0);
									
									return b->nr;
								}
								deler= 500;
								if( (b->type & BUTPOIN)!=FLO ) {

									if( (b->max-b->min)<100 ) deler= 200.0;
									if( (b->max-b->min)<25 ) deler= 50.0;

								}
								if(qual & 3) deler*= 10.0;

								ButtonsGetmouse(mval);
								f+= ((float)(mval[0]-sx))/deler;
								if(f>1) f=1;
								if(f<0) f=0;
								sx= mval[0];
								tempf= ( b->min + f*(b->max-b->min));
								if(a==1) {

									temp= ffloor(tempf+.5);
									if(qual & 32) temp=10*(temp/10);
									if( temp>=b->min && temp<=b->max) {
										a= GetButVal(b,&value);
										if(temp != (long)value ) {
											pos=1;
											SetButVal(b,tempf,temp);
											DrawBut(b,1);
											if(b->func) b->func();
											winset(winakt);
										}
									}

								}
								else {

									/* if(qual & 32) temp=10*(temp/10); */
									if( tempf>=b->min && tempf<=b->max) {
										a= GetButVal(b,&value);
										if(tempf != value ) {
											pos=1;
											SetButVal(b,tempf,temp);
											DrawBut(b,1);
										}
									}

								}
								sginap(1);
							}
							if(temp!=32767 && pos==0) {  /* plus 1 of min 1 */
								if(a==1) {

									if(sx<(b->x1+b->x2)/2) temp--;
									else temp++;
									if( temp>=b->min && temp<=b->max)
										SetButVal(b,tempf,temp);

								}
								else {

									if(sx<(b->x1+b->x2)/2) tempf-=.01;
									else tempf+=.01;
									if( tempf>=b->min && tempf<=b->max)
										SetButVal(b,tempf,temp);

								}
							}
							DrawBut(b,0);
							break;
						case BUTRET:	/* keert terug, daarna SetBut aanroepen */
							DrawBut(b,1);
							break;
						case TOG3:	/* driestand met bitjes */

							if( BTST(*(b->poin+2),b->type & 31)) {
								*(b->poin+2)= BCLR(*(b->poin+2),b->type & 31);
								*(b->poin)=BCLR(*(b->poin),b->type & 31);
								DrawBut(b,0);
							}
							else if( BTST(*(b->poin),b->type & 31)) {
								*(b->poin+2)=BSET(*(b->poin+2),b->type & 31);
								DrawBut(b,1);
							} else {
								*(b->poin)=BSET(*(b->poin),b->type & 31);
								DrawBut(b,1);
							}
							break;
						case TEX:	/* tekstbut */
							dotextbut(b);
							break;
						case MENU:
							DrawBut(b, 1);
							retval= domenubut(b);
							DrawBut(b, 0);
							break;
						case ICONROW:
							DrawBut(b, 1);
							ButtonsGetmouse(mval);
							sx= mval[0];
							GetButVal(b,&value);
							a= 0;
							while(getbutton(LEFTMOUSE)) {
								ButtonsGetmouse(mval);
								w= (mval[0]+10-sx)/20;
								if(w!=a) {
									temp= (long)value +w;
									if(temp< (int)b->min) temp= b->min;
									if(temp> (int)b->max) temp= b->max;
								
									SetButVal(b,tempf,temp);
									DrawBut(b, 1);
								}
								a= w;
								sginap(2);
							}
							DrawBut(b, 0);
							break;
						}
						if(retval) return b->nr; else return 0;
					}
				}
			}
		}
		b++;
	}

	bb= bb->next;
	goto DoButaktievewindow;
}

/* ******************START VAN BUTGROUP FUNCTIES****************** */



void BGflush()
{
	struct Link * link;

	while (butbase->first){
		link = butbase->first;
		remlink(butbase,link);
		free(link);
	}
}


void BGadd(type,nr,str,x1,y1,x2,y2,poin,min,max,a1,a2)
short type,nr;
char *str;
short x1,y1,x2,y2;
char *poin;
float min,max;
short a1,a2;
{
	struct Bgrp * bgrp;

	bgrp = mallocstruct(struct Bgrp,1);
	bgrp->type = type;
	bgrp->nr = nr;
	bgrp->str = str;
	bgrp->x1 = x1;
	bgrp->y1 = y1;
	bgrp->x2 = x2;
	bgrp->y2 = y2;
	bgrp->poin = poin;
	bgrp->min = min;
	bgrp->max = max;
	bgrp->a1 = a1;
	bgrp->a2 = a2;

	bgrp->font= BGfont;
	bgrp->col= BGcol;
	bgrp->drawtype= BGdrawtype;
	bgrp->func = BGbutfunc;
	BGbutfunc = 0;
	
	addtail(butbase,bgrp);
}


void BGaddq(type,nr,str,x2,y2,poin,min,max,a1,a2)
short type,nr;
char *str;
short x2,y2;
char *poin;
float min,max;
short a1,a2;
{
	short x1 = 0, y1 = 0;
	BGadd(type,nr,str,x1,y1,x2,y2,poin,min,max,a1,a2);
}


void BGnewline()
{
	struct Bgrp * bgrp;

	/* definitie van newline: poin = x2 = y2 = 0 */

	bgrp = callocstruct(struct Bgrp,1);
	addtail(butbase,bgrp);
}

void BGposition(x,y,w,h)
short x,y;
ushort w,h;
{
	BG_x = x;
	BG_y = y;
	BG_w = w;
	BG_h = h;
}

void BGspacing(x,y)
ushort x,y;
{
	BG_xsp = x;
	BG_ysp = y;
}

void BGdirection(dir)
char dir;
{
	switch(dir){
	case 'U':
	case 'u':
		BG_dir = 'u';
		break;
	case 'D':
	case 'd':
		BG_dir = 'd';
		break;
	default:
		printf("ButGroup: direction '%c' ignored\n",dir);
	}
}

void BGdraw()
{
	struct Bgrp * bgrp , *fstbut;
	float xfac,yfac,xpos,ypos;
	short x,y = 0,buts, lines = 0, maxy, butsfound = 0;

	bgrp = (struct Bgrp *) butbase->first;
	if (bgrp == 0) return;

	while (bgrp){
		fstbut = bgrp;
		x = buts = maxy = 0;

		while (bgrp->poin != 0 && bgrp->x2 != 0 && bgrp->y2 != 0){
			buts ++;
			x += bgrp->x2;
			if (bgrp->y2 > maxy) maxy = bgrp->y2;
			bgrp = bgrp->next;
			if (bgrp == 0) break;
		}

		lines ++;

		if (buts){
			butsfound = 1;
			y += maxy;
			xfac = (BG_w - (BG_xsp * (buts - 1.0)))/(float) x;
			xpos = BG_x;

			while (fstbut->poin != 0 && fstbut->x2 != 0 && fstbut->y2 != 0){
				fstbut->x1 = xpos + 0.5;
				xpos += fstbut->x2 * xfac;
				fstbut->x2 = xpos + 0.5 - fstbut->x1;
				xpos += BG_xsp;
				fstbut = fstbut->next;
				if (fstbut == 0) break;
			}
		}
		if (bgrp) bgrp = bgrp->next; /* over newline heenspringen */
	}

	if (butsfound){
		yfac = (BG_h - (BG_ysp * (lines - 1.0)))/(float) y;
		ypos = BG_y;
		if (BG_dir == 'd'){
			yfac = -yfac;
			ypos += BG_h;
		}

		bgrp = (struct Bgrp *) butbase->first;
		if (bgrp == 0) return;

		while (bgrp){
			fstbut = bgrp;
			buts = maxy = 0;

			while (bgrp->poin != 0 && bgrp->x2 != 0 && bgrp->y2 != 0){
				buts = 1;
				if (bgrp->y2 > maxy) maxy = bgrp->y2;
				bgrp = bgrp->next;
				if (bgrp == 0) break;
			}

			if (buts){
				while (fstbut->poin != 0 && fstbut->x2 != 0 && fstbut->y2 != 0){
					fstbut->y1 = ypos + 0.5;
					fstbut->y2 = (ypos + 0.5 + yfac * fstbut->y2) - fstbut->y1;

					if (BG_dir == 'd'){
						fstbut->y2 = - fstbut->y2;
						fstbut->y1 -= fstbut->y2;
					}
					fstbut = fstbut->next;
					if (fstbut == 0) break;
				}
				ypos += yfac * maxy;
			}
			if (BG_dir == 'd') ypos -= BG_ysp;
			else ypos += BG_ysp;

			if (bgrp) bgrp = bgrp->next; /* over newline heenspringen */
		}
	}

	bgrp = (struct Bgrp *) butbase->first;
	while (bgrp){
		if (bgrp->poin != 0 && bgrp->x2 != 0 && bgrp->y2 != 0) {

			SetButFont(bgrp->font);
			SetButCol(bgrp->col);
			SetButShape(bgrp->drawtype);
			SetButFunc(bgrp->func);
			
			DefBut(bgrp->type,bgrp->nr,bgrp->str,bgrp->x1,bgrp->y1,bgrp->x2,bgrp->y2,bgrp->poin,bgrp->min,bgrp->max,bgrp->a1,bgrp->a2);
		}
		bgrp = bgrp->next;
	}

	BGflush();
}

