default all: paint totga anim fs
progs:paint fs displaytape
NGOBJS = util.o /usr/people/frank/source/gl_util.o /usr/people/trace/mips1/Button.o /usr/people/frank/source/storage.o

CFLAGS = -Xcpluscomm

util.o: util.c util.h
	cc -O -c -cckr util.c -mips1

animsplit: animsplit.c imbuf/anim.c
	cc -g animsplit.c util.o -lmovie -lgl -lX11 -ldmedia -lcl -lawareaudio -laudio -limbuf -lm -o animsplit

frompict: frompict.c
	cc -DDEBUG -O -o frompict frompict.c util.o -limbuf -lm

fitpoly: fitpoly.c 
	cc -g fitpoly.c -o fitpoly util.o  -lgl -limbuf -lm 

tracesd: tracesd.c util.o network.o
	cc -g tracesd.c -o tracesd util.o network.o

renderd: renderd.o renderd_main.o util.o network.o
	cc -g renderd.o renderd_main.o -o renderd_new util.o network.o -lfam -lC
	if (-x ./renderd_new) safemv -b renderd_new renderd
	
renderd.o: renderd.h

Render: Render.o Render_cb.o Render_main.o renderd.o network.o
	cc -O Render.o Render_cb.o Render_main.o renderd.o network.o util.o -lfam -lC -lforms -lX11 -lm -o Render_new
	if (-x ./Render_new) safemv -b Render_new Render

Chroma: Chroma.o  Chroma_main.o
	cc -O Chroma.o Chroma_main.o util.o -o Chroma -lforms -lmovie -lgl -lX11 -ldmedia -lcl -lawareaudio -laudio -limbuf -lGL -lm

Chroma_main.o:
	cc -O -O2 -c Chroma_main.c

Render_bug: Render.o Render_cb.o Render_main.o renderd.o network.o
	cc -O Render.o Render_cb.o Render_main.o renderd.o network.o util.o -lfam -lC -lforms -lX11 -lm -lmalloc -o Render_bug

Render_cb.o Render_main.o: renderd.h


cdd: cdd.c
	cc -g -DDEBUG cdd.c -o cdd util.o -lgl -limbuf -lm

ed: util.o gl_util.o ed.o /usr/lib/libimbuf.a
	cc -O2 ed.o util.o gl_util.o -lgl -limbuf -lm -o ed
	
eddebug: util.o /usr/lib/libimbuf.a
	cc -g ed.c util.o gl_util.o -lgl -limbuf -lm -o ed

chat: chat.o util.o
	cc -g chat.o util.o -lgl -o chat

fullscreen:fullscreen.c  /usr/lib/libimbuf.a
	cc -O2 -cckr fullscreen.c util.o -lgl -lm -limbuf -lfm -lc -lm -o fullscreen

paint:paint.o util.o
	cc -g paint.o util.o -lgl -lX11 -limbuf -lm -lc -o paint

paint.o:paint.c
	cc -g -c paint.c
	
cpaint:paint.o cimbuf.o
	cc paint.o cimbuf.o -g -lgl -lm -lfm -lc -o cpaint

render: render.c util.o network.o Button.o
	cc -g render.c -o render util.o network.o Button.o -lm -lfm -lgl
	
compres:compres.o imbuf.o storage.o
	cc -O2 compres.o imbuf.o util.o storage.o -o compres

dirtape:dirtape.o storage.o
	cc -O2 dirtape.o storage.o util.o -lgl -o dirtape

getdir: getdir.o storage.o storage.h
	cc -g getdir.o storage.o -o -lc getdir

zoom:zoom.c
	cc -O2 zoom.c util.o -lgl -limbuf -lm -o zoom
	strip zoom
	
oops:oops.o util.o gl_util.o
	cc -g oops.o util.o gl_util.o Button.o -lfm -lgl -lm -o oops

oops.o:
	cc -g -cckr -O -c oops.c

showiff:showiff.o imbuf.o iff.h
	cc -O2 showiff.o imbuf.o util.o -lgl -o /usr/people/bin/showiff


fs: /usr/include/local/util.h Button.o storage.o gl_util.o util.o storage.h
	cc -g fs.c -mips1 -D_XLIB_H_ -o fs_o $(NGOBJS) -limbuf -lm -lfm -lgl
#	#if (-x /usr/people/bin/fs) /bin/su - root -c "chown root /usr/people/bin/fs ; chmod +s /usr/people/bin/fs"

# fs:Button.o storage.o gl_util.o util.o /usr/include/local/util.h
# 	cc -g fs.c -o /usr/people/bin/fs gl_util.o Button.o storage.o util.o -limbuf -lm -lfm -lgl
# 	#if (-x /usr/people/bin/fs) /bin/su - root -c "chown root /usr/people/bin/fs ; chmod +s /usr/people/bin/fs"

displaytape.4.0.5:displaytape.o storage.o Button.o vfrutil.o gl_util.o util.o /usr/lib/libimbuf.a
	cc -O2 displaytape.o storage.o Button.o vfrutil.o gl_util.o util.o -lfm -lsvideo -lXext -lgl -limbuf -lvfr -lfft -lm -lmpc -o displaytape

displaytape:displaytape.o storage.o Button.o vfrutil.o gl_util.o util.o /usr/lib/libimbuf.a
	cc -O2 displaytape.o storage.o Button.o vfrutil.o gl_util.o util.o -lfm -lgl -limbuf -lvfr -lm -lmpc -o displaytape

vpaste:vpaste.o util.o /usr/lib/libimbuf.a
	cc -O2 vpaste.o util.o -lfm -lsvideo -lXext -lgl -limbuf -lvfr -lm -lmpc -o vpaste

vlan:vlan.o vfrutil.o util.o /usr/lib/libimbuf.a
	cc -O2 vlan.o vfrutil.o util.o -lgl -limbuf -lvfr -lm -o vlan

video:video.o
	cc -O2 -o video video.o -lvl -lgl -lX11 -ldmedia -limbuf -limage

video.o:
	cc -g -c video.c

digitize:digitize.o Button.o gl_util.o vfrutil.o util.o network.o /usr/lib/libimbuf.a
	cc -g digitize.o Button.o gl_util.o vfrutil.o util.o network.o -limbuf -lvfr -lfm -lgl -lm -lmpc -o digitize

dt:displaytape.o imbuf.o storage.o Button.o util.o
	cc -O2 -s displaytape.o imbuf.o storage.o Button.o util.o -lgl -lvfr -lfm -lm -lmpc -o dt
tapeloader:tapeloader.o util.o storage.o
	cc -O2 tapeloader.o util.o storage.o -o tapeloader

anim: anim.o util.o gl_util.o /usr/lib/libimbuf.a
	cc -g anim.o util.o gl_util.o -lmovie -lgl -lX11 -ldmedia -lcl -lawareaudio -laudio -limbuf -lm -o anim

movie : movie.o util.o gl_util.o /usr/lib/libimbuf.a
	cc -O2 movie.o util.o -lmovie -lgl -lX11 -limbuf -ldmedia -lcl -lawareaudio -laudio -lm -o movie

flick : flick.o util.o /usr/lib/libimbuf.a
	cc -g flick.o util.o -limbuf -lm -lmpc -o flick

stiff : stiff.c util.o
	cc -O2 -Wo,"-loopunroll 0" stiff.c util.o -lm -o stiff

slice : slice.o Button.o util.o gl_util.o /usr/lib/libimbuf.a
	cc -O2 slice.o Button.o util.o gl_util.o -lfm -lgl -limbuf -lm -lmpc -o slice

anim5 : anim5.o util.o /usr/lib/libimbuf.a
	cc -O2 anim5.o util.o -lgl -limbuf -lm -lmpc -o anim5

cmap : cmap.o util.o /usr/lib/libimbuf.a
	cc -O2 cmap.o util.o -lgl -limbuf -lm -lmpc -o cmap

anim24 : anim24.o util.o /usr/lib/libimbuf.a
	cc -O2 anim24.o util.o -lgl -lm -lmpc -limbuf -lm -lmpc -o anim24

mini : mini.o util.o storage.o /usr/lib/libimbuf.a
	cc -O2 mini.o util.o storage.o -lgl -limbuf -lm -lmpc -o mini

restoretape : restoretape.o storage.o util.o
	cc -O2 restoretape.o storage.o util.o -o restoretape

showvfr: showvfr.o
	cc -O2 showvfr.o util.o -lgl -limbuf -lvfr -lm -lmpc -float -o showvfr

vfrpal: vfrpal.o
	cc -O2 vfrpal.o util.o -lgl -limbuf -lvfr -lm -lmpc -float -o vfrpal

cmapanim : imbuf.o util.o cmapanim.o iff.h util.h
	cc -O2 cmapanim.o imbuf.o util.o  -lgl -float -o cmapanim

wu : util.o wu.o iff.h util.h Button.o gl_util.o /usr/lib/libimbuf.a
	cc -O2 wu.o Button.o gl_util.o util.o -limbuf -lfm -lgl -lm -float -o wu

morph : util.o morph.o gl_util.o gl_util.h iff.h util.h /usr/lib/libimbuf.a
	cc -O2 morph.o gl_util.o util.o -limbuf -lgl -lm -float -o morph

morphdebug : util.o iff.h util.h gl_util.o gl_util.h /usr/lib/libimbuf.a
	cc -g morph.c util.o gl_util.o -limbuf -lgl -lm -float -o morph

morph.o: morph.c
	cc -O -c -float morph.c

wu.o:wu.c
	cc -O -c -float wu.c

totga:util.o totga.o /usr/lib/libimbuf.a
	cc -O2 totga.o util.o -lgl -limbuf -lm -o totga

toiff:util.o totga.c /usr/lib/libimbuf.a
	cc -O2 totga.c -DSTRIP util.o -limbuf -lm -o toiff


yuvrgb:util.o gl_util.o Button.o yuvrgb.c
	cc -O2 yuvrgb.c /usr/people/trace/arith.o util.o gl_util.o Button.o -lfm -lgl -lX11 -limbuf -lm -o abcdefg
	cp abcdefg yuvrgb
	rm abcdefg
	
grab:grab.c util.o 
	cc -g grab.c util.o -lvfr -limbuf -lm -lmpc -o grab

pyro: pyro.o arith.o
	cc -O2 pyro.o util.o arith.o -lfm -lgl -limbuf -lm -lmpc -o pyro

lib : imbuf.c iff.h
	cc -G 0 -c -DSHLIB imbuf.c
	su root -c "mkshlib -s specfile -t libimbuf -h libimbuf.a"
	su root -c "mv libimbuf* /usr/lib/"

imbuf.o:imbuf.c iff.h
	cc -O -float -c imbuf.c

newstiff: newstiff.c
	cc -g newstiff.c -o newstiff -lstiff -lm

storage.o: storage.c
	cc -g -mips1 -c storage.c

mapdef : mapdef.o
	cc -g mapdef.o /usr/people/trace/mips1/util.o /usr/people/frank/source/gl_util.o -lgl -lm -o mapdef

audiosplit: audiosplit.o
	cc audiosplit.o -o audiosplit -laudiofile -lm
	
fullscreen.o paint.o imbuf.o compres.o: iff.h
displaytape.o: iff.h storage.h
storage.o fs.o dirtape.o : storage.h

.c.o:
	cc -g -c $(CFLAGS) $<
