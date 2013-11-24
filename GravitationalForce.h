#ifndef GRAVITATIONAL_FORCE_H
#define GRAVITATIONAL_FORCE_H

#include "Force.h"

#define GRAVITY -9.8f

class GravitationalForce : public Force
{
private:
  GravitationalForce(PhysModel* target)
      : Force(target)
  {
    //
  }
public:
  static GravitationalForce* create(PhysModel* target);
  
  virtual void applyForce(PhysModel* target, const PhysState& state, Derivative* derivative);
  virtual void draw(float alpha);
};

#endif