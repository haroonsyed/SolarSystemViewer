#pragma once
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <random>
#include <GLFW/glfw3.h>
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "../physics/QuadTree/QuadTree.h"
#include "../graphics/shader/shaderManager.h"
#include "../physics/QuadTreeGPU/quadTreeUtil.cpp"
#include "../physics/system.h"


glm::vec2 boundStart = glm::vec2(-1e10, -1e10);
glm::vec2 boundRange = abs(boundStart * 2.0f);
Boundary boundary(boundStart, boundRange);

// Random num generation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dist(boundStart.x, abs(boundStart.x));

unsigned int SSBO_BODIES;
unsigned int SSBO_TREE;
unsigned int SSBO_TREE_BUFFER_SIZE;

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

QuadTreeUtil qUtil;
const unsigned int sizeOfBody = sizeof(Body);
const unsigned int sizeOfTreeCell = sizeof(TreeCell);


void testTreesAreEqualSingle(std::vector<TreeCell>& tree, std::vector<TreeCell>& expected) {
	REQUIRE(tree.size() == expected.size());
	for (int i = tree.size() - 1; i >= 0; i--) {
		TreeCell cell = tree[i];
		TreeCell expectedCell = expected[i];
		INFO(std::to_string(i));
		REQUIRE(cell.lock == expectedCell.lock);
		if ( cell.numberOfBodies > 0 ) {
			INFO("COMPUTED: \n" + qUtil.printBody(cell.body));
			INFO("EXPECTED: \n" + qUtil.printTreeCell(expectedCell));
			REQUIRE(aboutEqualsVector(glm::vec2(cell.COM), glm::vec2(expectedCell.COM), abs(expectedCell.COM.y) * 0.01));
			REQUIRE(aboutEqualsFloat(cell.mass, expectedCell.mass, expectedCell.mass * 0.01));
		}
	}
}

void testLeavesAreEqualSingle(std::vector<TreeCell>& tree, TreeCell& currTreeCell, QuadTree* expected) {

  REQUIRE(currTreeCell.lock < 0);
  if (currTreeCell.lock == -2) {
	REQUIRE(currTreeCell.childCell > 0);
  }

  bool isLeaf = currTreeCell.lock == -1;
  bool expectedIsLeaf = expected->isLeaf();

  REQUIRE(isLeaf == expectedIsLeaf);


  if (!expectedIsLeaf) {
	REQUIRE(currTreeCell.childCell + 4 <= tree.size());

	// Continue traversal to 4 children
	testLeavesAreEqualSingle(tree, tree[currTreeCell.childCell + 0], expected->m_Q1);
	testLeavesAreEqualSingle(tree, tree[currTreeCell.childCell + 1], expected->m_Q2);
	testLeavesAreEqualSingle(tree, tree[currTreeCell.childCell + 2], expected->m_Q3);
	testLeavesAreEqualSingle(tree, tree[currTreeCell.childCell + 3], expected->m_Q4);
	return;
  }

  // Empty leaf
  if (expected->isEmptyNode()) {
	REQUIRE(currTreeCell.childCell == -1);
	return;
  }

  // Compare the bodies at this leaf
  INFO("COMPUTED: \n" + qUtil.printBody(currTreeCell.body));
  INFO("EXPECTED: \n" + expected->m_body->getPrintString());
  REQUIRE(aboutEqualsVector(glm::vec2(currTreeCell.body.position), glm::vec2(expected->m_body->getPosition()), abs(expected->m_body->getPosition().y) * 0.01));
  REQUIRE(aboutEqualsFloat(currTreeCell.body.mass, expected->m_body->getMass(), expected->m_body->getMass() * 0.01));
}

void testLeavesAreEqualSingle(std::vector<TreeCell>& tree, QuadTree* expected) {
  // Do a traversal, check leaves are equal (since those are deterministic)
  testLeavesAreEqualSingle(tree, tree[0], expected);
}

GravBody* getGravBodyFromBody(Body body) {
	GravBody* gravBody = new GravBody();
	gravBody->setPosition(glm::vec3(body.position));
	gravBody->setVelocity(glm::vec3(body.velocity));
	gravBody->setMass(body.mass);
	return gravBody;
}

QuadTree* createExpectedFromBodies(std::vector<Body>& bodies, int treeSize) {
	QuadTree* root = new QuadTree(boundary);

	for (const auto& body : bodies) {
		GravBody* gravBody = getGravBodyFromBody(body);
		if (!root->insert(gravBody)) {
		  delete gravBody;
		  INFO("An error occured in expected result setup.");
		  REQUIRE(false);
		}
	}

	root->aggregateCenterAndTotalMass();

	return root;
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
	std::cout << sizeOfBody << " " << sizeOfTreeCell << std::endl;

	REQUIRE(window != nullptr);
}

TEST_CASE("Clear tree.") {
	const unsigned int treeSize = 100;

	// Create SSBO_TREE
	glGenBuffers(1, &SSBO_TREE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);

	glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

	double startTime = glfwGetTime();
	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	glDispatchCompute(ceil(treeSize/32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results, in reality clear will only be used on the first node.
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	unsigned int treeBufferSize;
	glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);
	for (const auto& cell : tree) {
		REQUIRE(cell.lock == -1);
		REQUIRE(cell.childCell == -1);
		REQUIRE(treeBufferSize == 1);
	}

	glDeleteBuffers(1, &SSBO_TREE);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);

}

TEST_CASE("Place single body in tree.") {
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

	glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	unsigned int treeBufferSize;
	glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

	REQUIRE(aboutEqualsVector(tree[0].body.position, bodies[0].position, 1e-8));
	REQUIRE(aboutEqualsVector(tree[0].body.velocity, bodies[0].velocity, 1e-8));
	REQUIRE(aboutEqualsFloat(tree[0].body.mass, bodies[0].mass, 1e-8));
	REQUIRE(tree[0].lock == -1);
	REQUIRE(tree[0].childCell == -2);
	REQUIRE(treeBufferSize == 1);
	
	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);

}

TEST_CASE("Place 2 bodies different quadrants.") {
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

  glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  glUniform1ui(treeSizeLoc, treeSize);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
  glUniform1ui(treeSizeLoc, treeSize);
  glUniform1ui(bodySizeLoc, bodies.size());

  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  glDispatchCompute(ceil(treeSize/32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Check results
  std::vector<TreeCell> tree(treeSize);
  glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
  unsigned int treeBufferSize;
  glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

  QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
  testLeavesAreEqualSingle(tree, expected);

  glDeleteBuffers(1, &SSBO_BODIES);
  glDeleteBuffers(1, &SSBO_TREE);
  glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  delete expected;
}


TEST_CASE("Place 4 bodies different quadrants.") {
  const unsigned int treeSize = 5;

  // Create input data
  std::vector<Body> bodies{
	  Body{glm::vec4(1.0f, 1.0f, 0.0f, 0.0f), glm::vec4(3.0f), 51.0f},
	  Body{glm::vec4(-1.0f, 1.0f, 0.0f, 0.0f), glm::vec4(3.0f), 51.0f},
	  Body{glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f), glm::vec4(3.0f), 51.0f},
	  Body{glm::vec4(1.0f, -1.0f, 0.0f, 0.0f), glm::vec4(3.0f), 51.0f},
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

  glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  glUniform1ui(treeSizeLoc, treeSize);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
  glUniform1ui(treeSizeLoc, treeSize);
  glUniform1ui(bodySizeLoc, bodies.size());

  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  glDispatchCompute(ceil(treeSize/32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Check results
  std::vector<TreeCell> tree(treeSize);
  glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
  unsigned int treeBufferSize;
  glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

  QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
  testLeavesAreEqualSingle(tree, expected);

  glDeleteBuffers(1, &SSBO_BODIES);
  glDeleteBuffers(1, &SSBO_TREE);
  glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  delete expected;
}

TEST_CASE("Place 2 bodies nested quadrant.") {
  const unsigned int treeSize = 64;

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

  glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  glUniform1ui(treeSizeLoc, treeSize);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
  glUniform1ui(treeSizeLoc, treeSize);
  glUniform1ui(bodySizeLoc, bodies.size());

  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  glDispatchCompute(ceil(treeSize/32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Check results
  std::vector<TreeCell> tree(treeSize);
  glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
  unsigned int treeBufferSize;
  glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

  QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
  testLeavesAreEqualSingle(tree, expected);

  glDeleteBuffers(1, &SSBO_BODIES);
  glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glDeleteBuffers(1, &SSBO_TREE);
  delete expected;
}

TEST_CASE("Double nested and single nested.") {
  const unsigned int treeSize = 64;

  // Create input data
  std::vector<Body> bodies{
			Body{glm::vec4(1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q3->Q1
			Body{glm::vec4(3e9f), glm::vec4(3.0f), 51.0f}, // Q1->Q3->Q4
			Body{glm::vec4(6e9f, 6e9f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q1
			Body{glm::vec4(1.0f, 6e9f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q2
			Body{glm::vec4(6e9f, 1.0f, 1.0f, 1.0f), glm::vec4(3.0f), 51.0f}, // Q1->Q4
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

  glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

  ShaderManager* shaderManager = ShaderManager::getInstance();
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  glUniform1ui(treeSizeLoc, treeSize);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
  unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
  glUniform1ui(treeSizeLoc, treeSize);
  glUniform1ui(bodySizeLoc, bodies.size());

  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
  glDispatchCompute(ceil(treeSize/32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
  glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Check results
  std::vector<TreeCell> tree(treeSize);
  glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
  unsigned int treeBufferSize;
  glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

  QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
  testLeavesAreEqualSingle(tree, expected);

  glDeleteBuffers(1, &SSBO_BODIES);
  glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
  glDeleteBuffers(1, &SSBO_TREE);
  delete expected;
}

TEST_CASE("Small stress test.") {

  for (int i = 0; i < 1000; i++) {
	const unsigned int treeSize = 1000;

	// Create input data
	std::vector<Body> bodies(31);
	double range = 1e10;
	double inc = range / bodies.size(); // Used to make exactly uniform distribution
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

	glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute(ceil(treeSize/32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	unsigned int treeBufferSize;
	glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

	QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
	testLeavesAreEqualSingle(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glDeleteBuffers(1, &SSBO_TREE);
	delete expected;

  }

}

TEST_CASE("Random body 1k leaf test.") {
  const unsigned int treeSize = 10000;

  for (int i = 0; i < 100; i++) {
	// Create input data
	std::vector<Body> bodies(100);
	double range = 1e10;
	double inc = range / bodies.size(); // Used to make exactly uniform distribution
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

	glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute((treeSize/32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	unsigned int treeBufferSize;
	glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

	QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
	testLeavesAreEqualSingle(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	delete expected;
  }
}

TEST_CASE("Random body 1M leaf test.") {
  const unsigned int treeSize = 10000000;

  for (int i = 0; i < 100; i++) {
	// Create input data
	std::vector<Body> bodies(1000000);
	double range = 1e10;
	double inc = range / bodies.size(); // Used to make exactly uniform distribution
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

	glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);

	ShaderManager* shaderManager = ShaderManager::getInstance();
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	glUniform1ui(treeSizeLoc, treeSize);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
	glUniform1ui(treeSizeLoc, treeSize);
	glUniform1ui(bodySizeLoc, bodies.size());

	shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
	glDispatchCompute((treeSize / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
	glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Check results
	std::vector<TreeCell> tree(treeSize);
	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
	unsigned int treeBufferSize;
	glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);

	QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
	testLeavesAreEqualSingle(tree, expected);

	glDeleteBuffers(1, &SSBO_BODIES);
	glDeleteBuffers(1, &SSBO_TREE);
	glDeleteBuffers(1, &SSBO_TREE_BUFFER_SIZE);
	delete expected;
  }
}


//TEST_CASE("Random body 1M leaf test.") {
//  const unsigned int treeSize = 10000000;
//
//  // Create input data
//  std::vector<Body> bodies(1000000);
//  double range = 1e10;
//  double inc = range / bodies.size(); // Used to make exactly uniform distribution
//  for (int i = 0; i < bodies.size(); i++) {
//	bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), 51.0f };
//  }
//
//  // Create SSBO_BODIES
//  glGenBuffers(1, &SSBO_BODIES);
//  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_BODIES);
//  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfBody * bodies.size(), &bodies[0], GL_DYNAMIC_DRAW);
//  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_BODIES);
//
//  // Create SSBO_TREE
//  glGenBuffers(1, &SSBO_TREE);
//  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE);
//  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
//  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);
//
//  glGenBuffers(1, &SSBO_TREE_BUFFER_SIZE);
//  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_TREE_BUFFER_SIZE);
//  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1, nullptr, GL_DYNAMIC_DRAW);
//  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, SSBO_TREE_BUFFER_SIZE);
//
//  ShaderManager* shaderManager = ShaderManager::getInstance();
//  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
//  unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//  glUniform1ui(treeSizeLoc, treeSize);
//  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//  treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//  unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//  glUniform1ui(treeSizeLoc, treeSize);
//  glUniform1ui(bodySizeLoc, bodies.size());
//
//  shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
//  glDispatchCompute((treeSize / 32.0), 1, 1);
//  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//  shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//  glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//  // Check results
//  std::vector<TreeCell> tree(treeSize);
//  glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
//  unsigned int treeBufferSize;
//  glGetNamedBufferSubData(SSBO_TREE_BUFFER_SIZE, 0, sizeof(GLuint) * 1, &treeBufferSize);
//
//  QuadTree* expected = createExpectedFromBodies(bodies, treeSize);
//  testLeavesAreEqualSingle(tree, expected);
//
//  glDeleteBuffers(1, &SSBO_BODIES);
//  glDeleteBuffers(1, &SSBO_TREE);
//  delete expected;
//}


//
//TEST_CASE("Test aggregation of total COM and total mass.") {
//
//	const unsigned int numberOfLevelsInTree = 11;
//	const unsigned int treeSize = qUtil.sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
//	clearGLErrors();
//
//	// Create input data
//	std::vector<Body> bodies(100000);
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
//			glUniform1ui(levelStartLoc, qUtil.startPositionOfLevel(i));
//
//			// Dispatch compute for that level of tree
//			int numberOfParentCells = qUtil.numberOfCellsInLevel(i - 1);
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
//TEST_CASE("Test aggregation to non-leaf cells of COM and total mass and accuracy single-cell-body.") {
//
//	const unsigned int numberOfLevelsInTree = 11;
//	const unsigned int treeSize = qUtil.sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
//	clearGLErrors();
//
//	// Create input data
//	std::vector<Body> bodies(100);
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
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);
//
//	printErrors();
//
//	ShaderManager* shaderManager = ShaderManager::getInstance();
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//	glUniform1ui(bodySizeLoc, bodies.size());
//
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree.comp");
//	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//
//	double startTime = glfwGetTime();
//	for (int i = 0; i < 100; i++) {
//		startTime = glfwGetTime();
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
//		glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree.comp");
//		unsigned int levelStartLoc = glGetUniformLocation(shaderManager->getBoundShader(), "levelStart");
//		for (int i = numberOfLevelsInTree - 1; i >= 1; i--) {
//			// Set level
//			glUniform1ui(levelStartLoc, qUtil.startPositionOfLevel(i));
//
//			// Dispatch compute for that level of tree
//			int numberOfParentCells = qUtil.numberOfCellsInLevel(i - 1);
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
//	std::vector<TreeCell> tree(treeSize);
//	glGetNamedBufferSubData(SSBO_TREE, 0, sizeOfTreeCell * treeSize, &tree[0]);
//	glFinish();
//
//	//Check with expected
//	startTime = glfwGetTime();
//	std::vector<TreeCell> expected = createExpectedFromBodies(bodies, treeSize);
//	std::cout << "Time to calculate on CPU: " << (glfwGetTime() - startTime) << std::endl;
//
//	testTreesAreEqualSingle(tree, expected);
//
//	glDeleteBuffers(1, &SSBO_BODIES);
//	glDeleteBuffers(1, &SSBO_TREE);
//	
//}

//TEST_CASE("Test small amount of bodies full force calculation matches.") {
//
//	// I may need to remap the ranges here (nvm this is adjusted for with G multiplication):
//	// MASS: 2e29 (small star) - 2e40 (close to theoretical limit)
//	// If everything is in units of a small star (2e29). Then range of values is
//	// 1-2e11 for small star vs the largest black hole. When doing G * m1 * m2, this is "safe"
//
//	// This would require G = 6.67e-11 / 2e29...
//
//	// Distance: 
//	// Let's use ~0.1 light year = 1e15. A galaxy width is 100k light years
//	// This would give us a range with plenty of headroom (around 1e16 light years) for interactions
//
//	// The G for this = 6.67e-11 / 2e29 / 1e15 = 3.335e-55
//
//	const unsigned int numberOfLevelsInTree = 12;
//	const unsigned int treeSize = qUtil.sizeOfTreeGivenNumberOfLevels(numberOfLevelsInTree);
//	clearGLErrors();
//
//	// Constants for the time-step
//	const float SIUnitScaleFactor = 1e10;
//	const float G = 6.67430e-11 / SIUnitScaleFactor / SIUnitScaleFactor;
//	float deltaT = 0.0166666f; // Can be anything, this corresponds to 60fps
//	const float timeFactor = 60; // n years per second
//
//	// Create input data
//	//std::vector<Body> bodies(100);
//	//for (int i = 0; i < bodies.size(); i++) {
//	//	bodies[i] = Body{ glm::vec4(dist(gen),dist(gen),0,0), glm::vec4(0.0), 2e13 };
//	//}
//
//	// Create input data (Basically 4 black holes within 0.2 light years of each other (pretty extreme conditions)
//	std::vector<Body> bodies{
//		Body{ glm::vec4(1,1,0,0), glm::vec4(0.0), 1e30 },
//		Body{ glm::vec4(-1,-1,0,0), glm::vec4(0.0), 1e30 },
//		Body{ glm::vec4(-1,1,0,0), glm::vec4(0.0), 1e30 },
//		Body{ glm::vec4(1,-1,0,0), glm::vec4(0.0), 1e30 },
//	};
//
//	//Create system to check expected against
//	System system;
//	system.setSIUnitScaleFactor(SIUnitScaleFactor);
//	system.setTimeFactor(timeFactor);
//	for (auto const& body : bodies) {
//		system.addBody(getGravBodyFromBody(body));
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
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfTreeCell * treeSize, nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, SSBO_TREE);
//
//	printErrors();
//
//	ShaderManager* shaderManager = ShaderManager::getInstance();
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//	unsigned int treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//	unsigned int bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//	glUniform1ui(bodySizeLoc, bodies.size());
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree.comp");
//	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//
//	shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_forces_quad_tree.comp");
//	treeSizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "treeSize");
//	glUniform1ui(treeSizeLoc, treeSize);
//	bodySizeLoc = glGetUniformLocation(shaderManager->getBoundShader(), "bodySize");
//	glUniform1ui(bodySizeLoc, bodies.size());
//	unsigned int deltaTLoc = glGetUniformLocation(shaderManager->getBoundShader(), "deltaT");
//	glUniform1f(deltaTLoc, deltaT * timeFactor);
//	unsigned int GLoc = glGetUniformLocation(shaderManager->getBoundShader(), "G");
//	glUniform1f(GLoc, G);
//
//	const int numberOfIterations = 10;
//	double startTime = glfwGetTime();
//	for (int i = 0; i < numberOfIterations; i++) {
//		startTime = glfwGetTime();
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/clear_quad_tree.comp");
//		glDispatchCompute(ceil(treeSize / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/build_quad_tree.comp");
//		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_mass_quad_tree.comp");
//		unsigned int levelStartLoc = glGetUniformLocation(shaderManager->getBoundShader(), "levelStart");
//		for (int i = numberOfLevelsInTree - 1; i >= 1; i--) {
//			// Set level
//			glUniform1ui(levelStartLoc, qUtil.startPositionOfLevel(i));
//
//			// Dispatch compute for that level of tree
//			int numberOfParentCells = qUtil.numberOfCellsInLevel(i - 1);
//			glDispatchCompute(numberOfParentCells, 1, 1);
//			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//		}
//
//		shaderManager->bindComputeShader("../assets/shaders/compute/physics/sum_forces_quad_tree.comp");
//		glDispatchCompute(ceil(bodies.size() / 32.0), 1, 1);
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		glFinish();
//		std::cout << "Time to do all tree calculations (GPU): " << (glfwGetTime() - startTime) << std::endl;
//
//	}
//
//	printErrors();
//
//	// Get results
//	std::vector<Body> result(bodies.size());
//	glGetNamedBufferSubData(SSBO_BODIES, 0, sizeOfBody * bodies.size(), &result[0]);
//	glFinish();
//
//	double maxNodes = 0;
//	for (auto const& body : result) {
//		maxNodes = max(maxNodes, body.align1);
//	}
//	std::cout << "Max recursion: " << maxNodes << std::endl;
//
//	for (int i = 0; i < numberOfIterations; i++) {
//		system.update(deltaT);
//	}
//
//	createExpectedFromBodies(bodies, treeSize);
//	std::vector<GravBody*> expected = system.getBodies();
//
//	for (int i = 0; i < bodies.size(); i++) {
//		INFO(i);
//		INFO("POSITION: \n")
//		INFO(result[i].position.x);
//		INFO(result[i].position.y);
//		INFO("\n")
//		INFO(expected[i]->getPosition().x);
//		INFO(expected[i]->getPosition().y);
//		INFO("\n")
//		INFO("VELOCITY: \n")
//		INFO(result[i].velocity.x);
//		INFO(result[i].velocity.y);
//
//		INFO("\n")
//		INFO(expected[i]->getVelocity().x);
//		INFO(expected[i]->getVelocity().y);
//
//
//		REQUIRE(aboutEqualsVector(result[i].position, glm::vec4(expected[i]->getPosition(), 0.0), 1));
//		REQUIRE(aboutEqualsVector(result[i].velocity, glm::vec4(expected[i]->getVelocity(), 0.0), 1));
//	}
//
//	glDeleteBuffers(1, &SSBO_BODIES);
//	glDeleteBuffers(1, &SSBO_TREE);
//
//}

