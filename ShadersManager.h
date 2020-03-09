#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "shader.h"

#define SET_TO_ALL_SHADERS(setFunc, ...) setFunc(__VA_ARGS__)

class ShadersManager
{
    static int NEXT_ID;
    std::map<int, Shader> shaders;
public:
    int AddShader(Shader && shader);
    int CreateShader(const GLchar* vertexPath, const GLchar* fragmentPath);
    int GetShaderID(const GLchar* vertexPath, const GLchar* fragmentPath);

    template<typename T>
    void set(const std::string &name, const T& value)
    {
        for(auto& id_shader : shaders)
        {
            Shader& shader = id_shader.second;
            shader.use();
            shader.set(name, value);
        }
    }
    
    template<typename T>
    void set3(const std::string &name, const T& value1, const T& value2, const T& value3)
    {
        for(auto& id_shader : shaders)
        {
            Shader& shader = id_shader.second;
            shader.use();
            shader.set(name, value1, value2, value3);
        }
    }
    
    template<typename T>
    void set4(const std::string &name, const T& value1, const T& value2, const T& value3, const T& value4)
    {
        for(auto& id_shader : shaders)
        {
            Shader& shader = id_shader.second;
            shader.use();
            shader.set(name, value1, value2, value3, value4);
        }
    }

    void setMat4(const std::string &name, const glm::mat4 &mat);
    void setVec3(const std::string &name, float x, float y, float z);
    void setVec3(const std::string &name, const glm::vec3 &vec);

    class Shader& GetShader(int id);

};