#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "horrorWorld.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


std::vector<std::vector<std::string>> generateWorld(const char* wallFilePath)
{
    std::string building = wallReader(wallFilePath);
    return wallSections(building);
}

void renderWorld(Shader& shader, std::vector<std::vector<std::string>> floors, glm::vec3 startPos, float rotation)
{

    for (int i = 0; i < floors.size(); i++)
        generatePlatform(shader, floors[i], startPos + glm::vec3(0.0f, i, 0.0f), rotation);
}

void generatePlatform(Shader& ourShader, const std::vector<std::string>& floor, glm::vec3 startPos, float rotation)
{
    for (int z = 0; z < floor.size(); z++)
    {
        for (int x = 0; x < floor.at(z).size(); x++)
        {
            if (floor.at(z).at(x) == '#')
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(startPos.x + x, startPos.y, startPos.z + z));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
}

std::string wallReader(const char* wallFilePath)
{
    std::string walls;

    std::ifstream wallfile;

    wallfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        wallfile.open(wallFilePath);

        std::stringstream wallFileStream;

        wallFileStream << wallfile.rdbuf();

        wallfile.close();

        walls = wallFileStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::PLATFORMREADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return walls;
}

std::vector<std::vector<std::string>> wallSections(const std::string& walls, const char& delimiter)
{
    std::vector<std::vector<std::string>> floors;
    std::vector<std::string> floor; 

    std::string section; 
    std::string row;

    std::stringstream ss(walls);
    std::istringstream sectionStream;
    

    while (std::getline(ss, section, delimiter))
    {
        std::istringstream sectionStream(section);

        while (std::getline(sectionStream, row))
            if (!row.empty()) floor.push_back(row);

        floors.push_back(floor);
        floor.clear();
    }

    return floors;
}