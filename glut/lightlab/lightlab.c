
/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <GL/glut.h>

enum {
  BRASS, RED_PLASTIC, EMERALD, SLATE
} MaterialType;
enum {
  TORUS_MATERIAL = 1, TEAPOT_MATERIAL = 2, ICO_MATERIAL = 3
} MaterialDisplayList;
enum {
  LIGHT_OFF, LIGHT_RED, LIGHT_WHITE, LIGHT_GREEN
} LightValues;

GLfloat redLight[] = {1.0, 0.0, 0.0, 1.0},
        greenLight[] = {0.0, 1.0, 0.0, 1.0},
        whiteLight[] = {1.0, 1.0, 1.0, 1.0};
GLfloat leftLightPosition[] = {-1.0, 0.0, 1.0, 0.0},
        rightLightPosition[] = {1.0, 0.0, 1.0, 0.0};
GLfloat brassAmbient[] = {0.33, 0.22, 0.03, 1.0},
        brassDiffuse[] = {0.78, 0.57, 0.11, 1.0},
        brassSpecular[] = {0.99, 0.91, 0.81, 1.0},
        brassShineiness = 27.8;
GLfloat redPlasticAmbient[] = {0.0, 0.0, 0.0},
        redPlasticDiffuse[] = {0.5, 0.0, 0.0},
        redPlasticSpecular[] = {0.7, 0.6, 0.6},
        redPlasticShininess = 32.0;
GLfloat emeraldAmbient[] = {0.0215, 0.1745, 0.0215},
        emeraldDiffuse[] = {0.07568, 0.61424, 0.07568},
        emeraldSpecular[] = {0.633, 0.727811, 0.633},
        emeraldShininess = 76.8;
GLfloat slateAmbient[] = {0.02, 0.02, 0.02},
        slateDiffuse[] = {0.02, 0.01, 0.01},
        slateSpecular[] = {0.4, 0.4, 0.4},
        slateShininess = .78125;
int shadeModel = GL_SMOOTH;
char *leftLight, *rightLight;
char *icoMaterial, *teapotMaterial, *torusMaterial;

/* printf-style interface for rendering text using a GLUT
   stroke font. */
void
output(GLfloat x, GLfloat y, char *format,...)
{
  va_list args;
  char buffer[200], *p;

  /* Use stdarg.h macros for variable argument list processing. */
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
  glPushMatrix();
  glTranslatef(x, y, 0);
  for (p = buffer; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
  glPopMatrix();
}

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
    glScalef(1.3, 1.3, 1.3);
    glRotatef(20.0, 1.0, 0.0, 0.0);
    glPushMatrix();
      glTranslatef(-0.65, 0.7, 0.0);
      glRotatef(90.0, 1.0, 0.0, 0.0);
      glCallList(TORUS_MATERIAL);
      glutSolidTorus(0.275, 0.85, 10, 15);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(-0.75, -0.8, 0.0);
      glCallList(TEAPOT_MATERIAL);
      glutSolidTeapot(0.7);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(1.0, 0.0, -1.0);
      glCallList(ICO_MATERIAL);
      glutSolidIcosahedron();
    glPopMatrix();
  glPopMatrix();
  glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
      glLoadIdentity();
      gluOrtho2D(0, 3000, 0, 3000);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
        glLoadIdentity();
        output(80, 2800, "Torus: %s", torusMaterial);
        output(80, 2650, "Icosahedron: %s", icoMaterial);
        output(80, 2500, "Teapot: %s", teapotMaterial);
        output(80, 250, "Left light: %s", leftLight);
        output(1700, 250, "Right light: %s", rightLight);
        output(850, 100, "Shade model: %s",
          shadeModel == GL_SMOOTH ? "smooth" : "flat");
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  glPopAttrib();
  glutSwapBuffers();
}

void
lightSelect(GLenum which, int value, char **label)
{
  glEnable(which);
  switch (value) {
  case LIGHT_OFF:
    *label = "off";
    glDisable(which);
    break;
  case LIGHT_RED:
    *label = "red";
    glLightfv(which, GL_DIFFUSE, redLight);
    break;
  case LIGHT_WHITE:
    *label = "white";
    glLightfv(which, GL_DIFFUSE, whiteLight);
    break;
  case LIGHT_GREEN:
    *label = "green";
    glLightfv(which, GL_DIFFUSE, greenLight);
    break;
  }
  glutPostRedisplay();
}

void
leftLightSelect(int value)
{
  lightSelect(GL_LIGHT0, value, &leftLight);
}

void
rightLightSelect(int value)
{
  lightSelect(GL_LIGHT1, value, &rightLight);
}

void
material(int dlist, GLfloat * ambient, GLfloat * diffuse,
  GLfloat * specular, GLfloat shininess)
{
  glNewList(dlist, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  glEndList();
}

char *
materialSelect(int object, int value)
{
  glutPostRedisplay();
  switch (value) {
  case BRASS:
    material(object, brassAmbient,
      brassDiffuse, brassSpecular, brassShineiness);
    return "brass";
  case RED_PLASTIC:
    material(object, redPlasticAmbient, redPlasticDiffuse,
      redPlasticSpecular, redPlasticShininess);
    return "red plastic";
  case EMERALD:
    material(object, emeraldAmbient, emeraldDiffuse,
      emeraldSpecular, emeraldShininess);
    return "emerald";
  case SLATE:
    material(object, slateAmbient, slateDiffuse,
      slateSpecular, slateShininess);
    return "slate";
  }
  return NULL;          /* avoid bogus warning! */
}

void
torusSelect(int value)
{
  torusMaterial = materialSelect(TORUS_MATERIAL, value);
}

void
teapotSelect(int value)
{
  teapotMaterial = materialSelect(TEAPOT_MATERIAL, value);
}

void
icoSelect(int value)
{
  icoMaterial = materialSelect(ICO_MATERIAL, value);
}

void
mainMenuSelect(int value)
{
  if (value == 666)
    exit(0);
  glShadeModel(shadeModel = value);
  glutPostRedisplay();
}

int
main(int argc, char **argv)
{
  int leftLightMenu, rightLightMenu, torusMenu, teapotMenu, icoMenu;

  glutInitWindowSize(400, 400);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Lighting Laboratory");
  glutDisplayFunc(display);

#define LIGHT_MENU_ENTRIES() \
    glutAddMenuEntry("Disable", LIGHT_OFF); \
    glutAddMenuEntry("Red", LIGHT_RED); \
    glutAddMenuEntry("White", LIGHT_WHITE); \
    glutAddMenuEntry("Green", LIGHT_GREEN);
#define MATERIAL_MENU_ENTRIES() \
    glutAddMenuEntry("Brass", BRASS); \
    glutAddMenuEntry("Red plastic", RED_PLASTIC); \
    glutAddMenuEntry("Emerald", EMERALD); \
    glutAddMenuEntry("Slate", SLATE);

  leftLightMenu = glutCreateMenu(leftLightSelect);
  LIGHT_MENU_ENTRIES();
  rightLightMenu = glutCreateMenu(rightLightSelect);
  LIGHT_MENU_ENTRIES();
  torusMenu = glutCreateMenu(torusSelect);
  MATERIAL_MENU_ENTRIES();
  teapotMenu = glutCreateMenu(teapotSelect);
  MATERIAL_MENU_ENTRIES();
  icoMenu = glutCreateMenu(icoSelect);
  MATERIAL_MENU_ENTRIES();

  glutCreateMenu(mainMenuSelect);
  glutAddMenuEntry("Smooth shading", GL_SMOOTH);
  glutAddMenuEntry("Flat shading", GL_FLAT);
  glutAddSubMenu("Left light", leftLightMenu);
  glutAddSubMenu("Right light", rightLightMenu);
  glutAddSubMenu("Torus", torusMenu);
  glutAddSubMenu("Teapot", teapotMenu);
  glutAddSubMenu("Icosahedron", icoMenu);
  glutAddMenuEntry("Quit", 666);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  glLightfv(GL_LIGHT0, GL_POSITION, leftLightPosition);
  glLightfv(GL_LIGHT0, GL_SPECULAR, whiteLight);
  glLightfv(GL_LIGHT1, GL_POSITION, rightLightPosition);
  glLightfv(GL_LIGHT1, GL_SPECULAR, whiteLight);
  leftLightSelect(LIGHT_RED);
  rightLightSelect(LIGHT_GREEN);
  torusSelect(RED_PLASTIC);
  teapotSelect(BRASS);
  icoSelect(EMERALD);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glLineWidth(1.0);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* degrees field of view */ 50.0,
    /* aspect ratio */ 1.0, /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
  glTranslatef(0.0, 0.0, -1.0);

  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
