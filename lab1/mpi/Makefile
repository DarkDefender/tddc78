CXX = mpic++

CFLAGS = -g -O0 

LFLAGS = -g

all: blur

clean:
	-$(RM) *.o blur

blur: ppmio.o gaussw.o main.o
	${CXX} -o $@ ppmio.o gaussw.o main.o $(LFLAGS)

