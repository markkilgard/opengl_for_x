
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* Molecule Viewer file reader */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

#include "molview.h"

/* Helper routines for fileLoadMolecule. */
static char *addAtomType(Molecule * mol, char *name, float radius,
  float red, float green, float blue);
static char *addAtomInstance(Molecule * mol, char *name,
  float x, float y, float z);
static void freeMolecule(Molecule * mol);
static char *calculateMoleculeExtents(Molecule * mol);

#define MAX_LEN 256

char *
fileLoadMolecule(char *filename, Molecule ** return_mol)
{
  FILE *file;
  char line[MAX_LEN], name[MAX_LEN];
  float radius, red, green, blue, x, y, z;
  char *error, fchar;
  Molecule *mol;
  int cnt;

  file = fopen(filename, "r");
  if (!file)
    return "Could not open file.";
  mol = (Molecule *) malloc(sizeof(Molecule));
  if (!mol) {
    fclose(file);
    return "Memory allocation failure";
  }
  mol->natoms = 0;
  mol->types = NULL;
  mol->instances = NULL;
  mol->selbuf = NULL;
  for (;;) {
    if (!fgets(line, MAX_LEN, file))
      break;
    fchar = line[0];
    if (fchar == '!' || fchar == ';' || fchar == '\n')
      continue;         /* Comment or blank line. */
    cnt = sscanf(line, "%[A-Za-z] = %f ( %f, %f, %f )", name, &radius, &red, &green, &blue);
    if (cnt == 5) {
      error = addAtomType(mol, name, radius, red, green, blue);
      if (error)
        goto ReadError;
      continue;
    }
    cnt = sscanf(line, "%[A-Za-z] : %f %f %f", name, &x, &y, &z);
    if (cnt == 4) {
      error = addAtomInstance(mol, name, x, y, z);
      if (error)
        goto ReadError;
      continue;
    }
    error = "File format incorrect.";
    goto ReadError;
  }
  error = calculateMoleculeExtents(mol);
  if (error)
    goto ReadError;

  /* Select buffer should have room for 4 entries
     (#,z1,z2,name) for each atom. */
  mol->selbuf = (GLuint *) malloc(4 * mol->natoms * sizeof(GLuint));
  if (mol->selbuf == NULL) {
    error = "Memory allocation failure";
    goto ReadError;
  }
  fclose(file);
  if (*return_mol)
    freeMolecule(*return_mol);  /* Free old molecule if
                                   necessary. */
  *return_mol = mol;
  return NULL;

ReadError:
  freeMolecule(mol);
  fclose(file);
  return error;
}

static char *
addAtomType(Molecule * mol, char *name, float radius,
  float red, float green, float blue)
{
  AtomType *type;

  type = mol->types;
  while (type) {
    if (!strcmp(type->name, name))
      return "Duplicate atom type encountered.";
    type = type->next;
  }
  type = (AtomType *) malloc(sizeof(AtomType));
  if (!type)
    return "Memory allocation failure.";
  type->name = strdup(name);
  if (!type->name) {
    free(type);
    return "Memory allocation failure.";
  }
  type->radius = radius;
  type->color[0] = red * 255;
  type->color[1] = green * 255;
  type->color[2] = blue * 255;
  type->next = mol->types;
  mol->types = type;
  return NULL;
}

static char *
addAtomInstance(Molecule * mol, char *name, float x, float y, float z)
{
  AtomInstance *inst;
  AtomType *type;

  type = mol->types;
  while (type) {
    if (!strcmp(type->name, name)) {
      inst = (AtomInstance *) malloc(sizeof(AtomInstance));
      if (!inst)
        return "Memory allocation failure.";
      inst->atom = type;
      inst->x = x;
      inst->y = y;
      inst->z = z;
      inst->next = mol->instances;
      mol->instances = inst;
      mol->natoms++;
      return NULL;
    }
    type = type->next;
  }
  return "Name of atom instance not already encountered.";
}

static void
freeMolecule(Molecule * mol)
{
  AtomType *type, *next_type;
  AtomInstance *inst, *next_inst;

  type = mol->types;
  while (type) {
    free(type->name);
    next_type = type->next;
    free(type);
    type = next_type;
  }
  inst = mol->instances;
  while (inst) {
    next_inst = inst->next;
    free(inst);
    inst = next_inst;
  }
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

static char *
calculateMoleculeExtents(Molecule * mol)
{
  AtomInstance *inst;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  float radius;

  inst = mol->instances;
  if (!inst)
    return "Must have at least one atom.";
  radius = inst->atom->radius;
  xmin = inst->x - radius;
  xmax = inst->x + radius;
  ymin = inst->y - radius;
  ymax = inst->y + radius;
  zmin = inst->z - radius;
  zmax = inst->z + radius;
  inst = inst->next;
  while (inst) {
    radius = inst->atom->radius;
    xmin = MIN(inst->x - radius, xmin);
    xmax = MAX(inst->x + radius, xmax);
    ymin = MIN(inst->y - radius, ymin);
    ymax = MAX(inst->y + radius, ymax);
    zmin = MIN(inst->z - radius, zmin);
    zmax = MAX(inst->z + radius, zmax);
    inst = inst->next;
  }
  mol->xmin = xmin;
  mol->ymin = ymin;
  mol->zmin = zmin;
  mol->xsize = xmax - xmin;
  mol->ysize = ymax - ymin;
  mol->zsize = zmax - zmin;
  mol->maxdim = MAX(MAX(mol->xsize, mol->ysize), mol->zsize);
  return NULL;
}
