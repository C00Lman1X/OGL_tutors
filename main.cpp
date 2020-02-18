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
glm::vec3 objectPos(0.f, 0.f, 0.f);
double lastX = 320.0, lastY = 240.0;
bool drawLight = true;
Camera camera(glm::vec3(0.f, 0.f, 3.f));

void processInput(GLFWwindow *window, float dt);
void mouse_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double dx, double dy);
GLuint loadTexture(const char *path);

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
	glfwSetMouseButtonCallback(wnd, mouse_button_callback);

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

	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
		glm::vec3( 0.7f,  0.2f,  2.0f),
		glm::vec3( 2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3( 0.0f,  0.0f, -3.0f)
	};

	GLfloat vertices[] = {
		//abc
		-1.f, 1.618f, 0.f, 	0.f, 3.236f, 1.236f,	0.f, 0.f,
		1.f, 1.618f, 0.f, 	0.f, 3.236f, 1.236f,	0.f, 1.f,
		0.f, 1.f, 1.618f, 	0.f, 3.236f, 1.236f,	1.f, 0.f,
		//abg
		-1.f, 1.618f, 0.f, 	0.f, 3.236f, -1.236f,	0.f, 0.f,
		1.f, 1.618f, 0.f, 	0.f, 3.236f, -1.236f,	0.f, 1.f,
		0.f, 1.f, -1.618f, 	0.f, 3.236f, -1.236f,	1.f, 0.f,
		//bce
		1.f, 1.618f, 0.f, 	2.f, 2.f, 2.f,			0.f, 0.f,
		0.f, 1.f, 1.618f, 	2.f, 2.f, 2.f,			0.f, 1.f,
		1.618f, 0.f, 1.f, 	2.f, 2.f, 2.f,			1.f, 0.f,
		//bef
		1.f, 1.618f, 0.f, 	3.236f, 1.236f, 0.f,	0.f, 0.f,
		1.618f, 0.f, 1.f, 	3.236f, 1.236f, 0.f,	0.f, 1.f,
		1.618f, 0.f, -1.f, 	3.236f, 1.236f, 0.f,	1.f, 0.f,
		//bfg
		1.f, 1.618f, 0.f, 	2.f, 2.f, -2.f,			0.f, 0.f,
		1.618f, 0.f, -1.f, 	2.f, 2.f, -2.f,			0.f, 1.f,
		0.f, 1.f, -1.618f, 	2.f, 2.f, -2.f,			1.f, 0.f,
		//agl
		-1.f, 1.618f, 0.f, 	-2.f, 2.f, -2.f,		0.f, 0.f,
		0.f, 1.f, -1.618f, 	-2.f, 2.f, -2.f,		0.f, 1.f,
		-1.618f, 0.f, -1.f, -2.f, 2.f, -2.f,		1.f, 0.f,
		//akl
		-1.f, 1.618f, 0.f, 	-3.236f, 1.236f, 0.f,	0.f, 0.f,
		-1.618f, 0.f, 1.f, 	-3.236f, 1.236f, 0.f,	0.f, 1.f,
		-1.618f, 0.f, -1.f, -3.236f, 1.236f, 0.f,	1.f, 0.f,
		//akc
		-1.f, 1.618f, 0.f, 	-2.f, 2.f, 2.f,			0.f, 0.f,
		-1.618f, 0.f, 1.f, 	-2.f, 2.f, 2.f,			0.f, 1.f,
		0.f, 1.f, 1.618f, 	-2.f, 2.f, 2.f,			1.f, 0.f,
		//ckd
		0.f, 1.f, 1.618f, 	-1.236f, 0.f, 3.236f,	0.f, 0.f,
		-1.618f, 0.f, 1.f, 	-1.236f, 0.f, 3.236f,	0.f, 1.f,
		0.f, -1.f, 1.618f, 	-1.236f, 0.f, 3.236f,	1.f, 0.f,
		//cde
		0.f, 1.f, 1.618f, 	1.236f, 0.f, 3.236f,	0.f, 0.f,
		0.f, -1.f, 1.618f, 	1.236f, 0.f, 3.236f,	0.f, 1.f,
		1.618f, 0.f, 1.f, 	1.236f, 0.f, 3.236f,	1.f, 0.f,
		//glh
		0.f, 1.f, -1.618f, 	-1.236f, 0.f, -3.236f,	0.f, 0.f,
		-1.618f, 0.f, -1.f, 	-1.236f, 0.f, -3.236f,	0.f, 1.f,
		0.f, -1.f, -1.618f, 	-1.236f, 0.f, -3.236f,	1.f, 0.f,
		//ghf
		0.f, 1.f, -1.618f, 	1.236f, 0.f, -3.236f,	0.f, 0.f,
		0.f, -1.f, -1.618f, 	1.236f, 0.f, -3.236f,	0.f, 1.f,
		1.618f, 0.f, -1.f,	1.236f, 0.f, -3.236f,	1.f, 0.f,
		//dkj
		0.f, -1.f, 1.618f, 	-2.f, -2.f, 2.f,		0.f, 0.f,
		-1.618f, 0.f, 1.f, 	-2.f, -2.f, 2.f,		0.f, 1.f,
		-1.f, -1.618f, 0.f, 	-2.f, -2.f, 2.f,		1.f, 0.f,
		//jkl
		-1.f, -1.618f, 0.f, 	-3.236f, -1.236f, 0.f,	0.f, 0.f,
		-1.618f, 0.f, 1.f, 	-3.236f, -1.236f, 0.f,	0.f, 1.f,
		-1.618f, 0.f, -1.f, 	-3.236f, -1.236f, 0.f,	1.f, 0.f,
		//jlh
		-1.f, -1.618f, 0.f, 	-2.f, -2.f, -2.f,		0.f, 0.f,
		-1.618f, 0.f, -1.f, 	-2.f, -2.f, -2.f,		0.f, 1.f,
		0.f, -1.f, -1.618f, 	-2.f, -2.f, -2.f,		1.f, 0.f,
		//dji
		0.f, -1.f, 1.618f, 	0.f, -3.236f, 1.236f,	0.f, 0.f,
		-1.f, -1.618f, 0.f, 	0.f, -3.236f, 1.236f,	0.f, 1.f,
		1.f, -1.618f, 0.f, 	0.f, -3.236f, 1.236f,	1.f, 0.f,
		//jih
		-1.f, -1.618f, 0.f, 	0.f, -3.236f, -1.236f,	0.f, 0.f,
		1.f, -1.618f, 0.f, 	0.f, -3.236f, -1.236f,	0.f, 1.f,
		0.f, -1.f, -1.618f, 	0.f, -3.236f, -1.236f,	1.f, 0.f,
		//dei
		0.f, -1.f, 1.618f, 	2.f, -2.f, 2.f,			0.f, 0.f,
		1.618f, 0.f, 1.f, 	2.f, -2.f, 2.f,			0.f, 1.f,
		1.f, -1.618f, 0.f, 	2.f, -2.f, 2.f,			1.f, 0.f,
		//eif
		1.618f, 0.f, 1.f, 	3.236f, -1.236f, 0.f,	0.f, 0.f,
		1.f, -1.618f, 0.f, 	3.236f, -1.236f, 0.f,	0.f, 1.f,
		1.618f, 0.f, -1.f, 	3.236f, -1.236f, 0.f,	1.f, 0.f,
		//ifh
		1.f, -1.618f, 0.f, 	2.f, -2.f, -2.f,		0.f, 0.f,
		1.618f, 0.f, -1.f, 	2.f, -2.f, -2.f,		0.f, 1.f,
		0.f, -1.f, -1.618f, 	2.f, -2.f, -2.f,		1.f, 0.f,
	};
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));

	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	GLuint diffuseMap = loadTexture("textures/container2.png");
	GLuint specularMap = loadTexture("textures/container2_specular.png");

	float dt = 0.f;
	float lastFrame = 0.f;
	while(!glfwWindowShouldClose(wnd))
	{
		float currentFrame = (float)glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(wnd, dt);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.setInt("material.diffuse", 0);
		shader.setInt("material.specular", 1);
		shader.setFloat("material.shininess", 32.f);

		for (int i = 0; i < 4; ++i)
		{
			std::string lightName = "pointLights[";
			lightName += char('0' + i);
			lightName += "]";
			shader.setVec3((lightName + ".ambient").c_str(), 0.1f, 0.1f, 0.1f);
			shader.setVec3((lightName + ".diffuse").c_str(), 0.5f, 0.5f, 0.5f);
			shader.setVec3((lightName + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
			shader.setVec3((lightName + ".position").c_str(), pointLightPositions[i]);
			shader.setFloat((lightName + ".constant").c_str(), 1.0f);
			shader.setFloat((lightName + ".linear").c_str(), 0.09f);
			shader.setFloat((lightName + ".quadratic").c_str(), 0.032f);
		}
		shader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
		shader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
		shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		shader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
		shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
		shader.setVec3("spotLight.direction", camera.Front);
		shader.setVec3("viewPos", camera.Position);

		// 0, 0, 0,809
		glm::vec3 viewDir = glm::normalize(camera.Position - glm::vec3(0.5f, 0.809f, 0.f));
		std::cout << viewDir.x << " " << viewDir.y << " " << viewDir.z << std::endl;
		std::cout << camera.Front.x << " " << camera.Front.y << " " << camera.Front.z << std::endl;
		std::cout << glm::dot(viewDir, -camera.Front) << std::endl;


		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 640.f / 480.f, 0.1f, 100.f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		glBindVertexArray(VAO);
		for(GLuint i = 0; i < 10; i++)
		{
			glm::mat4 model(1.f);
			model = glm::translate(model, objectPos + cubePositions[i]);
			model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
			float angle = 20.f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, 0.3f, 0.5f));
			shader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 60);
		}

		if (drawLight)
		{
			lampShader.use();
			lampShader.setMat4("view", view);
			lampShader.setMat4("projection", projection);
			for(int i = 0; i < 4; i++)
			{
				glm::mat4 model(1.f);
				model = glm::translate(model, pointLightPositions[i]);
				model = glm::scale(model, glm::vec3(0.2f));
				lampShader.setMat4("model", model);
				glBindVertexArray(lightVAO);
				glDrawArrays(GL_TRIANGLES, 0, 60);
			}
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

	float dx = (float)(x - lastX);
	float dy = (float)(lastY - y);
	lastX = x;
	lastY = y;
	
	camera.ProcessMouseMovement(dx, dy);
}

void scroll_callback(GLFWwindow *window, double dx, double dy)
{
	camera.ProcessMouseScroll((float)dy);
}

GLuint loadTexture(const char *path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			camera.ResetZoom();
	}
}