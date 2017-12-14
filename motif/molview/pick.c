
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* molview: pick.c - OpenGL selection code for picking atoms. */

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "molview.h"

static AtomInstance *processHits(Molecule *mol, GLint hits, GLuint buffer[]);

AtomInstance *
pickScene(Molecule * mol, int x, int y)
{
  GLint viewport[4];
  float aspect;
  int hits;

  glGetIntegerv(GL_VIEWPORT, viewport);

  glSelectBuffer(4*mol->natoms, mol->selbuf);
  (void) glRenderMode(GL_SELECT);

  /* XXX Bug workaround! */
  glDisable(GL_CULL_FACE);

  glInitNames();
  glPushName(-1);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(x, viewport[3] - y, 1.0, 1.0, viewport);
  aspect = ((float) viewport[2])/viewport[3];
  gluPerspective(60.0, aspect, 1.0, 7.0);
  glMatrixMode(GL_MODELVIEW);
  renderMolecule(mol);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  hits = glRenderMode(GL_RENDER);

  /* XXX Bug workaround! */
  glEnable(GL_CULL_FACE);

  return processHits(mol, hits, mol->selbuf);
}

static AtomInstance *
atomInfo(Molecule *mol, int picked)
{
  AtomInstance *atom;
  int name;

  name = 0;
  atom = mol->instances;
  while (picked != name) {
    atom = atom->next;
    name++;
  }
  return atom;
}

AtomInstance *
processHits(Molecule *mol, GLint hits, GLuint buffer[])
{
  AtomInstance *atom;
  unsigned int i;
  GLint names;
  GLuint *ptr;
  GLuint minz, z, match;

  if(hits < 0 ) {
    fprintf(stderr, "WARNING: select buffer overflow!\n");
    return NULL;
  }
#ifdef DEBUG
  printf("hits = %d\n", hits);
  ptr = (GLuint *) buffer;
  for (i = 0; i < hits; i++) {  /* for each hit  */
    int j;

    names = *ptr;
    printf(" number of names for hit = %d\n", *ptr);
    ptr++;
    printf("  z1 is %g;", (float) *ptr/0xffffffff);
    ptr++;
    printf(" z2 is %g\n", (float) *ptr/0xffffffff);
    ptr++;
    printf("   the name is ");
    for (j = 0; j < names; j++) {  /* For each name. */
      printf("%d ", *ptr);
      ptr++;
    }
    printf("\n");
  }
#endif
  if(hits == 0)
    return NULL;
  minz = 0xffffffff;
  ptr = (GLuint *) buffer;
  for (i = 0; i < hits; i++) {  /* for each hit  */
    names = *ptr;
    ptr++;
    z = *ptr;
    ptr++;
    ptr++;
    if(z <= minz) {
      minz = z;
      match = *ptr;
    }
    ptr += names;
  }
  atom = atomInfo(mol, match);

  return atom;
}
