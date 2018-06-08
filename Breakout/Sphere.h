#pragma once
#pragma once
#include "Shader.h"
#include <vector>
#include "Mesh.h"

class Sphere
{
public:
	Sphere();
	~Sphere();

	virtual void RenderSimple(Shader shader);

private:
	void initRenderData();

	unsigned int m_VBO, m_VAO, m_EBO;
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
};

