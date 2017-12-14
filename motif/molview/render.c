
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* molview: render.h - interface for OpenGL molecule rendering code. */

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <stdio.h>

#include "molview.h"

int sphereVersion = LO_RES_SPHERE;
float curquat[4];
float lastquat[4];

static float mat_specular[] = {.72, .8, .93, 1.0};
static float mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
static float mat_shininess[] = {128.0};

static float light_ambient[] = {0.1, 0.1, 0.1, 1.0};
static float light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
static float light_specular[] = {1.0, 1.0, 1.0, 1.0};
static float light_position[] = {1.0, 1.0, 1.5, 0.0};
static float light0_position[] = {-1.0, -1.0, 1.5, 0.0};

void
renderInit(void)
{
  GLUquadricObj *quadObj;

  quadObj = gluNewQuadric();
  gluQuadricDrawStyle(quadObj, GLU_FILL);
  gluQuadricOrientation(quadObj, GLU_OUTSIDE);
  gluQuadricNormals(quadObj, GLU_SMOOTH);

  /* hi-detail sphere */
  glNewList(HI_RES_SPHERE, GL_COMPILE);
  gluSphere(quadObj, 1.0, 32, 32);
  glEndList();

  /* lo-detail sphere */
  glNewList(LO_RES_SPHERE, GL_COMPILE);
  gluSphere(quadObj, 1.0, 6, 6);
  glEndList();

  gluDeleteQuadric(quadObj);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.4, 0.4, 0.4, 0.0);
  glClearDepth(1.0);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);

  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -4);

  trackball(curquat, 0.0, 0.0, 0.0, 0.0);
}

static GLfloat winWidth, winHeight;

void
renderReshape(int width, int height)
{
  float aspect;

  winWidth = width;
  winHeight = height;
  glViewport(0, 0, winWidth, winHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  aspect = ((float) winWidth) / winHeight;
  gluPerspective(60.0, aspect, 1.0, 7.0);
  glMatrixMode(GL_MODELVIEW);
}

void
renderAtoms(Molecule * mol)
{
  AtomInstance *atom;
  float radius;
  int name;

#define SCALE_FACTOR 4.0

  atom = mol->instances;

  glEnable(GL_COLOR_MATERIAL);
  glScalef(SCALE_FACTOR / mol->maxdim,
    SCALE_FACTOR / mol->maxdim,
    SCALE_FACTOR / mol->maxdim);
  glTranslatef(-(mol->xmin + mol->xsize / 2),
    -(mol->ymin + mol->ysize / 2),
    -(mol->zmin + mol->zsize / 2));

  name = 0;
  while (atom) {
    glLoadName(name);
    glPushMatrix();
    glTranslatef(atom->x, atom->y, atom->z);
    radius = atom->atom->radius;
    glScalef(radius, radius, radius);
    glColor3ubv(atom->atom->color);
    glCallList(sphereVersion);
    glPopMatrix();
    atom = atom->next;
    name++;
  }
  glDisable(GL_COLOR_MATERIAL);
}

#ifdef DEBUG
void
sniff_for_opengl_errors(void)
{
  int error;

  printf("sniff\n");
  while ((error = glGetError()) != GL_NO_ERROR)
    fprintf(stderr, "GL error: %s\n", gluErrorString(error));
}
#endif

void
renderMolecule(Molecule * mol)
{
  GLfloat m[4][4];

  glPushMatrix();
  build_rotmatrix(m, curquat);
  glMultMatrixf(&m[0][0]);
  renderAtoms(mol);
  glPopMatrix();
#ifdef DEBUG
  sniff_for_opengl_errors();
#endif
}

void
renderNoMolecule(void)
{
  extern XFontStruct *fontInfo;
  static char message[] = "No molecule loaded.";
  static int width = 0;

  if (width == 0)
    width = XTextWidth(fontInfo, message, sizeof(message));
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glDisable(GL_LIGHTING);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, winWidth, 0, winHeight);
  glColor3f(0.0, 0.0, 0.0);
  glRasterPos2i(0, 0);
  glBitmap(0, 0, 0, 0,
    winWidth / 2 - width / 2,
    winHeight / 2 - (fontInfo->ascent + fontInfo->descent) / 2, 0);
  glCallLists(sizeof(message), GL_UNSIGNED_BYTE, message);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void
renderScene(Molecule * mol)
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  if (mol) {
    renderMolecule(mol);
  } else {
    renderNoMolecule();
  }
  swap();
}
