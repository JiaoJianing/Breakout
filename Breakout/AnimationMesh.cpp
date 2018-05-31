#include "AnimationMesh.h"

AnimationMesh::AnimationMesh(std::vector<AnimationVertex> vertices, std::vector<VertexBoneData> bones, std::vector<unsigned int> indices, std::vector<Texture> textureus)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textureus;
	this->bones = bones;

	setupMesh();
}


AnimationMesh::~AnimationMesh()
{
}

void AnimationMesh::Draw(Shader shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int reflectNr = 1;
	unsigned int normalNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++) {
		//激活纹理单元
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = textures[i].getType();
		if (name == "texture_diffuse") {
			number = std::to_string(diffuseNr++);
		}
		else if (name == "texture_specular") {
			number = std::to_string(specularNr++);
		}
		else if (name == "texture_reflect") {
			number = std::to_string(reflectNr++);
		}
		else if (name == "texture_normal") {
			number = std::to_string(normalNr++);
		}

		shader.setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].GetID());
	}
	glActiveTexture(GL_TEXTURE0);

	//绘制网格
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

unsigned int AnimationMesh::getVAO()
{
	return VAO;
}

void AnimationMesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, VBOs);
	glGenBuffers(1, &EBO);

#pragma region 配置VAO
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimationVertex), &vertices[0], GL_STATIC_DRAW);
	//位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimationVertex), (void*)0);
	//纹理
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(AnimationVertex), (void*)offsetof(AnimationVertex, texCoord));
	//法线
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AnimationVertex), (void*)offsetof(AnimationVertex, normal));

	//骨骼动画信息
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0]) * bones.size(), &bones[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)16);

	//索引缓冲
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
#pragma endregion
}

void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
	for (unsigned int i = 0; i < 4; i++) {
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	//assert(0);
}
