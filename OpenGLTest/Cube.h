#pragma once
#include "Shader.h"

//Α’·½Με
class Cube
{
public:
	Cube();
	~Cube();

	void Draw(Shader shader);

	void SetPos(const glm::vec3& pos);

	void SetColor(const glm::vec4& color);

private:
	glm::vec3 m_pos;
	glm::vec4 m_color;
	unsigned int m_VBO, m_VAO;
};

