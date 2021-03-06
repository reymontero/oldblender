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

 * vanillaRenderPipe.c
 *
 * 28-06-2000 nzc
 *
 * Version: $Id: vanillaRenderPipe.c,v 1.15 2000/09/21 13:03:59 nzc Exp $
 *
 */

/*
  The render pipe
  ---------------

  The overall results of the render pass should end up in R.rectot. This 
  buffer already exists, and although its contents may change, its location
  may not. A lot of other routines depend on it!

*/

/* global includes */
#include "blender.h"      /* does exports.h, which does arithb_ext.h ...     */
#include <limits.h>       /* INT_MIN,MAX are used here                       */ 

/* local includes (from the render module) */
#include "render.h"       /* all kinds of stuff                              */
#include "zbuf.h"         /* for vergzvlak, zbufclip, zbufclipwire           */
#include "rendercore.h"   /* for scanlinesky                                 */
#include "edgeRender.h"   /* all edge rendering stuff                        */
#include "pixelshading.h" /* painting the pixels                             */

/* general calculus and data manipulation, also local                        */
#include "vectorops.h"
#include "matrixops.h"
#include "pixelblending.h"
#include "zbufferdatastruct.h"

/* own includes */
#include "vanillaRenderPipe.h"
#include "vanillaRenderPipe_int.h"

/* ------------------------------------------------------------------------- */
/* Debug defines: disable all for production level code.                     */
/* These variables control faking of rendered colours, extra tracing,        */
/* extra error checking and such.                                            */
/* ------------------------------------------------------------------------- */

/* if defined: _very_ explicit tracing and checking enabled                  */
/*  #define RE_FULL_SAFETY */
/* if defined: use 'simple' alpha thresholding on oversampling               */
/* #define RE_SIMPLE_ALPHA_THRESHOLD */

/* ------------------------------------------------------------------------- */

#ifdef RE_FULL_SAFETY
/* Full safety does the following:                                           */
/* - add extra bounds checking                                               */
/* - add extra type checking                                                 */
/* - do a little performance analysis                                        */
/* - trace the original sources                                              */

/* trace the version of the source */
char vanillaRenderPipe_ext_h[]   = VANILLARENDERPIPE_EXT_H;
char vanillaRenderPipe_int_h[]   = VANILLARENDERPIPE_INT_H;
char vanillaRenderPipe_types_h[] = VANILLARENDERPIPE_TYPES_H;
char vanillaRenderPipe_c[]       =
"$Id: vanillaRenderPipe.c,v 1.15 2000/09/21 13:03:59 nzc Exp $";
/* counters for error handling */
static int conflictsresolved; /* number of conflicts in one frame            */

#include "errorHandler.h"
#endif /* RE_FULL_SAFETY stuff */

/* ------------------------------------------------------------------------- */

/* External : -------------------------------------------------------------- */

extern float centLut[16];    /* Lookup for jitter offsets.                   */
extern uint  Zsample;        /* Nr. of the currently active oversample. This */
                             /* counter must be set explicitly by the        */
                             /* function that builds the z-buffer.           */
                             /* The buffer-filling functions use it.         */
extern float jit[64][2];     /* Table with jitter offsets                    */
extern float jitweight[64];  /* Table with jitter weights                    */
extern float Zjitx,Zjity;    /* The x,y values for jitter offset             */

extern float Zmulx, Zmuly;   /* Some kind of scale?                          */
extern char *centmask;       /* Set in initrender.c. Seems to give the sub-  */
                             /* pixel offsets for a given osa count.         */

extern uint Zvlnr;           /* Face rendering pointer and counter: these    */
extern VlakRen *Zvlr;        /* are used for 'caching' render results.       */

extern void (*zbuffunc)(float *, float *, float *); /* These function        */
extern void (*zbuflinefunc)(); /* pointers are used for z buffer filling.    */

extern float panophi;        /* Panorama angle (I think).                    */
extern float panovco;        /* Cosine and sine factors. Don't know exactly  */
extern float panovsi;        /* what these do.                               */

extern char cmask[256];      /* When a pixel is supersampled, we must        */
extern char *centmask;       /* compute its colour on a point _on_ the face. */
                             /* These two are used to compute an offset to   */
                             /* guarantee we use valid coordinates.          */

/* unsorted */
extern float holoofs, fmask[256];
extern ushort usegamtab, shortcol[4], 
	*mask1[9], *mask2[9], *igamtab1, *igamtab2, *gamtab;

extern RE_APixstrExt *APixbufExt;/*Zbuffer: linked list of face, halo indices*/

/* Globals : --------------------------------------------------------------- */

RE_COLBUFTYPE *AColourBuffer; /* Buffer for colours of 1 line of pixels      */
static int     Aminy;         /* y value of first line in the accu buffer    */
static int     Amaxy;         /* y value of last line in the accu buffer     */
                              /* -also used to clip when zbuffering          */

/* Buffer width refers to the size of the buffers we build. Image size is    */
/* the same as R.rectx, R.recty.                                             */
static int     imageHeight;   /* image size in pixels in y direction         */
static int     imageWidth;    /* image size in pixels in x direction         */
static int     bufferHeight;  /* image size in pixels in y direction         */
static int     bufferWidth;   /* image size in pixels in x direction         */
static int     zBufferWidth;  /* special width because zbuffer needs to be   */
                              /* wider */

static int     Azvoordeel;    /* A small offset for transparent rendering.   */
int            alphaLUT[32];  /* alpha lookuptable, for oversampling         */
                              /* Its function has been superceded because    */
                              /* pixels are always integrated. This          */
                              /* performs the same normalization.            */
int            osaNr;         /* The oversample number. I keep it            */
                              /* separately here, because I treat no OSA     */
                              /* as if it were osa=1.                        */
RE_COLBUFTYPE  collector[4];  /* used throughout as pixel colour accu        */
RE_COLBUFTYPE  sampcol[RE_MAX_OSA_COUNT * 4]; /* subpixel accu buffer        */
/*  static RE_RenderStatistics renderStats; */
/* Stats about the previous pass      */

/* ------------------------------------------------------------------------- */

void zBufShadeAdvanced()
{
    VertRen *addvert();
    int      y, keepLooping = 1;
    float xjit = 0.0, yjit = 0.0;

#ifdef RE_FULL_SAFETY
    /* reset trace */
	RE_errortrace_reset();
    conflictsresolved = 0;
    fprintf(stderr, "\n*** Activated full error trace on "
            "unified renderer using:\n\t%s\n\t%s\n\t%s\n\t%s", 
            vanillaRenderPipe_c, vanillaRenderPipe_ext_h,
            vanillaRenderPipe_int_h, vanillaRenderPipe_types_h);
#endif

    Zjitx=Zjity= -0.5; /* jitter preset: 0.5 pixel */

	/* EDGE: for edge rendering we should compute a larger buffer, but this  */
	/* may require modifications at a deeper level. For now, we just         */
	/* 'ignore' edge pixels.                                                 */
	imageHeight  = R.recty;
	imageWidth   = R.rectx;
	bufferHeight = R.recty;
	bufferWidth  = R.rectx;
   
    /* Set osaNr. Treat 'no osa' as 'osa = 1' */
    if(R.r.mode & R_OSA) {
		osaNr = R.osa;
		if(osaNr > 16) { /* check was moved from calcZBufLine */
			printf("zBufShadeAdvanced> osa too large (internal error)\n");
			G.afbreek= 1;
			return;
		}
    } else {
        /* little hack */
        osaNr = 1;
        xjit = jit[0][0];
        yjit = jit[0][1];
        jit[0][0] = 0.45;
        jit[0][1] = 0.45;        
    }

    RE_setwindowclip(0, -1); /* just to be sure, reset the view matrix       */

	initRenderBuffers(bufferWidth);
	
    y = 0;
    while ( (y < bufferHeight) && keepLooping) {
		calcZBufLine(y);
		R.vlaknr= -1; /* huh? why reset this counter? for shadePixel!        */
		renderZBufLine(y);
		transferColourBufferToOutput(y);
		scanlinesky((char*) (R.rectot + (y * imageWidth)), y); 
		if((y & 1) && G.background!=1) RE_render_display(y-1, y);		
		if(RE_test_break()) keepLooping = 0;
		y++; 
    }
	freeRenderBuffers();

	/* Edge rendering is done purely as a post-effect                        */
	if(R.r.mode & R_EDGE) {
		addEdges((char*)R.rectot, imageWidth, imageHeight,
				 osaNr, R.r.edgeint, G.compat, G.notonlysolid);
	}
	
#ifdef RE_FULL_SAFETY
	fprintf(stderr, "\n--- resolved %d conflicts", conflictsresolved);
	fflush(stderr);
#endif

	if (!(R.r.mode & R_OSA)) {
		jit[0][0] = xjit;
		jit[0][1] = yjit; 
	}
    
} /* end of void zbufshadeAdvanced() */

/* ------------------------------------------------------------------------- */

void initRenderBuffers(int bwidth) 
{

    /* The +1 is needed because the fill-functions use a +1 offset when      */
    /* filling in pixels. Mind that also the buffer-clearing function needs  */
    /* this offset (done in calcZBufLine).                                   */
	/* The offset is wrong: it shouldn't be there. I need to fix this still. */
    AColourBuffer = callocN(4 * sizeof(RE_COLBUFTYPE) * bwidth, 
                            "Acolrow");
	zBufferWidth = bwidth + 1;
	initZbuffer(bwidth + 1);

    Aminy= -1000; /* indices of lines in the z buffer: no lines buffered     */
    Amaxy= -1000;

	/* Gamma may become a slider                                             */ 
	if (!gammaTableIsInitialised()) makeGammaTables(RE_DEFAULT_GAMMA); 

} /* End of void initZBuffers(void) */

/* ------------------------------------------------------------------------- */

void freeRenderBuffers(void) {	
    if (AColourBuffer) freeN(AColourBuffer);
	freeZbuffer();
} /* End of void freeZBuffers(void) */

/* ------------------------------------------------------------------------- */

void calcZBufLine(int y)
{
    extern float jit[64][2];  /* For jittered z buffering */

    int part;
    int keepLooping = 1;

    if(y<0) return;

	/* zbuffer fix: here? */
	Zmulx= ((float) bufferWidth)/2.0;
  	Zmuly= ((float) bufferHeight)/2.0;

	
	/* use these buffer fill functions */    
	zbuffunc     = zBufferFillFace;
	zbuflinefunc = zBufferFillEdge;
	    
    /* (FORALL y: Aminy =< y =< Amaxy: y is buffered)                        */
    if( (y < Aminy) || (y > Amaxy)) {
        /* prepare buffer */
        part  = (y/RE_ZBUFLEN);     /* These two lines are mystifying me...  */
        Aminy = part * RE_ZBUFLEN;  /* Possibly for rounding things?         */
        Amaxy = Aminy + RE_ZBUFLEN - 1;
/*          if(Amaxy >= R.recty) Amaxy = R.recty-1; */
        if(Amaxy >= bufferHeight) Amaxy = bufferHeight - 1;
		resetZbuffer();
        
        Zsample = 0; /* Zsample is used internally !                         */
        while ( (Zsample < osaNr) && keepLooping ) {
            /* Apply jitter to this pixel. The jitter offsets are globals.   */
            /* They are added in zbufclip()                                  */
            /* Negative: these offsets are added to the vertex coordinates   */
            /* so it equals translating viewpoint over the positive vector.  */
            Zjitx= -jit[Zsample][0];
            Zjity= -jit[Zsample][1];

            keepLooping = fillZBufDistances();
            
            if(RE_test_break()) keepLooping = 0;
            Zsample++;
        }
    };
    
} /*End of void calcZBufLine(int y) */

/* ------------------------------------------------------------------------- */

int countAndSortPixelFaces(int zrow[RE_MAX_FACES_PER_PIXEL][RE_PIXELFIELDSIZE], 
                           RE_APixstrExt *ap)
{
#ifdef RE_FULL_SAFETY
	char fname[] = "countAndSortPixelFaces";
#endif
    int totvlak;          /* face counter                          */
    int i;                /* generic counter                       */

    totvlak= 0;
    while(ap) {
        for(i=0; i<4; i++) {
            if(ap->t[i]) {
                zrow[totvlak][0] = ap->zmin[i];
                zrow[totvlak][1] = ap->p[i];
                zrow[totvlak][2] = ap->mask[i];
                zrow[totvlak][3] = ap->t[i]; 
                zrow[totvlak][4] = ap->zmax[i];
                totvlak++;
                if(totvlak > (RE_MAX_FACES_PER_PIXEL - 1)) 
				{
                    totvlak = (RE_MAX_FACES_PER_PIXEL - 1);
#ifdef RE_FULL_SAFETY
					RE_error(RE_TOO_MANY_FACES, fname);
#endif
				}
            } else break;
        };
        ap= ap->next;
    }
    
    if(totvlak==2) { /* Sort faces ----------------------------- */
        if(zrow[0][0] < zrow[1][0]) {
            i= zrow[0][0]; zrow[0][0]= zrow[1][0]; zrow[1][0]= i;
            i= zrow[0][1]; zrow[0][1]= zrow[1][1]; zrow[1][1]= i;
            i= zrow[0][2]; zrow[0][2]= zrow[1][2]; zrow[1][2]= i;
            i= zrow[0][3]; zrow[0][3]= zrow[1][3]; zrow[1][3]= i;
            i= zrow[0][4]; zrow[0][4]= zrow[1][4]; zrow[1][4]= i;
        } /* else: two faces, and ordering is ok */                    
    } else if (totvlak != 1) qsort(zrow, totvlak, 
                                   sizeof(int)*RE_PIXELFIELDSIZE, vergzvlak);	
    return totvlak;
} /* end of int countAndSortPixelFaces(int* zrow,RE_APixstrExt *ap ) */

/* ------------------------------------------------------------------------- */
/* Oversampler v2 - check CVS for older versions                             */
/*                                                                           */
/* - multiple blend functions ?                                              */
/* - x-rays?                                                                 */
/* - volumetric stuff ?                                                      */
/* - maybe the oversampling should move to the shading part                  */
/* - putting extra buffering for oversampled edge rendering here?            */
/*                                                                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */
void oversamplePixel(int zrow[RE_MAX_FACES_PER_PIXEL][RE_PIXELFIELDSIZE],
					 int totvlak, float x, float y, int osaNr) 
{
#ifdef RE_FULL_SAFETY
    char* fname = "oversampleConflicts";
#endif
    int           i;         /* the ubiquitous counter                       */
    float         xs = 0.0;
    float         ys = 0.0;  /* coordinates for the render-spot              */
    int           save_totvlak; /* store for faces-behind-this-pixel counter */
    /* store intermediate results: zrow is also used as stack!               */
    RE_COLBUFTYPE colourStack[4 * RE_MAX_FACES_PER_PIXEL];
    int           Cstack[RE_MAX_FACES_PER_PIXEL];
    void         *datastack[RE_MAX_FACES_PER_PIXEL];
    float         alphathreshold[RE_MAX_OSA_COUNT];
    RE_COLBUFTYPE *stackptr         = NULL; /* pointer into colourstack      */
    int           Ccount            = 0;
    int           Cthresh           = 0;
    int           fullsubpixelflags = 0;
    int           inconflict        = 0;
    int           saturated         = 0;
    int           saturationthreshold = 0;
	int           covered           = 0; /* flags covered subpixels          */

    /* reset counters that haven't been reset already                        */
    for(i = 0; i < osaNr; i++) alphathreshold[i] = 0.0;
    save_totvlak        = totvlak;
    stackptr            = colourStack;
    saturationthreshold = ( (1<<osaNr) - 1);
	
    /* 1. Forward pass: all faces, high to low                               */
    while ( (!saturated || (saturated && inconflict) ) && (totvlak > 0) ) {
        totvlak--;
			
        i= centmask[ zrow[totvlak][RE_MASK] ]; /* recenter sample position - */
        xs= (float)x+centLut[i &  15];
        ys= (float)y+centLut[i >> 4];
        
        /* stack face ----------- */
		datastack[totvlak] = renderPixel(xs, ys, zrow[totvlak]);
        cpFloatColV(collector, stackptr);
        stackptr +=4;
		/* This is done so that spothalos are properly overlayed on halos    */
  		if(zrow[totvlak][RE_TYPE] & RE_POLY) covered |= zrow[totvlak][RE_MASK]; 

        /* calculate conflict parameters: ---------------------------------- */
        if( zrow[totvlak][RE_ZMIN] < Cthresh ) {
            inconflict = 1;
			/* Prevent from switching on bad data. This may be done more     */
			/* efficiently later on. It is _quite_ important.                */
			if (totvlak == save_totvlak - 1) Ccount = 0;
			else if(Ccount == 0)             Ccount = 2;
			else                             Ccount++;
            Cstack[totvlak] = Ccount;
            if (zrow[totvlak][RE_ZMAX] > Cthresh) 
				Cthresh = zrow[totvlak][RE_ZMAX]; 
#ifdef RE_FULL_SAFETY
            if (Ccount == 2) conflictsresolved++; 
#endif
        } else { 
            Cthresh         = zrow[totvlak][RE_ZMAX];
            Ccount          = 0;
            Cstack[totvlak] = 0;
            if (totvlak > 0 )
				inconflict = (zrow[totvlak-1][RE_ZMIN] < Cthresh);
			else inconflict = 0;
        }
		
		/* alpha threshold ---------------- */
		/* There are currently two ways of blending: alpha-over, and add.    */
		/* Add-blending does strange things, in the sense that alpha is      */
		/* simply added, and colour is sort of alpha-over blended. Using the */
		/* same thresholding relation seems to work ok. For less than unity  */
		/* add factor, the alpha threshold may rise faster, but currently we */
		/* do not check for this factor.                                     */
		for(i = 0; i < osaNr; i++) {
			if ( zrow[totvlak][RE_MASK] & (1<<i)) {
				alphathreshold[i] += 
					((1.0 - alphathreshold[i]) * collector[3]);
				if (alphathreshold[i] > RE_FULL_ALPHA_FLOAT) 
					fullsubpixelflags |= (1<<i);
			}
		}
		saturated = (fullsubpixelflags >= saturationthreshold);

    } /* done stacking ----------------------------------------------------- */
	
    /* 2. sample the colour stack: back to front --------------------------- */
	/*    is there a possible way to ignore alpha? this would save 25% work  */
    while (totvlak < save_totvlak ) {
        stackptr -=4; /* ! so stackptr now points to 4 valid colour floats   */
		if (Cstack[totvlak] == 0) {
			/* no conflict: sample one colour into multiple bins */
			blendOverFloatRow(zrow[totvlak][RE_TYPE], 
							  sampcol, 
							  stackptr,
							  datastack[totvlak], 
							  zrow[totvlak][RE_MASK], 
							  osaNr);
			totvlak++;
		} else {
            /* conflict: resolve, and sample per bin                         */
            /* The counter value indicates how many faces are in conflict.   */
            /* For all of these, we need to recalculate z depth for all      */
            /* samples. During this correction, totvlak points to the        */
            /* bottom of the conflict.                                       */
            
            int cbuf[RE_MAX_FACES_PER_PIXEL][4];
            /* cbuf[0] = zdepth, [1] = rel. offset into colourstack,         */
            /* [2] = blend type, [3] = index into data stack                 */
            int face;
            int layer;
            float dx, dy;

			/* Does this loop go on too long?                                */
            for(i = 0; i< osaNr; i++) { /* per bin, buffer all faces         */
                /* Positive jit here, because I translate the viewpoint.     */
                dx = jit[i][0];
                dy = jit[i][1];
                xs = (float)x + dx;
                ys = (float)y + dy;   
                /* have to add jitter shift! otherwise clipping and          */
                /* conversion give wrong result                              */
                /* RE_setwindowclip(0, -1); */ 
                /* just to be sure, reset the view matrix? */

                face = 0;  /* only counts covering faces ------------------- */
                layer = 0; /* counts all faces ----------------------------- */

                while (layer < Cstack[totvlak] ) {
                    if ( (1<<i) & zrow[totvlak + layer][RE_MASK] )  {
                        cbuf[face][0] = 
                            calcDepth(xs, ys, 
                                      datastack[totvlak + layer],
                                      zrow[totvlak + layer][RE_TYPE]);
#ifdef RE_FULL_SAFETY
                        if ( (cbuf[face][0] > zrow[totvlak+layer][RE_ZMAX])
                             || (cbuf[face][0] < zrow[totvlak+layer][RE_ZMIN]) )
                            RE_error(RE_DEPTH_MISMATCH, fname);
                        if (datastack[totvlak+layer] == NULL) 
                            RE_error(RE_BAD_DATA_POINTER, fname);
#endif
                        cbuf[face][1] = layer;
                        cbuf[face][2] = zrow[totvlak + layer][RE_TYPE];
                        cbuf[face][3] = totvlak + layer;
                        face++;
                    }
                    layer++;
                }
                
                /* Sort. Low z = high index */
                qsort(cbuf, face, sizeof(int)*4, vergzvlak); 
                                
                /* blend. low index = high z, so start from the bottom */
                for(layer = 0; layer < face; layer++) {
                    blendOverFloat(cbuf[layer][2],    /* type */
                                   sampcol + (4 * i), /* dest */
                                   stackptr - (4 * cbuf[layer][1]),
                                   datastack[cbuf[layer][3]]);
                }
            } /* end bin loop */
            
            /* Phew! Now correct all stack pointers and resume. */
            stackptr -= (4 * (Cstack[totvlak] - 1));
            totvlak += Cstack[totvlak];
        }
    }

	/* Done sampling. Now we still need to fill in pixels that were not      */
	/* covered at all It seems strange that we have to check for empty alpha */
	/* but somehow this is necessary. Check out the cover condition :)....   */
	if((R.flag & R_LAMPHALO) /*  && !(covered & ((1 << (osaNr+1))-1)) */) {
		float halocol[4];
		renderSpotHaloPixel(x, y, halocol);
		/* test seems to be wrong? */
		if (halocol[3] > RE_EMPTY_COLOUR_FLOAT) {
			for (i = 0; i < osaNr; i++) {
				/* here's a pinch: if the pixel was only covered by a halo,  */
				/* we still need to fill spothalo. How do we detect this?    */
  				if (!(covered & (1 << i)))
					addAlphaOverFloat(sampcol + (4 * i), halocol);
			}
		}
	}
	
} /* end of void oversamplePixel(..., ...) */

/* ------------------------------------------------------------------------- */
/* Rendering: per line                                                       */
/*                                                                           */
/* For each pixel in this line, we render as follows:                        */
/* a. Count the number of objects buffered for this pixel, and sort on z     */
/* ------- Result is left in zrow                                            */
/* b. Shade the pixel:                                                       */
/*  1. From front to back: calculate the colour for this object              */
/*  2. Blend this colour in with the already calculated colour               */
/*    Repeat 1. and 2. until no faces remain.                                */
/*  For each pixel, a face is only rendered once, even if it is              */
/*  jittered. All subpixels get the colour of the weighted centre            */
/*  of the jitter-positions this face covers.                                */
/* ------- Result is left in sampcol[]                                       */
/* c. Copy the result to the colour buffer                                   */
/*                                                                           */
/* zrow may need some clarification:                                         */
/* 0 - min. distance                                                         */
/* 1 - face/halo index                                                       */
/* 2 - masks                                                                 */
/* 3 - type RE_POLY or RE_HALO                                               */
/* 4 - max. distance                                                         */
/* It is used to store copies of RE_APixstrExt records. These are sorted for */
/* distance, and then used for rendering pixels. zrow might be replaced by   */
/* an RE_APixstrExt* array                                                   */
/* - redo the numbering to something more logical                            */
void renderZBufLine(int y) {
    int  zrow[RE_MAX_FACES_PER_PIXEL][RE_PIXELFIELDSIZE];
    RE_APixstrExt *ap;       /* iterator for the face-lists        */
    int x;                   /* pixel counter                      */
    int totvlak;             /* faces-behind-this-pixel counter    */
    RE_COLBUFTYPE *colbuf;   /* pointer into the line buffer       */
    RE_COLBUFTYPE *j = NULL; /* generic pixel pointer              */
    int i;                   /* yet another counter                */
	
    extern ushort usegamtab; /* this variable is pushed temporarily*/
    int tempgam;

    /* Prepare buffers and iterators */
    colbuf    = AColourBuffer;
    eraseColBuf(AColourBuffer);
    ap        = APixbufExt + (zBufferWidth * (y - Aminy));

    tempgam   = usegamtab; /* is this used at all? */
    usegamtab = 0;

    /* Rendering: give the right colour to this pixel (shade it) */
	for( x = 0; x < bufferWidth; x++, ap++, colbuf+=4) {
		if(ap->t[0]) {
            /* reset sample collector */
            j = sampcol;
            for(i = 0; i < osaNr; i++, j+=4) { 
                j[0] = RE_ZERO_COLOUR_FLOAT; j[1] = RE_ZERO_COLOUR_FLOAT; 
                j[2] = RE_ZERO_COLOUR_FLOAT; j[3] = RE_ZERO_COLOUR_FLOAT;
            };

            /* a.1. count and sort number of faces */
            totvlak = countAndSortPixelFaces(zrow, ap);

			/* a.2. EDGE: insert min. distance in the extra buffer */
			/* Only works for non-oversampled pictures.            */
/*  			if(R.r.mode & R_EDGE) { */
				/* find the first object matching our criteria.*/
/*  				int found = 0; */
/*  				i = totvlak - 1;  */
/*  				while ( (i >= 0) && (!found) ) { */
					/* our criteria are simple for now */
/*  					if (zrow[i][RE_TYPE] == RE_POLY) { */
/*  						insertInEdgeBuffer(x, y, zrow[i][RE_ZMIN]); */
/*  						found = 1; */
/*  					} */
/*  					i--; */
/*  				} */
/*  				if (!found) insertInEdgeBuffer(x, y, 0x7FFFFFFF); */
/*  			} */
			
            /* b,c. oversample all subpixels, then integrate                 */
			oversamplePixel(zrow, totvlak, x, y, osaNr);
			sampleFloatColV2FloatColV(sampcol, colbuf, osaNr);
		} else {
			/* EDGE: insert max zdist in the buffer */
/*  			if(R.r.mode & R_EDGE) insertInEdgeBuffer(x, y, 0x7FFFFFFF); */
			
			/* Also sky-pixels should be filled in here for good measure.    */
			/* However, doing sky as scanline is more efficient, so we don't.*/
			/* Spothalos are part of the normal pixelshader, so for covered  */
			/* pixels they are handled ok.                                   */
			if(R.flag & R_LAMPHALO) renderSpotHaloPixel(x, y, colbuf);
		}
    } /* End of pixel loop */
    
    usegamtab= tempgam;
    
} /* End of void renderZBufLine(int y) */


/* ------------------------------------------------------------------------- */

int fillZBufDistances() 
{
    int keepLooping = 1;

    keepLooping = zBufferAllFaces(); /* Solid and transparent faces*/
    keepLooping = zBufferAllHalos() && keepLooping; /* ...and halos*/
    return keepLooping;

} /* End of void fillZBufDistances() */

/* ------------------------------------------------------------------------- */
/* Transparent faces and the 'Azvoordeel'                                    */
/* A transparent face can get a z-offset, which is a                         */
/* way of pretending the face is a bit closer than it                        */
/* actually is. This is used in animations, when faces                       */
/* that are used to glue on animated characters, items,                      */
/* et. need their shadows to be drawn on top of the                          */
/* objects they stand on. The Azvoordeel is added to                         */
/* the calculated z-coordinate in the buffer-fill                            */
/* procedures.                                                               */
int zBufferAllFaces(void) 
{
    int keepLooping = 1; 
    int faceCounter; /* counter for face number */
    float vec[3], hoco[4], mul, zval, fval; 
    Material *ma=0;
    
    faceCounter = 0;
    
    while ( (faceCounter < R.totvlak) && keepLooping) {
        if((faceCounter & 255)==0) { Zvlr= R.blovl[faceCounter>>8]; }
        else Zvlr++;
        
        ma= Zvlr->mat;
        
        /* VERY dangerous construction... zoffs is set by a slide in the ui */
        /* so it should be safe...                                          */
        if((ma->mode & (MA_ZTRA)) && (ma->zoffs != 0.0)) {
            mul= 0x7FFFFFFF;
            zval= mul*(1.0+Zvlr->v1->ho[2]/Zvlr->v1->ho[3]);
            
            VECCOPY(vec, Zvlr->v1->co);
            /* z is negatief, wordt anders geclipt */ 
            vec[2]-= ma->zoffs;
            RE_projectverto(vec, hoco); /* vec onto hoco */
            fval= mul*(1.0+hoco[2]/hoco[3]);
            
            Azvoordeel= (int) fabs(zval - fval );
        } else {
            Azvoordeel= 0;
        } 
        /* face number is used in the fill functions */
        Zvlnr = faceCounter + 1;
        
        if(Zvlr->flag & R_VISIBLE) { /* might test for this sooner...  */
      
            if(ma->mode & (MA_WIRE)) zbufclipwire(Zvlr);
            else {
                zbufclip(Zvlr->v1->ho,   Zvlr->v2->ho,   Zvlr->v3->ho, 
                         Zvlr->v1->clip, Zvlr->v2->clip, Zvlr->v3->clip);
                if(Zvlr->v4) {
                    Zvlnr+= 0x800000; /* in a sense, the 'adjoint' face */
                    zbufclip(Zvlr->v1->ho,   Zvlr->v3->ho,   Zvlr->v4->ho, 
                             Zvlr->v1->clip, Zvlr->v3->clip, Zvlr->v4->clip);
                }
            }
        }
        if(RE_test_break()) keepLooping = 0; 
        faceCounter++;
    }
    return keepLooping;
} /* End of int zBufferAllFaces(void) */

/* ------------------------------------------------------------------------- */
/* We cheat a little here: we only fill the halo on the first pass, and we   */
/* set a full complement of mask flags. This can be done because we consider */
/* halos to be flat (billboards), so we do not have to correct the z range   */
/* every time we insert a halo. Also, halos fall off to zero at the edges,   */
/* so we can safely render them in pixels where they do not exist.           */
int zBufferAllHalos(void)
{
    HaloRen *har = NULL;
    unsigned int haloCounter = 0;
    int dist = 0;
    int keepLooping = 1;
    short miny = 0, maxy = 0, minx = 0, maxx = 0;
    short ycount = 0, xcount = 0;
    RE_APixstrExt *ap, *apoffset;
    int mask; /* jitter mask */

	if (!Zsample) 
	{
		/* old version: mask was just a single bit. Now, we fill all bits */
		/* mask = (1 << Zsample); */
		mask = (1 << osaNr) - 1 ; /*  0xFFFF:  16 bits are needed, one F is 4 bits */
    
		while ( (haloCounter < R.tothalo) && keepLooping) {
			if((haloCounter & 255)==0) har= R.bloha[haloCounter>>8];
			else har++;
			/* Only buffer the current alpha buffer contents!!! The line       */
			/* indices have already been clipped to picture size.              */ 
			minx = ffloor(har->xs - har->rad); /* assume min =< max is true    */
			if (minx < 0 ) minx = 0;
			maxx = fceil(har->xs + har->rad);
			/* Do the extra -1 because of the +1 later on. I guess halos might */
			/* have to start one pixel sooner? Or maybe the lower clip should  */
			/* be adjusted                                                     */
			if (maxx >= zBufferWidth - 1) maxx = zBufferWidth - 2;
			miny = har->miny;
			if (miny < Aminy) miny = Aminy;  /* can be more efficient ! --\/   */
			maxy = har->maxy;
			if (maxy > Amaxy) maxy = Amaxy;
			
			if ( (minx <= maxx) && (miny <= maxy)) {            
				/* distance to this halo? */
				dist = har->zBufDist/*   * R.ycor */;
				/* strange that the ycor influences the z coordinate ..*/
				
				ycount = miny;
				while (ycount <= maxy) {
					apoffset = APixbufExt + (zBufferWidth * (ycount - Aminy));  /* ... here! */
					/* if we do edge rendering, this must be shifted, I think */
					ap = apoffset + (minx + 1); /* this may be the culprit .... */
					xcount = minx;
					while (xcount <= maxx) {    
  						insertFlatObjectNoOsa(ap, haloCounter, RE_HALO, dist, mask);
						xcount++;
						ap++;
					}
					ycount++;
				}
			}
			if(RE_test_break()) keepLooping = 0;
			haloCounter++;
		}
	} 

    return keepLooping;
} /* end of int zbufferAllHalos(void) */

/* ------------------------------------------------------------------------- */
void zBufferFillHalo(void)
{
    /* so far, intentionally empty */
} /* end of void zBufferFillHalo(void) */

/* ------------------------------------------------------------------------- */
void zBufferFillFace(float *v1, float *v2, float *v3)  
{
	/* Coordinates of the vertices are specified in ZCS */
	 
	RE_APixstrExt *ap, *apofs;
	double z0; /* used as temp var*/
	double xx1;
	double zxd,zyd,zy0, tmp;
	float *minv,*maxv,*midv;
	register int zverg,zvlak,x;
	int my0,my2,sn1,sn2,rectx,zd;
	int y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2, mask;

	/* These used to be doubles.  We may want to change them back if the     */
	/* loss of accuracy proves to be a problem? There does not seem to be    */
	/* any performance issues here, so I'll just keep the doubles.           */
	/*  	float vec0[3], vec1[3], vec2[3]; */
	double vec0[3], vec1[3], vec2[3];

	/* MIN MAX */
	/* sort vertices for min mid max y value */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1])      { minv=v1; midv=v2; maxv=v3;}
		else if(v1[1]<v3[1]) { minv=v1; midv=v3; maxv=v2;}
		else	             { minv=v3; midv=v1; maxv=v2;}
	}
	else {
		if(v1[1]<v3[1]) 	 { minv=v2; midv=v1; maxv=v3;}
		else if(v2[1]<v3[1]) { minv=v2; midv=v3; maxv=v1;}
		else	             { minv=v3; midv=v2; maxv=v1;}
	}

	if(minv[1] == maxv[1]) return;	/* beveiliging 'nul' grote vlakken */

	my0  = fceil(minv[1]);
	my2  = ffloor(maxv[1]);
	omsl = ffloor(midv[1]);

	/* outside the current z buffer slice: clip whole face */
	if( (my2 < Aminy) || (my0 > Amaxy)) return;

	if(my0<Aminy) my0= Aminy;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1>2.0/65536.0) {
		z0= (maxv[0]-minv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx0= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(my2-minv[1])+minv[0]);
		xs0= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1>2.0/65536.0) {
		z0= (maxv[0]-midv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx1= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(my2-midv[1])+midv[0]);
		xs1= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1>2.0/65536.0) {
		z0= (midv[0]-minv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx2= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(omsl-minv[1])+minv[0]);
		xs2= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	/* xyz_1 = v_1 - v_2 */
	MTC_diff3DFF(vec1, v1, v2);
	/* xyz_2 = v_2 - v_3 */
	MTC_diff3DFF(vec2, v2, v3);
	/* xyz_0 = xyz_1 cross xyz_2 */
	MTC_cross3Double(vec0, vec1, vec2);

	/* cross product of two of the sides is 0 => this face is too small */
	if(vec0[2]==0.0) return;

	if(midv[1] == maxv[1]) omsl= my2;
	if(omsl < Aminy) omsl= Aminy-1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 > Amaxy) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (vec0[0]*v1[0]+vec0[1]*v1[1])/vec0[2]+v1[2];

	zxd= -vec0[0]/vec0[2];
	zyd= -vec0[1]/vec0[2];
	zy0= my2*zyd+xx1;
	zd= (int)CLAMPIS(zxd, INT_MIN, INT_MAX);

	/* start-ofset in rect */
	/*    	rectx= R.rectx;  */
	/* I suspect this var needs very careful setting... When edge rendering  */
	/* is on, this is strange */
  	rectx = zBufferWidth;
	apofs= (APixbufExt + rectx*(my2-Aminy));

	mask= 1<<Zsample;
	zvlak= Zvlnr;

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		MTC_swapInt(&xs0, &xs1);
		MTC_swapInt(&dx0, &dx1);
		xs3= 1;	/* flag */

	}

	for(y=my2;y>omsl;y--) {

		sn1= xs0>>16;
		xs0+= dx0;

		sn2= xs1>>16;
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (int) CLAMPIS((sn1*zxd+zy0), INT_MIN, INT_MAX);
		ap= apofs+sn1;
		
		x= sn2-sn1;
		
		zverg-= Azvoordeel;
		
		while(x>=0) {
			insertObject(ap, Zvlnr, RE_POLY, zverg, mask);
			zverg+= zd;
			ap++;
			x--;
		}
		zy0-= zyd;
		apofs-= rectx;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(; y>=my0; y--) {

		sn1= xs0>>16;
		xs0+= dx0;

		sn2= xs2>>16;
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (int) CLAMPIS((sn1*zxd+zy0), INT_MIN, INT_MAX);
		ap= apofs+sn1;

		x= sn2-sn1;
      
		zverg-= Azvoordeel;
      
		while(x>=0) {
			insertObject(ap, Zvlnr, RE_POLY, zverg, mask);
			zverg+= zd;
			ap++; 
			x--;
		}
		
		zy0-=zyd;
		apofs-= rectx;
	}
} /* end of void zBufferFillFace(float *v1, float *v2, float *v3) */

/* ------------------------------------------------------------------------- */

void zBufferFillEdge(float *vec1, float *vec2)
{
	RE_APixstrExt *ap;
	int start, end, x, y, oldx, oldy, ofs;
	int dz, vergz, mask;
	float dx, dy;
	float v1[3], v2[3];
	
	dx= vec2[0]-vec1[0];
	dy= vec2[1]-vec1[1];
	
	if(fabs(dx) > fabs(dy)) {

		/* alle lijnen van links naar rechts */
		if(vec1[0]<vec2[0]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[0]);
		end= start+ffloor(dx);
		if(end >= zBufferWidth) end = zBufferWidth - 1;
		
		oldy= ffloor(v1[1]);
		dy/= dx;
		
		vergz= v1[2];
		vergz-= Azvoordeel;
		dz= (v2[2]-v1[2])/dx;
		
		ap    = (APixbufExt+ zBufferWidth*(oldy-Aminy) +start);
		mask  = 1<<Zsample;	
		
		if(dy<0) ofs= -zBufferWidth;
		else ofs= zBufferWidth;
		
		for(x= start; x<=end; x++, ap++) {
			
			y= ffloor(v1[1]);
			if(y!=oldy) {
				oldy= y;
				ap+= ofs;
			}
			
			if(x>=0 && y>=Aminy && y<=Amaxy) {
				insertObject(ap, Zvlnr, RE_POLY, vergz, mask);
			}
			
			v1[1]+= dy;
			vergz+= dz;
		}
	}
	else {
	
		/* alle lijnen van onder naar boven */
		if(vec1[1]<vec2[1]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[1]);
		end= start+ffloor(dy);
		
		if(start>Amaxy || end<Aminy) return;
		
		if(end>Amaxy) end= Amaxy;
		
		oldx= ffloor(v1[0]);
		dx/= dy;
		
		vergz= v1[2];
		vergz-= Azvoordeel;
		dz= (v2[2]-v1[2])/dy;

		ap= (APixbufExt+ zBufferWidth*(start-Aminy) +oldx);
		mask= 1<<Zsample;
				
		if(dx<0) ofs= -1;
		else ofs= 1;

		for(y= start; y<=end; y++, ap += zBufferWidth) {
			
			x= ffloor(v1[0]);
			if(x!=oldx) {
				oldx= x;
				ap+= ofs;
			}
			
			if(x>=0 && y>=Aminy && (x < zBufferWidth)) {
				insertObject(ap, Zvlnr, RE_POLY, vergz, mask);
			}
			
			v1[0]+= dx;
			vergz+= dz;
		}
	}
} /* End of void zBufferFillEdge(float *vec1, float *vec2) */

/* ------------------------------------------------------------------------- */
/* Colour buffer related:                                                    */
/* The colour buffer is a buffer of a single screen line. It contains        */
/* four fields of type RE_COLBUFTYPE per pixel.                              */
/* ------------------------------------------------------------------------- */
void transferColourBufferToOutput(int y)
{
    /* Copy the contents of AColourBuffer to R.rectot + y * R.rectx */
    int x = 0;
    RE_COLBUFTYPE *buf = AColourBuffer;
    uchar *target = (uchar*) (R.rectot + (y * imageWidth));

#ifdef RE_FULL_SAFETY
	/* since the R.rectot always has size imageWidth * imageHeight, this     */
	/* check is possible. I may want to check this per assignment later on.  */
	if ( (y < 0) || ((y > (imageHeight - 1) ))) {
		char fname[] = "transferColourBufferToOutput";
		RE_error_int(RE_WRITE_OUTSIDE_COLOUR_BUFFER, fname, y);
		return;
	}
#endif
					
	/* Copy the first <imageWidth> pixels. We can do some more clipping on    */
	/* the z buffer, I think.                                                 */
	while (x < imageWidth) {
		cpFloatColV2CharColV(buf, target);
        target+=4;
        buf+=4;
        x++;
    }
} /* end of void transferColourBufferToOutput(int y) */

/* ------------------------------------------------------------------------- */

void eraseColBuf(RE_COLBUFTYPE *buf) {
    /* By definition, the buffer's length is 4 * R.rectx items */
    int i = 0;
/*      while (i < 4 * R.rectx) { */
    while (i < 4 * bufferWidth) {
        *buf = RE_ZERO_COLOUR_FLOAT;
        buf++; i++;
    }
} /* End of void eraseColBuf(RE_COLBUFTYPE *buf) */

/* ------------------------------------------------------------------------- */

int calcDepth(float x, float y, void* data, int type)
{
#ifdef RE_FULL_SAFETY
    char fname[] = "calcDepth";
    if (data == NULL) {
        RE_error(RE_BAD_DATA_POINTER, fname);
        return 0; 
    }
#endif
    
    if (type & RE_POLY) {
        VlakRen* vlr = (VlakRen*) data;
        VertRen* v1;
        float dvlak, deler, fac, hoco_z, hoco_w;
        int zbuf_co;
        
        v1 = vlr->v1;
        
        /* vertex dot face normal: WCS */
        dvlak= v1->co[0]*vlr->n[0]+v1->co[1]*vlr->n[1]+v1->co[2]*vlr->n[2]; 
        
        /* jitter has been added to x, y ! */
        /* view vector R.view: screen coords */
        if( (G.special1 & G_HOLO) && 
            ((Camera *)G.scene->camera->data)->flag & CAM_HOLO2) {
            R.view[0]= (x+(R.xstart) + 0.5  +holoofs);
        } else R.view[0]= (x+(R.xstart) + 0.5  );
        
        if(R.flag & R_SEC_FIELD) {
            if(R.r.mode & R_ODDFIELD) R.view[1]= (y + R.ystart)*R.ycor;
            else R.view[1]= (y+R.ystart + 1.0)*R.ycor;
        } else R.view[1]= (y+R.ystart  + 0.5 )*R.ycor;
        

		/* for pano, another rotation in the xz plane is needed.... */

        /* this is ok, in WCS */
        R.view[2]= -R.viewfac;  /* distance to viewplane */
        
        /* face normal dot view vector: but how can this work? */
		deler = MTC_dot3Float(vlr->n, R.view);
        if (deler!=0.0) fac = dvlak/deler;
        else fac = 0.0;
        
        /* indices are wrong.... but gives almost the right value? */
        hoco_z =  (fac*R.view[2]) * R.winmat[2][2] + R.winmat[3][2]; 
        hoco_w =  (fac*R.view[2]) * R.winmat[2][3] + R.winmat[3][3]; 
        
        zbuf_co = 0x7FFFFFFF*(hoco_z/hoco_w);            
        
        return  zbuf_co; /* z component of R.co */
    } else if (type & RE_HALO) {
        HaloRen* har = (HaloRen*) data;
        return har->zBufDist;
    }
#ifdef RE_FULL_SAFETY
    else RE_error(RE_BAD_FACE_TYPE, fname);
#endif /* RE_FULL_SAFETY */
    return 0;
} /* end of int calcDepth(float x, float y, void* data, int type) */

/* Maybe these two should be in pixelblendeing.c---------------------------- */

void blendOverFloat(int type, float* dest, float* source, void* data)
{
#ifdef RE_FULL_SAFETY
    char fname[] = "blendOverFloat";
    if (data == NULL){
        RE_error(RE_BAD_DATA_POINTER, fname);
        return; 
    }
#endif

    if (type & RE_POLY) {
        VlakRen *ver = (VlakRen*) data;
        if ((ver->mat != NULL) && (ver->mat->add > RE_FACE_ADD_THRESHOLD)) {
            char addf = (char) (ver->mat->add * 255.0);
            addalphaAddfacFloat(dest, source, addf);
        }
        else
            addAlphaOverFloat(dest, source);
    } else if (type & RE_HALO) {
        HaloRen *har= (HaloRen*) data;
        addalphaAddfacFloat(dest, source, har->add);
    }

#ifdef RE_FULL_SAFETY
    else  RE_error(RE_BAD_FACE_TYPE, fname);
#endif

} /* end of void blendOverFloat(int , float*, float*, void*) */

/* ------------------------------------------------------------------------- */
void blendOverFloatRow(int type, float* dest, float* source, 
                       void* data, int mask, int osaNr) 
{
#ifdef RE_FULL_SAFETY
    char* fname = "blendOverFloatRow";
    if (data == NULL) {
        RE_error(RE_BAD_DATA_POINTER, fname);
        return; 
    }
#endif
    
    if (type & RE_POLY) {
        VlakRen *ver = (VlakRen*) data;
        if ((ver->mat != NULL) 
            && (ver->mat->add > RE_FACE_ADD_THRESHOLD)) {
            char addf = (ver->mat->add * 255.0);
            addAddSampColF(dest, source, mask, osaNr, addf);
        } else {
            addOverSampColF(dest, source, mask, osaNr);
        }
    } else if (type & RE_HALO) {
        HaloRen *har = (HaloRen*) data;
        addAddSampColF(dest, source, mask, osaNr, har->add);
    }
#ifdef RE_FULL_SAFETY
    else  RE_error(RE_BAD_FACE_TYPE, fname);
#endif
} /* end of void blendOverFloatRow(int, float*, float*, void*) */

/* ------------------------------------------------------------------------- */

/* eof vanillaRenderPipe.c */

