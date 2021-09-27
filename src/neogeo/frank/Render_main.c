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

#include "renderd.h"

#include "forms.h"
#include "Render.h"
#include <netdb.h>
#include <math.h>

char lastfile[MAXLINELEN] = "/data/";
FD_Render *fd_Render;

Render * select_render;
System * select_system;
extern long portnumber;
extern char myname[];
int autoselect = TRUE, ignore_next_select = FALSE;

void alphasort_list (ListBase * list)
{
	ListBase sortlist = {0, 0};
	Header * header, * sort;
	
	while (list->first) {
		header = list->first;
		remlink(list, header);
		sort = sortlist.first;

		while (sort) {
			if (strcmp(sort->infofile, header->infofile) > 0) break;
			sort = sort->next;
		}
		
		if (sort) insertlink(&sortlist, sort->prev, header);
		else addtail(&sortlist, header);
	}
	
	list->first = sortlist.first;
	list->last = sortlist.last;
}

void priority_sort_list (ListBase * list)
{
	ListBase sortlist = {0, 0};
	Render * header, * sort;
	
	while (list->first) {
		header = list->first;
		remlink(list, header);
		sort = sortlist.first;

		while (sort) {
			if (header->pri_int > sort->pri_int) break;
			else if (header->pri_int == sort->pri_int) {
				if (header->time_int < sort->time_int) break;
			}
			sort = sort->next;
		}
		
		if (sort) insertlink(&sortlist, sort->prev, header);
		else addtail(&sortlist, header);
	}
	
	list->first = sortlist.first;
	list->last = sortlist.last;
}

int check_ready()
{
	Render * render;
	int redraw = FALSE;
	
	render = renderlist->first;
	while (render) {
		if ((render->done_int + render->error_int) == render->total_int) {
			if (render->beep) {
				render->beep = FALSE;
				redraw = TRUE;
				fl_ringbell(100);
				fl_check_forms();
				Sginap(15);
				Sginap(15);
				fl_ringbell(100);
			}
			if (render->ready == FALSE) {
				redraw = TRUE;
				render->ready = TRUE;
			}
		} else {
			if (render->ready == TRUE) {
				redraw = TRUE;
				render->ready = FALSE;
			}
		}
		render = render->next;
	}
	
	return(redraw);
}

int check_wait()
{
	System * system;
	int redraw = FALSE;
	
	system = systemlist->first;
	while (system) {
		if (system->status_int & (WAIT | OFFLINE)) {
			if (system->wait == FALSE) {
				redraw = TRUE;
				system->wait = TRUE;
			}
		} else {
			if (system->wait == TRUE) {
				redraw = TRUE;
				system->wait = FALSE;
			}
		}
		system = system->next;
	}
	
	return (redraw);
}

void add_string_width (ListBase * list)
{
	Header * header, * largest = 0;

	header = list->first;
	largest = header;
	
	while (header) {
		header->largest = FALSE;
		header->largest = TRUE;
		header->string_width = fl_get_string_width(FL_NORMAL_STYLE, FL_NORMAL_FONT, header->string, strlen(header->string));
		if (header->string_width > largest->string_width) largest = header;
		
		header = header->next;
	}
	
	if (largest) largest->largest = TRUE;
}

void set_render_strings ()
{
	Render * render;
	int len;
	
	render = renderlist->first;
	while (render) {
		
		if (render->file) {
			strcpy(render->string, render->file->buf + 2);
			len = strlen(render->string);
			if (render->string[len - 1] == 10) render->string[len - 1] = 0;
			if (render->scene) {
				strcat(render->string, " ~ ");
				strcat(render->string, render->scene->buf + 2);
			}
		} else strcpy(render->string, render->infofile);
		
		len = strlen(render->string);
		if (render->string[len - 1] == 10) render->string[len - 1] = 0;
		
		render = render->next;
	}
	
	add_string_width(renderlist);
}


void sort_renderlist (ListBase * list)
{
	ListBase sortlist = {0, 0};
	Render * render, * sort, * ready = 0;
	
	priority_sort_list(list);
	
	while (list->first) {
		render = list->first;
		remlink(list, render);
		
		if (render->ready) {
			if (ready) {
				insertlink(&sortlist, ready, render);
			} else {
				addhead(&sortlist, render);
			}
			ready = render;
		} else {
			addtail(&sortlist, render);
		}
	}
	
	list->first = sortlist.first;
	list->last = sortlist.last;
	
}

void sort_systemlist (ListBase * list)
{
	ListBase sortlist = {0, 0};
	System * system, * sort, * wait = 0;
	
	alphasort_list(list);
	
	while (list->first) {
		system = list->first;
		remlink(list, system);
		
		if (system->wait) {
			if (wait) {
				insertlink(&sortlist, wait, system);
			} else {
				addhead(&sortlist, system);
			}
			wait = system;
		} else {
			addtail(&sortlist, system);
		}
	}
	
	list->first = sortlist.first;
	list->last = sortlist.last;
	
}

void set_select()
{
	int cur_line, total_lines, total_select, pri_line;
	Render * render, * highpri = 0;
	System * system;
	const char * txt;
	
	total_lines = fl_get_browser_maxline(fd_Render->Animations);
	total_select = 0;
	
	render = renderlist->first;
	for (cur_line = 1; cur_line <= total_lines; cur_line++) {

		if (render == 0) break;
		render->selected = FALSE;
		
		txt = fl_get_browser_line(fd_Render->Animations, cur_line);

		if (txt) {
			if (strcmp("@-", txt)) {
				/* is geen lege regel */
				if (fl_isselected_browser_line(fd_Render->Animations, cur_line)) {
					render->selected = TRUE;
					select_render = render;
					total_select++;
				}
				render->linenum = cur_line;
				render = render->next;
			} else {
				/* volgende is renderfile met de hoogste prioriteit */
				highpri = render;
				pri_line = cur_line + 1;
			}
		}
	}
	
	if ((total_select == 0) | (autoselect == TRUE)) {
		if (total_select) {
			render = renderlist->first;
			while (render) {
				render->selected = FALSE;
				render = render->next;
			}
		}
		if (highpri) {
			select_render = highpri;
			select_render->selected = TRUE;
			fl_select_browser_line(fd_Render->Animations, pri_line);
			if (autoselect == FALSE) ignore_next_select = TRUE;
		}
		
		autoselect = TRUE;
		
	} else if (total_select != 1) select_render = 0;
	
	total_lines = fl_get_browser_maxline(fd_Render->Systems);
	total_select = 0;

	system = systemlist->first;
	for (cur_line = 1; cur_line <= total_lines; cur_line++) {
	
		if (system == 0) break;
		system->selected = FALSE;
		
		txt = fl_get_browser_line(fd_Render->Systems, cur_line);
		if (txt) {
			if (strcmp("@-", txt)) {
				if (fl_isselected_browser_line(fd_Render->Systems, cur_line)) {
					system->selected = TRUE;
					select_system = system;
					total_select++;
				}
				system->linenum = cur_line;
				system = system->next;
			}
		}
	}
	
	if (total_select != 1) select_system = 0;
}


int update_all()
{
	static int first = TRUE, oldcolor = -1;
	static Render * last_render = (void *) -1;
	static System * last_system = (void *) -1;
	char txt[MAXLINELEN], tmp[MAXLINELEN], *str;
	int len, topline, color;
	int x, y, redraw_file = FALSE, redraw_system = FALSE, above_line;
	int redraw_render_browser, redraw_system_browser;
	Render * render;
	System * system;
	extern timestruc_t	dir_time;
	double	fdone, ferr, fbusy, ftodo, ftot, power;
	FL_OBJECT *obj;
		

		if (select_render) {
			switch(check_infofile(select_render)) {
				case FILE_CHANGED:
					redraw_file = TRUE;
					break;
				case FILE_REMOVED:
					select_render = NULL;
					redraw_file = TRUE;
					break;
			}
		}
		
		if (select_system) {
			switch(check_infofile(select_system)) {
				case FILE_CHANGED:
					redraw_system = TRUE;
					break;
				case FILE_REMOVED:
					select_system = NULL;
					redraw_system = TRUE;
					break;
			}
		}
		
		/* autoselect verandering */
		
		fl_freeze_form(fd_Render->Render);

		set_select();
		
		redraw_system_browser = redraw_render_browser = sync_dir();
		redraw_render_browser |= sync_files(renderlist->first);
		redraw_system_browser |= sync_files(systemlist->first);
		
		redraw_render_browser |= check_ready();
		redraw_system_browser |= check_wait();
		

		/*fl_freeze_form(fd_Render->Render);*/

		if (redraw_render_browser) {
			set_render_strings();
			sort_renderlist(renderlist);
			topline = fl_get_browser_topline(fd_Render->Animations);
			fl_clear_browser(fd_Render->Animations);

			above_line = TRUE;
			render = renderlist->first;
			while (render) {
				if (render->ready == FALSE) {
					if (above_line) fl_add_browser_line(fd_Render->Animations, "@-");
					above_line = FALSE;
				}
				
				strcpy(txt, "");
				if (render->error_int) strcat(txt, "@C1");
				if (render->largest == FALSE) strcat(txt, "@r");
				
				strcat(txt, render->string);
				
				fl_add_browser_line(fd_Render->Animations, txt);
				if (render->selected) fl_select_browser_line(fd_Render->Animations, fl_get_browser_maxline(fd_Render->Animations));

				render = render->next;
			}
			
			if (above_line) fl_add_browser_line(fd_Render->Animations, "@-");
			fl_set_browser_topline(fd_Render->Animations, topline);
		}
		
			
		/* zet kleur van system browser */
		
		if (select_system) {
			str = strrchr(select_system->infofile, ':');
			if (str) {
				if (strcmp(str + 1, myname)) color = FL_SLATEBLUE;
				else color = FL_YELLOW;

				if (color != oldcolor) {
					fl_set_object_color(fd_Render->Systems, FL_COL1, color);
					oldcolor = color;
					redraw_system_browser = TRUE;
				}
			}
		}

		if (redraw_system_browser) {
			sort_systemlist(systemlist);
			topline = fl_get_browser_topline(fd_Render->Systems);
			fl_clear_browser(fd_Render->Systems);

			above_line = TRUE;
			system = systemlist->first;
			while (system) {
				if (system->wait == FALSE) {
					if (above_line) fl_add_browser_line(fd_Render->Systems, "@-");
					above_line = FALSE;
				}
				str = strrchr(system->infofile, '/');
				fl_add_browser_line(fd_Render->Systems, str + 2);
				if (system->selected) fl_select_browser_line(fd_Render->Systems, fl_get_browser_maxline(fd_Render->Systems));
				system = system->next;
			}

			if (above_line) fl_add_browser_line(fd_Render->Systems, "@-");
			fl_set_browser_topline(fd_Render->Systems, topline);
		}
		
		set_select();
		if (select_render != last_render) redraw_file = TRUE;
		if (select_system != last_system) redraw_system = TRUE;
				
		if (redraw_file) {
			render = last_render = select_render;

			fl_set_button(fd_Render->Pri_Max, 0);
			fl_set_button(fd_Render->Pri_High, 0);
			fl_set_button(fd_Render->Pri_Def, 0);
			fl_set_button(fd_Render->Pri_Low, 0);
			fl_set_button(fd_Render->Pri_Min, 0);

			fl_set_button(fd_Render->Load_64, 0);
			fl_set_button(fd_Render->Load_48, 0);
			fl_set_button(fd_Render->Load_32, 0);
			fl_set_button(fd_Render->Load_16, 0);
			fl_set_button(fd_Render->Beep, 0);
			
			
			fl_hide_object(fd_Render->Retry);

			if (select_render == 0) {
				fl_set_object_lcol(fd_Render->Done_black, FL_INACTIVE);
				fl_set_object_label(fd_Render->Done_black, "Done");
				fl_redraw_object(fd_Render->Done_black);
				
			} else {
				if (render->error_int) {
					fl_show_object(fd_Render->Retry);
				}
				
				fl_set_object_lcol(fd_Render->Done_black, FL_BLACK);
				obj = fd_Render->Done_black;				
				
				fdone = render->done_int / (double) render->total_int;
				ferr  = render->error_int / (double) render->total_int;
				fbusy = render->busy_int / (double) render->total_int;
				ftodo = render->todo_int / (double) render->total_int;
				
				if (render->total_int < 100) {
					sprintf(txt, "Done  %d %%", (int) ((fdone * 100.0) + 0.5));
				} else {
					sprintf(txt, "Done  %.1f %%", fdone * 100.0);					
				}
				fl_set_object_label(fd_Render->Done_black, txt);
				
				if (render->total_int > (obj->w / 3)) {
					/* gamma 2.0 is net iets te klein */
					
					power = log(3.0 / obj->w) / log(1.0 / render->total_int);
					/*printf("power %f\n", power);*/
					
					/*printf("%.2f %.2f %.2f %.2f\n", fdone, ferr, fbusy, ftodo);*/
					
					fdone = pow(fdone, power);
					ferr = pow(ferr, power);
					fbusy = pow(fbusy, power);
					ftodo = pow(ftodo, power);
					
					ftot = fdone + ferr + fbusy + ftodo;
	
					fdone /= ftot;
					ferr /= ftot;
					fbusy /= ftot;
					ftodo /= ftot;
					
					/*printf("%.2f %.2f %.2f %.2f\n", fdone, ferr, fbusy, ftodo);*/
				}
				
				fd_Render->Done_yellow->w = fdone * obj->w;
				fd_Render->Done_red->w = (fdone + ferr) * obj->w;
				fd_Render->Done_green->w = (fdone + ferr + fbusy) * obj->w;

				fl_redraw_object(fd_Render->Done_black);
				fl_redraw_object(fd_Render->Done_yellow);
				fl_redraw_object(fd_Render->Done_red);
				fl_redraw_object(fd_Render->Done_green);

				if (render->pri_int == PRI_MIN) fl_set_button(fd_Render->Pri_Min, 1);
				else if (render->pri_int < (PRI_LOW + PRI_DEF) / 2) fl_set_button(fd_Render->Pri_Low, 1);
				else if (render->pri_int < (PRI_DEF + PRI_HIGH) / 2) fl_set_button(fd_Render->Pri_Def, 1);
				else if (render->pri_int < (PRI_HIGH + PRI_MAX) / 2) fl_set_button(fd_Render->Pri_High, 1);
				else fl_set_button(fd_Render->Pri_Max, 1);
				
				if (render->load_int <= 16) fl_set_button(fd_Render->Load_16, 1);
				else if (render->load_int <= 32) fl_set_button(fd_Render->Load_32, 1);
				else if (render->load_int <= 48)fl_set_button(fd_Render->Load_48, 1);
				else fl_set_button(fd_Render->Load_64, 1);

				fl_set_button(fd_Render->Beep, render->beep);
			}
		}
		
		if (redraw_system) {
			system = last_system = select_system;
			
			fl_set_button(fd_Render->Busy, 0);
			fl_set_button(fd_Render->Offline, 0);
			fl_set_button(fd_Render->Wait, 0);
			fl_set_button(fd_Render->Finish, 0);

			fl_set_button(fd_Render->Accept_64, 0);
			fl_set_button(fd_Render->Accept_48, 0);
			fl_set_button(fd_Render->Accept_32, 0);
			fl_set_button(fd_Render->Accept_16, 0);
			
			if (select_system) {
				if (system->max_load_int <= 16) fl_set_button(fd_Render->Accept_16, 1);
				else if (system->max_load_int <= 32) fl_set_button(fd_Render->Accept_32, 1);
				else if (system->max_load_int <= 48) fl_set_button(fd_Render->Accept_48, 1);
				else if (system->max_load_int <= 64) fl_set_button(fd_Render->Accept_64, 1);
				
				if (system->status_int & BUSY) fl_set_button(fd_Render->Busy, 1);
				if (system->status_int & WAIT) fl_set_button(fd_Render->Wait, 1);
				if (system->status_int & FINI) fl_set_button(fd_Render->Finish, 1);
				if (system->status_int & OFFLINE) fl_set_button(fd_Render->Offline, 1);
			}
		}
						
	fl_unfreeze_form(fd_Render->Render);
	return(0);
}

static void set_fonts(void)
{
  fl_set_font_name(0, "-*-helvetica-medium-r-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(1, "-*-helvetica-bold-r-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(2, "-*-helvetica-medium-o-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(3, "-*-helvetica-bold-o-*-*-*-?-100-100-p-*-*-*");

  fl_set_font_name(4, "-*-courier-medium-r-*-*-*-?-100-100-m-*-*-*");
  fl_set_font_name(5, "-*-courier-bold-r-*-*-*-?-100-100-m-*-*-*");
  fl_set_font_name(6, "-*-courier-medium-o-*-*-*-?-100-100-m-*-*-*");
  fl_set_font_name(7, "-*-courier-bold-o-*-*-*-?-100-100-m-*-*-*");

  fl_set_font_name(8, "-*-times-medium-r-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(9, "-*-times-bold-r-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(10, "-*-times-medium-i-*-*-*-?-100-100-p-*-*-*");
  fl_set_font_name(11, "-*-times-bold-i-*-*-*-?-100-100-p-*-*-*");

  fl_set_font_name(12, "-*-charter-medium-r-*-*-*-?-100-100-*-*-*-*");
  fl_set_font_name(13, "-*-charter-bold-r-*-*-*-?-100-100-*-*-*-*");
  fl_set_font_name(14, "-*-charter-medium-i-*-*-*-?-100-100-*-*-*-*");
  fl_set_font_name(15, "-*-symbol-medium-r-*-*-*-?-100-100-*-*-*-*");
}

void handle_multicast(int fd, void * rt)
{
	handle_mcast_r();
	update_all();
}

int main(int argc, char *argv[])
{
	fd_set  readers;
	int		Xfd, num, i;
	struct  timeval time_out;
	int		background = TRUE;
	
		/* process arg's */
		while (argc > 1) {
			if (argv[1][0] == '-') {
				i = 1;
				while (argv[1][i]) {
					switch(argv[1][i]) {
					case 'v':
						verbose++;
						break;
					case 'f':
						background = FALSE;
						break;
					default:
						printf("unknown option %c\n", argv[1][i]);
					}
					i++;
				}
				argc--;
				argv++;
			} else break;
		}

	if (background) {
		if (fork()) exit(0);
	}
	
	init_renderd();
	
	fl_initialize(&argc, argv, 0, 0, 0);

	/*set_fonts();*/

	fd_Render = create_form_Render();
	
	/* fill-in form initialization code */
	
	fl_deactivate_object(fd_Render->CPU);
	fl_deactivate_object(fd_Render->Memory);
	fl_deactivate_object(fd_Render->Swap);

	fl_deactivate_object(fd_Render->Busy);
	fl_deactivate_object(fd_Render->Offline);

	fd_Render->Done_yellow->w = 0;
	fd_Render->Done_red->w = 0;
	fd_Render->Done_green->w = 0;
	
	fl_set_object_lcol(fd_Render->Done_black, FL_INACTIVE);
	fl_hide_object(fd_Render->Retry);
	
	/* show the first form */

	fl_set_form_dblbuffer(fd_Render->Render, TRUE);
	fl_show_form(fd_Render->Render,FL_FREE_SIZE,FL_FULLBORDER,"Render");
	
	/* newstyle */
		Xfd = ConnectionNumber(fl_get_display());
		
		while (1) {
			update_all();
			fl_check_forms();
			
			
			FD_ZERO(&readers);
			FD_SET(Xfd, &readers);
			FD_SET(mcast_r, &readers);
	
			/* set a 10 second time out */
			time_out.tv_sec = 10;
			time_out.tv_usec = 0;
			
			while (num = select(FD_SETSIZE,&readers,0,0,&time_out)) {
				if (num > 0) {
					/* is there something on the socket? */
					if (FD_ISSET(Xfd, &readers)){
						while(fl_check_forms());
						num--;
					}
					
					if (FD_ISSET(mcast_r, &readers)){
						/* deze komen altijd per 2 dus wacht op volgende
						 * 1 = plaatje klaar, 2 = start nieuw plaatje
						 */
						sginap(10);
						handle_mcast_r();
						num--;
					}
					time_out.tv_sec = 0;
					time_out.tv_usec = 10000;
				} else break;
			}
		}
}

