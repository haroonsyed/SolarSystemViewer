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

const unsigned int numOfBodies = 20;
const unsigned int alignmentOffset = (16 - (sizeof(Body) * numOfBodies + sizeof(GLint))%16)/4.0;
struct TreeCellMultiBody {
	Body bodies[numOfBodies];
	GLint lock;		// Is used to indicate lock and state of the cell
					// -1: Unlocked/null (insert body here)
					// -2: Locked (try again)
					// pos #: Non-leaf node (Continue traversal)
	GLint align[alignmentOffset];
};

// These are adjusted for std:430 alignment
const unsigned int sizeOfBody = sizeof(Body);
const unsigned int sizeOfTreeCell = sizeof(TreeCell);
const unsigned int sizeOfTreeCellMultiBody = sizeof(TreeCellMultiBody);
