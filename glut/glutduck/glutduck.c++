// Based on an example from the Inventor Mentor chapter 13, example 5.

#include <stdio.h>
#include <unistd.h>
#include <GL/glut.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>

int spinning = 0, moving  = 0, begin;
SoSeparator *root;
SoRotationXYZ *duckRotXYZ;
float angle = 0.0;
SbViewportRegion myViewport;

void
reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  myViewport.setWindowSize(w,h);
}

void
renderScene(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  SoGLRenderAction myRenderAction(myViewport);
  myRenderAction.apply(root);
}

void
redraw(void)
{
  renderScene();
  glutSwapBuffers();
}

int
duckScene(void)
{
  root = new SoSeparator;
  root->ref();

  // Add a camera and light
  SoPerspectiveCamera *myCamera = new SoPerspectiveCamera;
  myCamera->position.setValue(0., -4., 8.0);
  myCamera->heightAngle = M_PI/2.5;
  myCamera->nearDistance = 1.0;
  myCamera->farDistance = 15.0;
  root->addChild(myCamera);
  root->addChild(new SoDirectionalLight);

  // Rotate scene slightly to get better view
  SoRotationXYZ *globalRotXYZ = new SoRotationXYZ;
  globalRotXYZ->axis = SoRotationXYZ::X;
  globalRotXYZ->angle = M_PI/9;
  root->addChild(globalRotXYZ);

  // Pond group
  SoSeparator *pond = new SoSeparator;
  root->addChild(pond);
  SoMaterial *cylMaterial = new SoMaterial;
  cylMaterial->diffuseColor.setValue(0., 0.3, 0.8);
  pond->addChild(cylMaterial);
  SoTranslation *cylTranslation = new SoTranslation;
  cylTranslation->translation.setValue(0., -6.725, 0.);
  pond->addChild(cylTranslation);
  SoCylinder *myCylinder = new SoCylinder;
  myCylinder->radius.setValue(4.0);
  myCylinder->height.setValue(0.5);
  pond->addChild(myCylinder);

  // Duck group
  SoSeparator *duck = new SoSeparator;
  root->addChild(duck);

  // Read the duck object from a file and add to the group
  SoInput myInput;
  if (!myInput.openFile("duck.iv")) {
    if (!myInput.openFile("/usr/share/src/Inventor/examples/data/duck.iv"))
      return 1;
  }
  SoSeparator *duckObject = SoDB::readAll(&myInput);
  if (duckObject == NULL) return 1;

  // Set up the duck transformations
  duckRotXYZ = new SoRotationXYZ;
  duck->addChild(duckRotXYZ);
  duckRotXYZ->angle = angle;
  duckRotXYZ->axis = SoRotationXYZ::Y;  // rotate about Y axis
  SoTransform *initialTransform = new SoTransform;
  initialTransform->translation.setValue(0., 0., 3.);
  initialTransform->scaleFactor.setValue(6., 6., 6.);
  duck->addChild(initialTransform);
  duck->addChild(duckObject);
  return 0;
}

void
updateModels(void)
{
  duckRotXYZ->angle = angle;
  glutPostRedisplay();
}

void
animate(void)
{
  angle += 0.1;
  updateModels();
}

void
setAnimation(int enable)
{
  if(enable) {
    spinning = 1;
    glutIdleFunc(animate);
  } else {
    spinning = 0;
    glutIdleFunc(NULL);
    glutPostRedisplay();
  }
}

void
keyboard(unsigned char ch, int x, int y)
{
  if(ch == ' ') {
    setAnimation(0);
    animate();
  }
}

void
menuSelect(int item)
{
  switch(item) {
  case 1:
    setAnimation(0);
    animate();
    break;
  case 2:
    if(!spinning) setAnimation(1);
      else setAnimation(0);
    break;
  }
}

void
vis(int visible)
{
  if (visible == GLUT_VISIBLE) {
    if (spinning) glutIdleFunc(animate);
  } else {
    if (spinning) glutIdleFunc(NULL);
  }
}

void
mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    setAnimation(0);
    moving = 1;
    begin = x;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    moving = 0;
    glutPostRedisplay();
  }
}

void
motion(int x, int y)
{
  if (moving) {
    angle = angle + .01 * (x - begin);
    begin = x;
    updateModels();
  }
}

int
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  SoDB::init();
  if(duckScene()) {
    fprintf(stderr, "couldn't read IV file\n");
    exit(1);
  }
  glutCreateWindow("GLUT Inventor Duck Pond");
  glutReshapeFunc(reshape);
  glutDisplayFunc(redraw);
  glutCreateMenu(menuSelect);
  glutAddMenuEntry("Step", 1);
  glutAddMenuEntry("Toggle animation", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutVisibilityFunc(vis);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.132, 0.542, 0.132, 1.0);
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
