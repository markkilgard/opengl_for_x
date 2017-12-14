
all: old_xt_tablet
	
old_xt_tablet : old_xt_tablet.o
	cc -o old_xt_tablet old_xt_tablet.o -lGLw -lGL -lXm -lXt -lXi -lXext -lX11

clean:
	-rm -f *.o
	-rm old_xt_tablet

