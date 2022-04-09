#pragma once
#include <vector>
#include "gravBody.h"

class System {
  private:
    float G = 6.67430e-11; // Modified by scale factors!

    std::vector<GravBody*> m_bodies;
    float m_timeFactor;

    // The scaling factors are needed to avoid float errors with using just SI units.
    float m_physicsDistanceFactor;
    float m_physicsMassFactor;

  public:
	  System();
    void setPhysicsDistanceFactor(float physicsDistanceFactor);
    void setPhysicsMassFactor(float physicsMassFactor);
    void addBody(GravBody* body);
    std::vector<GravBody*> getBodies();
    void update(float deltaT);
};