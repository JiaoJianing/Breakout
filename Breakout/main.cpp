#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "Cube.h"
#include "Quad.h"
#include "Terrain.h"
#include "AnimationModel.h"
#include "Water.h"
#include "Sphere.h"

float screenWidth = 1024, screenHeight = 1024;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;
bool blinn = true;
float exposure = 0.0f;

Camera camera(screenWidth, screenHeight);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, screenWidth, screenHeight);
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.OnMouseMove(xpos, ypos);
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			camera.OnMouseDown();
		}
		else if (action == GLFW_RELEASE) {
			camera.OnMouseUp();
		}
	}
}

void key_click_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
	if (action != GLFW_PRESS) {
		return;
	}

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_B:
	{
		blinn = !blinn;
		std::cout << "use blinn: " << (blinn ? "true" : "false" )<< std::endl;
	}
		break;
	default:
		camera.OnKeyboard(key);
		break;
	}
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
	camera.OnMouseScroll(xOffset, yOffset);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_UP);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_W);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_S);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_DOWN);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_A);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_D);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		camera.OnKeyboard(GLFW_KEY_RIGHT);
	}
}

unsigned int loadCubeMap(std::vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	int width, height, nrChannels;
	for (int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else {
			std::cout << "Cube Map failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void renderSkyBox(Sphere& sphere, Shader shader, unsigned int skyboxTexture, glm::mat4& matProj, glm::mat4& matView, glm::vec3 viewPos) {
	GLint OldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
	GLint OldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

	glDepthFunc(GL_LEQUAL);

	shader.use();
	shader.setMatrix4("view", matView);
	shader.setMatrix4("projection", matProj);
	glm::mat4 model;
	model = glm::translate(model, viewPos);
	shader.setMatrix4("model", model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	sphere.RenderSimple(shader);
}

int main(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, 0);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Breakout", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);//鼠标移动
	glfwSetMouseButtonCallback(window, mouse_click_callback);//鼠标点击
	glfwSetKeyCallback(window, key_click_callback);//键盘按下
	glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮

	Terrain terrain(30.0f, 2.0f);
	terrain.LoadHeightmap("resources/Terrain/terrain0-16bbp-257x257.raw", 16, 257, 257);

	Shader shaderTerrain_upWater("shaders/deferred_rendering/terrain.vs", "shaders/deferred_rendering/terrain_upwater.fs");
	shaderTerrain_upWater.use();
	shaderTerrain_upWater.setInt("texture_grass", 0);
	shaderTerrain_upWater.setInt("texture_rock", 1);
	shaderTerrain_upWater.setInt("texture_snow", 2);

	Shader shaderTerrain_underWater("shaders/deferred_rendering/terrain.vs", "shaders/deferred_rendering/terrain_underwater.fs");
	shaderTerrain_underWater.use();
	shaderTerrain_underWater.setInt("texture_grass", 0);
	shaderTerrain_underWater.setInt("texture_rock", 1);
	shaderTerrain_underWater.setInt("texture_snow", 2);

	Shader shaderTerrain_underWater_caust("shaders/deferred_rendering/terrain.vs", "shaders/deferred_rendering/terrain_underwater_caust.fs");
	shaderTerrain_underWater_caust.use();
	shaderTerrain_underWater_caust.setInt("texture_grass", 0);
	shaderTerrain_underWater_caust.setInt("texture_rock", 1);
	shaderTerrain_underWater_caust.setInt("texture_snow", 2);
	shaderTerrain_underWater_caust.setInt("texture_caust", 3);
	shaderTerrain_underWater_caust.setFloat("caustScale", 4.0f);
	shaderTerrain_underWater_caust.setVec3("fogColor", glm::vec3(0.2f, 0.2f, 0.9f));
	shaderTerrain_underWater_caust.setFloat("fogStart", 0.0f);
	shaderTerrain_underWater_caust.setFloat("fogEnd", 50.0f);

	Water water;
	Shader shaderWater("shaders/deferred_rendering/water/water.vs", "shaders/deferred_rendering/water/water.fs");
	shaderWater.use();
	shaderWater.setInt("texture_reflect", 0);
	shaderWater.setInt("texture_normal", 1);
	shaderWater.setInt("texture_dudv", 2);
	shaderWater.setInt("texture_refract", 3);
	shaderWater.setInt("texture_depth", 4);
	shaderWater.setVec4("waterColor", glm::vec4(0.1f, 0.2f, 0.4f, 1.0f));
	shaderWater.setVec3("lightPos", glm::vec3(100.0f, 150.0f, 100.0f));
	unsigned int normalTexture = Texture("resources/water/normalmap.bmp").GetID();
	unsigned int dudvTexture = Texture("resources/water/dudvmap.bmp").GetID();
	std::vector<unsigned int> caustTextures;
	for (int i = 0; i < 32; i++) {
		std::string caustTextureName = "resources/water/caust" + std::to_string(i / 10) + std::to_string(i % 10) + ".bmp";
		caustTextures.push_back(Texture::loadTexture(caustTextureName));
	}

	Quad test;
	Shader shaderTest("shaders/deferred_rendering/water/test.vs", "shaders/deferred_rendering/water/test.fs");
	shaderTest.use();
	shaderTest.setInt("scene", 0);

#pragma region Water FBO
#pragma region reflect
	unsigned int reflectFBO;
	unsigned int reflectTexture, rbo;
	glGenFramebuffers(1, &reflectFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
	glGenTextures(1, &reflectTexture);
	glBindTexture(GL_TEXTURE_2D, reflectTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectTexture, 0);	
	
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma region refract
	unsigned int refractFBO;
	unsigned int refractTexture, depthTexture;
	glGenFramebuffers(1, &refractFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
	glGenTextures(1, &refractTexture);
	glBindTexture(GL_TEXTURE_2D, refractTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractTexture, 0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma endregion

	Shader shaderSkybox("shaders/deferred_rendering/skybox.vs", "shaders/deferred_rendering/skybox.fs");
	Shader shaderSkybox1("shaders/deferred_rendering/skybox1.vs", "shaders/deferred_rendering/skybox.fs");
	std::vector<std::string> cubeMaps;
	cubeMaps.push_back("resources/skybox3/right.jpg");
	cubeMaps.push_back("resources/skybox3/left.jpg");
	cubeMaps.push_back("resources/skybox3/top.jpg");
	cubeMaps.push_back("resources/skybox3/bottom.jpg");
	cubeMaps.push_back("resources/skybox3/front.jpg");
	cubeMaps.push_back("resources/skybox3/back.jpg");
	unsigned int skyboxTexture = loadCubeMap(cubeMaps);

	Sphere skydome;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		//绘制
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//线框模式
		float currentFrame = glfwGetTime();
		deltaFrame = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//计算帧率
		float frameRate = 1 / deltaFrame;
		//std::cout << "Current FPS: " << frameRate << std::endl;
		glClearColor(0.1f, 0.2f, 0.4f, 1.0f);

		processInput(window);
		camera.Update(currentFrame, deltaFrame);

		glm::mat4 model;
		glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
		glm::mat4 projection = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 1000.0f);

#pragma region render reflect
		//render Caustics
		static int startIndex = 0;
		static int frameCount = 0;
		if (frameCount == 5)
		{
			startIndex = ((startIndex + 1) % 32);
			frameCount = 0;
		}
		frameCount++;

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (camera.GetPos().y > 10.0f) {
			shaderTerrain_underWater.use();
			model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
			shaderTerrain_underWater.setMatrix4("model", model);
			shaderTerrain_underWater.setMatrix4("view", view);
			shaderTerrain_underWater.setMatrix4("projection", projection);
			shaderTerrain_underWater.setFloat("waterHeight", 10.0f);

			glCullFace(GL_FRONT);
			terrain.Render(shaderTerrain_underWater);
			glCullFace(GL_BACK);
			renderSkyBox(skydome, shaderSkybox, skyboxTexture, projection, view, camera.GetPos());
		}
		else {
			model = glm::mat4();
			shaderTerrain_upWater.use();
			shaderTerrain_upWater.setMatrix4("model", model);
			shaderTerrain_upWater.setMatrix4("view", view);
			shaderTerrain_upWater.setMatrix4("projection", projection);
			shaderTerrain_upWater.setFloat("waterHeight", 10.0f);
			terrain.Render(shaderTerrain_upWater);
			renderSkyBox(skydome, shaderSkybox, skyboxTexture, projection, view, camera.GetPos());
		}
#pragma endregion

#pragma region render refract
		glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (camera.GetPos().y > 10.0f) {
			shaderTerrain_underWater.use();
			model = glm::mat4();
			shaderTerrain_underWater.setMatrix4("model", model);
			shaderTerrain_underWater.setMatrix4("view", view);
			shaderTerrain_underWater.setMatrix4("projection", projection);
			shaderTerrain_underWater.setFloat("waterHeight", 10.0f);
			terrain.Render(shaderTerrain_underWater);
			renderSkyBox(skydome, shaderSkybox1, skyboxTexture, projection, view, camera.GetPos());
		}
		else {
			glCullFace(GL_FRONT);
			model = glm::mat4();
			shaderTerrain_upWater.use();
			shaderTerrain_upWater.setMatrix4("model", model);
			shaderTerrain_upWater.setMatrix4("view", view);
			shaderTerrain_upWater.setMatrix4("projection", projection);
			shaderTerrain_upWater.setFloat("waterHeight", 10.0f);
			terrain.Render(shaderTerrain_upWater);
			glCullFace(GL_BACK);
			renderSkyBox(skydome, shaderSkybox1, skyboxTexture, projection, view, camera.GetPos());
		}
#pragma endregion

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, screenWidth, screenHeight);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, caustTextures[startIndex]);
		shaderTerrain_underWater_caust.use();
		model = glm::mat4();
		shaderTerrain_underWater_caust.setMatrix4("model", model);
		shaderTerrain_underWater_caust.setMatrix4("view", view);
		shaderTerrain_underWater_caust.setMatrix4("projection", projection);
		shaderTerrain_underWater_caust.setFloat("waterHeight", 10.0f);
		shaderTerrain_underWater_caust.setVec3("viewPos", camera.GetPos());
		terrain.Render(shaderTerrain_underWater_caust);

		model = glm::mat4();
		shaderTerrain_upWater.use();
		shaderTerrain_upWater.setMatrix4("model", model);
		shaderTerrain_upWater.setMatrix4("view", view);
		shaderTerrain_upWater.setMatrix4("projection", projection);
		shaderTerrain_upWater.setFloat("waterHeight", 10.0f);
		terrain.Render(shaderTerrain_upWater);

		glDisable(GL_CULL_FACE);
		shaderWater.use();
		shaderWater.setMatrix4("view", view);
		shaderWater.setMatrix4("projection", projection);
		shaderWater.setVec3("viewPos", camera.GetPos());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflectTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dudvTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, refractTexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		water.Render(shaderWater);
		glEnable(GL_CULL_FACE);
		renderSkyBox(skydome, shaderSkybox1, skyboxTexture, projection, view, camera.GetPos());

		//shaderTest.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, reflectTexture);
		//test.Draw(shaderTest);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}