#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 localNormal;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snow;
uniform float heightThreshold;//阈值之上用snow 之下根据陡峭程度混合grass和rock
uniform float waterHeight;

void main()
{
	if (fragPos.y <= waterHeight){
		discard;
	}
	
	vec3 grassColor = texture(texture_grass, texCoord * 16.0).rgb;
	vec3 rockColor = texture(texture_rock, texCoord * 32.0).rgb;
	vec3 snowColor = texture(texture_snow, texCoord * 32.0).rgb;

	vec3 normal = normalize(localNormal);
	float factor = dot(normal, vec3(0.0, 1.0, 0.0));
	vec3 mixColor = mix(grassColor, rockColor, 1 - factor);

	factor = abs(fragPos.y) / heightThreshold;
	factor = pow(factor, 4);
	mixColor = mix(snowColor, mixColor, 1 - factor);

	FragColor = vec4(mixColor, 1.0);
};