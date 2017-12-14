
all: origami

CFLAGS = -I.. -O

origami : origami.o  ../libglut/libglut.a
	cc -o origami origami.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11

clean:
	-rm -f *.o
	-rm origami

