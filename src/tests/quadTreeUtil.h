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