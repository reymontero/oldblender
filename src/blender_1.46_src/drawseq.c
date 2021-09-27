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



/* drawseq.c  mei 94		GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "sequence.h"

int no_rightbox=0, no_leftbox= 0;

void EmbossBoxf(x1, y1, x2, y2, sel, dark, light)
float x1, y1, x2, y2;
int sel;
ulong dark, light;
{

	if(sel) cpack(dark); 
	else cpack(light);
	if(sel) linewidth(2);
	rect(x1, y2, x2, y2);	/* boven */
	if(no_leftbox==0) rect(x1, y1, x1, y2);	/* links */
	
	if(sel && G.machine!=ENTRY) linewidth(1);
	
	if(sel) cpack(light); 
	else cpack(dark);	
	rect(x1, y1, x2, y1);	/* onder */
	if(no_rightbox==0) rect(x2, y1, x2, y2);	/* rechts */
	
}

char *give_seqname(Sequence *seq)
{
	if(seq->type==SEQ_META) return "META";
	else if(seq->type==SEQ_SCENE) return "SCENE";
	else if(seq->type==SEQ_MOVIE) return "MOVIE";
	else if(seq->type<SEQ_EFFECT) return seq->strip->dir;
	else if(seq->type==SEQ_CROSS) return "CROSS";
	else if(seq->type==SEQ_GAMCROSS) return "GAMMA CROSS";
	else if(seq->type==SEQ_ADD) return "ADD";
	else if(seq->type==SEQ_SUB) return "SUB";
	else if(seq->type==SEQ_MUL) return "MUL";
	else if(seq->type==SEQ_ALPHAOVER) return "ALPHAOVER";
	else if(seq->type==SEQ_ALPHAUNDER) return "ALPHAUNDER";
	else if(seq->type==SEQ_OVERDROP) return "OVER DROP";
	else if(seq->type==SEQ_PLUGIN) {
		if(seq->plugin && seq->plugin->doit) return seq->plugin->pname;
		return "PLUGIN";
	}
	else return "EFFECT";
	
}

void draw_cfra_seq()
{
	float x1, y1, x2, y2;
	
	x1= CFRA;
	y1= G.v2d->cur.ymin;
	x2= CFRA+1;
	y2= G.v2d->cur.ymax;
	
	cpack(0x309050);
	sboxf(x1, y1, x2, y2);
}

ulong seq_color(Sequence *seq)
{
	switch(seq->type) {
	case SEQ_META:
		return 0x509090;
	case SEQ_MOVIE:
		return 0x805040;
	case SEQ_SCENE:
		if(seq->scene==G.scene) return 0x709050;
		return 0x609060;
	case SEQ_CROSS:
		return 0x505090;
	case SEQ_GAMCROSS:
		return 0x5040A0;
	case SEQ_ADD:
		return 0x6060A0;
	case SEQ_SUB:
		return 0x8060A0;
	case SEQ_MUL:
		return 0x8080A0;
	case SEQ_ALPHAOVER:
		return 0x6080A0;
	case SEQ_ALPHAUNDER:
		return 0x9080A0;
	case SEQ_OVERDROP:
		return 0x5080B0;
	case SEQ_PLUGIN:
		return 0x906000;
	default:
		return 0x906060;
	}
	
}

void drawmeta_contents(Sequence *seqm, float x1, float y1, float x2, float y2)
{
	Sequence *seq;
	float dx;
	int a, nr;
	
	nr= 0;
	WHILE_SEQ(&seqm->seqbase) {
		nr++;
	}
	END_SEQ
	
	dx= (x2-x1)/nr;
	
	WHILE_SEQ(&seqm->seqbase) {
		cpack(seq_color(seq));
		sboxf(x1, y1, x1+0.9*dx, y2);
		EmbossBoxf(x1, y1, x1+0.9*dx, y2, 0, 0x404040, 0xB0B0B0);
		x1+= dx;
	}
	END_SEQ
}

void drawseq(Sequence *seq)
{
	float v1[2], v2[2], x1, x2, y1, y2;
	ulong body, dark, light;
	int len, size;
	short mval[2];
	char str[120], *strp;
	

	if(seq->startdisp > seq->enddisp) body= 0x707070;

	body= seq_color(seq);
	dark= 0x202020;
	light= 0xB0B0B0;

	if(G.moving && (seq->flag & SELECT)) {
		if(seq->flag & SEQ_OVERLAP) dark= light= 0x4040FF;
		else {
			if(seq->flag & (SEQ_LEFTSEL+SEQ_RIGHTSEL));
			else dark= light= 0xFFFFFF;
		}
	}

	/* body */
	if(seq->startstill) x1= seq->start;
	else x1= seq->startdisp;
	y1= seq->machine+0.2;
	if(seq->endstill) x2= seq->start+seq->len;
	else x2= seq->enddisp;
	y2= seq->machine+0.8;
	
	cpack(body);
	sboxf(x1, y1, x2, y2);
	EmbossBoxf(x1, y1, x2, y2, seq->flag & 1, dark, light);
	
	v1[1]= y1;
	v2[1]= y2;
	if(seq->type < SEQ_EFFECT) {

		/* decoratie: balkjes */
		x1= seq->startdisp;
		x2= seq->enddisp;
		
		if(seq->startofs) {
			cpack(0x707070);
			sboxf((float)(seq->start), y1-0.2, x1, y1);
			EmbossBoxf((float)(seq->start), y1-0.2, x1, y1, seq->flag & 1, dark, light);
		}
		if(seq->endofs) {
			cpack(0x707070);
			sboxf(x2, y2, (float)(seq->start+seq->len), y2+0.2);
			EmbossBoxf(x2, y2, (float)(seq->start+seq->len), y2+0.2, seq->flag & 1, dark, light);
		}

		if(seq->startstill) {
			cpack(body);
			sboxf(x1, y1+0.1, (float)(seq->start), y1+0.5);
			no_rightbox= 1;
			EmbossBoxf(x1, y1+0.1, (float)(seq->start), y1+0.5, seq->flag & 1, dark, light);
			no_rightbox= 0;
		}
		if(seq->endstill) {
			cpack(body);
			sboxf((float)(seq->start+seq->len), y1+0.1, x2, y1+0.5);
			no_leftbox= 1;
			EmbossBoxf((float)(seq->start+seq->len), y1+0.1, x2, y1+0.5, seq->flag & 1, dark, light);
			no_leftbox= 0;
		}
		
	}

	/* berekenen of seq lang genoeg is om naam te printen */
	x1= seq->startdisp+seq->handsize;
	x2= seq->enddisp-seq->handsize;
	
	/* maar eerst de inhoud van de meta */
	if(seq->type==SEQ_META) drawmeta_contents(seq, x1, y1+0.15, x2, y2-0.15);
	
	if(x1<G.v2d->cur.xmin) x1= G.v2d->cur.xmin;
	else if(x1>G.v2d->cur.xmax) x1= G.v2d->cur.xmax;
	if(x2<G.v2d->cur.xmin) x2= G.v2d->cur.xmin;
	else if(x2>G.v2d->cur.xmax) x2= G.v2d->cur.xmax;
	
	if(x1 != x2) {
		v1[0]= x1;
		ipoco_to_areaco_noclip(v1, mval);
		x1= mval[0];
		v2[0]= x2;
		ipoco_to_areaco_noclip(v2, mval);
		x2= mval[0];
		size= x2-x1;
		
		if(seq->type == SEQ_META) sprintf(str, "%d %s:", seq->len, give_seqname(seq));
		else if(seq->type == SEQ_SCENE) {
			if(seq->scene) sprintf(str, "%d %s %s", seq->len, give_seqname(seq), seq->scene->id.name+2);
			else sprintf(str, "%d %s", seq->len, give_seqname(seq));
		}
		else if(seq->type & SEQ_EFFECT) {
			if(seq->seq3!=seq->seq2 && seq->seq1!=seq->seq3)
				sprintf(str, "%d %s: %d-%d (use %d)", seq->len, give_seqname(seq), seq->seq1->machine, seq->seq2->machine, seq->seq3->machine);
			else 
				sprintf(str, "%d %s: %d-%d", seq->len, give_seqname(seq), seq->seq1->machine, seq->seq2->machine);
		}
		else if(seq->name[2]) sprintf(str, "%s", seq->name+2);
		else sprintf(str, "%d %s%s", seq->len, seq->strip->dir, seq->strip->stripdata->name);

		strp= str;
		
		while( (len= fmgetstrwidth(G.font, strp)) > size) {
			if(len < 10) break;
			if(strp[1]==0) break;
			strp++;
		}
		
		mval[0]= (x1+x2-len+1)/2;
		mval[1]= 1;
		areamouseco_to_ipoco(mval, &x1, &x2);
		
		if(seq->flag & SELECT) cpack(0xFFFFFF);
		else cpack(0x0);
		cmov2(x1, y1+0.2);
		fmprstr(strp);
	}

	if(seq->type < SEQ_EFFECT) {

		/* decoratie: driehoekjes */
		x1= seq->startdisp;
		x2= seq->enddisp;
		
		body+= 0x101010;
		dark= 0x202020;
		light= 0xB0B0B0;
		
		/* linker driehoek */
		
		if(seq->flag & SEQ_LEFTSEL) {
			cpack(body+0x20);
			if(G.moving) {
				if(seq->flag & SEQ_OVERLAP) dark= light= 0x4040FF;
				else dark= light= 0xFFFFFF;
			}
		}
		else {
			cpack(body);
		}
		bgnpolygon();
			v1[0]= x1; v2f(v1);
			v2[0]= x1; v2f(v2);
			v2[0]+= seq->handsize; v2[1]= (y1+y2)/2.0; v2f(v2); v2[1]= y2;
		endpolygon();
		
		cpack(light);
		bgnline();
			v1[0]= x1; v2f(v1);
			v2[0]= x1; v2f(v2);
			v2[0]+= seq->handsize; v2[1]= (y1+y2)/2.0; v2f(v2); v2[1]= y2;
			cpack(dark);
			v2f(v1);
		endline();
		
		if(G.moving || (seq->flag & SEQ_LEFTSEL)) {
			cpack(0xFFFFFF);
			cmov2(x1, y1+0.2);
			sprintf(str, "%d", seq->startdisp);
			fmprstr(str);
		}
		
		/* rechter driehoek */
		
		dark= 0x202020;
		light= 0xB0B0B0;

		if(seq->flag & SEQ_RIGHTSEL) {
			cpack(body+0x20);
			if(G.moving) {
				if(seq->flag & SEQ_OVERLAP) dark= light= 0x4040FF;
				else dark= light= 0xFFFFFF;
			}
		}
		else {
			cpack(body);
		}
		bgnpolygon();
			v2[0]= x2; v2f(v2);
			v1[0]= x2; v2f(v1);
			v2[0]-= seq->handsize; v2[1]= (y1+y2)/2.0; v2f(v2); v2[1]= y2;
		endpolygon();
		
		cpack(dark);
		bgnline();
			v2[0]= x2; v2f(v2);
			v1[0]= x2; v2f(v1);
			v1[0]-= seq->handsize; v1[1]= (y1+y2)/2.0; v2f(v1); v1[1]= y2;
			cpack(light);
			v2f(v2);
		endline();

		if(G.moving || (seq->flag & SEQ_RIGHTSEL)) {
			cpack(0xFFFFFF);
			cmov2(x2-seq->handsize/2, y1+0.2);
			sprintf(str, "%d", seq->enddisp-1);
			fmprstr(str);
		}

		
	}
}

Sequence *special_seq_update= 0;

void set_special_seq_update(int val)
{
	Sequence *seq;
	int x;

	/* als met muis in sequence && LEFTMOUSE */
	if(val) {
		special_seq_update= find_nearest_seq(&x);
	}
	else special_seq_update= 0;
}


void draw_image_seq()
{
	SpaceSeq *sseq;
	StripElem *se;
	struct ImBuf *ibuf;
	ulong *rect;
	int cx, cy, xim, yim, x1, x2, y1, y2;
	short mval[2];
	
	cpack(0);
	clear();
	curarea->win_swap= 1;
	
	ibuf= give_ibuf_seq(CFRA);
	if(special_seq_update) {
		if(se= special_seq_update->curelem) {
			if(se->ok==2) {
				if(se->se1)
					ibuf= se->se1->ibuf;
			}
			else ibuf= se->ibuf;
		}
	}
	if(ibuf==0 || ibuf->rect==0) return;
	
	sseq= curarea->spacedata.first;
	if(sseq==0) return;
	
	/* plek berekenen */
	x1= curarea->winrct.xmin+(curarea->winx-sseq->zoom*ibuf->x)/2;
	y1= curarea->winrct.ymin+(curarea->winy-sseq->zoom*ibuf->y)/2;
	
	
	rectwrite_part(curarea->winrct.xmin, curarea->winrct.ymin, 
				curarea->winrct.xmax, curarea->winrct.ymax, 
				x1, y1, ibuf->x, ibuf->y, (float)sseq->zoom, ibuf->rect);

}

void draw_extra_seqinfo()
{
	extern Sequence *last_seq;
	StripElem *se, *last;
	float xco, xfac;
	int sta, end;
	char str[256];
	
	if(last_seq==0) return;
		
	/* xfac: afm 1 pixel */
	xfac= G.v2d->cur.xmax - G.v2d->cur.xmin;
	xfac/= (float)(G.v2d->mask.xmax-G.v2d->mask.xmin);
	xco= G.v2d->cur.xmin+40*xfac;
	
	cpack(0);
	
	/* NAAM */
	cmov2(xco, 0.3);
	strcpy(str, give_seqname(last_seq));
	fmprstr(str);
	xco += xfac*fmgetstrwidth(G.font, str) +30.0*xfac;

	if(last_seq->type==SEQ_SCENE && last_seq->scene) {
		cmov2(xco, 0.3);
		fmprstr(last_seq->scene->id.name+2);
		xco += xfac*fmgetstrwidth(G.font, last_seq->scene->id.name+2) +30.0*xfac;
	}

	/* LEN */
	if(last_seq->type & SEQ_EFFECT)
		sprintf(str, "len: %d   From %d - %d", last_seq->len, last_seq->startdisp, last_seq->enddisp-1);
	else 
		sprintf(str, "len: %d (%d)", last_seq->enddisp-last_seq->startdisp, last_seq->len);
	
	cmov2(xco, 0.3);
	fmprstr(str);
	xco += xfac*fmgetstrwidth(G.font, str) +30.0*xfac;

	if(last_seq->type==SEQ_IMAGE) {
	
		/* CURRENT */
		se= (StripElem *)give_stripelem(last_seq, CFRA);
		if(se) {
			sprintf(str, "cur: %s", se->name);
			cmov2(xco, 0.3);
			fmprstr(str);
			xco += xfac*fmgetstrwidth(G.font, str) +30.0*xfac;
		}
		
		/* FIRST EN LAST */
	
		if(last_seq->strip) {
			se= last_seq->strip->stripdata;
			last= se+last_seq->len-1;
			if(last_seq->startofs) se+= last_seq->startofs;
			if(last_seq->endofs) last-= last_seq->endofs;
			
			sprintf(str, "First: %s at %d     Last: %s at %d", se->name, last_seq->startdisp, last->name, last_seq->enddisp-1);
			cmov2(xco, 0.3);
			fmprstr(str);
			xco += xfac*fmgetstrwidth(G.font, str) +30.0*xfac;
			
			/* orig size */
			sprintf(str, "OrigSize: %d x %d", last_seq->strip->orx, last_seq->strip->ory);
			cmov2(xco, 0.3);
			fmprstr(str);
			xco += xfac*fmgetstrwidth(G.font, str) +30.0*xfac;
		}
	}
	else if(last_seq->type==SEQ_MOVIE) {
	
		sta= last_seq->startofs;
		end= last_seq->len-1-last_seq->endofs;
	
		sprintf(str, "%s   %s%s  First: %d at %d     Last: %d at %d     Cur: %d", 
				last_seq->name+2, last_seq->strip->dir, last_seq->strip->stripdata->name, 
				sta, last_seq->startdisp, end, last_seq->enddisp-1, CFRA-last_seq->startdisp);
		
		cmov2(xco, 0.3);
		fmprstr(str);
	}
}

void drawseqspace()
{
	SpaceSeq *sseq;
	Editing *ed;
	Sequence *seq;
	int ofsx, ofsy, a;

	ed= G.scene->ed;
	
	sseq= curarea->spacedata.first;
	if(sseq->mainb==1) {
		draw_image_seq();
		return;
	}

	if(ed && ed->metastack.first) cpack(0x687070);
	else cpack(0x606060);
	clear();
	fmsetfont(G.font);
	
	calc_scrollrcts();

	if(curarea->winx>SCROLLB+10 && curarea->winy>SCROLLH+10) {
		if(G.v2d->scroll) {	
			ofsx= curarea->winrct.xmin;	/* ivm mywin */
			ofsy= curarea->winrct.ymin;
			viewport(ofsx+G.v2d->mask.xmin, ofsx+G.v2d->mask.xmax-1, ofsy+G.v2d->mask.ymin, ofsy+G.v2d->mask.ymax-1);
		}
	}


	ortho2(G.v2d->cur.xmin, G.v2d->cur.xmax, G.v2d->cur.ymin, G.v2d->cur.ymax);
	
	cpack(0x585858);
	sboxf(G.v2d->cur.xmin, 0.0, G.v2d->cur.xmax, 1.0);
	
	boundbox_seq();
	calc_ipogrid();	
	draw_ipogrid();
	draw_cfra_seq();

	/* sequenties: eerst de deselect */
	
	if(ed) {
		seq= ed->seqbasep->first;
		while(seq) {
			if(seq->flag & SELECT); else drawseq(seq);
			seq= seq->next;
		}
	}
	ed= G.scene->ed;
	if(ed) {
		seq= ed->seqbasep->first;
		while(seq) {
			if(seq->flag & SELECT) drawseq(seq);
			seq= seq->next;
		}
	}

	draw_extra_seqinfo();

	/* restore viewport */
	winset(curarea->win);

	if(curarea->winx>SCROLLB+10 && curarea->winy>SCROLLH+10) {
		
		/* ortho op pixelnivo curarea */
		ortho2(-0.5, curarea->winx+0.5, -0.5, curarea->winy+0.5);
		
		if(G.v2d->scroll) {
			drawscroll(0);
		}
		
		
	}
	
	curarea->win_swap= 1;
}



