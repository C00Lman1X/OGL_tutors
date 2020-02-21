#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "shader.h"
#include "stb_image.h"
#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globalData.h"
const glm::vec3 GlobalData::DEFAULT_CAMERA_POS{0.f, 0.f, 3.f};

void processInput(GLFWwindow *window, float dt);
void mouse_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double dx, double dy);
GLuint loadTexture(const char *path);

void SetLights(Shader& shader);

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
		if (DATA.mode == GL_LINE)
			DATA.mode = GL_FILL;
		else
			DATA.mode = GL_LINE;
		glPolygonMode(GL_FRONT_AND_BACK, DATA.mode);
	}
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		DATA.camera.Position = GlobalData::DEFAULT_CAMERA_POS;
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		DATA.drawLight = !DATA.drawLight;
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

	Model model("C:\\Users\\aliaksei.dziakanchuk\\Documents\\OGL_tutors\\nanosuit\\nanosuit.obj");
	Model model2("C:\\Users\\aliaksei.dziakanchuk\\Documents\\OGL_tutors\\shapes\\sphere.nff");

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

		//SetLights(shader);

		glm::mat4 view = DATA.camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(DATA.camera.Zoom), 640.f / 480.f, 0.1f, 100.f);
		//projection = glm::mat4(1.f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		model.Draw(shader);

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window, float dt)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::FORWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::BACKWARD, dt);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::LEFT, dt);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::RIGHT, dt);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::UP, dt);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		DATA.camera.ProcessKeyboard(Camera_Movement::DOWN, dt);

	/*
	float speed = 4.f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		DATA.objectPos.z -= speed * dt;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		DATA.objectPos.z += speed * dt;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		DATA.objectPos.x += speed * dt;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		DATA.objectPos.x -= speed * dt;
	*/
}

void mouse_callback(GLFWwindow *window, double x, double y)
{
	static bool firstMouse = true;
	if (firstMouse)
	{
		DATA.lastX = x;
		DATA.lastY = y;
		firstMouse = false;
	}

	float dx = (float)(x - DATA.lastX);
	float dy = (float)(DATA.lastY - y);
	DATA.lastX = x;
	DATA.lastY = y;
	
	DATA.camera.ProcessMouseMovement(dx, dy);
}

void scroll_callback(GLFWwindow *window, double dx, double dy)
{
	DATA.camera.ProcessMouseScroll((float)dy);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			DATA.camera.ResetZoom();
	}
}
/*
void SetLights(Shader& shader)
{
	int pointLightsCount = 0, dirLightsCount = 0, spotLightsCount = 0;
	for(Light light : DATA.lights)
	{
		switch (light.type)
		{
		case 0:
		{
			std::string lightName = "pointLights[" + std::to_string(pointLightsCount++) + "]";
			shader.
		}
			break;
		case 1:
			lightName = 
		
		default:
			break;
		}
	}
}
*/