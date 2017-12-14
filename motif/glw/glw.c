
/* Copyright (c) Mark J. Kilgard, 1995, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdio.h>
#include <Xm/Form.h>    /* Motif Form widget. */
#include <Xm/Frame.h>   /* Motif Frame widget. */
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>  /* For XA_RGB_DEFAULT_MAP. */
#include <X11/Xmu/StdCmap.h>  /* For XmuLookupStandardColormap. */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/GLw/GLwMDrawA.h>  /* Motif OpenGL drawing area. */

static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 12,
  GLX_RED_SIZE, 1, None};
static int dblBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 12,
  GLX_DOUBLEBUFFER, GLX_RED_SIZE, 1, None};
static String fallbackResources[] = {
  "*glxarea*width: 300", "*glxarea*height: 300",
  "*frame*x: 20", "*frame*y: 20",
  "*frame*topOffset: 20", "*frame*bottomOffset: 20",
  "*frame*rightOffset: 20", "*frame*leftOffset: 20",
  "*frame*shadowType: SHADOW_IN", NULL
};
Display *dpy;
XtAppContext app;
XtWorkProcId workId = 0;
Widget toplevel, form, frame, glxarea;
XVisualInfo *visinfo;
GLXContext glxcontext;
Colormap cmap;
Bool doubleBuffer = True, spinning = False;

void draw(void);
Boolean spin(XtPointer clientData);
void mapStateChanged(Widget w, XtPointer clientData, XEvent * event, Boolean * cont);
Colormap getShareableColormap(XVisualInfo * vi);
void graphicsInit(Widget w, XtPointer clientData, XtPointer call);
void expose(Widget w, XtPointer clientData, XtPointer call);
void resize(Widget w, XtPointer clientData, XtPointer call);
void input(Widget w, XtPointer clientData, XtPointer callData);

int
main(int argc, char **argv)
{
  /* Step 1. */
  toplevel = XtAppInitialize(&app, "Glw", NULL, 0, &argc, argv,
    fallbackResources, NULL, 0);

  /* Step 2. */
  XtAddEventHandler(toplevel, StructureNotifyMask,
    False, mapStateChanged, NULL);

  /* Step 3. */
  dpy = XtDisplay(toplevel);
  visinfo = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
  if (visinfo == NULL) {
    visinfo = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);
    if (visinfo == NULL)
      XtAppError(app, "no good visual");
    doubleBuffer = False;
  }

  /* Step 4. */
  form = XmCreateForm(toplevel, "form", NULL, 0);
  XtManageChild(form);
  frame = XmCreateFrame(form, "frame", NULL, 0);
  XtVaSetValues(frame,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);
  XtManageChild(frame);

  /* Step 5. */
  cmap = getShareableColormap(visinfo);

  /* Step 6. */
  glxarea = XtVaCreateManagedWidget("glxarea",
    glwMDrawingAreaWidgetClass, frame,
    GLwNvisualInfo, visinfo,
    XtNcolormap, cmap,
    NULL);

  /* Step 7. */
  XtAddCallback(glxarea, GLwNginitCallback, graphicsInit, NULL);
  XtAddCallback(glxarea, GLwNexposeCallback, expose, NULL);
  XtAddCallback(glxarea, GLwNresizeCallback, resize, NULL);
  XtAddCallback(glxarea, GLwNinputCallback, input, NULL);

  /* Step 8. */
  XtRealizeWidget(toplevel);
  XtAppMainLoop(app);
  return 0;             /* ANSI C requires main to return int. */
}

void
graphicsInit(Widget w, XtPointer clientData, XtPointer call)
{
  XVisualInfo *visinfo;

  /* Create OpenGL rendering context. */
  XtVaGetValues(w, GLwNvisualInfo, &visinfo, NULL);
  glxcontext = glXCreateContext(XtDisplay(w), visinfo,
    0,                  /* No sharing. */
    True);              /* Direct rendering if possible. */

  /* Setup OpenGL state. */
  glXMakeCurrent(XtDisplay(w), XtWindow(w), glxcontext);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);  /* clear to black */
  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, 1.0, 10.0, 200.0);
  glMatrixMode(GL_MODELVIEW);
  glTranslatef(0.0, 0.0, -50.0);
  glRotatef(-58.0, 0.0, 1.0, 0.0);
}

void
resize(Widget w,
  XtPointer clientData, XtPointer call)
{
  GLwDrawingAreaCallbackStruct *callData;

  callData = (GLwDrawingAreaCallbackStruct *) call;
  glXMakeCurrent(XtDisplay(w), XtWindow(w), glxcontext);
  glXWaitX();
  glViewport(0, 0, callData->width, callData->height);
}

void
expose(Widget w,
  XtPointer clientData, XtPointer call)
{
  draw();
}

void
draw(void)
{
  glXMakeCurrent(dpy, XtWindow(glxarea), glxcontext);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(-10.0, -10.0, 0.0);
    glColor3f(0.7, 0.7, 0.7);
    glVertex3f(10.0, -10.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f(-10.0, 10.0, 0.0);
  glEnd();
  glBegin(GL_POLYGON);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(0.0, -10.0, -10.0);
    glColor3f(0.0, 1.0, 0.7);
    glVertex3f(0.0, -10.0, 10.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 5.0, -10.0);
  glEnd();
  glBegin(GL_POLYGON);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(-10.0, 6.0, 4.0);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(-10.0, 3.0, 4.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(4.0, -9.0, -10.0);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(4.0, -6.0, -10.0);
  glEnd();

  if (doubleBuffer)
    glXSwapBuffers(dpy, XtWindow(glxarea));
  else
    glFlush();

  /* Avoid indirect rendering latency from queuing. */
  if (!glXIsDirect(dpy, glxcontext))
    glFinish();
}

void
input(Widget w, XtPointer clientData, XtPointer callData)
{
  XmDrawingAreaCallbackStruct *cd = (XmDrawingAreaCallbackStruct *) callData;
  char buffer[1];
  KeySym keysym;

  switch (cd->event->type) {
  case KeyPress:
    if (XLookupString((XKeyEvent *) cd->event, buffer, 1, &keysym, NULL) > 0) {
      switch (keysym) {
      case XK_space:   /* The spacebar. */
        if (spinning) {
          XtRemoveWorkProc(workId);
          spinning = False;
        } else {
          workId = XtAppAddWorkProc(app, spin, NULL);
          spinning = True;
        }
        break;
      }
    }
    break;
  }
}

Boolean
spin(XtPointer clientData)
{
  glXMakeCurrent(dpy, XtWindow(glxarea), glxcontext);
  glRotatef(2.5, 1.0, 0.0, 0.0);
  draw();
  return False;         /* Leave work proc active. */
}

void
mapStateChanged(Widget w, XtPointer clientData,
  XEvent * event, Boolean * cont)
{
  switch (event->type) {
  case MapNotify:
    if (spinning && workId != 0)
      workId = XtAppAddWorkProc(app, spin, NULL);
    break;
  case UnmapNotify:
    if (spinning)
      XtRemoveWorkProc(workId);
    break;
  }
}

Colormap
getShareableColormap(XVisualInfo * vi)
{
  Status status;
  XStandardColormap *standardCmaps;
  Colormap cmap;
  int i, numCmaps;

  /* Be lazy; using DirectColor too involved for this example. */
  if (vi->class != TrueColor)
    XtAppError(app, "no support for non-TrueColor visual");
  /* If no standard colormap but TrueColor, just make an
     unshared one. */
  status = XmuLookupStandardColormap(dpy, vi->screen, vi->visualid,
    vi->depth, XA_RGB_DEFAULT_MAP,
    False,              /* Replace. */
    True);              /* Retain. */
  if (status == 1) {
    status = XGetRGBColormaps(dpy, RootWindow(dpy, vi->screen),
      &standardCmaps, &numCmaps, XA_RGB_DEFAULT_MAP);
    if (status == 1)
      for (i = 0; i < numCmaps; i++)
        if (standardCmaps[i].visualid == vi->visualid) {
          cmap = standardCmaps[i].colormap;
          XFree(standardCmaps);
          return cmap;
        }
  }
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  return cmap;
}
