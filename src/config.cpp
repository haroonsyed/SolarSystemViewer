#include "config.h"

Config* Config::m_instance = nullptr;

Config::Config() {
  m_SCR_WIDTH = 800;
  m_SCR_HEIGHT = 600;
  m_TARGET_FRAMERATE = 60;
  m_INPUT_POLL_RATE = 1000; // Default to 1000Hz (limited by fps so doesn't matter)
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
unsigned int Config::getInputPollRate() {
  return m_INPUT_POLL_RATE;
}
