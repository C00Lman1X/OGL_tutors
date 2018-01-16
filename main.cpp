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
glm::vec3 objectPos(0.0f, 0.0f, 0.0f);
float lastX = 320, lastY = 240;
bool drawLight = true;
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
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		camera.Position = lightPos;
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		drawLight = !drawLight;
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
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader lampShader("shaders/vertex_lamp.glsl", "shaders/fragment_lamp.glsl");

	GLfloat vertices[] = {
		-1, 1.618, 0, 	0.f, 3.236f, 1.236f,	1.f,
		1, 1.618, 0, 	0.f, 3.236f, 1.236f,	1.f,
		0, 1, 1.618, 	0.f, 3.236f, 1.236f,	1.f,//abc
		-1, 1.618, 0, 	0.f, 3.236f, -1.236f,	2.f,
		1, 1.618, 0, 	0.f, 3.236f, -1.236f,	2.f,
		0, 1, -1.618, 	0.f, 3.236f, -1.236f,	2.f, //abg
		1, 1.618, 0, 	2.f, 2.f, 2.f,			3.f,
		0, 1, 1.618, 	2.f, 2.f, 2.f,			3.f,
		1.618, 0, 1, 	2.f, 2.f, 2.f,			3.f, //bce
		1, 1.618, 0, 	3.236f, 1.236f, 0.f,	4.f,
		1.618, 0, 1, 	3.236f, 1.236f, 0.f,	4.f,
		1.618, 0, -1, 	3.236f, 1.236f, 0.f,	4.f, //bef
		1, 1.618, 0, 	2.f, 2.f, -2.f,			5.f,
		1.618, 0, -1, 	2.f, 2.f, -2.f,			5.f,
		0, 1, -1.618, 	2.f, 2.f, -2.f,			5.f, //bfg
		-1, 1.618, 0, 	-2.f, 2.f, -2.f,		6.f,
		0, 1, -1.618, 	-2.f, 2.f, -2.f,		6.f,
		-1.618, 0, -1, 	-2.f, 2.f, -2.f,		6.f, //agl
		-1, 1.618, 0, 	-3.236f, 1.236f, 0.f,	7.f,
		-1.618, 0, 1, 	-3.236f, 1.236f, 0.f,	7.f,
		-1.618, 0, -1, 	-3.236f, 1.236f, 0.f,	7.f, //akl
		-1, 1.618, 0, 	-2.f, 2.f, 2.f,			8.f,
		-1.618, 0, 1, 	-2.f, 2.f, 2.f,			8.f,
		0, 1, 1.618, 	-2.f, 2.f, 2.f,			8.f, //akc
		0, 1, 1.618, 	-1.236f, 0.f, 3.236f,	9.f,
		-1.618, 0, 1, 	-1.236f, 0.f, 3.236f,	9.f,
		0, -1, 1.618, 	-1.236f, 0.f, 3.236f,	9.f, //ckd
		0, 1, 1.618, 	1.236f, 0.f, 3.236f,	10.f,
		0, -1, 1.618, 	1.236f, 0.f, 3.236f,	10.f,
		1.618, 0, 1, 	1.236f, 0.f, 3.236f,	10.f, //cde
		0, 1, -1.618, 	-1.236f, 0.f, -3.236f,	11.f,
		-1.618, 0, -1, 	-1.236f, 0.f, -3.236f,	11.f,
		0, -1, -1.618, 	-1.236f, 0.f, -3.236f,	11.f, //glh
		0, 1, -1.618, 	1.236f, 0.f, -3.236f,	12.f,
		0, -1, -1.618, 	1.236f, 0.f, -3.236f,	12.f,
		1.618, 0, -1,	1.236f, 0.f, -3.236f,	12.f, //ghf
		0, -1, 1.618, 	-2.f, -2.f, 2.f,		13.f,
		-1.618, 0, 1, 	-2.f, -2.f, 2.f,		13.f,
		-1, -1.618, 0, 	-2.f, -2.f, 2.f,		13.f, //dkj
		-1, -1.618, 0, 	-3.236f, -1.236f, 0.f,	14.f,
		-1.618, 0, 1, 	-3.236f, -1.236f, 0.f,	14.f,
		-1.618, 0, -1, 	-3.236f, -1.236f, 0.f,	14.f, //jkl
		-1, -1.618, 0, 	-2.f, -2.f, -2.f,		15.f,
		-1.618, 0, -1, 	-2.f, -2.f, -2.f,		15.f,
		0, -1, -1.618, 	-2.f, -2.f, -2.f,		15.f, //jlh
		0, -1, 1.618, 	0.f, -3.236f, 1.236f,	16.f,
		-1, -1.618, 0, 	0.f, -3.236f, 1.236f,	16.f,
		1, -1.618, 0, 	0.f, -3.236f, 1.236f,	16.f, //dji
		-1, -1.618, 0, 	0.f, -3.236f, -1.236f,	17.f,
		1, -1.618, 0, 	0.f, -3.236f, -1.236f,	17.f,
		0, -1, -1.618, 	0.f, -3.236f, -1.236f,	17.f, //jih
		0, -1, 1.618, 	2.f, -2.f, 2.f,			18.f,
		1.618, 0, 1, 	2.f, -2.f, 2.f,			18.f,
		1, -1.618, 0, 	2.f, -2.f, 2.f,			18.f, //dei
		1.618, 0, 1, 	3.236f, -1.236f, 0.f,	19.f,
		1, -1.618, 0, 	3.236f, -1.236f, 0.f,	19.f,
		1.618, 0, -1, 	3.236f, -1.236f, 0.f,	19.f, //eif
		1, -1.618, 0, 	2.f, -2.f, -2.f,		20.f,
		1.618, 0, -1, 	2.f, -2.f, -2.f,		20.f,
		0, -1, -1.618, 	2.f, -2.f, -2.f,		20.f //ifh
	};
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));

	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid *)0);
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
		shader.setVec3("material.ambient", 0.25f, 0.25f, 0.25f);
		shader.setVec3("material.diffuse", 0.4f, 0.4f, 0.4f);
		shader.setVec3("material.specular", 0.774597f, 0.774597f, 0.774597f);
		shader.setFloat("material.shininess", 32.f);
		shader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
		shader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 640.f / 480.f, 0.1f, 100.f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		glm::mat4 model(1.f);
		model = glm::translate(model, objectPos);
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		shader.setMat4("model", model);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 60);

		if (drawLight)
		{
			lampShader.use();
			lampShader.setMat4("view", view);
			lampShader.setMat4("projection", projection);
			model = glm::mat4(1.f);
			model = glm::translate(model, lightPos);
			model = glm::scale(model, glm::vec3(0.2f));
			lampShader.setMat4("model", model);
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 60);
		}

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window, float dt)
{
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

	float speed = 4.f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		objectPos.z -= speed * dt;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		objectPos.z += speed * dt;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		objectPos.x += speed * dt;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		objectPos.x -= speed * dt;
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