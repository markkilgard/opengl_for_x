
/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <GL/glut.h>

GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
GLUquadricObj *qobj;

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCallList(1);      /* Render sphere display list. */
  glutSwapBuffers();
}

void
gfxinit(void)
{
  qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_FILL);
  glNewList(1, GL_COMPILE);  /* Create sphere display list. */
  gluSphere(qobj, /* Radius */ 1.0,
    /* Slices */ 20,  /* Stacks */ 20);
  glEndList();
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* Field of view in degree */ 40.0, 
    /* Aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 3,  /* Eye is at (0,0,3) */
    0.0, 0.0, 0.0,      /* Center is at (0,0,0) */
    0.0, 1.0, 0.);      /* Up is in positive Y direction. */
  glTranslatef(0.0, 0.0, -1.0);
}

void
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("sphere");
  glutDisplayFunc(display);
  gfxinit();
  glutMainLoop();
}
