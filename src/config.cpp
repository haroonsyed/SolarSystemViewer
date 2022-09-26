#include "config.h"

Config* Config::m_instance = nullptr;

Config::Config() {
  m_SCR_WIDTH = 800;
  m_SCR_HEIGHT = 600;
  m_TARGET_FRAMERATE = 60;
  m_MOUSE_SENSITIVITY = 0.8;
  m_bloomEnabled = true;
  m_autoExposureEnabled = false;
  m_renderSkyBox = false;
  m_autoExposureControl = 1e-2;
  m_auto_exposure_center_range = 0.20;
  m_bloomThreshold = 150.0f;
}

Config* Config::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new Config();
  }
  return m_instance;
}
unsigned int Config::getScreenWidth() {
  return m_SCR_WIDTH;
}
unsigned int Config::getScreenHeight() {
  return m_SCR_HEIGHT;
}
void Config::setScreenWidth(unsigned int width) {
  m_SCR_WIDTH = width;
}
void Config::setScreenHeight(unsigned int height) {
  m_SCR_HEIGHT = height;
}
unsigned int Config::getTargetFramerate() {
  return m_TARGET_FRAMERATE;
}
double Config::getMouseSensitivity() {
  return m_MOUSE_SENSITIVITY;
}

double Config::getAutoExposureRange() {
  return m_auto_exposure_center_range;
}

bool Config::getBloomEnabled() {
    return m_bloomEnabled;
}

bool Config::getAutoExposureEnabled() {
  return m_autoExposureEnabled;
}

float Config::getAutoExposureControl() {
  return m_autoExposureControl;
}

float Config::getBloomThreshold() {
  return m_bloomThreshold;
}

bool Config::getSkyBoxEnabled() {
    return m_renderSkyBox;
}