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

#include <local/iff.h>

#include <gl/image.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <local/util.h>
#include <local/gl_util.h>
#include <local/Button.h>
#include <math.h>
#include <fcntl.h>
#include <sys/sysmips.h>	/* voor fp-exeption trap */
#include <sys/times.h>

/*

zipfork "make morph >/dev/console"                                          
zipfork "morph /data/morph/jongen /data/morph/meisje >/dev/console"                                          
zipfork "morph /fast/demo/0250 >/dev/console"                                          

*/

#define NEW(x) callocstructN(struct x, 1, # x)
#define double float

struct morphline{
	float co[2][2];
	float d[2];
	float len;
	float len2;
};


struct morphvec{
	struct morphvec * next,  *prev;
	long flags;
	struct morphline in, out;
};

struct morphdata{
	float x, y, sum;
};


struct sum{
	long a, b, g, r;
};

#define MV_SEL	    (1 << 16)
#define MV_PMASK    (3)
#define MV_ADDED	(1 << 17)
#define MV_DELETE	(1 << 18)

long do_ease = FALSE, xtra = FALSE, fast = TRUE, anim = FALSE, fields = FALSE;
long inwin1, inwin2, outwin;
struct ImBuf * ibuf1, *ibuf2, *obuf;
char filename[128], titlehead[128];
short smallres = 4;
long smallxres, smallyres;
short frame = 10;

struct ListBase _morphbase = {0, 0}, *morphbase = &_morphbase;
struct ListBase _shadowbase = {0, 0}, *shadowbase = &_morphbase;

float ibleft, ibbottom, ibright, ibtop;

void mapat(float left, float bottom, float right, float top)
{
	ibleft = left;
	ibbottom = bottom;
	ibright = right;
	ibtop = top;
}


float inter(float v1, float v2, float v3, float va, float vc)
{
	float vb;
	
	/* 
		v1		v2		v3
		va	   ?vb?		vc
		
		(v1 - v2) : (v1 - v3) = (va - vb) : (va - vc) ==>
	 vb = va - ((va - vc) x (v1 - v2) / (v1 - v3))
	*/
	
	if (v1 == v3) return (va);
	vb = va - ((va - vc) * (v1 - v2) / (v1 - v3));

	return (vb);
}


void my_lrectwrite(float left, float bottom, float right, float top, struct ImBuf * ibuf)
{
	ulong *rect, *newrect;
	static ulong _newrect[4096];
	long x,y;
	long ofsx, ofsy, stepx, stepy, lasty, sizex, sizey;
	Matrix mat;
	float cox1, coy1, cox2, coy2;
	short newx, newy;

	if (ibuf == 0) return;
	if (ibuf->rect == 0) return;

	/* clip coordinaten in image-space (0 -> 1) */
	
	if (left == right || top == bottom) return;
	
	if (left < ibleft) left = ibleft;
	if (bottom < ibbottom) bottom = ibbottom;
	if (right > ibright) right = ibright;
	if (top > ibtop) top = ibtop;
	
	getmatrix(mat);

	for (y = 0; y < 4; y++){
		for (x = 0; x < 4; x++){
			printf("%.4f ", mat[y][x]);
		}
		printf("\n");
	}

	/* projectie */
	cox1 = (mat[0][0] * left) + mat[3][0];
	coy1 = (mat[1][1] * bottom) + mat[3][1];
	cox2 = (mat[0][0] * right) + mat[3][0];
	coy2 = (mat[1][1] * top) + mat[3][1];
	
	printf("%f %f %f %f\n", cox1, coy1, cox2, coy2);

	/* clippen met viewvolume */
	
	if (cox1 < -1.0) {
		left = inter(cox1, -1.0, cox2, left, right);
		cox1 = -1.0;
	}

	if (coy1 < -1.0) {
		bottom = inter(coy1, -1.0, coy2, bottom, top);
		coy1 = -1.0;
	}
	
	if (cox2 > 1.0) {
		right = inter(cox1, 1.0, cox2, left, right);
		cox2 = 1.0;
	}

	if (coy2 > 1.0) {
		top = inter(coy1, 1.0, coy2, bottom, top);
		coy2 = 1.0;
	}
	
	/* omzetten naar window coordinaten */
	
	getsize(&sizex,  &sizey);
	getorigin(&ofsx, &ofsy);
	
	cox1 = ((sizex + 1.0) * cox1 + sizex) / 2.0;
	cox2 = ((sizex + 1.0) * cox2 + sizex) / 2.0;

	coy1 = ((sizey + 1.0) * coy1 + sizey) / 2.0;
	coy2 = ((sizey + 1.0) * coy2 + sizey) / 2.0;
		
	sizex = getgdesc(GD_XPMAX);
	sizey = getgdesc(GD_YPMAX);
	
	/* clippen met scherm */
	
	if (cox1 + ofsx < 0) {
		left = inter(cox1, -ofsx, cox2, left, right);
		cox1 = -ofsx;		
	}

	if (coy1 + ofsy < 0) {
		bottom = inter(coy1, -ofsy, coy2, bottom, top);
		coy1 = -ofsy;
	}

	if (cox2 + ofsx > sizex) {
		right = inter(cox1, sizex - ofsx, cox2, left, right);
		cox2 = sizex - ofsx;		
	}

	if (coy2 + ofsy > sizey) {
		top = inter(coy1, sizey - ofsy, coy2, bottom, top);
		coy2 = sizey - ofsy;
	}

	printf("%f %f %f %f\n", cox1, coy1, cox2, coy2);
	
	/* coordinaten in plaatje berekenen */
	
	left = ibuf->x * (left - ibleft) / (ibright - ibleft);
	right = ibuf->x * (right - ibleft) / (ibright - ibleft);
	
	bottom = ibuf->y * (bottom - ibbottom) / (ibtop - ibbottom);
	top = ibuf->y * (top - ibbottom) / (ibtop - ibbottom);
	
	printf("%f %f %f %f\n\n", left, bottom, right, top);
	
	
	stepx = (65536.0 * (right - left) / (cox2 - cox1)) + 0.5;
	stepy = (65536.0 * (top - bottom) / (coy2 - coy1)) + 0.5;
	
	printf("%x %x\n", stepx, stepy);
	ofsy = 65536 * (bottom + 0.5);
	lasty = -0xffffff;
	newy = coy2;
	newx = cox2 - cox1;
	
	for (y = coy1; y < newy ; y++){
		newrect = _newrect;
		rect = ibuf->rect;
		if ((ofsy >> 16) >= ibuf->y) break;
		
		if ((ofsy >> 16) != (lasty >> 16)) {
			lasty = ofsy;
			rect += (ofsy >> 16) * ibuf->x;
			
			ofsx = 65536 * (left + 0.5);
			for (x = newx ; x>0 ; x--){
				*newrect++ = rect[ofsx >> 16];
				ofsx += stepx;
			}
		}
		ofsy += stepy;
		lrectwrite(cox1, y, cox1 + newx - 1, y, _newrect);
	}
}


float ease(float factor)
{
	return ((3.0 * factor * factor) - (2.0 * factor * factor * factor));

}

long point(struct ImBuf *ibuf, float fx, float fy)
{
	long x, y;

	if (ibuf == 0) return(0);
	if (ibuf->rect == 0) return(0);

	x = fx + 0.5; 
	y = fy + 0.5;

	if (x < 0 || y < 0 || x >= ibuf->x || y >= ibuf->y) return(0);
	return(ibuf->rect[(y * ibuf->x) + x]);
}


void plot(struct ImBuf *ibuf, float fx, float fy, long col)
{
	long x, y;

	if (ibuf == 0) return;
	if (ibuf->rect == 0) return;

	x = fx + 0.5; 
	y = fy + 0.5;
	if (x < 0 || y < 0 || x >= ibuf->x || y >= ibuf->y) return;
	ibuf->rect[(y * ibuf->x) + x] = col;
}


void initmorphline(struct morphline * mline)
{
	float x, y, len;

	x = mline->co[1][0] - mline->co[0][0];
	y = mline->co[1][1] - mline->co[0][1];

	mline->d[0] = x;
	mline->d[1] = y;

	len = (x * x) + (y * y);
	if (len == 0) len = 0.0001;
	mline->len2 = len;

	len = sqrtf(len);
	mline->len = len;
}

void sample_at(double * col, struct sum * sum0, double x, double y, short step, short add)
{
	double t1, t2, t3, t4;
	double a, b, g, r;
	struct sum * sum1;
	
	/* geeft een geinterpoleerde sample terug */

	sum1 = sum0 + step;
	
	t3 = sum0[0].a; t4 = sum0[1].a;
	t1 = t3 + x * (t4 - t3);
	t3 = sum1[0].a; t4 = sum1[1].a;
	t2 = t3 + x * (t4 - t3);
	a = t1 + y * (t2 - t1);

	t3 = sum0[0].b; t4 = sum0[1].b;
	t1 = t3 + x * (t4 - t3);
	t3 = sum1[0].b; t4 = sum1[1].b;
	t2 = t3 + x * (t4 - t3);
	b = t1 + y * (t2 - t1);

	t3 = sum0[0].g; t4 = sum0[1].g;
	t1 = t3 + x * (t4 - t3);
	t3 = sum1[0].g; t4 = sum1[1].g;
	t2 = t3 + x * (t4 - t3);
	g = t1 + y * (t2 - t1);

	t3 = sum0[0].r; t4 = sum0[1].r;
	t1 = t3 + x * (t4 - t3);
	t3 = sum1[0].r; t4 = sum1[1].r;
	t2 = t3 + x * (t4 - t3);
	r = t1 + y * (t2 - t1);
	
	if (add == '+'){
		col[0] += a;
		col[1] += b;
		col[2] += g;
		col[3] += r;		
	}else{
		col[0] -= a;
		col[1] -= b;
		col[2] -= g;
		col[3] -= r;		
	}
}


double subareasample_2x_zo_traag(struct ImBuf *ibuf, double x1, double y1, double x2, double y2, double *col)
{
	double div;
	long startx, endx, starty, endy;
	struct sum * sum;
	uchar * rect;
	
	sum = ibuf->userdata;
	if (sum == 0) return(0.0);	

	/*sum += ibuf->x + 1 + 1;*/

	div = (x2 - x1) * (y2 - y1);
	if (div == 0.0) return (0.0);

	startx = x1; x1 -= startx;
	endx = x2; x2 -= endx;
	starty = y1; y1 -= starty;
	endy = y2; y2 -= endy;
	
	if (startx == endx && starty == endy){
		rect = (uchar *) &ibuf->rect[starty * ibuf->x + startx];
		col[0] = rect[0] * div;
		col[1] = rect[1] * div;
		col[2] = rect[2] * div;
		col[3] = rect[3] * div;		
	}else{
		starty *= ibuf->x + 1;
		endy *= ibuf->x + 1;
		sample_at(col, sum + endy + endx, x2, y2, ibuf->x + 1, '+');
		sample_at(col, sum + starty + endx, x2, y1, ibuf->x + 1, '-');
		sample_at(col, sum + endy + startx, x1, y2, ibuf->x + 1, '-');
		sample_at(col, sum + starty + startx, x1, y1, ibuf->x + 1, '+');
	}
	return(div);
}


float subareasample(struct ImBuf *ibuf, float x1, float y1, float x2, float y2, float *col)
{
	/* sample box, is reeds geclipt en minx enz zijn op ibuf size gezet.
     * Vergroot uit met antialiased edges van de pixels */

	float muly, mulx, div, a=0, b=0, g=0, r=0;
	long x, y, startx, endx, starty, endy;
	uchar *rect;

	/* alle coordinaten zijn >= 0.0. ffloor() is dus overbodig */
	
	startx = x1; endx = x2;
	starty = y1; endy = y2;

	if (starty == endy && startx == endx) {
		rect = (uchar *) &ibuf->rect[starty * ibuf->x + startx];
		div = (x2 - x1) * (y2 - y1);
		col[0] = div * (short) rect[0];
		col[1] = div * (short) rect[1];
		col[2] = div * (short) rect[2];
		col[3] = div * (short) rect[3];
	} else {
		div = 0.0;
		for(y = starty; y <= endy; y++) {
			rect = (uchar *) &ibuf->rect[y * ibuf->x + startx];

			muly = 1.0;
			if(y == starty) muly= ((float ) 1.0) - (y1 - y);
			if(y == endy) muly= (y2 - y);

			for(x = startx; x <= endx; x++) {
				mulx = muly;
				if(x == startx) mulx *= ((float ) 1.0) - (x1 - x);
				if(x == endx) mulx *= (x2 - x);

				a += mulx * (short) rect[0];
				b += mulx * (short) rect[1];
				g += mulx * (short) rect[2];
				r += mulx * (short) rect[3];
				div += mulx;
				rect += 4;
			}
		}
		col[0] = a;
		col[1] = b;
		col[2] = g;
		col[3] = r;
	}

	return(div);
}


long areasample(struct ImBuf *ibuf, double x1, double y1, double x2, double y2)
{
	double col[4], div = 0.0, t;
	short startx, endx, starty, endy;
	short a, b, g, r;

	if (ibuf == 0) return(0);
	col[0] = col[1] = col[2] = col[3] = 0.0;
	
	if (x1 > x2) {
		t = x1; 
		x1 = x2; 
		x2 = t;
	}
	if (y1 > y2) {
		t = y1; 
		y1 = y2; 
		y2 = t;
	}

	startx = floor(x1); 
	endx = floor(x2);
	starty = floor(y1); 
	endy = floor(y2);

	if (endy < 0 || endx < 0 || starty >= ibuf->y || startx >= ibuf->x) return(0);
	if (startx == endx && starty == endy) return(ibuf->rect[starty * ibuf->x + startx]);

	/* clipping */

	if (startx < 0){
		div -= x1 * (y2 - y1);
		x1 = 0;
	}
	if (starty < 0){
		div -= y1 * (x2 - x1);
		y1 = 0;
	}
	if (endx >= ibuf->x){
		div += (x2 - ibuf->x) * (y2 - y1);
		x2 = ibuf->x - (float) 0.000001;    /* clip */
	}
	if (endy >= ibuf->y){
		div += (y2 - ibuf->y) * (x2 - x1);
		y2 = ibuf->y - (float) 0.000001;    /* clip */
	}

	div += subareasample(ibuf, x1, y1, x2, y2, col);

	if (div == 0.0) return(0);

	a = (col[0] / div) + (float) 0.5;
	b = (col[1] / div) + (float) 0.5;
	g = (col[2] / div) + (float) 0.5;
	r = (col[3] / div) + (float) 0.5;

	if (a & 256) {
		if (a < 0) a = 0;
		else a = 255;
	}
	if (b & 256) {
		if (b < 0) b = 0;
		else b = 255;
	}
	if (g & 256) {
		if (g < 0) g = 0;
		else g = 255;
	}
	if (r & 256) {
		if (r < 0) r = 0;
		else r = 255;
	}
	
	return ((a << 24) + (b << 16) + (g << 8) + r);
}


void makesumtable(struct ImBuf * ibuf)
{
	long x, y, a, b, g, r;
	struct sum * sum;
	uchar * rect;
	
	if (ibuf == 0) return;
	if (ibuf->rect == 0) return;
	sum = callocstructN(struct sum, (ibuf->x + 1) * (ibuf->y + 1), "sumdata");
	if (sum == 0) return;
	ibuf->userdata = sum;
	
	/* invullen van de sumtable: rect[0][0] -> sum[1][1] */
	sum += ibuf->x + 1 + 1;
	rect = (uchar *) ibuf->rect;
	
	for (y = ibuf->y; y > 0; y--){
		for (x = ibuf->x; x > 0; x--){
			sum->a = rect[0] + sum[-1].a + sum[-(ibuf->x + 1)].a - sum[-(ibuf->x + 1) - 1].a;
			sum->b = rect[1] + sum[-1].b + sum[-(ibuf->x + 1)].b - sum[-(ibuf->x + 1) - 1].b;
			sum->g = rect[2] + sum[-1].g + sum[-(ibuf->x + 1)].g - sum[-(ibuf->x + 1) - 1].g;
			sum->r = rect[3] + sum[-1].r + sum[-(ibuf->x + 1)].r - sum[-(ibuf->x + 1) - 1].r;
			sum ++;
			rect += 4;
		}
		sum ++; /* eerste waarde overslaan */
	}
}

float xy_to_xy(struct morphline *in, struct morphline *out, long maxx, long maxy, struct morphdata * data, long step)
{
	float a, b, c, d, u, v, force, t1, t2;
	float dxdx, dydx;
	float dudx, dvdx;
	float nx, ny;
	long x, y;

	initmorphline(in);
	initmorphline(out);

	a = out->co[0][0]; 
	b = out->co[0][1];
	c = out->d[0]; 
	d = out->d[1];

	dudx =  step * c / out->len2;
	dvdx =  step * d / out->len;

	dxdx = (dudx * in->d[0]) + (dvdx * in->d[1] / in->len);
	dydx = (dudx * in->d[1]) - (dvdx * in->d[0] / in->len);

	for (y = 0; y < maxy ; y += step){
		u = ((y * d) - (b * d) - (a * c)) / out->len2;
		v = ((b * c) - (a * d) - (y * c)) / out->len;

		nx = in->co[0][0] + (u * in->d[0]) + (v * in->d[1] / in->len);
		ny = in->co[0][1] + (u * in->d[1]) - (v * in->d[0] / in->len);

		for (x = 0; x < maxx ; x += step){
			if (u < (float) 0.0) {
				t1 = x - a;
				t2 = y - b;
				force = (t1 * t1) + (t2 * t2);
			} else if (u > (float) 1.0){
				t1 = x - out->co[1][0];
				t2 = y - out->co[1][1];
				force = (t1 * t1) + (t2 * t2);
			} else force = v * v;

			force = ((float) 1.0) / (((float) 1.0) + force);

			data->x += nx * force;
			data->y += ny * force;
			data->sum += force;
			data++;

			u += dudx;
			v += dvdx;
			nx += dxdx;
			ny += dydx;
		}
	}
}


float xy_to_xy_lin(struct morphline *in, struct morphline *out, long maxx, long maxy, struct morphdata * data, long step)
{
	float a, b, c, d, u, v, force, t1, t2;
	float dxdx, dydx;
	float dudx, dvdx;
	float nx, ny;
	long x, y;

	initmorphline(in);
	initmorphline(out);

	a = out->co[0][0]; 
	b = out->co[0][1];
	c = out->d[0]; 
	d = out->d[1];

	dudx =  step * c / out->len2;
	dvdx =  step * d / out->len2;

	dxdx = (dudx * in->d[0]) + (dvdx * in->d[1]);
	dydx = (dudx * in->d[1]) - (dvdx * in->d[0]);

	for (y = 0; y < maxy ; y += step){

		u = ((y * d) - (b * d) - (a * c)) / out->len2;
		v = ((b * c) - (a * d) - (y * c)) / out->len2;

		nx = in->co[0][0] + (u * in->d[0]) + (v * in->d[1]);
		ny = in->co[0][1] + (u * in->d[1]) - (v * in->d[0]);

		for (x = 0; x < maxx ; x += step){
			if (u < 0.0) {
				t1 = x - a;
				t2 = y - b;
				force = (t1 * t1) + (t2 * t2);
			} else if (u > 1.0){
				t1 = x - out->co[1][0];
				t2 = y - out->co[1][1];
				force = (t1 * t1) + (t2 * t2);
			} else {
				force = v * out->len2 / out->len;
				force *= force;
			}

			if (do_ease) force = sqrtf(force);
			if (xtra) force = 1.0 / (0.00001 + force);
			else force = 1.0 / (1.0 + force);

			data->x += nx * force;
			data->y += ny * force;
			data->sum += force;
			data++;

			u += dudx;
			v += dvdx;
			nx += dxdx;
			ny += dydx;
		}
	}
}

void drawline(struct morphvec *mvec, long col)
{
	if (mvec == 0) return;
	
	winset(inwin1);
	cpack(0);
	linewidth(4);
	bgnline();
		v2f(mvec->in.co[0]);
		v2f(mvec->in.co[1]);
	endline();
	linewidth(2);
	cpack(col);
	bgnline();
		v2f(mvec->in.co[0]);
		v2f(mvec->in.co[1]);
	endline();
	
	winset(inwin2);
	cpack(0);
	linewidth(4);
	bgnline();
		v2f(mvec->out.co[0]);
		v2f(mvec->out.co[1]);
	endline();
	linewidth(2);
	cpack(col);
	bgnline();
		v2f(mvec->out.co[0]);
		v2f(mvec->out.co[1]);
	endline();
}


void draw_ov_line(struct morphvec *mvec)
{
	if (mvec == 0) return;
	
	winset(inwin1);
	color(0);
	clear();
	color(1);
	mapcolor(1, 0, 0, 0);
	linewidth(4);
	bgnline();
		v2f(mvec->in.co[0]);
		v2f(mvec->in.co[1]);
	endline();
	linewidth(2);
	color(3);
	mapcolor(3, 0xff, 0xff, 0xff);
	bgnline();
		v2f(mvec->in.co[0]);
		v2f(mvec->in.co[1]);
	endline();
	
	winset(inwin2);
	color(0);
	clear();
	color(1);
	mapcolor(1, 0, 0, 0);
	linewidth(4);
	bgnline();
		v2f(mvec->out.co[0]);
		v2f(mvec->out.co[1]);
	endline();
	linewidth(2);
	color(3);
	mapcolor(3, 0xff, 0xff, 0xff);
	bgnline();
		v2f(mvec->out.co[0]);
		v2f(mvec->out.co[1]);
	endline();
}

void draw_rect(struct morphline * line, struct ImBuf *ibuf)
{
	float x1, x2, y1, y2, t;

	if (line == 0) return;
	if (ibuf == 0) return;
	pushviewport();

	x1 = line->co[0][0] * ibuf->x;
	y1 = line->co[0][1] * ibuf->y;
	x2 = line->co[1][0] * ibuf->x;
	y2 = line->co[1][1] * ibuf->y;
	
	if (x2 < x1) {t = x1; x1 = x2; x2 = t;}
	if (y2 < y1) {t = y1; y1 = y2; y2 = t;}

#define BOXXTRA 3.0

	x1 -= BOXXTRA;
	y1 -= BOXXTRA;
	x2 += BOXXTRA;
	y2 += BOXXTRA;
	
	scrmask(x1 + 0.5, x2 + 0.5, y1 + 0.5, y2 + 0.5);
	lrectwrite(0, 0, ibuf->x - 1, ibuf->y - 1, ibuf->rect);
	popviewport();
}


void draw_select(struct morphvec *mvec)
{
	drawline(mvec, 0xffffffff);
}

void draw_normal(struct morphvec *mvec)
{
	drawline(mvec, 0xff808080);	
}


void draw_all()
{
	struct morphvec *mvec;

	mvec = morphbase->first;
	while (mvec){
		draw_normal(mvec);
		mvec = mvec->next;
	}
}


void swap_lines()
{
	struct morphvec *mvec;
	struct morphline t;
	
	mvec = morphbase->first;
	while (mvec){
		t = mvec->in;
		mvec->in = mvec->out;
		mvec->out = t;
		mvec = mvec->next;
	}
}


void swap_all()
{
	struct ImBuf * tbuf;

	swap_lines();
	
	tbuf = ibuf1;
	ibuf1 = ibuf2;
	ibuf2 = tbuf;
}


void draw_clear(struct morphvec *mvec)
{
	struct morphvec *mvec2;
	if (mvec == 0) return;

	winset(inwin1);
	draw_rect(&mvec->in, ibuf1);
	winset(inwin2);
	draw_rect(&mvec->out, ibuf2);

	mvec2 = morphbase->first;
	while (mvec2){
		if (mvec2 != mvec) draw_normal(mvec2);
		mvec2 = mvec2->next;
	}
	
}


struct morphvec * getline()
{
	long ofsx, ofsy, sizex, sizey, event;
	short val, go = TRUE;
	float sx, sy;
	struct morphvec *mvec;
	
	mvec = NEW(morphvec);

	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);
	sx = 1.0 / sizex;
	sy = 1.0 / sizey;

	mvec->in.co[0][0] = mvec->out.co[0][0] = sx * (mousexN - ofsx);
	mvec->in.co[0][1] = mvec->out.co[0][1] = sy * (mouseyN - ofsy);

	winset(inwin1);
	drawmode(OVERDRAW);
	winset(inwin2);
	drawmode(OVERDRAW);
	
	while (go){
		do{
			event = qreadN(&val);
			if (event == LEFTMOUSE || event == ESCKEY) go = FALSE;
		}while (qtest() && go);
		
		mvec->in.co[1][0] = mvec->out.co[1][0] = sx * (mousexN - ofsx);
		mvec->in.co[1][1] = mvec->out.co[1][1] = sy * (mouseyN - ofsy);
		draw_ov_line(mvec);
	}
	
	winset(inwin1);
	color(0); clear();
	drawmode(NORMALDRAW);
	winset(inwin2);
	color(0); clear();
	drawmode(NORMALDRAW);

	if (event == ESCKEY){
		freeN(mvec);
		return(0);
	}
	
	addtail(morphbase, mvec);
	return(mvec);
}


void move_mvec(struct morphvec *mvec)
{
	float * vec, sx, sy;
	short val, go = TRUE;
	long event, mousex, mousey;
	long sizex, sizey;
	struct morphvec backvec;
	
	backvec = *mvec;
	if (mvec == 0) return;
	switch(mvec->flags & MV_PMASK){
	case 0:
		vec = mvec->in.co[0];
		break;
	case 1:
		vec = mvec->in.co[1];
		break;
	case 2:
		vec = mvec->out.co[0];
		break;
	case 3:
		vec = mvec->out.co[1];
		break;
	}
	
	getsize(&sizex, &sizey);
	sx = 1.0 / sizex;
	sy = 1.0 / sizey;
	
	draw_clear(mvec);
	
	winset(inwin1);
	drawmode(OVERDRAW);
	winset(inwin2);
	drawmode(OVERDRAW);

	mousex = mousexN;
	mousey = mouseyN;

	while(go){
		draw_ov_line(mvec);
		do {
			switch (event = qreadN(&val)){
			case ESCKEY:
				*mvec = backvec;
			case MIDDLEMOUSE:
				go = FALSE;
				break;
			case MOUSEX:
			case MOUSEY:
				vec[0] += (mousexN - mousex) * sx;
				vec[1] += (mouseyN - mousey) * sy;
				mousex = mousexN;
				mousey = mouseyN;
				break;
			}
		}while(qtest() && go);
	}

	winset(inwin1);
	color(0); clear();
	drawmode(NORMALDRAW);
	winset(inwin2);
	color(0); clear();
	drawmode(NORMALDRAW);
	
	draw_select(mvec);
}

struct morphvec * select_mvec(long out)
{
	long ofsx, ofsy, sizex, sizey, ofs = 0;
	struct morphvec *mvec, *msel = 0;
	float x, y, dist, mindist, a, b;
	struct morphline *mline;
	mvec = morphbase->first;

	getorigin(&ofsx,&ofsy);
	getsize(&sizex,&sizey);
	x = mousexN - ofsx;
	y = mouseyN - ofsy;
	x /= sizex;
	y /= sizey;
	mindist = 1000000;

	if (out) ofs = 2;
	while(mvec){
		mvec->flags = 0;

		if (out) mline = &mvec->out;
		else mline = &mvec->in;

		a = mline->co[0][0] - x;
		b = mline->co[0][1] - y;
		dist = (a * a) + (b * b);
		if (dist < mindist){
			mindist = dist;
			msel = mvec;
			mvec->flags = ofs;
		}

		a = mline->co[1][0] - x;
		b = mline->co[1][1] - y;
		dist = (a * a) + (b * b);
		if (dist < mindist){
			mindist = dist;
			msel = mvec;
			mvec->flags = ofs + 1;
		}

		mvec = mvec->next;
	}

	if (msel) msel->flags |= MV_SEL;
	return(msel);
}


void save()
{
	char back[128];
	long file = 0, ok = 0;
	struct morphvec * mvec;

	strcpy(back, filename);

	if (fileselect("Save Morph", filename) == 0) return;

	file=open(filename,O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (file >= 0) {
		if (write(file, "MRF1", 4) == 4) ok = TRUE;
	}

	if (ok) {
		ok = FALSE;
		mvec = morphbase->first;
		while (mvec){
			if (write(file, mvec, sizeof(struct morphvec)) != sizeof(struct morphvec)) break;
			mvec = mvec->next;
		}

		if (mvec == 0) ok = TRUE;
	}

	if (ok == 0){
		perror("morph - save");
		strcpy(filename, back);
	}
	if (file > 0) close(file);
}


void load(long ask)
{
	char back[128], buf[10];
	long size, file = 0, ok = 0;
	struct morphvec * mvec, vec;


	strcpy(back, filename);
	if (ask) if (fileselect("Load Morph", filename) == 0) return;

	file=open(filename,O_RDONLY);
	if (file > 0) {
		if (read(file, buf, 4) == 4){
			buf[4] = 0;
			if (strcmp(buf, "MRF1") == 0) ok = TRUE;
		}
	}

	if (ok) {
		ok = FALSE;

		while (morphbase->first){
			mvec = morphbase->first;
			remlink(morphbase, mvec);
			freeN(mvec);
		}

		while (1){
			size = read(file, &vec, sizeof(struct morphvec));
			if (size != sizeof(struct morphvec)) break;
			mvec = NEW(morphvec);
			*mvec = vec;
			addtail(morphbase, mvec);
		}

		if (size != 0) printf("morph - load: unexpected end of file\n");
		ok = TRUE;
	}

	if (ok == 0){
		perror("morph - load");
		strcpy(filename, back);
	}

	if (file > 0) close(file);
}

void qdevs()
{
	qdevice(INPUTCHANGE);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RAWKEYBD);
	qdevice(WINSHUT);
	qdevice(WINQUIT);
	qdevice(INSERTKEY);
	qdevice(HOMEKEY);
	qdevice(PAGEUPKEY);
	qdevice(KEYBD);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(F1KEY);
	qdevice(F2KEY);
	qdevice(F3KEY);
	qdevice(ESCKEY);
	qdevice(LEFTARROWKEY);
	qdevice(RIGHTARROWKEY);
	qdevice(UPARROWKEY);
	qdevice(DOWNARROWKEY);	
}


void add_outline()
{
	struct morphvec * mvec;

	mvec = NEW(morphvec);
	addtail(morphbase, mvec);
	mvec->in.co[0][0] = mvec->out.co[0][0] = 0;
	mvec->in.co[0][1] = mvec->out.co[0][1] = 0;
	mvec->in.co[1][0] = mvec->out.co[1][0] = 1;
	mvec->in.co[1][1] = mvec->out.co[1][1] = 0;

	mvec = NEW(morphvec);
	addtail(morphbase, mvec);
	mvec->in.co[0][0] = mvec->out.co[0][0] = 0;
	mvec->in.co[0][1] = mvec->out.co[0][1] = 1;
	mvec->in.co[1][0] = mvec->out.co[1][0] = 1;
	mvec->in.co[1][1] = mvec->out.co[1][1] = 1;

	mvec = NEW(morphvec);
	addtail(morphbase, mvec);
	mvec->in.co[0][0] = mvec->out.co[0][0] = 0;
	mvec->in.co[0][1] = mvec->out.co[0][1] = 0;
	mvec->in.co[1][0] = mvec->out.co[1][0] = 0;
	mvec->in.co[1][1] = mvec->out.co[1][1] = 1;

	mvec = NEW(morphvec);
	addtail(morphbase, mvec);
	mvec->in.co[0][0] = mvec->out.co[0][0] = 1;
	mvec->in.co[0][1] = mvec->out.co[0][1] = 0;
	mvec->in.co[1][0] = mvec->out.co[1][0] = 1;
	mvec->in.co[1][1] = mvec->out.co[1][1] = 1;
}


void smalltolarge(struct morphdata *data, struct morphdata *small, long lrgx, long lrgy, long smlx)
{
	float smallxstep, smallystep, facx, facy;
	long x, y, stepx, stepy;
	struct morphdata * _small;
	float x1, x2, x3, x4, y1, y2, y3, y4, t1, t2;
	
	_small = small;
	smallxstep = 1.0 / smallxres;
	smallystep = 1.0 / smallyres;
	stepy = smallyres;
	facy = 0.0;
	
	for (y = 0; y <lrgy; y++){
		facx = 0.0;
		stepx = smallxres;
		small = _small;

		x1 = small[0].x; x2 = small[1].x; x3 = small[smlx].x; x4 = small[smlx + 1].x;
		y1 = small[0].y; y2 = small[1].y; y3 = small[smlx].y; y4 = small[smlx + 1].y;
		
		for (x = 0; x <lrgx; x++){

			t1 = x1 + facx * (x2 - x1);
			t2 = x3 + facx * (x4 - x3);
			data->x = t1 + facy * (t2 - t1);
			t1 = y1 + facx * (y2 - y1);
			t2 = y3 + facx * (y4 - y3);
			data->y = t1 + facy * (t2 - t1);

			data++;
			if (--stepx <= 0){
				small++;
				stepx = smallxres;
				facx = 0.0;
				x1 = x2; x3 = x4;
				y1 = y2; y3 = y4;
				x2 = small[1].x; x4 = small[smlx + 1].x;
				y2 = small[1].y; y4 = small[smlx + 1].y;
			}else facx += smallxstep;
		}
		
		if (--stepy <= 0) {
			_small += smlx;
			stepy = smallyres;
			facy = 0.0;
		}else facy += smallystep;
	}
}


void samples_to_rect(struct ImBuf *obuf, struct ImBuf *ibuf, struct morphdata * data)
{
	ulong *rect;
	float xmin, ymin, xmax, ymax;
	float f1, f2, f3, f4;
	long x, y;
	long stijd, etijd;
	struct tms voidbuf;
	
	stijd = times(&voidbuf);
	rect = obuf->rect;
	if (fast){
		for (y = obuf->y; y > 0 ; y--){
			for (x = obuf->x; x > 0 ; x--){
				*rect++ = point(ibuf, data->x, data->y);
				data++;
			}
			data++;
		}
	} else{
		for (y = obuf->y; y > 0 ; y--){
			for (x = obuf->x; x > 0 ; x--){
				f1 = data[0].x;
				f2 = data[1].x;
				f3 = data[obuf->x + 1].x;
				f4 = data[obuf->x + 2].x;
				xmin = MIN4(f1, f2, f3, f4);
				xmax = MAX4(f1, f2, f3, f4);
				f1 = data[0].y;
				f2 = data[1].y;
				f3 = data[obuf->x + 1].y;
				f4 = data[obuf->x + 2].y;
				ymin = MIN4(f1, f2, f3, f4);
				ymax = MAX4(f1, f2, f3, f4);
				*rect++ = areasample(ibuf, xmin, ymin, xmax, ymax);
				data++;
			}
			data++;
		}
	}
	etijd = times(&voidbuf);
	/*printf("Time: %d\n", etijd - stijd);*/
	lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);
}


void morph(struct ImBuf *obuf, struct ImBuf *ibuf, float fac1, float fac2)
{
	struct morphdata * data, *_data;
	struct morphdata * smalldata, *_smalldata;
	struct morphvec * mvec;
	struct morphline in, out;
	long x, y, count = 0;
	short smallx, smally;
	float f1, f2, f3, f4;
	char title[128];
	long dofields =  FALSE;
	

	if (fields) {
		if ((2 * ibuf->y) < ibuf->x) dofields =  TRUE;
	}

	mvec = morphbase->first;
	while (mvec){
		count++;
		mvec = mvec->next;
	}
	
	smallxres = smallyres = smallres;
	if (dofields) smallxres *= 2;

	/* waarom gaat smallres == 3 fout bij + 1 ?? */
	if (smallxres > 1){
		smallx = (obuf->x / smallxres) + 2;
		smally = (obuf->y / smallyres) + 2;
	}else{
		smallx = obuf->x + 1;
		smally = obuf->y + 1;
	}
	
	_data = callocstructN(struct morphdata, (obuf->x + 1) * (obuf->y + 1), "morphdata");
	_smalldata = callocstructN(struct morphdata, smallx * smally, "smalldata");
	
	mvec = morphbase->first;
	while (mvec){
		sprintf(title, "%s %d lines to go", titlehead, count--);
		winset(outwin);
		wintitle(title);
		
		for (x = 0; x <= 1; x++){
			f1 = mvec->in.co[x][0] * obuf->x;
			f2 = mvec->out.co[x][0] * obuf->x;
			if (dofields) {
				f1 /= 2.0;
				f2 /= 2.0;
			}
			in.co[x][0] = f1 + fac1 * (f2 - f1);
			out.co[x][0] = f1 + fac2 * (f2 - f1);

			f1 = mvec->in.co[x][1] * obuf->y;
			f2 = mvec->out.co[x][1] * obuf->y;
			in.co[x][1] = f1 + fac1 * (f2 - f1);
			out.co[x][1] = f1 + fac2 * (f2 - f1);
		}
		
		xy_to_xy(&in, &out, smallx * smallres, smally * smallres, _smalldata, smallres);
		mvec = mvec->next;
	}

	/* coordinaten berekenen */

	sprintf(title, "%s building image", titlehead);
	winset(outwin);
	wintitle(title);

	/* hoe lossen we dit op ? */
	
	smalldata = _smalldata;
	for (y = 0; y < smally; y++){
		for (x = 0; x < smallx; x++){
			if (smalldata->sum != 0.0){
				smalldata->x = smalldata->x / smalldata->sum;
				smalldata->y = smalldata->y / smalldata->sum;
			} else {
				smalldata->x = smallres * x;
				smalldata->y = smallres * y;
			}
			smalldata++;
		}
	}
	
	if (dofields) {
		smalldata = _smalldata;
		for (y = 0; y < smally; y++){
			for (x = 0; x < smallx; x++){
				smalldata->x *= 2.0;
				smalldata++;
			}
		}
	}
	
	if (smallres > 1){
		smalltolarge(_data, _smalldata, obuf->x + 1,  obuf->y + 1, smallx);
		samples_to_rect(obuf, ibuf, _data);
	} else {
		samples_to_rect(obuf, ibuf, _smalldata);
	}
	
	freeN(_data);
	freeN(_smalldata);
}


void interpolate(struct ImBuf *obuf1, struct ImBuf *ibuf1, struct ImBuf *ibuf2, float factor)
{
	struct ImBuf * obuf2;
	struct morphvec * mvec, *mvecn;
	extern rectfill();
	extern rectblend();
	char title[100];
	
	/* nieuwe manier van interpoleren: lijnstukken worden geinterpoleerd */

	rectoptot(obuf1, 0, rectfill, 0);
	obuf2 = dupImBuf(obuf1);

	/* factor = 0: plaatje 1
     * factor = 1: plaatje 2
     */

	strcpy(titlehead, "Image 1: ");
	morph(obuf1, ibuf1, 0.0, factor);

	strcpy(titlehead, "Image 2: ");
	morph(obuf2, ibuf2, 1.0, factor);

	winset(outwin);
	wintitle("Blending");

	sprintf(title, "Out %.1f%%", 100.0 * factor);

	if (factor < 0.0) factor = 0.0;
	else if (factor > 1.0) factor = 1.0;

	factor = ease(factor);
	rectoptot(obuf1, obuf2, rectblend, *((long *) &factor));
	freeImBuf(obuf2);
	
	wintitle(title);
}


void newimg(struct ImBuf * ibuf, char * name)
{
	struct ImBuf * tbuf;
	extern rectcpy();
	
	tbuf = loadiffname(name, IB_rect);
	if (tbuf) {
		rectoptot(ibuf, tbuf, rectcpy);
		strcpy(ibuf->name, name);
		freeImBuf(tbuf);
	} else strcpy(name, ibuf->name);
}

main(argc,argv)
long argc;
char **argv;
{
	long event, actwin, i, j, framecount = 10;
	struct morphvec * mvec, *msel = 0, *mselo = 0;
	extern rectfill();
	short val, go = TRUE;
	extern rectblend();
	char in1[128], in2[128], out[128], name[128];
	struct ImBuf * tbuf;
	extern rectcpy();
	float fac;
	
	strcpy(filename, "/data/morph/");

	if (argc == 1){
		ibuf1 = loadiffname("/data/irisdata/codim/klaar/pano4.tga",LI_rect);
	} else {
		ibuf1 = loadiffname(argv[1],LI_rect);
		strcpy(filename, argv[1]);
		if (argc > 2){
			ibuf2 = loadiffname(argv[2],LI_rect);
		}
	}
	
	if (ibuf1 == 0){
		printf("nopic\n");
		exit(0);
	}
	if (ibuf2 == 0){
		ibuf2 = dupImBuf(ibuf1);
	} else{
		scaleImBuf(ibuf2, ibuf1->x, ibuf1->y);
	}

	prefsize(ibuf1->x,ibuf1->y);
	inwin1 = winopen("In 1");
	RGBmode();
	gconfig();
	lrectwrite(0, 0, ibuf1->x-1, ibuf1->y-1, ibuf1->rect);
	ortho2(0.0, 1.0, 0.0, 1.0);
	
	prefsize(ibuf1->x,ibuf1->y);
	inwin2 = winopen("In 2");
	RGBmode();
	gconfig();
	lrectwrite(0, 0, ibuf2->x-1, ibuf2->y-1, ibuf2->rect);
	ortho2(0.0, 1.0, 0.0, 1.0);

	prefsize(ibuf1->x,ibuf1->y);
	outwin = winopen("Out");
	RGBmode();
	gconfig();

	obuf = dupImBuf(ibuf1);
	rectoptot(obuf, 0, rectfill, 0);
	lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);

	/*obuf->ftype = TGA;
	freecmapImBuf(obuf);
	obuf->depth = 32;*/

	add_outline();
	

/*	makesumtable(ibuf1);
	makesumtable(ibuf2);	
*/	
	qdevs();

	actwin = outwin;

	sysmips(MIPS_FPSIGINTR, 1);
	if (argc > 3) {
		strcpy(filename, argv[3]);
		load(0);
		draw_all();
	}
	
	strcpy(in1, ibuf1->name);
	strcpy(in2, ibuf2->name);
	strcpy(out, ibuf2->name);

	while(go){
		fflush(stdout);
		event = qreadN(&val);
		switch(event){
		case KEYBD:
			switch(val){
			case '!':
				newname(in1, -1);
				newname(in2, -1);			
			case '@':
				if (val != '!'){
					newname(in1, +1);
					newname(in2, +1);			
				}
				newimg(ibuf1, in1);
				newimg(ibuf2, in2);
				qenter(REDRAW, inwin1);
				break;
			case '1':
				smallres = 1;
				break;
			case '2':
				smallres = 2;
				break;
			case '3':
				smallres = 3;
				break;
			case '4':
				smallres = 4;
				break;
			case '5':
				smallres = 5;
				break;
			case '6':
				smallres = 6;
				break;
			case '7':
				smallres = 8;
				break;
			case '8':
				smallres = 10;
				break;
			case '9':
				smallres = 12;
				break;
			case ' ':
				interpolate(obuf, ibuf1, ibuf2, frame / 20.0);
				winset(outwin);
				lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);
				break;
			case 'a':
				anim = !anim;
				printf("anim: %d\n", anim);
				break;
			case 'f':
				fast = !fast;
				printf("fast: %d\n", fast);
				break;
			case 'F':
				fields = !fields;
				printf("fields: %d\n", fields);
				break;
			case 'k':
				winset(inwin1);
				lrectwrite(0, 0, ibuf1->x - 1, ibuf1->y - 1, ibuf1->rect);
				winset(inwin2);
				lrectwrite(0, 0, ibuf2->x - 1, ibuf2->y - 1, ibuf2->rect);
				break;
			case 'e':
				if (do_ease) do_ease = 0;
				else do_ease = 1;
				
				printf("do_ease: %d\n", do_ease);
				break;
			case 'E':
				do_ease = -1;
				printf("do_ease: %d\n", do_ease);
				break;
			case 'm':
				if (fields) {
					de_interlace(ibuf1);
					de_interlace(obuf);
				}
				morph(obuf, ibuf1, 0.0, frame / 20.0);
				lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);
				if (fields) {
					interlace(ibuf1);
					interlace(obuf);
				}
				break;
			case 'M':
				winset(outwin);
				if (fileselect("Save Image", out)) {
					char int1[128], int2[128];
					
					strcpy(int1, in1);
					strcpy(int2, in2);
					
					for (i = 0; i <= framecount; i++) {
					
						fac = ((float) i) / framecount;
						fac = fac + do_ease * (ease(fac) - fac);
						
						if (fields) {
							de_interlace(ibuf1);
							de_interlace(ibuf2);
							de_interlace(obuf);

							interpolate(obuf, ibuf1, ibuf2, fac);
							
							if (i != framecount) {
								fac = ((float) i + 0.5) / framecount;
								fac = fac + do_ease * (ease(fac) - fac);
							}
							ibuf1->rect += ibuf1->y * ibuf1->x;
							ibuf2->rect += ibuf2->y * ibuf2->x;
							obuf->rect += obuf->y * obuf->x;
							
							interpolate(obuf, ibuf1, ibuf2, fac);

							ibuf1->rect -= ibuf1->y * ibuf1->x;
							ibuf2->rect -= ibuf2->y * ibuf2->x;
							obuf->rect -= obuf->y * obuf->x;

							interlace(ibuf1);
							interlace(ibuf2);
							interlace(obuf);							
						} else {
							interpolate(obuf, ibuf1, ibuf2, fac);
						}
						lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);
						saveiff(obuf, out, SI_rect);
						newname(out, +1);
						if (anim) {
							newname(int1, +1);
							newimg(ibuf1, int1);
							newname(int2, +1);
							newimg(ibuf2, int2);							
						}
					}
					if (anim) {
						newimg(ibuf1, in1);
						newimg(ibuf2, in2);						
					}
				}
				break;
			case 'o':
				add_outline(ibuf1->x - 1, ibuf1->y - 1);
				break;
			case 'S':
				swap_lines();
				qenter(REDRAW, 0);
				break;
			case 's':
				swap_all();
				strcpy(name, in1);
				strcpy(in1, in2);
				strcpy(in2, name);
				qenter(REDRAW, 0);
				break;
			case 'w':
				if (fileselect("Save Image", out)) {
					saveiff(obuf, out, SI_rect);
				}
				break;
			case 'x':
				if (msel && actwin != outwin){
					draw_clear(msel);
					remlink(morphbase, msel);
					freeN(msel);
					mselo = msel = 0;
				}
				break;
			}
			break;
		case LEFTARROWKEY:
			if (val) {
				frame --;
				printf("\r %d%%  \r", frame * 5); 
			}
			break;
		case RIGHTARROWKEY:
			if (val) {
				frame ++;
				printf("\r %d%%  \r", frame * 5); 
			}
			break;
		case UPARROWKEY:
			if (val) {
				framecount ++;
				printf("\r framecount: %d  \r", framecount); 
			}
			break;
		case DOWNARROWKEY:
			if (val) {
				if (framecount > 1) framecount --;
				printf("\r framecount: %d  \r", framecount); 
			}
			break;
		case LEFTMOUSE:
			if (val && actwin != outwin){
				mvec = getline();
				if (mvec) {
					draw_normal(msel);
					msel = mselo = mvec;
					draw_select(msel);
				}
			}
			break;
		case MIDDLEMOUSE:
			if (val && actwin != outwin){
				winset(actwin);
				move_mvec(msel);
			}
			break;
		case MOUSEX:
		case MOUSEY:
			event = qtest();
			if (event != MOUSEX && event != MOUSEY && actwin != outwin){
				winset(actwin);
				if (actwin == inwin1) msel = select_mvec(0);
				else msel = select_mvec(1);

				if (msel != mselo){
					draw_normal(mselo);
					draw_select(msel);
					mselo = msel;
				}
			}
			break;
		case REDRAW:
			while (qtest() == REDRAW) event = qread(&val);
			winset(inwin2);
			lrectwrite(0, 0, ibuf2->x-1, ibuf2->y-1, ibuf2->rect);
			/*ortho2(0.0, 1.0, 0.0, 1.0);*/
			winset(inwin1);
			lrectwrite(0, 0, ibuf1->x-1, ibuf1->y-1, ibuf1->rect);
			/*ortho2(0.0, 1.0, 0.0, 1.0);*/
			winset(outwin);
			lrectwrite(0, 0, obuf->x-1, obuf->y-1, obuf->rect);
			/*ortho2(0.0, 1.0, 0.0, 1.0);*/
			draw_all();
			break;
		case INPUTCHANGE:
			actwin = val;
			break;
		case WINSHUT:
		case WINQUIT:
			go = FALSE;
			break;
		case F1KEY:
			if (val){
				load(1);
				draw_all();
				mselo = 0;
			}
			break;
		case F2KEY:
			if (val) save();
			break;
		case F3KEY:
			if (val) {
				if (fileselect("Save Image", out)) {
					saveiff(obuf, out, SI_rect);
				}
			}
			break;
		}
	}
	gexit();
	exit(0);
}

