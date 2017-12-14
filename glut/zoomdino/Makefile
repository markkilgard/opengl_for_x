
all: zoomdino

CFLAGS = -I.. -O

zoomdino : zoomdino.o  ../libglut/libglut.a
	cc -o zoomdino zoomdino.o -L../libglut -lglut -lGL -lGLU -lXmu -lXext -lX11

clean:
	-rm -f *.o
	-rm zoomdino

