#pragma once
#include <glm/glm.hpp>
#include "../graphics/object/object.h"

class GravBody : public Object {
	private:
	  glm::vec3 m_velocity;
	  float m_mass;

	public:
	  GravBody();
	  glm::vec3 getVelocity();
	  void setVelocity(float x, float y, float z);
	  void setVelocity(glm::vec3 velocity);
	  float getMass();
	  void setMass(float mass);
	  void print();

};