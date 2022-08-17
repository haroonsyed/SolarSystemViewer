#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>

// These are adjusted for std:430 alignment
const unsigned int sizeOfBody = sizeof(GLfloat) * (4 + 4 + 4);
const unsigned int sizeOfTreeCell = sizeof(GLfloat) * (4 + 4 + 4);

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
	GLint childIndex; // Is used to indicate lock and state of the cell
					// -1: Unlocked/null (insert body here)
					// -2: Locked (try again)
					// pos #: Non-leaf node (Continue traversal)
	GLint align1;     // Simply used for alignment
	GLint align2;     // Simply used for alignment
};
