OPTIONS = -Wall -g -O0 -I/usr/include/opencv
OPENCV = -lcxcore -lcv -lhighgui -lcvaux
LIB = $(OPENCV)

all:unwarp

unwarp:main.o
	g++ -o $@ $^ $(OPTIONS) $(LIB)

main.o:main.cpp
	g++ -c $(OPTIONS) main.cpp

clean:
	rm -f *.o
