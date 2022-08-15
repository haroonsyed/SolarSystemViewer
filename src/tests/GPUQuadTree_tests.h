#pragma once
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <catch2/catch.hpp>
#include "../graphics/shader/shaderManager.h"
#include <iostream>
#include <GLFW/glfw3.h>

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

// These are adjusted for std:430 alignment
const unsigned int sizeOfBody = sizeof(float) * (4 + 4 + 4);
const unsigned int sizeOfTreeCell = sizeof(float) * (4 + 4 + 4);

unsigned int SSBO_BODIES;
unsigned int SSBO_TREE;


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
	const unsigned int treeSize = 1000;

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
		glDispatchCompute(bodies.size(), 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Check results
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
		std::vector<TreeCell> tree(treeSize);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeOfTreeCell * treeSize, &tree[0]);

		for (int i = 0; i < bodies.size(); i++) {
			Body body = bodies[i];
			TreeCell cell = tree[i];
			REQUIRE(cell.position == body.position);
			REQUIRE(cell.velocity == body.velocity);
			REQUIRE(cell.mass == body.mass);
		}

	}

}

TEST_CASE("Test creation of tree. 2 bodies.") {
	const unsigned int treeSize = 100;

	// Create input data
	std::vector<Body> bodies{
		Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f},
		Body{glm::vec4(-1.0f), glm::vec4(3.0f), 51.0f},
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
	glDispatchCompute(bodies.size(), 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Check results
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	std::vector<TreeCell> tree(treeSize);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeOfTreeCell * treeSize, &tree[0]);

	REQUIRE(tree[1].position == bodies[0].position);
	REQUIRE(tree[1].velocity == bodies[0].velocity);
	REQUIRE(tree[1].mass == bodies[0].mass);

	REQUIRE(tree[3].position == bodies[1].position);
	REQUIRE(tree[3].velocity == bodies[1].velocity);
	REQUIRE(tree[3].mass == bodies[1].mass);

}



