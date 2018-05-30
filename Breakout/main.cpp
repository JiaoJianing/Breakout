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

glm::mat4 lightProjs[3];
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
//glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
glm::vec3 lightDirection(-1.0f, -1.0f, -1.0f);
glm::vec3 lightUp(0.0f, 1.0f, 0.0f);
float cascadeEndClipSpace[3];

void calcFrustums2() {
	glm::vec3 cameraPos(0.0f, 30.0f, 3.0f);
	glm::vec3 cameraTarget(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	//glm::mat4 cameraView = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
	glm::mat4 cameraView = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());

	glm::mat4 cameraProjs[3];
	cameraProjs[0] = glm::perspective(45.0f, screenWidth / (float)screenHeight, 0.1f, 30.0f);
	cameraProjs[1] = glm::perspective(45.0f, screenWidth / (float)screenHeight, 25.0f, 80.0f);
	cameraProjs[2] = glm::perspective(45.0f, screenWidth / (float)screenHeight, 75.0f, 400.0f);

	for (int k = 0; k < 3; k++) {
		glm::mat4 cameraViewProj = cameraProjs[k] * cameraView;
		glm::mat4 cameraInvViewProj = glm::inverse(cameraViewProj);

		glm::vec4 ndcCoords[8] = {
			glm::vec4(1.0f, 1.0f, -1.0f, 1.0f), //top right near
			glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f), //top left near
			glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), //bottom left near
			glm::vec4(1.0f, -1.0f, -1.0f, 1.0f), //bottom right near
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
		};

		glm::vec3 corners[8];
		for (int i = 0; i < 8; i++) {
			glm::vec4 corner = cameraInvViewProj * ndcCoords[i];
			corner.w = 1.0f / corner.w;
			corner.x *= corner.w;
			corner.y *= corner.w;
			corner.z *= corner.w;
			corners[i] = corner;
		}

		glm::vec3 frustumCenter;
		for (int j = 0; j < 8; j++) {
			frustumCenter = frustumCenter + corners[j];
		}
		frustumCenter = frustumCenter * (1.0f / 8.0f);

		float radius = glm::length(corners[1] - corners[7]) / 2.0f;
		float texelsPerUnit = 1024.0f / (radius * 2.0f);
		glm::mat4 matScale;
		matScale = glm::scale(matScale, glm::vec3(texelsPerUnit));
		glm::mat4 matView = glm::lookAt(-lightDirection, lightPos, lightUp);
		matView *= matScale;
		glm::mat4 matInvView = glm::inverse(matView);

		frustumCenter = matView * glm::vec4(frustumCenter, 1.0f);
		frustumCenter.x = (float)floor(frustumCenter.x);
		frustumCenter.y = (float)floor(frustumCenter.y);
		frustumCenter = matInvView * glm::vec4(frustumCenter, 1.0f);

		glm::vec3 eye = frustumCenter - (lightDirection * radius * 2.0f);
		glm::mat4 lookat = glm::lookAt(eye, frustumCenter, lightUp);
		glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, -6*radius, 6*radius);
		
		lightProjs[k] = proj * lookat;
		cascadeEndClipSpace[k] = radius * 2.0f;
	}
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

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);//鼠标移动
	glfwSetMouseButtonCallback(window, mouse_click_callback);//鼠标点击
	glfwSetKeyCallback(window, key_click_callback);//键盘按下
	glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮

#pragma region shadow texture
	Shader shaderShadow("shaders/deferred_rendering/rendershadow.vs", "shaders/deferred_rendering/rendershadow.fs");
	shaderShadow.use();
	shaderShadow.setInt("texture_shadow", 0);
	Quad shadowQuad;
#pragma endregion

	Terrain terrain(30.0f, 2.0f);
	terrain.LoadHeightmap("resources/Terrain/terrain0-16bbp-257x257.raw", 16, 257, 257);

	Shader shaderTerrain("shaders/deferred_rendering/terrain.vs", "shaders/deferred_rendering/terrain.fs");
	shaderTerrain.use();
	shaderTerrain.setInt("texture_grass", 0);
	shaderTerrain.setInt("texture_rock", 1);
	shaderTerrain.setInt("texture_snow", 2);
	shaderTerrain.setInt("texture_shadow[0]", 3);
	shaderTerrain.setInt("texture_shadow[1]", 4);
	shaderTerrain.setInt("texture_shadow[2]", 5);

	//AnimationModel boblampclean("models/boblampclean/boblampclean.md5mesh");
	AnimationModel boblampclean("models/ninja/dwarf.x");
	Shader shaderBoblamp("shaders/deferred_rendering/animation_model.vs", "shaders/deferred_rendering/animation_model.fs");
	shaderBoblamp.use();
	shaderBoblamp.setInt("texture_shadow[0]", 4);
	shaderBoblamp.setInt("texture_shadow[1]", 5);
	shaderBoblamp.setInt("texture_shadow[2]", 6);

	Model nanosuit("models/nanosuit/nanosuit.obj");
	Shader shaderNanosuit("shaders/deferred_rendering/model.vs", "shaders/deferred_rendering/model.fs");
	shaderNanosuit.use();
	shaderNanosuit.setInt("texture_shadow[0]", 4);
	shaderNanosuit.setInt("texture_shadow[1]", 5);
	shaderNanosuit.setInt("texture_shadow[2]", 6);

#pragma region shadow FBO
	unsigned int shadowFBO;
	unsigned int shadowTexture[3];
	glGenFramebuffers(1, &shadowFBO);
	glGenTextures(3, shadowTexture);
	for (unsigned int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, shadowTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture[0], 0);
	// Disable writes to the color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader shaderCSM("shaders/deferred_rendering/csm.vs", "shaders/deferred_rendering/csm.fs");
	Shader shaderCSM_Animation("shaders/deferred_rendering/animation_csm.vs", "shaders/deferred_rendering/animation_csm.fs");
#pragma endregion

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

		glViewport(0, 0, screenWidth, screenHeight);

		processInput(window);
		camera.Update(currentFrame, deltaFrame);
		calcFrustums2();

		glm::mat4 model;
		glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
		glm::mat4 projection = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 1000.0f);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		std::vector<glm::mat4> Transforms;
		boblampclean.BoneTransform(currentFrame, Transforms);

#pragma region render shadow pass
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		shaderCSM.use();
		for (int i = 0; i < 3; i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture[i], 0);
			glClear(GL_DEPTH_BUFFER_BIT);
			shaderCSM.setMatrix4("projection", lightProjs[i]);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-5, 23, 0));
			model = glm::scale(model, glm::vec3(0.2f));
			shaderCSM.setMatrix4("model", model);
			nanosuit.Draw(shaderCSM);

			model = glm::mat4();
			shaderCSM.setMatrix4("model", model);
			terrain.Render(shaderCSM);

			shaderCSM_Animation.use();
			shaderCSM_Animation.setMatrix4("projection", lightProjs[i]);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, 25, 0));
			//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
			model = glm::scale(model, glm::vec3(0.05f));
			shaderCSM_Animation.setMatrix4("model", model);		
			for (unsigned i = 0; i < Transforms.size(); i++) {
				shaderCSM_Animation.setMatrix4("gBones[" + std::to_string(i) + "]", Transforms[i]);
			}
			boblampclean.Draw(shaderCSM_Animation);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderNanosuit.use();
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[0]);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[1]);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[2]);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-5, 23, 0));
		model = glm::scale(model, glm::vec3(0.2f));
		shaderNanosuit.setMatrix4("model", model);
		shaderNanosuit.setMatrix4("view", view);
		shaderNanosuit.setMatrix4("projection", projection);
		shaderNanosuit.setMatrix4("lightMVP[0]", lightProjs[0]);
		shaderNanosuit.setMatrix4("lightMVP[1]", lightProjs[1]);
		shaderNanosuit.setMatrix4("lightMVP[2]", lightProjs[2]);
		shaderNanosuit.setFloat("cascadeEndClipSpace[0]", cascadeEndClipSpace[0]);
		shaderNanosuit.setFloat("cascadeEndClipSpace[1]", cascadeEndClipSpace[1]);
		shaderNanosuit.setFloat("cascadeEndClipSpace[2]", cascadeEndClipSpace[2]);
		shaderNanosuit.setVec3("lightDirection", -lightDirection);
		nanosuit.Draw(shaderNanosuit);

		shaderBoblamp.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0, 25, 0));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model = glm::scale(model, glm::vec3(0.05f));		
		shaderBoblamp.setMatrix4("model", model);
		shaderBoblamp.setMatrix4("view", view);
		shaderBoblamp.setMatrix4("projection", projection);
		shaderBoblamp.setMatrix4("lightMVP[0]", lightProjs[0]);
		shaderBoblamp.setMatrix4("lightMVP[1]", lightProjs[1]);
		shaderBoblamp.setMatrix4("lightMVP[2]", lightProjs[2]);
		shaderBoblamp.setFloat("cascadeEndClipSpace[0]", cascadeEndClipSpace[0]);
		shaderBoblamp.setFloat("cascadeEndClipSpace[1]", cascadeEndClipSpace[1]);
		shaderBoblamp.setFloat("cascadeEndClipSpace[2]", cascadeEndClipSpace[2]);
		shaderBoblamp.setVec3("lightDirection", -lightDirection);

		for (unsigned i = 0; i < Transforms.size(); i++) {
			shaderBoblamp.setMatrix4("gBones[" + std::to_string(i) + "]", Transforms[i]);
		}

		boblampclean.Draw(shaderBoblamp);

		shaderTerrain.use();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[0]);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[1]);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[2]);
		shaderTerrain.setMatrix4("view", view);
		shaderTerrain.setMatrix4("projection", projection);
		shaderTerrain.setMatrix4("lightMVP[0]", lightProjs[0]);
		shaderTerrain.setMatrix4("lightMVP[1]", lightProjs[1]);
		shaderTerrain.setMatrix4("lightMVP[2]", lightProjs[2]);
		shaderTerrain.setFloat("cascadeEndClipSpace[0]", cascadeEndClipSpace[0]);
		shaderTerrain.setFloat("cascadeEndClipSpace[1]", cascadeEndClipSpace[1]);
		shaderTerrain.setFloat("cascadeEndClipSpace[2]", cascadeEndClipSpace[2]);
		shaderTerrain.setVec3("lightDirection", -lightDirection);
		terrain.Render(shaderTerrain);
		
		shaderShadow.use();
		for (int i = 0; i < 3; i++) {
			glViewport(210 * i, 0, 200, 200);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowTexture[i]);
			shadowQuad.Draw(shaderShadow);
		}

		glm::mat4 rotateMat;
		rotateMat = glm::rotate(rotateMat, glm::radians(currentFrame * 0.01f), glm::vec3(0, 1, 0));
		//lightTarget = rotateMat * glm::vec4(lightTarget, 1.0);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}