CC = mpic++

CFLAGS = -g -O0 

LFLAGS = -g

all: blurc

clean:
	-$(RM) *.o blur

blurc: ppmio.o gaussw.o main.o
	${CC} -o $@ ppmio.o gaussw.o main.o $(LFLAGS)

