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



/*  sequence.c      MIXED MODEL
 * 
 *  juni 95
 *  
 * 
 */

#include "blender.h"
#include "render.h"
#include "sequence.h"
#include "plugin.h"

Sequence *seq_arr[MAXSEQ+1];
int seqrectx, seqrecty;


/* Alle support voor plugin sequences: */


void open_plugin_seq(PluginSeq *pis, char *seqname)
{
	void (*fptr)();
	char *cp;
	
	/* voor zekerheid: (hier wordt op getest) */
	pis->doit= 0;
	pis->pname= 0;
	pis->stnames= 0;

	/* open the needed object */
	pis->handle = dlopen(pis->name, RTLD_NOW);
	
	if(pis->handle==0) {
		printf("%s\n", dlerror());
		error("Check error console");
		return;
	}
	
	/* find address of functions and data objects */
	pis->doit = dlsym(pis->handle, "plugin_seq_doit");

	fptr= dlsym(pis->handle, "plugin_seq_getinfo");
	fptr( &pis->stypes, &pis->vars);
	
	pis->pname= (char *)dlsym(pis->handle, "_name");
	pis->stnames= (char *)dlsym(pis->handle, "_stnames");
	pis->varstr= (void *)dlsym(pis->handle, "_varstr");
	pis->result= (float *)dlsym(pis->handle, "_result");

	cp= dlsym(pis->handle, "seqname");
	if(cp) strcpy(cp, seqname);

	fptr= dlsym(pis->handle, "plugin_seq_init");
	if(fptr) fptr();

}

PluginSeq *add_plugin_seq(char *str, char *seqname)
{
	void *handle;
	void (*fptr)();
	PluginSeq *pis;
	VarStruct *varstr;
	int a;
	
	pis= callocN(sizeof(PluginSeq), "PluginSeq");
	
	strcpy(pis->name, str);
	open_plugin_seq(pis, seqname);
	
	if(pis->doit==0) {
		if(pis->handle==0) errorstr("no plugin:", str, 0);
		else errorstr("in plugin:", str, 0);
		freeN(pis);
		return;
	}
	
	/* default waardes */
	varstr= pis->varstr;
	for(a=0; a<pis->vars; a++, varstr++) {
		if( (varstr->type & FLO)==FLO)
			pis->data[a]= varstr->def;
		else if( (varstr->type & INT)==INT)
			*((int *)(pis->data+a))= (int) varstr->def;
		else if( (varstr->type & LON)==LON)
			*((int *)(pis->data+a))= (int) varstr->def;
	}

	return pis;
}

void free_plugin_seq(PluginSeq *pis)
{

	if(pis==0) return;
	
	/* geen dlclose: dezelfde plugin kan meerdere keren geopend zijn: 1 handle */
	freeN(pis);
	
}

/* ***************** END PLUGIN ************************ */

void free_stripdata(int len, StripElem *se)
{
	StripElem *seo;
	int a;

	seo= se;
	
	for(a=0; a<len; a++, se++) {
		if(se->ibuf && se->ok!=2) freeImBuf(se->ibuf);
	}

	freeN(seo);

}

void free_strip(Strip *strip)
{
	StripElem *se;
	
	
	strip->us--;
	if(strip->us>0) return;
	if(strip->us<0) {
		printf("error: negative users in strip\n");
		return;
	}
	
	if(strip->stripdata) {
		free_stripdata(strip->len, strip->stripdata);
	}
	freeN(strip);
}

void new_stripdata(Sequence *seq)
{

	if(seq->strip) {
		if(seq->strip->stripdata) free_stripdata(seq->strip->len, seq->strip->stripdata);
		seq->strip->stripdata= 0;
		seq->strip->len= seq->len;
		if(seq->len>0) seq->strip->stripdata= callocN(seq->len*sizeof(StripElem), "stripelems");
	}
}

void free_sequence(Sequence *seq)
{
	extern Sequence *last_seq;
	
	if(seq->strip) free_strip(seq->strip);

	if(seq->anim) free_anim(seq->anim);
	
	free_plugin_seq(seq->plugin);
	
	if(seq==last_seq) last_seq= 0;
	
	freeN(seq);
}

int do_seq_count(ListBase *seqbase, int *totseq)
{
	Sequence *seq;
	
	seq= seqbase->first;
	while(seq) {
		(*totseq)++;
		if(seq->seqbase.first) do_seq_count(&seq->seqbase, totseq);
		seq= seq->next;
	}
}

int do_build_seqar(ListBase *seqbase, Sequence ***seqar, int depth)
{
	Sequence *seq;
	
	seq= seqbase->first;
	while(seq) {
		seq->depth= depth;
		if(seq->seqbase.first) do_build_seqar(&seq->seqbase, seqar, depth+1);
		**seqar= seq;
		(*seqar)++;
		seq= seq->next;
	}
}

void build_seqar(ListBase *seqbase, Sequence  ***seqar, int *totseq)
{
	Sequence *seq, **tseqar;
	
	*totseq= 0;
	do_seq_count(seqbase, totseq);
	
	if(*totseq==0) {
		*seqar= 0;
		return;
	}
	*seqar= mallocN(4* *totseq, "seqar");
	tseqar= *seqar;
	
	do_build_seqar(seqbase, seqar, 0);
	*seqar= tseqar;
}

void free_editing(Editing *ed)
{
	ListBase *lb;
	MetaStack *ms;
	Sequence *seq;
	
	if(ed==0) return;

	WHILE_SEQ(&ed->seqbase) {
		free_sequence(seq);
	}
	END_SEQ
	
	while(ms= ed->metastack.first) {
		remlink(&ed->metastack, ms);
		freeN(ms);
	}
	
	freeN(ed);
}

void calc_sequence(Sequence *seq)
{
	Sequence *seqm;
	float temp;
	int min, max;
	
	/* eerst recursief alle meta's aflopen */
	seqm= seq->seqbase.first;
	while(seqm) {
		if(seqm->seqbase.first) calc_sequence(seqm);
		seqm= seqm->next;
	}
	
	/* effecten: en meta automatische start en end */
	
	if(seq->type & SEQ_EFFECT) {
		/* pointers */
		if(seq->seq2==0) seq->seq2= seq->seq1;
		if(seq->seq3==0) seq->seq3= seq->seq1;
	
		/* effecten gaan van seq1 -> seq2: testen */
		
		/* we nemen de grootste start en de kleinste eind */
		
		seq->start= seq->startdisp= MAX2(seq->seq1->startdisp, seq->seq2->startdisp);
		seq->enddisp= MIN2(seq->seq1->enddisp, seq->seq2->enddisp);
		seq->len= seq->enddisp - seq->startdisp;
		
		if(seq->strip && seq->len!=seq->strip->len) {
			new_stripdata(seq);
		}

	}
	else {
		if(seq->type==SEQ_META) {
			seqm= seq->seqbase.first;
			min= 1000000;
			max= -1000000;
			while(seqm) {
				if(seqm->startdisp < min) min= seqm->startdisp;
				if(seqm->enddisp > max) max= seqm->enddisp;
				seqm= seqm->next;
			}
			seq->start= min;
			seq->len= max-min;
	
			if(seq->strip && seq->len!=seq->strip->len) {
				new_stripdata(seq);
			}
		}
		
		
		if(seq->startofs && seq->startstill) seq->startstill= 0;
		if(seq->endofs && seq->endstill) seq->endstill= 0;
	
		seq->startdisp= seq->start + seq->startofs - seq->startstill;
		seq->enddisp= seq->start+seq->len - seq->endofs + seq->endstill;
		
		seq->handsize= 10.0;	/* 10 frames */
		if( seq->enddisp-seq->startdisp < 20 ) {
			seq->handsize= 0.5*(seq->enddisp-seq->startdisp);
		}
		else if(seq->enddisp-seq->startdisp > 250) {
			seq->handsize= (seq->enddisp-seq->startdisp)/25;
		}
	}
}

void sort_seq()
{
	/* alle strips in soort bij elkaar en op volgorde van machine */
	ListBase seqbase, effbase;
	Editing *ed;
	Sequence *seq, *seqt;
	
	ed= G.scene->ed;
	if(ed==0) return;
	
	seqbase.first= seqbase.last= 0;
	effbase.first= effbase.last= 0;
	
	while(seq= ed->seqbasep->first) {
		remlink(ed->seqbasep, seq);
		
		if(seq->type & SEQ_EFFECT) {
			seqt= effbase.first;
			while(seqt) {
				if(seqt->machine>=seq->machine) {
					insertlinkbefore(&effbase, seqt, seq);
					break;
				}
				seqt= seqt->next;
			}
			if(seqt==0) addtail(&effbase, seq);
		}
		else {
			seqt= seqbase.first;
			while(seqt) {
				if(seqt->machine>=seq->machine) {
					insertlinkbefore(&seqbase, seqt, seq);
					break;
				}
				seqt= seqt->next;
			}
			if(seqt==0) addtail(&seqbase, seq);
		}
	}
	
	addlisttolist(&seqbase, &effbase);
	*(ed->seqbasep)= seqbase;
}


void clear_scene_in_allseqs(Scene *sce)
{
	Scene *sce1;
	Editing *ed;
	Sequence *seq;
	
	/* als er een scene delete is: alle seqs testen */
	
	sce1= G.main->scene.first;
	while(sce1) {
		if(sce1!=sce && sce1->ed) {
			ed= sce1->ed;
			
			WHILE_SEQ(&ed->seqbase) {
				
				if(seq->scene==sce) seq->scene= 0;
				
			}
			END_SEQ
		}
		
		sce1= sce1->id.next;
	}
}

/* ***************** DO THE SEQUENCE ***************** */

void do_alphaover_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int fac2, mfac, fac, fac4;
	int xo, tempc;
	char *rt1, *rt2, *rt, col[4];
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac2= 256.0*facf0;
	fac4= 256.0*facf1;

	while(y--) {
			
		x= xo;
		while(x--) {
			
			/* rt = rt1 over rt2  (alpha van rt1) */	
			
			fac= fac2;
			mfac= 256 - ( (fac2*rt1[0])>>8 );
			
			if(fac==0) *( (ulong *)rt) = *( (ulong *)rt2);
			else if(mfac==0) *( (ulong *)rt) = *( (ulong *)rt1);
			else {
				tempc= ( fac*rt1[0] + mfac*rt2[0])>>8;
				if(tempc>255) rt[0]= 255; else rt[0]= tempc;
				tempc= ( fac*rt1[1] + mfac*rt2[1])>>8;
				if(tempc>255) rt[1]= 255; else rt[1]= tempc;
				tempc= ( fac*rt1[2] + mfac*rt2[2])>>8;
				if(tempc>255) rt[2]= 255; else rt[2]= tempc;
				tempc= ( fac*rt1[3] + mfac*rt2[3])>>8;
				if(tempc>255) rt[3]= 255; else rt[3]= tempc;
			}
			rt1+= 4; rt2+= 4; rt+= 4;
		}
		
		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
				
			fac= fac4;
			mfac= 256 - ( (fac4*rt1[0])>>8 );
			
			if(fac==0) *( (ulong *)rt) = *( (ulong *)rt2);
			else if(mfac==0) *( (ulong *)rt) = *( (ulong *)rt1);
			else {
				tempc= ( fac*rt1[0] + mfac*rt2[0])>>8;
				if(tempc>255) rt[0]= 255; else rt[0]= tempc;
				tempc= ( fac*rt1[1] + mfac*rt2[1])>>8;
				if(tempc>255) rt[1]= 255; else rt[1]= tempc;
				tempc= ( fac*rt1[2] + mfac*rt2[2])>>8;
				if(tempc>255) rt[2]= 255; else rt[2]= tempc;
				tempc= ( fac*rt1[3] + mfac*rt2[3])>>8;
				if(tempc>255) rt[3]= 255; else rt[3]= tempc;
			}
			rt1+= 4; rt2+= 4; rt+= 4;
		}
	}
}

void do_alphaunder_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int fac2, mfac, fac, fac4;
	int xo;
	char *rt1, *rt2, *rt, col[4];
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac2= 256.0*facf0;
	fac4= 256.0*facf1;

	while(y--) {
			
		x= xo;
		while(x--) {
			
			/* rt = rt1 under rt2  (alpha van rt2) */
			
			/* deze ingewikkelde optimalisering is omdat 
			 * de 'skybuf' ingecrosst kan worden
			 */
			if(rt2[0]==0 && fac2==256) *( (ulong *)rt) = *( (ulong *)rt1);
			else if(rt2[0]==255) *( (ulong *)rt) = *( (ulong *)rt2);
			else {
				mfac= rt2[0];
				fac= (fac2*(256-mfac))>>8;
				
				if(fac==0) *( (ulong *)rt) = *( (ulong *)rt2);
				else {
					rt[0]= ( fac*rt1[0] + mfac*rt2[0])>>8;
					rt[1]= ( fac*rt1[1] + mfac*rt2[1])>>8;
					rt[2]= ( fac*rt1[2] + mfac*rt2[2])>>8;
					rt[3]= ( fac*rt1[3] + mfac*rt2[3])>>8;
				}
			}
			rt1+= 4; rt2+= 4; rt+= 4;
		}

		if(y==0) break;
		y--;

		x= xo;
		while(x--) {

			if(rt2[0]==0 && fac4==256) *( (ulong *)rt) = *( (ulong *)rt1);
			else if(rt2[0]==255) *( (ulong *)rt) = *( (ulong *)rt2);
			else {
				mfac= rt2[0];
				fac= (fac4*(256-mfac))>>8;
				
				if(fac==0) *( (ulong *)rt) = *( (ulong *)rt2);
				else {
					rt[0]= ( fac*rt1[0] + mfac*rt2[0])>>8;
					rt[1]= ( fac*rt1[1] + mfac*rt2[1])>>8;
					rt[2]= ( fac*rt1[2] + mfac*rt2[2])>>8;
					rt[3]= ( fac*rt1[3] + mfac*rt2[3])>>8;
				}
			}
			rt1+= 4; rt2+= 4; rt+= 4;
		}
	}
}


void do_cross_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int fac1, fac2, fac3, fac4;
	int xo;
	char *rt1, *rt2, *rt;
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac2= 256.0*facf0;
	fac1= 256-fac2;
	fac4= 256.0*facf1;
	fac3= 256-fac4;
	
	while(y--) {
			
		x= xo;
		while(x--) {
	
			rt[0]= (fac1*rt1[0] + fac2*rt2[0])>>8;
			rt[1]= (fac1*rt1[1] + fac2*rt2[1])>>8;
			rt[2]= (fac1*rt1[2] + fac2*rt2[2])>>8;
			rt[3]= (fac1*rt1[3] + fac2*rt2[3])>>8;
			
			rt1+= 4; rt2+= 4; rt+= 4;
		}
		
		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
	
			rt[0]= (fac3*rt1[0] + fac4*rt2[0])>>8;
			rt[1]= (fac3*rt1[1] + fac4*rt2[1])>>8;
			rt[2]= (fac3*rt1[2] + fac4*rt2[2])>>8;
			rt[3]= (fac3*rt1[3] + fac4*rt2[3])>>8;
			
			rt1+= 4; rt2+= 4; rt+= 4;
		}
		
	}
}

void do_gammacross_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	extern ushort *igamtab1, *gamtab;
	int fac1, fac2, fac3, fac4, col;
	int xo;
	char *rt1, *rt2, *rt;
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac2= 256.0*facf0;
	fac1= 256-fac2;
	fac4= 256.0*facf1;
	fac3= 256-fac4;
	
	while(y--) {
			
		x= xo;
		while(x--) {
	
			col= (fac1*igamtab1[rt1[0]] + fac2*igamtab1[rt2[0]])>>8;
			if(col>65535) rt[0]= 255; else rt[0]= *( (char *)(gamtab+col));
			col=(fac1*igamtab1[rt1[1]] + fac2*igamtab1[rt2[1]])>>8;
			if(col>65535) rt[1]= 255; else rt[1]= *( (char *)(gamtab+col));
			col= (fac1*igamtab1[rt1[2]] + fac2*igamtab1[rt2[2]])>>8;
			if(col>65535) rt[2]= 255; else rt[2]= *( (char *)(gamtab+col));
			col= (fac1*igamtab1[rt1[3]] + fac2*igamtab1[rt2[3]])>>8;
			if(col>65535) rt[3]= 255; else rt[3]= *( (char *)(gamtab+col));
			
			rt1+= 4; rt2+= 4; rt+= 4;
		}
		
		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
	
			col= (fac1*igamtab1[rt1[0]] + fac2*igamtab1[rt2[0]])>>8;
			if(col>65535) rt[0]= 255; else rt[0]= *( (char *)(gamtab+col));
			col= (fac1*igamtab1[rt1[1]] + fac2*igamtab1[rt2[1]])>>8;
			if(col>65535) rt[1]= 255; else rt[1]= *( (char *)(gamtab+col));
			col= (fac1*igamtab1[rt1[2]] + fac2*igamtab1[rt2[2]])>>8;
			if(col>65535) rt[2]= 255; else rt[2]= *( (char *)(gamtab+col));
			col= (fac1*igamtab1[rt1[3]] + fac2*igamtab1[rt2[3]])>>8;
			if(col>65535) rt[3]= 255; else rt[3]= *( (char *)(gamtab+col));
			
			rt1+= 4; rt2+= 4; rt+= 4;
		}
		
	}
}

void do_add_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int col, xo, fac1, fac2, fac3, fac4;
	char *rt1, *rt2, *rt;
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac1= 256.0*facf0;
	fac3= 256.0*facf1;
	
	while(y--) {
			
		x= xo;
		while(x--) {

			col= rt1[0]+ ((fac1*rt2[0])>>8);
			if(col>255) rt[0]= 255; else rt[0]= col;
			col= rt1[1]+ ((fac1*rt2[1])>>8);
			if(col>255) rt[1]= 255; else rt[1]= col;
			col= rt1[2]+ ((fac1*rt2[2])>>8);
			if(col>255) rt[2]= 255; else rt[2]= col;
			col= rt1[3]+ ((fac1*rt2[3])>>8);
			if(col>255) rt[3]= 255; else rt[3]= col;
	
			rt1+= 4; rt2+= 4; rt+= 4;
		}

		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
	
			col= rt1[0]+ ((fac3*rt2[0])>>8);
			if(col>255) rt[0]= 255; else rt[0]= col;
			col= rt1[1]+ ((fac3*rt2[1])>>8);
			if(col>255) rt[1]= 255; else rt[1]= col;
			col= rt1[2]+ ((fac3*rt2[2])>>8);
			if(col>255) rt[2]= 255; else rt[2]= col;
			col= rt1[3]+ ((fac3*rt2[3])>>8);
			if(col>255) rt[3]= 255; else rt[3]= col;
	
			rt1+= 4; rt2+= 4; rt+= 4;
		}
	}
}

void do_sub_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int col, xo, fac1, fac2, fac3, fac4;
	char *rt1, *rt2, *rt;
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac1= 256.0*facf0;
	fac3= 256.0*facf1;
	
	while(y--) {
			
		x= xo;
		while(x--) {

			col= rt1[0]- ((fac1*rt2[0])>>8);
			if(col<0) rt[0]= 0; else rt[0]= col;
			col= rt1[1]- ((fac1*rt2[1])>>8);
			if(col<0) rt[1]= 0; else rt[1]= col;
			col= rt1[2]- ((fac1*rt2[2])>>8);
			if(col<0) rt[2]= 0; else rt[2]= col;
			col= rt1[3]- ((fac1*rt2[3])>>8);
			if(col<0) rt[3]= 0; else rt[3]= col;
	
			rt1+= 4; rt2+= 4; rt+= 4;
		}

		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
	
			col= rt1[0]- ((fac3*rt2[0])>>8);
			if(col<0) rt[0]= 0; else rt[0]= col;
			col= rt1[1]- ((fac3*rt2[1])>>8);
			if(col<0) rt[1]= 0; else rt[1]= col;
			col= rt1[2]- ((fac3*rt2[2])>>8);
			if(col<0) rt[2]= 0; else rt[2]= col;
			col= rt1[3]- ((fac3*rt2[3])>>8);
			if(col<0) rt[3]= 0; else rt[3]= col;
	
			rt1+= 4; rt2+= 4; rt+= 4;
		}
	}
}

						/* L E T  O P:  rect2 en rect1 omgekeerd */
void do_drop_effect(float facf0, float facf1, int x, int y, ulong *rect2, ulong *rect1, ulong *out)
{
	int col, xo, yo, temp, fac1, fac2, fac3, fac4;
	int xofs= -8, yofs= 8;
	char *rt1, *rt2, *rt;
	
	xo= x;
	yo= y;
	
	rt2= (char *)(rect2 + yofs*x + xofs);
	
	rt1= (char *)rect1;
	rt= (char *)out;
	
	fac1= 70.0*facf0;
	fac3= 70.0*facf1;
	
	while(y-- > 0) {
		
		temp= y-yofs;
		if(temp > 0 && temp < yo) {
		
			x= xo;
			while(x--) {
					
				temp= x+xofs;
				if(temp > 0 && temp < xo) {
				
					temp= ((fac1*rt2[0])>>8);
		
					col= rt1[0]- temp;
					if(col<0) rt[0]= 0; else rt[0]= col;
					col= rt1[1]- temp;
					if(col<0) rt[1]= 0; else rt[1]= col;
					col= rt1[2]- temp;
					if(col<0) rt[2]= 0; else rt[2]= col;
					col= rt1[3]- temp;
					if(col<0) rt[3]= 0; else rt[3]= col;
				}
				else *( (ulong *)rt) = *( (ulong *)rt1);	
				
				rt1+= 4; rt2+= 4; rt+= 4;
			}
		}
		else {
			x= xo;
			while(x--) {
				*( (ulong *)rt) = *( (ulong *)rt1);	
				rt1+= 4; rt2+= 4; rt+= 4;
			}
		}
	
		if(y==0) break;
		y--;
			
		temp= y-yofs;
		if(temp > 0 && temp < yo) {
			
			x= xo;
			while(x--) {
				
				temp= x+xofs;
				if(temp > 0 && temp < xo) {
				
					temp= ((fac3*rt2[0])>>8);
					
					col= rt1[0]- temp;
					if(col<0) rt[0]= 0; else rt[0]= col;
					col= rt1[1]- temp;
					if(col<0) rt[1]= 0; else rt[1]= col;
					col= rt1[2]- temp;
					if(col<0) rt[2]= 0; else rt[2]= col;
					col= rt1[3]- temp;
					if(col<0) rt[3]= 0; else rt[3]= col;
				}
				else *( (ulong *)rt) = *( (ulong *)rt1);	
				
				rt1+= 4; rt2+= 4; rt+= 4;
			}
		}
		else {
			x= xo;
			while(x--) {
				*( (ulong *)rt) = *( (ulong *)rt1);	
				rt1+= 4; rt2+= 4; rt+= 4;
			}
		}
	}
}


void do_mul_effect(float facf0, float facf1, int x, int y, ulong *rect1, ulong *rect2, ulong *out)
{
	int col, xo, fac1, fac2, fac3, fac4;
	char *rt1, *rt2, *rt;
	
	xo= x;
	rt1= (char *)rect1;
	rt2= (char *)rect2;
	rt= (char *)out;
	
	fac1= 256.0*facf0;
	fac3= 256.0*facf1;
	
	/* formule:
	 *		fac*(a*b) + (1-fac)*a  => fac*a*(b-1)+a
	 */
	
	while(y--) {
			
		x= xo;
		while(x--) {
			
			rt[0]= rt1[0] + ((fac1*rt1[0]*(rt2[0]-256))>>16);
			rt[1]= rt1[1] + ((fac1*rt1[1]*(rt2[1]-256))>>16);
			rt[2]= rt1[2] + ((fac1*rt1[2]*(rt2[2]-256))>>16);
			rt[3]= rt1[3] + ((fac1*rt1[3]*(rt2[3]-256))>>16);

			rt1+= 4; rt2+= 4; rt+= 4;
		}

		if(y==0) break;
		y--;
		
		x= xo;
		while(x--) {
	
			rt[0]= rt1[0] + ((fac3*rt1[0]*(rt2[0]-256))>>16);
			rt[1]= rt1[1] + ((fac3*rt1[1]*(rt2[1]-256))>>16);
			rt[2]= rt1[2] + ((fac3*rt1[2]*(rt2[2]-256))>>16);
			rt[3]= rt1[3] + ((fac3*rt1[3]*(rt2[3]-256))>>16);
	
			rt1+= 4; rt2+= 4; rt+= 4;
		}
	}
}

void make_black_ibuf(ImBuf *ibuf)
{
	ulong *rect;
	int tot;	
	
	if(ibuf==0 || ibuf->rect==0) return;
	
	tot= ibuf->x*ibuf->y;
	rect= ibuf->rect;
	while(tot--) *(rect++)= 0;
	
}

void multibuf(ImBuf *ibuf, float fmul)
{
	char *rt;
	int a, mul, icol;
	
	mul= 256.0*fmul;

	a= ibuf->x*ibuf->y;
	rt= (char *)ibuf->rect;
	while(a--) {
		
		icol= (mul*rt[0])>>8;
		if(icol>254) rt[0]= 255; else rt[0]= icol;
		icol= (mul*rt[1])>>8;
		if(icol>254) rt[1]= 255; else rt[1]= icol;
		icol= (mul*rt[2])>>8;
		if(icol>254) rt[2]= 255; else rt[2]= icol;
		icol= (mul*rt[3])>>8;
		if(icol>254) rt[3]= 255; else rt[3]= icol;
		
		rt+= 4;
	}
}

void do_effect(int cfra, Sequence *seq, StripElem *se)
{
	StripElem *se1, *se2, *se3;
	float fac, facf, *fp;
	int x, y;
	char *cp;
	
	if(se->se1==0 || se->se2==0 || se->se3==0) {
		make_black_ibuf(se->ibuf);
		return;
	}
	
	/* als metastrip: andere se's */
	if(se->se1->ok==2) se1= se->se1->se1;
	else se1= se->se1;

	if(se->se2->ok==2) se2= se->se2->se1;
	else se2= se->se2;
	
	if(se->se3->ok==2) se3= se->se3->se1;
	else se3= se->se3;
	
	if(se1==0 || se2==0 || se3==0 || se1->ibuf==0 || se2->ibuf==0 || se3->ibuf==0) {
		make_black_ibuf(se->ibuf);
		return;
	}
	
	x= se2->ibuf->x;
	y= se2->ibuf->y;
	
	if(seq->ipo && seq->ipo->curve.first) {
		do_seq_ipo(seq);
		fac= seq->facf0;
		facf= seq->facf1;
	}
	else if ELEM3( seq->type, SEQ_CROSS, SEQ_GAMCROSS, SEQ_PLUGIN) {
		fac= cfra - seq->startdisp;
		facf= fac+0.5;
		fac /= seq->len;
		facf /= seq->len;
	}
	else {
		fac= facf= 1.0;
	}
	
	if( G.scene->r.mode & R_FIELDS ); else facf= fac;
	
	switch(seq->type) {
	case SEQ_CROSS:
		do_cross_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_GAMCROSS:
		do_gammacross_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_ADD:
		do_add_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_SUB:
		do_sub_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_MUL:
		do_mul_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_ALPHAOVER:
		do_alphaover_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_OVERDROP:
		do_drop_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		do_alphaover_effect(fac, facf, x, y, se1->ibuf->rect, se->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_ALPHAUNDER:
		do_alphaunder_effect(fac, facf, x, y, se1->ibuf->rect, se2->ibuf->rect, se->ibuf->rect);
		break;
	case SEQ_PLUGIN:
		if(seq->plugin && seq->plugin->doit) {
			
			fp= dlsym(seq->plugin->handle, "cfra");
			if(fp) *fp= frame_to_float(CFRA);
			
			cp= dlsym(seq->plugin->handle, "seqname");
			if(cp) strcpy(cp, seq->name+2);

			seq->plugin->doit(seq->plugin->data, fac, facf, x, y, se1->ibuf, se2->ibuf, se->ibuf, se3->ibuf);
		}
		break;
	}
	
}

int evaluate_seq_frame(int cfra)
{
	Sequence *seq;
	Editing *ed;
	int totseq=0;
	
	bzero(seq_arr, 4*MAXSEQ);
	
	ed= G.scene->ed;
	if(ed==0) return 0;
	
	seq= ed->seqbasep->first;
	while(seq) {
		if(seq->startdisp <=cfra && seq->enddisp > cfra) {
			seq_arr[seq->machine]= seq;
			totseq++;
		}
		seq= seq->next;
	}
	
	return totseq;
}

StripElem *give_stripelem(Sequence *seq, int cfra)
{
	Strip *strip;
	StripElem *se;
	int nr;
	
	strip= seq->strip;
	se= strip->stripdata;
	
	if(se==0) return 0;
	if(seq->startdisp >cfra || seq->enddisp <= cfra) return 0;
	
	if(cfra <= seq->start) nr= 0;
	else if(cfra >= seq->start+seq->len-1) nr= seq->len-1;
	else nr= cfra-seq->start;
	
	se+= nr;
	se->nr= nr;
	
	return se;
}

void set_meta_stripdata(Sequence *seqm)
{
	Sequence *seq, *seqim, *seqeff;
	Editing *ed;
	ListBase *tempbase;
	StripElem *se;
	int a, cfra, b;
	
	/* zet alle ->se1 pointers in stripdata, dan kan daar de ibuf uitgelezen */
	
	ed= G.scene->ed;
	if(ed==0) return;
	
	tempbase= ed->seqbasep;
	ed->seqbasep= &seqm->seqbase;
	
	se= seqm->strip->stripdata;
	for(a=0; a<seqm->len; a++, se++) {
		cfra= a+seqm->start;
		if(evaluate_seq_frame(cfra)) {
			
			/* we nemen de hoogste effectstrip of de laagste imagestrip/metastrip */
			seqim= seqeff= 0;
			
			for(b=1; b<MAXSEQ; b++) {
				if(seq_arr[b]) {
					seq= seq_arr[b];
					if(seq->type & SEQ_EFFECT) {
						if(seqeff==0) seqeff= seq;
						else if(seqeff->machine < seq->machine) seqeff= seq;
					}
					else {
						if(seqim==0) seqim= seq;
						else if(seqim->machine > seq->machine) seqim= seq;
					}
				}
			}
			if(seqeff) seq= seqeff;
			else if(seqim) seq= seqim;
			else seq= 0;
			
			if(seq) {
				se->se1= give_stripelem(seq, cfra);
			}
			else se->se1= 0;
		}
	}

	ed->seqbasep= tempbase;
}



/* HULPFUNKTIES VOOR GIVE_IBUF_SEQ */

int do_seq_count_cfra(ListBase *seqbase, int *totseq, int cfra)
{
	Sequence *seq;
	
	seq= seqbase->first;
	while(seq) {
		if(seq->startdisp <=cfra && seq->enddisp > cfra) {

			if(seq->seqbase.first) {
				
				if(cfra< seq->start) do_seq_count_cfra(&seq->seqbase, totseq, seq->start);
				else if(cfra> seq->start+seq->len-1) do_seq_count_cfra(&seq->seqbase, totseq, seq->start+seq->len-1);
				else do_seq_count_cfra(&seq->seqbase, totseq, cfra);
			}

			(*totseq)++;
		}
		seq= seq->next;
	}
}

int do_build_seqar_cfra(ListBase *seqbase, Sequence ***seqar, int cfra)
{
	Sequence *seq;
	StripElem *se;
	Scene *oldsce;
	ImBuf *ibuf;
	ulong *rectot;
	int oldbg, oldcfra, doseq;
	char name[120];
	
	seq= seqbase->first;
	while(seq) {
		
		/* op nul zetten ivm free_imbuf_seq... */
		seq->curelem= 0;
		
		if(seq->startdisp <=cfra && seq->enddisp > cfra) {
		
			if(seq->seqbase.first) {
				if(cfra< seq->start) do_build_seqar_cfra(&seq->seqbase, seqar, seq->start);
				else if(cfra> seq->start+seq->len-1) do_build_seqar_cfra(&seq->seqbase, seqar, seq->start+seq->len-1);
				else do_build_seqar_cfra(&seq->seqbase, seqar, cfra);
			}

			**seqar= seq;
			(*seqar)++;
			
			se=seq->curelem= give_stripelem(seq, cfra);

			if(se) {
				if(seq->type == SEQ_META) {
					se->ok= 2;
					if(se->se1==0) set_meta_stripdata(seq);
					if(se->se1) {
						se->ibuf= se->se1->ibuf;
					}
				}
				else if(seq->type & SEQ_EFFECT) {
				
					/* testen of image te klein is: opnieuw maken */
					if(se->ibuf) {
						if(se->ibuf->x < seqrectx || se->ibuf->y < seqrecty) {
							freeImBuf(se->ibuf);
							se->ibuf= 0;
						}
					}
					
					/* moet het effect (opnieuw) berekend? */
					
					if(se->ibuf==0 || (se->se1 != seq->seq1->curelem) || (se->se2 != seq->seq2->curelem) || (se->se3 != seq->seq3->curelem)) {
						se->se1= seq->seq1->curelem;
						se->se2= seq->seq2->curelem;
						se->se3= seq->seq3->curelem;
						
						if(se->ibuf==0) se->ibuf= allocImBuf(seqrectx, seqrecty, 32, IB_rect, 0);
			
						do_effect(cfra, seq, se);
					}
					
					/* size testen */
					if(se->ibuf) {
						if(se->ibuf->x != seqrectx || se->ibuf->y != seqrecty ) {
							if(G.scene->r.mode & R_OSA) 
								scaleImBuf(se->ibuf, seqrectx, seqrecty);
							else 
								scalefastImBuf(se->ibuf, seqrectx, seqrecty);
						}
					}
				}
				else if(seq->type < SEQ_EFFECT) {
					
					if(se->ibuf) {
						/* testen of image te klein is: opnieuw laden */
						if(se->ibuf->x < seqrectx || se->ibuf->y < seqrecty) {
							freeImBuf(se->ibuf);
							se->ibuf= 0;
							se->ok= 1;
						}
					}

					if(seq->type==SEQ_IMAGE) {
						if(se->ok && se->ibuf==0) {
						
							/* als playanim of render: geen waitcursor doen */
							if((G.f & G_PLAYANIM)==0) waitcursor(1);
						
							strcpy(name, seq->strip->dir);
							strcat(name, se->name);
							convertstringcode(name);
							se->ibuf= loadiffname(name, IB_rect);

							if((G.f & G_PLAYANIM)==0) waitcursor(0);
							
							if(se->ibuf==0) se->ok= 0;
							else {
								if(se->ibuf->depth==32) converttopremul(se->ibuf);
								seq->strip->orx= se->ibuf->x;
								seq->strip->ory= se->ibuf->y;
							}
						}
					}
					else if(seq->type==SEQ_MOVIE) {
						if(se->ok && se->ibuf==0) {
						
							/* als playanim of render: geen waitcursor doen */
							if((G.f & G_PLAYANIM)==0) waitcursor(1);
						
							if(seq->anim==0) {
								strcpy(name, seq->strip->dir);
								strcat(name, seq->strip->stripdata->name);
								convertstringcode(name);
								seq->anim = openanim(name, IB_rect);
							}
							if(seq->anim) {
								se->ibuf = anim_absolute(seq->anim, se->nr);
							}
							
							if(se->ibuf==0) se->ok= 0;
							else {
								if(se->ibuf->depth==32) converttopremul(se->ibuf);
								seq->strip->orx= se->ibuf->x;
								seq->strip->ory= se->ibuf->y;
								if(seq->flag & SEQ_FILTERY) filtery(se->ibuf);
								if(seq->mul==0.0) seq->mul= 1.0;
								if(seq->mul != 1.0) multibuf(se->ibuf, seq->mul);
							}
							if((G.f & G_PLAYANIM)==0) waitcursor(0);							
						}
					}
					else if(seq->type==SEQ_SCENE && se->ibuf==0) {
						View3D *vd;

						oldsce= G.scene;
						set_scene_bg(seq->scene);
						
						/* oneindige lus voorkomen */
						doseq= G.scene->r.scemode & R_DOSEQ;
						G.scene->r.scemode &= ~R_DOSEQ;
						
						/* vanalles bewaren */
						oldcfra= CFRA; CFRA= seq->sfra + se->nr;
						waitcursor(1);
						oldbg= G.background; G.background= 1;
						rectot= R.rectot; R.rectot= 0;
						/* dit is nodig omdat de huidige 3D window niet de layers mag leveren, alsof het background render is */
						vd= G.vd;
						G.vd= 0;
						
						initrender();
			
						se->ibuf= allocImBuf(R.rectx, R.recty, 32, IB_rect, 0);
						if(R.rectot) memcpy(se->ibuf->rect, R.rectot, 4*R.rectx*R.recty);
	
						/* en restore */
						G.vd= vd;
						G.background= oldbg;
						if((G.f & G_PLAYANIM)==0) waitcursor(0);
						CFRA= oldcfra;
						if(R.rectot) freeN(R.rectot);
						R.rectot= rectot;
						G.scene->r.scemode |= doseq;
						set_scene_bg(oldsce);
						
						/* restore!! */
						R.rectx= seqrectx;
						R.recty= seqrecty;
					}
					
					/* size testen */
					if(se->ibuf) {
						if(se->ibuf->x != seqrectx || se->ibuf->y != seqrecty ) {
						
							if (se->ibuf->y > 288) {
								if (seqrecty > 288) scalefieldImBuf(se->ibuf, seqrectx, seqrecty);
								else {
									de_interlace(se->ibuf);

									if(G.scene->r.mode & R_OSA) 
										scaleImBuf(se->ibuf, seqrectx, seqrecty);
									else 
										scalefastImBuf(se->ibuf, seqrectx, seqrecty);
								}
							}
							else {
								if(G.scene->r.mode & R_OSA) 
									scaleImBuf(se->ibuf, seqrectx, seqrecty);
								else 
									scalefastImBuf(se->ibuf, seqrectx, seqrecty);
							}
						}
					}
				}
			}
		}

		seq= seq->next;
	}
}

ImBuf *give_ibuf_seq(int cfra)
{
	Sequence **tseqar, **seqar;
	Sequence *seq, *seqfirst=0, *effirst=0;
	Editing *ed;
	StripElem *se;
	int seqnr, totseq;

	/* we maken recursief een 'stack' van sequences, deze is ook
	 * gesorteerd en kan gewoon doorlopen worden.
	 * Deze methode is vooral ontwikkeld voor stills voor en achter meta's
	 */

	totseq= 0;
	ed= G.scene->ed;
	if(ed==0) return 0;
	do_seq_count_cfra(ed->seqbasep, &totseq, cfra);

	if(totseq==0) return 0;
	
	seqrectx= (G.scene->r.size*G.scene->r.xsch)/100;
	seqrecty= (G.scene->r.size*G.scene->r.ysch)/100;


	/* tseqar is nodig omdat in do_build_... de pointer verandert */
	seqar= tseqar= callocN(4*totseq, "seqar");
	
	/* deze fie laadt en maakt ook de ibufs */
	do_build_seqar_cfra(ed->seqbasep, &seqar, cfra);
	seqar= tseqar;
	
	for(seqnr=0; seqnr<totseq; seqnr++) {
		seq= seqar[seqnr];		

		se= seq->curelem;
		if(se) {
			if(seq->type==SEQ_META) {
				
				/* onderste strip! */
				if(seqfirst==0) seqfirst= seq;
				else if(seqfirst->depth > seq->depth) seqfirst= seq;
				else if(seqfirst->machine > seq->machine) seqfirst= seq;
				
			}
			else if(seq->type & SEQ_EFFECT) {
				
				/* bovenste strip! */
				if(seqfirst==0) seqfirst= seq;
				else if(seqfirst->depth > seq->depth) seqfirst= seq;
				else if(seqfirst->machine < seq->machine) seqfirst= seq;
				

			}
			else if(seq->type < SEQ_EFFECT) {	/* images */
				
				/* onderste strip! zodat je bovenin altijd hulptroep kan bewaren */
				
				if(seqfirst==0) seqfirst= seq;
				else if(seqfirst->depth > seq->depth) seqfirst= seq;
				else if(seqfirst->machine > seq->machine) seqfirst= seq;
				
			}
		}
	}
	
	freeN(seqar);	
	
	if(seqfirst->curelem==0) return 0;
	return seqfirst->curelem->ibuf;

}

void free_imbuf_seq_except(int cfra)
{
	Sequence *seq;
	StripElem *se;
	Editing *ed;
	int a;
	
	ed= G.scene->ed;
	if(ed==0) return;
	
	WHILE_SEQ(&ed->seqbase) {		
		
		if(seq->strip) {
		
			if( seq->type==SEQ_META ) {
				;
			}
			else {
				se= seq->strip->stripdata;
				for(a=0; a<seq->len; a++, se++) {
					if(se!=seq->curelem && se->ibuf) {
						freeImBuf(se->ibuf);
						se->ibuf= 0;
						se->ok= 1;
						se->se1= se->se2= se->se3= 0;
					}
				}
			}
			
			if(seq->type==SEQ_MOVIE) {
				if(seq->startdisp > cfra || seq->enddisp < cfra) {
					if(seq->anim) {
						free_anim(seq->anim);
						seq->anim = 0;
					}
				}
			}
		}
	}
	END_SEQ
}

void do_render_seq()
{
	static ImBuf *lastibuf=0;
	ImBuf *ibuf;
	extern ImBuf * izbuf;
	
	/* plaatje in R.rectot kopieeren */
	
	G.f |= G_PLAYANIM;	/* waitcursor patch */
	
	ibuf= give_ibuf_seq(CFRA);
	if(ibuf) {
		memcpy(R.rectot, ibuf->rect, 4*R.rectx*R.recty);
		if (ibuf->zbuf) {
			if (izbuf) freeImBuf(izbuf);
			izbuf = allocImBuf(ibuf->x, ibuf->y, 0, IB_zbuf, 0);
			memcpy(izbuf->zbuf, ibuf->zbuf, 4 * ibuf->x * ibuf->y);
		}
		free_imbuf_seq_except(CFRA);
	}
	G.f &= ~G_PLAYANIM;
	
}

