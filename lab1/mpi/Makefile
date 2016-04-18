CXX = mpic++

CFLAGS = -g -O0 

LFLAGS = -g

all: thres

clean:
	-$(RM) *.o thres

thres: ppmio.o gaussw.o t_main.o
	${CXX} -o $@ ppmio.o gaussw.o t_main.o $(LFLAGS)

