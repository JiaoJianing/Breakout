#pragma once
#include "Shader.h"

class Water
{
public:
	Water();
	~Water();

	void Render(Shader shader);

private:
	unsigned int m_VAO, m_VBO;
};

