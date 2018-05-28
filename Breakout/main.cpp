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

glm::mat4 lightProj[3];
std::vector<glm::vec3> frustumVertices;
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
glm::vec3 lightTarget(-1.0f, -1.0f, -1.0f);
glm::vec3 lightUp(0.0f, 1.0f, 0.0f);
float cascadeEnd[4];
void calcFrustums() {
	frustumVertices.clear();

	glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	//glm::mat4 cameraViewMat = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
	glm::mat4 cameraViewMat = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
	glm::mat4 cameraInvViewMat = glm::inverse(cameraViewMat);

	glm::mat4 lightViewMat = glm::lookAt(lightPos, lightPos + lightTarget, lightUp);


	float ar = screenHeight / (float)screenWidth;
	float fov = 45.0f;
	float tanHalfHFov = tanf(glm::radians(fov / 2.0f));
	float tanHalfVFov = tanf(glm::radians(fov * ar / 2.0f));

	cascadeEnd[0] = -1.0f;
	cascadeEnd[1] = -25.0f;
	cascadeEnd[2] = -90.0f;
	cascadeEnd[3] = -200.0f;

	for (int i = 0; i < 3; i++) {
		float xn = cascadeEnd[i] * tanHalfHFov;
		float xf = cascadeEnd[i + 1] * tanHalfHFov;
		float yn = cascadeEnd[i] * tanHalfVFov;
		float yf = cascadeEnd[i + 1] * tanHalfVFov;

		glm::vec4 frustumCorners[8] = {
			// near face
			glm::vec4(xn,   yn, cascadeEnd[i], 1.0), //right top
			glm::vec4(-xn,  yn, cascadeEnd[i], 1.0), //left top
			glm::vec4(xn,  -yn, cascadeEnd[i], 1.0), // right bottom
			glm::vec4(-xn, -yn, cascadeEnd[i], 1.0), // left bottom

			// far face
			glm::vec4(xf,   yf, cascadeEnd[i + 1], 1.0),
			glm::vec4(-xf,  yf, cascadeEnd[i + 1], 1.0),
			glm::vec4(xf,  -yf, cascadeEnd[i + 1], 1.0),
			glm::vec4(-xf, -yf, cascadeEnd[i + 1], 1.0)
		};

		glm::vec4 frustumCornersL[8];

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		glm::vec3 lightCenter = glm::vec3(0.0f);
		for (unsigned int j = 0; j < 8; j++) {
			//frustumCorners[j] = cameraInvViewMat * frustumCorners[j];
			glm::vec4 vW = cameraInvViewMat * frustumCorners[j];
			lightCenter += glm::vec3(vW);

			frustumCornersL[j] = lightViewMat * vW;

			minX = __min(minX, frustumCornersL[j].x);
			maxX = __max(maxX, frustumCornersL[j].x);
			minY = __min(minY, frustumCornersL[j].y);
			maxY = __max(maxY, frustumCornersL[j].y);
			minZ = __min(minZ, frustumCornersL[j].z);
			maxZ = __max(maxZ, frustumCornersL[j].z);
		}
		lightCenter = lightCenter / 8.0f;
		glm::mat4 lightView = glm::lookAt(lightCenter, lightCenter + lightTarget, lightUp);

		lightProj[i] = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ) * lightView;

		/*frustumVertices.push_back(frustumCorners[1]);
		frustumVertices.push_back(frustumCorners[5]);
		frustumVertices.push_back(frustumCorners[7]);

		frustumVertices.push_back(frustumCorners[1]);
		frustumVertices.push_back(frustumCorners[7]);
		frustumVertices.push_back(frustumCorners[3]);

		frustumVertices.push_back(frustumCorners[0]);
		frustumVertices.push_back(frustumCorners[4]);
		frustumVertices.push_back(frustumCorners[5]);

		frustumVertices.push_back(frustumCorners[0]);
		frustumVertices.push_back(frustumCorners[5]);
		frustumVertices.push_back(frustumCorners[1]);

		frustumVertices.push_back(frustumCorners[0]);
		frustumVertices.push_back(frustumCorners[2]);
		frustumVertices.push_back(frustumCorners[6]);

		frustumVertices.push_back(frustumCorners[0]);
		frustumVertices.push_back(frustumCorners[6]);
		frustumVertices.push_back(frustumCorners[4]);

		frustumVertices.push_back(frustumCorners[3]);
		frustumVertices.push_back(frustumCorners[7]);
		frustumVertices.push_back(frustumCorners[6]);

		frustumVertices.push_back(frustumCorners[3]);
		frustumVertices.push_back(frustumCorners[6]);
		frustumVertices.push_back(frustumCorners[2]);*/

		glm::vec3 righttopNear(maxX, maxY, minZ);
		glm::vec3 lefttopNear(minX, maxY, minZ);
		glm::vec3 rightbottomNear(maxX, minY, minZ);
		glm::vec3 leftbottomNear(minX, minY, minZ);
		glm::vec3 righttopFar(maxX, maxY, maxZ);
		glm::vec3 lefttopFar(minX, maxY, maxZ);
		glm::vec3 rightbottomFar(maxX, minY, maxZ);
		glm::vec3 leftbottomFar(minX, minY, maxZ);

		frustumVertices.push_back(lefttopNear);
		frustumVertices.push_back(lefttopFar);
		frustumVertices.push_back(leftbottomFar);

		frustumVertices.push_back(lefttopNear);
		frustumVertices.push_back(leftbottomFar);
		frustumVertices.push_back(leftbottomNear);

		frustumVertices.push_back(righttopNear);
		frustumVertices.push_back(righttopFar);
		frustumVertices.push_back(lefttopFar);

		frustumVertices.push_back(righttopNear);
		frustumVertices.push_back(lefttopFar);
		frustumVertices.push_back(lefttopNear);

		frustumVertices.push_back(righttopNear);
		frustumVertices.push_back(rightbottomNear);
		frustumVertices.push_back(rightbottomFar);

		frustumVertices.push_back(righttopNear);
		frustumVertices.push_back(rightbottomFar);
		frustumVertices.push_back(righttopFar);

		frustumVertices.push_back(leftbottomNear);
		frustumVertices.push_back(leftbottomFar);
		frustumVertices.push_back(rightbottomFar);

		frustumVertices.push_back(leftbottomNear);
		frustumVertices.push_back(rightbottomFar);
		frustumVertices.push_back(rightbottomNear);
	}
}

void calcFrustums2() {
	frustumVertices.clear();
	glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	glm::mat4 cameraView = glm::lookAt(cameraPos, cameraPos + cameraTarget, cameraUp);
	glm::mat4 cameraProj = glm::perspective(45.0f, screenWidth / (float)screenHeight, 0.1f, 200.0f);

	glm::mat4 cameraViewProj = cameraProj * cameraView;
	glm::mat4 cameraInvViewProj = glm::inverse(cameraViewProj);

	glm::vec4 ndcCoords[8] = {
		glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
		glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
	};

	for (int i = 0; i < 8; i++) {
		glm::vec4 corner = cameraInvViewProj * ndcCoords[i];
		corner = corner * corner.w;
		int a = 0;
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

#pragma region render frustum
	Shader shaderFrustum("shaders/deferred_rendering/frustum.vs", "shaders/deferred_rendering/frustum.fs");
	unsigned int VAO;
	unsigned int VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, frustumVertices.size() * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindVertexArray(0);
#pragma endregion

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

	float cascadeEndClipSpace[3];
	for (unsigned int i = 0; i < 3; i++) {
		glm::mat4 Proj = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 200.0f);
		glm::vec4 vView(0.0f, 0.0f, cascadeEnd[i + 1], 1.0f);
		glm::vec4 vClip = Proj * vView;
		cascadeEndClipSpace[i] = vClip.z;
	}

#pragma region shadow FBO
	unsigned int shadowFBO;
	unsigned int shadowTexture[3];
	glGenFramebuffers(1, &shadowFBO);
	glGenTextures(3, shadowTexture);
	for (unsigned int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, shadowTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
		calcFrustums();

		glm::mat4 model;
		glm::mat4 view = glm::lookAt(camera.GetPos(), camera.GetPos() + camera.GetTarget(), camera.GetUp());
		glm::mat4 projection = glm::perspective(camera.GetFov(), screenWidth / screenHeight, 0.1f, 1000.0f);

		glm::mat4 lightViewMat = glm::lookAt(lightPos, lightPos + lightTarget, lightUp);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

#pragma region render shadow pass
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		shaderCSM.use();
		for (int i = 0; i < 3; i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture[i], 0);
			glClear(GL_DEPTH_BUFFER_BIT);
			//shaderCSM.setMatrix4("view", lightViewMat);
			shaderCSM.setMatrix4("projection", lightProj[i]);
			terrain.Render(shaderCSM);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderTerrain.use();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[0]);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[1]);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowTexture[2]);
		shaderTerrain.setMatrix4("view", view);
		shaderTerrain.setMatrix4("projection", projection);
		shaderTerrain.setMatrix4("lightMVP[0]", lightProj[0]);
		shaderTerrain.setMatrix4("lightMVP[1]", lightProj[1]);
		shaderTerrain.setMatrix4("lightMVP[2]", lightProj[2]);
		shaderTerrain.setFloat("cascadeEndClipSpace[0]", cascadeEndClipSpace[0]);
		shaderTerrain.setFloat("cascadeEndClipSpace[1]", cascadeEndClipSpace[1]);
		shaderTerrain.setFloat("cascadeEndClipSpace[2]", cascadeEndClipSpace[2]);
		terrain.Render(shaderTerrain);
		
		shaderShadow.use();
		for (int i = 0; i < 3; i++) {
			glViewport(210 * i, 0, 200, 200);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowTexture[i]);
			shadowQuad.Draw(shaderShadow);
		}

#pragma region render frustum
		//shaderFrustum.use();
		//shaderFrustum.setMatrix4("view", view);
		//shaderFrustum.setMatrix4("projection", projection);
		//glBindVertexArray(VAO);
		//glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//glBufferData(GL_ARRAY_BUFFER, frustumVertices.size() * sizeof(glm::vec3), &frustumVertices[0], GL_DYNAMIC_DRAW);
		//glDrawArrays(GL_TRIANGLES, 0, frustumVertices.size());
		//glBindVertexArray(0);
#pragma endregion

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}