#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "AnimationMesh.h"
#include "Texture.h"
#include <map>

class AnimationModel
{
public:
	AnimationModel(const char* path);
	~AnimationModel();

	void LoadModel(const char* path);

	void Draw(Shader shader);

	std::vector<AnimationMesh>& getMeshes();

	void BoneTransform(float timeSeconds, std::vector<glm::mat4>& transforms);

	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);    
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

private:
	std::vector<AnimationMesh> meshes;
	std::string directory;

	void loadModel(std::string path);

	void processNode(aiNode* node, const aiScene* scene);

	AnimationMesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	void loadBones(aiMesh* mesh, std::vector<VertexBoneData>& bones);

	glm::mat4 aiMat2GlmMat(const aiMatrix4x4& mat);
	glm::mat4 aiMat2GlmMat(const aiMatrix3x3& mat);

private:
	std::vector<Texture> textures_loaded;

	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;
	glm::mat4 m_GlobalInverseTransform;

	const aiScene* m_Scene;
	Assimp::Importer m_Importer;
};

