#version 330 core

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	//属性
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPos - fragPos);

	//第一阶段 定向光
	vec3 result = calcDirLight(dirLight, norm, viewDir);
	//第二阶段 点光源
	for (int i=0; i< NR_POINT_LIGHTS; i++){
		result += calcPointLight(pointLights[i], norm, fragPos, viewDir);
	}
	//第三阶段 聚光
	result += calcSpotLight(spotLight, norm, fragPos, viewDir);
	
	FragColor = vec4(result, 1.0);
};

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = normalize(-light.direction);

	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	//镜面光
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	//合并结果
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb;
	
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);

	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	//镜面光
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	//衰减
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	//合并结果
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb;
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);

	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	//镜面光
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	//衰减
	float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	//合并结果
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}