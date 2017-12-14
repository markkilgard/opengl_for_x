
all: blender

CFLAGS = -I.. -O

blender : blender.o  ../libglut/libglut.a
	cc -o blender blender.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm blender

