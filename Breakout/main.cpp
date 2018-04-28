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
#include <random>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "Cube.h"
#include "Quad.h"

float screenWidth = 800, screenHeight = 600;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;
bool blinn = true;
float exposure = 0.0f;

glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

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

float lerp(float a, float b, float f) {
	return a + f * (b - a);
}

int main(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	//准备G-Buffer
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gTexture[3];
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glGenTextures(3, gTexture);
	//位置和深度信息
	glBindTexture(GL_TEXTURE_2D, gTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[0], GL_TEXTURE_2D, gTexture[0], 0);
	//法线信息
	glBindTexture(GL_TEXTURE_2D, gTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[1], GL_TEXTURE_2D, gTexture[1], 0);
	//反射颜色信息
	glBindTexture(GL_TEXTURE_2D, gTexture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[2], GL_TEXTURE_2D, gTexture[2], 0);
	//显式告知OpenGL使用MRT
	glDrawBuffers(3, attachments);

	//深度附件
	unsigned int depthRBO;
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	//检查fbo完整性
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Frame Buffer Not Completed!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//准备ssao阶段的帧缓冲
	unsigned int ssaoFBO;
	unsigned int ssaoTexture;
	glGenTextures(1, &ssaoTexture);
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//生成随机采样核心
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);//随机浮点数
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; i++) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	//生成随机转动向量
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0
		);
		ssaoNoise.push_back(noise);
	}
	//包含随机转动的贴图
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	//用来进行模糊的帧缓冲
	unsigned int blurFBO;
	unsigned int blurTexture;
	glGenTextures(1, &blurTexture);
	glGenFramebuffers(1, &blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glBindTexture(GL_TEXTURE_2D, blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Cube floor;
	floor.SetScale(glm::vec3(20.0f, 1.0f, 20.0f));
	floor.SetPos(glm::vec3(0.0f, -1.0f, 0.0f));
	Quad screen;
	Model nanosuit("models/nanosuit/nanosuit.obj");

	Shader gbufferShader("shaders/ssao/gbuffer.vs", "shaders/ssao/gbuffer.fs");
	Shader ssaoShader("shaders/ssao/ssao.vs", "shaders/ssao/ssao.fs");
	Shader blurShader("shaders/ssao/blur.vs", "shaders/ssao/blur.fs");
	Shader screenShader("shaders/ssao/screen.vs", "shaders/ssao/screen.fs");

	screenShader.use();
	screenShader.setInt("texture_posDepth", 0);
	screenShader.setInt("texture_normal", 1);
	screenShader.setInt("texture_albedo", 2);
	screenShader.setInt("texture_blur", 3);

	ssaoShader.use();
	ssaoShader.setInt("texture_posDepth", 0);
	ssaoShader.setInt("texture_normal", 1);
	ssaoShader.setInt("texture_noise", 2);

	blurShader.use();
	blurShader.setInt("ssaoTexture", 0);

	while (!glfwWindowShouldClose(window))
	{
		//绘制
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//线框模式
		float currentFrame = glfwGetTime();
		deltaFrame = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//计算帧率
		float frameRate = 1 / deltaFrame;
		//std::cout << "Current FPS: " << frameRate << std::endl;

		processInput(window);
		camera.Update(currentFrame, deltaFrame);

		glm::mat4 model;
		glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
		glm::mat4 projection = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 100.0f);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		//渲染到gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gbufferShader.use();
		gbufferShader.setFloat("nearPlane", 0.1f);
		gbufferShader.setFloat("farPlane", 100.0f);
		gbufferShader.setMatrix4("view", view);
		gbufferShader.setMatrix4("projection", projection);
		floor.Draw(gbufferShader);

		model = glm::mat4();
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		gbufferShader.setMatrix4("model", model);
		nanosuit.Draw(gbufferShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//ssao阶段
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		ssaoShader.use();
		ssaoShader.setMatrix4("projection", projection);
		ssaoShader.setFloat("screenWidth", screenWidth);
		ssaoShader.setFloat("screenHeight", screenHeight);
		for (int i = 0; i < 64; i++) {
			ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTexture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gTexture[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		screen.Draw(ssaoShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//进行模糊
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
		blurShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoTexture);
		screen.Draw(blurShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//渲染到屏幕
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenShader.use();
		screenShader.setVec3("light.Position", lightPos);
		screenShader.setVec3("light.Color", lightColor);
		screenShader.setFloat("light.Linear", 0.09);
		screenShader.setFloat("light.Quadratic", 0.032);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTexture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gTexture[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gTexture[2]);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, blurTexture);
		screen.Draw(screenShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}