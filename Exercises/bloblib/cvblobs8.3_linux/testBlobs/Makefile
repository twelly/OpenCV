OPTIONS = -Wall -g -O0
#OPTIONS = -O9
#SFML = -lsfml-graphics -lsfml-window -lsfml-system
#SDL = -lSDL
OPENCV = -L . -lcxcore -lcv -lhighgui -lcvaux -lblob
LIB = $(SDL) $(SFML) $(OPENCV)

all:extractblob

extractblob:main.o
	g++ -o $@ $^ $(OPTIONS) $(LIB)

main.o:main.cpp
	g++ -c $(OPTIONS) main.cpp

clean:
	rm -f *.o

remake:clear all

