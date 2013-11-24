#include "SceneObject.h"

void SceneObject::translate(glm::vec3 trans)
{
  position_ += trans;
}
void SceneObject::setPosition(glm::vec3 pos)
{
  position_ = pos;
}

void SceneObject::resetTransforms()
{
  position_ = glm::vec3();
}