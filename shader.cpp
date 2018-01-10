#include "shader.h"

Shader::Shader(const GLchar *vertexPath, const GLchar *fragmentPath)
{
    char *vShaderCode = new char[4096];
    char *fShaderCode = new char[4096];
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertexPath);
		vShaderFile.seekg(0, vShaderFile.end);
		int vertexLength = vShaderFile.tellg();
		vShaderFile.seekg(0, vShaderFile.beg);
		fShaderFile.open(fragmentPath);
		fShaderFile.seekg(0, fShaderFile.end);
		int fragmentLength = fShaderFile.tellg();
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
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set4Float(const std::string &name, float f1, float f2, float f3, float f4) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), f1, f2, f3, f4);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}