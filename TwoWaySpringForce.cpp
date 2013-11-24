#include "TwoWaySpringForce.h"

TwoWaySpringForce* TwoWaySpringForce::create(PhysModel* target, PhysModel* secondTarget, float k, float b, glm::vec3 attachOffset, glm::vec3 secondAttachOffset)
{
  return new TwoWaySpringForce(target, secondTarget, k, b, attachOffset, secondAttachOffset);
}

TwoWaySpringForce::TwoWaySpringForce(PhysModel* target, PhysModel* secondTarget, float k, float b, glm::vec3 attachOffset, glm::vec3 secondAttachOffset)
  : SpringForce(target, glm::vec3(), k, b, attachOffset)
{
  this->secondTarget = secondTarget;
  secondTarget->addForce(this);
  
  this->secondAttachOffset = secondAttachOffset;
  
  currentAttachPos = target->getPosition();
  secondCurrentAttachPos = secondTarget->getPosition();
}

void TwoWaySpringForce::applyForce(PhysModel* target, const PhysState& state, Derivative* derivative)
{
  // x = vector difference between the target point and attachment point on the object
  
  glm::mat4 transMatrix = glm::translate(state.position);
  glm::mat4 rotMatrix = glm::toMat4(state.orientation);
  glm::mat4 scaleMatrix = glm::scale(glm::vec3(target->getScale()));
  glm::mat4 transform = transMatrix * rotMatrix * scaleMatrix;
  glm::vec3 attachPos;
  
  glm::vec3 otherPos;
  if(target == this->target)
  {
    glm::vec4 attachmentOffset(attachOffset.x, attachOffset.y, attachOffset.z, 1.0f);
    glm::vec4 attach = transform * attachmentOffset;
    attachPos = glm::vec3(attach.x, attach.y, attach.z);

    otherPos = this->secondTarget->getPosition();
    currentAttachPos = attachPos;
  }
  else
  {
    glm::vec4 attachmentOffset(secondAttachOffset.x, secondAttachOffset.y, secondAttachOffset.z, 1.0f);
    glm::vec4 attach = transform * attachmentOffset;
    attachPos = glm::vec3(attach.x, attach.y, attach.z);
  
    otherPos = this->target->getPosition();
    secondCurrentAttachPos = attachPos;
  }
  
  glm::vec3 x = attachPos - otherPos;
  glm::vec3 v = state.velocity() + glm::cross(state.angularVelocity(), otherPos - state.position);
  
  glm::vec3 f = (-k * x) - (b * v);
  f *= 0.5f; // TODO
  derivative->force += f;
  derivative->torque += glm::cross(f, state.position - attachPos);
}

void TwoWaySpringForce::draw(float alpha)
{
  glm::vec3 toTarget = secondCurrentAttachPos - currentAttachPos;
  
  for(int i = 0; i <= NUM_MARKERS; ++i)
  {
    glm::vec3 markerPos = currentAttachPos + (toTarget * (i / (float)NUM_MARKERS));
    model->setPosition(markerPos);
    model->draw(alpha);
  }
}