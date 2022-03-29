#include "system.h"
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>
#include "../config.h"

System::System() {
  // Influenced by G constant, default is aesthetic/arbitrary
  // The physics is made framerate independent by dividing by framerate for deltaT
  // Note that very low TargetFramerate (not lag though!) will cause physics to behave incorrectly
  const double DEFAULT_TIME_FACTOR = 1000;
  m_timeFactor = 1 * DEFAULT_TIME_FACTOR * (DEFAULT_TIME_FACTOR / Config::getInstance()->getTargetFramerate());
}

void System::addBody(GravBody* body) {
  m_bodies.push_back(body);
}

std::vector<GravBody*> System::getBodies() {
  return m_bodies;
}

void System::update() {
  
  const double G = 6.67430e-11 * 1e-12; // 1e-6 is a scale factor to avoid float error

  // DEBUG, print every x seconds
  if (std::fmod(glfwGetTime(),2) < (1.0/Config::getInstance()->getTargetFramerate())) {
    for (auto body : m_bodies) {
      body->print();
    }
  }

  // Possible to half time in future by storing force between bodies instead
  std::unordered_map<int, std::pair<glm::vec3, glm::vec3>> map;


  for(int i=0; i<m_bodies.size(); i++) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = m_bodies[i]->getMass();

    for(int j=0; j<m_bodies.size(); j++) {
      if(i != j) {
        
        // (G*M1*M2)/R^2
        float M2 = m_bodies[j]->getMass();
        
        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 temp = m_bodies[j]->getPosition() - m_bodies[i]->getPosition();
        float r2 = glm::dot(temp, temp);
        if (r2 < 25) {
            // Clamp force if two bodies pass close (5 megaM/5000km) to each other.
            // Effect is that they will continue current velocity.
            continue;
        }
        float magnitude = (G*M1*M2)/r2;


        glm::vec3 direction = glm::normalize(temp);

        force = force + (magnitude * direction); // Sum up all forces on object

      }
    }

    // Determine new velocity 
    // vf=vi+a*t where a=F/bodies

    glm::vec3 acceleration = force / M1;
    glm::vec3 velocity = m_bodies[i]->getVelocity() + (m_timeFactor * acceleration);
    glm::vec3 position = m_bodies[i]->getPosition() + (m_timeFactor * velocity);
    map[i] = std::make_pair(velocity, position);

  }


  // Update position and velocity
  for(int i=0; i<m_bodies.size(); i++) {
    
    m_bodies[i]->setVelocity(map[i].first);
    m_bodies[i]->setPosition(map[i].second);

  }

}

