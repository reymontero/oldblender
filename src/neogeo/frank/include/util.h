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

#ifndef UTIL_H

#define UTIL_H

#include <sys/types.h>

#ifdef debug
#include <malloc.h>
#endif

#ifndef	NULL
#define NULL			0
#endif

#ifndef	FALSE
#define FALSE			0
#endif

#ifndef	TRUE
#define TRUE			1
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

#define mallocstruct(x,y) (x*)malloc((y)* sizeof(x))
#define callocstruct(x,y) (x*)calloc((y), sizeof(x))
#define mallocstructN(x,y,name) (x*)mallocN((y)* sizeof(x),name)
#define callocstructN(x,y,name) (x*)callocN((y)* sizeof(x),name)

#define RMK(x)

#define ELEM(a, b, c)		( (a)==(b) || (a)==(c) )
#define ELEM3(a, b, c, d)	( ELEM(a, b, c) || (a)==(d) )
#define ELEM4(a, b, c, d, e)	( ELEM(a, b, c) || ELEM(a, d, e) )
#define ELEM5(a, b, c, d, e, f)	( ELEM(a, b, c) || ELEM3(a, d, e, f) )
#define ELEM6(a, b, c, d, e, f, g)	( ELEM(a, b, c) || ELEM4(a, d, e, f, g) )

#define MIN2(x,y)		( (x)<(y) ? (x) : (y) )
#define MIN3(x,y,z)		MIN2( MIN2((x),(y)) , (z) )
#define MIN4(x,y,z,a)		MIN2( MIN2((x),(y)) , MIN2((z),(a)) )

#define MAX2(x,y)		( (x)>(y) ? (x) : (y) )
#define MAX3(x,y,z)		MAX2( MAX2((x),(y)) , (z) )
#define MAX4(x,y,z,a)		MAX2( MAX2((x),(y)) , MAX2((z),(a)) )

#define SWAP(type, a, b)	{ type sw_ap; sw_ap=(a); (a)=(b); (b)=sw_ap; }

#ifndef ABS
#define ABS(x)	((x) < 0 ? -(x) : (x))
#endif

#ifdef AMIGA
#	define GL(x) (*((long *)(x)))
#else
#	define GL(x) (((ushort *)(x))[0] << 16 | ((ushort *)(x))[1])
#endif

#define GS(x) (((uchar *)(x))[0] << 8 | ((uchar *)(x))[1])
#define GSS(x) (((uchar *)(x))[1] << 8 | ((uchar *)(x))[0])

#define SWAP_L(x) (((x << 24) & 0xff000000) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x >> 24) & 0xff))
#define SWAP_S(x) (((x << 8) & 0xff00) | ((x >> 8) & 0xff))

#define MAKE_ID(a,b,c,d) ( (long)(a)<<24 | (long)(b)<<16 | (c)<<8 | (d) )

#ifdef __ANSI_CPP__
	/* ansi */
	#define NEW(x) (x*)mallocN(sizeof(x),# x)
	#define CLN(x) (x*)callocN(sizeof(x),# x)
	#define PRINT(d, var1)	printf(# var1 ":%" # d "\n", var1)
	#define PRINT2(d, e, var1, var2)	printf(# var1 ":%" # d " " # var2 ":%" # e "\n", var1, var2)
	#define PRINT3(d, e, f, var1, var2, var3)	printf(# var1 ":%" # d " " # var2 ":%" # e " " # var3 ":%" # f "\n", var1, var2, var3)
	#define PRINT4(d, e, f, g, var1, var2, var3, var4)	printf(# var1 ":%" # d " " # var2 ":%" # e " " # var3 ":%" # f " " # var4 ":%" # g "\n", var1, var2, var3, var4)
#else
	/* -cckr */
#define NEW(x) (x*)mallocN(sizeof(x),"x")
#define CLN(x) (x*)callocN(sizeof(x),"x")
#endif


/* deze definities horen hier helemaal niet thuis
#define LSHIFT	(1<<0)
#define RSHIFT	(1<<1)
#define SHIFT	(LSHIFT | RSHIFT)
#define LALT	(1<<2)
#define RALT	(1<<3)
#define ALT	(LALT | RALT)
#define LCTRL	(1<<4)
#define RCTRL	(1<<5)
#define CTRL	(LCTRL | RCTRL)
#define LMOUSE	(1<<16)
#define MMOUSE	(1<<17)
#define RMOUSE	(1<<18)
#define MOUSE	(LMOUSE | MMOUSE | RMOUSE)
*/

typedef struct Link
{
	struct Link *next,*prev;
}Link;


typedef struct ListBase
{
	void *first,*last;
}ListBase;


typedef struct MemHead {
	long tag1;
	struct MemHead *next,*prev;
	char * name;
	char * nextname;
	long len;
	long level;
	long tag2;
} MemHead;

typedef struct MemTail {
	long tag3;
} MemTail;

#define MEMTAG1 'MEMO'
#define MEMTAG2 'RYBL'
#define MEMTAG3 'OCK!'
#define MEMFREE 'FREE'
#define MEMNEXT(x) ((MemHead *)(((char *) x) - ((char *) & (((MemHead *)0)->next))))

extern short totblock;
extern long mem_in_use;
extern long hw_clock();
extern void (*memory_error)();
extern void *mallocN(long,char *);
extern void *callocN(long,char *);
extern short freeN(void *);
extern short fileselect(char*,char*);
extern long stringdec(char *,char *,char *,ushort *);
extern void stringenc(char *,char *,char *,ushort, long);
extern char * get_retrace_pointer();
extern void ap_framelen(char * cmd,long i);
extern char *tcode_to_string(long len);
extern long string_to_tcode(char * str);
extern void * load_to_mem(char * name);
extern long countlist(ListBase *);
extern void Sginap(long);

#ifdef debug
#define malloc(x) mallocN(x,"debug")
#define free(x) freeN(x)
#define calloc(x,y) callocN((x)*(y),"debug")
#endif

#endif /* UTIL_H */

