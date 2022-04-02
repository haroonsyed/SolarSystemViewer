#include "gravBody.h"
#include <iostream>

GravBody::GravBody() {
  m_velocity = glm::vec3(1.0);
  m_mass = 1.0;
}
glm::vec3 GravBody::getVelocity() {
  return m_velocity;
}
void GravBody::setVelocity(float x, float y, float z) {
  m_velocity = glm::vec3(x, y, z);
}
void GravBody::setVelocity(glm::vec3 velocity) {
  m_velocity = velocity;
}
float GravBody::getMass() {
  return m_mass;
}
void GravBody::setMass(float mass) {
  m_mass = mass;
}
void GravBody::print() {
	const float vx = m_velocity.x;
	const float vy = m_velocity.y;
	const float vz = m_velocity.z;
	const float px = getPosition().x;
	const float py = getPosition().y;
	const float pz = getPosition().z;
	std::cout << "Body: " << getName() << std::endl;
	std::cout << "Velocity: " << vx << " " << vy << " " << vz << std::endl;
	std::cout << "Position: " << px << " " << py << " " << pz << std::endl;
	std::cout << "" << std::endl;
}

