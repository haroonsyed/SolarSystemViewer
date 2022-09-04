#pragma once
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include "quadTreeStructs.h"
#include "quadTreeUtil.h"
#include <glm/glm.hpp>
#include "../graphics/shader/shaderManager.h"
#include "../physics/QuadTree/QuadTree.h"
#include <random>


glm::vec2 boundStart = glm::vec2(-1e10, -1e10);
glm::vec2 boundRange = abs(boundStart * 2.0f);
Boundary boundary(boundStart, boundRange);

// Random num generation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dist(boundStart.x, abs(boundStart.x));

unsigned int SSBO_BODIES;
unsigned int SSBO_TREE;

const GLfloat epsilon = 1e-2;

void clearGLErrors() {
	while (glGetError() != GL_NO_ERROR);
}

void printErrors() {
	GLenum error = glGetError();
	while (error != GL_NO_ERROR) {
		std::cout << "OGL_ERROR: " << error << std::endl;
		error = glGetError();
	}
}

std::string printTreeCell(TreeCell& cell) {
	return ("BODY_POSITION: " + std::to_string(cell.bodies[0].position.x) + " " + std::to_string(cell.bodies[0].position.y) + " " + std::to_string(cell.bodies[0].position.z) + "\n"
		+ "BODY_VELOCITY: " + std::to_string(cell.bodies[0].velocity.x) + " " + std::to_string(cell.bodies[0].velocity.y) + " " + std::to_string(cell.bodies[0].velocity.z) + "\n"
		+ "CELL_MASS: " + std::to_string(cell.mass) + "\n"
		+ "CELL_COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
}

std::string printBody(Body& body) {
	return ("POSITION: " + std::to_string(body.position.x) + " " + std::to_string(body.position.y) + " " + std::to_string(body.position.z) + "\n"
		+ "VELOCITY: " + std::to_string(body.velocity.x) + " " + std::to_string(body.velocity.y) + " " + std::to_string(body.velocity.z) + "\n"
		+ "MASS: " + std::to_string(body.mass) + "\n");
}

std::string printTreeCellMultiBody(TreeCellMultiBody& cell) {
	return ("COM: " + std::to_string(cell.COM.x) + " " + std::to_string(cell.COM.y) + " " + std::to_string(cell.COM.z) + "\n"
		+ "MASS: " + std::to_string(cell.mass) + "\n"
		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
}

template <typename T>
bool aboutEqualsVector(T a, T b) {
	return glm::all( glm::epsilonEqual(a, b, epsilon) );
}

template <typename T>
bool aboutEqualsVector(T a, T b, float epsilon) {
	return glm::all(glm::epsilonEqual(a, b, epsilon ));
}

bool aboutEqualsFloat(float a, float b) {
	return abs(a-b) < epsilon;
}

bool aboutEqualsFloat(float a, float b, float epsilon) {
	return abs(a - b) < epsilon;
}

void testTreesAreEqualSingle(std::vector<TreeCell>& tree, std::vector<TreeCell>& expected) {
	REQUIRE(tree.size() == expected.size());
	for (int i = tree.size() - 1; i >= 0; i--) {
		TreeCell cell = tree[i];
		TreeCell expectedCell = expected[i];
		INFO(std::to_string(i));
		REQUIRE(cell.lock == expectedCell.lock);
		if (cell.lock == -1 && expectedCell.bodies[0].mass > 0.0) {
			INFO("COMPUTED: " + printBody(cell.bodies[0]));
			INFO("EXPECTED: " + printTreeCell(expectedCell));
			REQUIRE(aboutEqualsVector(glm::vec2(cell.bodies[0].position), glm::vec2(expectedCell.bodies[0].position), abs(min(cell.bodies[0].position.x, expectedCell.bodies[0].position.y)) * 0.01));
			REQUIRE(aboutEqualsFloat(cell.bodies[0].mass, expectedCell.bodies[0].mass, expectedCell.bodies[0].mass * 0.01));
		}
	}
}

void testLeavesAreEqualSingle(std::vector<TreeCell>& tree, std::vector<TreeCell>& expected) {
	REQUIRE(tree.size() == expected.size());
	for (int i = tree.size() - 1; i >= 0; i--) {
		TreeCell cell = tree[i];
		TreeCell expectedCell = expected[i];
		INFO(std::to_string(i));
		REQUIRE(cell.lock == expectedCell.lock);
		if (cell.lock == -1 && expectedCell.bodies[0].mass > 0.0) {
			INFO("COMPUTED: " + printBody(cell.bodies[0]));
			INFO("EXPECTED: " + printTreeCell(expectedCell));
			REQUIRE(aboutEqualsVector(glm::vec2(cell.bodies[0].position), glm::vec2(expectedCell.bodies[0].position), abs(min(cell.bodies[0].position.x, expectedCell.bodies[0].position.y)) * 0.01));
			REQUIRE(aboutEqualsFloat(cell.bodies[0].mass, expectedCell.bodies[0].mass, expectedCell.bodies[0].mass * 0.01));
		}
	}
}

GravBody* getGravBodyFromBody(Body body) {
	GravBody* gravBody = new GravBody();
	gravBody->setPosition(glm::vec3(body.position));
	gravBody->setVelocity(glm::vec3(body.velocity));
	gravBody->setMass(body.mass);
	return gravBody;
}

std::vector<TreeCell> createExpectedFromBodies(std::vector<Body>& bodies, int treeSize) {
	QuadTree root(boundary);

	for (const auto& body : bodies) {
		GravBody* gravBody = getGravBodyFromBody(body);
		root.insert(gravBody);
	}

	root.aggregateCenterAndTotalMass();

	return root.convertQuadTreeObjectToArray(&root, treeSize);
}

TEST_CASE("INIT_TESTS") {
	// Create Context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1, 1, "viewGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	glViewport(0, 0, 100, 100);

	// glew: load all OpenGL function pointers
	glewInit();

	// Clear errors
	clearGLErrors();
	std::cout << sizeOfBody << " " << sizeOfTreeCell << " " << sizeOfTreeCellMultiBody << std::endl;

	REQUIRE(window != nullptr);
}

TEST_CASE("Clear single-body-cell tree.") {
	const unsigned int treeSize = 100;

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	double startTime = glfwGetTime();
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	for (const auto& cell : tree) {
		REQUIRE(cell.mass == 0);
		REQUIRE(aboutEqualsVector(cell.COM, glm::vec4(0.0)));
		REQUIRE(cell.numberOfBodies == 0);
		REQUIRE(cell.lock == -1);
	}

	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Place single body in parent single-body-cell.") {
	const unsigned int treeSize = 5;

	// Create input data
	std::vector<Body> bodies(1);
	for (int i = 0; i < bodies.size(); i++) {
		bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), 51.0f };
	}

	// Create SSBO_BODIES
	glGenBuffers(1, &SSBO_BODIES);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);

	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
	testTreesAreEqualSingle(tree, expected);
	
	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Place 4 bodies different quadrants single-body-cell.") {
	const unsigned int treeSize = 5;

	// Create input data
	std::vector<Body> bodies{
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(1.0f, -1.0f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f},
	};

	// Create SSBO_BODIES
	glGenBuffers(1, &SSBO_BODIES);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);

	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
	testLeavesAreEqualSingle(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Place 2 bodies nested quadrant single-body-cell.") {
	const unsigned int numberOfLevelsInTree = 3;
	const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
	
	// Create input data
	std::vector<Body> bodies{
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q3
		Body{glm::vec4(6e9f, 6e9f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q1
	};

	// Create SSBO_BODIES
	glGenBuffers(1, &SSBO_BODIES);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);

	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
	testLeavesAreEqualSingle(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Random body 1 million multi-tree cell. Also ensures no random runtime error.") {

	const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(11);
	clearGLErrors();

	// Create input data
	std::vector<Body> bodies(1000000);
	for (int i = 0; i < bodies.size(); i++) {
		bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), 51.0f };
	}

	// Create SSBO_BODIES
	glGenBuffers(1, &SSBO_BODIES);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCellMultiBody * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	printErrors();

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(bodySizeLoc, bodies.size());

	for (int i = 0; i < 100; i++) {

		double startTime = glfwGetTime();
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
		glDispatchCompute(ceil(treeSize/ 32.0), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glFinish();
		//std::cout << "Time to calculate clear tree (MULTI): " << (glfwGetTime() - startTime) << std::endl;
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glFinish();
		std::cout << "Time to calculate physics (MULTI) (including clearing): " << (glfwGetTime() - startTime) << std::endl;
		printErrors();

	}
	std::cout << sizeOfBody << " " << sizeOfTreeCell << " " << sizeOfTreeCellMultiBody << std::endl;

	// Check results
	std::vector<TreeCellMultiBody> tree(treeSize);
	double startTime = glfwGetTime();
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);
	glFinish();
	std::cout << "Time to download results: " << (glfwGetTime() - startTime) << std::endl;

	// Check result
	int count = 0;
	for (const auto& cell : tree) {
		for (const auto& body : cell.bodies) {
			if (body.mass > 0.0 && cell.lock == -1) {
				count++;
			}
		}
	}
	std::cout << "NUMBER OF LEAVES IN TREE: " << count << std::endl;

	REQUIRE(count >= bodies.size() * 0.97);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

//TEST_CASE("Test aggregation of total COM and total mass.") {
//
//	const unsigned int numberOfLevelsInTree = 11;
//	const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
//	clearGLErrors();
//
//	// Create input data
//	std::vector<Body> bodies(10000);
//	for (int i = 0; i < bodies.size(); i++) {
//		bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), (float)abs(dist(gen)) };
//	}
//
//	// Create SSBO_BODIES
//	glGenBuffers(1, &SSBO_BODIES);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);
//
//	// Create SSBO_TREE
//	glGenBuffers(1, &SSBO_TREE);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCellMultiBody * treeSize, nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);
//
//	printErrors();
//
//	ShaderManager* shaderManager = ShaderManager::getInstance();
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
//	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//	glUniform1ui(bodySizeLoc, bodies.size());
//
//	double startTime = glfwGetTime();
//	for (int i = 0; i < 100; i++) {
//		startTime = glfwGetTime();
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
//		glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
//		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree_multi.comp");
//		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//		glUniform1ui(treeSizeLoc, treeSize);
//		unsigned int levelStartLoc = glGetUniformLocation(shaderManager->getBoundShader(), "levelStart");
//		for (int i = numberOfLevelsInTree - 1; i >= 1; i--) {
//			// Set level
//			glUniform1ui(levelStartLoc, startPositionOfLevel(i));
//
//			// Dispatch compute for that level of tree
//			int numberOfParentCells = numberOfCellsInLevel(i - 1);
//			glDispatchCompute(numberOfParentCells, 1, 1);
//			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		}
//		glFinish();
//		std::cout << "Time to clear, build and aggregate tree: " << (glfwGetTime() - startTime) << std::endl;
//
//	}
//	
//
//	printErrors();
//
//	// Check results
//	std::vector<TreeCellMultiBody> tree(treeSize);
//	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);
//	glFinish();
//
//	//Check size and COM
//	float totalMass = 0.0;
//	glm::vec4 centerOfMass = glm::vec4(0.0);
//	for (const auto& cell : tree) {
//		if (cell.lock == -1) {
//			for (int i = 0; i < cell.numberOfBodies; i++) {
//				Body body = cell.bodies[i];
//				totalMass += body.mass;
//				centerOfMass += body.mass * body.position;
//			}
//		}
//	}
//	centerOfMass /= totalMass;
//
//	std::string info = "Computed mass: " + std::to_string(tree[0].mass);
//	info += "\nComputed COM: " + std::to_string(tree[0].COM.x) + " " + std::to_string(tree[0].COM.y);
//	info += "\nExpected mass: " + std::to_string(totalMass);
//	info += "\nExpected COM: " + std::to_string(centerOfMass.x) + " " + std::to_string(centerOfMass.y);
//	INFO(info);
//	REQUIRE(aboutEqualsFloat(totalMass, tree[0].mass, totalMass * 0.01));
//	REQUIRE(aboutEqualsVector(centerOfMass, tree[0].COM, abs(min(centerOfMass.x, centerOfMass.y)) * 0.01));
//
//	glDeleteBuffers(1, &SSBO_BODIES);
//	glDeleteBuffers(1, &SSBO_TREE);
//
//}
//
//TEST_CASE("Test aggregation to non-leaf cells of COM and total mass and accuracy.") {
//
//	const unsigned int numberOfLevelsInTree = 3;
//	const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
//	clearGLErrors();
//
//	// Create input data
//	std::vector<Body> bodies(2);
//	for (int i = 0; i < bodies.size(); i++) {
//		bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), (float)abs(dist(gen)) };
//	}
//
//	// Create SSBO_BODIES
//	glGenBuffers(1, &SSBO_BODIES);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);
//
//	// Create SSBO_TREE
//	glGenBuffers(1, &SSBO_TREE);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCellMultiBody * treeSize, nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);
//
//	printErrors();
//
//	ShaderManager* shaderManager = ShaderManager::getInstance();
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
//	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//	glUniform1ui(bodySizeLoc, bodies.size());
//
//	double startTime = glfwGetTime();
//	for (int i = 0; i < 100; i++) {
//		startTime = glfwGetTime();
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
//		glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
//		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree_multi.comp");
//		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//		glUniform1ui(treeSizeLoc, treeSize);
//		unsigned int levelStartLoc = glGetUniformLocation(shaderManager->getBoundShader(), "levelStart");
//		for (int i = numberOfLevelsInTree - 1; i >= 1; i--) {
//			// Set level
//			glUniform1ui(levelStartLoc, startPositionOfLevel(i));
//
//			// Dispatch compute for that level of tree
//			int numberOfParentCells = numberOfCellsInLevel(i - 1);
//			glDispatchCompute(numberOfParentCells, 1, 1);
//			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		}
//		glFinish();
//		std::cout << "Time to clear, build and aggregate tree: " << (glfwGetTime() - startTime) << std::endl;
//
//	}
//
//
//	printErrors();
//
//	// Check results
//	std::vector<TreeCellMultiBody> tree(treeSize);
//	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);
//	glFinish();
//
//	//Check with expected
//	startTime = glfwGetTime();
//	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
//	std::cout << "Time to calculate on CPU: " << (glfwGetTime() - startTime) << std::endl;
//
//	testTreesAreEqualMultiSingle(tree, expected);
//
//	glDeleteBuffers(1, &SSBO_BODIES);
//	glDeleteBuffers(1, &SSBO_TREE);
//	
//}




