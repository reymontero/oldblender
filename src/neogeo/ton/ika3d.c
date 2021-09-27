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



/* ika.c juli-aug nov 93 */

/* *************  MAIN LOOP && EDITING && ALGEMENE FUNKTIES *************** */

/* DOEN :
 * 
 * - als alles rechtopstaat (obj 2) klappert de quaternion om als je er 1 keer aan trekt.
 *   tijdelijk opgelost door y coordinaat van bovenste punt iets te veranderen
 * 
 * 
 * 
 */


#include "ika.h"

struct ListBase bpbase, rodbase, jointbase, tubase;
struct Skeleton *skel=0;

float maxmom=0.0;

int mainwin, subwin, subwin2, NP, NR, NJ;
int orgx=250, orgy=300, sizex=800, sizey=600;

extern float dproj;

void printAt(x, y, c, poin)
int x, y;
char c;
void *poin;
{
	char str[100];

	frontbuffer(1);
	
	persp(0);
	cpack(0);
	sboxfs(x-10, y-2, x+300, y+12);

	cpack(0xFFFFFF);
	cmov2i(x, y);

	if(c=='f') sprintf(str, "%f", *( (float *)poin ));

	charstr(str);
	frontbuffer(0);
	persp(1);
}

void flerp3(val, n1, n2)	/* val is de mate waarmee n1 gedempt wordt met n2 */
float val, *n1, *n2;
{
	float minval;
	
	if(val==1.0) {
		VECCOPY(n1, n2);
		return;
	}
	else if(val==0.0) return;
	
	minval= 1.0-val;
	n1[0]= minval*n1[0]+val*n2[0];
	n1[1]= minval*n1[1]+val*n2[1];
	n1[2]= minval*n1[2]+val*n2[2];
}

void getmouseco(mval)
short *mval;
{
	static Device mdev[2]= {MOUSEX, MOUSEY};

	getdev(2, mdev, mval);
	mval[0]-= orgx;
	mval[1]-= orgy;
	
}

struct BodyPoint *addbodypoint()
{
	struct BodyPoint *p1;

	p1= callocN(sizeof(struct BodyPoint), "addbodypoint");
	addtail(&bpbase, p1);
	NP++;
	
	return p1;
}

void freebodypoint(p1)
struct BodyPoint *p1;
{

	remlink(&bpbase, p1);
	freeN(p1);
	NP--;
	
}

struct Rod *addrod()
{
	struct Rod *rod;

	rod= callocN(sizeof(struct Rod), "addrod");
	addtail(&rodbase, rod);
	NR++;
	
	return rod;
}

void freerod(rod)
struct Rod *rod;
{

	remlink(&rodbase, rod);
	freeN(rod);
	NR--;
	
}

struct Joint *addjoint()
{
	struct Joint *joint;

	joint= callocN(sizeof(struct Joint), "addjoint");
	addtail(&jointbase, joint);
	NJ++;
	
	return joint;
}

void freejoint(joint)
struct Joint *joint;
{

	remlink(&jointbase, joint);
	freeN(joint);
	NJ--;
	
}

void addobject1()
{
	struct BodyPoint *p1, *p2, *p3, *p4;
	struct Rod *rod;
	struct Joint *joint;

	p1= addbodypoint();
	p1->r[0]= 0.0;
	p1->r[1]= -2.0;
	p1->r[2]= 1.0;
	p1->m= 1.0;
	
	p2= addbodypoint();
	p2->r[0]= 20.0;
	p2->r[1]= 0.0;
	p2->r[2]= 130.0;
	p2->m= 1.0;

	p3= addbodypoint();
	p3->r[0]= -101.0;
	p3->r[1]= -1.0;
	p3->r[2]= 200.0;
	p3->m= 1.0;

	p4= addbodypoint();
	p4->r[0]= -210.0;
	p4->r[1]= 1.0;
	p4->r[2]= 101.0;
	p4->m= 1.0;


	rod= addrod();
	rod->p1= p1;
	rod->p2= p2;
	init_rod(rod, 0.0);
	
	rod= addrod();
	rod->p1= p2;
	rod->p2= p3;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p3;
	rod->p2= p4;
	init_rod(rod, 0.0);

	joint= addjoint();
	joint->r1= rod->prev->prev;
	joint->r2= rod->prev;
	joint->p1= p1;
	joint->p2= p2;
	joint->p3= p3;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);

	joint= addjoint();
	joint->r1= rod->prev;
	joint->r2= rod;
	joint->p1= p2;
	joint->p2= p3;
	joint->p3= p4;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 1);

	if(skel==0) {
		skel= callocN(sizeof(struct Skeleton), "initrod");
		skel->r1= rod->prev->prev;
		skel->r2= rod->prev;
		skel->r3= rod;
	}
}

void addobject2()	/* T_ROD+H_ROD+T_ROD */
{
	struct BodyPoint *p1, *p2, *p3, *p4, *p5, *p6;
	struct Rod *rod;
	struct Joint *joint;

	p1= addbodypoint();
	p1->r[1]= 0.0;
	p1->r[2]= 0.0;
	p1->m= 1.0;
	
	p2= addbodypoint();
	p2->r[1]= -75.0;
	p2->r[2]= 150.0;
	p2->m= 1.0;

	p3= addbodypoint();
	p3->r[1]= 75.0;
	p3->r[2]= 150.0;
	p3->m= 1.0;

	p4= addbodypoint();
	p4->r[1]= -75.0;
	p4->r[2]= 300.0;
	p4->m= 1.0;

	p5= addbodypoint();
	p5->r[1]= 75.0;
	p5->r[2]= 300.0;
	p5->m= 1.0;

	p6= addbodypoint();
	p6->r[1]= 1.0;
	p6->r[2]= 450.0;
	p6->m= 1.0;


	rod= addrod();
	rod->p1= p1;
	rod->p2= p2;
	rod->p3= p3;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p2;
	rod->p2= p3;
	rod->p3= p4;
	rod->p4= p5;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p6;
	rod->p2= p4;
	rod->p3= p5;
	init_rod(rod, 0.0);


	joint= addjoint();
	joint->r1= rod->prev->prev;
	joint->r2= rod->prev;
	joint->p1= p1;
	joint->p2= p2; joint->p2a= p3;
	joint->p3= p4; joint->p3a= p5;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);


	joint= addjoint();
	joint->r1= rod->prev;
	joint->r2= rod;
	joint->p1= p2; joint->p1a= p3;
	joint->p2= p4; joint->p2a= p5;
	joint->p3= p6; 
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);
	
	skel= callocN(sizeof(struct Skeleton), "initrod");
	skel->r1= rod->prev->prev;
	skel->r2= rod->prev;
	skel->r3= rod;
}

void addobject3()	/* T_ROD+H_ROD+H_ROD+T_ROD */
{
	struct BodyPoint *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
	struct Rod *rod;
	struct Joint *joint;

	p1= addbodypoint();
	p1->r[1]= 0.0;
	p1->r[2]= 0.0;
	p1->m= 1.0;
	
	p2= addbodypoint();
	p2->r[1]= -50.0;
	p2->r[2]= 100.0;
	p2->m= 1.0;

	p3= addbodypoint();
	p3->r[1]= 50.0;
	p3->r[2]= 100.0;
	p3->m= 1.0;

	p4= addbodypoint();
	p4->r[1]= -50.0;
	p4->r[2]= 200.0;
	p4->m= 1.0;

	p5= addbodypoint();
	p5->r[1]= 50.0;
	p5->r[2]= 200.0;
	p5->m= 1.0;

	p6= addbodypoint();
	p6->r[1]= -50.0;
	p6->r[2]= 300.0;
	p6->m= 1.0;

	p7= addbodypoint();
	p7->r[1]= 50.0;
	p7->r[2]= 300.0;
	p7->m= 1.0;

	p8= addbodypoint();
	p8->r[1]= 0.0;
	p8->r[2]= 400.0;
	p8->m= 1.0;


	rod= addrod();
	rod->p1= p1;
	rod->p2= p2;
	rod->p3= p3;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p2;
	rod->p2= p3;
	rod->p3= p4;
	rod->p4= p5;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p4;
	rod->p2= p5;
	rod->p3= p6;
	rod->p4= p7;
	init_rod(rod, 0.0);

	rod= addrod();
	rod->p1= p8;
	rod->p2= p6;
	rod->p3= p7;
	init_rod(rod, 0.0);


	joint= addjoint();
	joint->r1= rod->prev->prev->prev;
	joint->r2= rod->prev->prev;
	joint->p1= p1;
	joint->p2= p2; joint->p2a= p3;
	joint->p3= p4; joint->p3a= p5;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);

	joint= addjoint();
	joint->r1= rod->prev->prev;
	joint->r2= rod->prev;
	joint->p1= p2; joint->p1a= p3;
	joint->p2= p4; joint->p2a= p5;
	joint->p3= p6; joint->p3a= p7;
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);


	joint= addjoint();
	joint->r1= rod->prev;
	joint->r2= rod;
	joint->p1= p4; joint->p1a= p5;
	joint->p2= p6; joint->p2a= p7;
	joint->p3= p8; 
	init_joint(joint, 1.0, 1.0, 0.0, 0.0, 0);
}

void addtube()
{
	struct Tube *tu;
	float *fp, height, rad, curz, dz, ang, si, co;
	int a, b;
	
	height= 450.0;
	rad= 30.0;
	
	tu= mallocN(sizeof(struct Tube), "addtube");
	addtail(&tubase, tu);
	tu->vec[0]= 0.0;
	tu->vec[1]= 0.0;
	tu->vec[2]= 225.0;
	tu->u= 12;
	tu->v= 20;
	fp=tu->data= mallocN(tu->u*tu->v*3*4, "addtube2");
	
	curz= -0.5*height;
	dz= height/(float)(tu->v-1);
	for(a=0; a<tu->v; a++) {
		for(b=0; b<tu->u; b++, fp+=3) {
			ang= 2.0*M_PI*b/(float)(tu->u-1);
			si= fsin(ang);
			co= fcos(ang);
			fp[0]= si*rad+ co*rad;
			fp[1]= co*rad- si*rad;
			fp[2]= curz;
		}
		curz+= dz;
	}
}

void make_orig(struct Rod *rod)
{
	/* maakt duplicate van roddata in orig */
	struct RodDeform *rdef;

	if(rod->rdef) freeN(rod->rdef);
	rdef= rod->rdef= callocN(sizeof(struct RodDeform), "make_orig");

	if(rod->type==ROD) {
		VECCOPY(rdef->orig1, rod->p1->r);
		VECCOPY(rdef->orig2, rod->p2->r);
	}
	else if(rod->type==T_ROD) {
		VecMidf(rdef->orig1, rod->p2->r, rod->p3->r);
		VECCOPY(rdef->orig2, rod->p1->r);
	}
	else if(rod->type==H_ROD) {
		VecMidf(rdef->orig1, rod->p1->r, rod->p2->r);
		VecMidf(rdef->orig2, rod->p3->r, rod->p4->r);
	}
	Mat3CpyMat3(rdef->origmat, rod->mat);
	Mat3Inv(rdef->originv, rdef->origmat);
	
	VecMidf(rdef->origmid, rdef->orig1, rdef->orig2);
}

void copy_defstate(struct Rod *rod)
{
	/* maakt duplicate van roddata in new */
	struct RodDeform *rdef;

	if(rod->rdef==0) return;
	rdef= rod->rdef;
	
	if(rod->type==ROD) {
		VECCOPY(rdef->v1, rod->p1->r);
		VECCOPY(rdef->v2, rod->p2->r);
	}
	else if(rod->type==T_ROD) {
		VecMidf(rdef->v1, rod->p2->r, rod->p3->r);
		VECCOPY(rdef->v2, rod->p1->r);
	}
	else if(rod->type==H_ROD) {
		VecMidf(rdef->v1, rod->p1->r, rod->p2->r);
		VecMidf(rdef->v2, rod->p3->r, rod->p4->r);
	}
	Mat3CpyMat3(rdef->mat, rod->mat);
}

void deselectallpoints()
{
	struct BodyPoint *bp;
	
	bp= bpbase.first;
	while(bp) {
		bp->s= 0;
		bp= bp->next;
	}
}

void selectpoint(mval)
short mval[2];
{
	struct BodyPoint *bp, *sel= 0;
	int mindist= 10000, x, y, test;

	bp= bpbase.first;
	while(bp) {
		bp->s= 0;
		x= bp->sx-mval[0];
		y= bp->sy-mval[1];
		test= x*x+y*y;
		if(test<mindist) {
			sel= bp;
			mindist= test;
		}
		bp= bp->next;
	}
	if(sel) {
		sel->s= 1;
	}
}

void freeAll()
{
	struct Tube *tu;
	struct Rod *rod;
	
	tu= tubase.first;
	while(tu) {
		freeN(tu->data);
		tu= tu->next;
	}
	freelistN(&tubase);

	freelistN(&bpbase);

	rod= rodbase.first;
	while(rod) {
		if(rod->rdef) freeN(rod->rdef);
		rod= rod->next;
	}
	freelistN(&rodbase);
	freelistN(&jointbase);
	
	if(skel) freeN(skel);

	if(totblock!=0) {
		printf("Error Totblck: %d\n",totblock);
		printmemlist();
	}
	
}

void quattest()
{
	float si, co, vec[3], q1[4], q2[4], q3[4];
	
	q2[0]= 0.0;
	q2[1]= vec[0]= 1.0;
	q2[2]= vec[1]= 1.0;
	q2[3]= vec[2]= 0.0;
	
	co= fcos(0.5*0.25*M_PI);
	si= fsin(0.5*0.25*M_PI);
	
	q3[0]= co;
	q3[1]= 0.0;
	q3[2]= 0.0;
	q3[3]= -si;
	
	QuatMul(q1, q2, q3);
	
	q3[3]= -q3[3];
	
	QuatMul(q2, q3, q1);
	
	printf("%f %f %f %f\n", q2[0], q2[1], q2[2], q2[3]);
	
}

main()
{
	struct Rod *rod;
	short loop=1, event, val, mval[2], grabbing=0, mstart[2];
	Device mdev[2];
	
	prefposition(orgx, orgx+sizex-1, orgy, orgy+sizey);
	mainwin= winopen("IKA");
	RGBmode();
	doublebuffer();
	gconfig();
	sginap(3);
	cpack(0);
	clear();
	
/*	subwin= swinopen(mainwin);
	winposition(10, sizex-10, 10, sizey-10);
	RGBmode();
	doublebuffer();
	gconfig();
	sginap(3);	
	reshapeviewport();
	cpack(0);
	clear();

	subwin2= swinopen(mainwin);
	winposition(110, 200, 10, 100);
	RGBmode();
	doublebuffer();
	gconfig();
	sginap(3);	
	reshapeviewport();
	cpack(0);
	clear();
*/
	winset(mainwin);

	qdevice(RAWKEYBD);
	qdevice(ESCKEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
	qdevice(SPACEKEY);
	mdev[0]= MOUSEX;
	mdev[1]= MOUSEY;

	/* init vars */
	NP= 0;
	NR= 0;
	NJ= 0;
	bpbase.first= bpbase.last= 0;
	rodbase.first= rodbase.last= 0;
	jointbase.first= jointbase.last= 0;
	
	addobject2();
	itterate();		/* om object definitief goed te krijgen */

	rod= rodbase.first;
	make_orig(rod);
	make_orig(rod->next);
	make_orig(rod->next->next);
	
	addtube();
	
	/* quattest(); */
	
	winset(mainwin);
	mmode(MVIEWING);
	deflighting();
	bindlighting();

/*	winset(subwin);
	mmode(MVIEWING);
	deflighting();
	bindlighting();
*/	
	drawscene();
	
	while(loop) {
		
		if(qtest()) {
			event= qread(&val);
			if(val) {
				switch(event) {
				case MIDDLEMOUSE:
					viewmove();
					break;
				case RIGHTMOUSE:
					grabbing= 1;
					getmouseco(mstart);
					selectpoint(mstart);
					break;
				case SPACEKEY:
					addobject1();
					break;
				case PADPLUSKEY:
					dproj/= 1.1;
					break;
				case PADMINUS:
					dproj*= 1.1;
					break;
				case ESCKEY:
					gexit();
					freeAll();
					exit(0);
				}
			}
			else {
				if(event==RIGHTMOUSE) {
					grabbing= 0;
					deselectallpoints();
				}
			}
		}
		else sginap(1);

		if(grabbing) calc_mouseforce(mstart);
		apply_forces();
		itterate();
		
		drawscene();
	}
}

