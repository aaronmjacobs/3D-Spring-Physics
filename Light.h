#ifndef LIGHT_H
#define LIGHT_H

#include "GLBridge.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Model.h"

class PhysModel;

class Light : public SceneObject
{
private:
  glm::vec3 color;
  float constFalloff, linearFalloff, squareFalloff;
  LightHandles handles;
  Model* model;
  PhysModel* attachment;

public:
  Light(glm::vec3 position, glm::vec3 color, float constFalloff, float linearFalloff, float squareFalloff);
  void attachTo(PhysModel* physModel);
  void detach();
  bool isAttachedTo(PhysModel* physModel);
  virtual void draw(float alpha); // override
  void drawModel();
};

#endif