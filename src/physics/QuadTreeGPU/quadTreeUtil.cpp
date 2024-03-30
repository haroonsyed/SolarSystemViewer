#include "quadTreeStructs.h"
#include <math.h>

class QuadTreeUtil {

public: 
	std::string printBody(Body& body) {
		return ("BODY_POSITION: " + std::to_string(body.position.x) + " " + std::to_string(body.position.y) + " " + std::to_string(body.position.z) + "\n"
			+ "BODY_VELOCITY: " + std::to_string(body.velocity.x) + " " + std::to_string(body.velocity.y) + " " + std::to_string(body.velocity.z) + "\n"
			+ "BODY_MASS: " + std::to_string(body.mass) + "\n\n");
	}

	std::string printTreeCell(TreeCell& cell) {
		return (printBody(cell.body)
			+ "CELL_MASS: " + std::to_string(cell.mass) + "\n"
			+ "CELL_COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
			+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
	}

	//std::string printTreeCellMultiBody(TreeCellMultiBody& cell) {
	//	return ("COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
	//		+ "MASS: " + std::to_string(cell.mass) + "\n"
	//		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
	//}

};
