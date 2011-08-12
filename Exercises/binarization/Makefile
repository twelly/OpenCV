OPTIONS = -Wall -g -O0 -I/usr/include/opencv
OPENCV = -lcxcore -lcv -lhighgui -lcvaux
LIB = $(OPENCV)

all:binarize

binarize:main.o
	g++ -o $@ $^ $(LIB)

main.o:main.cpp
	g++ -c $(OPTIONS) main.cpp

clean:
	rm -f *.o

remake:clear all

