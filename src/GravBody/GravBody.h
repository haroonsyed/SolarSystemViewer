#pragma once
#include <glm/glm.hpp>

class GravBody {
	private:
		glm::vec3 position;
		glm::vec3 velocity;
		float mass;

	public:
		GravBody();

		getPosition();
		setPosition();
		getVelocity();
		setVelocity();
		getMass();
		setMass();

};