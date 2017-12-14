
all: xdevices
	
xdevices : xdevices.o
	cc -o xdevices xdevices.o -lXi -lXext -lX11

clean:
	-rm -f *.o
	-rm xdevices

