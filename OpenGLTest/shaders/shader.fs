#version 330 core

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

void main()
{
	//环境光
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).xyz;

	//漫反射
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).xyz;

	//镜面光
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	vec3 specular = light.specular * (spec * texture(material.specular, texCoord).xyz);

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
};