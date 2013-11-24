#ifndef SPRING_FORCE_H
#define SPRING_FORCE_H

#include "Force.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#define NUM_MARKERS 15

class Model;

class SpringForce : public Force
{
protected:
  static Model* model;
  glm::vec3 position;
  glm::vec3 attachOffset;
  glm::vec3 currentAttachPos;
  float k, b;
  SpringForce(PhysModel* target, glm::vec3 position, float k, float b, glm::vec3 attachOffset);
  
public:
  static SpringForce* create(PhysModel* target, glm::vec3 position, float k, float b, glm::vec3 attachOffset = glm::vec3(-0.5f, 0.0f, 0.0f)); // TODO
  virtual ~SpringForce();
  void setPosition(glm::vec3 position)
  {
    this->position = position;
  }
  void translate(glm::vec3 delta)
  {
    this->position += delta;
  }
  virtual void applyForce(PhysModel* target, const PhysState& state, Derivative* derivative);
  virtual void draw(float alpha);
};

#endif