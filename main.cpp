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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globalData.h"
const glm::vec3 GlobalData::DEFAULT_CAMERA_POS{0.f, 1.5f, 3.f};

void processInput(GLFWwindow *window, float dt);
void mouse_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double dx, double dy);
GLuint loadTexture(const char *path);
void SortModelsByDepth();

void SetLights(int shaderId);
void DrawGUI();

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

	std::array<float, 24> quadVertices = {  
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};
	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	
	glBindVertexArray(quadVAO);	
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(float), &quadVertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float)*2));
	glBindVertexArray(0);

	std::vector<Vertex> vertices;
	vertices.push_back(Vertex{glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{0.f, 1.f}});
	vertices.push_back(Vertex{glm::vec3{1.f, 0.f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{1.f, 1.f}});
	vertices.push_back(Vertex{glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{0.f, 0.f}});
	vertices.push_back(Vertex{glm::vec3{1.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 1.f}, glm::vec2{1.f, 0.f}});
	std::vector<GLuint> indices{0, 1, 2, 1, 3, 2};
	
	Texture texture;
	texture.id = TextureFromFile("container.jpg", "textures");
	texture.type = "texture_diffuse";

	Mesh mesh2D{vertices, indices, std::vector<Texture>{texture}};

	ShadersManager& shadersManager = DATA.shadersManager;

	glViewport(0, 0, (GLsizei)DATA.width, (GLsizei)DATA.height);
	glEnable(GL_BLEND);
	glSet(GL_CULL_FACE, DATA.faceCulling);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// FRAMEBUFFER >>>
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)DATA.width, (GLsizei)DATA.height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)DATA.width, (GLsizei)DATA.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER::Framebuffer isn't complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// FRAMEBUFFER <<<

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(wnd, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	int modelShaderID = shadersManager.CreateShader("shaders/vertex.glsl", "shaders/fragment.glsl");
	int lightShaderID = shadersManager.CreateShader("shaders/vertex_lamp.glsl", "shaders/fragment_lamp.glsl");
	int solidShaderID = shadersManager.CreateShader("shaders/vertex_solid.glsl", "shaders/fragment_solid.glsl");
	int textureShaderID = shadersManager.CreateShader("shaders/vertex_2D.glsl", "shaders/fragment_model.glsl");
	int screenShaderID = shadersManager.CreateShader("shaders/vertex_quad.glsl", "shaders/fragment_quad.glsl");
	
	Model spotLightModel("shapes/cone.nff", lightShaderID);
	Model pointLightModel("shapes/sphere.nff", lightShaderID);

	// Scene description >>>
	/*
	Model model("nanosuit/nanosuit.obj", modelShaderID);
	model.location = {0.f, 0.f, -3.f};
	model.opaque = true;
	DATA.models.push_back(&model);

	Model sphereModel("shapes/sphere.nff", modelShaderID);
	sphereModel.solidColor = true;
	sphereModel.color = {1.f, 0.5f, 0.f, 1.f};
	sphereModel.location = {0.f, 1.05f, -5.f};
	sphereModel.outline = true;
	DATA.models.push_back(&sphereModel);

	Model cube{"shapes/cube.nff", modelShaderID};
	DATA.models.push_back(&cube);
	DATA.models.back()->solidColor = true;
	DATA.models.back()->color = {0.f, 0.5f, 1.0f, 1.f};
	DATA.models.back()->location = {-2.f, 1.05f, -5.f};
	DATA.models.back()->outline = true;

	*/

	Model floor("shapes/cube.nff", modelShaderID);
	floor.solidColor = true;
	floor.color = {0.3f, 0.3f, 0.3f, 1.f};
	floor.SetScale({50.f, 0.05f, 50.f});
	floor.ChangeName("floor");
	DATA.FLOOR_ID = floor.ID;
	DATA.models.push_back(&floor);
	
	DATA.models.emplace_back(new Model{"shapes/textured_cube.nff", modelShaderID});
	DATA.models.back()->SetLocation({0.f, 1.f, 0.f});
	DATA.models.back()->transparentCube = true;
	DATA.models.back()->SortFaces();

	DATA.models.emplace_back(new Model{mesh2D, modelShaderID, glm::vec3{-1.5f, 0.05f, -1.48f}});
	DATA.models.back()->ChangeName("window");
	DATA.models.emplace_back(new Model{mesh2D, modelShaderID, glm::vec3{1.5f, 0.05f, 1.51f}});
	DATA.models.back()->ChangeName("window");
	DATA.models.emplace_back(new Model{mesh2D, modelShaderID, glm::vec3{0.0f, 0.05f, 0.7f}});
	DATA.models.back()->ChangeName("window");
	DATA.models.emplace_back(new Model{mesh2D, modelShaderID, glm::vec3{-0.3f, 0.05f,-2.3f}});
	DATA.models.back()->ChangeName("window");
	DATA.models.emplace_back(new Model{mesh2D, modelShaderID, glm::vec3{0.5f, 0.05f,-0.6f}});
	DATA.models.back()->ChangeName("window");

	for(Model* model : DATA.models)
		DATA.unsortedModels.push_back(model);
	
	Light dirLight;
	dirLight.type = 1;
	DATA.lights.push_back(dirLight);
	// Scene description <<<

	float dt = 0.f;
	float lastFrame = 0.f;
	while(!glfwWindowShouldClose(wnd))
	{
		float currentFrame = (float)glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(wnd, dt);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

		SetLights(modelShaderID);

		glm::mat4 view = DATA.camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(DATA.camera.Zoom), DATA.width / DATA.height, 0.1f, 100.f);
		shadersManager.set("view", view);
		shadersManager.set("projection", projection);
		shadersManager.set("viewPos", DATA.camera.Position);

		SortModelsByDepth();

		for(Model* model : DATA.models)
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

		for(Light& light : DATA.lights)
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

		for(Model* model : DATA.models)
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

		DrawGUI();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		shadersManager.GetShader(screenShaderID).use();
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

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
	if (ImGui::GetIO().WantCaptureMouse)
		return;

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

void SetLights(int shaderId)
{
	Shader& shader = DATA.shadersManager.GetShader(shaderId);
	shader.use();
	int pointLightsCount = 0, dirLightsCount = 0, spotLightsCount = 0;
	for(const Light& light : DATA.lights)
	{
		std::string lightName;
		switch (light.type)
		{
		case 0:
		{
			lightName = "pointLights[" + std::to_string(pointLightsCount++) + "]";
			shader.set(lightName + ".position", light.location);
			shader.set(lightName + ".constant", light.constant);
			shader.set(lightName + ".linear", light.linear);
			shader.set(lightName + ".quadratic", light.quadratic);
			break;
		}
		case 1:
		{
			lightName = "dirLights[" + std::to_string(dirLightsCount++) + "]";
			shader.set(lightName + ".direction", light.direction);
			break;
		}
		case 2:
		{
			lightName = "spotLights[" + std::to_string(spotLightsCount++) + "]";
			shader.set(lightName + ".innerCutOff", light.innerCutOff);
			shader.set(lightName + ".outerCutOff", light.outerCutOff);
			shader.set(lightName + ".direction", light.direction);
			shader.set(lightName + ".position", light.location);
			break;
		}
		
		default:
			break;
		}

		shader.set(lightName + ".ambient", light.ambient);
		shader.set(lightName + ".diffuse", light.diffuse);
		shader.set(lightName + ".specular", light.specular);
	}

	shader.set("dirLightsCount", dirLightsCount);
	shader.set("pointLightsCount", pointLightsCount);
	shader.set("spotLightsCount", spotLightsCount);
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

			/*
			std::stringstream sstr;
			sstr << "lastX: " << DATA.lastX << "  lastY: " << DATA.lastY;
			ImGui::Text(sstr.str().c_str());
			*/
		}

		if (ImGui::CollapsingHeader("Models"))
		{
			int imGuiID = 0;
			for(Model* pModel : DATA.unsortedModels)
			{
				Model& model = *pModel;
				ImGui::PushID(++imGuiID);
				ImGui::Text("%s", model.GetName().c_str());
				ImGui::Indent();

				auto location = model.GetLocation();
				if (ImGui::DragFloat3("location", (float*)&location, 0.01f))
					model.SetLocation(location);
				auto scale = model.GetScale();
				if (ImGui::DragFloat3("scale", (float*)&scale, 0.01f))
					model.SetScale(scale);
				auto rotation = model.GetRotation();
				if (ImGui::DragFloat3("rotation", (float*)&rotation, 1.f))
					model.SetRotation(rotation);
				if (model.solidColor)
					ImGui::ColorEdit4("color", (float*)&model.color, ImGuiColorEditFlags_Float);
				ImGui::Checkbox("Outline", &model.outline);
				ImGui::SetNextItemWidth(100.f);
				ImGui::SameLine(); ImGui::DragFloat("Shininess", &model.shininess);

				ImGui::Unindent();
				ImGui::PopID();
			}
		}

		ImGui::Text("Camera at (%.3f, %.3f, %.3f)", DATA.camera.Position.x, DATA.camera.Position.y, DATA.camera.Position.z);
		ImGui::Indent(); ImGui::Text("looking at (%.3f, %.3f, %.3f)", DATA.camera.Front.x, DATA.camera.Front.y, DATA.camera.Front.z); ImGui::Unindent(); 
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

		if (ImGui::Checkbox("Face culling", &DATA.faceCulling))
			glSet(GL_CULL_FACE, DATA.faceCulling);			

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
	std::sort(DATA.models.begin(), DATA.models.end(), [](const Model* model1, const Model* model2) {
		if (model1->opaque)
			return true;
		if (model2->opaque)
			return false;

		float dist1 = glm::distance(DATA.camera.Position, model1->GetLocation());
		if (model1->ID == DATA.FLOOR_ID)
			dist1 = std::numeric_limits<float>::max();
		float dist2 = glm::distance(DATA.camera.Position, model2->GetLocation());
		if (model2->ID == DATA.FLOOR_ID)
			dist2 = std::numeric_limits<float>::max();

		return dist1 > dist2;
	});
}