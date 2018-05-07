#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <iostream>
#include <string>
#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"

float screenWidth = 800, screenHeight = 600;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;

Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	camera.SetViewport(0, 0, width, height);
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.Move2D(xpos, ypos);
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	camera.SetPos(button, action, x, y);
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
	default:
		break;
	}
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
	if (yOffset > 0) {
		camera.SetPos(3, GLFW_PRESS, 0, 0);
	}
	else {
		camera.SetPos(4, GLFW_PRESS, 0, 0);
	}
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		camera.Move(FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.Move(FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.Move(BACK);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		camera.Move(BACK);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.Move(LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		camera.Move(LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.Move(RIGHT);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		camera.Move(RIGHT);
	}
}

int main(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Breakout", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, screenWidth, screenHeight);
	camera.SetViewport(0, 0, screenWidth, screenHeight);
	camera.SetMode(FREE);
	camera.SetPosition(glm::vec3(0, 0, -1));
	camera.SetLookAt(glm::vec3(0, 0, 0));
	camera.SetClipping(.1, 1000);
	camera.SetFOV(45);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);//鼠标移动
	glfwSetMouseButtonCallback(window, mouse_click_callback);//鼠标点击
	glfwSetKeyCallback(window, key_click_callback);//键盘按下
	glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮

	//查询最多支持多少顶点属性
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;

	glEnable(GL_DEPTH_TEST);

#pragma region 光源属性准备
	glm::vec3 dirLightDirection(-0.2f, -1.0f, -0.3f);
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
#pragma endregion

	std::string path = "models/nanosuit/nanosuit.obj";
	Model nanosuit(path.c_str());

	Shader modelShader("shaders/model/nanosuit.vs", "shaders/model/nanosuit.fs");

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//绘制
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//线框模式
		float currentFrame = glfwGetTime();
		deltaFrame = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

#pragma region 绘制模型
		modelShader.use();

		camera.Update();
		glm::mat4 view, projection, m;
		camera.GetMatricies(projection, view, m);

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glm::mat3 normalMat;
		normalMat = glm::transpose(glm::inverse(model));

		modelShader.setMatrix4fv("view", glm::value_ptr(view));
		modelShader.setMatrix4fv("projection", glm::value_ptr(projection));
		modelShader.setMatrix4fv("model", glm::value_ptr(model));
		modelShader.setMatrix3fv("normalMat", glm::value_ptr(normalMat));

		modelShader.setFloat3("viewPos", camera.camera_position.x, camera.camera_position.y, camera.camera_position.z);
		modelShader.setFloat("material.shininess", 32.0f);

		//平行光
		modelShader.setFloat3("dirLight.direction", dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		modelShader.setFloat3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		modelShader.setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		modelShader.setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		//点光源
		for (int i = 0; i < 4; i++) {
			modelShader.setFloat3(("pointLights[" + std::to_string(i) + "]" + ".position").c_str(), pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
			modelShader.setFloat3(("pointLights[" + std::to_string(i) + "]" + ".ambient").c_str(), 0.05f, 0.05f, 0.05f);
			modelShader.setFloat3(("pointLights[" + std::to_string(i) + "]" + ".diffuse").c_str(), 0.8f, 0.8f, 0.8f);
			modelShader.setFloat3(("pointLights[" + std::to_string(i) + "]" + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
			modelShader.setFloat(("pointLights[" + std::to_string(i) + "]" + ".constant").c_str(), 1.0f);
			modelShader.setFloat(("pointLights[" + std::to_string(i) + "]" + "linear").c_str(), 0.09f);
			modelShader.setFloat(("pointLights[" + std::to_string(i) + "]" + "quadratic").c_str(), 0.032f);
		}

		//聚光灯
		modelShader.setFloat3("spotLight.position", camera.camera_position.x, camera.camera_position.y, camera.camera_position.z);
		modelShader.setFloat3("spotLight.direction", camera.camera_direction.x, camera.camera_direction.y, camera.camera_direction.z);
		modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		modelShader.setFloat3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		modelShader.setFloat3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		modelShader.setFloat3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		modelShader.setFloat("spotLight.constant", 1.0f);
		modelShader.setFloat("spotLight.linear", 0.09);
		modelShader.setFloat("spotLight.quadratic", 0.032);

		nanosuit.Draw(modelShader);
#pragma endregion

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}