
all: sovinfo
	
sovinfo : sovinfo.o sovlayerutil.o
	cc -o sovinfo sovinfo.o sovlayerutil.o -lX11

clean:
	-rm -f *.o
	-rm sovinfo

