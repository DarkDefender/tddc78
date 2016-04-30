CXX = mpic++

CXXFLAGS = -std=c++11 -g -O3 -march=native 

LFLAGS = -g

all: part

clean:
	-$(RM) *.o part

part: physics.o main.o
	${CXX} -o $@ physics.o main.o $(LFLAGS)

