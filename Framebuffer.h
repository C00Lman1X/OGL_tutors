#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Framebuffer
{
    GLuint fbo;
    GLuint rbo;
    GLuint textureID;
	GLuint quadVAO, quadVBO;

    glm::vec4 clearColor;

public:
    Framebuffer(GLsizei width, GLsizei height, glm::vec4 color);
    void Use();
    void Draw();
};