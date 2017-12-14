
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>

XDeviceInfoPtr
list_input_devices(Display * display, int *ndevices)
{
  int i, j, k;
  XDeviceInfoPtr list, slist;
  XAnyClassPtr any;
  XKeyInfoPtr key;
  XButtonInfoPtr b;
  XValuatorInfoPtr v;
  XAxisInfoPtr a;

  list = (XDeviceInfoPtr) XListInputDevices(display, ndevices);
  slist = list;
  printf("The number of available input devices is %d\n", *ndevices);
  for (i = 0; i < *ndevices; i++, list++) {
    printf("\nDevice id is %d\n", list->id);
    printf("Device type is %s\n", XGetAtomName(display, list->type));
    printf("Device name is %s\n", list->name);
    printf("Num_classes is %d\n", list->num_classes);
    if (list->num_classes > 0) {
      any = (XAnyClassPtr) (list->inputclassinfo);
      for (j = 0; j < list->num_classes; j++) {
        printf("\tInput class is %d\n", any->class);
        printf("\tLength is %d\n", any->length);
        switch (any->class) {
        case KeyClass:
          key = (XKeyInfoPtr) any;
          printf("\tNum_keys is %d\n", key->num_keys);
          printf("\tMin_keycode is %d\n", key->min_keycode);
          printf("\tMax_keycode is %d\n", key->max_keycode);
          break;
        case ButtonClass:
          b = (XButtonInfoPtr) any;
          printf("\tNum_buttons is %d\n", b->num_buttons);
          break;
        case ValuatorClass:
          v = (XValuatorInfoPtr) any;
          a = (XAxisInfoPtr) ((char *) v +
            sizeof(XValuatorInfo));
          printf("\tMode is %d\n", v->mode);
          printf("\tNum_axes is %d\n\n", v->num_axes);
          for (k = 0; k < v->num_axes; k++, a++) {
            printf("\t\tMin_value is %d\n", a->min_value);
            printf("\t\tMax_value is %d\n", a->max_value);
            printf("\t\tResolution is %d\n\n", a->resolution);
          }
          break;
        default:
          printf("unknown class\n");
        }
        any = (XAnyClassPtr) ((char *) any + any->length);
      }
    }
  }
  return slist;
}

main(int argc, char **argv)
{
  int ndevices;
  Display *display;
  XDeviceInfoPtr slist;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    printf("Could not open display.\n");
    exit(1);
  }
  slist = list_input_devices(display, &ndevices);
  if (slist) {
    XFreeDeviceList(slist);
  }
}
