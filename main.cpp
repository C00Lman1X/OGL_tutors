#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
	DATA.width = (float)width;
	DATA.height = (float)height;
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

	wnd = glfwCreateWindow((int)DATA.width, (int)DATA.height, "Yo, GLFW!", NULL, NULL);
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

	glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwMakeContextCurrent(wnd);
	
	if (glfwRawMouseMotionSupported())
    	glfwSetInputMode(wnd, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, (GLsizei)DATA.width, (GLsizei)DATA.height);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(wnd, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader lightShader("shaders/vertex_lamp.glsl", "shaders/fragment_lamp.glsl");

	std::vector<Model> models;
	Model model("nanosuit\\nanosuit.obj");
	model.location = {0.f, 0.f, -3.f};
	models.push_back(model);

	Model sphereModel("shapes\\sphere.nff");
	sphereModel.solidColor = true;
	sphereModel.color = {1.f, 0.5f, 0.f};
	models.push_back(sphereModel);
	
	Model coneModel("shapes\\cone.nff");

	Light dirLight;
	dirLight.type = 1;
	DATA.lights.push_back(dirLight);

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

		shader.setVec3("viewPos", DATA.camera.Position);
		SetLights(shader);

		glm::mat4 view = DATA.camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(DATA.camera.Zoom), DATA.width / DATA.height, 0.1f, 100.f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		for(Model& model : models)
			model.Draw(shader);

		auto lightToDelete = DATA.lights.end();
		int lightTypeToAdd = -1;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		{
			ImGui::Begin("Lights");
			int i = 0;
			for(auto itLight = DATA.lights.begin(); itLight != DATA.lights.end(); ++itLight)
			{
				ImGui::PushID(++i);
				Light& light = *itLight;
				if (light.type == 0)
				{
					ImGui::Text("Point");
					ImGui::Indent();
					ImGui::DragFloat3("location", (float*)&light.location, 0.01f);
				}
				else if (light.type == 1)
				{
					ImGui::Text("Directional");
					ImGui::Indent();
					ImGui::DragFloat3("direction", (float*)&light.direction, 0.01f, -1.0f, 1.0f);
				}
				else if (light.type == 2)
				{
					ImGui::Text("Spot");
					ImGui::Indent();
					ImGui::DragFloat3("location", (float*)&light.location, 0.01f);
					ImGui::DragFloat3("direction", (float*)&light.direction, 0.01f, -1.0f, 1.0f);
					ImGui::DragFloat("innerCutOff", &light.innerCutOff, 0.001f, 0.f, 1.f);
					ImGui::DragFloat("outerCutOff", &light.outerCutOff, 0.001f, 0.f, 1.f);
				}
				ImGui::ColorEdit3("ambient", (float*)&light.ambient, ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("diffuse", (float*)&light.diffuse, ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("specular", (float*)&light.specular, ImGuiColorEditFlags_Float);
				if (ImGui::Button("Delete light"))
					lightToDelete = itLight;

				ImGui::Unindent();
				ImGui::PopID();
			}

			if (ImGui::Button("AddPointLight"))
			{
				lightTypeToAdd = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("AddDirectionalLight"))
			{
				lightTypeToAdd = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("AddSpotLight"))
			{
				lightTypeToAdd = 2;
			}

			std::stringstream sstr;
			sstr << "lastX: " << DATA.lastX << "  lastY: " << DATA.lastY;
			ImGui::Text(sstr.str().c_str());

			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		lightShader.use();
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		for(Light& light : DATA.lights)
		{
			if (light.type == 0)
			{
				sphereModel.location = light.location;
				sphereModel.color = light.diffuse;
				sphereModel.scale = {0.1f, 0.1f, 0.1f};
				sphereModel.Draw(lightShader, true);
			}
			else if (light.type == 2)
			{
				coneModel.color = light.diffuse;
				coneModel.scale = {0.2f, 0.2f, 0.2f};
				coneModel.location = light.location;

				glm::vec3 axis = glm::cross({0.f, 1.f, 0.f}, glm::normalize(light.direction));
				float angle = glm::acos(glm::dot({0.f, 1.f, 0.f}, glm::normalize(light.direction)));
				coneModel.Draw(lightShader, true, angle, axis);
			}
		}

		glfwSwapBuffers(wnd);
		glfwPollEvents();

		if (lightToDelete != DATA.lights.end())
			DATA.lights.erase(lightToDelete);
		if (lightTypeToAdd != -1)
			DATA.lights.emplace_back(lightTypeToAdd);
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window, float dt)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;
		
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
	ImGuiIO& io = ImGui::GetIO();
	
	if (DATA.cursorCaptured)
	{
		float dx = (float)(x - DATA.lastX);
		float dy = (float)(DATA.lastY - y);
		DATA.lastX = x;
		DATA.lastY = y;
		DATA.camera.ProcessMouseMovement(dx, dy);
	}
}

void scroll_callback(GLFWwindow *window, double dx, double dy)
{
	DATA.camera.ProcessMouseScroll((float)dy);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS && !io.WantCaptureMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		DATA.cursorCaptured = true;
		glfwGetCursorPos(window, &DATA.lastX, &DATA.lastY);
	}
	if (action == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		DATA.cursorCaptured = false;
	}
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			DATA.camera.ResetZoom();
	}
}

void SetLights(Shader& shader)
{
	int pointLightsCount = 0, dirLightsCount = 0, spotLightsCount = 0;
	for(const Light& light : DATA.lights)
	{
		std::string lightName;
		switch (light.type)
		{
		case 0:
		{
			lightName = "pointLights[" + std::to_string(pointLightsCount++) + "]";
			shader.setVec3(lightName + ".position", light.location);
			shader.setFloat(lightName + ".constant", light.constant);
			shader.setFloat(lightName + ".linear", light.linear);
			shader.setFloat(lightName + ".quadratic", light.quadratic);
			break;
		}
		case 1:
		{
			lightName = "dirLights[" + std::to_string(dirLightsCount++) + "]";
			shader.setVec3(lightName + ".direction", light.direction);
			break;
		}
		case 2:
		{
			lightName = "spotLights[" + std::to_string(spotLightsCount++) + "]";
			shader.setFloat(lightName + ".innerCutOff", light.innerCutOff);
			shader.setFloat(lightName + ".outerCutOff", light.outerCutOff);
			shader.setVec3(lightName + ".direction", light.direction);
			shader.setVec3(lightName + ".position", light.location);
			break;
		}
		
		default:
			break;
		}

		shader.setVec3(lightName + ".ambient", light.ambient);
		shader.setVec3(lightName + ".diffuse", light.diffuse);
		shader.setVec3(lightName + ".specular", light.specular);
	}

	shader.setInt("dirLightsCount", dirLightsCount);
	shader.setInt("pointLightsCount", pointLightsCount);
	shader.setInt("spotLightsCount", spotLightsCount);
}