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



/*  editfont.c      GRAPHICS
 * 
 *  maart 95
 *  
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "edit.h"

#define MAXTEXT	1000

int textediting=0;

char findaccent(char char1, char code)
{
	char new= 0;
	
	if(char1=='a') {
		if(code=='`') new= 224;
		else if(code==39) new= 225;
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

char *textbuf=0;
char *oldstr;


void do_textedit(ushort event, short val)
{
	Curve *cu;
	static int accentcode= 0;
	int x, doit=0, cursmove=0;

	cu= G.obedit->data;
	
	val= rawtoascii(event);

	if ((val > 31 && val < 200 && val != 127) || (val==13) || (val==10) || (val==9) ) {
		if(accentcode && cu->pos>1) {
			textbuf[cu->pos-1]= findaccent(textbuf[cu->pos-1], val);
		}
		else if(cu->len<MAXTEXT-1) {
			if(G.qual & LR_ALTKEY ) {
				if(val=='t') val= 137;
				else if(val=='c') val= 169;
				else if(val=='r') val= 174;
				else if(val=='f') val= 164;
				else if(val=='p') val= 163;
				else if(val=='y') val= 165;
				else if(val=='.') val= 138;
				else if(val=='g') val= 176;
				else if(val=='s') val= 223;
				else if(val=='1') val= 185;
				else if(val=='2') val= 178;
				else if(val=='3') val= 179;
				else if(val=='%') val= 139;
				else if(val=='?') val= 191;
				else if(val=='!') val= 161;
				else if(val=='x') val= 215;
				else if(val=='>') val= 187;
				else if(val=='<') val= 171;
				else if(val=='v') val= 1001;
			}
			if(val==1001) {
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
					if(cu->len+filelen<MAXTEXT) {
						strcat( textbuf, strp);
						cu->len= strlen(textbuf);
						cu->pos= cu->len;
					}
					freeN(strp);
				}
			}
			else {
				for(x= cu->len; x>cu->pos; x--) textbuf[x]= textbuf[x-1];
				textbuf[cu->pos]= val;
				
				cu->pos++;
				cu->len++;
				textbuf[cu->len]='\0';
			}
		}
		accentcode= 0;
		doit= 1;
		
	}
	else {
		cursmove= 0;
		
		switch(val) {
		case 0:
			break;
		case '\b':	/* backspace */
			if(cu->len!=0) {
				if(getbutton(LEFTALTKEY) || getbutton(RIGHTALTKEY) ) {
					if(cu->pos>0) accentcode= 1;
				}
				else if(G.qual & LR_SHIFTKEY) {
					if(okee("Clear text")) {
						cu->pos= 0;
						textbuf[0]= 0;
						cu->len= 0;
					}
				}
				else if(cu->pos>0) {
					for(x=cu->pos;x<=cu->len;x++) textbuf[x-1]= textbuf[x];
					cu->pos--;
					textbuf[--cu->len]='\0';
				}
				doit= 1;
			}
			break;
		case 255:	/* right */
			cu->pos++;
			cursmove= FO_CURS;
			break;
		case 254:	/* left */
			cu->pos--;
			cursmove=FO_CURS;
			break;
		case 253:	/* shift right */
			while(cu->pos<cu->len) {
				if( textbuf[cu->pos]==0) break;
				if( textbuf[cu->pos]=='\n') break;
				if( textbuf[cu->pos]=='\r') break;
				cu->pos++;
			}
			cursmove=FO_CURS;
			break;
		case 252:	/* shift left */
			while(cu->pos>0) {
				if( textbuf[cu->pos-1]=='\n') break;
				if( textbuf[cu->pos-1]=='\r') break;
				cu->pos--;
			}
			cursmove=FO_CURS;
			break;
		case 250:	/* up */
			if(G.qual & LR_ALTKEY) {
				if (cu->pos && textbuf[cu->pos - 1] < 255) {
					textbuf[cu->pos - 1]++;
					doit= 1;
				}
			}
			else cursmove=FO_CURSUP;
			break;
		case 251:	/* down */
			if(G.qual & LR_ALTKEY) {
				if (cu->pos && textbuf[cu->pos - 1] > 1) {
					textbuf[cu->pos - 1]--;
					doit= 1;
				}
			}
			else cursmove= FO_CURSDOWN;
			break;
		case 248:	/* shift up */
			cu->pos= 0;
			cursmove= FO_CURS;
			break;
		case 249:	/* shift down */
			cu->pos= cu->len;
			cursmove= FO_CURS;
			break;
		}
		if(cursmove) {
			if(cu->pos>cu->len) cu->pos= cu->len;
			else if(cu->pos>=MAXTEXT) cu->pos= MAXTEXT;
			else if(cu->pos<0) cu->pos= 0;
		}
	}
	if(doit || cursmove) {
		text_to_curve(G.obedit, cursmove);
		if(cursmove==0) makeDispList(G.obedit);
		allqueue(REDRAWVIEW3D, 0);
	}
}

void make_editText()
{
	Curve *cu;
	char *str;

	cu= G.obedit->data;
	if(textbuf==0) textbuf= mallocN(MAXTEXT, "texteditbuf");
	strncpy(textbuf, cu->str, MAXTEXT);
	oldstr= cu->str;
	cu->str= textbuf;

	cu->len= strlen(textbuf);
	if(cu->pos>cu->len) cu->pos= cu->len;
	
	text_to_curve(G.obedit, 0);
	makeDispList(G.obedit);
	
	textediting= 1;
}

void load_editText()
{
	Curve *cu;
	
	cu= G.obedit->data;

	freeN(oldstr);
	oldstr= 0;
	
	cu->str= mallocN(cu->len+1, "tekstedit");
	strcpy(cu->str, textbuf);
	
	/* this memory system is weak... */
	freeN(textbuf);
	textbuf= 0;
	
	cu->len= strlen(cu->str);
	textediting= 0;
}

void remake_editText()
{
	Curve *cu;
		
	if(okee("Reload Original text")==0) return;
	
	strncpy(textbuf, oldstr, MAXTEXT);
	cu= G.obedit->data;
	cu->len= strlen(textbuf);
	if(cu->pos>cu->len) cu->pos= cu->len;
	
	text_to_curve(G.obedit, 0);
	makeDispList(G.obedit);
	
	allqueue(REDRAWVIEW3D, 0);
}

void free_editText()
{
	if(oldstr) freeN(oldstr);
	textbuf= oldstr= 0;
	textediting= 0;
}

void add_font_prim()
{
	Curve *cu;

	if(G.obedit) return;
	
	add_object(OB_FONT);
	G.obedit= BASACT->object;
	where_is_object(G.obedit);
	
	cu= G.obedit->data;
	
	if(G.main->vfont.first==0) load_firstfont();
	
	cu->vfont= G.main->vfont.first;
	cu->vfont->id.us++;
	cu->str= mallocN(12, "str");
	strcpy(cu->str, "Text");
	cu->pos= 4;
	
	make_editText();
	allqueue(REDRAWVIEW3D, 0);
}

void to_upper()
{
	Curve *cu;
	int len, ok;
	char *str;
	
	if(G.obedit==0) {
		return;
	}
	
	ok= 0;
	cu= G.obedit->data;
	
	len= strlen(cu->str);
	str= cu->str;
	while(len) {
		if( *str>=97 && *str<=122) {
			ok= 1;
			*str-= 32;
		}
		len--;
		str++;
	}
	
	if(ok==0) {
		len= strlen(cu->str);
		str= cu->str;
		while(len) {
			if( *str>=65 && *str<=90) {
				*str+= 32;
			}
			len--;
			str++;
		}
	}
	text_to_curve(G.obedit, 0);
	makeDispList(G.obedit);

	allqueue(REDRAWVIEW3D, 0);

}



