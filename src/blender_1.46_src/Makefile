# voor het maken van blender:

default: makesdna DNA blender

T = /usr/people/trace/
F = /usr/people/frank/source/
I = /usr/people/include/

OPTSo = -lfm -lgl_s -float -lfastm -limbuf -lm -limage -lmpc -lmalloc -o
OPTS  = -lfm_s -lgl_s -lc_s -float -limbuf -lm -limage  -lfastm -o
OPTSnos= -lfm -lgl -lX11 -lc -float -lfastm -limbuf -lm -lmpc -limage -o


OB_OLD = $(T)noise.o $(T)util.o $(F)storage.o
OB_EDIT = editobject.o editmesh.o edit.o isect.o scanfill.o
OB_GRAPHICS  = Button.o screen.o mywindow.o space.o toolbox.o usiblender.o view.o drawview.o drawscene.o
OB_GRAPHICS1 = toets.o buttons.o headerbuttons.o drawipo.o drawobject.o editview.o filesel.o
OB_FILE = genfile.o readfile.o writefile.o
OB_STANDALONE = arithb.o
OB_MIXMODEL = blender.o library.o scene.o ipo.o object.o mesh.o material.o
OB_RENDER = 

.c.o:
	cc -g -float -c $<
#         -g of -0

makesdna: genfile.o 
	cc genfile.o $(T)util.o makesdna.c -lc -o makesdna

DNA: blender.h  screen.h graphics.h genfile.o
	makesdna


blender:  $(OB_OLD) $(OB_EDIT) $(OB_GRAPHICS) $(OB_GRAPHICS1) $(OB_MIXMODEL) $(OB_STANDALONE) $(OB_FILE) $(OB_RENDER) $(F)imbuf/libimbuf.a
	cc -g $(OB_OLD) $(OB_EDIT) $(OB_GRAPHICS) $(OB_GRAPHICS1) $(OB_MIXMODEL) $(OB_STANDALONE) $(OB_FILE) $(OB_RENDER) $(OPTS) blender 
#         -g of -02


$(OB_EDIT):  blender.h screen.h graphics.h edit.h
$(OB_GRAPHICS): blender.h screen.h graphics.h
$(OB_GRAPHICS1): blender.h screen.h graphics.h
$(OB_MIXMODEL): blender.h screen.h 
$(OB_FILE): blender.h screen.h file.h
#$(OB_RENDER): blender.h

struct: genfile.o 
	cc genfile.o $(T)util.o struct.c -lc -o struct

