
all: mjkwarp

CFLAGS = -I.. -O

mjkwarp : mjkwarp.o  mjkimage.o ../libglut/libglut.a
	cc -o mjkwarp mjkwarp.o mjkimage.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11 -lm

mjkimage.o : mjkimage.c.Z
	zcat mjkimage.c.Z > mjkimage.c
	cc $(CFLAGS) -c mjkimage.c

clean:
	-rm -f *.o
	-rm mjkwarp mjkimage.c

