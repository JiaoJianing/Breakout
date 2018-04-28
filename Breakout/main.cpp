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

float screenWidth = 800, screenHeight = 600;
float shadowWidth = 1024, shadowHeight = 1024;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;
bool blinn = true;

Camera camera(screenWidth, screenHeight);
Model nanosuit;

glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
float modelRotAngle = 0;

unsigned int cubeVAO, cubeVBO;
unsigned int cubeTexture;

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

unsigned int loadTexture(char const *path)
{
	unsigned int textureID;
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

		if (format == GL_RGBA) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
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

void RenderScene(Shader shader, float currentFrame) {
	//绘制立方体房间
	glm::mat4 model;
	model = glm::mat4();
	model = glm::scale(model, glm::vec3(10.0f));
	shader.setMatrix4("model", model);
	glDisable(GL_CULL_FACE);
	shader.setInt("reverse_normal", 1);
	glBindVertexArray(cubeVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
	shader.setInt("reverse_normal", 0);

	modelRotAngle += deltaFrame * 50;
	if (modelRotAngle >= 360) modelRotAngle = 0;
	//绘制模型 缩放 旋转 平移
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-4.0f, 0.0f, -4.0f));
	model = glm::rotate(model, glm::radians(modelRotAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	shader.setMatrix4("model", model);
	nanosuit.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-4.0f, -0.0f, 2.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	shader.setMatrix4("model", model);
	nanosuit.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 2.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	shader.setMatrix4("model", model);
	nanosuit.Draw(shader);
}

void RenderLight(Shader shader) {
	//绘制光源
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.1f));
	shader.setMatrix4("model", model);
	shader.setVec3("lightColor", glm::vec3(1.0f));

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
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

	glEnable(GL_MULTISAMPLE);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);//鼠标移动
	glfwSetMouseButtonCallback(window, mouse_click_callback);//鼠标点击
	glfwSetKeyCallback(window, key_click_callback);//键盘按下
	glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮

	//立方体数据
	float CubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
		// Front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
		// Left face
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
		// Right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
		// Top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left   
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), &CubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(cubeVAO);

	//生成渲染深度立方体贴图的帧缓冲
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//通过几何着色器，1次渲染调用即可绘制出深度立方体贴图的6个面
	//创建立方体贴图的6个面
	unsigned int depthCubeMap;
	glGenTextures(1, &depthCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	cubeTexture = loadTexture("resources/wood.png");

	Shader lightShader("shaders/shadow/light.vs", "shaders/shadow/light.fs");
	Shader pointShadowShader("shaders/shadow/point_shadow.vs","shaders/shadow/point_shadow.fs");
	Shader depthShader("shaders/shadow/depth.vs", "shaders/shadow/depth.fs", "shaders/shadow/depth.gs");

	pointShadowShader.use();
	pointShadowShader.setInt("material.texture_diffuse1", 0);
	pointShadowShader.setInt("depthTexture", 3);

	nanosuit.LoadModel("models/nanosuit/nanosuit.obj");

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

		//渲染深度立方体贴图
		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		float _near = 1.0f;
		float _far = 25.0f;
		glm::mat4 model;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), shadowWidth / shadowHeight, _near, _far);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		depthShader.use();
		depthShader.setVec3("lightPos", lightPos);
		depthShader.setFloat("far_plane", _far);
		for (int i = 0; i < shadowTransforms.size(); i++) {
			std::string paramName = std::string("shadowMatrices[" + std::to_string(i) + std::string("]"));
			depthShader.setMatrix4(paramName.c_str(), shadowTransforms[i]);
		}

		RenderScene(depthShader, currentFrame);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//用深度立方体贴图渲染场景
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
		glm::mat4 projection = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 100.0f);
		pointShadowShader.use();
		pointShadowShader.setMatrix4("view", view);
		pointShadowShader.setMatrix4("projection", projection);
		pointShadowShader.setFloat("far_plane", _far);
		pointShadowShader.setVec3("viewPos", camera.GetPos());
		pointShadowShader.setVec3("lightPos", lightPos);
		pointShadowShader.setInt("blinn", blinn);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

		RenderScene(pointShadowShader, currentFrame);

		//绘制光源
		lightShader.use();
		lightShader.setMatrix4("view", view);
		lightShader.setMatrix4("projection", projection);
		RenderLight(lightShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}