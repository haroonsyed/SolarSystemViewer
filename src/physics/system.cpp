#include "system.h"
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>
#include "../config.h"
#include "QuadTree/QuadTree.h"

System::System() {
  m_timeFactor = 60 * 60 * 23.9345; // Default Once earth day per second;
  m_SIUnitScaleFactor = 1e6f;
}

float System::getSIUnitScaleFactor() {
  return m_SIUnitScaleFactor;
}

void System::setSIUnitScaleFactor(float SIUnitScaleFactor) {
  m_SIUnitScaleFactor = SIUnitScaleFactor;
  G = 6.67430e-11 / SIUnitScaleFactor / SIUnitScaleFactor; // Since newton is kg*m
}

void System::addBody(GravBody* body) {
  m_bodies.push_back(body);
}

std::vector<GravBody*> System::getBodies() {
  return m_bodies;
}

void System::update(float deltaT) {
  if (deltaT > 0.1f) {
    // Don't calculate physics when deltaT is large, introduces error into calculation
    return;
  }

  // Let compute shader calculate large simulations
  if (m_bodies.size() > 1000) {
    return;
  }

  // The physics is made framerate independent by dividing by framerate for deltaT
  float adjustedTimeFactor = m_timeFactor * deltaT;

  // Holds the velocity and position as calculated for each object
  std::unordered_map<int, std::pair<glm::vec3, glm::vec3>> map;

  // First build the quad tree
  glm::vec2 boundStart = glm::vec2(-1e3, -1e3);
  glm::vec2 boundRange = glm::abs(boundStart * 2.0f);
  Boundary bounds(boundStart, boundRange);
  QuadTree qTree(bounds);
  
  // Insert all bodies into quad tree
  for (auto body : m_bodies) {
      qTree.insert(body);
  }

  // Caclulate center of mass and total mass of quad trees
  qTree.aggregateCenterAndTotalMass();


  const float theta = 1.0;
  for(int i=0; i<m_bodies.size(); i++) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = m_bodies[i]->getMass();

    const auto relevantBodies = qTree.barnesHutQuery(m_bodies[i], theta);

    std::cout << "Relevant bodies for: " << m_bodies[i]->getName() << std::endl;
    for(int j = 0; j < relevantBodies.size(); j++) {
        std::cout << relevantBodies[j]->getName() << " " << std::endl;
      if(relevantBodies[j] != m_bodies[i]) { // Don't do gravity with itself

          
        
        // (G*M1*M2)/R^2
        float M2 = relevantBodies[j]->getMass();
        
        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 r = relevantBodies[j]->getPosition() - m_bodies[i]->getPosition();
        float r2 = glm::dot(r, r);
        if (r2 < 25) {
            // Clamp force if two bodies pass close (5 megaM/5000km) to each other.
            // Effect is that they will continue current velocity.
            continue;
        }
        float magnitude = (G*M1*M2)/r2;


        glm::vec3 direction = glm::normalize(r);

        force = force + (magnitude * direction); // Sum up all forces on object

      }
    }

    // Determine new velocity 
    // vf=vi+a*t where a=F/bodies

    glm::vec3 acceleration = force / M1;
    glm::vec3 velocity = m_bodies[i]->getVelocity() + (adjustedTimeFactor * acceleration);
    glm::vec3 position = m_bodies[i]->getPosition() + (adjustedTimeFactor * velocity);
    map[i] = std::make_pair(velocity, position);

  }


  // Update position and velocity
  for (int i = 0; i < m_bodies.size(); i++) {

    GravBody* body = m_bodies[i];

    float period = (2 * 3.14159265f) / body->getRotationSpeed();
    float rotSpeed = body->getRotationSpeed() * period;

    body->setVelocity(map[i].first);
    body->setPosition(map[i].second);
    body->rotate(glm::angleAxis(
      body->getRotationSpeed() * adjustedTimeFactor,
      body->getAxis()
    ));

  }

}

