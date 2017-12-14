
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* Molecule Viewer user interface */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <X11/GLw/GLwMDrawA.h>  /* Motif OpenGL drawing area
                                   widget */
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "molview.h"

Molecule *mol = NULL;
XtWorkProcId animateID, hiResID, redisplayID;
int spinning;
int pendingAutoHiRes;
int beginx, beginy;
int autoHiRes = 1;
int momentum  = 1;
XtTranslations trans = NULL;

static int redisplayPending = 0;

void postRedisplay(void);

void
stopSpinning(void)
{
  if (spinning) {
    spinning = 0;
    XtRemoveTimeOut(animateID);
  }
}

void
stopAutoHiRes(void)
{
  if(pendingAutoHiRes) {
    XtRemoveTimeOut(hiResID);
    pendingAutoHiRes = 0;
  }
}

void
makeHiRes(void)
{
  sphereVersion = HI_RES_SPHERE;
  postRedisplay();
}

void
hiresTimeout(XtPointer closure, XtIntervalId *id)
{
  makeHiRes();
}

void
makeLoRes(void)
{
  stopAutoHiRes();
  sphereVersion = LO_RES_SPHERE;
}

Boolean
handleRedisplay(XtPointer closure)
{
  renderScene(mol);
  if(autoHiRes && !spinning && !pendingAutoHiRes) {
    pendingAutoHiRes = 1;
    hiResID = XtAppAddTimeOut(app, 2000, hiresTimeout, 0);
  }
  redisplayPending = 0;
  return True;
}

void
postRedisplay(void)
{
  if(!redisplayPending) {
    redisplayID = XtAppAddWorkProc(app, handleRedisplay, 0);
    redisplayPending = 1;
  }
}

void
animate(XtPointer closure, XtIntervalId *id)
{
  add_quats(lastquat, curquat, curquat);
  postRedisplay();
  animateID = XtAppAddTimeOut(app, 1, animate, 0);
}

void
startRotation(Widget w, XEvent * event, String * params,
  Cardinal * num_params)
{
  int x, y;

  x = event->xbutton.x;
  y = event->xbutton.y;
  beginx = x;
  beginy = y;
  stopSpinning();
}

void
rotation(Widget w, XEvent * event, String * params,
  Cardinal * num_params)
{
  int x, y;

  x = event->xbutton.x;
  y = event->xbutton.y;
  trackball(lastquat,
    (2.0*beginx - viewWidth) / viewWidth,
    (viewHeight - 2.0*beginy) / viewHeight,
    (2.0*x - viewWidth) / viewWidth,
    (viewHeight - 2.0*y) / viewHeight
    );
  beginx = x;
  beginy = y;
  if (momentum) {
    if(!spinning) {
      spinning = 1;
      makeLoRes();
      animateID = XtAppAddTimeOut(app, 1, animate, 0);
    }
  } else {
    makeLoRes();
    add_quats(lastquat, curquat, curquat);
    postRedisplay();
  }
}

void
updatePickInfo(AtomInstance *atom)
{
  char buf[64];

  if(atom) {
    XmTextSetString(labels[0], atom->atom->name);
    sprintf(buf, "(%.3g, %.3g, %.3g)", atom->x, atom->y, atom->z);
    XmTextSetString(labels[1], buf);
    sprintf(buf, "%g", atom->atom->radius);
    XmTextSetString(labels[2], buf);
  } else {
    XmTextSetString(labels[0], "n/a");
    XmTextSetString(labels[1], "n/a");
    XmTextSetString(labels[2], "n/a");
  }
}

void
clearPickInfo(void)
{
  updatePickInfo(NULL);
}

void
doPick(Widget w, XEvent * event, String * params, Cardinal * num_params)
{
  AtomInstance *atom;
  int x, y;

  x = event->xbutton.x;
  y = event->xbutton.y;
  atom = pickScene(mol, x, y);

  updatePickInfo(atom);
}

void
activateMenu(Widget w, XtPointer clientData, XEvent *event, Boolean *cont)
{
  if(overlayVisual) {
    /* Ensure that overlay popup menu's colormap is installed. */
    XInstallColormap(dpy, overlayColormap);
  }
  XmMenuPosition(popup, &event->xbutton);
  XtManageChild(popup);
}

static char *glxareaTranslations =
  "#override\n\
  <Btn1Down>:startRotation()\n\
  <Btn1Motion>:rotation()\n\
  <Btn2Down>:doPick()\n";

void
draw(Widget w, XtPointer data, XtPointer callData)
{
  postRedisplay();
}

void
resize(Widget w, XtPointer data, XtPointer callData)
{
  GLwDrawingAreaCallbackStruct *resize =
    (GLwDrawingAreaCallbackStruct*) callData;

  if (madeCurrent) {
    viewWidth = resize->width;
    viewHeight = resize->height;
    glXMakeCurrent(dpy, glxwin, cx);
    glXWaitX();
    renderReshape(viewWidth, viewHeight);
  }
}

void
fileBoxOk(Widget w, XtPointer data, XtPointer callData)
{
  XmFileSelectionBoxCallbackStruct *info =
    (XmFileSelectionBoxCallbackStruct *) callData;
  char *filename, *error;

  XmStringGetLtoR(info->value, XmSTRING_DEFAULT_CHARSET, &filename);
  error = fileLoadMolecule(filename, &mol);
  if (filename)
    XtFree(filename);
  if (error) {
    XmString text;

    if (!dialog) {
      dialog = XmCreateErrorDialog(w, "error", NULL, 0);
      XtUnmanageChild(XmMessageBoxGetChild(dialog,
        XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(dialog,
        XmDIALOG_HELP_BUTTON));
    }
    text = XmStringCreateSimple(error);
    XtVaSetValues(dialog, XmNmessageString, text, NULL);
    XmStringFree(text);
    XtManageChild(dialog);
    XtPopup(XtParent(dialog), XtGrabNone);
    return;
  }
  XtPopdown(XtParent(w));
  if (dialog)
    XtPopdown(XtParent(dialog));

  /* Delay setting up user interaction translation until there
     is data to interact with. */
  if (trans == NULL) {
    trans = XtParseTranslationTable(glxareaTranslations);
    XtOverrideTranslations(glxarea, trans);
  }

  updatePickInfo(NULL);
  postRedisplay();
}

void
fileboxCancel(Widget w, XtPointer data, XtPointer callData)
{
  XtPopdown(XtParent(w));
  if (dialog)
    XtPopdown(XtParent(dialog));
}

void
openMolecule(Widget w, XtPointer data, XtPointer callData)
{
  static Widget filebox = NULL;
  Widget button;
  Arg args[1];

  if (filebox == NULL) {
    XtSetArg(args[0], XmNpattern,
      XmStringCreate("*.mol", XmSTRING_DEFAULT_CHARSET));
    filebox = XmCreateFileSelectionDialog(toplevel, "filebox",
      args, 1);
    XtAddCallback(filebox, XmNcancelCallback, fileboxCancel, 0);
    XtAddCallback(filebox, XmNokCallback, fileBoxOk, 0);
    /* Remove help button. */
    button = XmFileSelectionBoxGetChild(filebox,
      XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(button);
  }
  XtManageChild(filebox);
  XtPopup(XtParent(filebox), XtGrabNone);
}

void
quit(Widget w, XtPointer data, XtPointer callData)
{
  exit(0);
}

void
swap(void)
{
  glXSwapBuffers(dpy, glxwin);
}

void
init(Widget w, XtPointer data, XtPointer callData)
{
  glxwin = XtWindow(w);
  glXMakeCurrent(XtDisplay(w), XtWindow(w), cx);
  renderInit();
  renderReshape(viewWidth, viewHeight);
}

void
processMenuUse(Widget w, XtPointer clientData, XtPointer callData)
{
  int button = (int) clientData;
    switch (button) {
    case 0:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case 1:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case 2:
	momentum = 1 - momentum;
	stopSpinning();
	break;
    case 3:
        autoHiRes = 1 - autoHiRes;
        if(autoHiRes && !spinning)
          makeHiRes();
        else
          makeLoRes();
        break;
    }
    postRedisplay();
}

void
ensurePulldownColormapInstalled(Widget w, XtPointer clientData,
  XtPointer callData)
{
  /* Ensure that overlay pulldown menu's colormap is installed. */
  XInstallColormap(dpy, overlayColormap);
}

void mapStateChanged(Widget w, XtPointer data,
  XEvent * event, Boolean * cont)
{
    switch (event->type) {
    case MapNotify:
        if (spinning) animateID = XtAppAddTimeOut(app, 1, animate, 0);
        break;
    case UnmapNotify:
        if (spinning) XtRemoveTimeOut(animateID);
        break;
    }
}
