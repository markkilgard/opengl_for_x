
all: paperplane motifonly
	
paperplane : paperplane.o
	cc -o paperplane paperplane.o -lGLw -lGL -lXm -lXt -lXext -lX11 -lm

motifonly : paperplane.c
	cc -DnoGLwidget -o motifonly paperplane.c -lGL -lXm -lXt -lXext -lX11 -lm
clean:
	-rm -f *.o
	-rm paperplane motifonly

