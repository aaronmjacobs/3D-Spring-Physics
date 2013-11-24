#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "glm/glm.hpp"

class SceneObject
{
protected:
  glm::vec3 position_;

public:
  virtual void translate(glm::vec3 trans);
  virtual void setPosition(glm::vec3 pos);
  virtual void resetTransforms();
  virtual void draw(float alpha) = 0;
  glm::vec3 getPosition()
  {
    return position_;
  }
};

#endif