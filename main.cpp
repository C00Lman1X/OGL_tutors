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
const GLchar *BASIC_FRAGMENT_SHADER = "\
#version 330 core\n\
out vec4 FragColor;\n\
\n\
void main()\n\
{\n\
	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); \n\
}";
const GLchar *BASIC_FRAGMENT_SHADER_YELLOW = "\
#version 330 core\n\
out vec4 FragColor;\n\
\n\
void main()\n\
{\n\
	FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f); \n\
}";

GLenum mode = GL_LINE;

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
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		if (mode == GL_LINE)
			mode = GL_FILL;
		else
			mode = GL_LINE;
		glPolygonMode(GL_FRONT_AND_BACK, mode);
	}
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

	GLuint VAO[2];
	glGenVertexArrays(2, VAO);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &BASIC_VERTEX_SHADER, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &BASIC_FRAGMENT_SHADER, NULL);
	glCompileShader(fragmentShader);

	GLuint fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderYellow, 1, &BASIC_FRAGMENT_SHADER_YELLOW, NULL);
	glCompileShader(fragmentShaderYellow);

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

	GLuint shaderProgramYellow = glCreateProgram();
	glAttachShader(shaderProgramYellow, vertexShader);
	glAttachShader(shaderProgramYellow, fragmentShaderYellow);
	glLinkProgram(shaderProgramYellow);

	GLfloat vertices[2][9] = {
		{
			-1.f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f,
		  	0.f, -0.5f, 0.0f
		},
		{
			0.f, 0.5f, 0.0f,
		 	0.5f, -0.5f, 0.0f,
		  	1.f, 0.5f, 0.0f
		}
	};
	GLuint VBO[2];
	glGenBuffers(2, VBO);
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]), vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[1]), vertices[1], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

	GLuint indices[] = {
		0, 1, 2,
		2, 3, 4
	};
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	while(!glfwWindowShouldClose(wnd))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(shaderProgramYellow);
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glfwTerminate();
	return 0;
}