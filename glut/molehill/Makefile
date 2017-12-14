
all: molehill

CFLAGS = -I.. -O

molehill : molehill.o  ../libglut/libglut.a
	cc -o molehill molehill.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11

clean:
	-rm -f *.o
	-rm molehill

