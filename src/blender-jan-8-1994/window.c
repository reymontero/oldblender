
/*  window.c   jan 94     GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"
#include "screen.h"


main()
{
	ScrArea *sa;
	ScrEdge *se;
	float fac;
	long wx, wy, orx, ory;
	short event, val;
	
	initscreen();
	
	screenmain();
	
}
