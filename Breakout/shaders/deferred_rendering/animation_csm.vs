#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexCoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in ivec4 aBoneIDs;
layout (location=4) in vec4 aWeights;

const int MAX_BONES = 100;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 gBones[MAX_BONES];

void main()						
{			
    mat4 BoneTransform = gBones[aBoneIDs[0]] * aWeights[0];
    BoneTransform     += gBones[aBoneIDs[1]] * aWeights[1];
    BoneTransform     += gBones[aBoneIDs[2]] * aWeights[2];
    BoneTransform     += gBones[aBoneIDs[3]] * aWeights[3];

	gl_Position = projection * model * BoneTransform * vec4(aPos, 1.0);
};