
all: glxdino
	
glxdino : glxdino.o
	cc -o glxdino glxdino.o -lGLU -lGL -lXmu -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm -f glxdino

