#include "GravitationalForce.h"
#include "PhysModel.h"

GravitationalForce* GravitationalForce::create(PhysModel* target)
{
  return new GravitationalForce(target);
}

void GravitationalForce::applyForce(PhysModel* target, const PhysState& state, Derivative* derivative)
{
  derivative->force += glm::vec3(0.0f, state.mass * GRAVITY, 0.0f);
}

void GravitationalForce::draw(float alpha)
{
  //
}