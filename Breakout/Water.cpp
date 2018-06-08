#include "Water.h"
#include <glad/glad.h>
#include <glm.hpp>
#include <vector>

const float kNormalMapScale = 0.25f;
float g_WaterUV = 35.0f;
float g_WaterFlow = 0.0015f;
std::vector<glm::vec3> positions;
std::vector<glm::vec2> refrCoords;
std::vector<glm::vec2> normCoords;
int positionSize, refrCoordSize, normCoordSize;

Water::Water()
{
	refrCoords.resize(6);
	normCoords.resize(6);

	positions.push_back(glm::vec3(-256.0f, 10.0f, -256.0f)); //back left
	positions.push_back(glm::vec3(-256.0f, 10.0f, 256.0f));  //front left
	positions.push_back(glm::vec3(256.0f, 10.0f, 256.0f));   //front right
	positions.push_back(glm::vec3(-256.0f, 10.0f, -256.0f)); //back left
	positions.push_back(glm::vec3(256.0f, 10.0f, 256.0f));   //front right
	positions.push_back(glm::vec3(256.0f, 10.0f, -256.0f));  //back right

	positionSize = sizeof(glm::vec3) * positions.size();
	refrCoordSize = sizeof(glm::vec2) * refrCoords.size();
	normCoordSize = sizeof(glm::vec2) * normCoords.size();

	int size = sizeof(glm::vec3);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, positionSize + refrCoordSize + normCoordSize, 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, &positions[0]);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize, refrCoordSize, &refrCoords[0]);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize + refrCoordSize, normCoordSize, &normCoords[0]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(positionSize));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(positionSize + refrCoordSize));
	glBindVertexArray(0);
}


Water::~Water()
{
}

void Water::Render(Shader shader)
{
	// Create a static variable for the movement of the water
	static float move = 0.0f;

	// Use this variable for the normal map and make it slower
	// than the refraction map's speed.  We want the refraction
	// map to be jittery, but not the normal map's waviness.
	float move2 = move * kNormalMapScale;

	// Set the refraction map's UV coordinates to our global g_WaterUV
	float refrUV = g_WaterUV;

	// Set our normal map's UV scale and shrink it by kNormalMapScale
	float normalUV = g_WaterUV * kNormalMapScale;

	// Move the water by our global speed
	move += g_WaterFlow;

	refrCoords.clear();
	normCoords.clear();

	refrCoords.push_back(glm::vec2(0.0f, refrUV - move));//back left
	refrCoords.push_back(glm::vec2(0.0f, 0.0f - move));  //front left
	refrCoords.push_back(glm::vec2(refrUV, 0.0f - move));  //front right
	refrCoords.push_back(glm::vec2(0.0f, refrUV - move));//back left
	refrCoords.push_back(glm::vec2(refrUV, 0.0f - move));  //front right
	refrCoords.push_back(glm::vec2(refrUV, refrUV - move));//back right

	normCoords.push_back(glm::vec2(0.0f, normalUV + move2));
	normCoords.push_back(glm::vec2(0.0f, 0.0f + move2));
	normCoords.push_back(glm::vec2(normalUV, 0.0f + move2));
	normCoords.push_back(glm::vec2(0.0f, normalUV + move2));
	normCoords.push_back(glm::vec2(normalUV, 0.0f + move2));
	normCoords.push_back(glm::vec2(normalUV, normalUV + move2));

	shader.use();

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize, refrCoordSize, &refrCoords[0]);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize + refrCoordSize, normCoordSize, &normCoords[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
