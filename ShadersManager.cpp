#include "ShadersManager.h"
#include "shader.h"

int ShadersManager::NEXT_ID = 1;

int ShadersManager::AddShader(Shader && shader)
{
    shaders.insert({NEXT_ID, shader});
    return NEXT_ID++;
}

int ShadersManager::CreateShader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
    Shader shader{vertexPath, fragmentPath};
    return AddShader(std::move(shader));
}

int ShadersManager::GetShaderID(const GLchar* vertexPath, const GLchar* fragmentPath)
{
    auto it = std::find_if(shaders.begin(), shaders.end(), [vertexPath, fragmentPath](const std::pair<int, Shader>& p){
        return p.second.vShaderName == vertexPath && p.second.fShaderName == fragmentPath;
    });
    if (it != shaders.end())
        return it->first;
    return CreateShader(vertexPath, fragmentPath);
}

Shader& ShadersManager::GetShader(int id)
{
    return shaders.at(id);
}