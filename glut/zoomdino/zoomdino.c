
/* Copyright (c) Mark J. Kilgard, 1994.  */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* zoomdino demonstrates GLUT 3.0's new overlay support.  Both
   rubber-banding the display of a help message use the
   overlays. */

#include <stdio.h>
#include <math.h>       /* for sqrt() */
#include <GL/glut.h>

typedef enum {
  RESERVED, BODY_SIDE, BODY_EDGE, BODY_WHOLE, ARM_SIDE, ARM_EDGE, ARM_WHOLE,
  LEG_SIDE, LEG_EDGE, LEG_WHOLE, EYE_SIDE, EYE_EDGE, EYE_WHOLE, DINOSAUR
} displayLists;

GLfloat angle = -150;   /* in degrees */
GLfloat angleDelta = 0; /* in degrees */
int doubleBuffer = 1, iconic = 0, keepAspect = 0;
int spinning, spinBegin;
int W = 300, H = 300;
GLdouble bodyWidth = 2.0;
GLfloat body[][2] = { {0, 3}, {1, 1}, {5, 1}, {8, 4}, {10, 4}, {11, 5},
  {11, 11.5}, {13, 12}, {13, 13}, {10, 13.5}, {13, 14}, {13, 15}, {11, 16},
  {8, 16}, {7, 15}, {7, 13}, {8, 12}, {7, 11}, {6, 6}, {4, 3}, {3, 2},
  {1, 2} };
GLfloat arm[][2] = { {8, 10}, {9, 9}, {10, 9}, {13, 8}, {14, 9}, {16, 9},
  {15, 9.5}, {16, 10}, {15, 10}, {15.5, 11}, {14.5, 10}, {14, 11}, {14, 10},
  {13, 9}, {11, 11}, {9, 11} };
GLfloat leg[][2] = { {8, 6}, {8, 4}, {9, 3}, {9, 2}, {8, 1}, {8, 0.5}, {9, 0},
  {12, 0}, {10, 1}, {10, 2}, {12, 4}, {11, 6}, {10, 7}, {9, 7} };
GLfloat eye[][2] = { {8.75, 15}, {9, 14.7}, {9.6, 14.7}, {10.1, 15},
  {9.6, 15.25}, {9, 15.25} };
GLfloat lightZeroPosition[] = {10.0, 4.0, 10.0, 1.0};
GLfloat lightZeroColor[] = {0.8, 1.0, 0.8, 1.0}; /* green-tinted */
GLfloat lightOnePosition[] = {-1.0, -2.0, 1.0, 0.0};
GLfloat lightOneColor[] = {0.6, 0.3, 0.2, 1.0}; /* red-tinted */
GLfloat skinColor[] = {0.1, 1.0, 0.1, 1.0}, eyeColor[] = {1.0, 0.2, 0.2, 1.0};
int overlaySupport, red, transparent, rubberBanding;
int anchorX, anchorY, stretchX, stretchY;
float viewX, viewY, viewX2, viewY2, viewWidth, viewHeight;
float windowX, windowY, windowX2, windowY2, windowW, windowH;

void
extrudeSolidFromPolygon(GLfloat data[][2], unsigned int dataSize,
  GLdouble thickness, GLuint side, GLuint edge, GLuint whole)
{
  static GLUtriangulatorObj *tobj = NULL;
  GLdouble vertex[3], dx, dy, len;
  int i;
  int count = dataSize / (2 * sizeof(GLfloat));

  if (tobj == NULL) {
    tobj = gluNewTess();  /* create and initialize a GLU
                             polygontesselation object */
    gluTessCallback(tobj, GLU_BEGIN, glBegin);
    gluTessCallback(tobj, GLU_VERTEX, glVertex2fv);  /* semi-tricky 

                                                      */
    gluTessCallback(tobj, GLU_END, glEnd);
  }
  glNewList(side, GL_COMPILE);
  glShadeModel(GL_SMOOTH);  /* smooth minimizes seeing
                               tessellation */
  gluBeginPolygon(tobj);
  for (i = 0; i < count; i++) {
    vertex[0] = data[i][0];
    vertex[1] = data[i][1];
    vertex[2] = 0;
    gluTessVertex(tobj, vertex, data[i]);
  }
  gluEndPolygon(tobj);
  glEndList();
  glNewList(edge, GL_COMPILE);
  glShadeModel(GL_FLAT);  /* flat shade keeps angular hands
                             from being "smoothed" */
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= count; i++) {
    /* mod function handles closing the edge */
    glVertex3f(data[i % count][0], data[i % count][1], 0.0);
    glVertex3f(data[i % count][0], data[i % count][1], thickness);

    /* Calculate a unit normal by dividing by Euclidean
       distance. We could be lazy and use
       glEnable(GL_NORMALIZE) so we could pass in arbitrary
       normals for a very slight performance hit. */

    dx = data[(i + 1) % count][1] - data[i % count][1];
    dy = data[i % count][0] - data[(i + 1) % count][0];
    len = sqrt(dx * dx + dy * dy);
    glNormal3f(dx / len, dy / len, 0.0);
  }
  glEnd();
  glEndList();
  glNewList(whole, GL_COMPILE);
  glFrontFace(GL_CW);
  glCallList(edge);
  glNormal3f(0.0, 0.0, -1.0);  /* constant normal for side */
  glCallList(side);
  glPushMatrix();
  glTranslatef(0.0, 0.0, thickness);
  glFrontFace(GL_CCW);
  glNormal3f(0.0, 0.0, 1.0);  /* opposite normal for other side 

                               */
  glCallList(side);
  glPopMatrix();
  glEndList();
}

void
makeDinosaur(void)
{
  GLfloat bodyWidth = 3.0;

  extrudeSolidFromPolygon(body, sizeof(body), bodyWidth,
    BODY_SIDE, BODY_EDGE, BODY_WHOLE);
  extrudeSolidFromPolygon(arm, sizeof(arm), bodyWidth / 4,
    ARM_SIDE, ARM_EDGE, ARM_WHOLE);
  extrudeSolidFromPolygon(leg, sizeof(leg), bodyWidth / 2,
    LEG_SIDE, LEG_EDGE, LEG_WHOLE);
  extrudeSolidFromPolygon(eye, sizeof(eye), bodyWidth + 0.2,
    EYE_SIDE, EYE_EDGE, EYE_WHOLE);
  glNewList(DINOSAUR, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, skinColor);
  glCallList(BODY_WHOLE);
  glPushMatrix();
  glTranslatef(0.0, 0.0, bodyWidth);
  glCallList(ARM_WHOLE);
  glCallList(LEG_WHOLE);
  glTranslatef(0.0, 0.0, -bodyWidth - bodyWidth / 4);
  glCallList(ARM_WHOLE);
  glTranslatef(0.0, 0.0, -bodyWidth / 4);
  glCallList(LEG_WHOLE);
  glTranslatef(0.0, 0.0, bodyWidth / 2 - 0.1);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, eyeColor);
  glCallList(EYE_WHOLE);
  glPopMatrix();
  glEndList();
}

void
resetProjection(void)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  viewX = -1.0;
  viewWidth = 2.0;
  viewY = -1.0;
  viewHeight = 2.0;
  glFrustum(viewX, viewX + viewWidth, viewY, viewY + viewHeight, 1.0, 40);
  glMatrixMode(GL_MODELVIEW);
}

void redraw(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void vis(int visble);
void controlLights(int value);
void redrawOverlay(void);
void idle(void);

void
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("zoomdino");
  glutDisplayFunc(redraw);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutVisibilityFunc(vis);

  glutCreateMenu(controlLights);
  glutAddMenuEntry("Toggle right light", 1);
  glutAddMenuEntry("Toggle left light", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  makeDinosaur();
  resetProjection();
  gluLookAt(0.0, 0.0, 30.0,  /* eye is at (0,0,30) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);
  glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_INDEX);
  overlaySupport = glutLayerGet(GLUT_OVERLAY_POSSIBLE);
  if (overlaySupport == 0) {
    printf("Sorry, no whizzy zoomdino overlay usage!\n");
  } else {
    glutEstablishOverlay();
    glutHideOverlay();
    transparent = glutLayerGet(GLUT_TRANSPARENT_INDEX);
    glClearIndex(transparent);
    red = (transparent + 1) % glutGet(GLUT_WINDOW_COLORMAP_SIZE);
    glutSetColor(red, 1.0, 0.0, 0.0);  /* Red. */
    glutOverlayDisplayFunc(redrawOverlay);
    glutSetWindowTitle("zoomdino with rubber-banding");
  }
  glutMainLoop();
}

void
redraw(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glRotatef(angle, 0.0, 1.0, 0.0);
  glTranslatef(-8, -8, -bodyWidth / 2);
  glCallList(DINOSAUR);
  glPopMatrix();
  glutSwapBuffers();
}

void
reshape(int w, int h)
{
  if (overlaySupport) {
    glutUseLayer(GLUT_OVERLAY);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glScalef(1, -1, 1);
    glTranslatef(0, -h, 0);
    glMatrixMode(GL_MODELVIEW);
    glutUseLayer(GLUT_NORMAL);
  }
  glViewport(0, 0, w, h);
  W = w;
  H = h;
}

void
redrawOverlay(void)
{
  static int prevStretchX, prevStretchY;

  if (glutLayerGet(GLUT_OVERLAY_DAMAGED)) {
    /* Damage means we need a full clear. */
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    /* Undraw last rubber-band. */
    glIndexi(transparent);
    glBegin(GL_LINE_LOOP);
    glVertex2i(anchorX, anchorY);
    glVertex2i(anchorX, prevStretchY);
    glVertex2i(prevStretchX, prevStretchY);
    glVertex2i(prevStretchX, anchorY);
    glEnd();
  }
  glIndexi(red);
  glBegin(GL_LINE_LOOP);
  glVertex2i(anchorX, anchorY);
  glVertex2i(anchorX, stretchY);
  glVertex2i(stretchX, stretchY);
  glVertex2i(stretchX, anchorY);
  glEnd();
  prevStretchX = stretchX;
  prevStretchY = stretchY;
}

void
mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
      spinning = 1;
      spinBegin = x;
      angleDelta = 0;
    } else if (state == GLUT_UP) {
      glutSetCursor(GLUT_CURSOR_INHERIT);
      spinning = 0;
      if (angleDelta == 0)
        glutIdleFunc(NULL);
    }
  }
  if (button == GLUT_MIDDLE_BUTTON) {
    if (state == GLUT_DOWN) {
      rubberBanding = 1;
      anchorX = x;
      anchorY = y;
      stretchX = x;
      stretchY = y;
      if (overlaySupport)
        glutShowOverlay();
    } else if (state == GLUT_UP) {
      if (overlaySupport)
        glutHideOverlay();
      rubberBanding = 0;
      glutUseLayer(GLUT_NORMAL);

#define max(a,b)  ((a) > (b) ? (a) : (b))
#define min(a,b)  ((a) < (b) ? (a) : (b))

      windowX = min(anchorX, stretchX);
      windowY = min(H - anchorY, H - stretchY);
      windowX2 = max(anchorX, stretchX);
      windowY2 = max(H - anchorY, H - stretchY);

      windowW = windowX2 - windowX;
      windowH = windowY2 - windowY;
      if (windowW == 0 || windowH == 0) {
        resetProjection();
      } else {
        viewX2 = windowX2 / W * viewWidth + viewX;
        viewX = windowX / W * viewWidth + viewX;
        viewY2 = windowY2 / H * viewHeight + viewY;
        viewY = windowY / H * viewHeight + viewY;
        viewWidth = viewX2 - viewX;
        viewHeight = viewY2 - viewY;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(viewX, viewX + viewWidth, viewY, viewY + viewHeight, 1.0, 40);
        glMatrixMode(GL_MODELVIEW);
      }
      glutPostRedisplay();
    }
  }
}

void
motion(int x, int y)
{
  if (rubberBanding) {
    stretchX = x;
    stretchY = y;
    if (overlaySupport)
      glutPostOverlayRedisplay();
  }
  if (spinning) {
    angleDelta = (x - spinBegin) / 2;
    if (angleDelta)
      glutIdleFunc(idle);
    else
      glutIdleFunc(NULL);
    spinBegin = x;
  }
}

void
idle(void)
{
  angle += angleDelta;
  glutPostRedisplay();
}

void
vis(int visible)
{
  if (visible != GLUT_VISIBLE) {
    if (angleDelta)
      glutIdleFunc(NULL);
  } else {
    if (angleDelta)
      glutIdleFunc(idle);
  }
}

int lightZeroSwitch = 1, lightOneSwitch = 1;

void
controlLights(int value)
{
  glutUseLayer(GLUT_NORMAL);
  switch (value) {
  case 1:
    lightZeroSwitch = !lightZeroSwitch;
    if (lightZeroSwitch)
      glEnable(GL_LIGHT0);
    else
      glDisable(GL_LIGHT0);
    break;
  case 2:
    lightOneSwitch = !lightOneSwitch;
    if (lightOneSwitch)
      glEnable(GL_LIGHT1);
    else
      glDisable(GL_LIGHT1);
    break;
  }
  glutPostRedisplay();
}
