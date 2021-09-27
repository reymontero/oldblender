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

/* inbetween.c  aug 93 */


#include <local/iff.h>
#include <gl/image.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <local/util.h>
#include <local/gl_util.h>
#include <math.h>

/* codes is werkbuffer:
 *		1: pixel binnen in vorm
		128: edge
		129: edge zonder vector
 */


int sizex, sizey, transx, transy;

void convertworktoibuf(ibuf, work)
struct ImBuf *ibuf;
uchar *work;
{
	ulong x, y, *lp;
	uchar *cp;
	
	lp= ibuf->rect;
	cp= work;
	
	for(y=0; y<ibuf->y; y++) {
		for(x=0; x<ibuf->x; x++, lp++, cp++) {
			if(*cp==1) *lp= 0x808080;
			else if(*cp==128) *lp= 0xFFFFFF;
			else *lp= 0;
		}
	}
}

void drawvectorbuf(vbuf, work1, work2)
float *vbuf;
uchar *work1, *work2;
{
	float vec[2], *fp;
	long win;
	int x, y;
	uchar *cp;
	
	prefsize(2*sizex, 2*sizey);
	win= winopen(vbuf);
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	ortho2(-0.5, (float)sizex-0.5, -0.5, (float)sizey-0.5);

	cp= work1;
	fp= vbuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++, fp+=2) {
			if(cp[0]==128) {
				cpack(0xFFFFFF);
				vec[0]= x;
				vec[1]= y;
				bgnline();
					v2f(vec);
					vec[0]+= fp[0];
					vec[1]+= fp[1];
					v2f(vec);
				endline();
			}
			else if(cp[0]==129) {
				cpack(0xFFFF);
				vec[0]= x;
				vec[1]= y;
				bgnpoint(); v2f(vec); endpoint();
			}
		}
	}
	
	cpack(0xFF);
	cp= work2;
	bgnpoint();
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++) {
			if(cp[0] & 128) {
				vec[0]= x;
				vec[1]= y;
				v2f(vec);
			}
		}
	}
	endpoint();
	
	cpack(0xFF00);
	cp= work1;
	bgnpoint();
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++) {
			if(cp[0] & 128) {
				vec[0]= x;
				vec[1]= y;
				v2f(vec);
			}
		}
	}
	endpoint();
	
	while( getbutton(ESCKEY)==0) sginap(2);
	
	winclose(win);
}

void drawfrombuf(vbuf)
float *vbuf;
{
	float vec[2], *fp;
	long win;
	int x, y;
	uchar *cp;
	
	prefsize(2*sizex, 2*sizey);
	win= winopen(vbuf);
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	ortho2(-0.5, (float)sizex-0.5, -0.5, (float)sizey-0.5);

	fp= vbuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++, fp+=2) {
			if( (x & 3)==0 && (y & 3)==0 ) {
				if(fp[0]!=0.0 || fp[1]!=0.0) {
					cpack(0xFFFFFF);
					vec[0]= x;
					vec[1]= y;
					bgnline();
						v2f(vec);
						vec[0]+= fp[0];
						vec[1]+= fp[1];
						v2f(vec);
					endline();
				}
			}
		}
	}
	while( getbutton(ESCKEY)) sginap(2);
	sginap(3);
	while( getbutton(ESCKEY)==0) sginap(2);
	
	winclose(win);
}

void drawfrombuf1( work1)
uchar *work1;
{
	float vec[2], *fp;
	long win;
	int x, y;
	uchar *cp;
	
	prefsize(2*sizex, 2*sizey);
	win= winopen("frombuf");
	RGBmode();
	gconfig();
	cpack(0);
	clear();
	ortho2(-0.5, (float)sizex-0.5, -0.5, (float)sizey-0.5);

	cp= work1;
	bgnpoint();
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++) {
			cpack(0);
			if(cp[0] ==1) cpack(0xFFFFFF);
			else if(cp[0] ==2) cpack(0xFF00FF);
			else if(cp[0] ==3) cpack(0x00FF);
				
			vec[0]= x;
			vec[1]= y;
			v2f(vec);
		}
	}
	endpoint();
	
	while( getbutton(ESCKEY)==0) sginap(2);
	
	winclose(win);
}

void animvectorbuf(vbuf, work1, work2)
float *vbuf;
uchar *work1, *work2;
{
	float vec[2], *fp, fac;
	long win;
	int x, y;
	uchar *cp;
	
	prefsize(2*sizex, 2*sizey);
	win= winopen("vbuf");
	RGBmode();
	doublebuffer();
	gconfig();
	cpack(0);
	clear();
	swapbuffers();
	ortho2(-0.5, (float)sizex-0.5, -0.5, (float)sizey-0.5);

	fac= 0.0;
	while( getbutton(ESCKEY)==0) {
		cpack(0);
		clear();
		cp= work1;
		fp= vbuf;
		bgnpoint();
		for(y=0; y<sizey; y++) {
			for(x=0; x<sizex; x++, cp++, fp+=2) {
				if(cp[0]==128) {
					cpack(0xFFFFFF);
					vec[0]= x-transx+fac*(fp[0]+transx);
					vec[1]= y-transy+fac*(fp[1]+transy);
					v2f(vec);
				}
				else if(cp[0]==129) {
					cpack(0xFFFF);
					vec[0]= x-transx;
					vec[1]= y-transy;
					v2f(vec);
				}
			}
		}
		endpoint();
		fac+= .06;
		if(fac>=1.0) fac= 0.0;
		swapbuffers();
	}
	
	winclose(win);
}

void initworkrect(work, ibuf)
uchar *work;
struct ImBuf *ibuf;
{
	/* maak rect 1 kleur met edge en verwijder losstaande pixels */
	ulong x, y, transp, *lp, num;
	uchar *cp, *cp0, *cp1;
	
	lp= ibuf->rect;
	cp= work;
	transp= *lp;
	
	for(y=0; y<ibuf->y; y++) {
		for(x=0; x<ibuf->x; x++, lp++, cp++) {
			if(*lp==transp) *cp= 0;
			else *cp= 1;
		}
	}

	/* losstaand verwijderen */
	cp0= work;
	cp= cp0+ibuf->x;
	cp1= cp+ibuf->x;
	
	for(y=0; y<ibuf->y-2; y++) {
		for(x=0; x<ibuf->x-2; x++, cp0++, cp++, cp1++) {
			num= cp0[0]+cp0[1]+cp0[2]+cp[0]+cp[1]+cp[2]+cp1[0]+cp1[1]+cp1[2];
			if(num==1 && cp[1]) cp[1]= 0;
			if(num==8 && cp[1]==0) cp[1]= 1;  
		}
		cp0+=2;
		cp+=2;
		cp1+=2;
	}
	
	/* edge en losstaand verwijderen */
	cp0= work;
	cp= cp0+ibuf->x;
	cp1= cp+ibuf->x;
	
	for(y=0; y<ibuf->y-2; y++) {
		for(x=0; x<ibuf->x-2; x++, cp0++, cp++, cp1++) {
			if(cp[1]) {
				num= cp0[1]+cp[0]+cp[1]+cp[2]+cp1[1];

				num= (num>>7) + (num & 127);
				if(num<5) cp[1]= 128;
			}
		}
		cp0+=2;
		cp+=2;
		cp1+=2;
	}
}

void translatework(work)
char *work;
{
	int x, y, ofs;
	char *temp, *cp1, *cp2;
	
	ofs= sizex*transy+transx;
	temp= callocN(sizex*sizey, "translatework");
	cp1= work;
	cp2= temp;
	
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp1++, cp2++) {
			if(*cp1) {
				if(x+transx>=0 && x+transx<sizex) {
					if(y+transy>=0 && y+transy<sizey) {
						cp2[ofs]= *cp1;
					}
				}
			}
		}
	}
	memcpy(work, temp, sizex*sizey);
	
	freeN(temp);
}

void find_main_translate(work1, work2)
char *work1, *work2;
{
	int minx1, maxx1, miny1, maxy1, gemx1=0, gemy1=0;
	int minx2, maxx2, miny2, maxy2, gemx2=0, gemy2=0;
	int x, y, count;
	char *cp;
	
	minx1= miny1= minx2= miny2= 10000;
	maxx1= maxy1= maxx2= maxy2= -10000;
	
	count= 0;
	cp= work1;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++) {
			if(*cp) {
				if(x<minx1) minx1= x;
				if(y<miny1) miny1= y;
				if(x>maxx1) maxx1= x;
				if(y>maxy1) maxy1= y;
				gemx1+= x;
				gemy1+= y;
				count++;
			}
		}
	}
	gemx1/= count;
	gemy1/= count;
	
	count= 0;
	cp= work2;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++) {
			if(*cp) {
				if(x<minx2) minx2= x;
				if(y<miny2) miny2= y;
				if(x>maxx2) maxx2= x;
				if(y>maxy2) maxy2= y;
				gemx2+= x;
				gemy2+= y;
				count++;
			}
		}
	}
	gemx2/= count;
	gemy2/= count;
	
	transx= gemx2-gemx1;
	transy= gemy2-gemy1;
}

int walkdist(work1, work2, sx, sy, dx, dy)
uchar *work1, *work2;
int sx, sy, dx, dy;
{
	
	/* - vertek vanuit work1[sy][sx]==EDGE
	 * - alleen lopen als work1!=EDGE && (work1 XOR work2)
	 * - stoppen als work2[y][x]==EDGE of dist>max
	 * - bij diagonaal lopen ook pixels ernaast kijken: anders glipt ie er tussendoor!
	 */
	 
	int distplus= 0, distmin= 0, wofs, val, sxo, syo, diag, max, getsecond;
	int firstnotedge, solminout=0, solplusout=0;
	uchar *wo1, *wo2;
	
	sxo= sx;
	syo= sy;
	
	
	if(dx!=0 && dy!=0) {
		max= 50;
		diag= 1;
	}
	else {
		max= 70;
		diag= 0;
	}
	
	/* eerst ene kant op */
	
	wo1= work1+ sizex*sy+sx;
	wo2= work2+ sizex*sy+sx;
	
	wofs= sizex*dy+dx;
	
	val= 0;
	getsecond= 0;	/* als eerste pixel van wandeling 'leeg' is, pas de tweede edge pakken */
	firstnotedge= 0;
	while(distplus<max) {
		if(sx>=sizex) break;
		else if(sx<0) break;
		if(sy>=sizey) break;
		else if(sy<0) break;
		
		if(firstnotedge==0) {
			if(*wo1 & 128);
			else {
				firstnotedge= 1;
				if(*wo1==0 && *wo2==0) {
					getsecond= 1;
					solplusout= 1;	/* solution outside */
				}
			}
		}
		
		if(*wo2 & 128) {
			if(getsecond==1) getsecond= 0;
			else {
				val= 1;
				break;
			}
		}
		else if(diag) {	/* ernaast kijken */
			if( (wo2[dx] & 128) || (wo2[dy*sizex] & 128) ) {
				if(getsecond==1) getsecond= 0;
				else {
					val= 1;
					break;
				}
			}
		}
		
		if(firstnotedge==1) {
			if(*wo1 && *wo2) break;	/* verkeerde richting */
		}
		sx+= dx;
		sy+= dy;
		wo1+= wofs;
		wo2+= wofs;
		distplus++;
	}
	
	if(val && distplus==0) return 0;
	if(val==0) distplus= 100;
	
	/* andere kant op */
	sx= sxo;
	sy= syo;
	wo1= work1+ sizex*sy+sx;
	wo2= work2+ sizex*sy+sx;
	dx= -dx;
	dy= -dy;
	wofs= sizex*dy+dx;
	
	val= 0;
	getsecond= 0;
	firstnotedge= 0;
	while(distmin<max) {
		if(sx>=sizex) break;
		else if(sx<0) break;
		if(sy>=sizey) break;
		else if(sy<0) break;
		
		if(firstnotedge==0) {
			if(*wo1 & 128);
			else {
				firstnotedge= 1;
				if(*wo1==0 && *wo2==0) {
					getsecond= 1;
					solminout= 1;	/* solution outside */
				}
			}
		}

		if(*wo2 & 128) {
			if(getsecond==1) getsecond= 0;
			else {
				val= 1;
				break;
			}
		}
		else if(diag) {	/* ernaast kijken */
			if( (wo2[dx] & 128) || (wo2[dy*sizex] & 128) ) {
				if(getsecond==1) getsecond= 0;
				else {
					val= 1;
					break;
				}
			}
		}
		
		if(firstnotedge==1) {
			if(*wo1 && *wo2) break;	/* verkeerde richting */
		}
		
		sx+= dx;
		sy+= dy;
		wo1+= wofs;
		wo2+= wofs;
		distmin++;
	}
	
	if(val==0) distmin= 100;
	
	if(solminout && solplusout) return 100;
	
	if(solminout && distplus!=100) return distplus;
	if(solplusout && distmin!=100) return (-distmin);
	
	if( distmin<distplus) return (-distmin);
	else return (distplus);
	
}

void makevectorbuf(vectorbuf, work1, work2, invert)
float *vectorbuf;
uchar *work1, *work2;
int invert;
{
	float *fp, div, fx, fy, lenfach, lenfacv, lenfacd1, lenfacd2, totlen;
	int x, y, nx, ny, disth, distv, distd1, distd2, d1, d2, d3, d4, min;
	uchar *cp, *cp2;
	
	/* alle pixels van work1 aflopen, als het een edgepixel is:
	 *  - bepaal vector naar dichtstbijzijnde andere edge
	 */
	
	if(invert);
	else bzero(vectorbuf, 8*sizex*sizey);
	
	cp= work1;	
	fp= vectorbuf;
	
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp++, fp+=2) {
			if(cp[0]==128) {
			
				/* horizontaal */
				disth= walkdist(work1, work2, x, y, 1, 0);
				if(disth!=0) {
					/* verticaal */
					distv= walkdist(work1, work2, x, y, 0, 1);
					if(distv!=0) {
						/* diagonaal 1*/
						distd1= walkdist(work1, work2, x, y, 1, 1);
						if(distd1!=0) {
							/* diagonaal 2*/
							distd2= walkdist(work1, work2, x, y, 1, -1);
							if(distd2!=0) {
								if(disth+distv+distd1+distd2==400) cp[0]= 129;
								else {
									/* de kortste afstand: te hoekig , dus: */
									/* de gemiddelde vector */
									totlen= 0.0;
									if(disth!=100) {
										lenfach= 1.0/(float)abs(disth);
										totlen+= 1.0;
									}
									else lenfach= 0.0;
									if(distv!=100) {
										lenfacv= 1.0/(float)abs(distv);
										totlen+= 1.0;
									}
									else lenfacv= 0.0;
									if(distd1!=100) {
										lenfacd1= 1.0/(float)abs(distd1);
										totlen+= 1.0;
									}
									else lenfacd1= 0.0;
									if(distd2!=100) {
										lenfacd2= 1.0/(float)abs(distd2);
										totlen+= 1.0;
									}
									else lenfacd2= 0.0;
									
									/* gemiddelde lengte vectoren */
									totlen/= (lenfach+lenfacv+lenfacd1+lenfacd2);
									
									lenfach*=lenfach;	/* kwadraat van de afstand: net iets mooier */
									lenfacv*=lenfacv;
									lenfacd1*=lenfacd1;
									lenfacd2*=lenfacd2;
									
									div= lenfach+lenfacv+lenfacd1+lenfacd2;
									fx= fy= 0;
									
									if(disth!=100) {
										fx+= (lenfach/div)*disth;
									}
									if(distv!=100) {
										fy+= (lenfacv/div)*distv;
									}
									if(distd1!=100) {
										lenfacd1*= 0.707/div;
										fx+= lenfacd1*distd1;
										fy+= lenfacd1*distd1;
									}
									if(distd2!=100) {
										lenfacd2*= 0.707/div;
										fx+= lenfacd2*distd2;
										fy-= lenfacd2*distd2;
									}
									
									/* vector corrigeren op lengte */
									
									div= fsqrt(fx*fx+fy*fy);
									fx*= totlen/div;
									fy*= totlen/div;
									
									if(invert==0) {
										fp[0]= fx; fp[1]= fy;
									}
									else {
										nx= x+(int)(fx+0.5);
										ny= y+(int)(fy+0.5);
										if(nx>=0 && ny>=0 && nx<sizex && ny<sizey) {
											fp= vectorbuf+ 2*(ny*sizex+nx);
											
											if(fp[0]!=0.0 || fp[1]!=0.0) {
												fp[0]= 0.5*(fp[0]-fx);
												fp[1]= 0.5*(fp[1]-fy);
											}
											else {
												fp[0]= -fx;
												fp[1]= -fy;
											}
										}
										cp2= work2+ sizex*ny+nx;
										*cp2|= 128;
									}
									
								}
							}
						}
					}
				}
			}
		}
	}
}

void filtervectors_hor(vectorbuf)
float *vectorbuf;
{
	float *fp, *tempf, *fpt, div;
	int x, y;
	
	tempf= callocN(8*sizex, "filt");
	fp= vectorbuf;
	for(y=0; y<sizey; y++) {
		fpt= tempf+2;
		for(x=0; x<sizex-2; x++, fp+=2, fpt+=2) {

			fpt[0]= fp[0]+2.0*fp[2]+fp[4];
			fpt[1]= fp[1]+2.0*fp[3]+fp[5];
			
			fpt[0]*= 0.25;			
			fpt[1]*= 0.25;			

		}
		fp+= 4;
		memcpy(fp-2*sizex, tempf, 8*sizex);
	}
	freeN(tempf);
}

void filtervectors_vert(vectorbuf)
float *vectorbuf;
{
	float *fp, *tempf, *fpt, div;
	int x, y;
	
	tempf= callocN(8*sizex, "filt");

	for(x=0; x<sizex; x++) {
		fpt= tempf+2;
		fp= vectorbuf+2*x;
		for(y=0; y<sizey-2; y++, fp+=2*sizex, fpt+=2) {

			fpt[0]= fp[0]+2.0*fp[2*sizex]+fp[4*sizex];
			fpt[1]= fp[1]+2.0*fp[2*sizex+1]+fp[4*sizex+1];
			
			fpt[0]*= 0.25;			
			fpt[1]*= 0.25;			
		}
		fpt= tempf;
		fp= vectorbuf+2*x;
		for(y=0; y<sizey; y++, fp+=2*sizex, fpt+=2) {
			fp[0]= fpt[0];
			fp[1]= fpt[1];
		}
	}
	freeN(tempf);
}

void accumfrombuf(dx, dy, count, fp, w)
float dx, dy;
char *count;
float *fp;
int w;
{

	if(dx==0.0 && dy==0.0) return;
	
	if(*count<250) {
		fp[0]-= w*dx;
		fp[1]-= w*dy;
		(*count)+= w;
	}
}

void fillscanfrombuf(fp, nr, valx, valy, count)
float *fp;
short nr;
float valx, valy;
char *count;
{


	while(nr >0) {
		fp[0]= valx;
		fp[1]= valy;
		*count |= 2;
		count++;
		fp+= 2;
		nr--;
	}
}

void interpolscanfrombuf(fp, nr, count)
float *fp;
int nr;
char *count;
{
	float sx, sy, ex, ey, d, fac;
	
	sx= fp[-2];
	sy= fp[-1];
	ex= fp[2*nr];
	ey= fp[2*nr+1];
	
	fac=d= 1.0/(float)(nr+1);
	
	/* op fp-2 staat startwaarde, op fp+nr staat eindwaarde */
	
	while(nr>0) {
		fp[0]= sx+ fac*(ex-sx);
		fp[1]= sy+ fac*(ey-sy);
		
		*count |= 2;
		count++;
		fac+= d;
		fp+=2;
		nr--;
	}
	
}

void middelvectors(frombuf, countbuf, sum)
float *frombuf;
char *countbuf;
int sum;
{
	float *fp1, *fp2, *fp3, div, fx, fy;
	int x, y;
	char *cp1, *cp2, *cp3;
	
	fp1= frombuf;
	fp2= fp1+2*sizex;
	fp3= fp2+2*sizex;
	cp1= countbuf;
	cp2= cp1+sizex;
	cp3= cp2+sizex;
	
	for(y=1; y<sizey-1; y++) {
		for(x=1; x<sizex-1; x++) {
			if( ( (x+y) & 1 )==sum ) {
				if(cp2[1]==0) {
					div= fx= fy= 0.0;
					
					if(cp2[0]) { div+= 2.0; fx+= 2.0*fp2[0]; fy+= 2.0*fp2[1]; }
					if(cp2[2]) { div+= 2.0; fx+= 2.0*fp2[4]; fy+= 2.0*fp2[5]; }
					
					if(cp1[0]) { div+= 1.0; fx+= fp1[0]; fy+= fp1[1]; }
					if(cp1[1]) { div+= 2.0; fx+= 2.0*fp1[2]; fy+= 2*fp1[3]; }
					if(cp1[2]) { div+= 1.0; fx+= fp1[4]; fy+= fp1[5]; }
					
					if(cp3[0]) { div+= 1.0; fx+= fp3[0]; fy+= fp3[1]; }
					if(cp3[1]) { div+= 2.0; fx+= 2.0*fp3[2]; fy+= 2.0*fp3[3]; }
					if(cp3[2]) { div+= 1.0; fx+= fp3[4]; fy+= fp3[5]; }
					
					if(div!=0.0) {
						fp2[2]= fx/div;
						fp2[3]= fy/div;
						cp2[1]= 1;
					}
				}
			}
			fp1+=2; fp2+=2; fp3+=2;
			cp1++; cp2++; cp3++;
		}
		fp1+=4; fp2+=4; fp3+=4;
		cp1+=2; cp2+=2; cp3+=2;
	}
	
}

void middelvectors1(frombuf, countbuf)
float *frombuf;
char *countbuf;
{
	float *fp1, *fp2, *fp3, div, fx, fy;
	int x, y;
	char *cp1, *cp2, *cp3;
	
	fp1= frombuf;
	fp2= fp1+2*sizex;
	fp3= fp2+2*sizex;
	cp1= countbuf;
	cp2= cp1+sizex;
	cp3= cp2+sizex;
	
	for(y=1; y<sizey-1; y++) {
		for(x=1; x<sizex-1; x++) {
			if(cp2[1]==0) {
				div= fx= fy= 0.0;
				
				if(cp2[0]==1) { div+= 2.0; fx+= 2.0*fp2[0]; fy+= 2.0*fp2[1]; }
				if(cp2[2]==1) { div+= 2.0; fx+= 2.0*fp2[4]; fy+= 2.0*fp2[5]; }
				
				if(cp1[0]==1) { div+= 1.0; fx+= fp1[0]; fy+= fp1[1]; }
				if(cp1[1]==1) { div+= 2.0; fx+= 2.0*fp1[2]; fy+= 2*fp1[3]; }
				if(cp1[2]==1) { div+= 1.0; fx+= fp1[4]; fy+= fp1[5]; }
				
				if(cp3[0]==1) { div+= 1.0; fx+= fp3[0]; fy+= fp3[1]; }
				if(cp3[1]==1) { div+= 2.0; fx+= 2.0*fp3[2]; fy+= 2.0*fp3[3]; }
				if(cp3[2]==1) { div+= 1.0; fx+= fp3[4]; fy+= fp3[5]; }
				
				if(div!=0.0) {
					fp2[2]= fx/div;
					fp2[3]= fy/div;
					cp2[1]= 2;
				}
			}

			fp1+=2; fp2+=2; fp3+=2;
			cp1++; cp2++; cp3++;
		}
		fp1+=4; fp2+=4; fp3+=4;
		cp1+=2; cp2+=2; cp3+=2;
	}
	
	cp1= countbuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp1++) {
			if(*cp1 == 2) *cp1= 1;
		}
	}
}

void makefrombuf(vectorbuf, work, frombuf)
float *vectorbuf;
char *work;
float *frombuf;
{
	float dx, dy, *fp, *fp2, startvec[2], endvec[2];
	int x, y, xor, yor, nx, ny, start, end, loop;
	char *countbuf, *cp1, *cp2;

	countbuf= callocN(sizex*sizey, "makefrombuf");
	
	bzero(frombuf, 8*sizex*sizey);
	
	/* fromvectoren accumuleren met filter */
	cp1= work;
	fp= vectorbuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp1++, fp+=2) {
			if(cp1[0]==128) {
				
				dx= (fp[0]);
				dy= (fp[1]);

				nx= x+(int)(dx+0.5);
				ny= y+(int)(dy+0.5);
				
				/* de fromvec accumuleren */
				if(nx>=0 && ny>=0 && nx<sizex && ny<sizey) {
					cp2= countbuf+ny*sizex+nx;
					fp2= frombuf+2*(ny*sizex+nx);
				
					accumfrombuf(dx, dy, cp2, fp2, 8);
					
					if(nx>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 4);
						if(nx>1) accumfrombuf(dx, dy, cp2-2, fp2-4, 2);
					if(nx<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 4);
						if(nx<sizex-2) accumfrombuf(dx, dy, cp2+2, fp2+4, 2);
					
					if(ny>0) {
						cp2-= sizex;
						fp2-= 2*sizex;
						accumfrombuf(dx, dy, cp2, fp2, 4);
						if(nx>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 2);
						if(nx<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 2);
						cp2+= sizex;
						fp2+= 2*sizex;
					}
					if(ny>1) accumfrombuf(dx, dy, cp2-2*sizex, fp2-4*sizex, 2);
					
					if(ny<sizey-1) {
						cp2+= sizex;
						fp2+= 2*sizex;
						accumfrombuf(dx, dy, cp2, fp2, 4);
						if(nx>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 2);
						if(nx<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 2);
						cp2-= sizex;
						fp2-= 2*sizex;
					}
					if(ny<sizey-2) accumfrombuf(dx, dy, cp2+2*sizex, fp2+4*sizex, 2);
				}
				
				/* doen we ook, met minder gewicht de to-vec: */
				cp2= countbuf+y*sizex+x;
				fp2= frombuf+2*(y*sizex+x);
			
				accumfrombuf(dx, dy, cp2, fp2, 4);
				
				if(x>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 2);
				if(x<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 2);
				
				if(y>0) {
					cp2-= sizex;
					fp2-= 2*sizex;
					accumfrombuf(dx, dy, cp2, fp2, 2);
					if(nx>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 1);
					if(nx<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 1);
					cp2+= sizex;
					fp2+= 2*sizex;
				}

				if(y<sizey-1) {
					cp2+= sizex;
					fp2+= 2*sizex;
					accumfrombuf(dx, dy, cp2, fp2, 2);
					if(nx>0) accumfrombuf(dx, dy, cp2-1, fp2-2, 1);
					if(nx<sizex-1) accumfrombuf(dx, dy, cp2+1, fp2+2, 1);
				}
				
			}
		}
	}
	
	/* normaliseren */
	cp1= countbuf;
	fp= frombuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp1++, fp+=2) {
			if( *cp1 ) {
				fp[0]/= (float)*cp1;
				fp[1]/= (float)*cp1;
				*cp1= 1;
			}
		}
	}
	
	/* de omliggende pixels die niet zijn gezet worden gemiddeld */

	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	middelvectors1(frombuf, countbuf);
	
	middelvectors(frombuf, countbuf, 0);
	middelvectors(frombuf, countbuf, 1);
	
	/* extrapoleren in scanlines x */

	if(TRUE) {
		cp1= countbuf;
		fp= frombuf;
		for(y=0; y<sizey; y++) {
			
			/* loop scanline af tot eerste waarde is bereikt */
			start= 0;
			end= 0;
			while( (cp1[end] & 1 )==0) {
				end++;
				if(end==sizex) break;
			}
			if(end!=sizex) {
				fillscanfrombuf(fp+2, end, fp[2*end], fp[2*end+1], cp1);
				
				start= end;
				end= start+1;
				loop= 1;
				while(loop) {
					loop= 0;
					/* zoek naar de eerste "lege' pixel */
					
					while(start<sizex && (cp1[start] & 1 )!=0) start++;
					
					if(start<sizex) {
					
						/* op start-1 staat de startwaarde, nu zoeken naar eindwaarde */
						end= start+1;
						while(end<sizex && (cp1[end] & 1 )==0) end++;
						
						/* als eindwaarde laatste pixel is, vullen met startwaarde */
						if(end==sizex) fillscanfrombuf(fp+2*start, end-1-start, fp[2*start-2], fp[2*start-1], cp1+start);
						else {
							/* op end staat eindwaarde */	
							interpolscanfrombuf(fp+2*start, end-start, cp1+start);
							start= end;
							loop= 1;
						}
					}
				}
			}
			
			cp1+= sizex;
			fp+= 2*sizex;
		}
	}
	
	filtervectors_hor(frombuf);
	filtervectors_vert(frombuf);	

	/* drawfrombuf(frombuf); */
	/* exit(0); */

	freeN(countbuf);
}

void applyfrombuf(frombuf, ibuf, obuf, fac)
float *frombuf;
struct ImBuf *ibuf, *obuf;
float fac;
{
	float *fp, tx, ty;
	int x, y, nx, ny;
	ulong *ri, *ro;
	
	tx= fac*transx;
	ty= fac*transy;
	
	ro= obuf->rect;
	fp= frombuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, ro++, fp+=2) {
			nx= x+ (int)(fac*fp[0]-tx+0.5);
			ny= y+ (int)(fac*fp[1]-ty+0.5);

			if(nx<0) nx=0;
			else if(nx>=sizex) nx= sizex-1;
			if(ny<0) ny=0;
			else if(ny>=sizey) ny= sizey-1;
			
			ri= ibuf->rect+ ny*sizex+nx;
			*ro= *ri;
		}
	}
	
}

void blendbufs(orect, arect, fac)
ulong *orect, *arect;
float fac;
{
	int f1, f2, x, y;
	char *cp1, *cp2;
	
	/* orect= a*orect+ (1-a)*arect */
	
	f1= 255.5*fac;
	f2= 255-f1;
	
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, orect++, arect++) {
			if(*orect != *arect) {
				cp1= (char *)orect;
				cp2= (char *)arect;
				
				cp1[1]= (f1*cp1[1]+f2*cp2[1])>>8;
				cp1[2]= (f1*cp1[2]+f2*cp2[2])>>8;
				cp1[3]= (f1*cp1[3]+f2*cp2[3])>>8;
			}
		}
	}
}

void invertvectorbuf(buf, vectorbuf)
float *buf, *vectorbuf;
{
	float *fp1, *fp2;
	int x, y;
	
	fp1= vectorbuf;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, fp1+=2) {
		}
	}
}

main(argc,argv)
long argc;
uchar **argv;
{
	struct ImBuf *ibuf1, *ibuf2, *obuf, *abuf;
	float *vectorbuf, *frombuf;
	int a, tweens=5, ofs;
	uchar *work1, *work2;
	uchar name[128];

	if (argc >= 3) {
		ibuf1 = loadiffname(argv[1], LI_rect);

		ibuf2 = loadiffname(argv[2],LI_rect);
		
		if(argc>3) ofs= atoi(argv[3]);
	}
	else {
		printf("usage: inbetween image1 image2\n");
		exit(0);
	}
	
	if(ibuf1->x!=ibuf2->x || ibuf1->y!=ibuf2->y) {
		printf("Error: different image size\n");
		exit(0);
	}
	

	ibuf1->ftype= TGA;
	freecmapImBuf(ibuf1);
	ibuf1->depth= 24;
	ibuf2->ftype= TGA;
	freecmapImBuf(ibuf2);
	ibuf2->depth= 24;
	obuf= dupImBuf(ibuf1);
	
	sizex= ibuf1->x;
	sizey= ibuf1->y;
		
	work1= mallocN(ibuf1->x*ibuf1->y, "work1");
	work2= mallocN(ibuf1->x*ibuf1->y, "work2");
	vectorbuf= mallocN(8*ibuf1->x*ibuf1->y, "vector");
	frombuf= mallocN(8*ibuf1->x*ibuf1->y, "from");
	
	initworkrect(work1, ibuf1);		/* edgedetectie */
	initworkrect(work2, ibuf2);
	
	find_main_translate(work1, work2);


	/* van plaatje 1 naar plaatje 2  *************** */
	
	translatework(work1);
	
/* goto inspring; */

	makevectorbuf(vectorbuf, work1, work2, 0);
	
/* drawvectorbuf(vectorbuf, work1, work2); */
/* exit(0); */

	makefrombuf(vectorbuf, work1, frombuf);

	for(a=1; a<tweens; a++) {

		applyfrombuf(frombuf, ibuf1, obuf, ((float)a)/(float)tweens);

		if(a+ofs<10) sprintf(name, "/pics/test/inbetw/000%d", a+ofs);
		else if(a+ofs<100) sprintf(name, "/pics/test/inbetw/00%d", a+ofs);
		else sprintf(name, "/pics/test/inbetw/0%d", a+ofs);

		saveiff(obuf, name, SI_rect);
		printf("saved %s\n", name);
	}

	/* van plaatje 2 naar plaatje 1 *************** */

inspring:

	transx= -transx;
	transy= -transy;

	translatework(work1);
	translatework(work2);


	makevectorbuf(vectorbuf, work2, work1, 0);
	
/* drawvectorbuf(vectorbuf, work2, work1); */
/* exit(0); */

	makefrombuf(vectorbuf, work2, frombuf);
	
	for(a=1; a<tweens; a++) {
		
		applyfrombuf(frombuf, ibuf2, obuf, ((float)(tweens-a))/(float)tweens);
		
		if(a+ofs<10) sprintf(name, "/pics/test/inbetw/000%d", a+ofs);
		else if(a+ofs<100) sprintf(name, "/pics/test/inbetw/00%d", a+ofs);
		else sprintf(name, "/pics/test/inbetw/0%d", a+ofs);

		abuf= loadiffname(name, LI_rect);
		
		blendbufs(obuf->rect, abuf->rect, ((float)a)/(float)tweens);
		freeImBuf(abuf);
		
		saveiff(obuf, name, SI_rect);
		printf("saved %s\n", name);
	}
	
	if(a+ofs<10) sprintf(name, "/pics/test/inbetw/000%d", a+ofs);
	else if(a+ofs<100) sprintf(name, "/pics/test/inbetw/00%d", a+ofs);
	else sprintf(name, "/pics/test/inbetw/0%d", a+ofs);
	saveiff(ibuf2, name, SI_rect);

	
	freeN(work1);
	freeN(work2);
	freeN(vectorbuf);
	freeN(frombuf);
}

