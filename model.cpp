#include "model.h"
#include "mesh.h"
#include "shader.h"
#include "stb_image.h"
#include "globalData.h"
#include <glm/gtc/matrix_transform.hpp>

int Model::NEXT_ID = 0;

void Model::SetLocation(const glm::vec3& location)
{
    this->location = location;
    if (transparentCube)
        SortFaces();
}

void Model::SetScale(const glm::vec3& scale)
{
    this->scale = scale;
    if (transparentCube)
        SortFaces();
}

void Model::SetRotation(const glm::vec3& rotation)
{
    this->rotation = rotation;
    if (transparentCube)
        SortFaces();
}

Model::Model(const Mesh& mesh, int shaderId, glm::vec3 location_/* = {0.f, 0.f, 0.f}*/, glm::vec3 scale_/* = {1.f, 1.f, 1.f}*/, glm::vec3 rotation_/* = {0.f, 0.f, 0.f}*/)
    : location(location_)
    , scale(scale_)
    , rotation(rotation_)
    , shaderID(shaderId)
    , ID(NEXT_ID++)
{
    meshes.push_back(std::move(mesh));

    name = std::to_string(ID) + "_";
}

void Model::Draw(Shader& shader)
{
    for(GLuint i = 0; i < meshes.size(); ++i)
        meshes[i].Draw(shader);
}

void Model::DrawPointLight()
{
    Shader& shader = DATA.shadersManager.GetShader(shaderID);
    shader.use();
    shader.set("color", color);

    modelMat = glm::mat4{1.f};
    modelMat = glm::translate(modelMat, location);
    modelMat = glm::scale(modelMat, scale);
    modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1.f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0.f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0.f, 0.0f, 1.0f));
    shader.set("model", modelMat);

    Draw(shader);
}

void Model::DrawSpotLight(float angle, glm::vec3 axis)
{
    Shader& shader = DATA.shadersManager.GetShader(shaderID);
    shader.use();
    shader.set("color", color);

    modelMat = glm::mat4{1.f};
    modelMat = glm::translate(modelMat, location);
    modelMat = glm::scale(modelMat, scale);
    modelMat = glm::rotate(modelMat, angle, axis);
    shader.set("model", modelMat);

    Draw(shader);
}

void Model::DrawModel()
{
    Shader& shader = DATA.shadersManager.GetShader(shaderID);
    shader.use();
    shader.set("isSolidColor", solidColor);
    shader.set("color", color);
    shader.set("material.shininess", shininess);
    shader.set("opaque", opaque);

    modelMat = glm::mat4{1.f};
    modelMat = glm::translate(modelMat, location);
    modelMat = glm::scale(modelMat, scale);
    modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1.f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0.f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0.f, 0.0f, 1.0f));
    shader.set("model", modelMat);

    Draw(shader);
}

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    glm::mat3 scaleMat{1.f};
    for(int i = 0; i < 3; ++i)
    {
        if (node->mTransformation[i][i] != 1.f)
        {
            scaleMat[i][i] = node->mTransformation[i][i];
        }
    }
    for(GLuint i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, scaleMat));
    }

    for(GLuint i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, glm::mat3 scale/* = glm::mat3{1.f}*/)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    for(GLuint i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.Position = scale * vertex.Position;
        vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        if (mesh->mTextureCoords[0])
            vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        else
            vertex.TexCoords = {0.f, 0.f};
        
        vertices.push_back(vertex);
    }
    
    for(GLuint i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for(GLuint j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(GLuint i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(GLuint j = 0; j < textures_loaded.size() && !skip; ++j)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
            }
        }
        if (!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

void Model::ChangeName(const std::string& newName)
{
    name = std::to_string(ID) + '_' + newName;
}

GLuint TextureFromFile(const char *path, const std::string& directory, bool gamma/* = false*/)
{
    std::string filename(path);
    filename = directory + '/' + filename;

	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	GLubyte *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (!data)
    {
        filename = path;
        filename = "textures/" + filename;
        data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    }
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Texture failed to load at path: " << filename.c_str() << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Model::SortFaces()
{
    Mesh& cubeMesh = meshes[0];
    const size_t facesCount = 6;
    const size_t faceIndicesCount = cubeMesh.indices.size() / facesCount;
    std::vector<std::pair<bool, size_t>> sortedFaces;
    for(size_t face = 0; face < facesCount; ++face)
    {
        const size_t firstIndexOfFace = face*facesCount;
        const glm::vec3 faceNorm = glm::mat3(glm::transpose(glm::inverse(modelMat))) * cubeMesh.vertices[firstIndexOfFace].Normal;
        const glm::vec3 vertexWorldPosition = glm::vec3{modelMat * glm::vec4{cubeMesh.vertices[firstIndexOfFace].Position, 1.f}};
        const glm::vec3 dirToCam = glm::normalize(DATA.camera.Position - vertexWorldPosition);
        bool lookToCam = glm::dot(faceNorm, dirToCam) >= 0.f;
        sortedFaces.emplace_back(lookToCam, face);
    }
    std::sort(sortedFaces.begin(), sortedFaces.end(), [](const std::pair<bool, size_t>& p1, const std::pair<bool, size_t>& p2) {
        return p1.first < p2.first;
    });

    cubeMesh.indices.clear();
    for(auto& p : sortedFaces)
    {
        for(size_t i = 0; i < faceIndicesCount; ++i)
            cubeMesh.indices.push_back(GLuint(p.second * faceIndicesCount + i));
    }
}