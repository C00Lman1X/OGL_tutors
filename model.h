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
    Model(const char *path, int shaderId)
        : ID(NEXT_ID++)
    {
        loadModel(path);
        std::string sPath{path};
        size_t pos = sPath.find_last_of('/') + 1;
        size_t count = sPath.find_last_of('.') - pos;
        name = std::to_string(ID) + "_" + sPath.substr(pos, count);

        shaderID = shaderId;
    }
    Model(const Mesh& mesh, int shaderId, glm::vec3 location_ = {0.f, 0.f, 0.f}, glm::vec3 scale_ = {1.f, 1.f, 1.f}, glm::vec3 rotation_ = {0.f, 0.f, 0.f});
    void DrawPointLight();
    void DrawSpotLight(float angle, glm::vec3 axis);
    void DrawModel();

    const glm::vec3& GetLocation() const { return location; }
    void SetLocation(const glm::vec3& location);
    const glm::vec3& GetScale() const { return scale; }
    void SetScale(const glm::vec3& location);
    const glm::vec3& GetRotation() const { return rotation; }
    void SetRotation(const glm::vec3& rotation);
    
    void SortFaces();

    bool solidColor = false;
    glm::vec4 color{0.f, 0.f, 0.f, 1.f};

    float shininess = 32.f;

    bool outline = false;
    bool opaque = false;
    bool transparentCube = false;
    int shaderID = 0;
    int ID = 0;

    const std::string& GetName() const { return name; }
    void ChangeName(const std::string& newName);

private:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    std::string name;
    
    void Draw(Shader& shader);

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, glm::mat3 scale = glm::mat3{1.f});
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

    static int NEXT_ID;
    
    glm::mat4 modelMat{1.f};
    
    glm::vec3 location{0.f};
    glm::vec3 scale{1.f};
    glm::vec3 rotation{0.f};
};

GLuint TextureFromFile(const char *path, const std::string& directory, bool gamma = false);
GLuint CubemapFromFile(const std::vector<std::string> &faces);