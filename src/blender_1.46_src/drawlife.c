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



/*  drawlife.c     GRAPHICS
 * 
 *  maart 96
 *  
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "sector.h"


void drawlife(Object *ob, int dt, ulong col)
{
	Life *lf;
	Object *obedit;
	float vec[3];
	int flag;
	
	lf= ob->data;

	if(G.f & G_BACKBUFSEL);
	else {

		cpack(col);

		if(lf->flag & LF_GHOST_OTHER) {
			if( (G.f & G_BACKBUFSEL)==0) {
				flag= lf->flag;
				obedit= G.obedit;
				G.obedit= 0;
			
				if(lf->flag & LF_SHOW_TEXMESH) lf->flag &= ~LF_SHOW_TEXMESH;
				else lf->flag |= LF_SHOW_TEXMESH;
				
				cpack(0x505050);
				drawmeshwire(ob);
				
				lf->flag= flag;
				G.obedit= obedit;
				cpack(col);
			}
		}
		
		/* schaduw */
		if(lf->type == LF_DYNAMIC) {
			if(lf->floor) {
				float tmat[4][4];
			
				getmatrix(tmat);
				
				cpack(0);
				setpattern(1);
				
				loadmatrix(G.vd->viewmat);
				translate(ob->obmat[3][0], ob->obmat[3][1], ob->obmat[3][2]);
				
				/* floorloc is t.o.v. oldloc!!! */
				vec[0]= lf->floorloc[0];
				vec[1]= lf->floorloc[1];
				vec[2]= lf->floorloc[2];

				vec[2]+= 0.2*lf->axsize;
				bgnpolygon();
				vec[0]-= 0.4*lf->axsize;
				vec[1]-= 0.4*lf->axsize;
				v3f(vec);
				vec[0]+= 0.8*lf->axsize;
				v3f(vec);
				vec[1]+= 0.8*lf->axsize;
				v3f(vec);
				vec[0]-= 0.8*lf->axsize;
				v3f(vec);
				endpolygon();
				setpattern(0);
				cpack(col);
				
				loadmatrix(tmat);
			}
		}
	}

	if(lf->collision) {
		if(lf->collision->type==OB_LIFE) cpack(0xFFFFFF);
	}
	if(lf->dflag & LF_NO_DAMAGE) cpack(0x00FF00);

	if(lf->flag & LF_SHOW_TEXMESH) {
		if(lf->texmesh) {

			if(ob==G.obedit) drawmeshwire(ob);
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else if(dt==OB_WIRE) drawmeshwire(ob);
			else {
				
				if((G.f & (G_PICKSEL))) drawmeshwire(ob);
				else if((G.f & (G_BACKBUFSEL))) drawmeshsolid(ob, 0);
				else draw_tface_mesh(ob, lf->texmesh, dt);
			}
		}		
	}
	else {
		if(lf->dynamesh) {
			
			if(dt>OB_SOLID) dt= OB_SOLID;	/* shaded bij dynamesh: werkt niet */
			
			if(ob==G.obedit) drawmeshwire(ob);
			else if(dt==OB_BOUNDBOX) drawboundbox(ob);
			else if(dt==OB_WIRE) drawmeshwire(ob);
			else  drawDispList(ob, dt);	
		}
	}
	
	if(lf->flag & LF_SPHERE) {
		float tmat[4][4], imat[4][4];
		
		vec[0]= vec[1]= vec[2]= 0.0;
		getmatrix(tmat);
		Mat4Invert(imat, tmat);
		
		setlinestyle(1);
		drawcircball(vec, lf->axsize, imat);
		setlinestyle(0);
	}
	
	/* letop: is globale snelheidsvector (rotatie uit life eigenlijk wissen) */
	/* zit faceselecting in de weg
	VECCOPY(vec, lf->speed);
	bgnline();
	v3f(vec);
	VecAddf(vec, vec, lf->speed);
	v3f(vec);
	endline();
	*/
}

