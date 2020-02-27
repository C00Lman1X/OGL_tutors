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

Shader& ShadersManager::GetShader(int id)
{
    return shaders.at(id);
}