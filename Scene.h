#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "SceneObject.h"
#include "PhysModel.h"
#include "Light.h"
#include "MatrixStack.h"

class Scene
{
private:
  std::vector<SceneObject*> sceneObjects;
  std::vector<Light*> lights;
  std::vector<PhysModel*> physObjects;
  Model* collisionSurface;

public:
  static MatrixStack stack;

  Scene();
  void add(SceneObject* sceneObject);
  bool remove(SceneObject* sceneObject);
  void add(Light* light);
  bool remove(Light* light);
  void add(PhysModel* physObject);
  bool remove(PhysModel* physObject);
  void setCollisionSurface(Model* model)
  {
    collisionSurface = model;
  }
  int getNumLights()
  {
    return lights.size();
  }
  void draw(float alpha);
  void step(float t, float dt);
  PhysModel* select(glm::vec3 start, glm::vec3 end);
};

#endif