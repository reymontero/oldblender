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

#include <stdio.h>
#include <malloc.h>
#include <local/util.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <local/iff.h>


/*

zipfork "cc -g vector.c util.o -lgl_s -limbuf -limage -lm -o vector >/dev/console"                                                        
zipfork "vector /data/logo.obj /data/rt > /dev/console"                                                        

*/

main(int argc, char ** argv)
{
    FILE * fp;
    long verts, i, j, vlakoffset, poly, end, dum, lines, first, pixels;
    long *linedata, *ld, new, prev;
    float *vertdata, *vd, min[3], max[3], mid, mul, co[2];
    float x, y, dx, dy, len, totlen, totpixels;
    char s[100];
    short val;
    struct ImBuf *ibuf;
    extern rectfill();
    char * rect;
    
    if (argc < 3) {
	printf("%s Videoscape.in Targa.out\n", argv[0]);
	exit(1);
    }
    
    fp= fopen(argv[1], "r");
    if(fp==NULL) {
	printf("Can't read %s\n", argv[1]);
	return;
    }

    fscanf(fp, "%40s", s);
    fscanf(fp, "%d\n", &verts);
    if(verts<=0) {
	printf("%d: illegal number of vertices\n", verts);
	exit(1);
    }
    
    vd = vertdata = malloc(sizeof(float) * 3 * verts);

    min[0]= min[1]= min[2]= 1.0e20;
    max[0]= max[1]= max[2]= -1.0e20;
    for(i=0;i<verts;i++) {
	fscanf(fp, "%f %f %f", vd, vd+1, vd+2);
	for(j=0; j<3; j++) {
	    min[j]= MIN2(min[j], vd[j]);
	    max[j]= MAX2(max[j], vd[j]);
	}
	vd+=3;
    }
    
    /* de vlakken */

    vlakoffset = ftell(fp);
    lines = 0;
    do{
	end = fscanf(fp,"%d", &poly);
	if(end <= 0) break;
	lines += poly;
	
	for(i=0;i<poly;i++) {
	    end = fscanf(fp,"%f", &dum);
	    if (end <= 0) break;
	}
	
	end= fscanf(fp,"%d", &dum); /* kleur */
    }while (end > 0);
    
    fseek(fp, vlakoffset, SEEK_SET);
    ld = linedata = malloc((lines + 1) * 2 * sizeof(long));
    
    do{
	end = fscanf(fp,"%d", &poly);
	if(end <= 0) break;
	
	for(i = 0;i < poly; i++) {
	    end = fscanf(fp,"%d", &new);
	    if (end <= 0) break;

	    if (i == 0) first = new;
	    else {
		*ld ++ = prev;
		*ld ++ = new;
	    }
	    prev = new;
	}
	*ld ++ = prev;
	*ld ++ = first;
	
	end= fscanf(fp,"%d", &dum); /* kleur */
    }while (end > 0);
    
    fclose(fp);
    
    /* centreren */
    for (i = 0; i < 3; i++){
	mid = (max[i] + min[i]) / 2.0;
	mul = max[i] - min[i];
	if (mul != 0.0){
	    mul = 1.5 / mul;
	    vd = vertdata;
	    vd += i;
	    for (j = 0; j < verts; j++){
		vd[0] = mul * (vd[0] - mid);
		vd += 3;
	    }
	}
    }
    
    /* totale lijn lengte berekenen */
    
    ld = linedata;
    totlen = 0.0;
    for(i = 0; i < lines; i++){
	dx = vertdata[3 * ld[0] + 0] - vertdata[3 * ld[1] + 0];
	dy = vertdata[3 * ld[0] + 1] - vertdata[3 * ld[1] + 1];
	totlen += sqrtf(dx * dx + dy * dy);
	ld += 2;
    }
    printf("totlen: %f\n", totlen);

    ibuf = allocImBuf(700, 550, 24, 1, 0);
    rectoptot(ibuf, 0, rectfill, 0x808080);
    totpixels = (ibuf->x * ibuf->y);
    mul = totpixels / totlen;
    
    rect = (char *) ibuf->rect;
    
    ld = linedata;
    for (i = 0; i < lines; i++){
	x = vertdata[3 * ld[0] + 0];
	y = vertdata[3 * ld[0] + 1];
	dx = vertdata[3 * ld[1] + 0] - x;
	dy = vertdata[3 * ld[1] + 1] - y;	
	len = sqrtf(dx * dx + dy * dy);

	pixels = len * mul;
	dx /= pixels;
	dy /= pixels;
	
	for (j = pixels; j > 0; j--){
	    rect[1] = 128.0 * x + 128.5;
	    rect[3] = 128.0 * y + 128.5;
	    x += dx;
	    y += dy;
	    rect += 4;
	}
	ld += 2;
    }
    
    saveiff(ibuf, argv[2], SI_rect);
    
    prefsize(200, 200);
    winopen("vect");
    color(0);
    clear();
    color(7);
    ortho2(-1.0, 1.0, -1.0, 1.0);
    
    ld = linedata;
    for(i = 0; i < lines; i++){
	bgnline();
	    v2f(vertdata + 3 * ld[0]);
	    v2f(vertdata + 3 * ld[1]);
	    ld += 2;
	endline();
    }
    
    while (qread(&val));
}

