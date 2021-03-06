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


/* drawimasel.c  juli 96		GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "ipo.h"
#include "imasel.h"

#define IMALINESIZE 16

/* GLOBALS */
extern char *fsmenu;

void viewgate(short sx, short sy, short ex, short ey) 
{
	short wx, wy;
	wx = curarea->winrct.xmin; wy = curarea->winrct.ymin;
	glViewport(wx+sx, wy+sy, (wx+ex )-(wx+sx)+1, (wy+ey )-(wy+sy)+1); glScissor(wx+sx, wy+sy, (wx+ex )-(wx+sx)+1, (wy+ey )-(wy+sy)+1);
	ortho2((float)sx , (float)ex, (float)sy, (float)ey);
}	
	
void areaview (void) 
{
	short wx, wy;
	wx = curarea->winrct.xmin; wy = curarea->winrct.ymin;
	glViewport(wx,  wy, (wx+curarea->winx)-(wx)+1, (wy+curarea->winy)-( wy)+1); glScissor(wx,  wy, (wx+curarea->winx)-(wx)+1, (wy+curarea->winy)-( wy)+1);
	ortho2(0.0, (float)curarea->winx-1.0, 0.0, (float)curarea->winy-1.0);
}		
		
void calc_hilite(SpaceImaSel *simasel)
{
	OneSelectableIma *ima;
	ImaDir  *direntry;
	short   mx, my;
	int     i, area_event;
	
	if (simasel->hilite > -1) {
		direntry = simasel->firstdir; 
		while(direntry){
			direntry->hilite = 0;
			direntry = direntry->next;
		}	
		simasel->hilite = -1;
	}
	
	if (simasel->totalima){
		simasel->hilite_ima =  0;
		ima = simasel->first_sel_ima;
		while (ima){
			ima->selectable = 0;
			ima = ima->next;
		}
	}
		
	area_event = 0;
	mx = simasel->mx;
	my = simasel->my;
		
	if (simasel->desx > 0){
		if ( (mx > simasel->desx) && (mx < simasel->deex) && (my > simasel->desy) && (my < simasel->deey) ) area_event = IMS_INDIR;
	}
	if (simasel->fesx > 0){
		if ( (mx > simasel->fesx) && (mx < simasel->feex) && (my > simasel->fesy) && (my < simasel->feey) ) area_event = IMS_INFILE;
	}	
	
	switch(area_event){
	case IMS_INDIR:
		simasel->hilite = simasel->topdir + ((simasel->deey - my - 4) / IMALINESIZE);
		
		if (my >= simasel->deey)					simasel->hilite = -1;
		if (simasel->hilite >= simasel->totaldirs)  simasel->hilite = -1;
	
		if (simasel->hilite > -1){
			direntry = simasel->firstdir; 
			for (i = simasel->hilite; i>0; i--){
				direntry = direntry->next;
			}
			direntry->hilite = 1;
			
		}
		simasel->mouse_move_redraw = 1;
		break;
	
	case IMS_INFILE:
		if (simasel->totalima){
			ima = simasel->first_sel_ima;
			while (ima){
				ima->selectable = 0;
				
				if (ima->draw_me) {
					if ((mx > ima->sx) && (mx < ima->sx+76) && (my > ima->sy-16) && (my < ima->sy+76)) {
						ima->selectable = 1;
						simasel->hilite_ima = ima;
						simasel->mouse_move_redraw = 1;
					}
				}
				
				ima = ima->next;
			}	
		}
		break;
	}
}


void make_sima_area(SpaceImaSel *simasel)
{
	OneSelectableIma *ima;
	short rh, dm, sx, sy, sc;
	short boxperline, boxlines, boxlinesinview, boxlinesleft;

/*  ima slider  box */
	simasel->fssx =  8; 
	simasel->fssy = 8; 
	simasel->fsex = 30; 
	simasel->fsey = curarea->winy-64;
/*  ima entry's  box */
	simasel->fesx = simasel->fsex + 8; 
	simasel->fesy = simasel->fssy; 
	simasel->feex = curarea->winx- 8;  
	simasel->feey = curarea->winy-64;
/*  ima names */
	simasel->dnsx = 38; 
	simasel->dnsy = curarea->winy - 29; 
	simasel->dnw  = curarea->winx - 8 - 38; 
	simasel->dnh  = 21;
	simasel->fnsx = simasel->fesx; 
	simasel->fnsy = curarea->winy - 29 - 29; 
	simasel->fnw  = curarea->winx - 8 - simasel->fnsx; 
	simasel->fnh  = 21;
	
	if ((simasel->mode & 1)==1){
	/*  dir slider  box */
		simasel->dssx =   8; 
		
		simasel->dsex =  30; 
		simasel->dsey = curarea->winy-64;
	/*  dir entry's  box */
		simasel->desx =  38;
		simasel->desy =   8; 
		
		simasel->deex = 208; 
		simasel->deey = curarea->winy-64;
		simasel->dssy = simasel->desy; 
		if (simasel->deex > (curarea->winx -8) ) simasel->deex = curarea->winx - 8;
		if (simasel->deex <= simasel->desx ) simasel->dssx =  0;
	/*  file slider & entry & name box ++ */
		simasel->fssx += 216; 
		simasel->fsex += 216;
		simasel->fesx += 216;
		simasel->fnsx += 216;
		simasel->fnw  -= 216;
	}else{
		simasel->desx =  0;
	}
	
	if ((simasel->mode & 2) == 2){	
		simasel->fesy += 32;
		simasel->infsx = simasel->fesx; simasel->infsy =  8;
		simasel->infex = simasel->feex; simasel->infey = 28;
	}else{
		simasel->infsx = 0;
	}
	
	simasel->dsdh = simasel->deey - simasel->desy - 4;
	
	if (simasel->dsdh  <= 16)  { simasel->desx  = 0; }
	if ((simasel->feex-16)  <= simasel->fesx)  { simasel->fesx  = 0; }
	if ((simasel->infex-16) <= simasel->infsx) { simasel->infsx = 0; }
	
	if ((simasel->deey  ) <= simasel->desy)	   { simasel->desx  = 0; }
	if ((simasel->feey  ) <= simasel->fesy)	   { simasel->fesx  = 0; }
	if ((simasel->infey )  > simasel->feey)    { simasel->infsx  = 0;}
	
	/* Dir Slider */
	if (simasel->desx  != 0){
		simasel->dirsli = 0;
		
		simasel->dirsli_lines = (simasel->dsdh / IMALINESIZE);
		simasel->dirsli_h  = 0;
		
		if (simasel->topdir < 0) simasel->topdir = 0;
		if (simasel->topdir > (simasel->totaldirs - simasel->dirsli_lines) ) simasel->topdir = (simasel->totaldirs - simasel->dirsli_lines);
		
		if ( (simasel->totaldirs * IMALINESIZE) >= simasel->dsdh ){
			simasel->dirsli = 1;
			simasel->dirsli_sx = simasel->dssx+2;
			simasel->dirsli_ex = simasel->dsex-2;	
			
			simasel->dirsli_h   = (simasel->dsdh) * (float)simasel->dirsli_lines / (float)simasel->totaldirs;
			simasel->dirsli_ey  = simasel->dsey - 2;
			if (simasel->topdir) {
				rh = (simasel->dsdh - simasel->dirsli_h);
				simasel->dirsli_ey -= rh * (float)((float)simasel->topdir / (float)(simasel->totaldirs - simasel->dirsli_lines ));
			}
			
			if (simasel->dirsli_h < 4) simasel->dirsli_h = 4;
			
		}else{
			simasel->topdir = 0;
		}
	}
	
	if (simasel->totalima){
		/* there are images */
		
		ima = simasel->first_sel_ima;
		
		
		boxperline      =   (simasel->feex - simasel->fesx) /  80;
		if (boxperline) boxlines = 1 + (simasel->totalima / boxperline); else boxlines = 1;
		boxlinesinview  =   (simasel->feey - simasel->fesy) / 100;
		boxlinesleft    =   boxlines - boxlinesinview;
		
		if (boxlinesleft > 0){
			/* slider needed */
			
			simasel->slider_height = boxlinesinview / (float)(boxlines+1);
			simasel->slider_space  = 1.0 - simasel->slider_height;
			
			simasel->imasli_sx = simasel->fssx+1; 
			simasel->imasli_ex = simasel->fsex-1; 
			simasel->fsdh	   = simasel->fsey -  simasel->fssy - 4;
		
			simasel->imasli_h  = simasel->fsdh * simasel->slider_height;
			if (simasel->imasli_h < 6) simasel->imasli_h = 6;
			simasel->imasli_ey = simasel->fsey - 2 - (simasel->fsdh * simasel->slider_space * simasel->image_slider);
			
			simasel->imasli = 1;
		
		}else{
			simasel->image_slider = 0;
			simasel->imasli = 0;
		}
		
		sc = simasel->image_slider * (boxlinesleft * 100);
		
		simasel->curimax = simasel->fesx + 8;
		simasel->curimay = simasel->feey - 90 + sc;
		
		dm = 1;
		if (simasel->curimay-2  < simasel->fesy) dm = 0;
		if (simasel->curimay+80 > simasel->feey) dm = 0;
		if (simasel->curimax+72 > simasel->feex) dm = 0;
		
		simasel->total_selected = 0;
		while (ima){
			ima->draw_me = dm;
			
			if (ima->selected) simasel->total_selected++;
	
			ima->sx = simasel->curimax;
			ima->sy = simasel->curimay+16;
			
			ima->ex = ima->sx + ima->dw;
			ima->ey = ima->sy + ima->dh;
			
			simasel->curimax += 80;
			if (simasel->curimax + 72 > simasel->feex){
				
				simasel->curimax  = simasel->fesx + 8;
				simasel->curimay -= 100;
				
				dm = 1;
				if (simasel->curimay+80 > simasel->feey) dm = 0;
				if (simasel->curimay-8  < simasel->fesy) dm = 0;
				
			}
			ima = ima->next;
		}
	}
}

void draw_sima_area(SpaceImaSel *simasel)
{	
	OneSelectableIma *ima;
	ImaDir      *direntry;
	int   i, info, l;
	short sx, sy, ex, ey, sc;
	uint *rect;
	char naam[256], infostr[256];
	
	extern struct ButIcon BGicon[];
	
	glClearColor(0.4375, 0.4375, 0.4375, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	fmsetfont(G.font);
	
	sprintf(naam, "win %d", curarea->win);
	DefButBlock(naam, curarea->win, G.font, 10, 6, 4);
	SetButCol(4);
	
	if (simasel->desx >  0){
		/*  DIR ENTRYS */
		cpack(C_DERK);
		glRecti(simasel->dssx,  simasel->dssy,  simasel->dsex,  simasel->dsey);
		glRecti(simasel->desx,  simasel->desy,  simasel->deex,  simasel->deey);
	
		EmbossBox2(simasel->dssx, simasel->dssy, simasel->dsex, simasel->dsey,1, C_LO, C_HI);
		EmbossBox2(simasel->desx, simasel->desy, simasel->deex, simasel->deey,1, C_LO, C_HI);
		
		if (simasel->dirsli == 1){
			sx = simasel->dirsli_sx;
			sy = simasel->dirsli_ey - simasel->dirsli_h;
			ex = simasel->dirsli_ex;
			ey = simasel->dirsli_ey;
			
			glRecti(sx,  sy,  ex,  ey);
			EmbossBox2(sx, sy, ex,ey,1, C_HI, C_LO);
		}
		if (simasel->totaldirs){
			sx = simasel->desx+8;
			sy = simasel->deey-IMALINESIZE;
			fmsetfont(G.font);
			
			direntry = simasel->firstdir; 
			if (simasel->topdir){
				for(i = simasel->topdir; i>0; i--){
					direntry = direntry->next;
				}
			}
			viewgate(simasel->desx, simasel->desy, simasel->deex-4, simasel->deey);
			
			i = simasel->dirsli_lines;
			if (i > simasel->totaldirs) i = simasel->totaldirs;
			for(;i > 0; i--){
				strcpy(naam,  direntry->name);
				
				cpack(0xFFFFFF);
				if (direntry->selected == 1){
					cpack(0x7777CC);
					glRecti(simasel->desx+2,  sy-4,  simasel->deex-4,  sy+IMALINESIZE-4);
					cpack(0xFFFFFF);
				}
				if (direntry->hilite == 1){
					cpack(0x999999);
					glRecti(simasel->desx+2,  sy-4,  simasel->deex-4,  sy+IMALINESIZE-4);
					cpack(0xFFFFFF);
				}
				
				glRasterPos2i(sx,  sy);fmprstr(naam);
				
				direntry = direntry->next;
				sy-=IMALINESIZE;
			}
			areaview();
			
		}
		
		/* status icons */
		
		glPixelStorei(GL_UNPACK_ROW_LENGTH,  BGicon[0].xim);
		sx = simasel->desx;
		sy = simasel->deey+6;
		
		rect= BGicon[0].rect + (5*21 + 3)*BGicon[0].xim + (6*20 + 3);
		if (bitset(simasel->fase, IMS_FOUND_BIP)) rect+= 20;
		if (bitset(simasel->fase, IMS_WRITE_NO_BIP)) rect= BGicon[0].rect + (5*21 + 3)*BGicon[0].xim + (8*20 + 3);
		
		lrectwrite(sx, sy, sx+14,sy+15, rect); sx+=16;
		
		rect= BGicon[0].rect + (5*21 + 3)*BGicon[0].xim + (2*20 + 3);
		if (bitset(simasel->fase, IMS_KNOW_INF)) rect+= 20;
		lrectwrite(sx, sy, sx+14,sy+15, rect); sx+=16;
		
		rect= BGicon[0].rect + (5*21 + 3)*BGicon[0].xim + (4*20 + 3);
		if (bitset(simasel->fase, IMS_KNOW_IMA)) rect+= 20;
		lrectwrite(sx, sy, sx+14,sy+15, rect); sx+=16;

		glPixelStorei(GL_UNPACK_ROW_LENGTH,  0);
	}
	
	if (simasel->fesx >  0){
		
		cpack(C_DARK);

		glRecti(simasel->fssx,  simasel->fssy,  simasel->fsex,  simasel->fsey);

		glRecti(simasel->fesx,  simasel->fesy,  simasel->feex,  simasel->feey);

		EmbossBox2(simasel->fssx-1, simasel->fssy-1, simasel->fsex+1, simasel->fsey+1,1, C_LO, C_HI);
		EmbossBox2(simasel->fesx-1, simasel->fesy-1, simasel->feex+1, simasel->feey+1,1, C_LO, C_HI);
		
		if (simasel->fnw > 8) DefBut(TEX, 2,"", simasel->fnsx, simasel->fnsy, simasel->fnw, simasel->fnh, simasel->file, 0.0, (float)FILE_MAXFILE-1, 0, 0);
		
		if (simasel->imasli == 1){
			sx = simasel->imasli_sx;
			sy = simasel->imasli_ey - simasel->imasli_h;
			ex = simasel->imasli_ex;
			ey = simasel->imasli_ey;
			
			cpack(C_BACK);

			glRecti(sx,  sy,  ex,  ey);
			EmbossBox2(sx, sy, ex,ey,1, C_HI, C_LO);
		}
	
		info = 0;
		strcpy(infostr, "");
		if (simasel->totalima){
			viewgate(simasel->fesx, simasel->fesy, simasel->feex, simasel->feey);
		
			fmsetfont(G.fonts);
			
			ima = simasel->first_sel_ima;
			
			while (ima){
				sc = 0;
				
				sx = ima->sx- 6; sy = ima->sy-20 + sc; 
				ex = ima->sx+71; ey = ima->sy+70 + sc;  
				
				if(ima->selected == 1){
					cpack(0xCC6666);
					glRecti(sx, sy,  ex, ey);
				}
				if(ima->selectable == 1){
					if (ima->selected ) cpack(0xEE8888); else cpack(0x999999);
					
					if (((simasel->mode & 8) != 8) && (simasel->hilite_ima == ima)){
						glRecti(sx, sy,  ex, ey); EmbossBox2(sx,sy, ex,ey, 1, C_LO, C_HI);
					}
					if (ima->disksize/1000 > 1000){ sprintf(infostr,  "%s  %.2fMb  x%i y%i  %i bits ",ima->file_name,(ima->disksize/1024)/1024.0, ima->orgx, ima->orgy, ima->orgd);
					}else{ sprintf(infostr,  "%s  %dKb  x%i y%i  %i bits ",  ima->file_name,ima->disksize/1024,          ima->orgx, ima->orgy, ima->orgd);
					}	
					if (ima->anim == 1){ strcat (infostr, "movie"); }else{
						str_image_type(ima->ibuf_type, naam);
						strcat (infostr, naam);
					}
					info = 1;
				}
				
				sx = ima->sx; sy = ima->sy + sc;
				ex = ima->ex; ey = ima->ey + sc;
				
				if (ima->anim == 0) cpack(C_DARK); else cpack(C_DERK);
				
				glRecti(sx, sy,  ex, ey);
				EmbossBox2(sx-1,sy-1, ex+1,ey+1, 1, C_HI, C_LO);
				
				cpack(0);
				strcpy(naam, ima->file_name);
				naam[11] = 0;
			
				glRasterPos2i(sx+32-fmgetstrwidth(G.fonts, naam) / 2  ,  sy-16);
				fmprstr(naam);
				if ((ima) && (ima->pict) && (ima->pict->rect)){
					if ( (ey > simasel->fesy) && (sy < simasel->feey)){
						lrectwrite(sx, sy, ex-1, ey-1, ima->pict->rect);
					}
				}
			
				ima = ima->next;
			}
			
			if ((simasel->mode & 8) == 8) {	 /* if loep */
				
				if (bitset(simasel->fase, IMS_KNOW_IMA) && (simasel->hilite_ima)) {
			
				 	ima = simasel->hilite_ima;
					glPixelZoom(2.0,  2.0);
					
					sx = ima->sx + (ima->ex - ima->sx)/2 - (ima->ex - ima->sx);
					sy = ima->sy + (ima->ey - ima->sy)/2 - (ima->ey - ima->sy);
					
					ex = sx + 2*(ima->ex - ima->sx);
					ey = sy + 2*(ima->ey - ima->sy);
					
					/* cpack(C_DERK); */
					/* EmbossBox2(sx-8,sy-8, ex+8,ey+8, 1, C_HI, C_LO); */
					/* glRecti(sx-7, sy-7,  ex+7, ey+7); */
					EmbossBox2(sx-1,sy-1, ex+1,ey+1, 0, C_HI, C_LO);
				
					lrectwrite(sx, sy, sx+ (ima->ex - ima->sx)-1, sy+ (ima->ey - ima->sy)-1, ima->pict->rect);
					
					glPixelZoom(1.0,  1.0);
				}
			}
			areaview();  /*  reset viewgate */
		}
		
		
		/* INFO */
		if (simasel->infsx > 0){
			cpack(C_DARK);

			glRecti(simasel->infsx,  simasel->infsy,  simasel->infex,  simasel->infey);
			EmbossBox2(simasel->infsx, simasel->infsy, simasel->infex, simasel->infey,1, C_LO, C_HI);
		
			if ((info)&&(strlen(infostr) > 0)){
				
				sx = curarea->winrct.xmin;
				sy = curarea->winrct.ymin;
				
				viewgate(simasel->infsx, simasel->infsy, simasel->infex, simasel->infey);
			
				fmsetfont(G.font);
				cpack(0xAAAAAA);
				glRasterPos2i(simasel->infsx+4,  simasel->infsy+6);
				fmprstr(infostr);
				
				areaview();  /*  reset viewgate */
	
			}
		}
	}
	if (simasel->dnw > 8) DefBut(TEX, 1,"", simasel->dnsx, simasel->dnsy, simasel->dnw, simasel->dnh, simasel->dir,  0.0, (float)FILE_MAXFILE-1, 0, 0);
	
	if (curarea->winx > 16){
		SetButShape(2);
		DefBut(BUT,4, "P", 8, curarea->winy-29, 20, 21, 0, 0, 0, 0, 0);
		if(fsmenu) {
			DefBut(MENU|SHO,3, fsmenu, 8, curarea->winy-58, 20, 21, &simasel->fileselmenuitem, 0, 0, 0, 0);
		}
	}
}

void select_ima_files(SpaceImaSel *simasel)
{
	short set_reset;
	short mval[2], oval[2];
	
	set_reset =  1 - (simasel->hilite_ima->selected);
	
	getmouseco_areawin(mval);
	oval[0] = mval[0] + 1;
	
	while(get_mbut()&R_MOUSE) { 
		getmouseco_areawin(mval);
		if ((oval[0] != mval[0]) || (oval[1] != mval[1])){
			simasel->mx = mval[0];
			simasel->my = mval[1];
			
			calc_hilite(simasel);
			
			if (simasel->hilite_ima){
				simasel->hilite_ima->selected = set_reset;
				curarea->windraw();
				screen_swapbuffers();	
			}
			oval[0] = mval[0];
			oval[1] = mval[1];
		}
	}
}

void move_imadir_sli(SpaceImaSel *simasel)
{
	
	short mval[2], oval[2], lval[2], fh;
	float rh;
	
	getmouseco_areawin(mval);
	
	if ((mval[0] > simasel->dirsli_sx) && 
	    (mval[0] < simasel->dirsli_ex) && 
		(mval[1] > simasel->dirsli_ey - simasel->dirsli_h) &&
		(mval[1] < simasel->dirsli_ey) ){
		
		/*  extactly in the slider  */
		fh = simasel->dirsli_ey - mval[1];
		lval[1]=1;oval[1] = mval[1];
		while(get_mbut()&L_MOUSE) { 
			getmouseco_areawin(mval);
			if (mval[1] != lval[1]){
				
				rh = (float)(simasel->dsey - mval[1] - fh - simasel->dssy) / (simasel->dsdh - simasel->dirsli_h);
				
				simasel->topdir = 1 + rh * (simasel->totaldirs - simasel->dirsli_lines);
				
				curarea->windraw();
				EmbossBox2(simasel->dirsli_sx, simasel->dirsli_ey - simasel->dirsli_h, 
						   simasel->dirsli_ex, simasel->dirsli_ey,1, C_LO, C_HI);
			    screen_swapbuffers();
				lval[1] = mval[1];
			}
		}	
	}else{
		if (mval[1] < simasel->dirsli_ey - simasel->dirsli_h)
			simasel->topdir += (simasel->dirsli_lines - 1); 
		else
			simasel->topdir -= (simasel->dirsli_lines - 1); 
			
		while(get_mbut()&L_MOUSE) {  }
	}
}

void move_imafile_sli(SpaceImaSel *simasel)
{
	short mval[2], cmy, smy, omy;
	short ssl, sdh, ssv;
	float rh;
	
	getmouseco_areawin(mval);
	cmy = mval[1];
	
	if ((mval[0] > simasel->imasli_sx) && 
	    (mval[0] < simasel->imasli_ex) && 
		(mval[1] > simasel->imasli_ey - simasel->imasli_h) &&
		(mval[1] < simasel->imasli_ey) ){
		
		ssv = simasel->fsey - simasel->imasli_ey - 2;
		
		while(get_mbut()&L_MOUSE) { 
			getmouseco_areawin(mval);
			if (mval[1] != omy){
				sdh = simasel->fsdh - simasel->imasli_h;
				ssl = cmy -  mval[1] + ssv;
				
				if (ssl < 0)   { ssl = 0; }
				if (ssl > sdh) { ssl = sdh; }
				
				simasel->image_slider = ssl / (float)sdh;
				
				curarea->windraw();
				EmbossBox2(simasel->imasli_sx, simasel->imasli_ey - simasel->imasli_h, 
						   simasel->imasli_ex, simasel->imasli_ey,1, C_LO, C_HI);
			   
				screen_swapbuffers();
				omy = mval[1];
			}
		}
	}else{
		while(get_mbut()&L_MOUSE) {  }
	}
}

void ima_select_all(SpaceImaSel *simasel)
{
	OneSelectableIma *ima;
	int reselect = 0;
	
	ima = simasel->first_sel_ima;
	if (!ima) return;
	
	while(ima){
		if (ima->selected == 1) reselect = 1;
		ima = ima->next;
	}
	ima = simasel->first_sel_ima;
	if (reselect == 1){
		while(ima){
			ima->selected = 0;
			ima = ima->next;
		}
	}else{
		while(ima){
			ima->selected = 1;
			ima = ima->next;
		}
	}
}

void pibplay(SpaceImaSel *simasel)
{
	OneSelectableIma *ima;
	int sx= 8, sy= 8;
	
	ima = simasel->first_sel_ima;
	if (!ima) return ;
	
	sx = curarea->winrct.xmin + 8; 
	sy = curarea->winrct.ymin + 8;
	
	while(!(get_mbut()&L_MOUSE)){
		curarea->windraw();	 
		
		lrectwrite(sx, sy, sx+ima->dw-1, sy+ima->dh-1, ima->pict->rect);
		
		ima = ima->next;
		if (!ima) ima = simasel->first_sel_ima;
		screen_swapbuffers();	
	}
}



/* ************** hoofdtekenfunktie ************** */

void drawimasel()	/* hoofdtekenfunktie */
{
	SpaceImaSel *simasel;
	simasel= curarea->spacedata.first;
	
	/* ortho: xmin xmax, ymin, ymax! */
	ortho2(0.0, (float)curarea->winx-1.0, 0.0, (float)curarea->winy-1.0);
	
	if (simasel->fase == 0){
		checkdir(simasel->dir);
		clear_ima_dir(simasel);
	}

	if (!bitset(simasel->fase, IMS_KNOW_DIR)){
		if(simasel->firstdir)	free_ima_dir(simasel->firstdir);
		if(simasel->firstfile) 	free_ima_dir(simasel->firstfile);
		simasel->firstdir   = 0;
		simasel->firstfile  = 0;
	
		if (get_ima_dir(simasel->dir, IMS_DIR,  &simasel->totaldirs,  &simasel->firstdir) < 0){
			/* error */
			strcpy(simasel->dir, simasel->dor);
			get_ima_dir(simasel->dir, IMS_DIR,  &simasel->totaldirs,  &simasel->firstdir);
		}
		
		if (get_ima_dir(simasel->dir, IMS_FILE, &simasel->totalfiles, &simasel->firstfile) < 0){
			/* error */
			strcpy(simasel->file, simasel->fole);
			get_ima_dir(simasel->dir, IMS_FILE, &simasel->totalfiles, &simasel->firstfile);
		}
		
		simasel->topdir  = 0;
		simasel->topfile = 0;
		simasel->fase |= IMS_KNOW_DIR;
		
		check_for_pib(simasel);
		
		strcpy(simasel->fole, simasel->file);
		strcpy(simasel->dor,  simasel->dir);
	}
		
	if (!bitset(simasel->fase, IMS_FOUND_BIP)){
		/* Make the first Bip file ever in this directory */
		if ( !bitset(simasel->fase, IMS_KNOW_INF)){
			if (!bitset(simasel->fase, IMS_DOTHE_INF)){
				if(simasel->first_sel_ima)	free_sel_ima(simasel->first_sel_ima);
				simasel->first_sel_ima   = 0;
				simasel->fase |= IMS_DOTHE_INF;
				addafterqueue(curarea->win, AFTERIMASELIMA, 1);
			}
		}
	}else{
		if (!bitset(simasel->fase, IMS_KNOW_BIP)){
			addafterqueue(curarea->win, AFTERPIBREAD, 1);
		}
	}
	
	make_sima_area(simasel);
	calc_hilite(simasel);
	draw_sima_area(simasel);
	
	curarea->win_swap= WIN_BACK_OK;
}


