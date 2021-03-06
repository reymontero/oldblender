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



/*  keyval.c   july 2000
 *  
 *  
 * 
 * 
 *  ton roosendaal
 * Version: $Id: keyval.c,v 1.3 2000/07/21 09:05:26 nzc Exp $
 */


#include "blender.h"
#include "graphics.h"
#include "game.h"



char *key_event_to_string(ushort event)
{

	switch(event) {
	case AKEY:
		return "A";
		break;
	case BKEY:
		return "B";
		break;
	case CKEY:
		return "C";
		break;
	case DKEY:
		return "D";
		break;
	case EKEY:
		return "E";
		break;
	case FKEY:
		return "F";
		break;
	case GKEY:
		return "G";
		break;
	case HKEY:
		return "H";
		break;
	case IKEY:
		return "I";
		break;
	case JKEY:
		return "J";
		break;
	case KKEY:
		return "K";
		break;
	case LKEY:
		return "L";
		break;
	case MKEY:
		return "M";
		break;
	case NKEY:
		return "N";
		break;
	case OKEY:
		return "O";
		break;
	case PKEY:
		return "P";
		break;
	case QKEY:
		return "Q";
		break;
	case RKEY:
		return "R";
		break;
	case SKEY:
		return "S";
		break;
	case TKEY:
		return "T";
		break;
	case UKEY:
		return "U";
		break;
	case VKEY:
		return "V";
		break;
	case WKEY:
		return "W";
		break;
	case XKEY:
		return "X";
		break;
	case YKEY:
		return "Y";
		break;
	case ZKEY:
		return "Z";
		break;

	case ZEROKEY:
		return "Zero";
		break;
	case ONEKEY:
		return "One";
		break;
	case TWOKEY:
		return "Two";
		break;
	case THREEKEY:
		return "Three";
		break;
	case FOURKEY:
		return "Four";
		break;
	case FIVEKEY:
		return "Five";
		break;
	case SIXKEY:
		return "Six";
		break;
	case SEVENKEY:
		return "Seven";
		break;
	case EIGHTKEY:
		return "Eight";
		break;
	case NINEKEY:
		return "Nine";
		break;

	case LEFTCTRLKEY:
		return "Leftctrl";
		break;
	case LEFTALTKEY:
		return "Leftalt";
		break;
	case	RIGHTALTKEY:
		return "Rightalt";
		break;
	case	RIGHTCTRLKEY:
		return "Rightctrl";
		break;
	case RIGHTSHIFTKEY:
		return "Rightshift";
		break;
	case LEFTSHIFTKEY:
		return "Leftshift";
		break;

	case ESCKEY:
		return "Esc";
		break;
	case TABKEY:
		return "Tab";
		break;
	case RETKEY:
		return "Ret";
		break;
	case SPACEKEY:
		return "Space";
		break;
	case LINEFEEDKEY:
		return "Linefeed";
		break;
	case BACKSPACEKEY:
		return "Backspace";
		break;
	case DELKEY:
		return "Del";
		break;
	case SEMICOLONKEY:
		return "Semicolon";
		break;
	case PERIODKEY:
		return "Period";
		break;
	case COMMAKEY:
		return "Comma";
		break;
	case QUOTEKEY:
		return "Quote";
		break;
	case ACCENTGRAVEKEY:
		return "Accentgrave";
		break;
	case MINUSKEY:
		return "Minus";
		break;
	case SLASHKEY:
		return "Slash";
		break;
	case BACKSLASHKEY:
		return "Backslash";
		break;
	case EQUALKEY:
		return "Equal";
		break;
	case LEFTBRACKETKEY:
		return "Leftbracket";
		break;
	case RIGHTBRACKETKEY:
		return "Rightbracket";
		break;

	case LEFTARROWKEY:
		return "Leftarrow";
		break;
	case DOWNARROWKEY:
		return "Downarrow";
		break;
	case RIGHTARROWKEY:
		return "Rightarrow";
		break;
	case UPARROWKEY:
		return "Uparrow";
		break;

	case PAD2:
		return "Pad2";
		break;
	case PAD4:
		return "Pad4";
		break;
	case PAD6:
		return "Pad6";
		break;
	case PAD8:
		return "Pad8";
		break;
	case PAD1:
		return "Pad1";
		break;
	case PAD3:
		return "Pad3";
		break;
	case PAD5:
		return "Pad5";
		break;
	case PAD7:
		return "Pad7";
		break;
	case PAD9:
		return "Pad9";
		break;

	case PADPERIOD:
		return "Padperiod";
		break;
	case PADVIRGULEKEY:
		return "Padvirgule";
		break;
	case PADASTERKEY:
		return "Padaster";
		break;

	case PAD0:
		return "Pad0";
		break;
	case PADMINUS:
		return "Padminus";
		break;
	case PADENTER:
		return "Padenter";
		break;
	case PADPLUSKEY:
		return "Padplus";
		break;

	case	F1KEY:
		return "F1";
		break;
	case	F2KEY:
		return "F2";
		break;
	case	F3KEY:
		return "F3";
		break;
	case	F4KEY:
		return "F4";
		break;
	case	F5KEY:
		return "F5";
		break;
	case	F6KEY:
		return "F6";
		break;
	case	F7KEY:
		return "F7";
		break;
	case	F8KEY:
		return "F8";
		break;
	case	F9KEY:
		return "F9";
		break;
	case	F10KEY:
		return "F10";
		break;
	case	F11KEY:
		return "F11";
		break;
	case	F12KEY:
		return "F12";
		break;

	case	PAUSEKEY:
		return "Pause";
		break;
	case	INSERTKEY:
		return "Insert";
		break;
	case	HOMEKEY:
		return "Home";
		break;
	case	PAGEUPKEY:
		return "Pageup";
		break;
	case	PAGEDOWNKEY:
		return "Pagedown";
		break;
	case	ENDKEY:
		return "End";
		break;
	}
	
	return "";
}

