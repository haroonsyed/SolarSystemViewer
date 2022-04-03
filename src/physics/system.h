#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "gravBody.h"

class System {
  private:
    std::vector<GravBody*> m_bodies;
    float m_timeFactor;
    float m_physicsDistanceFactor;
    float m_physicsMassFactor;

  public:
	  System();
    void setPhysicsDistanceFactor(float physicsDistanceFactor);
    void setPhysicsMassFactor(float physicsMassFactor);
    void addBody(GravBody* body);
    std::vector<GravBody*> getBodies();
    void update();
};