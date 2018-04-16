#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;

layout (std140) uniform Matrices{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;

out vec3 Position;
out vec3 Normal;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	Position = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
};