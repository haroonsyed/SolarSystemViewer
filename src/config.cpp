#include "config.h"

Config* Config::instance = nullptr;

Config::Config() {
  SCR_WIDTH = 800;
  SCR_HEIGHT = 600;
  TARGET_FRAMERATE = 60;
  INPUT_POLL_RATE = 1000; // Default to 1000Hz (limited by fps so doesn't matter)
}

Config* Config::getInstance() {
  if (instance == nullptr) {
    instance = new Config();
  }
  return instance;
}
unsigned int Config::getScreenWidth() {
  return SCR_WIDTH;
}
unsigned int Config::getScreenHeight() {
  return SCR_HEIGHT;
}
void Config::setScreenWidth(unsigned int width) {
  SCR_WIDTH = width;
}
void Config::setScreenHeight(unsigned int height) {
  SCR_HEIGHT = height;
}
unsigned int Config::getTargetFramerate() {
  return TARGET_FRAMERATE;
}
unsigned int Config::getInputPollRate() {
  return INPUT_POLL_RATE;
}
