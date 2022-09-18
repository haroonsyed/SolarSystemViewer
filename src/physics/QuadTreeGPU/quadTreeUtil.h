#pragma once
#include <math.h>

// These are adjusted for std:430 alignment
const unsigned int sizeOfBody = sizeof(Body);
const unsigned int sizeOfTreeCell = sizeof(TreeCell);
const unsigned int sizeOfTreeCellMultiBody = sizeof(TreeCellMultiBody);

// Here input the number of levels (1 indexed)
unsigned int sizeOfTreeGivenNumberOfLevels(int level) {
	// From geometric sequence formula:
	/*
		for(int i=0; i<level; i++) {
			position += pow(4, i);
		}

		==

		S = a1 * ( (r^ (n -1) ) / (r - 1) )
		S = 1 * (4^(n-1))/3

	*/

	return (pow(4, level) - 1) / 3;
}

// Levels are zero-indexed
unsigned int startPositionOfLevel(int level) {
	return sizeOfTreeGivenNumberOfLevels(level);

}

// Levels are zero-indexed
unsigned int numberOfCellsInLevel(int level) {
	return pow(4, level);
}

std::string printBody(Body& body) {
	return ("BODY_POSITION: " + std::to_string(body.position.x) + " " + std::to_string(body.position.y) + " " + std::to_string(body.position.z) + "\n"
		+ "BODY_VELOCITY: " + std::to_string(body.velocity.x) + " " + std::to_string(body.velocity.y) + " " + std::to_string(body.velocity.z) + "\n"
		+ "BODY_MASS: " + std::to_string(body.mass) + "\n\n");
}

std::string printTreeCell(TreeCell& cell) {
	return (printBody(cell.bodies[0])
		+ "CELL_MASS: " + std::to_string(cell.mass) + "\n"
		+ "CELL_COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
}

std::string printTreeCellMultiBody(TreeCellMultiBody& cell) {
	return ("COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
		+ "MASS: " + std::to_string(cell.mass) + "\n"
		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
}