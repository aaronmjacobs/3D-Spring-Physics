#include <sstream>
#include "GLBridge.h"

TransHandles GLBridge::transHandles;
MaterialHandles GLBridge::materialHandles;
GLint GLBridge::h_uViewMatrix;
GLint GLBridge::h_uProjMatrix;
GLint GLBridge::h_uCameraPos;
int GLBridge::shaderProgram;

GLint GLBridge::numLights;
LightHandles GLBridge::lightHandlesArray[MAX_LIGHTS];
int GLBridge::lightHandlesInUse;

int GLBridge::InstallShader(const GLchar *vShaderName, const GLchar *fShaderName)
{
  GLuint VS; //handles to shader object
  GLuint FS;
  GLint vCompiled, linked; //status of shader
  GLint vCompiledF, linkedF; //status of shader

  VS = glCreateShader(GL_VERTEX_SHADER);
  FS = glCreateShader(GL_FRAGMENT_SHADER);

  //load the source
  glShaderSource(VS, 1, &vShaderName, NULL);

  //compile shader and print log
  glCompileShader(VS);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
  printShaderInfoLog(VS);

  if (!vCompiled) {
      printf("Error compiling the shader %s", vShaderName);
      return 0;
  }

/***************************************/

  //load the source
  glShaderSource(FS, 1, &fShaderName, NULL);

  //compile shader and print log
  glCompileShader(FS);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetShaderiv(FS, GL_COMPILE_STATUS, &vCompiledF);
  printShaderInfoLog(FS);

  if (!vCompiled) {
      printf("Error compiling the shader %s", fShaderName);
      return 0;
  }

/**************************************/

  //create a program object and attach the compiled shader
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, VS);
  glAttachShader(shaderProgram, FS);

  glLinkProgram(shaderProgram);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
  printProgramInfoLog(shaderProgram);

  glUseProgram(shaderProgram);

  /* get handles to attribute data */
  transHandles.aPosition = safe_glGetAttribLocation(shaderProgram, "aPosition");
  transHandles.aNormal = safe_glGetAttribLocation(shaderProgram, "aNormal");
  transHandles.uModelMatrix = safe_glGetUniformLocation(shaderProgram, "uModelMatrix");
  transHandles.uNormalMatrix = safe_glGetUniformLocation(shaderProgram, "uNormalMatrix");

  materialHandles.uAmbientColor = safe_glGetUniformLocation(shaderProgram, "uMaterial.ambient");
  materialHandles.uDiffuseColor = safe_glGetUniformLocation(shaderProgram, "uMaterial.diffuse");
  materialHandles.uSpecularColor = safe_glGetUniformLocation(shaderProgram, "uMaterial.specular");
  materialHandles.uEmissionColor = safe_glGetUniformLocation(shaderProgram, "uMaterial.emission");
  materialHandles.uShininess = safe_glGetUniformLocation(shaderProgram, "uMaterial.shininess");

  h_uCameraPos = safe_glGetUniformLocation(shaderProgram, "uCameraPos");

  h_uProjMatrix = safe_glGetUniformLocation(shaderProgram, "uProjMatrix");
  h_uViewMatrix = safe_glGetUniformLocation(shaderProgram, "uViewMatrix");
  
  numLights = safe_glGetUniformLocation(shaderProgram, "uNumLights");
  
  for(int i = 0; i < MAX_LIGHTS; ++i)
  {
    std::stringstream ss;
    ss << "uLights[" << i << "]";
    std::string index = ss.str();

    lightHandlesArray[i].uLightPosition = safe_glGetUniformLocation(shaderProgram, (index + ".position").c_str());
    lightHandlesArray[i].uLightColor = safe_glGetUniformLocation(shaderProgram, (index + ".color").c_str());
    lightHandlesArray[i].uLightConstFalloff = safe_glGetUniformLocation(shaderProgram, (index + ".constFalloff").c_str());
    lightHandlesArray[i].uLightLinearFalloff = safe_glGetUniformLocation(shaderProgram, (index + ".linearFalloff").c_str());
    lightHandlesArray[i].uLightSquareFalloff = safe_glGetUniformLocation(shaderProgram, (index + ".squareFalloff").c_str());
  }

  printf("sucessfully installed shader %d\n", shaderProgram);
  return 1;
}

GLint GLBridge::getNumLightsHandle()
{
  return numLights;
}

TransHandles GLBridge::getTransHandles()
{
  return transHandles;
}

MaterialHandles GLBridge::getMaterialHandles()
{
  return materialHandles;
}

LightHandles GLBridge::getLightHandles()
{
  if(lightHandlesInUse < MAX_LIGHTS)
  {
    return lightHandlesArray[lightHandlesInUse++];
  }
  
  // TODO
  return LightHandles();
}

void GLBridge::onDraw()
{
  lightHandlesInUse = 0;
}

GLint GLBridge::getUViewMatrix()
{
  return h_uViewMatrix;
}

GLint GLBridge::getUProjMatrix()
{
  return h_uProjMatrix;
}

GLint GLBridge::getUCameraPos()
{
  return h_uCameraPos;
}

int GLBridge::getShaderProgram()
{
  return shaderProgram;
}