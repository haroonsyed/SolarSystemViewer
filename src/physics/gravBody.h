#pragma once
#include "nlohmann/json.hpp"
#include "../graphics/object/object.h"

class GravBody : public Object {
	private:
		glm::vec3 m_axis;
		float m_rotationSpeed; // In rad/s
		std::vector<std::string> m_planetInfo;

	public:
		GravBody();
		GravBody(float SIUnitScaleFactor, nlohmann::json jsonData);
		glm::vec3 getAxis();
		void setAxis(float x, float y, float z);
		void setTilt(float degrees);
		float getRotationSpeed();
		void setRotationSpeedFromPeriod(float hours);
		void print();
		std::string getPrintString();

		void addPlanetInfo(std::string info);
		std::vector<std::string> getPlanetInfo();
};