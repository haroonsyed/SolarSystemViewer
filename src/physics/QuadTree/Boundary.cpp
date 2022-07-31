#include "Boundary.h"

Boundary::Boundary(glm::vec2 position, glm::vec2 dimensions) {
	this->m_position = position;
	this->m_dimensions = dimensions;
}

Boundary::Boundary(const Boundary& old_obj) {
	m_position = old_obj.m_position;
	m_dimensions = old_obj.m_dimensions;
}

bool Boundary::containsPoint(glm::vec2 point) {
	return glm::all(glm::greaterThanEqual(point, m_position)) && glm::all(glm::lessThanEqual(point, m_position+m_dimensions));
}

bool Boundary::overlapsBoundary(Boundary& range) {
	auto m_maxBounds = m_position + m_dimensions;
	auto range_maxBounds = range.getPosition() + range.getDimensions();
	return glm::all(glm::lessThan(m_position,range_maxBounds)) && glm::all(glm::lessThan(range.getPosition(), m_maxBounds));
}

glm::vec2 Boundary::getPosition() {
	return m_position;
}

glm::vec2 Boundary::getDimensions() {
	return m_dimensions;
}

