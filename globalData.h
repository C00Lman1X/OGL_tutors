#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "camera.h"

struct Light {
    int type; // 0 - point light, 1 - direction light, 2 - spotlight
    glm::vec3 ambient{0.1f, 0.1f, 0.1f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};

    // for point light (http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation)
    float constant{1.f}, linear{0.09f}, quadratic{0.032f};
    
    // only for point and spot light
    glm::vec3 location{0.f, 0.f, 0.f};
    
    // only for spot and direction light
    glm::vec3 direction{-0.5f, -1.f, -0.3f};
    
    //only for spotlight
    float innerCutOff{0.97629600712f}; // cos(12.5)
    float outerCutOff{0.95371695074f}; // cos(17.5)

    Light(int type_ = 0) : type(type_) {
        if (type == 2)
        {
            diffuse = {0.5f, 0.5f, 0.5f};
            specular = {0.5f, 0.5f, 0.5f};
        }
    }
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
    bool drawLight = true;
    Camera camera;

    float width = 640.f;
    float height = 480.f;

    // cursor processing
    bool cursorCaptured = false;
    double lastX, lastY;

    static const glm::vec3 DEFAULT_CAMERA_POS;

private:
    GlobalData()
        : camera(DEFAULT_CAMERA_POS)
    {
    }
};

#define DATA GlobalData::GetInstance()