#pragma once

#include <glm/glm.hpp>

#include "shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

std::vector<std::vector<std::string>>generateWorld(const char* wallFilePath);

std::vector<std::vector<std::string>> wallSections(const std::string& walls, const char& delimiter = '-');

void generatePlatform(Shader& shader, const std::vector<std::string>& floor, glm::vec3 startPos = glm::vec3(0.0f), float rotation = 0.0f);

void renderWorld(Shader& shader, std::vector<std::vector<std::string>> floors, glm::vec3 startPos = glm::vec3(0.0f), float rotation = 0.0f);

std::string wallReader(const char* wallFilePath);

