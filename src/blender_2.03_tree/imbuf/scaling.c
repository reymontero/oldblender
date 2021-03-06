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

#include "imbuf.h"


#define OBJECTBLOK "scaling"

/************************************************************************/
/*								SCALING									*/
/************************************************************************/


struct ImBuf *half_x(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	uchar *p1,*_p1,*dest;
	short a,r,g,b,x,y;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);
	
	if (ibuf1->x <= 1) return(dupImBuf(ibuf1));
	
	ibuf2 = allocImBuf((ibuf1->x)/2 , ibuf1->y , ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	_p1 = (uchar *) ibuf1->rect;
	dest=(uchar *) ibuf2->rect;

	for(y=ibuf2->y;y>0;y--){
		p1 = _p1;
		for(x = ibuf2->x ; x>0 ; x--){
			a = *(p1++) ;
			b = *(p1++) ;
			g = *(p1++) ;
			r = *(p1++);
			a += *(p1++) ;
			b += *(p1++) ;
			g += *(p1++) ;
			r += *(p1++);
			*(dest++) = a >> 1;
			*(dest++) = b >> 1;
			*(dest++) = g >> 1;
			*(dest++) = r >> 1;
		}
		_p1 += (ibuf1->x << 2);
	}
	return (ibuf2);
}


struct ImBuf *double_fast_x(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	int *p1,*dest, i, col;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2 = allocImBuf(2 * ibuf1->x , ibuf1->y , ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	p1 = (int *) ibuf1->rect;
	dest=(int *) ibuf2->rect;

	for(i = ibuf1->y * ibuf1->x ; i>0 ; i--) {
		col = *p1++;
		*dest++ = col;
		*dest++ = col;
	}

	return (ibuf2);
}

struct ImBuf *double_x(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2 = double_fast_x(ibuf1);

	filterx(ibuf2);
	return (ibuf2);
}


struct ImBuf *half_y(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	uchar *p1,*p2,*_p1,*dest;
	short a,r,g,b,x,y;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);
	if (ibuf1->y <= 1) return(dupImBuf(ibuf1));

	ibuf2 = allocImBuf(ibuf1->x , (ibuf1->y) / 2 , ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	_p1 = (uchar *) ibuf1->rect;
	dest=(uchar *) ibuf2->rect;

	for(y=ibuf2->y ; y>0 ; y--){
		p1 = _p1;
		p2 = _p1 + (ibuf1->x << 2);
		for(x = ibuf2->x ; x>0 ; x--){
			a = *(p1++) ;
			b = *(p1++) ;
			g = *(p1++) ;
			r = *(p1++);
			a += *(p2++) ;
			b += *(p2++) ;
			g += *(p2++) ;
			r += *(p2++);
			*(dest++) = a >> 1;
			*(dest++) = b >> 1;
			*(dest++) = g >> 1;
			*(dest++) = r >> 1;
		}
		_p1 += (ibuf1->x << 3);
	}
	return (ibuf2);
}


struct ImBuf *double_fast_y(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	int *p1, *dest1, *dest2;
	short x,y;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2 = allocImBuf(ibuf1->x , 2 * ibuf1->y , ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	p1 = (int *) ibuf1->rect;
	dest1=(int *) ibuf2->rect;

	for(y = ibuf1->y ; y>0 ; y--){
		dest2 = dest1 + ibuf2->x;
		for(x = ibuf2->x ; x>0 ; x--) *dest1++ = *dest2++ = *p1++;
		dest1 = dest2;
	}

	return (ibuf2);
}

struct ImBuf *double_y(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;

	if (ibuf1==0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2 = double_fast_y(ibuf1);
	
	filtery(ibuf2);
	return (ibuf2);
}


struct ImBuf *onehalf(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	uchar *p1,*p2,*dest;
	int a,r,g,b,x,y,i;

	if (ibuf1 == 0) return (0);
	if (ibuf1->rect == 0) return (0);

	if (ibuf1->x <= 1) return(half_y(ibuf1));
	if (ibuf1->y <= 1) return(half_x(ibuf1));
	
	ibuf2=allocImBuf((ibuf1->x)/2,(ibuf1->y)/2,ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	p1 = (uchar *) ibuf1->rect;
	dest=(uchar *) ibuf2->rect;

	for(y=ibuf2->y;y>0;y--){
		p2 = p1 + (ibuf1->x << 2);
		for(x=ibuf2->x;x>0;x--){
			dest[0] = (p1[0] + p2[0] + p1[4] + p2[4]) >> 2;
			dest[1] = (p1[1] + p2[1] + p1[5] + p2[5]) >> 2;
			dest[2] = (p1[2] + p2[2] + p1[6] + p2[6]) >> 2;
			dest[3] = (p1[3] + p2[3] + p1[7] + p2[7]) >> 2;
			p1 += 8; 
			p2 += 8; 
			dest += 4;
		}
		p1=p2;
		if(ibuf1->x & 1) {
			p1+=4;
		}
	}
	return (ibuf2);
}



struct ImBuf *onethird(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	uchar *p1,*p2,*p3,*dest;
	short a,r,g,b,x,y,i;

	if (ibuf1 == 0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2=allocImBuf((ibuf1->x)/3,(ibuf1->y)/3,ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	p1 = (uchar *) ibuf1->rect;
	dest=(uchar *) ibuf2->rect;

	for(y=ibuf2->y;y>0;y--){
		p2 = p1 + (ibuf1->x << 2);
		p3 = p2 + (ibuf1->x << 2);
		for(x=ibuf2->x;x>0;x--){
			a=r=g=b=0;
			for (i=3;i>0;i--){
				a += *(p1++) + *(p2++) + *(p3++);
				b += *(p1++) + *(p2++) + *(p3++);
				g += *(p1++) + *(p2++) + *(p3++);
				r += *(p1++) + *(p2++) + *(p3++);
			}
			*(dest++) = a/9;
			*(dest++) = b/9;
			*(dest++) = g/9;
			*(dest++) = r/9;
		}
		p1=p3;
	}
	return (ibuf2);
}


struct ImBuf *halflace(struct ImBuf *ibuf1)
{
	struct ImBuf *ibuf2;
	uchar *p1,*p2,*dest;
	short a,r,g,b,x,y,i;

	if (ibuf1 == 0) return (0);
	if (ibuf1->rect == 0) return (0);

	ibuf2=allocImBuf((ibuf1->x)/4,(ibuf1->y)/2,ibuf1->depth,1,0);
	if (ibuf2==0) return (0);

	p1 = (uchar *) ibuf1->rect;
	dest=(uchar *) ibuf2->rect;

	for(y= ibuf2->y / 2 ; y>0;y--){
		p2 = p1 + (ibuf1->x << 3);
		for(x = 2 * ibuf2->x;x>0;x--){
			a=r=g=b=0;
			for (i=4;i>0;i--){
				a += *(p1++) + *(p2++);
				b += *(p1++) + *(p2++);
				g += *(p1++) + *(p2++);
				r += *(p1++) + *(p2++);
			}
			*(dest++) = a >> 3;
			*(dest++) = b >> 3;
			*(dest++) = g >> 3;
			*(dest++) = r >> 3;
		}
		p1 = p2;
	}
	return (ibuf2);
}


struct ImBuf *scaledownx(struct ImBuf *ibuf, int newx)
{
	uchar *rect,*_newrect,*newrect;
	float sample, add, val, nval;
	int x, y, i;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	_newrect = (uchar *) malloc(newx * ibuf->y * sizeof(int));
	if (_newrect == 0) return(ibuf);

	add = (ibuf->x - 0.001) / newx;

	/* all four components, rgba/abgr */
	for(i=3 ; i >= 0 ; i--){
		rect = (uchar *) ibuf->rect;
		rect += i;
		newrect = _newrect + i;

		for (y = ibuf->y; y>0 ; y--){
			val = sample = 0.0;

			for (x = newx ; x>0 ; x--){
				nval = - val * sample;
				sample += add;

				while (sample >= 1.0){
					sample -= 1.0;
					nval += *rect;
					rect += 4;
				}
				val = *rect;
				rect += 4;
				nval += sample * val;
				sample -= 1.0;
				*newrect = (nval/add) + 0.5;
				newrect += 4;
			}
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = (uint *) _newrect;
	ibuf->x = newx;
	return(ibuf);
}


struct ImBuf *scaledowny(struct ImBuf *ibuf, int newy)
{
	uchar *rect,*_newrect,*newrect;
	float sample,add,val,nval;
	int x,y,i,skipx;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	_newrect = (uchar *) malloc(newy * ibuf->x * sizeof(int));
	if (_newrect == 0) return(ibuf);

	add = (ibuf->y - 0.001) / newy;
	skipx = 4 * ibuf->x;

	/* all four components, rgba/abgr */
	for(i=3 ; i>=0 ; i--){
		for (x = skipx - 4; x>=0 ; x-= 4){
			rect = ((uchar *) ibuf->rect) + i + x;
			newrect = _newrect + i + x;
			val = sample = 0.0;

			for (y = newy ; y>0 ; y--){
				nval = - val * sample;
				sample += add;

				while (sample >= 1.0){
					sample -= 1.0;
					nval += *rect;
					rect += skipx;
				}
				val = *rect;
				rect += skipx;
				nval += sample * val;
				sample -= 1.0;
				*newrect = (nval/add) + 0.5;
				newrect += skipx;
			}
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = (uint *) _newrect;
	ibuf->y = newy;
	return(ibuf);
}


struct ImBuf *scaleupx_byte(struct ImBuf *ibuf, int newx)
{
	uchar *rect,*_newrect,*newrect;
	float sample,add,val,nval,diff;
	int x,y,i;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	_newrect = (uchar *) malloc(newx * ibuf->y * sizeof(int));
	if (_newrect == 0) return(ibuf);

	add = (ibuf->x - 1.001) / (newx - 1.0);

	/* all four components, rgba/abgr */
	for(i=3 ; i>=0 ; i--){
		rect = (uchar *) ibuf->rect;
		rect += i;
		newrect = _newrect + i;

		for (y = ibuf->y; y>0 ; y--){

			sample = 0;
			val = *rect ;
			rect += 4;
			nval = *rect;
			rect += 4;
			diff = nval - val;
			val += 0.5;

			for (x = newx ; x>0 ; x--){
				if (sample >= 1.0){
					sample -= 1.0;
					val = nval ;
					nval = *rect ;
					rect += 4;
					diff = nval - val ;
					val += 0.5;
				}
				*newrect = val + sample * diff;
				newrect += 4;
				sample += add;
			}
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = (uint *) _newrect;
	ibuf->x = newx;
	return(ibuf);
}


struct ImBuf *scaleupx(struct ImBuf *ibuf, int newx)
{
	uchar *rect,*_newrect,*newrect;
	float sample,add;
	float val_a,nval_a,diff_a;
	float val_b,nval_b,diff_b;
	float val_g,nval_g,diff_g;
	float val_r,nval_r,diff_r;
	int x,y,i;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	_newrect = (uchar *) malloc(newx * ibuf->y * sizeof(int));
	if (_newrect == 0) return(ibuf);

	add = (ibuf->x - 1.001) / (newx - 1.0);

	rect = (uchar *) ibuf->rect;
	newrect = _newrect;

	for (y = ibuf->y; y>0 ; y--){

		sample = 0;
		val_a = rect[0] ;
		nval_a = rect[4];
		diff_a = nval_a - val_a ;
		val_a += 0.5;

		val_b = rect[1] ;
		nval_b = rect[5];
		diff_b = nval_b - val_b ;
		val_b += 0.5;

		val_g = rect[2] ;
		nval_g = rect[6];
		diff_g = nval_g - val_g ;
		val_g += 0.5;

		val_r = rect[3] ;
		nval_r = rect[7];
		diff_r = nval_r - val_r ;
		val_r += 0.5;

		rect += 8;
		for (x = newx ; x>0 ; x--){
			if (sample >= 1.0){
				sample -= 1.0;
				val_a = nval_a ;
				nval_a = rect[0] ;
				diff_a = nval_a - val_a ;
				val_a += 0.5;

				val_b = nval_b ;
				nval_b = rect[1] ;
				diff_b = nval_b - val_b ;
				val_b += 0.5;

				val_g = nval_g ;
				nval_g = rect[2] ;
				diff_g = nval_g - val_g ;
				val_g += 0.5;

				val_r = nval_r ;
				nval_r = rect[3] ;
				diff_r = nval_r - val_r ;
				val_r += 0.5;
				rect += 4;
			}
			newrect[0] = val_a + sample * diff_a;
			newrect[1] = val_b + sample * diff_b;
			newrect[2] = val_g + sample * diff_g;
			newrect[3] = val_r + sample * diff_r;
			newrect += 4;
			sample += add;
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = (uint *) _newrect;
	ibuf->x = newx;
	return(ibuf);
}


struct ImBuf *scaleupy(struct ImBuf *ibuf, int newy)
{
	uchar *rect,*_newrect,*newrect;
	float sample,add,val,nval,diff;
	int x,y,i,skipx;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	_newrect = (uchar *)malloc(newy * ibuf->x * sizeof(int));
	if (_newrect == 0) return(ibuf);

	add = (ibuf->y - 1.001) / (newy - 1.0);
	skipx = 4 * ibuf->x;

	/* all four components, rgba/abgr */
	for(i=3 ; i>=0 ; i--){
		for (x = skipx - 4; x >= 0 ; x -= 4){
			rect = (uchar *) ibuf->rect;
			rect += i + x;
			newrect = _newrect + i + x;

			sample = 0;
			val = *rect ;
			rect += skipx;
			nval = *rect;
			rect += skipx;
			diff = nval - val;
			val += 0.5;

			for (y = newy ; y>0 ; y--){
				if (sample >= 1.0){
					sample -= 1.0;
					val = nval;
					nval = *rect;
					rect += skipx;
					diff = nval - val;
					val += 0.5;
				}
				*newrect = val + sample * diff;
				newrect += skipx;
				sample += add;
			}
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = (uint *) _newrect;
	ibuf->y = newy;
	return(ibuf);
}

void scalefast_Z_ImBuf(ImBuf *ibuf, short newx, short newy)
{
	uint *rect,*_newrect,*newrect;
	int x,y;
	int ofsx,ofsy,stepx,stepy;

	if (ibuf->zbuf) {
		_newrect = malloc(newx * newy * sizeof(int));
		if (_newrect == 0) return;
	
		stepx = (65536.0 * (ibuf->x - 1.0) / (newx - 1.0)) + 0.5;
		stepy = (65536.0 * (ibuf->y - 1.0) / (newy - 1.0)) + 0.5;
		ofsy = 32768;

		newrect = _newrect;
	
		for (y = newy; y > 0 ; y--){
			rect = (uint*) ibuf->zbuf;
			rect += (ofsy >> 16) * ibuf->x;
			ofsy += stepy;
			ofsx = 32768;
			for (x = newx ; x > 0 ; x--){
				*newrect++ = rect[ofsx >> 16];
				ofsx += stepx;
			}
		}
	
		freezbufImBuf(ibuf);
		ibuf->mall |= IB_zbuf;
		ibuf->zbuf = (int*) _newrect;
	}
}

struct ImBuf *scaleImBuf(struct ImBuf * ibuf, short newx, short newy)
{
	if (ibuf == 0) return (0);
	if (ibuf->rect == 0) return (ibuf);

	if (newx < ibuf->x) if (newx) scaledownx(ibuf,newx);
	if (newy < ibuf->y) if (newy) scaledowny(ibuf,newy);
	if (newx > ibuf->x) if (newx) scaleupx(ibuf,newx);
	if (newy > ibuf->y) if (newy) scaleupy(ibuf,newy);

	scalefast_Z_ImBuf(ibuf, newx, newy);
	
	return(ibuf);
}


struct ImBuf *scalefastImBuf(struct ImBuf *ibuf, short newx, short newy)
{
	uint *rect,*_newrect,*newrect;
	int x,y;
	int ofsx,ofsy,stepx,stepy;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(ibuf);

	if (newx == ibuf->x && newy == ibuf->y) return(ibuf);

	_newrect = malloc(newx * newy * sizeof(int));
	if (_newrect == 0) return(ibuf);

	newrect = _newrect;
	stepx = (65536.0 * (ibuf->x - 1.0) / (newx - 1.0)) + 0.5;
	stepy = (65536.0 * (ibuf->y - 1.0) / (newy - 1.0)) + 0.5;
	ofsy = 32768;

	for (y = newy; y > 0 ; y--){
		rect = ibuf->rect;
		rect += (ofsy >> 16) * ibuf->x;
		ofsy += stepy;
		ofsx = 32768;
		for (x = newx ; x>0 ; x--){
			*newrect++ = rect[ofsx >> 16];
			ofsx += stepx;
		}
	}

	freerectImBuf(ibuf);
	ibuf->mall |= IB_rect;
	ibuf->rect = _newrect;
	
	scalefast_Z_ImBuf(ibuf, newx, newy);
	
	ibuf->x = newx;
	ibuf->y = newy;
	return(ibuf);
}


struct ImBuf *generic_fieldscale(struct ImBuf *ibuf, short newx, short newy, struct ImBuf *(*scalefunc)(ImBuf *, short, short) )
{
	struct ImBuf *sbuf1, *sbuf2;
	extern void rectcpy();
	
	sbuf1 = allocImBuf(ibuf->x, ibuf->y / 2, ibuf->depth, IB_rect, 0);
	sbuf2 = allocImBuf(ibuf->x, ibuf->y / 2, ibuf->depth, IB_rect, 0);
	
	ibuf->x *= 2;
	rectop(sbuf1, ibuf, 0, 0, 0, 0, 32767, 32767, rectcpy);
	rectop(sbuf2, ibuf, 0, 0, sbuf2->x, 0, 32767, 32767, rectcpy);
	
	freerectImBuf(ibuf);
	ibuf->x = newx;
	ibuf->y = newy;
	addrectImBuf(ibuf);
	
	scalefunc(sbuf1, newx, newy / 2);
	scalefunc(sbuf2, newx, newy / 2);	
	
	ibuf->x *= 2;
	
	rectop(ibuf, sbuf1, 0, 0, 0, 0, 32767, 32767, rectcpy);
	rectop(ibuf, sbuf2, sbuf2->x, 0, 0, 0, 32767, 32767, rectcpy);
	
	ibuf->x /= 2;
	
	freeImBuf(sbuf1);
	freeImBuf(sbuf2);
	
	return(ibuf);
}


struct ImBuf *scalefastfieldImBuf(struct ImBuf *ibuf, short newx, short newy)
{
	return(generic_fieldscale(ibuf, newx, newy, scalefastImBuf));
}

struct ImBuf *scalefieldImBuf(struct ImBuf *ibuf, short newx, short newy)
{
	return(generic_fieldscale(ibuf, newx, newy, scaleImBuf));
}

