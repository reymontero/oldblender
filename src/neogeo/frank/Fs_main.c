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

#include "Fs.h"

int main(int argc, char *argv[])
{
   FD_Fs *fd_Fs;

   fl_initialize(&argc, argv, 0, 0, 0);
   fd_Fs = create_form_Fs();

   /* fill-in form initialization code */

   /* show the first form */
   fl_show_form(fd_Fs->Fs,FL_PLACE_CENTER|FL_FREE_SIZE,FL_FULLBORDER,"Fs");
   fl_do_forms();
   return 0;
}

