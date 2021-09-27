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



/*  texture.c        MIX MODEL
 * 
 *  maart 95
 * 
 */

#include "blender.h"
#include "render.h"

float Tin, Tr, Tg, Tb, Ta;


void free_texture(Tex *te)
{
	
}


void default_tex(Tex *tex)
{
	tex->stype= 0;
	tex->cropxmin= tex->cropymin= 0.0;
	tex->cropxmax= tex->cropymax= 1.0;
	tex->xrepeat= tex->yrepeat= 0;
	tex->fie_ima= 2;
	tex->sfra= 1;
	tex->frames= 0;
	tex->offset= 0;
	tex->noisesize= 0.25;
	tex->noisedepth= 2;
	tex->turbul= 5.0;
	tex->bright= 1.0;
	tex->contrast= 1.0;
	
}

Tex *add_texture(char *name)
{
	Tex *tex;

	tex= alloc_libblock(&G.main->tex, ID_TE, name);
	
	default_tex(tex);
	
	return tex;
}

void default_mtex(MTex *mtex)
{
	mtex->texco= TEXCO_ORCO;
	mtex->mapto= MAP_COL;
	mtex->object= 0;
	mtex->projx= PROJ_X;
	mtex->projy= PROJ_Y;
	mtex->projz= PROJ_Z;
	mtex->mapping= MTEX_FLAT;
	mtex->ofs[0]= 0.0;
	mtex->ofs[1]= 0.0;
	mtex->ofs[2]= 0.0;
	mtex->size[0]= 1.0;
	mtex->size[1]= 1.0;
	mtex->size[2]= 1.0;
	mtex->tex= 0;
	mtex->texflag= 0;
	mtex->colormodel= 0;
	mtex->r= 1.0;
	mtex->g= 1.0;
	mtex->b= 1.0;
	mtex->k= 1.0;
	mtex->def_var= 1.0;
	mtex->blendtype= MTEX_BLEND;
	mtex->colfac= 1.0;
	mtex->norfac= 0.5;
	mtex->varfac= 1.0;
}


MTex *add_mtex()
{
	MTex *mtex;
	
	mtex= callocN(sizeof(MTex), "add_mtex");
	
	default_mtex(mtex);
	
	return mtex;
}

Tex *copy_texture(Tex *tex)
{
	Tex *texn;
	int a;
	
	texn= copy_libblock(tex);
	if(texn->type==TEX_IMAGE) id_us_plus(texn->ima);
	else texn->ima= 0;
	
	return texn;
}

void autotexname(Tex *tex)
{
	extern char texstr[15][8];	/* buttons.c */
	
	if(tex) new_id(&G.main->tex, tex, texstr[tex->type]);
}


void init_render_texture(Tex *tex)
{

	tex->bright-= 0.5;	/* optim */
}

void init_render_textures()
{
	Tex *tex;
	
	tex= G.main->tex.first;
	while(tex) {
		if(tex->id.us) init_render_texture(tex);
		tex= tex->id.next;
	}
	
}

void end_render_texture(Tex *tex)
{
	
	tex->bright+= 0.5;	/* optim */
}

void end_render_textures()
{
	Tex *tex;
	
	tex= G.main->tex.first;
	while(tex) {
		if(tex->id.us) end_render_texture(tex);
		tex= tex->id.next;
	}
	
}


/* ************************** */

int clouds(Tex *tex, float *texvec)
{

	Tin= turbulence(tex->noisesize, texvec[0], texvec[1], texvec[2], tex->noisedepth);

	if(tex->stype==1) {

		Tr= Tin;
		Tg= turbulence(tex->noisesize, texvec[1], texvec[0], texvec[2], tex->noisedepth);

		Tb= turbulence(tex->noisesize,texvec[1],texvec[2],texvec[0], tex->noisedepth);
		
		BRICONRGB;
		
		return 1;
	}
	
	BRICON;
	
	return 0;
}

int blend(Tex *tex, float *texvec)
{

	if(tex->stype==0) {
		Tin= (1.0+texvec[0])/2.0;
	}
	else if(tex->stype==1) {
		Tin= (2.0+texvec[0]+texvec[1])/4.0;
	}
	else {
		Tin= 1.0-fsqrt(texvec[0]*texvec[0]+	texvec[1]*texvec[1]+texvec[2]*texvec[2]);
		if(Tin<0.0) Tin= 0.0;
		if(tex->stype==3) Tin*= Tin;
	}
	
	BRICON;

	return 0;
}

int wood(Tex *tex, float *texvec)
{
	
	if(tex->stype==0) {
		Tin= 0.5+0.5*fsin( (texvec[0]+texvec[1]+texvec[2])*10.0 );
	}
	else if(tex->stype==1) {
		Tin= 0.5+0.5*fsin( fsqrt(texvec[0]*texvec[0]+texvec[1]*texvec[1]+texvec[2]*texvec[2])*20.0 );
	}
	else if(tex->stype==2) {
		Tin= hnoise(tex->noisesize, texvec[0], texvec[1], texvec[2]);
		Tin= 0.5+ 0.5*fsin(tex->turbul*Tin+(texvec[0]+texvec[1]+texvec[2])*10.0);
	}
	else if(tex->stype==3) {
		Tin= hnoise(tex->noisesize, texvec[0], texvec[1], texvec[2]);
		Tin= 0.5+ 0.5*fsin(tex->turbul*Tin+(fsqrt(texvec[0]*texvec[0]+texvec[1]*texvec[1]+texvec[2]*texvec[2]))*20.0);
	}
	BRICON;
	
	return 0;
}

int marble(Tex *tex, float *texvec)
{
	float n, turb;

	n= 5.0*(texvec[0]+texvec[1]+texvec[2]);

	Tin = 0.5+0.5*fsin(n+tex->turbul*turbulence(tex->noisesize, texvec[0],texvec[1],texvec[2], tex->noisedepth));

	switch (tex->stype) {
	case 1:
		Tin= fsqrt(Tin);
		break;
	case 2:
		Tin= fsqrt(Tin);
		Tin= fsqrt(Tin);
		break;
	}
	
	BRICON;
	
	return 0;
}



int magic(Tex *tex, float *texvec)
{
	float tt, x, y, z, turb=1.0;
	int n;

	n= tex->noisedepth;
	turb= tex->turbul/5.0;
	
	x=  fsin( ( texvec[0]+texvec[1]+texvec[2])*5.0 );
	y=  fcos( (-texvec[0]+texvec[1]-texvec[2])*5.0 );
	z= -fcos( (-texvec[0]-texvec[1]+texvec[2])*5.0 );
	if(n>0) {
		x*= turb; 
		y*= turb; 
		z*= turb;
		y= -fcos(x-y+z);
		y*= turb;
		if(n>1) {
			x= fcos(x-y-z);
			x*= turb;
			if(n>2) {
				z= fsin(-x-y-z);
				z*= turb;
				if(n>3) {
					x= -fcos(-x+y-z);
					x*= turb;
					if(n>4) {
						y= -fsin(-x+y+z);
						y*= turb;
						if(n>5) {
							y= -fcos(-x+y+z);
							y*= turb;
							if(n>6) {
								x= fcos(x+y+z);
								x*= turb;
								if(n>7) {
									z= fsin(x+y-z);
									z*= turb;
									if(n>8) {
										x= -fcos(-x-y+z);
										x*= turb;
										if(n>9) {
											y= -fsin(x-y+z);
											y*= turb;
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

	if(turb!=0.0) {
		turb*= 2.0;
		x/= turb; 
		y/= turb; 
		z/= turb;
	}
	Tr= 0.5-x;
	Tg= 0.5-y;
	Tb= 0.5-z;

	BRICONRGB;
	
	return 1;
}


int stucci(Tex *tex, float *texvec)
{
	float b2, vec[3];
	float ofs;

	ofs= tex->turbul/200.0;
	
	b2= hnoise(tex->noisesize, texvec[0], texvec[1], texvec[2]);
	if(tex->stype) ofs*=(b2*b2);
	vec[0]= b2-hnoise(tex->noisesize, texvec[0]+ofs, texvec[1], texvec[2]);
	vec[1]= b2-hnoise(tex->noisesize, texvec[0], texvec[1]+ofs, texvec[2]);
	vec[2]= b2-hnoise(tex->noisesize, texvec[0], texvec[1], texvec[2]+ofs);

	if(tex->stype==1) {
		R.vn[0]+= vec[0];
		R.vn[1]+= vec[1];
		R.vn[2]+= vec[2];
	}
	else {
		R.vn[0]-= vec[0];
		R.vn[1]-= vec[1];
		R.vn[2]-= vec[2];
	}
	Normalise(R.vn);
	
	/* eigenlijk alleen voor preview */
	Tin= 0.5+(vec[0]+vec[1]+vec[2]);
	if(Tin<0.0) Tin= 0.0;
		
	return 0;
}

int texnoise(Tex *tex)
{
	float div=3.0;
	long val, ran, loop;
	
	ran= random();
	val= (ran & 3);
	
	loop= tex->noisedepth;
	while(loop--) {
		ran= (ran>>2);
		val*= (ran & 3);
		div*= 3.0;
	}
	
	Tin= ((float)val)/div;;

	BRICON;
	
	return 0;
}

/* *************** PROJEKTIES ******************* */

void tubemap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float len;

	*adr2 = (z + 1.0) / 2.0;

	len= fsqrt(x*x+y*y);
	if(len>0) {
		*adr1 = (1.0 - (fatan2(x/len,y/len) / M_PI)) / 2.0;
	}
}


void spheremap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float len;

	len= fsqrt(x*x+y*y+z*z);
	if(len>0.0) {
		*adr1 = (1.0 - fatan2(x,y)/M_PI )/2.0;
		z/=len;
		*adr2 = 1.0- facos(z)/M_PI;
	}
}

int cubemap(x, y, z, adr1, adr2)
float x, y, z, *adr1, *adr2;
{
	float x1, y1, z1;
	long ret;
	short *nor;
	
	
	if(R.vlr && R.vlr->face) {
		/* nor= R.vlr->vl->n; */
		/* x1= abs(nor[0]); */
		/* y1= abs(nor[1]); */
		/* z1= abs(nor[2]); */
	}
	else {
		x1 = fabs(x);
		y1 = fabs(y);
		z1 = fabs(z);
	}
	
	if(z1>=x1 && z1>=y1) {
		*adr1 = x;
		*adr2 = y;
		ret= 0;
	}
	else if(y1>=x1 && y1>=z1) {
		*adr1 = x;
		*adr2 = z;
		ret= 1;
	}
	else {
		*adr1 = y;
		*adr2 = z;
		ret= 2;
	}

	*adr1 = (*adr1 + 1.0) / 2.0;
	*adr2 = (*adr2 + 1.0) / 2.0;
	
	return ret;
}


void do_2d_mapping(MTex *mtex, float *t, float *dxt, float *dyt)
{
	Tex *tex;
	float fx, fy, fac1, area[8];
	int ok, proj, areaflag= 0, wrap;
	
	wrap= mtex->mapping;
	tex= mtex->tex;
	
	if(R.ali==0) {
		
		if(wrap==MTEX_FLAT) {
			fx = (t[0] + 1.0) / 2.0;
			fy = (t[1] + 1.0) / 2.0;
		}
		else if(wrap==MTEX_TUBE) tubemap(t[0], t[1], t[2], &fx, &fy);
		else if(wrap==MTEX_SPHERE) spheremap(t[0], t[1], t[2], &fx, &fy);
		else cubemap(t[0], t[1], t[2], &fx, &fy);
		
		/* repeat */
		if(tex->xrepeat>1) {
			fx *= tex->xrepeat;
			if(fx>1.0) fx -= (int)(fx);
		}
		if(tex->yrepeat>1) {
			fy *= tex->yrepeat;
			if(fy>1.0) fy -= (int)(fy);
		}
		
		/* crop */
		if(tex->cropxmin!=0.0 || tex->cropxmax!=1.0) {
			fac1= tex->cropxmax - tex->cropxmin;
			fx= tex->cropxmin+ fx*fac1;
		}
		if(tex->cropymin!=0.0 || tex->cropymax!=1.0) {
			fac1= tex->cropymax - tex->cropymin;
			fy= tex->cropymin+ fy*fac1;
		}

		t[0]= fx;
		t[1]= fy;
	}
	else {
		
		if(wrap==MTEX_FLAT) {
			fx= (t[0] + 1.0) / 2.0;
			fy= (t[1] + 1.0) / 2.0;
			dxt[0]/= 2.0; 
			dxt[1]/= 2.0;
			dyt[0]/= 2.0; 
			dyt[1]/= 2.0;
		}
		else if ELEM(wrap, MTEX_TUBE, MTEX_SPHERE) {
			/* uitzondering: de naad achter (y<0.0) */
			ok= 1;
			if(t[1]<=0.0) {
				fx= t[0]+dxt[0];
				fy= t[0]+dyt[0];
				if(fx>=0.0 && fy>=0.0 && t[0]>=0.0);
				else if(fx<=0.0 && fy<=0.0 && t[0]<=0.0);
				else ok= 0;
			}
			if(ok) {
				if(wrap==MTEX_TUBE) {
					tubemap(t[0], t[1], t[2], area, area+1);
					tubemap(t[0]+dxt[0], t[1]+dxt[1], t[2]+dxt[2], area+2, area+3);
					tubemap(t[0]+dyt[0], t[1]+dyt[1], t[2]+dyt[2], area+4, area+5);
				}
				else { 
					spheremap(t[0], t[1], t[2],area,area+1);
					spheremap(t[0]+dxt[0], t[1]+dxt[1], t[2]+dxt[2], area+2, area+3);
					spheremap(t[0]+dyt[0], t[1]+dyt[1], t[2]+dyt[2], area+4, area+5);
				}
				areaflag= 1;
			}
			else {
				if(wrap==MTEX_TUBE) tubemap(t[0], t[1], t[2], &fx, &fy);
				else spheremap(t[0], t[1], t[2], &fx, &fy);
				dxt[0]/= 2.0; 
				dxt[1]/= 2.0;
				dyt[0]/= 2.0; 
				dyt[1]/= 2.0;
			}
		}
		else {
			proj= cubemap(t[0], t[1], t[2], &fx, &fy);
			if(proj==1) {
				dxt[1]= dxt[2];
				dyt[1]= dyt[2];
			}
			else if(proj==2) {
				dxt[0]= dxt[1];
				dyt[0]= dyt[1];
				dxt[1]= dxt[2];
				dyt[1]= dyt[2];
			}
			dxt[0]/= 2.0; 
			dxt[1]/= 2.0;
			dyt[0]/= 2.0; 
			dyt[1]/= 2.0;
		}
		
		/* als area dan dxt[] en dyt[] opnieuw berekenen */
		if(areaflag) {
			fx= area[0]; 
			fy= area[1];
			dxt[0]= area[2]-fx;
			dxt[1]= area[3]-fy;
			dyt[0]= area[4]-fx;
			dyt[1]= area[5]-fy;
		}
		
		/* repeat */
		if(tex->xrepeat>1) {
			fx *= tex->xrepeat;
			dxt[0]*= tex->xrepeat;
			dyt[0]*= tex->xrepeat;
			if(fx>1.0) fx -= (int)(fx);
		}
		if(tex->yrepeat>1) {
			fy *= tex->yrepeat;
			dxt[1]*= tex->yrepeat;
			dyt[1]*= tex->yrepeat;
			if(fy>1.0) fy -= (int)(fy);
		}
		
		/* crop */
		if(tex->cropxmin!=0.0 || tex->cropxmax!=1.0) {
			fac1= tex->cropxmax - tex->cropxmin;
			fx= tex->cropxmin+ fx*fac1;
			dxt[0]*= fac1;
			dyt[0]*= fac1;
		}
		if(tex->cropymin!=0.0 || tex->cropymax!=1.0) {
			fac1= tex->cropymax - tex->cropymin;
			fy= tex->cropymin+ fy*fac1;
			dxt[1]*= fac1;
			dyt[1]*= fac1;
		}
		
		t[0]= fx;
		t[1]= fy;

	}
}



/* ************************************** */

int multitex(Tex *tex, float *texvec)
{
	switch(tex->type) {
	
	case 0:
		Tin= 0.0;
		return 0;
	case TEX_CLOUDS:
		return clouds(tex, texvec); 
	case TEX_WOOD:
		return wood(tex, texvec); 
	case TEX_MARBLE:
		return marble(tex, texvec); 
	case TEX_MAGIC:
		return magic(tex, texvec); 
	case TEX_BLEND:
		return blend(tex, texvec);
	case TEX_STUCCI:
		return stucci(tex, texvec); 
	case TEX_NOISE:
		return texnoise(tex); 
	case TEX_IMAGE:
		/* if(R.osatex) return imagewraposa(tex, texvec);  */
		/* else return imagewrap(tex, texvec);  */
		return imagewrap(tex, texvec);
		break;
	}
}

void do_material_tex()
{
	Material *mat_col, *mat_colspec, *mat_colmir, *mat_ref;
	Material *mat_spec, *mat_har, *mat_emit, *mat_alpha;
	MTex *mtex;
	float *co, texvec[3], fact, facm, factt, facmm, facmul, stencilTin=1.0;
	int tex_nr, rgb= 0;
	
	
	/* hier flag testen of er wel tex is */
	
	mat_col=mat_colspec=mat_colmir=mat_ref=mat_spec=mat_har=mat_emit=mat_alpha= R.mat;
	
	for(tex_nr=0; tex_nr<8; tex_nr++) {
		if(R.mat->mtex[tex_nr]) {
			mtex= R.mat->mtex[tex_nr];
			
			if(mtex->tex==0) continue;
			/* if(mtex->mapto==0) continue; */
			
			/* welke coords */
			co= R.lo;
			
			/* placement */
			if(mtex->projx) texvec[0]= mtex->size[0]*(co[mtex->projx-1]+mtex->ofs[0]);
			else texvec[0]= mtex->size[0]*(mtex->ofs[0]);
			
			if(mtex->projy) texvec[1]= mtex->size[1]*(co[mtex->projy-1]+mtex->ofs[1]);
			else texvec[1]= mtex->size[1]*(mtex->ofs[1]);
			
			if(mtex->projz) texvec[2]= mtex->size[2]*(co[mtex->projz-1]+mtex->ofs[2]);
			else texvec[2]= mtex->size[2]*(mtex->ofs[2]);
			
			
			/* texture */
			if(mtex->tex->type==TEX_IMAGE) do_2d_mapping(mtex, texvec, 0, 0);
			
			rgb= multitex(mtex->tex, texvec);
			
			/* texture uitgang */
			if(rgb && (mtex->texflag & MTEX_RGBTOINT)) {
				Tin= (0.35*Tr+0.45*Tg+0.2*Tb);
				rgb= 0;
			}
			if(mtex->texflag & MTEX_NEGATIVE) {
				if(rgb) {
					Tr= 1.0-Tr;
					Tg= 1.0-Tg;
					Tb= 1.0-Tb;
				}
				else Tin= 1.0-Tin;
			}
			if(mtex->texflag & MTEX_STENCIL) {
				if(rgb) {
					
				}
				else {
					fact= Tin;
					Tin*= stencilTin;
					stencilTin*= fact;
				}
			}
			else {
				if(rgb) ;
				else Tin*= stencilTin;
			}
			
			/* mapping */
			if(mtex->mapto & (MAP_COL+MAP_COLSPEC+MAP_COLMIR)) {
				
				if(rgb==0) {
					Tr= mtex->r;
					Tg= mtex->g;
					Tb= mtex->b;
				}
				else Tin= 1.0;

				fact= Tin*mtex->colfac;
				facm= 1.0-fact;
				if(mtex->blendtype==MTEX_MUL) facm= 1.0-mtex->colfac;
				if(mtex->blendtype==MTEX_SUB) fact= -fact;

				if(mtex->mapto & MAP_COL) {
					if(mtex->blendtype==MTEX_BLEND) {
						R.matren->r= (fact*Tr + facm*mat_col->r);
						R.matren->g= (fact*Tg + facm*mat_col->g);
						R.matren->b= (fact*Tb + facm*mat_col->b);
					}
					else if(mtex->blendtype==MTEX_MUL) {
						R.matren->r= (facm+fact*Tr)*mat_col->r;
						R.matren->g= (facm+fact*Tg)*mat_col->g;
						R.matren->b= (facm+fact*Tb)*mat_col->b;
					}
					else {
						R.matren->r= (fact*Tr + mat_col->r);
						R.matren->g= (fact*Tg + mat_col->g);
						R.matren->b= (fact*Tb + mat_col->b);
					}
					mat_col= R.matren;
				}
				if(mtex->mapto & MAP_COLSPEC) {
					if(mtex->blendtype==MTEX_BLEND) {
						R.matren->specr= (fact*Tr + facm*mat_colspec->specr);
						R.matren->specg= (fact*Tg + facm*mat_colspec->specg);
						R.matren->specb= (fact*Tb + facm*mat_colspec->specb);
					}
					else if(mtex->blendtype==MTEX_MUL) {
						R.matren->specr= (facm+fact*Tr)*mat_colspec->specr;
						R.matren->specg= (facm+fact*Tg)*mat_colspec->specg;
						R.matren->specb= (facm+fact*Tb)*mat_colspec->specb;
					}
					else {
						R.matren->specr= (fact*Tr + mat_colspec->specr);
						R.matren->specg= (fact*Tg + mat_colspec->specg);
						R.matren->specb= (fact*Tb + mat_colspec->specb);
					}
					mat_colspec= R.matren;
				}
				if(mtex->mapto & MAP_COLMIR) {
					if(mtex->blendtype==MTEX_BLEND) {
						R.matren->mirr= (fact*Tr + facm*mat_colmir->mirr);
						R.matren->mirg= (fact*Tg + facm*mat_colmir->mirg);
						R.matren->mirb= (fact*Tb + facm*mat_colmir->mirb);
					}
					else if(mtex->blendtype==MTEX_MUL) {
						R.matren->mirr= (facm+fact*Tr)*mat_colmir->mirr;
						R.matren->mirg= (facm+fact*Tg)*mat_colmir->mirg;
						R.matren->mirb= (facm+fact*Tb)*mat_colmir->mirb;
					}
					else {
						R.matren->mirr= (fact*Tr + mat_colmir->mirr);
						R.matren->mirg= (fact*Tg + mat_colmir->mirg);
						R.matren->mirb= (fact*Tb + mat_colmir->mirb);
					}
					mat_colmir= R.matren;
				}
			}
			if(mtex->mapto & MAP_VARS) {
				if(rgb) Tin= (0.35*Tr+0.45*Tg+0.2*Tb);
				
				fact= Tin*mtex->varfac;
				facm= 1.0-fact;
				if(mtex->blendtype==MTEX_MUL) facmul= 1.0-mtex->varfac;
				if(mtex->blendtype==MTEX_SUB) fact= -fact;

				if(mtex->mapto & MAP_REF) {
					if(mtex->maptoneg & MAP_REF) {factt= facm; facmm= fact;}
					else {factt= fact; facmm= facm;}
					
					if(mtex->blendtype==MTEX_BLEND)
						R.matren->ref= factt*mtex->def_var+ facmm*mat_ref->ref;
					else if(mtex->blendtype==MTEX_MUL)
						R.matren->ref= (facmul+factt)*mat_ref->ref;
					else {
						R.matren->ref= factt+mat_ref->ref;
						if(R.matren->ref<0.0) R.matren->ref= 0.0;
					}
					mat_ref= R.matren;
				}
				if(mtex->mapto & MAP_SPEC) {
					if(mtex->maptoneg & MAP_SPEC) {factt= facm; facmm= fact;}
					else {factt= fact; facmm= facm;}
					
					if(mtex->blendtype==MTEX_BLEND)
						R.matren->spec= factt*mtex->def_var+ facmm*mat_spec->spec;
					else if(mtex->blendtype==MTEX_MUL)
						R.matren->spec= (facmul+factt)*mat_spec->spec;
					else {
						R.matren->spec= factt+mat_spec->spec;
						if(R.matren->spec<0.0) R.matren->spec= 0.0;
					}
					mat_spec= R.matren;
				}
				if(mtex->mapto & MAP_EMIT) {
					if(mtex->maptoneg & MAP_EMIT) {factt= facm; facmm= fact;}
					else {factt= fact; facmm= facm;}
					
					if(mtex->blendtype==MTEX_BLEND)
						R.matren->emit= factt*mtex->def_var+ facmm*mat_emit->emit;
					else if(mtex->blendtype==MTEX_MUL)
						R.matren->emit= (facmul+factt)*mat_emit->emit;
					else {
						R.matren->emit= factt+mat_emit->emit;
						if(R.matren->emit<0.0) R.matren->emit= 0.0;
					}
					mat_emit= R.matren;
				}
				if(mtex->mapto & MAP_ALPHA) {
					if(mtex->maptoneg & MAP_ALPHA) {factt= facm; facmm= fact;}
					else {factt= fact; facmm= facm;}
					
					if(mtex->blendtype==MTEX_BLEND)
						R.matren->alpha= factt*mtex->def_var+ facmm*mat_alpha->alpha;
					else if(mtex->blendtype==MTEX_MUL)
						R.matren->alpha= (facmul+factt)*mat_alpha->alpha;
					else {
						R.matren->alpha= factt+mat_alpha->alpha;
						if(R.matren->alpha<0.0) R.matren->alpha= 0.0;
						else if(R.matren->alpha>1.0) R.matren->alpha= 1.0;
					}
					mat_alpha= R.matren;
				}
				if(mtex->mapto & MAP_HAR) {
					if(mtex->maptoneg & MAP_HAR) {factt= facm; facmm= fact;}
					else {factt= fact; facmm= facm;}
					
					if(mtex->blendtype==MTEX_BLEND) {
						R.matren->har= 128.0*factt*mtex->def_var+ facmm*mat_har->har;
					} else if(mtex->blendtype==MTEX_MUL) {
						R.matren->har= (facmul+factt)*mat_har->har;
					} else {
						R.matren->har= 128.0*factt+mat_har->har;
						if(R.matren->har<1) R.matren->har= 1;
					}
					mat_har= R.matren;
				}
			}
		}
	}
}


