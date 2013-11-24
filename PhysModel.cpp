#include "PhysModel.h"
#include "Force.h"
#include "SpringForce.h"
#include "Maths.h"
#include "GravitationalForce.h"
#include "glm/gtx/quaternion.hpp"

#define AIR_FRICTION 0.2f
#define GROUND_STATIC_FRICTION 5.0f
#define GROUND_KINETIC_FRICTION 2.0f

PhysModel::PhysModel(Mesh* mesh,
                     Material material,
                     float mass,
                     glm::vec3 position)
  : Model(mesh, material)
{
  currentState.mass = lastState.mass = nextState.mass = mass;
  currentState.inverseMass = lastState.inverseMass = nextState.inverseMass = 1.0f / currentState.mass;
  currentState.inertia = lastState.inertia = nextState.inertia = 1.0f; // TODO actually calculate based off of model size
  currentState.inverseInertia = lastState.inverseInertia = nextState.inverseInertia = 1.0f / currentState.inertia;
  
  currentState.position = lastState.position = nextState.position = position_ = position;
  currentState.linearMomentum = lastState.linearMomentum = nextState.linearMomentum = glm::vec3();
  currentState.orientation = lastState.orientation = nextState.orientation = glm::quat(1, 0, 0, 0);
  currentState.angularMomentum = lastState.angularMomentum = nextState.angularMomentum = glm::vec3();
  
  currentState.friction = lastState.friction = nextState.friction = AIR_FRICTION;
  
  visible = true;
}

PhysModel::~PhysModel()
{
  // We /have/ to iterate backwards, as when we delete each force, it is
  // automatically removed from the vector. On top of that, it's more efficient.
  for(size_t i = forces.size(); i > 0; --i)
  {
    delete forces[i - 1];
  }
}

glm::quat PhysState::spin()
{
  glm::vec3 angularVel = angularVelocity();
  orientation = glm::normalize(orientation);
  return 0.5f * glm::quat(0, angularVel.x, angularVel.y, angularVel.z) * orientation;
}

Derivative PhysModel::evaluate(PhysState state) // float? double? time to make a decision!
{
  Derivative result;
  result.velocity = state.velocity();
  result.spin = state.spin();

  // Apply forces to the current state
  applyForces(state, &result);

  return result;
}

Derivative PhysModel::evaluate(PhysState state, float dt, const Derivative& derivative)
{
  state.position += derivative.velocity * dt;
  state.linearMomentum += derivative.force * dt;
  state.orientation = state.orientation + derivative.spin * dt;
  state.angularMomentum += derivative.torque * dt;
  
  return evaluate(state);
}

void PhysModel::addForce(Force* force)
{
  forces.push_back(force);
}

bool PhysModel::removeForce(Force* force)
{
  for(std::vector<Force*>::iterator it = forces.begin(); it != forces.end(); ++it)
  {
    if(*it == force)
    {
      forces.erase(it);
      return true;
    }
  }
  
  return false;
}

void PhysModel::deleteSpringForce()
{
  for(size_t i = 0; i < forces.size(); ++i)
  {
    SpringForce* sForce = dynamic_cast<SpringForce*>(forces[i]);
    if(sForce)
    {
      delete sForce;
      return;
    }
  }
}

void PhysModel::applyForces(const PhysState& state, Derivative* derivative)
{
  // Sum up all current forces
  for(size_t i = 0; i < forces.size(); ++i)
  {
    forces[i]->applyForce(this, state, derivative);
  }
  
  // Apply friction
  glm::vec3 inverseVelocity(state.velocity());
  inverseVelocity *= -state.friction;
  derivative->force += inverseVelocity;
  
  glm::vec3 inverseAngularVelocity(state.angularVelocity());
  inverseAngularVelocity *= -state.friction;
  derivative->torque += inverseAngularVelocity;
}

void PhysModel::step(const double t, const double dt)
{
  lastState = currentState;
  currentState = nextState;
  
  // Integrate
  Derivative a = evaluate(currentState);
  Derivative b = evaluate(currentState, dt * 0.5f, a);
  Derivative c = evaluate(currentState, dt * 0.5f, b);
  Derivative d = evaluate(currentState, dt, c);
  
  float timeAdjust = 1.0f / 6.0f * dt;
  
  glm::vec3 dPos = (a.velocity + 2.0f * (b.velocity + c.velocity) + d.velocity);
  dPos *= timeAdjust;
  nextState.position += dPos;
  
  glm::vec3 dLinMoment = (a.force + 2.0f * (b.force + c.force) + d.force);
  dLinMoment *= timeAdjust;
  nextState.linearMomentum += dLinMoment;
  
  glm::quat dOrient = (a.spin + 2.0f * (b.spin + c.spin) + d.spin);
  dOrient *= timeAdjust;
  nextState.orientation = nextState.orientation + dOrient;
  
  glm::vec3 dAngMoment = (a.torque + 2.0f * (b.torque + c.torque) + d.torque);
  dAngMoment *= timeAdjust;
  nextState.angularMomentum += dAngMoment;
  
  if(onGround)
  {
    if(glm::length(nextState.linearMomentum) < 1.0f)
    {
      nextState.friction = GROUND_STATIC_FRICTION;
    }
    else
    {
      nextState.friction = GROUND_KINETIC_FRICTION;
    }
  }
  else
  {
    nextState.friction = AIR_FRICTION;
  }
}

void PhysModel::translate(glm::vec3 trans)
{
  position_ += trans;
  nextState.position += trans;
}

void PhysModel::draw(float alpha)
{
  // Calculate our rotation matrix from our orientation quaternion
  rotation_ = glm::toMat4((currentState.orientation * alpha) + (lastState.orientation * (1.0f - alpha)));
  position_ = (currentState.position * alpha) + (lastState.position * (1.0f - alpha));
  
  if(visible)
  {
    Model::draw(alpha);
  }
  
  for(size_t i = 0; i < forces.size(); ++i)
  {
    forces[i]->draw(alpha);
  }
}

bool PhysModel::intersects(Model* other)
{
  Bounds tRel, oRel;
  
  tRel.min = mesh->bounds.min * scale_ + nextState.position;
  tRel.max = mesh->bounds.max * scale_ + nextState.position;
  oRel.min = other->getMesh()->bounds.min * other->getScale() + other->getPosition();
  oRel.max = other->getMesh()->bounds.max * other->getScale() + other->getPosition();

  return !(max(tRel.min.x, oRel.min.x) > min(tRel.max.x, oRel.max.x)
        || max(tRel.min.y, oRel.min.y) > min(tRel.max.y, oRel.max.y)
        || max(tRel.min.z, oRel.min.z) > min(tRel.max.z, oRel.max.z));
}

bool PhysModel::isInOrBelow(Model* other)
{
  Bounds tRel, oRel;
  
  tRel.min = mesh->bounds.min * scale_ + nextState.position;
  tRel.max = mesh->bounds.max * scale_ + nextState.position;
  oRel.min = other->getMesh()->bounds.min * other->getScale() + other->getPosition();
  oRel.max = other->getMesh()->bounds.max * other->getScale() + other->getPosition();
  
  // Check if we are below
  if(tRel.min.y < oRel.max.y)
  {
    float diff = tRel.max.y - tRel.min.y;
    tRel.min.y = oRel.min.y - 0.1f;
    tRel.max.y = tRel.min.y + diff;
  }

  return !(max(tRel.min.x, oRel.min.x) > min(tRel.max.x, oRel.max.x)
        || max(tRel.min.y, oRel.min.y) > min(tRel.max.y, oRel.max.y)
        || max(tRel.min.z, oRel.min.z) > min(tRel.max.z, oRel.max.z));
}

void PhysModel::bounce(float elasticity, Model* other)
{
  Bounds tRel, oRel;
    
  tRel.min = mesh->bounds.min * scale_ + nextState.position;
  oRel.max = other->getMesh()->bounds.max * other->getScale() + other->getPosition();
  
  float yDiff = oRel.max.y - tRel.min.y;
  
  nextState.linearMomentum.y *= -elasticity;
  nextState.position.y += yDiff;
  nextState.friction = GROUND_STATIC_FRICTION; // Slow down
}

void PhysModel::setOnGround(bool onGround)
{
  this->onGround = onGround;
  if(onGround)
  {
    nextState.linearMomentum.y = 0.0f;
    nextState.position.y = currentState.position.y;
  }
}

void PhysModel::toggleGravity()
{
  for(size_t i = 0; i < forces.size(); ++i)
  {
    GravitationalForce* gForce = dynamic_cast<GravitationalForce*>(forces[i]);
    if(gForce)
    {
      delete gForce;
      return;
    }
  }
  
  GravitationalForce::create(this);
}

void PhysModel::addCollision(Model* other)
{
  collidingModels.push_back(other);
}

bool PhysModel::removeCollision(Model* other)
{
  for(std::vector<Model*>::iterator it = collidingModels.begin(); it != collidingModels.end(); ++it)
  {
    if(*it == other)
    {
      collidingModels.erase(it);
      return true;
    }
  }
  
  return false;
}

bool PhysModel::wasCollidingWith(Model* other)
{
  for(std::vector<Model*>::iterator it = collidingModels.begin(); it != collidingModels.end(); ++it)
  {
    if(*it == other)
    {
      return true;
    }
  }
  
  return false;
}