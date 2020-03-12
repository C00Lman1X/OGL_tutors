#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <array>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.h"
#include "ShadersManager.h"
#include "stb_image.h"
#include "model.h"
#include "Framebuffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "json.hpp"

using json = nlohmann::json;

#include "globalData.h"
const glm::vec3 GlobalData::DEFAULT_CAMERA_POS{0.f, 1.5f, 3.f};

void processInput(GLFWwindow *window, float dt);
void mouse_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double dx, double dy);
GLuint loadTexture(const char *path);
void SortModelsByDepth();

void SetLights();
void DrawGUI();
void LoadSceneFromJSON();

glm::vec3 getVec3(const std::vector<float> vec)
{
	return {vec[0], vec[1], vec[2]};
}

void error_callback(int error, const char *description)
{
	std::cerr << "Error: " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow *wnd, int width, int height)
{
	glViewport(0, 0, width, height);
	DATA.width = (float)width;
	DATA.height = (float)height;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

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

int main(int argc, char **argv)
{
	GLFWwindow *wnd;

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

	ShadersManager &shadersManager = DATA.shadersManager;

	glViewport(0, 0, (GLsizei)DATA.width, (GLsizei)DATA.height);
	glEnable(GL_BLEND);
	glSet(GL_CULL_FACE, DATA.faceCulling);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glDepthFunc(GL_LEQUAL);

	Framebuffer frameBuffer((GLsizei)DATA.width, (GLsizei)DATA.height, {0.1f, 0.1f, 0.1f, 1.0f});

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(wnd, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	int modelShaderID = shadersManager.CreateShader("shaders/vertex.glsl", "shaders/fragment.glsl");
	int lightShaderID = shadersManager.CreateShader("shaders/vertex_lamp.glsl", "shaders/fragment_lamp.glsl");
	int solidShaderID = shadersManager.CreateShader("shaders/vertex_solid.glsl", "shaders/fragment_solid.glsl");
	int textureShaderID = shadersManager.CreateShader("shaders/vertex_2D.glsl", "shaders/fragment_model.glsl");

	// screen shaders
	int screenShaderID = shadersManager.CreateShader("shaders/vertex_quad.glsl", "shaders/fragment_quad.glsl");
	DATA.SCREEN_SHADER_ID = screenShaderID;
	DATA.currentScreenShader = screenShaderID;
	int inverseShaderID = shadersManager.CreateShader("shaders/vertex_quad.glsl", "shaders/fragment_quad_inverse.glsl");
	int grayscaleShaderID = shadersManager.CreateShader("shaders/vertex_quad.glsl", "shaders/fragment_quad_grayscale.glsl");
	int kernelShaderID = shadersManager.CreateShader("shaders/vertex_quad.glsl", "shaders/fragment_quad_kernel.glsl");

	int skyboxShaderID = shadersManager.CreateShader("shaders/vertex_skybox.glsl", "shaders/fragment_skybox.glsl");

	Model spotLightModel("shapes/cone.nff", lightShaderID);
	Model pointLightModel("shapes/sphere.nff", lightShaderID);

	std::vector<std::string> faces{
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg"};
	GLuint cubemapTexture = CubemapFromFile(faces);
	std::array<float, 108> skyboxVertices = {
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f};
	
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1,&skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);	

	// Scene description >>>
	LoadSceneFromJSON();
	for (Model *model : DATA.models)
		DATA.unsortedModels.push_back(model);
	// Scene description <<<

	float dt = 0.f;
	float lastFrame = 0.f;
	while (!glfwWindowShouldClose(wnd))
	{
		float currentFrame = (float)glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(wnd, dt);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		frameBuffer.Use();

		SetLights();

		glm::mat4 view = DATA.camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(DATA.camera.Zoom), DATA.width / DATA.height, 0.1f, 100.f);
		shadersManager.set("view", view);
		shadersManager.set("projection", projection);
		shadersManager.set("viewPos", DATA.camera.Position);

		SortModelsByDepth();

		for (Model *model : DATA.models)
		{
			if (model->outline)
			{
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);
			}
			else
			{
				glStencilMask(0x00);
			}
			model->DrawModel();
		}

		for (Light &light : DATA.lights)
		{
			if (light.type == 0)
			{
				pointLightModel.SetLocation(light.location);
				pointLightModel.color = glm::vec4{light.diffuse, 1.f};
				pointLightModel.SetScale({0.1f, 0.1f, 0.1f});
				pointLightModel.DrawPointLight();
			}
			else if (light.type == 2)
			{
				spotLightModel.color = glm::vec4{light.diffuse, 1.f};
				spotLightModel.SetScale({0.2f, 0.2f, 0.2f});
				spotLightModel.SetLocation(light.location);

				static const glm::vec3 up{0.f, 1.f, 0.f};
				glm::vec3 axis = glm::cross(up, glm::normalize(light.direction));
				float angle = glm::acos(glm::dot(up, glm::normalize(light.direction)));
				spotLightModel.DrawSpotLight(angle, axis);
			}
		}
		
		glDepthMask(GL_FALSE);
		shadersManager.GetShader(skyboxShaderID).use();
		view = glm::mat4(glm::mat3(view));
		shadersManager.GetShader(skyboxShaderID).set("view", view);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		for (Model *model : DATA.models)
		{
			if (!model->outline)
				continue;

			// render outline >>>
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glDisable(GL_DEPTH_TEST);

			auto tmpShader = model->shaderID;
			model->shaderID = solidShaderID;

			model->DrawModel();

			model->shaderID = tmpShader;

			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			glEnable(GL_DEPTH_TEST);
			// render outline <<<
		}

		shadersManager.GetShader(DATA.currentScreenShader).use();
		frameBuffer.Draw();

		DrawGUI();

		glfwSwapBuffers(wnd);
		glfwPollEvents();
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
	ImGuiIO &io = ImGui::GetIO();

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
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	DATA.camera.ProcessMouseScroll((float)dy);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	ImGuiIO &io = ImGui::GetIO();
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

void SetLights()
{
	int pointLightsCount = 0, dirLightsCount = 0, spotLightsCount = 0;
	for (const Light &light : DATA.lights)
	{
		std::string lightName;
		switch (light.type)
		{
		case 0:
		{
			lightName = "pointLights[" + std::to_string(pointLightsCount++) + "]";
			DATA.shadersManager.set(lightName + ".position", light.location);
			DATA.shadersManager.set(lightName + ".constant", light.constant);
			DATA.shadersManager.set(lightName + ".linear", light.linear);
			DATA.shadersManager.set(lightName + ".quadratic", light.quadratic);
			break;
		}
		case 1:
		{
			lightName = "dirLights[" + std::to_string(dirLightsCount++) + "]";
			DATA.shadersManager.set(lightName + ".direction", light.direction);
			break;
		}
		case 2:
		{
			lightName = "spotLights[" + std::to_string(spotLightsCount++) + "]";
			DATA.shadersManager.set(lightName + ".innerCutOff", light.innerCutOff);
			DATA.shadersManager.set(lightName + ".outerCutOff", light.outerCutOff);
			DATA.shadersManager.set(lightName + ".direction", light.direction);
			DATA.shadersManager.set(lightName + ".position", light.location);
			break;
		}

		default:
			break;
		}

		DATA.shadersManager.set(lightName + ".ambient", light.ambient);
		DATA.shadersManager.set(lightName + ".diffuse", light.diffuse);
		DATA.shadersManager.set(lightName + ".specular", light.specular);
	}

	DATA.shadersManager.set("dirLightsCount", dirLightsCount);
	DATA.shadersManager.set("pointLightsCount", pointLightsCount);
	DATA.shadersManager.set("spotLightsCount", spotLightsCount);
}

void DrawGUI()
{
	auto lightToDelete = DATA.lights.end();
	int lightTypeToAdd = -1;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (ImGui::CollapsingHeader("Lights"))
		{
			int i = 0;
			for (auto itLight = DATA.lights.begin(); itLight != DATA.lights.end(); ++itLight)
			{
				ImGui::PushID(++i);
				Light &light = *itLight;
				if (light.type == 0)
				{
					ImGui::Text("Point");
					ImGui::Indent();
					ImGui::DragFloat3("location", (float *)&light.location, 0.01f);
				}
				else if (light.type == 1)
				{
					ImGui::Text("Directional");
					ImGui::Indent();
					ImGui::DragFloat3("direction", (float *)&light.direction, 0.01f, -1.0f, 1.0f);
				}
				else if (light.type == 2)
				{
					ImGui::Text("Spot");
					ImGui::Indent();
					ImGui::DragFloat3("location", (float *)&light.location, 0.01f);
					ImGui::DragFloat3("direction", (float *)&light.direction, 0.01f, -1.0f, 1.0f);
					ImGui::DragFloat("innerCutOff", &light.innerCutOff, 0.001f, 0.f, 1.f);
					ImGui::DragFloat("outerCutOff", &light.outerCutOff, 0.001f, 0.f, 1.f);
				}
				ImGui::ColorEdit3("ambient", (float *)&light.ambient, ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("diffuse", (float *)&light.diffuse, ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("specular", (float *)&light.specular, ImGuiColorEditFlags_Float);
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

			/*
			std::stringstream sstr;
			sstr << "lastX: " << DATA.lastX << "  lastY: " << DATA.lastY;
			ImGui::Text(sstr.str().c_str());
			*/
		}

		if (ImGui::CollapsingHeader("Models"))
		{
			int imGuiID = 0;
			for (Model *pModel : DATA.unsortedModels)
			{
				Model &model = *pModel;
				ImGui::PushID(++imGuiID);
				ImGui::Text("%s", model.GetName().c_str());
				ImGui::Indent();

				auto location = model.GetLocation();
				if (ImGui::DragFloat3("location", (float *)&location, 0.01f))
					model.SetLocation(location);
				auto scale = model.GetScale();
				if (ImGui::DragFloat3("scale", (float *)&scale, 0.01f))
					model.SetScale(scale);
				auto rotation = model.GetRotation();
				if (ImGui::DragFloat3("rotation", (float *)&rotation, 1.f))
					model.SetRotation(rotation);
				if (model.solidColor)
					ImGui::ColorEdit4("color", (float *)&model.color, ImGuiColorEditFlags_Float);
				ImGui::Checkbox("Outline", &model.outline);
				ImGui::SetNextItemWidth(100.f);
				ImGui::SameLine();
				ImGui::DragFloat("Shininess", &model.shininess);

				ImGui::Unindent();
				ImGui::PopID();
			}
		}

		if (ImGui::CollapsingHeader("Post-processing"))
		{
			bool changed = false;
			changed = ImGui::RadioButton("Default", &DATA.postEffect, 0) || changed;
			changed = ImGui::RadioButton("Inverse", &DATA.postEffect, 1) || changed;
			changed = ImGui::RadioButton("Grayscale", &DATA.postEffect, 2) || changed;
			changed = ImGui::RadioButton("With kernel", &DATA.postEffect, 3) || changed;
			if (changed)
				DATA.currentScreenShader = DATA.SCREEN_SHADER_ID + DATA.postEffect;

			if (DATA.postEffect == 3)
			{
				ImGui::Indent();
				changed = false;
				changed = ImGui::RadioButton("Sharpen", &DATA.currentKernel, 0) || changed;
				changed = ImGui::RadioButton("Blur", &DATA.currentKernel, 1) || changed;
				changed = ImGui::RadioButton("Edge Detection", &DATA.currentKernel, 2) || changed;
				if (changed)
				{
					Shader &shader = DATA.shadersManager.GetShader(DATA.SCREEN_SHADER_ID + DATA.postEffect);
					float kernel[] = {
						0.f, 0.f, 0.f,
						0.f, 1.f, 0.f,
						0.f, 0.f, 0.f};
					switch (DATA.currentKernel)
					{
					case 0:
						kernel[0] = 2.f;
						kernel[1] = 2.f;
						kernel[2] = 2.f;
						kernel[3] = 2.f;
						kernel[4] = -15.f;
						kernel[5] = 2.f;
						kernel[6] = 2.f;
						kernel[7] = 2.f;
						kernel[8] = 2.f;
						break;
					case 1:
						kernel[0] = 1.f / 16.f;
						kernel[1] = 2.f / 16.f;
						kernel[2] = 1.f / 16.f;
						kernel[3] = 2.f / 16.f;
						kernel[4] = 4.f / 16.f;
						kernel[5] = 2.f / 16.f;
						kernel[6] = 1.f / 16.f;
						kernel[7] = 2.f / 16.f;
						kernel[8] = 1.f / 16.f;
						break;
					case 2:
						kernel[0] = 1.f;
						kernel[1] = 1.f;
						kernel[2] = 1.f;
						kernel[3] = 1.f;
						kernel[4] = -8.f;
						kernel[5] = 1.f;
						kernel[6] = 1.f;
						kernel[7] = 1.f;
						kernel[8] = 1.f;
						break;

					default:
						break;
					}
					shader.set("kernel", kernel, 9);
				}
				ImGui::Unindent();
			}
		}

		ImGui::Text("Camera at (%.3f, %.3f, %.3f)", DATA.camera.Position.x, DATA.camera.Position.y, DATA.camera.Position.z);
		ImGui::Indent();
		ImGui::Text("looking at (%.3f, %.3f, %.3f)", DATA.camera.Front.x, DATA.camera.Front.y, DATA.camera.Front.z);
		ImGui::Unindent();
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

		if (ImGui::Checkbox("Face culling", &DATA.faceCulling))
			glSet(GL_CULL_FACE, DATA.faceCulling);

		if (ImGui::Checkbox("Evening", &DATA.evening))
		{
			int shaderID = DATA.shadersManager.GetShaderID("vertex_skybox.glsl", "fragment_skybox.glsl");
			auto& shader = DATA.shadersManager.GetShader(shaderID);
			shader.use();
			shader.set("evening", DATA.evening);
		}

		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (lightToDelete != DATA.lights.end())
		DATA.lights.erase(lightToDelete);
	if (lightTypeToAdd != -1)
		DATA.lights.emplace_back(lightTypeToAdd);
}

void SortModelsByDepth()
{
	std::sort(DATA.models.begin(), DATA.models.end(), [](const Model *model1, const Model *model2) {
		if (model2->opaque)
			return false;
		if (model1->opaque)
			return true;

		float dist1 = glm::distance(DATA.camera.Position, model1->GetLocation());
		float dist2 = glm::distance(DATA.camera.Position, model2->GetLocation());

		return dist1 > dist2;
	});
}

void LoadSceneFromJSON()
{
	std::ifstream i("scenes/scene.json");
	json jScene;
	try
	{
		i >> jScene;
	}
	catch (const nlohmann::detail::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
		exit(1);
	}

	for (auto &jLight : jScene["Lights"])
	{
		Light light(jLight.value("type", 0));
		light.ambient = getVec3(jLight.value("ambient", std::vector<float>{0.1f, 0.1f, 0.1f}));
		light.diffuse = getVec3(jLight.value("diffuse", std::vector<float>{1.0f, 1.0f, 1.0f}));
		light.specular = getVec3(jLight.value("specular", std::vector<float>{1.0f, 1.0f, 1.0f}));

		light.constant = jLight.value("constant", 1.f);
		light.linear = jLight.value("linear", 0.09f);
		light.quadratic = jLight.value("quadratic", 0.032f);

		light.location = getVec3(jLight.value("location", std::vector<float>{0.0f, 0.0f, 0.0f}));
		light.direction = getVec3(jLight.value("direction", std::vector<float>{-0.5f, -1.0f, -0.3f}));

		light.innerCutOff = jLight.value("innerCutOff", 0.97629600712f);
		light.outerCutOff = jLight.value("outerCutOff", 0.95371695074f);

		DATA.lights.push_back(light);
	}

	for (auto &jModel : jScene["Models"])
	{
		int shaderID = DATA.shadersManager.GetShaderID(jModel["vShader"].get<std::string>().c_str(), jModel["fShader"].get<std::string>().c_str());
		Model *model = nullptr;
		if (jModel.find("path") != jModel.end())
		{
			model = new Model(jModel["path"].get<std::string>().c_str(), shaderID);
		}
		else
		{
			std::vector<Vertex> vertices;
			vertices.push_back(Vertex{glm::vec3{-0.5f, -0.5f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{0.f, 1.f}});
			vertices.push_back(Vertex{glm::vec3{0.5f, -0.5f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{1.f, 1.f}});
			vertices.push_back(Vertex{glm::vec3{-0.5f, 0.5f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{0.f, 0.f}});
			vertices.push_back(Vertex{glm::vec3{0.5f, 0.5f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{1.f, 0.f}});
			std::vector<GLuint> indices{0, 1, 2, 1, 3, 2};

			Texture texture;
			texture.id = TextureFromFile(jModel.value("texture", "").c_str(), "");
			texture.type = "texture_diffuse";

			Mesh mesh2D{vertices, indices, std::vector<Texture>{texture}};
			model = new Model{mesh2D, shaderID};
		}
		if (!model)
			continue;

		model->SetLocation(getVec3(jModel.value("location", std::vector<float>{0.f, 0.f, 0.f})));
		model->SetScale(getVec3(jModel.value("scale", std::vector<float>{1.f, 1.f, 1.f})));
		model->color = glm::vec4{getVec3(jModel.value("color", std::vector<float>{0.f, 0.f, 0.f})), 1.f};
		model->opaque = jModel.value("opaque", false);
		model->solidColor = jModel.value("solidColor", false);
		model->outline = jModel.value("outline", false);
		if (jModel.find("name") != jModel.end())
			model->ChangeName(jModel.value("name", ""));
		model->transparentCube = jModel.value("transparentCube", false);
		if (model->transparentCube)
			model->SortFaces();
		DATA.models.push_back(model);
	}
}