#include "Boundary.h"

Boundary::Boundary(glm::vec2 position, glm::vec2 dimensions) {
	this->m_position = position;
	this->m_dimensions = dimensions;
}

bool Boundary::containsPoint(glm::vec2 point) {
	return (point >= m_position) && (point <= (m_position + m_dimensions));
}

bool Boundary::overlapsBoundary(Boundary* range) {
	const m_maxBounds = m_position + m_dimensions;
	const range_maxBounds = range->getPosition() + range->getDimensions();
	return (m_position < range_maxBounds) && (range->getPosition() < m_maxBounds);
}

glm::vec2 Boundary::getPosition() {
	return m_position;
}

glm::vec2 Boundary::getDimensions() {
	return m_dimensions;
}

