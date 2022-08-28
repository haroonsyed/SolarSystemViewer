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
	return ("POSITION: " + std::to_string(cell.position.x) + " " + std::to_string(cell.position.y) + " " + std::to_string(cell.position.z) + "\n"
		+ "VELOCITY: " + std::to_string(cell.velocity.x) + " " + std::to_string(cell.velocity.y) + " " + std::to_string(cell.velocity.z) + "\n"
		+ "MASS: " + std::to_string(cell.mass) + "\n"
		+ "LOCK: " + std::to_string(cell.lock) + "\n\n");
}

template <typename T>
bool aboutEqualsVector(T a, T b) {
	return glm::all( glm::epsilonEqual(a, b, epsilon) );
}

bool aboutEqualsFloat(float a, float b) {
	return abs(a-b) < epsilon;
}

void testTreesAreEqual(std::vector<TreeCell>& tree, std::vector<TreeCell>& expected) {
	REQUIRE(tree.size() == expected.size());
	for (int i = 0; i < tree.size(); i++) {
		TreeCell cell = tree[i];
		TreeCell expectedCell = expected[i];
		REQUIRE(cell.lock == expectedCell.lock);
		REQUIRE(aboutEqualsVector(glm::vec2(cell.position), glm::vec2(expectedCell.position)));
		REQUIRE(aboutEqualsVector(glm::vec2(cell.velocity), glm::vec2(expectedCell.velocity)));
		REQUIRE(aboutEqualsFloat(cell.mass, expectedCell.mass));
	}
}

void testLeavesAreEqual(std::vector<TreeCell>& tree, std::vector<TreeCell>& expected) {
	REQUIRE(tree.size() == expected.size());
	for (int i = 0; i < tree.size(); i++) {
		TreeCell cell = tree[i];
		TreeCell expectedCell = expected[i];
		INFO("INDEX: " + std::to_string(i));
		REQUIRE(cell.lock == expectedCell.lock);
		if (expectedCell.lock == -1) {
			REQUIRE(aboutEqualsVector(glm::vec2(cell.position), glm::vec2(expectedCell.position)));
			REQUIRE(aboutEqualsVector(glm::vec2(cell.velocity), glm::vec2(expectedCell.velocity)));
			REQUIRE(aboutEqualsFloat(cell.mass, expectedCell.mass));
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

	REQUIRE(window != nullptr);
}

TEST_CASE("Test clearing of tree") {
	const unsigned int treeSize = 100;

	// Create SSBO_BODIES
	glGenBuffers(1, &SSBO_BODIES);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(treeSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	std::vector<TreeCell> tree(treeSize);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeOfTreeCell * treeSize, &tree[0]);

	for (const auto& cell : tree) {
		REQUIRE(cell.mass == -1);
		REQUIRE(cell.lock == -1);
	}

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Test creation of tree. Single body.") {
	
	for (int i = 0; i < 50; i++) {

		const unsigned int treeSize = 5;

		// Create input data
		std::vector<Body> bodies{
			Body{glm::vec4(5.0f), glm::vec4(7.0f), 51.0f}
		};

		// Create SSBO_BODIES
		glGenBuffers(1, &SSBO_BODIES);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * treeSize, &bodies[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);

		// Create SSBO_TREE
		glGenBuffers(1, &SSBO_TREE);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

		ShaderManager* shaderManager = ShaderManager::getInstance();
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
		glDispatchCompute(treeSize, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Check results
		std::vector<TreeCell> tree(treeSize);
		glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
		std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

		testLeavesAreEqual(tree, expected);

		glDeleteBuffers(1, &SSBO_BODIES);
		glDeleteBuffers(1, &SSBO_TREE);
	}


}

TEST_CASE("Test creation of tree. 2 bodies, different quadrants.") {

	for (int i = 0; i < 100; i++) {

		const unsigned int treeSize = 5;

		// Create input data
		std::vector<Body> bodies{
			Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
			Body{glm::vec4(-1.0f), glm::vec4(3.0f), 51.0f},
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
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
		glDispatchCompute(treeSize, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Check results
		std::vector<TreeCell> tree(treeSize);
		glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
		std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

		testLeavesAreEqual(tree, expected);

		glDeleteBuffers(1, &SSBO_BODIES);
		glDeleteBuffers(1, &SSBO_TREE);

	}

}

TEST_CASE("Test creation of tree. 4 bodies, different quadrants.") {

	for (int i = 0; i < 100; i++) {
		const unsigned int treeSize = 100;

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
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
		glDispatchCompute(treeSize, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Check results
		std::vector<TreeCell> tree(treeSize);
		glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
		std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

		testLeavesAreEqual(tree, expected);

		glDeleteBuffers(1, &SSBO_BODIES);
		glDeleteBuffers(1, &SSBO_TREE);

	}

}

TEST_CASE("Insertion of two bodies, same quadrant (one level nested deep).") {
	const unsigned int treeSize = 100;

	// Create input data
	std::vector<Body> bodies{
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(1 + 1e10/2), glm::vec4(3.0f), 51.0f},
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
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(treeSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

	testLeavesAreEqual(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Insertion of two bodies, same exact position. (Get rid of one)") {
	const unsigned int treeSize = 100;

	// Create input data
	std::vector<Body> bodies{
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
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
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(treeSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

	testLeavesAreEqual(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Insertion of 10000 random bodies.") {
	const unsigned int treeSize = 25000;

	// Create input data
	std::vector<Body> bodies(1000);
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

	glFinish();
	double startTime = glfwGetTime();
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(treeSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	std::cout << "Time to calculate physics: " << (glfwGetTime() - startTime) << std::endl;

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	startTime = glfwGetTime();
	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
	std::cout << "Time to calculate on CPU: " << (glfwGetTime() - startTime) << std::endl;

	std::string msg = "";
	/*for (int i = 0; i < tree.size(); i++) {
		msg += "INDEX: " + std::to_string(i) + "\n";
		msg += printTreeCell(tree[i]);
	}
	for (int i = 0; i < expected.size(); i++) {
		msg += "INDEX: " + std::to_string(i) + "\n";
		msg += printTreeCell(expected[i]);
	}*/

	// Count number of bodies in each
	int count = 0;
	for (int i = 0; i < tree.size(); i++) {
		if (tree[i].mass > 0.0 && tree[i].lock == -1) {
			count++;
		}
	}
	msg += "\n\nLEAF COUNT IN GPU_COMPUTE: " + std::to_string(count);
	count = 0;
	for (int i = 0; i < expected.size(); i++) {
		if (expected[i].mass > 0.0 && expected[i].lock == -1) {
			count++;
		}
	}
	msg += "\nLEAF COUNT IN CPU_COMPUTE: " + std::to_string(count);

	std::cout << msg << std::endl;
	//testLeavesAreEqual(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Clear multi-tree cell.") {
	const unsigned int treeSize = 100;

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCellMultiBody * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	double startTime = glfwGetTime();
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCellMultiBody> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);
	for (const auto& cell : tree) {
		REQUIRE(cell.numberOfBodies == 0);
		REQUIRE(cell.lock == -1);
	}

	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("No nesting fill parent cell.") {
	const unsigned int treeSize = 1000;

	// Create input data
	std::vector<Body> bodies(numOfBodies);
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

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
	glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	std::cout << sizeOfBody << " " << sizeOfTreeCell << " " << sizeOfTreeCellMultiBody << std::endl;

	// Check results
	std::vector<TreeCellMultiBody> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);

	std::string msg = "";

	// Count number of bodies in each
	int count = 0;
	for (int i = 0; i < tree.size(); i++) {
		if (tree[i].lock == -1) {
			count+=tree[i].numberOfBodies;
		}
	}
	msg += "\n\nLEAF COUNT IN GPU_COMPUTE: " + std::to_string(count);

	std::cout << msg << std::endl;
	REQUIRE(count == bodies.size());

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

	std::string msg = "";

	// Count number of bodies in each
	int count = 0;
	for (int i = 0; i < tree.size(); i++) {
		if (tree[i].lock == -1) {
			count += tree[i].numberOfBodies;
		}
	}
	msg += "\n\nLEAF COUNT IN GPU_COMPUTE: " + std::to_string(count);

	std::cout << msg << std::endl;
	// Okay if a couple bodies are too nested (say they are flung out of tree)
	REQUIRE((count >= 0.97 * bodies.size() && count <= bodies.size()));

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Test aggregation to non-leaf cells of COM and total mass.") {

	const unsigned int numberOfLevelsInTree = 11;
	const unsigned int treeSize = sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
	clearGLErrors();

	// Create input data
	std::vector<Body> bodies(1000000);
	for (int i = 0; i < bodies.size(); i++) {
		bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), 1.0f };
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

	double startTime = glfwGetTime();
	for (int i = 0; i < 100; i++) {
		startTime = glfwGetTime();
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree_multi.comp");
		glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree_multi.comp");
		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree_multi.comp");
		unsigned int levelStartLoc = glGetUniformLocation(shaderManager->getBoundShader(), "levelStart");
		for (int i = numberOfLevelsInTree - 1; i >= 0; i--) {
			// Set level
			glUniform1ui(levelStartLoc, startPositionOfLevel(i));

			// Dispatch compute for that level of tree
			glDispatchCompute(ceil(numberOfCellsInLevel(i) / 32.0), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		glFinish();
		std::cout << "Time to clear, build and aggregate tree: " << (glfwGetTime() - startTime) << std::endl;

	}
	

	printErrors();

	// Check results
	std::vector<TreeCellMultiBody> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCellMultiBody * treeSize, &tree[0]);
	glFinish();

	//Check size and COM
	double totalMass = 0.0;
	glm::vec4 centerOfMass = glm::vec4(0.0);
	for (const auto& cell : tree) {
		if (cell.lock == -1) {
			for (int i = 0; i < cell.numberOfBodies; i++) {
				Body body = cell.bodies[i];
				totalMass += body.mass;
				centerOfMass += body.mass * body.position;
			}
		}
	}
	centerOfMass /= totalMass;

	// Okay if a couple bodies are too nested (say they are flung out of tree)
	REQUIRE(totalMass == tree[0].mass);
	REQUIRE(aboutEqualsVector(centerOfMass, tree[0].COM));

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}




