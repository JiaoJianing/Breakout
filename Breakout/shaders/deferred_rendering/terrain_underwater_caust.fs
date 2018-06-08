#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 localNormal;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snow;
uniform sampler2D texture_caust;
uniform float heightThreshold;//阈值之上用snow 之下根据陡峭程度混合grass和rock
uniform float waterHeight;
uniform float caustScale;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 viewPos;

void main()
{
	if (fragPos.y >= waterHeight){
		discard;
	}
	
	vec3 grassColor = texture(texture_grass, texCoord * 16.0).rgb;
	vec3 rockColor = texture(texture_rock, texCoord * 32.0).rgb;
	vec3 snowColor = texture(texture_snow, texCoord * 32.0).rgb;
	vec3 caustColor = texture(texture_caust, texCoord * caustScale).rgb;

	vec3 normal = normalize(localNormal);
	float factor = dot(normal, vec3(0.0, 1.0, 0.0));
	vec3 mixColor = mix(grassColor, rockColor, 1 - factor);

	factor = abs(fragPos.y) / heightThreshold;
	factor = pow(factor, 4);
	mixColor = mix(snowColor, mixColor, 1 - factor);
	
	mixColor = mix(caustColor, mixColor, 0.5);
	
	float dist = length(viewPos - fragPos);
	float fogFactor = (fogEnd - dist) / (fogEnd - fogStart);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	mixColor = mix(fogColor, mixColor, 1 - fogFactor);

	FragColor = vec4(mixColor, 1.0);
};