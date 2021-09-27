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

#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <vfr/vframer.h> 
#include <string.h>			/* strcpy etc.. */
#include <ctype.h>			/* isdigit */
#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h> 
#include <local/iff.h>
#include <local/vfrutil.h>
#include <errno.h>

ulong yuvbars[] = {
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,
0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80, 0x80ff80,

0x7dfd80, 0x7cfd80, 0x7bfc80, 0x7bfc80, 0x7bfb80, 0x79fa81, 0x73fa82, 0x67f583, 0x58e286,
0x47c689, 0x38b28b, 0x2dad8d, 0x27ad8e, 0x25ac8f, 0x24ab8f, 0x24ab8f, 0x24ab8f, 0x23ab8f,

0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,
0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90, 0x20aa90,

0x24a98c, 0x25a98b, 0x25a98a, 0x26a98a, 0x26a88a, 0x29a887, 0x31a880, 0x40a673, 0x549e61,
0x6b924e, 0x7f8a3c, 0x8e882f, 0x968828, 0x998825, 0x9a8725, 0x9a8725, 0x9a8724, 0x9b8723,

0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,
0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720, 0xa08720,

0x9c8620, 0x9c8621, 0x9b8621, 0x9b8621, 0x9b8621, 0x988521, 0x928522, 0x878424, 0x787f27,
0x677829, 0x58732c, 0x4d722e, 0x47722f, 0x44712f, 0x44712f, 0x44712f, 0x44712f, 0x43712f,

0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,
0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130, 0x407130,

0x447035, 0x457036, 0x457037, 0x467037, 0x466f37, 0x496f3b, 0x516f45, 0x606d58, 0x746571,
0x8b5a8e, 0x9f52a7, 0xae50ba, 0xb650c4, 0xb950c8, 0xb94fc8, 0xba4fc8, 0xba4fc9, 0xbb4fca,

0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,
0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0, 0xc04fd0,

0xbc4ed0, 0xbb4ed0, 0xbb4ed0, 0xbb4ed0, 0xbb4ed0, 0xb84dd0, 0xb24dd1, 0xa74cd3, 0x9847d6,
0x8740d8, 0x783bdb, 0x6d3add, 0x673ade, 0x6439de, 0x6439de, 0x6439de, 0x6339de, 0x6339df,

0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,
0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0, 0x6039e0,

0x6438dc, 0x6538db, 0x6538da, 0x6538da, 0x6637da, 0x6937d7, 0x7137d0, 0x8035c3, 0x942db1,
0xab219e, 0xbf198c, 0xce177f, 0xd61778, 0xd91775, 0xd91675, 0xda1675, 0xda1674, 0xdb1673,

0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,
0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670, 0xe01670,

0xdc1570, 0xdb1570, 0xdb1570, 0xdb1570, 0xda1570, 0xd81571, 0xd21572, 0xc71374, 0xb80e76,
0xa70779, 0x98027c, 0x8c017d, 0x86017e, 0x84017f, 0x84007f, 0x84007f, 0x83007f, 0x82007f,

0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080, 0x800080,
};

long barcolors[] = {
    0xffffff, 0x00c0c0, 0xc0c000, 0x00c000, 0xc000c0, 0x0000c0, 0xc00000,0x000000};

VFR_DEV *vfr = 0;
long vlan = 0;
long vfr_verbose = 0;
int vlan_lock = -1;
char * lock_name = "/tmp/vlan_lock";
long bypass_sginap = FALSE;

int vlan_verbose(int new)
{
	int old;
	
	old = vfr_verbose;
	vfr_verbose = new;

	return(old);
}

long escaped()
{
	long esc = FALSE;
	short val;

	while(qtest()){
		if (qread(&val) == ESCKEY)
			esc = TRUE;
	}

	return(esc);
}


void vfr_getimage(struct ImBuf * ibuf,long flags)
{
	long x, y, i,back,left=0,right=0,top=0,bot=0;
	ulong *rect , *fbuf;
	extern void rectfill();
	
	if (vfr == 0) return;
	if (ibuf == 0) return;

	fbuf=vfr->pframe_buffer;
	rect =(ulong *) ibuf->rect;

	if (rect == 0){
		fprintf(stderr,"vfr_getimage no rect\n");
		return;
	}
	
	x = ibuf->x ;
	y = ibuf->y;

	if (flags & (1<<TOYUV)) rectoptot(ibuf, 0, rectfill, 0x800080);
	else rectoptot(ibuf, 0, rectfill, 0);
	
	if (x != 720){
		if (ibuf->x > 720){
			rect += (x - 720) >> 1;
		} else{
			left = (720 - ibuf->x) >> 1;
			right = 720 - ibuf->x- left;
		}
	}
	if (y != 576){
		if (ibuf->y > 576){
			rect += x * ((y - 576) >> 1);
			y = 576;
		} else{
			top = (576 - ibuf-> y) >> 1;
			bot = 576 - ibuf->y - top;
		}
	}
	
	rect += x * (y-1);

	if (flags & (1 << SHIFTBIT)){
		top ++;
		if (bot) bot--;
		else y --;
	}

	fbuf += 1024 * top;

	for (;y>0;y--){
		x = left;

		for (i = 720 - (left + right); i > 0 ; i--){
			rect[x-left] = fbuf[x];
			x++;
		}

		fbuf += 1024;
		rect -= ibuf->x;
	}
}


void vfr_rgb(struct ImBuf * ibuf, long flags)
{
	long x, y, i, back, left=0, right=0, top=0, bot=0, mode, xres;
	ulong *rect , *fbuf;

	if (vfr == 0) return;
	if (ibuf == 0) return;

	fbuf=vfr->pframe_buffer;
	xres = vfr_map_mode_to_active_width(VFR_DEVICE_MODE(*vfr));
	mode = vfr->mode;
	
	if (mode == VFR_RGB_625 | (mode == VFR_RY_625 && (flags & (1 << TOYUV)))) {
		mode = 0;
	} else {
		mode = 1;
		fbuf = vfr->pshadow_buffer;
	}
	
	rect =(ulong *) ibuf->rect;
	if (rect == 0){
		fprintf(stderr,"vfr_rgb no rect\n");
		return;
	}
	x = ibuf->x ;
	y = ibuf->y;

	back = rect[0];

	if (rect[(x * y) - 1] != back || rect[x-1] != back || rect[x * (y-1)] !=back){
		if (flags & (1<<TOYUV)) back = 0x800080;
		else back = 0;
	}

	if (x != xres){
		if (ibuf->x > xres){
			rect += (x - xres) >> 1;
		} else{
			left = (xres - ibuf->x) >> 1;
			right = xres - ibuf->x- left;
			/* printf("l:%d r:%d\n",left,right); */
		}
	}
	if (y != 576){
		if (ibuf->y > 576){
			rect += x * ((y - 576) >> 1);
			y = 576;
		} else{
			top = (576 - ibuf-> y) >> 1;
			bot = 576 - ibuf->y - top;
		}
	}

	rect += x * (y-1);

	if (flags & (1 << SHIFTBIT)){
		top ++;
		if (bot) bot--;
		else y --;
	}

	for(; top>0 ; top--){
		for (i = 0 ; i<xres ; i++) fbuf[i] = back;
		fbuf += 1024;
	}
	for (;y>0;y--){
		x = 0;
		for (i = left ; i>0 ; i--) fbuf[x++] = back;

		for (i = xres - (left + right); i>0 ; i--){
			fbuf[x] = rect[x-left];
			x++;
		}
		for (i = right ; i>0 ; i--) fbuf[x++] = back;

		fbuf += 1024;
		rect -= ibuf->x;
	}
	for(; bot>0 ; bot--){
		for (i = 0 ; i<xres ; i++) fbuf[i] = back;
		fbuf += 1024;
	}
	
	if (mode) vfr_convert_from_rgb (vfr, xres);
}


void colorbars_oud(long flags)
{
	struct ImBuf *ibuf;
	long i,num,x,y,col;
	uchar * rect;
	extern void rectfill();
	
	num = sizeof(barcolors) / 4;

	ibuf = allocImBuf(720, 576, 24, 1, 0);
	if (ibuf == 0) return;

	x = ibuf->x / num;

	for (i=0 ; i<num ; i++){
		col = barcolors[i];
		if (flags & (1<<TOYUV)) col = colcspace(col,rgbbeta);
		rectop(ibuf, 0, i * x, 0, 0, 0, x, ibuf->y, rectfill, col);
	}

	filterx(ibuf);

	vfr_rgb(ibuf, flags);
	freeImBuf(ibuf);
}


void colorbars(long flags)
{
	struct ImBuf *ibuf, *fbuf = 0;
	long i,num,x,y,col;
	extern void rectcpy();
	extern void rectfill();
	extern void rectalphaover();
	uchar * rect;

	num = sizeof(barcolors) / 4;

	ibuf = allocImBuf(720, 576, 24, 1, 0);
	if (ibuf == 0) return;
	fbuf = loadiffname("/data/pics/overlay", IB_rect);
	
	if (flags & (1<<TOYUV)) {
		memcpy(ibuf->rect, yuvbars, sizeof(yuvbars));
		for (y = 1; y < ibuf->y; y *= 2){
			rectop(ibuf, ibuf, 0, y, 0, 0, 32767, y, rectcpy);
		}
		if (fbuf) {
			rect = (uchar *) fbuf->rect;
			for (i = fbuf->x * fbuf->y; i > 0 ; i--) {
				/* ontpremullen */
				if (rect[0]) rect[0] = 255;
				rect += 4;
			}
			cspace(fbuf, rgbbeta);
		}
	} else{
		x = ibuf->x / num;
		for (i=0 ; i<num ; i++){
			col = barcolors[i];
			rectop(ibuf, 0, i * x, 0, 0, 0, x, ibuf->y, rectfill, col);
		}
		filterx(ibuf);
	}
	if (fbuf){
		rectoptot(ibuf, fbuf, rectalphaover);
		freeImBuf(fbuf);
	}
	vfr_rgb(ibuf, flags);
	freeImBuf(ibuf);
}

void textimage(char *str, long flags)
{
	struct ImBuf *ibuf;
	long i,num,x,y,col;
	char cmd[100];

	sprintf(cmd, "/usr/people/trace/stringtoimage '%s'", str);
	
	system(cmd);

	ibuf = loadiffname("/usr/tmp/0001", IB_rect);
	if(ibuf==0) return;
	
	if (flags & (1<<TOYUV)) cspace(ibuf, rgbbeta);
	
	vfr_rgb(ibuf, flags);
	freeImBuf(ibuf);
}


void black(long flags)
{
	struct ImBuf *ibuf;
	long col;
	extern void rectfill();

	ibuf = allocImBuf(720, 576, 24, 1, 0);
	if (ibuf == 0) return;

	col= 0x000000;
	if (flags & (1<<TOYUV)) col = colcspace(col,rgbbeta);

	rectoptot(ibuf, 0, rectfill, col);
	vfr_rgb(ibuf, flags);
	freeImBuf(ibuf);
}


void inittape(long flags)
{
	long i;

	/* de wachtlussen moeten natuurlijk slimmer, en op de een of andere
     * manier moet de starttijdcode ingesteld kunnen worden.
     */

	black(flags);
	if (escaped()) return;

	vlancmd("RC");

	for (i = 25; i > 0; i--){
		if (escaped()){
			vlancmd("SH0");
			return;
		}
		sginap(100);
	}
	colorbars(flags);
	for (i = 85; i > 0; i--){
		if (escaped()){
			vlancmd("SH0");
			return;
		}
		sginap(100);
	}
	black(flags);
}

void init_vlan_lock()
{
	if (vlan_lock != -1) return;
	vlan_lock = open(lock_name, O_WRONLY);
	if (vlan_lock == -1 && errno == ENOENT) {
		vlan_lock = open(lock_name, O_WRONLY | O_CREAT, 0666);
	}
}


int sginap(long ticks) {
	if (bypass_sginap == FALSE) Sginap(ticks);
}

char *vlancmd(char * command)
{
	char *ret, tmp[128];
	int old_bypass = bypass_sginap;
	
	bypass_sginap = TRUE;
	
	if (vlan){
		if (vlan_lock != -1) lockf(vlan_lock, F_LOCK, 0);
		
/*			if (strcmp(command, "PF") == 0) {	/ * tijdcode zetten als PF aangeroepen wordt * /
				ret = vfr_vlan_cmd(vfr,"RI");	/ * inpunt lezen * /
				sprintf(tmp, "SG %s\n", ret);
				vfr_vlan_cmd(vfr,tmp);			/ * tijdcode gelijk aan inpunt zetten * /
			}
			*/
			ret = vfr_vlan_cmd(vfr,command);
		if (vlan_lock != -1) lockf(vlan_lock, F_ULOCK, 0);
		
		if (strstr(ret,"ERROR")){
			fprintf(stdout,"  WARNING: %s return: %s\n",command,ret);
			fflush(stdout);
		} else{
			if (vfr_verbose > 1) {
				if (ret[0]) {
					fprintf(stdout,"  vlancmd: %s return: %s.\n",command,ret);
				}else{
					fprintf(stdout,"  vlancmd: %s return: OK.\n",command);
				}
			} else if (ret[0] && vfr_verbose) {
				fprintf(stdout,"  vlancmd: %s return: %s.\r",command,ret);
				fflush(stdout);
			}
		}
		bypass_sginap = old_bypass;
		return (ret);
	}
	
	bypass_sginap = old_bypass;
	return("ERROR");
}


long recorder_ready()
{
	long status;

	memcpy(&status,vlancmd("SR"),4);
	switch (status){
	case 'PAUS':
	case 'STOP':
	case 'PLAY':
	case 'FAST':
	case 'REWI':
	case 'SEAR':
		return(TRUE);
		break;
	default:
		return(FALSE);
		break;
	}
}


char * vlantime(char * code, long time)
{
	char cmd[64];

	strcpy(cmd,code);
	strcat(cmd,tcode_to_string(time));
	return (vlancmd(cmd));
}

