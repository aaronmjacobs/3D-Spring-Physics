#ifndef TWO_WAY_SPRING_FORCE_H
#define TWO_WAY_SPRING_FORCE_H

#include "Force.h"
#include "SpringForce.h"

class TwoWaySpringForce : public SpringForce
{
private:
  PhysModel* secondTarget;
  glm::vec3 secondCurrentAttachPos;
  glm::vec3 secondAttachOffset;
  TwoWaySpringForce(PhysModel* target, PhysModel* secondTarget, float k, float b, glm::vec3 attachOffset, glm::vec3 secondAttachOffset);

public:
  static TwoWaySpringForce* create(PhysModel* target, PhysModel* secondTarget, float k, float b, glm::vec3 attachOffset = glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3 secondAttachOffset = glm::vec3(-0.5f, 0.0f, 0.0f)); // TODO
  virtual ~TwoWaySpringForce()
  {
    secondTarget->removeForce(this);
  }
  virtual void applyForce(PhysModel* target, const PhysState& state, Derivative* derivative);
  virtual void draw(float alpha);
};

#endif