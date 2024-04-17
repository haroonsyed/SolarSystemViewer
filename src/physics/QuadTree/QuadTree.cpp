#include "QuadTree.h"
#include <iostream>

QuadTree::~QuadTree() {
	// Delete all the aggregate bodies (non-leaf nodes)
	if (m_body != nullptr) {
		  delete m_body;
	}

	if (m_Q1 != nullptr) {
		  delete m_Q1;
		  delete m_Q2;
		  delete m_Q3;
		  delete m_Q4;
	}
}

QuadTree::QuadTree(const Boundary& boundary) : m_boundary(boundary) {
	m_body = nullptr;
	m_Q1 = nullptr;
	m_Q2 = nullptr;
	m_Q3 = nullptr;
	m_Q4 = nullptr;
}

bool QuadTree::isLeaf() {
  return m_Q1 == nullptr;
}

bool QuadTree::isEmptyNode() {
  return m_body == nullptr;
}

QuadTree* QuadTree::quadrantPredictor(glm::vec2 bodyPosition, glm::vec2 boundPosition, glm::vec2 boundRange) {
  glm::vec2 center = boundPosition + boundRange * 0.5f;
  if (bodyPosition.x >= center.x) {
	if (bodyPosition.y >= center.y) {
	  return this->m_Q1;
	}
	else {
	  return this->m_Q4;
	}
  }
  else {
	if (bodyPosition.y >= center.y) {
	  return this->m_Q2;
	}
	else {
	  return this->m_Q3;
	}
  }
}

bool QuadTree::insert(GravBody* bodyToInsert) {
	//Invalid position for this quadrant
	//if (!m_boundary.containsPoint(bodyToInsert->getPosition())) return false;

	// Has not subdivided yet
	if (m_Q1 == nullptr && m_body == nullptr) {
		// Insert the point to this body
		m_body = bodyToInsert;
		return true;
	}
	else {
		// Check if we need to subdivide
		if (m_Q1 == nullptr) {
			
			if (m_body->getPosition() == bodyToInsert->getPosition()) {
			  std::cout << "Found duplicate while inserting, ignoring..." << std::endl;
			  return false;
			}
			
			// Subdivide and reinsert body belonging to this tree
			subdivide();
			insert(m_body);

			m_body = nullptr;
		}
		// Now insert the body to insert
		QuadTree* targetQuadrant = quadrantPredictor(glm::vec2(bodyToInsert->getPosition()), this->m_boundary.getPosition(), this->m_boundary.getDimensions());
		bool insertSuccessful = targetQuadrant->insert(bodyToInsert);
		//bool insertSuccessful = m_Q1->insert(bodyToInsert) ||
		//  m_Q2->insert(bodyToInsert) ||
		//  m_Q3->insert(bodyToInsert) ||
		//  m_Q4->insert(bodyToInsert);

		//if (!insertSuccessful) {
		//  bool inBoundingBox = m_boundary.containsPoint(bodyToInsert->getPosition());
		//  std::cout << "Could not insert body" << std::endl;
		//  std::cout << "Should have gone in quadrant " << std::to_string(quadrantPredictor(glm::vec2(bodyToInsert->getPosition()), this->m_boundary.getPosition(), this->m_boundary.getDimensions())) << std::endl;

		//  m_Q1->insert(bodyToInsert);
		//  m_Q2->insert(bodyToInsert);
		//  m_Q3->insert(bodyToInsert);
		//  m_Q4->insert(bodyToInsert);
		//}

		return insertSuccessful;
	}
}

void QuadTree::subdivide() {
	glm::vec2 subdividedDimensions = m_boundary.getDimensions() * 0.5f;
	glm::vec2 m_position = m_boundary.getPosition();
	glm::vec2 centerOfSubdivision = m_boundary.getPosition() + subdividedDimensions;

	Boundary boundQ1(centerOfSubdivision, subdividedDimensions);
	Boundary boundQ2(glm::vec2(m_position.x, centerOfSubdivision.y), subdividedDimensions);
	Boundary boundQ3(m_position, subdividedDimensions);
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