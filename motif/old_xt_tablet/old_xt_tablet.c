
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* old_xt_tablet.c - OpenGL Motif program using tablet events */

/* compile: cc -o old_xt_tablet old_xt_tablet.c -lGLw -lGLU -lGL -lXm -lXt -Xi -lXext -lX11 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <X11/GLw/GLwMDrawA.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>  /* For IEVENTS,
                                        unfortunately not in
                                        XInput.h */
int attribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
String fallbackResources[] = {
  "*glxwidget*width: 300", "*glxwidget*height: 300",
  NULL
};
Display *dpy;
XtAppContext appctx;
Widget toplevel, frame, glxwidget;
XDevice *tablet;
int tabletMotionNotify, tabletPressNotify, tabletReleaseNotify;
GLenum btnState[4] = {GL_LINE, GL_LINE, GL_LINE, GL_LINE};
int axisMin[2], axisRange[2];
int tabletPos[2] = {2000, 2000};
Bool direct, redisplayPosted = False;

void expose(Widget w, XtPointer client_data, XtPointer call);
void resize(Widget w, XtPointer client_data, XtPointer call);
void initializeTablet(void);
void MyAppMainLoop(XtAppContext app);

void
main(int argc, char *argv[])
{
  XVisualInfo *visinfo;
  GLXContext glxcontext;

  toplevel = XtOpenApplication(&appctx, "tablet", NULL, 0, &argc, argv,
    fallbackResources, applicationShellWidgetClass, NULL, 0);
  dpy = XtDisplay(toplevel);

  frame = XmCreateFrame(toplevel, "frame", NULL, 0);
  XtManageChild(frame);

  /* specify visual directly */
  if (!(visinfo = glXChooseVisual(dpy, DefaultScreen(dpy), attribs)))
    XtAppError(appctx, "no suitable RGB visual");

  glxwidget = XtVaCreateManagedWidget("glxwidget", glwMDrawingAreaWidgetClass,
    frame, GLwNvisualInfo, visinfo, XtNwidth, 300, XtNheight, 300, NULL);
  XtAddCallback(glxwidget, GLwNexposeCallback, expose, NULL);
  XtAddCallback(glxwidget, GLwNresizeCallback, resize, NULL);

  initializeTablet();

  XtRealizeWidget(toplevel);

  glxcontext = glXCreateContext(dpy, visinfo, 0, GL_TRUE);
  direct = glXIsDirect(dpy, glxcontext);
  GLwDrawingAreaMakeCurrent(glxwidget, glxcontext);

  glMatrixMode(GL_PROJECTION);
  glOrtho(0, 4000, 0, 4000, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.5, 0.5, 0.5, 0.);

  MyAppMainLoop(appctx);
}

void tabletHandler(Widget w, XtPointer client_data,
  XEvent * event, Boolean * continue_to_dispatch);
int xiEventBase;

void
MyAppMainLoop(XtAppContext app)
{
  XEvent event;
  Boolean continue_to_dispatch;

  while (1) {
    XtAppNextEvent(app, &event);
    if ((event.xany.type >= xiEventBase) && (event.xany.type < xiEventBase + IEVENTS)) {
      tabletHandler(XtWindowToWidget(event.xany.display, event.xany.window),
        NULL, &event, &continue_to_dispatch);
      continue;
    }
    XtDispatchEvent(&event);
  }
}

void
initializeTablet(void)
{
  Bool exists;
  XExtensionVersion *version;
  XDeviceInfoPtr deviceInfo, device;
  XAnyClassPtr any;
  XEventClass tabletMotionClass, tabletPressClass, tabletPressGrabClass,
    tabletReleaseClass;
  XButtonInfoPtr b;
  XValuatorInfoPtr v;
  XAxisInfoPtr a;
  int opcode, eventBase, error_base;
  int num_dev;
  int i, j, k;

  exists = XQueryExtension(dpy, "XInputExtension", &opcode,
    &eventBase, &error_base);
  if (!exists) {
    goto noDevices;
  }
  version = XGetExtensionVersion(dpy, "XInputExtension");
  if (version == NULL || ((int) version) == NoSuchExtension) {
    goto noDevices;
  }
  XFree(version);
  deviceInfo = XListInputDevices(dpy, &num_dev);
  if (deviceInfo) {
    for (i = 0; i < num_dev; i++) {
      device = &deviceInfo[i];
      any = (XAnyClassPtr) device->inputclassinfo;

      if (!strcmp(device->name, "tablet")) {
        v = NULL;
        b = NULL;
        for (j = 0; j < device->num_classes; j++) {
          switch (any->class) {
          case ButtonClass:
            b = (XButtonInfoPtr) any;
            /* Sanity check: at least 1 button (normally 4). */
            if (b->num_buttons < 1) {
              goto skip_device;
            }
            break;
          case ValuatorClass:
            v = (XValuatorInfoPtr) any;
            /* Sanity check: exactly 2 valuators? */
            if (v->num_axes != 2) {
              goto skip_device;
            }
            a = (XAxisInfoPtr) ((char *) v + sizeof(XValuatorInfo));
            for (k = 0; k < 2; k++, a++) {
              axisMin[k] = a->min_value;
              axisRange[k] = a->max_value - a->min_value;
            }
            break;
          }
          any = (XAnyClassPtr) ((char *) any + any->length);
        }
        tablet = XOpenDevice(dpy, device->id);
        if (tablet) {
          XEventClass eventList[4];

          xiEventBase = eventBase;

          DeviceMotionNotify(tablet,
            tabletMotionNotify, tabletMotionClass);
          DeviceButtonPress(tablet,
            tabletPressNotify, tabletPressClass);
          DeviceButtonPressGrab(tablet,
            notUsedByMacro, tabletPressGrabClass);
          DeviceButtonRelease(tablet,
            tabletReleaseNotify, tabletReleaseClass);

          XtRealizeWidget(toplevel);
          eventList[0] = tabletMotionClass;
          eventList[1] = tabletPressClass;
          eventList[2] = tabletPressGrabClass;
          eventList[3] = tabletReleaseClass;
          XSelectExtensionEvent(dpy, XtWindow(glxwidget), eventList, 4);

          XFreeDeviceList(deviceInfo);
          return;
        }
      }
    skip_device:;
    }
    XFreeDeviceList(deviceInfo);
  }
noDevices:
  fprintf(stderr, "old_xt_tablet: no tablet device found!\n");
  fprintf(stderr, "               continuing without tablet support.\n");
  return;
}

void tabletPosChange(int first, int count, int *data);
void postRedisplay(void);

void
tabletHandler(Widget w, XtPointer client_data,
  XEvent * event, Boolean * continue_to_dispatch)
{
  if (event->type == tabletMotionNotify) {
    XDeviceMotionEvent *devmot = (XDeviceMotionEvent *) event;

    tabletPosChange(devmot->first_axis, devmot->axes_count,
      devmot->axis_data);
  } else if (event->type == tabletPressNotify) {
    XDeviceButtonPressedEvent *devbtn = (XDeviceButtonEvent *) event;

    btnState[devbtn->button - 1] = GL_FILL;
    postRedisplay();
  } else if (event->type == tabletReleaseNotify) {
    XDeviceButtonReleasedEvent *devbtn = (XDeviceButtonEvent *) event;

    btnState[devbtn->button - 1] = GL_LINE;
    postRedisplay();
  }
}

int normalizeTabletPos(int axis, int rawValue);
void queryTabletPos(void);

void
tabletPosChange(int first, int count, int *data)
{
  int i, value, genEvent = 0;

  for (i = first; i < first + count; i++) {
    switch (i) {
    case 0:            /* X axis */
    case 1:            /* Y axis */
      value = normalizeTabletPos(i, data[i - first]);
      if (value != tabletPos[i]) {
        tabletPos[i] = value;
        genEvent = 1;
      }
      break;
    }
  }
  if (tabletPos[0] == -1 || tabletPos[1] == -1) {
    queryTabletPos();
    genEvent = 1;
  }
  if (genEvent) {
    postRedisplay();
  }
}

int
normalizeTabletPos(int axis, int rawValue)
{
  assert(rawValue >= axisMin[axis]);
  assert(rawValue <= axisMin[axis] + axisRange[axis]);
  /* Normalize rawValue to between 0 and 4000. */
  return ((rawValue - axisMin[axis]) * 4000) /
    axisRange[axis];
}

void
queryTabletPos(void)
{
  XDeviceState *state;
  XInputClass *any;
  XValuatorState *v;
  int i;

  state = XQueryDeviceState(dpy, tablet);
  any = state->data;
  for (i = 0; i < state->num_classes; i++) {
    switch (any->class) {
    case ValuatorClass:
      v = (XValuatorState *) any;
      if (v->num_valuators < 2)
        goto end;
      if (tabletPos[0] == -1)
        tabletPos[0] = normalizeTabletPos(0, v->valuators[0]);
      if (tabletPos[1] == -1)
        tabletPos[1] = normalizeTabletPos(1, v->valuators[1]);
    }
    any = (XInputClass *) ((char *) any + any->length);
  }
end:
  XFreeDeviceState(state);
}

void drawScene(void);

void
doRedisplay(void *client_data, XtIntervalId * id)
{
  drawScene();
  redisplayPosted = False;
}

void
postRedisplay(void)
{
  if (!redisplayPosted) {
    redisplayPosted = True;
    XtAppAddTimeOut(appctx, 5, doRedisplay, NULL);
  }
}

void
expose(Widget w, XtPointer client_data, XtPointer call)
{
  postRedisplay();
}

void
diamond(GLenum mode)
{
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK, mode);
  glRotatef(45., 0., 0., 1.);
  glRectf(-0.5, -0.5, 1, 1);
  glPopMatrix();
}

void
puck(void)
{
  /* Make a puck out of 4 diamonds. */

  glTranslatef(0, 1.2, 0);
  glColor3f(1.0, 1.0, 0.0);
  diamond(btnState[0]);

  glTranslatef(-1.2, -1.2, 0);
  glColor3f(1.0, 1.0, 1.0);
  diamond(btnState[1]);

  glTranslatef(1.2, -1.2, 0);
  glColor3f(0.0, 0.0, 1.0);
  diamond(btnState[2]);

  glTranslatef(1.2, 1.2, 0);
  glColor3f(0.0, 1.0, 0.0);
  diamond(btnState[3]);
}

void
drawScene(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glPushMatrix();
  glTranslatef(tabletPos[0], tabletPos[1], 0);
  glScalef(200, 200, 1);
  puck();
  glPopMatrix();
  GLwDrawingAreaSwapBuffers(glxwidget);
  /* To improve net interactivity. */
  if (!direct)
    glFinish();
}

void
resize(Widget w, XtPointer client_data, XtPointer call)
{
  GLwDrawingAreaCallbackStruct *call_data;
  call_data = (GLwDrawingAreaCallbackStruct *) call;

  glXWaitX();
  glViewport(0, 0, call_data->width, call_data->height);
}
