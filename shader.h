#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <map>

class Shader
{
public:
    GLuint ID;
    std::string vShaderName;
    std::string fShaderName;

    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

    void use();

    void set(const std::string& name, bool value) const;
    void set(const std::string& name, int value) const;
    void set(const std::string& name, float value) const;
    void set(const std::string &name, float x, float y, float z) const;
    void set(const std::string &name, const glm::vec3 &vec) const;
    void set(const std::string &name, const glm::vec4 &vec) const;
    void set(const std::string &name, const glm::mat4 &mat) const;
    void set(const std::string &name, float f1, float f2, float f3, float f4) const;
    void set(const std::string &name, float *f, int count);

private:
    GLint getUniformLoc(const std::string &name) const;

    mutable std::map<std::string, bool> checkedUniforms; // true - is active
};

#endif