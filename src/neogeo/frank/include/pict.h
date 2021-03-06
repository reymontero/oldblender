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

/*

 * Copyright (C) 1991, Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef PICTDEF
#define PICTDEF

/*
 * pict.h - header file for PICT files
 *
 */
#define HEADER_SIZE		512

/* Opcodes */
#define PICT_NOP		0x00
#define PICT_clipRgn		0x01
#define PICT_bkPat		0x02
#define PICT_txFont		0x03
#define PICT_txFace		0x04
#define PICT_txMode		0x05
#define PICT_spExtra		0x06
#define PICT_pnSize		0x07
#define PICT_pnMode		0x08
#define PICT_pnPat		0x09
#define PICT_FillPat		0x0A
#define PICT_ovSize		0x0B
#define PICT_Origin		0x0C
#define PICT_txSize		0x0D
#define PICT_fgColor		0x0E
#define PICT_bkColor		0x0F
#define PICT_txRatio		0x10
#define PICT_picVersion		0x11
#define PICT_bkPixPat		0x12
#define PICT_pnPixPat		0x13
#define PICT_FillPixPat		0x14
#define PICT_pnLocHFrac		0x15
#define PICT_chExtra		0x16
#define PICT_RGBFgCol		0x1A
#define PICT_RGBBkCol		0x1B
#define PICT_HiliteMode		0x1C
#define PICT_HiliteColor	0x1D
#define PICT_DefHilite		0x1E
#define PICT_OpColor		0x1F
#define PICT_line		0x20
#define PICT_line_from		0x21
#define PICT_short_line		0x22
#define PICT_short_line_from	0x23
#define PICT_long_text		0x28
#define PICT_DH_text		0x29
#define PICT_DV_text		0x2A
#define PICT_DHDV_text		0x2B
#define PICT_frameRect		0x30
#define PICT_paintRect		0x31
#define PICT_eraseRect		0x32
#define PICT_invertRect		0x33
#define PICT_fillRect		0x34
#define PICT_frameSameRect	0x38
#define PICT_paintSameRect	0x39
#define PICT_eraseSameRect	0x3A
#define PICT_invertSameRect	0x3B
#define PICT_fillSameRect	0x3C
#define PICT_frameRRect		0x40
#define PICT_paintRRect		0x41
#define PICT_eraseRRect		0x42
#define PICT_invertRRect	0x43
#define PICT_fillRRect		0x44
#define PICT_frameSameRRect	0x48
#define PICT_paintSameRRect	0x49
#define PICT_eraseSameRRect	0x4A
#define PICT_invertSameRRect	0x4B
#define PICT_fillSameRRect	0x4C
#define PICT_apple1		0x4D
#define PICT_frameOval		0x50
#define PICT_paintOval		0x51
#define PICT_eraseOval		0x52
#define PICT_invertOval		0x53
#define PICT_fillOval		0x54
#define PICT_frameSameOval	0x58
#define PICT_paintSameOval	0x59
#define PICT_eraseSameOval	0x5A
#define PICT_invertSameOval	0x5B
#define PICT_fillSameOval	0x5C
#define PICT_frameArc		0x60
#define PICT_paintArc		0x61
#define PICT_eraseArc		0x62
#define PICT_invertArc		0x63
#define PICT_fillArc		0x64
#define PICT_frameSameArc	0x68
#define PICT_paintSameArc	0x69
#define PICT_eraseSameArc	0x6A
#define PICT_invertSameArc	0x6B
#define PICT_fillSameArc	0x6C
#define PICT_framePoly		0x70
#define PICT_paintPoly		0x71
#define PICT_erasePoly		0x72
#define PICT_invertPoly		0x73
#define PICT_fillPoly		0x74
#define PICT_frameSamePoly	0x78
#define PICT_paintSamePoly	0x79
#define PICT_eraseSamePoly	0x7A
#define PICT_invertSamePoly	0x7B
#define PICT_fillSamePoly	0x7C
#define PICT_frameRgn		0x80
#define PICT_paintRgn		0x81
#define PICT_eraseRgn		0x82
#define PICT_invertRgn		0x83
#define PICT_fillRgn		0x84
#define PICT_frameSameRgn	0x88
#define PICT_paintSameRgn	0x89
#define PICT_eraseSameRgn	0x8A
#define PICT_invertSameRgn	0x8B
#define PICT_fillSameRgn	0x8C
#define PICT_BitsRect		0x90
#define PICT_BitsRgn		0x91
#define PICT_PackBitsRect	0x98
#define PICT_PackBitsRgn	0x99
#define PICT_Pack32BitsRect	0x9A
#define PICT_shortComment	0xA0
#define PICT_longComment	0xA1
#define PICT_EndOfPicture	0xFF
#define PICT_reservedHeader	0x0C00

#define srcCopy			0
#define srcOr			1
#define srcXor			2
#define srcBic			3
#define notSrcCopy		4
#define notSrcOr		5
#define notSrcXor		6
#define notSrcBic		7
#define patCopy			8
#define patOr			9
#define patXor			10
#define patBic			11
#define notPatCopy		12
#define notPatOr		13
#define notPatXor		14
#define notPatBic		15

#define bold			0x01
#define italic			0x02
#define underline		0x04
#define outline			0x08
#define shadow			0x10
#define condense		0x20
#define	extend			0x40

typedef struct {
    int top, left, bottom, right;
} Rect;

typedef struct {
    int v,h;
} Point;

typedef struct {
    unsigned int Red, Green, Blue;
} RGBColor;

typedef struct Polygon {
    int     polySize;
    Rect    polyBBox;
    Point   *polyPoints;
} Polygon;

typedef struct Region {
    int     rgnSize;
    Rect    rgnBBox;
} Region;

typedef struct Pattern {
    float	mix;
    int		idx;
    unsigned char pat[8];
} Pattern;

typedef long	Fixed;

typedef struct PixMap {
    int		baseAddr;
    int		rowBytes;
    Rect	Bounds;
    int		version;
    int		packType;
    long	packSize;
    Fixed	hRes;
    Fixed	vRes;
    int		pixelType;
    int		pixelSize;
    int		cmpCount;
    int		cmpSize;
    int		planeBytes;
    int		pmTable;
    int		pmReserved;

    int		bitmap;		/* nonMac flag: really a bitmap */
    short	**r, **g, **b; 	/* pixels stored here */
    struct PixMap *next; 	/* handy for making lists of PixMaps */
    Rect	dstRect; 	/* Move coords */
} PixMap;

typedef struct ctEntry {
    int     val, red, blue, green;
} ctEntry;

typedef struct ColorTable {
    int     ctseed;
    int     transIndex;
    int     ctSize;
    RGBColor *ctTable;
} ColorTable;

#endif

