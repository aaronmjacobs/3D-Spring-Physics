#include "Scene.h"
#include "Force.h"

MatrixStack Scene::stack;

Scene::Scene()
{
  //
}

void Scene::add(SceneObject* sceneObject)
{
  sceneObjects.push_back(sceneObject);
}

bool Scene::remove(SceneObject* sceneObject)
{
  for(std::vector<SceneObject*>::iterator it = sceneObjects.begin(); it != sceneObjects.end(); ++it)
  {
    if(*it == sceneObject)
    {
      sceneObjects.erase(it);
      return true;
    }
  }
  
  return false;
}

void Scene::add(Light* light)
{
  lights.push_back(light);
}

bool Scene::remove(Light* light)
{
  for(std::vector<Light*>::iterator it = lights.begin(); it != lights.end(); ++it)
  {
    if(*it == light)
    {
      lights.erase(it);
      return true;
    }
  }
  
  return false;
}

void Scene::add(PhysModel* physObject)
{
  physObjects.push_back(physObject);
}

bool Scene::remove(PhysModel* physObject)
{
  for(std::vector<PhysModel*>::iterator it = physObjects.begin(); it != physObjects.end(); ++it)
  {
    if(*it == physObject)
    {
      for(std::vector<Light*>::iterator lit = lights.begin(); lit != lights.end();)
      {
        if((*lit)->isAttachedTo(physObject))
        {
          Light* toRemove = *lit;
          lit = lights.erase(lit);
          delete toRemove;
        }
        else
        {
          ++lit;
        }
      }
      
      physObjects.erase(it);
      return true;
    }
  }
  
  return false;
}

void Scene::draw(float alpha)
{
  for(size_t i = 0; i < lights.size(); ++i)
  {
    lights[i]->draw(alpha);
  }
  
  for(size_t i = 0; i < sceneObjects.size(); ++i)
  {
    sceneObjects[i]->draw(alpha);
  }
  
  for(size_t i = 0; i < physObjects.size(); ++i)
  {
    physObjects[i]->draw(alpha);
  }
}

void Scene::step(float t, float dt)
{
  for(unsigned int i = 0; i < physObjects.size(); ++i)
  {
    physObjects[i]->step(t, dt);
    
    if(collisionSurface)
    {
      if(physObjects[i]->isInOrBelow(collisionSurface))
      {
        if(physObjects[i]->wasCollidingWith(collisionSurface))
        {
          physObjects[i]->setOnGround(true);
          continue;
        }
        else
        {
          physObjects[i]->bounce(0.3f, collisionSurface);
          physObjects[i]->addCollision(collisionSurface);
        }
      }
      else
      {
        physObjects[i]->removeCollision(collisionSurface);
      }
    }
    physObjects[i]->setOnGround(false);
  }
}

PhysModel* Scene::select(glm::vec3 start, glm::vec3 end)
{
  PhysModel* hit = NULL;
  float depth, minDepth;

  for(unsigned int i = 0; i < physObjects.size(); ++i)
  {
    if(physObjects[i]->intersectionDepth(start, end, &depth))
    {
      if(!hit || depth < minDepth)
      {
        hit = physObjects[i];
        minDepth = depth;
      }
    }
  }
  
  return hit;
}