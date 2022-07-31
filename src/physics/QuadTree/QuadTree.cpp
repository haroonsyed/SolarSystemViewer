#include "QuadTree.h"

QuadTree::~QuadTree() {
	// Delete all the aggregate bodies (non-leaf nodes)
	if (m_body != nullptr && m_Q1 != nullptr) {
		delete m_body;
		delete m_Q1;
		delete m_Q2;
		delete m_Q3;
		delete m_Q4;
	}
}

QuadTree::QuadTree(Boundary& boundary) {
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
		return true;
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
	glm::vec2 subdividedDimensions = m_boundary.getDimensions() / 2.0f;
	glm::vec2 m_position = m_boundary.getPosition();
	glm::vec2 centerOfSubdivision = m_boundary.getPosition() + subdividedDimensions;

	Boundary boundQ1(centerOfSubdivision, subdividedDimensions);
	Boundary boundQ2(glm::vec2(m_position.x, centerOfSubdivision.y), subdividedDimensions);
	Boundary boundQ3(m_boundary.getPosition(), subdividedDimensions);
	Boundary boundQ4(glm::vec2(centerOfSubdivision.x, m_position.y), subdividedDimensions);
	m_Q1 = new QuadTree(boundQ1);
	m_Q2 = new QuadTree(boundQ2);
	m_Q3 = new QuadTree(boundQ3);
	m_Q4 = new QuadTree(boundQ4);

}

std::vector<GravBody*> QuadTree::query(Boundary& range) {
	std::vector<GravBody*> result;
	return query(range, result);
}

// Gets all gravBodies in the range (ignores aggregate bodies)
std::vector<GravBody*> QuadTree::query(Boundary& range, std::vector<GravBody*>& result) {
	if (range.overlapsBoundary(m_boundary)) {
		if ((m_body != nullptr && m_Q1 == nullptr) && (range.containsPoint(m_body->getPosition())) ) {
			result.push_back(m_body);
		}
		
		// Go throoughg subdivisions
		if (m_Q1 != nullptr) {
			auto bodiesQ1 = m_Q1->query(range, result);
			auto bodiesQ2 = m_Q2->query(range, result);
			auto bodiesQ3 = m_Q3->query(range, result);
			auto bodiesQ4 = m_Q4->query(range, result);
		}

	}
	return result;
}


GravBody* QuadTree::aggregateCenterAndTotalMass() {
	// Set this m_body to a new body with total mass and COM of children bodies
	if(m_body == nullptr) {
		if(m_Q1 != nullptr) {

			// Set this body to an empty one with mass 0 as default
			m_body = new GravBody();
			m_body->setMass(0.0);
			m_body->setPosition(glm::vec3(0.0));

			// Traverse through the subdivision
			GravBody* aggQ1 = m_Q1->aggregateCenterAndTotalMass();
			GravBody* aggQ2 = m_Q2->aggregateCenterAndTotalMass();
			GravBody* aggQ3 = m_Q3->aggregateCenterAndTotalMass();
			GravBody* aggQ4 = m_Q4->aggregateCenterAndTotalMass();

			std::vector<GravBody*> nonNullBodies;
			if (aggQ1 != nullptr) nonNullBodies.push_back(aggQ1);
			if (aggQ2 != nullptr) nonNullBodies.push_back(aggQ2);
			if (aggQ3 != nullptr) nonNullBodies.push_back(aggQ3);
			if (aggQ4 != nullptr) nonNullBodies.push_back(aggQ4);

			float mass = 0.0;
			glm::vec2 centerOfMass = glm::vec2(0.0);

			for (auto body : nonNullBodies) {

				const float bodyMass = body->getMass();
				mass += bodyMass;
				centerOfMass += glm::vec2(body->getPosition() * bodyMass);

			}
			centerOfMass /= mass;

			m_body->setMass(mass);
			m_body->setPosition(glm::vec3(centerOfMass, 0.0f));

		}
	}
	return m_body;
}

std::vector<GravBody*> QuadTree::barnesHutQuery(GravBody* body, float theta) {
	std::vector<GravBody*> result;
	barnesHutQuery(body, theta, result);
	return result;
}

void QuadTree::barnesHutQuery(GravBody* body, float theta, std::vector<GravBody*>& result) {
	
	// At leaf node
	if (m_Q1 == nullptr) {
		// Check if leaf node has a value
		if (m_body != nullptr) {
			result.push_back(m_body);
		};
		return;
	}
	
	// Decide if we should go further in tree based on theta=width/COM
	glm::vec2 COM = m_body->getPosition();
	glm::vec2 bodyPosition = glm::vec2(body->getPosition());
	float distanceToCenterOfCell = glm::length(COM - bodyPosition);
	float cellWidth = m_boundary.getDimensions().x;
	float thisTheta = cellWidth/distanceToCenterOfCell;

	if (thisTheta < theta) {
		// Return the aggregate body
		result.push_back(m_body);
	}
	else {
		m_Q1->barnesHutQuery(body, theta, result);
		m_Q2->barnesHutQuery(body, theta, result);
		m_Q3->barnesHutQuery(body, theta, result);
		m_Q4->barnesHutQuery(body, theta, result);
	}
}

