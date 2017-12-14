
all: pixmap2eps
	
pixmap2eps : pixmap2eps.o
	cc -o pixmap2eps pixmap2eps.c -lGLU -lGL -lXext -lX11

clean:
	-rm -f *.o
	-rm -f pixmap2eps dino.bw.eps dino.rgb.eps

