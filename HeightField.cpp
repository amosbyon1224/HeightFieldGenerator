/*
  Height Field Generator
  Generates a height field off of an input image
  Part of an assignment for CSCI 420
  Amos Byon
  October 11, 2015
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string>

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

/* enum to track which modifier for the mouse */
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

/* enum to track which rendering mode to display */
typedef enum { POINTS, WIREFRAME, SHADED, WIREONSHADED } RENDERSTATE;

/* enum to track render resolution */
typedef enum {
  REND_LOW = 8,
  REND_MED = 4,
  REND_HIGH = 2,
  REND_ULTRA = 1} RENDERQUALITY;

/* set default control state */
CONTROLSTATE g_ControlState = ROTATE;

/* set default render state */
RENDERSTATE g_RenderState = SHADED;

/* set default render quality */
RENDERQUALITY g_RenderQual = REND_ULTRA;

/* since raw height values extracted from the image are
too high, we need to divide by some value in order
to bring the height value in 3D world to a more
manageable value */
float heightDampener = 2000.0;

/* state of the world */
float g_vLandRotate[3] = {-30.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* Previous amount of ticks for animation */
int prev_ticks = 0;

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

/* jpg counter */
int jpgCounter = 0;

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

/* renders the current heightfield as points */
void drawPointCloud(){
  //width and height of the picture
  int width = g_pHeightData->nx;
  int height = g_pHeightData->ny;

  //defulat starting position to render
  float xCoor = -0.5;
  float yCoor = 0.5;

  //amount to traverse at each point
  float step = (g_RenderQual)/((float)width);

  float heightValRAW = 0;
  float heightVal = 0;

  //iterate through image and extract height values
  for(int i = 0; i < height; i += g_RenderQual){
    for(int j = 0; j < width; j += g_RenderQual){
      heightValRAW = PIC_PIXEL(g_pHeightData, j, i, 0);
      heightVal = heightValRAW/heightDampener;
      glColor3f(1.0, 1.0, 1.0);
      glVertex3f(xCoor, heightVal, yCoor);
      xCoor += step;
    }
    xCoor = -0.5;
    yCoor -= step;
  }
}

/* renders the current heightfield as shaded model */
void drawShaded(){
  //width and height of the picture
  int width = g_pHeightData->nx;
  int height = g_pHeightData->ny;

  //default starting position to render
  float xCoor = -0.5;
  float yCoor = 0.5;

  //amount to traverse at each point
  float step = (g_RenderQual)/((float)width);

  float heightValRAW = 0;
  float heightVal = 0;
  
  //iterate through image and extract height vaules
  for(int i = 0; i < height-1; i += g_RenderQual){
    glBegin(GL_TRIANGLE_STRIP);
    for(int j = 0; j < width; j += g_RenderQual){
      heightValRAW = PIC_PIXEL(g_pHeightData, j, i, 0);
      heightVal = heightValRAW/heightDampener;
      glColor3f(heightValRAW/255+0.2, heightValRAW/255+0.2, heightValRAW/255+0.1);
      glVertex3f(xCoor, heightVal, yCoor);

      heightValRAW = PIC_PIXEL(g_pHeightData, j, i + g_RenderQual, 0);
      heightVal = heightValRAW/heightDampener;
      glColor3f(heightValRAW/255+0.2, heightValRAW/255+0.2, heightValRAW/255+0.1);
      glVertex3f(xCoor, heightVal, yCoor-step);
      xCoor += step;
    }
    glEnd();
    xCoor = -0.5;
    yCoor -= step;
  }
}

/* renders the current heightfield as wireframe model */
void drawWireframe(){
  //width and height of the picture
  int width = g_pHeightData->nx;
  int height = g_pHeightData->ny;

  //default starting position to render
  float xCoor = -0.5;
  float yCoor = 0.5;

  //amount to traverse at each point
  float step = (g_RenderQual)/((float)width);

  float heightValRAW = 0;
  float heightVal = 0;
  
  //iterate through image and extract height values
  for(int i = 0; i < height-1; i += g_RenderQual){
    glBegin(GL_LINE_STRIP);
    for(int j = 0; j < width; j += g_RenderQual){
      heightValRAW = PIC_PIXEL(g_pHeightData, j, i, 0);
      heightVal = heightValRAW/heightDampener;
      glColor3f(1.0, 1.0, 1.0);
      glVertex3f(xCoor, heightVal, yCoor);

      heightValRAW = PIC_PIXEL(g_pHeightData, j, i + g_RenderQual, 0);
      heightVal = heightValRAW/heightDampener;
      glColor3f(1.0, 1.0, 1.0);
      glVertex3f(xCoor, heightVal, yCoor-step);
      xCoor += step;
    }
    glEnd();
    xCoor = -0.5;
    yCoor -= step;
  }
}

void myinit()
{
  /* setup gl view here */
  glEnable(GL_DEPTH_TEST);
}

void display(void)
{
  //clear the previous screen data
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(0.0, 0.0, -1.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);

  //apply mouse rotations
  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);

  //apply mouse translations
  glTranslatef(g_vLandTranslate[0],
               g_vLandTranslate[1],
               g_vLandTranslate[2]);

  //apply mouse scaling
  glScalef(g_vLandScale[0],
           g_vLandScale[1],
           g_vLandScale[2]);

  //draw stuff depending on user render mode
  switch(g_RenderState){
    case POINTS:
      glBegin(GL_POINTS);
        drawPointCloud();
      glEnd();
      break;
    case WIREFRAME:
      drawWireframe();
      break;
    case SHADED:
      drawShaded();
      break;
    case WIREONSHADED:
      drawShaded();
      drawWireframe();
      break;
  };

  //save animation to jpgs
  /*
  std::string fileTemp = std::to_string(jpgCounter) + ".jpg";
  char * filename = new char[fileTemp.length() + 1];
  std::strcpy(filename, fileTemp.c_str());
  saveScreenshot(filename);
  jpgCounter++;

  //delete[] filename;
  */

  //swap buffers to display rendered screen
  glutSwapBuffers();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void reshape(int width, int height){
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(65.0, (float)width / height, 0.01, 1000.0);

  glMatrixMode(GL_MODELVIEW);
}

void doIdle()
{
  //get the amount of time that has elapsed
  int t = glutGet(GLUT_ELAPSED_TIME);

  //get amount of time between frames
  int deltaTicks = t - prev_ticks;

  //keep track of updated elapsed time
  prev_ticks = t;

  //rotate model based on elapsed time since last frame
  g_vLandRotate[1] += (0.01 + deltaTicks*0.01);
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/* assigns keyboard buttons to certain functions */
void keyboard(unsigned char c, int x, int y){
  if(c == '1'){
    g_RenderState = SHADED;
  }
  if(c == '2'){
    g_RenderState = WIREFRAME;
  }
  if(c == '3'){
    g_RenderState = POINTS;
  }
  if(c == '4'){
    g_RenderState = WIREONSHADED;
  }
  if(c == '7'){
    g_RenderQual = REND_LOW;
  }
  if(c == '8'){
    g_RenderQual = REND_MED;
  }
  if(c == '9'){
    g_RenderQual = REND_HIGH;
  }
  if(c == '0'){
    g_RenderQual = REND_ULTRA;
  }
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }
  glutInit(&argc,argv);
  
  /*
    create a window here..should be double buffered and use depth testing
  
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  glutInitWindowSize(640, 480);
  glutCreateWindow("Height Map");

  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);

  glutReshapeFunc(reshape);
  
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* replace with any animate code */
  glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);
  /* callback for keyboard changes */
  glutKeyboardFunc(keyboard);

  /* do initialization */
  myinit();

  glutMainLoop();
  return(0);
}