#pragma once
#include <glm/glm.hpp>
#include "../graphics/object/object.h"

class GravBody : public Object {
	private:
	  glm::vec3 velocity;
	  float mass;

	public:
	  GravBody();
	  glm::vec3 getVelocity();
	  void setVelocity(float x, float y, float z);
	  void setVelocity(glm::vec3 velocity);
	  float getMass();
	  void setMass(float mass);
	  void print();

};