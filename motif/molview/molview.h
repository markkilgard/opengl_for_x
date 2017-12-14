
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "sovLayerUtil.h"

/*-- mol_data.h --*/

typedef struct _AtomType AtomType;
struct _AtomType {
  char *name;
  float radius;
  GLubyte color[3];     /* red, green, blue */
  AtomType *next;
};

typedef struct _AtomInstance AtomInstance;
struct _AtomInstance {
  AtomType *atom;
  float x, y, z;
  AtomInstance *next;
};

typedef struct {
  int natoms;
  AtomType *types;
  AtomInstance *instances;
  float xmin, ymin, zmin;
  float xsize, ysize, zsize;
  float maxdim;
  GLuint *selbuf;
} Molecule;

/*-- mol_file.h --*/

extern char *fileLoadMolecule(char *filename, Molecule ** return_mol);

/*-- gui_init.h --*/

extern XFontStruct *fontInfo;
extern Widget labels[], glxarea, dialog, popup, toplevel;
extern Window glxwin;
extern Bool doubleBuffer, madeCurrent;
extern GLXContext cx;
extern Display *dpy;
extern XtAppContext app;
extern Visual *overlayVisual;
extern Colormap overlayColormap;
extern Dimension viewWidth, viewHeight;

/*-- gui_run.h --*/

extern XtActionsRec actionsTable[];

extern void startRotation(Widget w, XEvent * event, String * params, Cardinal * num_params);
extern void rotation(Widget w, XEvent * event, String * params, Cardinal * num_params);
extern void doPick(Widget w, XEvent * event, String * params, Cardinal * num_params);

extern void mapStateChanged(Widget w, XtPointer data, XEvent *event, Boolean *cont);

extern void ensurePulldownColormapInstalled(Widget w, XtPointer clientData, XtPointer callData);

extern void openMolecule(Widget w, XtPointer data, XtPointer callData);
extern void quit(Widget w, XtPointer data, XtPointer callData);
extern void draw(Widget w, XtPointer data, XtPointer callData);
extern void resize(Widget w, XtPointer data, XtPointer callData);
extern void init(Widget w, XtPointer data, XtPointer callData);
extern void processMenuUse(Widget w, XtPointer clientData, XtPointer callData);

extern void activateMenu(Widget w, XtPointer clientData, XEvent *event, Boolean *cont);

extern void clearPickInfo(void);
extern void swap(void);

/*-- pick.h --*/

extern AtomInstance *pickScene(Molecule * mol, int x, int y);

/*-- render.h --*/

#define HI_RES_SPHERE 1
#define LO_RES_SPHERE 2

extern float curquat[];
extern float lastquat[];
extern int sphereVersion;
extern void renderInit(void);
extern void renderReshape(int width, int height);
extern void renderMolecule(Molecule *mol);
extern void renderScene(Molecule * mol);

/*-- trackball.h --*/

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y);
void add_quats(float *q1, float *q2, float *dest);
void build_rotmatrix(float m[4][4], float q[4]);
void axis_to_quat(float a[3], float phi, float q[4]);
