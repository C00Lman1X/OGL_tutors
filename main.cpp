#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "shader.h"

GLenum mode = GL_FILL;

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

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	//shader.use();
	//shader.set4Float("offset", 0.5f, 0.0f, 0.0f, 0.0f);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	GLfloat vertices[] = {
		// position         // colors
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
	};
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3*sizeof(GLfloat)));

	/*Element Buffer Object (EBO)*/
	// GLuint indices[] = {
	// 	0, 1, 2,
	// 	2, 3, 4
	// };
	// GLuint EBO;
	// glGenBuffers(1, &EBO);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	while(!glfwWindowShouldClose(wnd))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		double timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		shader.set4Float("globalColor", 0.0f, greenValue, 0.0f, 1.0f);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}