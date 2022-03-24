#pragma once
#include <glm/glm.hpp>
#include "../mesh/mesh.h"

class GravBody {
	private:
    glm::vec3 position;
		glm::vec3 velocity;
		float mass;
		Mesh mesh;

	public:
		GravBody();

		glm::vec3 getPosition();
		void setPosition(float x, float y, float z);
		void setPosition(glm::vec3 position);
		glm::vec3 getVelocity();
		void setVelocity(float x, float y, float z);
		void setVelocity(glm::vec3 velocity);
		float getMass();
		void setMass(float mass);
		void print();
		Mesh* getMesh();

};