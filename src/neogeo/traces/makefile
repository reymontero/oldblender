# voor het maken van traces:

CC = cc

T = /usr/people/trace/
I = /usr/people/include/
D = /usr/people/div/

# LET OP: ONDER 5.2 MAG DE -lfastm OPTIE MAAR OP ENKELE PLEKKEN STAAN, ZIE OPTS2

OPTS  = -lgl -lX11 -lfm -float -limbuf -lmpc -lmalloc -lm -limage -lmovie -ldmedia -lcl -lawareaudio -laudio -o
OPTSt  = -lfm -lgl -lX11 -limbuf -lm -limage -lmpc -lmalloc -o

OPTS2 = -lmovie -lfm -lgl -lX11 -float -limbuf -lm -limage -ldmedia -lcl -lawareaudio -laudio -lc -lmalloc -lfastm -o
OPTS4000 = -non_shared /usr/lib/mips2/nonshared/libm.a /usr/lib/mips2/nonshared/libfastm.a -lfm -lgl -float -limbuf -limage -lc -lmalloc -o

OBJECT1 = add.o edit.o verdit.o view.o draw.o spline.o toets.o traces.o code.o toolbox.o anim.o files.o
OBJECT5 = ray.o initrender.o zbuf.o render.o textu.o shadbuf.o  util.o effect.o
#OBJECT5U= ray.u initrender.u zbuf.u render.u textu.u shadbuf.u  util.u effect.u
OBJECT6 = isect.o trabuts.o font.o nurb.o displist.o psfont.o exotic.o

OBJECT2 = Button.o
OBJECT3 = fitcurves.o GGVecLib.o
OBJECT4 = arith.o noise.o storage.o

OBJECT7= calcika.o editika.o

default: psfont.o traces

.c.o:
	$(CC) -O -mips2 -float -cckr -D_XLIB_H_ -c $<

psfont.o:
	$(CC) -O -float -c $<

traces: $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) /usr/people/frank/source/imbuf/libimbuf.a
	$(CC) -O2 -mips2 $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) $(OPTS2) traces_new 
	if (-x /usr/people/trace/traces_new) safemv traces_new traces &
	if (-x /usr/people/trace/traces) /bin/su - root -c "chown root /usr/people/trace/traces ; chmod +s /usr/people/trace/traces"

traces4000: $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) /usr/people/frank/source/imbuf/libimbuf.a
	$(CC) -O2 -mips2 $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) $(OPTS4000) traces4000
	if (-x /usr/people/trace/traces) /bin/su - root -c "chown root /usr/people/trace/traces4000 ; chmod +s /usr/people/trace/traces4000"

tracesg: $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) /usr/people/frank/source/imbuf/libimbuf.a
	$(CC) -g $(OBJECT1) $(OBJECT2) $(OBJECT3) $(OBJECT4) $(OBJECT5) $(OBJECT6) $(OBJECT7) $(OPTS2) tracesg

brender: $(OBJECT5) brender.o
	$(CC) -O2 -Wo,"-loopunroll 0" brender.o $(OBJECT4) $(OBJECT5) $(OPTS) brender 
	
$(OBJECT1) : $(I)Trace.h $(I)util.h
$(OBJECT5) : $(I)Trace.h $(I)util.h
$(OBJECT6) : $(I)Trace.h $(I)util.h

$(OBJECT2) trabuts.o : $(I)Button.h

$(OBJECT7) add.o draw.o verdit.o edit.o files.o trabuts.o: ika.h

toets.o initrender.o : iff.h
storage.o : storage.h

jit: jit.o code.o noise.o
	$(CC) -O noise.o code.o arith.o jit.o $(OPTS) jit
