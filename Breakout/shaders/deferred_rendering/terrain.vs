#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightMVP[3];

out vec2 texCoord;
out vec3 fragPos;
out vec3 localNormal;

out vec4 lightSpacePos[3];
out float clipSpacePosZ;

void main()						
{							
	vec4 pos = vec4(aPos, 1.0);
	gl_Position = projection * view * pos;
	texCoord = aTexCoord;
	fragPos = aPos;
	localNormal = aNormal;

	for (int i=0; i<3; i++){
		lightSpacePos[i] = lightMVP[i] * pos;
	}
	clipSpacePosZ = gl_Position.z;
};