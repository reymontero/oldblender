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

#include <stdlib.h>			/* voorkomt dat je bij malloc type moet aangeven */
#include <gl/gl.h>
#include <gl/device.h>
#include <local/iff.h>
#include <local/util.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <ctype.h>
#include <sys/mman.h> /* mapped memory */


/*

bindkeyo -r f1,'cc -O3  rt.c  -o rt \n'
bindkeyo -r f2,'rt \n'

*/

float v1[] = {1.0 , 12.0 , 4.0};
float v2[] = {1.0 , 6.0 , 3.0};
float v3[] = {1.0 , 2.0 , 2.0};
float v4[] = {1.0 , 0.0 , 1.0};
/*
float v1[] = {1.0 , 12.0 , 4.0};
float v2[] = {1.0 , 6.0 , 3.0};
float v3[] = {1.0 , 2.0 , 2.0};
float v4[] = {2.0 , 0.0 , 1.0};
*/



short kramer33(v1,v2,v3,v4,vec)
float *v1,*v2,*v3,*v4,*vec;
{
	double x0,x1,x2,t00,t01,t02,t10,t11,t12,t20,t21,t22;
	double m0,m1,m2,deeldet,det1,det2,det3;
	double x,y,z;

	/* vergelijkingen zijn:

	v1[0] * x + v2[0] * y + v3[0] * z = v4[0]
	v1[1] * x + v2[1] * y + v3[1] * z = v4[1]
	v1[2] * x + v2[2] * y + v3[2] * z = v4[2]

	*/

	t00= v1[0];
	t01= v1[1];
	t02= v1[2];
	t10= v2[0];
	t11= v2[1];
	t12= v2[2];
	t20= v3[0];
	t21= v3[1];
	t22= v3[2];
	
	x0=t11*t22-t12*t21;
	x1=t12*t20-t10*t22;
	x2=t10*t21-t11*t20;

	deeldet=t00*x0+t01*x1+t02*x2;
	if(deeldet==0.0) return -1;
	
	m0= v4[0];
	m1= v4[1];
	m2= v4[2];
	det1=m0*x0+m1*x1+m2*x2;
	x= det1/deeldet;

	det2=t00*(m1*t22-m2*t21);
	det2+=t01*(m2*t20-m0*t22);
	det2+=t02*(m0*t21-m1*t20);
	y= det2/deeldet;

	det3=t00*(t11*m2-t12*m1);
	det3+=t01*(t12*m0-t10*m2);
	det3+=t02*(t10*m1-t11*m0);
	z=det3/deeldet;

	vec[0]= x;
	vec[1]= y;
	vec[2]= z;

	return 0;
}


main(argc,argv)
long argc;
char **argv;
{
	float ret[3];


	kramer33(v1,v2,v3,v4,ret);
	printf("%f %f %f\n",ret[0],ret[1],ret[2]);
}

