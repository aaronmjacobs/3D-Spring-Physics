#include "SpringForce.h"
#include "PhysModel.h"

Model* SpringForce::model;

SpringForce* SpringForce::create(PhysModel* target, glm::vec3 position, float k, float b, glm::vec3 attachOffset)
{
  return new SpringForce(target, position, k, b, attachOffset);
}

SpringForce::SpringForce(PhysModel* target, glm::vec3 position, float k, float b, glm::vec3 attachOffset)
  : Force(target)
{
  if(!model)
  {
    Material springMaterial;
    glm::vec3 baseSpringColor(0.0f, 1.0f, 0.5f);
    springMaterial.ambient = baseSpringColor * 0.2f;
    springMaterial.diffuse = baseSpringColor * 0.4f;
    springMaterial.specular = glm::vec3(0.4f, 0.4f, 0.4f);
    springMaterial.emission = baseSpringColor * 0.0f;
    springMaterial.shininess = 200.0f;
    Mesh* sphereMesh = Mesh::load("SimpleModels/sphere.obj", false);
    model = new Model(sphereMesh, springMaterial);
    model->scale(0.15f);
  }
  
  this->position = position;
  this->k = k;
  this->b = b;
  this->attachOffset = attachOffset;
  currentAttachPos = target->getPosition();
}

SpringForce::~SpringForce()
{
  //
}

void SpringForce::applyForce(PhysModel* target, const PhysState& state, Derivative* derivative)
{
  glm::mat4 transMatrix = glm::translate(state.position);
  glm::mat4 rotMatrix = glm::toMat4(state.orientation);
  glm::mat4 scaleMatrix = glm::scale(glm::vec3(target->getScale()));
  glm::mat4 transform = transMatrix * rotMatrix * scaleMatrix;
  glm::vec4 attachmentOffset(attachOffset.x, attachOffset.y, attachOffset.z, 1.0f);
  glm::vec4 attach = transform * attachmentOffset;
  glm::vec3 attachPos(attach.x, attach.y, attach.z);
  currentAttachPos = attachPos;
  
  glm::vec3 x = attachPos - this->position;
  glm::vec3 v = state.velocity() + glm::cross(state.angularVelocity(), this->position - state.position);
  
  glm::vec3 f = (-k * x) - (b * v);
  derivative->force += f;
  derivative->torque += glm::cross(f, state.position - attachPos);
}

void SpringForce::draw(float alpha)
{
  glm::vec3 toTarget = currentAttachPos - position;
  
  for(int i = 0; i <= NUM_MARKERS; ++i)
  {
    glm::vec3 markerPos = position + (toTarget * (i / (float)NUM_MARKERS));
    model->setPosition(markerPos);
    model->draw(alpha);
  }
}