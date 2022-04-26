#include "gameController.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../graphics/screen/screenManager.h"
#include "../config.h"

GameController* GameController::m_instance = nullptr;

double GameController::m_mouseX = 0;
double GameController::m_mouseY = 0;
double GameController::m_deltaX = 0;
double GameController::m_deltaY = 0;
std::unordered_set<unsigned int> GameController::m_heldKeys;
bool GameController::m_showGui = false;

GLFWwindow* GameController::m_window = nullptr;
Scene* GameController::m_boundScene = nullptr;
Gui* GameController::gui = nullptr;
int GameController::m_focusedBody = -1;
GravBody* target = nullptr;


GameController* GameController::getInstance(GLFWwindow* window, Scene* scene) {
  if (m_instance == nullptr) {
    m_instance = new GameController(window, scene);
    gui = new Gui();
  }
  return m_instance;
}

GameController::GameController(GLFWwindow* window, Scene* scene) {
  m_window = window;
  m_boundScene = scene;
  glfwSetKeyCallback(m_window, processKeyInput);
  glfwSetMouseButtonCallback(m_window, processMousePress);
  glfwSetCursorPosCallback(m_window, processMousePos);
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void GameController::processKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();

  // CLOSE APPLICATION
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, true);
  }

  // Actions to perform on key press edge
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
    m_focusedBody += 1;
  }

  if (key == GLFW_KEY_G && action == GLFW_PRESS) {
    toggleGUI();
  }

  // Store keys to be held
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_W);
  }
  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_A);
  }
  if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_S);
  }
  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_D);
  }
  if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_Z);
  }
  if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_KEY_X);
  }

  // Check for the release of keys to remove
  if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_W);
  }
  if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_A);
  }
  if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_S);
  }
  if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_D);
  }
  if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_Z);
  }
  if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_KEY_X);
  }

}

void GameController::processMousePress(GLFWwindow* window, int button, int action, int mods) {

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_MOUSE_BUTTON_LEFT);
  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    m_heldKeys.insert(GLFW_MOUSE_BUTTON_MIDDLE);
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_MOUSE_BUTTON_LEFT);
  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    m_heldKeys.erase(GLFW_MOUSE_BUTTON_MIDDLE);
  }

}

void GameController::processMousePos(GLFWwindow* window, double xpos, double ypos) {

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();

  m_mouseX = xpos;
  m_mouseY = ypos;

  if (!m_showGui) {
    // Center cursor
    m_deltaX = m_mouseX - SCR_WIDTH / 2;
    m_deltaY = m_mouseY - SCR_HEIGHT / 2;
    glfwSetCursorPos(m_window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
  }

}

void GameController::updateFocusedPlanet() {
  
  auto physicsSystem = m_boundScene->getPhysicsSystem();
  auto bodies = physicsSystem->getBodies();
  Camera* camera = m_boundScene->getCamera();

  
  if (m_focusedBody == bodies.size()) {
    camera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
    m_focusedBody = -1;
  }

  if (m_focusedBody > -1) {

    if (m_focusedBody < bodies.size()) {
      target = bodies[m_focusedBody];
    }

    if (target != nullptr) {

      camera->setUp(glm::vec3(0.0f, 0.0f, 1.0f));

      float physicsDistanceFactor = physicsSystem->getPhysicsDistanceFactor();
      float universeScaleFactor = m_boundScene->getUniverseScaleFactor();

      glm::vec3 scaledTargetPos = target->getPosition() / universeScaleFactor;
      float radius = target->getScale();
      float tanFov = glm::tan(glm::radians(camera->getFov() / 2.0f));
      float camDistance = 3 * radius / tanFov; // 3 accounts for most screen aspect ratios

      camera->setCameraTarget(scaledTargetPos);
      camera->setCameraPosition(scaledTargetPos + glm::vec3(0.0f, camDistance, 0.0f));
      
    }
    else {
      std::cout << "Could not find target planet: " << m_focusedBody << std::endl;
    }

  }

}

void GameController::toggleGUI() {

  // Get config data
  Config* config = Config::getInstance();
  unsigned int SCR_WIDTH = config->getScreenWidth();
  unsigned int SCR_HEIGHT = config->getScreenHeight();

  m_showGui = !m_showGui;

  if (m_showGui) {
    // Don't hide cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  else {
    // Hide and center cursor
    m_deltaX -= SCR_WIDTH / 2;
    m_deltaY -= SCR_HEIGHT / 2;
    glfwSetCursorPos(m_window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }

}

void GameController::updateCamera(float deltaT) {

  Config* config = Config::getInstance();

  Camera* camera = m_boundScene->getCamera();
  glm::vec3 camPos = camera->getCameraPosition();
  glm::vec3 camTarget = camera->getCameraTarget();
  glm::vec3 up = camera->getUp();

  float deltaDistance = deltaT * glm::distance(camPos, camTarget);
  glm::vec3 viewDir = glm::normalize(camTarget - camPos);

  // Use mouse + LF to move camera. Use Scroll / X,Z to zoom
  if (m_heldKeys.count(GLFW_MOUSE_BUTTON_LEFT)) {

    double sensitivity = config->getMouseSensitivity();
    float adjustedDeltaX = sensitivity * deltaDistance * m_deltaX;
    float adjustedDeltaY = sensitivity * deltaDistance * m_deltaY;
    glm::vec3 left = glm::vec3(-1.0, 0.0, 0.0);

    camPos += adjustedDeltaY * up;
    camTarget += adjustedDeltaY * up;
    camPos += adjustedDeltaX * left;
    camTarget += adjustedDeltaX * left;

  }

  if (m_heldKeys.count(GLFW_KEY_Z)) {
    camPos += deltaDistance * viewDir;
  }
  else if (m_heldKeys.count(GLFW_KEY_X)) {
    camPos -= deltaDistance * viewDir;
  }

  camera->setCameraPosition(camPos);
  camera->setCameraTarget(camTarget);

}

void GameController::update(float deltaT) {
  m_boundScene->getPhysicsSystem()->update(deltaT);
  updateCamera(deltaT);
  updateFocusedPlanet();
}

void GameController::render(float deltaT) {

  // Create framebuffer and setup screen to render to
  ScreenManager* screenManager = ScreenManager::getInstance();

  // Clear previous frame
  screenManager->bindSceneBuffer();
  screenManager->clearScreenBuffer();

  m_boundScene->render();

  // Render to screen
  screenManager->renderToScreen();

  // Render GUI ontop
  if (target == nullptr)
  {
      std::vector<std::string> temp;
      temp.push_back("Solar System View");
      gui->render(1.0f / deltaT, m_showGui, temp);
  }
  else
  {
      gui->render(1.0f / deltaT, m_showGui, target->getPlanetInfo());
  }

}
