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

#include "forms.h"

#include "Chroma.h"
#include <local/iff.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <math.h>

ImBuf * orig = 0, * value, * alpha, * filtbuf, * output = 0, * result, * keyback, * sub = 0, * atanlookup;
FD_Chroma *fd_Chroma;
extern void rectfill();
extern void rectalphaunder();
extern void rectcpy();
extern void rectcpymask();
extern void rectcpyalpha();
extern void rectmakepremul();

#define RESULT_BLACK 0
#define RESULT_IMAGE 1
#define RESULT_WHITE 2
#define RESULT_ALPHA 3

#define FILTUP		1
#define FILTSTD		0
#define FILTDWN		2

int ofsx = 0, ofsy = 0;
int maxx, maxy;

int yzoom = 2;
float lr = -1, lg = -1, lb = -1, mr = -1, mg = -1, mb = -1;
int rendering = FALSE;

int subx, suby, subw, subh;
struct anim * anim = 0;
int filtcount = 0, cropcount = 1, resulttype = RESULT_IMAGE;
extern int anti_mask;	

/* callbacks for form Chroma */

void set_filter(FL_OBJECT *ob, long data)
{
  /* fill-in code for callback */
  filtcount = data;
  calc_filter();
}

void set_crop(FL_OBJECT *ob, long data)
{
  /* fill-in code for callback */
  cropcount = data;
  calc_filter();
}

void set_result(FL_OBJECT *ob, long data)
{
  /* fill-in code for callback */
  resulttype = data;
  calc_result();
}

void calc_atan_space(ImBuf * ibuf)
{
	long x, y;
	double u, v, t, l;
	uchar * rect;
	
	rect = (uchar *) ibuf->rect;
	for (y = ibuf->y; y > 0 ; y--) {
		for (x = ibuf->x; x > 0 ; x--) {
			u = rect[1] - 128;
			v = rect[3] - 128;

			t = 128.0 * ((atan2(u, v) / M_PI) + 1.0);
			l = 193.0 - (256.0 * sqrt(((u * u) + (v * v)) / 32768.0));			
			
			l *= (255.0 / 193.0);
			
			if (t > 255.0) t = 255.0;
			if (l > 255.0) l = 255.0;			
			else if (l < 0.0) l = 0.0;			
						
			rect[1] = t;
			rect[3] = l;
			rect += 4;
		}
	}
}

void init_atan_space()
{
	int x, y;
	ulong * rect;
	
	atanlookup = allocImBuf(256, 256, 32, IB_rect, 0);
	rect = atanlookup->rect;
	
	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			*rect++ = (y << 16) + x;
		}
	}
	calc_atan_space(atanlookup);
}


void new_atan_space(ImBuf * ibuf)
{
	int i, col, t;
	ulong * rect;
	
	if (atanlookup == 0) init_atan_space();

	rect = ibuf->rect;
	for (i = ibuf->x * ibuf->y - 1; i >= 0; i--) {
		col = rect[i] & 0xff00ff;
		col += col >> 8;
		
		rect[i] = atanlookup->rect[col & 0xffff];
	}
}

redraw_color(FL_OBJECT *ob, Window win, int w, int h, XEvent *xev, void *ud)
{
	fl_activate_glcanvas(fd_Chroma->Color1);
	glClearColor(lr / 255.0, lg / 255.0, lb / 255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

make_sub(ImBuf * ibuf)
{
	if (sub) freeImBuf(sub);
	
	if (rendering) {
		subw = orig->x;
		subh = orig->y;
		subx = 0;
		suby = 0;
	} else {
		subw = fd_Chroma->Orig_out->w;
		subh = fd_Chroma->Orig_out->h / yzoom;
		subx = ofsx;
		suby = ofsy;
	}
	
	sub = allocImBuf(subw, subh, 32, IB_rect, 0);
	rectop(sub, ibuf, 0, 0, subx, suby, 32768, 32768, rectcpy, 0);
}

return_sub(ImBuf * ibuf)
{
	rectop(ibuf, sub, subx, suby, 0, 0, 32768, 32768, rectcpy, 0);
	
	freeImBuf(sub);
	sub = 0;
}

int
ibuf_expose(FL_OBJECT *ob, Window win, int w, int h, XEvent *xev, void *ud)
{
	ImBuf * ibuf = ud;
		
	if (ob == fd_Chroma->Orig_out) ibuf = orig;
	
	fl_activate_glcanvas(ob);
	glPixelZoom(1, - yzoom);
	glRasterPos2d(-1.0, 1.0);
	glViewport(0,0, (GLint)ob->w, (GLint)ob->h);
	
	glDrawPixels(orig->x, orig->y, GL_ABGR_EXT, GL_UNSIGNED_BYTE, ibuf->rect + (ofsy * orig->x) + ofsx);

}

calc_result()
{
	int i, a, y;
	float col, fa, fn, subkey;
	uchar * irect, * orect, * rect;
	ImBuf * tbuf;
	
	make_sub(orig);
	
	subkey = fl_get_slider_value(fd_Chroma->SubKey);
	
	if (subkey != 0.0) {
		tbuf = allocImBuf(sub->x, sub->y, 32, IB_rect, 0);
		rectop(tbuf, alpha, 0, 0, subx, suby, 32767, 32767, rectcpy, 0);
		
		orect = (uchar *) sub->rect;
		irect = (uchar *) tbuf->rect;
		
		for (i = sub->x * sub->y; i > 0; i--) {
			a = irect[0];
			if (a != 0 && a != 255) {	
					fa = irect[3] / 255.0;
					fn = subkey * (1.0 - fa);
					fa = 1.0 - fn;
					
					col = (orect[1] - fn * lb) / fa;
					if (col <= 0.0) orect[1] = 0;
					else if (col >= 255.0) orect[1] = 255;
					else orect[1] = col + 0.5;
					
					col = (orect[2] - fn * lg) / fa;
					if (col <= 0.0) orect[2] = 0;
					else if (col >= 255.0) orect[2] = 255;
					else orect[2] = col + 0.5;
					
					col = (orect[3] - fn * lr) / fa;
					if (col <= 0.0) orect[3] = 0;
					else if (col >= 255.0) orect[3] = 255;
					else orect[3] = col + 0.5;
			}
			irect += 4;
			orect += 4;
		}
		freeImBuf(tbuf);
	}
	
	rectop(sub, filtbuf, 0, 0, subx, suby, 32767, 32767, rectcpyalpha, 0);
	
	
	if (fl_get_button(fd_Chroma->SubUV)) {
		rect = (uchar *) sub->rect;
		for (i = sub->x * sub->y; i > 0; i--) {
			a = rect[0];
			if (a != 0 && a != 255) {
				y = .299 * rect[3] + .587 * rect[2] + .114 * rect[1];
				
				rect [1] = (a * rect[1] + (255 - a) * y) / 255;
				rect [2] = (a * rect[2] + (255 - a) * y) / 255;
				rect [3] = (a * rect[3] + (255 - a) * y) / 255;
			}
			rect += 4;
		}
	}
	
	if (rendering) {
		if (output) freeImBuf(output);
		output = dupImBuf(sub);
	}
	
	if (resulttype == RESULT_IMAGE) {
		rectop(sub, keyback, 0, 0, subx, suby, 32767, 32767, rectalphaunder, 0);
	} else if (resulttype == RESULT_ALPHA) {
		rect = (uchar *) sub->rect;
		for (i = sub->x * sub->y; i > 0; i--) {
			rect[1] = rect[2] = rect[3] = rect[0];
			rect += 4;
		}		
	}else {
		tbuf = allocImBuf(sub->x, sub->y, 32, IB_rect, 0);
		if (resulttype == RESULT_WHITE) rectoptot(tbuf, 0, rectfill, 0xffffffff);
		else rectoptot(tbuf, 0, rectfill, 0xff000000);
		rectoptot(sub, tbuf, rectalphaunder, 0);
		freeImBuf(tbuf);
	}
	
	return_sub(result);
	ibuf_expose(fd_Chroma->Result_out, 0, 0, 0, 0, result);	
}


void filter_r(struct ImBuf *ibuf)
{
	long i;
	uchar * rect;

	rect = (uchar *) ibuf->rect;
	for (i = ibuf->y; i > 0; i--){
		filtrow(rect, ibuf->x);
		rect += 4 * ibuf->x;
	}

	rect = (uchar *) ibuf->rect;
	for (i = ibuf->x; i > 0; i--){
		filtcolum(rect, ibuf->y, 4 * ibuf->x);
		rect += 4;
	}
}


calc_filter()
{
	uchar * irect, * orect;
	int filter, type = FILTSTD, skip;
	ImBuf * tbuf;
	int i, j, col;
	
	if (fl_get_button(fd_Chroma->Fminus)) type = FILTDWN;
	if (fl_get_button(fd_Chroma->Fplus)) type = FILTUP;

	make_sub(alpha);
	
	if (cropcount) {
		for (j = cropcount; j > 0; j--) {
			tbuf = dupImBuf(sub);
			
			irect = (uchar *) tbuf->rect;
			orect = (uchar *) sub->rect;
			
			irect += 4;
			orect += 4;
			
			if (type == FILTUP) {
				for (i = sub->x * sub->y - 2; i > 0 ; i--)
				{
					orect[0] = MAX3(irect[-4], irect[0], irect[4]);
					orect += 4;
					irect += 4;
				}
			} else {
				for (i = sub->x * sub->y - 2; i > 0 ; i--)
				{
					orect[0] = MIN3(irect[-4], irect[0], irect[4]);
					orect += 4;
					irect += 4;
				}
			}
			
			skip = sub->x * 4;
			
			irect = (uchar *) tbuf->rect;
			orect = (uchar *) sub->rect;
			
			irect += skip;
			orect += skip;
			
			if (type == FILTUP) {
				for (i = sub->x * (sub->y - 2); i > 0 ; i--)
				{
					orect[0] = MAX4(irect[- skip], irect[0], irect[+ skip], orect[0]);
					orect += 4;
					irect += 4;
				}
			} else {
				for (i = sub->x * (sub->y - 2); i > 0 ; i--)
				{
					orect[0] = MIN4(irect[- skip], irect[0], irect[+ skip], orect[0]);
					orect += 4;
					irect += 4;
				}			
			}
			freeImBuf(tbuf);
		}
	}
		
	for (filter = filtcount;filter > 0; filter --) {
		tbuf = dupImBuf(sub);
		
		filter_r(tbuf);

		irect = (uchar *) tbuf->rect;
		orect = (uchar *) sub->rect;
		
		switch (type) {
		case FILTUP:
			for (i = tbuf->x * tbuf->y; i > 0 ; i--){
				if (irect[0] > orect[0]) {
					col = irect[0] - orect[0];
					col = orect[0] + (col + col);
					if (col > 255) col = 255;
					orect[0] = col;
					orect[2] = 255;
				}
				irect += 4; orect += 4;
			}
			break;
		case FILTDWN:
			for (i = tbuf->x * tbuf->y; i > 0 ; i--){
				if (irect[0] < orect[0]) {
					col = orect[0] - irect[0];
					col = orect[0] - (col + col);
					if (col < 0) col = 0;
					orect[0] = col;
					orect[2] = 255;
				}
				irect += 4; orect += 4;
			}
			break;
		default:
			for (i = tbuf->x * tbuf->y; i > 0 ; i--){
				orect[3] = orect[2] = orect[1] = orect[0] = irect[0];
				irect += 4; orect += 4;
			}
			break;
		}
		
		freeImBuf(tbuf);
	}
	
	return_sub(filtbuf);
	
	ibuf_expose(fd_Chroma->Alpha_out, 0, 0, 0, 0, filtbuf);
	calc_result();

}

calc_alpha()
{
	int i;
	long long color;
	int hue, sat;
	float softhue, softsat;
	uchar * crect;
	ulong * lrect;
	int alpha1 = fl_get_button(fd_Chroma->Alpha1);
	long long huetab[256];
	long long sattab[256];
	int hueon, saton;
	
	hue = fl_get_slider_value(fd_Chroma->Hue);
	sat = fl_get_slider_value(fd_Chroma->Sat);
	softhue = fl_get_slider_value(fd_Chroma->SoftHue);
	softsat = fl_get_slider_value(fd_Chroma->SoftSat);
	
	hue -= softhue;
	sat -= softsat;
	softhue = 255.0 / (2 * softhue + 1);
	softsat = 255.0 / (2 * softsat + 1);
	
	hueon = ! fl_get_button(fd_Chroma->HueOff);
	saton = ! fl_get_button(fd_Chroma->SatOff);
	
	for (i = 0; i < 256; i++) {
		if (hueon) {
			color = softhue * (i - hue) + 0.5;
			if (color < 0) color = 0;
			else if (color > 255) color = 255;
			huetab[i] = (color << 16);
		} else huetab[i] = 0;
		
		if (saton) {
			color = softsat * (i - sat) + 0.5;
			if (color < 0) color = 0;
			else if (color > 255) color = 255;
			
			sattab[i] = (color << 0);
		} else sattab[i] = 0;
	}
	
	make_sub(value);
	
	lrect = sub->rect;
	crect = (uchar *) sub->rect;
	
	for (i = sub->x * sub->y; i > 0; i--) {
		color = huetab[crect[1]] + sattab[crect[3]];
		
		if (alpha1) * lrect = color;
		else * lrect = (~ color) & 0xff00ff;
		
		lrect ++;
		crect += 4;
	}
	
	if (fl_get_slider_value(fd_Chroma->SoftHue) == 0.0)
	{
		anti_mask = 0xff0000;
		antialias(sub);
	}
	
	if (fl_get_slider_value(fd_Chroma->SoftSat) == 0.0) {
		anti_mask = 0x0000ff;
		antialias(sub);
	}

	crect = (uchar *) sub->rect;
	for (i = sub->x * sub->y; i > 0; i--) {
		color = crect[1] + crect[3];
		if (color > 0xff) color = 0xff;
		
		crect[0] = color;
		
		crect += 4;
	}
		
	return_sub(alpha);
	calc_filter();
}

calc_value(float lr, float lg, float lb){
	int x, y, col, r, g, b, ty, tt, tl;
	int dr, dg, db, color;
	uchar * rect;
	ImBuf * rtbuf;
	
	rtbuf = allocImBuf(1, 1, 24, IB_rect, 0);
		r = lr; g = lg; b = lb;
		rtbuf->rect[0] = (b << 16) + (g << 8) + r;
		cspace(rtbuf, rgbyuv);
		new_atan_space(rtbuf);
		
		rect = (uchar *) rtbuf->rect;
		ty = rect[2];
		tt = rect[1];
		tl = rect[3];
		/*printf("y:%3d hue:%3d sat:%3d\n", ty, tt, tl);*/
		
	freeImBuf(rtbuf);
	if (tl == 80) {
		printf("Can't key on grayscale\n");
		return;
	}
	
	make_sub(orig);
	cspace(sub, rgbyuv);
	new_atan_space(sub);
	
	rect = (uchar *) sub->rect;
	
	for (x = sub->x; x > 0; x--) {
		for (y = sub->y; y > 0; y--) {
			db = rect[1] - tt;
			if (db > 128) db -= 256;
			if (db < -128) db += 256;
			db = 2 * ABS(db);
			
			if (db > 255) db = 255;
			
			rect[1] = db;
			rect[2] = 0;
			rect += 4;
		}
	}
	
	return_sub(value);
	
	ibuf_expose(fd_Chroma->Value_out, 0, 0, 0, 0, value);
	calc_alpha();
}

int
scroll_image(FL_OBJECT *ob, Window win, int w, int h, XEvent *xev, void *ud)
{
	FL_Coord mx, my, omx = 10000, omy;
	unsigned int keymask;
	int r = 0, g = 0, b = 0, count = 0, col;
	
	fl_get_win_mouse(fl_get_canvas_id(ob), &mx, &my, &keymask);
	omx = mx;
	omy = my;
	
	do {
		fl_get_win_mouse(fl_get_canvas_id(ob), &mx, &my, &keymask);
		if (omx != mx || omy != my) {
			ofsx += omx - mx;
			ofsy += omy - my;

			if (ofsx < 0) ofsx = 0;
			if (ofsx >= maxx) ofsx = maxx - 1;
			if (ofsy < 0) ofsy = 0;
			if (ofsy >= maxy) ofsy = maxy - 1;
			
			ibuf_expose(ob, 0, 0, 0, 0, ud);
			omx = mx;
			omy = my;
		} else sginap(1);		
	} while(keymask & (Button3Mask | Button2Mask));
	
	ibuf_expose(fd_Chroma->Orig_out, 0, 0, 0, 0, orig);
	if (rendering == FALSE) calc_value(lr, lg, lb);
}

int
color_select(FL_OBJECT *ob, Window win, int w, int h, XEvent *xev, void *ud)
{
	FL_Coord mx, my, omx = 10000, omy;
	unsigned int keymask;
	int r = 0, g = 0, b = 0, count = 0, col;
	static int first = TRUE;
	
	fl_get_win_mouse(fl_get_canvas_id(ob), &mx, &my, &keymask);
	if (keymask & (Button3Mask | Button2Mask)) return(scroll_image(ob, win, w, h, xev, ud));
	
	omx = mx - 1;
	do {
		fl_get_win_mouse(fl_get_canvas_id(ob), &mx, &my, &keymask);
		my /= yzoom;
		if (omx != mx || omy != my) {
			if (mx >= 0 && my >= 0 && mx < orig->x && my < orig->y) {
				omx = mx;
				omy = my;
								
				count++;
				col = orig->rect[(mx + ofsx) + orig->x * (my + ofsy)];
				r += col & 0xff;
				g += (col >> 8) & 0xff;
				b += (col >> 16) & 0xff;
				
				if (keymask & (Button1Mask | ShiftMask) || first) {
					lr = r / count;
					lg = g / count;
					lb = b / count;
				}
				if (keymask & (Button2Mask | ShiftMask) || first) {
					mr = r / count;
					mg = g / count;
					mb = b / count;					
				}
				
				redraw_color(0, 0, 0, 0, 0, 0);
			}
		} else sginap(1);
		
	} while(keymask & (Button1Mask | Button2Mask));
	
	first = FALSE;
	
	/*calc_value_(r / count, g / count, b / count);*/
	calc_value(lr, lg, lb);
}

int
value_select(FL_OBJECT *ob, Window win, int w, int h, XEvent *xev, void *ud)
{
	FL_Coord mx, my, omx = 10000, omy;
	unsigned int keymask;
	
	fl_get_win_mouse(fl_get_canvas_id(ob), &mx, &my, &keymask);
	if (keymask & (Button3Mask | Button2Mask)) return(scroll_image(ob, win, w, h, xev, ud));
}


void make_premul(struct ImBuf *ibuf)
{
	uchar * rect;
	long i, alpha;
	
	rect = (uchar * ) ibuf->rect;
	
	for (i = ibuf->x * ibuf->y; i > 0; i--){
		alpha = rect[0];
		rect[1] = (rect[1] * alpha ) / 255;
		rect[2] = (rect[2] * alpha ) / 255;
		rect[3] = (rect[3] * alpha ) / 255;
		rect += 4;
	}
}

new_orig()
{
	fl_set_form_title(fd_Chroma->Chroma, orig->name);

	orig->x *= 2;
	orig->y /= 2;
	
	ibuf_expose(fd_Chroma->Orig_out, 0, 0, 0, 0, orig);
}

int main(int argc, char *argv[])
{
	int attrib[]= {GLX_RGBA,GLX_DEPTH_SIZE,1,
                     GLX_RED_SIZE,1,GLX_GREEN_SIZE,1,GLX_BLUE_SIZE,1,None};
	int inputtype = 0;
	int startposition = 0;
	int ret;
	char iname[512], oname[512];
	FL_OBJECT *obj;
	
	if (argc == 1) {
		printf("Usage: %s image\n", argv[0]);
		exit(1);
	}

	anti_mask = 0xff000000;
	initmoviepointers();

	inputtype = isanim(argv[1]);
	if (inputtype == ANIM_SEQUENCE) inputtype = 0;

	if (inputtype) {
		anim = open_anim(argv[1], ANIM_DFLT, IB_rect | IB_ttob);
		if (anim) {
			if (argc == 3) startposition = atoi(argv[2]);
			
			if (startposition > 0) startposition--;
			else startposition = 0;
			
			orig = anim_absolute(anim, startposition);
		}
	}
	
	if (orig == 0) orig = loadiffname(argv[1], IB_rect | IB_ttob);
	if (orig == 0) {
		perror(argv[1]);
	}
	
	/*if (fork()) exit(0);*/
		
	fl_initialize(&argc, argv, 0, 0, 0);
	fl_set_glcanvas_defaults(attrib);
	fd_Chroma = create_form_Chroma();
	
	new_orig();

	value = allocImBuf(orig->x, orig->y, 32, IB_rect, 0);
	rectoptot(value, 0, rectfill, 0x0);
	alpha = dupImBuf(value);
	filtbuf = dupImBuf(value);
	output = dupImBuf(value);
	result = dupImBuf(value);

	maxx = 1 + orig->x - fd_Chroma->Orig_out->w;
	maxy = 1 + (yzoom * orig->y - fd_Chroma->Orig_out->h) / yzoom;

	keyback = loadiffname("/data/pics/Chroma_backbuf", IB_rect);
	scaleImBuf(keyback, orig->x, orig->y);

	/* fill-in form initialization code */

		fl_add_canvas_handler(fd_Chroma->Orig_out, Expose, ibuf_expose, orig);
		fl_add_canvas_handler(fd_Chroma->Value_out, Expose, ibuf_expose, value);
		fl_add_canvas_handler(fd_Chroma->Alpha_out, Expose, ibuf_expose, filtbuf);
		fl_add_canvas_handler(fd_Chroma->Result_out, Expose, ibuf_expose, result);
		fl_add_canvas_handler(fd_Chroma->Color1, Expose, redraw_color, 0);

		fl_add_canvas_handler(fd_Chroma->Orig_out, ButtonPress, color_select, orig);
		fl_add_canvas_handler(fd_Chroma->Value_out, ButtonPress, value_select, value);
		fl_add_canvas_handler(fd_Chroma->Alpha_out, ButtonPress, scroll_image, filtbuf);
		fl_add_canvas_handler(fd_Chroma->Result_out, ButtonPress, scroll_image, result);

		if (anim) fl_set_slider_value(fd_Chroma->MovieFrame, anim->curposition / (anim->duration - 1.0));
		
	/* show the first form */
	fl_show_form(fd_Chroma->Chroma,FL_PLACE_CENTER,FL_FULLBORDER,"Chroma");

	fl_set_form_title(fd_Chroma->Chroma, orig->name);
	
	while (1) {
		obj = fl_do_forms();
		if (obj == fd_Chroma->Hue) calc_alpha();
		if (obj == fd_Chroma->Sat) calc_alpha();
		if (obj == fd_Chroma->SoftHue) calc_alpha();
		if (obj == fd_Chroma->SoftSat) calc_alpha();
		if (obj == fd_Chroma->Alpha0) calc_alpha();
		if (obj == fd_Chroma->Alpha1) calc_alpha();
		if (obj == fd_Chroma->HueOff) {
			if (fl_get_button(fd_Chroma->HueOff)) fl_set_button(fd_Chroma->SatOff, 0);
			calc_alpha();
		}
		if (obj == fd_Chroma->SatOff) {
			if (fl_get_button(fd_Chroma->SatOff)) fl_set_button(fd_Chroma->HueOff, 0);
			calc_alpha();
		}

		if (obj == fd_Chroma->Fminus) calc_filter();
		if (obj == fd_Chroma->Fnormal) calc_filter();
		if (obj == fd_Chroma->Fplus) calc_filter();
		if (obj == fd_Chroma->Crop) calc_filter();
		
		if (obj == fd_Chroma->SubUV) calc_result();
		if (obj == fd_Chroma->SubKey) calc_result();
  
		if (obj == fd_Chroma->MovieFrame) {
			ImBuf * tbuf;
			
			do {
				tbuf = orig;
				orig = anim_absolute(anim, fl_get_slider_value(fd_Chroma->MovieFrame) * (anim->duration - 1) + 0.5);
				if (orig) {
					freeImBuf(tbuf);
					new_orig();
				} else orig = tbuf;
			} while(fl_check_forms() == fd_Chroma->MovieFrame);
			
			calc_value(lr, lg, lb);
		}
		
		if (obj == fd_Chroma->Load) {
			struct anim * tanim;
			char name[1024];
			
			strcpy(name, anim->name);
			
			if (fileselect("Select new MOVIE", name)) {
				ImBuf * tbuf = orig;
				tanim = anim;
				
				anim = open_anim(name, ANIM_DFLT, IB_rect | IB_ttob);
				if (anim) {
					orig = anim_absolute(anim, fl_get_slider_value(fd_Chroma->MovieFrame) * (anim->duration - 1) + 0.5);
					if (orig) {
						freeImBuf(tbuf);
						new_orig();
						close_anim(tanim);
						calc_value(lr, lg, lb);
					} else {
						orig = tbuf;
						close_anim(anim);
						anim = 0;
					}
				}
				if (anim == 0) anim = tanim;
			}
		}
		
		if (obj == fd_Chroma->Render) {
			ImBuf * tbuf;
			
			strcpy(iname, orig->name);
			strcpy(oname, orig->name);
			strcat(oname, ".key");
			
			tbuf = orig;
			
			if (anim) {
				startposition = anim->curposition;
				sprintf(oname, "%s.%04d", anim->name, anim->curposition + 1);
				ret = fileselect("Select first OUTPUT image", oname);
			} else {
				ret = fileselect("Select first INPUT image", iname);
			}
			
			if (ret) {
				rendering = TRUE;
				
				do {
					if (anim) {
						orig = anim_absolute(anim, anim->curposition);
						anim->curposition++;
					} else {
						orig = loadiffname(iname, IB_rect | IB_ttob);
						newname(iname, +1);
					}
					
					if (orig) {
						new_orig();
						if (anim) fl_set_slider_value(fd_Chroma->MovieFrame, anim->curposition / (anim->duration - 1.0));
						if (fl_check_forms() == fd_Chroma->Stop) {
								freeImBuf(orig);
								orig = 0;
								break;
							}
						calc_value(lr, lg, lb);
							if (fl_check_forms() == fd_Chroma->Stop) {
								freeImBuf(orig);
								orig = 0;
								break;
							}
							
						
						output->x /= 2;
						output->y *= 2;
						
						output->depth = 32;
						output->ftype = TGA;
						
						flipy(output);
						make_premul(output);
						fflush(stdout);

						saveiff(output, oname, IB_rect);
							if (fl_check_forms() == fd_Chroma->Stop) {
								freeImBuf(orig);
								orig = 0;
								break;
							}
						newname(oname, +1);
						freeImBuf(orig);
					}
				}while(orig);
				
				rendering = FALSE;
				if (anim) anim->curposition = startposition;
			}
			orig = tbuf;
			ibuf_expose(fd_Chroma->Orig_out, 0, 0, 0, 0, orig);
			calc_value(lr, lg, lb);
			fl_set_form_title(fd_Chroma->Chroma, orig->name);
			if (anim) fl_set_slider_value(fd_Chroma->MovieFrame, anim->curposition / (anim->duration - 1.0));
		}
	}
}

