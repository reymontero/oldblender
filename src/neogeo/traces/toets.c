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

/*	toets.c	*/




/*	persptoetsen(event)
	mainlus()
	writeTlog();
*/

#include <stdlib.h>
#include <gl/gl.h>
#include <gl/device.h>
#include "/usr/people/include/Trace.h"
#include "/usr/people/frank/source/iff.h"
#include "/usr/people/include/Button.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/syssgi.h>

extern short backbufok;
short keycode[100], cursortype=0;

/* Beveiliging ongedaangemaakt (frank) */

/* 
#undef LEFTCTRLKEY	
#define LEFTCTRLKEY	keycode[1]
#undef RIGHTSHIFTKEY	
#define RIGHTSHIFTKEY	keycode[2]
#undef LEFTSHIFTKEY	
#define LEFTSHIFTKEY	keycode[3]
#undef TABKEY		
#define TABKEY		keycode[4]
#undef RETKEY		
#define RETKEY		keycode[5]
#undef LEFTARROWKEY	
#define LEFTARROWKEY	keycode[6]
#undef DOWNARROWKEY	
#define DOWNARROWKEY	keycode[7]
#undef RIGHTARROWKEY	
#define RIGHTARROWKEY	keycode[8]
#undef UPARROWKEY	
#define UPARROWKEY	keycode[9]
#undef LEFTMOUSE	
#define LEFTMOUSE	keycode[10]
#undef MIDDLEMOUSE	
#define MIDDLEMOUSE	keycode[11]
#undef	F1KEY 		
#define	F1KEY 		keycode[12]
#undef	F2KEY 		
#define	F2KEY 		keycode[13]
#undef	F3KEY 		
#define	F3KEY 		keycode[14]
#undef	F4KEY 		
#define	F4KEY 		keycode[15]
#undef	F5KEY 		
#define	F5KEY 		keycode[16]
#undef	F6KEY 		
#define	F6KEY 		keycode[17]
#undef	F7KEY 		
#define	F7KEY 		keycode[18]
#undef	F8KEY 		
#define	F8KEY 		keycode[19]
#undef	F9KEY 		
#define	F9KEY 		keycode[20]
#undef	F10KEY		
#define	F10KEY		keycode[21]
#undef	F11KEY		
#define	F11KEY		keycode[22]
#undef	F12KEY		
#define	F12KEY		keycode[23]

#undef AKEY		
#define AKEY		keycode[24]
#undef BKEY		
#define BKEY		keycode[25]	
#undef CKEY		
#define CKEY		keycode[26]	
#undef DKEY		
#define DKEY		keycode[27]	
#undef EKEY		
#define EKEY		keycode[28]	
#undef FKEY		
#define FKEY		keycode[29]
#undef GKEY		
#define GKEY		keycode[30]
#undef HKEY		
#define HKEY		keycode[31]
#undef IKEY		
#define IKEY		keycode[32]
#undef JKEY		
#define JKEY		keycode[33]
#undef KKEY		
#define KKEY		keycode[34]
#undef LKEY		
#define LKEY		keycode[35]
#undef MKEY		
#define MKEY		keycode[36]
#undef NKEY		
#define NKEY		keycode[37]
#undef OKEY		
#define OKEY		keycode[38]
#undef PKEY		
#define PKEY		keycode[39]
#undef QKEY		
#define QKEY		keycode[40]
#undef RKEY		
#define RKEY		keycode[41]
#undef SKEY		
#define SKEY		keycode[42]
#undef TKEY		
#define TKEY		keycode[43]
#undef UKEY		
#define UKEY		keycode[44]
#undef VKEY		
#define VKEY		keycode[45]
#undef WKEY		
#define WKEY		keycode[46]
#undef XKEY		
#define XKEY		keycode[47]
#undef YKEY		
#define YKEY		keycode[48]
#undef ZKEY		
#define ZKEY		keycode[49]
#undef ZEROKEY		
#define ZEROKEY		keycode[50]
#undef ONEKEY		
#define ONEKEY		keycode[51]
#undef TWOKEY		
#define TWOKEY		keycode[52]
#undef THREEKEY	
#define THREEKEY	keycode[53]
#undef FOURKEY		
#define FOURKEY		keycode[54]
#undef FIVEKEY		
#define FIVEKEY		keycode[55]
#undef SIXKEY		
#define SIXKEY		keycode[56]
#undef SEVENKEY	
#define SEVENKEY	keycode[57]
#undef EIGHTKEY	
#define EIGHTKEY	keycode[58]
#undef NINEKEY		
#define NINEKEY		keycode[59]
#undef PAD0		
#define PAD0		keycode[60]
#undef PAD1		
#define PAD1		keycode[61]
#undef PAD2		
#define PAD2		keycode[62]
#undef PAD3		
#define PAD3		keycode[63]
#undef PAD4		
#define PAD4		keycode[64]
#undef PAD5		
#define PAD5		keycode[65]
#undef PAD6		
#define PAD6		keycode[66]
#undef PAD7		
#define PAD7		keycode[67]
#undef PAD8		
#define PAD8		keycode[68]
#undef PAD9		
#define PAD9		keycode[69]
#undef LEFTALTKEY 	
#define LEFTALTKEY 	keycode[70]
#undef	RIGHTALTKEY 	
#define	RIGHTALTKEY 	keycode[71]
#undef	RIGHTCTRLKEY 	
#define	RIGHTCTRLKEY 	keycode[72]
#undef	PAUSEKEY	
#define	PAUSEKEY	keycode[73]
#undef	INSERTKEY	
#define	INSERTKEY	keycode[74]
#undef	HOMEKEY 	
#define	HOMEKEY 	keycode[75]
#undef	PAGEUPKEY 	
#define	PAGEUPKEY 	keycode[76]
#undef	ENDKEY		
#define	ENDKEY		keycode[77]
#undef	PAGEDOWNKEY	
#define	PAGEDOWNKEY	keycode[78]
#undef	NUMLOCKKEY	
#define	NUMLOCKKEY	keycode[79]
#undef	PADVIRGULEKEY 	
#define	PADVIRGULEKEY 	keycode[80]
#undef PADASTERKEY 	
#define PADASTERKEY 	keycode[81]
#undef PADPLUSKEY 	
#define PADPLUSKEY 	keycode[82]

*/

/* #undef SPACEKEY 	 */
/* #define SPACEKEY 	keycode[83] */

extern float mat1[4][4];


void persptoetsen(event)
short event;
{
	float zoom,z,dx=0,dy=0;
	static long perspo=1;
	long vec[3],ok=0;
	Device mdev[2];
	short mval[2],rt2,rt3;

	mdev[0]= MOUSEX;
	mdev[1]= MOUSEY;
	getdev(2,mdev,mval);
	mval[1]-= S.vys;

	if(mval[1]<0 && G.mainb==7) {
		if(event==PADPLUSKEY) {
			ipozoom(1);
			return;
		}
		else if(event==PADMINUS) {
			ipozoom(0);
			return;
		}
	}

	if(event==PADENTER) {
		draw_textuspace_ext();
		return;
	}

	if(view0.persp==3) {
		/* pixelview */
		if(event==PADPLUSKEY) {
			ok=1;
			view0.rt1++;
		}
		if(event==PADMINUS) {
			ok=1;
			view0.rt1--;
		}
		if(view0.rt1<0) view0.rt1=0;
		else if(view0.rt1>12) view0.rt1=12;
		if(G.qual & 3) {
			zoom= 0.5+0.5*(float)(2<<view0.rt1);
			if(event==PAD6) {
				ok=1;
				view0.rt2+= 320/zoom;
			}
			if(event==PAD4) {
				ok=1;
				view0.rt2-= 320/zoom;
			}
			if(event==PAD8) {
				ok=1;
				view0.rt3+= 200/zoom;
			}
			if(event==PAD2) {
				ok=1;
				view0.rt3-= 200/zoom;
			}
			if(view0.rt2<-320) view0.rt2= -320;
			if(view0.rt2> 320) view0.rt2=  320;
			if(view0.rt3<-250) view0.rt3= -250;
			if(view0.rt3> 250) view0.rt3=  250;
		}
		if(ok) {
			projektie();
			return;
		}
	}
	if(G.qual & 3) {
		if(view0.persp==2 && (event==PADPLUSKEY || event==PADMINUS) ) {
			vec[0]= view0.tar[0]-view0.obs[0];
			vec[1]= view0.tar[1]-view0.obs[1];
			vec[2]= view0.tar[2]-view0.obs[2];
			if(G.qual & 1) z= .1;
			else z= .01;
			if(event==PADMINUS) z= -z;
			view0.obs[0]+= z*vec[0];
			view0.obs[1]+= z*vec[1];
			view0.obs[2]+= z*vec[2];
		}
		else if(event==PAD0) {
			view0.persp=3;
			rt2= mval[0]-640;
			rt3= mval[1]-400;
			if(rt2<-320 || rt2>320 || rt3<-250 || rt3>250);
			else {
				view0.rt2= rt2;
				view0.rt3= rt3;
			}
		}
		else if(event==PAD7) {
			view0.theta=0;
			view0.phi=PI;
			view0.view=7;
		}
		else if(event==PAD1) {
			view0.theta=PI;
			view0.phi=0.5*PI;
			view0.view=1;
		}
		else if(event==PAD3) {
			view0.theta= -0.5*PI;
			view0.phi=0.5*PI;
			view0.view=3;
		}
		else {
			if(event) z=64*initgrabz(0,0,0);
			if(event==PAD6) {
				dx= -z/640;
			} else if(event==PAD4) {
				dx= z/640;
			} else if(event==PAD8) {
				dy= -z/400;
			} else if(event==PAD2) {
				dy= z/400;
			}
			if(dx!=0 || dy!=0) {
				view0.ofs[0]+= (G.persinv[0][0]*dx + G.persinv[1][0]*dy);
				view0.ofs[1]+= (G.persinv[0][1]*dx + G.persinv[1][1]*dy);
				view0.ofs[2]+= (G.persinv[0][2]*dx + G.persinv[1][2]*dy);
			}
		}
	}
	else {
		if(view0.persp>=2 && event!=PAD9) view0.persp= perspo;

		if(event==PAD7) {
			view0.theta=0;
			view0.phi=0;
			view0.view=7;
		} else if(event==PAD1) {
			view0.theta=0;
			view0.phi=0.5*PI;
			view0.view=1;
		} else if(event==PAD3) {
			view0.theta= 0.5*PI;
			view0.phi=0.5*PI;
			view0.view=3;
		} else if(event==PAD6) {
			view0.theta+= PI/36;
			view0.view=0;
		} else if(event==PAD4) {
			view0.theta-= PI/36;
			view0.view=0;
		} else if(event==PAD2) {
			view0.phi+= PI/36;
			view0.view=0;
		} else if(event==PAD8) {
			view0.phi-= PI/36;
			view0.view=0;
		} else if(event==PADMINUS) {
			/* deze min en max staan ook in viewmove() */
			if(view0.dproj<500) view0.dproj*=1.2;
		} else if(event==PADPLUSKEY) {
			if(view0.dproj>.0001) view0.dproj*=.83333;
		} else if(event==PAD5) {
			if(view0.persp==1) view0.persp=0;
			else view0.persp=1;
		} else if(event==PAD0) {
			view0.persp=2;
			view0.view= 0;
			if(G.basact && (G.basact->f & 1) ) {
				if(G.basact->soort==4) {
					view0.cambase= G.basact;
				}
				else if(G.qual & 48) view0.cambase= G.basact;
			}
		} else if(event==PAD9) {
			countall();
			loadkeyposall();
		}

		if(view0.persp==0) {
			if(view0.theta==0.0 && view0.phi==0.0) view0.view=7;
			if(view0.theta==0.0 && view0.phi==0.5*PI) view0.view=1;
			if(view0.theta==0.5*PI && view0.phi==0.5*PI) view0.view=3;
		}
		if(view0.persp<2) perspo= view0.persp;
	}
	projektie();
}


void writeTlog()
{
	long file;
	char name[100],*home;

	home = getenv("HOME");
	if (home){
		strcpy(name,home);
		strcat(name,"/.Tlog");
		file=open(name, O_WRONLY | O_CREAT | O_TRUNC);
		if (file >= 0){
			fchmod(file,0664);
			write(file,G.sce,strlen(G.sce));
			close(file);
		} else perror("$HOME/.Tlog");
	} else perror("$HOME");
}


short traces_qread(val)
short *val;
{
	/* zet vlaggen in G.qual */
	short event;
	
	event= qread(val);
	
	if(event==RIGHTSHIFTKEY) {
		if(*val) G.qual |=1;
		else G.qual &= ~1;
	}
	else if(event==LEFTSHIFTKEY) {
		if(*val) G.qual |=2;
		else G.qual &= ~2;
	}
	else if(event==RIGHTALTKEY) {
		if(*val) G.qual |=4;
		else G.qual &= ~4;
	}
	else if(event==LEFTALTKEY) {
		if(*val) G.qual |=8;
		else G.qual &= ~8;
	}
	else if(event==RIGHTCTRLKEY) {
		if(*val) G.qual |=16;
		else G.qual &= ~16;
	}
	else if(event==LEFTCTRLKEY) {
		if(*val) G.qual |=32;
		else G.qual &= ~32;
	}
	else if(event==INPUTCHANGE || event==DRAWOVERLAY) {
			/* overlay: waarschijnlijk was dit windowmenu ALT/RIGHTMOUSE */
		G.qual= 0;
		/* if(getbutton(RIGHTSHIFTKEY)) G.qual +=1; */
		/* if(getbutton(LEFTSHIFTKEY)) G.qual +=2; */
		/* if(getbutton(RIGHTALTKEY)) G.qual +=4; */
		/* if(getbutton(LEFTALTKEY)) G.qual +=8; */
		/* if(getbutton(RIGHTCTRLKEY)) G.qual +=16; */
		/* if(getbutton(LEFTCTRLKEY)) G.qual +=32; */
	}

	return event;
}

void toets(event, val, qual)
short event, val, qual;
{
	static long poptog= 0;
	struct Base *base;
	struct MoData *mo;
	struct Bezier *bez;
	struct ImBuf *ibuf;
	Device mdev[2];
	short mval[2], button(),countkeys();
	short a, x, y;
	char string[40],dir[100],name[100];

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;

	if( event==LEFTARROWKEY || event==DOWNARROWKEY) {
		if(event==DOWNARROWKEY) G.cfra-=10;
		else G.cfra--;
		if(qual & 3) G.cfra=G.sfra;
		if(G.cfra<1) G.cfra=1;
		winset(G.winar[1]);
		frontbuffer(1);
		backbuffer(1);
		SetButs(103,103);
		loadkeyposall();
		if(G.script) doscript();
		if(G.mainb==7) tekenspline2d(0);
		projektie();
	
	} else if( event==RIGHTARROWKEY || event==UPARROWKEY) {
		if(event==UPARROWKEY) G.cfra+=10;
		else G.cfra++;
		if(qual & 3) G.cfra=G.efra;
		if(G.cfra>G.efra) G.cfra=G.efra;
		winset(G.winar[1]);
		frontbuffer(1);
		backbuffer(1);
		SetButs(103,103);
		loadkeyposall();
		if(G.script) doscript();
		if(G.mainb==7) tekenspline2d(0);
		projektie();
	
	} else if( event==PAGEDOWNKEY || event==PAGEUPKEY) {
		a=countkeys();
		if(a!=0) {
			if(G.ebase) {
				error("Not in EditBase mode");
				return;
			}
			if(event==PAGEUPKEY) G.actkey+=1;
			else G.actkey-=1;
			if(G.actkey<0) G.actkey=a;
			else if(G.actkey>a) G.actkey=0;
			projektie();
			if(G.mainb==7) tekenspline2d(0);
		}
	} else if( event==TABKEY) {
		if(G.basact && (G.basact->lay & view0.lay)) {
			if(G.ebase==0) enter_ebasemode(G.basact);
			else exit_ebasemode();
		}
		else if(G.ebase) exit_ebasemode();
	
/* LETTERS */
	} else if(event==AKEY) {
		if(qual & 3){
			tbox_setmain(2);
			toolbox();
		}
		else if(qual & 12) animfast();
		else if(qual & 48) apply();
		else deselectall();
	
	} else if(event==BKEY) {
		if(qual & 3) selector(1); /* render border */
		else selector(0);	/* select border */
	
	} else if(event==CKEY) {
		if(qual & 3) {
			float min[3], max[3], afm[3];
		
			min[0]= min[1]= min[2]= 0;	/* ook origin in beeld */
			max[0]= max[1]= max[2]= 0;

			base= G.firstbase;
			while(base) {
				if(base->lay & view0.lay) minmaxbase(base, min, max);
				base= base->next;
			}
			VecSubf(afm, max, min);
			view0.ofs[0]= -(min[0]+max[0])/2.0;
			view0.ofs[1]= -(min[1]+max[1])/2.0;
			view0.ofs[2]= -(min[2]+max[2])/2.0;
			afm[0]= MAX3(afm[0], afm[1], afm[2]);
			if(afm[0]>0.0) view0.dproj= afm[0]/22000.0;
			view0.muis[0]= 0;
			view0.muis[1]= 0;
			view0.muis[2]= 0;
			if(view0.persp>1) view0.persp= 1;
		}
		else if(qual & 48) copymenu();
		else if(qual & 12) convertmenu();
		else if(G.ebase!=0 && ELEM3(G.ebase->soort, 5, 11, -2) ) {
			makecyclicNurb();
			makeDispList(G.ebase);		/* ook berekenpad */
		} else {
			view0.ofs[0]= -view0.muis[0];
			view0.ofs[1]= -view0.muis[1];
			view0.ofs[2]= -view0.muis[2];
		}
		projektie();
	
	} else if(event==DKEY) {
		if(G.qual & 48) {
			imagestodisplist();
		}
		else if(qual & 3) {
			if(G.ebase) {
				if(G.ebase->soort==1) adduplicateVV();
				else if ELEM3(G.ebase->soort, 5, 11, -2) adduplicateNurb();
				else if(G.ebase->soort== -4) adduplicateIka();
			}
			else adduplicate();
		}
		else {
			if(G.ebase) error("Not in EditBase mode");
			else deletekeypos();
		}
	
	} else if(event==EKEY) {
		if(G.ebase!=0) {
			if(G.ebase->soort==1) extrudeVV();
			else if(G.ebase->soort==5) extrudeNurb();
		}
	
	} else if(event==FKEY) {
		if(qual & 3) fillVV();
		else if(qual & 12) {
			sortfaces();
		}
		else if(qual & 48) fitinWspace();
		else if(G.ebase) {
			if(G.ebase->soort==1) addedgevlak_ebase();
			else if ELEM3(G.ebase->soort, 5, 11, -2) addsegmentNurb();
		}
		else if(view0.cambase && view0.persp==2) fly();
	
	} else if(event==GKEY) {
		grabber();
	
	} else if(event==HKEY) {
		if(winget()==G.winar[1] && G.mainb==7) {
			if(qual & 3) {
				calchandles(G.spline,0);
			}
			else calchandle(G.spline,G.actp2,0);
			schrijfipospline(1);
			tekenspline2d(0);
		}
		else if(G.ebase!=0 && ELEM(G.ebase->soort, -2, 11)) {
			if(qual & 48) autocalchandlesNurb_all(1);	/* flag=1, selected */
			else if(qual & 3) sethandlesNurb(1);
			else sethandlesNurb(3);
			makeDispList(G.ebase);
			projektie();
		}
	
	} else if(event==IKEY) {
		if(qual & 12) {
			schr_inventor();
		}
		else if(qual & 48) {
			ipomouse(G.qual & 3);	/* +shift is met anim */
		}
		else {
			getdev(2, mdev, mval);
			if(winget()==G.winar[1] && G.mainb==7 && mval[0]>640) {
				insertsplinepunt2d();
			} else {
				insertkeypos();
			}
		}
	} else if(event==JKEY) {
		if(G.ebase && G.ebase->soort== -4) makejoint();
		else if(qual & 48) join();
		else if(G.winar[9]) {
			if(R.rectspare==0) R.rectspare= (ulong *)callocN(4*R.rectx*R.recty, "rectot");
			SWAP(ulong *, R.rectspare, R.rectot);
			winset(G.winar[9]);
			lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot);
		}
	
	} else if(event==KKEY) {
		if(G.ebase) {
			if(G.qual & 3) makeskeleton();
			else if(G.ebase->soort== -4) connect_ika(); 
			else if(G.ebase->soort==5) printknots();
		}
	
	} else if(event==LKEY) {
		if(G.ebase) {
			if(G.ebase->soort==1) selectconnected();
			else if(G.ebase->soort== -4) selectconnectedIka();
			else if ELEM3(G.ebase->soort, 5, 11, -2) selectconnectedNurb();
		}
	
	} else if(event==MKEY) {
		if(qual & 12) clearbase(3); /* mat */
		else movetolayer();
	
	} else if(event==NKEY) {
		if(qual & 48 ) recalcnormals();
		else if(qual & 12) {
			winset(G.winar[1]);
			if(poptog & 1) winpop(); else winpush();
			winset(G.winar[0]);
			if(poptog & 1) winpop(); else winpush();
			if(G.winar[9]) {
				winset(G.winar[9]);
				if(poptog & 1) winpop(); else winpush();
			}
			poptog++;
		}
		else {
			persp(0);
			frontbuffer(1);
			backbuffer(0);
			base=G.firstbase;
			while(base) {
				if(base->lay & view0.lay) {
					if(base->sx!= 3200) {
						strcpy(name,base->str);
						if(base->nnr) {
							sprintf(string,".%d",base->nnr);
							strcat(name,string);
						}
						cpack(0x737373);
						cmov2i(base->sx-1,base->sy-1);
						charstr(name);
						cmov2i(base->sx+1,base->sy+1);
						charstr(name);
						if(base->f & 1) cpack(0xFFAAFF);
						else cpack(0);
						cmov2i(base->sx,base->sy);
						charstr(name);
					}
				}
				base=base->next;
			}
			frontbuffer(0);
			backbuffer(1);
			persp(1);
		}
	
	} else if(event==OKEY) {
		if(qual & 3) {
			if(view0.cambase) {
				if(okee("restore Observer")) {
					view0.cambase= 0;
					projektie();
				}
			}
			else {
				if(okee("Observer parent")) {
					if(G.basact && (G.basact->f & 1)) {
						view0.opar=G.basact;
						projektie();
					}
					else view0.opar= 0;
				}
			}
		}
		else if(qual & 12) clearbase(4); /* orig */
		else if(view0.persp<2 && view0.cambase==0) movedinges(view0.obs,1);
	
	} else if(event==PKEY) {
		if(qual & 48) makeparent();
		else if(qual & 12) clearbase(1);
		else if(qual & 3) addstartframe();
		else if(G.ebase) {
			if(G.ebase->soort==1) separateVV();
			else if(G.ebase->soort==5 || G.ebase->soort==11) separateNurb();
		}
	
	} else if(event==QKEY) {
		if(qual & 12) clearbase(5); /* quat */
		else sluit(1);
	
	} else if(event==RKEY) {
		if(qual & 48) renamebase();
		else if(qual & 12) clearbase(5);
		else if(qual & 3) selectrowNurb();
		else roteer();
	
	} else if(event==SKEY) {
		if(qual & 12) {	/* sizeview */
			a= G.move2;
			G.move2=4;
			sizing();
			G.move2=a;
		} else if(qual & 48) {	/* shear */
			a= G.move2;
			G.move2=5;
			sizing();
			G.move2=a;
		}
		else if(qual & 3) {		/* snapmenu */
			getdev(2, mdev, mval);
			
			if(winget()==G.winar[1] && G.mainb==7 && mval[0]>640) {
				bezier_snapmenu();
			} else {
				snapmenu();
			}
		}
		else sizing();
	
	} else if(event==TKEY) {
		if(qual & 3) {
			if(okee("Target parent")) {
				if(G.basact && (G.basact->f & 1)) {
					view0.tpar=G.basact;
					projektie();
				}
				else view0.tpar= 0;
			}
		}
		else if(qual & 48) maketrack();
		else if(qual & 12) clearbase(2);
		else if(view0.cambase==0) movedinges(view0.tar,2);
	
	} else if(event==UKEY) {
		if(G.ebase) remake_ebasedata();
	
	} else if(event==VKEY) {
		if(qual & 48) storeview();
		else if(qual & 12) {
			if (qual & 3) makeimagesize(1);	/* in displist.c */
			else makeimagesize(0);
		} else if(winget()==G.winar[1] && G.mainb==7) {
			if(qual & 3) {
				calchandles(G.spline,1);
			}
			else calchandle(G.spline,G.actp2,1);
			schrijfipospline(1);
			tekenspline2d(0);
		}
		else if(G.ebase!=0 && ELEM(G.ebase->soort, 11, -2)) {
			sethandlesNurb(2);
			makeDispList(G.ebase);
			projektie();
		}
	} else if(event==WKEY) {
		if(qual & 12) {
			schr_videoscape();
		}
		else if(qual & 48) {
			strcpy(dir,G.sce);
			schrijfscene(dir);
			writeTlog();
		}
		else if(G.qual & 3) {
			if(G.ebase) vertbend();
			else basebend();
		}
		else if(G.ebase) printweightsNurb();
		else workfieldbuttons();
	
	} else if(event==XKEY) {
		if(winget()==G.winar[1] && G.mainb==7) {
			delsplinepunt2d();
		} else if(G.ebase) {
			if(G.ebase->soort== 1) delVV();
			else if ELEM3(G.ebase->soort, 5, 11, -2) delNurb();
			else if(G.ebase->soort== -4) delIka();
		} else {
			if(qual & 3) {
				/* wisalles(1); */
				projektie();
			} else if(qual & 48) {
				strcpy(dir,G.scr);
				val=ffileselect("EXECUTE SCRIPT",G.scr);
				if(val) doscript();
			}
			else deleteobj();
		}
	
	} else if(event==YKEY) {
		if(G.ebase) {
			if(G.ebase->soort==1) splitVV();
			else if(G.ebase->soort== -4) splitIka();
		}
	
	} else if(event==ZKEY) {
		if(G.winakt==G.winar[9]) zoomwin();
		else {
			if(G.zbuf) G.zbuf= 0;
			else G.zbuf= 1;
			SetButs(104, 104);
			mainbuts(104);
		}
	}

}

void mainlus()
{
	extern struct BGpic bgpic;
	long file, org[2], size[2], temp, rodeguru;
	Device mdev[2];
	short DoButtons();
	short event, a, x, y, mval[2], lus=1, val;
	char string[40], dir[100], name[100];

	mdev[0] = MOUSEX;
	mdev[1] = MOUSEY;


	while (lus) {
		event= traces_qread(&val);

		if(event==NUMLOCKKEY) {    /* moet hier omdat loslaten telt */
			if(getvaluator(NUMLOCKKEY)) initlocalview();
			else endlocalview();
		}
		else if(val) {

			/* alle keyrepeats eruit mits val==1 */
			if(event<500) {
				while(qtest() && qtest()==event ) {
					traces_qread(&val);
					if(val==0) break;
				}
			}

/*MUIS*/
			if(event==LEFTMOUSE) {
				getdev(2, mdev, mval);

				if(G.winakt==G.winar[0]) {
					muiscursor();
				}
				else if(G.winakt==G.winar[1] && G.mainb==7 && mval[0]>=(S.bxw-40)/2 && mval[1]<S.byw-20) {
					ipomove();
				}
				else {
					event=DoButtons();
					if(event) {
						if(event==66) donaambutton();
						else if(event==67) dobasebutton();
						else if(event==68) dobaselaybutton();
						else if(event==101) projektie();
						else if(event>=100 && event<=110) mainbuts(event);
						else if(event>110 && event<126) layerbuts(event);
						else if(event>150 && event<199) viewbuts(event);
						else if(event>=200 && event<299) objbuts(event);
						else if(event>=300 && event<399) textubuts(event);
						else if(event>=400 && event<499) lampbuts(event);
						else if(event>=600 && event<699) wrldbuts(event);
						else if(event>700 && event<799) movebuts(event);
						else if(event>800 && event<899) displaybuts(event);
						else if(event>900 && event<999) editbuts(event);
					}
				}

			} else if(event==MIDDLEMOUSE) {
				getdev(2, mdev, mval);
				if(winget()==G.winar[0]) viewmove();
				else if(G.mainb==7 && mval[0]>=(S.bxw-40)/2) ipomove();

			} else if(event==RIGHTMOUSE) {
				getdev(2, mdev, mval);
				if(winget()==G.winar[0]) muisselect();
				else if(G.mainb==7 && mval[0]>=S.bxw/2) muisspline2d();
				else windowmenu();

/*F TOETS*/
			} else if(event==F1KEY) {
				strcpy(dir, G.sce);
				if(getbutton(LEFTSHIFTKEY)) {
					val= ffileselect("APPEND SCENE", dir);
					if(val) leesscene(dir,128+28);
				}
				else if(getbutton(LEFTCTRLKEY)) {
					val= ffileselect("LOAD CURRY", dir);
					if(val) leesCurry(dir);
				}
				else {
					val=ffileselect("LOAD SCENE",dir);
					if(val){
						leesscene(dir,0);
						/* writeTlog(); */
					}
				}
				projektie();

			} else if(event==F2KEY) {
				strcpy(dir,G.sce);
				val=ffileselect("SAVE SCENE",dir);
				if(val) {
					schrijfscene(dir);
					writeTlog();
				}
				projektie();

			} else if(event==F3KEY) {
				if(R.rectot) {
					strcpy(dir,G.ima);

					if(R.imtype==0) val=ffileselect("SAVE IRIS",dir);
					else if(R.imtype==1) val=ffileselect("SAVE TARGA",dir);
					else if(R.imtype==2) val=ffileselect("SAVE IFF",dir);
					else if(R.imtype==3) val=ffileselect("SAVE HAMX",dir);
					else if(R.imtype==5) val=ffileselect("SAVE JPEG", dir);
					else val=ffileselect("SAVE FTYPE",dir);

					if(val) {
						setcursorN(2);	/* pas op: doet winset */

						strcpy(G.ima,dir);
						file=open(dir,O_RDONLY);
						close(file);
						if( (file==-1) || saveover(dir) ) {
							ulong * temp;

							G.afbreek= 0;
							temp = mallocN(R.rectx*R.recty*sizeof(long));
							if (temp){
								memcpy(temp,R.rectot,R.rectx*R.recty*sizeof(long));
								schrijfplaatje(dir);
								freeN(R.rectot);
								R.rectot = temp;
							} else G.afbreek = TRUE;

							if(G.afbreek) {
								error("Can't write image");
								G.afbreek= 0;
							}
						}
						setcursorN(cursortype);
					}
				}
				projektie();

			} else if(event==F4KEY) {
				G.mainb=4;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F5KEY) {
				G.mainb=5;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F6KEY) {
				G.mainb=6;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F7KEY) {
				G.mainb=7;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F8KEY) {
				G.mainb=8;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F9KEY) {
				G.mainb=9;
				SetButs(100,100);
				mainbuts(0);

			} else if(event==F10KEY) {
				if(G.qual & 48) teststructs();

			} else if(event==F11KEY) {
				if(G.winar[9]) {
					winclose(G.winar[9]);
					G.winar[9]=0;
				}
				else if(R.rectot) {
					x=R.rectx-1;
					y=R.recty-1;
					winset(G.winar[0]);
					drawmode(OVERDRAW);
					color(0);
					clear();
					drawmode(NORMALDRAW);
					prefposition(R.xof,R.xof+x,R.yof,R.yof+y);
					noborder();
					foreground();
					G.winar[9]=winopen("render");
					RGBmode();
					singlebuffer();
					gconfig();
					winset(G.winar[9]);
					sginap(3);
					lrectwrite(0,0,x,y,R.rectot);
				}
				else error("Can't display");

			} else if(event==F12KEY) {
				initrender(0);

/*BIJZ */
			} else if(event==ESCKEY) {
				if(G.winar[9]) {
					winclose(G.winar[9]);
					G.winar[9]=0;
				} else {
					G.mainb=1;
					SetButs(100,100);
					mainbuts(0);
				}

			} else if( event==PAD0 || event==PAD1 || event==PAD2 ||
			    event==PAD3 || event==PAD4 || event==PAD5 ||
			    event==PAD6 || event==PAD7 || event==PAD8 ||
			    event==PAD9 || event==PAD9 ||
			    event==PADMINUS || event==PADPLUSKEY ||
			    event==PADENTER	) {
				persptoetsen(event);

			} else if(event==ACCENTGRAVEKEY) {
				if(G.localview==0) {
					view0.lay= 32767;
					SetButs(111,126);
					projektie();
				}
			}
			else if(event==ONEKEY) layerbuts(0);
			else if(event==TWOKEY) layerbuts(1);
			else if(event==THREEKEY) layerbuts(2);
			else if(event==FOURKEY) layerbuts(3);
			else if(event==FIVEKEY) layerbuts(4);
			else if(event==SIXKEY) layerbuts(5);
			else if(event==SEVENKEY) layerbuts(6);
			else if(event==EIGHTKEY) layerbuts(7);
			else if(event==NINEKEY) layerbuts(8);
			else if(event==ZEROKEY) layerbuts(9);
			else if(event==MINUSKEY) layerbuts(10);
			else if(event==EQUALKEY) layerbuts(11);

			else if(event==SPACEKEY) toolbox();

/*WINDOW*/
			else if(event==REDRAW) {
				winset(val);
				drawmode(OVERDRAW);
				color(0);
				clear();

				drawmode(NORMALDRAW);
				if(val==G.winar[0]) {
					frontbuffer(0);
					backbuffer(1);
					cpack(0x737373);
					clear();
					bgpic.dproj= 0;
					projektie();
				}
				else if(val==G.winar[1]) {
					
					winset(G.winar[1]);
					mmode(MSINGLE);

					reshapeviewport();
					loadmatrix(mat1);
					ortho2(0, 1279, 0, 223-1);

					tekenmainbuts(0);
				}
				else if(G.winar[9]==val) {
					lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot);
					if(winget()==G.winar[9]) {
						getorigin(org,org+1);
						R.xof=org[0];
						R.yof=org[1];
					}
				}
			} else if(event==INPUTCHANGE) {
				winset(val);
				G.winakt= val;
				if(val==G.winar[0]) {
					frontbuffer(0);
					backbuffer(1);
				} else if(val==G.winar[1]) {
					frontbuffer(1);
					backbuffer(1);
				}
			
			} else if(event==WINFREEZE) {
				if(val!=G.winar[9]) {
					if(G.winar[9]) winclose(G.winar[9]);
					G.winar[9]= 0;
					if(val==G.winar[0]) {
						winclose(G.winar[1]);
						G.winar[1]= 0;
					}
					else if(val==G.winar[1]) {
						winclose(G.winar[0]);
						G.winar[0]= 0;
					}
				}

			} else if(event==WINTHAW) {
				if(G.winar[0]==0) {
					openviewwin();
					projektie();
				}
				if(G.winar[1]==0) {
					openbuttonwin();
					tekenmainbuts(0);
				}
				else {	/* wel de matrix zetten, gaat af en toe mis! */
					winset(G.winar[1]);
					mmode(MSINGLE);		/* dit was de boosdoener! */

					reshapeviewport();
					loadmatrix(mat1);
					ortho2(0, 1279, 0, 223-1);
				}
				
			} else if(event==QREADERROR) {
				qreset();

			} else if(event==QFULL) {
				printf("qfull\n");
				qreset();

			}
			else toets(event, val, G.qual);
			
			

		}

		/* test curstype */
		a=0;
		if(G.ebase) a=1;
		if(a!=cursortype) {
			cursortype= a;
			setcursorN(cursortype);
		}
		if(qtest()==0 && backbufok==0) {
			backbufprojektie(1);
		}

		if(G.winakt) {
			for(a=0;a<10;a++) {
				if(G.winakt==G.winar[a]) winset(G.winakt);
			}
		}
		
		/* kan voorkomen als numlock niet in queue zit: */
		if(G.localview && getvaluator(NUMLOCKKEY)==0) endlocalview();
	}
}

