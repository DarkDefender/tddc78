
CFLAGS = -O3 -march=native 

LFLAGS= -lpthread -lrt -lm

all: thres blur

clean:
	-$(RM) *.o thres

thres: threshold.o ppm.o
	cc -o $@ threshold.o ppm.o $(LFLAGS)

blur: blur.o ppm.o gaussw.o
	cc -o $@ blur.o ppm.o gaussw.o $(LFLAGS)
