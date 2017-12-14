
all: dials
	
dials : dials.o
	cc -o dials dials.o -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm -f dials

