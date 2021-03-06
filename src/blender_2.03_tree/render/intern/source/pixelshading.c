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

 * Shading of pixels
 *
 * 11-09-2000 nzc
 *
 * Version: $Id: pixelshading.c,v 1.5 2000/09/20 11:31:41 nzc Exp $
 *
 * Shading hierarchy:
 *
 * (externally visible)
 *
 * renderPixel----------
 *                     | renderHaloPixel-- shadeHaloFloat
 *                     |                
 *                     | renderFacePixel-- shadeLampLusFloat
 *                                       | shadeSpotHaloPixelFloat-- spotHaloFloat
 *
 *
 * renderSpotHaloPixel--(should call shadeSpotHaloPixelFloat, but there's numerical)
 *                      ( issues there... need to iron that out still              )
 */

/* probably needs a few external things... which ones? */
#include "render.h"
#include "vanillaRenderPipe_types.h"
#include "pixelblending.h"
#include "initrender.h"
#include "renderfg.h"
#include "zbuf.h"
#include "rendercore.h"
#include "texture.h"
#include "zbufferdatastruct.h"
#include "shadbuf.h"

#include "matrixops.h"
#include "vectorops.h"
#include "errorHandler.h"

#include "pixelshading.h"

/* ------------------------------------------------------------------------- */
/* maybe declare local functions here?                                       */
/* ------------------------------------------------------------------------- */

/* The collector is the communication channel with the render pipe.          */
extern RE_COLBUFTYPE  collector[4];  /* used throughout as pixel colour accu */
extern ushort usegamtab, shortcol[4], 
	*mask1[9], *mask2[9];
extern ushort *igamtab1; /* forward trafo, byte lookup table */
extern ushort *igamtab2; /* forward trafo, short lookup table*/
extern ushort *gamtab;   /* inverse trafo, shorts  */
/* shortcol was the old collector */
extern float holoofs, fmask[256];
extern float panophi;        /* Panorama angle (I think).                    */
extern float panovco;        /* Cosine and sine factors. Don't know exactly  */
extern float panovsi;        /* what these do.                               */
extern float Zmulx, Zmuly;   /* Some kind of scale?                          */

/* ------------------------------------------------------------------------- */
/* if defined: do full error tracing and reporting here                      */
/*  #define RE_PIXSHADE_FULL_SAFETY */
/* if defined: use fake (dummy) colours for filling pixels (all is purple)   */
/* #define RE_FAKE_PIXELS */
/* if defined: use fake (dummy) colours for filling faces (all blue)         */
/* #define RE_FAKE_FACE_PIXELS */
/* if defined: use fake (dummy) colours for filling halos (all red)          */
/* #define RE_FAKE_HALO_PIXELS */
/* if defined: use fake (dummy) colours for filling spothalos (green)        */
/* #define RE_FAKE_SPOTHALO_PIXELS   */

/* ------------------------------------------------------------------------- */

void *renderPixel(float x, float y, int *obdata)
{
    void* data = NULL;
#ifdef RE_PIXSHADE_FULL_SAFETY
    char fname[] = "renderPixel";
#endif
#ifdef RE_FAKE_PIXELS
    collector[0] = RE_UNITY_COLOUR_FLOAT;
    collector[1] = 0;
    collector[2] = RE_UNITY_COLOUR_FLOAT;
    collector[3] = RE_UNITY_COLOUR_FLOAT;
    return NULL;  
#endif
    
    if (obdata[3] & RE_POLY) {
        /* face pixels aren't rendered in floats yet, so we wrap it here */
		data = renderFacePixel(x, y, obdata[1]);
    }
    else if (obdata[3] & RE_HALO) {
        data = renderHaloPixel(x, y, obdata[1]);
    }
	else if( obdata[1] == 0 ) {	
		/* for lamphalo, but doesn't seem to be called? Actually it is, and  */
		/* it returns NULL pointers. */
        data = renderFacePixel(x, y, obdata[1]);
 	}		
#ifdef RE_PIXSHADE_FULL_SAFETY
    else RE_error(RE_BAD_FACE_TYPE, fname);
#endif /* RE_PIXSHADE_FULL_SAFETY */    
    return data;
   
} /* end of void renderPixel(float x, float y, int *obdata) */

/* ------------------------------------------------------------------------- */

void *renderFacePixel(float x, float y, int vlaknr) 
/* Result goes into <collector>                                              */
{
#ifdef RE_PIXSHADE_FULL_SAFETY
	char fname[]= "renderFacePixelFloat";
#endif
    static VlakRen *vlr; /* static, because we don't want to recalculate vlr */
                         /* when we already know it                          */
    static VertRen *v1, *v2, *v3;
    static float t00, t01, t10, t11, dvlak, n1[3], n2[3], n3[3];
    static float s00, s01, s10, s11;
    float *o1, *o2, *o3;
    float u, v, l, dl, hox, hoy, detsh, fac, deler, alpha;
    char *cp1, *cp2, *cp3;

/*  	RE_error(RE_TRACE_COUNTER, fname); */
	
#ifdef RE_FAKE_FACE_PIXELS
    collector[0] = 0;
    collector[1] = 0;
    collector[2] = RE_UNITY_COLOUR_FLOAT;
    collector[3] = RE_UNITY_COLOUR_FLOAT;
    /* try to keep the rest as clean as possible... */
    if( ((vlaknr & 0x7FFFFF) <= R.totvlak) 
        && ! ((R.vlaknr== -1)||(vlaknr<=0)) ) {
        /* a bit superfluous to do this always, but    */
        /* when this is switched on, it doesn't matter */
        vlr= addvlak( (vlaknr-1) & 0x7FFFFF);
    } else vlr = NULL; 
    return vlr;
#endif
    
    if(R.vlaknr== -1) {	/* doet initrender */
        /* also set in the pixelrender loop */
        vlr= R.vlr= 0;
    }
    
    if(vlaknr<=0) {	/* sky */
        R.vlaknr= 0;
		collector[3] = 0.0;
    }
    else if( (vlaknr & 0x7FFFFF) <= R.totvlak) {

		/* What follows now is a large bunch of texture coordinate mappings. */
	    /* When this face is the same as the previous one, that means all    */
        /* the coordinate remapping does not need to be recomputed.          */
        if(vlaknr!=R.vlaknr) {
            
            vlr= addvlak( (vlaknr-1) & 0x7FFFFF);
            
            R.mat= vlr->mat;
            R.matren= R.mat->ren;
            
            if(R.matren==0) {	/* tijdelijk voor debug */
				collector[3] = RE_UNITY_COLOUR_FLOAT;
				collector[2] = 0.0;
				collector[1] = RE_UNITY_COLOUR_FLOAT;
				collector[0] = RE_UNITY_COLOUR_FLOAT;
                return NULL;
            }
            
            R.vlr= vlr;
            
            R.vno= vlr->n;
            R.osatex= (R.matren->texco & TEXCO_OSA);
            R.vlaknr= vlaknr;
            
            v1= vlr->v1;
            dvlak= MTC_dot3Float(v1->co, vlr->n);
            
            if( (vlr->flag & R_SMOOTH) || (R.matren->texco & NEED_UV)) {	/* uv nodig */
                if(vlaknr & 0x800000) {
                    v2= vlr->v3;
                    v3= vlr->v4;
                }
                else {
                    v2= vlr->v2;
                    v3= vlr->v3;
                }
                
                if(vlr->snproj==0) {
                    t00= v3->co[0]-v1->co[0]; t01= v3->co[1]-v1->co[1];
                    t10= v3->co[0]-v2->co[0]; t11= v3->co[1]-v2->co[1];
                }
                else if(vlr->snproj==1) {
                    t00= v3->co[0]-v1->co[0]; t01= v3->co[2]-v1->co[2];
                    t10= v3->co[0]-v2->co[0]; t11= v3->co[2]-v2->co[2];
                }
                else {
                    t00= v3->co[1]-v1->co[1]; t01= v3->co[2]-v1->co[2];
                    t10= v3->co[1]-v2->co[1]; t11= v3->co[2]-v2->co[2];
                }
                
                detsh= t00*t11-t10*t01;
                t00/= detsh; t01/=detsh; 
                t10/=detsh; t11/=detsh;
                
                if(vlr->flag & R_SMOOTH) { /* puno's goedzetten */
                    if(vlr->puno & ME_FLIPV1) MTC_cp3FloatInv(v1->n, n1);
                    else                      MTC_cp3Float(v1->n, n1);
					
					if(vlaknr & 0x800000) {
                        if(vlr->puno & ME_FLIPV3) MTC_cp3FloatInv(v2->n, n2);
                        else                      MTC_cp3Float(v2->n, n2);
                        
                        if(vlr->puno & ME_FLIPV4) MTC_cp3FloatInv(v3->n, n3);
                        else                      MTC_cp3Float(v3->n, n3);
                    }
                    else {
                        if(vlr->puno & ME_FLIPV2) MTC_cp3FloatInv(v2->n, n2);
                        else                      MTC_cp3Float(v2->n, n2);
                        
                        if(vlr->puno & ME_FLIPV3) MTC_cp3FloatInv(v3->n, n3);
                        else                      MTC_cp3Float(v3->n, n3);
                    }
                }
                if(R.matren->texco & TEXCO_STICKY) {
                    s00= v3->ho[0]/v3->ho[3] - v1->ho[0]/v1->ho[3];
                    s01= v3->ho[1]/v3->ho[3] - v1->ho[1]/v1->ho[3];
                    s10= v3->ho[0]/v3->ho[3] - v2->ho[0]/v2->ho[3];
                    s11= v3->ho[1]/v3->ho[3] - v2->ho[1]/v2->ho[3];
                    
                    detsh= s00*s11-s10*s01;
                    s00/= detsh; s01/=detsh; 
                    s10/=detsh; s11/=detsh;
                }
            }
        } /* end of if vlaknr*/
        
		/* This trafo might be migrated to a separate function. It is used   */
		/* quite often.                                                      */
        /* COXYZ nieuwe methode */
        if( (G.special1 & G_HOLO)
			&& (((Camera *)G.scene->camera->data)->flag & CAM_HOLO2) ) {
            R.view[0]= (x+(R.xstart)+1.0+holoofs);
        }
        else {
            R.view[0]= (x+(R.xstart)+1.0);
        }
        
        if(R.flag & R_SEC_FIELD) {
            if(R.r.mode & R_ODDFIELD) R.view[1]= (y+R.ystart+0.5)*R.ycor;
            else R.view[1]= (y+R.ystart+1.5)*R.ycor;
        }
        else R.view[1]= (y+R.ystart+1.0)*R.ycor;
        
        R.view[2]= -R.viewfac;
        
        if(R.r.mode & R_PANORAMA) {
            u= R.view[0]; v= R.view[2];
            R.view[0]= panovco*u + panovsi*v;
            R.view[2]= -panovsi*u + panovco*v;
        }
        
        deler= vlr->n[0]*R.view[0] + vlr->n[1]*R.view[1] + vlr->n[2]*R.view[2];
        if (deler!=0.0) fac= R.zcor= dvlak/deler;
        else fac= R.zcor= 0.0;
        
        R.co[0]= fac*R.view[0];
        R.co[1]= fac*R.view[1];
        R.co[2]= fac*R.view[2];
        
        if(R.osatex || (R.r.mode & R_SHADOW) ) {
            u= dvlak/(deler-vlr->n[0]);
            v= dvlak/(deler- R.ycor*vlr->n[1]);
            
            O.dxco[0]= R.co[0]- (R.view[0]-1.0)*u;
            O.dxco[1]= R.co[1]- (R.view[1])*u;
            O.dxco[2]= R.co[2]- (R.view[2])*u;
            
            O.dyco[0]= R.co[0]- (R.view[0])*v;
            O.dyco[1]= R.co[1]- (R.view[1]-1.0*R.ycor)*v;
            O.dyco[2]= R.co[2]- (R.view[2])*v;
            
        }
        
        fac= Normalise(R.view);
        R.zcor*= fac;	/* voor mist */
        
        if(R.osatex) {
            if( (R.matren->texco & TEXCO_REFL) ) {
                O.dxview= 1.0/fac;
                O.dyview= R.ycor/fac;
            }
        }
        
        /* UV en TEX*/
        if( (vlr->flag & R_SMOOTH) || (R.matren->texco & NEED_UV)) {
            if(vlr->snproj==0) {
                u= (R.co[0]-v3->co[0])*t11-(R.co[1]-v3->co[1])*t10;
                v= (R.co[1]-v3->co[1])*t00-(R.co[0]-v3->co[0])*t01;
                if(R.osatex) {
                    O.dxuv[0]=  O.dxco[0]*t11- O.dxco[1]*t10;
                    O.dxuv[1]=  O.dxco[1]*t00- O.dxco[0]*t01;
                    O.dyuv[0]=  O.dyco[0]*t11- O.dyco[1]*t10;
                    O.dyuv[1]=  O.dyco[1]*t00- O.dyco[0]*t01;
                }
            }
            else if(vlr->snproj==1) {
                u= (R.co[0]-v3->co[0])*t11-(R.co[2]-v3->co[2])*t10;
                v= (R.co[2]-v3->co[2])*t00-(R.co[0]-v3->co[0])*t01;
                if(R.osatex) {
                    O.dxuv[0]=  O.dxco[0]*t11- O.dxco[2]*t10;
                    O.dxuv[1]=  O.dxco[2]*t00- O.dxco[0]*t01;
                    O.dyuv[0]=  O.dyco[0]*t11- O.dyco[2]*t10;
                    O.dyuv[1]=  O.dyco[2]*t00- O.dyco[0]*t01;
                }
            }
            else {
                u= (R.co[1]-v3->co[1])*t11-(R.co[2]-v3->co[2])*t10;
                v= (R.co[2]-v3->co[2])*t00-(R.co[1]-v3->co[1])*t01;
                if(R.osatex) {
                    O.dxuv[0]=  O.dxco[1]*t11- O.dxco[2]*t10;
                    O.dxuv[1]=  O.dxco[2]*t00- O.dxco[1]*t01;
                    O.dyuv[0]=  O.dyco[1]*t11- O.dyco[2]*t10;
                    O.dyuv[1]=  O.dyco[2]*t00- O.dyco[1]*t01;
                }
            }
            l= 1.0+u+v;
            
            if(vlr->flag & R_SMOOTH) {
                R.vn[0]= l*n3[0]-u*n1[0]-v*n2[0];
                R.vn[1]= l*n3[1]-u*n1[1]-v*n2[1];
                R.vn[2]= l*n3[2]-u*n1[2]-v*n2[2];
                
                Normalise(R.vn);
                if(R.osatex && (R.matren->texco & (TEXCO_NORM+TEXCO_REFL)) ) {
                    dl= O.dxuv[0]+O.dxuv[1];
                    O.dxno[0]= dl*n3[0]-O.dxuv[0]*n1[0]-O.dxuv[1]*n2[0];
                    O.dxno[1]= dl*n3[1]-O.dxuv[0]*n1[1]-O.dxuv[1]*n2[1];
                    O.dxno[2]= dl*n3[2]-O.dxuv[0]*n1[2]-O.dxuv[1]*n2[2];
                    dl= O.dyuv[0]+O.dyuv[1];
                    O.dyno[0]= dl*n3[0]-O.dyuv[0]*n1[0]-O.dyuv[1]*n2[0];
                    O.dyno[1]= dl*n3[1]-O.dyuv[0]*n1[1]-O.dyuv[1]*n2[1];
                    O.dyno[2]= dl*n3[2]-O.dyuv[0]*n1[2]-O.dyuv[1]*n2[2];
                    
                }
            }
            else {
                VECCOPY(R.vn, vlr->n);
            }
            
            if(R.matren->mode & MA_ZINV) {	/* z invert */
                /* R.vn[0]= -R.vn[0]; */
                /* R.vn[1]= -R.vn[1]; */
            }
            
            if(R.matren->texco & TEXCO_ORCO) {
                if(v2->orco) {
                    o1= v1->orco;
                    o2= v2->orco;
                    o3= v3->orco;
                    
                    R.lo[0]= l*o3[0]-u*o1[0]-v*o2[0];
                    R.lo[1]= l*o3[1]-u*o1[1]-v*o2[1];
                    R.lo[2]= l*o3[2]-u*o1[2]-v*o2[2];
                    
                    if(R.osatex) {
                        dl= O.dxuv[0]+O.dxuv[1];
                        O.dxlo[0]= dl*o3[0]-O.dxuv[0]*o1[0]-O.dxuv[1]*o2[0];
                        O.dxlo[1]= dl*o3[1]-O.dxuv[0]*o1[1]-O.dxuv[1]*o2[1];
                        O.dxlo[2]= dl*o3[2]-O.dxuv[0]*o1[2]-O.dxuv[1]*o2[2];
                        dl= O.dyuv[0]+O.dyuv[1];
                        O.dylo[0]= dl*o3[0]-O.dyuv[0]*o1[0]-O.dyuv[1]*o2[0];
                        O.dylo[1]= dl*o3[1]-O.dyuv[0]*o1[1]-O.dyuv[1]*o2[1];
                        O.dylo[2]= dl*o3[2]-O.dyuv[0]*o1[2]-O.dyuv[1]*o2[2];
                    }
                }
            }
            
            if(R.matren->texco & TEXCO_GLOB) {
                VECCOPY(R.gl, R.co);
                MTC_Mat4MulVecfl(R.viewinv, R.gl);
                if(R.osatex) {
                    VECCOPY(O.dxgl, O.dxco);
                    MTC_Mat3MulVecfl(R.imat, O.dxco);
                    VECCOPY(O.dygl, O.dyco);
                    MTC_Mat3MulVecfl(R.imat, O.dyco);
                }
            }
            if((R.matren->texco & TEXCO_UV) || (R.matren->mode & (MA_VERTEXCOL|MA_FACETEXTURE)))  {
                if(R.vlr->tface) {
                    float *uv1, *uv2, *uv3;
                    
                    uv1= R.vlr->tface->uv[0];
                    if( (vlaknr & 0x800000) || (R.vlr->flag & R_FACE_SPLIT) ) {
                        uv2= R.vlr->tface->uv[2];
                        uv3= R.vlr->tface->uv[3];
                    }
                    else {
                        uv2= R.vlr->tface->uv[1];
                        uv3= R.vlr->tface->uv[2];
                    }
                    
                    R.uv[0]= -1.0 + 2.0*(l*uv3[0]-u*uv1[0]-v*uv2[0]);
                    R.uv[1]= -1.0 + 2.0*(l*uv3[1]-u*uv1[1]-v*uv2[1]);
                    
                    if(R.osatex) {
                        float duv[2];
                        
                        dl= O.dxuv[0]+O.dxuv[1];
                        duv[0]= O.dxuv[0]; 
                        duv[1]= O.dxuv[1];
                        
                        O.dxuv[0]= 2.0*(dl*uv3[0]-duv[0]*uv1[0]-duv[1]*uv2[0]);
                        O.dxuv[1]= 2.0*(dl*uv3[1]-duv[0]*uv1[1]-duv[1]*uv2[1]);
                        
                        dl= O.dyuv[0]+O.dyuv[1];
                        duv[0]= O.dyuv[0]; 
                        duv[1]= O.dyuv[1];
                        
                        O.dyuv[0]= 2.0*(dl*uv3[0]-duv[0]*uv1[0]-duv[1]*uv2[0]);
                        O.dyuv[1]= 2.0*(dl*uv3[1]-duv[0]*uv1[1]-duv[1]*uv2[1]);
                    }
                }
                else {
                    R.uv[0]= 2.0*(u+.5);
                    R.uv[1]= 2.0*(v+.5);
                }
            }
            if(R.matren->texco & TEXCO_NORM) {
                R.orn[0]= R.vn[0];
                R.orn[1]= -R.vn[1];
                R.orn[2]= R.vn[2];
            }
            if(R.matren->mode & MA_VERTEXCOL) {

				/* some colour calculations here */
				cp1= (char *)vlr->vcol;
                if(cp1) {
                    if( (vlaknr & 0x800000) || (R.vlr->flag & R_FACE_SPLIT) ) {
                        cp2= (char *)(vlr->vcol+2);
                        cp3= (char *)(vlr->vcol+3);
                    }
                    else {
                        cp2= (char *)(vlr->vcol+1);
                        cp3= (char *)(vlr->vcol+2);
                    }
                    R.vcol[0]= (l*cp3[3]-u*cp1[3]-v*cp2[3])/255.0;
                    R.vcol[1]= (l*cp3[2]-u*cp1[2]-v*cp2[2])/255.0;
                    R.vcol[2]= (l*cp3[1]-u*cp1[1]-v*cp2[1])/255.0;
                    
                }
                else {
                    R.vcol[0]= 0.0;
                    R.vcol[1]= 0.0;
                    R.vcol[2]= 0.0;
                }
            }
            if(R.matren->mode & MA_FACETEXTURE) {
                if((R.matren->mode & MA_VERTEXCOL)==0) {
                    R.vcol[0]= 1.0;
                    R.vcol[1]= 1.0;
                    R.vcol[2]= 1.0;
                }
				/* shading here */
                if(vlr->tface) render_realtime_texture();
            }
            
            /* hierna klopt de u en v EN O.dxuv en O.dyuv niet meer */
            if(R.matren->texco & TEXCO_STICKY) {
                if(v2->sticky) {
                    
                    /* opnieuw u en v berekenen */
                    hox= x/Zmulx -1.0;
                    hoy= y/Zmuly -1.0;
                    u= (hox - v3->ho[0]/v3->ho[3])*s11
						- (hoy - v3->ho[1]/v3->ho[3])*s10;
                    v= (hoy - v3->ho[1]/v3->ho[3])*s00
						- (hox - v3->ho[0]/v3->ho[3])*s01;
                    l= 1.0+u+v;
                    
                    o1= v1->sticky;
                    o2= v2->sticky;
                    o3= v3->sticky;
                    
                    R.sticky[0]= l*o3[0]-u*o1[0]-v*o2[0];
                    R.sticky[1]= l*o3[1]-u*o1[1]-v*o2[1];
                    
                    if(R.osatex) {
                        O.dxuv[0]=  s11/Zmulx;
                        O.dxuv[1]=  - s01/Zmulx;
                        O.dyuv[0]=  - s10/Zmuly;
                        O.dyuv[1]=  s00/Zmuly;
                        
                        dl= O.dxuv[0]+O.dxuv[1];
                        O.dxsticky[0]= dl*o3[0]-O.dxuv[0]*o1[0]-O.dxuv[1]*o2[0];
                        O.dxsticky[1]= dl*o3[1]-O.dxuv[0]*o1[1]-O.dxuv[1]*o2[1];
                        dl= O.dyuv[0]+O.dyuv[1];
                        O.dysticky[0]= dl*o3[0]-O.dyuv[0]*o1[0]-O.dyuv[1]*o2[0];
                        O.dysticky[1]= dl*o3[1]-O.dyuv[0]*o1[1]-O.dyuv[1]*o2[1];
                    }
                }
            }
        }
        else {
            VECCOPY(R.vn, vlr->n);
        }
        if(R.matren->texco & TEXCO_WINDOW) {
            R.winco[0]= (x+(R.xstart))/(float)R.afmx;
            R.winco[1]= (y+(R.ystart))/(float)R.afmy;
        }
        
		/* After all texture coordinates are set and converted and           */
		/* transformed, we need to put some colour on it:                    */
		shadeLampLusFloat(); 

        /* MIST */
        if( (R.wrld.mode & WO_MIST) && (R.matren->mode & MA_NOMIST)==0 ){
			/* alpha returned in float? */
            alpha= mistfactor(R.co);
        }
        else alpha= 1.0;
        
        /* RAYTRACE (tijdelijk?) UITGESCHAKELD */
        
        if(R.matren->alpha!=1.0 || alpha!=1.0) {
            fac= alpha*(R.matren->alpha);
            
			/* gamma..  maybe this conflicts with my calculations */
            if(R.osa && usegamtab) fac*= fac;
			
			collector[0] *= fac; /* This applies to transparent faces! Even  */
			collector[1] *= fac; /* though it may seem to be a premul op, it */
			collector[2] *= fac; /* isn't.                                   */
			collector[3] = fac;  /* doesn't need scaling? */
        }
        else {
			collector[3] = 1.0;
        }
    }
    else {
		collector[0] = 1.0;
		collector[1] = 1.0;
		collector[2] = 0.0;
		collector[3] = 1.0;
    }

	/* Spothalos: do this here for covered pixels. It seems messy to place   */
	/* it here, structure-wise, but it's more efficient. Also, not having it */
	/* here makes it difficult to do proper overlaying later on.             */
	/* It starts off with a coordinate transform again.                      */
	if(R.flag & R_LAMPHALO) {
		if(vlaknr<=0) {	/* bereken viewvec en zet R.co op far */

			/* this view vector stuff should get its own function */
			if( (G.special1 & G_HOLO) && 
				((Camera *)G.scene->camera->data)->flag & CAM_HOLO2) {
				R.view[0]= (x+(R.xstart)+1.0+holoofs);
			} else {
				R.view[0]= (x+(R.xstart)+1.0);
			}

			if(R.flag & R_SEC_FIELD) {
				if(R.r.mode & R_ODDFIELD) R.view[1]= (y+R.ystart+0.5)*R.ycor;
				else R.view[1]= (y+R.ystart+1.5)*R.ycor;
			} else {
				R.view[1]= (y+R.ystart+1.0)*R.ycor;
			}
			
			R.view[2]= -R.viewfac;
			
			if(R.r.mode & R_PANORAMA) {
				u= R.view[0]; v= R.view[2];
				R.view[0]= panovco*u + panovsi*v;
				R.view[2]= -panovsi*u + panovco*v;
			}

			R.co[2]= 0.0;
			
		}
		shadeSpotHaloPixelFloat(collector);
				
	}
	
#ifdef RE_PIXSHADE_FULL_SAFETY
	if (!vlr) RE_error(RE_BAD_DATA_POINTER, fname);
#endif
	
	return vlr;
    
} /* end of void renderFacePixelFloat(float x, float y, int vlaknr) */

/* ------------------------------------------------------------------------- */
/* - uses R.view to determine which pixel, I think?                          */
/* - the spothalo is dumped quite unceremoniously on top of the col vector   */
void shadeSpotHaloPixelFloat(float *col)
{
	LampRen *lar;
	float factor = 0.0;
	int a;
	float rescol[4];
	
	
	for(a=0; a<R.totlamp; a++) {
		lar= R.la[a];
		if(lar->type==LA_SPOT && (lar->mode & LA_HALO) && lar->haint>0) {
	
			if(lar->org) {
				lar->r= lar->org->r;
				lar->g= lar->org->g;
				lar->b= lar->org->b;
			}

			/* determines how much spothalo we see */
			spotHaloFloat(lar, R.view, &factor);
			
			if(factor>0.0) {
				
				/* i*= 65536.0;  would convert [0,1] to ushort */

				if(factor > RE_FULL_COLOUR_FLOAT) rescol[3]= 1.0;
				else rescol[3]= factor;

				/* erg vervelend: gammagecorrigeerd renderen EN addalphaADD  */
				/* gaat niet goed samen                                      */
				/* eigenlijk moet er een aparte 'optel' gamma komen          */
				
				rescol[0] = factor * lar->r; /* Lampren rgb's are floats     */
				rescol[1] = factor * lar->g;
				rescol[2] = factor * lar->b;
				
				/* ---->add values, disregard alpha                          */
				/* - check for dest. alpha = 0. If so , just copy            */
				/* this is a slightly different approach: I do the gamma     */
				/* correction BEFORE the addition. What does the other       */
				/* approach do?                                              */
				if (col[3]< RE_EMPTY_COLOUR_FLOAT) {
					col[0] = gammaCorrect(rescol[0]);
					col[1] = gammaCorrect(rescol[1]);
					col[2] = gammaCorrect(rescol[2]);
					col[3] = rescol[3];
				} else {
					col[0] += gammaCorrect(rescol[0]);
					col[1] += gammaCorrect(rescol[1]);
					col[2] += gammaCorrect(rescol[2]);
					col[3] += rescol[3];
				}

				/* The other option: */
/*  				col[0] = gammaCorrect(col[0] + rescol[0]); */
/*  				col[1] = gammaCorrect(col[1] + rescol[1]); */
/*  				col[2] = gammaCorrect(col[2] + rescol[2]); */
/*  				col[3] = rescol[3]; */
				/* this clipping may have to go? Actually, if it's */
				/* done sooner, it may be more efficient */
				if(col[0] > RE_FULL_COLOUR_FLOAT) col[0] = 1.0;
				if(col[1] > RE_FULL_COLOUR_FLOAT) col[1] = 1.0;
				if(col[2] > RE_FULL_COLOUR_FLOAT) col[2] = 1.0;
				if(col[3] > RE_FULL_COLOUR_FLOAT) col[3] = 1.0;
				if(col[0] < RE_EMPTY_COLOUR_FLOAT) col[0] = 0.0;
				if(col[1] < RE_EMPTY_COLOUR_FLOAT) col[1] = 0.0;
				if(col[2] < RE_EMPTY_COLOUR_FLOAT) col[2] = 0.0;
				if(col[3] < RE_EMPTY_COLOUR_FLOAT) col[3] = 0.0;
			}
		}
	}
	
	if(col[0] < RE_EMPTY_COLOUR_FLOAT) col[0] = 0.0;
	if(col[1] < RE_EMPTY_COLOUR_FLOAT) col[1] = 0.0;
	if(col[2] < RE_EMPTY_COLOUR_FLOAT) col[2] = 0.0;
	if(col[3] < RE_EMPTY_COLOUR_FLOAT) col[3] = 0.0;

}

/* ------------------------------------------------------------------------- */

void spotHaloFloat(struct LampRen *lar, float *view, float *intens)
{
	double a, b, c, disc, nray[3], npos[3];
	float t0, t1 = 0.0, t2= 0.0, t3, haint;
	float p1[3], p2[3], ladist, maxz = 0.0, maxy = 0.0;
	int snijp, doclip=1, use_yco=0;
	int ok1=0, ok2=0;
	
	*intens= 0.0;
	haint= lar->haint;
	
	VECCOPY(npos, lar->sh_invcampos);	/* in initlamp berekend */
	
	/* view roteren */
	VECCOPY(nray, view);
	MTC_Mat3MulVecd(lar->imat, nray);
	
	if(R.wrld.mode & WO_MIST) {
		/* een beetje patch... */
		R.zcor= -lar->co[2];
		haint *= mistfactor(lar->co);
		if(haint==0.0) {
			return;
		}
	}


	/* maxz roteren */
	if(R.co[2]==0) doclip= 0;	/* is als halo op sky */
	else {
		p1[0]= R.co[0]-lar->co[0];
		p1[1]= R.co[1]-lar->co[1];
		p1[2]= R.co[2]-lar->co[2];
	
		maxz= lar->imat[0][2]*p1[0]+lar->imat[1][2]*p1[1]+lar->imat[2][2]*p1[2];
		maxz*= lar->sh_zfac;
		maxy= lar->imat[0][1]*p1[0]+lar->imat[1][1]*p1[1]+lar->imat[2][1]*p1[2];

		if( fabs(nray[2]) <0.000001 ) use_yco= 1;
	}
	
	/* z scalen zodat het volume genormaliseerd is */	
	nray[2]*= lar->sh_zfac;
	/* nray hoeft niet genormaliseerd */
	
	ladist= lar->sh_zfac*lar->dist;
	
	/* oplossen */
	a = nray[0] * nray[0] + nray[1] * nray[1] - nray[2]*nray[2];
	b = nray[0] * npos[0] + nray[1] * npos[1] - nray[2]*npos[2];
	c = npos[0] * npos[0] + npos[1] * npos[1] - npos[2]*npos[2];

	snijp= 0;
	if (fabs(a) < 0.00000001) {
		/*
		 * Only one intersection point...
		 */
		return;
	}
	else {
		disc = b*b - a*c;
		
		if(disc==0.0) {
			t1=t2= (-b)/ a;
			snijp= 2;
		}
		else if (disc > 0.0) {
			disc = sqrt(disc);
			t1 = (-b + disc) / a;
			t2 = (-b - disc) / a;
			snijp= 2;
		}
	}
	if(snijp==2) {
		/* sorteren */
		if(t1>t2) {
			a= t1; t1= t2; t2= a;
		}

		/* z van snijpunten met diabolo */
		p1[2]= npos[2] + t1*nray[2];
		p2[2]= npos[2] + t2*nray[2];

		/* beide punten evalueren */
		if(p1[2]<=0.0) ok1= 1;
		if(p2[2]<=0.0 && t1!=t2) ok2= 1;
		
		/* minstens 1 punt met negatieve z */
		if(ok1==0 && ok2==0) return;
		
		/* snijpunt met -ladist, de bodem van de kegel */
		if(use_yco==0) {
			t3= (-ladist-npos[2])/nray[2];
				
			/* moet 1 van de snijpunten worden vervangen? */
			if(ok1) {
				if(p1[2]<-ladist) t1= t3;
			}
			else {
				ok1= 1;
				t1= t3;
			}
			if(ok2) {
				if(p2[2]<-ladist) t2= t3;
			}
			else {
				ok2= 1;
				t2= t3;
			}
		}
		else if(ok1==0 || ok2==0) return;
		
		/* minstens 1 zichtbaar snijpunt */
		if(t1<0.0 && t2<0.0) return;
		
		if(t1<0.0) t1= 0.0;
		if(t2<0.0) t2= 0.0;
		
		if(t1==t2) return;
		
		/* voor zekerheid nog eens sorteren */
		if(t1>t2) {
			a= t1; t1= t2; t2= a;
		}
		
		/* t0 berekenen: is de maximale zichtbare z (als halo door vlak      */
		/* doorsneden wordt)                                                 */ 
		if(doclip) {
			if(use_yco==0) t0= (maxz-npos[2])/nray[2];
			else t0= (maxy-npos[1])/nray[1];

			if(t0<t1) return;
			if(t0<t2) t2= t0;
		}

		/* bereken punten */
		p1[0]= npos[0] + t1*nray[0];
		p1[1]= npos[1] + t1*nray[1];
		p1[2]= npos[2] + t1*nray[2];
		p2[0]= npos[0] + t2*nray[0];
		p2[1]= npos[1] + t2*nray[1];
		p2[2]= npos[2] + t2*nray[2];
		
			
		/* nu hebben we twee punten, hiermee maken we drie lengtes */
		
		a= fsqrt(p1[0]*p1[0]+p1[1]*p1[1]+p1[2]*p1[2]);
		b= fsqrt(p2[0]*p2[0]+p2[1]*p2[1]+p2[2]*p2[2]);
		c= VecLenf(p1, p2);
		
		a/= ladist;
		a= fsqrt(a);
		b/= ladist; 
		b= fsqrt(b);
		c/= ladist;
		
		*intens= c*( (1.0-a)+(1.0-b) );
		
		/* LET OP: a,b en c NIET op 1.0 clippen, dit geeft kleine
		   overflowtjes op de rand (vooral bij smalle halo's) */
		if(*intens<=0.0) return;
		
		/* zachte gebied */
		/* vervalt omdat t0 nu ook voor p1/p2 wordt gebruikt */
		/* if(doclip && t0<t2) { */
		/* 	*intens *= (t0-t1)/(t2-t1); */
		/* } */
		
		*intens *= haint;
		
		if(lar->shb && lar->shb->shadhalostep) {
			/* from shadbuf.c, returns float */
			*intens *= shadow_halo(lar, p1, p2);
		}
		/* if(lar->mode & LA_TEXTURE)  do_lamphalo_tex(lar, p1, p2, intens); */
		
	}
} /* end of void spotHaloFloat(struct LampRen *, float *view, float *intens) */

/* ------------------------------------------------------------------------- */

void shadeLampLusFloat()
{
	LampRen *lar;
	register Material *ma;
	float i, inp, inpr, t, lv[3], lampdist, ld = 0;
	float ir, ig, ib;
	float isr=0,isg=0,isb=0;
	float lvrot[3], *vn, *view, shadfac, soft;
	int a;

	vn= R.vn;
	view= R.view;
	ma= R.matren;
	
	/* aparte lus */
	if(ma->mode & MA_ONLYSHADOW) {
		shadfac= ir= 0.0;
		for(a=0; a<R.totlamp; a++) {
			lar= R.la[a];
			
			if(lar->mode & LA_LAYER) if((lar->lay & R.vlr->lay)==0) continue;
			
			if(lar->shb) {
				/* alleen testen binnen spotbundel */
				lv[0]= R.co[0]-lar->co[0];
				lv[1]= R.co[1]-lar->co[1];
				lv[2]= R.co[2]-lar->co[2];
				Normalise(lv);
				inpr= lv[0]*lar->vec[0]+lv[1]*lar->vec[1]+lv[2]*lar->vec[2];
				if(inpr>lar->spotsi) {
					
					inp= vn[0]*lv[0] + vn[1]*lv[1] + vn[2]*lv[2];
					
					i= testshadowbuf(lar->shb, inp);

					t= inpr - lar->spotsi;
					if(t<lar->spotbl && lar->spotbl!=0.0) {
						t/= lar->spotbl;
						t*= t;
						i= t*i+(1.0-t);
					}
					
					shadfac+= i;
					ir+= 1.0;
				}
				else {
					shadfac+= 1.0;
					ir+= 1.0;
				}
			}
		}
		if(ir>0.0) shadfac/= ir;
		ma->alpha= (R.mat->alpha)*(1.0-shadfac);

		collector[0] = 0.0;
		collector[1] = 0.0;
		collector[2] = 0.0;
		/* alpha is not set.... why?*/
		return;
	}
		
	if(ma->mode & (MA_VERTEXCOLP|MA_FACETEXTURE)) {
		ma->r= R.vcol[0];
		ma->g= R.vcol[1];
		ma->b= R.vcol[2];
	}

	/* mirror reflection colour */
	R.refcol[0]= R.refcol[1]= R.refcol[2]= R.refcol[3]= 0.0;

	if(ma->texco) {

		if(ma->texco & TEXCO_REFL) {
			RE_calc_R_ref();
		}
		
		if(ma->mode & (MA_VERTEXCOLP|MA_FACETEXTURE)) {
			R.mat->r= R.vcol[0];
			R.mat->g= R.vcol[1];
			R.mat->b= R.vcol[2];
		}

		do_material_tex();
	}
	
	if(ma->mode & MA_SHLESS) {
		if( (ma->mode & (MA_VERTEXCOL+MA_VERTEXCOLP+MA_FACETEXTURE) )) {
			ir= R.vcol[0]*ma->r;
			ig= R.vcol[1]*ma->g;
			ib= R.vcol[2]*ma->b;
		}
		else {
			ir= ma->r; /* apparently stored as [0,1]? */
			ig= ma->g;
			ib= ma->b;
		}

		/* This gamma table stuff may not be necessary? What about colour    */
		/* clipping? I may use my own gamma routines here.                   */
		if(usegamtab) { /* basically: when oversampling */
			collector[0] = gammaCorrect(collector[0]);
			collector[1] = gammaCorrect(collector[1]);
			collector[2] = gammaCorrect(collector[2]);
		}
		else {
			/* prob. always bigger than 0*/
			collector[0] = ir; /* no clipping, no alpha */
			collector[1] = ig;
			collector[2] = ib;
		}
		return;
	}

	if( (ma->mode & (MA_VERTEXCOL+MA_VERTEXCOLP))== MA_VERTEXCOL ) {
		ir= ma->emit+R.vcol[0];
		ig= ma->emit+R.vcol[1];
		ib= ma->emit+R.vcol[2];
	}
	else ir= ig= ib= ma->emit;

	for(a=0; a<R.totlamp; a++) {
		lar= R.la[a];

		/* test op lamplayer */
		if(lar->mode & LA_LAYER) if((lar->lay & R.vlr->lay)==0) continue;

		/* lampdist berekening */
		if(lar->type==LA_SUN || lar->type==LA_HEMI) {
			VECCOPY(lv, lar->vec);
			lampdist= 1.0;
		}
		else {
			lv[0]= R.co[0]-lar->co[0];
			lv[1]= R.co[1]-lar->co[1];
			lv[2]= R.co[2]-lar->co[2];
			ld= fsqrt(lv[0]*lv[0]+lv[1]*lv[1]+lv[2]*lv[2]);
			lv[0]/= ld;
			lv[1]/= ld;
			lv[2]/= ld;
			
			/* ld wordt verderop nog gebruikt (texco's) */
			
			if(lar->mode & LA_QUAD) {
				t= 1.0;
				if(lar->ld1>0.0)
					t= lar->dist/(lar->dist+lar->ld1*ld);
				if(lar->ld2>0.0)
					t*= lar->distkw/(lar->distkw+lar->ld2*ld*ld);

				lampdist= t;
			}
			else {
				lampdist= (lar->dist/(lar->dist+ld));
			}

			if(lar->mode & LA_SPHERE) {
				t= lar->dist - ld;
				if(t<0.0) continue;
				
				t/= lar->dist;
				lampdist*= (t);
			}
			
		}
		
		if(lar->mode & LA_TEXTURE)  do_lamp_tex(lar, lv);

		if(lar->type==LA_SPOT) {

			/* hier de fie Inp() vertaagt! */
			
			if(lar->mode & LA_SQUARE) {
				if(lv[0]*lar->vec[0]+lv[1]*lar->vec[1]+lv[2]*lar->vec[2]>0.0) {
					float x;
					
					/* rotate view to lampspace */
					VECCOPY(lvrot, lv);
					MTC_Mat3MulVecfl(lar->imat, lvrot);
					
					x= MAX2(fabs(lvrot[0]/lvrot[2]) , fabs(lvrot[1]/lvrot[2]));
					/* 1.0/(sqrt(1+x*x)) is equivalent to fcos(atanf(x)) */

					inpr= 1.0/(sqrt(1+x*x));
				}
				else inpr= 0.0;
			}
			else {
				inpr= lv[0]*lar->vec[0]+lv[1]*lar->vec[1]+lv[2]*lar->vec[2];
			}

			t= lar->spotsi;
			if(inpr<t) continue;
			else {
				t= inpr-t;
				i= 1.0;
				soft= 1.0;
				if(t<lar->spotbl && lar->spotbl!=0.0) {
					/* zachte gebied */
					i= t/lar->spotbl;
					t= i*i;
					soft= (3.0*t-2.0*t*i);
					inpr*= soft;
				}
				if(lar->mode & LA_ONLYSHADOW && lar->shb) {
					if(ma->mode & MA_SHADOW) {
						/* inprodukt positief: voorzijde vlak! */
						inp= vn[0]*lv[0] + vn[1]*lv[1] + vn[2]*lv[2];
						if(inp>0.0) {
					
							/* testshadowbuf==0.0 : 100% schaduw */
							shadfac= 1.0-testshadowbuf(lar->shb, inp);
							
							if(shadfac>0.0) {
								shadfac*= inp*soft*lar->energy;
								ir -= shadfac;
								ig -= shadfac;
								ib -= shadfac;
								
								continue;
							}
						}
					}
				}
				lampdist*=inpr;
			}
			if(lar->mode & LA_ONLYSHADOW) continue;

			if(lar->mode & LA_OSATEX) {
				R.osatex= 1;	/* signaal voor multitex() */
				
				O.dxlv[0]= lv[0] - (R.co[0]-lar->co[0]+O.dxco[0])/ld;
				O.dxlv[1]= lv[1] - (R.co[1]-lar->co[1]+O.dxco[1])/ld;
				O.dxlv[2]= lv[2] - (R.co[2]-lar->co[2]+O.dxco[2])/ld;

				O.dylv[0]= lv[0] - (R.co[0]-lar->co[0]+O.dyco[0])/ld;
				O.dylv[1]= lv[1] - (R.co[1]-lar->co[1]+O.dyco[1])/ld;
				O.dylv[2]= lv[2] - (R.co[2]-lar->co[2]+O.dyco[2])/ld;
			}
			
		}

		/* inprodukt en reflectivity*/
		inp=i= vn[0]*lv[0] + vn[1]*lv[1] + vn[2]*lv[2];
		if(lar->type==LA_HEMI) {
			i= 0.5*i+0.5;
		}
		if(i>0.0) {
			i*= lampdist*ma->ref;
		}

		/* schaduw en spec */
		if(i> -0.41) {			/* beetje willekeurig, beetje getest */
			shadfac= 1.0;
			if(lar->shb) {
				if(ma->mode & MA_SHADOW) {
					shadfac= testshadowbuf(lar->shb, inp);
					if(shadfac==0.0) continue;
					i*= shadfac;
				}
			}
			/* specularity */
			
			if(ma->spec!=0.0) {
				
				if(lar->type==LA_SUN || lar->type==LA_HEMI) {
					if(lar->type==LA_SUN) {
						lv[2]-= 1.0;
					}
					else {
						lv[0]+= view[0];
						lv[1]+= view[1];
						lv[2]+= view[2];
					}
					
					Normalise(lv);
					
					t= vn[0]*lv[0]+vn[1]*lv[1]+vn[2]*lv[2];
					
					if(lar->type==LA_HEMI) {
						t= 0.5*t+0.5;
					}
					/* let op: shadfac en lampdist uit onderstaande */
					
					/* no more speclim */

					t= ma->spec*RE_Spec(t, ma->har);
					isr+= t*(lar->r * ma->specr);
					isg+= t*(lar->g * ma->specg);
					isb+= t*(lar->b * ma->specb);
				}
				else {
					/* Does specular reflection? This would be the place     */
					/* to put BRDFs.                                         */
					t= shadfac*ma->spec*lampdist*CookTorr(vn, lv, view, ma->har);
					isr+= t*(lar->r * ma->specr);
					isg+= t*(lar->g * ma->specg);
					isb+= t*(lar->b * ma->specb);
				}
			}
		}
		if(i>0.0) {
			ir+= i*lar->r;
			ig+= i*lar->g;
			ib+= i*lar->b;
		}
	}

	/* clipping: maybe don't clip? */
	if(ir<0.0) ir= 0.0;
	if(ig<0.0) ig= 0.0;
	if(ib<0.0) ib= 0.0;
	if(isr<0.0) isr= 0.0;
	if(isg<0.0) isg= 0.0;
	if(isb<0.0) isb= 0.0;

	if(ma->mode & MA_ZTRA) {	/* ztra shade */
		if(ma->spectra!=0.0) {

			t = MAX3(isr, isb, isg);
			t *= ma->spectra;
			if(t>1.0) t= 1.0;
			if(ma->mapto & MAP_ALPHA) ma->alpha= (1.0-t)*ma->alpha+t;
			else ma->alpha= (1.0-t)*R.mat->alpha+t;
		}
	}
		
	if(R.refcol[0]==0.0) {
		collector[0] = (ma->r * ir) + ma->ambr + isr;
		collector[1] = (ma->g * ig) + ma->ambg + isg;
		collector[2] = (ma->b * ib) + ma->ambb + isb;
		/* clip for >0 ? */
	}
	else {
		collector[0] = (ma->mirr * R.refcol[1])
			+ ((1.0 - (ma->mirr * R.refcol[0])) * ((ma->r * ir) + ma->ambr))
			+ isr;
		collector[1] = (ma->mirg*R.refcol[2])
			+ ((1.0 - (ma->mirg * R.refcol[0])) * ((ma->g * ig) +ma->ambg))
			+isg;
		collector[2] = (ma->mirb*R.refcol[3])
			+ ((1.0 - (ma->mirb * R.refcol[0])) * ((ma->b * ib) +ma->ambb))
			+isb;
	}

	if(usegamtab) {
		collector[0] = gammaCorrect(collector[0]);
		collector[1] = gammaCorrect(collector[1]);
		collector[2] = gammaCorrect(collector[2]);
	}

}

/* ------------------------------------------------------------------------- */

void* renderHaloPixel(float x, float y, int haloNr) {
    HaloRen *har = NULL;
    float dist = 0.0;
    uint zz = 0;
    
#ifdef RE_FAKE_HALO_PIXELS
    collector[0] = RE_UNITY_COLOUR_FLOAT;
    collector[1] = 0;
    collector[2] = 0;
    collector[3] = RE_UNITY_COLOUR_FLOAT;
    har = addhalo(haloNr); /* crash prevention */
    return har;
#endif

    /* Find har to go with haloNr */
    har = addhalo(haloNr);
                    
    /* zz is a strange number... This call should effect that halo's are  */
    /* never cut? Seems a bit strange to me now...                        */
    /* This might be the zbuffer depth                                    */
    zz = calchalo_z(har, 0x7FFFFFFF);

    /* distance of this point wrt. the halo center. Maybe xcor is also needed? */
    dist = ((x - har->xs) * (x - har->xs)) 
        +  ((y - har->ys) * (y - har->ys) * R.ycor * R.ycor) ;

    collector[0] = RE_ZERO_COLOUR_FLOAT; collector[1] = RE_ZERO_COLOUR_FLOAT; 
    collector[2] = RE_ZERO_COLOUR_FLOAT; collector[3] = RE_ZERO_COLOUR_FLOAT;

    if (dist < har->radsq) {
        shadeHaloFloat(har, collector, zz, dist, 
					  (x - har->xs), (y - har->ys) * R.ycor, har->flarec);  
    }; /* else: this pixel is not rendered for this halo: no colour */

    return (void*) har;

} /* end of void* renderHaloPixel(float x, float y, int haloNr) */

/* ------------------------------------------------------------------------- */

void shadeHaloFloat(HaloRen *har, 
					float *col, uint zz, 
					float dist, float xn, 
					float yn, short flarec)
{
	/* in col invullen */
	/* migrate: fill collector */
	float t, zn, radist, ringf=0.0, linef=0.0, alpha, si, co, colf[4];
	int a;
   
	if(R.wrld.mode & WO_MIST) {
       if(har->type & HA_ONLYSKY) {
           /* sterren geen mist */
           alpha= har->alfa;
       }
       else {
           /* een beetje patch... */
           R.zcor= -har->co[2];
           alpha= mistfactor(har->co)*har->alfa;
       }
	}
	else alpha= har->alfa;
	
	if(alpha==0.0) {
		col[0] = 0.0;
		col[1] = 0.0;
		col[2] = 0.0;
		col[3] = 0.0;
		return;
	}

	radist= fsqrt(dist);

	/* let op: hiermee wordt gesjoemeld: flarec wordt op nul gezet in de pixstruct */
	if(flarec) har->pixels+= (int)(har->rad-radist);

	if(har->ringc) {
		extern float hashvectf[];
		float *rc, fac;
		int ofs;
		
		/* per ring een alicirc */
		ofs= har->seed;
		
		for(a= har->ringc; a>0; a--, ofs+=2) {
			
			rc= hashvectf + (ofs % 768);
			
			fac= fabs( rc[1]*(har->rad*fabs(rc[0]) - radist) );
			
			if(fac< 1.0) {
				ringf+= (1.0-fac);
			}
		}
	}

	if(har->type & HA_VECT) {
		dist= fabs( har->cos*(yn) - har->sin*(xn) )/har->rad;
		if(dist>1.0) dist= 1.0;
		if(har->tex) {
			zn= har->sin*xn - har->cos*yn;
			yn= har->cos*xn + har->sin*yn;
			xn= zn;
		}
	}
	else dist= dist/har->radsq;

	if(har->type & HA_FLARECIRC) {
		
		dist= 0.5+fabs(dist-0.5);
		
	}

	if(har->hard>=30) {
		dist= fsqrt(dist);
		if(har->hard>=40) {
			dist= fsin(dist*M_PI_2);
			if(har->hard>=50) {
				dist= fsqrt(dist);
			}
		}
	}
	else if(har->hard<20) dist*=dist;

	dist=(1.0-dist);
	
	if(har->linec) {
		extern float hashvectf[];
		float *rc, fac;
		int ofs;
		
		/* per starpoint een aliline */
		ofs= har->seed;
		
		for(a= har->linec; a>0; a--, ofs+=3) {
			
			rc= hashvectf + (ofs % 768);
			
			fac= fabs( (xn)*rc[0]+(yn)*rc[1]);
			
			if(fac< 1.0 ) {
				linef+= (1.0-fac);
			}
		}
		
		linef*= dist;
		
	}

	if(har->starpoints) {
		float ster, hoek;
		/* rotatie */
		hoek= fatan2(yn, xn);
		hoek*= (1.0+0.25*har->starpoints);
		
		co= fcos(hoek);
		si= fsin(hoek);
		
		hoek= (co*xn+si*yn)*(co*yn-si*xn);
		
		ster= fabs(hoek);
		if(ster>1.0) {
			ster= (har->rad)/(ster);
			
			if(ster<1.0) dist*= fsqrt(ster);
		}
	}
	
	/* halo wordt doorsneden? */
	if(har->zs> zz-har->zd) {
		t= ((float)(zz-har->zs))/(float)har->zd;
		alpha*= fsqrt(fsqrt(t));
	}

	dist*= alpha;
	ringf*= dist;
	linef*= alpha;
	
	if(dist<0.003) {
		col[0] = 0.0;
		col[1] = 0.0;
		col[2] = 0.0;
		col[3] = 0.0;
		return;
	}

	/* The colour is either the rgb spec-ed by the user, or extracted from   */
	/* the texture                                                           */
	if(har->tex) {
		colf[3]= dist;
		do_halo_tex(har, xn, yn, colf);
		colf[0]*= colf[3];
		colf[1]*= colf[3];
		colf[2]*= colf[3];
		
	}
	else {
		colf[0]= dist*har->r;
		colf[1]= dist*har->g;
		colf[2]= dist*har->b;
		if(har->type & HA_XALPHA) colf[3]= dist*dist;
		else colf[3]= dist;
	}

	if(har->mat && har->mat->mode & MA_HALO_SHADE) {
		/* we test for lights because of preview... */
		if(R.totlamp) render_lighting_halo(har, colf);
	}

	/* Next, we do the line and ring factor modifications. It seems we do    */
	/* uchar calculations, but it's basically doing float arith with a 255   */
	/* scale factor.                                                         */
	if(linef!=0.0) {
		Material *ma= har->mat;
		linef *= 255.0;
		
		colf[0]+= linef * ma->specr;
		colf[1]+= linef * ma->specg;
		colf[2]+= linef * ma->specb;
		
		if(har->type & HA_XALPHA) colf[3]+= linef*linef;
		else colf[3]+= linef;
	}
	if(ringf!=0.0) {
		Material *ma= har->mat;
		ringf *= 255.0;

		colf[0]+= ringf * ma->mirr;
		colf[1]+= ringf * ma->mirg;
		colf[2]+= ringf * ma->mirb;
		
		if(har->type & HA_XALPHA) colf[3]+= ringf*ringf;
		else colf[3]+= ringf;
	}

	/* convert to [0.0; 1.0] range */
	col[0] = colf[0] / 255.0;
	col[1] = colf[1] / 255.0;
	col[2] = colf[2] / 255.0;
	col[3] = colf[3];

} /* end of shadeHaloFloat() */

/* ------------------------------------------------------------------------- */

void renderSpotHaloPixel(float x, float y, float* colbuf)
{
	float u = 0.0, v = 0.0;
	
#ifdef RE_FAKE_SPOTHALO_PIXELS
	colbuf[0] = 0.0;
	colbuf[1] = 1.0;
	colbuf[2] = 0.0;
	colbuf[3] = 1.0;
	return;
#endif
	
	/* Strange fix? otherwise done inside shadepixel. It's sort  */
	/* of like telling this is a 'sky' pixel.                    */
	R.vlaknr = 0;
	shortcol[3] = 0;
	
	if( (G.special1 & G_HOLO) && ((Camera *)G.scene->camera->data)->flag & CAM_HOLO2) {
		R.view[0]= (x+(R.xstart)+1.0+holoofs);
	}
	else {
		R.view[0]= (x+(R.xstart)+1.0);
	}
	
	if(R.flag & R_SEC_FIELD) {
		if(R.r.mode & R_ODDFIELD) R.view[1]= (y+R.ystart+0.5)*R.ycor;
		else R.view[1]= (y+R.ystart+1.5)*R.ycor;
	}
	else R.view[1]= (y+R.ystart+1.0)*R.ycor;
	
	R.view[2]= -R.viewfac;
	
	if(R.r.mode & R_PANORAMA) {
		u= R.view[0]; v= R.view[2];
		R.view[0]= panovco*u + panovsi*v;
		R.view[2]= -panovsi*u + panovco*v;
	}
	
	R.co[2]= 0.0;

	/* has become shadeSpotHaloPixel! A numerical fix is still needed here.  */
  	renderspothaloFix(shortcol); 
  	cpShortColV2FloatColV(shortcol, colbuf); 
	
} /* end of void renderSpotHaloPixel(float x, float y, float colbuf[4]) */

/* ------------------------------------------------------------------------- */

void renderspothaloFix(ushort *col)
{
	LampRen *lar;
	float i;
	int colt, a;
	ushort scol[4];
	
	
	for(a=0; a<R.totlamp; a++) {
		lar= R.la[a];
		if(lar->type==LA_SPOT && (lar->mode & LA_HALO) && lar->haint>0) {
	
			if(lar->org) {
				lar->r= lar->org->r;
				lar->g= lar->org->g;
				lar->b= lar->org->b;
			}

			spotHaloFloat(lar, R.view, &i);
			
			if(i>0.0) {

				i*= 65536.0;
				
				colt= i;
				if(colt>65535) scol[3]= 65535; else scol[3]= colt;

				/* erg vervelend: gammagecorrigeerd renderen EN */
				/* addalphaADD gaat niet goed samen */
				/* eigenlijk moet er een aparte 'optel' gamma komen */
				
				colt= i*lar->r;
				if(colt>65535) scol[0]= 65535; else scol[0]= colt;
				if(usegamtab)  scol[0]= igamtab2[scol[0]];
				colt= i*lar->g;
				if(colt>65535) scol[1]= 65535; else scol[1]= colt;
				if(usegamtab)  scol[1]= igamtab2[scol[1]];
				colt= i*lar->b;
				if(colt>65535) scol[2]= 65535; else scol[2]= colt;
				if(usegamtab) scol[2]= igamtab2[scol[2]];
				
				addalphaAddshort(col, scol);
			} 
		} 
	} 
} 

/* ------------------------------------------------------------------------- */

/* eof */

