#pragma once
#include <string>

class Config {

private:

  static Config* m_instance;
  unsigned int m_SCR_WIDTH;
  unsigned int m_SCR_HEIGHT;
  unsigned int m_TARGET_FRAMERATE;
  double m_MOUSE_SENSITIVITY;

  Config();

public:
  static Config* getInstance();
  unsigned int getScreenWidth();
  void setScreenWidth(unsigned int width);
  void setScreenHeight(unsigned int height);
  unsigned int getScreenHeight();
  unsigned int getTargetFramerate();
  double getMouseSensitivity();

};
