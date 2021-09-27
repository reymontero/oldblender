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

#include <fcntl.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <fmclient.h>
#include <local/util.h>
#include <local/gl_util.h>
#include <local/Button.h>
#include <malloc.h>

/* 
bindkeyo -r f1,'make oops \n'
bindkeyo -r f2,'oops \n'
*/


#define MAX_IO_OOPS 8
#define OOPSIN 1
#define OOPSOUT 2
#define OOPSIO (0xF)
#define OOPSTYPE (0xFFF0)
#define OOPSVOID (0 << 4)
#define OOPSOOPS (1 << 4)
#define OOPSLONG (10 << 4)
#define OOPSCHAR (11 << 4)

#define OOPSW 60
#define OOPSH 40
#define OOPSTP 20
#define OOPSNONE -1
#define OOPSBASE -2
#define OOPSPROG -3

#define OOPSMACRO 1
#define OOPSSAVED 2
#define OOPSPARENT 4

/* fouten:

	het probleem van de verbindingen.
	1. eerst alle verbindigen doorlussen (alle subroutines).
	2. variabelen exporteren inclusief de variabelen waar een verbinding heen loopt,  maar die nu voorzien zijn
	3. verbindingen weer splitsen.
	4. verbindingen die niet meer te maken zijn weggooien.


	als een macro veranderd, moeten eigenlijk alle macro's die die macro bevatten ook opnieuw
	gecompileerd worden.

	er moet een beveiliging komen tegen het dubbel gebruiken van namen

	Er moet een data pointer komen die intern gebruikt kan worden voor bijv. strings.
	Er moet ook een functie pointer gereserveerd worden voor het met de linker muis-
	toets klikken op een functie. (fs, trekbuts, etc.);
*/



struct OopsBase
{
    struct OopsBase *next, *prev;
    struct OopsCon *export;
    short x,y;
    struct OopsData *data;
    long value[MAX_IO_OOPS];
    void *userdata;
    long usersize;
    long old;
};


struct OopsData{
    long (*func)();
    char name[10];
    void (*mousefunc)();
    short num;
    short type[MAX_IO_OOPS];
    short locx[MAX_IO_OOPS];
    short locy[MAX_IO_OOPS];
    long flags;
    short level;
    short users;
    long old;

    struct OopsCon *up,*down;
    struct ListBase objlist , conlist;
};


struct OopsCon
{
    struct OopsCon *next, *prev,*brother;

    struct OopsBase *in,*out;
    short inport,outport;
    long flags;
};


fmfonthandle oopsfont=0;
long oopswin;
long mousex,mousey,ofsx,ofsy;
struct ListBase oopsfunc_,oopsprog_,oopscon_;
struct ListBase *oopsfunc = &oopsfunc_;
struct ListBase *oopsprog = &oopsprog_;
struct ListBase *oopscon = &oopscon_;

struct ListBase *mainoops = &oopsprog_;
struct ListBase *maincon = &oopscon_;


void initlist(listbase)
struct ListBase *listbase;
{
    if (listbase == 0) return;
    listbase->first = 0;
    listbase->last = 0;
}


long mul(a,b)
long a,b;
{
    return(a*b);
}

struct OopsData Oops_mul =
{
    mul,"MUL", 0, 3,(OOPSOUT|OOPSLONG),(OOPSIN|OOPSLONG),(OOPSIN|OOPSLONG)
};


long add(a,b)
long a,b;
{
    return(a+b);
}

struct OopsData Oops_add =
{
    add,"ADD", 0, 3,(OOPSOUT|OOPSLONG),(OOPSIN|OOPSLONG),(OOPSIN|OOPSLONG)
};


long one()
{
    return(1);
}

struct OopsData Oops_1 =
{
    one,"1", 0, 1,(OOPSOUT|OOPSLONG),
};


long two()
{
    return(2);
}

struct OopsData Oops_2 =
{
    two,"2", 0, 1,(OOPSOUT|OOPSLONG),
};


long three()
{
    return(3);
}

struct OopsData Oops_3 =
{
    three,"3", 0, 1,(OOPSOUT|OOPSLONG),
};


void printi(c)
long c;
{
    printf("%d\n",c);
}


struct OopsData Oops_printi =
{
    (long (*)()) printi,"PRINTI", 0, 2,OOPSVOID,(OOPSIN|OOPSLONG),
};


void printc(c)
char *c;
{
    printf("%s\n",c);
}


struct OopsData Oops_printc =
{
    (long (*)()) printc,"PRINTC", 0, 2,OOPSVOID,(OOPSIN|OOPSCHAR),
};


void filesel(oops)
struct OopsBase *oops;
{
    char string[250], *newstring;
    long size;
    
    if (getwd(string) == 0) strcpy(string, "/usr/people/");
    
    if (oops->userdata!= 0 && oops->usersize > 0){
	strncpy(string, oops->userdata, sizeof(string) -1);
	string[sizeof(string) - 1] = 0;
    }
    
    if (fileselect("Select File", string)){
	size = strlen(string);
	if (size >0) {
	    size ++;
	    if (oops->userdata!= 0 && oops->usersize > 0) free(oops->userdata);
	    newstring = malloc(size);
	    strcpy(newstring, string);
	    oops->userdata = newstring;
	    oops->usersize = size;
	}
    }
}


char *file(oops)
struct OopsBase *oops;
{
    return(oops->userdata);
}


struct OopsData Oops_file =
{
    (long (*)()) file,"FILE", filesel, 2,(OOPSOUT|OOPSCHAR), OOPSOOPS,
};


void exec_oops(oops)
struct OopsBase *oops;
{
    short i;
    long v[MAX_IO_OOPS], ret;
    struct OopsCon *connect;
    struct OopsData *data;

    while (oops){
	data = oops->data;
	if(data->flags & OOPSMACRO){
	    connect = data->down;
	    while (connect){
		connect->in->value[connect->inport] = oops->value[connect->outport];
		connect = connect->next;
	    }
	    exec_oops(data->objlist.first);
	    connect = data->up;
	    while (connect){
		oops->value[connect->inport] = connect->out->value[connect->outport];
		connect = connect->next;
	    }
	} else if(data->func != 0){
	    for(i=data->num-1; i>0 ; i--){
		if (data->type[i] & OOPSIN) {
		    v[i] = oops->value[i];
		} else if (data->type[i] & OOPSOUT){
		    v[i] = (long) (oops->value + i);
		} else if (data->type[i] == OOPSOOPS){
		    v[i] = (long) oops;
		}
	    }

	    switch(data->num){
	    case 3:
		oops->value[0] = (*data->func)(v[1],v[2]);
		break;
	    case 2:
		oops->value[0] = (*data->func)(v[1]);
		break;
	    case 1:
		oops->value[0] = (*data->func)();
		break;
	    }
	}

	connect = oops->export;
	while (connect){
	    connect->in->value[connect->inport] = oops->value[connect->outport];
	    connect = connect->brother;
	}

	oops = oops->next;
    }
}


long sortoops(oops1,oops2)
struct OopsBase **oops1,**oops2;
{
    if ((*oops1)->y  < (*oops2)->y) return (1);
    if ((*oops1)->y  > (*oops2)->y) return (-1);
    if ((*oops1)->x  > (*oops2)->x) return (1);
    if ((*oops1)->x  < (*oops2)->x) return (-1);
    return (0);
}


void compile_oops(oopsprog,oopscon)
struct ListBase *oopsprog,*oopscon;
{
    struct OopsBase *oops,**oopstab;
    struct OopsCon *connect;
    long port,i,count;

    /* alle modules wissen en tellen */

    oops = (struct OopsBase *) oopsprog->first;
    count = 0;
    while (oops){
	oops->export = 0;
	for (i = oops->data->num -1 ; i>=0 ; i--) oops->value[i] = 0;
	oops = oops->next;
	count ++;
    }

    /* daarna op y,x sorteren */

    if (count == 0) return;
    if (count > 1){
	oopstab = mallocstruct(struct OopsBase *,count);
	if (oopstab == 0) return;

	oops = (struct OopsBase *) oopsprog->first;
	count = 0;
	while (oops){
	    oopstab[count] = oops;
	    oops = oops->next;
	    count ++;
	}
	qsort(oopstab,count,4,sortoops);
	for (count -- ; count >= 0; count --){
	    remlink(oopsprog,oopstab[count]);
	    addhead(oopsprog,oopstab[count]);
	}
	free(oopstab);
    }

    /* verbindingen leggen */
    /*	 elke OOPS wijst naar een lijst van connections van variabelen die geexporteerd moeten worden
		connect = oops->export is de eerste,
		connect->brother wijst naar de volgende
	*/


    connect = (struct OopsCon *) oopscon->first;
    while (connect){
	oops = connect->out;
	connect->brother = oops->export;
	oops->export = connect;
	connect = connect->next;
    }
}


struct OopsCon *addconnect(conlist, oops1, port1, oops2, port2)
struct ListBase * conlist;
struct OopsBase *oops1, *oops2;
short port1, port2;
{
    struct OopsCon *connect = 0;

    if (oops1 == 0) return (0);
    if (oops2 == 0) return (0);
    if (conlist == 0) return (0);
    
    if (oops1->data->type[port1] & OOPSOUT){
	struct OopsBase *oops;
	short port;
    
	oops = oops1 ; 
	oops1 = oops2 ; 
	oops2 = oops;
	port = port1 ; 
	port1 = port2 ; 
	port2 = port;
    }
    if ((oops1->data->type[port1] & OOPSIN) && (oops2->data->type[port2] & OOPSOUT)){
	if ((oops1->data->type[port1] & OOPSTYPE) == (oops2->data->type[port2] & OOPSTYPE)){
	    connect = (struct OopsCon *) conlist->first;
	    while (connect){
		if ((connect->in == oops1) & (connect->inport == port1)){
		    if ((connect->out == oops2) & (connect->outport == port2)) break;
		}
		connect = connect->next;
	    }
 	    if (connect){
		remlink(conlist,connect);
		free(connect);
		connect = 0;
	    } else {
		if (connect = mallocstruct(struct OopsCon,1)){
		    connect->in = oops1;
		    connect->out = oops2;
		    connect->inport = port1;
		    connect->outport = port2;
		    addhead(conlist,connect);
		}
	    }
	}
    }
    return(connect);
}


struct OopsBase *findold(base, old)
struct ListBase *base;
long old;
{
    struct OopsBase *oops;
    
    oops = (struct OopsBase *) base->first;
    while (oops){
	if (oops->old == old) break;
	if (oops->data->old == old) break;
	oops = oops->next;
    }
    
    return(oops);
}


struct OopsBase * dupoops(base,old)
struct ListBase *base;
struct OopsBase *old;
{
    struct OopsBase *oops;

    oops = mallocstruct(struct OopsBase,1);
    if (oops == 0) return;
    *oops = *old;

    addtail(base,oops);
    return (oops);
}


struct OopsBase * newoops()
{
    struct OopsBase *oops;
    struct OopsData *data;

    oops = callocstruct(struct OopsBase,1);
    if (oops == 0) return(0);
    data = callocstruct(struct OopsData,1);
    if (data == 0){
	free(oops);
	return(0);
    }
    oops->data = data;
    return(oops);
}


void clearlevels(oops)
struct OopsBase *oops;
{
    while(oops){
	oops->data->level = 0;
	oops = oops->next;
    }
}


long calclevel(macro)
struct OopsBase *macro;
{
    struct OopsBase *oops;
    long mlevel = 0,level = 0;

    if (macro){
	if (macro->data->flags & OOPSMACRO){
	    /* is ie al berekend? return */
	    if (macro->data->level) return (macro->data->level);
	    oops = (struct OopsBase*) macro->data->objlist.first;
	    while (oops){
		if (oops->data->flags & OOPSMACRO){
		    level = calclevel(oops);
		    if (level > mlevel) mlevel = level;
		}
		oops = oops->next;
	    }
	    mlevel ++;
	    macro->data->level = mlevel;
	}
    }
    return (mlevel);
}


long saveobj(file,oops)
long file;
struct OopsBase *oops;
{
    long buf[100];

    buf[0] = 'FUNC';
    buf[1] = 12;
    buf[2] = (oops->x << 16) + oops->y;
    buf[3] = (long) oops;
    buf[4] = (long) oops->data;

    if (write(file,buf,12+8) != 12+8) return (-1);
    return(0);
}


long saveconnect(file,chunk,connect)
long file,chunk;
struct OopsCon *connect;
{
    long buf[10];

    buf[0] = chunk;
    buf[1] = 16;
    buf[2] = (long) connect->in;
    buf[3] = (long) connect->out;
    buf[4] = (connect->inport << 16) + connect->outport;
    buf[5] = connect->flags;

    if (write(file,buf,16+8) != 16+8) return (-1);
    return(0);
}


long savemacro(file,macro)
long file;
struct OopsBase *macro;
{
    struct OopsBase *oops;
    struct OopsData *data;
    struct OopsCon *connect;
    long buf[10],i;

    data = macro->data;
    i = (strlen(data->name) + 4) & ~3;  /* +4 om de 0 mee te copieren */

    buf[0] = 'DATA';
    buf[1] = 4 + i;
    buf[2] = (long) data;
    strcpy(buf+3,data->name);

    if (write(file,buf,12+i) != 12+i) return (-1);

    oops = (struct OopsBase *) data->objlist.first;
    while (oops){
	if (saveobj(file,oops)) return (-1);
	oops = oops->next;
    }

    connect = (struct OopsCon *) data->down;
    while (connect){
	if (saveconnect(file,'INPT',connect)) return (-1);
	connect = connect->next;
    }

    connect = (struct OopsCon *) data->up;
    while (connect){
	if (saveconnect(file,'EXPT',connect)) return (-1);
	connect = connect->next;
    }

    connect = (struct OopsCon *) data->conlist.first;
    while (connect){
	if (saveconnect(file,'CNCT',connect)) return (-1);
	connect = connect->next;
    }

    return(0);
}


long saveall(file)
long file;
{
    long i,mlevel;
    long buf[10];
    struct OopsBase *oops;
    struct OopsData *data;

    /* chunks: MAIN MACR MEND NAME DATA CNCT INPT EXPT*/

    buf[0] = FORM;
    buf[1] = 0;
    buf[2] = 'OOPS';
    buf[3] = 'VERS';
    buf[4] = 4;
    buf[5] = 1;

    if (write(file,buf,24) != 24) return(-1);

    /* hier worden eerst alle macro's weggeschreven, diepste eerst, en daarna steeds hoger tot aan main */

    oops = (struct OopsBase *) oopsfunc->first;
    clearlevels(oops);
    while (oops) {
	if (oops->data->flags & OOPSMACRO){
	    calclevel(oops);
	    if (oops->data->level > mlevel) mlevel = oops->data->level;
	}
	oops = oops->next;
    }

    for (i = 0; i <= mlevel; i++){
	oops = (struct OopsBase *) oopsfunc->first;
	while (oops){
	    if (oops->data->level == i){
		buf[0] = 'MACR' ; 
		buf[1] = 0;
		if (write(file,buf,8)!= 8) return(-1);

		if (savemacro(file,oops) == -1) return(-1);

		buf[0] = 'MEND' ; 
		buf[1] = 0;
		if (write(file,buf,8)!= 8) return(-1);
	    }
	    oops = oops->next;
	}
    }

    buf[0] = 'MAIN'; 
    buf[1] = 0;
    if (write(file,buf,8) != 8) return (-1);

    oops = newoops();
    if (oops == 0) return (-1);

    oops->data->objlist = *mainoops;
    oops->data->conlist = *maincon;

    if (savemacro(file,oops) == -1) return(-1);

    buf[0] = 'MEND' ; 
    buf[1] = 0;
    if (write(file,buf,8)!= 8) return(-1);

    free(oops->data); 
    free(oops);

    return(0);
}


saveoops()
{
    long file;
    char name[100];

    strcpy(name,"/usr/people/frank/oops");
    file = open(name,O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (file == -1){
	perror(name);
	return;
    }

    if (saveall(file)){
    } else{
    }

    close(file);
}


void initoops(oops)
struct OopsBase *oops;
{
    long x,y,xst,in,out,i,j;
    struct OopsData *data;

    data = oops->data;
    in = out = 0;
    for (i = data->num - 1 ; i>=0 ; i--){
	if (data->type[i] & OOPSIN) in++;
	else if (data->type[i] & OOPSOUT) out++;
    }

    if (in){
	xst = OOPSW/(in+1);
	j = 0;
	x = xst;
	y = OOPSH;

	for(i=in;i>0;i--){
	    while ((data->type[j] & OOPSIN) == 0) j++;
	    data->locx[j] = x;
	    data->locy[j] = y;
	    x += xst;
	    j++;		
	}
    }

    if (out){
	xst = OOPSW/(out+1);
	j = 0;
	x = xst;
	y = 0;

	for(i=out;i>0;i--){
	    while ((data->type[j] & OOPSOUT) == 0) j++;
	    data->locx[j] = x;
	    data->locy[j] = y;
	    x += xst;
	    j++;		
	}
    }
}


void addupdown(macro)
struct OopsBase *macro;
{
    struct OopsBase *oops;
    struct OopsCon *connect,*connext;
    long i,j;

    /* verbindingen zouden eigenlijk zoveel mogelijk hersteld moeten worden */

    connect = macro->data->down;
    while(connect){
	connext = connect->next;
	free(connect);
	connect = connext;
    }
    macro->data->down = 0;

    connect = macro->data->up;
    while(connect){
	connext = connect->next;
	free(connect);
	connect = connext;
    }
    macro->data->up = 0;

    oops = (struct OopsBase *) macro->data->objlist.first;
    while (oops) {
	for (i = oops->data->num - 1; i>=0 ;i--) oops->value[i] = 0;
	oops = oops->next;
    }

    connect = (struct OopsCon *) macro->data->conlist.first;
    while (connect) {
	connect->in->value[connect->inport] = 1;
	connect->out->value[connect->outport] = 1;
	connect = connect->next;
    }

    oops = (struct OopsBase *) macro->data->objlist.first;
    j = 0;
    while (oops) {
	for (i = 0 ; i < oops->data->num ; i++){
	    if (oops->value[i] == 0 && oops->data->type[i] & (OOPSIN | OOPSOUT)){
		connect = mallocstruct(struct OopsCon,1);
		if (connect){
		    macro->data->type[j] = oops->data->type[i];
		    if (oops->data->type[i] & OOPSIN){
			connect->in = oops;
			connect->out = macro;
			connect->inport = i;
			connect->outport = j;
			connect->next = macro->data->down;
			macro->data->down = connect;
		    } else {
			connect->in = macro;
			connect->out = oops;
			connect->inport = j;
			connect->outport = i;
			connect->next = macro->data->up;
			macro->data->up = connect;
		    }
		    j ++;
		}
	    }
	    if (j >= MAX_IO_OOPS) break;
	}
	if (j >= MAX_IO_OOPS){
	    printf("Warning: to many connections\n");
	    break;
	}
	oops = oops->next;
    }
    macro->data->num = j;
}



long loadmacro(macrop,file)
struct OopsBase **macrop;
long file;
{
    long buf[10],len;
    struct OopsBase *oops, *oops1, *oops2;
    struct OopsData *data;
    struct OopsBase *macro = 0;
    short port1, port2;
    struct OopsCon *connect;
    
    *macrop = 0;
    
    if (read(file,buf,8) != 8) return(-1);
    if (buf[0] != 'DATA'){
	printf("error while reading macro\n");
	return(-1);
    }

    len = buf[1];
    if (read(file,buf,len) != len) return(-1);
    oops = (struct OopsBase *) oopsfunc->first;
    while (oops) {
	if (strcmp(oops->data->name,buf+1)== 0){
	    if ((oops->data->flags & OOPSMACRO) == 0){
		oops->data->old = buf[0];
		/*oops = 0;*/
		return(0);
	    } else {
		printf("function exists\noops...\n");
	    }
	}
	oops = oops->next;
    }

    macro = newoops();
    if (macro == 0) {
	perror(0); 
	return(-1);
    }
    data = macro->data;

    data->old = buf[0];
    strcpy(data->name,buf+1);
    data->flags = OOPSMACRO;

    while(read(file,buf,8) == 8){
	len = buf[1];
	switch (buf[0]){
	case 'FUNC':
	    if (read(file,buf,len) != len) return(-1);
	    oops = findold(oopsfunc, buf[2]);
	    if (oops==0){
		printf("Couldn't find function in macro %s\n",macro->data->name);
		return (-1);
	    }
	    oops = dupoops(&data->objlist,oops);
	    oops->x = buf[0] >> 16;
	    oops->y = buf[0] & 0xffff;
	    oops->old = buf[1];
	    oops->data->users++;
	    break;
	case 'EXPT':
	    if (read(file,buf,len) != len) return(-1);
/*
	    EXPT en IMPT zijn nooit getest!!!!!!!!!!!!!!!!!!!!!!!
	    
	    
	    oops1 = macro;
	    oops2 = findold(&data->objlist, buf[1]);
	    if (oops2){
		port1 = buf[2] >> 16;
		port2 = buf[2] & 0xffff;
		connect = mallocstruct(struct OopsCon,1);
		if (connect){
		    connect->in = oops1;
		    connect->out = oops2;
		    connect->inport = port1;
		    connect->outport = port2;
		    connect->flags = buf[3];
		    connect->next = data->up;
		    data->up = connect;
		}
	    }else printf("Illegal export connection in %s\n", data->name);
*/	    break;
	case 'INPT':
	    if (read(file,buf,len) != len) return(-1);
/*	    oops1 = findold(&data->objlist, buf[0]);
	    oops2 = macro;
	    if (oops1){
		port1 = buf[2] >> 16;
		port2 = buf[2] & 0xffff;
		connect = mallocstruct(struct OopsCon,1);
		if (connect){
		    connect->in = oops1;
		    connect->out = oops2;
		    connect->inport = port1;
		    connect->outport = port2;
		    connect->flags = buf[3];
		    connect->next = data->down;
		    data->down = connect;
		}
	    }else printf("Illegal inport connection in %s\n", data->name);
*/	    break;
	case 'CNCT':
	    if (read(file,buf,len) != len) return(-1);
	    oops1 = findold(&data->objlist, buf[0]);
	    oops2 = findold(&data->objlist, buf[1]);
	    port1 = buf[2] >> 16;
	    port2 = buf[2] & 0xffff;
	    connect = addconnect(&data->conlist, oops1, port1, oops2, port2);
	    if (connect) connect->flags = buf[3];
	    else printf("Illegal connection in %s\n", data->name);
	    break;
	case 'MEND':
	    compile_oops(&data->objlist,&data->conlist);
	    addupdown(macro);
	    initoops(macro);
	    *macrop = macro;
	    return (0);
	    break;
	default:
	    printf("unknown chunk in macro\n");
	    lseek(file,buf[1],1);
	    break;
	}
    }
    return(-1);
}


long loadall(file)
long file;
{
    long buf[10],version;
    struct OopsBase *oops;

    if (read(file,buf,12) != 12){
	perror(0);
	return(-1);
    }
    if ((buf[0] != FORM) || (buf[2] != 'OOPS')){
	printf("No Oops file\n");
	return(-1);
    }
    while(read(file,buf,8) == 8){
	switch(buf[0]){
	case 'VERS':
	    if (read(file,&version,4) !=4) return(-1);
	    break;
	case 'MACR':
	    if (loadmacro(&oops,file)) return(-1);
	    if (oops) addtail(oopsfunc,oops);
	    break;
	case 'MAIN':
	    if (loadmacro(&oops,file)) return(-1);
	    if (oops){
		*oopsprog = oops->data->objlist;
		*oopscon = oops->data->conlist;
		free(oops->data);
		free(oops);
	    }
	    return(0);
	default:
	    lseek(file,buf[1],1);
	    break;
	}
    }
    printf("premature end of file!\n");
}


void drawall()
{
    struct OopsBase *oops;
    struct OopsCon *connect;
    short port;
    long vert[3];

    color(15);
    clear();

    color(0);

    connect = (struct OopsCon *) oopscon->first;
    while (connect){

	bgnline();
	oops = connect->in;
	port = connect->inport;
	vert[0]=oops->x + oops->data->locx[port]; 
	vert[1]=oops->y + oops->data->locy[port];
	v2i(vert);

	oops = connect->out;
	port = connect->outport;
	vert[0]=oops->x + oops->data->locx[port]; 
	vert[1]=oops->y + oops->data->locy[port];
	v2i(vert);
	endline();
	connect = connect->next;
    }

    oops = (struct OopsBase *)oopsfunc->first;
    while (oops){
	drawoops(oops);
	oops = oops->next;
    }

    oops = (struct OopsBase *)oopsprog->first;
    while (oops){
	drawoops(oops);
	oops = oops->next;
    }
}


void oopsallign(oops)
struct OopsBase *oops;
{
    long x,y,x2,y2,xst,in,out,i,j;

    x = y = 10;
    while (oops){
	oops->x = x;
	oops->y = y;

	initoops(oops);

	y += OOPSH+OOPSTP;
	oops = oops->next;
    }
}


loadoops()
{
    long file;
    char name[100];

    strcpy(name,"/usr/people/frank/oops");
    file = open(name,O_RDONLY);
    if (file == -1){
	perror(name);
	return;
    }
    if (loadall(file)){
	printf("Error while reading %s\n",name);
    }

    oopsallign(oopsfunc->first);
    drawall();
    swapbuffers();

    close(file);
}


drawoops(oops)
struct OopsBase *oops;
{
    short x,y,x2,y2,i;
    struct OopsData *data;

    x=oops->x;
    y=oops->y;
    data = oops->data;

    color (0);
    rect(x,y,x+OOPSW,y+OOPSH);
    cmov2i(x+(OOPSW-fmgetstrwidth(oopsfont,data->name))/2,y+(OOPSH-6)/2);
    fmprstr(data->name);

    for (i = data->num - 1 ; i >= 0 ; i--) {
	x2 = x+data->locx[i];
	y2 = y+data->locy[i];
	switch(data->type[i] & OOPSTYPE){
	case OOPSVOID:
	    break;
	case OOPSLONG:
	    color(14);
	    rectfi(x2-3,y2-6,x2+3,y2+6);
	    color(0);
	    recti(x2-3,y2-6,x2+3,y2+6);
	    break;
	case OOPSCHAR:
	    color(13);
	    rectfi(x2-3,y2-6,x2+3,y2+6);
	    color(0);
	    recti(x2-3,y2-6,x2+3,y2+6);
	    break;
	}
    }
}


short selectoops(oopsact,cononly)
struct OopsBase **oopsact;
short cononly;
{
    long x,y;
    struct OopsBase *oops;
    long i,poort;
    ulong min = 500,dist;
    short dx,dy;

    x = mousex - ofsx;
    y = mousey - ofsy;
    *oopsact = 0;

    if (cononly==0){
	oops = (struct OopsBase *) oopsprog->first;
	while (oops){
	    if ((x > oops->x) && (y > oops->y) && (x < oops->x + OOPSW) && (y < oops->y + OOPSH)){
		*oopsact = oops;
		return (OOPSPROG);
	    }
	    oops = oops->next;
	}

	oops = (struct OopsBase *) oopsfunc->first;
	while (oops){
	    if ((x > oops->x) && (y > oops->y) && (x < oops->x + OOPSW) && (y < oops->y + OOPSH)){
		*oopsact = oops;
		return (OOPSBASE);
	    }
	    oops = oops->next;
	}
    }

    poort = OOPSNONE;
    oops = (struct OopsBase *) oopsprog->first;
    while (oops){
	for (i = oops->data->num-1; i >= 0 ; i--){
	    if (oops->data->type[i] & (OOPSIN | OOPSOUT)){
		dx = oops->x + oops->data->locx[i] - x;
		dy = oops->y + oops->data->locy[i] - y;
		dist = dx*dx + dy*dy;
		if (dist < min){
		    min = dist;
		    poort = i;
		    *oopsact = oops;
		}
	    }
	}
	oops = oops->next;
    }
    return (poort);
}


void positionoops(oops)
struct OopsBase *oops;
{
    long dev,dx,dy;
    short val;

    dx = oops->x - mousex;
    dy = oops->y - mousey;

    while (dev=qread(&val)){
	switch (dev){
	case ESCKEY:
	    return;
	    break;
	case LEFTMOUSE:
	    oops->x = (oops->x + 4) & ~7;
	    oops->y = (oops->y + 4) & ~7;
	    return;
	    break;
	case MOUSEX:
	    mousex = val;
	    break;
	case MOUSEY:
	    mousey = val;
	    break;
	}
	if (qtest()==0){
	    oops->x = mousex+dx;
	    oops->y = mousey+dy;
	    drawall();
	    swapbuffers();
	}
    }
}


struct OopsCon *findconnect(in,inport,out,outport)
struct OopsBase *in,*out;
short inport,outport;
{
    struct OopsCon *connect;

    connect = (struct OopsCon *) oopscon->first;
    while (connect){
	if ((connect->in == in) & (connect->inport == inport)){
	    if ((connect->out == out) & (connect->outport == outport)) return (connect);
	}
	connect = connect->next;
    }
    return (0);
}


void connectoops(oops1,port1)
struct OopsBase *oops1;
short port1;
{
    struct OopsBase *oops2;
    struct OopsCon *connect;
    short port2,val;
    long vert[3],dev,i;

    while (dev=qread(&val)){
	switch (dev){
	case ESCKEY:
	    if (val) return;
	    break;
	case LEFTMOUSE:
	    if (val==0){
		port2 = selectoops(&oops2,TRUE);
		switch (port2){
		case OOPSBASE:
		case OOPSPROG:
		case OOPSNONE:
		    break;
		default:
		    addconnect(oopscon, oops1, port1, oops2, port2);
		    break;
		}
	    }
	    return;
	    break;
	case MOUSEX:
	    mousex = val;
	    break;
	case MOUSEY:
	    mousey = val;
	    break;
	}

	if (qtest() == 0){
	    drawall();
	    color(0);
	    bgnline();
	    vert[0]=oops1->x + oops1->data->locx[port1]; 
	    vert[1]=oops1->y + oops1->data->locy[port1];
	    v2i(vert);
	    vert[0]=mousex-ofsx; 
	    vert[1]=mousey-ofsy;
	    v2i(vert);
	    endline();
	    swapbuffers();
	}
    }
}


void remoops(oops)
struct OopsBase *oops;
{
    struct OopsCon *connect,*nextconnect;

    remlink(oopsprog,oops);
    free(oops);

    connect = (struct OopsCon *)oopscon->first;

    while (connect){
	if ((connect->in == oops) | (connect->out == oops)){
	    nextconnect =  connect->next;
	    remlink(oopscon,connect);
	    free(connect);
	    connect = nextconnect;
	} else connect = connect->next;
    }
}


void changename(oops)
struct OopsBase *oops;
{
    long orx,ory,wx=150,wy=20,maxx,maxy,win;
    Device mdev[2];
    short mval[2];
    short dum;

    mdev[0]=MOUSEX; 
    mdev[1]=MOUSEY;
    getdev(2, mdev, mval);
    maxx = getgdesc(GD_XPMAX) -1;
    maxy = getgdesc(GD_YPMAX) -1;

    orx = mval[0] - (wx/2);
    ory = mval[1] - (wy/2);

    if (orx < 20) orx = 20;
    if (ory < 20) ory = 20;
    if (orx  > maxx - wx - 20) orx = maxx - wx - 20;
    if (ory  > maxy - wy - 40) ory = maxy - wy - 40;

    unqdevice(INPUTCHANGE);
    prefposition(orx,orx+wx-1,ory,ory+wy-1);
    noborder();
    win = winopen("FileSelect");
    gconfig();
    sginap(10);

    while (qtest()) qread(&dum);

    DefButBlock("Oops",win,oopsfont,10,0,0);
    DefBut(TEX,1,"name: ",0,0,150,20,		oops->data->name ,0.0,10.0);
    DoButtons();
    winclose(win);
    qdevice(INPUTCHANGE);

}


void makemacro(minx,miny,maxx,maxy)
short minx,miny,maxx,maxy;
{
    struct OopsBase *oops,*oopsnext,*macro;
    struct OopsData *data;
    struct OopsCon *connect,*connext,*consplit;
    long count;
    short x,y;

    count = 0;
    oops = (struct OopsBase *) oopsprog->first;
    while (oops){
	x = oops->x;
	y = oops->y;
	if (x >= minx && x <= maxx && y >= miny && y <= maxy) count++;
	oops = oops->next;
    }

    if (count == 0) return;

    macro = newoops();
    if (macro == 0) return;
    data = macro->data;

    initlist(&data->objlist);
    initlist(&data->conlist);

    connect = (struct OopsCon*) oopscon->first;
    while (connect){
	connect->flags = 0;
	connect = connect->next;
    }

    oops = (struct OopsBase *) oopsprog->first;
    while (oops){
	x = oops->x;
	y = oops->y;
	oopsnext = oops->next;

	if (x >= minx && x <= maxx && y >= miny && y <= maxy){
	    remlink(oopsprog,oops);
	    addtail(&data->objlist , oops);

	    connect = (struct OopsCon*) oopscon->first;
	    while (connect){
		connext = connect->next;
		if (oops == connect->in) connect->flags += 1;
		if (oops == connect->out) connect->flags += 2;
		if (connect->flags == 3){
		    remlink(oopscon,connect);
		    addtail(&data->conlist,connect);
		}
		connect = connext;
	    }
	}
	oops = oopsnext;
    }

    data->flags = OOPSMACRO;
    data->users = 1;
    macro->x = (minx+maxx)/2;
    macro->y = (miny+maxy)/2;
    strcpy(data->name,"macro");
    strcpy(data->name,"");

    compile_oops(&data->objlist,&data->conlist);
    addupdown(macro);
    initoops(macro);
    addhead(oopsprog,macro);

    connect = (struct OopsCon*) oopscon->first;
    while (connect){
	connext = connect->next;
	if (connect->flags){
	    /* deze verbinding is verbroken, we gaan proberen om 'm weer te herstellen */

	    switch (connect->flags){
	    case 1:
		consplit = macro->data->down;
		while (consplit){
		    if ((consplit->in == connect->in) && (consplit->inport == connect->inport)){
			connect->in = consplit->out;
			connect->inport = consplit->outport;
			break;
		    }
		    consplit = consplit->next;
		}
		break;
	    case 2:
		consplit = macro->data->up;
		while (consplit){
		    if ((consplit->out == connect->out) && (consplit->outport == connect->outport)){
			connect->out = consplit->in;
			connect->outport = consplit->inport;
			break;
		    }
		    consplit = consplit->next;
		}
		break;
	    }
	    if (consplit == 0){
		/* verbinding kon niet gemaakt worden */
		remlink(oopscon,connect);
		free(connect);
	    }
	}
	connect = connext;
    }
    changename(macro);
    dupoops(oopsfunc,macro);
    oopsallign(oopsfunc->first);
}


void edit()
{
    long dev,i,wx,wy;
    short val;
    short minx,miny,maxx,maxy;
    struct OopsBase *oops;
    struct ListBase *progback,*conback;

    drawall();
    swapbuffers();

    while (dev=qread(&val)){
	switch (dev){
	case KEYBD:
	    switch(val){
	    case 27:
		return;
		break;
	    case 'm':
	    case 'M':
		if (getrect(&minx,&miny,&maxx,&maxy)){
		    makemacro(minx,miny,maxx,maxy);
		    drawall();
		    swapbuffers();
		}
		break;
	    case 'n':
	    case 'N':
		if (selectoops(&oops,FALSE) != OOPSNONE){
		    if (oops->data->flags & OOPSMACRO){
			changename(oops);
		    }
		}
		break;
	    case 'r':
	    case 'R':
		compile_oops(oopsprog,oopscon);
		exec_oops(oopsprog->first);
		break;
	    case 'x':
	    case 'X':
		i = selectoops(&oops,FALSE);
		switch (i){
		case OOPSBASE:
		    if (oops->data->flags & OOPSMACRO){
			if (oops->data->users <= 0){
			    /* alle kinderen ook vrijgeven */
			    remlink(oopsfunc,oops);
			    free(oops->data);
			    free(oops);
			}
		    }
		    break;
		case OOPSPROG:
		    oops->data->users --;
		    remoops(oops);
		    break;
		}
		drawall();
		swapbuffers();
		break;
	    case 'l':
	    case 'L':
		i = selectoops(&oops,FALSE);		/* testfunctie */
		switch (i){
		case OOPSBASE:
		case OOPSPROG:
		    clearlevels(oopsfunc->first);
		    calclevel(oops);
		    printf("level: %d\n",oops->data->level);
		    break;
		}
		break;
	    }
	    break;
	case F1KEY:
	    if (val) loadoops();
	    break;
	case F2KEY:
	    if (val) saveoops();
	    break;
	case REDRAW:
	    getsize(&wx,&wy);
	    ortho2(-0.5,wx-0.5,-0.5,wy-0.5);
	    viewport(0,wx-1,0,wy-1);
	    drawmode(OVERDRAW);
	    color(0);
	    clear();
	    drawmode(NORMALDRAW);
	    getorigin(&ofsx,&ofsy);
	    drawall();
	    swapbuffers();
	    break;
	case LEFTMOUSE:
	    if (val){
		i = selectoops(&oops,FALSE);
		switch (i){
		case OOPSBASE:
		    if (oops->data->flags & OOPSPARENT){
			printf("No recursion allowed\n");
			break;
		    }
		    oops = dupoops(oopsprog,oops);
		    oops->data->users ++;
		case OOPSPROG:
		    if (oops) positionoops(oops);
		    drawall();
		    swapbuffers();
		    break;
		case OOPSNONE:
		    break;
		default:
		    connectoops(oops,i);
		    drawall();
		    swapbuffers();
		    break;
		}
	    }
	    break;
	case MIDDLEMOUSE:
	    if (val){
		i = selectoops(&oops,FALSE);
		switch (i){
		case OOPSBASE:
		case OOPSPROG:
		    if (oops->data->flags & OOPSMACRO){
			oops->data->flags |= OOPSPARENT;
			progback = oopsprog;
			conback = oopscon;
			oopsprog = &oops->data->objlist;
			oopscon = &oops->data->conlist;
			edit();

			oops->data->flags &= ~OOPSPARENT;
			/* dit moet natuurlijk allemaal veel slimmer */

			compile_oops(&oops->data->objlist,&oops->data->conlist);
			addupdown(oops);
			initoops(oops);

			oopsprog = progback;
			oopscon = conback;
			drawall();
			swapbuffers();
		    }else if (i == OOPSPROG){
			if (oops->data->mousefunc){
			    (*oops->data->mousefunc)(oops);
			}
		    }
		    break;
		default:
		    return;
		    break;
		}
	    }
	    break;
	case MOUSEX:
	    mousex = val;
	    break;
	case MOUSEY:
	    mousey = val;
	    break;
	}
    }
}


main()
{
    fmfonthandle fmfont;
    struct OopsBase oops;
    long vars[50],i;

    initlist(oopsfunc);
    initlist(oopsprog);
    initlist(oopscon);

    oops.data = &Oops_file;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_printc;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_printi;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_mul;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_add;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_1;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_2;
    dupoops(oopsfunc,&oops);
    oops.data = &Oops_3;
    dupoops(oopsfunc,&oops);

    foreground();

    fminit();
    if(fmfont=fmfindfont("Helvetica-Narrow-Bold")){
	oopsfont=fmscalefont(fmfont,12.0);
    } else exit(0);
    fmsetfont(oopsfont);

    prefsize(500,500);
    oopswin = winopen("Oops");
    doublebuffer();
    gconfig();
    minsize(500,500);
    maxsize(getgdesc(GD_XPMAX)-1,getgdesc(GD_YPMAX)-1);
    winconstraints();
    /*
	glcompat(GLC_SOFTATTACH, 1);
	loopt vast bij fileselect
    */
    qdevice(RAWKEYBD);
    qdevice(KEYBD);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);
    qdevice(MOUSEX);
    qdevice(MOUSEY);
    qdevice(LEFTSHIFTKEY);
    qdevice(RIGHTSHIFTKEY);
    
    oopsallign(oopsfunc->first);
    getorigin(&ofsx,&ofsy);

    while(1){
	edit();

	compile_oops(oopsprog,oopscon);
	exec_oops(oopsprog->first);
    }
    fmfreefont(oopsfont);
    gexit();
}

