CXX = mpic++

CFLAGS = -g -O0 

LFLAGS = -g

all: thres blur

clean:
	-$(RM) *.o thres blur

thres: ppmio.o t_main.o
	${CXX} -o $@ ppmio.o t_main.o $(LFLAGS)

blur: ppmio.o gaussw.o b_main.o
	${CXX} -o $@ ppmio.o gaussw.o b_main.o $(LFLAGS)
