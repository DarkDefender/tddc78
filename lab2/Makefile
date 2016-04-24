
CFLAGS = -O3 -march=native 

LFLAGS= -lpthread -lrt -lm

all: thres

clean:
	-$(RM) *.o thres

thres: threshold.o ppm.o
	cc -o $@ threshold.o ppm.o $(LFLAGS)
