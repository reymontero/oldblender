
/* usiblender.c  jan 94		GRAPHICS
 * 
 * 
 * 
 */

#include "blender.h"
#include "graphics.h"


void exitblender()
{
	
	freescreens();	/* window.c */

	if(totblock!=0) {
		printf("Error Totblck: %d\n",totblock);
		printmemlist();
	}
	gexit();
	
	exit(0);
}
