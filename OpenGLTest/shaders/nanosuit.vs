#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

layout (std140) uniform Matrices{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;
uniform mat3 normalMat;

out vec3 normal;
out vec3 fragPos;
out vec2 texCoord;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	normal = normalMat * aNormal;
	fragPos = vec3(model * vec4(aPos, 1.0));
	texCoord = aTexCoord;
};