
all: splatlogo

CFLAGS = -I.. -O

splatlogo : splatlogo.o  logo.o ../libglut/libglut.a
	cc -o splatlogo splatlogo.o logo.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11

logo.o : logo.c.Z
	zcat logo.c.Z > logo.c
	cc $(CFLAGS) -c logo.c

clean:
	-rm -f *.o
	-rm splatlogo logo.c

