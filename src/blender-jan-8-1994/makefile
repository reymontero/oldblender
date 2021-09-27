# voor het maken van blender:

CC = cc

T = /usr/people/trace/
F = /usr/people/frank/source/
I = /usr/people/include/

OPTS  = -lfm -lgl_s -float -lfastm -limbuf -lm -limage -lmpc -lmalloc -o

OB_OLD = $(T)arith.o $(T)noise.o $(T)util.o $(T)Button.o
OB_GRAPHICS = screen.o window.o toolbox.o usiblender.o
OB_MIXMODEL = blender.o
OB_RENDER = 

default: blender

.c.o:
	$(CC) -g -float -c $<
#         -g of -0


blender: $(OB_OLD) $(OB_GRAPHICS) $(OB_MIXMODEL) $(OB_RENDER) $(F)imbuf/libimbuf.a
	$(CC) -g $(OB_OLD) $(OB_GRAPHICS) $(OB_MIXMODEL) $(OB_RENDER) $(OPTS) blender 
#         -g of -02
#	if (-x /usr/people/blend/blender_new) mv blender_new blender

