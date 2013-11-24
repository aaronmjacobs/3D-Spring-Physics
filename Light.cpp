#include "Light.h"
#include "Mesh.h"
#include "PhysModel.h"

Light::Light(glm::vec3 position, glm::vec3 color, float constFalloff, float linearFalloff, float squareFalloff)
{
  this->position_ = position;
  this->color = color;
  this->constFalloff = constFalloff;
  this->linearFalloff = linearFalloff;
  this->squareFalloff = squareFalloff;
  this->attachment = NULL;
  //this->handles = GLBridge::getLightHandles();
}

void Light::attachTo(PhysModel* physModel)
{
  attachment = physModel;
  attachment->setVisible(false);
}

void Light::detach()
{
  attachment->setVisible(true);
  attachment = NULL;
}

bool Light::isAttachedTo(PhysModel* physModel)
{
  return physModel && physModel == this->attachment;
}

void Light::draw(float alpha)
{
  if(attachment)
  {
    setPosition(attachment->getPosition());
  }
  
  this->handles = GLBridge::getLightHandles();
  glUniform3fv(handles.uLightPosition, 1, glm::value_ptr(position_));
  glUniform3fv(handles.uLightColor, 1, glm::value_ptr(color));
  glUniform1f(handles.uLightConstFalloff, constFalloff);
  glUniform1f(handles.uLightLinearFalloff, linearFalloff);
  glUniform1f(handles.uLightSquareFalloff, squareFalloff);
  
  if(model)
  {
    model->setPosition(position_);
    model->draw(alpha);
  }
}

void Light::drawModel()
{
  Material lightMaterial;
  lightMaterial.ambient = color * 0.0f;
  lightMaterial.diffuse = color * 0.0f;
  lightMaterial.specular = color * 0.0f;
  lightMaterial.emission = color * 1.0f;
  lightMaterial.shininess = 1.0f;
  
  Mesh* sphere = Mesh::load("SimpleModels/sphere.obj", true);
  model = new Model(sphere, lightMaterial);
  model->scale(0.7f);
}