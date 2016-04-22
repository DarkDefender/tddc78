
CFLAGS = -ggdb 

LFLAGS= -lpthread -lrt -lm

all: blurc thresc thres

clean:
	-$(RM) *.o blur thresc thres

blurc: ppmio.o gaussw.o blurfilter.o blurmain.o
	cc -o $@ ppmio.o gaussw.o blurfilter.o blurmain.o $(LFLAGS)

thresc: thresmain.o ppmio.o thresfilter.o
	cc -o $@ thresmain.o ppmio.o thresfilter.o $(LFLAGS)

thres: threshold.o ppm.o
	cc -o $@ threshold.o ppm.o $(LFLAGS)

arc:
	tar cf - *.c *.h *.f90 Makefile Makefile.sgi|gzip - > filters.tar.gz
