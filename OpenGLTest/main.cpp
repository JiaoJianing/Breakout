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

float screenWidth = 800, screenHeight = 600;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;
bool blinn = true;
float exposure = 0.0f;

Camera camera(screenWidth, screenHeight);

glm::vec3 lightPos(1.0f, 1.0f, 3.0f);
glm::vec3 lightSrcPos(2.0f, 1.0f, 2.0f);
float lightRotAngle = 0;

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

int main(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGLTest", NULL, NULL);
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

	//准备浮点数帧缓冲，以实现hdr
	unsigned int hdrFBO;
	unsigned int hdrTexture[2];
	unsigned int hdrRBO;
	glGenFramebuffers(1, &hdrFBO);
	//颜色附件
	glGenTextures(2, hdrTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, hdrTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, hdrTexture[i], 0);
	}
	//显示告诉OpenGL使用MRT
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	//深度附件
	glGenRenderbuffers(1, &hdrRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//准备gauss模糊需要的两个帧缓冲
	unsigned int gaussFBO[2];
	unsigned int gaussTexture[2];
	glGenFramebuffers(2, gaussFBO);
	glGenTextures(2, gaussTexture);
	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, gaussFBO[i]);
		glBindTexture(GL_TEXTURE_2D, gaussTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gaussTexture[i], 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//准备灯光数据
	std::vector<glm::vec3> lightColors;
	lightColors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));
	lightColors.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.2f));
	lightColors.push_back(glm::vec3(0.0f, 0.1f, 0.0f));
	std::vector<glm::vec3> lightPositions;
	lightPositions.push_back(glm::vec3(0.0f, 0.0f, -5.5f));
	lightPositions.push_back(glm::vec3(-0.4f, 0.0f, 0.0f));
	lightPositions.push_back(glm::vec3(0.3f, 0.0f, 0.0f));
	lightPositions.push_back(glm::vec3(-0.1f, 0.0f, -2.0f));

	Shader screenShader("shaders/bloom/full_screen.vs", "shaders/bloom/full_screen.fs");
	Shader gaussShader("shaders/bloom/full_screen.vs", "shaders/bloom/gauss.fs");
	Shader cubeShader("shaders/bloom/cube.vs", "shaders/bloom/cube.fs");
	
	Cube cube;
	cube.SetScale(glm::vec3(1.0f, 1.0f, 10.0f));
	//全屏quad
	Quad screen;
	//高斯模糊quad
	Quad gauss;

	screenShader.use();
	screenShader.setInt("hdrTexture", 0);
	screenShader.setInt("gaussTexture", 1);
	gaussShader.use();
	gaussShader.setInt("texture1", 0);

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

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//渲染到帧缓冲
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cubeShader.use();
		cubeShader.setMatrix4("view", view);
		cubeShader.setMatrix4("projection", projection);
		cubeShader.setVec3("viewPos", camera.GetPos());
		cubeShader.setInt("reverse_normal", 1);
		cubeShader.setInt("blinn", blinn);
		for (int i = 0; i < lightPositions.size(); i++) {
			cubeShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
			cubeShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
		}
		cube.Draw(cubeShader);

		//明亮部分进行gauss模糊
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		gaussShader.use();
		for (unsigned int i = 0; i < amount; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, gaussFBO[horizontal]);
			gaussShader.setInt("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? hdrTexture[1] : gaussTexture[!horizontal]);
			gauss.Draw(gaussShader);
			horizontal = !horizontal;
			if (first_iteration) {
				first_iteration = false;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//渲染全屏矩形
		exposure = (sin(currentFrame) + 1) /2;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenShader.use();
		screenShader.setFloat("exposure", exposure);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gaussTexture[0]);
		screen.Draw(screenShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}