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
/*
 *	frompict -
 *		Convert a mac PICT file into an IRIS image file.
 *
 *				Paul Haeberli - 1990
 *
 */
#include <stdio.h>
#include <local/pict.h>
#include <local/iff.h>

typedef struct opent {
	int opcode;
	char *name;
	int nbytes;
} opent;

#define CANTSKIP	(-1)
#define NOTFOUND	(-2)
#define TABSIZE		(sizeof(optab)/sizeof(opent))

opent optab[] = {
	PICT_NOP, "NOP", 0, 			/* we can skip these ok */
	PICT_shortComment, "shortComment", 2, 	
	    PICT_clipRgn, "clipRgn", 10,
	    PICT_bkPat, "bkPat", 9,
	    PICT_txFace, "txFace", 1,
	    PICT_pnSize, "pnSize", 4,
	    PICT_pnPat, "pnPat", 8,
	    PICT_FillPat, "FillPat", 8,
	    PICT_Origin, "Origin", 4,
	    PICT_frameRect, "frameRect", 8,
	    PICT_paintRect, "paintRect", 8,
	    PICT_eraseRect, "eraseRect", 8,
	    PICT_invertRect, "invertRect", 8,
	    PICT_fillRect, "fillRect", 8,
	    PICT_frameSameRect, "frameSameRect", 0,
	    PICT_paintSameRect, "paintSameRect", 0,
	    PICT_eraseSameRect, "eraseSameRect", 0,
	    PICT_invertSameRect, "invertSameRect", 0,
	    PICT_fillSameRect, "fillSameRect", 0,
	    PICT_apple1, "Apple reserved 1", 0,
	    PICT_frameRRect, "frameRRect", 8,
	    PICT_paintRRect, "paintRRect", 8,
	    PICT_eraseRRect, "eraseRRect", 8,
	    PICT_invertRRect, "invertRRect", 8,
	    PICT_fillRRect, "fillRRect", 8,
	    PICT_frameSameRRect, "frameSameRRect", 0,
	    PICT_paintSameRRect, "paintSameRRect", 0,
	    PICT_eraseSameRRect, "eraseSameRRect", 0,
	    PICT_invertSameRRect, "invertSameRRect", 0,
	    PICT_fillSameRRect, "fillSameRRect", 0,
	    PICT_frameRgn, "frameRgn", 10,
	    PICT_paintRgn, "paintRgn", 10,
	    PICT_eraseRgn, "eraseRgn", 10,
	    PICT_invertRgn, "invertRgn", 10,
	    PICT_fillRgn, "fillRgn", 10,
	    PICT_frameSameRgn, "frameSameRgn", 0,
	    PICT_paintSameRgn, "paintSameRgn", 0,
	    PICT_eraseSameRgn, "eraseSameRgn", 0,
	    PICT_invertSameRgn, "invertSameRgn", 0,
	    PICT_fillSameRgn, "fillSameRgn", 0,
	    PICT_DefHilite, "DefHilite", 0,

	    PICT_txFont, "txFont", CANTSKIP,	/* we don't knowhow big these are */
	PICT_txMode, "txMode", CANTSKIP,
	    PICT_spExtra, "spExtra", CANTSKIP,
	    PICT_pnMode, "pnMode", CANTSKIP,
	    PICT_ovSize, "ovSize", CANTSKIP,
	    PICT_txSize, "txSize", CANTSKIP,
	    PICT_fgColor, "fgColor", CANTSKIP,
	    PICT_bkColor, "bkColor", CANTSKIP,
	    PICT_txRatio, "txRatio", CANTSKIP,
	    PICT_line, "line", CANTSKIP,
	    PICT_line_from, "line_from", CANTSKIP,
	    PICT_short_line, "short_line", CANTSKIP,
	    PICT_short_line_from, "short_line_from", CANTSKIP,
	    PICT_long_text, "long_text", CANTSKIP,
	    PICT_DH_text, "DH_text", CANTSKIP,
	    PICT_DV_text, "DV_text", CANTSKIP,
	    PICT_DHDV_text, "DHDV_text", CANTSKIP,
	    PICT_frameOval, "frameOval", CANTSKIP,
	    PICT_paintOval, "paintOval", CANTSKIP,
	    PICT_eraseOval, "eraseOval", CANTSKIP,
	    PICT_invertOval, "invertOval", CANTSKIP,
	    PICT_fillOval, "fillOval", CANTSKIP,
	    PICT_frameSameOval, "frameSameOval", CANTSKIP,
	    PICT_paintSameOval, "paintSameOval", CANTSKIP,
	    PICT_eraseSameOval, "eraseSameOval", CANTSKIP,
	    PICT_invertSameOval, "invertSameOval", CANTSKIP,
	    PICT_fillSameOval, "fillSameOval", CANTSKIP,
	    PICT_frameArc, "frameArc", CANTSKIP,
	    PICT_paintArc, "paintArc", CANTSKIP,
	    PICT_eraseArc, "eraseArc", CANTSKIP,
	    PICT_invertArc, "invertArc", CANTSKIP,
	    PICT_fillArc, "fillArc", CANTSKIP,
	    PICT_frameSameArc, "frameSameArc", CANTSKIP,
	    PICT_paintSameArc, "paintSameArc", CANTSKIP,
	    PICT_eraseSameArc, "eraseSameArc", CANTSKIP,
	    PICT_invertSameArc, "invertSameArc", CANTSKIP,
	    PICT_fillSameArc, "fillSameArc", CANTSKIP,
	    PICT_framePoly, "framePoly", CANTSKIP,
	    PICT_paintPoly, "paintPoly", CANTSKIP,
	    PICT_erasePoly, "erasePoly", CANTSKIP,
	    PICT_invertPoly, "invertPoly", CANTSKIP,
	    PICT_fillPoly, "fillPoly", CANTSKIP,
	    PICT_frameSamePoly, "frameSamePoly", CANTSKIP,
	    PICT_paintSamePoly, "paintSamePoly", CANTSKIP,
	    PICT_eraseSamePoly, "eraseSamePoly", CANTSKIP,
	    PICT_invertSamePoly, "invertSamePoly", CANTSKIP,
	    PICT_fillSamePoly, "fillSamePoly", CANTSKIP,
};

ctos(cptr,sptr,n)
register unsigned char *cptr;
register unsigned short *sptr;
register int n;
{
	while(n--) {
		if(n>=8) {
			sptr[0] = cptr[0];
			sptr[1] = cptr[1];
			sptr[2] = cptr[2];
			sptr[3] = cptr[3];
			sptr[4] = cptr[4];
			sptr[5] = cptr[5];
			sptr[6] = cptr[6];
			sptr[7] = cptr[7];
			sptr+=8;
			cptr+=8;
			n -= 7;
		} else {
			*sptr++ = *cptr++;
		}
	}
}


int getskip(opcode)
int opcode;
{
	int i;
	opent *o;

	o = optab;
	for(i=0; i<TABSIZE; i++) {
		if(o->opcode == opcode)
			return o->nbytes;
		o++;
	}
	return NOTFOUND;
}

char *getname(opcode)
int opcode;
{
	int i;
	opent *o;

	o = optab;
	for(i=0; i<TABSIZE; i++) {
		if(o->opcode == opcode)
			return o->name;
		o++;
	}
	return "UNKNOWN";
}

unsigned char pbuf[8*4096];
unsigned char obuf[8*4096];
short rbuf[4096];
short gbuf[4096];
short bbuf[4096];
int bytesRead;
FILE *inf;
IMAGE *image;
int imgxsize, imgysize;
int gotbits, curiy;

maprow(cbuf,n,pixelsize,rbuf,gbuf,bbuf,rtab,gtab,btab)
unsigned char *cbuf;
int n, pixelsize;
short *rbuf, *gbuf, *bbuf;
int rtab[], gtab[], btab[];
{
	int left, dat, mask, val;

	left = 0;
	mask = (1<<pixelsize)-1;
	while(n--) {
		if(!left) {
			dat = *cbuf++;
			left = 8;
		}
		left = left-pixelsize;
		val = (dat>>left)&mask;
		*rbuf++ = rtab[val];
		*gbuf++ = gtab[val];
		*bbuf++ = btab[val];
	}
}

pix15tos(pbuf,rbuf,gbuf,bbuf,n)
unsigned char *pbuf;
short *rbuf, *gbuf, *bbuf;
int n;
{
	int h, l, s;

	while(n--) {
		h = *pbuf++;
		l = *pbuf++;
		s = (h<<8)+l;
		*rbuf++ = 255*((s>>10)&0x1f)/31;
		*gbuf++ = 255*((s>>5 )&0x1f)/31;
		*bbuf++ = 255*((s>>0 )&0x1f)/31;
	}
}

getrect()
{
	int val;

	bytesRead += getshort(inf,&val);
	bytesRead += getshort(inf,&val);
	bytesRead += getshort(inf,&val);
	bytesRead += getshort(inf,&val);
}

readimage(name,hasmap)
char *name;
int hasmap;
{
	int y, packtype;
	int rowbytes, thisrowbytes;
	int bmxsize, bmysize, pixelsize;
	int left, top, right, bottom;
	int imgversion, pixeltype, cmpcount;
	int i, val, ctsize;
	int pos, rval, gval, bval;
	int rtab[256], gtab[256], btab[256];
	long lval;
	int hibit;

	/* base addr */
	if(!hasmap) {
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"base addr is %d\n",lval);
#endif
	}

	/* rowbytes */
	bytesRead += getshort(inf,&rowbytes);
	hibit = rowbytes & 0x8000;
	rowbytes &= 0x7fff;

	/* bounds */
	bytesRead += getshort(inf,&top);
#ifdef DEBUG
	fprintf(stderr,"TOP IS %d\n",top);
#endif
	bytesRead += getshort(inf,&left);
	bytesRead += getshort(inf,&bottom);
	bytesRead += getshort(inf,&right);
	bmysize = bottom-top;
	bmxsize = right-left;

#ifdef DEBUG
	fprintf(stderr,"bounds: top %d left %d bottom %d right %d\n",top,left,bottom,right);
	fprintf(stderr,"bm size: %d by %d\n",bmxsize,bmysize);
#endif

	if(bottom>imgysize)
		imgysize = bottom;
	if(right>imgxsize)
		imgxsize = right;

	if(!image) {
#ifdef DEBUG
		fprintf(stderr,"FULL IMAGE SIZE IS %d by %d\n",imgxsize,imgysize);
#endif
		image = iopen(name,"w",RLE(1),3,imgxsize,imgysize,3);
		if(!image) {
			fprintf(stderr,"frompict: can't open ooutput file\n");
			exit(1);
		}
		curiy = image->ysize;
	}

	/* version */
	if(hibit) {
#ifdef DEBUG
		fprintf(stderr,"hi bit set\n");
#endif
		bytesRead += getshort(inf,&imgversion);
		if(imgversion != 0) {
			fprintf(stderr,"strange version %d\n",imgversion);
			exit(1);
		}

		/* packtype */
		bytesRead += getshort(inf,&val);
		packtype = val;

		/* packsize */
		bytesRead += getlong(inf,&lval);

		/* hres */
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"hres %d\n",lval);
#endif

		/* vres */
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"vres %d\n",lval);
#endif

		/* pixeltype */
		bytesRead += getshort(inf,&val);
		pixeltype = val;
#ifdef DEBUG
		fprintf(stderr,"pixel type %d\n",val);
#endif
		if(hasmap) {
			if(pixeltype != 0) {
				fprintf(stderr,"frompict: pixeltype must be 0 if hasmap\n");
				exit(1);
			}
		} else {
			if(pixeltype != 16) {
				fprintf(stderr,"frompict: pixeltype must be 16\n");
				exit(1);
			}
		}

		/* pixelsize */
		bytesRead += getshort(inf,&val);
		pixelsize = val;
#ifdef DEBUG
		fprintf(stderr,"pixelsize %d\n",pixelsize);
#endif

		/* cmpcount */
		bytesRead += getshort(inf,&val);
		cmpcount = val;
#ifdef DEBUG
		fprintf(stderr,"cmpcount %d\n",cmpcount);
#endif
		if(hasmap) {
			if(cmpcount != 1) {
				fprintf(stderr,"frompict: strange cmpcount %d\n",cmpcount);
				exit(1);
			}
		} else {
			if(cmpcount != 3 && cmpcount != 4) {
				fprintf(stderr,"frompict: strange cmpcount %d\n",cmpcount);
				exit(1);
			}
		}

		/* cmpsize */
		bytesRead += getshort(inf,&val);
#ifdef DEBUG
		fprintf(stderr,"cmpsize %d\n",val);
#endif

		/* planebytes */
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"planebytes %d\n",lval);
#endif

		/* pmtable */
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"pmtable %d\n",lval);
#endif

		/* pmreserved */
		bytesRead += getlong(inf,&lval);
#ifdef DEBUG
		fprintf(stderr,"pmreserved %d\n",lval);
#endif

		if(hasmap) {
			bytesRead += getlong(inf,&lval);
#ifdef DEBUG
			fprintf(stderr,"ctseed is %d\n",lval);
#endif
			bytesRead += getshort(inf,&val);
#ifdef DEBUG
			fprintf(stderr,"ctflags is %d\n",val);
#endif
			bytesRead += getshort(inf,&val);
			ctsize = val+1;
#ifdef DEBUG
			fprintf(stderr,"ctsize is %d\n",ctsize);
#endif
			if(ctsize>256) {
				fprintf(stderr,"ctsize too big %d\n",ctsize);
				exit(1);
			}
#ifdef DEBUG
			fprintf(stderr,"colormap %d slots\n",ctsize);
#endif
			for(i=0; i<ctsize; i++) {
				bytesRead += getshort(inf,&pos);
				bytesRead += getshort(inf,&rval);
				bytesRead += getshort(inf,&gval);
				bytesRead += getshort(inf,&bval);
#ifdef CMAPDEBUG
				fprintf(stderr,"pos 0x%x: 0x%x 0x%x 0x%x\n",pos,rval,gval,bval);
#endif
				pos = i;
#ifdef STRANGE
				if(pos>=ctsize) {
					exit(1);
				}
#endif
				rtab[pos] = rval>>8;
				gtab[pos] = gval>>8;
				btab[pos] = bval>>8;
			}
		}
	} else {
		pixelsize = 1;
		rtab[0] = gtab[0] = btab[0] = 255;
		rtab[1] = gtab[1] = btab[1] = 0;
	}

	/* scrrect */
	getrect();

	/* dstrect */
	getrect();

	/* transfer mode */
	bytesRead += getshort(inf,&val);

#ifdef DEBUG
	fprintf(stderr,"transfer mode is 0x%x\n",val);
#endif
	for(y=0; y<bmysize; y++) {
		if(hasmap) {
			if(rowbytes<8) {
				bytesRead += getbytes(inf,obuf,rowbytes);
			} else {
				if(rowbytes>250)
					bytesRead += getshort(inf,&val);
				else 
					bytesRead += getbyte(inf,&val);
				thisrowbytes = val;
				bytesRead += getbytes(inf,pbuf,thisrowbytes);
				unpack8(pbuf,obuf,thisrowbytes);
			}
			maprow(obuf,imgxsize,pixelsize,rbuf,gbuf,bbuf,rtab,gtab,btab);
		} else {
			if(rowbytes>250)
				bytesRead += getshort(inf,&val);
			else 
				bytesRead += getbyte(inf,&val);
			thisrowbytes = val;
			bytesRead += getbytes(inf,pbuf,thisrowbytes);
			if(pixelsize == 32) {
				if(packtype != 4) {
					fprintf(stderr,"frompict: packtype must be 4 for 32 bit img\n");
					exit(1);
				}
				unpack8(pbuf,obuf,thisrowbytes);
				if(cmpcount == 3) {
					ctos(obuf+0*imgxsize,rbuf,imgxsize);
					ctos(obuf+1*imgxsize,gbuf,imgxsize);
					ctos(obuf+2*imgxsize,bbuf,imgxsize);
				} else {
					ctos(obuf+1*imgxsize,rbuf,imgxsize);
					ctos(obuf+2*imgxsize,gbuf,imgxsize);
					ctos(obuf+3*imgxsize,bbuf,imgxsize);
				}
			} else if(pixelsize == 16) {
				if(packtype != 3) {
					fprintf(stderr,"frompict: packtype must be 3 for 16 bit img\n");
					exit(1);
				}
				unpack16(pbuf,obuf,thisrowbytes);
				pix15tos(obuf,rbuf,gbuf,bbuf,imgxsize);
			} else {
				fprintf(stderr,"frompict: strange pixel size %d\n",pixelsize);
				exit(1);
			}
		}
		curiy--;
		if(curiy>=0 && curiy<image->ysize) {
			putrow(image,rbuf,curiy,0);
			putrow(image,gbuf,curiy,1);
			putrow(image,bbuf,curiy,2);
		}
	}
	gotbits++;
}

readpict(char * outimage)
{
	int picsize, oldBytesRead, endOfPicture, opcode;
	int i, size;
	int val, x1, y1, x2, y2;
	int rows, cols, row, col;
	int version, skip;
	unsigned char **bits, *bP;

	/* Read size. */
	bytesRead = getshort(inf,&picsize);

	/* Read picFrame. */
	bytesRead += getRect(inf,&x1,&y1,&x2,&y2);

#ifdef DEBUG
	fprintf(stderr,"picFrame: %d %d %d %d\n",x1, y1, x2, y2);
#endif
	rows = y2 - y1;
	cols = x2 - x1;
	imgysize = rows;
	imgxsize = cols;

	endOfPicture = 0;
	gotbits = 0;
	version = 1;

	while(!endOfPicture) {
		switch(version) {
		case 1:
			bytesRead += getbyte( inf, &opcode );
			break;
		case 2:
			bytesRead += getshort( inf, &opcode );
			break;
		default:
			fprintf(stderr,"unknown version - bad poop\n");
			exit(1);
		}
#ifdef DEBUG
		fprintf(stderr,"bytesread %d opcode 0x%x\n",bytesRead,opcode);
#endif
		oldBytesRead = bytesRead;
		switch(opcode) {
		case PICT_picVersion:
			bytesRead += getbyte(inf,&version);
			if(version!=1 && version!=2) {
				fprintf(stderr,"strange PICT version - %d",version);
				exit(1);
			}
#ifdef DEBUG
		fprintf(stderr,"bytesread %d version 0x%x\n",bytesRead,version);
#endif
			if(version == 2) {
				bytesRead += getbyte(inf,&val);
				if(val != 0xff) {
					fprintf(stderr,"version2 bad poop1\n");
					exit(1);
				}
				bytesRead += getshort(inf,&val);
				if(val != 0x0c00) {
					fprintf(stderr,"version2 bad poop2\n");
					exit(1);
				}
				for(i=0; i<24; i++)
					bytesRead += getbyte(inf,&val);
			}
			break;

		case PICT_PackBitsRect:
#ifdef DEBUG
			fprintf(stderr,"PackBItsRect\n");
#endif 
			readimage(outimage,1);
			break;

		case 0x009a:
#ifdef DEBUG
			fprintf(stderr,"Opcode: 9a\n");
#endif 
			readimage(outimage,0);
			break;

		case PICT_longComment:
			bytesRead += skipbytes(inf,2);
			bytesRead += getshort(inf,&size);
			bytesRead += skipbytes(inf,size);
			break;

		case PICT_BitsRect:
		case PICT_BitsRgn:
		case PICT_PackBitsRgn:
			bytesRead += getBits(inf,opcode,bits,rows,cols);
			gotbits = 1;
			break;

		case PICT_EndOfPicture:
			endOfPicture = 1;
			break;

		default:
			skip = getskip(opcode);
			if(skip == CANTSKIP) {
				fprintf(stderr,"can't skip opcode [%s]\n",getname(opcode));
				exit(1);
			} else if(skip == NOTFOUND) {
				if ( version == 2 )
					fprintf(stderr,"unknown PICT2 op 0x%04x\n",opcode);
				else 
					fprintf(stderr,"unknown PICT op 0x%04x\n",opcode);
				exit(1);
			} else if(skip)
				bytesRead += skipbytes(inf,skip);
			break;
		}
		if(version==2 && (bytesRead-oldBytesRead)&1) {
			bytesRead += skipbytes(inf,1);
		}

	}
	if(!gotbits) {
		fprintf(stderr,"no pixel rectangles found\n");
		exit(1);
	}
	
	iclose(image);
	image = 0;
}

main( argc, argv )
int argc;
char *argv[];
{
	int picsize, oldBytesRead, endOfPicture, opcode;
	int i, size;
	int val, x1, y1, x2, y2;
	int rows, cols, row, col;
	int version, skip, more;
	unsigned char **bits, *bP;
	char name[256];
	
	if(argc<3) {
		fprintf(stderr,"usage: frompict input.pict out.rgb\n");
		exit(1);
	}
	inf = fopen(argv[1],"r");
	if(!inf) {
		fprintf(stderr,"frompict: can't open input file\n");
		exit(1);
	}
	/*skipbytes(inf,HEADER_SIZE);*/
	skipbytes(inf,260);

	strcpy(name, argv[2]);
	
	while(1) {
		readpict(name);
		fflush(stderr);
		printf("written: %s\n", name);
		if (getlong(inf, &more) != 4) break;
		/*if (more != 0x96c8) break;*/
		fflush(stdout);
		newname(name, +1);
	}
}

getBits(fd, opcode, bits, rows, cols)
FILE *fd;
int opcode, rows, cols;
unsigned char **bits;
{
	int rowBytes, mode;
	int bnd_x1, bnd_y1, bnd_x2, bnd_y2;
	int src_x1, src_y1, src_x2, src_y2;
	int dst_x1, dst_y1, dst_x2, dst_y2;
	int padRight, b, br, row;
	int col;
	unsigned char *bP;

	br = getshort(fd,&rowBytes);
	br += getRect(fd,&bnd_x1,&bnd_y1,&bnd_x2,&bnd_y2);
	br += getRect(fd,&src_x1,&src_y1,&src_x2,&src_y2 );
	br += getRect(fd,&dst_x1,&dst_y1,&dst_x2,&dst_y2 );
	br += getshort(fd,&mode);
	if(mode != 1)
		fprintf(stderr,"bits rectangle mode %d != 1 - Ignoring",mode);
	if(opcode == PICT_BitsRgn || opcode == PICT_PackBitsRgn)
		br += skipbytes(fd,10);

	padRight = rowBytes*8 - (bnd_x2-bnd_x1);

	getInit(opcode);

	for(row = bnd_y1; row < bnd_y2; row++) {
		br += getInitRow(fd);
		if(row>=0 && row<rows) {
			for(col = bnd_x1, bP = &(bits[row][bnd_x1]); 
			    col < bnd_x2; col++, bP++) {
				br += getBit( fd, &b );
				if(col >= 0 && col < cols)
					*bP = b ? 0 : 255;
			}
			for(col = 0; col < padRight; col++)
				br += getBit( fd, &b );
		} else {
			for(col = bnd_x1; col < bnd_x2; col++)
				br += getBit( fd, &b );
			for(col = 0; col < padRight; col++)
				br += getBit( fd, &b );
		}
	}
	return br;
}

getbytes(fd,buf,n)
FILE *fd;
unsigned char *buf;
int n;
{
	int c;

	c = n;
	while(c--)
		*buf++ = getc(fd);
	return n;
}

getbyte(fd,val)
FILE *fd;
int *val;
{
	*val = getc(fd);
	if (*val == EOF) {
		fprintf(stderr,"premature EOF\n");
		exit(1);
	}
	return 1;
}

skipbytes(fd,n)
FILE *fd;
int n;
{
	int i, j;

	for (i=0; i<n; i++)
		getbyte(fd,&j);
	return n;
}

getshort(fd,val)
FILE *fd;
int *val;
{
	int br, h, l;

	br  = getbyte(fd,&h);
	br += getbyte(fd,&l);
	*val = (h<<8) + l;
	return br;
}

getlong(fd,val)
FILE *fd;
long *val;
{
	int br, b1, b2, b3, b4;

	br  = getbyte(fd,&b1);
	br += getbyte(fd,&b2);
	br += getbyte(fd,&b3);
	br += getbyte(fd,&b4);
	*val = (b1<<24)+(b2<<16)+(b3<<8)+b4;
	return br;
}

getRect(fd,x1,y1,x2,y2)
FILE *fd;
int *x1, *y1, *x2, *y2;
{
	int br;

	br  = getshort(fd,y1);
	br += getshort(fd,x1);
	br += getshort(fd,y2);
	br += getshort(fd,x2);
	return br;
}

int packed;
int count, repeat, item, bitshift;

getInit(opcode)
int opcode;
{
	packed = (opcode==PICT_PackBitsRect || opcode==PICT_PackBitsRgn);
	bitshift = -1;
	count = 0;
	return 0;
}

getInitRow(fd)
FILE *fd;
{
	int br;

	br = 0;
	if(packed)
		br += skipbytes(fd,1);		/* skip one junk byte */
	return br;
}

getBit(fd,iP)
FILE *fd;
int *iP;
{
	int br;

	br = 0;
	if (bitshift<0) {
		if (packed) {
			if (count<=0) {
				br += getbyte(fd,&count);
				if (count<128) {
					repeat = 0;
					br += getbyte(fd,&item);
				} else {
					repeat = 1;
					br += getbyte(fd,&item);
					count = 256 - count;
				}
			} else {
				if (!repeat) {
					br += getbyte(fd,&item);
				}
				count--;
			}
		} else
			br += getbyte(fd,&item);
		bitshift = 7;
	}
	*iP = (item>>bitshift--)&1;
	return br;
}

unpack8(pbuf,obuf,npacked)
unsigned char *pbuf, *obuf;
int npacked;
{
	int pos, cnt, val;

	pos = 0;
	while(npacked>0) {
		cnt = *pbuf++;
		npacked--;
		if((cnt&0x80)==0) {
			cnt++;
			while(cnt--) {
				obuf[pos++] = *pbuf++;
				npacked--;
			}
		} else {
			cnt = 257-cnt;
			val = *pbuf++;
			npacked--;
			while (cnt--)
				obuf[pos++] = val;
		}
	}
	if(npacked != 0)
		fprintf(stderr,"small unpack error %d\n",npacked);
}

unpack16(pbuf,obuf,npacked)
unsigned char *pbuf, *obuf;
int npacked;
{
	int pos, cnt, val0, val1;

	pos = 0;
	while(npacked>0) {
		cnt = *pbuf++;
		npacked--;
		if((cnt&0x80)==0) {
			cnt++;
			while(cnt--) {
				obuf[pos++] = *pbuf++;
				npacked--;
				obuf[pos++] = *pbuf++;
				npacked--;
			}
		} else {
			cnt = 257-cnt;
			val0 = *pbuf++;
			npacked--;
			val1 = *pbuf++;
			npacked--;
			while (cnt--) {
				obuf[pos++] = val0;
				obuf[pos++] = val1;
			}
		}
	}
	if(npacked != 0)
		fprintf(stderr,"small unpack error %d\n",npacked);
}

