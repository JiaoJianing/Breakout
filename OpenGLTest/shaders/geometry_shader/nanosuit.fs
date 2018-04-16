#version 330 core

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
};

out vec4 FragColor;

uniform Material material;

in vec2 texCoord;

void main()
{
	FragColor = texture(material.texture_diffuse1, texCoord);
};