#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const GLchar* BASIC_VERTEX_SHADER = "\
#version 330 core\n\
layout(location = 0) in vec3 aPos;\n\
\n\
void main()\n\
{\n\
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
}";
const GLchar* BASIC_FRAGMENT_SHADER = "\
#version 330 core\n\
out vec4 FragColor;\n\
\n\
void main()\n\
{\n\
	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); \n\
}";

void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* wnd, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char ** argv)
{
    GLFWwindow* wnd;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // global callbacks
    glfwSetErrorCallback(error_callback);

    wnd = glfwCreateWindow(640, 480, "Yo, GLFW!", NULL, NULL);
    if (!wnd)
    {
        glfwTerminate();
        return -1;
    }

    // window callbacks
    glfwSetKeyCallback(wnd, key_callback);
    glfwSetFramebufferSizeCallback(wnd, framebuffer_size_callback);

    glfwMakeContextCurrent(wnd);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 640, 480);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &BASIC_VERTEX_SHADER, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &BASIC_FRAGMENT_SHADER, NULL);
	glCompileShader(fragmentShader);

/*	shader compile check */
// 	int  success;
// 	char infoLog[512];
// 	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
// 	if (!success)
// 	{
// 		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
// 		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
// 	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(wnd))
    {
        glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(wnd);
        glfwPollEvents();
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

    glfwTerminate();
    return 0;
}