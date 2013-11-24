#ifndef PHYS_MODEL_H
#define PHYS_MODEL_H

#include <vector>

#include "Model.h"

class Force;

struct PhysState
{
  // Constants
  float mass, inverseMass;
  float inertia, inverseInertia;
  float friction;
  
  // Primary values
  glm::vec3 position;
  glm::vec3 linearMomentum;
  glm::quat orientation;
  glm::vec3 angularMomentum;
  
  // Secondary values
  glm::vec3 velocity() const
  {
    return linearMomentum * inverseMass;
  }
  glm::vec3 angularVelocity() const
  {
    return angularMomentum * inverseInertia;
  }
  glm::quat spin();
};

struct Derivative
{
  glm::vec3 velocity;
  glm::vec3 force;
  glm::quat spin;
  glm::vec3 torque;
};

class PhysModel : public Model
{
private:
  PhysState currentState, lastState, nextState;
  std::vector<Force*> forces;
  std::vector<Model*> collidingModels;
  bool onGround;
  bool visible;
  
  // Calculations
  Derivative evaluate(PhysState state);
  Derivative evaluate(PhysState state, float dt, const Derivative& derivative);
  void applyForces(const PhysState& state, Derivative* derivative);

public:  
  PhysModel(Mesh* mesh,
            Material material,
            float mass = 1.0f,
            glm::vec3 position = glm::vec3());
  ~PhysModel();
  glm::vec3 getVelocity()
  {
    return currentState.velocity();
  }
  void bounce(float elasticity, Model* other);
  void setOnGround(bool onGround);
  void step(const double t, const double dt);
  void addForce(Force* force);
  bool removeForce(Force* force);
  void deleteSpringForce();
  virtual void translate(glm::vec3 trans);
  virtual void draw(float alpha); // override
  bool intersects(Model* other);
  bool isInOrBelow(Model* other);
  bool isOnGround()
  {
    return onGround;
  }
  void setVisible(bool visible)
  {
    this->visible = visible;
  }
  void toggleGravity();
  void addCollision(Model* other);
  bool removeCollision(Model* other);
  bool wasCollidingWith(Model* other);
};

#endif