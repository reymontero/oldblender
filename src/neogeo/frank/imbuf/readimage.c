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

#include "imbuf.h"


#define OBJECTBLOK "readimage"

int IB_verbose = TRUE;

struct ImBuf *loadiffmem(long *mem, long flags)
{
	long len,maxlen;
	struct ImBuf *ibuf;

	maxlen= (GET_BIG_LONG(mem+1) + 1) & ~1;

	if (GET_ID(mem) == CAT){
		mem += 3;
		maxlen -= 4;
		while(maxlen > 0){
			if (GET_ID(mem) == FORM){
				len = ((GET_BIG_LONG(mem+1) + 1) & ~1) + 8;
				if ((GET_ID(mem+2) == ILBM) | (GET_ID(mem+2) == IMAG) | (GET_ID(mem+2) == ANIM)) break;
				mem = (long *)((uchar *)mem +len);
				maxlen -= len;
			}else return(0);
		}
	}

	if (maxlen > 0){
		if (GET_ID(mem) == FORM){
			if (GET_ID(mem+2) == ILBM){
				return (loadamiga(mem,flags));
			}else if (GET_ID(mem+5) == ILBM){			/* animaties */
				return (loadamiga(mem+3,flags));
			}else if (GET_ID(mem+2) == IMAG){
				return (loadcdi(mem,flags));
			}else if (GET_ID(mem+2) == ANIM){
				return (loadanim(mem,flags));
			}
		} else if ((BIG_LONG(mem[0]) == 0x10000000) && ((BIG_LONG(mem[1]) & 0xf0ffffff) == 0)) {
			return(loadtim((ushort *) mem, flags));
		} else if (GS(mem) == IMAGIC | GSS(mem) == IMAGIC){
			return (loadiris((uchar *) mem,flags));
		} else if ((BIG_LONG(mem[0]) & 0xfffffff0) == 0xffd8ffe0) {
			return (0);
		}
	}

	ibuf = loadtarga((uchar *) mem,flags);
	if (ibuf) return(ibuf);

	if (IB_verbose) fprintf(stderr,"Unknown fileformat\n");
	return (0);
}


struct ImBuf *loadifffile(file,flags)
long file,flags;
{
	struct ImBuf *ibuf;
	long size,*mem;

	if (file == -1) return (0);

	size = lseek(file,0L,SEEK_END);
	lseek(file,0L,SEEK_SET);
	
#ifdef AMIGA
	mem=(long *)malloc(size);
	if (mem==0){
		printf("Out of mem\n");
		return (0);
	}

	if (read(file,mem,size)!=size){
		printf("Read Error\n");
		free(mem);
		return (0);
	}

	ibuf = loadiffmem(mem,flags | IB_freem);
#else
	mem=(long *)mmap(0,size,PROT_READ,MAP_SHARED,file,0);
	if (mem==(long *)-1){
		printf("Couldn't get mapping\n");
		return (0);
	}

	ibuf = loadiffmem(mem,flags);

	if (munmap(mem,size)){
		printf("Couldn't unmap file.\n");
	}
#endif
	return(ibuf);
}


struct ImBuf *loadiffname(char *naam,long flags)
{
	long file;
	struct ImBuf *ibuf;
	long buf[1];

	file = open(naam,O_RDONLY);

	if (file == -1) return (0);
	
	ibuf=loadifffile(file,flags);

	if (ibuf == 0){
		if (read(file,buf,4) != 4) buf[0] = 0;
		if ((BIG_LONG(buf[0]) & 0xfffffff0) == 0xffd8ffe0) ibuf = loadjpeg(naam, flags);
	}
	
	if (ibuf) {
	    strncpy(ibuf->name, naam, sizeof(ibuf->name));
		if (flags & IB_fields) de_interlace(ibuf);
	}
	close(file);
	return(ibuf);
}


struct ImBuf *testiffname(char *naam,long flags)
{
	long file;
	struct ImBuf *ibuf;

	flags |= IB_test;
	file = open(naam,O_RDONLY);

	if (file<=0) return (0);
	
	ibuf=loadifffile(file,flags);
	if (ibuf) {
	    strncpy(ibuf->name, naam, sizeof(ibuf->name));
	}
	close(file);
	return(ibuf);
}


