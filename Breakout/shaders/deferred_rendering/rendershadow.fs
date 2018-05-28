#version 330 core

out vec4 FragColor;

uniform sampler2D texture_shadow;

in vec2 texCoord;

void main()
{
	float depth = texture(texture_shadow, texCoord).r;
	FragColor = vec4(vec3(depth), 1.0);
};