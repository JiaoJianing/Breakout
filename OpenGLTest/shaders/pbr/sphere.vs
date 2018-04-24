#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;
out vec3 worldPos;
out vec3 normal;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	texCoords = aTexCoord;
	worldPos = vec3(model * vec4(aPos, 1.0));
	normal = mat3(model) * aNormal;
}