#pragma once
#include <glm/glm.hpp>
#include "nlohmann/json.hpp"
#include "../graphics/object/object.h"

class GravBody : public Object {
	private:
	  glm::vec3 m_velocity;
		glm::vec3 m_axis;
	  float m_mass;
		float m_rotationSpeed; // In rad/s
		std::vector<std::string> m_planetInfo;

	public:
	  GravBody();
		GravBody(float SIUnitScaleFactor, nlohmann::json jsonData);
	  glm::vec3 getVelocity();
	  void setVelocity(float x, float y, float z);
	  void setVelocity(glm::vec3 velocity);
		glm::vec3 getAxis();
		void setAxis(float x, float y, float z);
		void setTilt(float degrees);
		float getRotationSpeed();
		void setRotationSpeedFromPeriod(float hours);
	  float getMass();
	  void setMass(float mass);
	  void print();

		void addPlanetInfo(std::string info);
		std::vector<std::string> getPlanetInfo();
};