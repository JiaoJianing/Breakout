#version 330 core
out vec4 FragColor;

uniform samplerCube texture_skybox;

in vec3 texCoord;

void main()
{
	FragColor = texture(texture_skybox, texCoord);
};