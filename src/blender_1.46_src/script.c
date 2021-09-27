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

/* object.c  		MIXED MODEL

 * 
 * jan 95
 * 
 */

#include "blender.h"
#include "screen.h"
#include "edit.h"
#include "ipo.h"
#include "render.h"


#include <stdio.h>

char * get_next_line(char * buf, long buf_size, FILE *fd) {
	char string[1024], *comment;
	
	while (fgets(buf, buf_size, fd)) {
		comment = strchr(buf, '#');
		if (comment) {
			comment[0] = 0;
			comment++;
		}
		/* staat er wel iets of is het een lege regel */
		
		if (sscanf(buf, "%1s", string) > 0) {
			/*printf("found :%s # %s\n", buf, comment);*/
			return(buf);
		}
	}
	
	return (0);
}

do_script(char * dir)
{
	Object * object;
	struct Main *main;
	long count = 1;
	FILE * script;
	char name[1024], string[1024];
	double x, y, z;
	
	main= G.mainbase.first;	


	/* alle bases resetten */
	object = main->object.first;
	
	while (object){
		if (object->id.name[2] == '*' && object->id.name[strlen(object->id.name) - 4] == '.') {
			object->loc[0] = 0;
			object->loc[1] = 0;
			object->loc[2] = 0;
		}
		object = object->id.next;
	}
		
	
	/*sprintf(name, "%s%0.4d", dir, CFRA);*/
	sprintf(name, "/render/scripts/%0.4d", CFRA);
	
	if (R.flag & R_SEC_FIELD) strcat(name, "_");
	
	script = fopen(name, "r");
	if (script != NULL ) {
		while (get_next_line(string, sizeof(string), script)) {
			if (sscanf(string, "%s%lf%lf%lf", name, &x, &y, &z) == 4) {
				/*printf("%s %f %f %f\n", name, x, y, z);*/
				
				object = main->object.first;
				
				while (object){
					if (strcmp(name, object->id.name +3) == 0) {
						object->loc[0] = x;
						object->loc[1] = y;
						object->loc[2] = z;
						break;
					}
					object = object->id.next;
				}
			}
		}
	}
	
	fclose(script);
}

