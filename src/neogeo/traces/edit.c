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

/*	edit.c	*/


/*	 
bindkey -r f1,'make\n'
bindkey -r f2,'/usr/people/trace/traces\n'

	muiscursor()
	deselectall()
	deselectall_ex(b)
	clearbase(mode)
	muisselect()
	copymenu()
	snapmenu()

	maketrack()
	makeparent()
	movetolayer()	
	selector()
	recalcnormals()
	apply()
	
	initprintoverdraw()
	grabber()
	roteer()
	sizing()
*/


#include <gl/gl.h>
#include <math.h>
#include <gl/device.h>
#include <string.h>
#include "/usr/people/include/Trace.h"

#define PARENT(base)	( (base)->p || (base)->pkey )

extern short backbufok;
extern ListBase editNurb;
extern ListBase bpbase;
extern void VecMulf(float *v1, float f);

long *getbasekeyadr();


void muiscursor_oud()
{
	float fx,fy,fz,dx,dy,fw, vec4[4];
	long *t;
	Device mdev[2];
	short mval[2], mx, my;

	t= view0.muis;
	winset(G.winar[0]);
	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	while(getbutton(LEFTMOUSE)) {
		getdev(2, mdev, mval);

		mval[1]-= S.vys;
		if(mval[0]!=view0.mx || mval[1]!=view0.my) {

			pnti(t[0], t[1], t[2]);
			getgpos(&fx,&fy,&fz,&fw);

			mx= view0.mx; 
			my= view0.my;
			/* als mx==3200: toch even proberen de schermco te berekenen */
			if(view0.mx==3200) {
				VECCOPY(vec4, view0.muis);
				vec4[3]= 1.0;
				Mat4MulVec4fl(G.persmat, vec4);
				if( vec4[3]!=0  ) {
					fx= 640.0+640.0*vec4[0]/vec4[3];
					if( fabs(fx)<30000) {
						fy= 400.0+400.0*vec4[1]/vec4[3];
						if(fabs(fy)<30000) {
							mx= fx; 
							my= fy;
						}
					}
				}
			}

			if(mx!=3200) {
				dx= (mx-mval[0])*fw/640.0;
				dy= (my-mval[1])*fw/400.0;

				t[0]-= (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
				t[1]-= (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
				t[2]-= (G.persinv[0][2]*dx + G.persinv[1][2]*dy);
			} else {
				dx= ((float)(mval[0]-640))*fw/640.0;
				dy= ((float)(mval[1]-400))*fw/400.0;
				fz= fz/fw;
				t[0]= (G.persinv[0][0]*dx + G.persinv[1][0]*dy+ G.persinv[2][0]*fz)-view0.ofs[0];
				t[1]= (G.persinv[0][1]*dx + G.persinv[1][1]*dy+ G.persinv[2][1]*fz)-view0.ofs[1];
				t[2]= (G.persinv[0][2]*dx + G.persinv[1][2]*dy+ G.persinv[2][2]*fz)-view0.ofs[2];
			}
			berekenschermco(t,&view0.mx);
			tekenoverdraw(0);
		}
		if(getbutton(RIGHTMOUSE)!=0 && G.ebase!=0) {
			if(G.ebase->soort==1) addvert_ebase();
			else if ELEM(G.ebase->soort, 5, 11) addvert_Nurb();
			break;
		}
	}
}

#define MOVES 50
#define DONT_KNOW 0
#define GRAB 1
#define ROTATE 2
#define SIZE 3

long interpret_move(float mouse[][2], long count)
{
	long i, j, dist, dir = 0;
	float x1, x2, y1, y2, d1, d2, md, d, inp, sq;

	if (count <= 10) return (GRAB);

	/* filteren */
	/*printf("count %d\n", count);*/
	for( j = 3 ; j > 0; j--){
		x1 = mouse[1][0];
		y1 = mouse[1][1];
		for (i = 2; i < count; i++){
			x2 = mouse[i-1][0];
			y2 = mouse[i-1][1];
			mouse[i-1][0] = ((x1 + mouse[i][0]) /4.0) + (x2 / 2.0);
			mouse[i-1][1] = ((y1 + mouse[i][1]) /4.0) + (y2 / 2.0);
			x1 = x2;
			y1 = y2;
		}
	}

	/*
    cpack(0xff);
    bgnline();
	for (i = 0; i < count; i++){
	    v2f(mouse[i]);
	}
    endline();
	*/
	/* maak directions overzicht */
	for (i = 0; i <= count - 2; i++){
		x1 = mouse[i][0] - mouse[i + 1][0];
		y1 = mouse[i][1] - mouse[i + 1][1];

		if (x1 < -0.5){
			if (y1 < -0.5) dir |= 32;
			else if (y1 > 0.5) dir |= 128;
			else dir |= 64;
		} else if (x1 > 0.5){
			if (y1 < -0.5) dir |= 8;
			else if (y1 > 0.5) dir |= 2;
			else dir |= 4;
		} else{
			if (y1 < -0.5) dir |= 16;
			else if (y1 > 0.5) dir |= 1;
			else dir |= 0;
		}
	}


	/*    for (i = 7; i >= 0; i--){
	if (dir & (1 << i)) printf("X");
	else printf(".");
    }
    printf("\n");
	*/
	/* alle kruisjes naar rechts halen */
	for (i = 7; i>=0 ; i--){
		if (dir & 128) dir = (dir << 1) + 1;
		else break;
	}
	dir &= 255;
	for (i = 7; i>=0 ; i--){
		if ((dir & 1) == 0) dir >>= 1;
		else break;
	}

	/*    for (i = 7; i >= 0; i--){
	if (dir & (1 << i)) printf("X");
	else printf(".");
    }
    printf("\n");
	*/
	/* theorie zegt: 1 richting: rechte lijn
     * meer aaneengesloten richtingen: cirkel
     * onderbroken en minstens 1 bit gezet in hoogste 4 bits: size
     */
	switch(dir){
	case 1:
		return (GRAB);
		break;
	case 3:
	case 7:
		x1 = mouse[0][0] - mouse[count >> 1][0];
		y1 = mouse[0][1] - mouse[count >> 1][1];
		x2 = mouse[count >> 1][0] - mouse[count - 1][0];
		y2 = mouse[count >> 1][1] - mouse[count - 1][1];
		d1 = (x1 * x1) + (y1 * y1);
		d2 = (x2 * x2) + (y2 * y2);
		sq = sqrtf(d1);
		x1 /= sq; 
		y1 /= sq;
		sq = sqrtf(d2);
		x2 /= sq; 
		y2 /= sq;
		inp = (x1 * x2) + (y1 * y2);
		/*printf("%f\n", inp);*/
		if (inp > 0.9) return (GRAB);
		else return (ROTATE);
		break;
	case 15:
	case 31:
	case 63:
	case 127:
	case 255:
		return (ROTATE);
		break;
	default:
		/* bij size moeten minstens een van de hogere bits gezet zijn */
		if (dir < 16) return (ROTATE);
		else return (SIZE);
	}

	return (DONT_KNOW);
}


void muiscursor()
{
	float fx,fy,fz,dx,dy,fw, vec4[4];
	long *t, i = 1, ofsx, ofsy, event, mousex, mousey;
	Device mdev[2];
	short mval[2], mx, my, val;
	float mcords[MOVES][2];
	void grabber(), sizing(), roteer();

	t= view0.muis;
	winset(G.winar[0]);

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	if(G.qual==0) {

	
		qdevice(MOUSEX);
		qdevice(MOUSEY);
	
		getorigin(&ofsx, &ofsy);
		getdev(2, mdev, mval);
		mcords[0][0] = mousex = mval[0] - ofsx;
		mcords[0][1] = mousey = mval[1] - ofsy;
	
		drawmode(OVERDRAW);
		persp(0);
		color(3);
		bgnline();
	
		while(getbutton(LEFTMOUSE)) {
			switch (traces_qread(&val)){
			case MOUSEY:
				mousey = val - ofsy;
				if (mousex != mcords[i-1][0] || mousey != mcords[i-1][1]) {
					mcords[i][0] = mousex;
					mcords[i][1] = mousey;
					v2f(mcords[i]);
					i++;
				}
				break;
			case MOUSEX:
				mousex = val - ofsx;
				break;
			case LEFTMOUSE:
				break;
			default:
				i = -1;
				break;
			}
			if (i == MOVES || i == -1) break;
		}
		/* je hebt de neiging om de muis los te laten voordat de beweging afgemaakt is, dus even door lezen */
	
		if (qtest() == 0) sginap(3);
		while (event = qtest() && i < MOVES){
			if (event != MOUSEX && event != MOUSEY) break;
			/*printf("adding\n");*/
			event = traces_qread(&val);
			if (event == MOUSEY){
				mousey = val - ofsy;
				if (mousex != mcords[i-1][0] || mousey != mcords[i-1][1]) {
					mcords[i][0] = mousex;
					mcords[i][1] = mousey;
					v2f(mcords[i]);
					i++;
				}
			} else mousex = val - ofsx;
			if (i == MOVES) break;
			if (qtest() == 0) sginap(3);
		}
	
		endline();
		persp(1);
		drawmode(NORMALDRAW);
	
		unqdevice(MOUSEX);
		unqdevice(MOUSEY);
	
		if (i > 5){
			i = interpret_move(mcords, i);
			switch (i){
			case GRAB:
				grabber();
				break;
			case ROTATE:
				roteer();
				break;
			case SIZE:
				sizing();
				break;
			}
	
			drawmode(OVERDRAW);
			color(0);
			clear();
			drawmode(NORMALDRAW);
			tekenoverdraw(0);
			return;
		}
	}
	
	getdev(2, mdev, mval);
	mval[1]-= S.vys;
	if(mval[0]!=view0.mx || mval[1]!=view0.my) {

		pnti(t[0], t[1], t[2]);
		getgpos(&fx,&fy,&fz,&fw);

		mx= view0.mx;
		my= view0.my;
		/* als mx==3200: toch even proberen de schermco te berekenen */
		if(view0.mx==3200) {
			VECCOPY(vec4, view0.muis);
			vec4[3]= 1.0;
			Mat4MulVec4fl(G.persmat, vec4);
			if( vec4[3]!=0  ) {
				fx= (S.vxw/2)+(S.vxw/2)*vec4[0]/vec4[3];
				if( fabs(fx)<30000) {
					fy= (S.vyw/2)+(S.vyw/2)*vec4[1]/vec4[3];
					if(fabs(fy)<30000) {
						mx= fx;
						my= fy;
					}
				}
			}
		}

		if(mx!=3200) {
			dx= (mx-mval[0])*fw/(S.vxw/2);
			dy= (my-mval[1])*fw/(S.vyw/2);

			t[0]-= (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
			t[1]-= (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
			t[2]-= (G.persinv[0][2]*dx + G.persinv[1][2]*dy);
		} else {
			dx= ((float)(mval[0]-(S.vxw/2)))*fw/(S.vxw/2);
			dy= ((float)(mval[1]-(S.vyw/2)))*fw/(S.vyw/2);
			fz= fz/fw;
			t[0]= (G.persinv[0][0]*dx + G.persinv[1][0]*dy+ G.persinv[2][0]*fz)-view0.ofs[0];
			t[1]= (G.persinv[0][1]*dx + G.persinv[1][1]*dy+ G.persinv[2][1]*fz)-view0.ofs[1];
			t[2]= (G.persinv[0][2]*dx + G.persinv[1][2]*dy+ G.persinv[2][2]*fz)-view0.ofs[2];
		}
		berekenschermco(t,&view0.mx);
		tekenoverdraw(0);
	}
	if(G.ebase!=0 && (getbutton(RIGHTMOUSE)!=0 || (G.qual & 48) ) ) {
		if(G.ebase->soort==1) addvert_ebase();
		else if ELEM3(G.ebase->soort, 5, 11, -2) addvert_Nurb();
	}
}


void deselectall()	/* is toggle */
{
	struct Base *base;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct EditVert *eve;
	struct BodyPoint *bop;
	long a, b;
	short xs,ys;

	winset(G.winar[0]);
	if(G.ebase && G.ebase->soort==1) {
		if(G.ebase->lay & view0.lay) {
			a= 0;
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					a= 1;
					break;
				}
				eve= eve->next;
			}
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->h==0) {
					if(a) eve->f&= -2;
					else eve->f|= 1;
				}
				eve= eve->next;
			}
			tekenvertices_ext();
		}
	}
	else if(G.ebase && ELEM3(G.ebase->soort, 5, 11, -2) ) {
		if(G.ebase->lay & view0.lay) {
			a= 0;
			nu= editNurb.first;
			while(nu) {
				if((nu->type & 7)==1) {
					b= nu->pntsu;
					bezt= nu->bezt;
					while(b--) {
						if(bezt->hide==0) {
							if(bezt->f1 & 1) {
								a=1; 
								break;
							}
							if(bezt->f2 & 1) {
								a=1; 
								break;
							}
							if(bezt->f3 & 1) {
								a=1; 
								break;
							}
						}
						bezt++;
					}
				}
				else {
					b= nu->pntsu*nu->pntsv;
					bp= nu->bp;
					while(b--) {
						if(bp->hide==0) {
							if(bp->f1 & 1) {
								a=1; 
								break;
							}
						}
						bp++;
					}
				}
				if(a) break;
				nu= nu->next;
			}

			nu= editNurb.first;
			while(nu) {
				if((nu->type & 7)==1) {
					b= nu->pntsu;
					bezt= nu->bezt;
					while(b--) {
						if(bezt->hide==0) {
							if(a) {
								bezt->f1 &= ~1;
								bezt->f2 &= ~1;
								bezt->f3 &= ~1;
							}
							else {
								bezt->f1 |= 1;
								bezt->f2 |= 1;
								bezt->f3 |= 1;
							}
						}
						bezt++;
					}
				}
				else {
					b= nu->pntsu*nu->pntsv;
					bp= nu->bp;
					while(b--) {
						if(bp->hide==0) {
							if(a) bp->f1 &= ~ 1;
							else bp->f1 |= 1;
						}
						bp++;
					}
				}
				nu= nu->next;
			}
			projektie();
		}
	}
	else if(G.ebase && G.ebase->soort== -4) {
		if(G.ebase->lay & view0.lay) {
			
			a= 0;
			bop= bpbase.first;
			while(bop) {
				if(bop->f & 1) {
					a= 1;
					break;
				}
				bop= bop->next;
			}
			
			bop= bpbase.first;
			while(bop) {
				if(a) bop->f= 0;
				else bop->f= 1;
				
				bop= bop->next;
			}
			projektie();
		}
	}
	else if(G.ebase);
	else {
		a= 0;
		base=G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				a= 1;
				break;
			}
			base= base->next;
		}
		base=G.firstbase;
		while(base) {
			if(base->lay & view0.lay) {
				if(a) base->f&= -2;
				else base->f|= 1;
			}
			base= base->next;
		}

		projektie();
	}
	countall();
	tekenoverdraw(0);
}

void deselectall_ex(b)   /* alles deselect behalve b */
struct Base *b;
{
	struct Base *base;

	base=G.firstbase;
	while(base) {
		if(base->f & 1) {
			if(base->lay & view0.lay) {
				if(b!=base) {
					base->f -=1;
					tekenbase_ext(base);
				}
			}
		}
		base= base->next;
	}
}

short setparentflags(lay,flag)
short lay,flag;
{
	/* alle bases met kinderen (lay en flag) krijgen base->f2 & 16 */
	/* als dit voorkomt, return 1 */
	struct Base *base;
	short ret=0;

	base= G.firstbase;
	while(base) {
		base->f2&= ~16;
		base= base->next;
	}
	base= G.firstbase;
	while(base) {
		if(base->lay & lay) {
			if(base->f & flag) {
				if(base->p) {
					if(base->p->lay & lay) {
						if(base->p->f & flag) {
							base->p->f2|=16;
							ret= 1;
						}
					}
				}
			}
		}
		base= base->next;
	}
	return ret;
}

void clearbase(mode)
short mode;
{
	struct Base *base;
	struct ObData *ob;
	float *af,tmat[4][4],omat[3][3];
	long *ai;
	short parentlus;

	switch(mode) {
	case 1:
		if((mode= pupmenu("OK ? %t|Clear Parent %x1|and keep transform %x6"))== -1) return;
		parentlus=setparentflags(view0.lay,1);
		if(G.f & 4) {
			if(mode==6) makeduplibase_real();
		}
		break;
	case 2:
		if(okee("Clear Track")==0) return;
		break;
	case 3:
		if(okee("Clear Matrix")==0) return;
		break;
	case 4:
		if(okee("Clear Origin")==0) return;
		break;
	case 5:
		if(okee("Clear Quaternion")==0) return;
		break;
	case 7:
		/* pas op 6 is in gebruik */
		break;
	}
	base=G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			switch(mode){
			case 1: 
			case 6:
				/* bij mode 6: alleen parent wissen als base geen ouder is */
				if(mode==1) base->p=0;
				else if(base->p) {
					if((base->f2 & 16)==0) {
						parentbase(base,tmat,omat);
						Mat3CpyMat4(base->m,tmat);
						VECCOPY(base->v,tmat[3]);
						base->q[0]= 1.0;
						base->q[1]=base->q[2]=base->q[3]= 0.0;
						base->p=0;
					}
				}
				if(base->p==0) {
					base->skelnr= 0;
					base->rodnr= 0;
				}
				if(base->soort & 1) {
					ob= (struct ObData *)base->d;
					if(ob->floatverts) freeN(ob->floatverts);
					ob->floatverts= 0;
				}
				break;
			case 2:
				base->t=0; 
				break;
			case 3:
				ai= getbasekeyadr(base,3);
				Mat3One(ai);
				if(base->key==0)
					base->f= (base->f | 8); 
				break;
			case 4:
				ai= getbasekeyadr(base,1);
				ai[0]=ai[1]=ai[2]=0;
				break;
			case 5:
				af= (float *)getbasekeyadr(base,2);
				af[0]=1.0; 
				af[1]=af[2]=af[3]=0.0;
				if(base->key==0)
					base->f= (base->f | 4); 
				break;
			}
		}
		base= base->next;
		if(mode==6 && base==0) {
			if(parentlus) base= G.firstbase;
			parentlus= setparentflags(view0.lay,1);
		}
	}
	projektie();
}

void convertmenu()
{
	struct ObData *ob,*obact;
	struct PerfSph *ps;
	struct ColBlck *col,*ocol;
	struct Base *base,*tbase;
	struct Key *key, *keyn, *prevk;
	long len;
	short event,colt,ocolt,min,type;
	char menustr[160];

	if(G.ebase) return;
	type= 0;
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(type==0) type= base->soort;
			else if(type!=base->soort) {
				type= 0;
				break;
			}
		}
		base= base->next;
	}
	if(type==0 || type==1 || type==2) {
		error("Can't convert");
		return;
	}

	menustr[0]= 0;
	if(type== -2) {
		strcpy(menustr, "Convert Move Base to %t|");
		strcat(menustr, "Curve%x11");
	}
	else if(type==5) {
		strcpy(menustr, "Convert Nurbs Object to %t|");
		strcat(menustr, "Triface%x1");
	}
	else if(type==9) {
		strcpy(menustr, "Convert Text Object to %t|");
		strcat(menustr, "Nurbs%x5|Curve%x11");
	}
	else if(type==11) {
		strcpy(menustr, "Convert Curve Object to %t|");
		strcat(menustr, "Triface%x1|Move%x102|Nurbs%x11");
	}

	event= pupmenu(menustr);

	if(event== -1) return;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			switch(type) {
			case 102:
				if(event==11) move_to_curve(base);
				break;
			case 5:
				if(event==1) nurbs_to_triface(base);
				break;
			case 9:
				if(event==11) text_to_curve(base);
				break;
			case 11:
				if(event==1) nurbs_to_triface(base);
				else if(event==102) curve_to_move(base);
				break;
			}
		}
		base= base->next;
	}
	if(G.mainb==5) tekenobjbuts(1);
	else if(G.mainb==9) tekeneditbuts(0);
	projektie();
}

void ebase_copymenu()
{
	extern struct Nurb *lastnu;
	extern short isNurbsel();
	struct Nurb *nu;
	short event;
	char str[140];
	
	if ELEM3(G.ebase->soort, 5, 11, -2) {
		if(lastnu==0) return;
		
		strcpy(str, "COPY  %t|Resolution %x1|Order  %x2");
		
		event= pupmenu(str);
		if(event>0) {
			nu= editNurb.first;
			while(nu) {
				if(nu!=lastnu && isNurbsel(nu) ) {
					if(event==1) {
						nu->resolu= lastnu->resolu;
						nu->resolv= lastnu->resolv;
					}
					else if(event==2) {
						nu->orderu= lastnu->orderu;
						nu->orderv= lastnu->orderv;
					}
				}
				nu= nu->next;
			}
			makeDispList(G.ebase);
		}
	}
}

void copymenu()
{
	struct ObData *ob,*obact;
	struct IkaData *ika;
	struct PerfSph *ps;
	struct Nurb *nu, *newnu, *duplicateNurb();
	struct ColBlck *col,*ocol;
	struct Base *base,*tbase, *dbase;
	struct Key *key, *keyn, *prevk;
	struct Bezier *bez;
	struct VV *vv;
	long *ai;
	long len;
	short event,colt,ocolt,min,type;
	char str[140];
	
	if(G.basact==0) return;
	if(G.ebase) {
		ebase_copymenu();
		return;
	}
	
	G.moving=1;
	tekenbase_ext(G.basact);
	
	strcpy(str, "COPY %t|Color%x1|Matrix%x2|Quat%x3|Origin%x4|Drawtype%x5|Len%x9|Startframe%x10|BaseKeys%x7");
	if(G.basact->soort==1) strcat(str, "|Verts+Faces%x6");
	else if(G.basact->soort==2) strcat(str, "|LampSettings%x8");
	else if(G.basact->soort==4) strcat(str, "|CameraNoise%x8");
	else if(G.basact->soort==5) strcat(str, "|Nurbs%x6");
	else if(G.basact->soort==9) strcat(str, "|Text%x6|FontSettings%x8");
	else if(G.basact->soort==11) strcat(str, "|Curves%x6|BevelSettings%x8");

	event= pupmenu(str);
	G.moving= 0;

	if(event== -1) {
		projektie();
		return;
	}
	else if(event==1) {
		if(G.basact->soort & 1) {
			if(G.basact->soort==3) {
				ps=(struct PerfSph *)G.basact->d;
				ocol= &ps->c;
				ocolt=1;
			} else {
				ob=(struct ObData *)G.basact->d;
				ocol=(struct ColBlck *)(ob+1);
				ocolt=ob->c;
			}
		} else {
			projektie();
			return;
		}
	}
	else if(event==6) {
		if(G.basact->soort & 1) {
			obact= (struct ObData *)G.basact->d;
			ocol=(struct ColBlck *)(obact+1);
		}
	}
	else if(event==5) {
		if ELEM5(G.basact->soort, 1, 5, 7, 9, 11) {
			ob= (struct ObData *)G.basact->d;
			type= ob->dt;
		}
		else if(G.basact->soort== -4) {
			ika= (struct IkaData *)G.basact->d;
			type= ika->dt;
		}
		else type=1;
	}

	tbase= G.basact;
	base=G.firstbase;
	while(base) {
		if(base!=G.basact) {
			if(TESTBASE(base)) {
				if(event==1) {
					if(base->soort & 1) {
						if(base->soort==3) {
							ps=(struct PerfSph *)base->d;
							col= &ps->c;
							colt=1;
						} else {
							ob=(struct ObData *)base->d;
							col=(struct ColBlck *)(ob+1);
							colt=ob->c;
						}
						min=ocolt;
						if(ocolt>colt) min=colt;
						memcpy(col,ocol,min*sizeof(struct ColBlck));
					}
				}
				else if(event==2) {  /* mat */
					G.basact= base; /* anders geeft getbasekeyadr geen key terug */
					ai= getbasekeyadr(base,3);
					memcpy(ai,tbase->m,36);
					G.basact= tbase;
					base->f&= ~8;
				}
				else if(event==3) {  /* quat */
					G.basact= base; /* anders geeft getbasekeyadr geen key terug */
					ai= getbasekeyadr(base,2);
					memcpy(ai,tbase->q,16);
					G.basact= tbase;
					base->f&= ~4;
				}
				else if(event==4) {  /* or */
					G.basact= base; /* anders geeft getbasekeyadr geen key terug */
					ai= getbasekeyadr(base,1);
					memcpy(ai,tbase->o,12);
					G.basact= tbase;
				}
				else if(event==5) {  /* drawt */
					if ELEM5(base->soort, 1, 5, 7, 9, 11) {
						ob= (struct ObData *)base->d;
						ob->dt= type;
						makeDispList(base);
					}
					else if(base->soort== -4) {
						ika= (struct IkaData *)G.basact->d;
						ika->dt= type;
					}
				}
				else if(event==9) {  /* len */
					base->len= G.basact->len;
				}
				else if(event==10) {  /* sf */
					base->sf= G.basact->sf;
				}
				else if(event==6) {  /* specific basedata */
					ob=(struct ObData *)base->d;
					if(G.basact->soort==1) {	/* verts & faces */
						if(base->soort==1) {
							ob->vv->us--;
							if(ob->vv->us<1) freeN(ob->vv);
							ob->vv= obact->vv;
							ob->vv->us++;
							if(ob->c<obact->c) {
								ai= (long *)mallocN(sizeof(struct ObData)+
									obact->c*sizeof(struct ColBlck),"copymenu");
								base->d= ai;
								freelistN(&ob->disp);
								memcpy(ai, ob, sizeof(struct ObData));
								freeN(ob);
								ob= (struct ObData *)ai;
								ob->c= obact->c;
								memcpy(ob+1,ocol,ob->c*sizeof(struct ColBlck));
							}
							
							/* displisten van alle users, ook deze base */
							vv= ob->vv;
							dbase= G.firstbase;
							while(dbase) {
								if(dbase!=G.basact && dbase->soort==1 && dbase->lay & view0.lay) {
									ob= (struct ObData *)dbase->d;
									if(ob->vv == vv) makeDispList(dbase);
								}
								dbase= dbase->next;
							}
						}
					}
					else if ELEM(G.basact->soort, 5, 11) {	/* Nurbs, curve */
						if(base->soort==G.basact->soort) {
							freeNurblist(&(ob->cu->curve));
							nu= obact->cu->curve.first;
							while(nu) {
								newnu= duplicateNurb(nu);
								addtail(&(ob->cu->curve), newnu);
								nu= nu->next;
							}
							ob->cu->ws= obact->cu->ws;
							
							if(obact->cu->key) {
								if(ob->cu->key) freekeys(ob->cu->key);
								key= obact->cu->key;
								prevk= 0;
								len= sizeof(struct Key) +4 +12*obact->cu->keyverts;
								while(key) {
									keyn= (struct Key *)mallocN(len, "copymenuKey");
									memcpy(keyn, key, len);
		
									if(prevk==0) ob->cu->key= keyn;
									else prevk->next= keyn;
									prevk= keyn;
		
									key= key->next;
								}
								ob->cu->keyverts= obact->cu->keyverts;
							}
							
							if(base->soort==11) {
								makeBevelList(base);
							}
							makeDispList(base);
							if(ob->c<obact->c) {
								ai= (long *)mallocN(sizeof(struct ObData)+
									obact->c*sizeof(struct ColBlck),"copymenu");
								base->d= ai;
								memcpy(ai,ob,sizeof(struct ObData));
								freeN(ob);
								ob= (struct ObData *)ai;
								ob->c= obact->c;
								memcpy(ob+1,ocol,ob->c*sizeof(struct ColBlck));
							}
						}
					}
					else if(G.basact->soort==9) {	/* text */
						if(base->soort==9) {
							len= strlen(obact->fo->str);
							if(len) {
								if(ob->fo->str) freeN(ob->fo->str);
								ob->fo->str= mallocN(len+2, "copymenu");
								strcpy(ob->fo->str, obact->fo->str);
								ob->fo->len= len;
								makepolytext(base);
								extrudepoly(base);
							}
						}
					}
				}
				else if(event==7) {	/* BaseKeys */
					if(base->key) freekeys(base->key);
					base->key= 0;
					if(G.basact->key) {
						key= G.basact->key;
						prevk= 0;
						len= sizeof(struct Key) +19*4;
						while(key) {
							keyn= (struct Key *)mallocN(len, "copymenuKey");
							memcpy(keyn, key, len);

							if(prevk==0) base->key= keyn;
							else prevk->next= keyn;
							prevk= keyn;

							key= key->next;
						}
					}
					if(base->ipokey) freeN(base->ipokey);
					base->ipokey= 0;
					if(G.basact->ipokey) {
						bez= G.basact->ipokey;
						len= sizeof(struct Bezier)+ 36*bez->cp;
						base->ipokey= (struct Bezier *)mallocN(len, "copymenuIpokey");
						memcpy(base->ipokey, bez, len);
					}
				}
				else if(event==8) {	/* settings */
					if(G.basact->soort==2) {
						if(base->soort==2) {	/* lamp */
							memcpy(base->d, G.basact->d, sizeof(struct LaData));
						}
					}
					else if(G.basact->soort==4) {	/* camera */
						base->noise= G.basact->noise;
						base->noisefreq= G.basact->noisefreq;
						base->nvecint= G.basact->nvecint;
						base->nrotint= G.basact->nrotint;
						if(base->iponoise) freeN(base->iponoise);
						if(G.basact->iponoise) {
							bez= G.basact->iponoise;
							len= sizeof(struct Bezier)+bez->cp*36;
							base->iponoise= (struct Bezier *)mallocN(len,"addupbaseii");
							memcpy(base->iponoise, G.basact->iponoise, len);
						}
					}
					else if(G.basact->soort==9) {	/* font */
						if(base->soort==9) {
							ob= (struct ObData *)base->d;
							obact= (struct ObData *)G.basact->d;
							freevfont(ob->fo->vf);
							obact->fo->vf->us++;
							ob->fo->vf= obact->fo->vf;
							memcpy(ob->fo->set, obact->fo->set, 12);
							memcpy( &(ob->po->f), &(obact->po->f), 10);
							ob->po->depth= obact->po->depth;
							makepolytext(base);
							extrudepoly(base);
						}
					}
					else if(G.basact->soort==11) {	/* curve */
						if(base->soort==11) {
							ob= (struct ObData *)base->d;
							obact= (struct ObData *)G.basact->d;
							ob->cu->bevbase= obact->cu->bevbase;
							memcpy( &(ob->cu->flag), &(obact->cu->flag), 14);
							makeBevelList(base);
							makeDispList(base);
						}
					}
				}
			}
		}
		base=base->next;
	}
	
	projektie();
}

void snapmenu()
{
	struct Base *base;
	struct ObData *ob;
	struct EditVert *eve;
	float gridf,tmat[4][4],imat[3][3],bmat[3][3],vec[3], min[3], max[3];
	short event;
	long count;

	event= pupmenu("SNAP %t|Sel -> Grid%x1|Sel -> Curs%x2|Curs-> Grid%x3|Curs-> Sel%x4");

	gridf= view0.grid;

	if(event== 1 || event==2) {  /* sel->grid  sel->curs  */

		if(G.ebase) {
			if  ELEM4(G.ebase->soort, 5, 11, -2, -4) bgnVedit();

			parentbase(G.ebase,tmat,bmat);
			Mat3CpyMat4(bmat,tmat);
			if(G.ebase->soort==1) {
				ob= (struct ObData *)G.ebase->d;
				Mat3MulFloat(bmat,ob->vv->ws);
			}
			else if  ELEM(G.ebase->soort, 5, 11) {
				ob= (struct ObData *)G.ebase->d;
				Mat3MulFloat(bmat,ob->cu->ws);
			}
			Mat3Inv(imat,bmat);

			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					if(event==2) {
						vec[0]= view0.muis[0]-tmat[3][0];
						vec[1]= view0.muis[1]-tmat[3][1];
						vec[2]= view0.muis[2]-tmat[3][2];
					} else {
						VECCOPY(vec,eve->co);
						Mat3MulVecfl(bmat,vec);
						VecAddf(vec,vec,tmat[3]);
						vec[0]= view0.grid*ffloor(.5+ vec[0]/gridf);
						vec[1]= view0.grid*ffloor(.5+ vec[1]/gridf);
						vec[2]= view0.grid*ffloor(.5+ vec[2]/gridf);
						VecSubf(vec,vec,tmat[3]);
					}
					Mat3MulVecfl(imat,vec);
					VECCOPY(eve->co,vec);
				}
				eve= eve->next;
			}
			if  ELEM4(G.ebase->soort, 5, 11, -2, -4) {
				copyVedit(); 
				endVedit(); 
			}

			projektie();
			return;
		}

		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				if(event==2) {
					vec[0]= -base->r[0]+view0.muis[0];
					vec[1]= -base->r[1]+view0.muis[1];
					vec[2]= -base->r[2]+view0.muis[2];
				} else {
					vec[0]= -base->r[0]+view0.grid*ffloor(.5+ base->r[0]/gridf);
					vec[1]= -base->r[1]+view0.grid*ffloor(.5+ base->r[1]/gridf);
					vec[2]= -base->r[2]+view0.grid*ffloor(.5+ base->r[2]/gridf);
				}
				if(base->p!=0 && G.hie) {
					parentbase(base,tmat,bmat);
					Mat3Inv(imat,bmat);
					Mat3MulVecfl(imat,vec);
					base->o[0]+= vec[0];
					base->o[1]+= vec[1];
					base->o[2]+= vec[2];
				}
				else {
					base->v[0]+= vec[0];
					base->v[1]+= vec[1];
					base->v[2]+= vec[2];
				}
			}
			base= base->next;
			if(G.ebase) base= 0;
		}
		projektie();
	}
	else if(event==3) {   /* curs to grid */
		view0.muis[0]= view0.grid*ffloor(.5+view0.muis[0]/gridf);
		view0.muis[1]= view0.grid*ffloor(.5+view0.muis[1]/gridf);
		view0.muis[2]= view0.grid*ffloor(.5+view0.muis[2]/gridf);
		berekenschermco(view0.muis,&view0.mx);
		tekenoverdraw(0);
	} else if(event==4) {   /* curs to sel */
		count= 0;
		min[0]= min[1]= min[2]= 1.0e20;
		max[0]= max[1]= max[2]= -1.0e20;
		if(G.ebase) {
			if  ELEM4(G.ebase->soort, 5, 11, -2, -4) bgnVedit();

			parentbase(G.ebase,tmat,bmat);
			Mat3CpyMat4(bmat,tmat);
			if(G.ebase->soort==1) {
				ob= (struct ObData *)G.ebase->d;
				Mat3MulFloat(bmat,ob->vv->ws);
			}
			else if  ELEM(G.ebase->soort, 5, 11) {
				ob= (struct ObData *)G.ebase->d;
				Mat3MulFloat(bmat,ob->cu->ws);
			}
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->f & 1) {
					VECCOPY(vec,eve->co);
					Mat3MulVecfl(bmat,vec);
					VecAddf(vec,vec,tmat[3]);
					MinMax3(min, max, vec);
					count++;
				}
				eve= eve->next;
			}
			if(count) {
				view0.muis[0]= (min[0]+max[0])/2;
				view0.muis[1]= (min[1]+max[1])/2;
				view0.muis[2]= (min[2]+max[2])/2;
			}
			if  ELEM4(G.ebase->soort, 5, 11, -2, -4) endVedit();

		} else {
			base= G.firstbase;
			while(base) {
				if(TESTBASE(base)) {
					VECCOPY(vec, base->r);
					MinMax3(min, max, vec);
					count++;
				}
				base= base->next;
			}
			if(count) {
				view0.muis[0]= (min[0]+max[0])/2;
				view0.muis[1]= (min[1]+max[1])/2;
				view0.muis[2]= (min[2]+max[2])/2;
			}
		}
		berekenschermco(view0.muis,&view0.mx);
		tekenoverdraw(0);
	}
}

void enter_ebasemode(base)
struct Base *base;
{
	struct IkaData *ika;
	struct MoData *mo;
	struct ObData *ob;

	if(base==0) return;
	if((base->lay & view0.lay)==0) return;

	if(base->soort== -4) {
		ika= (struct IkaData *)base->d;
		countkeys();
		if(ika->key) {
			if( (G.ipomode!=22) || (G.actkey==0) ) {
				error("Must have active key");
				return;
			}
		}
		G.ebase= base;
		make_ebaseIka();
	}
	else if(base->soort== -2) {
		mo= (struct MoData *)base->d;
		countkeys();
		if(mo->key) {
			if( (G.ipomode!=22) || (G.actkey==0) ) {
				error("Must have active key");
				return;
			}
		}
		G.ebase= base;
		make_ebaseNurb();
	}
	else if(base->soort==1) {
		ob= (struct ObData *)base->d;
		countkeys();
		if(ob->vv->key) {
			if( (G.ipomode!=22) || (G.actkey==0) ) {
				error("Must have active key");
				return;
			}
		}
		G.ebase= base;
		make_ebasedata();
	}
	else if(base->soort==9) {
		G.ebase= base;
		textedit();
	}
	else if(base->soort==5 || base->soort==11) {
		ob= (struct ObData *)base->d;
		countkeys();
		if(ob->cu->key) {
			if( (G.ipomode!=22) || (G.actkey==0) ) {
				error("Must have active key");
				return;
			}
		}
		G.ebase= base;
		make_ebaseNurb();
	}

	if(G.ebase) G.ebase->f&= ~1;	/* wel testen vanwege soort==9 */
	countall();
	projektie();
}

void exit_ebasemode()
{
	struct Base *base, *tbase;
	struct ObData *ob;
	struct MoData *mo;
	struct IkaData *ika;
	struct VV *vv;

	if(G.ebase==0) return;

	if(G.ebase->soort==1) {
		ob= (struct ObData *)G.ebase->d;
		vv= ob->vv;
		if(vv->key) {
			if(G.totvert!= vv->vert) {
				if(okee("Different # vertices: copy to all keys")==0) return;
			}
		}
		load_ebasedata();
		if(G.edve.first) freelist(&G.edve.first);
		if(G.eded.first) freelist(&G.eded.first);
		if(G.edvl.first) freelist(&G.edvl.first);
		free_hashedgetab();
	}
	else if ELEM(G.ebase->soort, 5, 11) {
		ob= (struct ObData *)G.ebase->d;
		if(G.ipomode==22 && ob->cu->key && G.actkey) {
			if(G.totvert!= ob->cu->keyverts) {
				if(okee("Different # vertices: copy to all keys")==0) return;
			}
		}
		load_ebaseNurb();
		freeNurblist(&editNurb);
	}
	else if(G.ebase->soort== -2) {
		mo= (struct MoData *)G.ebase->d;
		if(G.ipomode==22 && mo->key && G.actkey) {
			if(G.totvert!= mo->keyverts) {
				if(okee("Different # vertices: copy to all keys")==0) return;
			}
		}
		load_ebaseNurb();
		freeNurblist(&editNurb);
	}
	else if(G.ebase->soort== -4) {
		ika= (struct IkaData *)G.ebase->d;
		if(G.ipomode==22 && ika->key && G.actkey) {
			if(G.totvert!= ika->keyverts) {
				if(okee("Different # vertices: copy to all keys")==0) return;
			}
		}
		load_ebaseIka();
	}

	G.ebase->f|=1; /* weer select maken */
	base= G.ebase;
	G.ebase=0;
	makeDispList(base);	/* ebase moet 0 zijn voor curve-extrude */
	countall();
	loadkeypos(base, base);	/* voor vv key */
	
	/* heeft dit nog invloed op andere bases? */
	if(base->soort==11) {
		/* test of base als bevelcurve wordt gebruikt */
		tbase= G.firstbase;
		while(tbase) {
			if(tbase->soort==11) {
				ob= (struct ObData *)tbase->d;
				if(ob->cu->bevbase==base) {
					makeDispList(tbase);
				}
			}
			tbase= tbase->next;
		}
	}
	
	projektie();
}

ulong samplerect(buf, size, dontdo)
ulong *buf;
long size;
ulong dontdo;
{
	ulong *bufmin,*bufmax;
	long a,b,rc,tel,aantal,dirvec[4][2],maxob;
	ulong retval=0;
	
	maxob= G.totobj+G.totlamp+G.totmove+G.totpoly;
	maxob= ((maxob & 0xF00)<<12) + ((maxob & 0xF0)<<8) + ((maxob & 0xF)<<4);

	aantal= (size-1)/2;
	rc= 0;

	dirvec[0][0]= 1;
	dirvec[0][1]= 0;
	dirvec[1][0]= 0;
	dirvec[1][1]= -size;
	dirvec[2][0]= -1;
	dirvec[2][1]= 0;
	dirvec[3][0]= 0;
	dirvec[3][1]= size;

	bufmin= buf;
	bufmax= buf+ size*size;
	buf+= aantal*size+ aantal;

	for(tel=1;tel<=size;tel++) {

		for(a=0;a<2;a++) {
			for(b=0;b<tel;b++) {

				if(*buf && *buf<=maxob && *buf!=dontdo) return *buf;
				if( *buf==dontdo ) retval= dontdo;	/* als alleen kleur dontdo aanwezig is, wel dontdo teruggeven */
				
				buf+= (dirvec[rc][0]+dirvec[rc][1]);

				if(buf<bufmin || buf>=bufmax) return retval;
			}
			rc++;
			rc &= 3;
		}
	}
	return retval;
}

#define SELECTSIZE	51

void muisselect()
{
	void grabber();
	struct MoData *mo;
	struct Base *base, *b=0, *startbase=0,*oldbase;
	float *p,mat[4][4],omat[3][3];
	ulong rect[SELECTSIZE*SELECTSIZE],*dr,kleur=0;
	Device mdev[2];
	short hits, buffer[2000], dist=100, mval[2], temp,a,sco[2],x,y;

	winset(G.winar[0]);

	if(G.ebase) {
		if(G.ebase->soort==1) muis_ebase();
		else if ELEM3(G.ebase->soort, 5, 11, -2) muis_Nurb();
		else if(G.ebase->soort== -4) muis_ika();
		
		return;
	}

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	getdev(2, mdev, mval);
	mval[1]-= S.vys;

	if( (G.qual & 32) ) {
		/* iedere keer baselijst starten vanuit basact */
		if(G.basact) startbase=G.basact->next;
		if(startbase==0) startbase=G.firstbase;
		base=startbase;
		while(base) {
			if(base->lay & view0.lay) {
				temp=abs(base->sx -mval[0]) + abs(base->sy -mval[1]);
				if(base==G.basact) temp+=10;
				if(temp<dist ) {
					b=base; 
					dist=temp;
				}
			}
			base= base->next;
			if(base==0) base=G.firstbase;
			if(base==startbase) break;
		}

	}
	else if(G.machine==ENTRY) {
		hits= selectprojektie(buffer, 0, 0, 0, 0);
		
		if(hits>0) {
			if(G.basact) startbase=G.basact->next;
			if(startbase==0) startbase=G.firstbase;
			base=startbase;
			while(base) {
				if(base->lay & view0.lay) {
					for(a=0; a<hits; a++) {
						if(base->selcol==buffer[2*a+1]) b= base;
					}
				}
				if(b) break;
				base= base->next;
				if(base==0) base=G.firstbase;
				if(base==startbase) break;
			}
		}
	} else {

		/* met rectread testen */
		if(!backbufok) {
			backbufprojektie(0);
		}
		countall();
		a= SELECTSIZE*SELECTSIZE;
		dr= rect;
		while(a--) *(dr++)= 0;
		a= (SELECTSIZE-1)/2;
		lrectread(mval[0]-a,mval[1]-a,mval[0]+a,mval[1]+a,rect);

		dr= rect;

		for(y= mval[1]-a; y<=mval[1]+a; y++) {
			for(x=mval[0]-a; x<=mval[0]+a; x++) {
				if(x<0 || y<0) *dr= 0;
				else if(x>=S.x || y>=S.vyw) *dr= 0;
				else *dr&= 0xF0F0F0;
				dr++;
			}
		}
		
		if(G.basact && TESTBASE(G.basact) ) {	/* niet 2 x de zelfde */
			kleur= samplerect(rect, SELECTSIZE, G.basact->selcol);
		} else {
			kleur= samplerect(rect, SELECTSIZE, 0);
		}

		if(kleur==0) return;
		else {
			b= G.firstbase;
			while(b) {
				if(b->lay & view0.lay) {
					if(b->selcol==kleur) break;
				}
				b= b->next;
			}
		}
	}

	if(b) {
		oldbase= G.basact;   /* even onthouden, oa voor mainb==9 */
		G.basact= b;
		if((G.qual & 3)==0) {
			deselectall_ex(b);
			b->f |= 1;
		}
		else {
			if(b->f & 1) b->f--;
			else b->f++;
		}
		
		if(G.mainb==7) if(oldbase) oldbase->lastipo= G.ipomode;
		
		if(oldbase!=G.basact) {
			tekenbase_ext(b);
			if(G.mainb==4) if(b->soort==2) {
				tekenlampbuts(1);
			}
			if(G.mainb==5) if(b->soort & 1) {
				tekenobjbuts(1);
			}
			if(G.mainb==7) {
				a= 1;
				if(oldbase) {
					if(oldbase->soort== -2 && G.basact->soort!= -2) a= 0;
					if(oldbase->soort!= -2 && G.basact->soort== -2) a= 0;
				}
				G.ipomode= G.basact->lastipo;
				
				if(G.basact->soort!= -2 && G.ipomode<20) G.ipomode= 20;	/* patch */
				
				tekenmovebuts(a);
			}
			if(G.mainb==9) {
				a= 0; /* wel complete redraw */
				if(oldbase) if(oldbase->soort==b->soort) a=1;
				tekeneditbuts(a);
			}
		}
		else {
			tekenbase_ext(b);
		}
	}

	countall();
	tekenoverdraw(0);
	getmouseco(mval);
	x= mval[0]; 
	y= mval[1];
	while(getbutton(RIGHTMOUSE)) {
		gsync();
		getmouseco(mval);
		if(abs(mval[0]-x)+abs(mval[1]-y) > 14) {
			grabber();
			while(getbutton(RIGHTMOUSE))  gsync();
		}
	}
}

void minmaxbase(base, min, max)
struct Base *base;
float *min, *max;
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	struct CurveData *cu;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct MoData *mo;
	struct IkaData *ika;
	struct BodyPoint *bop;
	struct PolyData *po;
	float *fp, mat[4][4], omat[3][3], vec[3];
	long a, b, tot;
	short *p;
	
	if(base->soort & 1) {
		ob= (struct ObData *)base->d;
	}
	
	VECCOPY(vec, base->r);
	MinMax3(min, max, vec);

	parentbase(base, mat, omat);
	
	if(base->soort==1) {
		vv= ob->vv;
		Mat4MulFloat3(mat, vv->ws);
		tot= vv->vert;
		adrve= (struct VertOb *)(vv+1);
		while(tot--) {
			VECCOPY(vec, adrve->c);
			Mat4MulVecfl(mat, vec);
			MinMax3(min, max, vec);
			adrve++;
		}
	}
	else if ELEM3(base->soort, 5, 11, -2) {
		if(base->soort== -2) {
			mo= (struct MoData *)base->d;
			nu= mo->curve.first;
		}
		else {
			cu= ob->cu;
			Mat4MulFloat3(mat, cu->ws);
			nu= cu->curve.first;
		}
		while(nu) {
			if((nu->type & 7)==1) {
				bezt= nu->bezt;
				tot= nu->pntsu;
				while(tot--) {
					VECCOPY(vec, bezt->vec[0]);
					Mat4MulVecfl(mat, vec);
					MinMax3(min, max, vec);
					VECCOPY(vec, bezt->vec[1]);
					Mat4MulVecfl(mat, vec);
					MinMax3(min, max, vec);
					VECCOPY(vec, bezt->vec[2]);
					Mat4MulVecfl(mat, vec);
					MinMax3(min, max, vec);
					bezt++;
				}
			}
			else {
				bp= nu->bp;
				tot= nu->pntsu*nu->pntsv;
				while(tot--) {
					VECCOPY(vec, bp->vec);
					Mat4MulVecfl(mat, vec);
					MinMax3(min, max, vec);
					bp++;
				}
			}
			nu= nu->next;
		}
	}
	else if(base->soort==9) {
		po= ob->po;
		Mat4MulFloat3(mat, po->ws);
		
		p=(short *)(po+1);
		for(a=0;a<po->poly;a++) {
			tot= *p;
			p++;
			for(b=0; b<tot; b++) {
				vec[0]= p[0];
				vec[1]= p[1];
				vec[2]= 0.0;
				Mat4MulVecfl(mat, vec);
				MinMax3(min, max, vec);
				p+=3;
			}
		}
	}
	else if(base->soort== -4) {
		ika= (struct IkaData *)(base->d);
		bop= ika->bpbase.first;
		while(bop) {
			VECCOPY(vec, bop->r);
			Mat4MulVecfl(mat, vec);
			MinMax3(min, max, vec);
			bop= bop->next;
		}
	}
}

void maakbol()
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	float x1,y1,r,r1,fac;
	long a,tot;

	if(G.basact==0 || okee("Maak bol")==0) return;
	if(G.basact->soort!=1) return;

	ob=(struct ObData *)G.basact->d;
	vv= ob->vv;
	tot= vv->vert;
	adrve= (struct VertOb *)(vv+1);
	fac= 1.0/32767.0;
	for(a=0;a<tot;a++) {
		x1= fac*adrve->c[0];
		y1= fac*adrve->c[2];
		r= x1*x1+y1*y1;
		r1= fsqrt( fabs(1-r));
		if(r<1.0) adrve->c[1]= -32767*r1;
		else adrve->c[1]=0;
		adrve++;
	}


}

void wordruimtekleiner()
{
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	float x1,y1,r,r1,fac;
	long a,tot;

	if(G.basact==0 || okee("wordruimte .05 kleiner")==0) return;
	if(G.basact->soort!=1) return;

	ob=(struct ObData *)G.basact->d;
	vv= ob->vv;
	tot= vv->vert;
	adrve= (struct VertOb *)(vv+1);
	fac= .95;
	vv->ws= vv->ws/fac;
	for(a=0;a<tot;a++) {
		adrve->c[0]*=fac;
		adrve->c[1]*=fac;
		adrve->c[2]*=fac;
		adrve++;
	}
}

void makeparent()
{
	struct Base *base, *b;
	struct Key *key;
	float imat[3][3], mat[4][4], oldmat[4][4], omat[3][3];
	long vec[3], *l0;
	short a=0, vert[2], vert1[2], transf= 1, rodnr= 0, button(), mode, skelnr= 0;

	if(G.ebase) return;
	if(G.basact==0) return;

	G.moving=1;
	tekenbase_ext(G.basact);
	G.moving=0;

	vert1[0]=G.basact->sx;
	vert1[1]=G.basact->sy;

	base=G.firstbase;
	setlinestyle(1); 
	cpack(0);
	frontbuffer(1);
	backbuffer(0);
	while(base) {
		if(base->lay & view0.lay) {
			if(base->f & 1) {
				if(base!=G.basact) {
					a=1;
					bgnline();
					v3i(G.basact->r);
					v3i(base->r);
					endline();
				}
			}
		}
		base=base->next;
	}
	setlinestyle(0);
	frontbuffer(0);
	backbuffer(1);
	if(a==0) {
		tekenbase_ext(G.basact);
		return;
	}
	
	if(G.basact->soort== -4) {
		
		if(G.qual & 3) {
			if((mode= pupmenu("Make Parent without transform %t|Use Rod %x1|Use Skeleton %x2"))== -1) return;
			transf= 0;
		}
		else if((mode= pupmenu("Make Parent %t|Use Rod %x1|Use Skeleton %x2"))== -1) return;

		a= 1;
		if(mode==1) tekenrodnummers(G.basact);
		else tekenskelnummers(G.basact);
	}
	else {
		if(G.qual & 3) {
			a= okee("Make Parent without transform");
			transf= 0;
		}
		else a= okee("Make Parent");
	}
	
	if(a) {
		if(G.basact->soort== -4) {
			if(mode==1) {
				if(button(&rodnr, 0, 99, "Rod: ")==0) a= 0;
			}
			else {
				if(button(&skelnr, 0, 99, "Skeleton: ")==0) a= 0;
			}
		}
	}
	if(a) {
		base=G.firstbase;
		while(base) {
			if(base->lay & view0.lay) {
				if(base->f & 1) {
					if(base!=G.basact) {
						parentbase(base, oldmat, omat);
						
						base->p= G.basact;
						
						if(G.basact->soort== -4) {
							base->rodnr= rodnr;
							base->skelnr= skelnr;
						}
						else {
							base->rodnr= G.basact->rodnr;	/* ivm *childbase in parentbase() */
							base->skelnr= 0;
						}
						
						b=base;
						while(b) {
							b=b->p;
							if(b==base) break;
						}
						if(b==base) {
							base->p=0;
							error("Loop in parents");
							base->skelnr= base->rodnr= 0;
						}
						else {
							if(transf) {
								base->o[0]=base->o[1]=base->o[2]=0;
								base->q[1]=base->q[2]=base->q[3]= 0.0;
								base->q[0]= 1.0;
								Mat3One(base->m);
								
								parentbase(base, mat, omat);
								Mat3Inv(imat, omat);
								vec[0]= base->r[0]-base->v[0];
								vec[1]= base->r[1]-base->v[1];
								vec[2]= base->r[2]-base->v[2];
								Mat3MulVec(imat, vec);
								base->o[0]= -vec[0]; base->o[1]= -vec[1]; base->o[2]= -vec[2];
								
								/* nieuwe matrix: oudemat x inverse nieuwe */
								Mat3CpyMat4(omat, mat);

								Mat3Inv(imat, omat);
								Mat3CpyMat4(omat, oldmat);
								Mat3MulMat3(base->m, imat, omat);
								base->f &= ~8;

							}
							if(base->skelnr) calc_deform(base);
							
							if(base->key!=0 && okee("Change origin in keys")) {
							
								key= base->key;
								while(key) {
									l0=(long *)(key+1);
									l0[3]= l0[4]= l0[5]= 0;
									key= key->next;
								}
								loadkeypos(base, base);	/* staat verkeerd */
								parentbase(base, mat, omat);
								Mat3Inv(imat, omat);
								
								key= base->key;
								while(key) {
									l0=(long *)(key+1);

									vec[0]= base->r[0]-l0[0];
									vec[1]= base->r[1]-l0[1];
									vec[2]= base->r[2]-l0[2];
									Mat3MulVec(imat, vec);
									l0[3]= -vec[0]; l0[4]= -vec[1]; l0[5]= -vec[2];

									key=key->next;
								}
								loadkeypos(base, base);	/* staat-ie weer goed */
							}
						}
					}
				}
			}
			base=base->next;
		}
	}
	projektie();
}

void maketrack()
{
	struct Base *base, *b;
	short a=0,vert[2],vert1[2];

	if(G.basact==0) return;
	if(G.basact->sx==3200) return;

	G.moving=1;
	tekenbase_ext(G.basact);
	G.moving=0;

	if(okee("Track to")) {
		base=G.firstbase;
		while(base) {
			if(base->lay & view0.lay) {
				if(base->f & 1) {
					if(base!=G.basact) {
						base->q[0]= 1.0;
						base->q[1]= base->q[2]= base->q[3]= 0.0;
						base->t= G.basact;
					}
				}
			}
			base=base->next;
		}
	}
	projektie();
}

void join()
{
	struct Base *base, *first, *next, *lbase, *par;
	struct ObData *ob, *obn;
	struct VV *vv, *vvn;
	struct ColBlck *col, *testcol[16];
	struct VlakOb *adrvl, *adrvl1;
	struct VertOb *adrve, *adrve1;
	float fac, min[3], max[3], *vertdata, *vd, bmat[4][4], omat[3][3], cent[3];
	long len, ok, a, b, vertofs, soort= 0, totvlak, totvert, totcol, setnewcol;
	char newcol[16];

	if(G.ebase) return;
	if(okee("Join selected bases")==0) return;

	ok= 1;
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort>0 && (base->soort & 1)) {
				if(soort==0) soort= base->soort;
				else if(soort!=base->soort) {
					soort= 0;
					ok= 0;
					break;
				}
			}
		}
		base= base->next;
	}

	if(soort==0) {
		if(ok==0) error("Can't join mixed base types");
		return;
	}

	if(soort==1) {
		totvlak= totvert= totcol= 0;
		first= 0;
		ok= 0;
		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				if(base->soort==1) {
					if(first==0) {
						first= base;
						par= base->p;
					}
					else {
						ok= 1;
						if(base->p != par) par= 0;
					}
					ob= (struct ObData *)base->d;
					
					if(ob->vv->key) {
						ok= 0;
						error("Can't join with vertexkeys");
						break;
					}
					
					totvert+= ob->vv->vert;
					totvlak+= ob->vv->vlak;
					if(totcol) {
						setnewcol= 0;
						col= (struct ColBlck *)(ob+1);

						for(a=0; a<ob->c; a++) {
							/* mapping maken */
							newcol[a]= 16;
							for(b=0; b<totcol; b++) {
								if( memcmp(col, testcol[b], sizeof(struct ColBlck))==0 ) {
									if(a!=b) setnewcol= 1;
									newcol[a]= b;
									break;
								}
							}
							if(newcol[a]==16) {
								if(totcol<15) {
									testcol[totcol]= col;
									newcol[a]= totcol;
									totcol++;
									setnewcol= 1;
								}
								else newcol[a]= 15;
							}
							col++;
						}
						if(setnewcol) {
							if(ob->vv->us>1) {
								ob->vv->us--;
								len= sizeof(struct VV)+ob->vv->vert*sizeof(struct VertOb)+
								    ob->vv->vlak*sizeof(struct VlakOb);
								vvn= mallocN(len, "join4");
								memcpy(vvn, ob->vv, len);
								vvn->us= 1;
								ob->vv= vvn;
							}

							adrve= (struct VertOb *)(ob->vv+1);
							adrvl= (struct VlakOb *)(adrve+ob->vv->vert);
							for(a=0; a<ob->vv->vlak; a++) {
								b= (adrvl->ec & ~15);
								adrvl->ec= b+ newcol[adrvl->ec & 15];
								adrvl++;
							}
						}
					}
					else {
						col= (struct ColBlck *)(ob+1);
						for(a=0; a<ob->c; a++) {
							testcol[a]= col;
							col++;
						}
						totcol= ob->c;
					}
				}
			}
			base= base->next;
		}

		if(ok==0) return;   /* maar 1 base */

		ob= (struct ObData *)first->d;
		if(totcol!= ob->c) {
			obn= mallocN(sizeof(struct ObData)+totcol*sizeof(struct ColBlck), "join3");
			memcpy(obn, ob, sizeof(struct ObData));
			obn->c= totcol;
			col= (struct ColBlck *)(obn+1);
			for(a=0; a<totcol; a++) {
				memcpy(col, testcol[a], sizeof(struct ColBlck));
				col++;
			}
			freeN(ob);
			first->d= (long *)obn;
			ob= obn;
		}
		
		freelistN(&ob->disp);
		
		vvn= mallocN(sizeof(struct VV)+totvert*sizeof(struct VertOb)+totvlak*sizeof(struct VlakOb), "join");

		memcpy(vvn, ob->vv, sizeof(struct VV));
		vvn->vert= totvert;
		vvn->vlak= totvlak;
		vvn->us= 1;

		min[0]= min[1]= min[2]= 1.0e10;
		max[0]= max[1]= max[2]= -1.0e10;
		vd=vertdata= mallocN(totvert*12, "join2");

		vertofs= 0;
		adrve= (struct VertOb *)(vvn+1);
		adrvl= (struct VlakOb *)(adrve+vvn->vert);
		base= G.firstbase;
		lbase= 0;
		while(base) {
			next= base->next;
			if(TESTBASE(base)) {
				parentbase(base, bmat, omat);
				ob= (struct ObData *)base->d;
				vv= ob->vv;
				Mat4MulFloat3(bmat,vv->ws);
				adrve1= (struct VertOb *)(vv+1);
				for(a=0; a<vv->vert; a++) {
					VECCOPY(vd, adrve1->c);
					Mat4MulVecfl(bmat, vd);
					for(b=0; b<3; b++) {
						min[b]= MIN2(min[b], vd[b]);
						max[b]= MAX2(max[b], vd[b]);
					}
					adrve1++;
					vd+=3;
				}

				adrvl1= (struct VlakOb *)(adrve1);
				for(a=0; a<vv->vlak; a++) {
					adrvl->v1= adrvl1->v1+ vertofs;
					adrvl->v2= adrvl1->v2+ vertofs;
					adrvl->v3= adrvl1->v3+ vertofs;
					memcpy(adrvl->n, adrvl1->n, 10);
					adrvl++;
					adrvl1++;
				}

				vertofs+= vv->vert;

				if(base!=first) delbase(base, lbase);
				else lbase= base;
			}
			else lbase= base;
			base= next;
		}
		ob= (struct ObData *)first->d;
		if(ob->vv->us==1) {
			if(ob->vv->key) freekeys(ob->vv->key);
			freeN(ob->vv);
		}
		else ob->vv->us--;

		ob->vv= vvn;
		cent[0]= (max[0]+ min[0])/2;
		cent[1]= (max[1]+ min[1])/2;
		cent[2]= (max[2]+ min[2])/2;
		max[0]= (max[0]-min[0])/2;
		max[1]= (max[1]-min[1])/2;
		max[2]= (max[2]-min[2])/2;
		fac= 1.0+MAX3(max[0], max[1], max[2]);
		fac= 32767.0/fac;
		vvn->ws= 1.0/fac;

		Mat3One(first->m);
		first->q[0]= 1.0;
		first->q[1]= first->q[2]= first->q[3]= 0.0;
		first->o[0]= first->o[1]= first->o[2]= 0;
		first->f |= 12;
		VECCOPY(first->v, cent);
		first->p= par;
		first->t= 0;
		if(first->key) freekeys(first->key);
		if(first->pkey) freekeys(first->pkey);
		first->key= first->pkey= 0;

		adrve= (struct VertOb *)(vvn+1);
		vd= vertdata;
		for(a=0; a<totvert; a++) {
			adrve->c[0]= fac*(vd[0]-cent[0]);
			adrve->c[1]= fac*(vd[1]-cent[1]);
			adrve->c[2]= fac*(vd[2]-cent[2]);
			adrve++; 
			vd+=3;
		}
		freeN(vertdata);
		G.basact= first;
		makeDispList(G.basact);
		
		/* adrve= (struct VertOb *)(vvn+1); */
		/* adrvl= (struct VlakOb *)(adrve+vvn->vert); */
	}
	else if (soort == 11){
		struct CurveData * cu1, * cu2;
		struct Link * link;
		
		first = 0;
		lbase = 0;
		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				if(base->soort==11) {
					if (first == 0) {
						G.basact= first = base;
						/* parentbase(base, bmat, omat); */
						ob = (struct ObData *)base->d;
						
						cu1 = ob->cu;
					} else {
						ob = (struct ObData *)base->d;
						cu2 = ob->cu;
						while(cu2->curve.first) {
							link = cu2->curve.first;
							remlink(&cu2->curve, link);
							addtail(&cu1->curve, link);
						}
						base = lbase;
						delbase(base->next, lbase);
					}
				}
			}
			lbase = base;
			base= base->next;
		}
		makeBevelList(G.basact);
		makeDispList(G.basact);
	}

	tekenmainbuts(2);
	projektie();

}

void movetolayer()
{
	struct Base *base;
	short button(),lay, shade= 0;

	lay=G.layact;
	if(button(&lay,0,12,"MoveToLayer: ")) {
		if(lay==0) lay= -1;
		else lay= 1<<(lay-1);
		base=G.firstbase;
		while(base) {
			if(base->f & 1) {
				if(base->lay & view0.lay) {
					base->lay =lay;
					if(base->soort==2) shade= 1;
				}
			}
			base=base->next;
		}
		countall();
		testDispLists();
		if(shade) reshadeall();
		projektie();
	}
}

#define MAXLASSO 1024

int testlasso_pixel(xs, ys)
int xs, ys;
{
	ulong pix;
	
	if(xs>= 0 && xs<S.vxw) {
		if(ys>0 && ys<S.vyw) {
			lrectread(xs, ys, xs, ys, &pix);
			if(pix) return 1;
		}
	}
	return 0;
}

void selectlasso()
{
	struct Base *base;
	struct EditVert *eve;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct BodyPoint *bop;
	long i= 0, a, incr, sel, ofsx, ofsy, mousex, mousey, ok= 1, start=0;
	Device mdev[2];
	short event, mval[2], mx, my, val;
	short mcords[MAXLASSO][2];

	
	winset(G.winar[0]);
	if(G.zbuf) zbuffer(0);
	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	qdevice(MOUSEY);
	qdevice(MOUSEX);

	getorigin(&ofsx, &ofsy);
	getdev(2, mdev, mval);

	drawmode(OVERDRAW);
	color(0); clear();
	color(3);
	bgnline();

	cmov2i(10,10);
	charstr("LASSO SELECT");

	while(ok) {
		switch (event=traces_qread(&val)){
		case MOUSEY:
			mousey = val - ofsy;
			if(start) {
				if ( abs(mousex-mcords[i-1][0])>5 || abs(mousey-mcords[i-1][1])>5 ) {
					
					mcords[i][0] = mousex;
					mcords[i][1] = mousey;
					v2s(mcords[i]);
					i++;
				}
			}
			break;
		case MOUSEX:
			mousex = val - ofsx;
			break;
		case LEFTMOUSE:
			if(val==0) ok= 0;
			else start= 1;
			break;
		case RIGHTMOUSE:
			if(val==0) ok= 0;
			else start= 1;
			break;
		case ESCKEY:
			ok= 0;
			break;
		}
		if (i == MAXLASSO) {
			if(getbutton(RIGHTMOUSE)) event= RIGHTMOUSE;
			else event= LEFTMOUSE;
			break;
		}
	}
	
	endline();
	
	color(0); clear();
	
	drawmode(NORMALDRAW);
	
	if(i>2 && ok==0 && (event==LEFTMOUSE || event==RIGHTMOUSE)) {
	
		cpack(0); clear(); cpack(0xF0F0F0);
		backbufok= 0;
		
		/* poly tekenen in backbuf, incr zetten */
		if(G.zbuf) zbuffer(0);
		incr= 1;
		if(i>256) incr= 2;
		if(i>512) incr= 3;
		if(i>768) incr= 4;

		concave(TRUE);
		bgnpolygon();
		for(val=0; val<i; val+=incr) {
			v2s(mcords[val]);
		}
		endpolygon();
		concave(FALSE);

		if(G.zbuf) zbuffer(1);

		persp(1);

		if(event==LEFTMOUSE) sel= 1;
		else sel= 0;
		
		if(G.ebase) {
			if(G.edve.first) {
				eve= (struct EditVert *)G.edve.first;
				while(eve) {
					if(eve->h==0) {
						if( testlasso_pixel(eve->xs, eve->ys)) {
							if(sel) eve->f|= 1;
							else eve->f&= 254;
						}
					}
					eve= eve->next;
				}
				tekenvertices_ext();
				
				countall();
				tekenoverdraw(0);
			}
			else if ELEM3(G.ebase->soort, 5, 11, -2) {
				nu= editNurb.first;
				while(nu) {
					if((nu->type & 7)==1) {
						bezt= nu->bezt;
						a= nu->pntsu;
						while(a--) {
							if(bezt->hide==0) {
								if( testlasso_pixel(bezt->s[0][0], bezt->s[0][1])) {
									if(sel) bezt->f1|= 1;
									else bezt->f1 &= ~1;
								}
								if( testlasso_pixel(bezt->s[1][0], bezt->s[1][1])) {
									if(sel) bezt->f2|= 1;
									else bezt->f2 &= ~1;
								}
								if( testlasso_pixel(bezt->s[2][0], bezt->s[2][1])) {
									if(sel) bezt->f3|= 1;
									else bezt->f3 &= ~1;
								}
							}
							bezt++;
						}
					}
					else {
						bp= nu->bp;
						a= nu->pntsu*nu->pntsv;
						while(a--) {
							if(bp->hide==0) {
								if( testlasso_pixel(bp->s[0], bp->s[1])) {
									if(sel) bp->f1|= 1;
									else bp->f1 &= ~1;
								}
							}
							bp++;
						}
					}
					nu= nu->next;
				}
				countall();
				projektie();	/* geen tekenvertices: edges ook */
			}
			else if(G.ebase->soort== -4) {
				bop= bpbase.first;
				while(bop) {
					if( testlasso_pixel(bop->sx, bop->sy) ) {
						if(sel) bop->f |= 1;
						else bop->f &= ~1;
					}
					bop= bop->next;
				}
				projektie();
			}
		}
		else {	/* bases alleen met centrum */
			base= G.firstbase;
			
			while(base) {
				if(base->lay & view0.lay) {
					if( testlasso_pixel(base->sx, base->sy)) {
						if(sel) base->f|= 1;
						else base->f&= 254;
					}
				}
				base= base->next;
			}
			countall();
			projektie();
		}
	}
	else persp(1);

	unqdevice(MOUSEY);
	unqdevice(MOUSEX);

}

void selector(mode)
short mode;
{
	extern float obsviewXfac,obsviewYfac;
	struct Base *base;
	struct EditVert *eve;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct BodyPoint *bop;
	float fac, xx2, yy2;
	long a, x2, y2;
	Device mdev[2];
	short event, mval[2],xo,yo,x1,y1,xs,ys,sel=0,dum;
	char string[100];

	if(mode && view0.persp!=2) return;

	winset(G.winar[0]);
	mdev[0]=MOUSEX; 
	mdev[1]=MOUSEY;
	persp(0);
	drawmode(OVERDRAW);
	color(0); 
	clear();
	color(3-mode);

	getdev(2,mdev,mval);
	xo=mval[0]; 
	yo=mval[1]-S.vys;

	sboxs(0, yo, S.x, yo);
	sboxs(xo, 0, xo, S.y);
	while( TRUE ) {
		getdev(2,mdev,mval);
		mval[1]-= S.vys;
		if(mval[0]!=xo || mval[1]!=yo) {
			color(0);
			sboxs(0,yo,S.x,yo);
			sboxs(xo,0,xo,S.y);
			xo= mval[0]; 
			yo= mval[1];
			color(3-mode);
			sboxs(0,yo,S.x,yo);
			sboxs(xo,0,xo,S.y);
		}
		if (qtest()) {
			event= traces_qread(&dum);
			if(event==LEFTMOUSE || event==RIGHTMOUSE) {
				if(dum) break;
			}
			else if (event == ESCKEY) {
				color(0); 
				clear();
				drawmode(NORMALDRAW);
				persp(1);
				return;
			}
			else if(mode==0 && event==BKEY && dum==1) {
				selectlasso();
				return;
			}
		}
		sginap(2);
	}
	color(0);
	sboxs(0,yo,S.x,yo);
	sboxs(xo,0,xo,S.y);
	x1=xo; 
	y1=yo;

	while( TRUE ) {
		getdev(2,mdev,mval);
		mval[1]-= S.vys;
		if(mval[0]!=xo || mval[1]!=yo) {
			color(0);
			sboxs(x1,y1,xo,yo);
			rectfi(0,0,400,50);
			xo=mval[0]; 
			yo=mval[1];
			color(3-mode);
			sboxs(x1,y1,xo,yo);
			x2= abs(x1-xo);
			y2= abs(y1-yo);
			if(view0.persp==0) {
				xx2= x2*50*view0.dproj;
				yy2= y2*50*view0.dproj;
				sprintf(string,"x:%d y:%d d:%d     ",(long)xx2,(long)yy2,(long)fsqrt(xx2*xx2+yy2*yy2));
			}
			else {
				if(view0.persp>=2) {
					x2/= obsviewXfac;
					y2/= obsviewYfac;
				}
				sprintf(string,"x:%d y:%d d:%.2f     ",x2,y2,fsqrt(x2*x2+y2*y2));
			}
			
			cmov2i(10,10);
			charstr(string);
		}
		if (qtest()){
			event== traces_qread(&dum);
			if ( event== ESCKEY ) {
				color(0); 
				clear();
				drawmode(NORMALDRAW);
				persp(1);
				return;
			}
			else if(event==LEFTMOUSE || event==RIGHTMOUSE) break;
		}
		sginap(2);
	}
	if(event==LEFTMOUSE) sel=1;

	color(0); 
	clear();
	drawmode(NORMALDRAW);
	persp(1);
	if(x1<xo) {
		xo=x1; 
		x1=mval[0];
	}
	if(y1<yo) {
		yo=y1; 
		y1=mval[1];
	}

	if(xo==x1 || yo==y1) return;

	if(mode==1) {	/* renderborder */
		G.xminb= xo;
		G.yminb= yo;
		G.xmaxb= x1;
		G.ymaxb= y1;
	}
	else if(G.ebase) {  /* select border */
		if(G.edve.first) {
			eve= (struct EditVert *)G.edve.first;
			while(eve) {
				if(eve->h==0 && eve->xs>xo && eve->xs<x1) {
					if(eve->ys>yo && eve->ys<y1) {
						if(sel) eve->f|= 1;
						else eve->f&= 254;
					}
				}
				eve= eve->next;
			}
			tekenvertices_ext();
		}
		else if ELEM3(G.ebase->soort, 5, 11, -2) {
			nu= editNurb.first;
			while(nu) {
				if((nu->type & 7)==1) {
					bezt= nu->bezt;
					a= nu->pntsu;
					while(a--) {
						if(bezt->hide==0) {
							if(bezt->s[0][0]>xo && bezt->s[0][0]<x1) {
								if(bezt->s[0][1]>yo && bezt->s[0][1]<y1) {
									if(sel) bezt->f1|= 1;
									else bezt->f1 &= ~1;
								}
							}
							if(bezt->s[1][0]>xo && bezt->s[1][0]<x1) {
								if(bezt->s[1][1]>yo && bezt->s[1][1]<y1) {
									if(sel) bezt->f2|= 1;
									else bezt->f2 &= ~1;
								}
							}
							if(bezt->s[2][0]>xo && bezt->s[2][0]<x1) {
								if(bezt->s[2][1]>yo && bezt->s[2][1]<y1) {
									if(sel) bezt->f3|= 1;
									else bezt->f3 &= ~1;
								}
							}
						}
						bezt++;
					}
				}
				else {
					bp= nu->bp;
					a= nu->pntsu*nu->pntsv;
					while(a--) {
						if(bp->hide==0) {
							if(bp->s[0]>xo && bp->s[0]<x1) {
								if(bp->s[1]>yo && bp->s[1]<y1) {
									if(sel) bp->f1|= 1;
									else bp->f1 &= ~1;
								}
							}
						}
						bp++;
					}
				}
				nu= nu->next;
			}
			projektie();
		}
		else if(G.ebase->soort== -4) {
			bop= bpbase.first;
			while(bop) {
				if(bop->sx>xo && bop->sx<x1) {
					if(bop->sy>yo && bop->sy<y1) {
						if(sel) bop->f |= 1;
						else bop->f &= ~1;
					}
				}
				bop= bop->next;
			}
			vars_for_buttons();	/* editika.c */
			projektie();
		}
	}
	else {
		short hits, buffer[2000];
	
		base=G.firstbase;
		hits= selectprojektie(buffer, xo, yo, x1, y1);
		
		while(base) {
			if(base->lay & view0.lay) {
				for(a=0; a<hits; a++) {
					if(base->selcol== buffer[2*a+1]) {
						if(sel) base->f|= 1;
						else base->f&= 254;
						tekenbase_ext(base);
						break;
					}
				}
			}
			base=base->next;
		}
		if(G.machine!=ENTRY) {	/* i.v.m. backbufprojektie */
			base= G.firstbase;
			while(base) {
				if(base->lay & view0.lay) {
					base->selcol= ((base->selcol & 0xF00)<<12) + ((base->selcol & 0xF0)<<8) + ((base->selcol & 0xF)<<4);
				}
				base= base->next;
			}
		}
	}

	while( getbutton(LEFTMOUSE) || getbutton(RIGHTMOUSE) );
	while(qtest()) {
		if(traces_qread(&sel)==INPUTCHANGE) {
			if(sel) G.winakt=sel;
		}
	}
	if(mode==0) countall();

	if(mode && view0.persp==2) projektie();
	else tekenoverdraw(0);

}

void recalcnormals()
{
	struct Base *base;
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve,*adrve1, *adrve2, *adrve3;
	struct VlakOb *adrvl;
	int len;
	short a,col=1;

	if( okee("Recalc normals")==0 ) {
		if( okee("Vertices->normals")==0 ) return;
		if( button(&col,1,15,"colblnr:")==0) return;
		col--;
		base=G.basact;
		if(base) {
			if(base->soort==1) {
				ob=(struct ObData *)base->d;
				vv=ob->vv;
				adrve=(struct VertOb *)(vv+1);
				adrvl=(struct VlakOb *)(adrve+vv->vert);
				for(a=0;a<vv->vlak;a++,adrvl++) {
					if( (adrvl->ec & 15)==col) {
						adrve1= adrve+adrvl->v1;
						VECCOPY(adrve1->n,adrve1->c);
						adrve2= adrve+adrvl->v2;
						VECCOPY(adrve2->n,adrve2->c);
						adrve3= adrve+adrvl->v3;
						VECCOPY(adrve3->n,adrve3->c);

						adrvl->f=0;
						len=adrvl->n[0]*adrve1->n[0]+adrvl->n[1]*adrve1->n[1]+adrvl->n[2]*adrve1->n[2];
						if(len<0) adrvl->f=1;
						len=adrvl->n[0]*adrve2->n[0]+adrvl->n[1]*adrve2->n[1]+adrvl->n[2]*adrve2->n[2];
						if(len<0) adrvl->f+=2;
						len=adrvl->n[0]*adrve3->n[0]+adrvl->n[1]*adrve3->n[1]+adrvl->n[2]*adrve3->n[2];
						if(len<0) adrvl->f+=4;
					}
				}

			}
		}
		return;
	}

	if(G.ebase) {
		righthandfaces();	/* zet normalen rechtsdraaiend */
		projektie();
		return;
	}

	base=G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			if(base->soort==1) {
				ob=(struct ObData *)base->d;
				vv=ob->vv;
				adrve=(struct VertOb *)(vv+1);
				adrvl=(struct VlakOb *)(adrve+vv->vert);
				normalen(adrve,adrvl,vv->vert,vv->vlak);
			}
		}
		base=base->next;
	}
}



void docentre(base, mode)
struct Base *base;
int mode;	/* 0= origin zelfde, 1: vertices blijven op zelfde plek */
{
	struct Base *par;
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct MoData *mo;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Nurb *nu, *nu1;
	struct EditVert *eve;
	float *fp, fac, cent[3], min[3], max[3], mat[4][4], omat[3][3];
	float wspace;
	int c,a;

	min[0]=min[1]=min[2]= 1.0e10;
	max[0]=max[1]=max[2]= -1.0e10;

	if(G.ebase && mode) {
		error("Not in editbase mode");
		return;
	}

	if(G.ebase==base && base->soort==1) {
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			for(c=0;c<3;c++) {
				if(eve->co[c]>max[c]) max[c]= eve->co[c];
				if(eve->co[c]<min[c]) min[c]= eve->co[c];
			}
			eve= eve->next;
		}
		cent[0]= (min[0]+max[0])/2.0;
		cent[1]= (min[1]+max[1])/2.0;
		cent[2]= (min[2]+max[2])/2.0;
		eve= (struct EditVert *)G.edve.first;
		while(eve) {
			for(c=0;c<3;c++) {
				eve->co[c]-= cent[c];
			}
			eve= eve->next;
		}
	}
	else if(base->soort==1) {
		ob=(struct ObData *)base->d;
		vv=ob->vv;

		adrve=(struct VertOb *)(vv+1);
		adrvl=(struct VlakOb *)(adrve+vv->vert);

		for(a=0;a<vv->vert;a++,adrve++) {
			for(c=0;c<3;c++) {
				min[c]= MIN2(min[c],adrve->c[c]);
				max[c]= MAX2(max[c],adrve->c[c]);
			}
		}

		fac= 0.5*MAX3(max[0]-min[0],max[1]-min[1],max[2]-min[2]);
		cent[0]= (min[0]+max[0])/2.0;
		cent[1]= (min[1]+max[1])/2.0;
		cent[2]= (min[2]+max[2])/2.0;
		max[0]= (max[0]-min[0]+1)/2.0;
		max[1]= (max[1]-min[1]+1)/2.0;
		max[2]= (max[2]-min[2]+1)/2.0;

		if(fac<.001) fac= 1.0;
		else {
			fac/= 32767.0;
			vv->ws*= fac;
		}
		vv->afm[0]= vv->ws*max[0];
		vv->afm[1]= vv->ws*max[1];
		vv->afm[2]= vv->ws*max[2];
		
		adrve=(struct VertOb *)(vv+1);
		for(a=0;a<vv->vert;a++,adrve++) {
			for(c=0;c<3;c++)
				adrve->c[c]= (adrve->c[c]-cent[c])/fac;
		}
		
		if(mode) {
			VecMulf(cent, vv->ws/fac);
			parentbase(base, mat, omat);
			Mat3CpyMat4(omat, mat);
			
			Mat3MulVecfl(omat, cent);
			base->v[0]+= cent[0];
			base->v[1]+= cent[1];
			base->v[2]+= cent[2];
		}
		
		/* displisten van alle users, ook deze base */
		base= G.firstbase;
		while(base) {
			if(base->soort==1 && base->lay & view0.lay) {
				ob= (struct ObData *)base->d;
				if(ob->vv == vv) makeDispList(base);
			}
			base= base->next;
		}

	}
	else if ELEM3(base->soort, 5, 11, -2) {
		fp= 0;
		if(base==G.ebase) {
			nu1= editNurb.first;
		}
		else if(base->soort== -2) {
			mo= (struct MoData *)base->d;
			nu1= mo->curve.first;
		}
		else {
			ob=(struct ObData *)base->d;
			nu1= ob->cu->curve.first;
			fp= &(ob->cu->ws);
		}
		if(nu1==0) return;
		
		nu= nu1;
		while(nu) {
			minmaxNurb(nu, min, max);
			nu= nu->next;
		}
		
		fac= 0.5*MAX3(max[0]-min[0],max[1]-min[1],max[2]-min[2]);
		cent[0]= (min[0]+max[0])/2.0;
		cent[1]= (min[1]+max[1])/2.0;
		cent[2]= (min[2]+max[2])/2.0;

		if(base->soort==5) wspace= 32767.0;
		else wspace= 16384.0;

		if(fp==0) {
			fac= 1.0;
		}
		else {
			if(fac<.001) fac= 1.0;
			fac/= wspace;
			(*fp) *= fac;
			fac= 1.0/fac;
		}
			
		nu= nu1;
		while(nu) {
			if( (nu->type & 7)==1) {
				a= nu->pntsu;
				bezt= nu->bezt;
				while(a--) {
					VecSubf(bezt->vec[0], bezt->vec[0], cent);
					VecMulf(bezt->vec[0], fac);
					VecSubf(bezt->vec[1], bezt->vec[1], cent);
					VecMulf(bezt->vec[1], fac);
					VecSubf(bezt->vec[2], bezt->vec[2], cent);
					VecMulf(bezt->vec[2], fac);
					bezt++;
				}
			}
			else {
				a= nu->pntsu*nu->pntsv;
				bp= nu->bp;
				while(a--) {
					VecSubf(bp->vec, bp->vec, cent);
					VecMulf(bp->vec, fac);
					bp++;
				}
			}
			nu= nu->next;
		}
		fac= 1.0/fac;

		if(mode && fp) {
			VecMulf(cent, (*fp)/fac);
			parentbase(base, mat, omat);
			Mat3CpyMat4(omat, mat);
			
			Mat3MulVecfl(omat, cent);
			base->v[0]+= cent[0];
			base->v[1]+= cent[1];
			base->v[2]+= cent[2];
		}

		if(G.ebase== 0 && base->soort==11) makeBevelList(base);
		makeDispList(base);
	}

	projektie();

}

void fitinWspace()
{
	struct Base *base;
	struct ObData *ob;
	struct CurveData *cu;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	float *f,facx, facy, facz, cent[3],min[3],max[3], tmat[3][3], mat[3][3];
	float wspace;
	int c, a, ret=0;

	if(okee("Fit in Wspace")==0) return;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base) ) {
			if(base->soort==1) {
				ob=(struct ObData *)base->d;
				vv=ob->vv;
				
				if(vv->us>1) {
					error("Can't fit in a duplicate");
					ret= 1;
				}
				if(vv->key) {
					error("Can't fit in a key");
					ret= 1;
				}
				if(ret) return;
				
				min[0]=min[1]=min[2]= 1.0e10;
				max[0]=max[1]=max[2]= -1.0e10;
	
				adrve=(struct VertOb *)(vv+1);
				adrvl=(struct VlakOb *)(adrve+vv->vert);
				for(a=0;a<vv->vert;a++,adrve++) {
					for(c=0;c<3;c++) {
						min[c]= MIN2(min[c],adrve->c[c]);
						max[c]= MAX2(max[c],adrve->c[c]);
					}
				}
	
				facx= 0.5*(max[0]-min[0]);
				if(facx<=1.0) facx= 32767.0;
				facy= 0.5*(max[1]-min[1]);
				if(facy<=1.0) facy= 32767.0;
				facz= 0.5*(max[2]-min[2]);
				if(facz<=1.0) facz= 32767.0;
				cent[0]= (min[0]+max[0])/2.0;
				cent[1]= (min[1]+max[1])/2.0;
				cent[2]= (min[2]+max[2])/2.0;
				max[0]= (max[0]-min[0]+1)/2.0;
				max[1]= (max[1]-min[1]+1)/2.0;
				max[2]= (max[2]-min[2]+1)/2.0;
	
				facx/= 32767.0;
				facy/= 32767.0;
				facz/= 32767.0;
				Mat3One(mat);
				mat[0][0]= facx;
				mat[1][1]= facy;
				mat[2][2]= facz;
				Mat3CpyMat3(tmat, base->m);
				Mat3MulMat3(base->m, mat, tmat);
				base->f &= ~8;
			
				adrve=(struct VertOb *)(vv+1);
				for(a=0;a<vv->vert;a++,adrve++) {
					adrve->c[0]= (adrve->c[0]-cent[0])/facx;
					adrve->c[1]= (adrve->c[1]-cent[1])/facy;
					adrve->c[2]= (adrve->c[2]-cent[2])/facz;
				}
			}
			else if(base->soort==5) {
				ob= (struct ObData *)base->d;
				cu= ob->cu;
				
				if(cu->key) {
					error("Can't fit in a key");
					ret= 1;
				}
				if(ret) return;
				
				min[0]=min[1]=min[2]= 1.0e10;
				max[0]=max[1]=max[2]= -1.0e10;
	
				nu= cu->curve.first;
				while(nu) {
					a= nu->pntsu*nu->pntsv;
					bp= nu->bp;
					while(a--) {
						for(c=0;c<3;c++) {
							min[c]= MIN2(min[c],bp->vec[c]);
							max[c]= MAX2(max[c],bp->vec[c]);
						}
						bp++;
					}
					nu= nu->next;
				}
	
				facx= 0.5*(max[0]-min[0]);
				if(facx<=1.0) facx= 32767.0;
				facy= 0.5*(max[1]-min[1]);
				if(facy<=1.0) facy= 32767.0;
				facz= 0.5*(max[2]-min[2]);
				if(facz<=1.0) facz= 32767.0;
				
				cent[0]= (min[0]+max[0])/2.0;
				cent[1]= (min[1]+max[1])/2.0;
				cent[2]= (min[2]+max[2])/2.0;
				max[0]= (max[0]-min[0]+1)/2.0;
				max[1]= (max[1]-min[1]+1)/2.0;
				max[2]= (max[2]-min[2]+1)/2.0;
	
				facx/= 32767.0;
				facy/= 32767.0;
				facz/= 32767.0;
				Mat3One(mat);
				mat[0][0]= facx;
				mat[1][1]= facy;
				mat[2][2]= facz;
				Mat3CpyMat3(tmat, base->m);
				Mat3MulMat3(base->m, mat, tmat);
				base->f &= ~8;
			
				nu= cu->curve.first;
				while(nu) {
					a= nu->pntsu*nu->pntsv;
					bp= nu->bp;
					while(a--) {
						bp->vec[0]= (bp->vec[0]-cent[0])/facx;
						bp->vec[1]= (bp->vec[1]-cent[1])/facy;
						bp->vec[2]= (bp->vec[2]-cent[2])/facz;
						bp++;
					}
					nu= nu->next;
				}
				makeDispList(base);
			}
			else error("Can't fit in");
		}
	
		base= base->next;
	}
	projektie();
}

void testWspace(struct Base *base)
{
	struct ObData *ob;
	struct Nurb *nu;
	struct BezTriple *bezt;
	struct BPoint *bp;
	float fac, min[3], max[3], wspace;
	int a, b;
	
	fac= 1.0;
	min[0]= min[1]= min[2]= 1.0e20;
	max[0]= max[1]= max[2]= -1.0e20;

	if(base->soort==1) {
		
	}
	else if ELEM(base->soort, 5, 11) {
	
		if(base->soort==5) wspace= 32767.0;
		else wspace= 16384.0;
	
		ob= (struct ObData *)base->d;
		
		if(base==G.ebase) nu= editNurb.first;
		else nu= ob->cu->curve.first;
		
		if(nu) {
			while(nu) {
				minmaxNurb(nu, min, max);
				nu= nu->next;
			}
			min[0]= MAX3(fabs(min[0]), fabs(min[1]), fabs(min[2]));
			max[0]= MAX3(fabs(max[0]), fabs(max[1]), fabs(max[2]));
			fac= MAX2( min[0], max[0] );
			fac= wspace/fac;
			ob->cu->ws/= fac;
		}
		
		if(base==G.ebase) nu= editNurb.first;
		else nu= ob->cu->curve.first;
		
		while(nu) {
			if( (nu->type & 7)==1 ) {
				bezt= nu->bezt;
				a= nu->pntsu;
				while(a--) {
					for(b=0; b<3; b++) {
						VecMulf(bezt->vec[b], fac);
					}
					bezt++;
				}
			}
			else {
				bp= nu->bp;
				a= nu->pntsu*nu->pntsv;
				while(a--) {
					VecMulf(bp->vec, fac);
					bp++;
				}
			}
			nu= nu->next;
		}
	}
}

void apply()
{
	struct Base *base,*par;
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	struct MoData *mo;
	struct BezTriple *bezt;
	struct BPoint *bp;
	struct Nurb *nu;
	float *f,fac,vec[3],tmat[4][4],mat[3][3],min[3],max[3];
	int c,a;

	if(okee("Apply mat/quat")==0) return;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			par= base->p;
			base->p= 0;
			parentbase(base,tmat,mat);
			base->p= par;
			Mat3CpyMat4(mat,tmat);
			if ELEM3(base->soort, 5, 11, -2) {
				if(base->soort== -2) {
					mo= (struct MoData *)base->d;
					nu= mo->curve.first;
				}
				else {
					ob= (struct ObData *)base->d;
					nu= ob->cu->curve.first;
				}
				if(nu) {
					while(nu) {
						if(nu->bezt) {
							a= nu->pntsu;
							bezt= nu->bezt;
							while(a--) {
								Mat3MulVecfl(mat, bezt->vec[0]);
								Mat3MulVecfl(mat, bezt->vec[1]);	
								Mat3MulVecfl(mat, bezt->vec[2]);
								bezt++;
							}
						}
						else if(nu->bp) {
							a= nu->pntsu*nu->pntsv;
							bp= nu->bp;
							while(a--) {
								Mat3MulVecfl(mat, bp->vec);
								bp++;
							}
						}
						nu= nu->next;
					}
					Mat3One(base->m);
					base->q[0]= 1.0;
					base->q[1]=base->q[2]=base->q[3]= 0.0;
					makeDispList(base);
					testWspace(base);
				}
			}
			else if(base->soort==1) {
				ob=(struct ObData *)base->d;
				vv=ob->vv;
				if(vv->us>1) {
					error("Don't apply a duplicate");
					return;
				}
				if(vv->key) {
					error("Don't apply a key");
					return;
				}
				min[0]=min[1]=min[2]= 1.0e20;
				max[0]=max[1]=max[2]= -1.0e20;

				Mat3MulFloat(mat,vv->ws);
				adrve=(struct VertOb *)(vv+1);
				adrvl=(struct VlakOb *)(adrve+vv->vert);
				for(a=0;a<vv->vert;a++,adrve++) {
					VECCOPY(vec,adrve->c);
					Mat3MulVecfl(mat,vec);
					f= (float *)adrve->c;
					VECCOPY(f,vec);
					for(c=0;c<3;c++) {
						min[c]= MIN2(min[c],vec[c]);
						max[c]= MAX2(max[c],vec[c]);
					}
				}
				max[0]= MAX2(fabs(min[0]),fabs(max[0]));
				max[1]= MAX2(fabs(min[1]),fabs(max[1]));
				max[2]= MAX2(fabs(min[2]),fabs(max[2]));
				fac= MAX3(max[0],max[1],max[2]);
				fac= 32767.0/fac;
				vv->ws= 1.0/fac;
				VECCOPY(vv->afm,max);
				adrve=(struct VertOb *)(vv+1);
				for(a=0;a<vv->vert;a++,adrve++) {
					f= (float *)adrve->c;
					VECCOPY(vec,f);
					vec[0]*=fac;
					vec[1]*=fac;
					vec[2]*=fac;
					VECCOPY(adrve->c,vec);
				}

				Mat3One(base->m);
				base->q[0]= 1.0;
				base->q[1]=base->q[2]=base->q[3]= 0.0;

				adrve=(struct VertOb *)(vv+1);
				normalen(adrve,adrvl,vv->vert,vv->vlak);
			}
		}
		base= base->next;
	}

	projektie();

}


void fasterdraw()
{
	struct Base *base;
	struct ObData *ob;
	struct VV *vv;
	struct VertOb *adrve;
	struct VlakOb *adrvl;
	long toggle, a, ec;

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base) && (base->soort==1)) {
			ob= (struct ObData *)base->d;
			vv= ob->vv;
			vv->f1 &= ~1;
		}
		base= base->next;
	}

	base= G.firstbase;
	while(base) {
		if(TESTBASE(base) && (base->soort==1)) {
			ob= (struct ObData *)base->d;
			vv= ob->vv;
			if((vv->f1 & 1)==0) {
				vv->f1 |= 1;
				adrve= (struct VertOb *)(vv+1);
				adrvl= (struct VlakOb *)(adrve+vv->vert);
				toggle= 1;
				for(a=0; a<vv->vlak; a++) {
					if( (adrvl->ec & 32)==0 ) {
						toggle++;
						if(toggle & 1) adrvl->ec += 32;
					}
					if( (adrvl->ec & 64)==0 ) {
						toggle++;
						if(toggle & 1) adrvl->ec += 64;
					}
					if( (adrvl->ec & 128)==0 ) {
						toggle++;
						if(toggle & 1) adrvl->ec += 128;
					}
					adrvl++;
				}
			}
		}
		base= base->next;
	}

	/* belangrijk: vlaggen weer resetten */
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base) && (base->soort==1)) {
			ob= (struct ObData *)base->d;
			vv= ob->vv;
			vv->f1 &= ~1;
		}
		base= base->next;
	}

	projektie();
}

void clearworkbase()
{
	G.workbase->f= 9;
	G.workbase->f1= 2+8+16+64;
	G.workbase->f2= 0;
	G.workbase->q[0]= 1.0;
	G.workbase->q[1]= 0.0;
	G.workbase->q[2]= 0.0;
	G.workbase->q[3]= 0.0;
	Mat3One(G.workbase->m);
	G.workbase->t= 0;
	G.workbase->p= 0;
	G.workbase->v[0]= G.workbase->v[1]= G.workbase->v[2]= 0;
	G.workbase->o[0]= G.workbase->o[1]= G.workbase->o[2]= 0;
	G.workbase->sf= 1;
	G.workbase->rodnr= G.workbase->skelnr= 0;
	
}

/* ***************************************  */

void arrowsmovecursor(event)
short event;
{
	Device mdev[2];
	short mval[2],a=0;

	mdev[0]=MOUSEX; 
	mdev[1]=MOUSEY;
	getdev(2,mdev,mval);

	if(event==UPARROWKEY) {
		a=1; 
		mval[1]++;
	}
	if(event==DOWNARROWKEY) {
		a=1; 
		mval[1]--;
	}
	if(event==LEFTARROWKEY) {
		a=1; 
		mval[0]--;
	}
	if(event==RIGHTARROWKEY) {
		a=1; 
		mval[0]++;
	}
	if(a) {
		if(mval[0]<0) mval[0]=0;
		if(mval[1]<0) mval[1]=0;
		if(mval[0]>S.x) mval[0]=S.x;
		if(mval[1]>S.y) mval[1]=S.y;

		setvaluator(MOUSEX,mval[0],0,S.x-1);
		setvaluator(MOUSEY,mval[1],0,S.y-1);
	}
}

short testproj(xn,yn,mval)
short xn,yn,*mval;
{
	float x,y,z;

	/* welke beweging is het grootst? die wordt het */
	xn= (xn-mval[0]);
	yn= (yn-mval[1]);
	x = fabs(G.persinv[0][0]*xn + G.persinv[1][0]*yn);
	y = fabs(G.persinv[0][1]*xn + G.persinv[1][1]*yn);
	z = fabs(G.persinv[0][2]*xn + G.persinv[1][2]*yn);

	if(x>=y && x>=z) return 0;
	else if(y>=x && y>=z) return 1;
	else return 2;
}

void initprintoverdraw()
{

	winset(G.winar[1]);
	drawmode(OVERDRAW);
	color(1);
	sboxfs(0,200,1279,220);		/* ortho doet dit goed! */
	color(3);
	sboxs(0,200,1279,220);

}

void clearprintoverdraw()
{

	winset(G.winar[1]);
	drawmode(OVERDRAW);
	color(0);
	sboxfs(0,200,1279,220);
	drawmode(NORMALDRAW);
}

void endprintoverdraw()
{

	drawmode(NORMALDRAW);
	winset(G.winar[0]);
}

void setbaseflags_for_editing()
{
	/* als hie==1:
		als base selected en heeft parent selected:
			base->f=2, base->f2= 32,
		als base niet selected en parent selected:
			base->f=2
	    Verder: test op Duplibase (G.f & 4)
	*/
	struct Base *base,*b;
	struct MoData *mo;
	struct Key *key;
	short parsel, dupli=0;



	if(G.hie) {
		base= G.firstbase;
		while(base) {
			base->f&= ~2;
			if(base->soort== -2 && base->p==0) {
				mo= (struct MoData *)base->d;
				if(mo->f & 4) dupli= 1;
			}
			if(base->dupli) dupli= 1;
			
			if(base->lay & view0.lay) {
				parsel= 0;
				b= base->p;
				while(b) {
					if(TESTBASE(b)) {
						parsel= 1;
						break;
					}
					b= b->p;
				}
				if(base->pkey) {
					key= base->pkey;
					while(key) {
						b= (struct Base *) *( (long *)(key+1) );
						if(b && b!=base && TESTBASE(b)) {
							parsel= 1;
							break;
						}
						key= key->next;
					}
				}
				if(parsel) {
					if(base->f & 1) {
						base->f2|= 32;
						base->f|= 2;
						base->f-= 1;
					}
					else base->f|=2;
				}

				b= base->t;
				if(b) if(TESTBASE(b)) if((base->f & 1)==0)  base->f|=2;

			}
			base= base->next;
		}
	}

	if(dupli) G.f |= 4;
	else G.f &= ~4;

}

void clearbaseflags_for_editing()	/* doet ook shade */
{
	struct Base *base;
	struct ObData *ob;
	short shadeall= 0;

	if(G.zbuf) {
		/* is er een lamp verplaatst? */
		base= G.firstbase;
		while(base) {
			if(TESTBASE(base)) {
				if(base->soort==2) {
					shadeall= 1;
					break;
				}
			}
			base= base->next;
		}
		/* test de bases */
		base= G.firstbase;
		while(base) {
			if(base->soort & 1) {
				if(base->lay & view0.lay) {
					ob= (struct ObData *)base->d;
					if(shadeall) {
						if(ob->dt==2) shadeDispList(base);
					}
					else if(base->f & 3) {
						if(ob->dt==2) shadeDispList(base);
					}
				}
			}
			base= base->next;
		}
	}

	base= G.firstbase;
	while(base) {
		if(base->lay & view0.lay) {
			if(base->f2 & 32) {
				base->f|= 1;
				base->f2-= 32;
			}
		}
		base->f&= ~2;
		base= base->next;
	}
}

void addstartframe()
{
	struct Base *base;
	short addval=0;
	
	if(button(&addval, -1000, 1000, "Add Sf: ")==0) return;
	
	base= G.firstbase;
	while(base) {
		if(TESTBASE(base)) {
			base->sf+= addval;
		}
		base= base->next;
	}
	projektie();
}

void helpline(vec)
long *vec;
{
	short mval[2], mval1[2];

	getmouseco(mval);
	berekenschermco_noclip(vec, mval1);
	persp(0);
	frontbuffer(1);
	cpack(0);
	setlinestyle(1);
	bgnline(); 
	v2s(mval); 
	v2s(mval1); 
	endline();
	setlinestyle(0);
	persp(1);
	frontbuffer(0);
}

void grabber()
{
	struct Base *base,*b;
	struct Key *key;
	float dx,dy,z=0,mat[3][3],cmat[3][3],tmat[4][4];
	long *adror,*a,**adrin,*ai,dvec[3],vec[3];
	short teller=0,a1, val,mval[2],xn,yn,xm,ym,toets,proj,midtog=0, cambase=0;
	Device mdev[2];
	char s[40];

	if(G.firstbase==0) return;
	if(G.ebase) {
		vertgrabber();
		return;
	}

	G.moving=1;

	setbaseflags_for_editing();
	base=G.firstbase;
	while(base) {		/* aantal tellen en alvast tekenen */
		if(view0.lay & base->lay) {
			if(base->f & 1) {
				if(teller==0) z= initgrabz(base->v[0],base->v[1],base->v[2]);
				teller++;
			}
			if(base->f & 3) {
				tekenbase_ext(base);
			}
		}
		base= base->next;
	}
	if(teller) { 		/* het opslaan in temp blok */

		base=G.firstbase;
		adror=(long *)callocN(12*teller,"grabber0");
		adrin=(long **)callocN(4*teller,"grabber1");
		a=adror;
		teller=0;
		while(base) {
			if(view0.lay & base->lay) {
				if(base->f & 1) {
					if(G.hie && PARENT(base)) ai= getbasekeyadr(base,1);
					else ai= getbasekeyadr(base,0);

					adrin[teller++]=ai;
					VECCOPY(a,ai);
					a+=3;
				}
			}
			base=base->next;
		}
		mdev[0] = MOUSEX;
		mdev[1] = MOUSEY;

		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		xn=xm=mval[0]; 
		yn=ym=mval[1];

		dvec[0]=dvec[1]=dvec[2]=0;
		initprintoverdraw();
		sprintf(s,"Dx: %d",dvec[0]); 
		cmov2i(10,204); 
		charstr(s);
		sprintf(s,"Dy: %d",dvec[1]); 
		cmov2i(160,204); 
		charstr(s);
		sprintf(s,"Dz: %d",dvec[2]); 
		cmov2i(310,204); 
		charstr(s);
		endprintoverdraw();

		if(teller==1 && view0.cambase && view0.persp>=2) {
			if(TESTBASE(view0.cambase)) {
				cambase= 1;
				z= view0.grid;
			}
		}

		while(TRUE) {
			getdev(2, mdev, mval);
			mval[1]-= S.vys;
			if(mval[0]!=xm || mval[1]!=ym) {
				xm=mval[0]; 
				ym=mval[1];
				dx= 2.0*(xn-xm)*z/S.vxw;
				dy= 2.0*(yn-ym)*z/S.vyw;
				if(cambase) {
					dvec[0]+= dy*G.persinv[2][0];
					dvec[1]+= dy*G.persinv[2][1];
					dvec[2]+= dy*G.persinv[2][2];
					xm--;   /* blijft ie lopen */
				} else {
					dvec[0] = (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
					dvec[1] = (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
					dvec[2] = (G.persinv[0][2]*dx + G.persinv[1][2]*dy);
				}

				base=G.firstbase;
				if(G.move2==1) dvec[1]=dvec[2]=0;
				if(G.move2==2) dvec[0]=dvec[2]=0;
				if(G.move2==3) dvec[0]=dvec[1]=0;
				if(midtog) {
					if(proj==0) dvec[1]=dvec[2]=0;
					if(proj==1) dvec[0]=dvec[2]=0;
					if(proj==2) dvec[0]=dvec[1]=0;
				}
				if(G.qual & (48+3) ) {
					dy= view0.grid;
					if(G.qual & 3) dy/= 10.0;
					
					dx= dvec[0]/dy;
					dx= ffloor(dx+.5);
					dvec[0]= dx*dy;
					dx= dvec[1]/dy;
					dx= ffloor(dx+.5);
					dvec[1]= dx*dy;
					dx= dvec[2]/dy;
					dx= ffloor(dx+.5);
					dvec[2]= dx*dy;
				}
				a= adror;
				teller=0;
				while(base) {
					if(view0.lay & base->lay) {
						if(base->f & 1) {
							ai=adrin[teller++];
							if( PARENT(base) && G.hie!=0) {
								parentbase(base,tmat,mat);
								
								Mat3Inv(cmat,mat);
								VECCOPY(vec,dvec);

								Mat3MulVec(cmat,vec);

								ai[0]= *(a++)-vec[0];
								ai[1]= *(a++)-vec[1];
								ai[2]= *(a++)-vec[2];
							} else {
								ai[0]= *(a++)-dvec[0];
								ai[1]= *(a++)-dvec[1];
								ai[2]= *(a++)-dvec[2];
							}
						}
					}
					base=base->next;
				}
				projektie();
				initprintoverdraw();
				sprintf(s,"Dx: %d",-dvec[0]); 
				cmov2i(10,204); 
				charstr(s);
				sprintf(s,"Dy: %d",-dvec[1]); 
				cmov2i(160,204); 
				charstr(s);
				sprintf(s,"Dz: %d",-dvec[2]); 
				cmov2i(310,204); 
				charstr(s);
				endprintoverdraw();
			}
			else sginap(2);

			if( qtest() ) {
				toets= traces_qread(&val);
				if(val) {
					if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
					if(toets==MIDDLEMOUSE) {
						midtog= ~midtog;
						if(midtog) proj= testproj(xn,yn,mval);
					}
					arrowsmovecursor(toets);
				}
				if(toets==INPUTCHANGE) G.winakt= val;
				toets=0;
				xm= -10;	/* dan doetie projektie */
			}
		}
		if(toets==ESCKEY) {
			base=G.firstbase;
			a=adror;
			teller=0;
			while(base) {
				if(view0.lay & base->lay) {
					if(base->f & 1) {
						ai=adrin[teller++];
						VECCOPY(ai,a);
						a+=3;
						if(base->skelnr) calc_deform(base);
					}
				}
				base=base->next;
			}
		}
		freeN(adror);
		freeN(adrin);
		G.moving=0;
		clearprintoverdraw();
		clearbaseflags_for_editing();
		projektie();
	}
	G.moving=0;
}

void roteer()
{
	struct Base *base,*b;
	struct MoData *mo;
	struct Key *key;
	float dx,dy,mat[4][4],omat[3][3],imat[3][3],tmat[3][3],n[3],rotn[3], rotn1[3];
	float pmat[3][3], bmat[3][3], qtot[4], qpar[4];
	float *adror,*a,hoek=0,hoeko=0,deler,choek,dhoek,si,co;
	float **adrin,*ai,hoek1=0,hoek2=0;
	float q1[4],q2[4],q3[4], q5[4];
	long *adrve,*v,vec[3],vec1[3];
	short teller=0,teller2,a1,val,mval[2],toets,midtog=0,proj;
	short cambase=0, xc=0,yc=0,xn,yn,dx1,dy1,dx2,dy2, filt;
	Device mdev[2];
	char s[40];

	if(G.firstbase==0) return;
	if(G.ebase) {
		vertroteer();
		return;
	}

	G.moving=1;
	vec[0]=vec[1]=vec[2]=0;
	q1[0]=q1[1]=q1[2]=0; /* tijdelijk voor rotatie middelpunt */

	setbaseflags_for_editing();
	base=G.firstbase;
	while(base) {		/* aantal tellen en alvast tekenen en centrum */
		if(view0.lay & base->lay) {
			if(base->f & 1) {
				q1[0]+= base->r[0]; 
				q1[1]+= base->r[1]; 
				q1[2]+= base->r[2];
				teller++;
			}
			if(base->f & 3)	{
				tekenbase_ext(base);
				base->f&= ~4;	/* quateenheid flag wissen */
			}
		}
		base= base->next;
	}
	if(teller) {
		if(G.move2) {
			n[0]=n[1]=n[2]=0;
			if(G.move2==1) n[0]=1;
			if(G.move2==2) n[1]=1;
			if(G.move2==3) n[2]=1;
		} else {
			n[0]= -G.persinv[2][0];
			n[1]= -G.persinv[2][1];
			n[2]= -G.persinv[2][2];
			Normalise(n);
		}
		base=G.firstbase;	/* het opslaan in temp blok */

		adror=(float *)callocN(16*teller,"roteer1");
		adrve=(long *)callocN(24*teller,"roteer2");
		adrin=(float **)mallocN(4*teller,"roteer3");
		a=adror;
		v=adrve;
		teller2=0;
		while(base) {
			if(view0.lay & base->lay) {
				if(base->f & 1) {
					ai= (float *)getbasekeyadr(base,2);
					adrin[teller2++]=ai;
					QUATCOPY(a,ai); 
					a+=4;
					if(PARENT(base)) {
						VECCOPY(v,base->r);
					}
					else { 
						VECCOPY(v,base->v); 
					}
					v+=3;
					VECCOPY(v,base->o);
					v+=3;
				}
			}
			base=base->next;
		}

		if(G.move1==1) {
			berekenschermco_noclip(view0.muis,mval);
			xc= mval[0]; 
			yc= mval[1];
			VECCOPY(vec,view0.muis);
			teller=2;
		} else {
			vec[0]=q1[0]/teller; 
			vec[1]=q1[1]/teller; 
			vec[2]= q1[2]/teller;
			berekenschermco_noclip(vec,mval);
			xc= mval[0]; 
			yc= mval[1];
			if(G.move1==2) teller=0; /* rond eigen centrum */
		}
		if(teller==1 && view0.cambase && (view0.cambase->f & 1) && view0.persp>=2) {
			xc= 640; 
			yc= 400;
			cambase= 1;
		}

		mdev[0] = MOUSEX;
		mdev[1] = MOUSEY;
		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		dx1=xc-mval[0]; 
		dy1=yc-mval[1];
		xn= mval[0]; 
		yn= mval[1]; /* voor midtog */

		initprintoverdraw();
		sprintf(s,"Rotate: %3.3f\n",hoek); 
		cmov2i(10,204); 
		charstr(s);
		endprintoverdraw();

		if(teller>1) helpline(vec);

		VECCOPY(rotn,(G.persinv[0]));	/* op deze plek vanwege cambase rot */
		Normalise(rotn);
		VECCOPY(rotn1,(G.persinv[1]));
		Normalise(rotn1);

		while(TRUE) {
			getdev(2, mdev, mval);
			mval[1]-= S.vys;
			if(midtog) {
				dx2= -xn+mval[0]; 
				dy2= yn-mval[1];

				hoek1+= .2*(float)(dy2-dy1);
				if(G.qual & 32) hoek1= 5*ffloor(hoek1/5.0 +.5);
				choek= PI*hoek1/180.0;
				si= fsin(choek); 
				co= fcos(choek);
				q2[0]= co;
				q2[1]= si*rotn[0];
				q2[2]= si*rotn[1];
				q2[3]= si*rotn[2];

				hoek2+= .2*(float)(dx2-dx1);
				if(G.qual & 32) hoek2= 5*ffloor(hoek2/5.0 +.5);
				choek= PI*hoek2/180.0;
				si= fsin(choek); 
				co= fcos(choek);
				q3[0]= co;
				q3[1]= si*rotn1[0];
				q3[2]= si*rotn1[1];
				q3[3]= si*rotn1[2];
				QuatMul(q1,q2,q3);
				if( q1[0]!=hoeko ) {
					dx1=dx2; 
					dy1=dy2;
				}
			}
			else {
				dx2=xc-mval[0]; 
				dy2=yc-mval[1];
				deler=fsqrt( (float)(dx1*dx1+dy1*dy1)*(dx2*dx2+dy2*dy2));
				if(deler>1) {
					choek=(dx1*dx2+dy1*dy2)/deler;
					dhoek= 180.0*facos(choek)/PI;
					if( (dx1*dy2-dx2*dy1)<0 ) dhoek= -dhoek;
					if(G.qual & 3) hoek+= dhoek/30.0; 
					else hoek+=dhoek;
					/* if(hoek>=360) hoek-=360; */
					/* else if(hoek<-360) hoek+=360; */
					if(G.qual & 32) if(G.qual & 3) hoek=ffloor(hoek+.5);
					else hoek= 5*ffloor(hoek/5.0 +.5);
					/* if(hoek>=360) hoek-=360; */
					/* else if(hoek<-360) hoek+=360; */

					choek=PI*hoek/360.0;
					si= fsin(choek);
					co= fcos(choek);
					q1[0]= co;
					q1[1]= si*n[0];
					q1[2]= si*n[1];
					q1[3]= si*n[2];
					if( q1[0]!=hoeko ) {
						dx1=dx2; 
						dy1=dy2;
					}
				}
			}
			if( q1[0]!=hoeko ) {
				hoeko= q1[0];
				QuatToMat3(q1, tmat);
				a=adror;
				v=adrve;
				teller2=0;

				base=G.firstbase;
				while(base) {
					if(TESTBASE(base)) {

						QUATCOPY(q5, a);	/* de te veranderen quat */
						
						ai=adrin[teller2++];
						QUATCOPY(ai, q5);
						
						if(base->p || base->t) {
							/* voor qpar: met workbase, voor o.a. qfillparent */
							clearworkbase();
							VECCOPY(G.workbase->v, base->v);	/* voor track! */
							VECCOPY(G.workbase->o, base->o);	/* voor track! */
							G.workbase->tflag= base->tflag;
							G.workbase->upflag= base->upflag;
							G.workbase->p= base->p;
							G.workbase->t= base->t;
							G.workbase->f= base->f;
							G.workbase->f1= base->f1;
							G.workbase->f2= base->f2;
							G.workbase->sf= base->sf;
							G.workbase->rodnr= base->rodnr;
							G.workbase->skelnr= base->skelnr;
							
							parentbase(G.workbase, mat, omat);
							clearworkbase();

							Mat4ToQuat(mat, qpar);

							/* bepaal de qtot: totale quat incl eigen basequat */
							/* let op volgorde QuatMul(), andersom geeft pas fouten onder complexe omstandigheden */
							QuatMul(qtot, qpar, q5);

							/* totale eindrotatie */
							QuatMul(q2, q1, qtot);
						
							/* deze met inverse qpar terugroteren */
							qpar[1]= -qpar[1]; qpar[2]= -qpar[2]; qpar[3]= -qpar[3];
							QuatMul(q2, qpar, q2);

						}
						else {
							QuatMul(q2, q1, q5);
						}			
						QUATCOPY(ai, q2);
						
						if(teller>1) {
							if( PARENT(base) && G.hie) {
								VECCOPY(vec1,v);
								vec1[0]-= vec[0];
								vec1[1]-= vec[1];
								vec1[2]-= vec[2];
								Mat3MulVec(tmat,vec1);
								vec1[0]+= vec[0]-v[0];
								vec1[1]+= vec[1]-v[1];
								vec1[2]+= vec[2]-v[2];
								Mat3Inv(imat,omat);
								Mat3MulVec(imat,vec1);
								base->o[0]= v[3]+vec1[0];
								base->o[1]= v[4]+vec1[1];
								base->o[2]= v[5]+vec1[2];

							} else {
								vec1[0]= v[0]-vec[0];
								vec1[1]= v[1]-vec[1];
								vec1[2]= v[2]-vec[2];
								Mat3MulVec(tmat,vec1);
								base->v[0]= vec1[0]+vec[0];
								base->v[1]= vec1[1]+vec[1];
								base->v[2]= vec1[2]+vec[2];
							}
						}
						a+=4; 
						v+=6;
					}
					base=base->next;
				}

				projektie();
				if(teller>1) helpline(vec);

				initprintoverdraw();
				if(midtog) {
					sprintf(s,"Rotate: %3.3f %3.3f\n",hoek1,hoek2);
					cmov2i(10,204); 
					charstr(s);
				}
				else {
					sprintf(s,"Rotate: %3.3f\n",hoek);
					cmov2i(10,204); 
					charstr(s);
				}
				endprintoverdraw();
			}
			else sginap(2);

			if( qtest() ) {
				toets= traces_qread(&val);
				if(val) {
					if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
					if(toets==MIDDLEMOUSE) {
						midtog= ~midtog;
						hoeko-=1.0;
						if(midtog) {
							getdev(2, mdev, mval);
							mval[1]-= S.vys;
							xn=mval[0]; 
							yn=mval[1];
							hoeko= q1[0]-1.0;
							dx1= 0; 
							dy1= 0;
						}
						else {
							dx1=xc-mval[0]; 
							dy1=yc-mval[1];
						}
					}
					arrowsmovecursor(toets);
				}
				if(toets==INPUTCHANGE) G.winakt= val;
				toets=0;
			}
		}
		base=G.firstbase;
		a=adror;
		v=adrve;
		teller2=0;
		G.moving=0;

		while(base) {
			if(view0.lay & base->lay) {
				if(base->f & 1) {
					ai=adrin[teller2++];
					if(toets==ESCKEY) {
						if( PARENT(base) ) {
							VECCOPY(base->o,v+3);
						} else {
							VECCOPY(base->v,v); 
						}
						QUATCOPY(ai,a);
					}
					v+=6;
					a+=4;
					if(base->skelnr) calc_deform(base);
				}
			}
			base=base->next;
		}
		
		freeN(adror); 
		freeN(adrve); 
		freeN(adrin);

		clearprintoverdraw();
		clearbaseflags_for_editing();

		projektie();
	}
	G.moving=0;
}


void sizing()
{
	struct Base *base,*b;
	struct Key *key;
	float *adror,*a,**adrin,*ai,mat[4][4],bmat[3][3],pmat[3][3],imat[3][3],cmat[3][3],omat[3][3];
	float vmat[3][3];
	float xref, yref, sizex,sizey,sizez,sizexo,sizeyo,sizezo,fac;
	float persmat[3][3],persinv[3][3];
	long *adrve,*v,dvec[3],vec[3],vec1[3],xc=0,yc=0;
	short teller=0,teller2,a1, val,mval[2],xm,ym,xn,yn,toets;
	short eenheidsmat(),midtog=0,proj;
	Device mdev[2];
	char s[20];

	if(G.firstbase==0) return;
	if(G.ebase) {
		vertsizing();
		return;
	}

	G.moving=1;
	dvec[0]=dvec[1]=dvec[2]=0;
	xref= yref= 1.0;
	
	setbaseflags_for_editing();
	base=G.firstbase;
	while(base) {		/* aantal tellen en alvast tekenen */
		if(view0.lay & base->lay) {
			if(base->f & 1) {
				xc+= base->sx; 
				yc+= base->sy;
				dvec[0]+= base->r[0]; 
				dvec[1]+= base->r[1]; 
				dvec[2]+= base->r[2];
				teller++;
			}
			if(base->f & 3)	{
				tekenbase_ext(base);
				base->f&= ~8;	/* mateenheid flag wissen */
			}
		}
		base= base->next;
	}
	if(teller) {
		base=G.firstbase;		/* het opslaan in temp blok */

		adror=(float *)callocN(36*teller,"sizing1");
		adrin=(float **)callocN(4*teller,"sizing1");
		adrve=(long *)callocN(24*teller,"sizing2");
		a=adror;
		v=adrve;
		teller2=0;
		while(base) {
			if(view0.lay & base->lay) {
				if(base->f & 1) {
					ai=(float *)getbasekeyadr(base,3);

					adrin[teller2++]=ai;
					if( PARENT(base) ) {
						VECCOPY(v,base->r);
					}
					else {
						VECCOPY(v,base->v);
					}
					v+=3;
					VECCOPY(v,base->o);
					Mat3CpyMat3(a,ai);
					v+=3; 
					a+=9;
				}
			}
			base=base->next;
		}

		if(G.move1==1) {
			berekenschermco_noclip(view0.muis,mval);
			xc= mval[0]; 
			yc= mval[1];
			VECCOPY(vec,view0.muis);
			teller=2;
		} else {
			/* if(teller>1 && G.move1==0) { */
			vec[0]=dvec[0]/teller; 
			vec[1]=dvec[1]/teller; 
			vec[2]= dvec[2]/teller;
			berekenschermco_noclip(vec, mval);
			xc= mval[0]; 
			yc= mval[1];
			/* } */
			/* else teller=0; */
			if(G.move1==2) teller=0;
		}
		Mat3CpyMat4(persmat,G.persmat);
		Mat3CpyMat4(persinv,G.persinv);
		mdev[0] = MOUSEX;
		mdev[1] = MOUSEY;
		getdev(2, mdev, mval);
		mval[1]-= S.vys;
		xm=mval[0]; 
		ym=mval[1];
		sizexo=sizeyo=sizezo=1.0;
		fac= fsqrt( (float)((yc-ym)*(yc-ym)+(xm-xc)*(xm-xc)) );
		if(fac<2.0) fac= 2.0;

		initprintoverdraw();
		sprintf(s,"Size X: %3.3f",sizexo); 
		cmov2i(10,204); 
		charstr(s);
		sprintf(s,"Size Y: %3.3f",sizeyo); 
		cmov2i(160,204); 
		charstr(s);
		sprintf(s,"Size Z: %3.3f",sizezo); 
		cmov2i(310,204); 
		charstr(s);
		if(G.move2==4) {
			sprintf(s,"SizeView");
			cmov2i(460,204); 
			charstr(s);
		} else if(G.move2==5) {
			sprintf(s,"Shear");
			cmov2i(460,204); 
			charstr(s);
		}
		endprintoverdraw();

		while(TRUE) {
			getdev(2, mdev, mval);
			mval[1]-= S.vys;
			xn=mval[0]; 
			yn=mval[1];
			if(G.move2>=4) {
				sizex= 1.0-(float)(xm-xn)*0.005;
				if(G.move2==4) sizey= 1.0+(float)(ym-yn)*0.005;
				else sizey= 1.0;
				sizez= 1.0;
			} else {
				sizex=sizey=sizez= (fsqrt( (float)((yc-yn)*(yc-yn)+(xn-xc)*(xn-xc)) ))/fac;
				if(G.move2==1) sizey=sizez=1;
				if(G.move2==2) sizex=sizez=1;
				if(G.move2==3) sizey=sizex=1;
			}
			if(midtog) {
				if(proj==0) sizey=sizez=1;
				if(proj==1) sizex=sizez=1;
				if(proj==2) sizey=sizex=1;
			}
			if(G.qual & 32) {
				if (G.qual & 3) {
					sizex= (ffloor(100.0*sizex))/100.0;
					sizey= (ffloor(100.0*sizey))/100.0;
					sizez= (ffloor(100.0*sizez))/100.0;
					if(sizex==0.0) sizex== 0.01;
					if(sizey==0.0) sizey== 0.01;
					if(sizez==0.0) sizez== 0.01;
				} else {
					sizex= (ffloor(10.0*sizex))/10.0;
					sizey= (ffloor(10.0*sizey))/10.0;
					sizez= (ffloor(10.0*sizez))/10.0;
					if(sizex==0.0) sizex== 0.1;
					if(sizey==0.0) sizey== 0.1;
					if(sizez==0.0) sizez== 0.1;
				}
			} else if (G.qual & 3) {
				sizex = ((sizex - 1.0) / 10.0) + 1.0;
				sizey = ((sizey - 1.0) / 10.0) + 1.0;
				sizez = ((sizez - 1.0) / 10.0) + 1.0;
			}
				/* x flip */
			val= testproj(mval[0]+10, mval[1], mval);
			if(val==0) sizex*= xref;
			else if(val==1) sizey*= xref;
			else sizez*=xref;
				/* y flip */
			val= testproj(mval[0], mval[1]+10, mval);
			if(val==0) sizex*= yref;
			else if(val==1) sizey*= yref;
			else sizez*=yref;
			
			if(sizex==0.0) sizex== 0.001;
			if(sizey==0.0) sizey== 0.001;
			if(sizez==0.0) sizez== 0.001;

			if(sizex!=sizexo || sizey!=sizeyo || sizez!=sizezo) {
				base=G.firstbase;
				a=adror; 
				v=adrve;
				teller2=0;
				while(base) {
					if(view0.lay & base->lay) {
						if(base->f & 1) {

							ai=adrin[teller2++];
							Mat3CpyMat3(ai,a);
							parentbase(base,mat,omat);
							Mat3CpyMat4(bmat,mat);
							Mat3Inv(imat,bmat);

							Mat3One(cmat);
							cmat[0][0]= sizex;
							cmat[1][1]= sizey;
							cmat[2][2]= sizez;
							if(G.move2>=4) {
								if(G.move2==5) {

									cmat[0][0]= cmat[2][2]= cmat[1][1]= 1.0;
									cmat[1][0]=sizex-1.0;
								}
								
								/* eerst voor de basevecs: */
								Mat3MulSerie(vmat, persmat, cmat, persinv, 0);
								
								Mat3MulSerie(pmat, bmat, persmat, cmat, persinv, imat, 0);
								Mat3CpyMat3(cmat, pmat);
								
							} else {
								/* eerst voor de basevecs: */
								Mat3CpyMat3(vmat,cmat);
	
								Mat3MulMat3(pmat,cmat,bmat);
								Mat3MulMat3(cmat,imat,pmat);
							}

							Mat3MulMat3(ai,a,cmat);

							if(teller>1) {
								if( PARENT(base) && G.hie) {
									VECCOPY(vec1,v);
									vec1[0]= sizex*(vec1[0]-vec[0])+vec[0]-v[0];
									vec1[1]= sizey*(vec1[1]-vec[1])+vec[1]-v[1];
									vec1[2]= sizez*(vec1[2]-vec[2])+vec[2]-v[2];
								
									Mat3Inv(imat,omat);
									Mat3MulVec(imat,vec1);
									base->o[0]= v[3]+vec1[0];
									base->o[1]= v[4]+vec1[1];
									base->o[2]= v[5]+vec1[2];
								} else {
									base->v[0]= v[0]-vec[0];
									base->v[1]= v[1]-vec[1];
									base->v[2]= v[2]-vec[2];
									Mat3MulVec(vmat,base->v);
									base->v[0]+= vec[0];
									base->v[1]+= vec[1];
									base->v[2]+= vec[2];

									/* base->v[0]= sizex*(v[0]-vec[0])+vec[0]; */
									/* base->v[1]= sizey*(v[1]-vec[1])+vec[1]; */
									/* base->v[2]= sizez*(v[2]-vec[2])+vec[2]; */
								}
							}
							a+=9; 
							v+=6;
						}
					}
					base=base->next;
				}
				projektie();
				if(teller>1) helpline(vec);

				initprintoverdraw();
				sprintf(s,"Size X: %3.3f",sizex); 
				cmov2i(10,204); 
				charstr(s);
				sprintf(s,"Size Y: %3.3f",sizey); 
				cmov2i(160,204); 
				charstr(s);
				sprintf(s,"Size Z: %3.3f",sizez); 
				cmov2i(310,204); 
				charstr(s);
				if(G.move2==4) {
					sprintf(s,"SizeView");
					cmov2i(460,204); 
					charstr(s);
				} else if(G.move2==5) {
					sprintf(s,"Shear");
					cmov2i(460,204); 
					charstr(s);
				}
				endprintoverdraw();
			}
			else sginap(2);

			sizexo=sizex; 
			sizeyo=sizey; 
			sizezo=sizez;

			if( qtest() ) {
				toets= traces_qread(&val);
				if(val) {
					if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
					else if(toets==MIDDLEMOUSE) {
						midtog= ~midtog;
						if(midtog) proj= testproj(xm,ym,mval);
					}
					else if(toets==XKEY) xref= -xref;
					else if(toets==YKEY) yref= -yref;
					arrowsmovecursor(toets);
				}
				if(toets==INPUTCHANGE) G.winakt= val;
				toets=0;
				sizexo-= 1.0;
			}
			
		}
		if(toets==ESCKEY) {
			base=G.firstbase;
			a=adror; 
			v=adrve;
			teller2=0;
			while(base) {
				if(view0.lay & base->lay) {
					if(base->f & 1) {
						ai=adrin[teller2++];
						if( PARENT(base) ) {
							VECCOPY(base->o,v+3);
						}
						else {
							VECCOPY(base->v,v);
						}
						Mat3CpyMat3(ai,a);
						a+=9; 
						v+=6;
						if(base->skelnr) calc_deform(base);
					}
				}
				base=base->next;
			}
		}
		freeN(adror); 
		freeN(adrve); 
		freeN(adrin);
		G.moving=0;
		clearprintoverdraw();
		clearbaseflags_for_editing();
		projektie();
	}
	G.moving=0;
}

void basebend()
{
	;	/* wordt vanuit toets.c aangeroepen */
}

void ipomouse(anim)
long anim;
{
	/* neemt huidige ipo en zet deze op de muiseco (l-r) */
	/* bezier pointer wordt vervangen door rechte lijn */
	
	struct Bezier *new, *orig, **origpp=0;
	double *mousedata, *md;	/* GGems wil doubles */
	float fac, *data, minx, maxx, sizex, sizey;
	long a, count=0, maxcount=5000, record=0, len, temp1, temp2, onceround=0;
	short mval[2], toets, val, oldval[2];
	char s[50];
	
	if(G.mainb!=7) return;
	
	if(anim) {
		temp1=G.cfra;
		G.cfra= G.sfra;
		temp2=G.hie;
		G.hie=2;
		swapinterval(2);
	}

	if(G.basact && G.ebase==0) {
		G.f |= 128;
			
		give_curipo_pp(&origpp);
		
		if(origpp) {
			new= callocN(sizeof(struct Bezier)+36*2, "ipomouse");
			new->cp= 2;
			orig= *origpp;
			*origpp= new;
			
			data= (float *)(new+1);
			data[3]= IPOMINX;
			data[5]= 0.0;
			data[6]= data[3];
			data[8]= 0.0;
			data[12]= IPOMAXX;
			data[14]= 0.0;
			data[9]= data[12];
			data[11]= 0.0;
			
			oldval[0]= oldval[1]= 10000;
			
			md=mousedata= mallocN(2*8*maxcount, "ipomouse2");
			
			while(TRUE) {
				getmouseco(mval);
				
				if(record || oldval[0]!=mval[0] || oldval[1]!=mval[1]) {
				
					fac= mval[0]/(float)S.vxw;
					data[4]= data[7]= data[10]= data[13]= IPOMINY+ fac*(IPOMAXY-IPOMINY);
					
					initprintoverdraw();
					if(record) {
						if(anim) sprintf(s,"Ipo Mouse: %.3f Record: %d Frame: %d", fac, count, G.cfra);
						else sprintf(s,"Ipo Mouse: %.3f Record: %d", fac, count);
					}
					else {
						if(anim) sprintf(s,"Ipo Mouse: %.3f Frame: %d", fac, G.cfra);
						else sprintf(s,"Ipo Mouse: %.3f", fac);
					}
					cmov2i(10,204);
					charstr(s);
					endprintoverdraw();			
	
					if(anim) loadkeyposall();
					else loadkeypostype(G.ipomode, G.basact, G.basact);
					
					makeDispList(G.basact);		/* ook berekenpad */
					projektie();
					
					oldval[0]= mval[0];
					oldval[1]= mval[1];
					
					if(record) {
						md[0]= count;
						md[1]= mval[0];
						md+= 2;
						if(anim) {
							G.cfra++;
							if(G.cfra>G.efra) {
								G.cfra= G.sfra;
								count= 0;
								md= mousedata;
								onceround= 1;
							}
							else count++;
						}
						else {
							count++;
						}
						if(count>=maxcount) count--;
					}
				}
				else sginap(2);
				
				if( qtest() ) {
					toets= traces_qread(&val);
					if(val) {
						if(toets==ESCKEY || toets==LEFTMOUSE || toets==SPACEKEY) break;
					}
					if(toets==LEFTCTRLKEY || toets==RIGHTCTRLKEY) record= val;
				}
				
				if(record) {	/* af en toe raakt event verloren */
					if( getbutton(LEFTCTRLKEY) || getbutton(RIGHTCTRLKEY) );
					else record= 0;
				}
			}
			clearprintoverdraw();
			
			freeN(new);

			if(anim && onceround) {
				count= G.efra-G.sfra+1;
			}

			if(count>1) {
				/* normaliseren x (tijd) coordinaten */
				minx= 1.0e10;
				maxx= -1.0e10;
				md= mousedata;
				for(a=0; a<count; a++, md+=2) {
					if(minx>md[0]) minx= md[0];
					if(maxx<md[0]) maxx= md[0];
				}
				sizex= maxx-minx;
				sizex= (IPOMAXX-IPOMINX)/sizex;
				sizey= (IPOMAXY-IPOMINY)/(float)S.x;
				
				md= mousedata;
				for(a=0; a<count; a++, md+=2) {
					md[0]= IPOMINX+(md[0]-minx)*sizex;
					md[1]= IPOMINY+(md[1])*sizey;
				}
				
				FitCurve(mousedata, count, 3.0, 0, &new);
				if(orig) freeN(orig);
				*origpp= new;
				tekenspline2d(1);
	
			}
			else {
				*origpp= orig;
			}
			
			freeN(mousedata);			
		}
		G.f &= ~128;
		loadkeypostype(G.ipomode, G.basact, G.basact);
		makeDispList(G.basact);		/* ook berekenpad */
	}

	if(anim) {
		G.hie=temp2;
		G.cfra=temp1;
		swapinterval(1);
	}

	projektie();	/* ook om lijntjes te wissen */
}



