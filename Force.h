#ifndef FORCE_H
#define FORCE_H

#include "PhysModel.h"

class Force
{
protected:
  PhysModel* target;
  Force(PhysModel* target)
  {
    this->target = target;
    target->addForce(this);
  }

public:
  virtual ~Force()
  {
    target->removeForce(this);
  }

  virtual void applyForce(PhysModel* target, const PhysState& state, Derivative* derivative) = 0;
  virtual void draw(float alpha) = 0;
};

#endif