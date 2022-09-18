#include <gl/glew.h>
#include "system.h"
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>
#include "../config.h"
#include "QuadTree/QuadTree.h"
#include "../graphics/shader/shaderManager.h"
#include "QuadTreeGPU/quadTreeUtil.h"

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

void System::setTimeFactor(float timeFactor) {
    m_timeFactor = timeFactor;
}

void System::addBody(GravBody* body) {
  m_bodies.push_back(body);
}

// Sets all bodies in gpu. 
void System::setBodiesGPU(std::vector<Body>& bodies, int numberOfLevelsInTree) {
    
    // Determine the size of the tree to create based on numberofLevels
    const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);

    // Create SSBO_BODIES
    glGenBuffers(1, &m_SSBO_BODIES);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_BODIES);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_SSBO_BODIES);

    // Create SSBO_TREE
    glGenBuffers(1, &m_SSBO_TREE);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_TREE);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_SSBO_TREE);

    m_SSBO_BODIES_COUNT = bodies.size();
    m_SSBO_TREE_COUNT = treeSize;

}

std::vector<GravBody*> System::getBodies() {
  return m_bodies;
}

void System::updateUsingNaive(float adjustedTimeFactor) {

  // Holds the velocity and position as calculated for each object
  std::unordered_map<int, std::pair<glm::vec3, glm::vec3>> map;

  for (int i = 0; i < m_bodies.size(); i++) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = m_bodies[i]->getMass();

    for (int j = 0; j < m_bodies.size(); j++) {
      if (m_bodies[j] != m_bodies[i]) { // Don't do gravity with itself

        // (G*M1*M2)/R^2
        float M2 = m_bodies[j]->getMass();

        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 r = m_bodies[j]->getPosition() - m_bodies[i]->getPosition();
        float r2 = glm::dot(r, r);
        //if (r2 < (1e14 / (m_SIUnitScaleFactor * m_SIUnitScaleFactor))) {
        //  // Clamp force if two bodies pass close (1e7m) to each other.
        //  // Effect is that they will continue current velocity.
        //  continue;
        //}
        float magnitude = (G * M1 * M2) / r2;


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


void System::updateUsingBarnesHut(float adjustedTimeFactor) {

  // Holds the velocity and position as calculated for each object
  std::unordered_map<int, std::pair<glm::vec3, glm::vec3>> map;

  // First build the quad tree
  glm::vec2 boundStart = glm::vec2(-1e10, -1e10);
  glm::vec2 boundRange = glm::abs(boundStart * 2.0f);
  Boundary bounds(boundStart, boundRange);
  QuadTree qTree(bounds);

  double startTime = glfwGetTime();

  // Insert all bodies into quad tree
  for (auto body : m_bodies) {
    qTree.insert(body);
  }

  std::cout << "\nTime to build tree: " << (glfwGetTime() - startTime) * 1000 << " ms" << std::endl;
  double startAgg = glfwGetTime();


  // Caclulate center of mass and total mass of quad trees
  qTree.aggregateCenterAndTotalMass();
  std::cout << "Time to aggregate tree: " << (glfwGetTime() - startAgg) * 1000 << " ms" << std::endl;

  double calculateForceStart = glfwGetTime();

  const float theta = 1.5;
  for (int i = 0; i < m_bodies.size(); i++) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = m_bodies[i]->getMass();

    const std::vector<GravBody*> relevantBodies = qTree.barnesHutQuery(m_bodies[i], theta);

    for (int j = 0; j < relevantBodies.size(); j++) {
      if (relevantBodies[j] != m_bodies[i]) { // Don't do gravity with itself

        // (G*M1*M2)/R^2
        float M2 = relevantBodies[j]->getMass();

        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 r = relevantBodies[j]->getPosition() - m_bodies[i]->getPosition();
        float r2 = glm::dot(r, r);
        if (r2 < (1e14 / (m_SIUnitScaleFactor * m_SIUnitScaleFactor))) {
          // Clamp force if two bodies pass close (1e7m) to each other.
          // Effect is that they will continue current velocity.
          continue;
        }
        float magnitude = (G * M1 * M2) / r2;


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
  std::cout << "Time to calculate forces: " << (glfwGetTime() - calculateForceStart) * 1000 << " ms" << std::endl;

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

  double endTime = glfwGetTime();
  std::cout << "\nTime to process physics (CPU): " << (endTime - startTime) * 1000 << " ms" << std::endl;

}

void System::updateUsingBarnesHutGPU(float adjustedTimeFactor) {



}

void System::update(float deltaT) {
  if (deltaT > 0.1f) {
    // Don't calculate physics when deltaT is large, introduces error into calculation
    return;
  }

  // The physics is made framerate independent by dividing by framerate for deltaT
  float adjustedTimeFactor = m_timeFactor * deltaT;

  // Calculate physics
  updateUsingBarnesHut(adjustedTimeFactor);

}

