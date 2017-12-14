
all: glutsphere

CFLAGS = -I.. -O

glutsphere : glutsphere.o  ../libglut/libglut.a
	cc -o glutsphere glutsphere.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11

clean:
	-rm -f *.o
	-rm glutsphere

