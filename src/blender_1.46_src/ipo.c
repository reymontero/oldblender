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

/* ipo.c  		MIXED MODEL

 * 
 * jan 95
 * 
 */

#include "blender.h"
#include "ipo.h"
#include "render.h"
#include "edit.h"
#include "ika.h"
#include "sector.h"
#include "sequence.h"

#define SMALL -1.0e-10

/* Dit array is ervoor zodat defines zoals OB_LOC_X niet persee 0 hoeft te zijn.
   Ook voor toekomstige backward compatibility.
   Zo kan met een for-next lus alles worden afgelopen */

int ob_ar[OB_TOTIPO]= {
	OB_LOC_X, OB_LOC_Y, OB_LOC_Z, OB_DLOC_X, OB_DLOC_Y, OB_DLOC_Z, 
	OB_ROT_X, OB_ROT_Y, OB_ROT_Z, OB_DROT_X, OB_DROT_Y, OB_DROT_Z, 
	OB_SIZE_X, OB_SIZE_Y, OB_SIZE_Z, OB_DSIZE_X, OB_DSIZE_Y, OB_DSIZE_Z, 
	OB_LAY, OB_SPEED, OB_EFF_X, OB_EFF_Y, OB_EFF_Z
};

int ma_ar[MA_TOTIPO]= {
	MA_COL_R, MA_COL_G, MA_COL_B, 
	MA_SPEC_R, MA_SPEC_G, MA_SPEC_B, 
	MA_MIR_R, MA_MIR_G, MA_MIR_B, 
	MA_REF, MA_ALPHA, MA_EMIT, MA_AMB, 
	MA_SPEC, MA_HARD, MA_SPTR, MA_ANG, 
	MA_MODE, MA_HASIZE, 
	
	MA_MAP1+MAP_OFS_X, MA_MAP1+MAP_OFS_Y, MA_MAP1+MAP_OFS_Z, 
	MA_MAP1+MAP_SIZE_X, MA_MAP1+MAP_SIZE_Y, MA_MAP1+MAP_SIZE_Z, 
	MA_MAP1+MAP_R, MA_MAP1+MAP_G, MA_MAP1+MAP_B,
	MA_MAP1+MAP_DVAR, MA_MAP1+MAP_COLF, MA_MAP1+MAP_NORF, MA_MAP1+MAP_VARF
};

int seq_ar[SEQ_TOTIPO]= {
	SEQ_FAC1
};

int cu_ar[CU_TOTIPO]= {
	CU_SPEED
};

int key_ar[KEY_TOTIPO]= {
	KEY_SPEED, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
	21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

int wo_ar[WO_TOTIPO]= {
	WO_HOR_R, WO_HOR_G, WO_HOR_B, WO_ZEN_R, WO_ZEN_G, WO_ZEN_B, 
	WO_EXPOS, WO_MISI, WO_MISTDI, WO_MISTSTA, WO_MISTHI,
	WO_STAR_R, WO_STAR_G, WO_STAR_B, WO_STARDIST, WO_STARSIZE, 

	MA_MAP1+MAP_OFS_X, MA_MAP1+MAP_OFS_Y, MA_MAP1+MAP_OFS_Z, 
	MA_MAP1+MAP_SIZE_X, MA_MAP1+MAP_SIZE_Y, MA_MAP1+MAP_SIZE_Z, 
	MA_MAP1+MAP_R, MA_MAP1+MAP_G, MA_MAP1+MAP_B,
	MA_MAP1+MAP_DVAR, MA_MAP1+MAP_COLF, MA_MAP1+MAP_NORF, MA_MAP1+MAP_VARF
};

int la_ar[LA_TOTIPO]= {
	LA_ENERGY,  LA_COL_R, LA_COL_G,  LA_COL_B, 
	LA_DIST, LA_SPOTSI, LA_SPOTBL, 
	LA_QUAD1,  LA_QUAD2,  LA_HALOINT,  

	MA_MAP1+MAP_OFS_X, MA_MAP1+MAP_OFS_Y, MA_MAP1+MAP_OFS_Z, 
	MA_MAP1+MAP_SIZE_X, MA_MAP1+MAP_SIZE_Y, MA_MAP1+MAP_SIZE_Z, 
	MA_MAP1+MAP_R, MA_MAP1+MAP_G, MA_MAP1+MAP_B,
	MA_MAP1+MAP_DVAR, MA_MAP1+MAP_COLF, MA_MAP1+MAP_NORF, MA_MAP1+MAP_VARF
};



float frame_to_float(int cfra)		/* zie ook system_time in object.c */
{
	float ctime;
	
	ctime= (float)cfra;
	if(R.flag & R_SEC_FIELD) {
		if((R.r.mode & R_FIELDSTILL)==0) ctime+= 0.5;
	}
	ctime*= G.scene->r.framelen;
	
	return ctime;
}

/* niet ipo zelf vrijgeven */
void free_ipo(Ipo *ipo)
{
	IpoCurve *icu;
	
	icu= ipo->curve.first;
	while(icu) {
		if(icu->bezt) freeN(icu->bezt);
		if(icu->bp) freeN(icu->bp);
		icu= icu->next;
	}
	freelistN(&ipo->curve);
}

Ipo *add_ipo(char *name, int idcode)
{
	Ipo *ipo;
	
	ipo= alloc_libblock(&G.main->ipo, ID_IP, name);
	ipo->blocktype= idcode;
	
	return ipo;
}

Ipo *copy_ipo(Ipo *ipo)
{
	Ipo *ipon;
	IpoCurve *icu;
	
	if(ipo==0) return 0;
	
	ipon= copy_libblock(ipo);
	
	duplicatelist(&(ipon->curve), &(ipo->curve));
	
	icu= ipon->curve.first;
	while(icu) {
		icu->bp= dupallocN(icu->bp);
		icu->bezt= dupallocN(icu->bezt);
		icu= icu->next;
	}
	
	return ipon;
}

void make_local_obipo(Ipo *ipo)
{
	Object *ob;
	Ipo *ipon;
	int local=0, lib=0;
	
	/* - zijn er alleen lib users: niet doen
	 * - zijn er alleen locale users: flag zetten
	 * - mixed: copy
	 */

	ob= G.main->object.first;
	while(ob) {
		if(ob->ipo==ipo) {
			if(ob->id.lib) lib= 1;
			else local= 1;
		}
		ob= ob->id.next;
	}
	
	if(local && lib==0) {
		ipo->id.lib= 0;
		ipo->id.flag= LIB_LOCAL;
		new_id(0, ipo, 0);
	}
	else if(local && lib) {
		ipon= copy_ipo(ipo);
		ipon->id.us= 0;
		
		ob= G.main->object.first;
		while(ob) {
			if(ob->ipo==ipo) {
				
				if(ob->id.lib==0) {
					ob->ipo= ipon;
					ipon->id.us++;
					ipo->id.us--;
				}
			}
			ob= ob->id.next;
		}
	}
}

void make_local_matipo(Ipo *ipo)
{
	Material *ma;
	Ipo *ipon;
	int local=0, lib=0;
	
	/* - zijn er alleen lib users: niet doen
	 * - zijn er alleen locale users: flag zetten
	 * - mixed: copy
	 */

	ma= G.main->mat.first;
	while(ma) {
		if(ma->ipo==ipo) {
			if(ma->id.lib) lib= 1;
			else local= 1;
		}
		ma= ma->id.next;
	}
	
	if(local && lib==0) {
		ipo->id.lib= 0;
		ipo->id.flag= LIB_LOCAL;
		new_id(0, ipo, 0);
	}
	else if(local && lib) {
		ipon= copy_ipo(ipo);
		ipon->id.us= 0;
		
		ma= G.main->mat.first;
		while(ma) {
			if(ma->ipo==ipo) {
				
				if(ma->id.lib==0) {
					ma->ipo= ipon;
					ipon->id.us++;
					ipo->id.us--;
				}
			}
			ma= ma->id.next;
		}
	}
}

void make_local_keyipo(Ipo *ipo)
{
	Key *key;
	Ipo *ipon;
	int local=0, lib=0;
	
	/* - zijn er alleen lib users: niet doen
	 * - zijn er alleen locale users: flag zetten
	 * - mixed: copy
	 */

	key= G.main->key.first;
	while(key) {
		if(key->ipo==ipo) {
			if(key->id.lib) lib= 1;
			else local= 1;
		}
		key= key->id.next;
	}
	
	if(local && lib==0) {
		ipo->id.lib= 0;
		ipo->id.flag= LIB_LOCAL;
		new_id(0, ipo, 0);
	}
	else if(local && lib) {
		ipon= copy_ipo(ipo);
		ipon->id.us= 0;
		
		key= G.main->key.first;
		while(key) {
			if(key->ipo==ipo) {
				
				if(key->id.lib==0) {
					key->ipo= ipon;
					ipon->id.us++;
					ipo->id.us--;
				}
			}
			key= key->id.next;
		}
	}
}


void make_local_ipo(Ipo *ipo)
{
	
	if(ipo->id.lib==0) return;
	if(ipo->id.us==1) {
		ipo->id.lib= 0;
		ipo->id.flag= LIB_LOCAL;
		new_id(0, ipo, 0);
		return;
	}
	
	if(ipo->blocktype==ID_OB) make_local_obipo(ipo);
	else if(ipo->blocktype==ID_MA) make_local_matipo(ipo);
	else if(ipo->blocktype==ID_KE) make_local_keyipo(ipo);
	
}


void calchandles_ipocurve(IpoCurve *icu)
{
	BezTriple *bezt, *prev, *next;
	int a;

	a= icu->totvert;
	if(a<2) return;
	
	bezt= icu->bezt;
	prev= 0;
	next= bezt+1;

	while(a--) {

		if(bezt->vec[0][0]>bezt->vec[1][0]) bezt->vec[0][0]= bezt->vec[1][0];
		if(bezt->vec[2][0]<bezt->vec[1][0]) bezt->vec[2][0]= bezt->vec[1][0];

		calchandleNurb(bezt, prev, next, 1);	/* 1==speciale autohandle */

		prev= bezt;
		if(a==1) {
			next= 0;
		}
		else next++;
			
		/* voor automatische ease in en out */
		if(bezt->h1==HD_AUTO && bezt->h2==HD_AUTO) {
			if(a==0 || a==icu->totvert-1) {
				if(icu->extrap==IPO_HORIZ) {
					bezt->vec[0][1]= bezt->vec[2][1]= bezt->vec[1][1];
				}
			}
		}
		
		bezt++;
	}
}

void testhandles_ipocurve(IpoCurve *icu)
{
	/* Te gebruiken als er iets aan de handles is veranderd.
	 * Loopt alle BezTriples af met de volgende regels:
     * FASE 1: types veranderen?
     *  Autocalchandles: worden ligned als NOT(000 || 111)
     *  Vectorhandles worden 'niets' als (selected en andere niet) 
     * FASE 2: handles herberekenen
     */
	BezTriple *bezt;
	int flag, a;

	bezt= icu->bezt;
	if(bezt==0) return;
	
	a= icu->totvert;
	while(a--) {
		flag= 0;
		if(bezt->f1 & 1) flag++;
		if(bezt->f2 & 1) flag += 2;
		if(bezt->f3 & 1) flag += 4;

		if( !(flag==0 || flag==7) ) {
			if(bezt->h1==HD_AUTO) {   /* auto */
				bezt->h1= HD_ALIGN;
			}
			if(bezt->h2==HD_AUTO) {   /* auto */
				bezt->h2= HD_ALIGN;
			}

			if(bezt->h1==HD_VECT) {   /* vector */
				if(flag < 4) bezt->h1= 0;
			}
			if(bezt->h2==HD_VECT) {   /* vector */
				if( flag > 3) bezt->h2= 0;
			}
		}
		bezt++;
	}

	calchandles_ipocurve(icu);
}


int sort_time_ipocurve(IpoCurve *icu)
{
	BezTriple *bezt;
	BPoint *bp;
	int a, ok= 1;
	
	while(ok) {
		ok= 0;

		if(icu->bezt) {
			bezt= icu->bezt;
			a= icu->totvert;
			while(a--) {
				if(a>0) {
					if( bezt->vec[1][0] > (bezt+1)->vec[1][0]) {
						SWAP(BezTriple, *bezt, *(bezt+1));
						ok= 1;
					}
				}
				if(bezt->vec[0][0]>=bezt->vec[1][0] && bezt->vec[2][0]<=bezt->vec[1][0]) {
					SWAP(float, bezt->vec[0][0], bezt->vec[2][0]);
					SWAP(float, bezt->vec[0][1], bezt->vec[2][1]);
				}
				else {
					if(bezt->vec[0][0]>bezt->vec[1][0]) bezt->vec[0][0]= bezt->vec[1][0];
					if(bezt->vec[2][0]<bezt->vec[1][0]) bezt->vec[2][0]= bezt->vec[1][0];
				}
				bezt++;
			}
		}
		else {
			
		}
	}
}

int test_time_ipocurve(IpoCurve *icu)
{
	BezTriple *bezt;
	BPoint *bp;
	int a, ok= 1;
	
	if(icu->bezt) {
		bezt= icu->bezt;
		a= icu->totvert-1;
		while(a--) {
			if( bezt->vec[1][0] > (bezt+1)->vec[1][0]) {
				return 1;
			}
			bezt++;
		}	
	}
	else {
		
	}

	return 0;
}

void correct_bezpart(float *v1, float *v2, float *v3, float *v4)
{
	/* de totale lengte van de handles mag niet langer zijn
	 * dan de horizontale afstand tussen de punten (v1-v4)
	 */
	float h1[2], h2[2], len1, len2, len, fac;
	
	h1[0]= v1[0]-v2[0];
	h1[1]= v1[1]-v2[1];
	h2[0]= v4[0]-v3[0];
	h2[1]= v4[1]-v3[1];
	
	len= v4[0]- v1[0];
	len1= fabs(h1[0]);
	len2= fabs(h2[0]);
	
	if(len1+len2==0.0) return;
	if(len1+len2 > len) {
		fac= len/(len1+len2);
		
		v2[0]= (v1[0]-fac*h1[0]);
		v2[1]= (v1[1]-fac*h1[1]);
		
		v3[0]= (v4[0]-fac*h2[0]);
		v3[1]= (v4[1]-fac*h2[1]);
		
	}
}

/* *********************** ARITH *********************** */

int findzero(x, q0, q1, q2, q3, o)
float x, q0, q1, q2, q3, *o;
{
	double c0, c1, c2, c3, a, b, c, p, q, d, t, phi;
	int nr= 0;

	c0= q0-x;
	c1= 3*(q1-q0);
	c2= 3*(q0-2*q1+q2);
	c3= q3-q0+3*(q1-q2);
	
	if(c3!=0.0) {
		a= c2/c3;
		b= c1/c3;
		c= c0/c3;
		a= a/3;

		p= b/3-a*a;
		q= (2*a*a*a-a*b+c)/2;
		d= q*q+p*p*p;

		if(d>0.0) {
			t= sqrt(d);
			o[0]= Sqrt3d(-q+t)+Sqrt3d(-q-t)-a;
			if(o[0]>= SMALL && o[0]<=1.000001) return 1;
			else return 0;
		}
		else if(d==0.0) {
			t= Sqrt3d(-q);
			o[0]= 2*t-a;
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= -t-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		}
		else {
			phi= acos(-q/sqrt(-(p*p*p)));
			t= sqrt(-p);
			p= cos(phi/3);
			q= sqrt(3-3*p*p);
			o[0]= 2*t*p-a;
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= -t*(p+q)-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) nr++;
			o[nr]= -t*(p-q)-a;
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		}
	}
	else {
		a=c2;
		b=c1;
		c=c0;
		
		if(a!=0.0) {
			p=b*b-4*a*c;
			if(p>0) {
				p= sqrt(p);
				o[0]= (-b-p)/(2*a);
				if(o[0]>=SMALL && o[0]<=1.000001) nr++;
				o[nr]= (-b+p)/(2*a);
				if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
				else return nr;
			}
			else if(p==0) {
				o[0]= -b/(2*a);
				if(o[0]>=SMALL && o[0]<=1.000001) return 1;
				else return 0;
			}
		}
		else if(b!=0.0) {
			o[0]= -c/b;
			if(o[0]>=SMALL && o[0]<=1.000001) return 1;
			else return 0;
		}
		else if(c==0.0) {
			o[0]= 0.0;
			return 1;
		}
		return 0;	
	}
}

void berekeny(f1, f2, f3, f4, o, b)
float f1, f2, f3, f4, *o;
int b;
{
	float t, c0, c1, c2, c3;
	int a;

	c0= f1;
	c1= 3.0*(f2 - f1);
	c2= 3.0*(f1 - 2.0*f2 + f3);
	c3= f4 - f1 + 3.0*(f2-f3);
	
	for(a=0; a<b; a++) {
		t= o[a];
		o[a]= c0+t*c1+t*t*c2+t*t*t*c3;
	}
}
void berekenx(f, o, b)
float *f,*o;
int b;
{
	float t, c0, c1, c2, c3;
	int a;

	c0= f[0];
	c1= 3*(f[3]-f[0]);
	c2= 3*(f[0]-2*f[3]+f[6]);
	c3= f[9]-f[0]+3*(f[3]-f[6]);
	for(a=0; a<b; a++) {
		t= o[a];
		o[a]= c0+t*c1+t*t*c2+t*t*t*c3;
	}
}

void calc_icu(IpoCurve *icu, float ctime)
{
	BezTriple *bezt, *prevbezt;
	BPoint *bp, *prevbp;
	float v1[2], v2[2], v3[2], v4[2], opl[32], dx, fac;
	float cycdx, cycdy, ofs, cycyofs, ipotime;
	int a, b;

	ipotime= ctime;
	icu->curval= ipotime;
	
	cycyofs= 0.0;
	
	if(icu->bezt) {
		prevbezt= icu->bezt;
		bezt= prevbezt+1;
		a= icu->totvert-1;
		
		/* cyclic? */
		if(icu->extrap & IPO_CYCL) {
			ofs= icu->bezt->vec[1][0];
			cycdx= (icu->bezt+icu->totvert-1)->vec[1][0] - ofs;
			cycdy= (icu->bezt+icu->totvert-1)->vec[1][1] - icu->bezt->vec[1][1];
			if(cycdx!=0.0) {
				
				if(icu->extrap & IPO_DIR) {
					cycyofs= ffloor((ipotime-ofs)/cycdx);
					cycyofs*= cycdy;
				}

				ipotime= fmodf(ipotime-ofs, cycdx)+ofs;
				if(ipotime<ofs) ipotime+= cycdx;
			}
		}
		
		/* uiteinden? */
	
		if(prevbezt->vec[1][0]>=ipotime) {
			if( (icu->extrap & IPO_DIR) && icu->ipo!=IPO_CONST) {
				dx= prevbezt->vec[1][0]-ipotime;
				fac= prevbezt->vec[1][0]-prevbezt->vec[0][0];
				if(fac!=0.0) {
					fac= (prevbezt->vec[1][1]-prevbezt->vec[0][1])/fac;
					icu->curval= prevbezt->vec[1][1]-fac*dx;
				}
				else icu->curval= prevbezt->vec[1][1];
			}
			else icu->curval= prevbezt->vec[1][1];
			
			icu->curval+= cycyofs;
		}
		else if( (prevbezt+a)->vec[1][0]<=ipotime) {
			if( (icu->extrap & IPO_DIR) && icu->ipo!=IPO_CONST) {
				prevbezt+= a;
				dx= ipotime-prevbezt->vec[1][0];
				fac= prevbezt->vec[2][0]-prevbezt->vec[1][0];
				
				if(fac!=0) {
					fac= (prevbezt->vec[2][1]-prevbezt->vec[1][1])/fac;
					icu->curval= prevbezt->vec[1][1]+fac*dx;
				}
				else icu->curval= prevbezt->vec[1][1];
			}
			else icu->curval= (prevbezt+a)->vec[1][1];
			
			icu->curval+= cycyofs;
		}
		else {
			while(a--) {
				if(prevbezt->vec[1][0]<=ipotime && bezt->vec[1][0]>=ipotime) {
					if(icu->ipo==IPO_CONST) {
						icu->curval= prevbezt->vec[1][1]+cycyofs;
					}
					else if(icu->ipo==IPO_LIN) {
						fac= bezt->vec[1][0]-prevbezt->vec[1][0];
						if(fac==0) icu->curval= cycyofs+prevbezt->vec[1][1];
						else {
							fac= (ipotime-prevbezt->vec[1][0])/fac;
							icu->curval= cycyofs+prevbezt->vec[1][1]+ fac*(bezt->vec[1][1]-prevbezt->vec[1][1]);
						}
					}
					else {
						v1[0]= prevbezt->vec[1][0];
						v1[1]= prevbezt->vec[1][1];
						v2[0]= prevbezt->vec[2][0];
						v2[1]= prevbezt->vec[2][1];
						
						v3[0]= bezt->vec[0][0];
						v3[1]= bezt->vec[0][1];
						v4[0]= bezt->vec[1][0];
						v4[1]= bezt->vec[1][1];

						correct_bezpart(v1, v2, v3, v4);
						
						b= findzero(ipotime, v1[0], v2[0], v3[0], v4[0], opl);
						if(b) {
							berekeny(v1[1], v2[1], v3[1], v4[1], opl, 1);
							icu->curval= opl[0]+cycyofs;
							break;
						}
					}
				}
				prevbezt= bezt;
				bezt++;
			}
		}
	}

	if(icu->ymin < icu->ymax) {
		if(icu->curval < icu->ymin) icu->curval= icu->ymin;
		else if(icu->curval > icu->ymax) icu->curval= icu->ymax;
	}
}

void calc_ipo(Ipo *ipo, float ctime)
{
	IpoCurve *icu;

	icu= ipo->curve.first;
	while(icu) {
		
		if( (icu->flag & IPO_LOCK)==0) calc_icu(icu, ctime);
		
		icu= icu->next;
	}
}

/* ************************************** */
/*		DO THE IPO!						  */
/* ************************************** */

void write_ipo_poin(void *poin, int type, float val)
{

	switch(type) {
	case IPO_FLOAT:
		*( (float *)poin)= val;
		break;
	case IPO_FLOAT_DEGR:
		*( (float *)poin)= val*M_PI_2/9.0;
		break;
	case IPO_INT:
	case IPO_INT_BIT:
	case IPO_LONG:
		*( (int *)poin)= (int)val;
		break;
	case IPO_SHORT:
	case IPO_SHORT_BIT:
		*( (short *)poin)= (short)val;
		break;
	case IPO_CHAR:
	case IPO_CHAR_BIT:
		*( (char *)poin)= (char)val;
		break;
	}
}

float read_ipo_poin(void *poin, int type)
{
	float val;
	
	switch(type) {
	case IPO_FLOAT:
		val= *( (float *)poin);
		break;
	case IPO_FLOAT_DEGR:
		val= *( (float *)poin);
		val /= (M_PI_2/9.0);
		break;
	case IPO_INT:
	case IPO_INT_BIT:
	case IPO_LONG:
		val= *( (int *)poin);
		break;
	case IPO_SHORT:
	case IPO_SHORT_BIT:
		val= *( (short *)poin);
		break;
	case IPO_CHAR:
	case IPO_CHAR_BIT:
		val= *( (char *)poin);
		break;
	}
	return val;
}

void *give_mtex_poin(MTex *mtex, int adrcode )
{
	void *poin=0;
		
	switch(adrcode) {
	case MAP_OFS_X:
		poin= &(mtex->ofs[0]); break;
	case MAP_OFS_Y:
		poin= &(mtex->ofs[1]); break;
	case MAP_OFS_Z:
		poin= &(mtex->ofs[2]); break;
	case MAP_SIZE_X:
		poin= &(mtex->size[0]); break;
	case MAP_SIZE_Y:
		poin= &(mtex->size[1]); break;
	case MAP_SIZE_Z:
		poin= &(mtex->size[2]); break;
	case MAP_R:
		poin= &(mtex->r); break;
	case MAP_G:
		poin= &(mtex->g); break;
	case MAP_B:
		poin= &(mtex->b); break;
	case MAP_DVAR:
		poin= &(mtex->def_var); break;
	case MAP_COLF:
		poin= &(mtex->colfac); break;
	case MAP_NORF:
		poin= &(mtex->norfac); break;
	case MAP_VARF:
		poin= &(mtex->varfac); break;
	}
	
	return poin;
}


void *get_ipo_poin(ID *id, IpoCurve *icu, int *type)
{
	void *poin= 0;
	Object *ob;
	Material *ma;
	MTex *mtex;
	Ika *ika= 0;
	Sector *se=0;
	Life *lf=0;
	Lamp *la;
	Sequence *seq;
	World *wo;
	
	*type= IPO_FLOAT;

	if( GS(id->name)==ID_OB) {
		
		ob= (Object *)id;
		
		if(ob->type==OB_IKA) ika= ob->data;
		else if(ob->type==OB_SECTOR) se= ob->data;
		else if(ob->type==OB_LIFE) lf= ob->data;
		
		switch(icu->adrcode) {
		case OB_LOC_X:
			poin= &(ob->loc[0]); break;
		case OB_LOC_Y:
			poin= &(ob->loc[1]); break;
		case OB_LOC_Z:
			poin= &(ob->loc[2]); break;
		case OB_DLOC_X:
			poin= &(ob->dloc[0]); break;
		case OB_DLOC_Y:
			poin= &(ob->dloc[1]); break;
		case OB_DLOC_Z:
			poin= &(ob->dloc[2]); break;
	
		case OB_ROT_X:
			poin= &(ob->rot[0]); *type= IPO_FLOAT_DEGR; break;
		case OB_ROT_Y:
			poin= &(ob->rot[1]); *type= IPO_FLOAT_DEGR; break;
		case OB_ROT_Z:
			poin= &(ob->rot[2]); *type= IPO_FLOAT_DEGR; break;
		case OB_DROT_X:
			poin= &(ob->drot[0]); *type= IPO_FLOAT_DEGR; break;
		case OB_DROT_Y:
			poin= &(ob->drot[1]); *type= IPO_FLOAT_DEGR; break;
		case OB_DROT_Z:
			poin= &(ob->drot[2]); *type= IPO_FLOAT_DEGR; break;
			
		case OB_SIZE_X:
			poin= &(ob->size[0]); break;
		case OB_SIZE_Y:
			poin= &(ob->size[1]); break;
		case OB_SIZE_Z:
			poin= &(ob->size[2]); break;
		case OB_DSIZE_X:
			poin= &(ob->dsize[0]); break;
		case OB_DSIZE_Y:
			poin= &(ob->dsize[1]); break;
		case OB_DSIZE_Z:
			poin= &(ob->dsize[2]); break;
		
		case OB_LAY:
			poin= &(ob->lay); *type= IPO_INT_BIT; break;
			
		case OB_EFF_X:	/* OB_COL_R */
			if(ika) poin= &(ika->effg[0]);
			if(se) poin= &(se->r);
			if(lf) poin= &(lf->r);
			break;
		case OB_EFF_Y:	/* OB_COL_G */
			if(ika) poin= &(ika->effg[1]);
			if(se) poin= &(se->g);
			if(lf) poin= &(lf->g);
			break;
		case OB_EFF_Z:	/* OB_COL_B */
			if(ika) poin= &(ika->effg[2]);
			if(se) poin= &(se->b);
			if(lf) poin= &(lf->b);
			break;
		}
	}
	else if( GS(id->name)==ID_MA) {
		
		ma= (Material *)id;
		
		switch(icu->adrcode) {
		case MA_COL_R:
			poin= &(ma->r); break;
		case MA_COL_G:
			poin= &(ma->g); break;
		case MA_COL_B:
			poin= &(ma->b); break;
		case MA_SPEC_R:
			poin= &(ma->specr); break;
		case MA_SPEC_G:
			poin= &(ma->specg); break;
		case MA_SPEC_B:
			poin= &(ma->specb); break;
		case MA_MIR_R:
			poin= &(ma->mirr); break;
		case MA_MIR_G:
			poin= &(ma->mirg); break;
		case MA_MIR_B:
			poin= &(ma->mirb); break;
		case MA_REF:
			poin= &(ma->ref); break;
		case MA_ALPHA:
			poin= &(ma->alpha); break;
		case MA_EMIT:
			poin= &(ma->emit); break;
		case MA_AMB:
			poin= &(ma->amb); break;
		case MA_SPEC:
			poin= &(ma->spec); break;
		case MA_HARD:
			poin= &(ma->har); *type= IPO_SHORT; break;
		case MA_SPTR:
			poin= &(ma->spectra); break;
		case MA_ANG:
			poin= &(ma->ang); break;
		case MA_MODE:
			poin= &(ma->mode); *type= IPO_INT_BIT; break;
		case MA_HASIZE:
			poin= &(ma->hasize); break;
		}
		
		if(poin==0) {
			mtex= 0;
			if(icu->adrcode & MA_MAP1) mtex= ma->mtex[0];
			else if(icu->adrcode & MA_MAP2) mtex= ma->mtex[1];
			else if(icu->adrcode & MA_MAP3) mtex= ma->mtex[2];
			else if(icu->adrcode & MA_MAP4) mtex= ma->mtex[3];
			else if(icu->adrcode & MA_MAP5) mtex= ma->mtex[4];
			else if(icu->adrcode & MA_MAP6) mtex= ma->mtex[5];
			else if(icu->adrcode & MA_MAP7) mtex= ma->mtex[6];
			else if(icu->adrcode & MA_MAP8) mtex= ma->mtex[7];
			
			if(mtex) {
				poin= give_mtex_poin(mtex, icu->adrcode & (MA_MAP1-1) );
			}
		}
	}
	else if( GS(id->name)==ID_SEQ) {
		seq= (Sequence *)id;
		
		switch(icu->adrcode) {
		case SEQ_FAC1:
			poin= &(seq->facf0); break;
		}
	}
	else if( GS(id->name)==ID_CU) {
		
		poin= &(icu->curval);
		
	}
	else if( GS(id->name)==ID_KE) {
		
		poin= &(icu->curval);
		
	}
	else if(GS(id->name)==ID_WO) {
		
		wo= (World *)id;
		
		switch(icu->adrcode) {
		case WO_HOR_R:
			poin= &(wo->horr); break;
		case WO_HOR_G:
			poin= &(wo->horg); break;
		case WO_HOR_B:
			poin= &(wo->horb); break;
		case WO_ZEN_R:
			poin= &(wo->zenr); break;
		case WO_ZEN_G:
			poin= &(wo->zeng); break;
		case WO_ZEN_B:
			poin= &(wo->zenb); break;

		case WO_EXPOS:
			poin= &(wo->exposure); break;

		case WO_MISI:
			poin= &(wo->misi); break;
		case WO_MISTDI:
			poin= &(wo->mistdist); break;
		case WO_MISTSTA:
			poin= &(wo->miststa); break;
		case WO_MISTHI:
			poin= &(wo->misthi); break;

		case WO_STAR_R:
			poin= &(wo->starr); break;
		case WO_STAR_G:
			poin= &(wo->starg); break;
		case WO_STAR_B:
			poin= &(wo->starb); break;

		case WO_STARDIST:
			poin= &(wo->stardist); break;
		case WO_STARSIZE:
			poin= &(wo->starsize); break;
		}

		if(poin==0) {
			mtex= 0;
			if(icu->adrcode & MA_MAP1) mtex= wo->mtex[0];
			else if(icu->adrcode & MA_MAP2) mtex= wo->mtex[1];
			else if(icu->adrcode & MA_MAP3) mtex= wo->mtex[2];
			else if(icu->adrcode & MA_MAP4) mtex= wo->mtex[3];
			else if(icu->adrcode & MA_MAP5) mtex= wo->mtex[4];
			else if(icu->adrcode & MA_MAP6) mtex= wo->mtex[5];
			else if(icu->adrcode & MA_MAP7) mtex= wo->mtex[6];
			else if(icu->adrcode & MA_MAP8) mtex= wo->mtex[7];
			
			if(mtex) {
				poin= give_mtex_poin(mtex, icu->adrcode & (MA_MAP1-1) );
			}
		}
	}
	else if( GS(id->name)==ID_LA) {
		
		la= (Lamp *)id;
	
		switch(icu->adrcode) {
		case LA_ENERGY:
			poin= &(la->energy); break;		
		case LA_COL_R:
			poin= &(la->r); break;
		case LA_COL_G:
			poin= &(la->g); break;
		case LA_COL_B:
			poin= &(la->b); break;
		case LA_DIST:
			poin= &(la->dist); break;		
		case LA_SPOTSI:
			poin= &(la->spotsize); break;
		case LA_SPOTBL:
			poin= &(la->spotblend); break;
		case LA_QUAD1:
			poin= &(la->att1); break;
		case LA_QUAD2:
			poin= &(la->att2); break;
		case LA_HALOINT:
			poin= &(la->haint); break;
		}
		
		if(poin==0) {
			mtex= 0;
			if(icu->adrcode & MA_MAP1) mtex= la->mtex[0];
			else if(icu->adrcode & MA_MAP2) mtex= la->mtex[1];
			else if(icu->adrcode & MA_MAP3) mtex= la->mtex[2];
			else if(icu->adrcode & MA_MAP4) mtex= la->mtex[3];
			else if(icu->adrcode & MA_MAP5) mtex= la->mtex[4];
			else if(icu->adrcode & MA_MAP6) mtex= la->mtex[5];
			else if(icu->adrcode & MA_MAP7) mtex= la->mtex[6];
			else if(icu->adrcode & MA_MAP8) mtex= la->mtex[7];
			
			if(mtex) {
				poin= give_mtex_poin(mtex, icu->adrcode & (MA_MAP1-1) );
			}
		}
		
	}
	
	return poin;
}

void set_icu_vars(IpoCurve *icu)
{
	
	icu->ymin= icu->ymax= 0.0;
	icu->ipo= IPO_BEZ;
	
	if(icu->blocktype==ID_OB) {
	
		if(icu->adrcode==OB_LAY) {
			icu->ipo= IPO_CONST;
			icu->vartype= IPO_BITS;
		}
		
	}
	else if(icu->blocktype==ID_MA) {
		
		if(icu->adrcode < MA_MAP1) {
			switch(icu->adrcode) {
			case MA_HASIZE:
				icu->ymax= 10000.0; break;
			case MA_HARD:
				icu->ymax= 128.0; break;
			case MA_MODE:
				icu->ipo= IPO_CONST;
				icu->vartype= IPO_BITS;
				break;
				
			default:
				icu->ymax= 1.0;
				break;
			}
		}
		else {
			switch(icu->adrcode & (MA_MAP1-1)) {
			case MAP_OFS_X:
			case MAP_OFS_Y:
			case MAP_OFS_Z:
			case MAP_SIZE_X:
			case MAP_SIZE_Y:
			case MAP_SIZE_Z:
				icu->ymax= 100.0;
				icu->ymin= -100.0;
			
				break;
			case MAP_R:
			case MAP_G:
			case MAP_B:
			case MAP_DVAR:
			case MAP_COLF:
			case MAP_VARF:
				icu->ymax= 1.0;
				break;
			case MAP_NORF:
				icu->ymax= 5.0;
				break;
			}
		}
	}
	else if(icu->blocktype==ID_SEQ) {
	
		icu->ymax= 1.0;
		
	}
	else if(icu->blocktype==ID_CU) {
	
		icu->ymax= 1.0;
		
	}
	else if(icu->blocktype==ID_WO) {
		
		if(icu->adrcode < MA_MAP1) {
			switch(icu->adrcode) {
			case WO_EXPOS:
				icu->ymax= 5.0; break;
			case WO_MISTDI:
			case WO_MISTSTA:
			case WO_MISTHI:
			case WO_STARDIST:
			case WO_STARSIZE:
				break;
				
			default:
				icu->ymax= 1.0;
				break;
			}
		}
		else {
			switch(icu->adrcode & (MA_MAP1-1)) {
			case MAP_OFS_X:
			case MAP_OFS_Y:
			case MAP_OFS_Z:
			case MAP_SIZE_X:
			case MAP_SIZE_Y:
			case MAP_SIZE_Z:
				icu->ymax= 100.0;
				icu->ymin= -100.0;
			
				break;
			case MAP_R:
			case MAP_G:
			case MAP_B:
			case MAP_DVAR:
			case MAP_COLF:
			case MAP_NORF:
			case MAP_VARF:
				icu->ymax= 1.0;
			}
		}
	}
	else if(icu->blocktype==ID_LA) {
		if(icu->adrcode < MA_MAP1) {
			switch(icu->adrcode) {
			case LA_ENERGY:
			case LA_DIST:
				break;		
	
			case LA_COL_R:
			case LA_COL_G:
			case LA_COL_B:
			case LA_SPOTBL:
			case LA_QUAD1:
			case LA_QUAD2:
			case LA_HALOINT:
				icu->ymax= 1.0; break;
			case LA_SPOTSI:
				icu->ymax= 180.0; break;
			}
		}
		else {
			switch(icu->adrcode & (MA_MAP1-1)) {
			case MAP_OFS_X:
			case MAP_OFS_Y:
			case MAP_OFS_Z:
			case MAP_SIZE_X:
			case MAP_SIZE_Y:
			case MAP_SIZE_Z:
				icu->ymax= 100.0;
				icu->ymin= -100.0;
				break;
			case MAP_R:
			case MAP_G:
			case MAP_B:
			case MAP_DVAR:
			case MAP_COLF:
			case MAP_NORF:
			case MAP_VARF:
				icu->ymax= 1.0;
			}
		}
	}	
}


void execute_ipo(ID *id, Ipo *ipo)
{
	IpoCurve *icu;
	void *poin;
	int type;
	
	if(ipo==0) return;
	
	icu= ipo->curve.first;
	while(icu) {
	
		poin= get_ipo_poin(id, icu, &type);

		if(poin) write_ipo_poin(poin, type, icu->curval);

		icu= icu->next;
	}
}

void do_ipo_nocalc(Ipo *ipo)
{
	Object *ob;
	Material *ma;
	World *wo;
	Lamp *la;
	
	if(ipo==0) return;
	
	switch(ipo->blocktype) {
	case ID_OB:
		ob= G.main->object.first;
		while(ob) {
			if(ob->ipo==ipo) execute_ipo((ID *)ob, ipo);
			ob= ob->id.next;
		}
		break;
	case ID_MA:
		ma= G.main->mat.first;
		while(ma) {
			if(ma->ipo==ipo) execute_ipo((ID *)ma, ipo);
			ma= ma->id.next;
		}
		break;
	case ID_WO:
		wo= G.main->world.first;
		while(wo) {
			if(wo->ipo==ipo) execute_ipo((ID *)wo, ipo);
			wo= wo->id.next;
		}
		break;
	case ID_LA:
		la= G.main->lamp.first;
		while(la) {
			if(la->ipo==ipo) execute_ipo((ID *)la, ipo);
			la= la->id.next;
		}
	}
}

void do_ipo(Ipo *ipo)
{
	float ctime;
	
	if(ipo==0) return;
	
	ctime= frame_to_float(CFRA);
	calc_ipo(ipo, ctime);
	
	do_ipo_nocalc(ipo);
}



void do_mat_ipo(Material *ma)
{
	float ctime;
	ulong lay;
	
	if(ma==0 || ma->ipo==0) return;
	
	ctime= frame_to_float(CFRA);
	/* if(ob->ipoflag & OB_OFFS_OB) ctime-= ob->sf; */
	
	calc_ipo(ma->ipo, ctime);
	
	execute_ipo((ID *)ma, ma->ipo);
}

void do_ob_ipo(Object *ob)
{
	float ctime;
	ulong lay;
	
	if(ob->ipo==0) return;
	
	/* hier NIET ob->ctime zetten: bijv bij parent in onzichtb. layer */
	
	if(ob->ipoflag & OB_OFFS_OB) ctime= system_time(ob, 0, F_CFRA, 0.0);
	else ctime= system_time(0, 0, F_CFRA, 0.0);

	calc_ipo(ob->ipo, ctime);

	/* Patch: de localview onthouden */
	lay= ob->lay & 0xFF000000;
	
	execute_ipo((ID *)ob, ob->ipo);

	ob->lay |= lay;
	
	if(ob->id.name[2]=='S' && ob->id.name[3]=='C' && ob->id.name[4]=='E') {
		if(strcmp(G.scene->id.name+2, ob->id.name+6)==0) {
			G.scene->lay= ob->lay;
			
			copy_view3d_lock(0);
			/* hier geen REDRAW: gaat rondzingen! */
		}
	}
}

void do_seq_ipo(Sequence *seq)
{
	Editing *ed;
	float ctime, div;
	
	/* seq_ipo gaat iets anders: beide fields direkt berekenen */
	
	if(seq->ipo) {
		ctime= frame_to_float(CFRA-seq->startdisp);
		div= (seq->enddisp - seq->startdisp)/100.0;
		if(div==0) return;
		
		/* tweede field */
		calc_ipo(seq->ipo, (ctime+0.5)/div);
		execute_ipo((ID *)seq, seq->ipo);
		seq->facf1= seq->facf0;

		/* eerste field */
		calc_ipo(seq->ipo, ctime/div);
		execute_ipo((ID *)seq, seq->ipo);

	}
	else seq->facf1= seq->facf0= 1.0;
}

int has_ipo_code(Ipo *ipo, int code)
{
	IpoCurve *icu;
	
	if(ipo==0) return 0;
	
	icu= ipo->curve.first;
	while(icu) {
	
		if(icu->adrcode==code) return 1;
		
		icu= icu->next;
	}
	return 0;
}

void do_all_ipos()
{
	Base *base;
	Object *ob;
	Material *ma;
	World *wo;
	Ipo *ipo;
	Lamp *la;
	float ctime, gtime;
	int set, lay;
	
	ctime= gtime= frame_to_float(CFRA);
	
	if(R.flag & R_RENDERING) lay= G.scene->lay;
	else if(G.vd) lay= G.vd->lay;
	else return;
	
	ipo= G.main->ipo.first;
	while(ipo) {
		if(ipo->id.us && ipo->blocktype!=ID_OB) {
			calc_ipo(ipo, ctime);
		}
		ipo= ipo->id.next;
	}

	/* NEW: current scene ob ipo's */
	base= FIRSTBASE;
	set= 0;
	while(base) {
		if(base->object->ipo) {
			/* per object ipo ook de calc_ipo doen: ivm mogelijke timeoffs */
			do_ob_ipo(base->object);
			if(base->object->type==OB_MBALL) where_is_object(base->object);
		}
		base= base->next;
		
		if(base==0 && set==0 && G.scene->set) {
			set= 1;
			base= G.scene->set->base.first;
		}
	}

	ma= G.main->mat.first;
	while(ma) {
		if(ma->ipo) execute_ipo((ID *)ma, ma->ipo);
		ma= ma->id.next;
	}

	wo= G.main->world.first;
	while(wo) {
		if(wo->ipo) execute_ipo((ID *)wo, wo->ipo);
		wo= wo->id.next;
	}

	la= G.main->lamp.first;
	while(la) {
		if(la->ipo) execute_ipo((ID *)la, la->ipo);
		la= la->id.next;
	}

	/* voor het geval dat...  LET OP: 2x */
	base= FIRSTBASE;
	while(base) {
		
		/* alleen layer updaten als ipo */
		if( has_ipo_code(base->object->ipo, OB_LAY) ) {
			base->lay= base->object->lay;
		}
		
		/* mball en deform updaten */
		if(base->lay & lay) {
			ob= base->object;
			if(ob->type==OB_MBALL && ob->ipo) {
				if(ob->disp.first) makeDispList(ob);
			}
			else if(ob->parent) {
				if ELEM(ob->parent->type, OB_LATTICE, OB_IKA) makeDispList(ob);
			}
			
		}
		base= base->next;
	}
	
	/* voor het geval dat...*/
	if(G.scene->set) {
		base= G.scene->set->base.first;
		while(base) {
			
			/* alleen layer updaten als ipo */
			if( has_ipo_code(base->object->ipo, OB_LAY) ) {
				base->lay= base->object->lay;
			}
			
			base= base->next;
		}
	}
}


int calc_ipo_spec(Ipo *ipo, int adrcode, float *ctime)
{
	IpoCurve *icu;

	if(ipo==0) return 0;

	icu= ipo->curve.first;
	while(icu) {
		if(icu->adrcode == adrcode) {
			if(icu->flag & IPO_LOCK);
			else calc_icu(icu, *ctime);
			
			*ctime= icu->curval;
			return 1;
		}
		icu= icu->next;
	}
	
	return 0;
}


/* ************************** */

void clear_delta_obipo(Ipo *ipo)
{
	Object *ob;
	
	if(ipo==0) return;
	
	ob= G.main->object.first;
	while(ob) {
		if(ob->id.lib==0) {
			if(ob->ipo==ipo) {
				bzero(&ob->dloc, 12);
				bzero(&ob->drot, 12);
				bzero(&ob->dsize, 12);
			}
		}
		ob= ob->id.next;
	}
}

void add_to_cfra_elem(ListBase *lb, BezTriple *bezt)
{
	CfraElem *ce, *cen;
	
	ce= lb->first;
	while(ce) {
		
		if( ce->cfra==bezt->vec[1][0] ) {
			/* doen ivm dubbele keys */
			if(bezt->f2 & 1) ce->sel= bezt->f2;
			return;
		}
		else if(ce->cfra > bezt->vec[1][0]) break;
		
		ce= ce->next;
	}	
	
	cen= callocN(sizeof(CfraElem), "add_to_cfra_elem");	
	if(ce) insertlinkbefore(lb, ce, cen);
	else addtail(lb, cen);

	cen->cfra= bezt->vec[1][0];
	cen->sel= bezt->f2;
}



int make_cfra_list(Ipo *ipo, ListBase *elems)
{
	IpoCurve *icu;
	CfraElem *ce;
	BezTriple *bezt;
	float *fp;
	int a;
	
	if(ipo->blocktype==ID_OB) {
		icu= ipo->curve.first;
		while(icu) {
			if(icu->flag & IPO_VISIBLE) {
				switch(icu->adrcode) {
				case OB_DLOC_X:
				case OB_DLOC_Y:
				case OB_DLOC_Z:
				case OB_DROT_X:
				case OB_DROT_Y:
				case OB_DROT_Z:
				case OB_DSIZE_X:
				case OB_DSIZE_Y:
				case OB_DSIZE_Z:

				case OB_LOC_X:
				case OB_LOC_Y:
				case OB_LOC_Z:
				case OB_ROT_X:
				case OB_ROT_Y:
				case OB_ROT_Z:
				case OB_SIZE_X:
				case OB_SIZE_Y:
				case OB_SIZE_Z:
					bezt= icu->bezt;
					if(bezt) {
						a= icu->totvert;
						while(a--) {
							add_to_cfra_elem(elems, bezt);
							bezt++;
						}
					}
					break;
				}
			}
			icu= icu->next;
		}
	}
	if(ipo->showkey==0) {
		/* alle keys deselecteren */
		ce= elems->first;
		while(ce) {
			ce->sel= 0;
			ce= ce->next;
		}
	}
}

