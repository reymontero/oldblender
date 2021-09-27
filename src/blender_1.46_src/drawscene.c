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



/*	drawscene.c		GRAPHICS
 * 
 *  jan 95
 * 
 *  ook EDIT funkties
 * 
 *  alleen routines met GRAPHICS!
 * 
 */
	
#include "blender.h"
#include "graphics.h"



void set_scene(Scene *sce)		/* zie ook scene.c: set_scene_bg() */
{
	bScreen *sc;
	ScrArea *sa;
	View3D *vd;
	Base *base;
	
	G.scene= sce;

	sc= G.main->screen.first;
	while(sc) {
		if((U.flag & SCENEGLOBAL) || sc==G.curscreen) {
		
			if(sce != sc->scene) {
				/* alle area's endlocalview */
				sa= sc->areabase.first;
				while(sa) {
					endlocalview(sa);
					sa= sa->next;
				}		
				sc->scene= sce;
			}
			
		}
		sc= sc->id.next;
	}
	
	copy_view3d_lock();	/* space.c */

	/* zijn er camera's in de views die niet in de scene zitten? */
	sc= G.main->screen.first;
	while(sc) {
		if( (U.flag & SCENEGLOBAL) || sc==G.curscreen) {
			sa= sc->areabase.first;
			while(sa) {
				vd= sa->spacedata.first;
				while(vd) {
					if(vd->spacetype==SPACE_VIEW3D) {
			
						if(vd->camera) {
							
							if( object_in_scene(vd->camera, sce)==0) {
								vd->camera= 0;
								if(vd->persp>1) vd->persp= 1;
							}
						}
					}
					vd= vd->next;
				}
				sa= sa->next;
			}
		}
		sc= sc->id.next;
	}

	set_scene_bg(G.scene);	
	
	/* volledige redraw */
	allqueue(REDRAWALL, 0);
	allqueue(REDRAWDATASELECT, 0);	/* doet remake */
}

