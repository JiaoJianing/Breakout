#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform vec3 cubeColor;
uniform sampler2D cubeTexture;
uniform vec3 viewPos;
uniform bool blinn;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

void main()
{
	vec3 texColor = texture(cubeTexture, texCoord).rgb;
	vec3 norNormal = normalize(normal);
	//ambient
	vec3 ambient = 0.1 * texColor;
	//lighting
	vec3 lighting = vec3(0.0);
	for (int i=0; i<4; i++){
		//diffuse
		vec3 lightDir = normalize(lightPositions[i] - fragPos);
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 diffuse = lightColors[i] * diff * texColor;
		vec3 result = diffuse;
		//attenuation
		float distance = length(fragPos - lightPositions[i]);
		result *=  1.0 / (distance * distance);
		lighting += result;
	}

	FragColor = vec4(ambient + lighting, 1.0);

	//需要bloom的明亮部分输出到第二个渲染目标
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0){
		BrightColor = FragColor;
	}
};