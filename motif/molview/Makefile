
all: molview

OBJS = mol_file.o trackball.o render.c pick.o gui_init.o gui_run.o sovlayerutil.o

molview : $(OBJS)
	cc -o molview $(OBJS) -lGLw -lGLU -lGL -lXm -lXt -lXext -lX11 -lm

# The molview_sgi version uses SGI's nicer file chooser widget by linking
# with -lSgm
#
molview_sgi : $(OBJS)
	cc -o molview $(OBJS) -lSgm -lGLw -lGLU -lGL -lXm -lXt -lXext -lX11 -lm

clean:
	-rm -f *.o
	-rm molview molview_sgi

