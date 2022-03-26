#pragma once
#include <vector>
#include <glm/glm.hpp>

class Light {
private:
	glm::vec3 position;
	std::vector<float> color;
	float intensity;

public:
	void setPosition(glm::vec3 position);
	void setPosition(float x, float y, float z);
	glm::vec3 getPosition();
	void setColor(float red, float green, float blue);
	std::vector<float> getColor();
	void setIntensity(float intensity);
	float getIntensity();

};
