#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texture_position;
uniform sampler2D texture_normal;
uniform sampler2D texture_colorSpecular;

struct Light{
	vec3 Position;
	vec3 Color;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
	vec3 fragPos = texture(texture_colorSpecular, texCoord).rgb;
	vec3 normal = texture(texture_normal, texCoord).xyz;
	vec3 texColor = texture(texture_colorSpecular, texCoord).rgb;
	float Specular = texture(texture_colorSpecular, texCoord).a;

	vec3 viewDir = normalize(viewPos - fragPos);
	normal = normalize(normal);

	//计算光照
	//环境光
	vec3 lighting = texColor * 0.05;
	for (int i=0; i<NR_LIGHTS; i++){
		vec3 lightDir = normalize(lights[i].Position - fragPos);
		//漫反射
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 diffuse = texColor * diff * lights[i].Color;
		//镜面光
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
		vec3 specular = lights[i].Color * spec * Specular;
		//衰减
		float distance = length(lights[i].Position - fragPos);
		float attenuation = 1.0 / (1.0 + 0.7 * distance + 1.8 * distance * distance);

		diffuse *= attenuation;
		specular *= attenuation;

		lighting = lighting + diffuse + specular;
	}

	FragColor = vec4(lighting, 1.0);
};