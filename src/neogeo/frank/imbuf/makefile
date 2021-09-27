INCLUDES = -Ijpeg-6a/#-I/usr/include/gl
OBJS = \
allocimbuf.o	cmap.o        dither.o      targa.o       rectop.o\
amiga.o	        divers.o      iris.o        troep.o\
anim.o			cspace.o      filter.o      makecmap.o     util.o\
bitplanes.o 	data.o        ham.o         readimage.o   writeimage.o\
cdi.o			delta.o       iff.o         scaling.o		hamx.o\
rotate.o		antialias.o   izoom.o		tiff.o			jpeg.o\
tim.o			mdec_decode.o

CFLAGS = -O -mips1 $(INCLUDES)

all: libimbuf.so

clean:
	rm -f *.o
	rm -f libimbuf.a

install: all

# $(OBJS): iff.h imbuf.h util.h

data.o: matrix.h
hamx.o: hamx.c convert_arrays.h
convert_arrays.h: make_convert
	make_convert > convert_arrays.h	
make_convert: make_convert.c
	cc make_convert.c ../util.o -limbuf -lm -o make_convert
	
libimbuf.a: $(OBJS)
	touch libimbuf.a
	rm libimbuf.a
	cd jpeg-6a; make libjpeg.a
	ar scru libimbuf.a $(OBJS) jpeg-6a/*.o

libimbuf.so: libimbuf.a
	ld -soname libimbuf.so -shared -all libimbuf.a -o temp
	safemv -b temp libimbuf.so

