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

/*   	anim.c         */


/*  
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'


long	*getbasekeyadr(base,type)

float	frametofloat(cfra,sf,len,cycl)
	ease(cur,fr,sta,end)

	countkeys();
short	setkeys(fac,startkey,k,t,cycl)
	llerp(aantal,in,f0,f1,f2,f3,t)
	flerp(aantal,in,f0,f1,f2,f3,t)
	loadkeypos(base, childbase)
	loadkeyposall()
	insertkeypos(base)
	deletekeypos()

	calctrack(base)
	calcfillquat(q,base,childbase,fr)
	followpath()
	quatfirst(base,mat,imat,omat,vec,fo)
	matfirst(base,mat,omat,vec,fo)
	quatmofirst(base,childbase,mat,imat,omat,vec,fo)
long	*calcpath(base,childbase,fr)

	parentbase(base,mat)

short	compmat(m1,m2)
short	compbase(b1,b2)
struct Base *makeduplilist(first,par)
struct Base *duplibase()

******************************************************** */
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "/usr/people/include/Trace.h"

extern void VecMulf(float *v1, float f);

void parentbase();
short calcpathn();
struct Base *duplilist=0;
struct Base copybase;	/* temporary voor parentbase */
int induplibase=0;		/* voor parentbase: aangeven dat dupli's gemaakt worden */
int nofield=0;
int slurph_opt= 1;		/* snellere slurphing voor wire */

float frametotime(short fra)
{
	float fac, cfra, ipo();

	cfra= fra;

	if(G.timeipo) {
		fac= G.images;
		if(cfra<=fac) {
			cfra= (cfra)/fac;
			cfra= (G.images)*ipo(G.timeipo, cfra);
		}
	}
	
	cfra= G.framelen * cfra;
	if (R.f & 64) cfra += 0.5 * G.framelen;
	
	return cfra;
}

float frametofloat(cfra,sf,len,cycl)
float cfra;
short sf,len,cycl;
{
	/* geeft float tussen 0-1 terug */
	float f, ipo();

	if(cfra<1.0) cfra= 1.0;

	/* global time */
	if(G.timeipo) {
		f= G.images;
		if(cfra<=f) {
			cfra= (cfra)/f;
			cfra= (G.images)*ipo(G.timeipo, cfra);
		}
	}
	
	if ( (R.f & 64) && nofield==0 && (R.f & 512)==0 ) cfra+= .5;	/* tweede field */

	cfra*= G.framelen;

	while(sf> (short)cfra) {
		if(cycl & 1) sf-= len;
		else return 0.0;
	}
	if(cycl==0) if( cfra>=(float)(sf+len) ) return 1.0;

	f= cfra-(float)sf;

	while(f>(float)len) f-=len;

	if(cycl & 1) {
		f/=(float)len;
		if(f>=.999999) return 0.0;
	}
	else {
		f/=(float)(len-1.0);
		if(f>=.999999) return 1.0;
	}
	return f;
}

float ease(f1, f2, f3, f4)  /* current,lengte,startease,eindease */
float f1,f2,f3,f4;
{
	float t,qa,b1;

	if(f3>f2) f3= f2;
	if(f4>f2) f4= f2;

	if(f3!=0.0 || f4!=0.0) {
		if(f1<f3) {
			t= f1/f3;
			qa= t*t;
			b1= f3*(-t*qa/3+qa);
		}
		else if(f1>= f2-f4) {
			t= (f2-f1)/f4;
			qa=t*t;
			b1=f2-(f3+f4)/3-f4*(-t*qa/3+qa);
		}
		else {
			b1=f1-f3/3;
		}
		b1=b1/(f2-(f3+f4)/3);
	}
	else b1=f1/f2;
	return b1;
}

long *getbasekeyadr(base,type)
struct Base *base;
short type;
{
	/* test of base key heeft die ge-edit wordt */
	/* type: 0 vec, 1 or, 2 quat, 3 mat */
	/* geen keys of geen editmode: return baseadressen */

	struct Key *key;
	long *ai;
	short a1, keymode=0;
	
	if(G.keymode) keymode= 1;
	else if(G.basact==base) keymode= 1;

	if(keymode && G.ipomode==20 && base->key!=0) {
		key=base->key;
		a1=1;
		while(G.actkey!=0 && key!=0 && a1!=G.actkey) {
			key=key->next;
			a1++;
		}
		if(key==0) G.actkey=0;
		if(G.actkey) {
			ai= (long *)(key+1);
			if(type==0) return ai;
			else if(type==1) return ai+3;
			else if(type==2) return ai+6;
			else if(type==3) return ai+10;
		}

	}

	if(type==0) return base->v;
	else if(type==1) return base->o;
	else if(type==2) return (long *)base->q;
	else if(type==3) return (long *)base->m;

	return 0;
}

void give_curipo_pp(bez)	/* return pointerpointer */
struct Bezier ***bez;
{
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	
	*bez= 0;

	if(G.ipomode==10) *bez= &(G.timeipo);
	if(G.basact==0) return;
	
	if(G.ipomode==0) {
		mo= (struct MoData *)G.basact->d;
		*bez= &(mo->ipoqfi);
	}
	else if(G.ipomode==1) {
		mo= (struct MoData *)G.basact->d;
		*bez= &(mo->ipotra);
	}
	else if(G.ipomode==20) *bez= &(G.basact->ipokey);
	else if(G.ipomode==21) *bez= &(G.basact->ipopkey);
	else if(G.ipomode==23) *bez= &(G.basact->iponoise);
	else if(G.ipomode==22) {
		ob= (struct ObData *)G.basact->d;
		if ELEM3(G.basact->soort, 1, 5, 11) *bez= &(ob->ipovvkey);
		else if(G.basact->soort== -2) {
			mo= (struct MoData *)G.basact->d;
			*bez= &(mo->ipovkey);
		}
		else if(G.basact->soort== -4) {
			ika= (struct IkaData *)G.basact->d;
			*bez= &(ika->ipokey);
		}
	}
}

void give_curipo(bez)		/* return pointer */
struct Bezier **bez;
{
	struct Bezier **bez1;

	give_curipo_pp(&bez1);
	if(bez1) *bez= *bez1;
	else *bez= 0;
}

void give_curkey_pp(key)	/* return pointerpointer */
struct Key ***key;
{
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	
	*key= 0;

	if(G.basact==0) return;
	
	if(G.ipomode==20) *key= &(G.basact->key);
	else if(G.ipomode==21) *key= &(G.basact->pkey);
	else if(G.ipomode==22) {
		ob= (struct ObData *)G.basact->d;
		if(G.basact->soort==1)  *key= &(ob->vv->key);
		else if ELEM(G.basact->soort, 5, 11) *key= &(ob->cu->key);
		else if(G.basact->soort== -2) {
			mo= (struct MoData *)ob;
			*key= &(mo->key);
		}
		else if(G.basact->soort== -4) {
			ika= (struct IkaData *)ob;
			*key= &(ika->key);
		}
	}
}

void give_curkey(key)		/* return pointer */
struct Key **key;
{
	struct Key **key1;

	give_curkey_pp(&key1);
	if(key1) *key= *key1;
	else *key= 0;
}

short countkeys()
{
	/* telt aantal keys ahv. G.ipomode, geen keys return 0. Doet ook controle G.actkey */
	struct Key *key=0;
	short a=0;

	give_curkey(&key);
	while(key) {
		a++;
		key=key->next;
	}

	if(G.actkey>a) G.actkey= a;

	return a;
}

void set_four_ipo(d, data, type)
float d, *data;
long type;
{
	float d2, d3, fc;
	
	if(type==1) {
		data[0]= 0;
		data[1]= 1.0-d;
		data[2]= d;
		data[3]= 0.0;
	}
	else {
		d2= d*d;
		d3= d2*d;
		
		if(type==2) {
					/* oud: cardinal spline met 'alpha' 1.0 */
			data[0]= -d3	+2*d2	-d;
			data[1]= d3	-2*d2		+1.0;
			data[2]= -d3	+d2		+d;
			data[3]= d3	-d2	;
		}
		else if(type==0) {
					/* cardinal spline */
			fc= 0.71;
			
			data[0]= -fc*d3		+2.0*fc*d2		-fc*d;
			data[1]= (2.0-fc)*d3	+(fc-3.0)*d2				+1.0;
			data[2]= (fc-2.0)*d3	+(3.0-2.0*fc)*d2 +fc*d;
			data[3]= fc*d3			-fc*d2;
		}
		else if(type==3) {
					/* B spline */
			data[0]= -0.1666*d3	+0.5*d2	-0.5*d	+0.16666;
			data[1]= 0.5*d3		-d2				+0.6666;
			data[2]= -0.5*d3		+0.5*d2	+0.5*d	+0.1666;
			data[3]= 0.1666*d3			;
		}
	}
}

short setkeys(fac, firstkey, k, t, cycl)
float fac;
struct Key *firstkey,*k[];
float *t;
short cycl;
{
	/* return 1 betekent k[2] is de positie, 0 is interpoleren */
	struct Key *k1;
	float d, ofs=0, temp, fval[4];
	short aantal=0, bsplinetype;

	k1=k[0]=k[1]=k[2]=k[3]= firstkey;
	t[0]=t[1]=t[2]=t[3]= k1->pos;

	if(fac<0.0 || fac>1.0) return 1;

	if(k1->next==0) return 1;

	if(cycl) {	/* voorsorteren */
		k[2]= k1->next;
		k[3]= k[2]->next;
		if(k[3]==0) k[3]=k1;
		while(k1) {
			if(k1->next==0) k[0]=k1;
			k1=k1->next;
		}
		k1= k[1];
		t[0]= k[0]->pos;
		t[1]+=1.0;
		t[2]= k[2]->pos+1.0;
		t[3]= k[3]->pos+1.0;
		fac+=1;
		ofs=1;
		if(k[3]==k[1]) { 
			t[3]+=1; 
			ofs=2;
		}
		if(fac<t[1]) fac+=1;
		k1=k[3];
	}
	else {		/* voorsorteren */
		/* waarom dit voorsorteren niet eerder gedaan? voor juist interpoleren in begin noodz. */
		k[2]= k1->next;
		t[2]= k[2]->pos;
		k[3]= k[2]->next;
		if(k[3]==0) k[3]= k[2];
		t[3]= k[3]->pos;
		k1= k[3];
	}
	
	while( t[2]<fac ) {	/* goede plek vinden */
		if(k1->next==0) {
			if(cycl) {
				k1= firstkey;
				ofs+=1;
			}
			else if(t[2]==t[3]) break;
		}
		else k1= k1->next;

		t[0]= t[1]; 
		k[0]= k[1];
		t[1]= t[2]; 
		k[1]= k[2];
		t[2]= t[3]; 
		k[2]= k[3];
		t[3]= k1->pos+ofs; 
		k[3]= k1;

		if(ofs>3.1) break;
	}
	
	bsplinetype= 0;
	if(k[1]->f1==3 || k[2]->f1==3) bsplinetype= 1;
	
	if(cycl==0) {
		if(bsplinetype==0) {	/* B spline gaat niet door de punten */
			if(fac<=t[1]) {		/* fac voor 1e key */
				t[2]= t[1];
				k[2]= k[1];
				return 1;
			}
			if(fac>=t[2] ) {	/* fac na 2e key */
				return 1;
			}
		}
		else if(fac>t[2]) {	/* laatste key */
			fac= t[2];
			k[3]= k[2];
			t[3]= t[2];
		}
	}

	d= t[2]-t[1];
	if(d==0.0) {
		if(bsplinetype==0) {
			return 1;	/* beide keys gelijk */
		}
	}
	else d= (fac-t[1])/d;

	/* interpolatie */
	
	set_four_ipo(d, t, k[1]->f1);
	
	if(k[1]->f1 != k[2]->f1) {
		set_four_ipo(d, fval, k[2]->f1);
		
		temp= 1.0-d;
		t[0]= temp*t[0]+ d*fval[0];
		t[1]= temp*t[1]+ d*fval[1];
		t[2]= temp*t[2]+ d*fval[2];
		t[3]= temp*t[3]+ d*fval[3];
	}

	return 0;

}

void slerp(aantal,in,l0,l1,l2,l3,t)	/* short */
short aantal;
short *in,*l0,*l1,*l2,*l3;
float *t;
{
	short a;
	long temp;

	for(a=0; a<aantal; a++) {
		temp= t[0]*l0[a]+t[1]*l1[a]+t[2]*l2[a]+t[3]*l3[a];
		if(temp< -32767) temp= -32767;
		else if(temp>  32767) temp= 32767;
		in[a]= temp;
	}
}

void llerp(aantal,in,l0,l1,l2,l3,t)	/* long */
short aantal;
long *in,*l0,*l1,*l2,*l3;
float *t;
{
	short a;

	for(a=0;a<aantal;a++) {
		in[a]=t[0]*l0[a]+t[1]*l1[a]+t[2]*l2[a]+t[3]*l3[a];
	}
}

void flerp(aantal,in,f0,f1,f2,f3,t)	/* float */
short aantal;
float *in,*f0,*f1,*f2,*f3;
float *t;
{
	short a;

	for(a=0;a<aantal;a++) {
		in[a]=t[0]*f0[a]+t[1]*f1[a]+t[2]*f2[a]+t[3]*f3[a];
	}
}

void qlerp(aantal,in,f0,f1,f2,f3,t)	/* quat */
short aantal;
float *in,*f0,*f1,*f2,*f3;
float *t;
{
	float len, inp, t0, t1, t2, t3;
	short a;

	for(a=0;a<aantal;a++) {
		/* omklappen!  */
		t0= t[0];
		t1= t[1];
		t2= t[2];
		t3= t[3];
		
		inp= f0[0]*f1[0]+f0[1]*f1[1]+f0[2]*f1[2]+f0[3]*f1[3];
		if(inp<0.0) t1= -t1;

		inp= f1[0]*f2[0]+f1[1]*f2[1]+f1[2]*f2[2]+f1[3]*f2[3];
		if(t1*inp<0.0) t2= -t2;

		inp= f2[0]*f3[0]+f2[1]*f3[1]+f2[2]*f3[2]+f2[3]*f3[3];
		if(t2*inp<0.0) t3= -t3;

		in[0]= t0*f0[0]+ t1*f1[0]+ t2*f2[0]+ t3*f3[0];
		in[1]= t0*f0[1]+ t1*f1[1]+ t2*f2[1]+ t3*f3[1];
		in[2]= t0*f0[2]+ t1*f1[2]+ t2*f2[2]+ t3*f3[2];
		in[3]= t0*f0[3]+ t1*f1[3]+ t2*f2[3]+ t3*f3[3];

		len= fsqrt(in[0]*in[0]+in[1]*in[1]+in[2]*in[2]+in[3]*in[3]);
		if(len==0) {
			in[0]= 1.0;
		}
		in[0]/=len;
		in[1]/=len;
		in[2]/=len;
		in[3]/=len;

		in+=4; 
		f0+=4; 
		f1+=4; 
		f2+=4; 
		f3+=4;
	}
}

void wslerp(in,f0,f1,f2,f3,t)	/* wordspace */
float *in,*f0,*f1,*f2,*f3;
float *t;
{
	float w0, w1, w2, w3;

	w0= 1.0/(*f0);
	w1= 1.0/(*f1);
	w2= 1.0/(*f2);
	w3= 1.0/(*f3);

	*in= 1.0/(t[0]*w0+t[1]*w1+t[2]*w2+t[3]*w3);

}


struct Key *sortkeylist(basekey)
struct Key *basekey;
{
	/* geeft basekeypointer terug */

	struct Key *key,*keyn,*kprev;
	short lus= 1;

	if(basekey==0) return;

	while(lus) {
		lus= 0;
		key= basekey;
		keyn= key->next;
		kprev= 0;

		while(keyn) {
			if(keyn->pos==key->pos) keyn->pos+=.0001;
			else if(keyn->pos<key->pos) {
				if(kprev) kprev->next= keyn;
				else basekey= keyn;

				key->next= keyn->next;
				keyn->next= key;
				lus= 1;
			}

			kprev= key;
			key= key->next;
			if(key) keyn= key->next;
			else keyn= 0;
		}
	}
	return basekey;
}

void slurph_wspace(vv, fdata)
struct VV *vv;
float *fdata;
{
	float fac, *fd;
	float min[3], max[3];
	short *in;
	int a, c;
	
	min[0]=min[1]=min[2]= 1.0e20;
	max[0]=max[1]=max[2]= -1.0e20;

	a= vv->vert;
	fd= fdata;
	while(a--) {
		for(c=0;c<3;c++) {
			if(fd[c]>max[c]) max[c]= fd[c];
			if(fd[c]<min[c]) min[c]= fd[c];
		}
		fd+= 3;
	}
	max[0]= MAX2(fabs(min[0]),fabs(max[0]));
	max[1]= MAX2(fabs(min[1]),fabs(max[1]));
	max[2]= MAX2(fabs(min[2]),fabs(max[2]));
	fac= MAX3(max[0],max[1],max[2]);
	fac/= 32767.0;
	vv->ws= fac;
	
	fd= fdata;
	a= vv->vert;
	in= (short *)(vv+1);
	while(a--) {
		in[0]= fd[0]/fac;
		in[1]= fd[1]/fac;
		in[2]= fd[2]/fac;
		
		fd+=3;
		in+= 6;
	}
}

void loadkeypostype(type, base, childbase)
long type;
struct Base *base, *childbase;
{
	struct ObData *ob;
	struct VV *vv;
	struct CurveData *cu;
	struct MoData *mo;
	struct IkaData *ika;
	struct BodyPoint *bop;
	struct Rod *rod;
	struct Nurb *nu;
	struct Bezier *bez;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Key *k[4], *key;
	float fac, t[4], *f0, *f1, *f2, *f3, ipo(), cfra, delta;
	float *fd, *fdata, ws;
	long *l0,*l1,*l2,*l3, *in, a, b, tot;
	short flag, sar[3], *sp;

	if(base==0 || childbase==0) return;
	if(type==20) {
		if(base->key) {
			fac= frametofloat( (float)G.cfra, childbase->sf, base->len, base->f2 & 1);
			fac= ipo(base->ipokey,fac);
	
			flag= setkeys(fac, base->key, k, t, base->f2 & 1);
			if(flag==0) {
			
				l0= (long *)(k[0]+1);
				l1= (long *)(k[1]+1);
				l2= (long *)(k[2]+1);
				l3= (long *)(k[3]+1);
				llerp(3, base->v, l0, l1, l2, l3, t);
				llerp(3, base->o, l0+3, l1+3, l2+3, l3+3, t);
				qlerp(1, base->q, l0+6, l1+6, l2+6, l3+6, t);
				flerp(9, base->m, l0+10, l1+10, l2+10, l3+10, t);
			}
			else {
				l0=(long *)(k[2]+1);
				memcpy(base->v,l0,12);
				memcpy(base->o,l0+3,12+16+36);
			}
		}
	}
	else if(type==21) {
		if(base->pkey) {
			fac= frametofloat( (float)G.cfra, childbase->sf, base->len, base->f2 & 1);
			fac= ipo(base->ipopkey,fac);
	
			flag= setkeys(fac,base->pkey,k,t,base->f2 & 1);
			if(flag==0) {
				l0= (long *)(k[0]+1);
				l1= (long *)(k[1]+1);
				l2= (long *)(k[2]+1);
				l3= (long *)(k[3]+1);
	
				key= base->pkey;
				base->pkey= 0;	/* parentbase misleiden */
	
				base->p= (struct Base *)(*l0);
				parentbase(base, l0+1, l0+17);
				base->p= (struct Base *)(*l1);
				parentbase(base, l1+1, l1+17);
				base->p= (struct Base *)(*l2);
				parentbase(base, l2+1, l2+17);
				base->p= (struct Base *)(*l3);
				parentbase(base, l3+1, l3+17);
				base->p= 0;
	
				base->pkey= key;
	
				in= (long *)(key+1);
				flerp(25, in+1, l0+1, l1+1, l2+1, l3+1, t);
			}
			else {
				l2= (long *)(k[2]+1);
				base->p= (struct Base *)(*l2);
				key= base->pkey;
				base->pkey= 0;	/* misleiden parentbase */
				in= (long *)(key+1);
				parentbase(base, in+1, in+17);
				base->pkey= key;
				base->p= 0;
			}
		}
	}
	else if(type==22) {
		if(base->soort==1) {
			ob= (struct ObData *)base->d;
			vv= ob->vv;
			if(vv->key && base!=G.ebase) {	/* gaat anders raar doen vanwege de ->ws */
				
				if(ob->ef==5 && ob->elen>0) {	/* SLURPHING */
					delta= ob->elen;
					delta/= vv->vert;
					
					cfra= G.cfra+ob->elen;
					
					/* ivm wordruimte met tijdelijk datablok werken */
					fdata= fd= mallocN(3*4*vv->vert, "slurph");
					
					for(a=0; a<vv->vert; a++, fd+=3) {
						
						fac= frametofloat( cfra-=delta, childbase->sf, base->len, base->f2 & 1);
						fac= ipo(ob->ipovvkey, fac);
			
						flag= setkeys(fac, vv->key, k, t, base->f2 & 1);

						if(flag!=0) {
							l0= (long *)(k[2]+1);
							ws= *( (float *)l0 );
							
							l0+= 1+3*a;
							sp= (short *)l0;
							
							fd[0]= ws* sp[0];
							fd[1]= ws* sp[1];
							fd[2]= ws* sp[2];
						}
						else {
					
							l0= (long *)(k[0]+1);
							l1= (long *)(k[1]+1);
							l2= (long *)(k[2]+1);
							l3= (long *)(k[3]+1);
							wslerp(&ws, l0, l1, l2, l3, t);
							
							l0+= 1+3*a; l1+= 1+3*a; l2+= 1+3*a; l3+= 1+3*a; 
							
							slerp(3, sar, l0, l1, l2, l3, t);
							
							fd[0]= ws* sar[0];
							fd[1]= ws* sar[1];
							fd[2]= ws* sar[2];
						}
						
						if(slurph_opt && vv->vert>199) {
							b= vv->vert/100 -1;
							while(b--) {
								a++;
								if(a==vv->vert) break;
								cfra-= delta;
								fd+= 3;
								l0+= 3; l1+= 3; l2+= 3; l3+= 3;
								
								if(flag!=0) {
									sp= (short *)l0;
									
									fd[0]= ws* sp[0];
									fd[1]= ws* sp[1];
									fd[2]= ws* sp[2];
								}
								else {
									slerp(3, sar, l0, l1, l2, l3, t);
							
									fd[0]= ws* sar[0];
									fd[1]= ws* sar[1];
									fd[2]= ws* sar[2];
								}
							}
						}
					}
					
					/* opnieuw wordruimte bepalen */
					slurph_wspace(vv, fdata);
					
					freeN(fdata);
					
				}
				else {
					fac= frametofloat( (float)G.cfra, childbase->sf, base->len, base->f2 & 1);
					fac= ipo(ob->ipovvkey,fac);
		
					flag= setkeys(fac, vv->key, k, t, base->f2 & 1);
					if(flag==0) {
						l0= (long *)(k[0]+1);
						l1= (long *)(k[1]+1);
						l2= (long *)(k[2]+1);
						l3= (long *)(k[3]+1);
						wslerp(&vv->ws, l0, l1, l2, l3, t);
						in= (long *)(vv+1);
						a= vv->vert;
						l0++; 
						l1++; 
						l2++; 
						l3++;
						while(a--) {
							slerp(3, in, l0, l1, l2, l3, t);
							in+=3;	/* dit zijn long pointers, zodoende slaan ze puntnormalen over */
							l0+=3; 
							l1+=3; 
							l2+=3; 
							l3+=3;
						}
					}
					else {
						l0=(long *)(k[2]+1);
						in= (long *)(vv+1);
						vv->ws= *( (float *)l0 );
						memcpy(in, l0+1, 6*2*vv->vert);
					}
				}
				makeDispList(base);			
			}
		}
		else if ELEM3(base->soort, 5, 11, -2) {
			if(base->soort== -2) {
				mo= (struct MoData *)base->d;
				key= mo->key;
				bez= mo->ipovkey;
			}
			else {
				ob= (struct ObData *)base->d;
				cu= ob->cu;
				key= cu->key;
				bez= ob->ipovvkey;
			}
			
			if(key && base!=G.ebase) {	/* gaat anders raar doen vanwege de ->ws */
				fac= frametofloat( (float)G.cfra, childbase->sf, base->len, base->f2 & 1);
				
				fac= ipo(bez, fac);
	
				flag= setkeys(fac, key, k, t, base->f2 & 1);
				
				l0= (long *)(k[0]+1);
				l1= (long *)(k[1]+1);
				l2= (long *)(k[2]+1);
				l3= (long *)(k[3]+1);
				
				if(base->soort== -2) {
					tot= mo->keyverts;
					nu= mo->curve.first;
				}
				else {
					if(flag==0) wslerp(&cu->ws, l0, l1, l2, l3, t);
					else cu->ws= *( (float *)l2 );
					
					tot= cu->keyverts;
					nu= cu->curve.first;
				}
				
				
				l0++; 
				l1++; 
				l2++; 
				l3++;
				
				while(nu) {
					if( (nu->type & 7)==1 ) {
						bezt= nu->bezt;
						a= nu->pntsu;
						while(a--) {
							if(flag==0)  flerp(9, bezt->vec, l0, l1, l2, l3, t);
							else memcpy(bezt->vec, l2, 9*4);
							l0+= 9; l1+= 9;
							l2+= 9; l3+= 9;
							tot-= 3;
							bezt++;
						}
						calchandlesNurb(nu);
					}
					else {
						bp= nu->bp;
						a= nu->pntsu*nu->pntsv;
						while(a--) {
							if(flag==0) flerp(3, bp->vec, l0, l1, l2, l3, t);
							else memcpy(bp->vec, l2, 3*4);
							l0+= 3; l1+= 3;
							l2+= 3; l3+= 3;
							tot--;
							bp++;
						}
					}
					if(tot<0) {
						printf("Something wrong in loadkeypos\n");
						break;
					}
					nu= nu->next;
				}
				if(base->soort==11) makeBevelList(base);
				makeDispList(base);
			}
		}
		else if(base->soort== -4) {

			ika= (struct IkaData *)base->d;
			key= ika->key;
			bez= ika->ipokey;
			
			if(key && base!=G.ebase) {	/* gaat anders raar doen vanwege de ->ws */
				fac= frametofloat( (float)G.cfra, childbase->sf, base->len, base->f2 & 1);
				
				fac= ipo(bez, fac);
	
				flag= setkeys(fac, key, k, t, base->f2 & 1);
				
				l0= (long *)(k[0]+1);
				l1= (long *)(k[1]+1);
				l2= (long *)(k[2]+1);
				l3= (long *)(k[3]+1);
				
				tot= ika->keyverts;
				bop= ika->bpbase.first;
				
				while(bop) {
					
					if(flag==0)  flerp(3, bop->r, l0, l1, l2, l3, t);
					else memcpy(bop->r, l2, 3*4);
					l0+= 4; l1+= 4;
					l2+= 4; l3+= 4;
					
					bop= bop->next;
				}
				
				rod= ika->rodbase.first;
				while(rod) {
					if(flag==0) flerp(1, &rod->alpha, l0, l1, l2, l3, t);
					else memcpy(&rod->alpha, l2, 4);
					l0++; l1++;
					l2++; l3++;
					
					rod= rod->next;
				}
				
				calc_ika_ext(ika);
			}
		}
	}
}

void loadkeypos(base, childbase)
struct Base *base, *childbase;
{
	
	loadkeypostype(20, base, childbase);
	if(G.hie) loadkeypostype(21, base, childbase);
	loadkeypostype(22, base, childbase);
}

void freekeys(key)
struct Key *key;
{
	struct Key *nkey;

	while(key) {
		nkey= key->next;
		freeN(key);
		key= nkey;
	}
}

void loadkeyposall()
{
	struct Base *base;
	struct ObData *ob;

	base= G.firstbase;
	while(base) {
		base->f2 &= ~16;
		base= base->next;
	}

	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base->soort==9) {
				ob= (struct ObData *)base->d;
				if(ob->ef==4) {
					makepolytext(base);
					extrudepoly(base);					
				}
			}
			base->f2 |= 16;
			if(base->p)  base->p->f2 |= 16;
		}
		base= base->next;
	}

	base= G.firstbase;
	while(base) {
		if(base->f2 & 16) {
			loadkeypos(base, base);
			if(base->p) calc_deform(base);
		}
		base= base->next;
	}
	if(view0.cambase) {
		if( (view0.cambase->lay & view0.lay)==0 ) 
			loadkeypos(view0.cambase, view0.cambase);
	}
}

void curve_to_key(key, cu, mo)
struct Key *key;
struct CurveData *cu;
struct MoData *mo;
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float *temp;
	long a;
	
	temp= (float *)(key+1);
	
	if(cu) *temp= cu->ws;
	
	temp++;
	
	/* if(mo && cu) error("NEE DIT NIET"); */
	
	if(mo) nu= mo->curve.first;
	else nu= cu->curve.first;
	
	while(nu) {
		if( (nu->type & 7)==1 ) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				memcpy(temp, bezt->vec, 9*4);
				temp+= 9;
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				memcpy(temp, bp->vec, 3*4);
				temp+= 3;
				bp++;
			}
		}
		nu= nu->next;
	}
}

void key_to_curve(key, cu, mo)
struct Key *key;
struct CurveData *cu;
struct MoData *mo;
{
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float *temp;
	long a;
	
	temp= (float *)(key+1);
	if(cu) cu->ws= *temp;
	temp++;
	
	if(mo) nu= mo->curve.first;
	else nu= cu->curve.first;

	while(nu) {
		if( (nu->type & 7)==1 ) {
			bezt= nu->bezt;
			a= nu->pntsu;
			while(a--) {
				memcpy(bezt->vec, temp, 9*4);
				temp+= 9;
				bezt++;
			}
		}
		else {
			bp= nu->bp;
			a= nu->pntsu*nu->pntsv;
			while(a--) {
				memcpy(bp->vec, temp, 3*4);
				temp+= 3;
				bp++;
			}
		}
		nu= nu->next;
	}
}

void ika_to_key(struct IkaData *ika, struct Key *key)
{
	struct BodyPoint *bop;
	struct Rod *rod;
	int *temp;

	temp= (int *)(key+1);

	/* bodypoints */
	bop= ika->bpbase.first;
	while(bop) {
		memcpy(temp, bop->r, 4*3);
		temp+= 3;
		*temp= bop->f;
		temp++;
		bop= bop->next;
	}
	/* rods */
	rod= ika->rodbase.first;
	while(rod) {
		memcpy(temp, &rod->alpha, 4);
		temp++;
		rod= rod->next;
	}	
}

void key_to_ika(struct Key *key, struct IkaData *ika)
{
	struct BodyPoint *bop;
	struct Rod *rod;
	int *temp;

	temp= (int *)(key+1);

	bop= ika->bpbase.first;
	while(bop) {
		memcpy(bop->r, temp, 4*3);
		temp+= 3;
		bop->f= temp;
		temp++;
		bop= bop->next;
	}
	/* rods */
	rod= ika->rodbase.first;
	while(rod) {
		memcpy(&rod->alpha, temp, 4);
		temp++;
		rod= rod->next;
	}	
}


void insertkeypos_base(base)
struct Base *base;
{
	struct VV *vv;
	struct Bezier *bez;
	struct CurveData *cu=0;
	struct MoData *mo=0;
	struct Nurb *nu;
	struct IkaData *ika;
	struct BodyPoint *bop;
	struct Rod *rod;
	struct Key *k,*kn,**firstkey,*pkey;
	float ipo(), bmat[4][4], omat[3][3];
	long *temp, len, a;

	if(base==0) return;

	/* eerst nieuwe key aanmaken: kn  */
	if(G.ipomode==20) {
		kn= (struct Key *)callocN(sizeof(struct Key)+19*4,"insKey");
		temp=(long *)(kn+1);
		memcpy(temp,base->v,12);
		memcpy(temp+3,base->o,12+16+36);
		firstkey= &base->key;
		bez= base->ipokey;
	}
	else if(G.ipomode==21) {
		parentbase(base, bmat, omat);

		kn= (struct Key *)callocN(sizeof(struct Key)+4+ 4*25,"insKey1");
		temp=(long *)(kn+1);
		*temp= (long)(base->p);
		base->p= 0;
		memcpy(temp+1,bmat,4*16);
		memcpy(temp+17,omat,4*9);
		firstkey= &(base->pkey);
		bez= base->ipopkey;
	}
	else if(G.ipomode==22) {
		if(base->soort==1) {
			vv= ( (struct ObData *)base->d )->vv;
			len= 6*2*vv->vert;
			kn= (struct Key *)callocN(sizeof(struct Key)+len+4,"insKey2");
			temp= (long *)(kn+1);
			*( (float *)temp )= vv->ws;
			memcpy(temp+1,vv+1,len);
			firstkey= &(vv->key);
			bez= ( (struct ObData *)base->d )->ipovvkey;
		}
		else if ELEM(base->soort, 5, 11) {
			cu= ( (struct ObData *)base->d )->cu;
			cu->keyverts= 0;
			nu= cu->curve.first;
			while(nu) {
				if( (nu->type & 7)==1 ) cu->keyverts+= 3*nu->pntsu;
				else cu->keyverts+= nu->pntsu*nu->pntsv;
				nu= nu->next;
			}
			kn= (struct Key *)callocN(sizeof(struct Key)+ 3*4*cu->keyverts+4,"insKey3");
			temp= (long *)(kn+1);
			*( (float *)temp )= cu->ws;
			temp++;
			
			firstkey= &(cu->key);
			bez= ( (struct ObData *)base->d )->ipovvkey;
			
			curve_to_key(kn, cu, mo);
			
		}
		else if(base->soort== -2) {
			mo= (struct MoData *)base->d;
			mo->keyverts= 0;
			nu= mo->curve.first;
			while(nu) {
				if( (nu->type & 7)==1 ) mo->keyverts+= 3*nu->pntsu;
				else mo->keyverts+= nu->pntsu*nu->pntsv;
				nu= nu->next;
			}
			kn= (struct Key *)callocN(sizeof(struct Key)+ 3*4*mo->keyverts+4,"insKey3");
			temp= (long *)(kn+1);
			*( (float *)temp )= 1.0;
			temp++;
			
			firstkey= &(mo->key);
			bez= mo->ipovkey;
			
			curve_to_key(kn, cu, mo);
			
		}
		else if(base->soort== -4) {
			ika= (struct IkaData *)base->d;
			ika->keyverts= 0;
			bop= ika->bpbase.first;
			while(bop) {
				ika->keyverts++;
				bop= bop->next;
			}
			ika->keyrods= 0;
			rod= ika->rodbase.first;
			while(rod) {
				ika->keyrods++;
				rod= rod->next;
			}
			kn= (struct Key *)callocN(sizeof(struct Key)+ 4*ika->keyrods+ (4*4)*ika->keyverts,"insKey-4");
			
			firstkey= &(ika->key);
			bez= ika->ipokey;
	
			ika_to_key(ika, kn);
		}
		else return;
	}
	else {
		return;
	}

	kn->pos= frametofloat( (float)G.cfra,base->sf,base->len,0);
	kn->pos= ipo(bez,kn->pos);
	if(kn->pos==1.0) kn->pos= 0.99999999;
	kn->soort= G.ipomode;

	if(*firstkey==0) *firstkey= kn;
	else {
		pkey= 0;
		k= *firstkey;
		while(k) {
			if(k->pos==kn->pos) {
				error("Already key here!");
				freeN(kn);
				break;
			}
			else if(k->pos>kn->pos) {
				kn->next= k;
				if(pkey) pkey->next=kn;
				else *firstkey=kn;
				break;
			}
			else if(k->next==0) {
				kn->next=0;
				k->next=kn;
				break;
			}
			pkey=k;
			k=k->next;
		}
	}
}

void insertkeypos()
{
	struct Base *base;
	int event;


	if(G.ipomode==20) 
		event= pupmenu("Insert Base Key%t|Active %x1|Selected%x2");
	else if(G.ipomode==21)
		event= pupmenu("Insert Parent Key%t|Active %x1|Selected%x2");
	else if(G.ipomode==22)
		event= pupmenu("Insert Vertex Key%t|Active %x1|Selected%x2");

	if(event== -1) return;
	else if(event== 1) {
		if(G.basact && G.basact->lay & view0.lay) insertkeypos_base(G.basact);
	}
	else {
		base= G.firstbase;
		while(base) {
			if( TESTBASE(base) ) insertkeypos_base(base);
			base= base->next;
		}
	}

	tekenspline2d(1);
}

void deletekeypos_spec(firstkey, nr)
struct Key **firstkey;
short nr;
{
	struct Key *key, *pkey;
	short a=1;

	key= *firstkey;
	pkey= 0;
	while(key) {
		if(a==nr) {
			if(pkey) pkey->next= key->next;
			else *firstkey= key->next;
			freeN(key);
			break;
		}
		pkey= key;
		key= key->next;
		a++;
	}
}

void deletekeypos()
{
	struct Key *key=0,**firstkey,*pkey=0;
	short a=1;

	/* eerst uitzoeken welke key */

	if(G.basact==0) return;

	if(G.ipomode==20 && G.basact->key!=0) {
		if(G.actkey!=0 && okee("Delete Base Key")) {
			key=G.basact->key;
			firstkey= &(G.basact->key);
		}
	}
	else if(G.ipomode==21 && G.basact->pkey!=0) {
		if(G.actkey!=0 && okee("Delete Parent Key")) {
			key= G.basact->pkey;
			firstkey= &(G.basact->pkey);
		}
	}
	if(G.ipomode==22 && G.basact) {
		if(G.actkey!=0 && okee("Delete Vertex Key")) {
			if(G.basact->soort==1) {
				firstkey= &( ( (struct ObData *)G.basact->d )->vv->key );
				key= *firstkey;
			}
			else if ELEM(G.basact->soort, 5, 11) {
				firstkey= &( ( (struct ObData *)G.basact->d )->cu->key );
				key= *firstkey;
			}
			else if(G.basact->soort== -2) {
				firstkey= &( ( (struct MoData *)G.basact->d )->key );
				key= *firstkey;
			}
			else if(G.basact->soort== -4) {
				firstkey= &( ( (struct IkaData *)G.basact->d )->key );
				key= *firstkey;
			}
		}
	}

	if(key==0) return;

	deletekeypos_spec(firstkey, G.actkey);

	loadkeypos(G.basact, G.basact);
	tekenspline2d(1);
	projektie();
}

/* ******************************************** */

long copyK_ipomode, copyK_len, *copyK_data=0;

void copyKey()
{
	/* maak buffer met keydata, onthoud ipomode en keylengte */
	struct Key *key;
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	long a= 1;
	
	if(copyK_data) {
		freeN(copyK_data);
		copyK_data= 0;
	}
	
	if(G.actkey && G.basact) {
		copyK_ipomode= G.ipomode;
		if(G.ipomode==20) {
			copyK_len= 19*4;
			key= G.basact->key;
		}
		else if(G.ipomode==21) {
			copyK_len= 4+ 4*25;
			key= G.basact->pkey;
		}
		else if(G.ipomode==22) {
			ob= (struct ObData *)G.basact->d;
			if(G.basact->soort==1) {
				copyK_len= 4+6*2*ob->vv->vert;
				key= ob->vv->key;
			}
			else if ELEM(G.basact->soort, 5, 11) {
				copyK_len= 3*4*ob->cu->keyverts+4;
				key= ob->cu->key;
			}
			else if(G.basact->soort== -2) {
				mo= (struct MoData *)ob;
				copyK_len= 3*4*mo->keyverts+4;
				key= mo->key;
			}
			else if(G.basact->soort== -4) {
				ika= (struct IkaData *)ob;
				copyK_len= 4*4*ika->keyverts+4*ika->keyrods;
				key= ika->key;
			}
		}
		while(key) {
			if(a==G.actkey) {
				copyK_data= mallocN(copyK_len,"copyKey");
				memcpy(copyK_data, (key+1), copyK_len);
				break;
			}
			a++;
			key= key->next;
		}
	}
}

void pasteKey()
{
	struct Key *key;
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	long len=0, a= 1;

	if(G.ebase) {
		error("Not in EditBase mode");
		return;
	}
	if(copyK_data==0) return;
	if(copyK_ipomode!= G.ipomode) {
		error("Incompatible keys");
		return;
	}
	
	if(G.actkey && G.basact) {
		if(G.ipomode==20) {
			len= 19*4;
			key= G.basact->key;
		}
		else if(G.ipomode==21) {
			len= 4+ 4*25;
			key= G.basact->pkey;
		}
		else if(G.ipomode==22) {
			ob= (struct ObData *)G.basact->d;
			if(G.basact->soort==1) {
				len= 4+6*2*ob->vv->vert;
				key= ob->vv->key;
			}
			else if ELEM(G.basact->soort, 5, 11) {
				len= 3*4*ob->cu->keyverts+4;
				key= ob->cu->key;
			}
			else if(G.basact->soort== -2) {
				mo= (struct MoData *)ob;
				len= 3*4*mo->keyverts+4;
				key= mo->key;
			}
			else if(G.basact->soort== -4) {
				ika= (struct IkaData *)ob;
				len= 4*4*ika->keyverts+4*ika->keyrods;
				key= ika->key;
			}
		}
		if(len==copyK_len) {
			while(key) {
				if(a==G.actkey) {
					memcpy((key+1), copyK_data, copyK_len);
					break;
				}
				a++;
				key= key->next;
			}
			
			loadkeypos(G.basact);
			projektie();
			
		}
		else error("Incompatible key");
	}
}

/* ******************************************* */

extern float *vectoquat();

float *vectoquato(vec, proj)
float *vec;
short proj;
{
	/* vector in vec[0],vec[1],vec[2] */

	static float q1[4];
	float q2[4];
	float co,si,n[3],x2,y2,z2,x1,y1,z1,len1,len,b1;

	/* proj: 0 eerst xy vlak (y-as aligned)
	 *		 1 eerst xz vlak (x-as aligned)
	 *		 2 eerst yz vlak (z-as aligned)
	 */	

	q1[0]=1; 
	q1[1]=q1[2]=q1[3]=0.0;

	x2 = vec[0] ; y2 = vec[1] ; z2 = vec[2];
	if(proj>2) {
		x2= -x2; y2= -y2; z2= -z2;
		proj-= 3;
	}

	len1=fsqrt(x2*x2+y2*y2+z2*z2);
	if(len1 == 0.0) return(q1);

	if(proj==0) len= fsqrt(x2*x2+y2*y2);
	else if(proj==1) len= fsqrt(x2*x2+z2*z2);
	else len= fsqrt(y2*y2+z2*z2);
	
	if(len>0.0) {
		if(proj==0)  {
			b1= facos(y2/len);
			if(x2>0.0) b1= 2*PI-b1;
		}
		else if(proj==1) {
			b1= facos(x2/len);
			if(z2>0.0) b1= 2*PI-b1;
		}
		else {
			b1= facos(z2/len);
			if(y2>0.0) b1= 2*PI-b1;
		}
		b1+=PI;
		co=fcos(b1/2);
		si=fsin(b1/2);
	} else {
		co=1.0; 
		si=0.0; 
		b1=0.0;
	}
	q1[0]=co;
	if(proj==0) q1[3]=si;
	else if(proj==1) q1[2]=si;
	else q1[1]=si;

	x2/=len1; 
	y2/=len1; 
	z2/=len1;
	
	if(proj==0) {
		x1= -fsin(b1);
		y1= fcos(b1);
		z1= 0.0;
	} else if(proj==1) {
		z1= -fsin(b1);
		y1= 0.0;
		x1= fcos(b1);
	} else {
		x1= 0.0;
		y1= -fsin(b1);
		z1= fcos(b1);
	}
	n[0]= y2*z1-z2*y1;
	n[1]= x1*z2-x2*z1;
	n[2]= -y2*x1+y1*x2;
	
	len= fsqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	if(len>0.0) {
		n[0]/=len; 
		n[1]/=len; 
		n[2]/=len;
		
		if(proj==0) b1= x1*x2+y2*y1;
		else if(proj==1) b1= x1*x2+z2*z1;
		else b1= y1*y2+z2*z1;
		
		if(b1<-1.0) b1= -1.0; 
		else if(b1>1.0) b1=1.0;
		b1= (PI-facos(b1))/2.0;
		co= fcos(b1);
		si= fsin(b1);
		q2[0]=co; 
		q2[1]=si*n[0]; 
		q2[2]=si*n[1]; 
		q2[3]=si*n[2];
		QuatMul(q1,q2,q1);
	}
	return(q1);
}

void triatoquat(v1, v2, v3, quat)
float *v1, *v2, *v3, *quat;
{
	/* denkbeeldige x-as, y-as driehoek wordt geroteerd */
	float vec[3], q1[4], q2[4], n[3], si, co, hoek, mat[3][3], imat[3][3];
	
	/* eerst z-as op vlaknormaal */
	CalcNormFloat(v1, v2, v3, vec);

	n[0]= vec[1];
	n[1]= -vec[0];
	n[2]= 0.0;
	Normalise(n);
	
	if(n[0]==0.0 && n[1]==0.0) n[0]= 1.0;
	
	hoek= -0.5*facos(vec[2]);
	co= fcos(hoek);
	si= fsin(hoek);
	q1[0]= co;
	q1[1]= n[0]*si;
	q1[2]= n[1]*si;
	q1[3]= 0.0;
	
	/* v1-v2 lijn terug roteren */
	QuatToMat3(q1, mat);
	Mat3Inv(imat, mat);
	VecSubf(vec, v2, v1);
	Mat3MulVecfl(imat, vec);

	/* welke hoek maakt deze lijn met x-as? */
	vec[2]= 0.0;
	Normalise(vec);

	hoek= 0.5*fatan2(vec[1], vec[0]);
	co= fcos(hoek);
	si= fsin(hoek);
	q2[0]= co;
	q2[1]= 0.0;
	q2[2]= 0.0;
	q2[3]= si;
	
	QuatMul(quat, q1, q2);
	/* QUATCOPY(quat, q1); */
}

void initwarp(base)
struct Base *base;
{
	/* base moet type move zijn */
	
	
}

void calctrack(base,vec,q1)
struct Base *base;
float *vec;
float *q1;
{
	float n[3],*q2;

	n[0]= vec[0]- base->t->r[0];
	n[1]= vec[1]- base->t->r[1];
	n[2]= vec[2]- base->t->r[2];

	q2= vectoquat(n, base->tflag, base->upflag);
	QUATCOPY(q1,q2);
}

void calcfillquat(q,base,childbase,fr)
float *q;
struct Base *base,*childbase;
short fr;
{
	struct MoData *mo;
	float b1,co,si,ipo();

	if(base->soort!= -2) return;
	mo=(struct MoData *)base->d;

	b1= frametofloat( (float)fr, childbase->sf, base->len, base->f2 & 1);

	if(b1>=0.0) {
		if(mo->ipoqfi) b1=ipo(mo->ipoqfi,b1);

		b1= b1*(float)mo->qf;
		b1= PI*b1/360.0;
		co= fcos(b1);
		si= (fsin(b1))/32767.0;
		q[0]= co;
		q[1]= si*(float)mo->n[0];
		q[2]= si*(float)mo->n[1];
		q[3]= si*(float)mo->n[2];
	} else {
		q[0]=1.0; 
		q[1]=0.0; 
		q[2]=0.0; 
		q[3]=0.0;
	}
}

void calcpathdir(base,childbase,fr,vec)		/* richtingsvector */
struct Base *base,*childbase;
short fr;
float *vec;
{
	struct MoData *mo;
	struct DispList *dl;
	float b1, b2, *p, *p1, *p2, fac, vec1[3], vec2[3];
	short s1, s2;

	if(base->soort!= -2) return;
	mo=(struct MoData *)base->d;
	p=mo->data;
	if(p==0) return;

	b1= frametofloat( (float)fr, childbase->sf, base->len, base->f2 & 1);
	b2= frametofloat( (float)(fr+1), childbase->sf, base->len, base->f2 & 1);

	if(base->f2 & 1) {
		b1*=base->len;
		b2*=base->len;
	}
	else {
		b1*=(base->len-1);
		b2*=(base->len-1);
	}
	
	dl= mo->disp.first;
	
	if(b2<b1) {	/* mag alleen bij cyclic bezier */
		if(dl->type==DL_POLY);
		else {
			b2= b1;
			b1-= 1.0;
		}
	}

	s1= ffloor(b1);
	s2= s1+1;
	fac= (float)s2-b1;

	p1= p+ 3*s1;
	p2= p1+3;
	if(s2>=base->len) {
		if(dl->type==DL_POLY) p2=p;	/* cyclic bezier */
		else p2= p1;
	}
	b1= 1.0-fac;

	vec1[0]= fac*p1[0]+b1*p2[0];
	vec1[1]= fac*p1[1]+b1*p2[1];
	vec1[2]= fac*p1[2]+b1*p2[2];
	
	/* tweede vector */
	s1= ffloor(b2);
	s2= s1+1;
	fac= (float)s2-b2;

	p1= p+ 3*s1;
	p2= p1+3;
	if(s2>=base->len) {
		if(dl->type==DL_POLY) p2=p;	/* cyclic bezier */
		else p2= p1;
	}
	b2= 1.0-fac;

	vec2[0]= fac*p1[0]+b2*p2[0];
	vec2[1]= fac*p1[1]+b2*p2[1];
	vec2[2]= fac*p1[2]+b2*p2[2];

	vec[0]= vec2[0]-vec1[0];
	vec[1]= vec2[1]-vec1[1];
	vec[2]= vec2[2]-vec1[2];
}


void followpath(q,base,childbase,fr)
float *q;
struct Base *base,*childbase;
short fr;
{
	struct MoData *mo;
	float *p,*t1,*t2,*calcpath(), *q2, n[3];

	if(base->soort!= -2) return;
	mo=(struct MoData *)base->d;

	p=mo->data;
	if(p==0) return;

	t1=calcpath(base,childbase,G.cfra);
	t2=calcpath(base,childbase,G.cfra+1);

	if(t1==0) return;
	if(t1==t2) {
		if(t1==p) t2+=3;
		else t1-=3;
	}

	n[0]= (*t2- *t1);
	n[1]= (*(t2+1)- *(t1+1));
	n[2]= (*(t2+2)- *(t1+2));

	q2= vectoquat(n, childbase->tflag, childbase->upflag);
	QUATCOPY(q, q2);
}

void followpathn(q,base,childbase,fr)
float *q;
struct Base *base,*childbase;
short fr;
{
	struct MoData *mo;
	float *p,*t1,*t2, *q2, n[3], vec1[3], vec2[3];

	if(base->soort!= -2) return;
	mo=(struct MoData *)base->d;

	p=mo->data;
	if(p==0) return;

	/* calcpathn(base, childbase, G.cfra, vec1); */
	/* calcpathn(base, childbase, G.cfra+1, vec2); */
	
	calcpathdir(base, childbase, G.cfra, n);
	
	/* VecSubf(n, vec2, vec1); */
	
	if(n[0]==0.0 && n[1]==0.0 && n[2]==0.0) {
		/* oude methode */
		t1=calcpath(base,childbase,G.cfra);
		t2=calcpath(base,childbase,G.cfra+1);
	
		if(t1==0) return;
		if(t1==t2) {
			if(t1==p) t2+=3;
			else t1-=3;
		}
	
		n[0]= (*t2- *t1);
		n[1]= (*(t2+1)- *(t1+1));
		n[2]= (*(t2+2)- *(t1+2));
	}
	
	q2= vectoquat(n, base->tflag, base->upflag);
	QUATCOPY(q, q2);
}


void quatmofirst(base, childbase, mat, imat, omat, vec, filt)
struct Base *base,*childbase;
struct Matrix3 *mat,*imat,*omat;
float *vec;
short filt;
{
	struct MoData *mo;
	float q1[4], q2[4], q3[4], cmat[3][3], cent[3];
	short fo;

	q1[0]=0.0; 
	q2[0]=0.0; 
	q3[0]=0.0;

	if(base->soort== -2) {
		mo=(struct MoData *)base->d;
		if(mo->qf) calcfillquat(q1, base, childbase, G.cfra);
		if(mo->qfo) followpathn(q2, base, childbase, G.cfra);
	}
	else if(base->soort== -4) {
		if(childbase->rodnr) get_transform_rod(q1, cent, base, childbase->rodnr);
		else return;
	}
	
	if( (filt & 32)==0 ) {
		if(q1[0]!=0.0 && q2[0]!=0.0) {
			QuatMul(q3,q2,q1);
		}
		else if(q2[0]!=0.0) {
			QUATCOPY(q3,q2);
		}
		else if(q1[0]!=0.0) {
			QUATCOPY(q3,q1);
		}
	}
	if(q3[0]!=0.0) {
		QuatToMat3(q3, imat);

		if( (filt & 16)==0) {
			if(base->f1 & 2) {
				Mat3MulVecfl(imat,vec);

				Mat3CpyMat3(cmat,omat);
				Mat3MulMat3(omat,imat,cmat);
			}
		}

		if(filt & 32);
		else {
			fo=0;
			if(base->f1 & 4) fo=1;
			else if(base->f1 & 8) fo=2;
			if(fo) {
				if(fo==1) Mat3MulMat3(cmat,mat,imat);
				else Mat3MulMat3(cmat,imat,mat);
				Mat3CpyMat3(mat,cmat);
			}
		}
	}
	if( (filt & 16)==0) {
		if(base->soort== -4) {
			vec[0]+= cent[0];
			vec[1]+= cent[1];
			vec[2]+= cent[2];
		}
	}
}

void matmofirst(base,childbase,mat,imat,omat,vec)
struct Base *base,*childbase;
struct Matrix3 *mat,*imat,*omat;
float *vec;
{
	struct MoData *mo;
	short fo;

	if(mo->mkey==0) return;

}

void calcvertexquat(base, q)
struct Base *base;
float *q;
{
	extern ListBase editNurb;
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve, *adrv1;
	struct Nurb *nu;
	struct BPoint *bp;
	struct BezTriple *bezt;
	float qmat[3][3], cmat[3][3], v1[3], v2[3], v3[3], ws;
	long a, count;

	if ELEM3(base->soort, 1, 5, 11) {
		ob= (struct ObData *)base->d;
		/* test */
		if(ob->vl1<1) ob->vl1= 1;
		if(ob->vl2<1) ob->vl2= 2;
		if(ob->vl3<1) ob->vl3= 3;
		if(ob->vl4<1) ob->vl4= 0;
		
		if(base->soort==1) {
			vv= ob->vv;
			ws= vv->ws;
			if(ob->vl1>vv->vert) ob->vl1= 1;
			if(ob->vl2>vv->vert) ob->vl2= 2;
			if(ob->vl3>vv->vert) ob->vl3= 3;
			if(ob->vl4>vv->vert) ob->vl4= 0;
			adrve= (struct VertOb *)(vv+1);
	
			adrv1= adrve+ob->vl1-1;
			VECCOPY(v1, adrv1->c);
			adrv1= adrve+ob->vl2-1;
			VECCOPY(v2, adrv1->c);
			adrv1= adrve+ob->vl3-1;
			VECCOPY(v3, adrv1->c);
		}
		else if ELEM(base->soort, 5, 11) {
			nu= ob->cu->curve.first;
			if(base==G.ebase) nu= editNurb.first;
			
			ws= ob->cu->ws;
			count= 1;
			while(nu) {
				if((nu->type & 7)==1) {
					bezt= nu->bezt;
					a= nu->pntsu;
					while(a--) {
						if(count==ob->vl1) {
							VECCOPY(v1, bezt->vec[1]);
							VECCOPY(v2, bezt->vec[0]);
							VECCOPY(v3, bezt->vec[0]);
							if(base->upflag==0) v3[0]+= 1.0;
							else if(base->upflag==1) v3[1]+= 1.0;
							else v3[2]+= 1.0;
							break;
						}
						/* if(count==ob->vl1) {VECCOPY(v1, bezt->vec[1]);} */
						/* else if(count==ob->vl2) {VECCOPY(v2, bezt->vec[1]);} */
						/* else if(count==ob->vl3) {VECCOPY(v3, bezt->vec[1]); count= 0; break;} */
						count++;
						bezt++;
					}
					if(count==0) break;
				}
				else {
					bp= nu->bp;
					a= nu->pntsu*nu->pntsv;
					while(a--) {
						if(count==ob->vl1) memcpy(v1, bp->vec, 12);
						else if(count==ob->vl2) memcpy(v2, bp->vec, 12);
						else if(count==ob->vl3) {memcpy(v3, bp->vec, 12); count= 0; break;}
						count++;
						bp++;
					}
					if(count==0) break;
				}
				nu= nu->next;
			}
			
		}
		
		QuatToMat3(base->q, qmat);
		Mat3MulMat3(cmat, qmat, base->m);
		Mat3MulFloat(cmat, ws);
		Mat3MulVecfl(cmat, v1);
		Mat3MulVecfl(cmat, v2);
		Mat3MulVecfl(cmat, v3);
		
		if(base->soort==11) {
			VECCOPY(ob->vertloc, v1);
		}
		else {
			VecAddf(ob->vertloc, v1, v2);
			VecAddf(ob->vertloc, ob->vertloc, v3);
			VecMulf(ob->vertloc, 0.3333333);
		}
		triatoquat(v1, v2, v3, q);
	}
	else {
		QUATCOPY(q, base->q);
	}
}

void quatfirst(base,mat,imat,omat,vec,filt)
struct Base *base;
float *mat,*imat,*omat;
float *vec;
short filt;
{
	float cmat[3][3],q[4];
	short fo,qb;

	qb= (base->f & 4)==0;
	if(base->f2 & 4) qb= 1;
	
	if(qb) {
		if(base->f2 & 4) {	/* VERTEX PARENT */				
			calcvertexquat(base, q);
			QuatToMat3(q,imat);
		}
		else QuatToMat3(base->q,imat);

		if( (filt & 16)==0 && (base->f1 & 2) ) {
			Mat3MulVecfl(imat,vec);

			Mat3CpyMat3(cmat,omat);
			Mat3MulMat3(omat,imat,cmat);

		}
		if(filt & 32) return;

		fo=0;
		if(base->f1 & 4) fo=1;
		else if(base->f1 & 8) fo=2;

		if(fo) {
			if(fo==1) Mat3MulMat3(cmat,mat,imat);
			else Mat3MulMat3(cmat,imat,mat);
			Mat3CpyMat3(mat,cmat);
		}

	}

	if(filt & 32) return;

	if(base->t) {
		calctrack(base,base->r,q);
		QuatToMat3(q,imat);
		Mat3MulVecfl(imat,vec);
		Mat3CpyMat3(cmat,omat);
		Mat3MulMat3(omat,imat,cmat);
		Mat3CpyMat3(cmat,mat);
		Mat3MulMat3(mat,imat,cmat);
	}
}

void matfirst(base,mat,omat,vec,filt)
struct Base *base;
float *mat,*omat;
float *vec;
short filt;
{
	short fo,mb;
	float cmat[3][3];

	/* hier stond een overbodige? mat copy bmat */
	/* bmat vervalt omdat base->m zelfde is */
	mb= (base->f & 8)==0;
	if(mb) {
		if( (filt & 16)==0 && (base->f1 & 16) ) {
			Mat3MulVecfl(base->m, vec);

			Mat3CpyMat3(cmat,omat);
			Mat3MulMat3(omat,base->m,cmat);

		}
		if(filt & 64) return;

		fo=0;
		if(base->f1 & 32) fo=1;
		else if(base->f1 & 64) fo=2;
		if(fo) {
			if(fo==1) Mat3MulMat3(cmat,mat,base->m);
			else Mat3MulMat3(cmat,base->m,mat);
			Mat3CpyMat3(mat,cmat);
		}

	}
}


float *calcpath(base,childbase,fr)	/* geeft adres terug */
struct Base *base,*childbase;
short fr;
{
	struct MoData *mo;
	float fac,*p;
	short c,s;

	if(base->soort!= -2) return 0;
	mo=(struct MoData *)base->d;
	p=mo->data;
	if(p==0) return 0;

	/*	c=base->len;
	s=childbase->sf;
	if(fr<1) fr=1;
	while(s>fr) {
		if(base->f2 & 1) s-= c;
		else s=fr;
	}	
	if( (base->f2 & 1)==0 ) if(fr>=s+c) s=fr-c+1;
*/
	fac= frametofloat( (float)fr, childbase->sf, base->len, base->f2 & 1);

	if(base->f2 & 1) fac*= base->len;
	else fac*= (base->len-1);
	s= ffloor(fac);
	return p+ 3*s;
}

short calcpathn(base,childbase,fr,vec)	/* geeft OK terug */
struct Base *base,*childbase;
short fr;
float *vec;
{
	struct MoData *mo;
	struct DispList *dl;
	float b1,*p,*p1,*p2,fac;
	short s1,s2;

	if(base->soort!= -2) return 0;
	mo=(struct MoData *)base->d;
	p=mo->data;
	if(p==0) return 0;

	b1= frametofloat( (float)fr, childbase->sf, base->len, base->f2 & 1);
	if(base->f2 & 1) b1*=base->len;
	else b1*=(base->len-1);
	s1= ffloor(b1);
	s2= s1+1;
	fac= (float)s2-b1;

	p1= p+ 3*s1;
	dl= mo->disp.first;
	if(s2>=base->len) {
		if(dl->type==DL_POLY) p2=p;	/* cyclic bezier */
		else {
			VECCOPY(vec,p1);
			return 1;
		}
	}
	else p2= p1+3;
	b1= 1.0-fac;
	vec[0]= fac*p1[0]+b1*p2[0];
	vec[1]= fac*p1[1]+b1*p2[1];
	vec[2]= fac*p1[2]+b1*p2[2];

	return 1;
}

void matscript(mat)
float mat[][3];
{
	struct Base *base;
	FILE *fp;
	float fac,ease1,ease2,mat1[3][3],mat2[3][3];
	long i;
	long a,b,sta,len;


	fp=fopen("/usr/people/mat.script","r");
	if(fp==NULL) return;

	/* if(G.cfra<140) wrld.mistdist=70000;
	else wrld.mistdist=3000; */

	fscanf(fp,"%d",&sta);
	fscanf(fp,"%d",&len);
	fscanf(fp,"%f",&ease1);
	fscanf(fp,"%f",&ease2);
	for(a=0;a<3;a++) {
		for(b=0;b<3;b++) {
			fscanf(fp,"%f",&mat1[a][b]);
		}
	}
	for(a=0;a<3;a++) {
		for(b=0;b<3;b++) {
			fscanf(fp,"%f",&mat2[a][b]);
		}
	}
	fclose(fp);

	while(G.cfra<sta) {
		sta-=len;
	}
	while(G.cfra>sta+len-1) {
		sta+=len;
	}
	fac= ease((float)(G.cfra-sta+1),(float)len,ease1,ease2);
	fac= fsqrt(fac);
	for(a=0;a<3;a++) {
		for(b=0;b<3;b++) {
			mat[a][b]= fac*mat2[a][b]+ (1-fac)*mat1[a][b];
		}
	}


}

void settrapira(ob,fac)
struct ObData *ob;
float fac;
{
	struct ColBlck *cb;
	short a;
	char tra;

	cb= (struct ColBlck *)(ob+1);

	if(fac<0.11) {	/* zichtbaar */
		tra= 255*( fsqrt(fac/0.11) );
	}
	else {
		tra= 255;
	}
	for(a=0;a<ob->c;a++) {
		if(tra==0) cb->mode&= ~(64+128);
		else cb->mode|= 128;
		cb->tra= tra;
		cb++;
	}
}

/* *********** camera noise ********** */

void cameranoise(struct Base *base, float *bvec, float *bmat)
{
	extern float hashvectf[768];
	float strength, rad, fac, freq, fac1, *fp1, *fp2, *fp3, *fp4, t[4], vec[3];
	float q1[4], q2[4], rmat[3][3], tmat[3][3], sinus, sinus2;
	int ofs;
	
	fac= frametofloat( (float)G.cfra, base->sf, base->len, base->f2 & 1);
	strength= ipo(base->iponoise, fac);

	freq= base->noisefreq/G.framelen;

	if(base->noise & 768) {

		sinus= fsin(0.5*PI*fac*freq);

		sinus2= 1.0-fsin(0.25*PI*fac*freq);
	}
	
	ofs= 3*ffloor( fac*freq);
	fac1= (fac*freq) - ffloor(fac*freq);
		
	set_four_ipo(fac1, t, 3);	/* bspline */
		

	if( base->noise & 1 ) {
		
		if(base->noise & 256) {

			vec[0]= vec[1]= vec[2]= 0.0;
			if(base->noise & 2) vec[0]= sinus;
			if(base->noise & 4) vec[1]= sinus;
			if(base->noise & 8) vec[2]= sinus;
			
		}
		else {
			/* vind de vier hashvects waar het om gaat */
			
			fp1= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp2= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp3= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp4= hashvectf+ (ofs % 768);

			
			vec[0]= vec[1]= vec[2]= 0.0;
			if(base->noise & 2) vec[0]= t[0]*fp1[0]+ t[1]*fp2[0]+ t[2]*fp3[0]+ t[3]*fp4[0];
			if(base->noise & 4) vec[1]= t[0]*fp1[1]+ t[1]*fp2[1]+ t[2]*fp3[1]+ t[3]*fp4[1];
			if(base->noise & 8) vec[2]= t[0]*fp1[2]+ t[1]*fp2[2]+ t[2]*fp3[2]+ t[3]*fp4[2];
		}
		
		fac= strength*base->nvecint;
		vec[0]*= fac;
		vec[1]*= fac;
		vec[2]*= fac;
		
		/* roteren! */
		Mat3MulVecfl(bmat, vec);
		
		bvec[0]+= vec[0];
		bvec[1]+= vec[1];
		bvec[2]+= vec[2];
	}
	if( base->noise & 16 ) {
		
		if(base->noise & 512) {
			vec[0]= sinus2;
			vec[1]= sinus2;
			vec[2]= sinus2;

		}
		else {
			/* vind de vier hashvects waar het om gaat */
			ofs+= 131;
			
			fp1= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp2= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp3= hashvectf+ (ofs % 768);
			ofs+= 3;
			fp4= hashvectf+ (ofs % 768);
			
			vec[0]= t[0]*fp1[0]+ t[1]*fp2[0]+ t[2]*fp3[0]+ t[3]*fp4[0];
			vec[1]= t[0]*fp1[1]+ t[1]*fp2[1]+ t[2]*fp3[1]+ t[3]*fp4[1];
			vec[2]= t[0]*fp1[2]+ t[1]*fp2[2]+ t[2]*fp3[2]+ t[3]*fp4[2];
		}
		
		fac= strength*0.001*base->nrotint;
		vec[0]*= fac;
		vec[1]*= fac;
		vec[2]*= fac;
		
		q2[0]= 1.0; q2[1]= q2[2]= q2[3]= 0.0;
		q1[1]= q1[2]= q1[3]= 0.0;
		if(base->noise & 32) {
			q2[0]= fcos(vec[0]); 
			q2[1]= fsin(vec[0]);
		}
		if(base->noise & 64) {
			q1[0]= fcos(vec[1]);
			q1[2]= fsin(vec[1]);
			QuatMul(q2, q2, q1);
		}
		q1[2]= 0.0;
		if(base->noise & 128) {
			q1[0]= fcos(vec[2]); 
			q1[3]= fsin(vec[2]);
			QuatMul(q2, q2, q1);
		}
		QuatToMat3(q2, rmat);
		Mat3CpyMat3(tmat, bmat);
		Mat3MulMat3(bmat, tmat, rmat);
	}
}


void parentbase(base,mat4,omat)
struct Base *base;
float mat4[][4];
float *omat;
{
	struct Base *b, *childbase;
	struct ObData *ob;
	struct MoData *mo;
	float fac, q[4], mat[3][3], imat[3][3], cmat[3][3], *pa, pavec[3];
	float vec[3], len;
	long *temp;
	short qb, mb, filt, cfraont;

	if(base==0) return;
	if(G.hie && base->pkey && base->p==0) {
		temp= (long *)(base->pkey+1);
		temp++;
		memcpy(mat4, temp, 4*16);
		memcpy(omat, temp+16, 4*9);
		VECCOPY(base->r, mat4[3]);

		return;
	}

	VECCOPY(vec,base->v);

	qb= ((base->f & 4)==0);
	mb= ((base->f & 8)==0);

	if(qb) QuatToMat3(base->q,imat);
	if(qb && mb) {
		if(base->f1 & 1) Mat3MulMat3(mat,base->m,imat);
		else Mat3MulMat3(mat,imat,base->m);
	}
	else if(qb) Mat3CpyMat3(mat,imat);
	else Mat3CpyMat3(mat,base->m);

	if(G.hie) {
		if(base->p || base->t) {
			b= base->p;
			filt= base->f;
			if(b) {
				VECCOPY(vec,base->o);
				
				/*  AII receptor patch */
				/* if(view0.grid==1002) {
					if(strcmp(base->str, "a2")==0) {
						fac= frametofloat( (float)G.cfra, 90, base->len, base->f2 & 1);
						fac= 1.0+4.0*(1.0-fac);
						vec[0]*= fac;
						vec[1]*= fac;
						vec[2]*= fac;
					}
				}
				*/
				
				/* schuifpuzzelpatch */
				/* if(view0.grid==5001) {
					if(strcmp(base->str, "schuif")==0) {
						schuifpuzzel_base(base);
						VECCOPY(vec, base->o);
					}
				}
				*/
				
				childbase= base;
				
				if(base->f2 & 8) {	/* Startframe only Key */
					childbase= b;
				}
				
				Mat3One(omat);
			}

			while(b) {
				
				cfraont= G.cfra;
				if(b->soort== -2) {
					mo= (struct MoData *)b->d;
					
					if(mo->lock) {
						nofield= 1;
						if(mo->f & 4) {	/* duplibase */
							if(induplibase==0) G.cfra= mo->lock;
						}
						else G.cfra= mo->lock;
					}
					
				} 
								
				if(b->key) {
					if(childbase->sf>1) {
						memcpy(&copybase, b, 120);
						loadkeypostype(20, b, childbase);
						loadkeypostype(21, b, childbase);
					}
				}
				
				if(b->pkey && b->p==0) {
					temp= (long *)(b->pkey+1);
					temp++;
					Mat3CpyMat4(imat, temp);
					Mat3CpyMat3(cmat, mat);
					Mat3MulMat3(mat, imat, cmat);
					
					Mat3MulVecfl(imat, vec);
					
					Mat3CpyMat3(cmat, mat);
					memcpy(imat, temp+16, 4*9);
					Mat3MulMat3(omat, imat, cmat);
					
					pa= (float *)temp;
					vec[0]+= pa[12];
					vec[1]+= pa[13];
					vec[2]+= pa[14];
			
					break;
				}
				
				pa=0;

				if(b->soort== -2 || b->soort== -4) {
					quatmofirst(b, childbase, mat, imat, omat, vec, filt);
				}
								
				qb= ((b->f & 4)==0);
				mb= ((b->f & 8)==0);
				if(b->f1 & 1) {
					quatfirst(b,mat,imat,omat,vec,filt);
					matfirst(b,mat,omat,vec,filt);
				}
				else {
					matfirst(b,mat,omat,vec,filt);
					quatfirst(b,mat,imat,omat,vec,filt);
				}
				
				if(b->f2 & 4) {
					ob= (struct ObData *)b->d;
					vec[0]+= ob->vertloc[0];
					vec[1]+= ob->vertloc[1];
					vec[2]+= ob->vertloc[2];
				}
				
				if( (filt & 16)==0 && calcpathn(b,childbase,G.cfra,pavec) ) {
					if(qb && mb) {
						if(b->f1 & 1) Mat3MulMat3(cmat,b->m,imat);
						else Mat3MulMat3(cmat,imat,b->m);
					}
					else if(qb) Mat3CpyMat3(cmat,imat);
					else if(mb) Mat3CpyMat3(cmat,b->m);
					if(qb || mb) Mat3MulVecfl(cmat,pavec);
					vec[0]+= pavec[0];
					vec[1]+= pavec[1];
					vec[2]+= pavec[2];
				}

				if(b->p) {
					if( (filt & 16)==0 ) {
						vec[0]+= b->o[0];
						vec[1]+= b->o[1];
						vec[2]+= b->o[2];
					}
					filt|= b->f;
					
					if(b->key) {
						if(childbase->sf>1) memcpy(b, &copybase, 120);
					}
					
					if(childbase->f2 & 2) childbase= b;
					
					b= b->p;
				} else {
					if( (filt & 16)==0 ) {
						if(b->pkey==0) {
							vec[0]+= b->v[0];
							vec[1]+= b->v[1];
							vec[2]+= b->v[2];
						}
						else {
							pa= (float *)(b->pkey+1);
							pa++;
							vec[0]+= pa[12];
							vec[1]+= pa[13];
							vec[2]+= pa[14];
						}
					}
					if(b->key) {
						if(childbase->sf>1) memcpy(b, &copybase, 120);
					}

					b= 0;
				}
				/* if(childbase->f2 & 2) childbase= b; */
				
				G.cfra= cfraont;	/* ivm mo->lock */
				nofield= 0;	/* ivm mo->lock */
				
			}
			if(base->t) {
				calctrack(base,vec,q);
				QuatToMat3(q,imat);
				Mat3CpyMat3(cmat,mat);
				Mat3MulMat3(mat,imat,cmat);
			}
			VECCOPY(base->r2,base->o);
			Mat3MulVec(omat,base->r2);
		}
	}

	if(G.moving==0 && (base->noise & 17)) cameranoise(base, vec, mat);

	VECCOPY(base->r,vec);

	Mat4CpyMat3(mat4,mat);
	mat4[3][0] = vec[0];
	mat4[3][1] = vec[1];
	mat4[3][2] = vec[2];
}

short compbase(b1,b2)
struct Base *b1,*b2;
{
	short a,b;

	if(b1) if(b2)
		if(b1->v[0]==b2->v[0])
			if(b1->v[1]==b2->v[1])
				if(b1->v[2]==b2->v[2]) {
					for(a=0;a<3;a++) {
						for(b=0;b<3;b++) {
							if(b1->m[a][b]!=b2->m[a][b]) return 0;
						}
					}
					return 1;
				}
	return 0;
}

short compmat(m1,m2)
float m1[][4],m2[][4];
{
	short a,b;

	for(a=0;a<4;a++) {
		for(b=0;b<3;b++) {
			if(m1[a][b]!=m2[a][b]) return 0;
		}
	}
	return 1;
}

struct Base *vertexduplilist(first, par)
struct Base *first, *par;
{
	struct Base *base,*b,*new;
	struct ObData *ob;
	struct VertOb *adrve;
	float vec[3], pvec[3], dvec[3], qone[4], bmat[4][4], pmat[4][4], mat[3][3];
	float *q2;
	int totvert, a;
	
	qone[0]= 1.0; 
	qone[1]=qone[2]=qone[3]= 0.0;

	parentbase(par, pmat, mat);
	ob= (struct ObData *)(par->d);
	Mat4MulFloat3(pmat, ob->vv->ws);

	base= G.firstbase;
	while(base) {
		if(base->soort>0 && (base->lay & view0.lay) && G.ebase!=base) {
			b= base->p;
			while(b) {
				if(b==par) {
					parentbase(base, bmat, mat);
					
					adrve= (struct VertOb *)(ob->vv+1);
					adrve+= (ob->vv->vert-1);
					VECCOPY(pvec, adrve->c);
					Mat4MulVecfl(pmat, pvec);

					adrve= (struct VertOb *)(ob->vv+1);
					
					if(ob->ef==4) totvert= build_schiphol(par, ob->vv->vert);
					else totvert= ob->vv->vert;
					
					for(a=1; a<totvert; a++, adrve++) {
					
						/* bereken de extra offset (tov. nulpunt parent) die de kinderen krijgen */
						VECCOPY(vec, adrve->c);
						Mat4MulVecfl(pmat, vec);
						VecSubf(vec, vec, pmat[3]);
						VecAddf(vec, vec, bmat[3]);
						
						new= (struct Base *)mallocN(sizeof(struct Base),"newbasedupad");
						memcpy(new, base, sizeof(struct Base));
						Mat3CpyMat4(new->m, bmat);
						
						VECCOPY(new->v, vec);
						
						if(par->duplirot) {
							VecSubf(dvec, pvec, vec);
							q2= vectoquat(dvec, base->tflag, base->upflag);
							QUATCOPY(new->q, q2);
							new->f= 128;
						}
						else {
							QUATCOPY(new->q, qone);
							new->f= 4+128;
						}
						new->p= 0;
						new->t= 0;
						new->borig= base;
						if(first) first->next= new;
						else duplilist= new;
						first= new;
						first->next= 0;
						
						VECCOPY(pvec, vec);
					
					}
					break;
				}
				b= b->p;
			}
		}
		base= base->next;
	}

	return first;
}

struct Base *makeduplilist(first,par)
struct Base *first,*par;
{
	struct Base *base,*b,*new;
	struct MoData *mo;
	float qone[4], bmat[4][4],smat[4][4],tmat[4][4],mat[3][3];
	short cfra,sfra,efra,ident, delta, doit, cfraont;

	qone[0]= 1.0; 
	qone[1]=qone[2]=qone[3]= 0.0;

	mo= (struct MoData *)par->d;

	cfraont= G.cfra;
	if(mo->lock) cfra= mo->lock;
	else cfra= G.cfra;
		
	base= G.firstbase;
	while(base) {
		if(base->soort>0 && (base->lay & view0.lay) && G.ebase!=base) {
			b= base->p;
			
			/* PATCH */
			if(base->holo1==99) b= 0;
			
			while(b) {
				if(b==par) {
					/* dup op cfra en cfra-1 ... tot cfra-lenPad */
					/* als cfra>sf+lenPad sfra=sf+lenPad
					/* achteruit tellen dus */
					
					sfra= cfra-1;
					
					if(mo->dupon && mo->dupoff) delta= mo->dupon+mo->dupoff;
					else delta= 0;
					
					if(par->f2 & 1) sfra= base->sf+par->len-1;  /* cyclic */
					
					if(sfra<1) break;
					
					G.cfra= cfra;
					
					parentbase(base,bmat,mat);

					if(sfra>=base->sf+par->len) sfra= base->sf+par->len-1;

					efra= sfra-par->len+1;
					if(efra<1) efra=1;
					
					for(G.cfra=sfra; G.cfra>=efra; G.cfra--) {
					
						doit= 1;
						if(delta) {
							doit= abs((G.cfra -cfra) % delta);
							if(doit<mo->dupon) doit= 1;
							else doit= 0;
						}
						if(doit) {
							loadkeypos(base, base);
							parentbase(base,tmat,mat);
							ident= compmat(tmat,bmat);
							if(ident==0 && G.cfra!=sfra)
								ident= compmat(smat,tmat);
							if(ident==0) {
								new= (struct Base *)mallocN(sizeof(struct Base),"newbasedupad");
								memcpy(new,base,sizeof(struct Base));
								Mat3CpyMat4(new->m,tmat);
								VECCOPY(new->v,tmat[3]);
								QUATCOPY(new->q, qone);
								new->f1= base->f1;
								new->f2= base->f2;
								new->f= 4+128;
								new->p= 0;
								new->t= 0;
								new->borig= base;
								if(first) first->next= new;
								else duplilist= new;
								first= new;
								first->next= 0;
								memcpy(smat,tmat,64);
							}
						}
					}
					G.cfra= cfra;
					loadkeypos(base, base);	/* herstellen */
				}
				b= b->p;
			}
		}
		base= base->next;
	}
	
	G.cfra= cfraont;
	
	return first;
}

struct Base *duplibase()
{
	/* maakt nieuwe lijst met duplicate bases */
	struct Base *base,*next;
	struct MoData *mo;

	/* if(G.ebase && G.ebase->soort!= -2) return 0; */

	/* eerste oude vrijgeven */
	base= duplilist;
	while(base) {
		next= base->next;
		freeN(base);
		base= next;
	}
	duplilist= 0;

	if(G.firstbase==0 || G.hie==0) return 0;
	
	induplibase= 1;

	/* vind de duplibase, next is de plek waar toegevoegd wordt */
	next= duplilist;
	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base->soort== -2 && base->p==0) {
				mo= (struct MoData *)base->d;
				if(mo->f & 4) {
					next= makeduplilist(next, base);
				}
			}
			else if(base->soort==1) {
				if(base->dupli) 
					next= vertexduplilist(next, base);
			}
		}
		base= base->next;
	}

	induplibase= 0;

	return duplilist;
}

void makeduplibase_real()
{
	struct Base *base,*tfirst,*tlastor,*tlastnew, *borig;

	tfirst= G.firstbase;
	G.firstbase= duplibase();
	
	if(G.firstbase) {
		/* lastbase onhouden opnieuw berekenen en selectflag zetten*/
		tlastor= G.lastbase;
		G.lastbase= base= G.firstbase;
		while(base) {
			borig= base->borig;
			if(borig->f & 1) {
				base->f|=1;
				base->f&= ~128;
				base->f&= ~8;
			}
			G.lastbase= base;
			
			base= base->next;
		}
		tlastnew= G.lastbase;
		G.f|=1; /* okee en grabber in adduplicate() uitschakelen */
		adduplicate();
		G.f-=1;
		
		/* alles vanaf tlastnew zijn real_dupli's */
		tlastor->next= tlastnew->next;
		tlastnew->next= 0;
	}
	
	G.firstbase= tfirst;
	
	/* allemaal gedoe: even zeker weten dat de lastbase klopt */
	base= G.firstbase;
	while(base) {
		G.lastbase= base;
		base= base->next;
	}
}

