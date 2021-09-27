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

/* sequence.h    juni 95

 * 
 * 
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#define WHILE_SEQ(base)	{											\
							int totseq_, seq_; Sequence **seqar;	\
							build_seqar( base,  &seqar, &totseq_);	\
							for(seq_ = 0; seq_ < totseq_; seq_++) {	\
									seq= seqar[seq_];
								

#define END_SEQ					}						\
							if(seqar) freeN(seqar);		\
							}


typedef struct StripElem {
	char name[40];
	struct ImBuf *ibuf;
	short ok, nr;
	struct StripElem *se1, *se2, *se3;
	
} StripElem;

typedef struct Strip {
	struct Strip *next, *prev;
	short rt, len, us, done;
	short orx, ory;
	StripElem *stripdata;
	char dir[80];
	
} Strip;


typedef struct PluginSeq {
	char name[80];
	void *handle;
	
	char *pname;

	int stypes;
	char *stnames;
	
	int vars;
	void *varstr;
	float *result;
	
	float data[32];

	int (*doit)(void *, float, float, int, int, ImBuf *, ImBuf *, ImBuf *, ImBuf *);
	
} PluginSeq;


/* LET OP: eerste stuk identiek aan ID (ivm ipo's) */

typedef struct Sequence {

	struct Sequence *next, *prev, *new;
	void *lib;
	char name[24];
	
	short flag, type;
	int len;
	int start, startofs, endofs;
	int startstill, endstill;
	int machine, depth;
	int startdisp, enddisp;
	float mul, handsize;
	
	Strip *strip;
	StripElem *curelem;
	
	Ipo *ipo;
	Scene *scene;
	struct anim *anim;
	int sfra;
	float facf0, facf1;
	
	PluginSeq *plugin;

	/* pointers voor effecten: */
	struct Sequence *seq1, *seq2, *seq3;
	
	/* meta */
	ListBase seqbase;
	
} Sequence;


#
#
typedef struct MetaStack {
	struct MetaStack *next, *prev;
	ListBase *oldbasep;
	Sequence *parseq;
} MetaStack;

typedef struct Editing {
	ListBase *seqbasep;
	ListBase seqbase;
	ListBase metastack;
	short flag, rt;
	
} Editing;

	/* editseq.c */
extern Sequence *find_nearest_seq(int *hand);

	/* sequence.c */
extern int evaluate_seq_frame(int cfra);
extern ImBuf *give_ibuf_seq(int cfra);







#endif /* SEQUENCE_H */

