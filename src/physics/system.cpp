#include "system.h"

System::System() {
  timeFactor = 1000;
}

void System::addBody(GravBody& body) {
  bodies.push_back(body);
}

void System::update() {
  
  const float G = 6.67430e-11;

  // FIND A WAY TO OPTIMIZE (perhaps memoization?!?)
  for(GravBody &body1 : bodies) {
    glm::vec3 force = glm::vec3(0.0);
    const float M1 = body1.getMass();

    for(GravBody &body2 : bodies) {
      if(&body1 != &body2) {
        
        // (G*M1*M2)/R^2
        float M2 = body2.getMass();
        
        // Below avoids sqrt (otherwise one can use distance)
        glm::vec3 temp = body1.getPosition() - body2.getPosition();
        float r2 = glm::dot(temp, temp);
        float magnitude = (G*M1*M2)/r2;

        glm::vec3 direction = glm::normalize((-1.0f * temp));

        force = force + magnitude * direction; // Sum up all forces on object

      }
    }

    // Determine new velocity 
    // vf=vi+a*t where a=F/m
    glm::vec3 acceleration = force/M1;
    body1.setVelocity( body1.getVelocity() + acceleration * timeFactor );

    // Determine new position based on velocity
    body1.setPosition( body1.getPosition() + timeFactor * body1.getVelocity() );

  }

}

