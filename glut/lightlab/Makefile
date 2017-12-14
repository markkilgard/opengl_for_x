
all: lightlab

CFLAGS = -I.. -O

lightlab : lightlab.o  ../libglut/libglut.a
	cc -o lightlab lightlab.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm lightlab

