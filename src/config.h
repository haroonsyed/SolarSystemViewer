#pragma once
#include <string>

class Config {

private:

  static Config* m_instance;
  unsigned int m_SCR_WIDTH;
  unsigned int m_SCR_HEIGHT;
  unsigned int m_TARGET_FRAMERATE = 60;
  unsigned int m_INPUT_POLL_RATE = 1000; // Default to 1000Hz (limited by fps so doesn't matter)

  Config();

public:
  static Config* getInstance();
  unsigned int getScreenWidth();
  void setScreenWidth(unsigned int width);
  void setScreenHeight(unsigned int height);
  unsigned int getScreenHeight();
  unsigned int getTargetFramerate();
  unsigned int getInputPollRate();

};
