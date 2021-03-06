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

/*  text.c      MIXED MODEL

 * 
 *  april 99
 *  
 *  
 *  
 */


/***************/ /*

How Texts should work
--
A text should relate to a file as follows -
(Text *)->name should be the place where the 
	file will or has been saved.
	
(Text *)->flags has the following bits
	TXT_ISDIRTY - should always be set if the file in mem. differs from
					the file on disk, or if there is no file on disk.
	TXT_ISTMP - should always be set if the (Text *)->name file has not
					been written before, and attempts to save should result
					in "Save over?"
	TXT_ISMEM - should always be set if the Text has not been mapped to
					a file, in which case (Text *)->name may be NULL or garbage.			
	TXT_ISEXT - should always be set if the Text is not to be written into
					the .blend
	TXT_ISSCRIPT - should be set if the user has designated the text
					as a script.

Display
--
The st->top determines at what line the top of the text is displayed.
If the user moves the cursor the st containing that cursor should
be popped ... other st's retain their own top location.

*/ /***************/


/****************/ /*
	Undo

Undo/Redo works by storing
events in a queue, and a pointer
to the current position in the
queue...

Events are stored using an
arbitrary op-code system
to keep track of
a) the two cursors (normal and selected)
b) input (visible and control (ie backspace))

input data is stored as its
ASCII value, the opcodes are
then selected to not conflict.

opcodes with data in between are
written at the beginning and end
of the data to allow undo and redo
to simply check the code at the current
undo position

*/ /***************/

#include <ctype.h> /* isprint() */
#include "blender.h"
#include "graphics.h"
#include "text.h"

char *txt_cut_buffer= NULL;

static unsigned char undoing;

void free_text(Text *text)
{
	ScrArea *area;
	SpaceText *st;
	TextLine *tmp;
	
	if (!text) return;
	
	area= G.curscreen->areabase.first;
	while(area) {
		st= area->spacedata.first;
		while(st) {
			if(st->spacetype==SPACE_TEXT) {
				if(st->text == text) {					
					st->text= NULL;
					st->top= 0;
					
					addqueue(area->win, REDRAW, 1);
					addqueue(area->headwin, REDRAW, 1);
				}
			}
			st= st->next;
		}
		area= area->next;
	}
	
	tmp= text->lines.first;
	while (tmp) {
		freeN(tmp->line);
		tmp= tmp->next;		
	}
	
	freelistN(&text->lines);

	if(text->name) freeN(text->name);
	freeN(text->undo_buf);
}

void add_text_fs(char *file) {
	SpaceText *st= curarea->spacedata.first;
	ID *id;
	Text *text;

	if (!st) return;
	if (st->spacetype != SPACE_TEXT) return;

	id= (ID *)st->text;
	
	text= add_text(file);

	st->text= text;

	st->top= 0;
			
	allqueue(REDRAWTEXT, 0);
	allqueue(REDRAWHEADERS, 0);	
}

Text *add_empty_text(void) 
{
	Text *ta;
	TextLine *tmp;
	
	ta= alloc_libblock(&G.main->text, ID_TXT, "Text");
	ta->id.us= 1;
	
	ta->name= NULL;

	ta->undo_pos= -1;
	ta->undo_len= TXT_INIT_UNDO;
	ta->undo_buf= mallocN(ta->undo_len, "undo buf");
		
	ta->nlines=1;
	ta->flags= TXT_ISDIRTY | TXT_ISTMP | TXT_ISMEM;

	ta->lines.first= ta->lines.last= NULL;

	tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
	tmp->line= (char*) mallocN(1, "textline_string");

	tmp->line[0]=0;
	tmp->len= 0;
				
	tmp->next= NULL;
	tmp->prev= NULL;
				
	addhead(&ta->lines, tmp);
	
	ta->curl= ta->lines.first;
	ta->curc= 0;
	ta->sell= ta->lines.first;
	ta->selc= 0;

	return ta;
}

void reopen_text(Text *text)
{
	FILE *fp;
	int i, llen, len;
	unsigned char *buffer;
	TextLine *tmp;
	char sdir[FILE_MAXDIR];
	char sfile[FILE_MAXFILE];

	if (!text || !text->name) return;
	
	split_dirfile(text->name, sdir, sfile);
	
	text->lines.first= text->lines.last= NULL;
	text->curl= text->sell= NULL;

	fp= fopen(text->name, "r");
	if(fp==NULL) return;
	
	text->flags= TXT_ISTMP | TXT_ISEXT;
	
	fseek(fp, 0L, SEEK_END);
	len= ftell(fp);
	fseek(fp, 0L, SEEK_SET);	

	text->undo_pos= -1;
	
	buffer= mallocN(len, "text_buffer");
	fread(buffer, 1, len, fp);		
	fclose(fp);
	
	text->nlines=0;
	i=0;
	llen=0;
	for(i=0; i<len; i++) {
		if (buffer[i]=='\n') {
			tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
			tmp->line= (char*) mallocN(llen+1, "textline_string");
				
			if(llen) memcpy(tmp->line, &buffer[i-llen], llen);
			tmp->line[llen]=0;
			tmp->len= llen;
				
			addtail(&text->lines, tmp);
				
			text->nlines++;
				
			llen=0;
			continue;
		}
		llen++;
	}

	if (llen!=0 || text->nlines==0) {
		tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
		tmp->line= (char*) mallocN(llen+1, "textline_string");
		
		if(llen) memcpy(tmp->line, &buffer[i-llen], llen);

		tmp->line[llen]=0;
		tmp->len= llen;
				
		addtail(&text->lines, tmp);
		
		text->nlines++;
	}
	
	text->curl= text->sell= text->lines.first;
	text->curc= text->selc= 0;
	
	freeN(buffer);	
}

Text *add_text(char *file) 
{
	FILE *fp;
	int i, llen, len;
	unsigned char *buffer;
	TextLine *tmp;
	Text *ta;
	char sdir[FILE_MAXDIR];
	char sfile[FILE_MAXFILE];

	split_dirfile(file, sdir, sfile);
	
	fp= fopen(file, "r");
	if(fp==NULL) return NULL;
	
	ta= alloc_libblock(&G.main->text, ID_TXT, sfile);
	ta->id.us= 1;

	ta->lines.first= ta->lines.last= NULL;
	ta->curl= ta->sell= NULL;

/* 	ta->flags= TXT_ISTMP | TXT_ISEXT; */
	ta->flags= TXT_ISTMP;
	
	fseek(fp, 0L, SEEK_END);
	len= ftell(fp);
	fseek(fp, 0L, SEEK_SET);	

	ta->name= mallocN(strlen(file)+1, "text_name");
	strcpy(ta->name, file);

	ta->undo_pos= -1;
	ta->undo_len= TXT_INIT_UNDO;
	ta->undo_buf= mallocN(ta->undo_len, "undo buf");
	
	buffer= mallocN(len, "text_buffer");
	fread(buffer, 1, len, fp);		
	fclose(fp);
	
	ta->nlines=0;
	i=0;
	llen=0;
	for(i=0; i<len; i++) {
		if (buffer[i]=='\n') {
			tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
			tmp->line= (char*) mallocN(llen+1, "textline_string");
				
			if(llen) memcpy(tmp->line, &buffer[i-llen], llen);
			tmp->line[llen]=0;
			tmp->len= llen;
				
			addtail(&ta->lines, tmp);
				
			ta->nlines++;
				
			llen=0;
			continue;
		}
		llen++;
	}

	if (llen!=0 || ta->nlines==0) {
		tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
		tmp->line= (char*) mallocN(llen+1, "textline_string");
		
		if(llen) memcpy(tmp->line, &buffer[i-llen], llen);

		tmp->line[llen]=0;
		tmp->len= llen;
				
		addtail(&ta->lines, tmp);
		
		ta->nlines++;
	}
	
	ta->curl= ta->sell= ta->lines.first;
	ta->curc= ta->selc= 0;
	
	freeN(buffer);	

	return ta;
}

Text *copy_text(Text *ta)
{
	Text *tan;
	TextLine *line, *tmp;
	
	tan= copy_libblock(ta);
	
	tan->name= mallocN(strlen(ta->name)+1, "text_name");
	strcpy(tan->name, ta->name);
	
	tan->flags = ta->flags | TXT_ISDIRTY | TXT_ISTMP;
	
	tan->lines.first= tan->lines.last= NULL;
	tan->curl= tan->sell= NULL;
	
	tan->nlines= ta->nlines;

	line= ta->lines.first;	
	/* Walk down, reconstructing */
	while (line) {
		tmp= (TextLine*) mallocN(sizeof(TextLine), "textline");
		tmp->line= mallocN(line->len+1, "textline_string");
		strcpy(tmp->line, line->line);
		
		tmp->len= line->len;
		
		addtail(&tan->lines, tmp);
		
		line= line->next;
	}

	tan->curl= tan->sell= tan->lines.first;
	tan->curc= tan->selc= 0;

	return tan;
}

void init_textspace(ScrArea *sa)
{
	SpaceText *st;
	
	st= callocN(sizeof(SpaceText), "inittextspace");
	addhead(&sa->spacedata, st);

	set_func_space(sa);
	
	st->spacetype= SPACE_TEXT;	
	
	st->text= NULL;
	st->flags= 0;
	
	st->font_id= 5;
	st->lheight= 16;
	
	st->top= 0;
}

void check_text_copy(SpaceText *st)
{
	if (!st) return;
}

void free_textspace(SpaceText *st)
{
	if (!st) return;

	st->text= NULL;
}

/*****************************/
/* Editing utility functions */
/*****************************/

static void make_new_line (TextLine *line, char *newline) 
{
	freeN(line->line);
	
	line->line= newline;
	line->len= strlen(newline);	
}

TextLine *txt_new_line(char *str)
{
	TextLine *tmp;

	if(!str) str= "";
	
	tmp= (TextLine *) mallocN(sizeof(TextLine), "textline");
	tmp->line= mallocN(strlen(str)+1, "textline_string");
	strcpy(tmp->line, str);
	
	tmp->len= strlen(str);
	tmp->next= tmp->prev= NULL;
	
	return tmp;
}

TextLine *txt_new_linen(char *str, int n)
{
	TextLine *tmp;

	if(!str) str= "";
	
	tmp= (TextLine *) mallocN(sizeof(TextLine), "textline");
	tmp->line= mallocN(n+1, "textline_string");
	strncpy(tmp->line, str, n);
	tmp->line[n]=0;
	
	tmp->len= strlen(tmp->line);
	tmp->next= tmp->prev= NULL;
	
	return tmp;
}

void txt_clean_text (Text *text) 
{	
	TextLine *tmp, **top, **bot;
	
	if (!text) return;
	
	if (!text->lines.first) {
		if (text->lines.last) text->lines.first= text->lines.last;
		else text->lines.first= text->lines.last= txt_new_line(NULL);
	} 
	
	if (!text->lines.last) text->lines.last= text->lines.first;

	top= (TextLine **) &text->lines.first;
	bot= (TextLine **) &text->lines.last;
	
	while ((*top)->prev) *top= (*top)->prev;
	while ((*bot)->next) *bot= (*bot)->next;

	if(!text->curl) {
		if(text->sell) text->curl= text->sell;
		else text->curl= text->lines.first;
		text->curc= 0;
	}

	if(!text->sell) {
		text->sell= text->curl;
		text->selc= 0;
	}
}

int txt_get_span (TextLine *from, TextLine *to)
{
	int ret=0;
	TextLine *tmp= from;

	if (!to || !from) return 0;
	if (from==to) return 0;

	/* Look forwards */
	while (tmp) {
		if (tmp == to) return ret;
		ret++;
		tmp= tmp->next;
	}

	/* Look backwards */
	if (!tmp) {
		tmp= from;
		ret=0;
		while(tmp) {
			if (tmp == to) break;
			ret--;
			tmp= tmp->prev;
		}
		if(!tmp) ret=0;
	}

	return ret;	
}

int txt_illegal_char (char c)
{
	if (isprint(c) || c=='\t') return 0;
	
	return 1;
}

void txt_make_dirty (Text *text)
{
	text->flags != TXT_ISDIRTY;
	
	if (text->compiled) {
		text->compiled= NULL;
	}
}

/****************************/
/* Cursor utility functions */
/****************************/

void txt_curs_cur (Text *text, TextLine ***linep, int **charp)
{
	*linep= &text->curl; *charp= &text->curc;
}

void txt_curs_sel (Text *text, TextLine ***linep, int **charp)
{
	*linep= &text->sell; *charp= &text->selc;
}

void txt_curs_first (Text *text, TextLine **linep, int *charp)
{
	if (text->curl==text->sell) {
		*linep= text->curl;
		if (text->curc<text->selc) *charp= text->curc;
		else *charp= text->selc;
	} else if (txt_get_span(text->lines.first, text->curl)<txt_get_span(text->lines.first, text->sell)) {
		*linep= text->curl;
		*charp= text->curc;
	} else {
		*linep= text->sell;
		*charp= text->selc;		
	}
}

/****************************/
/* Cursor movement functions */
/****************************/

void txt_move_up(Text *text, short sel)
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else { txt_pop_first(text); txt_curs_cur(text, &linep, &charp); }
	if (!*linep) return;
	old= *charp;

	if((*linep)->prev) {
		*linep= (*linep)->prev;
		if (*charp > (*linep)->len) {
			*charp= (*linep)->len;
			if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, (*linep)->next), old, txt_get_span(text->lines.first, *linep), *charp);
		} else {
			if(!undoing) txt_undo_add_op(text, sel?UNDO_SUP:UNDO_CUP);
		}
	} else {
		*charp= 0;
		if(!undoing) txt_undo_add_op(text, sel?UNDO_SUP:UNDO_CUP);
	}

	if(!sel) txt_pop_sel(text);
}

void txt_move_down(Text *text, short sel) 
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else { txt_pop_last(text); txt_curs_cur(text, &linep, &charp); }
	if (!*linep) return;
	old= *charp;

	if((*linep)->next) {
		*linep= (*linep)->next;
		if (*charp > (*linep)->len) {
			*charp= (*linep)->len;
			if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, (*linep)->prev), old, txt_get_span(text->lines.first, *linep), *charp);
		} else
			if(!undoing) txt_undo_add_op(text, sel?UNDO_SDOWN:UNDO_CDOWN);	
	} else {
		*charp= (*linep)->len;
		if(!undoing) txt_undo_add_op(text, sel?UNDO_SDOWN:UNDO_CDOWN);
	}

	if(!sel) txt_pop_sel(text);
}

void txt_move_left(Text *text, short sel) 
{
	TextLine **linep;
	int *charp, oundoing= undoing;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else { txt_pop_first(text); txt_curs_cur(text, &linep, &charp); }
	if (!*linep) return;

	undoing= 1;
	if (*charp== 0) {
		if ((*linep)->prev) {
			txt_move_up(text, sel);
			*charp= (*linep)->len;
		}
	} else {
		(*charp)--;
	}
	undoing= oundoing;
	if(!undoing) txt_undo_add_op(text, sel?UNDO_SLEFT:UNDO_CLEFT);
	
	if(!sel) txt_pop_sel(text);
}

void txt_move_right(Text *text, short sel) 
{
	TextLine **linep;
	int *charp, oundoing= undoing;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else { txt_pop_last(text); txt_curs_cur(text, &linep, &charp); }
	if (!*linep) return;

	undoing= 1;
	if (*charp== (*linep)->len) {
		if ((*linep)->next) {
			txt_move_down(text, sel);
			*charp= 0;
		}
	} else {
		(*charp)++;
	}
	undoing= oundoing;
	if(!undoing) txt_undo_add_op(text, sel?UNDO_SRIGHT:UNDO_CRIGHT);

	if(!sel) txt_pop_sel(text);
}

void txt_move_bol (Text *text, short sel) 
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else txt_curs_cur(text, &linep, &charp);
	if (!*linep) return;
	old= *charp;
	
	*charp= 0;

	if(!sel) txt_pop_sel(text);
	if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, *linep), old, txt_get_span(text->lines.first, *linep), *charp);
}

void txt_move_eol (Text *text, short sel) 
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else txt_curs_cur(text, &linep, &charp);
	if (!*linep) return;
	old= *charp;
		
	*charp= (*linep)->len;

	if(!sel) txt_pop_sel(text);
	if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, *linep), old, txt_get_span(text->lines.first, *linep), *charp);
}

void txt_move_bof (Text *text, short sel)
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else txt_curs_cur(text, &linep, &charp);
	if (!*linep) return;
	old= *charp;

	*linep= text->lines.first;
	*charp= 0;

	if(!sel) txt_pop_sel(text);
	if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, *linep), old, txt_get_span(text->lines.first, *linep), *charp);
}

void txt_move_eof (Text *text, short sel)
{
	TextLine **linep;
	int *charp, old;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else txt_curs_cur(text, &linep, &charp);
	if (!*linep) return;
	old= *charp;

	*linep= text->lines.last;
	*charp= (*linep)->len;

	if(!sel) txt_pop_sel(text);
	if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, *linep), old, txt_get_span(text->lines.first, *linep), *charp);	
}

void txt_move_toline (Text *text, unsigned int line, short sel)
{
	TextLine **linep, *oldl;
	int *charp, oldc;
	unsigned int i;
	
	if (!text) return;
	if(sel) txt_curs_sel(text, &linep, &charp);
	else txt_curs_cur(text, &linep, &charp);
	if (!*linep) return;
	oldc= *charp;
	oldl= *linep;
	
	*linep= text->lines.first;
	for (i=0; i<line; i++) {
		if ((*linep)->next) *linep= (*linep)->next;
		else break;
	}
	*charp= 0;
	
	if(!sel) txt_pop_sel(text);
	if(!undoing) txt_undo_add_toop(text, sel?UNDO_STO:UNDO_CTO, txt_get_span(text->lines.first, oldl), oldc, txt_get_span(text->lines.first, *linep), *charp);	
}

/****************************/
/* Text selection functions */
/****************************/

void txt_curs_swap (Text *text)
{
	TextLine *tmpl;
	int tmpc;
		
	tmpl= text->curl;
	text->curl= text->sell;
	text->sell= tmpl;
	
	tmpc= text->curc;
	text->curc= text->selc;
	text->selc= tmpc;
	
	if(!undoing) txt_undo_add_op(text, UNDO_SWAP);
}

void txt_pop_first (Text *text)
{
	TextLine *tmpl;
	int tmpc;
			
	if (txt_get_span(text->curl, text->sell)<0 ||
		(text->curl==text->sell && text->curc>text->selc)) {	
		txt_curs_swap(text);
	}

	if(!undoing) txt_undo_add_toop(text, UNDO_STO,
		txt_get_span(text->lines.first, text->sell), 
		text->selc, 
		txt_get_span(text->lines.first, text->curl), 
		text->curc);		
	
	txt_pop_sel(text);
}

void txt_pop_last (Text *text)
{
	if (txt_get_span(text->curl, text->sell)>0 ||
		(text->curl==text->sell && text->curc<text->selc)) {
		txt_curs_swap(text);
	}

	if(!undoing) txt_undo_add_toop(text, UNDO_STO,
		txt_get_span(text->lines.first, text->sell), 
		text->selc, 
		txt_get_span(text->lines.first, text->curl), 
		text->curc);		
	
	txt_pop_sel(text);
}

void txt_pop_selr (Text *text)
{
	txt_undo_add_toop(text, UNDO_STO,
		txt_get_span(text->lines.first, text->sell), 
		text->selc, 
		txt_get_span(text->lines.first, text->curl), 
		text->curc);

	text->sell= text->curl;
	text->selc= text->curc;	
}

void txt_pop_sel (Text *text)
{
	text->sell= text->curl;
	text->selc= text->curc;	
}

void txt_delete_sel (Text *text)
{
	TextLine *tmpl;
	int tmpc;
	char *buf;
	
	if (!text) return;
	if (!text->curl) return;
	if (!text->sell) return;
	
	if (text->curl == text->sell && text->curc==text->selc) return;

		/* Flip so text->curl is before text->sell */
	if (txt_get_span(text->curl, text->sell)<0 ||
		(text->curl==text->sell && text->curc>text->selc)) {	
		txt_curs_swap(text);
	}

	if(!undoing) {
		buf= txt_sel_to_buf(text);
		txt_undo_add_block(text, UNDO_DBLOCK, buf);
		freeN(buf);
	}

	buf= mallocN(text->curc+(text->sell->len - text->selc)+1, "textline_string");
	
	strncpy(buf, text->curl->line, text->curc);
	strcpy(buf+text->curc, text->sell->line + text->selc);
	buf[text->curc+(text->sell->len - text->selc)]=0;
	
	make_new_line(text->curl, buf);
	
	tmpl= text->sell;
	while (tmpl != text->curl) {
		tmpl= tmpl->prev;
		if (!tmpl) break;
		
		txt_delete_line(text, tmpl->next);
	}
	
	text->sell= text->curl;
	text->selc= text->curc;
}

void txt_sel_all (Text *text)
{
	if (!text) return;

	text->curl= text->lines.first;
	text->curc= 0;
	
	text->sell= text->lines.last;
	text->selc= text->sell->len;
}

void txt_sel_line (Text *text)
{
	if (!text) return;
	if (!text->curl) return;
	
	text->curc= 0;
	text->sell= text->curl;
	text->selc= text->sell->len;
}

void txt_sel_par (Text *text)
{
	if (!text) return;
	
	error ("WRITE IT!!!!!!!!\n");
}

/***************************/
/* Cut and paste functions */
/***************************/

void txt_print_cutbuffer (void) 
{
	printf ("Cut buffer\n--\n%s\n--\n", txt_cut_buffer);	
}

char *txt_to_buf (Text *text)
{
	int length=0;
	TextLine *tmp, *linef, *linel;
	int charf, charl;
	char *buf;
	
	if (!text) return NULL;
	if (!text->curl) return NULL;
	if (!text->sell) return NULL;
		
	linef= text->lines.first;
	charf= 0;
		
	linel= text->lines.last;
	charl= linel->len;

	if (linef == text->lines.last) {
		length= charl-charf;

		buf= mallocN(length+2, "text buffer");
		
		strncpy(buf, linef->line + charf, length);
		buf[length]=0;
	} else {
		length+= linef->len - charf;
		length+= charl;
		length++; /* For the '\n' */
		
		tmp= linef->next;
		while (tmp && tmp!= linel) {
			length+= tmp->len+1;
			tmp= tmp->next;
		}
		
		buf= mallocN(length+2, "cut buffer");
		
		strncpy(buf, linef->line+ charf, linef->len-charf);
		length= linef->len-charf;
		
		buf[length++]='\n';
		
		tmp= linef->next;
		while (tmp && tmp!=linel) {
			strncpy(buf+length, tmp->line, tmp->len);
			length+= tmp->len;
			
			buf[length++]='\n';			
			
			tmp= tmp->next;
		}
		strncpy(buf+length, linel->line, charl);
		length+= charl;
		
		buf[length]=0;
	}
	
	return buf;
}

void txt_cut_sel (Text *text)
{
	txt_copy_sel(text);
	
	txt_delete_sel(text);
}

char *txt_sel_to_buf (Text *text)
{
	char *buf;
	int length=0;
	TextLine *tmp, *linef, *linel;
	int charf, charl;
	
	if (!text) return NULL;
	if (!text->curl) return NULL;
	if (!text->sell) return NULL;
	
	if (text->curl==text->sell) {
		linef= linel= text->curl;
		
		if (text->curc < text->selc) {
			charf= text->curc;
			charl= text->selc;
		} else{
			charf= text->selc;
			charl= text->curc;
		}
	} else if (txt_get_span(text->curl, text->sell)<0) {
		linef= text->sell;
		linel= text->curl;

		charf= text->selc;		
		charl= text->curc;
	} else {
		linef= text->curl;
		linel= text->sell;
		
		charf= text->curc;
		charl= text->selc;
	}

	if (linef == linel) {
		length= charl-charf;

		buf= mallocN(length+1, "sel buffer");
		
		strncpy(buf, linef->line + charf, length);
		buf[length]=0;
	} else {
		length+= linef->len - charf;
		length+= charl;
		length++; /* For the '\n' */
		
		tmp= linef->next;
		while (tmp && tmp!= linel) {
			length+= tmp->len+1;
			tmp= tmp->next;
		}
		
		buf= mallocN(length+1, "sel buffer");
		
		strncpy(buf, linef->line+ charf, linef->len-charf);
		length= linef->len-charf;
		
		buf[length++]='\n';
		
		tmp= linef->next;
		while (tmp && tmp!=linel) {
			strncpy(buf+length, tmp->line, tmp->len);
			length+= tmp->len;
			
			buf[length++]='\n';			
			
			tmp= tmp->next;
		}
		strncpy(buf+length, linel->line, charl);
		length+= charl;
		
		buf[length]=0;
	}	

	return buf;
}

void txt_copy_sel (Text *text)
{
	int length=0;
	TextLine *tmp, *linef, *linel;
	int charf, charl;
	
	if (!text) return;
	if (!text->curl) return;
	if (!text->sell) return;
	
	if (txt_cut_buffer) freeN(txt_cut_buffer);
	txt_cut_buffer= NULL;
	
	if (text->curl==text->sell) {
		linef= linel= text->curl;
		
		if (text->curc < text->selc) {
			charf= text->curc;
			charl= text->selc;
		} else{
			charf= text->selc;
			charl= text->curc;
		}
	} else if (txt_get_span(text->curl, text->sell)<0) {
		linef= text->sell;
		linel= text->curl;

		charf= text->selc;		
		charl= text->curc;
	} else {
		linef= text->curl;
		linel= text->sell;
		
		charf= text->curc;
		charl= text->selc;
	}

	if (linef == linel) {
		length= charl-charf;

		txt_cut_buffer= mallocN(length+1, "cut buffera");
		
		strncpy(txt_cut_buffer, linef->line + charf, length);
		txt_cut_buffer[length]=0;
	} else {
		length+= linef->len - charf;
		length+= charl;
		length++; /* For the '\n' */
		
		tmp= linef->next;
		while (tmp && tmp!= linel) {
			length+= tmp->len+1;
			tmp= tmp->next;
		}
		
		txt_cut_buffer= mallocN(length+1, "cut bufferb");
		
		strncpy(txt_cut_buffer, linef->line+ charf, linef->len-charf);
		length= linef->len-charf;
		
		txt_cut_buffer[length++]='\n';
		
		tmp= linef->next;
		while (tmp && tmp!=linel) {
			strncpy(txt_cut_buffer+length, tmp->line, tmp->len);
			length+= tmp->len;
			
			txt_cut_buffer[length++]='\n';			
			
			tmp= tmp->next;
		}
		strncpy(txt_cut_buffer+length, linel->line, charl);
		length+= charl;
		
		txt_cut_buffer[length]=0;
	}
}

void txt_paste_buf (Text *text, char *in_buffer)
{
	int i=0, l=0, j, u, len;
	TextLine *add;
	char *buf;

	if (!text) return;
	if (!in_buffer) return;

	txt_delete_sel(text);
	
	if(!undoing) txt_undo_add_block (text, UNDO_IBLOCK, in_buffer); 	

	u= undoing;
	undoing= 1;

	/* Read the first line (or as close as possible */
	while (in_buffer[i] && in_buffer[i]!='\n') {
		txt_add_char(text, in_buffer[i]);
		i++;
	}
	
	if (in_buffer[i]=='\n') txt_split_curline(text);
	else { undoing = u; return; }
	i++;

	/* Read as many full lines as we can */
	len= strlen(in_buffer);

	while (i<len) {
		l=0;

		while (in_buffer[i] && in_buffer[i]!='\n') {
			i++; l++;
		}
	
		if(in_buffer[i]=='\n') {
			add= txt_new_linen(in_buffer +(i-l), l);
			insertlinkbefore(&text->lines, text->curl, add);
			i++;
		} else {
			for (j= i-l; j<i && j<strlen(in_buffer); j++) {
				txt_add_char(text, in_buffer[j]);
			}
			break;
		}
	}
	
	undoing= u;
}

/******************/
/* Undo functions */
/******************/

#define MAX_UNDO_TEST(x) \
	while (text->undo_pos+x >= text->undo_len) { \
		if(text->undo_len*2 > TXT_MAX_UNDO) { \
			error("Undo limit reached, buffer cleared\n"); \
			freeN(text->undo_buf); \
			text->undo_len= TXT_INIT_UNDO; \
			text->undo_buf= mallocN(text->undo_len, "undo buf"); \
			text->undo_pos=-1; \
			return; \
		} else { \
			void *tmp= text->undo_buf; \
			text->undo_buf= callocN(text->undo_len*2, "undo buf"); \
			memcpy(text->undo_buf, tmp, text->undo_len); \
			text->undo_len*=2; \
			freeN(tmp); \
		} \
	}

static void dump_buffer(Text *text) {
	int i= 0;
	
	while (i++<text->undo_pos) printf("%d: %d %c\n", i, text->undo_buf[i], text->undo_buf[i]);
}

void txt_print_undo(Text *text)
{
	int i= 0;
	int op;
	char *ops;
	int linep, charp;
	
	dump_buffer(text);
	
	printf ("---< Undo Buffer >---\n");
	
	printf ("UndoPosition is %d\n", text->undo_pos);
	
	while (i<=text->undo_pos) {
		op= text->undo_buf[i];
		
		if (op==UNDO_CLEFT) {
			ops= "Cursor left";
		} else if (op==UNDO_CRIGHT) {
			ops= "Cursor right";
		} else if (op==UNDO_CUP) {
			ops= "Cursor up";
		} else if (op==UNDO_CDOWN) {
			ops= "Cursor down";
		} else if (op==UNDO_SLEFT) {
			ops= "Selection left";
		} else if (op==UNDO_SRIGHT) {
			ops= "Selection right";
		} else if (op==UNDO_SUP) {
			ops= "Selection up";
		} else if (op==UNDO_SDOWN) {
			ops= "Selection down";
		} else if (op==UNDO_STO) {
			ops= "Selection ";
		} else if (op==UNDO_CTO) {
			ops= "Cursor ";
		} else if (op==UNDO_INSERT) {
			ops= "Insert";
		} else if (op==UNDO_BS) {
			ops= "Backspace";
		} else if (op==UNDO_DEL) {
			ops= "Delete";
		} else if (op==UNDO_SWAP) {
			ops= "Cursor swap";
		} else if (op==UNDO_DBLOCK) {
			ops= "Delete text block";
		} else if (op==UNDO_IBLOCK) {
			ops= "Insert text block";
		} else {
			ops= "Unknown";
		}
		
		printf ("Op (%o) at %d = %s", op, i, ops);
		if (op==UNDO_INSERT || op==UNDO_BS || op==UNDO_DEL) {
			i++;
			printf (" - Char is %c", text->undo_buf[i]);  
			i++;
		} else if (op==UNDO_STO || op==UNDO_CTO) {
			i++;

			charp= text->undo_buf[i]; i++;
			charp= charp+(text->undo_buf[i]<<8); i++;

			linep= text->undo_buf[i]; i++;
			linep= linep+(text->undo_buf[i]<<8); i++;
			linep= linep+(text->undo_buf[i]<<16); i++;
			linep= linep+(text->undo_buf[i]<<24); i++;
			
			printf ("to <%d, %d> ", linep, charp);

			charp= text->undo_buf[i]; i++;
			charp= charp+(text->undo_buf[i]<<8); i++;

			linep= text->undo_buf[i]; i++;
			linep= linep+(text->undo_buf[i]<<8); i++;
			linep= linep+(text->undo_buf[i]<<16); i++;
			linep= linep+(text->undo_buf[i]<<24); i++;
			
			printf ("from <%d, %d>", linep, charp);
		} else if (op==UNDO_DBLOCK || op==UNDO_IBLOCK) {
			i++;

			linep= text->undo_buf[i]; i++;
			linep= linep+(text->undo_buf[i]<<8); i++;
			linep= linep+(text->undo_buf[i]<<16); i++;
			linep= linep+(text->undo_buf[i]<<24); i++;
			
			printf (" (length %d) <", linep);
			
			while (linep>0) {
				putchar(text->undo_buf[i]);
				linep--; i++;
			}
			
			linep= text->undo_buf[i]; i++;
			linep= linep+(text->undo_buf[i]<<8); i++;
			linep= linep+(text->undo_buf[i]<<16); i++;
			linep= linep+(text->undo_buf[i]<<24); i++;
			printf ("> (%d)", linep);
		}
		
		printf (" %d\n",  i);
		i++;
	}
}

void txt_undo_add_op(Text *text, int op)
{
	MAX_UNDO_TEST(2);
	
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;
	text->undo_buf[text->undo_pos+1]= 0;
}

void txt_undo_add_block(Text *text, int op, char *buf)
{
	unsigned int length;
	
	length= strlen(buf);
	
	MAX_UNDO_TEST(length+11);

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;
	
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>8)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>16)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>24)&0xff;

	text->undo_pos++;
	strncpy(text->undo_buf+text->undo_pos, buf, length);
	text->undo_pos+=length;

	text->undo_buf[text->undo_pos]= (length)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>8)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>16)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (length>>24)&0xff;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;
	
	text->undo_buf[text->undo_pos+1]= 0;
}

void txt_undo_add_toop(Text *text, int op, unsigned int froml, unsigned short fromc, unsigned int tol, unsigned short toc)
{
	MAX_UNDO_TEST(15);

	if (froml==tol && fromc==toc) return;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (fromc)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (fromc>>8)&0xff;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (froml)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (froml>>8)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (froml>>16)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (froml>>24)&0xff;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (toc)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (toc>>8)&0xff;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (tol)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (tol>>8)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (tol>>16)&0xff;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= (tol>>24)&0xff;

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;

	text->undo_buf[text->undo_pos+1]= 0;
}

void txt_undo_add_charop(Text *text, int op, char c)
{
	MAX_UNDO_TEST(4);

	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= c;
	text->undo_pos++;
	text->undo_buf[text->undo_pos]= op;
	text->undo_buf[text->undo_pos+1]= 0;
}

void txt_do_undo(Text *text)
{
	int op= text->undo_buf[text->undo_pos], i;
	unsigned int linep;
	unsigned short charp;
	TextLine *holdl;
	int holdc, holdln;
	char *buf;
	
	if (text->undo_pos<0) {
		return;
	}

	text->undo_pos--;

	undoing= 1;
	
	switch(op) {
		case UNDO_CLEFT:
			txt_move_right(text, 0);
			break;
			
		case UNDO_CRIGHT:
			txt_move_left(text, 0);
			break;
			
		case UNDO_CUP:
			txt_move_down(text, 0);
			break;
			
		case UNDO_CDOWN:
			txt_move_up(text, 0);
			break;

		case UNDO_SLEFT:
			txt_move_right(text, 1);
			break;

		case UNDO_SRIGHT:
			txt_move_left(text, 1);
			break;

		case UNDO_SUP:
			txt_move_down(text, 1);
			break;

		case UNDO_SDOWN:
			txt_move_up(text, 1);
			break;
		
		case UNDO_CTO:
		case UNDO_STO:
			text->undo_pos--;
			text->undo_pos--;
			text->undo_pos--;
			text->undo_pos--;
		
			text->undo_pos--;
			text->undo_pos--;
		
			linep= text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;

			charp= text->undo_buf[text->undo_pos]; text->undo_pos--;
			charp= (charp<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			
			if (op==UNDO_CTO) {
				txt_move_toline(text, linep, 0);
				text->curc= charp;
				txt_pop_sel(text);
			} else {
				txt_move_toline(text, linep, 1);
				text->selc= charp;
			}
			
			text->undo_pos--;
			break;
			
		case UNDO_INSERT:
			txt_backspace_char(text);
			text->undo_pos--;
			text->undo_pos--;
			break;

		case UNDO_BS:
			txt_add_char(text, text->undo_buf[text->undo_pos]);
			text->undo_pos--;
			text->undo_pos--;
			break;

		case UNDO_DEL:
			txt_add_char(text, text->undo_buf[text->undo_pos]);
			txt_move_left(text, 0);
			text->undo_pos--;
			text->undo_pos--;
			break;

		case UNDO_SWAP:
			txt_curs_swap(text);
			txt_do_undo(text); /* swaps should appear transparent */
			break;

		case UNDO_DBLOCK:
			linep= text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;

			buf= mallocN(linep+1, "dblock buffer");
			for (i=0; i < linep; i++){
				buf[(linep-1)-i]= text->undo_buf[text->undo_pos]; 
				text->undo_pos--;
			}
			buf[i]= 0;
			
			txt_curs_first(text, &holdl, &holdc);
			holdln= txt_get_span(text->lines.first, holdl);
			
			txt_paste_buf(text, buf);			
			freeN(buf);

			text->curl= text->lines.first;
			while (holdln>0) {
				if(text->curl->next)
					text->curl= text->curl->next;
					
				holdln--;
			}
			text->curc= holdc;

			linep= text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;

			text->undo_pos--;
			
			break;

		case UNDO_IBLOCK:
			linep= text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;
			linep= (linep<<8)+text->undo_buf[text->undo_pos]; text->undo_pos--;

			txt_delete_sel(text);
			while (linep>0) {
				txt_backspace_char(text);
				text->undo_pos--;
				linep--;
			}

			text->undo_pos--;
			text->undo_pos--;
			text->undo_pos--; 
			text->undo_pos--;
			
			text->undo_pos--;

			break;
			
		default:
			error ("Undo buffer error - resetting");
			text->undo_pos= -1;
			
			break;
	}
	
	undoing= 0;	
}

void txt_do_redo(Text *text)
{
	char op;
	unsigned int linep;
	unsigned short charp;
	int i;
	char *buf;
	
	text->undo_pos++;	
	op= text->undo_buf[text->undo_pos];
	
	if (!op) {
		text->undo_pos--;
		return;
	}
	
	undoing= 1;
	
	switch(op) {
		case UNDO_CLEFT:
			txt_move_left(text, 0);
			break;
			
		case UNDO_CRIGHT:
			txt_move_right(text, 0);
			break;
			
		case UNDO_CUP:
			txt_move_up(text, 0);
			break;
			
		case UNDO_CDOWN:
			txt_move_down(text, 0);
			break;

		case UNDO_SLEFT:
			txt_move_left(text, 1);
			break;

		case UNDO_SRIGHT:
			txt_move_right(text, 1);
			break;

		case UNDO_SUP:
			txt_move_up(text, 1);
			break;

		case UNDO_SDOWN:
			txt_move_down(text, 1);
			break;
		
		case UNDO_INSERT:
			text->undo_pos++;
			txt_add_char(text, text->undo_buf[text->undo_pos]);
			text->undo_pos++;
			break;

		case UNDO_BS:
			text->undo_pos++;
			txt_backspace_char(text);
			text->undo_pos++;
			break;

		case UNDO_DEL:
			text->undo_pos++;
			txt_delete_char(text);
			text->undo_pos++;
			break;

		case UNDO_SWAP:
			txt_curs_swap(text);
			txt_do_undo(text); /* swaps should appear transparent a*/
			break;
			
		case UNDO_CTO:
		case UNDO_STO:
			text->undo_pos++;
			text->undo_pos++;

			text->undo_pos++;
			text->undo_pos++;
			text->undo_pos++;
			text->undo_pos++;

			text->undo_pos++;

			charp= text->undo_buf[text->undo_pos];
			text->undo_pos++;
			charp= charp+(text->undo_buf[text->undo_pos]<<8);

			text->undo_pos++;
			linep= text->undo_buf[text->undo_pos]; text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<8); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<16); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<24); text->undo_pos++;
			
			if (op==UNDO_CTO) {
				txt_move_toline(text, linep, 0);
				text->curc= charp;
				txt_pop_sel(text);
			} else {
				txt_move_toline(text, linep, 1);
				text->selc= charp;
			}

			break;

		case UNDO_DBLOCK:
			text->undo_pos++;
			linep= text->undo_buf[text->undo_pos]; text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<8); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<16); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<24); text->undo_pos++;

			txt_delete_sel(text);
			text->undo_pos+=linep;

			text->undo_pos++;
			text->undo_pos++;
			text->undo_pos++; 
			text->undo_pos++;
			
			break;

		case UNDO_IBLOCK:
			text->undo_pos++;
			linep= text->undo_buf[text->undo_pos]; text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<8); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<16); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<24); text->undo_pos++;

			buf= mallocN(linep+1, "iblock buffer");
			memcpy (buf, &text->undo_buf[text->undo_pos], linep);
			text->undo_pos+= linep;
			buf[linep]= 0;
			
			txt_paste_buf(text, buf);			
			freeN(buf);

			linep= text->undo_buf[text->undo_pos]; text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<8); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<16); text->undo_pos++;
			linep= linep+(text->undo_buf[text->undo_pos]<<24); text->undo_pos++;

			break;

		default:
			error ("Undo buffer error - resetting");
			text->undo_pos= -1;

			break;
	}
	
	undoing= 0;	
}

/***************************/
/* File handling functions */
/***************************/

void save_mem_text (char *str)
{
	SpaceText *st= curarea->spacedata.first;
	Text *text;
	
	if (!str) return;
	
	if (!st) return;
	if (st->spacetype != SPACE_TEXT) return;

	text= st->text;
	if(!text) return;
	
	if (text->name) freeN(text->name);
	text->name= mallocN(strlen(str)+1, "textname");
	strcpy(text->name, str);

	text->flags ^= TXT_ISMEM;
		
	txt_write_file (text);
}

void txt_write_file (Text *text) 
{
	FILE *fp;
	TextLine *tmp;
	
	/* Do we need to get a filename? */
	if (text->flags & TXT_ISMEM) {
		activate_fileselect(FILE_SPECIAL, "SAVE TEXT FILE", G.sce, save_mem_text);
		return;	
	}
	
	/* Should we ask to save over? */
	if (text->flags & TXT_ISTMP) {
		if (fop_exists(text->name)) {
			if (!okee("Save over?")) return;
		} else if (!okee("Create new file?")) return;

		text->flags ^= TXT_ISTMP;
	}
		
	fp= fopen(text->name, "w");
	if (fp==NULL) {
		error("Unable to save file");
		return;
	}

	tmp= text->lines.first;
	while (tmp) {
		if (tmp->next) fprintf(fp, "%s\n", tmp->line);
		else fprintf(fp, "%s", tmp->line);
		
		tmp= tmp->next;
	}
	
	fclose (fp);
	
	if (text->flags&TXT_ISDIRTY) text->flags ^= TXT_ISDIRTY;
}

/**************************/
/* Line editing functions */ 
/**************************/

void txt_split_curline (Text *text) {
	TextLine *ins;
	char *left, *right;
	
	if (!text) return;
	if (!text->curl) return;

	txt_delete_sel(text);	

	/* Make the two half strings */

	left= mallocN(text->curc+1, "textline_string");
	if (text->curc) memcpy(left, text->curl->line, text->curc);
	left[text->curc]=0;
	
	right= mallocN(text->curl->len - text->curc+1, "textline_string");
	if (text->curl->len - text->curc) memcpy(right, text->curl->line+text->curc, text->curl->len-text->curc);
	right[text->curl->len - text->curc]=0;

	freeN(text->curl->line);

	/* Make the new TextLine */
	
	ins= mallocN(sizeof(TextLine), "textline");
	ins->line= left;
	ins->len= text->curc;
	
	text->curl->line= right;
	text->curl->len= text->curl->len - text->curc;
	
	insertlinkbefore(&text->lines, text->curl, ins);	
	
	text->curc=0;
	
	txt_make_dirty(text);
	txt_clean_text(text);
	
	txt_pop_sel(text);
	if(!undoing) txt_undo_add_charop(text, UNDO_INSERT, '\n');
}

void txt_go_somewhere (TextLine **line) 
{
	if (!line) return;
	
	if (!(*line))
		*line= txt_new_line(NULL);
	else if ((*line)->next)
		*line= (*line)->next;
	else if ((*line)->prev)
		*line= (*line)->prev;
	else
		*line= txt_new_line(NULL);
}

void txt_delete_line (Text *text, TextLine *line) 
{
	if (!text) return;
	if (!text->curl) return;

	remlink (&text->lines, line);
	
	if (line->line) freeN(line->line);
	freeN(line);

	txt_make_dirty(text);
	txt_clean_text(text);
}

void txt_combine_lines (Text *text, TextLine *linea, TextLine *lineb)
{
	char *tmp;
	
	if (!text) return;
	
	if(!linea || !lineb) return;
	
	tmp= mallocN(linea->len+lineb->len+1, "textline_string");
	
	strcpy(tmp, linea->line);
	strcat(tmp, lineb->line);
	
	make_new_line(linea, tmp);
	
	txt_delete_line(text, lineb); 
	
	txt_make_dirty(text);
	txt_clean_text(text);
}

void txt_delete_char (Text *text) 
{
	char c='\n';
	
	if (!text) return;
	if (!text->curl) return;

	if (text->sell != text->curl || text->selc != text->curc) {
		txt_delete_sel(text);
		return;
	}
	
	if (text->curc== text->curl->len) { /* Appending two lines */
		if (text->curl->next) {
			txt_combine_lines(text, text->curl, text->curl->next);
			txt_pop_sel(text);
		}
	} else { /* Just deleting a char */
		int i= text->curc;
		
		c= text->curl->line[i];
		while(i< text->curl->len) {
			text->curl->line[i]= text->curl->line[i+1];
			i++;
		}
		text->curl->len--;

		txt_pop_sel(text);
	}

	txt_make_dirty(text);
	txt_clean_text(text);
	
	if(!undoing) txt_undo_add_charop(text, UNDO_DEL, c);
}

void txt_backspace_char (Text *text) {
	char c='\n';
	
	if (!text) return;
	if (!text->curl) return;
	
	if (text->sell != text->curl || text->selc != text->curc) {
		txt_delete_sel(text);
		return;
	}
	
	if (text->curc==0) { /* Appending two lines */
		if (text->curl->prev) {
			text->curl= text->curl->prev;
			text->curc= text->curl->len;
			
			txt_combine_lines(text, text->curl, text->curl->next);
			txt_pop_sel(text);
		}
	} else { /* Just backspacing a char */
		int i= text->curc-1;
		
		c= text->curl->line[i];
		while(i< text->curl->len) {
			text->curl->line[i]= text->curl->line[i+1];
			i++;
		}
		text->curl->len--;
		text->curc--;
		
		txt_pop_sel(text);
	}

	txt_make_dirty(text);
	txt_clean_text(text);
	
	if(!undoing) txt_undo_add_charop(text, UNDO_BS, c);
}

void txt_add_char (Text *text, char add) {
	int len;
	char *tmp;
	
	if (!text) return;
	if (!text->curl) return;

	if (add=='\n') {
		txt_split_curline(text);
		return;
	}
	
	if(txt_illegal_char(add)) return;
	
	txt_delete_sel(text);
	
	tmp= mallocN(text->curl->len+2, "textline_string");
	
	if(text->curc) memcpy(tmp, text->curl->line, text->curc);
	tmp[text->curc]= add;
	
	len= text->curl->len - text->curc;
	if(len>0) memcpy(tmp+text->curc+1, text->curl->line+text->curc, len);
	tmp[text->curl->len+1]=0;
	
	make_new_line(text->curl, tmp);
		
	text->curc++;

	txt_pop_sel(text);
	
	txt_make_dirty(text);
	txt_clean_text(text);

	if(!undoing) txt_undo_add_charop(text, UNDO_INSERT, add);
}

