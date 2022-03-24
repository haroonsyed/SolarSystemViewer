#include "system.h"
#include <unordered_map>
#include <GLFW/glfw3.h>

System::System() {
  timeFactor = 1;
}

void System::addBody(GravBody& body) {
  bodies.push_back(body);
}

std::vector<GravBody> System::getBodies() {
    return bodies;
}

void System::update() {
  
  const double G = 6.67430e-11 * 1e-6; // 1e-6 is a scale factor to avoid float error

  // Possible to half time in future by storing force between bodies instead
  std::unordered_map<int, std::pair<glm::vec3, glm::vec3>> map;


  for(int i=0; i<bodies.size(); i++) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = bodies[i].getMass();

    for(int j=0; j<bodies.size(); j++) {
      if(i != j) {
        
        // (G*M1*M2)/R^2
        float M2 = bodies[j].getMass();
        
        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 temp = bodies[j].getPosition() - bodies[i].getPosition();
        float r2 = glm::dot(temp, temp);
        float num = G * M1 * M2;
        float magnitude = (G*M1*M2)/r2;

        glm::vec3 direction = glm::normalize(temp);

        force = force + (magnitude * direction); // Sum up all forces on object

      }
    }

    // Determine new velocity 
    // vf=vi+a*t where a=F/m
    glm::vec3 acceleration = force / M1;
    glm::vec3 velocity = bodies[i].getVelocity() + (timeFactor * acceleration);
    glm::vec3 position = bodies[i].getPosition() + (timeFactor * velocity);
    map[i] = std::make_pair(velocity, position);

  }


  // Update position and velocity
  for(int i=0; i<bodies.size(); i++) {
    
    bodies[i].setVelocity(map[i].first);
    bodies[i].setPosition(map[i].second);
    //if (i==0) {
    //    bodies[i].setPosition(((float)glfwGetTime()) * glm::vec3(10000));
    //}

  }

}

