#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "camera.h"

struct Light {
    int type; // 0 - point light, 1 - direction light, 2 - spotlight
    glm::vec3 location;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    // only for spot and direction light
    glm::vec3 direction;
    
    //only for spotlight
    float innerCutOff;
    float outerCutOff;
};

class GlobalData
{
public:
    static GlobalData& GetInstance()
    {
        static GlobalData obj;
        return obj;
    }
    
    GLenum mode = GL_FILL;
    std::vector<Light> lights;
    double lastX = 320.0, lastY = 240.0;
    bool drawLight = true;
    Camera camera;

    static const glm::vec3 DEFAULT_CAMERA_POS;

private:
    GlobalData()
        : camera(DEFAULT_CAMERA_POS)
    {
    }
};

#define DATA GlobalData::GetInstance()