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



/*  editimasel.c      GRAPHICS
 * 
 *  maart 97
 *  
 *  
 * 
 * Version: $Id: editimasel.c,v 1.6 2000/08/06 21:57:06 ton Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "interface.h"
#include "edit.h"
#include "imasel.h"

#define XIC 20
#define YIC 21

/* GLOBALS */
extern char *fsmenu;


void draw_icon_imasel()
{
	addqueue(curarea->win, REDRAW, 1);
}

void winqreadimasel(ushort event, short val)
{
	SpaceImaSel *simasel;
	ImaDir * direntry;
	struct tms voidbuf;
	
	short mval[2];
	short but_event;
	short area_event;
	short queredraw = 0;
	char  name[256];
	int newtime;
	static int prevtime=0;
	
	
	if(val==0) return;
	simasel= curarea->spacedata.first;
	
	area_event = 0;
	getmouseco_areawin(mval);
	simasel->mx= mval[0];
	simasel->my= mval[1];
	
	if (simasel->desx > 0){
		if ( (mval[0] > simasel->dssx) && (mval[0] < simasel->dsex) && (mval[1] > simasel->dssy) && (mval[1] < simasel->dsey) ) area_event = IMS_INDIRSLI;
		if ( (mval[0] > simasel->desx) && (mval[0] < simasel->deex) && (mval[1] > simasel->desy) && (mval[1] < simasel->deey) ) area_event = IMS_INDIR;
	}
	if (simasel->fesx > 0){
		if ( (mval[0] > simasel->fssx) && (mval[0] < simasel->fsex) && (mval[1] > simasel->fssy) && (mval[1] < simasel->fsey) ) area_event = IMS_INFILESLI;
		if ( (mval[0] > simasel->fesx) && (mval[0] < simasel->feex) && (mval[1] > simasel->fesy) && (mval[1] < simasel->feey) ) area_event = IMS_INFILE;
	}	
	
	if( event!=RETKEY && event!=PADENTER)
		if( uiDoBlocks(&curarea->uiblocks, event)!=UI_NOTHING ) event= 0;

	switch(event) {
	case AFTERPIBREAD:	
		get_pib_file(simasel);
		queredraw = 1;
		break;
			
	case AFTERIMASELIMA:
		if (bitset(simasel->fase, IMS_DOTHE_INF)){
			get_file_info(simasel);
			
			if (!bitset(simasel->fase, IMS_KNOW_INF)){
				addafterqueue(curarea->win, AFTERIMASELIMA, 1);
				
			}else{
				simasel->subfase = 0;
				simasel->imafase = 0;
				simasel->fase |= IMS_DOTHE_IMA;
				addafterqueue(curarea->win, AFTERIMASELGET, 1);
			}
		}
		break;
	case AFTERIMASELGET:
		if (bitset(simasel->fase, IMS_DOTHE_IMA)){
			get_next_image(simasel);
			if (simasel->ima_redraw > 0){
				newtime = times(&voidbuf);
				if (newtime - prevtime > 30) {
					simasel->ima_redraw = 0;
					queredraw = 1;
					prevtime = newtime;
				}
				
			}
			if (!bitset(simasel->fase, IMS_KNOW_IMA)){
				addafterqueue(curarea->win, AFTERIMASELGET, 1);
			}else{
				simasel->ima_redraw = 0;
				simasel->subfase    = 0;
				simasel->imafase    = 0;
				addqueue(curarea->win, AFTERIMAWRITE, 1);
				queredraw = 1;
			}
		}
		break;
	case  AFTERIMAWRITE:
		if (bitset(simasel->fase, IMS_KNOW_IMA)){
			write_new_pib(simasel);
			queredraw = 1;
		}
		break;	
	
	case RIGHTMOUSE:
		if ((area_event == IMS_INFILE) && (simasel->hilite_ima)){
			select_ima_files(simasel);
			queredraw = 1;
		}
		break;
	case UI_BUT_EVENT:
		
		/* bug: blender's interface kit also returns a '4'... what is it! */
		
		switch(val) {
		case 13:	/*  'P' */
			imadir_parent(simasel);
			queredraw = 1;
			
		case 1: /* dir entry */
			checkdir(simasel->dir);
			clear_ima_dir(simasel);
			queredraw = 1;
			break;
		
		case 3: /* fsmenu */
			uiGetNameMenu(simasel->dir, fsmenu, simasel->fileselmenuitem);
			checkdir(simasel->dir);
			clear_ima_dir(simasel);
		    queredraw = 1;
			break;
		}
		break;
		
	case LEFTMOUSE:
	case MIDDLEMOUSE:
	
		/* No button pressed */
		switch (area_event){
		case IMS_INDIRSLI:
			move_imadir_sli(simasel);
			queredraw = 1;
			break;
		case IMS_INFILESLI:
			move_imafile_sli(simasel);
			queredraw = 1;
			break;
		case IMS_INDIR:
			if (simasel->hilite > -1){
				change_imadir(simasel);
				queredraw = 1;
			}
			break;
		case IMS_INFILE:
			if (simasel->hilite_ima){
				strcpy(simasel->fole, simasel->hilite_ima->file_name);
				strcpy(simasel->file, simasel->hilite_ima->file_name);
				
				if (event == LEFTMOUSE) addqueue(curarea->win, IMALEFTMOUSE, 1);	
				
				if ((event == MIDDLEMOUSE)&&(simasel->returnfunc)){
					strcpy(name, simasel->dir);
					strcat(name, simasel->file);
					
					if(simasel->mode & IMS_STRINGCODE) makestringcode(name);
					
					simasel->returnfunc(name);
					filesel_prevspace();
				}
				queredraw = 1;
			}
			break;
		}
		break;
	
	case MOUSEX:
	case MOUSEY:
		getmouseco_areawin(mval);	/* lokaal screen */
		calc_hilite(simasel);
		if (simasel->mouse_move_redraw ){
			simasel->mouse_move_redraw = 0;
			queredraw = 1;
		}
		break;
		
	case PAGEUPKEY:
	case PAGEDOWNKEY:
		switch(area_event){
		case IMS_INDIRSLI:
		case IMS_INDIR:
			if (simasel->dirsli){
				if (event == PAGEUPKEY)   simasel->topdir -= (simasel->dirsli_lines - 1);
				if (event == PAGEDOWNKEY) simasel->topdir += (simasel->dirsli_lines - 1); 	
				queredraw = 1;
			}
			break;
		case IMS_INFILESLI:
		case IMS_INFILE:
			if(simasel->imasli){
				if (event == PAGEUPKEY)   simasel->image_slider -= simasel->slider_height;
				if (event == PAGEDOWNKEY) simasel->image_slider += simasel->slider_height;
				
				if(simasel->image_slider < 0.0)  simasel->image_slider = 0.0;
				if(simasel->image_slider > 1.0)  simasel->image_slider = 1.0;
				queredraw = 1;
			}	
			break;
		}
		break;
	
	case HOMEKEY:
		simasel->image_slider = 0.0;
		queredraw = 1;
		break;

	case ENDKEY:
		simasel->image_slider = 1.0;
		queredraw = 1;
		break;
			
	case AKEY:
		if (G.qual == 0){
			ima_select_all(simasel);
			queredraw = 1;
		}
		break;

	case PKEY:
		if(G.qual & LR_SHIFTKEY) {
			extern char bprogname[];	/* usiblender.c */
			
			sprintf(name, "%s -a %s%s", bprogname, simasel->dir, simasel->file);
			system(name);
		}
		if(G.qual & LR_CTRLKEY) {
			if(bitset(simasel->fase, IMS_KNOW_IMA)) pibplay(simasel);
		}
		if (G.qual == 0){
			imadir_parent(simasel);
			checkdir(simasel->dir);
			clear_ima_dir(simasel);
			queredraw = 1;
		}
		break;
		
	case IKEY:
		if ((G.qual == 0)&&(simasel->file)){
			sprintf(name, "$IMAGEEDITOR %s%s", simasel->dir, simasel->file);
			system(name);
			queredraw = 1;
		}

		break;
	
	case PADPLUSKEY:
	case EQUALKEY:
		newname(simasel->file, +1);
		queredraw = 1;
		break;
		
	case PADMINUS:
	case MINUSKEY:
		newname(simasel->file, -1);
		queredraw = 1;
		break;
		
	case BACKSLASHKEY:
	case SLASHKEY:
#ifdef WIN32
		strcpy(simasel->dir, "\\");
#else
		strcpy(simasel->dir, "/");
#endif
		clear_ima_dir(simasel);
		simasel->image_slider = 0.0;
		queredraw = 1;
		break;
		
	case PERIODKEY:
		clear_ima_dir(simasel);
		queredraw = 1;
		break;
	
	case ESCKEY:
		filesel_prevspace();
		break;

	case PADENTER:
	case RETKEY:
		if (simasel->returnfunc){
			strcpy(name, simasel->dir);
			strcat(name, simasel->file);
			simasel->returnfunc(name);
			filesel_prevspace();
		}
		break;
	}
	
		
	if (queredraw) addqueue(curarea->win, REDRAW, 1);
}



