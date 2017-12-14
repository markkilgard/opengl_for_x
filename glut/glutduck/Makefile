
# Requires Open Inventor and C++ to build!

all: glutduck

CFLAGS = -I.. -O

glutduck : glutduck.o  ../libglut/libglut.a
	CC -o glutduck glutduck.o -L../libglut -lInventor -lglut -lGL -lGLU -lXmu -lXext -lX11

glutduck.o : glutduck.c++
	CC $(CFLAGS) -c glutduck.c++

clean:
	-rm -f *.o
	-rm glutduck

