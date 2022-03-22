#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "gravBody.h";

class System {
	private:
    std::vector<GravBody> bodies;
    float timeFactor;

	public:
		System();
    void addBody(GravBody& body);
    void update();

};