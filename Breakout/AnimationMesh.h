#pragma once
#include <vector>
#include "Shader.h"
#include "Texture.h"

struct AnimationVertex {
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

struct BoneInfo {
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransform;
};

struct VertexBoneData {
	unsigned int IDs[4];
	float Weights[4];

	VertexBoneData() {
		memset(&IDs, 0, sizeof(IDs));
		memset(&Weights, 0, sizeof(Weights));
	}

	void AddBoneData(unsigned int BoneID, float Weight);
};

class AnimationMesh
{
public:
	AnimationMesh(std::vector<AnimationVertex> vertices, std::vector<VertexBoneData> bones, std::vector<unsigned int> indices, std::vector<Texture> textureus);
	~AnimationMesh();

public:
	std::vector<AnimationVertex> vertices;
	std::vector<VertexBoneData> bones;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	void Draw(Shader shader);

	unsigned int getVAO();

private:
	unsigned int VAO, EBO;
	unsigned int VBOs[2];
	void setupMesh();
};

