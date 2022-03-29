#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "gravBody.h";

class System {
  private:
    std::vector<GravBody*> m_bodies;
    float m_timeFactor;

  public:
	System();
    void addBody(GravBody* body);
    std::vector<GravBody*> getBodies();
    void update();

};