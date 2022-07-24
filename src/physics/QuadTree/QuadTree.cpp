#include "QuadTree.h"

QuadTree::QuadTree(Boundary boundary) {
	m_boundary = boundary;
	m_body = nullptr;
	m_Q1 = nullptr;
	m_Q2 = nullptr;
	m_Q3 = nullptr;
	m_Q4 = nullptr;
}

bool QuadTree::insert(GravBody* bodyToInsert) {
	//Invalid position for this quadrant
	if (!m_boundary.containsPoint(bodyToInsert->getPosition())) return false;

	// Has not subdivided yet
	if (m_Q1 == nullptr && m_body == nullptr) {
		// Insert the point to this body
		m_body = bodyToInsert;
	}
	else {
		// Check if we need to subdivide
		if (m_Q1 == nullptr) {
			
			// Discard duplicates to avoid infinite recursion (floating point numbers should take care of this for now)
			if (m_body->getPosition() == bodyToInsert->getPosition()) return false;
			
			// Subdivide and reinsert body belonging to this tree
			subdivide();
			insert(m_body);
			m_body = nullptr;

		}

		// Now insert the body to insert
		return (m_Q1->insert(bodyToInsert) || 
				m_Q2->insert(bodyToInsert) || 
				m_Q3->insert(bodyToInsert) || 
				m_Q4->insert(bodyToInsert)
			   );

	}

}

void QuadTree::subdivide() {
	glm::vec2 subdividedDimensions = m_boundary.getDimensions() / 2;
	glm::vec2 m_position = m_boundary.getPosition();
	glm::vec2 centerOfSubdivision = m_boundary.getPosition() + subdividedDimensions;

	m_Q1 = new QuadTree(Boundary(centerOfSubdivision, subdividedDimensions));
	m_Q2 = new QuadTree(Boundary(glm::vec2(m_position.x, centerOfSubdivision.y), subdividedDimensions));
	m_Q3 = new QuadTree(Boundary(m_boundary.getPosition(), subdividedDimensions));
	m_Q4 = new QuadTree(Boundary(glm::vec2(centerOfSubdivision.x, m_position.y), subdividedDimensions));

}



