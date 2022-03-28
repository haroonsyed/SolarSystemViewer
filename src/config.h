#pragma once
#include <string>

// Note, path from build folder is "../data/dolphins.obj"
// Path on visual studio (default if not using the CMakePresets.json) is "../../{path from build folder}"
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;
static const unsigned int TARGET_FRAMERATE = 60;
static const unsigned int INPUT_POLL_RATE = 1000; // Default to 1000Hz (limited by fps so doesn't matter)
