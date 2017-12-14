
all: new_xt_tablet
	
new_xt_tablet : new_xt_tablet.o
	cc -o new_xt_tablet new_xt_tablet.o -lGLw -lGL -lXm -lXt -lXi -lXext -lX11

clean:
	-rm -f *.o
	-rm new_xt_tablet

