/* graphics.h    dec 93 */


#include <gl/gl.h>
#include <gl/device.h>
#include <local/objfnt.h>
#include <fmclient.h>


#define LINE2S(v1, v2)	{bgnline(); v2s((short *)(v1)); v2s((short *)(v2)); endline();}
#define LINE3S(v1, v2)	{bgnline(); v3s((short *)(v1)); v3s((short *)(v2)); endline();}
#define LINE2I(v1, v2)	{bgnline(); v2i((int *)(v1)); v2i((int *)(v2)); endline();}
#define LINE3I(v1, v2)	{bgnline(); v3i((int *)(v1)); v3i((int *)(v2)); endline();}
#define LINE2F(v1, v2)	{bgnline(); v2f((float *)(v1)); v2f((float *)(v2)); endline();}
#define LINE3F(v1, v2)	{bgnline(); v3f((float *)(v1)); v3f((float *)(v2)); endline();}

typedef struct vec2s {
	short x, y;
} vec2s;

typedef struct vec2i {
	int x, y;
} vec2i;

typedef struct vec2f {
	float x, y;
} vec2f;

typedef struct vec2d {
	double x, y;
} vec2d;

typedef struct vec3i {
	int x, y, z;
} vec3i;

typedef struct vec3f {
	float x, y, z;
} vec3f;

typedef struct vec3d {
	double x, y, z;
} vec3d;

typedef struct vec4i {
	int x, y, z, w;
} vec4i;

typedef struct vec4f {
	float x, y, z, w;
} vec4f;

typedef struct vec4d {
	double x, y, z, w;
} vec4d;

typedef struct rcti {
    int xmin, xmax;
    int ymin, ymax;
} rcti;
