#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

uniform	mat4 view;
uniform	mat4 projection;
uniform mat4 model;
uniform mat3 normalMat;

out VS_OUT{
	vec2 texCoord;
}vs_out;
out vec3 normal;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	normal = normalMat * aNormal;
	vs_out.texCoord = aTexCoord;
};