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
#include "Sphere.h"
#include "Text.h"
#include "ResourceManager.h"
#include "Game.h"

float screenWidth = 800, screenHeight = 600;

float deltaFrame = 0.0f;
float lastFrame = 0.0f;

Game breakOut(screenWidth, screenHeight);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, screenWidth, screenHeight);
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
		}
		else if (action == GLFW_RELEASE) {
		}
	}
}

void key_click_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key <= 1024) {
		if (action == GLFW_PRESS) {
			breakOut.Keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			breakOut.Keys[key] = false;
		}
	}
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
}

int main(int argc, char** argv) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);//����ƶ�
	glfwSetMouseButtonCallback(window, mouse_click_callback);//�����
	glfwSetKeyCallback(window, key_click_callback);//���̰���
	glfwSetScrollCallback(window, scroll_callback);//������

	glViewport(0, 0, screenWidth, screenHeight);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	breakOut.Init();

	while (!glfwWindowShouldClose(window))
	{
		//����
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//�߿�ģʽ
		float currentFrame = glfwGetTime();
		deltaFrame = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		breakOut.ProcessInput(deltaFrame);

		breakOut.Update(deltaFrame);
		
		breakOut.Render();

		glfwSwapBuffers(window);
	}

	ResourceManager::getInstance()->Clear();

	ResourceManager::getInstance()->deleteInstance();
	Text::getInstance()->deleteInstance();

	glfwTerminate();
	return 0;
}