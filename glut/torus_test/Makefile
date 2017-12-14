
all: torus_test

CFLAGS = -I.. -O

torus_test : torus.o  gl_harness.o ../libglut/libglut.a
	cc -o torus_test torus.o gl_harness.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm torus_test

