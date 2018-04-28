#version 330 core
layout (location = 0) out vec3 PosColor;
layout (location = 1) out vec3 NormalColor;
layout (location = 2) out vec4 FragColorSpecular;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};

uniform Material material;

void main()
{
	PosColor = fragPos;
	NormalColor = normal;
	FragColorSpecular.rgb = texture(material.texture_diffuse1, texCoord).rgb;
	FragColorSpecular.a = texture(material.texture_specular1, texCoord).r;
};