#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>

struct Body {
	glm::vec4 position;
	glm::vec4 velocity;
	GLfloat mass;
	GLint align1;
	GLint align2;
	GLint align3;
};

struct TreeCell {
	glm::vec4 position;
	glm::vec4 velocity;
	GLfloat mass;
	GLint lock; // Is used to indicate lock and state of the cell
					// -1: Unlocked/null (insert body here)
					// -2: Locked (try again)
					// pos #: Non-leaf node (Continue traversal)
	GLint align1;
	GLint align2;
};

const unsigned int numOfBodies = 5;
const unsigned int alignmentOffset = (16 - (sizeof(Body) * numOfBodies + 2 * sizeof(GLint) + sizeof(GLfloat) + sizeof(glm::vec4)) % 16) / 4.0;
struct TreeCellMultiBody {
	Body bodies[numOfBodies];
	glm::vec4 COM;
	GLint lock;		// Is used to indicate lock and state of the cell
					// -1: Unlocked/null (insert body here)
					// -2: Locked (try again)
					// pos #: Non-leaf node (Continue traversal)
	GLint numberOfBodies;
	GLfloat mass;
	GLint align[alignmentOffset];
};
