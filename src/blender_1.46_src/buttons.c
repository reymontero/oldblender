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



/*  buttons.c   aug 94     GRAPHICS
 * 
 *  maart 95
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "edit.h"
#include "render.h"
#include "effect.h"
#include "sector.h"
#include "ika.h"
#include "plugin.h"

short bgpicmode=0, near=1000, far=1000;
short degr= 90, step= 9, turn= 1, editbutflag= 1;
float hspeed=.1, prspeed=0.0, prlen=0.0, doublimit= 0.001;
float extr_offs= 1.0, editbutweight=1.0, editbutsize=0.1, cumapsize= 1.0;

MTex emptytex;
char texstr[15][8]= {"None","Clouds","Wood","Marble","Magic","Blend","Stucci","Noise","Image","Plugin","","","",""};

#define BUTGREY			10
#define BUTGREEN		11
#define BUTBLUE			12
#define BUTSALMON		13
#define BUTPURPLE		14

/* *********************** */
/* *********************** */
#define B_VIEWBUTS		1100

#define B_LOADBGPIC		1001
#define B_BLENDBGPIC	1002
#define B_BGPICBROWSE	1003
#define B_BGPICTEX		1004

/* *********************** */
#define B_LAMPBUTS		1200

#define B_LAMPREDRAW	1101
#define B_COLLAMP		1102
#define B_TEXCLEARLAMP	1103

/* *********************** */
#define B_MATBUTS		1300

#define B_MATCOL		1201
#define B_SPECCOL		1202
#define B_MIRCOL		1203
#define B_ACTCOL		1204
#define B_MATFROM		1205
#define B_MATPRV		1206
#define B_MTEXCOL		1207
#define B_TEXCLEAR		1208
#define B_MATPRV_DRAW	1209
#define B_MTEXPASTE		1210
#define B_MTEXCOPY		1211
#define B_MATLAY		1212

/* *********************** */
#define B_TEXBUTS		1400

#define B_TEXTYPE		1301
#define B_DEFTEXVAR		1302
#define B_LOADTEXIMA	1303
#define B_NAMEIMA		1304
#define B_TEXCHANNEL	1305
#define B_TEXREDR_PRV	1306
#define B_TEXIMABROWSE	1307
#define B_IMAPTEST		1308
#define B_RELOADIMA		1309
#define B_LOADPLUGIN	1310
#define B_NAMEPLUGIN	1311
#define B_COLORBAND		1312
#define B_ADDCOLORBAND	1313
#define B_DELCOLORBAND	1314
#define B_CALCCBAND		1315
#define B_CALCCBAND2	1316
#define B_DOCOLORBAND	1317
#define B_REDRAWCBAND	1318
#define B_BANDCOL		1319
#define B_LOADTEXIMA1	1320

/* *********************** */
#define B_ANIMBUTS		1500

#define B_RECALCPATH	1401
#define B_MUL_IPO		1402
#define B_AUTOTIMEOFS	1403
#define B_FRAMEMAP		1404
#define B_NEWEFFECT		1405
#define B_PREVEFFECT	1406
#define B_NEXTEFFECT	1407
#define B_CHANGEEFFECT	1408
#define B_CALCEFFECT	1409
#define B_DELEFFECT		1410
#define B_RECALCAL		1411
#define B_SETSPEED		1412
#define B_PRINTSPEED	1413
#define B_PRINTLEN		1414

	/* heeft MAX_EFFECT standen! Volgende pas 1450... */
#define B_SELEFFECT	1430	


/* *********************** */
#define B_WORLDBUTS		1600

#define B_TEXCLEARWORLD	1501

/* *********************** */
#define B_RENDERBUTS	1700

#define B_FS_PIC		1601
#define B_FS_BACKBUF	1602

#define B_FS_FTYPE		1604
#define B_DORENDER		1605
#define B_DOANIM		1606
#define B_PLAYANIM		1607
#define B_PR_PAL		1608
#define B_PR_FULL		1609
#define B_PR_PRV		1610
#define B_PR_CDI		1611
#define B_PR_PAL169		1612
#define B_PR_D2MAC		1613
#define B_PR_MPEG		1614
#define B_REDRAWDISP	1615
#define B_SETBROWSE		1616
#define B_CLEARSET		1617
#define B_PR_PRESET		1618
#define B_PR_PANO		1619
#define B_FS_MOVIE		1620
#define B_INFOMOVIE		1621
#define B_IS_FTYPE		1622
#define B_IS_BACKBUF	1623
#define B_PR_PC			1624
#define B_OSA_BLUR1		1625
#define B_OSA_BLUR2		1626
#define B_PR_PANO360	1627
#define B_PR_HALFFIELDS	1628

/* *********************** */
#define B_COMMONEDITBUTS	2049

#define B_MATWICH		2003
#define B_MATNEW		2004
#define B_MATDEL		2005
#define B_MATASS		2006
#define B_MATSEL		2007
#define B_MATDESEL		2008
#define B_HIDE			2009
#define B_REVEAL		2010
#define B_SELSWAP		2011
#define B_SETSMOOTH		2012
#define B_SETSOLID		2013
#define B_AUTOTEX		2014

	/* 32 getallen! */
#define B_OBLAY			2018


#define B_MESHBUTS		2100

#define B_FLIPNORM		2050
#define B_SPIN			2051
#define B_SPINDUP		2052
#define B_EXTR			2053
#define B_SCREW			2054
#define B_EXTREP		2055
#define B_SPLIT			2056
#define B_REMDOUB		2057
#define B_SUBDIV		2058
#define B_FRACSUBDIV	2059
#define B_XSORT			2060
#define B_HASH			2061
#define B_DELSTICKY		2062
#define B_DELVERTCOL	2063
#define B_MAKE_TFACES	2064
#define B_TOSPHERE		2065

/* *********************** */
#define B_CURVEBUTS		2200

#define B_CONVERTPOLY	2101
#define B_CONVERTBEZ	2102
#define B_CONVERTBSPL	2103
#define B_CONVERTCARD	2104
#define B_CONVERTNURB	2105
#define B_UNIFU			2106
#define B_ENDPU			2107
#define B_BEZU			2108
#define B_UNIFV			2109
#define B_ENDPV			2110
#define B_BEZV			2111
#define B_SETWEIGHT		2112
#define B_SETW1			2113
#define B_SETW2			2114
#define B_SETW3			2115
#define B_SETORDER		2116
#define B_MAKEDISP		2117
#define B_SUBDIVCURVE	2118
#define B_SPINNURB		2119
#define B_CU3D			2120
#define B_SETRESOLU		2121
#define B_SETW4			2122


/* *********************** */
#define B_FONTBUTS		2300

#define B_MAKEFONT		2201
#define B_TOUPPER		2202
#define B_SETFONT		2203
#define B_LOADFONT		2204

/* *********************** */
#define B_IKABUTS		2400

#define B_IKASETREF		2301
#define B_IKARECALC		2302

/* *********************** */
#define B_CAMBUTS		2500

/* *********************** */
#define B_MBALLBUTS		2600

#define B_RECALCMBALL	2501

/* *********************** */
#define B_LATTBUTS		2700

#define B_RESIZELAT		2601
#define B_DRAWLAT		2602
#define B_LATTCHANGED	2603

/* *********************** */
#define B_GAMEBUTS		2800

#define B_ADDSENS		2701
#define B_DELSENS		2702
#define B_ADDPORT		2703
#define B_DELPORT		2704
#define B_TESTPORT		2705
#define B_TOTEVENT		2706
#define B_TOTACTION		2707
#define B_TESTNAME		2708
#define B_TESTK2K		2709

	/* (voorlopig?) 2 x 10 vrijhouden */
#define B_COPYVAR		2710
#define B_TESTVAR		2720

#define B_ALLTEXMESH	2730
#define B_ALLDYNAMESH	2731
#define B_LIFELAY		2732

/* *********************** */
#define B_FPAINTBUTS	2900

#define B_VPCOLSLI		2801
#define B_VPGAMMA		2802

#define B_COPY_TF_MODE	2804
#define B_COPY_TF_UV	2805
#define B_COPY_TF_COL	2806
#define B_REDR_3D_IMA	2807
#define B_HALO			2808

#define B_TWOSIDE		2812
#define B_OBCOLOR		2813
#define B_COPY_TF_TEX	2814
#define B_MASK			2815
#define B_SHADOW		2816

#define B_SHOWTEX		2832
#define B_ASSIGNMESH	2833

/* *********************** */
/*  BUTTON BUT: > 4000	   */




void test_obpoin_but(char *name, ID **idpp)
{
	ID *id;
	ListBase *lb;
	
	if(idpp == (ID **)&(emptytex.object)) {
		error("Add texture first");
		*idpp= 0;
		return;
	}
	
	id= G.main->object.first;
	while(id) {
		if(id->lib==0) {
			if( strcmp(name, id->name+2)==0 ) {
				*idpp= id;
				return;
			}
		}
		id= id->next;
	}
	*idpp= 0;
}

void test_meshpoin_but(char *name, ID **idpp)
{
	ID *id;
	ListBase *lb;
	
	if( *idpp ) (*idpp)->us--;
	
	id= G.main->mesh.first;
	while(id) {
		if(id->lib==0) {
			if( strcmp(name, id->name+2)==0 ) {
				*idpp= id;
				id_us_plus(id);
				return;
			}
		}
		id= id->next;
	}
	*idpp= 0;
}

void test_scenepoin_but(char *name, ID **idpp)
{
	ID *id;
	ListBase *lb;
	
	if( *idpp ) (*idpp)->us--;
	
	id= G.main->scene.first;
	while(id) {
		if(id->lib==0) {
			if( strcmp(name, id->name+2)==0 ) {
				*idpp= id;
				id_us_plus(id);
				return;
			}
		}
		id= id->next;
	}
	*idpp= 0;
}



/* ************************************* */

void do_common_editbuts(short event)
{
	EditVlak *evl;
	EditVert *eve;
	Base *base;
	Object *ob;
	Mesh *me;
	Nurb *nu;
	Curve *cu;
	Material *ma;
	MFace *mface;
	BezTriple *bezt;
	BPoint *bp;
	float fac;
	ulong local;
	int a, bit, index= -1;

	switch(event) {
		
	case B_MATWICH:
		if(G.obedit && G.obedit->actcol>0) {
			if ELEM3(G.obedit->type, OB_MESH, OB_LIFE, OB_SECTOR) {
				evl= G.edvl.first;
				while(evl) {
					if( vlakselectedAND(evl, 1) ) {
						if(index== -1) index= evl->mat_nr;
						else if(index!=evl->mat_nr) {
							error("Mixed colors");
							return;
						}
					}
					evl= evl->next;
				}
			}
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) {
				nu= editNurb.first;
				while(nu) {
					if( isNurbsel(nu) ) {
						if(index== -1) index= nu->mat_nr;
						else if(index!=nu->mat_nr) {
							error("Mixed colors");
							return;
						}
					}
					nu= nu->next;
				}				
			}
			if(index>=0) {
				G.obedit->actcol= index+1;
				addqueue(curarea->win, REDRAW, 1);
			}
		}
		break;
	case B_MATNEW:
		new_material_to_objectdata(OBACT);
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_MATDEL:
		delete_material_index();
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_MATASS:
		if(G.obedit && G.obedit->actcol>0) {
			if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) {
				evl= G.edvl.first;
				while(evl) {
					if( vlakselectedAND(evl, 1) )
						evl->mat_nr= G.obedit->actcol-1;
					evl= evl->next;
				}
			}
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) {
				nu= editNurb.first;
				while(nu) {
					if( isNurbsel(nu) )
						nu->mat_nr= G.obedit->actcol-1;
					nu= nu->next;
				}
			}
		}
		break;
	case B_MATSEL:
	case B_MATDESEL:
		if(G.obedit) {
			if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) {
				evl= G.edvl.first;
				while(evl) {
					if(evl->mat_nr== G.obedit->actcol-1) {
						if(event==B_MATSEL) {
							if(evl->v1->h==0) evl->v1->f |= 1;
							if(evl->v2->h==0) evl->v2->f |= 1;
							if(evl->v3->h==0) evl->v3->f |= 1;
							if(evl->v4 && evl->v4->h==0) evl->v4->f |= 1;
						}
						else {
							if(evl->v1->h==0) evl->v1->f &= ~1;
							if(evl->v2->h==0) evl->v2->f &= ~1;
							if(evl->v3->h==0) evl->v3->f &= ~1;
							if(evl->v4 && evl->v4->h==0) evl->v4->f &= ~1;
						}
					}
					evl= evl->next;
				}
				tekenvertices_ext( event==B_MATSEL );
			}
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) {
				nu= editNurb.first;
				while(nu) {
					if(nu->mat_nr==G.obedit->actcol-1) {
						if(nu->bezt) {
							a= nu->pntsu;
							bezt= nu->bezt;
							while(a--) {
								if(bezt->hide==0) {
									if(event==B_MATSEL) {
										bezt->f1 |= 1;
										bezt->f2 |= 1;
										bezt->f3 |= 1;
									}
									else {
										bezt->f1 &= ~1;
										bezt->f2 &= ~1;
										bezt->f3 &= ~1;
									}
								}
								bezt++;
							}
						}
						else if(nu->bp) {
							a= nu->pntsu*nu->pntsv;
							bp= nu->bp;
							while(a--) {
								if(bp->hide==0) {
									if(event==B_MATSEL) bp->f1 |= 1;
									else bp->f1 &= ~1;
								}
								bp++;
							}
						}
					}
					nu= nu->next;
				}
				allqueue(REDRAWVIEW3D, 0);
			}
		}
		break;
	case B_HIDE:
		if(G.obedit) {
			if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) hide_mesh(0);
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) hideNurb();
		}
		break;
	case B_REVEAL:
		if(G.obedit) {
			if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) reveal_mesh();
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) revealNurb();
		}
		else if(G.f & G_FACESELECT) reveal_tface();
		
		break;
	case B_SELSWAP:
		if(G.obedit) {
			if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) selectswap_mesh();
			else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) selectswapNurb();
		}
		break;
	case B_AUTOTEX:
		ob= OBACT;
		if(ob && G.obedit==0) {
			if(ob->type==OB_MESH) tex_space_mesh(ob->data);
			else if(ob->type==OB_MBALL) ;
			else tex_space_curve(ob->data);
		}
		break;
	case B_SETSMOOTH:
	case B_SETSOLID:
		if(G.obedit) {
			if(G.obedit->type == OB_MESH) {
				evl= G.edvl.first;
				while(evl) {
					if( vlakselectedAND(evl, 1) ) {
						if(event==B_SETSMOOTH) evl->flag |= ME_SMOOTH;
						else evl->flag &= ~ME_SMOOTH;
					}
					evl= evl->next;
				}
			}
			else {
				nu= editNurb.first;
				while(nu) {
					if(isNurbsel(nu)) {
						if(event==B_SETSMOOTH) nu->flag |= ME_SMOOTH;
						else nu->flag &= ~ME_SMOOTH;
					}
					nu= nu->next;
				}
				
			}
		}
		else {
			base= FIRSTBASE;
			while(base) {
				if(TESTBASELIB(base)) {
					if(base->object->type==OB_MESH) {
						me= base->object->data;
						mface= me->mface;
						for(a=0; a<me->totface; a++, mface++) {
							if(event==B_SETSMOOTH) mface->flag |= ME_SMOOTH;
							else mface->flag &= ~ME_SMOOTH;
						}
					}
					else if ELEM(base->object->type, OB_SURF, OB_CURVE) {
						cu= base->object->data;
						nu= cu->nurb.first;
						while(nu) {
							if(event==B_SETSMOOTH) nu->flag |= ME_SMOOTH;
							else nu->flag &= ~ME_SMOOTH;
							nu= nu->next;
						}
					}
				}
				base= base->next;
			}
			
		}
		break;

	default:
		if(event>=B_OBLAY && event<=B_OBLAY+31) {
			local= BASACT->lay & 0xFF000000;
			BASACT->lay -= local;
			if(BASACT->lay==0 || (G.qual & LR_SHIFTKEY)==0) {
				bit= event-B_OBLAY;
				BASACT->lay= 1<<bit;
				addqueue(curarea->win, REDRAW, 1);
			}
			BASACT->lay += local;
			/* optimale redraw */
			if( (OBACT->lay & G.vd->lay) && (BASACT->lay & G.vd->lay) );
			else if( (OBACT->lay & G.vd->lay)==0 && (BASACT->lay & G.vd->lay)==0 );
			else allqueue(REDRAWVIEW3D, 0);
			
			OBACT->lay= BASACT->lay;
		}
	}

}

void common_editbuts()
{
	Object *ob;
	ID *id;
	Material *ma;
	void *poin;
	float min;
	int xco, a, dx, dy;
	char str[32];
	
	ob= OBACT;
	if(ob==0) return;
	
	/* LAYERS */
	xco= 291;
	dx= 32;
	dy= 30;
	for(a=0; a<10; a++) {
		DefBut(TOG|INT|BIT|a+10, B_OBLAY+a+10, "",	xco+a*(dx/2), 180, dx/2, dy/2, &(BASACT->lay));
		DefBut(TOG|INT|BIT|a, B_OBLAY+a, "",	xco+a*(dx/2), 180+dy/2, dx/2, 1+dy/2, &(BASACT->lay));
		if(a==4) xco+= 5;
	}

	id= ob->data;
	if(id && id->lib) SetButLock(1);

	SetButCol(BUTGREY);
	DefBut(LABEL, 0, "Drawtype", 28,200,100,18, 0, 0, 0);
	DefBut(MENU|CHA, REDRAWVIEW3D, "Drawtype%t|Bbox %x1|Wire %x2|Solid %x3|Shaded %x4",	28,180,100,18, &ob->dt);
	DefBut(LABEL, 0, "Draw Extra",						28,160,100,18, 0, 0, 0);
	DefBut(TOG|CHA|BIT|0, REDRAWVIEW3D, "Boundbox",		28, 140, 100, 18, &ob->dtx);
	DefBut(TOG|CHA|BIT|1, REDRAWVIEW3D, "Axis",			28, 120, 100, 18, &ob->dtx);
	DefBut(TOG|CHA|BIT|2, REDRAWVIEW3D, "TexSpace",		28, 100, 100, 18, &ob->dtx);
	DefBut(TOG|CHA|BIT|3, REDRAWVIEW3D, "Name",			28, 80, 100, 18, &ob->dtx);
	DefBut(TOG|CHA|BIT|4, REDRAWVIEW3D, "Image",		28, 60, 100, 18, &ob->dtx);
	

	SetButCol(BUTSALMON);
	SetButFunc(docentre);
	DefBut(BUT,0,	"Do Centre",	28,37,100,20);
	SetButFunc(docentre_new);
	DefBut(BUT,0, "Centre New",	28,15,100,20);
	SetButCol(BUTGREY);
	
	/* material en select swap en hide */
	if ELEM7(ob->type, OB_MESH, OB_SECTOR, OB_LIFE, OB_CURVE, OB_SURF, OB_FONT, OB_MBALL) {
		
		if(ob->type!=OB_SECTOR && ob->type!=OB_LIFE) {
			if(ob->type==OB_MESH) poin= &( ((Mesh *)ob->data)->texflag );
			else if(ob->type==OB_MBALL) poin= &( ((MetaBall *)ob->data)->texflag );
			else poin= &( ((Curve *)ob->data)->texflag );
			DefBut(TOG|INT|BIT|0, B_AUTOTEX, "AutoTexSpace",	143,180,130,19, poin);
		}
		sprintf(str,"%d Mat:", ob->totcol);
		if(ob->totcol) min= 1.0; else min= 0.0;
		ma= give_current_material(ob, ob->actcol);
		
		if(ma) {
			DefBut(COL|FLO, 0, "",			291,123,24,30, &(ma->r));
			DefBut(LABEL, 0, ma->id.name+2, 318,146, 103, 30);
		}
		DefBut(NUM|CHA, B_REDR,	str,		318,123,103,30, &ob->actcol, min, (float)(ob->totcol));
		DefBut(BUT,B_MATWICH,	"?",		423,123,31,30);
		
		SetButCol(BUTSALMON);
		DefBut(BUT,B_MATNEW,	"New",		292,101,80,21);
		DefBut(BUT,B_MATDEL,	"Delete",	374,101,80,21);
		DefBut(BUT,B_MATASS,	"Assign",	291,47,162,26);

		SetButCol(BUTGREY);
		DefBut(BUT,B_MATSEL,	"Select",	292,76,79,22);
		DefBut(BUT,B_MATDESEL,	"Deselect",	373,76,79,21);
		
		if(ob->type!=OB_FONT) {
			DefBut(BUT,B_HIDE,		"Hide",		1091,152,77,18);
			DefBut(BUT,B_REVEAL,	"Reveal",	1171,152,86,18);
			DefBut(BUT,B_SELSWAP,	"Select Swap",	1091,129,166,18);
		}
		DefBut(BUT,B_SETSMOOTH,	"Set Smooth",	291,15,80,20);
		DefBut(BUT,B_SETSOLID,	"Set Solid",	373,15,80,20);

	}
	
}



/* *************************** MESH ******************************** */


void do_meshbuts(short event)
{
	Object *ob;
	Mesh *me;
	EditVlak *evl;
	EditVert *eve;
	float fac;
	int a;
	short randfac;
	char str[32];

	ob= OBACT;
	if(ob && (ob->type==OB_MESH || ob->type==OB_SECTOR || ob->type==OB_LIFE)) {
		
		me= get_mesh(ob);
		if(me==0) return;
		
		switch(event) {
		case B_DELSTICKY:
		
			if(me->msticky) freeN(me->msticky);
			me->msticky= 0;
			allqueue(REDRAWBUTSEDIT, 0);
			break;

		case B_DELVERTCOL:
			if(me->mcol) freeN(me->mcol);
			me->mcol= 0;
			freedisplist(&(ob->disp));
			allqueue(REDRAWBUTSEDIT, 0);
			allqueue(REDRAWVIEW3D, 0);
			break;

		case B_MAKE_TFACES:
			make_tfaces(me);
			allqueue(REDRAWBUTSEDIT, 0);
			break;
			
		case B_FLIPNORM:
			if(G.obedit) {
				flip_editnormals();
			}
			else flipnorm_mesh( get_mesh(ob) );
			
			allqueue(REDRAWVIEW3D, 0);
			break;
		}
	}
	
	if(G.obedit==0 || (G.obedit->type!=OB_MESH && G.obedit->type!=OB_SECTOR && G.obedit->type!=OB_LIFE)) return;
	
	switch(event) {
	case B_SPIN:
		if( select_area(SPACE_VIEW3D)) spin_mesh(step, degr, 0, 0);
		break;
	case B_SPINDUP:
		if( select_area(SPACE_VIEW3D)) spin_mesh(step, degr, 0, 1);
		break;
	case B_EXTR:
		G.f |= G_DISABLE_OK;
		if( select_area(SPACE_VIEW3D)) extrude_mesh();
		G.f -= G_DISABLE_OK;
		break;
	case B_SCREW:
		if( select_area(SPACE_VIEW3D)) screw_mesh(step, turn);
		break;
	case B_EXTREP:
		if( select_area(SPACE_VIEW3D)) extrude_repeat_mesh(step, extr_offs);
		break;
	case B_SPLIT:
		G.f |= G_DISABLE_OK;
		split_mesh();
		G.f -= G_DISABLE_OK;
		break;
	case B_REMDOUB:
		a= removedoublesflag(1, doublimit);
		sprintf(str, "Removed: %d\n", a);
		notice(str);
		break;
	case B_SUBDIV:
		waitcursor(1);
		subdivideflag(1, 0.0, editbutflag & B_BEAUTY);
		countall();
		waitcursor(0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_FRACSUBDIV:
		randfac= 10;
		if(button(&randfac, 1, 100, "Rand fac:")==0) return;
		waitcursor(1);
		fac= -( (float)randfac )/100;
		subdivideflag(1, fac, editbutflag & B_BEAUTY);
		countall();
		waitcursor(0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_XSORT:
		if( select_area(SPACE_VIEW3D)) xsortvert_flag(1);
		break;
	case B_HASH:
		hashvert_flag(1);
		break;
	case B_TOSPHERE:
		vertices_to_sphere();
		break;
	}
	/* LETOP: bovenstaande events alleen in editmode! */
}

void meshbuts()
{
	Object *ob;
	Sector *se;
	Mesh *me;
	float val;
	char str[64];
	
	ob= OBACT;
	if(ob==0) return;
	
	me= get_mesh(ob);
	
	if(me) {
		DefBut(TOG|SHO|BIT|1, REDRAWVIEW3D, "No Puno Flip",	143,139,130,18, &me->flag);
		DefBut(TOG|SHO|BIT|2, REDRAWVIEW3D, "Double Sided",	1090,184,164,19, &me->flag);
		DefBut(TOG|SHO|BIT|3, 0, "UV effect",		143,159,130,18, &me->flag);
		
		SetButCol(BUTSALMON);
		
		if(me->msticky) val= 1.0; else val= 0.0;
		DefBut(LABEL, 0, "Sticky", 137,55,50,20, 0, val, 0);
		if(me->msticky==0) {
			SetButFunc(make_sticky);
			DefBut(BUT, 0, "Make",	210,58,63,19);
		}
		else DefBut(BUT, B_DELSTICKY, "Delete", 210,58,63,19);
	
		if(me->mcol) val= 1.0; else val= 0.0;
		DefBut(LABEL, 0, "VertCol", 140,33,50,20, 0, val, 0);
		if(me->mcol==0) {
			SetButFunc(make_vertexcol);
			DefBut(BUT, 0, "Make",	209,36,64,19);
		}
		else DefBut(BUT, B_DELVERTCOL, "Delete", 209,36,64,19);
		
		if(me->tface) val= 1.0; else val= 0.0;
		DefBut(LABEL, 0, "TexFace", 142,13,50,20, 0, val, 0);
		if(me->tface==0) {
			DefBut(BUT, B_MAKE_TFACES, "Make",	209,14,64,20);
		}
		else DefBut(BUT, 0, "Delete", 209,14,64,20);

		SetButCol(BUTGREY);
	
		SetButFunc(test_meshpoin_but);
		DefBut(IDPOIN, 0, "TexMesh:",		479,182,247,19, &me->texcomesh);
	}		

	/* EDIT */
	SetButCol(BUTSALMON);
	DefBut(BUT,B_SPINDUP,"Spin Dup",	639,106,87,30);
	DefBut(BUT,B_EXTR,"Extrude",	477,139,249,30);
	DefBut(BUT,B_SPIN, "Spin",		558,106,78,30);
	DefBut(BUT,B_SCREW,"Screw",		477,106,79,30);
	DefBut(BUT,B_EXTREP, "ExtrudeRepeat",477,25,128,27);
	
	SetButCol(BUTGREY);
	DefBut(NUM|SHO,0,"Degr:",		477,82,78,19, &degr,10.0,360.0);
	DefBut(NUM|SHO,0,"Steps:",		558,82,78,19, &step,1.0,180.0);
	DefBut(NUM|SHO,0,"Turns:",		639,82,86,19, &turn,1.0,360.0);
	DefBut(TOG|SHO|BIT|0,0,"Clockwise",	639,55,86,23, &editbutflag);
	DefBut(TOG|SHO|BIT|1,0,"Keep Original",	477,55,156,23, &editbutflag);
	DefBut(NUM|FLO,0,"Offset:",		608,25,117,27, &extr_offs, 0.01, 10.0, 100);

	SetButCol(BUTSALMON);
	#ifndef FREE
	SetButFunc(intersect_mesh);
	DefBut(BUT,0,"Intersect",			748,170,94,35);
	#endif
	DefBut(BUT,B_SPLIT,"Split",			846,171,91,34);
	DefBut(BUT,B_TOSPHERE,"To Sphere",		748,133,91,34);
	DefBut(BUT,B_REMDOUB,"Rem Doubles",	958,173,101,32);
	DefBut(BUT,B_SUBDIV,"Subdivide",	749,94,94,35);
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|2, 0, "Beauty",	846,133,91,35, &editbutflag);
	SetButCol(BUTSALMON);
	DefBut(BUT,B_FRACSUBDIV, "Fract Subd",847,94,90,35);
	SetButFunc(vertexnoise);
	DefBut(BUT,0,"Noise",				749,54,93,34);
	SetButFunc(vertexsmooth);
	DefBut(BUT,0,"Smooth",				847,54,91,34);
	DefBut(BUT,B_XSORT,"Xsort",			749,14,93,34);
	DefBut(BUT,B_HASH,"Hash",			847,14,91,34);

	SetButCol(BUTGREY);
	DefBut(NUM|FLO,0,"Limit:",			959,151,100,19, &doublimit, 0.0001, 1.0, 10);
	SetButCol(BUTSALMON);
	SetButFunc(fasterdraw);
	DefBut(BUT,0,"FasterDraw",			961,15,98,20);

	SetButFunc(slowerdraw);
	DefBut(BUT,0,"SlowerDraw",			961,37,98,20);

	DefBut(BUT,B_FLIPNORM,"Flip Normals",961,59,98,20);

	SetButCol(BUTGREY);
	DefBut(NUM|FLO,		  REDRAWVIEW3D, "NSize:",		1090, 90, 164, 19, &editbutsize, 0.001, 1.0, 10, 0);
	DefBut(TOG|SHO|BIT|6, REDRAWVIEW3D, "Draw Normals",	1090,70,164,19, &G.f);
	DefBut(TOG|SHO|BIT|7, REDRAWVIEW3D, "Draw Faces",	1090,50,164,19, &G.f);
	DefBut(TOG|SHO|BIT|11, 0, "All edges",				1090,10,164,19, &G.f);

}

/* *************************** FONT ******************************** */

short give_vfontnr(vfont)
struct VFont *vfont;
{
	VFont *vf;
	short nr= 0;

	vf= G.main->vfont.first;
	while(vf) {
		if(vf==vfont) return nr;
		nr++;
		vf= vf->id.next;
	}
	return -1;
}

VFont *give_vfontpointer(int nr)	/* nr= button */
{
	VFont *vf;
	short tel= 0;

	vf= G.main->vfont.first;
	while(vf) {
		if(tel==nr) return vf;
		tel++;
		vf= vf->id.next;
	}
	return G.main->vfont.first;
}

VFont *exist_vfont(char *str)
{
	VFont *vf;

	vf= G.main->vfont.first;
	while(vf) {
		if(strcmp(vf->name, str)==0) return vf;
		vf= vf->id.next;
	}
	return 0;
}

char *give_vfontbutstr()
{
	VFont *vf;
	int len= 0;
	char *str, di[100], fi[100];

	vf= G.main->vfont.first;
	while(vf) {
		strcpy(di, vf->name);
		splitdirstring(di, fi);
		len+= strlen(fi)+3;
		vf= vf->id.next;
	}
	
	str= callocN(len+21, "vfontbutstr");
	strcpy(str, "FONTS %t");
	vf= G.main->vfont.first;
	while(vf) {
		strcat(str, "| ");
		
		strcpy(di, vf->name);
		splitdirstring(di, fi);
		
		strcat(str, fi);
		vf= vf->id.next;
	}
	return str;
}

void load_buts_vfont(char *name)
{
	VFont *vf;
	Curve *cu;
	
	if(OBACT && OBACT->type==OB_FONT) cu= OBACT->data;
	else return;
	
	vf= exist_vfont(name);
	if(vf==0) {
		vf= load_vfont(name);
		if(vf==0) return;
	}
	else id_us_plus(vf);
	
	if(cu->vfont) cu->vfont->id.us--;
	cu->vfont= vf;
	
	text_to_curve(OBACT, 0);
	makeDispList(OBACT);
	allqueue(REDRAWVIEW3D, 0);
	allqueue(REDRAWBUTSEDIT, 0);
}

void do_fontbuts(short event)
{
	Curve *cu;
	VFont *vf;
	Object *ob;
	ScrArea *sa;
	char str[80];
	
	ob= OBACT;
	
	switch(event) {
	case B_MAKEFONT:
		text_to_curve(ob, 0);
		makeDispList(ob);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_TOUPPER:
		to_upper();
		break;
	case B_LOADFONT:
		vf= give_vfontpointer(G.buts->texnr);
		if(vf && vf->id.prev!=vf->id.next) strcpy(str, vf->name);
		else strcpy(str, U.fontdir);
		
		sa= closest_bigger_area();
		areawinset(sa->win);

		activate_fileselect(FILE_SPECIAL, "SELECT FONT", str, load_buts_vfont);

		break;
	case B_SETFONT:
		if(ob) {
			cu= ob->data;

			vf= give_vfontpointer(G.buts->texnr);
			if(vf) {
				id_us_plus(vf);
				cu->vfont->id.us--;
				cu->vfont= vf;
				text_to_curve(ob, 0);
				makeDispList(ob);
				allqueue(REDRAWVIEW3D, 0);
				allqueue(REDRAWBUTSEDIT, 0);
			}
		}	
		break;
	}
}



void fontbuts()
{
	Curve *cu;
	ID *id;
	char *strp;
	
	if(OBACT==0) return;
	cu= OBACT->data;

	SetButCol(BUTGREEN);
	DefBut(ROW|SHO,B_MAKEFONT, "Left",		484,139,53,18, &cu->spacemode, 0.0,0.0);
	DefBut(ROW|SHO,B_MAKEFONT, "Middle",	604,139,61,18, &cu->spacemode, 0.0,1.0);
	DefBut(ROW|SHO,B_MAKEFONT, "Right",		540,139,62,18, &cu->spacemode, 0.0,2.0);
	DefBut(ROW|SHO,B_MAKEFONT, "Flush",		665,139,61,18, &cu->spacemode, 0.0,3.0);

	SetButCol(BUTGREY);

	SetButFunc(test_obpoin_but);
	DefBut(IDPOIN, B_MAKEDISP, "TextOnCurve:",	484,115,243,19, &cu->textoncurve);

	DefBut(NUM|FLO,B_MAKEFONT, "Size:",		482,56,121,19, &cu->fsize, 0.1,10.0, 10);
	DefBut(NUM|FLO,B_MAKEFONT, "Linedist:",	605,56,121,19, &cu->linedist, 0.0,10.0, 10);
	DefBut(NUM|FLO,B_MAKEFONT, "Spacing:",	482,34,121,19, &cu->spacing, 0.0,10.0, 10);
	DefBut(NUM|FLO,B_MAKEFONT, "Y offset:",	605,34,121,19, &cu->yof, -50.0,50.0, 10);
	DefBut(NUM|FLO,B_MAKEFONT, "Shear:",	482,12,121,19, &cu->shear, -1.0,1.0, 10);
	DefBut(NUM|FLO,B_MAKEFONT, "X offset:",	605,12,121,19, &cu->xof, -50.0,50.0, 10);

	DefBut(TEX,REDRAWVIEW3D, "Ob Family:",	752,192,164,19, cu->family, 0.0, 20.0);

	SetButCol(BUTSALMON);
	DefBut(BUT, B_TOUPPER, "ToUpper",		623,163,104,23);
	
	SetButCol(BUTGREY);

	G.buts->texnr= give_vfontnr(cu->vfont);
	
	strp= give_vfontbutstr();
	
	DefBut(MENU|SHO, B_SETFONT, strp, 484,191,246,20, &G.buts->texnr);
	freeN(strp);
	
	SetButCol(BUTSALMON);
	DefBut(BUT,B_LOADFONT, "Load Font",	484,163,103,23);
	
}

/* *************************** CURVE ******************************** */


void do_curvebuts(short event)
{
	extern Nurb *lastnu;
	Object *ob;
	Curve *cu;
	Nurb *nu;
	
	ob= OBACT;
	if(ob==0) return;
	
	switch(event) {	

	case B_CONVERTPOLY:
	case B_CONVERTBEZ:
	case B_CONVERTBSPL:
	case B_CONVERTCARD:
	case B_CONVERTNURB:
		if(G.obedit) {
			setsplinetype(event-B_CONVERTPOLY);
			makeDispList(G.obedit);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_UNIFU:
	case B_ENDPU:
	case B_BEZU:
	case B_UNIFV:
	case B_ENDPV:
	case B_BEZV:
		if(G.obedit) {
			nu= editNurb.first;
			while(nu) {
				if(isNurbsel(nu)) {
					if((nu->type & 7)==CU_NURBS) {
						if(event<B_UNIFV) {
							nu->flagu &= 1;
							nu->flagu += ((event-B_UNIFU)<<1);
							makeknots(nu, 1, nu->flagu>>1);
						}
						else if(nu->pntsv>1) {
							nu->flagv &= 1;
							nu->flagv += ((event-B_UNIFV)<<1);
							makeknots(nu, 2, nu->flagv>>1);
						}
					}
				}
				nu= nu->next;
			}
			makeDispList(G.obedit);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_SETWEIGHT:
		if(G.obedit) {
			weightflagNurb(1, editbutweight, 0);
			makeDispList(G.obedit);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_SETW1:
		editbutweight= 1.0;
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_SETW2:
		editbutweight= fsqrt(2.0)/4.0;
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_SETW3:
		editbutweight= 0.25;
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_SETW4:
		editbutweight= fsqrt(0.5);
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_SETORDER:
		if(G.obedit) {
			nu= lastnu;
			if(nu && (nu->type & 7)==CU_NURBS ) {
				if(nu->orderu>nu->pntsu) {
					nu->orderu= nu->pntsu;
					addqueue(curarea->win, REDRAW, 1);
				}
				makeknots(nu, 1, nu->flagu>>1);
				if(nu->orderv>nu->pntsv) {
					nu->orderv= nu->pntsv;
					addqueue(curarea->win, REDRAW, 1);
				}
				makeknots(nu, 2, nu->flagv>>1);
			}
			makeDispList(G.obedit);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_MAKEDISP:
		if(ob->type==OB_FONT) text_to_curve(ob, 0);
		makeDispList(ob);
		allqueue(REDRAWVIEW3D, 0);
		break;
	
	case B_SUBDIVCURVE:
		subdivideNurb();
		break;
	case B_SPINNURB:
		if(G.obedit==0 || G.obedit->type!=OB_SURF || (G.obedit->lay & G.vd->lay==0)) return;
		spinNurb(0, 0);
		countall();
		makeDispList(G.obedit);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_CU3D:	    /* allow 3D curve */
		if(G.obedit) {
			cu= G.obedit->data;
			nu= editNurb.first;
			while(nu) {
				nu->type &= ~CU_2D;
				if((cu->flag & CU_3D)==0) nu->type |= CU_2D;
				test2DNurb(nu);
				nu= nu->next;
			}
		}
		if(ob->type==OB_CURVE) {
			cu= ob->data;
			nu= cu->nurb.first;
			while(nu) {
				nu->type &= ~CU_2D;
				if((cu->flag & CU_3D)==0) nu->type |= CU_2D;
				test2DNurb(nu);
				nu= nu->next;
			}
		}
		break;
	case B_SETRESOLU:
		if(ob->type==OB_CURVE) {
			cu= ob->data;
			if(ob==G.obedit) nu= editNurb.first;
			else nu= cu->nurb.first;
			
			while(nu) {
				nu->resolu= cu->resolu;
				nu= nu->next;
			}
		}
		else if(ob->type==OB_FONT) text_to_curve(ob, 0);
		
		makeDispList(ob);
		allqueue(REDRAWVIEW3D, 0);

		break;
	}
}

void curvebuts()
{
	Object *ob;
	Curve *cu;
	Nurb *nu;
	extern Nurb *lastnu;
	short *sp;
	
	ob= OBACT;
	if(ob==0) return;
	cu= ob->data;

	if(ob->type==OB_CURVE || ob->type==OB_SURF) {
		SetButCol(BUTSALMON);
		DefBut(LABEL, 0, "Convert",	463,173,72, 18, 0, 0, 0);
		DefBut(BUT,B_CONVERTPOLY,"Poly",		467,152,72, 18);
		DefBut(BUT,B_CONVERTBEZ,"Bezier",	467,132,72, 18);
		DefBut(BUT,B_CONVERTBSPL,"Bspline",	467,112,72, 18);
		DefBut(BUT,B_CONVERTCARD,"Cardinal",	467,92,72, 18);
		DefBut(BUT,B_CONVERTNURB,"Nurb",		467,72,72, 18);
	
		DefBut(LABEL, 0, "Make Knots",562,173,102, 18, 0, 0, 0);
		DefBut(BUT,B_UNIFU,"Uniform U",	565,152,102, 18);
		DefBut(BUT,B_ENDPU,"Endpoint U",	565,132,102, 18);
		DefBut(BUT,B_BEZU,"Bezier U",	565,112,102, 18);
		DefBut(BUT,B_UNIFV,"V",		670,152,50, 18);
		DefBut(BUT,B_ENDPV,"V",		670,132,50, 18);
		DefBut(BUT,B_BEZV,"V",		670,112,50, 18);
	
		DefBut(BUT,B_SETWEIGHT,"Set Weight",	465,11,95,49);
		SetButCol(BUTGREY);
		DefBut(NUM|FLO,0,"Weight:",	564,36,102,22, &editbutweight, 0.01, 10.0, 10);
		DefBut(BUT,B_SETW1,"1.0",		669,36,50,22);
		DefBut(BUT,B_SETW2,"sqrt(2)/4",	564,11,57,20);
		DefBut(BUT,B_SETW3,"0.25",		621,11,43,20);
		DefBut(BUT,B_SETW4,"sqrt(0.5)",	664,11,57,20);
		
		if(ob==G.obedit) {
			nu= lastnu;
			if(nu==0) nu= editNurb.first;
			if(nu) sp= &(nu->orderu); 
			else sp= 0;
			DefBut(NUM|SHO, B_SETORDER, "Order U:", 565,91,102, 18, sp, 2.0, 6.0);
			if(nu) sp= &(nu->orderv); 
			else sp= 0;
			DefBut(NUM|SHO, B_SETORDER, "V:",	 670,91,50, 18, sp, 2.0, 6.0);
			if(nu) sp= &(nu->resolu); 
			else sp= 0;
			DefBut(NUM|SHO, B_MAKEDISP, "Resol U:", 565,70,102, 18, sp, 1.0, 128.0);
			if(nu) sp= &(nu->resolv); 
			else sp= 0;
			DefBut(NUM|SHO, B_MAKEDISP, "V:", 670,70,50, 18, sp, 1.0, 128.0);
		}

		SetButCol(BUTSALMON);
		DefBut(BUT, B_SUBDIVCURVE, "Subdivide",	1092,105,165,20);
	}

	if(ob->type==OB_SURF) {
		DefBut(BUT, B_SPINNURB, "Spin",	808,92,101,36);
	}
	else {

		SetButCol(BUTGREY);
		DefBut(TOG|SHO|BIT|5, 0, "UV Orco",			135,150,100,18, &cu->flag);
		
		DefBut(NUM|SHO, B_MAKEDISP, "DefResolU:",	752,163,132,21, &cu->resolu, 1.0, 128.0);
		SetButCol(BUTSALMON);
		DefBut(BUT, B_SETRESOLU, "Set",				887,163,29,21);
		
		SetButCol(BUTGREY);
		DefBut(NUM|SHO, B_MAKEDISP, "BevResol:",	753,30,163,18, &cu->bevresol, 0.0, 10.0);
		SetButFunc(test_obpoin_but);
		DefBut(IDPOIN, B_MAKEDISP, "BevOb:",		753,10,163,18, &cu->bevobj);
		DefBut(NUM|FLO, B_MAKEDISP, "Width:",		753,90,163,18, &cu->width, 0.0, 2.0, 1);
		DefBut(NUM|FLO, B_MAKEDISP, "Ext1:",		753,70,163,18, &cu->ext1, 0.0, 5.0, 10);
		DefBut(NUM|FLO, B_MAKEDISP, "Ext2:",		753,50,163,18, &cu->ext2, 0.0, 2.0, 1);
		SetButCol(BUTBLUE);
		if(ob->type==OB_FONT) {
			DefBut(TOG|SHO|BIT|1, B_MAKEDISP, "Front",	833,130,79,18, &cu->flag);
			DefBut(TOG|SHO|BIT|2, B_MAKEDISP, "Back",	753,130,76,18, &cu->flag);
		}
		else {
			DefBut(TOG|SHO|BIT|0, B_CU3D, "3D",			867,130,47,18, &cu->flag);
			DefBut(TOG|SHO|BIT|1, B_MAKEDISP, "Front",	810,130,55,18, &cu->flag);
			DefBut(TOG|SHO|BIT|2, B_MAKEDISP, "Back",	753,130,53,18, &cu->flag);
		}
		SetButCol(BUTGREY);
	}

	DefBut(NUM|FLO,		  REDRAWVIEW3D, "NSize:",		1090, 80, 164, 19, &editbutsize, 0.001, 1.0, 10, 0);
}

/* *************************** CAMERA ******************************** */


void camerabuts()
{
	extern float visifac[2], distfac[2];	/* sector.c */
	Camera *cam;
	Object *ob;
	float grid;
	
	if(G.vd) grid= G.vd->grid; else grid= 1.0;
	
	ob= OBACT;
	if(ob==0) return;
	
	cam= ob->data;
	DefBut(NUM|FLO,REDRAWVIEW3D, "Lens:", 470,178,160,20, &cam->lens, 1.0, 250.0, 100);
	DefBut(NUM|FLO,REDRAWVIEW3D, "ClipSta:", 470,147,160,20, &cam->clipsta, 0.001*grid, 100.0*grid, 10);
	DefBut(NUM|FLO,REDRAWVIEW3D, "ClipEnd:", 470,125,160,20, &cam->clipend, 1.0, 5000.0*grid, 100);
	DefBut(NUM|FLO,REDRAWVIEW3D, "DrawSize:", 470,90,160,20, &cam->drawsize, 0.1*grid, 10.0, 10);

	SetButCol(BUTGREEN);
	DefBut(TOG|SHO, REDRAWVIEW3D, "Ortho", 470,49,61,40, &cam->type);

	DefBut(TOG|SHO|BIT|0,REDRAWVIEW3D, "ShowLimits", 533,69,97,20, &cam->flag);
	DefBut(TOG|SHO|BIT|1,REDRAWVIEW3D, "Show Mist", 533,49,97,20, &cam->flag);
	DefBut(TOG|SHO|BIT|2, 0, "NetWork", 469,23,63,24, &cam->flag);
	DefBut(TOG|SHO|BIT|3,REDRAWVIEW3D, "Show NetClip", 533,23,97,24, &cam->flag);
	
	SetButCol(BUTGREY);
	/* DefBut(NUM|FLO,0, "Border:", 670,180,160,20, visifac, 0.01, 1.0, 10); */
	/* DefBut(NUM|FLO,0, "Centre:", 670,160,160,20, visifac+1, 0.01, 1.0, 10); */
	
	DefBut(NUM|FLO,REDRAWVIEW3D, "Net Sta:", 670,43,160,20, &cam->netsta, 0.1, 250.0, 10);
	DefBut(NUM|FLO,REDRAWVIEW3D, "Net End:", 670,23,160,20, &cam->netend, 0.1, 250.0, 10);
	DefBut(NUM|SHO, 0, "Hold:", 670,83,160,20, &cam->hold, 0.0, 250.0);
	
}

/* *************************** GAME ******************************** */


void do_gamebuts(short event)
{
	Base *base;
	Object *ob, *pob;
	Sector *se;
	Life *lf;
	Sensor *sn;
	Event *ev;
	Action *ac;
	Mesh *me;
	int a, len;
	
	ob= OBACT;
	if(ob==0) return;

	switch(event) {
	case B_ADDSENS:
		add_sensor(ob->data);
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_DELSENS:
		delete_sensor(ob->data);
		addqueue(curarea->win, REDRAW, 1);
		break;
	case B_ADDPORT:
		add_portal(ob->data);
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		sector_update(0);
		break;
	case B_DELPORT:
		delete_portal(ob->data);
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		sector_update(0);
		break;
	case B_TESTPORT:
		testportals(ob->data);
		sector_update(0);
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_TOTEVENT:
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		
		len= alloc_len(sn->events)/sizeof(Event);
		
		if( sn->totevent>len ) {
			ev= callocN( sn->totevent*sizeof(Event), "events");
			memcpy(ev, sn->events, len*sizeof(Event));
			freeN(sn->events);
			sn->events= ev;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_TOTACTION:
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		
		len= alloc_len(sn->actions)/sizeof(Action);
		
		if( sn->totaction>len ) {
			ac= callocN( sn->totaction*sizeof(Action), "actions");
			memcpy(ac, sn->actions, len*sizeof(Action));
			freeN(sn->actions);
			sn->actions= ac;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_TESTNAME:
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		
		a= sn->totevent;
		ev= sn->events;
		while(a--) {
			if(ev->name[0]) {
				if(ev->event==SN_CONTACT) {
					if( find_id("MA", ev->name)==0 ) ev->name[0]= 0;
				}
				else if ELEM(ev->event, SN_NEAR, SN_TRIPFACE) {
					if( find_id("OB", ev->name)==0 ) ev->name[0]= 0;
				}
			}
			ev++;
		}
		a= sn->totaction;
		ac= sn->actions;
		while(a--) {
			if(ac->name[0]) {
				if(ac->action<100);
				else if(ac->action<200) {	/* ipoos */
					pob= (Object *)find_id("OB", ac->name);
					if( pob==0 || pob->type!=OB_LIFE) ac->name[0]= 0;
					else {
						lf= pob->data;
						if(lf->type==LF_DYNAMIC) ac->name[0]= 0;
					}
					if(ac->name[0]==0) error("Only for props");
				}
				else if ELEM5(ac->action, SN_GOTO, SN_ADDLIFE, SN_REPLACELIFE, SN_TRACKTO, SN_ROBBIE_M) {
					pob= (Object *)find_id("OB", ac->name);
					if( pob==0 || pob->type!=OB_LIFE) ac->name[0]= 0;
				}
				else if(ac->action==SN_SETCAMERA) {
					pob= (Object *)find_id("OB", ac->name);
					if( pob==0 || pob->type!=OB_CAMERA) ac->name[0]= 0;
				}
				else if(ac->action==SN_REPLACEMESH) {
					me= (Mesh *)find_id("ME", ac->name);
					if( me==0 || me->tface==0) ac->name[0]= 0;
				}
			}
			ac++;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
		
	case B_TESTVAR:
	case B_TESTVAR+1:
	case B_TESTVAR+2:
	case B_TESTVAR+3:
	
		/* de current action naam: al in gebruik? */
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		ac= sn->actions + (event-B_TESTVAR);
		
		if(ac->name[0]) test_action_var(ob, ac);

		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_COPYVAR:
	case B_COPYVAR+1:
	case B_COPYVAR+2:
	case B_COPYVAR+3:
		
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		ac= sn->actions + (event-B_COPYVAR);	/* current */

		if(ac->name[0]) copy_action_var(ob, ac);

		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_TESTK2K:
		/* illegale combinaties opsporen */
		lf= ob->data;
		sn= lf->sensors+lf->actsens-1;
		ac= sn->actions;
		a= sn->totaction;
		while(a--) {
			if(ac->action==SN_K2K_OBIPO) {
				if(ac->var & SN_K2K_PINGPONG) ac->var &= ~SN_K2K_PREV;
			}
			ac++;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_ALLTEXMESH:
	case B_ALLDYNAMESH:
		base= FIRSTBASE;
		while(base) {
			if(base->object->type==OB_SECTOR) {
				se= base->object->data;
				if(event==B_ALLTEXMESH) se->flag |= SE_SHOW_TEXMESH;
				else se->flag &= ~SE_SHOW_TEXMESH;
			}
			else if(base->object->type==OB_LIFE) {
				lf= base->object->data;
				if(event==B_ALLTEXMESH) lf->flag |= LF_SHOW_TEXMESH;
				else lf->flag &= ~LF_SHOW_TEXMESH;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_LIFELAY:
		lf= ob->data;
		if(lf->lay==0) {
			lf->lay= 1;
			allqueue(REDRAWBUTSGAME, 0);
		}
		break;
	}

}

void gamebuts()
{
	Life *lf;
	Sector *se;
	Object *ob;
	int a, xco;
	char *strp, str[32], str1[32];
	
	#ifndef FREE
		
	ob= OBACT;
	if(ob==0) return;
	
	if(ob->type==OB_SECTOR) {
		se= ob->data;
		
		DefBut(NUM|SHO, 0, "Max Draw Sector:",	2,190,135,20, &G.scene->maxdrawsector, 1.0, 128.0);
		DefBut(NUM|FLO, 0, "Gravity:",			2,170,135,20, &G.scene->grav, 0.0, 1.0, 100);

		DefBut(NUM|FLO, 0, "CubemapSize:",			2,140,135,20, &cumapsize, 0.01, 5.0, 10);

		SetButCol(BUTSALMON);
		DefBut(BUT, B_ADDPORT, "ADD portal",		142,180,86,31);
		DefBut(BUT, B_DELPORT, "DEL",				231,180,44,31);
		SetButCol(BUTGREY);
		if(se->actport>se->totport) se->actport= se->totport;
		sprintf(str, "%d Current:", se->totport);
		DefBut(NUM|SHO, B_TESTPORT, str,		143,156,133,20, &se->actport, 1.0, (float)(se->totport));
		
		if(se->totport) {
			Portal *po;
			
			po= se->portals+se->actport-1;
			
			SetButCol(BUTGREEN);
			DefBut(ROW|SHO, B_TESTPORT, "X +", 143,132,45,20, &po->type, 12.0, 1.0);
			DefBut(ROW|SHO, B_TESTPORT, "X -", 143,110,45,20, &po->type, 12.0, 2.0);
			DefBut(ROW|SHO, B_TESTPORT, "Y +", 188,132,45,20, &po->type, 12.0, 3.0);
			DefBut(ROW|SHO, B_TESTPORT, "Y -", 188,110,45,20, &po->type, 12.0, 4.0);
			DefBut(ROW|SHO, B_TESTPORT, "Z +", 231,132,45,20, &po->type, 12.0, 5.0);
			DefBut(ROW|SHO, B_TESTPORT, "Z -", 231,110,45,20, &po->type, 12.0, 6.0);
			SetButCol(BUTGREY);
			DefBut(NUM|FLO, B_TESTPORT, "dU:", 141, 69, 135, 20, po->ofs, -8.0, 8.0, 10);
			DefBut(NUM|FLO, B_TESTPORT, "dV:", 141, 49, 135, 20, po->ofs+1, -8.0, 8.0, 10);
		}

		SetButCol(BUTGREY);
		DefBut(NUM|FLO, B_TESTPORT, "", 143,15,45,20, se->size, 0.125, 8.0, 10);
		DefBut(NUM|FLO, B_TESTPORT, "", 188,15,45,20, se->size+1, 0.125, 8.0, 10);
		DefBut(NUM|FLO, B_TESTPORT, "", 231,15,45,20, se->size+2, 0.125, 8.0, 10);
		
		SetButCol(BUTPURPLE);
		DefBut(TOG|SHO|BIT|4, 0, "Network Render",	486,180,138,40, &se->flag);
		sprintf(str, "Frames: %d", se->totfra);
		DefBut(LABEL, 0, str,	487,157,135,20);
		
		SetButCol(BUTGREEN);
		DefBut(TOG|SHO|BIT|0, 0, "Add scene name",	486,30,138,20, &G.scene->flag);
		
		ClearButLock();
	}
	else if(ob->type==OB_LIFE) {
		lf= ob->data;
		
		SetButCol(BUTGREEN);
		DefBut(ROW|CHA, B_REDR, "Prop",		10,200,50,20, &lf->type, 10.0, 0.0);
		DefBut(ROW|CHA, B_REDR, "Dyna",		62,200,43,20, &lf->type, 10.0, 1.0);
		DefBut(ROW|CHA, B_REDR, "Link",		108,200,51,20, &lf->type, 10.0, 2.0);

		DefBut(TOG|SHO|BIT|11, REDRAWVIEW3D, "Sphere",	175,200,79,20, &lf->flag);
		DefBut(TOG|SHO|BIT|7, 0,		"CalcLight",	257,200,82,20, &lf->flag);
		DefBut(TOG|SHO|BIT|12, 0,		"Do Fh",		341,200,72,20, &lf->flag);
		DefBut(TOG|SHO|BIT|5, 0,		"MainActor",	441,199,196,20, &lf->flag);
		DefBut(TOG|SHO|BIT|6, 0,		"CameraActor",	766,199,100,20, &lf->flag);
		
		SetButCol(BUTGREY);

		if(lf->type==LF_DYNAMIC) {
			DefBut(NUM|FLO, 0, "Frict:",		661,100,103,20, &lf->frict, 0.0, 1.0, 1);
			DefBut(NUM|FLO, 0, "RotFrict:",		661,80,103,20, &lf->rotfrict, 0.0, 1.0, 1);
			DefBut(NUM|FLO, 0, "Mass:",			660,10,104,20, &lf->mass, 0.0, 10.0, 10);
		}
		else if(lf->type==LF_LINK) {
			DefBut(NUM|FLO, 0, "RotFrict:",		661,80,103,20, &lf->rotfrict, 0.0, 1.0, 1);			
		}
		
		/* in writeblendpsx: minimum axsize=0.01 */
		DefBut(NUM|FLO, REDRAWVIEW3D, "Axis:",	661,60,103,20, &lf->axsize, 0.01, 2.5, 10);
		DefBut(NUM|SHO, 0, "Ipo Sta:",			660,40,104,20, &lf->sfra, 0.0, 1500.0);

		for(a=0; a<8; a++) {
			DefBut(TOG|CHA|BIT|a, B_LIFELAY, "",	660+a*(12), 200, 12, 20, &(lf->lay));
		}

		
		SetButCol(BUTGREEN);
		DefBut(TOG|SHO|BIT|0, B_REDR, "Sensors",	10,160,160,30, &lf->flag);
		
		ClearButLock();
		
		if(lf->flag & LF_SENSORS) {
			
			SetButCol(BUTSALMON);
			DefBut(BUT, B_ADDSENS, "ADD sensor",	171,160,86,30);
			DefBut(BUT, B_DELSENS, "DEL sensor",	259,160,91,30);
			SetButCol(BUTGREY);
	
			if(lf->actsens>lf->totsens) lf->actsens= lf->totsens;
			sprintf(str1, "%d Current:", lf->totsens);
			DefBut(NUM|SHO, B_REDR, str1,			352,160,131,30, &lf->actsens, 1.0, (float)(lf->totsens));
			
			if(lf->totsens) {
				Sensor *sn;
				Event *ev;
				Action *ac;
				
				sn= lf->sensors+lf->actsens-1;
				
				SetButCol(BUTSALMON);
				DefBut(NUM|SHO, B_TOTEVENT, "Events:",		484,160,74,30, &sn->totevent, 1.0, 3.0);
				DefBut(NUM|SHO, B_TOTACTION, "Actions:",	560,160,79,30, &sn->totaction, 1.0, 3.0);
				SetButCol(BUTGREY);
				
				eventnames_to_pupstring(&strp);
				ev= sn->events;
				a= sn->totevent;
				xco= 10;
				while(a--) {
					DefBut(LABEL, 0, "Event",			xco,140,90,20, 0, 0, 0);
					SetButCol(BUTBLUE);
					DefBut(MENU|SHO, B_REDR, strp,		xco,120,90,20, &ev->event);
					SetButCol(BUTGREY);
					
					if(ev->event < 32) {
						DefBut(TOG|SHO, 0, "Controller 2", xco,100,90,20, &ev->var);
					}
					else if(ev->event==SN_NEAR) {
						DefBut(NUM|FLO, 0, "Dist:",		xco,100,90,20, &ev->fac, 0.0, 100.0, 10);
						DefBut(NUM|FLO, 0, "Reset:",	xco,80,90,20, &ev->fac1, 0.0, 100.0, 10);
						DefBut(TEX, B_TESTNAME, "OB:",	xco,60,90,20, ev->name, 0.0, 15.0);
					}
					else if(ev->event==SN_TIMER) {
						DefBut(NUM|SHO, 0, "Time:",		xco,100,90,20, &ev->var, -1.0, 10000.0, 10);
					}
					else if(ev->event==SN_CONTACT) {
						DefBut(TEX, B_TESTNAME, "MA:",	xco,100,90,20, ev->name, 0.0, 15.0);
					}
					else if(ev->event==SN_VAR_EQUAL) {
						DefBut(TEX, 0,"",				xco,100,90,20, ev->name, 0.0, 15.0);
						DefBut(NUM|SHO, 0, "Val:",		xco,80,90,20, &ev->var, -1000.0, 1000.0);
					}
					else if(ev->event==SN_VAR_INTERVAL) {
						DefBut(TEX, 0,"",				xco,100,90,20, ev->name, 0.0, 15.0);
						DefBut(NUM|SHO, 0, "Min:",		xco,80,90,20, &ev->sfac, -1000.0, 1000.0);
						DefBut(NUM|SHO, 0, "Max:",		xco,60,90,20, &ev->sfac1, -1000.0, 1000.0);
					}
					else if(ev->event==SN_VAR_CHANGED) {
						DefBut(TEX, 0,"",				xco,100,90,20, ev->name, 0.0, 15.0);
					}
					else if(ev->event==SN_TRIPFACE) {
						DefBut(TEX, B_TESTNAME, "OB:",	xco,100,90,20, ev->name, 0.0, 15.0);
					}
					else if(ev->event==SN_COLLISION) {
						DefBut(NUM|SHO, 0, "Damp:",		xco,100,90,20, &ev->sfac, 0.0, 100.0);
						DefBut(TEX, B_TESTNAME, "OB:",	xco,60,90,20, ev->name, 0.0, 15.0);
					}

					SetButCol(BUTGREEN);
					DefBut(TOG|CHA|BIT|0, 0, "Hold",	xco,40,55,20, &ev->flag);
					DefBut(TOG|CHA|BIT|1, 0, "Not",		xco+55,40,35,20, &ev->flag);
					SetButCol(BUTGREY);
					DefBut(NUM|CHA, 0, "Pulse:",		xco,20,90,20, &ev->pulse, 0.0, 15.0);
					
					xco+= 90;
					ev++;
				}
				freeN(strp);
				
				actionnames_to_pupstring(&strp);
				ac= sn->actions;
				xco= 280;
				for(a=0; a<sn->totaction; a++) {
					
					DefBut(LABEL, 0, "Action",			xco,140,120,20, 0, 0, 0);
					SetButCol(BUTBLUE);
					DefBut(MENU|SHO, B_REDR, strp,		xco,120,120,20, &ac->action);
					SetButCol(BUTGREY);
					
					if(ac->action==0);
					else if(ac->action==SN_GOTO) {				/* 32: is short op PS */
						DefBut(NUM|FLO, 0, "X:",	xco,100,120,20, &ac->force, -32.0, 32.0, 10);
						DefBut(NUM|FLO, 0, "Y:",	xco,80,120,20, &ac->min, -32.0, 32.0, 10);
						DefBut(NUM|FLO, 0, "Z:",	xco,60,120,20, &ac->max, -32.0, 32.0, 10);
						DefBut(TEX, B_TESTNAME, "OB:",	xco,40,120,20, ac->name, 0.0, 15.0);
					}
					else if(ac->action<100) {
						DefBut(NUM|FLO, 0, "Force:",	xco,100,120,20, &ac->force, -16.0, 16.0, 10);
							
						if ELEM3(ac->action, SN_X_ROT, SN_Y_ROT, SN_Z_ROT) {
							SetButCol(BUTGREEN);
							DefBut(TOG|SHO|BIT|0, B_REDR, "Do Limits",	xco, 80,120,20, &ac->flag);
							SetButCol(BUTGREY);
							if(ac->flag & SN_LIMITS) {
								DefBut(NUM|FLO, 0, "Min:",	xco,60,120,20, &ac->min, -10.0, 10.0, 10);
								DefBut(NUM|FLO, 0, "Max:",	xco,40,120,20, &ac->max, -10.0, 10.0, 10);
							}
						}
						else DefBut(NUM|FLO, 0, "FrictFac:",xco,80,120,20, &ac->fac, 0.0, 2.0, 10);
						
					}
					else if(ac->action<200) {	/* ipoos */
						if(ac->action==SN_K2K_OBIPO) {
							DefBut(TOG|SHO|BIT|0, B_TESTK2K, "Prev",	xco,	60,60,20, &ac->var);
							DefBut(TOGN|SHO|BIT|0, B_TESTK2K, "Next",	xco+60,	60,60,20, &ac->var);
							DefBut(TOG|SHO|BIT|1, B_TESTK2K, "Cyclic",	xco,	40,50,20, &ac->var);
							DefBut(TOG|SHO|BIT|2, B_TESTK2K, "PingPong",xco+50,	40,70,20, &ac->var);
							DefBut(TOG|SHO|BIT|3, B_TESTK2K, "Hold",	xco,	20,120,20, &ac->var);
						}
						else {
							/* OB button tijdelijk weg: problemen met imat en dynachanged afhandeling */
							/* DefBut(TEX, B_TESTNAME, "OB:",xco,100,120,20, ac->name, 0.0, 15.0); */
							DefBut(NUM|SHO, 0, "Start:",	xco,100,120,20, &ac->butsta, 0.0, 1000.0);
							DefBut(NUM|SHO, 0, "End:",		xco,80,120,20, &ac->butend, 0.0, 1000.0);

							SetButCol(BUTGREEN);
							DefBut(TOG|SHO|BIT|4, B_REDR, "SetVar",	xco, 60,120,20, &ac->flag);
							SetButCol(BUTGREY);
							if(ac->flag & SN_IPO_SETVAR) {
								DefBut(TEX, 0, "",				xco,40,120,20, ac->name, 0.0, 15.0);
							}
							SetButCol(BUTGREEN);
							DefBut(TOG|SHO|BIT|1, 0, "IpoForce",	xco,	20,120,20, &ac->flag);
						}
						
					}
					else if(ac->action<300) {	/* divers */
						if(ac->action==SN_LAYERMOVE) {
							DefBut(NUM|SHO, 0, "To Lay:",	xco,100,120,20, &ac->butend, 1.0, 24.0);
						}
						else if(ac->action==SN_STARTTIMER) {
							DefBut(NUM|SHO, 0, "Time:",		xco,100,120,20, &ac->butsta, 1.0, 10000.0);
						}
						else if(ac->action==SN_ADDLIFE) {
							DefBut(TEX, B_TESTNAME, "OB:",	xco,100,120,20, ac->name, 0.0, 15.0);
							DefBut(NUM|SHO, 0, "Time:",		xco,80,120,20, &ac->butsta, 1.0, 10000.0);
						}
						else if(ac->action==SN_REPLACELIFE) {
							DefBut(TEX, B_TESTNAME, "OB:",	xco,100,120,20, ac->name, 0.0, 15.0);
						}
						else if(ac->action==SN_LOADFILE) {
							DefBut(TEX, 0, "File:",			xco,100,120,20, ac->name, 0.0, 15.0);
						}
						else if(ac->action==SN_VISIBLE) {
							DefBut(NUM|SHO, 0, "Extra time:",	xco,100,120,20, &ac->butsta, 0.0, 10000.0);
						}
						else if(ac->action==SN_REPLACEMESH) {
							DefBut(TEX, B_TESTNAME, "ME:",	xco,100,120,20, ac->name, 0.0, 15.0);
						}
						else if(ac->action==SN_SETCAMERA) {
							DefBut(TEX, B_TESTNAME, "OB:",	xco,100,120,20, ac->name, 0.0, 15.0);
						}
						else if(ac->action==SN_PLAYMOVIE) {
							DefBut(TEX, 0, "Movie:",xco,	100,120,20, ac->name, 0.0, 15.0);
							DefBut(NUM|SHO, 0, "Start:",	xco,	80,120,20, &ac->butsta, -1000.0, 10000.0);
							DefBut(NUM|SHO, 0, "End:",		xco,	60,120,20, &ac->butend, -1000.0, 10000.0);
						}
						else if(ac->action==SN_TRACKTO) {
							DefBut(TEX, B_TESTNAME, "OB:",	xco,100,120,20, ac->name, 0.0, 15.0);
							SetButCol(BUTGREEN);
							if(ac->cur!=0 && ac->cur!=1) ac->cur= 0;
							DefBut(TOG|SHO, 0, "3D track",	xco,80,120,20, &ac->cur);
							SetButCol(BUTGREY);
							DefBut(NUM|SHO, 0, "Time:",		xco,60,120,20, &ac->butsta, 0.0, 100.0);
						}
						else if(ac->action==SN_ADD_DAMAGE) {
							DefBut(NUM|SHO, 0, "Damage:",	xco,	60,120,20, &ac->var, -1000.0, 1000.0);
						}
						else if(ac->action==SN_ROBBIE_M) {
							DefBut(TEX, B_TESTNAME, "OB:",	xco,100,120,20, ac->name, 0.0, 15.0);
							DefBut(NUM|FLO, 0, "Height:",	xco,80,120,20, &ac->force, 0.0, 32.0, 10);
							DefBut(NUM|FLO, 0, "Mindist:",	xco,60,120,20, &ac->min, 0.0, 32.0, 10);
							DefBut(NUM|FLO, 0, "Maxdist:",	xco,40,120,20, &ac->max, 0.0, 32.0, 10);
							
						}
					}
					else if(ac->action<400) {	/* var */

						/* testvar && copyvar: denk aan '+a' ( ik handel er nu 4 af) */

						DefBut(TEX, B_TESTVAR+a, "Name:",		xco,100,120,20, ac->name, 0.0, 15.0);

						if(ac->action==SN_HOLD_VARIABLE);
						else if(ac->action==SN_TOG_VARIABLE) {
							DefBut(NUM|SHO, B_COPYVAR+a, "Init:",	xco,80,120,20, &ac->cur, -1000.0, 1000.0);
						}
						else {
							if(ac->action!=SN_IPO_VARIABLE)
								DefBut(NUM|SHO, 0, "Val:",			xco,	80,120,20, &ac->var, -1000.0, 1000.0);
							DefBut(NUM|SHO, B_COPYVAR+a, "Init:",	xco,	50,120,20, &ac->cur, -1000.0, 1000.0);
							DefBut(NUM|SHO, B_COPYVAR+a, "Min:",	xco,	30,60,20, &ac->sta, -1000.0, 1000.0);
							DefBut(NUM|SHO, B_COPYVAR+a, "Max:",	xco+60,	30,60,20, &ac->end, -1000.0, 1000.0);
							SetButCol(BUTGREEN);
							DefBut(TOG|SHO|BIT|0, B_COPYVAR+a, "Cyclic",	xco,	10,60,20, &ac->flag);
						}
						SetButCol(BUTGREEN);
						DefBut(TOGN|SHO|BIT|1, B_COPYVAR+a, "Print",	xco+60,	10,60,20, &ac->flag);
						SetButCol(BUTGREY);
					}
					
					xco+= 120;
					ac++;
				}
				freeN(strp);
			}
		}
	}
	/* all buttons: */
	SetButCol(BUTGREY);
	DefBut(BUT, B_ALLTEXMESH, "All TexMesh",		1155,200,119,19);
	DefBut(BUT, B_ALLDYNAMESH, "All DynaMesh",		1154,179,120,19);
	
	
	#endif
}

/* *************************** FACE/PAINT *************************** */

void do_fpaintbuts(short event)
{
	Mesh *me;
	Base *base;
	Object *ob;
	Sector *se;
	
	ob= OBACT;
	if(ob==0) return;

	switch(event) {
		
	case B_VPGAMMA:
		vpaint_dogamma();
		break;
	case B_COPY_TF_MODE:
	case B_COPY_TF_UV:
	case B_COPY_TF_COL:
	case B_COPY_TF_TEX:
		me= get_mesh(ob);
		if(me && me->tface) {
			extern TFace *lasttface;
			TFace *tface= me->tface;
			int a= me->totface;
			
			set_lasttface();
			if(lasttface) {
			
				while(a--) {
					if(tface!=lasttface && (tface->flag & SELECT)) {
						if(event==B_COPY_TF_MODE) {
							tface->mode= lasttface->mode;
							/* het transparante stuk */
							tface->flag &= ~TF_ABR;
							tface->flag |= (lasttface->flag & TF_ABR);
						}
						else if(event==B_COPY_TF_UV) {
							memcpy(tface->uv, lasttface->uv, sizeof(tface->uv));
							tface->tpage= lasttface->tpage;
							tface->tile= lasttface->tile;
							
							if(lasttface->mode & TF_TILES) tface->mode |= TF_TILES;
							else tface->mode &= ~TF_TILES;
							
						}
						else if(event==B_COPY_TF_TEX) {
							tface->tpage= lasttface->tpage;
							tface->tile= lasttface->tile;

							if(lasttface->mode & TF_TILES) tface->mode |= TF_TILES;
							else tface->mode &= ~TF_TILES;
						}
						else if(event==B_COPY_TF_COL) memcpy(tface->col, lasttface->col, sizeof(tface->col));
					}
					tface++;
				}
			}
			do_shared_vertexcol(me);
			allqueue(REDRAWVIEW3D, 0);
			allqueue(REDRAWIMAGE, 0);
		}
		break;
	case B_REDR_3D_IMA:
		allqueue(REDRAWVIEW3D, 0);
		allqueue(REDRAWIMAGE, 0);
		break;
	case B_HALO:
		{
			extern TFace *lasttface;

			set_lasttface();
			if(lasttface) {			
				lasttface->mode &= ~(TF_GOUR|TF_LIGHT);
				lasttface->mode |= (TF_TEX);
				allqueue(REDRAWBUTSGAME, 0);
			}
		}
		break;
	case B_TWOSIDE:
		{
			extern TFace *lasttface;

			set_lasttface();
			if(lasttface) {			
				lasttface->mode &= ~(TF_LIGHT);
				lasttface->mode |= (TF_GOUR+TF_TEX);
				allqueue(REDRAWBUTSGAME, 0);
			}
		}
		break;
	case B_OBCOLOR:
		{
			extern TFace *lasttface;

			set_lasttface();
			if(lasttface) {			
				lasttface->mode &= ~(TF_LIGHT|TF_TWOSIDE);
				allqueue(REDRAWBUTSGAME, 0);
			}
		}
		break;
	case B_MASK:
		{
			extern TFace *lasttface;

			set_lasttface();
			if(lasttface && lasttface->mode & TF_MASK) {	
				lasttface->mode = TF_MASK;
				allqueue(REDRAWBUTSGAME, 0);
			}
		}
		break;
	case B_SHADOW:
		{
			extern TFace *lasttface;

			set_lasttface();
			if(lasttface && lasttface->mode & TF_SHADOW) {	
				lasttface->mode = TF_TEX|TF_TRANSP|TF_SHADOW;
				allqueue(REDRAWBUTSGAME, 0);
			}
		}
		break;

	case B_ASSIGNMESH:
		
		test_object_materials(ob->data);
		allqueue(REDRAWVIEW3D, 0);
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_SHOWTEX:
		/* wordt door lifes en sectors gebruikt */
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
		
	}	
}

void fpaintbuts()
{
	extern VPaint Gvp;
	Object *ob;
	Sector *se;
	Life *lf;
	Mesh *me;
	int a, xco;
	char *strp, str[32], str1[32];
	
	ob= OBACT;
	if(ob==0) return;
	
	if(ob->type==OB_SECTOR) {
		se= ob->data;
		DefBut(BUT, 0, "ICON 0 10 3", 649,179,19,20);
		DefBut(BUT, 0, "ICON 0 11 3", 649,200,19,20);
		SetButFunc(test_meshpoin_but);
		DefBut(IDPOIN, B_ASSIGNMESH, "ME:",		669,200,151,20, &se->texmesh);
		SetButFunc(test_meshpoin_but);
		DefBut(IDPOIN, B_ASSIGNMESH, "ME:",		669,179,151,20, &se->dynamesh);
	
		SetButLock(G.obedit!=0);
		SetButCol(BUTBLUE);
		DefBut(TOG|SHO|BIT|0, B_SHOWTEX, "TEXMESH",		822,179,123,40, &se->flag);
		DefBut(TOGN|SHO|BIT|0, B_SHOWTEX, "DYNAMESH",	948,179,125,40, &se->flag);
		SetButCol(BUTGREY);
		DefBut(TOG|SHO|BIT|3, B_SHOWTEX, "Ghost",	1074,179,66,40, &se->flag);
	}
	else if(ob->type==OB_LIFE) { 
		lf= ob->data;
		
		DefBut(BUT, 0, "ICON 0 10 3", 649,179,19,20);
		DefBut(BUT, 0, "ICON 0 11 3", 649,200,19,20);
		SetButFunc(test_meshpoin_but);
		DefBut(IDPOIN, B_ASSIGNMESH, "ME:",		669,200,151,20, &lf->texmesh);
		SetButFunc(test_meshpoin_but);
		DefBut(IDPOIN, B_ASSIGNMESH, "ME:",		669,180,151,20, &lf->dynamesh);
	
		SetButLock(G.obedit!=0);
		SetButCol(BUTBLUE);
		DefBut(TOG|SHO|BIT|2, B_SHOWTEX, "TEXMESH",		822,180,123,40, &lf->flag);
		DefBut(TOGN|SHO|BIT|2, B_SHOWTEX, "DYNAMESH",	948,180,125,40, &lf->flag);
		SetButCol(BUTGREY);
		DefBut(TOG|SHO|BIT|3, B_SHOWTEX, "Ghost",	1074,180,66,40, &lf->flag);
	}

	/* VPAINT BUTTONS */
	SetButCol(BUTGREY);
	DefBut(LABEL, 0, "Vertex Paint",	1037,156,194,18, 19, 0, 0, 0);
	DefBut(NUMSLI|FLO, 0, "R ",			979,134,194,19, &Gvp.r, 0.0, 1.0, B_VPCOLSLI);
	DefBut(NUMSLI|FLO, 0, "G ",			979,114,194,19, &Gvp.g, 0.0, 1.0, B_VPCOLSLI);
	DefBut(NUMSLI|FLO, 0, "B ",			979,94,194,19, &Gvp.b, 0.0, 1.0, B_VPCOLSLI);
	DefBut(NUMSLI|FLO, 0, "Alpha ",		979,74,194,19, &Gvp.a, 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, 0, "Size ",		979,54,194,19, &Gvp.size, 2.0, 64.0, 0);
	
	DefBut(COL|FLO, B_VPCOLSLI, "",		1176,75,28,80, &(Gvp.r));
	
	DefBut(ROW|SHO, 0, "Mix",			1212,134,63,19, &Gvp.mode, 1.0, 0.0);
	DefBut(ROW|SHO, 0, "Add",			1212,114,63,19, &Gvp.mode, 1.0, 1.0);
	DefBut(ROW|SHO, 0, "Sub",			1212, 94,63,19, &Gvp.mode, 1.0, 2.0);
	DefBut(ROW|SHO, 0, "Mul",			1212, 74,63,19, &Gvp.mode, 1.0, 3.0);
	DefBut(ROW|SHO, 0, "Filter",		1212, 54,63,19, &Gvp.mode, 1.0, 4.0);
	
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|0, 0, "ColIndex", 	979,26,80,19, &Gvp.flag);
	DefBut(TOG|SHO|BIT|1, 0, "Area", 		1061,26,76,19, &Gvp.flag);
	DefBut(TOG|SHO|BIT|2, 0, "Soft", 		1139,26,67,19, &Gvp.flag);
	DefBut(TOG|SHO|BIT|3, 0, "Normals", 	1208,26,67,19, &Gvp.flag);
	
	SetButCol(BUTSALMON);
	DefBut(BUT, B_VPGAMMA, "Set", 	980,6,80,19);
	SetButCol(BUTGREY);
	DefBut(NUM|FLO, 0, "Mul:", 		1061,6,112,19, &Gvp.mul, 0.1, 50.0, 10);
	DefBut(NUM|FLO, 0, "Gamma:", 	1174,6,102,19, &Gvp.gamma, 0.1, 5.0, 10);

	DefBut(LABEL, 0, "Face Select",	700,156,194,18, 19, 0, 0, 0);
	if(G.f & G_FACESELECT) {
		extern TFace *lasttface;
		char *cp;
		
		set_lasttface();
		if(lasttface) {
			
			SetButCol(BUTGREEN);
			DefBut(TOG|SHO|BIT|1, REDRAWVIEW3D, "Gour",		650,134,64,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|2, B_REDR_3D_IMA, "Tex",		715,134,55,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|6, REDRAWVIEW3D,	"Shared",	771,134,68,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|4, REDRAWVIEW3D, "Light",	840,134,65,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|7, B_REDR_3D_IMA, "Tiles",	906,134,55,19, &lasttface->mode);
			
			cp= (char *)&lasttface->flag;	
			DefBut(TOG|SHO|BIT|5, B_REDR, "Transp",	650,110,64,19, &lasttface->mode);
			if(lasttface->mode & TF_TRANSP) {
				DefBut(ROW|CHA, 0, "Mix",			715,110,55,19, cp, 2.0, 0.0);
				DefBut(ROW|CHA, 0, "Add",			771,110,68,19, cp,	2.0, 1.0);
				DefBut(ROW|CHA, 0, "Sub",			840,110,65,19, cp,	2.0, 2.0);
				DefBut(ROW|CHA, 0, "Add 1/4",		906,110,55,19, cp, 2.0, 3.0);
			}
			DefBut(TOG|SHO|BIT|8, B_HALO, "Halo",	650,86,64,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|9, B_TWOSIDE, "Twoside",	715,86,55,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|11, B_OBCOLOR, "ObColor",777,86,63,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|12, B_MASK, "Mask",842,86,55,19, &lasttface->mode);
			DefBut(TOG|SHO|BIT|13, B_SHADOW, "Shadow",897,86,64,19, &lasttface->mode);
		}
	}
	SetButCol(BUTSALMON);
	DefBut(BUT, B_COPY_TF_MODE, "Copy DrawMode", 650,7,117,28);
	DefBut(BUT, B_COPY_TF_UV, "Copy UV+tex",		771,7,85,28);
	DefBut(BUT, B_COPY_TF_COL, "Copy VertCol",	859,7,103,28);
}

/* *************************** MBALL ******************************** */

void do_mballbuts(short event)
{
	extern MetaElem *lastelem;

	switch(event) {
	case B_RECALCMBALL:
		makeDispList(OBACT);
		allqueue(REDRAWVIEW3D, 0);
		break;
	}
}

void mballbuts()
{
	extern MetaElem *lastelem;
	MetaBall *mb;
	Object *ob;
	
	ob= OBACT;
	if(ob==0) return;
	
	mb= ob->data;
	
	if( has_id_number((ID *)ob)==0 ) {
		DefBut(NUMSLI|FLO, B_RECALCMBALL, "Wiresize:",	470,178,250,19, &mb->wiresize, 0.05, 1.0);
		DefBut(NUMSLI|FLO, 0, "Rendersize:",			470,158,250,19, &mb->rendersize, 0.05, 1.0);
		DefBut(NUMSLI|FLO, B_RECALCMBALL, "Threshold:", 470,138,250,19, &mb->thresh, 0.0001, 5.0);

		SetButCol(BUTBLUE);
		DefBut(LABEL, 0, "Update:",		438,108,120,19, 0, 0, 0);
		DefBut(ROW|SHO, 0, "Always",	471, 85, 120, 19, &mb->flag, 0.0, 0.0);
		DefBut(ROW|SHO, 0, "Half Res",	471, 65, 120, 19, &mb->flag, 0.0, 1.0);
		DefBut(ROW|SHO, 0, "Fast",		471, 45, 120, 19, &mb->flag, 0.0, 2.0);
		SetButCol(BUTGREY);
	}
	
	if(ob==G.obedit && lastelem) {
		DefBut(NUMSLI|FLO, B_RECALCMBALL, "Stiffness:", 750,178,250,19, &lastelem->s, 0.0, 10.0);
		DefBut(NUMSLI|FLO, B_RECALCMBALL, "Len:",		750,158,250,19, &lastelem->len, 0.0, 20.0);

		SetButCol(BUTGREEN);
		DefBut(TOG|SHO|BIT|1, B_RECALCMBALL, "Negative",752,116,60,19, &lastelem->flag);

		DefBut(ROW|SHO, B_RECALCMBALL, "Ball",			753,83,60,19, &lastelem->type, 1.0, 0.0);
		DefBut(ROW|SHO, B_RECALCMBALL, "TubeX",			753,62,60,19, &lastelem->type, 1.0, 1.0);
		DefBut(ROW|SHO, B_RECALCMBALL, "TubeY",			814,62,60,19, &lastelem->type, 1.0, 2.0);
		DefBut(ROW|SHO, B_RECALCMBALL, "TubeZ",			876,62,60,19, &lastelem->type, 1.0, 3.0);

	}
}

/* *************************** IKA ******************************** */

void do_ikabuts(short event)
{
	Base *base;
	Object *ob;
	
	ob= OBACT;
	
	switch(event) {
	case B_IKASETREF:
		base= FIRSTBASE;
		while(base) {
			if TESTBASELIB(base) {
				if(base->object->type==OB_IKA) init_defstate_ika(base->object);
			}
			base= base->next;
		}
		break;	
	case B_IKARECALC:
		itterate_ika(ob);
		break;
	}
	
}

void ikabuts()
{
	Ika *ika;
	Object *ob;
	Limb *li;
	Deform *def;
	int nr, cury;
	char str[32];
	
	ob= OBACT;
	if(ob==0) return;
	ika= ob->data;
	
	DefBut(NUMSLI|FLO, 0, "Mem ",			470,78,200,20, &ika->mem, 0.0, 1.0, 0);
	/* DefBut(NUMSLI|FLO, 0, "Slow eff ",		470,58,200,20, &ika->slow, 0.0, 1.0, 0); */
	DefBut(NUM|SHO, 0, "Iter: ",			470,38,200,20, &ika->iter, 2.0, 16.0, 0);

	SetButCol(BUTSALMON);
	DefBut(BUT, B_IKASETREF,	"Set Reference",		470,118,100,19);
	
	SetButCol(BUTGREY);
	DefBut(LABEL, 0, "Limb Weight",			680, 200, 150, 19, 0, 0, 0);
	nr= 0;
	cury= 180;
	li= ika->limbbase.first;
	while(li) {
		sprintf(str, "Limb %d:", nr);
		DefBut(NUM|FLO, 0, str,			680, cury, 150, 19, &li->fac, 0.01, 1.0, 10);
		nr++;
		cury-= 20;
		li= li->next;
	}
	
	DefBut(LABEL, 0, "Skeleton Weight",			880, 200, 150, 19, 0, 0, 0);
	nr= ika->totdef;
	cury= 180;
	def= ika->def;
	while(nr--) {
		/* tot 1 juli 1996 */
		if(def->fac==0.0) def->fac= 1.0;
		if(def->ob) {
			if(def->ob->type!=OB_IKA) sprintf(str, "%s   :", def->ob->id.name+2);
			else sprintf(str, "%s (%d):", def->ob->id.name+2, def->par1);
		}
		DefBut(NUM|FLO, 0, str,			880, cury, 200, 19, &def->fac, 0.01, 10.0, 10);
		def++;
		cury-= 20;
	}
}

/* *************************** LATTICE ******************************** */

void do_latticebuts(short event)
{
	Object *ob;
	Lattice *lt;
	Base *base;
	
	ob= OBACT;
	
	switch(event) {
	case B_RESIZELAT:
		if(ob) {
			if(ob==G.obedit) resizelattice(editLatt);
			else resizelattice(ob->data);
		}
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_DRAWLAT:
		if(ob==G.obedit) calc_lattverts_ext();
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_LATTCHANGED:
		
		lt= ob->data;
		if(lt->flag & LT_OUTSIDE) outside_lattice(lt);
		
		base= FIRSTBASE;
		while(base) {
			if(base->object->parent==ob) makeDispList(base->object);
			base= base->next;
		}
		allqueue(REDRAWVIEW3D, 0);
		
		break;
	}
}

void latticebuts()
{
	Lattice *lt;
	Object *ob;
	
	ob= OBACT;
	if(ob==0) return;
	if(ob==G.obedit) lt= editLatt;
	else lt= ob->data;

	SetButLock(lt->key!=0);
	SetButLock(ob==G.obedit);
	DefBut(NUM|SHO, B_RESIZELAT,	"U:",			470,178,100,19, &lt->pntsu, 1.0, 64.0);
	DefBut(NUM|SHO, B_RESIZELAT,	"V:",			470,158,100,19, &lt->pntsv, 1.0, 64.0);
	DefBut(NUM|SHO, B_RESIZELAT,	"W:",			470,138,100,19, &lt->pntsw, 1.0, 64.0);
	ClearButLock();
	
	SetButCol(BUTGREEN);
	DefBut(ROW|CHA, B_LATTCHANGED,		"Lin",		572, 178, 40, 19, &lt->typeu, 1.0, (float)KEY_LINEAR);
	DefBut(ROW|CHA, B_LATTCHANGED,		"Card",		612, 178, 40, 19, &lt->typeu, 1.0, (float)KEY_CARDINAL);
	DefBut(ROW|CHA, B_LATTCHANGED,		"B",		652, 178, 40, 19, &lt->typeu, 1.0, (float)KEY_BSPLINE);

	DefBut(ROW|CHA, B_LATTCHANGED,		"Lin",		572, 158, 40, 19, &lt->typev, 2.0, (float)KEY_LINEAR);
	DefBut(ROW|CHA, B_LATTCHANGED,		"Card",		612, 158, 40, 19, &lt->typev, 2.0, (float)KEY_CARDINAL);
	DefBut(ROW|CHA, B_LATTCHANGED,		"B",		652, 158, 40, 19, &lt->typev, 2.0, (float)KEY_BSPLINE);

	DefBut(ROW|CHA, B_LATTCHANGED,		"Lin",		572, 138, 40, 19, &lt->typew, 3.0, (float)KEY_LINEAR);
	DefBut(ROW|CHA, B_LATTCHANGED,		"Card",		612, 138, 40, 19, &lt->typew, 3.0, (float)KEY_CARDINAL);
	DefBut(ROW|CHA, B_LATTCHANGED,		"B",		652, 138, 40, 19, &lt->typew, 3.0, (float)KEY_BSPLINE);
	
	SetButCol(BUTSALMON);
	DefBut(BUT, B_RESIZELAT,	"Make Regular",		470,101,99,32);

	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|1, B_LATTCHANGED, "Outside",	571,101,120,31, &lt->flag);
}


/* *************************** TEXTURE ******************************** */

Tex *cur_imatex=0;
int prv_win= 0;

void load_tex_image(char *str)	/* aangeroepen vanuit fileselect */
{
	Image *ima=0;
	Tex *tex;
	
	tex= cur_imatex;
	if(tex->type!=TEX_IMAGE) return;

	ima= add_image(str);
	if(ima) {
		if(tex->ima) {
			tex->ima->id.us--;
		}
		tex->ima= ima;
		
		free_image(ima);	/* forceer opnieuw inlezen */
		ima->ok= 1;
	}
	
	allqueue(REDRAWBUTSTEX, 0);
	allqueue(REDRAWPAINT, 0);
	
	preview_changed(prv_win);
}

void load_plugin_tex(char *str)	/* aangeroepen vanuit fileselect */
{
	PluginTex *pit;
	Tex *tex;
	
	tex= cur_imatex;
	if(tex->type!=TEX_PLUGIN) return;
	
	if(tex->plugin) free_plugin_tex(tex->plugin);
	
	tex->stype= 0;
	tex->plugin= add_plugin_tex(str);

	allqueue(REDRAWBUTSTEX, 0);
	preview_changed(prv_win);
}

long vergcband(x1, x2)
CBData *x1, *x2;
{

	if( x1->pos > x2->pos ) return 1;
	else if( x1->pos < x2->pos) return -1;
	return 0;
}

void drawcolorband(ColorBand *coba, float x1, float y1, float sizex, float sizey)
{
	CBData *cbd;
	float v3[2], v1[2], v2[2];
	int a;
	
	if(coba==0) return;
	
	/* outline */
	v1[0]= x1; v1[1]= y1;
	linewidth(3);
	cpack(0x0);
	bgnclosedline();
		v2f(v1);
		v1[0]+= sizex;
		v2f(v1);
		v1[1]+= sizey;
		v2f(v1);
		v1[0]-= sizex;
		v2f(v1);
	endclosedline();
	if(G.machine==ENTRY) linewidth(2); else linewidth(1);


	shademodel(GOURAUD);
	cbd= coba->data;
	
	v1[0]= v2[0]= x1;
	v1[1]= y1;
	v2[1]= y1+sizey;
	
	bgnqstrip();
	
	c3f( &cbd->r );
	v2f(v1); v2f(v2);
	
	for(a=0; a<coba->tot; a++, cbd++) {
		
		v1[0]=v2[0]= x1+ cbd->pos*sizex;

		c3f( &cbd->r );
		v2f(v1); v2f(v2);
	}
	
	v1[0]=v2[0]= x1+ sizex;
	v2f(v1); v2f(v2);
	
	endqstrip();
	shademodel(FLAT);
	
	/* hulplijntjes */
	
	v1[0]= v2[0]=v3[0]= x1;
	v1[1]= y1;
	v2[1]= y1+0.5*sizey;
	v3[1]= y1+sizey;
	
	cbd= coba->data;
	for(a=0; a<coba->tot; a++, cbd++) {
		v1[0]=v2[0]=v3[0]= x1+ cbd->pos*sizex;
		
		if(a==coba->cur) linewidth(3);
		cpack(0x0);
		LINE2F(v1, v2);

		cpack(0xFFFFFF);
		LINE2F(v2, v3);
		
		if(a==coba->cur) {
			if(G.machine==ENTRY) linewidth(2); else linewidth(1);
		}
	}
}


void do_texbuts(short event)
{
	Tex *tex;
	ImBuf *ibuf;
	ScrArea *sa;
	ID *id, *idtest;
	CBData *cbd;
	float dx;
	int a, nr;
	short mvalo[2], mval[2];
	char *name, str[80];
	
	tex= G.buts->lockpoin;
	
	switch(event) {
	case B_TEXCHANNEL:
		addqueue(curarea->headwin, REDRAW, 1);
		preview_changed(curarea->win);
		allqueue(REDRAWBUTSTEX, 0);
		break;
	case B_TEXTYPE:
		if(tex==0) return;
		tex->stype= 0;
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;
	case B_DEFTEXVAR:
		if(tex==0) return;
		default_tex(tex);
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;
	case B_LOADTEXIMA:
	case B_LOADTEXIMA1:
		if(tex==0) return;
		/* globals: even onthouden: we maken andere area fileselect */
		cur_imatex= tex;
		prv_win= curarea->win;
		
		sa= closest_bigger_area();
		areawinset(sa->win);
		if(tex->ima) name= tex->ima->name;
		else name= U.textudir;
		
		if(event==B_LOADTEXIMA)
			activate_imageselect(FILE_SPECIAL, "SELECT IMAGE", name, load_tex_image);
		else 
			activate_fileselect(FILE_SPECIAL, "SELECT IMAGE", name, load_tex_image);
		
		break;
	case B_NAMEIMA:
		if(tex==0) return;
		if(tex->ima) {
			cur_imatex= tex;
			prv_win= curarea->win;
			
			/* naam in tex->ima is door button veranderd! */
			strcpy(str, tex->ima->name);
			if(tex->ima->ibuf) strcpy(tex->ima->name, tex->ima->ibuf->name);

			load_tex_image(str);
		}
		break;
	case B_TEXREDR_PRV:
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;
	case B_TEXIMABROWSE:
		if(tex) {
		
			if(G.buts->menunr== -2) {
				activate_databrowse(tex->ima, ID_IM, 0, B_TEXIMABROWSE, do_texbuts);
				return;
			}
			if(G.buts->menunr < 0) break;
		
			nr= 0;
			id= (ID *)tex->ima;

			idtest= G.main->image.first;
			while(idtest) {
				if(nr==G.buts->menunr) {
					break;
				}
				nr++;
				idtest= idtest->next;
			}
			if(idtest==0) {	/* geen new */
				return;
			}
		
			if(idtest!=id) {
				tex->ima= (Image *)idtest;
				id_us_plus(idtest);
				if(id) id->us--;
				
				allqueue(REDRAWBUTSTEX, 0);
				preview_changed(curarea->win);
			}
		}
		break;
	case B_IMAPTEST:
		if(tex) {
			if( (tex->imaflag & (TEX_FIELDS+TEX_MIPMAP))== TEX_FIELDS+TEX_MIPMAP ) {
				error("Cannot combine fields and mipmap");
				tex->imaflag -= TEX_MIPMAP;
				allqueue(REDRAWBUTSTEX, 0);
			}
			
			if(tex->ima && tex->ima->ibuf) {
				ibuf= tex->ima->ibuf;
				nr= 0;
				if( !(tex->imaflag & TEX_FIELDS) && (ibuf->flags & IB_fields) ) nr= 1;
				if( (tex->imaflag & TEX_FIELDS) && !(ibuf->flags & IB_fields) ) nr= 1;
				if(nr) {
					freeImBuf(ibuf);
					tex->ima->ibuf= 0;
					tex->ima->ok= 1;
					preview_changed(curarea->win);
				}
			}
		}
		break;
	case B_RELOADIMA:
		if(tex && tex->ima) {
			freeImBuf(tex->ima->ibuf);
			tex->ima->ibuf= 0;
			tex->ima->ok= 1;
			allqueue(REDRAWBUTSTEX, 0);
			preview_changed(curarea->win);
		}
		break;
		
	case B_LOADPLUGIN:
		if(tex==0) return;
		#ifdef FREE
			notice("Not in this version");
		#else
			/* globals: even onthouden: we maken andere area fileselect */
			cur_imatex= tex;
			prv_win= curarea->win;
			
			sa= closest_bigger_area();
			areawinset(sa->win);
			if(tex->plugin) strcpy(str, tex->plugin->name);
			else {
				strcpy(str, U.textudir);
				strcat(str, "proc/");
			}
			activate_fileselect(FILE_SPECIAL, "SELECT PLUGIN", str, load_plugin_tex);
		#endif
		break;

	case B_NAMEPLUGIN:
		if(tex==0 || tex->plugin==0) return;
		strcpy(str, tex->plugin->name);
		free_plugin_tex(tex->plugin);
		tex->stype= 0;
		tex->plugin= add_plugin_tex(str);
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;
		
	case B_COLORBAND:
		if(tex==0) return;
		if(tex->coba==0) tex->coba= add_colorband();
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;
	
	case B_ADDCOLORBAND:
		if(tex==0 || tex->coba==0) return;
		
		if(tex->coba->tot < MAXCOLORBAND-1) tex->coba->tot++;
		tex->coba->cur= tex->coba->tot-1;
		
		do_texbuts(B_CALCCBAND);
		
		break;

	case B_DELCOLORBAND:
		if(tex==0 || tex->coba==0 || tex->coba->tot<2) return;
		
		for(a=tex->coba->cur; a<tex->coba->tot; a++) {
			tex->coba->data[a]= tex->coba->data[a+1];
		}
		if(tex->coba->cur) tex->coba->cur--;
		tex->coba->tot--;

		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		break;

	case B_CALCCBAND:
	case B_CALCCBAND2:
		if(tex==0 || tex->coba==0 || tex->coba->tot<2) return;
		
		for(a=0; a<tex->coba->tot; a++) tex->coba->data[a].cur= a;
		qsort(tex->coba->data, tex->coba->tot, sizeof(CBData), (void *)vergcband);
		for(a=0; a<tex->coba->tot; a++) {
			if(tex->coba->data[a].cur==tex->coba->cur) {
				if(tex->coba->cur!=a) addqueue(curarea->win, REDRAW, 0);	/* button cur */
				tex->coba->cur= a;
				break;
			}
		}
		if(event==B_CALCCBAND2) return;
		
		allqueue(REDRAWBUTSTEX, 0);
		preview_changed(curarea->win);
		
		break;
		
	case B_DOCOLORBAND:
		if(tex==0 || tex->coba==0) return;
		
		cbd= tex->coba->data + tex->coba->cur;
		ButtonsGetmouse(mvalo);
		
		while(getbutton(LEFTMOUSE)) {
			ButtonsGetmouse(mval);
			if(mval[0]!=mvalo[0]) {
				dx= mval[0]-mvalo[0];
				dx/= 345.0;
				cbd->pos+= dx;
				CLAMP(cbd->pos, 0.0, 1.0);
				
				frontbuffer(TRUE);
				drawcolorband(tex->coba, 923,81,345,20);
				SetButs(B_CALCCBAND, B_CALCCBAND);
				frontbuffer(FALSE);
				
				do_texbuts(B_CALCCBAND2);
				cbd= tex->coba->data + tex->coba->cur;	/* ivm qsort */
				
				mvalo[0]= mval[0];
			}
			sginap(2);
		}
		preview_changed(curarea->win);
		
		break;
	
	case B_REDRAWCBAND:
		frontbuffer(TRUE);
		drawcolorband(tex->coba, 923,81,345,20);
		frontbuffer(FALSE);
		preview_changed(curarea->win);
		break;
	}
}


void texbuts()
{
	Object *ob;
	Material *ma=0;
	World *wrld=0;
	Lamp *la=0;
	ID *id;
	MTex *mtex;
	Tex *tex;
	VarStruct *varstr;
	PluginTex *pit;
	CBData *cbd;
	int a, xco, yco, loos, type;
	char str[30], *strp, *plugname, **stname;
	
	SetButCol(BUTSALMON);

	DefBut(ROW|CHA, B_TEXREDR_PRV, "Mat",		200,172,40,20, &G.buts->texfrom, 3.0, 0.0);
	DefBut(ROW|CHA, B_TEXREDR_PRV, "World",		240,172,52,20, &G.buts->texfrom, 3.0, 1.0);
	DefBut(ROW|CHA, B_TEXREDR_PRV, "Lamp",		292,172,46,20, &G.buts->texfrom, 3.0, 2.0);
	SetButCol(BUTGREY);
	
	if(G.buts->texfrom==0) {
		ob= OBACT;
		if(ob==0) return;
		id= ob->data;
		if(id==0) return;
	
		ma= give_current_material(ob, ob->actcol);
		if(ma==0) return;
		
	}
	else if(G.buts->texfrom==1) {
		wrld= G.scene->world;
		if(wrld==0) return;
		id= (ID *)wrld;
	}
	else if(G.buts->texfrom==2) {
		ob= OBACT;
		if(ob==0) return;
		if(ob->type!=OB_LAMP) return;
		la= ob->data;
		id= (ID *)la;
	}
	
	SetButLock(id->lib!=0);

	/* CHANNELS */
	yco= 140;
	for(a= 0; a<8; a++) {
		if(G.buts->texfrom==0) mtex= ma->mtex[a];
		else if(G.buts->texfrom==1) mtex= wrld->mtex[a];
		else if(G.buts->texfrom==2)  mtex= la->mtex[a];
		
		if(mtex && mtex->tex) splitIDname(mtex->tex->id.name+2, str, &loos);
		else strcpy(str, "");
		str[14]= 0;
		if(G.buts->texfrom==0) {
			DefBut(ROW|CHA, B_TEXCHANNEL, str,			200,yco,140,18, &(ma->texact), 0.0, (float)a);
		}
		else if(G.buts->texfrom==1) {
			DefBut(ROW|SHO, B_TEXCHANNEL, str,			200,yco,140,18, &(wrld->texact), 0.0, (float)a);
			if(a==5) break;
		}
		else if(G.buts->texfrom==2) {
			DefBut(ROW|SHO, B_TEXCHANNEL, str,			200,yco,140,18, &(la->texact), 0.0, (float)a);
			if(a==5) break;
		}
		yco-= 19;
	}
	
	SetButFunc(test_idbutton);
	if(G.buts->texfrom==0) {
		DefBut(TEX, B_IDNAME, "MA:",					200,195,140,20, ma->id.name+2, 0.0, 18.0);
		mtex= ma->mtex[ ma->texact ];
	}
	else if(G.buts->texfrom==1) {
		DefBut(TEX, B_IDNAME, "WO:",					200,195,140,20, wrld->id.name+2, 0.0, 18.0);
		mtex= wrld->mtex[ wrld->texact ];
	}
	else if(G.buts->texfrom==2) {
		DefBut(TEX, B_IDNAME, "LA:",					200,195,140,20, la->id.name+2, 0.0, 18.0);
		mtex= la->mtex[ la->texact ];
	}

	if(mtex==0 || mtex->tex==0) return;
	tex= mtex->tex;

	SetButLock(tex->id.lib!=0);

	DefBut(ROW|SHO, B_TEXTYPE, texstr[0],			350, 195, 75, 20, &tex->type, 1.0, 0.0);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_IMAGE],	425, 195, 75, 20, &tex->type, 1.0, (float)TEX_IMAGE);
	if(tex->plugin && tex->plugin->doit) strp= tex->plugin->pname; else strp= texstr[TEX_PLUGIN];
	DefBut(ROW|SHO, B_TEXTYPE, strp,				500, 195, 75, 20, &tex->type, 1.0, (float)TEX_PLUGIN);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_CLOUDS],	575, 195, 75, 20, &tex->type, 1.0, (float)TEX_CLOUDS);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_WOOD],	650, 195, 75, 20, &tex->type, 1.0, (float)TEX_WOOD);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_MARBLE],	725, 195, 75, 20, &tex->type, 1.0, (float)TEX_MARBLE);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_MAGIC],	800, 195, 75, 20, &tex->type, 1.0, (float)TEX_MAGIC);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_BLEND],	875, 195, 75, 20, &tex->type, 1.0, (float)TEX_BLEND);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_STUCCI],	950, 195, 75, 20, &tex->type, 1.0, (float)TEX_STUCCI);
	DefBut(ROW|SHO, B_TEXTYPE, texstr[TEX_NOISE],	1025, 195, 75, 20, &tex->type, 1.0, (float)TEX_NOISE);
	
	/* TYPES */
	SetButCol(BUTGREEN);	
	switch(tex->type) {
	case TEX_CLOUDS:
		DefBut(ROW|SHO, B_MATPRV, "Default",	350, 170, 75, 18, &tex->stype, 2.0, 0.0); 
		DefBut(ROW|SHO, B_MATPRV, "Color",		425, 170, 75, 18, &tex->stype, 2.0, 1.0); 
		SetButCol(BUTGREY);	
		DefBut(NUM|FLO, B_MATPRV, "NoiseSize :",	350, 110, 150, 19, &tex->noisesize, 0.0001, 2.0, 10);
		DefBut(NUM|SHO, B_MATPRV, "NoiseDepth:",	350, 90, 150, 19, &tex->noisedepth, 0.0, 6.0);
		SetButCol(BUTGREEN);
		DefBut(ROW|SHO, B_MATPRV, "Soft noise",		350, 40, 100, 19, &tex->noisetype, 12.0, 0.0);
		DefBut(ROW|SHO, B_MATPRV, "Hard noise",		450, 40, 100, 19, &tex->noisetype, 12.0, 1.0);
		break;

	case TEX_WOOD:
		DefBut(ROW|SHO, B_MATPRV, "Bands",		350, 170, 75, 18, &tex->stype, 2.0, 0.0); 
		DefBut(ROW|SHO, B_MATPRV, "Rings",		425, 170, 75, 18, &tex->stype, 2.0, 1.0); 
		DefBut(ROW|SHO, B_MATPRV, "BandNoise",	500, 170, 75, 18, &tex->stype, 2.0, 2.0); 
		DefBut(ROW|SHO, B_MATPRV, "RingNoise",	575, 170, 75, 18, &tex->stype, 2.0, 3.0); 
		SetButCol(BUTGREY);	
		DefBut(NUM|FLO, B_MATPRV, "NoiseSize :",	350, 110, 150, 19, &tex->noisesize, 0.0001, 2.0, 10);
		DefBut(NUM|FLO, B_MATPRV, "Turbulence:",	350, 90, 150, 19, &tex->turbul, 0.0, 200.0, 10);
		SetButCol(BUTGREEN);
		DefBut(ROW|SHO, B_MATPRV, "Soft noise",		350, 40, 100, 19, &tex->noisetype, 12.0, 0.0);
		DefBut(ROW|SHO, B_MATPRV, "Hard noise",		450, 40, 100, 19, &tex->noisetype, 12.0, 1.0);
		break;

	case TEX_MARBLE:
		DefBut(ROW|SHO, B_MATPRV, "Soft",		350, 170, 75, 18, &tex->stype, 2.0, 0.0); 
		DefBut(ROW|SHO, B_MATPRV, "Sharp",		425, 170, 75, 18, &tex->stype, 2.0, 1.0); 
		DefBut(ROW|SHO, B_MATPRV, "Sharper",	500, 170, 75, 18, &tex->stype, 2.0, 2.0); 
		SetButCol(BUTGREY);	
		DefBut(NUM|FLO, B_MATPRV, "NoiseSize :",	350, 110, 150, 19, &tex->noisesize, 0.0001, 2.0, 10);
		DefBut(NUM|SHO, B_MATPRV, "NoiseDepth:",	350, 90, 150, 19, &tex->noisedepth, 0.0, 6.0);
		DefBut(NUM|FLO, B_MATPRV, "Turbulence:",	350, 70, 150, 19, &tex->turbul, 0.0, 200.0, 10);
		SetButCol(BUTGREEN);
		DefBut(ROW|SHO, B_MATPRV, "Soft noise",		350, 40, 100, 19, &tex->noisetype, 12.0, 0.0);
		DefBut(ROW|SHO, B_MATPRV, "Hard noise",		450, 40, 100, 19, &tex->noisetype, 12.0, 1.0);
		break;

	case TEX_MAGIC:
		SetButCol(BUTGREY);	
		DefBut(NUM|FLO, B_MATPRV, "Size :",			350, 110, 150, 19, &tex->noisesize, 0.0001, 2.0, 10);
		DefBut(NUM|FLO, B_MATPRV, "Turbulence:",	350, 90, 150, 19, &tex->turbul, 0.0, 200.0, 10);
		break;

	case TEX_BLEND:
		DefBut(ROW|SHO, B_MATPRV, "Lin",		350, 170, 75, 18, &tex->stype, 2.0, 0.0); 
		DefBut(ROW|SHO, B_MATPRV, "Quad",		425, 170, 75, 18, &tex->stype, 2.0, 1.0); 
		DefBut(ROW|SHO, B_MATPRV, "Ease",		500, 170, 75, 18, &tex->stype, 2.0, 2.0); 
		DefBut(ROW|SHO, B_MATPRV, "Diag",		575, 170, 75, 18, &tex->stype, 2.0, 3.0); 		
		DefBut(ROW|SHO, B_MATPRV, "Sphere",		650, 170, 75, 18, &tex->stype, 2.0, 4.0); 		
		DefBut(ROW|SHO, B_MATPRV, "Halo",		725, 170, 75, 18, &tex->stype, 2.0, 5.0);
		
		DefBut(TOG|SHO|BIT|1, B_MATPRV, "Flip XY",	350, 130, 75, 18, &tex->flag);
		break;
		
	case TEX_STUCCI:
		DefBut(ROW|SHO, B_MATPRV, "Plastic",	350, 170, 75, 18, &tex->stype, 2.0, 0.0); 
		DefBut(ROW|SHO, B_MATPRV, "Wall In",	425, 170, 75, 18, &tex->stype, 2.0, 1.0); 		
		DefBut(ROW|SHO, B_MATPRV, "Wall Out",	500, 170, 75, 18, &tex->stype, 2.0, 2.0); 		
		SetButCol(BUTGREY);	
		DefBut(NUM|FLO, B_MATPRV, "NoiseSize :",	350, 110, 150, 19, &tex->noisesize, 0.0001, 2.0, 10);
		DefBut(NUM|FLO, B_MATPRV, "Turbulence:",	350, 90, 150, 19, &tex->turbul, 0.0, 200.0, 10);
		SetButCol(BUTGREEN);
		DefBut(ROW|SHO, B_MATPRV, "Soft noise",		350, 40, 100, 19, &tex->noisetype, 12.0, 0.0);
		DefBut(ROW|SHO, B_MATPRV, "Hard noise",		450, 40, 100, 19, &tex->noisetype, 12.0, 1.0);

		break;

	case TEX_NOISE:
		break;

	case TEX_IMAGE:
		
		break;
	}
	
	SetButCol(BUTSALMON);
	DefBut(BUT, B_DEFTEXVAR, "Default Vars",	1180,169,93,47);
	
	SetButCol(BUTGREY);
	/* SPECIFIC */
	if(tex->type==TEX_IMAGE) {
		DefBut(NUM|FLO, B_REDR, "MinX ",		350,30,140,19, &tex->cropxmin, -10.0, 10.0, 10);
		DefBut(NUM|FLO, B_REDR, "MaxX ",		350,10,140,19, &tex->cropxmax, -10.0, 10.0, 10);
		DefBut(NUM|FLO, B_REDR, "MinY ",		494,30,140,19, &tex->cropymin, -10.0, 10.0, 10);
		DefBut(NUM|FLO, B_REDR, "MaxY ",		494,10,140,19, &tex->cropymax, -10.0, 10.0, 10);


		DefBut(ROW|SHO, 0, "Extend",			350,85,69,19, &tex->extend, 4.0, 1.0);
		DefBut(ROW|SHO, 0, "Clip",				421,85,59,19, &tex->extend, 4.0, 2.0);
		DefBut(ROW|SHO, 0, "Repeat",			565,85,68,19, &tex->extend, 4.0, 3.0);
		DefBut(ROW|SHO, 0, "ClipCube",			482,85,82,19, &tex->extend, 4.0, 4.0);


		DefBut(NUM|SHO, B_MATPRV, "Xrepeat:",	350,60,140,19, &tex->xrepeat, 1.0, 512.0);
		DefBut(NUM|SHO, B_MATPRV, "Yrepeat:",	494,60,140,19, &tex->yrepeat, 1.0, 512.0);

		DefBut(NUM|FLO, B_MATPRV, "Filter :",	352,109,135,19, &tex->filtersize, 0.1, 25.0);
		
		DefBut(NUM|SHO, B_MATPRV, "Frames :",	642,110,150,19, &tex->frames, 0.0, 18000.0);
		DefBut(NUM|SHO, B_MATPRV, "Offset :",	642,90,150,19, &tex->offset, -9000.0, 9000.0);
		DefBut(NUM|SHO, B_MATPRV, "Fie/Ima:",	642,60,98,19, &tex->fie_ima, 1.0, 200.0);
		DefBut(NUM|SHO, B_MATPRV, "StartFr:",	642,30,150,19, &tex->sfra, 1.0, 9000.0);
		DefBut(NUM|SHO, B_MATPRV, "Len:",		642,10,150,19, &tex->len, 0.0, 9000.0);

		DefBut(NUM|SHO, B_MATPRV, "Fra:",		802,70,73,19, &(tex->fradur[0][0]), 0.0, 18000.0);
		DefBut(NUM|SHO, B_MATPRV, "",			879,70,37,19, &(tex->fradur[0][1]), 0.0, 250.0);
		DefBut(NUM|SHO, B_MATPRV, "Fra:",		802,50,73,19, &(tex->fradur[1][0]), 0.0, 18000.0);
		DefBut(NUM|SHO, B_MATPRV, "",			879,50,37,19, &(tex->fradur[1][1]), 0.0, 250.0);
		DefBut(NUM|SHO, B_MATPRV, "Fra:",		802,30,73,19, &(tex->fradur[2][0]), 0.0, 18000.0);
		DefBut(NUM|SHO, B_MATPRV, "",			879,30,37,19, &(tex->fradur[2][1]), 0.0, 250.0);
		DefBut(NUM|SHO, B_MATPRV, "Fra:",		802,10,73,19, &(tex->fradur[3][0]), 0.0, 18000.0);
		DefBut(NUM|SHO, B_MATPRV, "",			879,10,37,19, &(tex->fradur[3][1]), 0.0, 250.0);

		SetButCol(BUTGREEN);
		DefBut(TOG|SHO|BIT|6, 0, "Cyclic",		743,60,48,19, &tex->imaflag);
		
		SetButCol(BUTSALMON);
		DefBut(BUT, B_LOADTEXIMA, "Load Image", 350,137,132,24);
		SetButCol(BUTGREY);
		DefBut(BUT, B_LOADTEXIMA1, "", 485,137,10,24);

		id= (ID *)tex->ima;
		IDnames_to_pupstring(&strp, &(G.main->image), id, &(G.buts->menunr));
		if(strp[0]) {
			DefBut(MENU|SHO, B_TEXIMABROWSE, strp, 496,137,23,24, &(G.buts->menunr));
		}
		freeN(strp);

		if(tex->ima) {
			DefBut(TEX, B_NAMEIMA, "",			520,137,412,24, tex->ima->name, 0.0, 79.0);
			sprintf(str, "%d", tex->ima->id.us);
			DefBut(BUT, 0, str,					934,137,23,24);
			DefBut(BUT, B_RELOADIMA, "Reload",	960,137,65,24);
		}
		
		SetButCol(BUTGREEN);
		DefBut(TOG|SHO|BIT|0, 0, "InterPol",			350, 170, 75, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|1, B_MATPRV, "UseAlpha",		425, 170, 75, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|5, B_MATPRV, "CalcAlpha",	500, 170, 75, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|2, B_MATPRV, "NegAlpha",		575, 170, 75, 18, &tex->flag);
		DefBut(TOG|SHO|BIT|2, B_IMAPTEST, "MipMap",		650, 170, 75, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|12, B_IMAPTEST, "Genlock",	725, 170, 75, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|3, B_IMAPTEST, "Fields",		800, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|4, B_MATPRV, "Rot90",		850, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|7, B_RELOADIMA, "Movie",		900, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|8, 0, "Anti",				950, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|9, 0, "AntiSc",				1000, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|10, 0, "StField",			1050, 170, 50, 18, &tex->imaflag);
		DefBut(TOG|SHO|BIT|11, 0, "FiltX",				1100, 170, 50, 18, &tex->imaflag);
		
			/* MORKRAMIA BUTTONS */
			/* DefBut(TOG|SHO|BIT|14, 0, "MorkPatch",			1025, 170, 75, 18, &tex->imaflag); */
			/* DefBut(TOG|SHO|BIT|13, 0, "LasOpt Patch",		950, 170, 150, 18, &tex->imaflag); */

		SetButCol(BUTGREY);

		/* printen aantal frames anim */
		if(tex->ima && tex->ima->anim) {
			sprintf(str, "%d frs  ", tex->ima->anim->duration);
			DefBut(LABEL, 0, str,      834, 110, 90, 18, 0, 0, 0);
			sprintf(str, "%d cur  ", tex->ima->lastframe);
			DefBut(LABEL, 0, str,      834, 90, 90, 18, 0, 0, 0);
		}
		
		
	}
	else if(tex->type==TEX_PLUGIN) {

		if(tex->plugin && tex->plugin->doit) {
			
			pit= tex->plugin;
			
			SetButCol(BUTGREEN);
			for(a=0; a<pit->stypes; a++) {
				DefBut(ROW|SHO, B_MATPRV, pit->stnames+16*a,	350+75*a, 170, 75, 18, &tex->stype, 2.0, (float)a);
			}
			
			SetButCol(BUTGREY);
			varstr= pit->varstr;
			for(a=0; a<pit->vars; a++, varstr++) {
				xco= 350 + 140*(a/6);
				yco= 110 - 20*(a % 6);
				DefBut(varstr->type, B_MATPRV, varstr->name,	xco,yco,137,19, &(pit->data[a]), varstr->min, varstr->max, 100, 0);
			}

			DefBut(TEX, B_NAMEPLUGIN, "",			520,137,412,24, pit->name, 0.0, 79.0);
			
		}

		SetButCol(BUTSALMON);
		DefBut(BUT, B_LOADPLUGIN, "Load Plugin", 350,137,144,24);
		
	}

	/* COLORBAND */
	SetButCol(BUTSALMON);
	DefBut(TOG|SHO|BIT|0, B_COLORBAND, "Colorband",		923,103,102,20, &tex->flag);
	if(tex->flag & TEX_COLORBAND) {
		DefBut(BUT, B_ADDCOLORBAND, "Add",				1029,103,50,20);
		DefBut(BUT, B_DELCOLORBAND, "Del",				1218,104,50,20);
		SetButCol(BUTPURPLE);
		DefBut(NUM|SHO, B_REDR,		"Cur:",				1082,104,132,20, &tex->coba->cur, 0.0, (float)(tex->coba->tot-1));

		DefBut(LABEL, B_DOCOLORBAND, "", 923,81,345,20, 0, 0, 0); /* alleen voor event! */
		
		drawcolorband(tex->coba, 923,81,345,20);
		cbd= tex->coba->data + tex->coba->cur;
		
		DefBut(NUM|FLO, B_CALCCBAND, "Pos",			923,59,89,20, &cbd->pos, 0.0, 1.0, 10);
		SetButCol(BUTGREEN);
		DefBut(ROW|SHO, B_REDRAWCBAND, "E",		1013,59,20,20, &tex->coba->ipotype, 5.0, 1.0);
		DefBut(ROW|SHO, B_REDRAWCBAND, "L",		1033,59,20,20, &tex->coba->ipotype, 5.0, 0.0);
		DefBut(ROW|SHO, B_REDRAWCBAND, "S",		1053,59,20,20, &tex->coba->ipotype, 5.0, 2.0);
		SetButCol(BUTPURPLE);
		DefBut(COL|FLO, B_BANDCOL, "",					1076,59,28,20, &(cbd->r));
		DefBut(NUMSLI|FLO, B_REDRAWCBAND, "A ",			1107,58,163,20, &cbd->a, 0.0, 1.0, 0);
		
		DefBut(NUMSLI|FLO, B_REDRAWCBAND, "R ",			923,37,116,20, &cbd->r, 0.0, 1.0, B_BANDCOL);
		DefBut(NUMSLI|FLO, B_REDRAWCBAND, "G ",			1042,37,111,20, &cbd->g, 0.0, 1.0, B_BANDCOL);
		DefBut(NUMSLI|FLO, B_REDRAWCBAND, "B ",			1156,36,115,20, &cbd->b, 0.0, 1.0, B_BANDCOL);
		
	}


	/* RGB-BRICON */
	SetButCol(BUTGREY);
	DefBut(NUMSLI|FLO, B_MATPRV, "Bright",			923,11,166,20, &tex->bright, 0.0, 2.0, 0);
	
	DefBut(NUMSLI|FLO, B_MATPRV, "Contr",			1093,11,180,20, &tex->contrast, 0.01, 2.0, 0);

	if((tex->flag & TEX_COLORBAND)==0) {
		DefBut(NUMSLI|FLO, B_MATPRV, "R ",			923,37,116,20, &tex->rfac, 0.0, 2.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "G ",			1042,37,111,20, &tex->gfac, 0.0, 2.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "B ",			1156,36,115,20, &tex->bfac, 0.0, 2.0, 0);
	}
	
	/* PREVIEW RENDER */
	
	previewdraw();

	DefBut(BUT, B_MATPRV, "ICON 0 15 0",		84,195,21,20);
	DefBut(BUT, B_MATPRV, "ICON 0 14 0",		106,195,21,20);
	DefBut(BUT, B_MATPRV, "ICON 0 13 0",		128,195,21,20);

}

/* ****************************** MATERIAL ************************ */
MTex mtexcopybuf;


void do_matbuts(short event)
{
	static short mtexcopied=0;
	Material *ma;
	MTex *mtex;
	Object *ob;
	int a;

	switch(event) {		
	case B_ACTCOL:
		addqueue(curarea->headwin, REDRAW, 1);
		allqueue(REDRAWBUTSMAT, 0);
		allqueue(REDRAWIPO, 0);
		preview_changed(curarea->win);
		break;
	case B_MATFROM:

		addqueue(curarea->headwin, REDRAW, 1);
		allqueue(REDRAWBUTSMAT, 0);
		preview_changed(curarea->win);
		break;
	case B_MATPRV:
		/* dit event wordt ook door lamp, tex en sky gebruikt */
		preview_changed(curarea->win);
		break;
	case B_MATPRV_DRAW:
		preview_changed(curarea->win);
		allqueue(REDRAWBUTSMAT, 0);
		break;
	case B_TEXCLEAR:
		ma= G.buts->lockpoin;
		mtex= ma->mtex[ ma->texact ];
		if(mtex) {
			if(mtex->tex) mtex->tex->id.us--;
			freeN(mtex);
			ma->mtex[ ma->texact ]= 0;
			allqueue(REDRAWBUTSMAT, 0);
			allqueue(REDRAWOOPS, 0);
			preview_changed(curarea->win);
		}
		break;
	case B_MTEXCOPY:
		ma= G.buts->lockpoin;
		if(ma && ma->mtex[ma->texact] ) {
			mtex= ma->mtex[ma->texact];
			if(mtex->tex==0) {
				error("No texture available");
			}
			else {
				memcpy(&mtexcopybuf, ma->mtex[ma->texact], sizeof(MTex));
				notice("copied!");
				mtexcopied= 1;
			}
		}
		break;
	case B_MTEXPASTE:
		ma= G.buts->lockpoin;
		if(ma && mtexcopied && mtexcopybuf.tex) {
			if(ma->mtex[ma->texact]==0 ) ma->mtex[ma->texact]= mallocN(sizeof(MTex), "mtex"); 
			memcpy(ma->mtex[ma->texact], &mtexcopybuf, sizeof(MTex));
			
			id_us_plus(mtexcopybuf.tex);
			notice("pasted!");
			preview_changed(curarea->win);
			addqueue(curarea->win, REDRAW, 1);
		}
		break;
	case B_MATLAY:
		ma= G.buts->lockpoin;
		if(ma && ma->lay==0) {
			ma->lay= 1;
			addqueue(curarea->win, REDRAW, 1);
		}
	}
}

void matsectorbuts()
{
	Object *ob;
	Material *ma;
	float *colpoin, min;
	int a;
	char str[30], *strp;
	
	ob= OBACT;
	if(ob==0 || ob->data==0) return;
	if(ob->actcol==0) ob->actcol= 1;	/* ivm TOG|BIT button */
	
	sprintf(str, "%d Mat:", ob->totcol);
	if(ob->totcol) min= 1.0; else min= 0.0;
	DefBut(NUM|CHA, B_ACTCOL, str,	426,195,128,20, &(ob->actcol), min, (float)ob->totcol);
	
	if(ob->totcol==0) return;

	ma= give_current_material(ob, ob->actcol);
	
	if(ma==0) return;
	SetButLock(ma->id.lib!=0);
	
	colpoin= &(ma->r);

	DefBut(COL|FLO, B_MATCOL, "",		326,120,47,69, &(ma->r));

	DefBut(NUMSLI|FLO, B_MATPRV, "R ",			380,168,175,21, &(ma->r), 0.0, 1.0, B_MATCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "G ",			380,144,175,21, &(ma->g), 0.0, 1.0, B_MATCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "B ",			380,120,175,21, &(ma->b), 0.0, 1.0, B_MATCOL);

	DefBut(NUM|FLO, B_MATPRV, "Coll.Ref ",	380,90,175,19, &DYNA_REF(ma), 0.0, 1.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "Fh dist",	380,70,175,19, &DYNA_FH_DIST(ma), 0.0, 1.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "Fh int ",	380,50,175,19, &DYNA_FH_INT(ma), 0.0, 1.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "Fh Friction",380,30,175,20, &DYNA_FH_FRICT(ma), 0.0, 1.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "Fh XY Frict ",	380,10,175,20, &DYNA_FH_XYFRICT(ma), 0.0, 1.0, 10);
	
	
	for(a=0; a<8; a++) {
		DefBut(TOG|SHO|BIT|a, B_MATLAY, "",	326+a*(12), 195, 12, 20, &(ma->lay));
	}


	ma->spec= 0.0;
	ma->ref= 1.0;
	previewdraw();

}


void matbuts()
{
	Object *ob;
	Material *ma;
	ID *id, *idn;
	MTex *mtex;
	float *colpoin, min;
	int rgbsel, a, xco, loos;
	char str[30], *strp;
	
	ob= OBACT;
	if(ob==0 || ob->data==0) return;
	if(ob->actcol==0) ob->actcol= 1;	/* ivm TOG|BIT button */
	
	/* aangeven waar het materiaal aan hangt */
	SetButCol(BUTSALMON);
	DefBut(TOG|SHO|BIT|(ob->actcol-1), B_MATFROM, "OB",	342, 195, 33, 20, &ob->colbits);
	idn= ob->data;
	strncpy(str, idn->name, 2);
	str[2]= 0;
	SetButCol(BUTGREEN);
	DefBut(TOGN|SHO|BIT|(ob->actcol-1), B_MATFROM, str,		380, 195, 33, 20, &ob->colbits);
	SetButCol(BUTGREY);
	
	/* id is het blok waarvan materiaal wordt gepakt */
	if( BTST(ob->colbits, ob->actcol-1) ) id= (ID *)ob;
	else id= ob->data;

	sprintf(str, "%d Mat", ob->totcol);
	if(ob->totcol) min= 1.0; else min= 0.0;
	DefBut(NUM|CHA, B_ACTCOL, str,	415,195,140,20, &(ob->actcol), min, (float)ob->totcol);
	
	SetButLock(id->lib!=0);
	
	strncpy(str, id->name, 2);
	str[2]= ':'; str[3]= 0;
	SetButFunc(test_idbutton);
	DefBut(TEX, B_IDNAME, str,		200,195,140,20, id->name+2, 0.0, 18.0);

	if(ob->totcol==0) return;
	
	ma= give_current_material(ob, ob->actcol);
	
	if(ma==0) return;
	SetButLock(ma->id.lib!=0);
	
	SetButCol(BUTGREY);
	DefBut(ROW|SHO, REDRAWBUTSMAT, "RGB",			200,166,44,22, &(ma->colormodel), 1.0, (float)MA_RGB);
	DefBut(ROW|SHO, REDRAWBUTSMAT, "HSV",			200,143,44,22, &(ma->colormodel), 1.0, (float)MA_HSV);
	DefBut(ROW|SHO, REDRAWBUTSMAT, "CMYK",			200,120,44,22, &(ma->colormodel), 1.0, (float)MA_CMYK);

	if((ma->mode & MA_HALO)==0)
		DefBut(NUM|FLO, 0, "Zoffset:",		200,91,174,19, &(ma->zoffs), 0.0, 10.0);

	DefBut(COL|FLO, B_MIRCOL, "",		246,143,37,45, &(ma->mirr));
	DefBut(COL|FLO, B_SPECCOL, "",		287,143,37,45, &(ma->specr));
	DefBut(COL|FLO, B_MATCOL, "",		326,143,47,45, &(ma->r));

	if(ma->mode & MA_HALO) {
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Ring",			246,120,37,22, &(ma->rgbsel), 2.0, 2.0);
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Line",			287,120,37,22, &(ma->rgbsel), 2.0, 1.0);
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Halo",			326,120,47,22, &(ma->rgbsel), 2.0, 0.0);
	}
	else {
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Mir",			246,120,37,22, &(ma->rgbsel), 2.0, 2.0);
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Spec",			287,120,37,22, &(ma->rgbsel), 2.0, 1.0);
		DefBut(ROW|CHA, REDRAWBUTSMAT, "Color",			326,120,47,22, &(ma->rgbsel), 2.0, 0.0);
	}
	if(ma->rgbsel==0) {colpoin= &(ma->r); rgbsel= B_MATCOL;}
	else if(ma->rgbsel==1) {colpoin= &(ma->specr); rgbsel= B_SPECCOL;}
	else if(ma->rgbsel==2) {colpoin= &(ma->mirr); rgbsel= B_MIRCOL;}
	
	if(ma->rgbsel==0 && (ma->mode & MA_VERTEXCOLP));
	else if(ma->colormodel==MA_HSV) {
		SetButCol(BUTPURPLE);
		DefBut(HSVSLI|FLO, B_MATPRV, "H ",			380,168,175,21, colpoin, 0.0, 0.9999, rgbsel);
		DefBut(HSVSLI|FLO, B_MATPRV, "S ",			380,144,175,21, colpoin+1, 0.0, 1.0, rgbsel);
		DefBut(HSVSLI|FLO, B_MATPRV, "V ",			380,120,175,21, colpoin+2, 0.0, 1.0, rgbsel);
		SetButCol(BUTGREY);
	}
	else {
		DefBut(NUMSLI|FLO, B_MATPRV, "R ",			380,168,175,21, colpoin, 0.0, 1.0, rgbsel);
		DefBut(NUMSLI|FLO, B_MATPRV, "G ",			380,144,175,21, colpoin+1, 0.0, 1.0, rgbsel);
		DefBut(NUMSLI|FLO, B_MATPRV, "B ",			380,120,175,21, colpoin+2, 0.0, 1.0, rgbsel);
	}
	
	if(ma->mode & MA_HALO) {
		DefBut(NUM|FLO, B_MATPRV, "HaloSize: ",		200,70,175,18, &(ma->hasize), 0.0, 100.0, 10);
		DefBut(NUMSLI|FLO, B_MATPRV, "Alpha ",		200,50,175,18, &(ma->alpha), 0.0, 1.0, 0);
		DefBut(NUMSLI|SHO, B_MATPRV, "Hard ",		200,30,175,18, &(ma->har), 1.0, 127.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "Add  ",		200,10,175,18, &(ma->add), 0.0, 1.0, 0);
		
		DefBut(NUM|SHO, B_MATPRV, "Rings: ",		380,90,85,18, &(ma->ringc), 0.0, 24.0);
		DefBut(NUM|SHO, B_MATPRV, "Lines: ",		465,90,90,18, &(ma->linec), 0.0, 250.0);
		DefBut(NUM|SHO, B_MATPRV, "Star: ",			380,70,85,18, &(ma->starc), 3.0, 50.0);
		DefBut(NUM|CHA, B_MATPRV, "Seed: ",			465,70,90,18, &(ma->seed1), 0.0, 255.0);
		
		DefBut(NUM|FLO, B_MATPRV, "FlareSize: ",	380,50,85,18, &(ma->flaresize), 0.1, 25.0, 10);
		DefBut(NUM|FLO, B_MATPRV, "Sub Size: ",		465,50,90,18, &(ma->subsize), 0.1, 25.0, 10);
		DefBut(NUM|FLO, B_MATPRV, "FlareBoost: ",	380,30,175,18, &(ma->flareboost), 0.1, 10.0, 10);
		DefBut(NUM|CHA, B_MATPRV, "Fl.seed: ",		380,10,85,18, &(ma->seed2), 0.0, 255.0);
		DefBut(NUM|SHO, B_MATPRV, "Flares: ",		465,10,90,18, &(ma->flarec), 1.0, 32.0);

		SetButCol(BUTBLUE);
		
		DefBut(TOG|INT|BIT|15, B_MATPRV, "Flare",		571, 181, 77, 36, &(ma->mode));
		DefBut(TOG|INT|BIT|8, B_MATPRV, "Rings",		571, 143, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|9, B_MATPRV, "Lines",		571, 124, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|11, B_MATPRV, "Star",		571, 105, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|5, B_MATPRV_DRAW, "Halo",	571, 86, 77, 18, &(ma->mode));
		
		DefBut(TOG|INT|BIT|12, B_MATPRV, "HaloTex",		571, 67, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|13, B_MATPRV, "HaloPuno",	571, 48, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|10, B_MATPRV, "X Alpha",		571, 28, 77, 18, &(ma->mode));
	}
	else {
		DefBut(NUMSLI|FLO, B_MATPRV, "Spec ",		200,70,175,18, &(ma->spec), 0.0, 1.0, 0);
		DefBut(NUMSLI|SHO, B_MATPRV, "Hard ",		200,50,175,18, &(ma->har), 1.0, 127.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "SpTr ",		200,30,175,18, &(ma->spectra), 0.0, 1.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "Add  ",		200,10,175,18, &(ma->add), 0.0, 1.0, 0);
	
		DefBut(NUMSLI|FLO, B_MATPRV, "Ref   ",		380,70,175,18, &(ma->ref), 0.0, 1.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "Alpha ",		380,50,175,18, &(ma->alpha), 0.0, 1.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "Emit  ",		380,30,175,18, &(ma->emit), 0.0, 1.0, 0);
		DefBut(NUMSLI|FLO, B_MATPRV, "Amb   ",		380,10,175,18, &(ma->amb), 0.0, 1.0, 0);
	
		SetButCol(BUTBLUE);
	
		DefBut(TOG|INT|BIT|0, 0,	"Traceble",		571,200,77,18, &(ma->mode));
		DefBut(TOG|INT|BIT|1, 0,	"Shadow",		571,181,77,18, &(ma->mode));
		DefBut(TOG|INT|BIT|2, B_MATPRV, "Shadeless",	571, 162, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|3, 0,	"Wire",			571, 143, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|4, 0,	"VertexCol",		571, 124, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|7, B_REDR, "VCol Paint",	571,105, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|5, B_MATPRV_DRAW, "Halo",571, 86, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|6, 0,	"ZTransp",			571, 67, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|8, 0,	"ZInvert",			571, 48, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|9, 0,	"Env",			571, 29, 77, 18, &(ma->mode));
		DefBut(TOG|INT|BIT|10, 0,	"OnlyShadow",		571, 10, 77, 18, &(ma->mode));

		DefBut(TOG|INT|BIT|14, 0,	"No Mist",		477,95,77,18, &(ma->mode));
	}
	/* PREVIEW RENDER */
	
	previewdraw();

	DefBut(TOG|CHA|BIT|0, B_MATPRV, "ICON 0 2 7",		11,195,27,20, &(ma->pr_type));
	DefBut(ICONTOG|CHA|BIT|1, B_MATPRV, "ICON 0 12 3",		39,195,27,20, &(ma->pr_type));
	
	DefBut(BUT, B_MATPRV, "ICON 0 15 0",		84,195,21,20);
	DefBut(BUT, B_MATPRV, "ICON 0 14 0",		106,195,21,20);
	DefBut(BUT, B_MATPRV, "ICON 0 13 0",		128,195,21,20);
	DefBut(BUT, B_MATPRV, "ICON 0 0 7",		159,195,30,20);

	/* TEX CHANNELS */
	SetButCol(BUTGREY);
	xco= 665;
	for(a= 0; a<8; a++) {
		mtex= ma->mtex[a];
		if(mtex && mtex->tex) splitIDname(mtex->tex->id.name+2, str, &loos);
		else strcpy(str, "");
		str[10]= 0;
		DefBut(ROW|CHA, B_MATPRV_DRAW, str,	xco, 195, 63, 20, &(ma->texact), 3.0, (float)a);
		xco+= 65;
	}
	
	DefBut(BUT, B_MTEXCOPY, "ICON 0 14 7",	xco,195,20,21);
	DefBut(BUT, B_MTEXPASTE, "ICON 0 13 7",	xco+20,195,20,21);

	
	SetButCol(BUTGREEN);
	DefBut(TOG|CHA, B_MATPRV, "SepT", xco+40, 195, 40, 20, &(ma->septex));
	SetButCol(BUTGREY);

	mtex= ma->mtex[ ma->texact ];
	if(mtex==0) {
		mtex= &emptytex;
		default_mtex(mtex);
	}
	
	/* TEXCO */
	SetButCol(BUTGREEN);
	DefBut(ROW|SHO, B_MATPRV, "Object",		694,166,49,18, &(mtex->texco), 4.0, (float)TEXCO_OBJECT);
	SetButFunc(test_obpoin_but);
	DefBut(IDPOIN, B_MATPRV, "",		745,166,133,18, &(mtex->object));
	DefBut(ROW|SHO, B_MATPRV, "UV",			664,166,29,18, &(mtex->texco), 4.0, (float)TEXCO_UV);

	DefBut(ROW|SHO, B_MATPRV, "Glob",			665,146,35,18, &(mtex->texco), 4.0, (float)TEXCO_GLOB);
	DefBut(ROW|SHO, B_MATPRV, "Orco",			701,146,38,18, &(mtex->texco), 4.0, (float)TEXCO_ORCO);
	DefBut(ROW|SHO, B_MATPRV, "Stick",			739,146,38,18, &(mtex->texco), 4.0, (float)TEXCO_STICKY);
	DefBut(ROW|SHO, B_MATPRV, "Win",			779,146,31,18, &(mtex->texco), 4.0, (float)TEXCO_WINDOW);
	DefBut(ROW|SHO, B_MATPRV, "Nor",			811,146,32,18, &(mtex->texco), 4.0, (float)TEXCO_NORM);
	DefBut(ROW|SHO, B_MATPRV, "Refl",			844,146,33,18, &(mtex->texco), 4.0, (float)TEXCO_REFL);
	
	SetButCol(BUTGREY);
	
	/* COORDS */
	DefBut(ROW|CHA, B_MATPRV, "Flat",			666,114,48,18, &(mtex->mapping), 5.0, (float)MTEX_FLAT);
	DefBut(ROW|CHA, B_MATPRV, "Cube",			717,114,50,18, &(mtex->mapping), 5.0, (float)MTEX_CUBE);
	DefBut(ROW|CHA, B_MATPRV, "Tube",			666,94,48,18, &(mtex->mapping), 5.0, (float)MTEX_TUBE);
	DefBut(ROW|CHA, B_MATPRV, "Sphe",			716,94,50,18, &(mtex->mapping), 5.0, (float)MTEX_SPHERE);

	xco= 665;
	for(a=0; a<4; a++) {
		if(a==0) strcpy(str, "");
		else if(a==1) strcpy(str, "X");
		else if(a==2) strcpy(str, "Y");
		else strcpy(str, "Z");
		
		DefBut(ROW|CHA, B_MATPRV, str,			xco, 50, 24, 18, &(mtex->projx), 6.0, (float)a);
		DefBut(ROW|CHA, B_MATPRV, str,			xco, 30, 24, 18, &(mtex->projy), 7.0, (float)a);
		DefBut(ROW|CHA, B_MATPRV, str,			xco, 10, 24, 18, &(mtex->projz), 8.0, (float)a);
		xco+= 26;
	}
	
	DefBut(NUM|FLO, B_MATPRV, "ofsX",		778,114,100,18, mtex->ofs, -10.0, 10.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "ofsY",		778,94,100,18, mtex->ofs+1, -10.0, 10.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "ofsZ",		778,74,100,18, mtex->ofs+2, -10.0, 10.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeX",	778,50,100,18, mtex->size, -100.0, 100.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeY",	778,30,100,18, mtex->size+1, -100.0, 100.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeZ",	778,10,100,18, mtex->size+2, -100.0, 100.0, 10);
	
	/* TEXTUREBLOK SELECT */
	if(G.main->tex.first==0) {
		DefBut(MENU|SHO, B_EXTEXBROWSE, "ADD NEW %x 32767", 900,146,20,19, &(G.buts->texnr));
		id= 0;
	}
	else {
		id= (ID *)mtex->tex;
		IDnames_to_pupstring(&strp, &(G.main->tex), id, &(G.buts->texnr));
		if(strp[0]) strcat(strp, "|ADD NEW %x 32767");
		else strcat(strp, "ADD NEW %x 32767");
		DefBut(MENU|SHO, B_EXTEXBROWSE, strp, 900,146,20,19, &(G.buts->texnr));
		freeN(strp);
	}
	if(id) {
		DefBut(TEX, B_IDNAME, "TE:",	900,166,163,19, id->name+2, 0.0, 18.0);
		sprintf(str, "%d", id->us);
		DefBut(BUT, 0, str,				996,146,21,19);
		DefBut(BUT, B_AUTOTEXNAME, "ICON 0 7 4", 1041,146,21,19);
		if(id->lib) {
			if(ma->id.lib) DefBut(BUT, 0, "ICON 0 6 4",	1019,146,21,19);
			else DefBut(BUT, 0, "ICON 0 5 4",	1019,146,21,19);			
		}
		SetButCol(BUTSALMON);
		DefBut(BUT, B_TEXCLEAR, "Clear", 922, 146, 72, 19);
		SetButCol(BUTGREY);
	}
	
	/* TEXTURE OUTPUT */
	DefBut(TOG|SHO|BIT|1, B_MATPRV, "Stencil",	900,114,52,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|2, B_MATPRV, "Neg",		954,114,38,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "No RGB",	994,114,69,18, &(mtex->texflag));
	
	DefBut(COL|FLO, B_MTEXCOL, "",				900,100,163,12, &(mtex->r));

	if(ma->colormodel==MA_HSV) {
		SetButCol(BUTPURPLE);
		DefBut(HSVSLI|FLO, B_MATPRV, "H ",			900,80,163,18, &(mtex->r), 0.0, 1.0, B_MTEXCOL);
		DefBut(HSVSLI|FLO, B_MATPRV, "S ",			900,60,163,18, &(mtex->g), 0.0, 1.0, B_MTEXCOL);
		DefBut(HSVSLI|FLO, B_MATPRV, "V ",			900,40,163,18, &(mtex->b), 0.0, 1.0, B_MTEXCOL);
		SetButCol(BUTGREY);
	}
	else {
		DefBut(NUMSLI|FLO, B_MATPRV, "R ",			900,80,163,18, &(mtex->r), 0.0, 1.0, B_MTEXCOL);
		DefBut(NUMSLI|FLO, B_MATPRV, "G ",			900,60,163,18, &(mtex->g), 0.0, 1.0, B_MTEXCOL);
		DefBut(NUMSLI|FLO, B_MATPRV, "B ",			900,40,163,18, &(mtex->b), 0.0, 1.0, B_MTEXCOL);
	}
	
	DefBut(NUMSLI|FLO, B_MATPRV, "DVar ",		900,10,163,18, &(mtex->def_var), 0.0, 1.0, 0);
	
	/* MAP TO */
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "Col",		1087,166,35,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|1, B_MATPRV, "Nor",		1126,166,31,18, &(mtex->mapto));
	DefBut(TOG|SHO|BIT|2, B_MATPRV, "Csp",		1160,166,34,18, &(mtex->mapto));
	DefBut(TOG|SHO|BIT|3, B_MATPRV, "Cmir",		1196,166,35,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|4, B_MATPRV, "Ref",		1234,166,31,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|5, B_MATPRV, "Spec",	1087,146,36,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|8, B_MATPRV, "Hard",	1126,146,44,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|7, B_MATPRV, "Alpha",	1172,146,45,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|6, B_MATPRV, "Emit",	1220,146,45,18, &(mtex->mapto));
	DefBut(TOG3|SHO|BIT|9, B_MATPRV, "Extra",	1220,126,45,18, &(mtex->mapto)); 


	
/* 	DefBut(TOG|SHO|BIT|3, B_MATPRV, "Alpha Mix",1087,114,100,18, &(mtex->texflag)); */

	SetButCol(BUTGREY);
	DefBut(ROW|SHO, B_MATPRV, "Mix",			1087,94,48,18, &(mtex->blendtype), 9.0, (float)MTEX_BLEND);
	DefBut(ROW|SHO, B_MATPRV, "Mul",			1136,94,44,18, &(mtex->blendtype), 9.0, (float)MTEX_MUL);
	DefBut(ROW|SHO, B_MATPRV, "Add",			1182,94,41,18, &(mtex->blendtype), 9.0, (float)MTEX_ADD);
	DefBut(ROW|SHO, B_MATPRV, "Sub",			1226,94,40,18, &(mtex->blendtype), 9.0, (float)MTEX_SUB);
	
	DefBut(NUMSLI|FLO, B_MATPRV, "Col ",		1087,50,179,18, &(mtex->colfac), 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Nor ",		1087,30,179,18, &(mtex->norfac), 0.0, 5.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Var ",		1087,10,179,18, &(mtex->varfac), 0.0, 1.0, 0);
	
}


/* ************************ LAMP *************************** */

void do_lampbuts(short event)
{
	Lamp *la;
	MTex *mtex;
		
	switch(event) {
	case B_LAMPREDRAW:
		preview_changed(curarea->win);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_TEXCLEARLAMP:
		la= G.buts->lockpoin;
		mtex= la->mtex[ la->texact ];
		if(mtex) {
			if(mtex->tex) mtex->tex->id.us--;
			freeN(mtex);
			la->mtex[ la->texact ]= 0;
			allqueue(REDRAWBUTSLAMP, 0);
			allqueue(REDRAWOOPS, 0);
			preview_changed(curarea->win);
		}
		break;
	}
	
	if(event) freefastshade();
}


void lampbuts()
{
	Object *ob;
	Lamp *la;
	MTex *mtex;
	ID *id;
	float grid;
	int loos, xco, a;
	char *strp, str[32];
	
	if(G.vd) grid= G.vd->grid; else grid= 1.0;
	
	ob= OBACT;
	if(ob==0) return;
	if(ob->type!=OB_LAMP) return;
	la= ob->data;
	SetButLock(la->id.lib!=0);

	SetButCol(BUTGREEN);
	DefBut(ROW|SHO,B_LAMPREDRAW,"Lamp",	317,190,61,25,&la->type,1.0,(float)LA_LOCAL);
	DefBut(ROW|SHO,B_LAMPREDRAW,"Spot",	379,190,59,25,&la->type,1.0,(float)LA_SPOT);
	DefBut(ROW|SHO,B_LAMPREDRAW,"Sun",	439,190,58,25,&la->type,1.0,(float)LA_SUN);
	DefBut(ROW|SHO,B_LAMPREDRAW,"Hemi",	499,190,55,25,&la->type,1.0,(float)LA_HEMI);

	SetButCol(BUTGREY);
	DefBut(NUM|FLO,B_LAMPREDRAW,"Dist:",611,190,104,25,&la->dist, 0.01, 5000.0, 100);

	SetButCol(BUTBLUE);
	DefBut(TOG|SHO|BIT|3, B_MATPRV,"Quad",		203,190,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|6, REDRAWVIEW3D,"Sphere",203,170,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|0, REDRAWVIEW3D, "Shadows", 203,150,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|1, 0,"Halo",				203,130,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|2, 0,"Layer",				203,110,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|4, 0,"Negative",				203,90,100,19,&la->mode);
	DefBut(TOG|SHO|BIT|5, 0,"OnlyShadow",			203,70,100,19,&la->mode);

	SetButCol(BUTGREY);
	DefBut(ROW|SHO,0,"BufSi 512",	203,31,89,19,	&la->bufsize,2.0,512.0);
	DefBut(ROW|SHO,0,"768",			293,31,48,19,	&la->bufsize,2.0,768.0);
	DefBut(ROW|SHO,0,"1024",		203,10,59,19,	&la->bufsize,2.0,1024.0);
	DefBut(ROW|SHO,0,"1536",		263,9,77,19,	&la->bufsize,2.0,1536.0);
	DefBut(NUM|FLO,REDRAWVIEW3D,"ClipSta:",	346,30,146,19,	&la->clipsta, 0.1*grid,1000.0*grid, 10);
	DefBut(NUM|FLO,REDRAWVIEW3D,"ClipEnd:",	346,9,146,19,&la->clipend, 1.0, 5000.0*grid, 100);
	DefBut(NUM|SHO,0,"Samples:",	496,9,105,41,	&la->samp,1.0,5.0);
	DefBut(NUM|FLO,0,"Bias:",		605,30,108,19,	&la->bias, 0.01, 5.0, 1);
	DefBut(NUM|FLO,0,"Soft:",		605,8,108,19,	&la->soft,1.0,100.0, 100);
	
	SetButCol(BUTGREY);
	DefBut(NUMSLI|FLO,B_MATPRV,"Energy ",	520,156,195,20, &(la->energy), 0.0, 10.0, 0, 0);

	DefBut(NUMSLI|FLO,B_MATPRV,"R ",		520,128,194,20,&la->r, 0.0, 1.0, B_COLLAMP, 0);
	DefBut(NUMSLI|FLO,B_MATPRV,"G ",		520,108,194,20,&la->g, 0.0, 1.0, B_COLLAMP, 0);
	DefBut(NUMSLI|FLO,B_MATPRV,"B ",		520,88,194,20,&la->b, 0.0, 1.0, B_COLLAMP, 0);
	
	DefBut(COL|FLO, B_COLLAMP, "",			520,64,193,23, &la->r);
	
	DefBut(NUMSLI|FLO,B_LAMPREDRAW,"SpotSi ",317,157,192,19,&la->spotsize, 1.0, 180.0, 0);
	DefBut(NUMSLI|FLO,B_MATPRV,"SpotBl ",	316,136,192,19,&la->spotblend, 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO,B_MATPRV,"Quad1 ",	316,106,192,19,&la->att1, 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO,B_MATPRV,"Quad2 ",  	317,86,191,19,&la->att2, 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO,0,"HaloInt ",		316,64,193,19,&la->haint, 0.0, 1.0, 0);


	/* TEX CHANNELS */
	SetButCol(BUTGREY);
	xco= 745;
	for(a= 0; a<6; a++) {
		mtex= la->mtex[a];
		if(mtex && mtex->tex) splitIDname(mtex->tex->id.name+2, str, &loos);
		else strcpy(str, "");
		str[10]= 0;
		DefBut(ROW|SHO, B_REDR, str,	xco, 195, 83, 20, &(la->texact), 3.0, (float)a);
		xco+= 85;
	}
	
	mtex= la->mtex[ la->texact ];
	if(mtex==0) {
		mtex= &emptytex;
		default_mtex(mtex);
		mtex->texco= TEXCO_VIEW;
	}
	
	/* TEXCO */
	SetButCol(BUTGREEN);
	DefBut(ROW|SHO, B_MATPRV, "Object",		745,146,49,18, &(mtex->texco), 4.0, (float)TEXCO_OBJECT);
	SetButFunc(test_obpoin_but);
	DefBut(IDPOIN, B_MATPRV, "",		745,166,133,18, &(mtex->object));
	DefBut(ROW|SHO, B_MATPRV, "Glob",			795,146,45,18, &(mtex->texco), 4.0, (float)TEXCO_GLOB);
	DefBut(ROW|SHO, B_MATPRV, "View",			839,146,39,18, &(mtex->texco), 4.0, (float)TEXCO_VIEW);
	
	SetButCol(BUTGREY);	
	DefBut(NUM|FLO, B_MATPRV, "dX",		745,114,133,18, mtex->ofs, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "dY",		745,94,133,18, mtex->ofs+1, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "dZ",		745,74,133,18, mtex->ofs+2, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeX",	745,50,133,18, mtex->size, -10.0, 10.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeY",	745,30,133,18, mtex->size+1, -10.0, 10.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeZ",	745,10,133,18, mtex->size+2, -10.0, 10.0, 10);
	
	/* TEXTUREBLOK SELECT */
	id= (ID *)mtex->tex;
	IDnames_to_pupstring(&strp, &(G.main->tex), id, &(G.buts->texnr));
	if(strp[0]) strcat(strp, "|ADD NEW %x 32767");
	else strcat(strp, "ADD NEW %x 32767");
	
	/* werkt niet omdat lockpoin op lamp staat, niet op texture */
	DefBut(MENU|SHO, B_LTEXBROWSE, strp, 900,146,20,19, &(G.buts->texnr));
	
	freeN(strp);
	if(id) {
		DefBut(TEX, B_IDNAME, "TE:",	900,166,163,19, id->name+2, 0.0, 18.0);
		sprintf(str, "%d", id->us);
		DefBut(BUT, 0, str,				996,146,21,19);
		DefBut(BUT, B_AUTOTEXNAME, "ICON 0 7 4", 1041,146,21,19);
		if(id->lib) {
			if(la->id.lib) DefBut(BUT, 0, "ICON 0 6 4",	1019,146,21,19);
			else DefBut(BUT, 0, "ICON 0 5 4",	1019,146,21,19);			
		}
		SetButCol(BUTSALMON);
		DefBut(BUT, B_TEXCLEARLAMP, "Clear", 922, 146, 72, 19);
		SetButCol(BUTGREY);
	}
	
	/* TEXTURE OUTPUT */
	DefBut(TOG|SHO|BIT|1, B_MATPRV, "Stencil",	900,114,52,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|2, B_MATPRV, "Neg",		954,114,38,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "RGBtoInt",	994,114,69,18, &(mtex->texflag));
	
	DefBut(COL|FLO, B_MTEXCOL, "",				900,100,163,12, &(mtex->r));
	DefBut(NUMSLI|FLO, B_MATPRV, "R ",			900,80,163,18, &(mtex->r), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "G ",			900,60,163,18, &(mtex->g), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "B ",			900,40,163,18, &(mtex->b), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "DVar ",		900,10,163,18, &(mtex->def_var), 0.0, 1.0, 0);
	
	/* MAP TO */
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "Col",		1087,166,81,18, &(mtex->mapto));
	
	SetButCol(BUTGREY);
	DefBut(ROW|SHO, B_MATPRV, "Blend",			1087,114,48,18, &(mtex->blendtype), 9.0, (float)MTEX_BLEND);
	DefBut(ROW|SHO, B_MATPRV, "Mul",			1136,114,44,18, &(mtex->blendtype), 9.0, (float)MTEX_MUL);
	DefBut(ROW|SHO, B_MATPRV, "Add",			1182,114,41,18, &(mtex->blendtype), 9.0, (float)MTEX_ADD);
	DefBut(ROW|SHO, B_MATPRV, "Sub",			1226,114,40,18, &(mtex->blendtype), 9.0, (float)MTEX_SUB);
	
	DefBut(NUMSLI|FLO, B_MATPRV, "Col ",		1087,50,179,18, &(mtex->colfac), 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Nor ",		1087,30,179,18, &(mtex->norfac), 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Var ",		1087,10,179,18, &(mtex->varfac), 0.0, 1.0, 0);


	previewdraw();

}

/* ***************************** ANIM ************************** */

void do_animbuts(short event)
{
	Object *ob;
	Base *base;
	Effect *eff, *effn;
	int type;
	
	ob= OBACT;

	switch(event) {
		
	case B_RECALCPATH:
		calc_curvepath(OBACT);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_MUL_IPO:
		scale_editipo();
		allqueue(REDRAWBUTSANIM, 0);
		break;
	case B_AUTOTIMEOFS:
		auto_timeoffs();
		break;
	case B_FRAMEMAP:
		G.scene->r.framelen= G.scene->r.framapto;
		G.scene->r.framelen/= G.scene->r.images;
		break;
	case B_NEWEFFECT:
		if(ob) {
			copy_act_effect(ob);
		}
		allqueue(REDRAWBUTSANIM, 0);
		break;
	case B_DELEFFECT:
		if(ob==0) break;
		eff= ob->effect.first;
		while(eff) {
			effn= eff->next;
			if(eff->flag & SELECT) {
				remlink(&ob->effect, eff);
				free_effect(eff);
				break;
			}
			eff= effn;
		}
		allqueue(REDRAWBUTSANIM, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_NEXTEFFECT:
		if(ob==0) break;
		eff= ob->effect.first;
		while(eff) {
			if(eff->flag & SELECT) {
				if(eff->next) {
					eff->flag &= ~SELECT;
					eff->next->flag |= SELECT;
				}
				break;
			}
			eff= eff->next;
		}
		allqueue(REDRAWBUTSANIM, 0);
		break;
	case B_PREVEFFECT:
		if(ob==0) break;
		eff= ob->effect.first;
		while(eff) {
			if(eff->flag & SELECT) {
				if(eff->prev) {
					eff->flag &= ~SELECT;
					eff->prev->flag |= SELECT;
				}
				break;
			}
			eff= eff->next;
		}
		allqueue(REDRAWBUTSANIM, 0);
		break;
	case B_CHANGEEFFECT:
		if(ob==0) break;
		eff= ob->effect.first;
		while(eff) {
			if(eff->flag & SELECT) {
				if(eff->type!=eff->buttype) {
					remlink(&ob->effect, eff);
					type= eff->buttype;
					free_effect(eff);
					eff= add_effect(type);
					addtail(&ob->effect, eff);
				}
				break;
			}
			eff= eff->next;
		}
		allqueue(REDRAWBUTSANIM, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_CALCEFFECT:
		if(ob==0) break;
		eff= ob->effect.first;
		while(eff) {
			if(eff->flag & SELECT) {
				if(eff->type==EFF_PARTICLE) build_particle_system(ob);
				else if(eff->type==EFF_WAVE) object_wave(ob);
			}
			eff= eff->next;
		}
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_RECALCAL:
		base= FIRSTBASE;
		while(base) {
			if(base->lay & G.vd->lay) {
				ob= base->object;
				eff= ob->effect.first;
				while(eff) {
					if(eff->flag & SELECT) {
						if(eff->type==EFF_PARTICLE) build_particle_system(ob);
					}
					eff= eff->next;
				}
			}
			base= base->next;
		}
		allqueue(REDRAWVIEW3D, 0);
		break;
	case B_SETSPEED:
		set_speed_editipo(hspeed);
		break;
	case B_PRINTSPEED:
		ob= OBACT;
		if(ob) {
			float vec[3];
			CFRA++;
			do_ob_ipo(ob);
			where_is_object(ob);
			VECCOPY(vec, ob->obmat[3]);
			CFRA--;
			do_ob_ipo(ob);
			where_is_object(ob);
			VecSubf(vec, vec, ob->obmat[3]);
			prspeed= Normalise(vec);
			addqueue(curarea->win, REDRAW, 1);
		}
		break;
	case B_PRINTLEN:
		ob= OBACT;
		if(ob && ob->type==OB_CURVE) {
			Curve *cu=ob->data;
			
			if(cu->path) prlen= cu->path->totdist; else prlen= -1.0;
			addqueue(curarea->win, REDRAW, 1);
		}
		break;
	default:
		if(event>=B_SELEFFECT && event<B_SELEFFECT+MAX_EFFECT) {
			ob= OBACT;
			if(ob) {
				int a=B_SELEFFECT;
				
				eff= ob->effect.first;
				while(eff) {
					if(event==a) eff->flag |= SELECT;
					else eff->flag &= ~SELECT;
					
					a++;
					eff= eff->next;
				}
				allqueue(REDRAWBUTSANIM, 0);
			}
		}
	}
}


void animbuts()
{
	Object *ob;
	Mesh *me;
	Lattice *lt;
	Effect *eff;
	Key *key;
	Curve *cu;
	SpaceIpo *si;
	ScrArea *sa;
	int a, x, y;
	char str[32];
	
	DefBut(NUM|SHO,REDRAWSEQ,"Sta:",	320,17,93,27,&G.scene->r.sfra,1.0,18000.0);
	DefBut(NUM|SHO,REDRAWSEQ,"End:",	416,17,95,27,&G.scene->r.efra,1.0,18000.0);

	DefBut(NUM|SHO,B_FRAMEMAP,"Map Old:",	320,69,93,22,&G.scene->r.framapto,1.0,900.0);
	DefBut(NUM|SHO,B_FRAMEMAP,"Map New:",	416,69,95,22,&G.scene->r.images,1.0,900.0);

	DefBut(NUM|SHO, 0, "AnimSpeed:",	320,47,192,19, &G.animspeed, 1.0, 9.0);

	ob= OBACT;
	if(ob==0) return;
	
	DefBut(NUM|FLO, REDRAWALL, "TimeOffset:",				23,18,114,30, &ob->sf, -9000.0, 9000.0, 100);

	SetButCol(BUTGREEN);
	DefBut(TOG|CHA|BIT|0, REDRAWVIEW3D, "Draw Key",		25,144,94,19, &ob->ipoflag);
	DefBut(TOG|CHA|BIT|1, REDRAWVIEW3D, "Draw Key Sel",	25,123,93,19, &ob->ipoflag);

	#ifndef FREE
	DefBut(TOG|SHO|BIT|3, B_REDR, "HotSPot",		299,190,76,19, &ob->infoflag);
	if(ob->infoflag & OB_INFO_HOTSPOT) {
		DefBut(TOG|SHO, 0, "Don't render",		390,190,114,19, &ob->infoend);
		SetButCol(BUTGREY);
		DefBut(NUM|SHO, 0, "Nr:",				390,170,114,19, &ob->infostart, 0.0, 2500.0);
	}
	else {
		DefBut(TOG|SHO|BIT|0, 0, "INFO loc",	390,190,60,19, &ob->infoflag);
		DefBut(TOG|SHO|BIT|1, 0, "sco",			451,190,40,19, &ob->infoflag);
		DefBut(TOG|SHO|BIT|2, 0, "drot",		492,190,40,19, &ob->infoflag);
		SetButCol(BUTGREY);
		DefBut(NUM|SHO, 0, "Info sta",		390,170,79,19, &ob->infostart, 0.0, 2500.0);
		DefBut(NUM|SHO, 0, "End",			471,170,60,19, &ob->infoend, 0.0, 2500.0);
	}
	#endif
	
	DefBut(TOG|CHA|BIT|2, REDRAWALL, "Offs Ob",			25,64,60,20, &ob->ipoflag);
	DefBut(TOG|CHA|BIT|6, REDRAWALL, "Offs Par",		85,64,60,20, &ob->ipoflag);
	DefBut(TOG|CHA|BIT|7, REDRAWALL, "Offs Parti",		145,64,60,20, &ob->ipoflag);

	DefBut(TOG|SHO|BIT|4, 0, "SlowPar",			205,64,60,20, &ob->partype);

	/* DefBut(TOG|CHA|BIT|5, REDRAWALL, "Offs Path",	85,64,60,20, &ob->ipoflag); */
	/* DefBut(TOG|CHA|BIT|3, REDRAWALL, "Offs Mat",		145,64,60,20, &ob->ipoflag); */
	/* DefBut(TOG|CHA|BIT|4, REDRAWALL, "Offs VertKey",	205,64,60,20, &ob->ipoflag); */


	SetButCol(BUTGREY);
	DefBut(CHA|TOG|BIT|3, REDRAWVIEW3D, "DupliFrames",	125,144,93,20, &ob->transflag);
	DefBut(TOG|CHA|BIT|4, REDRAWVIEW3D, "DupliVerts",	125,123,93,19, &ob->transflag);

	DefBut(NUM|SHO, REDRAWVIEW3D, "DupSta:",	220,144,93,19, &ob->dupsta, 1.0, 1500.0);
	DefBut(NUM|SHO, REDRAWVIEW3D, "DupEnd",		316,144,93,19, &ob->dupend, 1.0, 2500.0);
	DefBut(NUM|SHO, REDRAWVIEW3D, "DupOn:",		220,122,93,19, &ob->dupon, 1.0, 1500.0);
	DefBut(NUM|SHO, REDRAWVIEW3D, "DupOff",		315,122,93,19, &ob->dupoff, 0.0, 1500.0);
	SetButCol(BUTGREEN);
	DefBut(TOG|CHA|BIT|6, REDRAWVIEW3D, "No Speed",		410,144,93,19, &ob->transflag);
	DefBut(TOG|CHA|BIT|7, REDRAWVIEW3D, "Powertrack",	410,122,93,19, &ob->transflag);

	SetButCol(BUTSALMON);
	DefBut(BUT, B_AUTOTIMEOFS, "Automatic Time",		140,18,104,31);
	SetButCol(BUTGREY);
	sprintf(str, "%.4f", prspeed);
	DefBut(LABEL, 0, str,			247,40,63,31, 0, 1.0, 0);
	DefBut(BUT, B_PRINTSPEED,	"PrSpeed",	247,18,63,31);
	
	if(ob->type==OB_MESH) {
		me= ob->data;
		if(me->key) {
			DefBut(NUM|SHO, 0, "Slurph:",				124,100,93,19, &(me->key->slurph), -500.0, 500.0);
		}
	}
	if(ob->type==OB_CURVE) {
		cu= ob->data;
		DefBut(NUM|SHO, B_RECALCPATH, "PathLen:",			34,100,90,19, &cu->pathlen, 1.0, 9000.0);
		DefBut(TOG|SHO|BIT|3, B_RECALCPATH, "CurvePath",	125,100,90,19 , &cu->flag);
		DefBut(TOG|SHO|BIT|4, REDRAWVIEW3D, "CurveFollow",	216,100,90,19, &cu->flag);

		sprintf(str, "%.4f", prlen);
		DefBut(LABEL, 0, str,			396,100,90,19, 0, 1.0, 0);
		DefBut(BUT, B_PRINTLEN,		"PrintLen",	306,100,90,19);
		
		/* if(cu->key) { */
		/* 	DefBut(NUM|SHO, 0, "Slurph:",				124,100,93,19, &(cu->key->slurph), -500.0, 500.0); */
		/* } */
	
	}
	if(ob->type==OB_LATTICE) {
		lt= ob->data;
		if(lt->key) {
			DefBut(NUM|SHO, 0, "Slurph:",				124,100,93,19, &(lt->key->slurph), -500.0, 500.0);
		}
	}
	
	SetButCol(BUTGREEN);
	DefBut(ROW|CHA,REDRAWVIEW3D,"TrackX",	27,190,58,17, &ob->trackflag, 12.0, 0.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"Y",		87,190,19,17, &ob->trackflag, 12.0, 1.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"Z",		107,190,19,17, &ob->trackflag, 12.0, 2.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"-X",		128,190,24,17, &ob->trackflag, 12.0, 3.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"-Y",		154,190,24,17, &ob->trackflag, 12.0, 4.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"-Z",		179,190,24,17, &ob->trackflag, 12.0, 5.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"UpX",		203,190,40,17, &ob->upflag, 13.0, 0.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"Y",		244,190,20,17, &ob->upflag, 13.0, 1.0);
	DefBut(ROW|CHA,REDRAWVIEW3D,"Z",		267,190,19,17, &ob->upflag, 13.0, 2.0);

	SetButCol(BUTSALMON);
	
	/* EFFECTS */
	
	drawedge(540, 0, 540, 240);
	drawedge(1010, 0, 1010, 240);
	
	DefBut(BUT, B_NEWEFFECT, "NEW Effect", 550,187,124,27);
	DefBut(BUT, B_DELEFFECT, "Delete", 676,187,62,27);
	SetButCol(BUTGREY);
	
	/* select effs */
	eff= ob->effect.first;
	a= 0;
	while(eff) {
		
		x= 15*(a % 10) + 740;
		y= 200 - 12*( abs(a/10) ) ;
		DefBut(TOG|SHO|BIT|0, B_SELEFFECT+a, "", x, y, 15, 12, &eff->flag);
		
		a++;
		if(a==MAX_EFFECT) break;
		eff= eff->next;
	}
	
	eff= ob->effect.first;
	while(eff) {
		if(eff->flag & SELECT) break;
		eff= eff->next;
	}
	
	if(eff) {
		#ifdef FREE
		DefBut(MENU|SHO, B_CHANGEEFFECT, "Build %x0|Particles %x1", 895,187,107,27, &eff->buttype);
		#else
		DefBut(MENU|SHO, B_CHANGEEFFECT, "Build %x0|Particles %x1|Wave %x2", 895,187,107,27, &eff->buttype);
		#endif
		
		if(eff->type==EFF_BUILD) {
			BuildEff *bld;
			
			bld= (BuildEff *)eff;
			
			DefBut(NUM|FLO, 0, "Len:",			649,138,95,21, &bld->len, 1.0, 9000.0, 100);
			DefBut(NUM|FLO, 0, "Sfra:",			746,138,94,22, &bld->sfra, 1.0, 9000.0, 100);
		}
		else if(eff->type==EFF_WAVE) {
			WaveEff *wav;
			
			wav= (WaveEff *)eff;
			
			SetButCol(BUTGREEN);
			DefBut(TOG|SHO|BIT|1, B_CALCEFFECT, "X",		782,135,54,23, &wav->flag);
			DefBut(TOG|SHO|BIT|2, B_CALCEFFECT, "Y",		840,135,47,23, &wav->flag);
			DefBut(TOG|SHO|BIT|3, B_CALCEFFECT, "Cycl",		890,135,111,23, &wav->flag);
			
			SetButCol(BUTGREY);
			DefBut(NUM|FLO, B_CALCEFFECT, "Sta x:",		550,135,113,24, &wav->startx, -100.0, 100.0, 100);
			DefBut(NUM|FLO, B_CALCEFFECT, "Sta y:",		665,135,104,24, &wav->starty, -100.0, 100.0, 100);
			
			DefBut(NUMSLI|FLO, B_CALCEFFECT, "Speed:",	550,100,216,20, &wav->speed, -2.0, 2.0, 0);
			DefBut(NUMSLI|FLO, B_CALCEFFECT, "Heigth:",	550,80,216,20, &wav->height, -2.0, 2.0, 0);
			DefBut(NUMSLI|FLO, B_CALCEFFECT, "Width:",	550,60,216,20, &wav->width, 0.0, 5.0, 0);
			DefBut(NUMSLI|FLO, B_CALCEFFECT, "Narrow:",	550,40,216,20, &wav->narrow, 0.0, 10.0, 0);

			DefBut(NUM|FLO, B_CALCEFFECT, "Time sta:",	780,100,219,20, &wav->timeoffs, -1000.0, 1000.0, 100);

			DefBut(NUM|FLO, B_CALCEFFECT, "Lifetime:",	780,80,219,20, &wav->lifetime,  -1000.0, 1000.0, 100);
			DefBut(NUM|FLO, B_CALCEFFECT, "Damptime:",	780,60,219,20, &wav->damp,  -1000.0, 1000.0, 100);

		}
		else if(eff->type==EFF_PARTICLE) {
			PartEff *paf;
			
			paf= (PartEff *)eff;
			
			DefBut(BUT, B_RECALCAL, "RecalcAll", 741,187,67,27);

			DefBut(NUM|INT, B_CALCEFFECT, "Tot:",		550,152,91,20, &paf->totpart, 1.0, 10000.0);
			DefBut(NUM|FLO, B_CALCEFFECT, "Sta:",		644,152,84,20, &paf->sta, -250.0, 9000.0, 100);
			DefBut(NUM|FLO, B_CALCEFFECT, "End:",		731,152,97,20, &paf->end, 1.0, 9000.0, 100);
			DefBut(NUM|FLO, B_CALCEFFECT, "Life:",		831,152,88,20, &paf->lifetime, 1.0, 9000.0, 100);
			DefBut(NUM|INT, B_CALCEFFECT, "Keys:",		922,152,80,20, &paf->totkey, 1.0, 32.0);
			
			SetButCol(BUTGREEN);
			DefBut(NUM|SHO, B_REDR,		"CurMul:",		550,130,91,20, &paf->curmult, 0.0, 3.0);
			SetButCol(BUTGREY);
			DefBut(NUM|SHO, B_CALCEFFECT, "Mat:",		644,130,84,20, paf->mat+paf->curmult, 1.0, 8.0);
			DefBut(NUM|FLO, B_CALCEFFECT, "Mult:",		730,130,98,20, paf->mult+paf->curmult, 0.0, 1.0, 10);
			DefBut(NUM|SHO, B_CALCEFFECT, "Child:",		922,130,80,20, paf->child+paf->curmult, 1.0, 600.0, 100);
			DefBut(NUM|FLO, B_CALCEFFECT, "Life:",		831,130,89,20, paf->life+paf->curmult, 1.0, 600.0, 100);

			DefBut(NUM|FLO, B_CALCEFFECT, "Randlife:",	550,99,96,20, &paf->randlife, 0.0, 2.0, 10);
			DefBut(NUM|INT, B_CALCEFFECT, "Seed:",		652,99,80,20, &paf->seed, 0.0, 255.0);

			DefBut(NUM|FLO, 0,			"VectSize",		885,99,116,20, &paf->vectsize, 0.0, 1.0, 10);			
			SetButCol(BUTGREEN);
			DefBut(TOG|SHO, 0, "Vect",					828,99,54,20, &paf->stype);
			DefBut(TOG|SHO|BIT|1, B_CALCEFFECT, "Bspline",			774,99,54,20, &paf->flag);
			
			SetButCol(BUTPURPLE);
			DefBut(NUM|FLO, B_CALCEFFECT, "Norm:",		550,67,96,20, &paf->normfac, -2.0, 2.0, 10);
			DefBut(NUM|FLO, B_CALCEFFECT, "Ob:",		649,67,86,20, &paf->obfac, -1.0, 1.0, 10);
			DefBut(NUM|FLO, B_CALCEFFECT, "Rand:",		738,67,86,20, &paf->randfac, 0.0, 2.0, 10);
			DefBut(NUM|FLO, B_CALCEFFECT, "Tex:",		826,67,85,20, &paf->texfac, 0.0, 2.0, 10);
			DefBut(NUM|FLO, B_CALCEFFECT, "Damp:",		913,67,89,20, &paf->damp, 0.0, 1.0, 10);

			SetButCol(BUTGREY);
			DefBut(NUM|FLO, B_CALCEFFECT, "X:",			550,31,72,20, paf->force, -1.0, 1.0, 1);
			DefBut(NUM|FLO, B_CALCEFFECT, "Y:",			624,31,78,20, paf->force+1,-1.0, 1.0, 1);
			DefBut(LABEL, 0, "Force:",					532,9,95,20, 0, 1.0, 0);
			DefBut(NUM|FLO, B_CALCEFFECT, "Z:",			623,9,79,20, paf->force+2, -1.0, 1.0, 1);

			DefBut(LABEL, 0, "Texture:",				702,9,95,20, 0, 1.0, 0);
			SetButCol(BUTGREEN);
			DefBut(ROW|SHO, B_CALCEFFECT, "Int",		875,9,32,43, &paf->texmap, 14.0, 0.0);
			DefBut(ROW|SHO, B_CALCEFFECT, "RGB",		911,31,45,20, &paf->texmap, 14.0, 1.0);
			DefBut(ROW|SHO, B_CALCEFFECT, "Grad",		958,31,44,20, &paf->texmap, 14.0, 2.0);
			SetButCol(BUTGREY);
			DefBut(NUM|FLO, B_CALCEFFECT, "Nabla:",		911,9,91,20, &paf->nabla, 0.0001, 1.0, 1);
			DefBut(NUM|FLO, B_CALCEFFECT, "X:",			722,31,74,20, paf->defvec, -1.0, 1.0, 1);
			DefBut(NUM|FLO, B_CALCEFFECT, "Y:",			798,31,74,20, paf->defvec+1,-1.0, 1.0, 1);
			DefBut(NUM|FLO, B_CALCEFFECT, "Z:",			797,9,75,20, paf->defvec+2, -1.0, 1.0, 1);

		}
	}
	
	
	/* IPO BUTTONS ALS LAATSTE */
	if(G.sipo) {
		/* bestaat deze? */
		sa= G.curscreen->areabase.first;
		while(sa) {
			if(sa->spacetype==SPACE_IPO && sa->spacedata.first==G.sipo) break;
			sa= sa->next;
		}
		if(sa==0) return;
	}
	else return;
	if(G.sipo->ipo==0) return;
	if(G.sipo->ipo->curve.first==0) return;
	
	sprintf(str, "%.3f", G.sipo->v2d.tot.xmin);
	DefBut(LABEL, 0, str,			1020, 140, 100, 19, 0, 0, 0);
	sprintf(str, "%.3f", G.sipo->v2d.tot.xmax);
	DefBut(LABEL, 0, str,			1120, 140, 100, 19, 0, 0, 0);

	DefBut(NUM|FLO, 0, "Xmin:",		1020, 120, 100, 19, &G.sipo->tot.xmin, -G.sipo->v2d.max[0], G.sipo->v2d.max[0], 100);
	DefBut(NUM|FLO, 0, "Xmax:",		1120, 120, 100, 19, &G.sipo->tot.xmax, -G.sipo->v2d.max[0], G.sipo->v2d.max[0], 100);
	
	sprintf(str, "%.3f", G.sipo->v2d.tot.ymin);
	DefBut(LABEL, 0, str,			1020, 100, 100, 19, 0, 0, 0);
	sprintf(str, "%.3f", G.sipo->v2d.tot.ymax);
	DefBut(LABEL, 0, str,			1120, 100, 100, 19, 0, 0, 0);

	DefBut(NUM|FLO, 0, "Ymin:",		1020, 80, 100, 19, &G.sipo->tot.ymin, -G.sipo->v2d.max[1], G.sipo->v2d.max[1], 100);
	DefBut(NUM|FLO, 0, "Ymax:",		1120, 80, 100, 19, &G.sipo->tot.ymax, -G.sipo->v2d.max[1], G.sipo->v2d.max[1], 100);

	SetButCol(BUTSALMON);
	DefBut(BUT, B_MUL_IPO,	"SET",		1220,79,50,62);
	
	
	/* SPEED BUTTON */
	SetButCol(BUTGREY);
	DefBut(NUM|FLO, 0, "Speed:",		1020,23,164,28, &hspeed, 0.0, 180.0, 1);
	
	SetButCol(BUTSALMON);
	DefBut(BUT, B_SETSPEED,	"SET",		1185,23,83,29);

}




/* ***************************** WORLD ************************** */

void do_worldbuts(short event)
{
	World *wrld;
	MTex *mtex;
	
	switch(event) {
	case B_TEXCLEARWORLD:
		wrld= G.buts->lockpoin;
		mtex= wrld->mtex[ wrld->texact ];
		if(mtex) {
			if(mtex->tex) mtex->tex->id.us--;
			freeN(mtex);
			wrld->mtex[ wrld->texact ]= 0;
			allqueue(REDRAWBUTSWORLD, 0);
			allqueue(REDRAWOOPS, 0);
			preview_changed(curarea->win);
		}
		break;
	}
}

void worldbuts()
{
	World *wrld;
	MTex *mtex;
	ID *id;
	int xco, a, loos;
	char str[30], *strp;

	wrld= G.scene->world;
	if(wrld==0) return;

	SetButLock(wrld->id.lib!=0);
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|1,B_MATPRV,"Real",	286,190,71,19, &wrld->skytype);
	DefBut(TOG|SHO|BIT|0,B_MATPRV,"Blend",	208,190,74,19, &wrld->skytype);
	DefBut(TOG|SHO|BIT|2,B_MATPRV,"Paper",	361,190,71,19, &wrld->skytype);
	SetButCol(BUTGREY);
	DefBut(NUMSLI|FLO,B_MATPRV,"HoR ",		200,55,175,18,	&(wrld->horr), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"HoG ",		200,34,175,18,	&(wrld->horg), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"HoB ",		200,13,175,18,	&(wrld->horb), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"ZeR ",		200,136,175,18,	&(wrld->zenr), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"ZeG ",		200,116,175,18,	&(wrld->zeng), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"ZeB ",		200,96,175,18,	&(wrld->zenb), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"AmbR ",	380,55,175,18,	&(wrld->ambr), 0.0, 1.0, 0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"AmbG ",	380,34,175,18,	&(wrld->ambg), 0.0, 1.0 ,0,0);
	DefBut(NUMSLI|FLO,B_MATPRV,"AmbB ",	380,13,175,18,	&(wrld->ambb), 0.0, 1.0 ,0,0);

	DefBut(NUMSLI|FLO,0, "Expos ",	380,92,175,18,	&(wrld->exposure), 0.2, 5.0, 0, 0);

	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|0,REDRAWVIEW3D,"Mist",	571,190,100,19, &wrld->mode);
	SetButCol(BUTGREY);
	DefBut(ROW|SHO, 0, "Qua", 571, 170, 33, 19, &wrld->mistype, 1.0, 0.0);
	DefBut(ROW|SHO, 0, "Lin", 604, 170, 33, 19, &wrld->mistype, 1.0, 1.0);
	DefBut(ROW|SHO, 0, "Sqr", 637, 170, 33, 19, &wrld->mistype, 1.0, 2.0);
	
	DefBut(NUM|FLO,REDRAWVIEW3D, "Sta:",			571,150,100,19, &wrld->miststa,0.0,1000.0, 10);
	DefBut(NUM|FLO,REDRAWVIEW3D, "Di:",			571,130,100,19, &wrld->mistdist,0.0,1000.0, 100);
	DefBut(NUM|FLO,0,"Hi:",			571,110,100,19, &wrld->misthi,0.0,100.0, 10);
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|1,0,	"Stars",571,90,100,19, &wrld->mode);
	SetButCol(BUTGREY);
	DefBut(NUM|FLO,0,"StarDist:",	571,70,100,19, &(wrld->stardist), 2.0, 1000.0, 100);
	DefBut(NUM|FLO,0,"MinDist:",	571,50,100,19, &(wrld->starmindist), 0.0, 1000.0, 100);
	DefBut(NUM|FLO,0,"Size:",		571,30,100,19, &(wrld->starsize), 0.0, 10.0, 10);
	DefBut(NUM|FLO,0,"Colnoise:",	571,10,100,19, &(wrld->starcolnoise), 0.0, 1.0, 100);


	/* TEX CHANNELS */
	SetButCol(BUTGREY);
	xco= 745;
	for(a= 0; a<6; a++) {
		mtex= wrld->mtex[a];
		if(mtex && mtex->tex) splitIDname(mtex->tex->id.name+2, str, &loos);
		else strcpy(str, "");
		str[10]= 0;
		DefBut(ROW|SHO, REDRAWBUTSWORLD, str,	xco, 195, 83, 20, &(wrld->texact), 3.0, (float)a);
		xco+= 85;
	}
	
	mtex= wrld->mtex[ wrld->texact ];
	if(mtex==0) {
		mtex= &emptytex;
		default_mtex(mtex);
		mtex->texco= TEXCO_VIEW;
	}
	
	/* TEXCO */
	SetButCol(BUTGREEN);
	DefBut(ROW|SHO, B_MATPRV, "Object",		745,146,49,18, &(mtex->texco), 4.0, (float)TEXCO_OBJECT);
	SetButFunc(test_obpoin_but);
	DefBut(IDPOIN, B_MATPRV, "",		745,166,133,18, &(mtex->object));
	DefBut(ROW|SHO, B_MATPRV, "View",			839,146,39,18, &(mtex->texco), 4.0, (float)TEXCO_VIEW);
	
	SetButCol(BUTGREY);	
	DefBut(NUM|FLO, B_MATPRV, "dX",		745,114,133,18, mtex->ofs, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "dY",		745,94,133,18, mtex->ofs+1, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "dZ",		745,74,133,18, mtex->ofs+2, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeX",	745,50,133,18, mtex->size, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeY",	745,30,133,18, mtex->size+1, -20.0, 20.0, 10);
	DefBut(NUM|FLO, B_MATPRV, "sizeZ",	745,10,133,18, mtex->size+2, -20.0, 20.0, 10);
	
	/* TEXTUREBLOK SELECT */
	id= (ID *)mtex->tex;
	IDnames_to_pupstring(&strp, &(G.main->tex), id, &(G.buts->texnr));
	if(strp[0]) strcat(strp, "|ADD NEW %x 32767");
	else strcat(strp, "ADD NEW %x 32767");
	DefBut(MENU|SHO, B_WTEXBROWSE, strp, 900,146,20,19, &(G.buts->texnr));
	freeN(strp);
	if(id) {
		DefBut(TEX, B_IDNAME, "TE:",	900,166,163,19, id->name+2, 0.0, 18.0);
		sprintf(str, "%d", id->us);
		DefBut(BUT, 0, str,				996,146,21,19);
		DefBut(BUT, B_AUTOTEXNAME, "ICON 0 7 4", 1041,146,21,19);
		if(id->lib) {
			if(wrld->id.lib) DefBut(BUT, 0, "ICON 0 6 4",	1019,146,21,19);
			else DefBut(BUT, 0, "ICON 0 5 4",	1019,146,21,19);			
		}
		SetButCol(BUTSALMON);
		DefBut(BUT, B_TEXCLEARWORLD, "Clear", 922, 146, 72, 19);
		SetButCol(BUTGREY);
	}
	
	/* TEXTURE OUTPUT */
	DefBut(TOG|SHO|BIT|1, B_MATPRV, "Stencil",	900,114,52,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|2, B_MATPRV, "Neg",		954,114,38,18, &(mtex->texflag));
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "RGBtoInt",	994,114,69,18, &(mtex->texflag));
	
	DefBut(COL|FLO, B_MTEXCOL, "",				900,100,163,12, &(mtex->r));
	DefBut(NUMSLI|FLO, B_MATPRV, "R ",			900,80,163,18, &(mtex->r), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "G ",			900,60,163,18, &(mtex->g), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "B ",			900,40,163,18, &(mtex->b), 0.0, 1.0, B_MTEXCOL);
	DefBut(NUMSLI|FLO, B_MATPRV, "DVar ",		900,10,163,18, &(mtex->def_var), 0.0, 1.0, 0);
	
	/* MAP TO */
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|0, B_MATPRV, "Blend",		1087,166,81,18, &(mtex->mapto));
	DefBut(TOG|SHO|BIT|1, B_MATPRV, "Hori",		1172,166,81,18, &(mtex->mapto));
	DefBut(TOG|SHO|BIT|2, B_MATPRV, "ZenUp",		1087,147,81,18, &(mtex->mapto));
	DefBut(TOG|SHO|BIT|3, B_MATPRV, "ZenDo",		1172,147,81,18, &(mtex->mapto));
	
	SetButCol(BUTGREY);
	DefBut(ROW|SHO, B_MATPRV, "Blend",			1087,114,48,18, &(mtex->blendtype), 9.0, (float)MTEX_BLEND);
	DefBut(ROW|SHO, B_MATPRV, "Mul",			1136,114,44,18, &(mtex->blendtype), 9.0, (float)MTEX_MUL);
	DefBut(ROW|SHO, B_MATPRV, "Add",			1182,114,41,18, &(mtex->blendtype), 9.0, (float)MTEX_ADD);
	DefBut(ROW|SHO, B_MATPRV, "Sub",			1226,114,40,18, &(mtex->blendtype), 9.0, (float)MTEX_SUB);
	
	DefBut(NUMSLI|FLO, B_MATPRV, "Col ",		1087,50,179,18, &(mtex->colfac), 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Nor ",		1087,30,179,18, &(mtex->norfac), 0.0, 1.0, 0);
	DefBut(NUMSLI|FLO, B_MATPRV, "Var ",		1087,10,179,18, &(mtex->varfac), 0.0, 1.0, 0);


	previewdraw();
}


/* ****************************  VIEW ************************ */

void load_bgpic_image(char *name)
{
	Image *ima;
	View3D *vd;
	
	vd= curarea->spacedata.first;
	while(vd) {
		if(vd->spacetype==SPACE_VIEW3D) break;
		vd= vd->next;
	}
	if(vd==0 || vd->bgpic==0) return;
	
	ima= add_image(name);
	if(ima) {
		if(vd->bgpic->ima) {
			vd->bgpic->ima->id.us--;
		}
		vd->bgpic->ima= ima;
		
		free_image(ima);	/* forceer opnieuw inlezen */
		ima->ok= 1;
	}
	allqueue(REDRAWBUTSVIEW, 0);
	
}

void do_viewbuts(short event)
{
	View3D *vd;
	ID *id, *idtest;
	int a, nr=0;
	char *name;
	
	vd= curarea->spacedata.first;
	while(vd) {
		if(vd->spacetype==SPACE_VIEW3D) break;
		vd= vd->next;
	}
	
	if(vd==0) return;

	switch(event) {
	case B_LOADBGPIC:
		if(vd->bgpic && vd->bgpic->ima) name= vd->bgpic->ima->name;
		else name= G.ima;
		
		activate_imageselect(FILE_SPECIAL, "SELECT IMAGE", name, load_bgpic_image);
		break;
	case B_BLENDBGPIC:
		if(vd->bgpic && vd->bgpic->rect) setalpha_bgpic(vd->bgpic);
		break;
	case B_BGPICBROWSE:
		if(vd->bgpic) {
			nr= 0;
			id= (ID *)vd->bgpic->ima;
			
			idtest= G.main->image.first;
			while(idtest) {
				if(nr==G.buts->menunr) {
					break;
				}
				nr++;
				idtest= idtest->next;
			}
			if(idtest==0) {	/* geen new */
				return;
			}
			if(idtest!=id) {
				vd->bgpic->ima= (Image *)idtest;
				id_us_plus(idtest);
				if(id) id->us--;
				
				/* redraw forceren */
				if(vd->bgpic->rect) freeN(vd->bgpic->rect);
				vd->bgpic->rect= 0;
				allqueue(REDRAWBUTSVIEW, 0);
			}
		}
		break;
	case B_BGPICTEX:
		
		idtest= G.main->tex.first;
		while(idtest) {
			if(nr==G.buts->texnr) {
				break;
			}
			nr++;
			idtest= idtest->next;
		}
		if(G.vd->bgpic) G.vd->bgpic->tex= (Tex *)idtest;
		allqueue(REDRAWBUTSVIEW, 0);
		
		break;
		
	}
}

void viewbuts()
{
	View3D *vd;
	ID *id;
	int a, b;
	char *strp;
	
	/* op zoek naar spacedata */
	vd= curarea->spacedata.first;
	while(vd) {
		if(vd->spacetype==SPACE_VIEW3D) break;
		vd= vd->next;
	}
	
	if(vd==0) return;
	
	if(vd->flag & V3D_DISPBGPIC) {
		if(vd->bgpic==0) {
			vd->bgpic= callocN(sizeof(BGpic), "bgpic");
			vd->bgpic->size= 5.0;
			vd->bgpic->blend= 0.5;
		}
	}
	
	DefBut(TOG|SHO|BIT|1, REDRAWBUTSVIEW, "BackGroundPic",	347,160,127,29 , &vd->flag);
	if(vd->bgpic) {
		DefBut(NUM|FLO, 0, "Size:",					478,160,82,29, &vd->bgpic->size, 0.1, 250.0, 100);
		
		id= (ID *)vd->bgpic->ima;
		IDnames_to_pupstring(&strp, &(G.main->image), id, &(G.buts->menunr));
		if(strp[0]) {
			DefBut(MENU|SHO, B_BGPICBROWSE, strp, 347,112,20,19, &(G.buts->menunr));
		}
		freeN(strp);
		DefBut(BUT,	    B_LOADBGPIC, "LOAD",		370,112,189,19);
		DefBut(NUMSLI|FLO, B_BLENDBGPIC, "Blend:",	347,84,213,19,&vd->bgpic->blend, 0.0,1.0, 0);
		
		if(vd->bgpic->ima) 
			DefBut(TEX,	    0,"BGpic: ",			347,136,211,19,&vd->bgpic->ima->name,0.0,100.0);

		/* textureblok: */
		id= (ID *)vd->bgpic->tex;
		IDnames_to_pupstring(&strp, &(G.main->tex), id, &(G.buts->texnr));
		if(strp[0]) strcat(strp, "|CLEAR %x 32767");
		
		DefBut(MENU|SHO, B_BGPICTEX, strp,		347, 20, 20,19, &(G.buts->texnr));
		freeN(strp);
		DefBut(LABEL, 0, "Select texture for animated backgroundimage", 508, 20, 20,19);
		
		if(id) DefBut(TEX, B_IDNAME, "TE:",	347,0,211,19, id->name+2, 0.0, 18.0);
	}

	DefBut(NUM|FLO, 0, "Grid:",			347, 60, 105, 19, &vd->grid, 0.001, 1000.0, 100);
	DefBut(NUM|SHO, 0, "GridLines:",	452, 60, 105, 19, &vd->gridlines, 4.0, 100.0, 100);
	DefBut(NUM|FLO, 0, "Lens:",			557, 60, 105, 19, &vd->lens, 10.0, 120.0, 100);
	
	DefBut(NUM|FLO, 0, "Near:",			347, 40, 105, 19, &vd->near, 0.1*vd->grid, 100.0, 100);
	DefBut(NUM|FLO, 0, "Far:",			452, 40, 105, 19, &vd->far, 1.0, 1000.0*vd->grid, 100);

	/* for(b=0; b<8; b++) { */
	/* 	for(a=0; a<8; a++) { */
	/* 		DefBut(TOG|CHA|BIT|(7-a), 0, "", 100+12*a, 100-12*b, 12, 12, &(arr[b])); */
	/* 	} */
	/* } */
	/* DefBut(BUT, 1001, "print",	50,100,50,20); */
	
}

void output_pic(char *name)
{
	strcpy(G.scene->r.pic, name);
	allqueue(REDRAWBUTSRENDER, 0);
}

void backbuf_pic(char *name)
{
	Image *ima;
	
	strcpy(G.scene->r.backbuf, name);
	allqueue(REDRAWBUTSRENDER, 0);

	ima= add_image(name);
	if(ima) {
		free_image(ima);	/* forceer opnieuw inlezen */
		ima->ok= 1;
	}
}

void ftype_pic(char *name)
{
	strcpy(G.scene->r.ftype, name);
	allqueue(REDRAWBUTSRENDER, 0);
}

void movie_pic(char *name)
{
	strcpy(G.scene->r.movie, name);
	allqueue(REDRAWBUTSRENDER, 0);
}

/* ****************************  VIEW ************************ */


void do_renderbuts(short event)
{
	extern char *bprogname;	/* usiblender.c */
	ScrArea *sa;
	View3D *vd;
	ID *id, *idtest;
	int nr;
	short len, val;
	char *adr, str[128], text[30];

	switch(event) {

	case B_DORENDER:
		do_renderfg(0);
		break;
	case B_PLAYANIM:
		
		/* hier komt alleen het childprocess */
		len= sprintf(str, "%s -a ", bprogname);
		makepicstring(str+len, G.scene->r.sfra);
		if(exist(str+len)) {
			system(str);
		}
		else errorstr("can't find image", str+len, 0);
		break;
		
	case B_DOANIM:
		do_renderfg(1);
		break;
	
	case B_FS_PIC:
		sa= closest_bigger_area();
		areawinset(sa->win);
		activate_fileselect(FILE_SPECIAL, "SELECT OUTPUT PICTURES", G.scene->r.pic, output_pic);
		break;
	case B_FS_BACKBUF:
		sa= closest_bigger_area();
		areawinset(sa->win);
		activate_fileselect(FILE_SPECIAL, "SELECT BACKBUF PICTURE", G.scene->r.backbuf, backbuf_pic);
		break;
	case B_IS_BACKBUF:
		sa= closest_bigger_area();
		areawinset(sa->win);
		activate_imageselect(FILE_SPECIAL, "SELECT BACKBUF PICTURE", G.scene->r.backbuf, backbuf_pic);
		break;
	case B_FS_FTYPE:
		sa= closest_bigger_area();
		areawinset(sa->win);
		activate_fileselect(FILE_SPECIAL, "SELECT FTYPE", G.scene->r.ftype, ftype_pic);
		break;
	case B_IS_FTYPE:
		sa= closest_bigger_area();
		areawinset(sa->win);
		activate_imageselect(FILE_SPECIAL, "SELECT FTYPE", G.scene->r.ftype, ftype_pic);
		break;
	
	case B_FS_MOVIE:
		sa= closest_bigger_area();
		areawinset(sa->win);
		if(G.scene->r.movie[0]==0) strcpy(G.scene->r.movie, "/render/");
		activate_fileselect(FILE_SPECIAL, "SELECT MOVIE", G.scene->r.movie, movie_pic);
		break;

	case B_REDRAWDISP:
		sa= G.curscreen->areabase.first;
		while(sa) {
			if(sa->spacetype==SPACE_VIEW3D) {
				vd= sa->spacedata.first;
				if((vd->flag & V3D_DISPIMAGE)) addqueue(sa->win, REDRAW, 1);
			}
			sa= sa->next;
		}
		break;
	
	case B_PR_PAL:
		G.scene->r.xsch= 720;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 54;
		G.scene->r.yasp= 51;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;
		
		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWBUTSRENDER, 0);
		allqueue(REDRAWVIEWCAM, 0);
		break;
	case B_PR_FULL:
		G.scene->r.xsch= 1280;
		G.scene->r.ysch= 1024;
		G.scene->r.xasp= 1;
		G.scene->r.yasp= 1;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWBUTSRENDER, 0);
		allqueue(REDRAWVIEWCAM, 0);
		break;
	case B_PR_PRV:
		G.scene->r.xsch= 640;
		G.scene->r.ysch= 512;
		G.scene->r.xasp= 1;
		G.scene->r.yasp= 1;
		G.scene->r.size= 50;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_CDI:
		G.scene->r.xsch= 384;
		G.scene->r.ysch= 280;
		G.scene->r.xasp= 1;
		G.scene->r.yasp= 1;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.15, 0.85, 0.15, 0.85);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_PAL169:
		G.scene->r.xsch= 720;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 64;
		G.scene->r.yasp= 45;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_D2MAC:
		G.scene->r.xsch= 1024;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 1;
		G.scene->r.yasp= 1;
		G.scene->r.size= 50;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_MPEG:
		G.scene->r.xsch= 368;
		G.scene->r.ysch= 272;
		G.scene->r.xasp= 105;
		G.scene->r.yasp= 100;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_PC:
		G.scene->r.xsch= 640;
		G.scene->r.ysch= 480;
		G.scene->r.xasp= 100;
		G.scene->r.yasp= 100;
		G.scene->r.size= 100;
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.0, 1.0, 0.0, 1.0);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_PRESET:
		G.scene->r.xsch= 720;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 54;
		G.scene->r.yasp= 51;
		G.scene->r.size= 100;
		G.scene->r.mode= R_OSA+R_SHADOW+R_FIELDS;
		G.scene->r.imtype= R_TARGA;
		G.scene->r.xparts=  G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;

	case B_PR_PANO360:			/* de 360 graden versie */
		G.scene->r.xsch= 384;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 2;
		G.scene->r.yasp= 1;
		
		G.scene->r.mode |= R_PANORAMA;
		G.scene->r.xparts=  8;
		G.scene->r.yparts= 1;
		
		if(G.scene->camera) {
			Camera *cam= G.scene->camera->data;
			cam->lens= 38.6;
		}
		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
		
	case B_PR_HALFFIELDS:
		G.scene->r.xsch= 384;
		G.scene->r.ysch= 576;
		G.scene->r.xasp= 2;
		G.scene->r.yasp= 1;
		G.scene->r.mode |= R_FIELDS;
		
		G.scene->r.mode &= ~R_PANORAMA;
		G.scene->r.xparts=  1;
		G.scene->r.yparts= 1;
		
		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_PR_PANO:			/* de lostride versie */
		G.scene->r.xsch= 36;
		G.scene->r.ysch= 176;
		G.scene->r.xasp= 115;
		G.scene->r.yasp= 100;
		G.scene->r.size= 100;
		G.scene->r.mode |= R_PANORAMA;
		G.scene->r.xparts=  16;
		G.scene->r.yparts= 1;

		init_rctf(&G.scene->r.safety, 0.1, 0.9, 0.1, 0.9);
		allqueue(REDRAWVIEWCAM, 0);
		allqueue(REDRAWBUTSRENDER, 0);
		break;
	case B_SETBROWSE:
		nr= 0;
		id= (ID *)G.scene->set;

		idtest= G.main->scene.first;
		while(idtest) {
			if(nr==G.buts->menunr) {
				break;
			}
			nr++;
			idtest= idtest->next;
		}
		if(idtest==0) {	/* geen new */
			return;
		}
		if(idtest== (ID *)G.scene) {
			error("Not allowed");
			return;
		}
		
		if(idtest!=id) {
			G.scene->set= (Scene *)idtest;
			/* scene heeft geen users of usernummers! */
			
			allqueue(REDRAWBUTSRENDER, 0);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_CLEARSET:
		if(G.scene->set) {
			
			G.scene->set= 0;
			
			allqueue(REDRAWBUTSRENDER, 0);
			allqueue(REDRAWVIEW3D, 0);
		}
		break;
	case B_INFOMOVIE:
		break;
	case B_OSA_BLUR1:
		if(G.scene->r.mode & R_MBLUR) {
			G.scene->r.mode -= R_MBLUR;
			allqueue(REDRAWBUTSRENDER, 0);
		}
		break;
	case B_OSA_BLUR2:
		if(G.scene->r.mode & R_OSA) {
			G.scene->r.mode -= R_OSA;
			allqueue(REDRAWBUTSRENDER, 0);
		}
		break;
	}
}

void renderbuts()
{
	ID *id;
	int a,b;
	static short voidshort= 0;
	char *strp;
	
	DefBut(TEX,0,"",				34,172,257,19,G.scene->r.pic, 0.0,79.0);
	DefBut(BUT,B_FS_PIC," ",		10,172,22,19);
	DefBut(TEX,0,"",				34,149,257,19,G.scene->r.backbuf, 0.0,79.0);
	DefBut(BUT,B_FS_BACKBUF," ",	21,149,11,19);
	DefBut(TEX,0,"",				34,126,257,19,G.scene->r.ftype,0.0,79.0);
	DefBut(BUT,B_FS_FTYPE," ",		21,126,11,19);
	DefBut(TEX,0,"",				34,103,231,19,G.scene->r.movie,0.0,79.0);
	DefBut(BUT,B_FS_MOVIE," ",		10,103,22,19);

	SetButCol(BUTSALMON);
	DefBut(BUT,B_IS_BACKBUF," ",	10,149,11,19);
	DefBut(BUT,B_IS_FTYPE," ",		10,126,11,19);
	SetButCol(BUTGREY);

	DefBut(LABEL,0,"Pics",				295,172,63,19, 0, 0, 0);
	DefBut(TOG|SHO|BIT|0, 0,"Backbuf",	295,149,63,19, &G.scene->r.bufflag);	
	DefBut(LABEL,0,"Ftype",				295,126,63,19, 0, 0, 0);
	DefBut(LABEL,0,"Movie",				295,103,63,19, 0, 0, 0);

	/* SET BUTTON */
	id= (ID *)G.scene->set;
	IDnames_to_pupstring_nr(&strp, &(G.main->scene), id, &(G.buts->menunr), 45);
	if(strp[0]) {
		DefBut(MENU|SHO, B_SETBROWSE, strp, 10,80,22,19, &(G.buts->menunr));
	}
	freeN(strp);
	
	SetButCol(BUTBLUE);

	if(G.scene->set) {
		SetButLock(1);
		SetButFunc(test_scenepoin_but);
		DefBut(IDPOIN, 0, "",			34,80,231,19, &(G.scene->set));
		ClearButLock();
	}
	SetButCol(BUTGREY);
	
	mdec_to_pupstring(&strp);
	DefBut(MENU|SHO, 0, strp, 267,102,24,21, &voidshort);
	freeN(strp);
	DefBut(BUT, B_CLEARSET, "ICON 0 0 4", 267,79,24,21);
	
	DefBut(LABEL, 0, "Set",				295,80,63,19, 0, 0, 0);
	
	for(b=0; b<3; b++) for(a=0;a<3;a++) {
		DefBut(TOG|SHO|BIT|(3*b+a),800,"",	34+18*a,11+12*b,16,10, &R.winpos);
	}

	DefBut(ROW|SHO, 0, "DispView",	99,28,77,18, &R.displaymode, 0.0, (float)R_DISPLAYVIEW);
	DefBut(ROW|SHO, B_REDRAWDISP, "DispWin",	99,10,78,18, &R.displaymode, 0.0, (float)R_DISPLAYWIN);


	SetButCol(BUTSALMON);
	DefBut(BUT,B_DORENDER,"RENDER",	369,142,192,47);
	
	SetButCol(BUTGREY);
	DefBut(TOG|SHO|BIT|1,0,"Shadows",	565,167,122,22, &G.scene->r.mode);
	/* DefBut(TOG|SHO|BIT|4,0,"Trace",	565,144,60,20, &G.scene->r.mode); */
	DefBut(TOG|SHO|BIT|10,0,"Panorama",565,144,122,20, &G.scene->r.mode);
	
	DefBut(ROW|SHO,0,"100%",		565,114,121,20,&G.scene->r.size,1.0,100.0);
	DefBut(ROW|SHO,0,"75%",			565,90,36,20,&G.scene->r.size,1.0,75.0);
	DefBut(ROW|SHO,0,"50%",			604,90,40,20,&G.scene->r.size,1.0,50.0);
	DefBut(ROW|SHO,0,"25%",			647,90,39,20,&G.scene->r.size,1.0,25.0);
	
	DefBut(TOG|SHO|BIT|0, B_OSA_BLUR1, "OSA",		369,114,124,20,&G.scene->r.mode);
	#ifndef FREE
	DefBut(NUM|FLO,0,"Bf:",							495,90,65,20,&G.scene->r.blurfac, 0.01, 5.0, 10);
	DefBut(TOG|SHO|BIT|13, B_OSA_BLUR2, "MBLUR",	495,114,66,20,&G.scene->r.mode);
	#endif
	
	DefBut(ROW|SHO,0,"5",			369,90,29,20,&G.scene->r.osa,2.0,5.0);
	DefBut(ROW|SHO,0,"8",			400,90,29,20,&G.scene->r.osa,2.0,8.0);
	DefBut(ROW|SHO,0,"11",			431,90,33,20,&G.scene->r.osa,2.0,11.0);
	DefBut(ROW|SHO,0,"16",			466,90,28,20,&G.scene->r.osa,2.0,16.0);
		
	DefBut(NUM|SHO,0,"Xparts:",		369,42,99,31,&G.scene->r.xparts,1.0, 64.0);
	DefBut(NUM|SHO,0,"Yparts:",		472,42,86,31,&G.scene->r.yparts,1.0, 64.0);

	DefBut(TOG|SHO|BIT|6,0,"Fields",564,42,106,31,&G.scene->r.mode);
	DefBut(TOG|SHO|BIT|7,0,"x",		671,42,14,31,&G.scene->r.mode);

	DefBut(ROW|SHO,800,"Sky",		369,11,38,24,&G.scene->r.alphamode,3.0,0.0);
	DefBut(ROW|SHO,800,"Premul",	410,11,54,24,&G.scene->r.alphamode,3.0,1.0);
	DefBut(ROW|SHO,800,"Key",		467,11,44,24,&G.scene->r.alphamode,3.0,2.0);


	/* DefBut(TOG|SHO|BIT|10,800,"Edge",168,37,70,22,&R.mode); */
	/* DefBut(NUM|SHO,800,"Eint:",		168,11,70,22,&R.edgeint,0.0,255.0); */

	DefBut(TOG|SHO|BIT|9,REDRAWVIEWCAM, "Border",	565,11,58,24, &G.scene->r.mode);
	DefBut(TOG|SHO|BIT|2,0, "Gamma",	626,11,58,24, &G.scene->r.mode);

	SetButCol(BUTSALMON);
	DefBut(BUT,B_DOANIM,"ANIM",		692,142,192,47);
	
	SetButCol(BUTBLUE);
	#ifdef FREE
	DefBut(TOG|SHO|BIT|0, 0, "Do Sequence",		692,115,192,24, &G.scene->r.scemode);
	DefBut(TOG|SHO|BIT|1, 0, "Render Daemon",	692,88,192,24, &G.scene->r.scemode);
	#else
	DefBut(TOG|SHO|BIT|0, 0, "Do Sequence",		692,115,119,24, &G.scene->r.scemode);
	DefBut(TOG|SHO|BIT|2, B_REDR, "NetWork",	813,115,70,24, &G.scene->r.scemode);
	DefBut(TOG|SHO|BIT|1, 0, "Render Daemon",	692,88,119,24, &G.scene->r.scemode);
	DefBut(TOG|SHO|BIT|3, B_REDR, "HotSpot",	813,88,70,24, &G.scene->r.scemode);
	#endif
	
	SetButCol(BUTGREY);
	DefBut(BUT,B_PLAYANIM, "PLAY",	692,40,94,33);

	DefBut(NUM|SHO, 0, "rt:",	790,40,95,33, &R.rt, 0.0, 256.0);

	DefBut(ROW|SHO,0,"BW",		891,10,48,20, &G.scene->r.planes, 5.0,(float)R_PLANESBW);
	DefBut(ROW|SHO,0,"RGB",		942,10,47,20, &G.scene->r.planes, 5.0,(float)R_PLANES24);
	DefBut(ROW|SHO,0,"RGBA",	992,10,55,20, &G.scene->r.planes, 5.0,(float)R_PLANES32);
	DefBut(ROW|SHO,0,"IRIZ",	1050,10,52,20, &G.scene->r.imtype, 6.0,(float)R_IRIZ);

	DefBut(ROW|SHO,0,"Targa",	892,90,62,20, &G.scene->r.imtype,6.0,(float)R_TARGA);
	#ifdef FREE
	DefBut(ROW|SHO,0,"Iris",	957,90,37,20, &G.scene->r.imtype,6.0,(float)R_IRIS);
	DefBut(ROW|SHO,0,"Ftype",	994,68,48,20, &G.scene->r.imtype,6.0,(float)R_FTYPE);
	#else
	DefBut(ROW|SHO,0,"Mdec",	957,90,37,20, &G.scene->r.imtype,6.0,(float)R_MDEC);
	DefBut(ROW|SHO,0,"TGA 4",	994,68,48,20, &G.scene->r.imtype,6.0,(float)R_TGA4);
	/* DefBut(ROW|SHO,0,"CMBB",	994,68,48,20, &G.scene->r.imtype,6.0,(float)R_CMBB); */
	#endif
	DefBut(ROW|SHO,0,"JPEG",	996,90,55,20, &G.scene->r.imtype,6.0,(float)R_JPEG90);
	DefBut(ROW|SHO,0,"HamX",	892,68,50,20, &G.scene->r.imtype,6.0,(float)R_HAMX);
	DefBut(ROW|SHO,0,"Ftype",	945,68,49,20, &G.scene->r.imtype,6.0,(float)R_FTYPE);

	if(G.scene->r.quality==0) G.scene->r.quality= 90;
	DefBut(NUM|SHO,0, "Quality:", 892,46,73,20, &G.scene->r.quality, 10.0, 100.0);

	#ifdef __sgi
	DefBut(ROW|SHO,0,"Movie",	1043,68,58,20, &G.scene->r.imtype,6.0,(float)R_MOVIE);
	DefBut(NUM|SHO,0, "MaxSize:", 968,46,86,20, &G.scene->r.maximsize, 0.0, 500.0);
	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|11,0, "Crop", 1056,46,45,20, &G.scene->r.mode);
	DefBut(TOG|SHO|BIT|12,0, "Cosmo", 1052,90,50,20, &G.scene->r.mode);
	#endif

	SetButCol(BUTGREEN);
	DefBut(TOG|SHO|BIT|11,0, "Crop", 1056,46,45,20, &G.scene->r.mode);

	SetButCol(BUTGREY);
	DefBut(NUM|SHO,REDRAWSEQ,"Sta:",	692,11,94,24, &G.scene->r.sfra,1.0,18000.0);
	DefBut(NUM|SHO,REDRAWSEQ,"End:",	790,11,95,24, &G.scene->r.efra,1.0,18000.0);

	DefBut(NUM|SHO,REDRAWVIEWCAM,"SizeX:",	892,158,100,31, &G.scene->r.xsch, 4.0, 4096.0);
	DefBut(NUM|SHO,REDRAWVIEWCAM,"SizeY:",	995,158,107,31, &G.scene->r.ysch, 4.0,4096.0);
	DefBut(NUM|SHO,REDRAWVIEWCAM,"AspX:",	892,132,100,20, &G.scene->r.xasp, 1.0,200.0);
	DefBut(NUM|SHO,REDRAWVIEWCAM,"AspY:",	995,132,107,20, &G.scene->r.yasp, 1.0,200.0);

	DefBut(BUT,B_PR_PAL, "PAL",			1129,170,133,18);
	DefBut(BUT,B_PR_PRESET, "Default",	1129,149,133,18);
	DefBut(BUT,B_PR_PRV, "Preview",		1129,115,133,18);
	/* DefBut(BUT,B_PR_CDI, "CDI",			1129,94,133,18); */
	DefBut(BUT,B_PR_PC, "PC",			1129,94,133,18);
	/* DefBut(BUT,B_PR_PAL169, "PAL 16:9",	1129,73,133,18); */
	DefBut(BUT,B_PR_HALFFIELDS, "HalfFields",	1129,73,133,18);
	DefBut(BUT,B_PR_PANO360, "PANO 360",		1129,52,133,18);
	DefBut(BUT,B_PR_MPEG, "MPEG",		1129,32,133,18);
	DefBut(BUT,B_PR_FULL, "FULL",		1129,12,133,18);
}


/* ***************************<>******************************** */
/* ***************************<>******************************** */
/* ***************************<>******************************** */

void drawbutspace()
{
	ID *id;
	Object *ob;
	float vec[2];
	char naam[20];
	
	ob= OBACT;
	
	ortho2(G.v2d->cur.xmin, G.v2d->cur.xmax, G.v2d->cur.ymin, G.v2d->cur.ymax);
	
	shademodel(GOURAUD);
	bgnpolygon();
	cpack(0x606060);
	vec[0]= G.v2d->cur.xmin;
	vec[1]= G.v2d->cur.ymax-15;
	v2f(vec);
	vec[0]= G.v2d->cur.xmax;
	v2f(vec);
	cpack(0x444444);
	vec[1]= G.v2d->cur.ymax;
	v2f(vec);
	vec[0]= G.v2d->cur.xmin;
	v2f(vec);
	endpolygon();
	shademodel(FLAT);
	
	cpack(0x606060);
	sboxf(G.v2d->cur.xmin, G.v2d->cur.ymin, G.v2d->cur.xmax, G.v2d->cur.ymax-15);

	sprintf(naam, "win %d", curarea->win);
	DefButBlock(naam, curarea->win, G.font, 120, 10, 2);
	
	SetButLock(G.scene->id.lib!=0);

	switch(G.buts->mainb) {
	case BUTS_VIEW:
		viewbuts();
		break;
	case BUTS_LAMP:
		lampbuts();
		break;
	case BUTS_MAT:
		if(ob==0) return;
		if ELEM(ob->type, OB_SECTOR, OB_LIFE) matsectorbuts();
		if(ob->type>=OB_LAMP) return;
		
		matbuts();
		break;
	case BUTS_TEX:		
		texbuts();
		break;
	case BUTS_ANIM:
		animbuts();
		break;
	case BUTS_WORLD:
		worldbuts();
		break;
	case BUTS_RENDER:
		renderbuts();
		break;
	case BUTS_GAME:
		gamebuts();
		break;
	case BUTS_FPAINT:
		fpaintbuts();
		break;
	case BUTS_EDIT:
		if(ob==0) return;
		
		common_editbuts();

		id= ob->data;
		if(id && id->lib) SetButLock(1);

		if ELEM3(ob->type, OB_MESH, OB_SECTOR, OB_LIFE) meshbuts();
		else if ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT) {
			curvebuts();
			if(ob->type==OB_FONT) fontbuts();
		}
		else if(ob->type==OB_CAMERA) camerabuts();
		else if(ob->type==OB_MBALL) mballbuts();
		else if(ob->type==OB_LATTICE) latticebuts();
		else if(ob->type==OB_IKA) ikabuts();
			
		break;
	}
	
	curarea->win_swap= 1;
}

void do_blenderbuttons(short event)
{
	SpaceButs *buts;
	
	/* teken ook de soortgelijke windows? */
	buts= curarea->spacedata.first;
	if(buts->mainb==BUTS_VIEW) allqueue(REDRAWBUTSVIEW, curarea->win);
	else if(buts->mainb==BUTS_LAMP) allqueue(REDRAWBUTSLAMP, curarea->win);
	else if(buts->mainb==BUTS_MAT || buts->mainb==BUTS_TEX) {
		allqueue(REDRAWBUTSMAT, curarea->win);
		allqueue(REDRAWBUTSTEX, curarea->win);
	}
	else if(buts->mainb==BUTS_WORLD) allqueue(REDRAWBUTSWORLD, curarea->win);
	else if(buts->mainb==BUTS_ANIM) allqueue(REDRAWBUTSANIM, curarea->win);
	else if(buts->mainb==BUTS_RENDER) allqueue(REDRAWBUTSRENDER, curarea->win);
	else if(buts->mainb==BUTS_EDIT) allqueue(REDRAWBUTSEDIT, curarea->win);
	
	if(event<=100) {
		do_global_buttons(event);
	}
	else if(event<=B_VIEWBUTS) {
		do_viewbuts(event);
	}
	else if(event<=B_LAMPBUTS) {
		do_lampbuts(event);
	}
	else if(event<=B_MATBUTS) {
		do_matbuts(event);
	}
	else if(event<=B_TEXBUTS) {
		do_texbuts(event);
	}
	else if(event<=B_ANIMBUTS) {
		do_animbuts(event);
	}
	else if(event<=B_WORLDBUTS) {
		do_worldbuts(event);
	}
	else if(event<=B_RENDERBUTS) {
		do_renderbuts(event);
	}
	else if(event<=B_COMMONEDITBUTS) {
		do_common_editbuts(event);
	}
	else if(event<=B_MESHBUTS) {
		do_meshbuts(event);
	}
	else if(event<=B_CURVEBUTS) {
		do_curvebuts(event);
	}
	else if(event<=B_FONTBUTS) {
		do_fontbuts(event);
	}
	else if(event<=B_IKABUTS) {
		do_ikabuts(event);
	}
	else if(event<=B_CAMBUTS) {
		;
	}
	else if(event<=B_MBALLBUTS) {
		do_mballbuts(event);
	}
	else if(event<=B_LATTBUTS) {
		do_latticebuts(event);
	}
	else if(event<=B_GAMEBUTS) {
		do_gamebuts(event);
	}
	else if(event<=B_FPAINTBUTS) {
		do_fpaintbuts(event);
	}
	else if(event>=REDRAWVIEW3D) allqueue(event, 0);
	
	
}


void redraw_test_buttons(Base *new)
{
	ScrArea *sa;
	SpaceButs *buts;
	
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa->spacetype==SPACE_BUTS) {
			buts= sa->spacedata.first;
			
			if(buts->mainb==BUTS_LAMP) {
				allqueue(REDRAWBUTSLAMP, 0);
				preview_changed(sa->win);
			}
			else if(buts->mainb==BUTS_MAT) {
				allqueue(REDRAWBUTSMAT, 0);
				preview_changed(sa->win);
			}
			else if(buts->mainb==BUTS_TEX) {
				allqueue(REDRAWBUTSTEX, 0);
				if(new && new->object->type==OB_LAMP) buts->texfrom= 2;
				else buts->texfrom= 0;
				preview_changed(sa->win);
			}
			else if(buts->mainb==BUTS_ANIM) {
				allqueue(REDRAWBUTSANIM, 0);
			}			
			else if(buts->mainb==BUTS_EDIT) {
				allqueue(REDRAWBUTSEDIT, 0);
			}			
			else if(buts->mainb==BUTS_GAME) {
				allqueue(REDRAWBUTSGAME, 0);
			}			
			else if(buts->mainb==BUTS_FPAINT) {
				allqueue(REDRAWBUTSGAME, 0);
			}			
		}
		sa= sa->next;
	}
}

/* ****************** EXTRAATJE **************** */

short button(short *var, short min, short max, char *str)
{
	short x1,x2,y1,y2;
	short oldmap[4][3], toets,val,mval[2], ret=0;

	if(min>max) min= max;

	getmouseco_sc(mval);
	
	if(mval[0]<150) mval[0]=150;
	if(mval[1]<30) mval[1]=30;
	if(mval[0]>1240) mval[0]=1240;
	if(mval[1]>980) mval[1]=980;

	winset(G.curscreen->mainwin);
	drawmode(PUPDRAW);

	if(G.machine==ENTRY) {
		getmcolor(1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor(2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor(3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		mapcolor(1, 170, 170, 170); 
		mapcolor(2, 0, 0, 0); 
		mapcolor(3, 240, 240, 240);
	}
	
	x1=mval[0]-150; 
	x2=mval[0]+20; 
	y1=mval[1]-20; 
	y2=mval[1]+20;
	color(1);
	setpattern(1);
	sboxfs(x1,y1+5,x2,y2-5);
	setpattern(0);
	sboxs(x1,y1+5,x2,y2-5);

	DefButBlock("button",G.curscreen->mainwin, G.font, 2, 1, 1);

	DefBut(NUM|SHO,4001,str,	x1+5,y1+10,125,20,var,(float)min,(float)max);
	DefBut(BUT,4002,"OK",	x1+136,y1+10,25,20);

	while(TRUE) {
		toets= extern_qread(&val);
		if(val) {
			if (toets == ESCKEY) break;
			if (toets == RETKEY) {
				ret = 1;
				break;
			}

			if(toets==LEFTMOUSE) {
				getmouseco_sc(mval);
				if(mval[0]<x1 || mval[0]>x2 || mval[1]<y1 || mval[1]>y2) break;
				else {
					if(DoButtons()==4002) {
						ret=1;
						break;
					}
				}
			}
		}
	}
	while(getbutton(LEFTMOUSE));

	FreeButBlock("button");
	color(0);
	sboxfs(x1,y1,x2,y2);

	if(G.machine==ENTRY) {
		mapcolor(1, oldmap[1][0], oldmap[1][1], oldmap[1][2]);
		mapcolor(2, oldmap[2][0], oldmap[2][1], oldmap[2][2]);
		mapcolor(3, oldmap[3][0], oldmap[3][1], oldmap[3][2]);
	}

	remake_qual();	/* in Button.c zit geen extern_qread */
	
	drawmode(NORMALDRAW);
	areawinset(curarea->win);
	return ret;
}


void do_mlayer_toets(int event, ulong *lay)
{
	static ulong oldlay= 1;
	
	if(event== -1) {
		if(*lay== (2<<20)-1) {
			if(G.qual & LR_SHIFTKEY) *lay= oldlay;
		}
		else {
			oldlay= *lay;
			*lay= (2<<20)-1;
		}
	}
	else {
		if(G.qual & LR_ALTKEY) {
			if(event<11) event+= 10;
		}
		if(G.qual & LR_SHIFTKEY) {
			if(*lay & (1<<event)) *lay -= (1<<event);
			else  *lay += (1<<event);
		}
		else *lay= (1<<event);
	}

}


int movetolayer_buts(ulong *lay)
{
	ulong oldlay;
	int bit;
	int dx, dy, a, x1, x2, y1, y2, sizex=160, sizey=30;
	short oldmap[4][3], toets, val, mval[2], ret=0;
	char str[12];
	
	if(G.vd->localview) {
		error("Not in localview ");
		return;
	}

	getmouseco_sc(mval);
	
	if(mval[0]<sizex/2) mval[0]=sizex/2;
	if(mval[1]<sizey/2) mval[0]=sizey/2;
	if(mval[0]>G.curscreen->sizex -sizex/2) mval[0]= G.curscreen->sizex -sizex/2;
	if(mval[1]>G.curscreen->sizey -sizey/2) mval[1]= G.curscreen->sizey -sizey/2;

	winset(G.curscreen->mainwin);
	drawmode(PUPDRAW);

	if(G.machine==ENTRY) {
		getmcolor(1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor(2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor(3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		mapcolor(1, 170, 170, 170); 
		mapcolor(2, 0, 0, 0); 
		mapcolor(3, 240, 240, 240);
	}
	
	x1= mval[0]-sizex/2; 
	x2= mval[0]+sizex/2; 
	y1= mval[1]-sizey/2; 
	y2= mval[1]+sizey/2;
	
	color(1);
	setpattern(1);
	sboxfi(x1-4, y1-4, x2+4, y2+4);
	setpattern(0);
	color(2);
	sboxi(x1-4, y1-4, x2+4, y2+4);
	
	DefButBlock("button", G.curscreen->mainwin, G.font, 32, 1, 1);

	dx= (sizex-5)/12;
	dy= sizey/2;
	
	for(a=0; a<10; a++) {
		DefBut(TOG|INT|BIT|a+10, 4001+10+a, "",	x1+a*dx, y1, dx, dy, lay);
		DefBut(TOG|INT|BIT|a, 4001+a, "",	x1+a*dx, y1+dy, dx, dy, lay);
		if(a==4) x1+= 5;
	}
	x1-= 5;
	
	DefBut(BUT, 4000, "OK", x1+10*dx+10, y1, 2*dx, 2*dy);
	
	while(TRUE) {
		toets= extern_qread(&val);
		if(val) {
		
			oldlay= *lay;
			
			if(toets==ONEKEY) {
				do_mlayer_toets(0, lay);
			}
			else if(toets==TWOKEY) {
				do_mlayer_toets(1, lay);
			}
			else if(toets==THREEKEY) {
				do_mlayer_toets(2, lay);
			}
			else if(toets==FOURKEY) {
				do_mlayer_toets(3, lay);
			}
			else if(toets==FIVEKEY) {
				do_mlayer_toets(4, lay);
			}
			else if(toets==SIXKEY) {
				do_mlayer_toets(5, lay);
			}
			else if(toets==SEVENKEY) {
				do_mlayer_toets(6, lay);
			}
			else if(toets==EIGHTKEY) {
				do_mlayer_toets(7, lay);
			}
			else if(toets==NINEKEY) {
				do_mlayer_toets(8, lay);
			}
			else if(toets==ZEROKEY) {
				do_mlayer_toets(9, lay);
			}
			else if(toets==MINUSKEY) {
				do_mlayer_toets(10, lay);
			}
			else if(toets==EQUALKEY) {
				do_mlayer_toets(11, lay);
			}
			else if(toets==ACCENTGRAVEKEY) {
				do_mlayer_toets(-1, lay);
			}
			
			if(*lay!=oldlay) {
				/* FrontbufferButs(TRUE); */
				SetButs(4001, 4020);
				/* FrontbufferButs(FALSE); */
			}
			
			if (toets == ESCKEY) break;
			else if (toets == RETKEY) {
				ret = 1;
				break;
			}
			else if(toets==MOUSEX || toets==MOUSEY) {
				getmouseco_sc(mval);
				if(mval[0]<x1-40 || mval[0]>x2+40 || mval[1]<y1-30 || mval[1]>y2+30) break;
			}
			else if(toets==LEFTMOUSE) {
				getmouseco_sc(mval);
				
				val= DoButtons();
				if(val) {
					if(val==4000) {
						ret= 1;
						break;
					}
					else if(val>4000 && val<4032) {
						if(*lay==0 || (G.qual & LR_SHIFTKEY)==0) {
							bit= val-4001;
							*lay= 1<<bit;
							SetButs(4001, 4031);
						}
					}
				}
			}
		}
	}
	while(getbutton(LEFTMOUSE));

	FreeButBlock("button");
	color(0);
		/* 1 pixel groter wissen ivm linewidth 2 op entry */
	sboxfi(x1-5, y1-5, x2+5, y2+5);

	if(G.machine==ENTRY) {
		mapcolor(1, oldmap[1][0], oldmap[1][1], oldmap[1][2]);
		mapcolor(2, oldmap[2][0], oldmap[2][1], oldmap[2][2]);
		mapcolor(3, oldmap[3][0], oldmap[3][1], oldmap[3][2]);
	}

	drawmode(NORMALDRAW);
	areawinset(curarea->win);
	return ret;
}

/* ********************** CLEVER_NUMBUTS ******************** */

#define MAXNUMBUTS	24

VarStruct numbuts[MAXNUMBUTS];
void *numbpoin[MAXNUMBUTS];
int numbdata[MAXNUMBUTS];

int do_clever_numbuts(char *name, int tot, int winevent)
{
	VarStruct *varstr;
	int bit;
	int dx, dy, a, x1, x2, y1, y2, sizex, sizey;
	short oldmap[4][3], toets, val, mval[2], ret=0;

	if(tot<=0 || tot>MAXNUMBUTS) return 0;

	getmouseco_sc(mval);

	/* size */
	sizex= 235;
	sizey= 30+20*(tot+1);
	
	/* midden */
	if(mval[0]<sizex/2) mval[0]=sizex/2;
	if(mval[1]<sizey/2) mval[1]=sizey/2;
	if(mval[0]>G.curscreen->sizex -sizex/2) mval[0]= G.curscreen->sizex -sizex/2;
	if(mval[1]>G.curscreen->sizey -sizey/2) mval[1]= G.curscreen->sizey -sizey/2;

	winset(G.curscreen->mainwin);
	drawmode(PUPDRAW);

	if(G.machine==ENTRY) {
		getmcolor(1, &oldmap[1][0], &oldmap[1][1], &oldmap[1][2]);
		getmcolor(2, &oldmap[2][0], &oldmap[2][1], &oldmap[2][2]);
		getmcolor(3, &oldmap[3][0], &oldmap[3][1], &oldmap[3][2]);
		mapcolor(1, 170, 170, 170); 
		mapcolor(2, 0, 0, 0); 
		mapcolor(3, 240, 240, 240);
	}
	
	x1= mval[0]-sizex/2; 
	x2= mval[0]+sizex/2; 
	y1= mval[1]-sizey/2; 
	y2= mval[1]+sizey/2;
	
	color(1);
	setpattern(1);
	sboxfi(x1-4, y1-4, x2+4, y2+4);
	setpattern(0);
	color(2);
	sboxi(x1-4, y1-4, x2+4, y2+4);
	
	DefButBlock("button", G.curscreen->mainwin, G.font, 32, 1, 1);
	
/* LET OP: TEX BUTTON UITZONDERING */
/* WAARSCHUWING: ALLEEN EEN ENKELE BITJES-BUTTON MOGELIJK: ER WORDT OP KOPIEDATA GEWERKT! */

	DefBut(LABEL, 0, name,	x1+5, y2-21, sizex-60, 19, 1.0, 0.0);	/* 0.0 op eind: geen clear */
	if(name[0]=='A' && name[7]=='O')
		DefBut(LABEL, 0, "Rotations in degrees!",	x1+5, y2-36, sizex-60, 19, 0.0, 0.0);	/* 0.0 op eind: geen clear */

	varstr= &numbuts[0];
	for(a=0; a<tot; a++, varstr++) {
		if( (varstr->type & BUTTYPE)==SLI)
			DefBut(varstr->type, 0,	varstr->name,	x1+50, y2-50-20*a, sizex-120, 8, &(numbdata[a]), varstr->min, varstr->max, 100, 0, 0);
		else  if(varstr->type==TEX)
			DefBut(TEX, 0,	varstr->name,	x1+15, y2-55-20*a, sizex-60, 19, numbpoin[a], varstr->min, varstr->max, 0, 0, 0);
		else  
			DefBut(varstr->type, 0,	varstr->name,	x1+15, y2-55-20*a, sizex-60, 19, &(numbdata[a]), varstr->min, varstr->max, 100, 0, 0);
	}

	DefBut(BUT, 4000, "OK", x1+sizex-40, y1+15, 25, sizey-50);
	
	while(TRUE) {
		toets= extern_qread(&val);
		if(val) {
		
			if (toets == ESCKEY) break;
			else if ELEM(toets, RETKEY, PADENTER) {
				ret = 1;
				break;
			}
			else if(toets==MOUSEX || toets==MOUSEY) {
				getmouseco_sc(mval);
				if(mval[0]<x1-120 || mval[0]>x2+120 || mval[1]<y1-50 || mval[1]>y2+50) break;
			}
			else if(toets==LEFTMOUSE) {
				getmouseco_sc(mval);

				val= DoButtons();
				if(val) {
					if(val==4000) {
						ret= 1;
						break;
					}
				}
			}
		}
	}
	while(getbutton(LEFTMOUSE));

	FreeButBlock("button");
	color(0);
		/* 1 pixel groter wissen ivm linewidth 2 op entry */
	sboxfi(x1-5, y1-5, x2+5, y2+5);

	if(G.machine==ENTRY) {
		mapcolor(1, oldmap[1][0], oldmap[1][1], oldmap[1][2]);
		mapcolor(2, oldmap[2][0], oldmap[2][1], oldmap[2][2]);
		mapcolor(3, oldmap[3][0], oldmap[3][1], oldmap[3][2]);
	}

	drawmode(NORMALDRAW);
	areawinset(curarea->win);
	remake_qual();	/* in Button.c zit geen extern_qread */
	
	if(ret==1) {
		varstr= &numbuts[0];
		for(a=0; a<tot; a++, varstr++) {
			if(varstr->type==TEX);
			else if ELEM3( (varstr->type & BUTPOIN), FLO, INT, LON ) memcpy(numbpoin[a], numbdata+a, 4);
			else if((varstr->type & BUTPOIN)==SHO ) *((short *)(numbpoin[a]))= *( (short *)(numbdata+a));

			if( strncmp(varstr->name, "Rot", 3)==0 ) {
				float *fp;
				
				fp= numbpoin[a];
				fp[0]= M_PI*fp[0]/180.0;
			}
		}
		
		if(winevent) addqueue(curarea->win, winevent, 1);
		
		return 1;
	}
	return 0;
}

void add_numbut(int nr, int type, char *str, float min, float max, void *poin)
{
	if(nr>=MAXNUMBUTS) return;

	numbuts[nr].type= type;
	strcpy(numbuts[nr].name, str);
	numbuts[nr].min= min;
	numbuts[nr].max= max;
	
	/* LET OP: TEX BUTTON UITZONDERING */
	
	numbpoin[nr]= poin;
	
	if ELEM3( (type & BUTPOIN), FLO, INT, LON ) memcpy(numbdata+nr, poin, 4);
	if((type & BUTPOIN)==SHO ) *((short *)(numbdata+nr))= *( (short *)poin);
	
	if( strncmp(numbuts[nr].name, "Rot", 3)==0 ) {
		float *fp;
		
		fp= (float *)(numbdata+nr);
		fp[0]= 180.0*fp[0]/M_PI;
	}

}

void clever_numbuts()
{
	Object *ob;
	float lim;
	char str[48];
	
	if(curarea->spacetype==SPACE_VIEW3D) {
		lim= 1000.0*G.vd->grid;

		if(G.obedit==0) {
			ob= OBACT;
			if(ob==0) return;
			
			add_numbut(0, NUM|FLO, "LocX:", -lim, lim, ob->loc);
			add_numbut(1, NUM|FLO, "LocY:", -lim, lim, ob->loc+1);
			add_numbut(2, NUM|FLO, "LocZ:", -lim, lim, ob->loc+2);
			
			add_numbut(3, NUM|FLO, "RotX:", -lim, lim, ob->rot);
			add_numbut(4, NUM|FLO, "RotY:", -lim, lim, ob->rot+1);
			add_numbut(5, NUM|FLO, "RotZ:", -lim, lim, ob->rot+2);
			
			add_numbut(6, NUM|FLO, "SizeX:", -lim, lim, ob->size);
			add_numbut(7, NUM|FLO, "SizeY:", -lim, lim, ob->size+1);
			add_numbut(8, NUM|FLO, "SizeZ:", -lim, lim, ob->size+2);
			
			sprintf(str, "Active Object: %s\n", ob->id.name+2);
			do_clever_numbuts(str, 9, REDRAW);
		}
		else if ELEM3(G.obedit->type, OB_MESH, OB_SECTOR, OB_LIFE) clever_numbuts_mesh();
		else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) clever_numbuts_curve();
	}
	else if(curarea->spacetype==SPACE_IPO) {
		clever_numbuts_ipo();
	}
	else if(curarea->spacetype==SPACE_SEQ) {
		clever_numbuts_seq();
	}
	
	
}


void replace_names_but()
{
	Image *ima= G.main->image.first;
	short len, tot=0;
	char old[64], new[64], temp[80];
	
	strcpy(old, "/data/");
	strcpy(new, "/data/");
	
	add_numbut(0, TEX, "Old:", 0, 63, old);
	add_numbut(1, TEX, "New:", 0, 63, new);

	if (do_clever_numbuts("Replace image name", 2, REDRAW) ) {
		
		len= strlen(old);
		
		while(ima) {
			
			if(strncmp(old, ima->name, len)==0) {
				
				strcpy(temp, new);
				strcat(temp, ima->name+len);
				strncpy(ima->name, temp, 79);
				
				if(ima->ibuf) freeImBuf(ima->ibuf);
				ima->ibuf= 0;
				ima->ok= 1;
				
				tot++;
			}
			
			ima= ima->id.next;
		}

		sprintf(temp, "Replaced %d names", tot);
		notice(temp);
	}
	
}




