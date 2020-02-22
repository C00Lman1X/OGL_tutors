#include "shader.h"
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const GLchar *vertexPath, const GLchar *fragmentPath)
{
    char *vShaderCode = new char[4096];
    char *fShaderCode = new char[8192];
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertexPath);
		vShaderFile.seekg(0, vShaderFile.end);
		std::streampos vertexLength = vShaderFile.tellg();
		vShaderFile.seekg(0, vShaderFile.beg);
		fShaderFile.open(fragmentPath);
		fShaderFile.seekg(0, fShaderFile.end);
		std::streampos fragmentLength = fShaderFile.tellg();
		fShaderFile.seekg(0, fShaderFile.beg);

        vShaderFile.read(vShaderCode, vertexLength);
		vShaderCode[vertexLength] = 0;
		fShaderFile.read(fShaderCode, fragmentLength);
		fShaderCode[fragmentLength] = 0;

        vShaderFile.close();
        fShaderFile.close();
    }
    catch(std::ifstream::failure e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    GLuint vertex, fragment;
    int success;
    GLchar infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    delete[] vShaderCode;
    delete[] fShaderCode;

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    GLint loc = getUniformLoc(name);
    glUniform1i(loc, (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    GLint loc = getUniformLoc(name);
    glUniform1i(loc, value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    GLint loc = getUniformLoc(name);
    glUniform1f(loc, value);
}

void Shader::set4Float(const std::string &name, float f1, float f2, float f3, float f4) const
{
    GLint loc = getUniformLoc(name);
    glUniform4f(loc, f1, f2, f3, f4);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    GLint loc = getUniformLoc(name);
    glUniform3f(loc, x, y, z);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const
{
    GLint loc = getUniformLoc(name);
    glUniform3f(loc, vec.x, vec.y, vec.z);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    GLint loc = getUniformLoc(name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

GLint Shader::getUniformLoc(const std::string &name) const
{
    auto it = checkedUniforms.find(name);
    if (it != checkedUniforms.end())
    {
        if (it->second)
            return glGetUniformLocation(ID, name.c_str());
        else
            return -1;
    }
    else
    {
        GLint loc = glGetUniformLocation(ID, name.c_str());
        if (loc == -1)
        {
            std::cerr << "ERROR::SHADER::PROGRAM::NO_UNIFORM " << name << std::endl;
            checkedUniforms[name] = false;
        }
        else
        {
            checkedUniforms[name] = true;
        }
        return loc;
    }
}