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


#define BTST(a,b)	( ( (a) & 1<<(b) )!=0 )
#define BCLR(a,b)	( (a) & ~(1<<(b)) )
#define BSET(a,b)	( (a) | 1<<(b) )

#define FIELD	(1.0/50)
#define SECS	15
#define PICS	750	/* secs/field */

long rdit[]={0,6,2,4,7,1,5,3};
long gdit[]={7,1,5,3,0,6,2,4};
long bdit[]={0,6,2,4,7,1,5,3};
long nodit[]={0,0,0,0,0,0,0,0};

float ditf[]={0.25,0.75,0.5,0.0};
float ditf4[]={0,192,48,240,128,64,176,112,32,224,16,208,160,96,144,80};


short *zigzagpat[4];

short zz0[32]= {
	-1,-1,
	-1,0, 0,0, 1,0,
	-2,1, -1,1, 0,1, 1,1,
	0,2, 1,2, 2,2, 3,2,
	0,3, 1,3, 2,3,
	2,4 };

short zz1[32]= {
	1,-4,
	1,-4, 2,-4, 3,-4,
	-1,-1, 0,-1, 1,-1, 2,-1,
	-1,0, 0,0, 1,0, 2,0,
	-2,1, -1,1, 0,1,
	0,2 };

short zz2[32]= {
	-3,-3,
	-3,-2, -2,-2, 1,-2,
	-4,-1, -3,-1, -2,-1, -1,-1,
	-2,0, -1,0, 0,0, 1,0,
	-2,1, -1,1, 0,1,
	0,2 };

short zz3[32]= {
	-1,-1,
	-1,0, 0,0, 1,0,
	-3,1, -2,1, -1,1, 0,1,
	-3,2, -2,2, -1,2, 0,2,
	-4,3, -3,3, -2,3,
	-2,4 };


float ysize[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

float left[PICS+100];
float right[PICS+100];

short bas[128]={2,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0,
		2,0,0,0, 1,0,0,1, 1,1,0,0, 1,0,0,0,
		2,0,0,0, 1,0,0,1, 0,0,0,0, 1,0,0,0,
		2,0,0,0, 1,0,0,1, 1,1,0,0, 1,0,0,0,
		2,0,0,0, 1,0,0,0, 1,1,0,0, 0,0,0,0,
		2,0,0,0, 1,0,0,1, 1,1,0,0, 1,0,0,0,
		2,0,0,0, 1,0,0,1, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};


short trom[128]=
	       {0,1,1,1, 2,0,1,1, 0,0,3,0, 2,0,1,1,
		0,1,1,1, 2,0,1,1, 0,0,3,0, 2,0,1,1,
		0,1,1,1, 2,0,1,1, 1,0,0,0, 2,0,1,1,
		0,1,1,1, 2,0,1,1, 0,0,3,0, 2,0,1,1,
		0,1,1,1, 2,3,3,2, 2,0,0,0, 0,2,0,0,
		0,0,1,0, 2,0,1,0, 0,0,0,0, 2,0,1,1,
		0,1,1,1, 2,0,1,2, 0,1,1,1, 1,0,1,1,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

short fluit[128]=
	       {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,1,1,1, 3,0,1,4,
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

void golfvorm(tel,uitslag,op,uit,l,r)
float tel,uitslag,op,uit,l,r;
{
	short b,fr;
	float u;

	u=uitslag;

	fr= (short)( tel/(8*FIELD) );
	if(fr>20) {
		uitslag*=op;
		for(b=1;b<20;b++) {
			left[fr-b]+=l*uitslag;
			right[fr-b]+=r*uitslag;
			uitslag*=op;
		}
	}
	uitslag=u;
	for(b=0;b<50;b++) {
		left[fr+b]+=l*uitslag;
		right[fr+b]+=r*uitslag;
		uitslag*=uit;
	}
}

/*	UIT DE PARENTBASE:
 	
	if( strncmp(base->str,"Speake",6)==0 ) {
		tijd= (float)G.cfra/25.0;
		if(R.f & 64) tijd+=.02;
		speakersize(tijd-1.0,&li,&re);
		if(base->str[6]=='l') base->m[1][1]= .00001+li*0.22;
		if(base->str[6]=='r') base->m[1][1]= .00001+re*0.22;
	}

	if(strncmp(base->str,"Wiel",4)==0) {
		tijd= frametofloat((float)G.cfra,2,G.efra,0);	
		tijd= ease(tijd,1.0,0.2,0.2);
		standf= 25000+ 15000*tijd;
		meterkast(base,standf);
	}
*/

void initspeakerdata()
{
	short a,tel;

	for(a=0;a<PICS;a++) {
		left[a]=0;
		right[a]=0;
	}

	/* lees bas en trom array */


	for(tel=0;tel<SECS*8;tel++) {
		if(bas[tel]) golfvorm((float)tel,(float)bas[tel],.6,.85,1.0,0.5);
		if(trom[tel]) golfvorm((float)tel,.5*(float)trom[tel],0.5,.75,0.5,1.0);
		if(fluit[tel]) golfvorm((float)tel,.2*(float)fluit[tel],0.7,.9,1.0,1.0);

	}
}

void speakersize(tijd,l,r)
float tijd,*l,*r;
{
	short fr;

	fr= (short)(tijd/FIELD+.1);

	if(fr>750 || fr<0) {
		*l=0;
		*r=0;
		return;
	}
	*l= left[ fr ];
	*r= right[ fr ];
}


void golf(ob)		/* uit view.c, aanroepen als drawtype==2 */
struct ObData *ob;
{
	short a,b,c,size,*mem,*point,val1,val2,val3;
	struct VV *vv;
	struct VertOb *a1;
	float f1;

	vv=ob->vv;	
	size = sqrt(vv->vert);

	mem=(short *) mallocN(vv->vert << 1,"golfmem");
	if (mem==0) return;
	
	srand(1);
	point=mem;
	for(a=0;a<vv->vert;a++) {
		f1 =  rand()*sin(G.cfra/11.0+rand()/1000.0)/10;
		f1 += rand()*sin(G.cfra/13.0+rand()/1000.0)/10;
		*(point++) = f1;
	}
	
	for(c=2;c>0;c--){
		for(a=0;a<size;a++){	
			point = mem + a;
			val1 = *point;
			val2 = val1;
			for (b=size-1;b>0;b--){
				val3 = point[size];
				*point = (val1 + (val2<<1) + val3) >> 2;
				val1 = val2;
				val2 = val3;
				point += size;
			}
			*point = (val2 + 3*val3) >> 2;
		}

		point = mem;
		for(a=0;a<size;a++){
			val1 = *point;
			val2 = val1;
			for (b=size-1;b>0;b--){
				val3 = point[1];
				*point = (val1 + (val2<<1) + val3) >> 2;
				val1 = val2;
				val2 = val3;
				point += 1;
			}
			*(point++) = (val2 + 3*val3) >> 2;
		}
	}
	a1=(struct VertOb *)(vv+1);
	point=mem;
	for(a=0;a<vv->vert;a++) {
		a1->c[2] = *(point++);
		a1++;
	}
	freeN(mem);

/*	normalen(adrve,adrvl,vv->vert,vv->vlak);
*/
}


float stand(standf,wiel)
double standf;
short wiel;
{
	static float w0=0.0,w1=0.0,w2=0.0,w3=0.0,w4=0.0;
	float fl,wo,w,fac1,fac2;

	if(wiel==0) {
		w0= standf-10*floor(standf/10);
		return w0;
	}
	else if(wiel==1) {
		wo=w1;
		if(w1== -1) return standf/10-10*floor(standf/100);
		w1= standf/10-10*floor(standf/100);
		fl= ffloor(w1);
		if(w1-fl <=.90) w=fl;
		else w= fl+ 10*(w1-fl-.9);
		fac1=2*(w1-wo);
		if(fac1<0) fac1+=20;
		if(fac1>1.0) {
			fac1=w1;
			w1= -1;
			return fac1;
		}
		fac2=1.0-fac1;
		return fac1*w1+fac2*w;
	}
	else if(wiel==2) {
		wo=w2;
		if(w2== -1) return standf/100-10*floor(standf/1000);
		w2= standf/100-10*floor(standf/1000);
		fl= ffloor(w2);
		if(w2-fl <.90) w=fl;
		else w= fl+ 10*(w2-fl-.9);
		fac1=2*(w2-wo);
		if(fac1<0) fac1+=20;
		if(fac1>1.0) {
			fac1=w2;
			w2= -1;
			return fac1;
		}
		fac2=1.0-fac1;
		return fac1*w2+fac2*w;
	}
	else if(wiel==3) {
		wo=w3;
		if(w3== -1) return standf/1000-10*floor(standf/10000);
		w3= standf/1000-10*floor(standf/10000);
		fl= ffloor(w3);
		if(w3-fl <.90) w=fl;
		else w= fl+ 10*(w3-fl-.9);
		fac1=2*(w3-wo);
		if(fac1<0) fac1+=20;
		if(fac1>1.0) {
			fac1=w3;
			w3= -1;
			return fac1;
		}
		fac2=1.0-fac1;
		return fac1*w3+fac2*w;
	}
	else {
		w4= standf/10000-10*floor(standf/100000);
		fl= ffloor(w4);
		if(w4-fl <.90) w=fl;
		else w= fl+ 10*(w4-fl-.9);
		return w;
	}
}




float standsimp(standf,wiel)
double standf;
short wiel;
{
	static float w0=0.0,w1=0.0,w2=0.0,w3=0.0,w4=0.0;
	float fl,wo,w,fac1,fac2;

	if(wiel==0) {
		w0= standf-10*floor(standf/10);
		return w0;
	}
	else if(wiel==1) {
		w1= standf/10-10*floor(standf/100);
		fl= ffloor(w1);
		if(w1-fl <=.90) w=fl;
		else w= fl+ 10*(w1-fl-.9);
		return w;
	}
	else if(wiel==2) {
		w2= standf/100-10*floor(standf/1000);
		fl= ffloor(w2);
		if(w2-fl <.90) w=fl;
		else w= fl+ 10*(w2-fl-.9);
		return w;
	}
	else if(wiel==3) {
		w3= standf/1000-10*floor(standf/10000);
		fl= ffloor(w3);
		if(w3-fl <.90) w=fl;
		else w= fl+ 10*(w3-fl-.9);
		return w;
	}
	else {
		w4= standf/10000-10*floor(standf/100000);
		fl= ffloor(w4);
		if(w4-fl <.90) w=fl;
		else w= fl+ 10*(w4-fl-.9);
		return w;
	}
}

void meterkast(base,fr)
struct Base *base;
double fr;
{
	float sta;	

	sta= -0.1*PI*standsimp(fr,base->nnr);
	base->q[0]= fcos(sta);
	base->q[1]=base->q[2]=0;
	base->q[3]= fsin(sta);
}



/*	dit in de centrale parentbase lus: */
	
/*				tijd= (float)(G.cfra+view0.tilt)/25.0;
				standlin= 4*tijd;
				if(tijd>6.0) {
					standf= standlin+pow(2.7,tijd-6.0)-1.0;	
				}
				else standf=standlin;
				meterkast(b,standf);
*/


*/ ************************* */

void initzigzagpat()
{
	short a;
	
	zigzagpat[0]= zz0;
	zigzagpat[1]= zz1;
	zigzagpat[2]= zz2;
	zigzagpat[3]= zz3;

	for(a=1;a<32;a+=2) {
		zz0[a]*=R.rectx;
		zz1[a]*=R.rectx;
		zz2[a]*=R.rectx;
		zz3[a]*=R.rectx;
	}
}

void endzigzagpat()
{
	short a;

	for(a=1;a<32;a+=2) {
		zz0[a]/=R.rectx;
		zz1[a]/=R.rectx;
		zz2[a]/=R.rectx;
		zz3[a]/=R.rectx;
	}
}



void sampledown()
{
	ulong *rectz, *ro,*ro1;
	short a,r,g,b,x,y,rect4,*pat;
	char *rt;
	register char *o1,*o2,*o3,*o4;

	if(G.winar[9]==0) return;
	if(okee("sample down")==0) return;
	rect4= 4*R.rectx;
	

		/* OSA */
	rectz= (ulong *)malloc(R.rectx*R.recty/4);
	
	if(okee("sample square")){
		for(y=0;y<R.recty;y+=4) {

			o1= (char *)(R.rectot+ y*R.rectx);
			o2= o1+ rect4;
			o3= o2+ rect4;
			o4= o3+ rect4;
			rt= (char *)(rectz+ y*R.rectx/16);

			for(x=0;x<R.rectx;x+=4) {
				r=g=b=0;
				for(a=0;a<4;a++) {
					b+= o1[1]+o2[1]+o3[1]+o4[1];
					g+= o1[2]+o2[2]+o3[2]+o4[2];
					r+= o1[3]+o2[3]+o3[3]+o4[3];
					o1+=4; o2+=4; o3+=4; o4+=4;
				}
				rt[1]= b/16;
				rt[2]= g/16;
				rt[3]= r/16;
				rt+=4;
			}
		}
	}
	else {
		initzigzagpat();

		for(y=4;y<R.recty-1;y+=4) {

			ro= (R.rectot+ y*R.rectx+1);
			rt= (char *)(rectz+ y*R.rectx/16+1);

			for(x=4;x<R.rectx-1;x+=4) {
				
				a= ( (x & 8) + (y & 16) )/8;
				pat= zigzagpat[a];
				r=g=b=0;

				for(a=0;a<32;a+=2) {
					o1= (char *)(ro+ pat[a]+ pat[a+1]);
					b+= o1[1];	
					g+= o1[2];	
					r+= o1[3];	
				}
				rt[1]= b/16;
				rt[2]= g/16;
				rt[3]= r/16;
				rt+=4;
				ro+=4;
			}
		}

		endzigzagpat();

	}

	winclose(G.winar[9]);
	qdevice(RIGHTMOUSE);
	G.winar[9]=0;
	R.rectx/=4;
	R.recty/=4;
	free(R.rectot);
	R.rectot= rectz;
}


/* ************************ */

short accumzbufshade()
{
	struct VlakRen *vlr;
	struct VertRen *v1,*v2,*v3;
	struct ColBlck *cb=0;
	float min,max;
	ulong *rectp,*rp,*rt1;
	long v,mx0,mx2,my0,my2;
	short transp,x,y,afbreek=0,col,rectx1,rectx2,recty1,recty2,testquESC();
	char r,g,b,*rt;

	rectp= (ulong *)mallocN(4*R.rectx*R.recty,"accumrect");
	zwritemask(0);
	cpack(0); clear();
	cpack(0x404040);

	for(v=0;v<G.totvlak;v++) {
		if((v & 255)==0) vlr= R.blovl[v>>8];
		else vlr++;

		if(vlr->col!=cb) {
			transp=vlr->col->snfl;
			cb= vlr->col;
			r= (cb->tra*cb->r)>>8;
			g= (cb->tra*cb->g)>>8;
			b= (cb->tra*cb->b)>>8;
		}
		if(transp) {

		  v1=vlr->v1;v2=vlr->v2;v3=vlr->v3;
		  /* miny, maxy */
		  min= MIN3(v1->ys,v2->ys,v3->ys);
		  max= MAX3(v1->ys,v2->ys,v3->ys);
		  my0= (long)ffloor(min)+R.afmy;
		  my2= (long)ffloor(max+1)+R.afmy;

		  if(my0!=my2) {
			if(my2>= 0 || my0< R.recty) {
				min= MIN3(v1->xs,v2->xs,v3->xs);
				max= MAX3(v1->xs,v2->xs,v3->xs);
				mx0= (long)ffloor(min)+R.afmx;
				mx2= (long)ffloor(max+1)+R.afmx;
				if(mx0!=mx2) {
					if(mx2>= 0 || mx0< R.rectx) {
						if(my0<0) recty1=0;
						else recty1= my0;
						if(my2>=R.recty) recty2=R.recty-1;
						else recty2= my2;
						if(mx0<0) rectx1=0;
						else rectx1= mx0;
						if(mx2>=R.rectx) rectx2=R.rectx-1;
						else rectx2= mx2;
						
						bgnpolygon();
							v3i(vlr->v1->co);
							v3i(vlr->v2->co);
							v3i(vlr->v3->co);
						endpolygon();

						lrectread(rectx1,recty1,rectx2,recty2,rectp);
						rp=rectp;
						rt1=R.rectot+R.rectx*recty1+rectx1;

						for(y=recty1;y<=recty2;y++) {
							rt= (char *)rt1;
							for(x=rectx1;x<=rectx2;x++) {
								if(*rp) {
									col= rt[1]+b;
									if(col>255) rt[1]= 255;
									else rt[1]= col;
									col= rt[2]+g;
									if(col>255) rt[2]= 255;
									else rt[2]= col;
									col= rt[3]+r;
									if(col>255) rt[3]= 255;
									else rt[3]= col;
								}
								*(rp++)=0;
								rt+=4;
							}
							rt1+=R.rectx;
						}
						lrectwrite(rectx1,recty1,rectx2,recty2,rectp);
					}
				}
			}
		  }
		  afbreek=testquESC();
		  if(afbreek) break;
		}
	}

	zwritemask(0xFFFFFFFF);
	freeN(rectp);

	return afbreek;
}

/* ******** SUBDIVIDE */

float zwaartelijn(v1,v2,v3,proj)
struct VertRen *v1,*v2,*v3;
short proj;
{
	/* lijn door renderpunt en v1 snijden met lijn v2-v3,
	   return lengte lijnstuk renderpunt en snijpunt
	*/
	float a,b,c,d,e,f,g,h,labda,vec[3],sn[3],VecLenf(),deler;
	short x=0,y=1;

	/* ahv proj a..f bepalen */
	if(proj==1) y= 2;
	else if(proj==2) {
		x= 1;
		y= 2;
	}

	a= R.co[x]-v1->co[x];
	b= R.co[y]-v1->co[y];
	c= v1->co[x];
	d= v1->co[y];
	e= v3->co[x]-v2->co[x];
	f= v3->co[y]-v2->co[y];
	g= v2->co[x];
	h= v2->co[y];

	deler= e*b-a*f;
	if(deler!=0) labda= (e*h- d*e- f*g+ f*c)/deler;
	else labda= 0;

	sn[0]= labda*(R.co[0]-v1->co[0])+v1->co[0];
	sn[1]= labda*(R.co[1]-v1->co[1])+v1->co[1];
	sn[2]= labda*(R.co[2]-v1->co[2])+v1->co[2];
	
	VECCOPY(vec,R.co);
	return VecLenf(sn,vec);
}


void subdivedge(v4,v1,v2,n1,n2)
struct VertRen *v4,*v1,*v2;
float *n1,*n2;
{
	float t[3],len=0,fac,uit;

	t[0]= n1[0]+n2[0];
	t[1]= n1[1]+n2[1];
	t[2]= n1[2]+n2[2];
	fac=0; /* 1.0 -0.5*fsqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]); */
	/*
	uit=n1[0]*v1->co[0]+n1[1]*v1->co[1]+n1[2]*v1->co[2];
	uit-= (n1[0]*v2->co[0]+n1[1]*v2->co[1]+n1[2]*v2->co[2]);
	len= VecLen(v1->co,v2->co);

	if(uit<0.0) len= -len;
	*/
	v4->n[0]= n1[0]+n2[0];
	v4->n[1]= n1[1]+n2[1];
	v4->n[2]= n1[2]+n2[2];
	Normalise(v4->n);
	v4->co[0]= (long)((v1->co[0]+v2->co[0])/2.0+len*fac*v4->n[0]);
	v4->co[1]= (long)((v1->co[1]+v2->co[1])/2.0+len*fac*v4->n[1]);
	v4->co[2]= (long)((v1->co[2]+v2->co[2])/2.0+len*fac*v4->n[2]);
	v4->v=0;
}

void subdivvlak(vlr,v1,v2,v3,vlro,n)
struct VlakRen *vlr;
struct VertRen *v1,*v2,*v3;
struct VlakRen *vlro;
float *n;
{

	vlr->v1= v1;
	vlr->v2= v2;
	vlr->v3= v3;
	vlr->n[0]= n[0]+v2->n[0]+v3->n[0];
	vlr->n[1]= n[1]+v2->n[1]+v3->n[1];
	vlr->n[2]= n[2]+v2->n[2]+v3->n[2];
	Normalise(vlr->n);
	vlr->puno=0;
	vlr->col= vlro->col;
	vlr->snproj= vlro->snproj;

}


void subdivide(vlr)
struct VlakRen *vlr;
{
	/* in: een vlak, uit: vier vlakken met puno's goed en ofset in veco's */
	struct VertRen *v1,*v2,*v3,*v4,*v5,*v6,*addvert();
	struct VlakRen *vlr1,*vlr2,*vlr3,*addvlak();
	float len,uit,co,n1[3],n2[3],n3[3];

	v1=vlr->v1;
	v2=vlr->v2;
	v3=vlr->v3;
	v4=addvert(G.totvert++);
	v5=addvert(G.totvert++);
	v6=addvert(G.totvert++);
	vlr1=addvlak(G.totvlak++);
	vlr2=addvlak(G.totvlak++);
	vlr3=addvlak(G.totvlak++);

	if(vlr->puno & 1) {
		n1[0]= -v1->n[0]; n1[1]= -v1->n[1]; n1[2]= -v1->n[2];
	} else {
		n1[0]= v1->n[0]; n1[1]= v1->n[1]; n1[2]= v1->n[2];
	}
	if(vlr->puno & 2) {
		n2[0]= -v2->n[0]; n2[1]= -v2->n[1]; n2[2]= -v2->n[2];
	} else {
		n2[0]= v2->n[0]; n2[1]= v2->n[1]; n2[2]= v2->n[2];
	}
	if(vlr->puno & 4) {
		n3[0]= -v3->n[0]; n3[1]= -v3->n[1]; n3[2]= -v3->n[2];
	} else {
		n3[0]= v3->n[0]; n3[1]= v3->n[1]; n3[2]= v3->n[2];
	}
	
	subdivedge(v4,v1,v2,n1,n2);
	subdivedge(v5,v2,v3,n2,n3);
	subdivedge(v6,v3,v1,n3,n1);

	subdivvlak(vlr1,v1,v4,v6,vlr,n1);
	subdivvlak(vlr2,v2,v5,v4,vlr,n2);
	subdivvlak(vlr3,v3,v6,v5,vlr,n3);

	if(vlr->puno & 1) vlr1->puno=1;
	if(vlr->puno & 2) vlr2->puno=1;
	if(vlr->puno & 4) vlr3->puno=1;
	vlr->v1= v4;
	vlr->v2= v5;
	vlr->v3= v6;
	vlr->puno=0;
}

void skydit(x,y)
short x,y;
{
	extern short itot;
	static short firsttime=0;
	float rf,gf,bf;
	long temp[2],*t;
	long dit2,dit,r,g,b;

	if((wrld.fs & 5)==0) {
		R.col.r= wrld.horr;
		R.col.g= wrld.horg;
		R.col.b= wrld.horb;
		return;
	}

	if(firsttime==0) {
		firsttime=1;
		for(r=0;r<16;r++) ditf4[r]/=255.0;
	}
	dit = (x & 3) + ((y & 1) << 2);
	dit2 = (x % 4) + 4*(y % 4);

	if(wrld.fs & 2) 
		wrld.inprz= fabs(R.view[0]*wrld.grvec[0]+ R.view[1]*wrld.grvec[1]+ R.view[2]*wrld.grvec[2]);
	else wrld.inprz= fabs(.5+R.view[1]);

	if(wrld.inprz>1.0) wrld.inprz=1.0;
	wrld.inprh= 1.0-wrld.inprz;

	if(wrld.fs & 4) {
		t=(long *)&wrld;
		temp[0]=t[0]; temp[1]=t[1];
		R.lo[0]= 32767*R.view[0];
		R.lo[1]= 32767*R.view[1];
		R.lo[2]= 32767*R.view[2];
		itot=256;
		if(wrld.stex[0]) skytex(0);
		if(wrld.stex[1]) skytex(1);
		if(wrld.stex[2]) skytex(2);
		if(wrld.stex[3]) skytex(3);
	}

	if(wrld.fs & 1) {
		rf= ditf4[dit2]+ wrld.inprh*wrld.horr+wrld.inprz*wrld.zenr;
		gf= ditf4[dit2]+ wrld.inprh*wrld.horg+wrld.inprz*wrld.zeng;
		bf= ditf4[dit2]+ wrld.inprh*wrld.horb+wrld.inprz*wrld.zenb;
		if (rf>255.0) R.col.r=255; else R.col.r= (char)rf;
		if (gf>255.0) R.col.g=255; else R.col.g= (char)gf;
		if (bf>255.0) R.col.b=255; else R.col.b= (char)bf;
	}
	else {
		R.col.r= wrld.horr;
		R.col.g= wrld.horg;
		R.col.b= wrld.horb;
	}		
	if(wrld.fs & 4) {
		t[0]=temp[0]; t[1]=temp[1];
	}
}


/******** OUDE ZBUFFER  ************* */

void zbufpoly(type)
short type;
{
	/* 0=shade 1=gouroud 2=keymat */
	struct PolyRen *p;
	double	knot[4],*t,col[4][3];
	short a,b,c,*v,len;

	setnurbsproperty( N_ERRORCHECKING, 1.0 );
	setnurbsproperty( N_PIXEL_TOLERANCE, 400.0 );
	setnurbsproperty( N_DISPLAY, N_FILL);

	knot[0]= -32760; knot[1]= -32760;  /* is ook wordruimte */
	knot[2]= 32760; knot[3]= 32760;
	
	/* gouraud met nurb: N_C4D moet toch 3D kleur hebben, stom! */
	/* for(a=0;a<4;a++) {
		col[a][0]= ((double)a)/4.0;
		col[a][1]= ((double)a)/4.0;
		col[a][2]= ((double)a)/4.0;
	} */
	
	for(a=0;a<G.totpoly;a++) {
		p= R.poly[a];

		if(type==0) cpack(G.totvlak+a+1);
		else if(type==1) cpack(((p->col->b)<<16)+((p->col->g)<<8) + (p->col->r));
		else if(type==2) cpack(0xFFFFFF);

		if(R.f & 16) {	/* transp of env */
			if(p->col->mode & 32) cpack(0);
			else if(p->col->tra) {
				if(type==2) cpack(0x010101*(255- p->col->tra));
			}
		}

		bgnsurface();
			nurbssurface(4,knot,4,knot,
				3*8,2*3*8,p->nurb,2,2,N_V3D);
			/* nurbssurface(4,knot,4,knot,
				3*8,2*3*8,col,2,2,N_C4D); */
			t= p->data;
			for(b=0;b<p->poly;b++) {
				len= (short) *(t++);
				bgntrim();
					pwlcurve(len,t,16,N_ST);
				endtrim();
				t+= 2*len;
			}
		endsurface();
	}
}

void zbufpipe(nr)
short nr;
{
	struct VlakRen *vlr;
	float *f,*feed;
	long orx,ory;
	
	getorigin(&orx,&ory);

	feed= (float *)mallocN(4*9*8,"feedbuf");
	feedback(feed,9*G.totvert);

	vlr= R.blovl[0];
	vlr+= nr;
	bgnpoint();
		passthrough(0);
		v3f(vlr->v1->co);
		printf("in feed %f %f %f\n",vlr->v1->co[0],vlr->v1->co[1],vlr->v1->co[2]);
	endpoint();
	endfeedback(feed);

	f= feed;
	f+=2;
	if(*f== FB_POINT) {
		if(f[3]<0x800000) f[3]= (float)(f[3]+0x7FFFFF);
		else f[3]= (float)(f[3]-0x800000);
		f[1]-= orx;
		f[2]-= ory;
		printf("feed %f %f %f\n",f[1],f[2],f[3]);

		f+=7;
	}

	freeN(feed);

}

void zbufferallo(key)
short key;
{
	struct VlakRen *vlr;
	struct ColBlck *cb;
	ulong v1,v2,v3,*rz,*rectz;
	long a;
	short transp,env;
/*
printmatrix4(G.winmat);
zbufpipe(0);
zbufpipe(3);
zbufpipe(5);
*/
	cb=0;
	if(key) cpack(0xFFFFFF);
	for(a=0;a<G.totvlak;a++) {
		if((a & 255)==0) vlr= R.blovl[a>>8];
		else vlr++;
		if(vlr->rt1) {
			if(R.f & (16+128)) {	/* transp of env */
				if(vlr->col!=cb) {
					cb= vlr->col;
					transp= cb->mode & 192;
					env= (cb->mode & 32);
				}
				if(transp==0 || key) {
					if(env) cpack(0);
					else if(key==0) cpack(a+1);
					else cpack(0x010101*(255- cb->tra));
					bgnpolygon();
						v3f(vlr->v1->co);
						v3f(vlr->v2->co);
						v3f(vlr->v3->co);
					endpolygon();
				}
			}
			else {
				if(key==0) cpack(a+1);
				bgnpolygon();
					v3f(vlr->v1->co);
					v3f(vlr->v2->co);
					v3f(vlr->v3->co);
				endpolygon();
			}
		}

		if(G.afbreek) break;
	}
	/* if(G.totpoly) zbufpoly(2*key); */
}

/* PATTERNS */


void setbit(pat,f)
short *pat,f;
{

	pat+= f>>4;
	f &= 0xf;
	*pat=BSET(*pat,f);

}

void clearbit(pat,f)
short *pat,f;
{

	pat+= f>>4;
	f &= 0xf;
	*pat=BCLR(*pat,f);

}

short readbit(pat,f)
short *pat,f;
{

	pat+= f>>4;
	f &= 0xf;
	return BTST(*pat,f);

}

void maakpattern16(tra,pat)
unsigned char tra,*pat;
{
	float splim,f;
	short c,p;
	char bpat[8];

	p= (256-tra)>>2;
	splim= 64.0/p;
	for(c=0;c<8;c++) bpat[c]=0;
	for(f=0;f<64;f+=splim) {
		setbit(bpat,(short)f);
	}
	for (c=0;c<8;c++){
		pat[2*c] = bpat[c];
		pat[2*c+1] = bpat[c];
		pat[2*c+16] = bpat[c];
		pat[2*c+17] = bpat[c];
	}
}

/*	De oude zbuf uitleesmethode:

		if(*rz<0x800000) zbuf= R.zcor*(float)(*rz+0x7FFFFF);
		else zbuf= R.zcor*(float)(*rz-0x800000);
		if(zbuf!=1.0) {
			
			z= 1.0+zbuf/(1.0-zbuf);
			R.co[0]= (x+R.xstart+1.0)*z;
			if(R.f & 64) R.co[1]= (y+R.ystart+1.5)*z*R.ycor;
			else R.co[1]= (y+R.ystart+1.0)*z*R.ycor;
			R.co[2]= -R.d*z;
*/
			
/*
printf("%d %d %d\n",R.co[0],R.co[1],R.co[2]);
printf("%f %f %f\n",v1->co[0],v1->co[1],v1->co[2]);

zbuf= 0xFFFFFF*( (-v1->co[2])*G.winmat[2][2]-0.5*G.winmat[3][2])/(v1->co[2]);
printf("met winmat %f\n",zbuf);

if(*rz<0x800000) zbuf= (float)(*rz+0x7FFFFF);
else zbuf= (float)(*rz-0x800000);
printf("uit zbuf %f\n",zbuf);

*/



/*  filter over zbuffer in zbufferall()

	rz=rectz=(ulong *)malloc(4*R.rectx*R.recty);
	readsource(SRC_ZBUFFER);
	lrectread(0,0,R.rectx-1,R.recty-1,rectz);
	readsource(SRC_AUTO);	

	for (c=4;c>0;c--){
	rz=rectz;
	for (b=R.recty;b>0;b--){
		v1=v2= *(rz);
		for(a=R.rectx-1;a>0;a--){
			v3=rz[1];
			*(rz++) = (v1+v3)>>1;
			v1=v2;
			v2=v3;
		}
	}
	for (b=0;b<R.rectx;b++){
		rz=rectz+b;
		v1=v2= *rz;
		for(a=R.recty-1;a>0;a--){
			v3=rz[R.rectx];
			*(rz) = (v1+v3)>>1;
			v1=v2;
			v2=v3;
			rz += R.rectx;
		}
	}
	}
	zbuffer(0);
	zdraw(1);
	lrectwrite(0,0,R.rectx-1,R.recty-1,rectz);
	zdraw(0);
	zbuffer(1);

*/

/* OUDE ANIMATIE AFSPEEL EN COMPRESSIE */

void addframe(fra)
short fra;
{
	struct tms timebuf;
	static long delta;
	long tijd;
	static short *mainframe=0;
	short x,y,*frame,r,g,b,d,dit[8],delay=0;
	register short *rn;
	register char *rt1,*rt2;

	if(fra>0 && mainframe==0) return;
	dit[0]=0; dit[2]=16; dit[4]=8; dit[6]=24;

	if(fra== -2) {	/* vrijgeven */
		if(mainframe) freeN(mainframe);
		mainframe=0;
	}
	else if(fra== -1) {	/* afspelen */
		if(mainframe==0) return;

		rectzoom(2.0,2.0);
		while(getbutton(ESCKEY)==0) {
			frame=mainframe;
			tijd= times(&timebuf);
			for(fra=0;fra<(G.efra-G.sfra+1);fra++) {
				rn=frame;
				rt1=(char *)R.rectot;
				rt1++;
				for(y=0;y<R.recty;y+=2) {
				   for(x=0;x<R.rectx;x+=2) {
					rt1[0]= (rn[0]>>8) & 248;
					rt1[1]= (rn[0] & 2047)>>3 ;
					rt1[2]= (rn[0] & 31)<<3;
					rn++;
					rt1+=4;
				}}
				sginap(delay);
				lrectwrite(0,0,R.rectx/2-1,R.recty/2-1,R.rectot);
				frame+=delta;
				if(getbutton(ESCKEY)) break;
			}
			tijd= (times(&timebuf)-tijd)/(G.efra-G.sfra+1);
			if(tijd>4) delay=0;		/* 4= 1/25 sec */
			else delay= (4-tijd) -1;
			if(delay<0) delay=0;
		}
		rectzoom(1.0,1.0);

	}
	else {		/* addframe */
		if(fra==0) {
			delta= 2*((short)(R.rectx/2))*((short)(R.recty/2));
			if( (G.efra-G.sfra)*delta>12200000) {
				error("Not enough memory ");
				mainframe=0;
				return;
			}
			mainframe=(short *)mallocN((G.efra-G.sfra+1)*delta+100,"mainframe");
			delta/=2;
		}	
		rt1=(char *)R.rectot;
		rt2=rt1+4*R.rectx;
		rn= mainframe+fra*delta;
		for(y=0;y<R.recty;y+=2) {
			for(x=0;x<R.rectx;x+=2) {
				rt1++; rt2++;
				b= *(rt1++)+ *(rt2++);
				g= *(rt1++)+ *(rt2++);
				r= *(rt1++)+ *(rt2++);
				rt1++; rt2++;
				d= dit[((x+y) & 7)];
				b+= *(rt1++)+ *(rt2++) +d;
				g+= *(rt1++)+ *(rt2++) +d/2;
				r+= *(rt1++)+ *(rt2++) +d;
				if(r>1023) r=1023;
				if(g>1023) g=1023;
				if(b>1023) b=1023;
				*(rn++)= ((b & 992)<<6) +((g & 1008)<<1) +(r>>5);
			}
			rt1+=4*R.rectx;
			rt2+=4*R.rectx;
		}
	}
}

void addframezoom(fra)
short fra;
{
	static long delta;
	static short *mainframe=0;
	short x,y,*frame,*rn,r,g,b,d,dit[4];
	char *rt1,*rt2;

	if(fra!=0 && mainframe==0) return;
	dit[0]=0; dit[1]=4; dit[2]=2; dit[3]=6;

	if(fra== -2) {	/* vrijgeven */
		if(mainframe) freeN(mainframe);
		mainframe=0;
	}
	else if(fra== -1) {	/* afspelen */
		if(mainframe==0) return;

		if(G.winar[9]) winclose(G.winar[9]);
		x=2*R.rectx; y=2*R.recty;
		prefposition(640-x/2,639+x/2,512-y/2,511+y/2);
		noborder();
		G.winar[9]=winopen("render");
		RGBmode();
		singlebuffer();
		gconfig();
		winset(G.winar[9]);
		
		rectzoom(2.0,2.0);
		while(getbutton(ESCKEY)==0) {
			frame=mainframe;
			for(fra=0;fra<(G.efra-G.sfra+1);fra++) {
				rn=frame;
				rt1=(char *)R.rectot;
				rt1++;
				for(y=0;y<R.recty;y++) {
				   for(x=0;x<R.rectx;x++) {
					rt1[0]= (rn[0]>>8) & 248;
					rt1[1]= (rn[0] & 2047)>>3 ;
					rt1[2]= (rn[0] & 31)<<3;
					rn++;
					rt1+=4;
				}}
				lrectwrite(0,0,R.rectx-1,R.recty-1,R.rectot);
				frame+=delta;
				gsync();gsync();
				if(getbutton(ESCKEY)) break;
			}
		}
		rectzoom(1.0,1.0);

	}
	else {		/* addframe */
		if(fra==0) {
			delta= 2*R.rectx*R.recty;
			if( (G.efra-G.sfra)*delta>12200000) {
				error("Not enough memory ");
				mainframe= 0;
				return;
			}
			mainframe=(short *)mallocN((G.efra-G.sfra+1)*delta+100,"mainframe");
			delta/=2;
		}	
		rt1=(char *)R.rectot;
		rn= mainframe+fra*delta;
		for(y=0;y<R.recty;y++) {
			for(x=0;x<R.rectx;x++) {
				rt1++;
				d= dit[((x+y) & 3)];
				b= *(rt1++) +d;
				g= *(rt1++) +d/2;
				r= *(rt1++) +d;
				if(r>255) r=255;
				if(g>255) g=255;
				if(b>255) b=255;
				*(rn++)= ((b & 248)<<8) +((g & 252)<<3) +(r>>3);
			}
		}
	}
}

/*  STOND IN DE ROTEERSCENE	
					if(ob->ef) {
						float * co , fac, ofs;
						
						fac = 50.0 * cb->ang;
						ofs = (2 * PI * (G.cfra - base->sf)) / base->len;
						co = ver->co;
						co[0] += fac * fsin(co[1] / 4000.0 + ofs);
						co[1] += fac * fsin(co[2] / 4000.0 + ofs);
						co[2] += fac * fsin(co[0] / 4000.0 + ofs);
						doe_puno = 1;
					}
*/

void calcnormQstrip(qs)
struct QStrip *qs;
{
    struct VertStrip *vs1, *vs2;
    struct FaceStrip *fs;
    float CalcNormFloat();
    long nr, u, v;
    
    /* eerst puno's op nul */
    nr= qs->pntsu*qs->pntsv;
    vs1= qs->verts;
    while(nr--) {
	vs1->n[0]= vs1->n[1]= vs1->n[2]= 0.0;
	vs1++;
    }
    /* vlaknormalen */
    fs= qs->faces;
    for(v= 0; v<qs->pntsv-1; v++) {
	vs1= qs->verts+ v*qs->pntsu;
	vs2= vs1+ qs->pntsu;
	for(u= 0; u<qs->pntsu-1; u++) {
	    CalcNormFloat(vs1->n, (vs1+1)->n, vs2, fs->n);
	    fs++;
	    vs1++;
	    vs2++;
	}
    }
    /* puno's door vlakno's op te tellen */
    fs= qs->faces;
    for(v= 0; u<qs->pntsv-1; u++) {
	vs1= qs->verts+ v*qs->pntsu;
	vs2= vs1+ qs->pntsu;
	for(u= 0; u<qs->pntsu-1; u++) {
	    vs1->n[0]+= fs->n[0]; vs1->n[1]+= fs->n[1]; vs1->n[2]+= fs->n[2];
	    vs2->n[0]+= fs->n[0]; vs2->n[1]+= fs->n[1]; vs2->n[2]+= fs->n[2];
	    vs1++;
	    vs2++;
	    vs1->n[0]+= fs->n[0]; vs1->n[1]+= fs->n[1]; vs1->n[2]+= fs->n[2];
	    vs2->n[0]+= fs->n[0]; vs2->n[1]+= fs->n[1]; vs2->n[2]+= fs->n[2];

	    fs++;
	}
    }
    /* normaliseren */
    nr= qs->pntsu*qs->pntsv;
    vs1= qs->verts;
    while(nr--) {
	Normalise(vs1->n);
	vs1++;
    }

}

	else if(svlak & 0x00F00000) {	/* qstrips */
		/* welke strip */
		if(R.svlako!= svlak) {
		    
		    changed= 0;
		    if(qs==0) {
			changed= 1;
			qs= R.qstrip.first;
		    }
		    if(svlak < qs->firstface) {
			qs= qs->prev;
			changed= 1;
			while(qs && svlak < qs->firstface) qs= qs->prev;
		    }
		    else {
			if(qs->next) {
			    if(svlak >= qs->next->firstface) {
				qs= qs->next;
				changed= 1;
				while(qs->next && svlak >= qs->next->firstface) {
				    qs= qs->next;
				}
			    }
			}
		    }
		    /* compleet nieuwe strip */
		    if(changed) {
			;
			;
		    }
		    R.svlako= svlak;
		}
		
		memcpy(&R.col,qs->col,sizeof(struct ColBlck));
		
		nr= svlak- qs->firstface;
		uof= nr % qs->pntsu;
		vof= nr/qs->pntsu;
		VECCOPY(R.vno, (qs->faces+nr)->n);
		VECCOPY(R.vn, (qs->faces+nr)->n);
		
		/* COXYZ nieuwe methode (de oude staat in oud.c) */
		R.view[0]= (x+(R.xstart)+1.0);
		if(R.f & 64) R.view[1]= (y+R.ystart+1.5)*R.ycor;
		else R.view[1]= (y+R.ystart+1.0)*R.ycor;
		R.view[2]= -R.d;

		deler= R.vno[0]*R.view[0] + R.vno[1]*R.view[1] + R.vno[2]*R.view[2];
		fac= dvlak/deler;
		R.co[0]= fac*R.view[0];
		R.co[1]= fac*R.view[1];
		R.co[2]= fac*R.view[2];
		shadelamplus(x, y);
		
		r= (char *)rp;
		r[3]= R.col.r;
		r[2]= R.col.g;
		r[1]= R.col.b;
		r[0]= 255;
	}

#define EDGE_S	1
#define EDGE_I	2
#define EDGE_F	3
#define CORN_S	4
#define CORN_I	5
#define CORN_F	6
#define TRIA_S	7
#define TRIA_I	8
#define TRIA_F	9


long curdraw, curcol, *curdlist, *countdlist, *startdlist, *enddlist;


void bgnDL()
{
    /* alloceert eerste geheugenblok, zet enkele variabelen */
    
    curdraw= curcol= -1;
     
    startdlist= (long *)mallocN(64000, "bgnDL");
    enddlist= startdlist+16000;
    curdlist= startdlist;

}

void addtoDL(type, p1, p2, p3)
float *p1, *p2, *p3;
{
    float v[3][3];
    long len;
    
    /* eerst alle pointers naar floatdata omzetten */
    switch(type) {
	case EDGE_S:
	    v[0][0]= (float) ((short *)p1)[0];
	    v[0][1]= (float) ((short *)p1)[1];
	    v[0][2]= (float) ((short *)p1)[2];
	    v[1][0]= (float) ((short *)p2)[0];
	    v[1][1]= (float) ((short *)p2)[1];
	    v[1][2]= (float) ((short *)p2)[2];
	    type= EDGE_F;
	    len= 6;
	    break;
	case EDGE_I:
	    v[0][0]= (float) ((long *)p1)[0];
	    v[0][1]= (float) ((long *)p1)[1];
	    v[0][2]= (float) ((long *)p1)[2];
	    v[1][0]= (float) ((long *)p2)[0];
	    v[1][1]= (float) ((long *)p2)[1];
	    v[1][2]= (float) ((long *)p2)[2];
	    type= EDGE_F;
	    len= 6;
	    break;
	case EDGE_F:
	    VECCOPY(v[0], p1);
	    VECCOPY(v[1], p2);
	    len= 6;
	    break;

	case CORN_S: case TRIA_S:
	    v[0][0]= (float) ((short *)p1)[0];
	    v[0][1]= (float) ((short *)p1)[1];
	    v[0][2]= (float) ((short *)p1)[2];
	    v[1][0]= (float) ((short *)p2)[0];
	    v[1][1]= (float) ((short *)p2)[1];
	    v[1][2]= (float) ((short *)p2)[2];
	    v[2][0]= (float) ((short *)p3)[0];
	    v[2][1]= (float) ((short *)p3)[1];
	    v[2][2]= (float) ((short *)p3)[2];
	    if(type== CORN_S) type= CORN_F;
	    else type= TRIA_F;
	    len= 9;
	    break;
	case CORN_I: case TRIA_I:
	    v[0][0]= (float) ((long *)p1)[0];
	    v[0][1]= (float) ((long *)p1)[1];
	    v[0][2]= (float) ((long *)p1)[2];
	    v[1][0]= (float) ((long *)p2)[0];
	    v[1][1]= (float) ((long *)p2)[1];
	    v[1][2]= (float) ((long *)p2)[2];
	    v[2][0]= (float) ((short *)p3)[0];
	    v[2][1]= (float) ((short *)p3)[1];
	    v[2][2]= (float) ((short *)p3)[2];
	    if(type== CORN_I) type= CORN_F;
	    else type= TRIA_F;
	    len= 9;
	    break;
	case CORN_F: case TRIA_F:
	    VECCOPY(v[0], p1);
	    VECCOPY(v[1], p2);
	    VECCOPY(v[2], p3);
	    len= 9;
	    break;
    }
    if(curdlist+len+3>=enddlist) return;
    
    if(curdraw!= type) {

	curdraw= type;
	curdlist[0]= type;
	curdlist[1]= curcol;
	curdlist[2]= 0;
	countdlist= curdlist+2;
	curdlist+= 3;
    }
    (*countdlist)++;
    memcpy(curdlist, v, 4*len);
    curdlist+= len;
}

long endDL()
{

    return (long)startdlist;
}

void drawDL(adr)
long *adr;
{
    long type, col, nr;

    curdlist= adr;
    enddlist= curdlist+ 16000;
    
    
    while(curdlist<enddlist) {
	type= *(curdlist++);
	col= *(curdlist++);
	cpack(col);
	
	switch(type) {
	    case EDGE_F:
		nr= *(curdlist++);
		while(nr>0) {
		    nr--;
		    bgnline(); v3f(curdlist); v3f(curdlist+3); endline();
		    curdlist+= 6;
		}
		break;
	    case CORN_F:
		nr= *(curdlist++);
		while(nr>0) {
		    nr--;
		    bgnline(); v3f(curdlist); v3f(curdlist+3); v3f(curdlist+6); endline();
		    curdlist+= 9;
		}
		break;
	    case TRIA_F:
		nr= *(curdlist++);
		while(nr>0) {
		    nr--;
		    bgnclosedline(); v3f(curdlist); v3f(curdlist+3); v3f(curdlist+6); endclosedline();
		    curdlist+= 9;
		}
		break;
	    default:
		return;
	}
    } 
}

