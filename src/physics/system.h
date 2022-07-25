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

    void updateUsingBarnesHut(float adjustedTimeFactor, std::unordered_map<int, std::pair<glm::vec3, glm::vec3>>& map);
    void updateUsingNaive(float adjustedTimeFactor, std::unordered_map<int, std::pair<glm::vec3, glm::vec3>>& map);

  public:
	  System();
    float getSIUnitScaleFactor();
    void setSIUnitScaleFactor(float physicsDistanceFactor);
    void addBody(GravBody* body);
    std::vector<GravBody*> getBodies();
    void update(float deltaT);
};