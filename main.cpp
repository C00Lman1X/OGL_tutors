#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "shader.h"
#include "camera.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLenum mode = GL_FILL;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
float lastX = 320, lastY = 240;
Camera camera(glm::vec3(0.f, 0.f, 3.f));

void processInput(GLFWwindow *window, float dt);
void mouse_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double dx, double dy);

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
	glfwSetCursorPosCallback(wnd, mouse_callback);
	glfwSetScrollCallback(wnd, scroll_callback);

	glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(wnd);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 640, 480);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader lampShader("shaders/vertex.glsl", "shaders/fragment_lamp.glsl");

	GLfloat vertices[] = {
		-1,      1.618,  0,     // a 0
		 1,      1.618,  0 ,    // b 1
		 0,      1,      1.618, // c 2
		 0,     -1,      1.618, // d 3
		 1.618,  0,      1,     // e 4
		 1.618,  0,     -1,     // f 5
		 0,      1,     -1.618, // g 6
		 0,     -1,     -1.618, // h 7
		 1,     -1.618,  0,     // i 8
		-1,     -1.618,  0,     // j 9
		-1.618,  0,      1,     // k 10
		-1.618,  0,     -1      // l 11
	};
	GLuint indices[] = {
		0, 1, 2,  //abc
		0, 1, 6,  //abg
		1, 2, 4,  //bce
		1, 4, 5,  //bef
		1, 5, 6,  //bfg
		0, 6, 11, //agl
		0, 10,11, //akl
		0, 10,2,  //akc
		2, 10,3,  //ckd
		2, 3, 4,  //cde
		6, 11,7,  //glh
		6, 7, 5,  //ghf
		3, 10,9,  //dkj
		9, 10,11, //jkl
		9, 11,7,  //jlh
		3, 9, 8,  //dji
		9, 8, 7,  //jih
		3, 4, 8,  //dei
		4, 8, 5,  //eif
		8, 5, 7,  //ifh
	};
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(0);

	float dt = 0.f;
	float lastFrame = 0.f;
	while(!glfwWindowShouldClose(wnd))
	{
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(wnd, dt);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), 640.f / 480.f, 0.1f, 100.f);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		glm::mat4 model(1.f);
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

		lampShader.use();
		glUniformMatrix4fv(glGetUniformLocation(lampShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lampShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lampShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		model = glm::mat4(1.f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		glBindVertexArray(lightVAO);
		glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window, float dt)
{
	float camSpeed = 5.f * dt;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, dt);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, dt);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::UP, dt);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::DOWN, dt);
}

void mouse_callback(GLFWwindow *window, double x, double y)
{
	static bool firstMouse = true;
	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float dx = x - lastX;
	float dy = lastY - y;
	lastX = x;
	lastY = y;
	
	camera.ProcessMouseMovement(dx, dy);
}

void scroll_callback(GLFWwindow *window, double dx, double dy)
{
	camera.ProcessMouseScroll(dy);
}