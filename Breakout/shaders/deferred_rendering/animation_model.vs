#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexCoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in ivec4 aBoneIDs;
layout (location=4) in vec4 aWeights;

const int MAX_BONES = 100;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightMVP[3];
uniform mat4 gBones[MAX_BONES];

out vec2 texCoord;

out vec4 lightSpacePos[3];
out float clipSpacePosZ;
out vec3 worldNormal;

void main()						
{				
    mat4 BoneTransform = gBones[aBoneIDs[0]] * aWeights[0];
    BoneTransform     += gBones[aBoneIDs[1]] * aWeights[1];
    BoneTransform     += gBones[aBoneIDs[2]] * aWeights[2];
    BoneTransform     += gBones[aBoneIDs[3]] * aWeights[3];

	vec4 pos = BoneTransform * vec4(aPos, 1.0);
	gl_Position = projection * view * model * pos;
	texCoord = aTexCoord;
	worldNormal = normalize(transpose(inverse(mat3(model * BoneTransform))) * aNormal);

	for (int i=0; i<3; i++){
		lightSpacePos[i] = lightMVP[i] * model * pos;
	}
	clipSpacePosZ = gl_Position.z;
};