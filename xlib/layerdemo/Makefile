
all: layerdemo
	
layerdemo : layerdemo.o sovlayerutil.o
	cc -o layerdemo layerdemo.o sovlayerutil.o -lX11 -lm

clean:
	-rm -f *.o
	-rm layerdemo

