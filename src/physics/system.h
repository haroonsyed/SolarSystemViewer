#pragma once
#include <vector>
#include "gravBody.h"

class System {
  private:
    float G = 6.67430e-11; // Modified by scale factor!

    std::vector<GravBody*> m_bodies;
    float m_timeFactor;

    // The scaling factor is needed to avoid float errors with using just SI units.
    float m_SIUnitScaleFactor;

  public:
	  System();
    float getSIUnitScaleFactor();
    void setSIUnitScaleFactor(float physicsDistanceFactor);
    void addBody(GravBody* body);
    std::vector<GravBody*> getBodies();
    void update(float deltaT);
};