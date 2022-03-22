#pragma once
#include <string>

// Note, path from build folder is "../data/dolphins.obj"
// Path on visual studio (default if not using the CMakePresets.json) is "../../../data/dolphins.obj"
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;
static bool useSeparateTriangles = true;
static const std::string meshFilePath = "../assets/models/sphere.obj";
static const std::string vertexShaderPath = "../shaders/phong.vs";
static const std::string fragShaderPath = "../shaders/phong.fs";