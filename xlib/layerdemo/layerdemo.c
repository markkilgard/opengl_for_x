
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* compile: cc -o layerdemo layerdemo.c sovlayerutil.c -lX11 -lm */

/* Tested on Silicon Graphics IRIX 4.0 and IRIX 5.0
   workstations and Hewlett-Packard workstations with CRX24
   and CRX48Z graphics hardware. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sovLayerUtil.h"

#define SIZE 400        /* Width and height of window. */

Display *dpy;
Window root, win, overlay;
Colormap cmap;
Visual *defaultVisual;
int screen, black, white, red, nVisuals, i, status;
GC normalGC, overlayGC;
XEvent event;
sovVisualInfo template;
sovVisualInfo *otherLayerInfo, *defaultLayerInfo;
XSetWindowAttributes winattrs;
XGCValues gcvals;
XColor color, exact;
int x = 0, y = SIZE / 2;

void
redrawNormalPlanes(void)
{
  /* Draw a black 43 legged octopus. */
  for (i = 0; i < 43; i++)
    XDrawLine(dpy, win, normalGC, SIZE / 2, SIZE / 2,
      (int) (cos(i * 0.15) * (SIZE / 2 - 5)) + SIZE / 2,
      (int) (sin(i * 0.15) * (SIZE / 2 - 12)) + SIZE / 2);
}

#define MESSAGE1 "This text is in the"
#define MESSAGE2 "OVERLAY PLANES"

void
redrawOverlayPlanes(void)
{
  XDrawString(dpy, overlay, overlayGC, x, y,
    MESSAGE1, sizeof(MESSAGE1) - 1);
  XDrawString(dpy, overlay, overlayGC, x, y + 15,
    MESSAGE2, sizeof(MESSAGE2) - 1);
}

void
fatalError(char *message)
{
  fprintf(stderr, "layerdemo: %s\n", message);
  exit(1);
}

void
main(int argc, char *argv[])
{
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    fatalError("cannot open display");
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);
  defaultVisual = DefaultVisual(dpy, screen);
  /* Find layer of default visual. */
  template.vinfo.visualid = defaultVisual->visualid;
  defaultLayerInfo = sovGetVisualInfo(dpy, VisualIDMask,
    &template, &nVisuals);
  /* Look for visual in layer "above" default visual with
     transparent pixel. */
  template.layer = defaultLayerInfo->layer + 1;
  template.vinfo.screen = screen;
  template.type = TransparentPixel;
  otherLayerInfo = sovGetVisualInfo(dpy,
    VisualScreenMask | VisualLayerMask | VisualTransparentType,
    &template, &nVisuals);
  if (otherLayerInfo == NULL) {
    /* Make sure default visual has transparent pixel. */
    if (defaultLayerInfo->type == None)
      fatalError("unable to find expected layer visuals");
    /* Visual not found "above" default visual, try looking
       "below". */
    template.layer = defaultLayerInfo->layer - 1;
    template.vinfo.screen = screen;
    otherLayerInfo = sovGetVisualInfo(dpy,
      VisualScreenMask | VisualLayerMask, &template, &nVisuals);
    if (otherLayerInfo == NULL)
      fatalError("unable to find layer below default visual");
    /* XCreateColormap uses AllocNone for two reasons:  1)
       haven't determined class of visual, visual could have
       static colormap and more importantly 2) transparent
       pixel might make AllocAll impossible. */
    cmap = XCreateColormap(dpy, root,
      otherLayerInfo->vinfo.visual, AllocNone);
    /* Not default colormap, must find our own black and white. */
    status = XAllocNamedColor(dpy, cmap, "black", &color, &exact);
    if (status == 0)
      fatalError("could not allocate black");
    black = color.pixel;
    status = XAllocNamedColor(dpy, cmap, "white", &color, &exact);
    if (status == 0)
      fatalError("could not allocate white");
    white = color.pixel;
    winattrs.background_pixel = white;
    winattrs.border_pixel = black;
    winattrs.colormap = cmap;
    win = XCreateWindow(dpy, root, 10, 10, SIZE, SIZE, 0,
      otherLayerInfo->vinfo.depth,
      InputOutput, otherLayerInfo->vinfo.visual,
      CWBackPixel | CWBorderPixel | CWColormap, &winattrs);
    status = XAllocNamedColor(dpy, DefaultColormap(dpy, screen),
      "red", &color, &exact);
    if (status == 0)
      fatalError("could not allocate red");
    winattrs.background_pixel = defaultLayerInfo->value;
    winattrs.border_pixel = 0;
    winattrs.colormap = DefaultColormap(dpy, screen);
    overlay = XCreateWindow(dpy, win, 0, 0, SIZE, SIZE, 0,
      DefaultDepth(dpy, screen),
      InputOutput, defaultVisual,
      CWBackPixel | CWBorderPixel | CWColormap, &winattrs);
  } else {
    /* Create lower window using default visual. */
    black = BlackPixel(dpy, screen);
    white = WhitePixel(dpy, screen);
    win = XCreateSimpleWindow(dpy, root, 10, 10, SIZE, SIZE, 1,
      black, white);
    /* See note above about AllocNone. */
    cmap = XCreateColormap(dpy, root,
      otherLayerInfo->vinfo.visual, AllocNone);
    status = XAllocNamedColor(dpy, cmap, "red", &color, &exact);
    if (status == 0)
      fatalError("could not allocate red");
    red = color.pixel;
    /* Use transparent pixel. */
    winattrs.background_pixel = otherLayerInfo->value;
    winattrs.border_pixel = 0;  /* No border but still
                                   necessary to avoid BadMatch. */
    winattrs.colormap = cmap;
    overlay = XCreateWindow(dpy, win, 0, 0, SIZE, SIZE, 0,
      otherLayerInfo->vinfo.depth,
      InputOutput, otherLayerInfo->vinfo.visual,
      CWBackPixel | CWBorderPixel | CWColormap, &winattrs);
  }
  XSelectInput(dpy, win, ExposureMask);
  XSelectInput(dpy, overlay, ExposureMask | ButtonPressMask);
  XSetWMColormapWindows(dpy, win, &overlay, 1);
  gcvals.foreground = black;
  gcvals.line_width = 8;
  gcvals.cap_style = CapRound;
  normalGC = XCreateGC(dpy, win,
    GCForeground | GCLineWidth | GCCapStyle, &gcvals);
  gcvals.foreground = red;
  overlayGC = XCreateGC(dpy, overlay, GCForeground, &gcvals);
  XMapSubwindows(dpy, win);
  XMapWindow(dpy, win);
  while (1) {
    XNextEvent(dpy, &event);
    switch (event.type) {
    case Expose:
      if (event.xexpose.window == win)
        redrawNormalPlanes();
      else
        redrawOverlayPlanes();
      break;
    case ButtonPress:
      x = random() % SIZE / 2;
      y = random() % SIZE;
      XClearWindow(dpy, overlay);
      redrawOverlayPlanes();
      break;
    }
  }
}
