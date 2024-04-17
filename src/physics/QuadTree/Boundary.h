#pragma once
#include <glm/glm.hpp>

class Boundary {
private:
    glm::vec2 m_position;
    glm::vec2 m_dimensions;

public:

    Boundary(glm::vec2 position, glm::vec2 dimensions);
    Boundary(const Boundary& old_obj);
    bool containsPoint(glm::vec2 position);
    bool overlapsBoundary(Boundary& range);
    glm::vec2 getPosition();
    glm::vec2 getDimensions();

};