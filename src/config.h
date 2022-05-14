#pragma once
#include <string>

class Config {

private:

  static Config* m_instance;
  unsigned int m_SCR_WIDTH;
  unsigned int m_SCR_HEIGHT;
  unsigned int m_TARGET_FRAMERATE;
  double m_MOUSE_SENSITIVITY;

  bool m_bloomEnabled;
  bool m_autoExposureEnabled;
  float m_autoExposureControl;
  double m_auto_exposure_center_range; // Percent of middle of screen to use for AE calculation

  Config();

public:
  static Config* getInstance();
  unsigned int getScreenWidth();
  void setScreenWidth(unsigned int width);
  void setScreenHeight(unsigned int height);
  unsigned int getScreenHeight();
  unsigned int getTargetFramerate();
  double getMouseSensitivity();
  double getAutoExposureRange();
  bool getBloomEnabled();
  bool getAutoExposureEnabled();
  float getAutoExposureControl();


};
