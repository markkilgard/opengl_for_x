
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>       /* for cos(), sin(), and sqrt() */
#include <GL/glut.h>

/* External texture image array found in mjkimage.c */
extern unsigned char mjk_image[];
extern int mjk_depth, mjk_height, mjk_width;  /* 3, 256, 256 */

float tick1 = 0, tick2 = 0, angle, size;
int set_timeout = 0, interval = 100, minify_menu, rate_menu;
#define CUBE 1
#define SQUARES 2
#define DRUM 3
int mode = SQUARES, spinning = 1, scaling = 1, visible = 0;

void
animate(int value)
{
  if (visible) {
    if (spinning || scaling)
      if (value) {
        if (spinning) {
          tick1 += 4 * (interval / 100.0);
          angle = ((int) tick1) % 360;
        }
        if (scaling) {
          tick2 += 2 * (interval / 100.0);
          size = .7 - .5 * sin(tick2 / 20.0);
        }
      }
  }
  glutPostRedisplay();
  set_timeout = 1;
}

void
redraw(void)
{
  int begin, end, elapsed;
  int i, j;
  float amplitude;

  if (set_timeout)
    begin = glutGet(GLUT_ELAPSED_TIME);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  if (mode != DRUM)
    glScalef(size, size, size);

  switch (mode) {

#define SQ_COLS 6
#define SQ_TILE_TEX_W (1.0/SQ_COLS)
#define SQ_ROWS 6
#define SQ_TILE_TEX_H (1.0/SQ_ROWS)
  case SQUARES:
    glTranslatef(-SQ_COLS / 2.0 + .5, -SQ_ROWS / 2.0 + .5, 0);
    for (i = 0; i < SQ_COLS; i++)
      for (j = 0; j < SQ_ROWS; j++) {
        glPushMatrix();
          glTranslatef(i, j, 0);
          glRotatef(angle, 0, 1, 1);
          glBegin(GL_QUADS);
            glTexCoord2f(i * SQ_TILE_TEX_W, j * SQ_TILE_TEX_H);
            glVertex2f(-.5, -.5);
            glTexCoord2f((i + 1) * SQ_TILE_TEX_W, j * SQ_TILE_TEX_H);
            glVertex2f(.5, -.5);
            glTexCoord2f((i + 1) * SQ_TILE_TEX_W, (j + 1) * SQ_TILE_TEX_H);
            glVertex2f(.5, .5);
            glTexCoord2f(i * SQ_TILE_TEX_W, (j + 1) * SQ_TILE_TEX_H);
            glVertex2f(-.5, .5);
          glEnd();
        glPopMatrix();
      }
    break;

#define DR_COLS 12
#define DR_TILE_TEX_W (1.0/DR_COLS)
#define DR_ROWS 12
#define DR_TILE_TEX_H (1.0/DR_ROWS)
#define Z(x,y)  (((DR_COLS-(x))*(x) + (DR_ROWS-(y))*(y)) * amplitude) - 28.0
  case DRUM:
    glRotatef(angle, 0, 0, 1);
    glTranslatef(-DR_COLS / 2.0 + .5, -DR_ROWS / 2.0 + .5, 0);
    amplitude = 0.4 * sin(tick2 / 6.0);
    for (i = 0; i < DR_COLS; i++)
      for (j = 0; j < DR_ROWS; j++) {
        glPushMatrix();
          glTranslatef(i, j, 0);
          glBegin(GL_QUADS);
            glTexCoord2f(i * DR_TILE_TEX_W, j * DR_TILE_TEX_H);
            glVertex3f(-.5, -.5, Z(i, j));
            glTexCoord2f((i + 1) * DR_TILE_TEX_W, j * DR_TILE_TEX_H);
            glVertex3f(.5, -.5, Z(i + 1, j));
            glTexCoord2f((i + 1) * DR_TILE_TEX_W, (j + 1) * DR_TILE_TEX_H);
            glVertex3f(.5, .5, Z(i + 1, j + 1));
            glTexCoord2f(i * DR_TILE_TEX_W, (j + 1) * DR_TILE_TEX_H);
            glVertex3f(-.5, .5, Z(i, j + 1));
          glEnd();
        glPopMatrix();
      }
    break;
  case CUBE:
    glRotatef(angle, 0, 1, 0);
    glBegin(GL_QUADS);
      /* Front. */
      glTexCoord2f(0.0, 0.0);
      glVertex3f(-1.0, -1.0, 1.0);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(1.0, -1.0, 1.0);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(1.0, 1.0, 1.0);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(-1.0, 1.0, 1.0);
      /* Back. */
      glTexCoord2f(0.0, 1.0);
      glVertex3f(-1.0, 1.0, -1.0);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(1.0, 1.0, -1.0);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(1.0, -1.0, -1.0);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(-1.0, -1.0, -1.0);
      /* Left. */
      glTexCoord2f(0.0, 0.0);
      glVertex3f(-1.0, -1.0, -1.0);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(-1.0, -1.0, 1.0);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(-1.0, 1.0, 1.0);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(-1.0, 1.0, -1.0);
      /* Right. */
      glTexCoord2f(0.0, 1.0);
      glVertex3f(1.0, 1.0, -1.0);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(1.0, 1.0, 1.0);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(1.0, -1.0, 1.0);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(1.0, -1.0, -1.0);
    glEnd();
  }
  glPopMatrix();
  glutSwapBuffers();
  if (set_timeout) {
    set_timeout = 0;
    end = glutGet(GLUT_ELAPSED_TIME);
    elapsed = end - begin;
    if(elapsed > interval)
      glutTimerFunc(0, animate, 1);
    else
      glutTimerFunc(interval - elapsed, animate, 1);
  }
}

void
visibility(int state)
{
  if (state == GLUT_VISIBLE) {
    visible = 1;
    animate(0);
  } else {
    visible = 0;
  }
}

void
minify_select(int value)
{
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
  gluBuild2DMipmaps(GL_TEXTURE_2D, mjk_depth, mjk_width, mjk_height,
    GL_RGB, GL_UNSIGNED_BYTE, mjk_image);
  glutPostRedisplay();
}

void
rate_select(int value)
{
  interval = value;
}

void
menu_select(int value)
{
  switch (value) {
  case 1:
    spinning = !spinning;
    if (spinning)
      animate(0);
    break;
  case 2:
    scaling = !scaling;
    if (scaling)
      animate(0);
    break;
  case 3:
    mode++;
    if (mode > DRUM)
      mode = CUBE;
    switch (mode) {
    case CUBE:
      glEnable(GL_CULL_FACE);
      glDisable(GL_DEPTH_TEST);
      break;
    case SQUARES:
      glDisable(GL_CULL_FACE);
      glDisable(GL_DEPTH_TEST);
      break;
    case DRUM:
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      break;
    }
    glutPostRedisplay();
    break;
  case 666:
    exit(0);
  }
}

int
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("mjkwarp");
  glutDisplayFunc(redraw);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
    /* aspect ratio */ 1.0, /* Z near */ 1.0, /* Z far */ 70.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,30) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */

  /* Image data packed tightly. */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  gluBuild2DMipmaps(GL_TEXTURE_2D, mjk_depth, mjk_width, mjk_height,
    GL_RGB, GL_UNSIGNED_BYTE, mjk_image);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glEnable(GL_TEXTURE_2D);
  glutVisibilityFunc(visibility);
  minify_menu = glutCreateMenu(minify_select);
  glutAddMenuEntry("Nearest", GL_NEAREST);
  glutAddMenuEntry("Linear", GL_LINEAR);
  glutAddMenuEntry("Nearest mipmap nearest", GL_NEAREST_MIPMAP_NEAREST);
  glutAddMenuEntry("Linear mipmap nearest", GL_LINEAR_MIPMAP_NEAREST);
  glutAddMenuEntry("Nearest mipmap linear", GL_NEAREST_MIPMAP_LINEAR);
  glutAddMenuEntry("Linear mipmap linear", GL_LINEAR_MIPMAP_LINEAR);
  rate_menu = glutCreateMenu(rate_select);
  glutAddMenuEntry(" 2/sec", 500);
  glutAddMenuEntry(" 6/sec", 166);
  glutAddMenuEntry("10/sec", 100);
  glutAddMenuEntry("20/sec", 50);
  glutAddMenuEntry("30/sec", 33);
  glutCreateMenu(menu_select);
  glutAddMenuEntry("Toggle spinning", 1);
  glutAddMenuEntry("Toggle scaling", 2);
  glutAddMenuEntry("Switch mode", 3);
  glutAddSubMenu("Minimum frame rate", rate_menu);
  glutAddSubMenu("Minify modes", minify_menu);
  glutAddMenuEntry("Quit", 666);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  menu_select(3);
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
