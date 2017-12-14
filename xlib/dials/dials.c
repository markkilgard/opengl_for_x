
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/Xatom.h>  /* for XA_RGB_DEFAULT_MAP atom */
#include <X11/Xmu/StdCmap.h>  /* for XmuLookupStandardColormap */
#include <X11/extensions/XInput.h>

#define PI            3.14159265358979323846
#define NUM_DIALS     8
#define NUM_BUTTONS   32
#define RELEASE       0
#define PRESS         1

Display *dpy;
Window win;
Atom wmDeleteWindow;
XDevice *dialsDevice;
int deviceMotionNotify = 0, deviceButtonPress = 0,
  deviceButtonPressGrab = 0, deviceButtonRelease = 0;
int dials[NUM_DIALS], buttons[NUM_BUTTONS];

static void fatalError(char *message);
static Colormap getShareableColormap(XVisualInfo * vi);
static void display(void);
static void resize(int width, int height);
static void initDialsAndButtons(void);
static void processEvents(void);

void
main(int argc, char **argv)
{
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;
  GLXContext cx;
  int width = 480, height = 420;
  int configuration[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None};

  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    fatalError("Could not open display.");

  if (!glXQueryExtension(dpy, NULL, NULL))
    fatalError("X server has no OpenGL GLX extension.");

  /* find an OpenGL-capable RGB visual with depth buffer */
  vi = glXChooseVisual(dpy, DefaultScreen(dpy), configuration);
  if (vi == NULL)
    fatalError("No appropriate double buffered RGB visual.");
  cmap = getShareableColormap(vi);

  /* create an OpenGL rendering context */
  cx = glXCreateContext(dpy, vi,
    NULL,            /* No sharing of display lists */
    True);           /* Direct rendering if possible */
  if (cx == NULL)
    fatalError("Could not create rendering context.");

  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | StructureNotifyMask;
  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
    0, 0, width, height,
    0, vi->depth, InputOutput, vi->visual,
    CWBorderPixel | CWColormap | CWEventMask, &swa);
  XSetStandardProperties(dpy, win, "OpenGL dials & buttons",
    "dials", None, argv, argc, NULL);
  wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);

  glXMakeCurrent(dpy, win, cx);
  glClearColor(0.5, 0.5, 0.5, 1.0);
  glLineWidth(3.0);
  resize(width, height);

  initDialsAndButtons();

  XMapWindow(dpy, win);
  processEvents();
}

int numButtons;
int numDials;
int *dialsResolution;

static void
initDialsAndButtons(void)
{
  XExtensionVersion *version;
  XDeviceInfoPtr deviceInfo, device;
  XAnyClassPtr any;
  XButtonInfoPtr b;
  XValuatorInfoPtr v;
  XAxisInfoPtr a;
  int numDevices, numButtons, numDials;
  int i, j, k;
  XEventClass eventList[4];

  version = XGetExtensionVersion(dpy, "XInputExtension");
  if (version != NULL && ((int) version) != NoSuchExtension) {
    XFree(version);
    deviceInfo = XListInputDevices(dpy, &numDevices);
    if (deviceInfo) {
      for (i = 0; i < numDevices; i++) {
        device = &deviceInfo[i];
        any = (XAnyClassPtr) device->inputclassinfo;
        if (!strcmp(device->name, "dial+buttons")) {
          v = NULL;
          b = NULL;
          for (j = 0; j < device->num_classes; j++) {
            switch (any->class) {
            case ButtonClass:
              b = (XButtonInfoPtr) any;
              numButtons = b->num_buttons;
              break;
            case ValuatorClass:
              v = (XValuatorInfoPtr) any;
              numDials = v->num_axes;
              dialsResolution = (int *) malloc(sizeof(int) * numDials);
              a = (XAxisInfoPtr) ((char *) v + sizeof(XValuatorInfo));
              for (k = 0; k < numDials; k++, a++) {
                dialsResolution[k] = a->resolution;
              }
            }
            any = (XAnyClassPtr) ((char *) any + any->length);
          }
          dialsDevice = XOpenDevice(dpy, device->id);
          if (dialsDevice) {
            DeviceMotionNotify(dialsDevice, deviceMotionNotify,
              eventList[0]);
            DeviceButtonPress(dialsDevice, deviceButtonPress,
              eventList[1]);
            DeviceButtonPressGrab(dialsDevice, deviceButtonPressGrab,
              eventList[2]);
            DeviceButtonRelease(dialsDevice, deviceButtonRelease,
              eventList[3]);
            XSelectExtensionEvent(dpy, win, eventList, 4);
            break;
          }
        }
      }
      XFreeDeviceList(deviceInfo);
    }
  }
}

static int
scaleDialValue(int axis, int rawValue)
{
  /* XXX Assumption made that the resolution of the device is
     number of clicks for one complete dial revolution.  This
     is true for SGI's dial & button box. */
  return (rawValue * 360.0) / dialsResolution[axis];
}

static void
processEvents(void)
{
  int needRedraw = 1;
  XEvent event;

  while (1) {
    do {
      XNextEvent(dpy, &event);
      switch (event.type) {
      case ConfigureNotify:
        resize(event.xconfigure.width, event.xconfigure.height);
        /* fall through... */
      case Expose:
        needRedraw = 1;
        break;
      case ClientMessage:
        if (event.xclient.data.l[0] == wmDeleteWindow)
          exit(0);
        break;
      default:
        if (deviceMotionNotify && event.type == deviceMotionNotify) {
          XDeviceMotionEvent *devmot = (XDeviceMotionEvent *) & event;

          if (devmot->deviceid == dialsDevice->device_id && devmot->window == win) {
            int i, first = devmot->first_axis, count = devmot->axes_count;

            for (i = first; i < first + count; i++) {
              if (i <= NUM_DIALS) {
                dials[i] = scaleDialValue(i, devmot->axis_data[i - first]) % 360;
              }
            }
            needRedraw = 1;
          }
        } else if (deviceButtonPress
          && event.type == deviceButtonPress) {
          XDeviceButtonEvent *devbtn = (XDeviceButtonEvent *) & event;

          if (devbtn->deviceid == dialsDevice->device_id
            && devbtn->window == win) {
            buttons[devbtn->button - 1] = PRESS;
            needRedraw = 1;
          }
        } else if (deviceButtonRelease
          && event.type == deviceButtonRelease) {
          XDeviceButtonEvent *devbtn = (XDeviceButtonEvent *) & event;

          if (devbtn->deviceid == dialsDevice->device_id
            && devbtn->window == win) {
            buttons[devbtn->button - 1] = RELEASE;
            needRedraw = 1;
          }
        }
      }
    } while (XPending(dpy));  /* loop to compress events */
    if (needRedraw) {
      display();
      needRedraw = 0;
    }
  }
}

static void
resize(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glScalef(1, -1, 1);
  glTranslatef(0, -h, 0);
}

static void
drawCircle(int x, int y, int r, int dir)
{
  float angle;

  glPushMatrix();
  glTranslatef(x, y, 0);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0, 0);
  for (angle = 2 * PI; angle >= 0; angle -= PI / 12) {
    glVertex2f(r * cos(angle), r * sin(angle));
  }
  glEnd();
  glColor3f(0, 0, 1);
  glBegin(GL_LINES);
  glVertex2f(0, 0);
  glVertex2f(r * cos(dir * PI / 180), r * sin(dir * PI / 180));
  glEnd();
  glPopMatrix();
}

static void
displayDials(void)
{
  int i;

  for (i = 0; i < NUM_DIALS; i++) {
    glColor3f(0, 1, 0);
    drawCircle(60 + ((i + 1) % 2) * 100, 60 + (i / 2) * 100,
      40, dials[NUM_DIALS - 1 - i] - 90);
  }
}

static void
displayButtons(void)
{
  int i, n;

  for (i = 0, n = 0; i < NUM_BUTTONS; i++, n++) {
    switch (n) {
    case 0:        /* No button in device's upper left corner. */
    case 5:        /* No button in device's upper right corner. */
    case 30:       /* No button in device's lower left corner. */
      n++;         /* Skip drawing this location. */
    }
    if (buttons[i] == PRESS) {
      glColor3f(1, 0, 0);
    } else {
      glColor3f(1, 1, 1);
    }
    glRecti((n % 6) * 40 + 240, (n / 6) * 40 + 20,
      (n % 6) * 40 + 260, (n / 6) * 40 + 40);
  }
}

static void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  displayDials();
  displayButtons();
  glXSwapBuffers(dpy, win);
}

static Colormap
getShareableColormap(XVisualInfo * vi)
{
  Status status;
  XStandardColormap *standardCmaps;
  Colormap cmap;
  int i, numCmaps;

  /* be lazy; using DirectColor too involved for this example */
  if (vi->class != TrueColor)
    fatalError("No support for non-TrueColor visual.");
  /* If no standard colormap but TrueColor, just make an
     unshared one. */
  status = XmuLookupStandardColormap(dpy, vi->screen, vi->visualid,
    vi->depth, XA_RGB_DEFAULT_MAP, /* replace */ False,  /* retain
                                                          */ True);
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
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
    vi->visual, AllocNone);
  return cmap;
}

static void
fatalError(char *message)
{
  fprintf(stderr, "dials: %s\n", message);
  exit(1);
}
