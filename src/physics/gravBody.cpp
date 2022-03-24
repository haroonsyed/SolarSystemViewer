#include "gravBody.h"
#include <iostream>

GravBody::GravBody() {
  position = glm::vec3(1.0);
  velocity = glm::vec3(1.0);
  mass = 1.0;
}

glm::vec3 GravBody::getPosition() {
  return position;
}
void GravBody::setPosition(float x, float y, float z) {
  this->position = glm::vec3(x, y, z);
}
void GravBody::setPosition(glm::vec3 position) {
  this->position = position;
}
glm::vec3 GravBody::getVelocity() {
  return velocity;
}
void GravBody::setVelocity(float x, float y, float z) {
  velocity = glm::vec3(x, y, z);
}
void GravBody::setVelocity(glm::vec3 velocity) {
  this->velocity = velocity;
}
float GravBody::getMass() {
  return mass;
}
void GravBody::setMass(float mass) {
  this->mass = mass;
}
void GravBody::print() {
	const float vx = velocity.x;
	const float vy = velocity.y;
	const float vz = velocity.z;
	const float px = position.x;
	const float py = position.y;
	const float pz = position.z;
	std::cout << "Velocity: " << vx << " " << vy << " " << vz << std::endl;
	std::cout << "Position: " << px << " " << py << " " << pz << std::endl;
}
Mesh* GravBody::getMesh() {
  return &mesh;
}
