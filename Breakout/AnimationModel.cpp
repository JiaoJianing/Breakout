#include "AnimationModel.h"
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include "stb_image.h"

AnimationModel::AnimationModel(const char* path)
	: m_Scene(0)
{
	loadModel(path);
}


AnimationModel::~AnimationModel()
{
}

void AnimationModel::LoadModel(const char* path)
{
	loadModel(path);
}

void AnimationModel::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

std::vector<AnimationMesh>& AnimationModel::getMeshes()
{
	return meshes;
}

void AnimationModel::BoneTransform(float timeSeconds, std::vector<glm::mat4>& transforms)
{
	glm::mat4 identity;

	float TicksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = timeSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)m_Scene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, m_Scene->mRootNode, identity);

	transforms.resize(m_NumBones);

	for (unsigned i = 0; i < m_NumBones; i++) {
		transforms[i] = m_BoneInfo[i].FinalTransform;
	}

}

void AnimationModel::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_Scene->mAnimations[0];

	glm::mat4 NodeTransformation(aiMat2GlmMat(pNode->mTransformation));

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		glm::mat4 ScalingM;
		ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		glm::mat4 RotationM = aiMat2GlmMat(RotationQ.GetMatrix());

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		glm::mat4 TranslationM;
		TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransform = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

void AnimationModel::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void AnimationModel::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void AnimationModel::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

unsigned int AnimationModel::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

unsigned int AnimationModel::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

unsigned int AnimationModel::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

const aiNodeAnim* AnimationModel::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return 0;
}

void AnimationModel::loadModel(std::string path)
{
	m_Scene = m_Importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);

	if (!m_Scene || m_Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_Scene->mRootNode) {
		std::cout << "Assimp load failed: " << m_Importer.GetErrorString() << std::endl;
		return;
	}
	m_GlobalInverseTransform = aiMat2GlmMat(m_Scene->mRootNode->mTransformation);
	m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

	directory = path.substr(0, path.find_last_of('/'));

	processNode(m_Scene->mRootNode, m_Scene);
}

void AnimationModel::processNode(aiNode* node, const aiScene* scene)
{
	//处理节点所有的网格
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	//处理它的子节点
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

AnimationMesh AnimationModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<AnimationVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::vector<VertexBoneData> bones;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		AnimationVertex vertex;
		//位置
		glm::vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertex.pos = position;
		//法线
		glm::vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		vertex.normal = normal;
		//uv坐标
		glm::vec2 texCoord;
		if (mesh->mTextureCoords[0])//检查是否有纹理
		{
			texCoord.x = mesh->mTextureCoords[0][i].x;
			texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		vertex.texCoord = texCoord;

		vertices.push_back(vertex);
	}

	bones.resize(vertices.size());
	loadBones(mesh, bones);

	//处理索引
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	//处理材质
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> reflectMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflect");
		textures.insert(textures.end(), reflectMaps.begin(), reflectMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	return AnimationMesh(vertices, bones, indices, textures);
}

std::vector<Texture> AnimationModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].getPath().c_str(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			//如果纹理没有加载过，就加载
			std::string fullPath = directory + "/" + str.C_Str();
			Texture texture(fullPath);
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}

	return textures;
}

void AnimationModel::loadBones(aiMesh* mesh, std::vector<VertexBoneData>& bones)
{
	for (unsigned int i = 0; i < mesh->mNumBones; i++) {
		unsigned int BoneIndex = 0;
		std::string BoneName(mesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			// Allocate an index for a new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = aiMat2GlmMat(mesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			unsigned int VertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[j].mWeight;
			bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}
}

glm::mat4 AnimationModel::aiMat2GlmMat(const aiMatrix4x4& mat)
{
	//return glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4,
	//	mat.b1, mat.b2, mat.b3, mat.b4,
	//	mat.c1, mat.c2, mat.c3, mat.c4,
	//	mat.d1, mat.d2, mat.d3, mat.d4);

	return glm::mat4(mat.a1, mat.b1, mat.c1, mat.d1,
					mat.a2, mat.b2, mat.c2, mat.d2,
					mat.a3, mat.b3, mat.c3, mat.d3,
					mat.a4, mat.b4, mat.c4, mat.d4);
}

glm::mat4 AnimationModel::aiMat2GlmMat(const aiMatrix3x3& mat)
{
	//return glm::mat4(mat.a1, mat.a2, mat.a3, 0.0f,
	//	mat.b1, mat.b2, mat.b3, 0.0f,
	//	mat.c1, mat.c2, mat.c3, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f);
	return glm::mat4(mat.a1, mat.b1, mat.c1, 0.0f,
					mat.a2, mat.b2, mat.c2, 0.0f,
					mat.a3, mat.b3, mat.c3, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
}
