#include $(ROOT)/usr/include/make/commondefs

OPTIMIZER  = -g

OBJECTS = play.o vlInit.o clInit.o alInit.o mvInit.o Xinit.o streamDecompress.o glxh2.o network.o util.o edl.o timecode.o Play.o Play_cb.o
HFILES = play.h
LLDLIBS= -lforms -lX11 -lmovie -lcl -ldmedia -lgl -laudio -lm -lvl -lgen
LDMFLAGS= -lsphere -lgl -lm -lX11 -s

TARGETS =  play

default all: $(TARGETS)

Play.o Play_cb.o Xinit.o edl.o: Play.h

play: $(OBJECTS)
	cc -o play_new $(OBJECTS) $(LLDLIBS)

	/usr/people/bin/safemv -b play_new play
	# /bin/su - root -c "chown root /usr/people/frank/source/play/play ; chmod +s /usr/people/frank/source/play/play"


Play:Play.o Play_cb.o Play_main.o
	cc -O -I/usr/include/local Play*.o -o Play -lforms -lX11 -lm 
	

timecode: timecode.o
	cc -o $@ timecode.o -laudiofile -lm
