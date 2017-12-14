
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* Molecule Viewer user interface */

#include <stdlib.h>
#include <stdio.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/LabelG.h>
#include <X11/GLw/GLwMDrawA.h>  /* Motif OpenGL drawing area
                                   widget */
#include <GL/glx.h>

#include "molview.h"
#include "sovLayerUtil.h"

static int config[] = {
  None, None,           /* Space for multisampling GLX
                           attributes if supported. */
  GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 12,
  GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
  None
};
static int *dblBuf = &config[2];
static int *snglBuf = &config[3];
static String fallbackResources[] = {
  "*sgiMode: true",     /* Try to enable Indigo Magic look & feel */
  "*useSchemes: all",   /* and SGI schemes. */
  "*title: Molecule Viewer",
  "*filebox_popup*title: Open molecule...",
  "*glxarea*width: 400",
  "*glxarea*height: 300",
  NULL
};
static XtActionsRec actionsTable[] = {
  {"startRotation", startRotation},
  {"rotation", rotation},
  {"doPick", doPick},
};

XtAppContext app;
Display *dpy;
Bool doubleBuffer = True, madeCurrent = False;
Widget toplevel, mainw, menubar, menupane, btn, cascade, frame, glxarea, dialog, popup, cmdw;
WidgetList menuWidgets;
Window glxwin;
GLXContext cx;
XVisualInfo *vi = NULL;
Arg args[10];
XFontStruct *fontInfo;
Dimension viewWidth, viewHeight;
char *textLabels[] = {"Atom Name:", "XYZ Position:", "Radius:"};
Widget labels[XtNumber(textLabels)];
Visual *overlayVisual = NULL;
int overlayDepth;
Colormap overlayColormap;

static void
detectOverlaySupport(Display *dpy)
{
  sovVisualInfo template, *overlayVisuals;
  int layer, nVisuals, i, entries;

  /* Need more than two colormap entries for reasonable menus. */
  entries = 2;
  for (layer = 1; layer <= 3; layer++) {
    template.layer = layer;
    template.vinfo.screen = DefaultScreen(dpy);
    overlayVisuals = sovGetVisualInfo(dpy,
      VisualScreenMask | VisualLayerMask, &template, &nVisuals);
    if (overlayVisuals) {
      for (i = 0; i < nVisuals; i++) {
        if (overlayVisuals[i].vinfo.visual->map_entries > entries) {
          overlayVisual = overlayVisuals[i].vinfo.visual;
          overlayDepth = overlayVisuals[i].vinfo.depth;
	  entries = overlayVisual->map_entries;
        }
      }
      XFree(overlayVisuals);
    }
  }
  if (overlayVisual)
    overlayColormap = XCreateColormap(dpy, DefaultRootWindow(dpy),
      overlayVisual, AllocNone);
}

static int
isSupportedByGLX(Display * dpy, char *extension)
{
#if defined(GLX_VERSION_1_1)
  static const char *extensions = NULL;
  const char *start;
  char *where, *terminator;
  int major, minor;

  glXQueryVersion(dpy, &major, &minor);
  /* Be careful not to call glXQueryExtensionsString if it
     looks like the server doesn't support GLX 1.1.
     Unfortunately, the original GLX 1.0 didn't have the notion 

     of GLX extensions. */
  if ((major == 1 && minor >= 1) || (major > 1)) {
    if (!extensions)
      extensions = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
    /* It takes a bit of care to be fool-proof about parsing
       the GLX extensions string.  Don't be fooled by
       sub-strings,  etc. */
    start = extensions;
    for (;;) {
      where = strstr(start, extension);
      if (!where)
        return 0;
      terminator = where + strlen(extension);
      if (where == start || *(where - 1) == ' ') {
        if (*terminator == ' ' || *terminator == '\0') {
          return 1;
        }
      }
      start = terminator;
    }
  }
#else
  /* No GLX extensions before GLX 1.1 */
#endif
  return 0;
}

int
main(int argc, char *argv[])
{
  static XmButtonType buttonTypes[] =
  {
    XmPUSHBUTTON, XmPUSHBUTTON, XmSEPARATOR, XmCHECKBUTTON, XmCHECKBUTTON,
  };
  XmString buttonLabels[XtNumber(buttonTypes)];
  int n, i;

  toplevel = XtAppInitialize(&app, "Molview", NULL, 0, &argc, argv,
    fallbackResources, NULL, 0);
  XtAppAddActions(app, actionsTable, XtNumber(actionsTable));
  dpy = XtDisplay(toplevel);

#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
  if (isSupportedByGLX(dpy, "GLX_SGIS_multisample")) {
    config[0] = GLX_SAMPLES_SGIS;
    config[1] = 4;
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), config);
  }
#endif

  if (vi == NULL) {
    /* Find an OpenGL-capable RGB visual with depth buffer. */
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
    if (vi == NULL) {
      vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);
      if (vi == NULL)
        XtAppError(app, "no RGB visual with depth buffer");
      doubleBuffer = False;
    }
  }
  /* Create an OpenGL rendering context. */
  cx = glXCreateContext(dpy, vi, /* No display list sharing */ None,
  /* Favor direct rendering */ True);
  if (cx == NULL)
    XtAppError(app, "could not create rendering context");
  mainw = XtVaCreateWidget("mainw", xmMainWindowWidgetClass, toplevel,
    XmNcommandWindowLocation, XmCOMMAND_BELOW_WORKSPACE,
    NULL);
  XtManageChild(mainw);

  XtAddEventHandler(toplevel, StructureNotifyMask, False,
    mapStateChanged, NULL);

  /* Try to find a good overlay visual for pulldown and popup menus. */
  detectOverlaySupport(dpy);

  /* Create menu bar. */
  menubar = XmCreateMenuBar(mainw, "menubar", NULL, 0);
  XtManageChild(menubar);
  n = 0;
  if (overlayVisual) {
    XtSetArg(args[n], XmNvisual, overlayVisual);
    n++;
    XtSetArg(args[n], XmNdepth, overlayDepth);
    n++;
    XtSetArg(args[n], XmNcolormap, overlayColormap);
    n++;
  }
  menupane = XmCreatePulldownMenu(menubar, "menupane", args, n);
  if (overlayVisual) {
    XtAddCallback(XtParent(menupane), XmNpopupCallback,
      ensurePulldownColormapInstalled, NULL);
  }
  btn = XmCreatePushButton(menupane, "Open molecule...", NULL, 0);
  XtAddCallback(btn, XmNactivateCallback, openMolecule, NULL);
  XtManageChild(btn);
  btn = XmCreatePushButton(menupane, "Quit", NULL, 0);
  XtAddCallback(btn, XmNactivateCallback, quit, NULL);
  XtManageChild(btn);
  XtSetArg(args[0], XmNsubMenuId, menupane);
  cascade = XmCreateCascadeButton(menubar, "File", args, 1);
  XtManageChild(cascade);

  /* Create framed drawing area for OpenGL rendering. */
  frame = XmCreateFrame(mainw, "frame", NULL, 0);
  XtManageChild(frame);
  glxarea = XtVaCreateManagedWidget("glxarea", glwMDrawingAreaWidgetClass,
    frame, GLwNvisualInfo, vi, NULL);
  XtVaGetValues(glxarea, XtNwidth, &viewWidth, XtNheight, &viewHeight, NULL);
  XtAddCallback(glxarea, XmNexposeCallback, draw, NULL);
  XtAddCallback(glxarea, XmNresizeCallback, resize, NULL);
  XtAddCallback(glxarea, GLwNginitCallback, init, NULL);

  /* Create atom pick result text field. */
  cmdw = XtVaCreateWidget("cmdw", xmRowColumnWidgetClass, mainw,
    XmNpacking, XmPACK_COLUMN,
    XmNnumColumns, XtNumber(textLabels),
    XmNisAligned, True,
    XmNentryAlignment, XmALIGNMENT_END,
    XmNorientation, XmHORIZONTAL,
    NULL);
  for (i = 0; i < XtNumber(textLabels); i++) {
    XtVaCreateManagedWidget(textLabels[i],
      xmLabelGadgetClass, cmdw, NULL);
    labels[i] = XtVaCreateManagedWidget("atom_info", xmTextWidgetClass, cmdw,
      XmNeditable, False,
      XmNcursorPositionVisible, False,
      XmNtraversalOn, False,
      XmNcolumns, 25,
      NULL);
  }
  clearPickInfo();
  XtManageChild(cmdw);

  /* Create popup menu. */
  buttonLabels[0] = XmStringCreateLocalized("Solid");
  buttonLabels[1] = XmStringCreateLocalized("Wireframe");
  buttonLabels[2] = NULL;
  buttonLabels[3] = XmStringCreateLocalized("Spin Momentum");
  buttonLabels[4] = XmStringCreateLocalized("Auto HiRes");
  n = 0;
  XtSetArg(args[n], XmNbuttonCount, XtNumber(buttonTypes)); n++;
  XtSetArg(args[n], XmNbuttons, buttonLabels); n++;
  XtSetArg(args[n], XmNbuttonType, buttonTypes); n++;
  XtSetArg(args[n], XmNbuttonSet, 4); n++;
  XtSetArg(args[n], XmNsimpleCallback, processMenuUse); n++;
  if (overlayVisual) {
    XtSetArg(args[n], XmNvisual, overlayVisual); n++;
    XtSetArg(args[n], XmNdepth, overlayDepth); n++;
    XtSetArg(args[n], XmNcolormap, overlayColormap); n++;
  }
  popup = XmCreateSimplePopupMenu(frame, "popup", args, n);
  XtAddEventHandler(frame, ButtonPressMask, False, activateMenu, &popup);
  XmStringFree(buttonLabels[0]);
  XmStringFree(buttonLabels[1]);
  XmStringFree(buttonLabels[3]);
  XmStringFree(buttonLabels[4]);

  XtVaGetValues(popup, XmNchildren, &menuWidgets, NULL);
  XmToggleButtonSetState(menuWidgets[3], True, False);
  XmToggleButtonSetState(menuWidgets[4], True, False);

  /* Set up application's window layout. */
  XmMainWindowSetAreas(mainw, menubar, cmdw, NULL, NULL, frame);
  XtRealizeWidget(toplevel);

  /* Once widget is realized (ie, associated with a created X
     window), we can bind the OpenGL rendering context to the
     window.  */

  glXMakeCurrent(dpy, XtWindow(glxarea), cx);
  madeCurrent = True;

  /* Get font for reporting no atom loaded. */
  fontInfo = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-normal--18-*-*-*-p-*-iso8859-1");
  if (fontInfo == NULL) {
    fontInfo = XLoadQueryFont(dpy, "fixed");
    if (fontInfo == NULL)
      XtAppError(app, "no X font available?");
  }
  glXUseXFont(fontInfo->fid, 32, 96, 1024 + 32);
  glListBase(1024);

  XtAppMainLoop(app);
  return 0;
}
