#pragma once
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include "quadTreeStructs.h"
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

std::string printTreeCell(TreeCell& cell) {
	return ("POSITION: " + std::to_string(cell.position.x) + " " + std::to_string(cell.position.y) + " " + std::to_string(cell.position.z) + "\n"
		+ "VELOCITY: " + std::to_string(cell.velocity.x) + " " + std::to_string(cell.velocity.y) + " " + std::to_string(cell.velocity.z) + "\n"
		+ "MASS:" + std::to_string(cell.mass) + "\n"
		+ "CHILD_INDEX" + std::to_string(cell.childIndex) + "\n");
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
		REQUIRE(cell.childIndex == expectedCell.childIndex);
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
		INFO(printTreeCell(cell));
		INFO(printTreeCell(expectedCell));
		REQUIRE(cell.childIndex == expectedCell.childIndex);
		if (expectedCell.childIndex == -1) {
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
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Check results
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	std::vector<TreeCell> tree(treeSize);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeOfTreeCell * treeSize, &tree[0]);

	for (const auto& cell : tree) {
		REQUIRE(cell.mass == -1);
		REQUIRE(cell.childIndex == -1);
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
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

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
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

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
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
		unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
		glUniform1ui(treeSizeLoc, treeSize);
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

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
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

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
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

	testLeavesAreEqual(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}

TEST_CASE("Insertion of 1000 random bodies.") {
	const unsigned int treeSize = 10000;

	// Create input data
	std::vector<Body> bodies(15);
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
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(treeSize, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, 1);
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);

	testLeavesAreEqual(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);

}


