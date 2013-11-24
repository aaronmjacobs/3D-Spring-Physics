/*
 * Physics Simulation by Aaron Jacobs
 * CSC 471 Final Project, Spring 2013
 */

#include <stdio.h>
#include <stdlib.h>

#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "glm/gtx/transform.hpp"

#include "GLBridge.h"

#include "Model.h"
#include "Light.h"
#include "Scene.h"
#include "Camera.h"
#include "PhysModel.h"
#include "SpringForce.h"
#include "TwoWaySpringForce.h"
#include "GravitationalForce.h"

using namespace std;

#define FRAME_DELAY 16

#define WINDOW_TITLE "Physics!"

#define CONTROL_DISABLED 0
#define ADD_MODEL 1
#define REMOVE_MODEL 2
#define ADD_LIGHT 3
#define ADD_SPRING 4
#define ADD_TWO_WAY_SPRING 5
#define REMOVE_SPRING 6
#define TOGGLE_GRAVITY 7
#define GRAB 8

static int lastMouseX, lastMouseY;
static bool walkingForward, walkingBackward, walkingLeft, walkingRight;
static bool scenePause = false;
static bool stepMode = false;
static int controlMode = CONTROL_DISABLED;
static PhysModel* lastHit, *grabbed;
static glm::vec3 nearPos, lastNearPos, lastIntoScreen;

static Scene scene;
static Camera* camera;

static int ShadeProg;
static float g_width, g_height;

static glm::mat4 projection;

static GLint h_uViewMatrix;
static GLint h_uProjMatrix;
static GLint h_uCameraPos;

static PhysModel* bunnyModel, *secondBunnyModel;
static Model* worldFloor;
static SpringForce* softMouseForce;

float randFloat(float low, float high)
{
  return low + (float)rand() / ((float)RAND_MAX / (high - low));
}

glm::vec3 randVec3(float low, float high)
{
  return glm::vec3(randFloat(low, high), randFloat(low, high), randFloat(low, high));
}

void InitGeom()
{
  Material bunnyMaterial;
  glm::vec3 baseBunnyColor(0.65f, 0.0f, 1.0f);
  bunnyMaterial.ambient = baseBunnyColor * 0.01f;
  bunnyMaterial.diffuse = baseBunnyColor * 0.4f;
  bunnyMaterial.specular = glm::vec3(0.4f, 0.4f, 0.4f);
  bunnyMaterial.emission = baseBunnyColor * 0.0f;
  bunnyMaterial.shininess = 200.0f;
  Mesh* bunnyMesh = Mesh::load("Models/bunny.orig.m", true);
  bunnyModel = new PhysModel(bunnyMesh, bunnyMaterial, 3.0f, glm::vec3(0.0f, 0.0f, -5.0f));
  
  GravitationalForce::create(bunnyModel);

  Material floorMaterial;
  glm::vec3 baseFloorColor(1.0f, 1.0f, 1.0f);
  floorMaterial.ambient = baseFloorColor * 0.3f;
  floorMaterial.diffuse = baseFloorColor * 0.6f;
  floorMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
  floorMaterial.emission = baseFloorColor * 0.0f;
  floorMaterial.shininess = 10.0f;
  Mesh* floorMesh = Mesh::load("SimpleModels/plane.m", false);
  worldFloor = new Model(floorMesh, floorMaterial);
  worldFloor->translate(glm::vec3(0.0f, -0.5f, 0.0f));
  worldFloor->scale(3.0f);
  
  Light* sceneLight = new Light(glm::vec3(0.0f, 0.0f, 0.0f), randVec3(0.0f, 0.5f), 0.1f, 0.005f, 0.001f);
  sceneLight->drawModel();
  sceneLight->attachTo(bunnyModel);

  scene.add(worldFloor);
  scene.add(bunnyModel);
  scene.add(sceneLight);
  
  scene.setCollisionSurface(worldFloor);

  camera = new Camera(h_uViewMatrix, h_uCameraPos);
}

void Initialize()
{
  srand(time(NULL));

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  // Depth Buffer Setup
  glClearDepth(1.0f);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  
  projection = glm::perspective(90.0f, g_width/g_height, 0.1f, 100.f);
}

static float alpha = 1.0f;

/* Main display function */
void Draw(void)
{
  // Clear
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start our shader
  glUseProgram(ShadeProg);

  // Projection
  glUniformMatrix4fv(h_uProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));

  // View
  camera->updateView();
  
  // Lights
  GLint numLights = GLBridge::getNumLightsHandle();
  glUniform1i(numLights, scene.getNumLights());
  GLBridge::onDraw(); // Resets light handles

  // Models
  scene.draw(alpha);

  // Disable the shader
  glUseProgram(0);

  glutSwapBuffers();
}

/* Reshape */
void ReshapeGL(int width, int height)
{
  g_width = (float)width;
  g_height = (float)height;
  glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));

  projection = glm::perspective(90.0f, g_width/g_height, 0.1f, 100.f);
}

#define WALK_SPEED 0.01f
float extraSpeed = 0.0f;

void keyboard(unsigned char key, int x, int y )
{
  char title[100];
  strncpy(title, WINDOW_TITLE, 100);
  switch(key)
  {
  case 'w':
    walkingForward = true;
    break;
  case 's':
    walkingBackward = true;
    break;
  case 'a':
    walkingLeft = true;
    break;
  case 'd':
    walkingRight = true;
    break;
  case 'p':
    scenePause = !scenePause;
    break;
  case 'l':
    stepMode = !stepMode;
    break;
  case 'e':
    if(extraSpeed > 0.0f)
    {
      extraSpeed = 0.0f;
    }
    else
    {
      extraSpeed = 0.1f;
    }
    break;
  case 'r':
    //scene.removeNextForce();
    break;
  case '1':
    controlMode = ADD_MODEL;
    glutSetWindowTitle(strcat(title, " - Add Model"));
    break;
  case '2':
    controlMode = REMOVE_MODEL;
    glutSetWindowTitle(strcat(title, " - Remove Model"));
    break;
  case '3':
    controlMode = ADD_LIGHT;
    glutSetWindowTitle(strcat(title, " - Add Light"));
    break;
  case '4':
    controlMode = ADD_SPRING;
    glutSetWindowTitle(strcat(title, " - Add Spring"));
    break;
  case '5':
    controlMode = ADD_TWO_WAY_SPRING;
    lastHit = NULL;
    glutSetWindowTitle(strcat(title, " - Add Two Way Spring"));
    break;
  case '6':
    controlMode = REMOVE_SPRING;
    glutSetWindowTitle(strcat(title, " - Remove Spring"));
    break;
  case '7':
    controlMode = TOGGLE_GRAVITY;
    glutSetWindowTitle(strcat(title, " - Toggle Gravity"));
    break;
  case '8':
    controlMode = GRAB;
    grabbed = NULL;
    glutSetWindowTitle(strcat(title, " - Grab"));
    break;
  case '0':
    controlMode = CONTROL_DISABLED;
    glutSetWindowTitle(title);
    break;
  case 'q': case 'Q' :
    exit(EXIT_SUCCESS);
    break;
  }
  
  if(controlMode != GRAB && softMouseForce)
  {
    delete softMouseForce;
    softMouseForce = NULL;
  }
}

void keyUp(unsigned char key, int x, int y)
{
  switch(key)
  {
  case 'w':
    walkingForward = false;
    break;
  case 's':
    walkingBackward = false;
    break;
  case 'a':
    walkingLeft = false;
    break;
  case 'd':
    walkingRight = false;
    break;
  case 'q': case 'Q' :
    exit(EXIT_SUCCESS);
    break;
  }
}

void genNearAndFar(int x, int y, glm::vec3* nearCoords, glm::vec3* farCoords)
{
  glm::vec3 windowCoords(x, g_height - y, 0);
  glm::vec4 viewport(0, 0, g_width, g_height);
  glm::mat4 view = camera->getViewMatrix();

  *nearCoords = glm::unProject(windowCoords, view, projection, viewport);
  windowCoords.z = 1;
  *farCoords = glm::unProject(windowCoords, view, projection, viewport);
}

static int lastButton;
void mouseClick(int button, int state, int x, int y)
{
  lastButton = button;
  if((button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON) && state == GLUT_DOWN)
  {
    glm::vec3 nearCoords, farCoords;
    genNearAndFar(x, y, &nearCoords, &farCoords);
    
    if(controlMode == ADD_MODEL)
    {
      Material bunnyMaterial;
      glm::vec3 baseBunnyColor(0.65f, 0.0f, 1.0f);
      bunnyMaterial.ambient = baseBunnyColor * 0.2f;
      bunnyMaterial.diffuse = baseBunnyColor * 0.4f;
      bunnyMaterial.specular = glm::vec3(0.4f, 0.4f, 0.4f);
      bunnyMaterial.emission = baseBunnyColor * 0.0f;
      bunnyMaterial.shininess = 200.0f;
      Mesh* bunnyMesh = Mesh::load("Models/bunny.orig.m", true);
      
      glm::vec3 addPos = nearCoords + (glm::normalize(farCoords - nearCoords) * 3.0f);
      
      PhysModel* toAdd = new PhysModel(bunnyMesh, bunnyMaterial, 3.0f, addPos);
      GravitationalForce::create(toAdd);
      
      scene.add(toAdd);
    }
    else
    {
      PhysModel* hit = scene.select(nearCoords, farCoords);
      if(hit)
      {
        switch(controlMode)
        {
        case REMOVE_MODEL:
          scene.remove(hit);
          delete hit;
          break;
        case ADD_LIGHT:
          if(scene.getNumLights() < MAX_LIGHTS)
          {
            Light* sceneLight = new Light(glm::vec3(0.0f, 0.0f, 0.0f), randVec3(0.0f, 0.5f), 0.1f, 0.005f, 0.001f);
            sceneLight->drawModel();
            sceneLight->attachTo(hit);
            scene.add(sceneLight);
          }
          break;
        case ADD_SPRING:
          SpringForce::create(hit, nearCoords, 4.0f, 0.5f, randVec3(-hit->getExtrema(), hit->getExtrema()));
          break;
        case ADD_TWO_WAY_SPRING:
          if(lastHit && hit != lastHit)
          {
            TwoWaySpringForce::create(hit, lastHit, 4.0f, 0.5f, randVec3(-hit->getExtrema(), hit->getExtrema()), randVec3(-lastHit->getExtrema(), lastHit->getExtrema()));
            lastHit = NULL;
          }
          else
          {
            lastHit = hit;
          }
          break;
        case REMOVE_SPRING:
          hit->deleteSpringForce();
          break;
        case TOGGLE_GRAVITY:
          hit->toggleGravity();
          break;
        case GRAB:
          nearPos = lastNearPos = nearCoords;
          lastIntoScreen = glm::normalize(farCoords - nearCoords);
          if(!softMouseForce)
          {
            grabbed = hit;
            softMouseForce = SpringForce::create(hit, hit->getPosition(), 6.0f, 0.5f, randVec3(-hit->getExtrema(), hit->getExtrema()));
          }
          break;
        default:
          break;
        }
      }
    }
  }
  
  if(state == GLUT_UP && controlMode == GRAB)
  {
    delete softMouseForce;
    softMouseForce = NULL;
  }
  
  lastMouseX = x;
  lastMouseY = y;
}

#define FLOAT_ADJUST 0.0000000000001f
#define ROTATE_SCALE 0.005f

void mouseMotion(int x, int y)
{
  if(lastButton == GLUT_LEFT_BUTTON)
  {
    glm::vec3 nearCoords, farCoords;
    genNearAndFar(x, y, &nearCoords, &farCoords);
    glm::vec3 intoScreen = glm::normalize(farCoords - nearCoords);
    
    if(controlMode == GRAB && softMouseForce && grabbed)
    {
      glm::vec3 toGrabbed = grabbed->getPosition() - nearPos;
      lastIntoScreen = glm::normalize(lastIntoScreen);
      intoScreen *= glm::length(toGrabbed);
      lastIntoScreen *= glm::length(toGrabbed);;
      
      softMouseForce->translate(intoScreen - lastIntoScreen);
      lastIntoScreen = intoScreen;
      
      lastNearPos = nearPos;
      nearPos = nearCoords;
    }
  }
  else if(lastButton == GLUT_RIGHT_BUTTON)
  {
    float dTheta = (x - lastMouseX) * ROTATE_SCALE;
    float dPhi = -(y - lastMouseY) * ROTATE_SCALE;
    camera->rotate(dPhi, dTheta);
  
    lastMouseX = x;
    lastMouseY = y;
  }
}

static int currentTime;
static double accumulator = 0.0;

static double t = 0.0;
const static double dt = 1.0 / 60.0;

void loop(int value)
{
  int now = glutGet(GLUT_ELAPSED_TIME);
  int frameTime = now - currentTime;
  if(frameTime > 250)
  {
    frameTime = 250; // Cap the frame time to avoid spiraling
  }
  currentTime = now;
  
  if(!scenePause)
  {
    accumulator += (frameTime / 1000.0);
    
    while(accumulator >= dt)
    {
      scene.step(t, dt);
      t += dt;
      accumulator -= dt;
    }
  }
  
  // Render using an interpolated state in order to prevent any jittering caused
  // by the physics rate and frame rate not being in sync (note this is purely a
  // visual thing - the actual physics steps that are kept throughout the
  // simulation do not use the interpolated values).
  alpha = accumulator / dt;

  if(walkingForward)
  {
    camera->fly((WALK_SPEED + extraSpeed) * frameTime);
  }
  if(walkingBackward)
  {
    camera->fly(-(WALK_SPEED + extraSpeed) * frameTime);
  }
  if(walkingLeft)
  {
    camera->strafe(-(WALK_SPEED + extraSpeed) * frameTime);
  }
  if(walkingRight)
  {
    camera->strafe((WALK_SPEED + extraSpeed) * frameTime);
  }
  
  if(stepMode)
  {
    scenePause = true;
  }

  glutPostRedisplay(); // Draw

  glutTimerFunc(FRAME_DELAY, loop, 0);
}

int main(int argc, char *argv[])
{
  g_width = 1280;
  g_height = 720;

  glutInit(&argc, argv);
  glutInitWindowSize(g_width, g_height);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow(WINDOW_TITLE);
  glutReshapeFunc(ReshapeGL);
  glutDisplayFunc(Draw);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyUp);
  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseMotion);

#ifdef _WIN32
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error initializing glew! " << glewGetErrorString(err) << std::endl;
        return 1;
    }
#endif

  Initialize();

  // Test the openGL version
  getGLversion();
  // Install the shaders
  if(!GLBridge::InstallShader(textFileRead((char *)"mesh.vert"), textFileRead((char *)"mesh.frag")))
  {
    printf("Error installing shader!\n");
    return 1;
  }
  
  /******************************************/
  h_uViewMatrix = GLBridge::getUViewMatrix();
  h_uProjMatrix = GLBridge::getUProjMatrix();
  h_uCameraPos = GLBridge::getUCameraPos();
  ShadeProg = GLBridge::getShaderProgram();
  /******************************************/

  InitGeom();

  currentTime = glutGet(GLUT_ELAPSED_TIME);
  loop(FRAME_DELAY);
  glutMainLoop();
}
