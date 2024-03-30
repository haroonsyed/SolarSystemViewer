#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include "nlohmann/json.hpp"

struct alignas(16) Body {
	glm::vec4 position;
	glm::vec4 velocity;
	GLfloat mass;
	GLint id; // Should be position in bodies[] gpu buffer
	GLint align2;
	GLint align3;
};

// Align multi tree cell to 16 byte boundary because structures are aligned to their largest member
struct alignas(16) TreeCell {
	Body body;
	glm::vec4 COM;
	GLint numberOfBodies;
	GLfloat mass;
	GLint lock;		// Is used to indicate lock and state of the cell
					// -1: Unlocked
					// -2: Not leaf node (continue traversal)
					// lock == bodyID: locked by that body
	GLint childCell;// Indicates position in treebuffer where child can be found
					// -1 if there is no child yet and no cell (free to insert here)
					// if there is a particle but no children (you wil need to subdivide)
					// # pointing to index of subtree
};