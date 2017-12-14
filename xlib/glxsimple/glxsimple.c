
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* compile: cc -o glxsimple glxsimple.c -lGL -lX11 */
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>

static int snglBuf[] = {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
                        GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 12, None};
static int dblBuf[] =  {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
                        GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 12, GLX_DOUBLEBUFFER, None};

Display *dpy;
Window win;
Bool doubleBuffer = True;
/* Initial 3D box orientation. */
GLfloat xAngle = 42.0, yAngle = 82.0, zAngle = 112.0;

void redraw(void);

void
fatalError(char *message)
{
  fprintf(stderr, "glxsimple: %s\n", message);
  exit(1);
}

void
main(int argc, char **argv)
{
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;
  GLXContext cx;
  XEvent event;
  Bool needRedraw = False, recalcModelView = True;
  int dummy;

  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    fatalError("could not open display");

  if (!glXQueryExtension(dpy, &dummy, &dummy))
    fatalError("X server has no OpenGL GLX extension");

  vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
  if (vi == NULL) {
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);
    if (vi == NULL)
      fatalError("no RGB visual with depth buffer");
    doubleBuffer = False;
  }
  if (vi->class != TrueColor)
    fatalError("TrueColor visual required for this program");

  cx = glXCreateContext(dpy, vi,
    /* No sharing of display lists */ None,
    /* Direct rendering if possible */ True);
  if (cx == NULL)
    fatalError("could not create rendering context");

  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 300, 300, 0, vi->depth,
    InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
  XSetStandardProperties(dpy, win, "glxsimple", "glxsimple", None, argv, argc, NULL);

  glXMakeCurrent(dpy, win, cx);

  XMapWindow(dpy, win);

  /* Enable depth buffering */
  glEnable(GL_DEPTH_TEST);
  /* Set up projection transform. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);

  while (1) {
    do {
      XNextEvent(dpy, &event);
      switch (event.type) {
      case ButtonPress:
        recalcModelView = True;
        switch (event.xbutton.button) {
        case 1:
          xAngle += 10.0;
          break;
        case 2:
          yAngle += 10.0;
          break;
        case 3:
          zAngle += 10.0;
          break;
        }
        break;
      case ConfigureNotify:
        glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
        /* Fall through... */
      case Expose:
        needRedraw = True;
        break;
      }
    } while (XPending(dpy));  /* Loop to compress events. */
    if (recalcModelView) {
      glMatrixMode(GL_MODELVIEW);
      /* Reset modelview matrix to the identity matrix. */
      glLoadIdentity();
      /* Move the camera back three units. */
      glTranslatef(0.0, 0.0, -3.0);
      /* Rotate by X, Y, and Z angles. */
      glRotatef(xAngle, 0.1, 0.0, 0.0);
      glRotatef(yAngle, 0.0, 0.1, 0.0);
      glRotatef(zAngle, 0.0, 0.0, 1.0);
      recalcModelView = False;
      needRedraw = True;
    }
    if (needRedraw) {
      redraw();
      needRedraw = False;
    }
  }
}

void
redraw(void)
{
  static Bool displayListInited = False;

  if (displayListInited) {
    /* If display list already exists, just execute it. */
    glCallList(1);
  } else {
    /* Otherwise compile and execute to create the display
       list. */
    glNewList(1, GL_COMPILE_AND_EXECUTE);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBegin(GL_QUADS);
        /* Front face */
        glColor3f(0.0, 0.7, 0.1);  /* Green */
        glVertex3f(-1.0, 1.0, 1.0);
        glVertex3f(1.0, 1.0, 1.0);
        glVertex3f(1.0, -1.0, 1.0);
        glVertex3f(-1.0, -1.0, 1.0);
        /* Back face */
        glColor3f(0.9, 1.0, 0.0);  /* Yellow */
        glVertex3f(-1.0, 1.0, -1.0);
        glVertex3f(1.0, 1.0, -1.0);
        glVertex3f(1.0, -1.0, -1.0);
        glVertex3f(-1.0, -1.0, -1.0);
        /* Top side face */
        glColor3f(0.2, 0.2, 1.0);  /* Blue */
        glVertex3f(-1.0, 1.0, 1.0);
        glVertex3f(1.0, 1.0, 1.0);
        glVertex3f(1.0, 1.0, -1.0);
        glVertex3f(-1.0, 1.0, -1.0);
        /* Bottom side face */
        glColor3f(0.7, 0.0, 0.1);  /* Red */
        glVertex3f(-1.0, -1.0, 1.0);
        glVertex3f(1.0, -1.0, 1.0);
        glVertex3f(1.0, -1.0, -1.0);
        glVertex3f(-1.0, -1.0, -1.0);
      glEnd();
    glEndList();
    displayListInited = True;
  }
  if (doubleBuffer)
    /* Buffer swap does implicit glFlush. */
    glXSwapBuffers(dpy, win);
  else
    /* Explicit flush for single buffered case. */
    glFlush();          
}
