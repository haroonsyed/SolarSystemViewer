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

// Align multi tree cell to 16 byte boundary
const unsigned int alignmentOffsetSingle = (16 - (sizeof(Body) + sizeof(glm::vec4) + 2 * sizeof(GLint) + sizeof(GLfloat)) % 16) / 4.0;
struct TreeCell {
	Body bodies[1];
	glm::vec4 COM;
	GLint lock;		// Is used to indicate lock and state of the cell
	// -1: Unlocked
	// -2: Not leaf node (continue traversal)
	// natural number: locked
	GLint numberOfBodies;
	GLfloat mass;
	GLint align[alignmentOffsetSingle];
};

const unsigned int numOfBodies = 3;
// Align multi tree cell to 16 byte boundary
const unsigned int alignmentOffset = (16 - (sizeof(Body) * numOfBodies + sizeof(glm::vec4) + 2 * sizeof(GLint) + sizeof(GLfloat)) % 16) / 4.0;
struct TreeCellMultiBody {
	Body bodies[numOfBodies];
	glm::vec4 COM;
	GLint lock;		// Is used to indicate lock and state of the cell
					// -1: Unlocked
					// -2: Not leaf node (continue traversal)
                    // natural number: locked
	GLint numberOfBodies;
	GLfloat mass;
	GLint align[alignmentOffset];
};
