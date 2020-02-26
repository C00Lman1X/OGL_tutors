#pragma once
#include <vector>
#include <string>
#include "glad/glad.h"
#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>

class Shader;

class Model
{
public:
    Model(char *path) {
        loadModel(path);
    }
    void Draw(Shader& shader, bool light = false, float angle = 0.f, glm::vec3 axis = {0.f, 0.f, 0.f});
    
    glm::vec3 location{0.f};
    glm::vec3 rotation{0.f};
    glm::vec3 scale{1.f};

    bool solidColor = false;
    glm::vec3 color{0.f, 0.f, 0.f};

    float shininess = 32.f;

private:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

GLuint TextureFromFile(const char *path, const std::string& directory, bool gamma = false);