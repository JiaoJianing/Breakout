#version 330 core

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
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
uniform samplerCube skybox;
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

	//计算反射
	vec3 I = normalize(fragPos - viewPos);
	vec3 R = reflect(I, normalize(normal));
	result += (texture(skybox, R) * texture(material.texture_reflect1, texCoord)).rgb;

	//计算折射
	//折射率 空气==>玻璃 空气折射率/玻璃折射率
	float refractRatio = 1.0f / 1.52f;
	R = refract(I, normalize(normal), refractRatio);
	
	FragColor = vec4(result, 1.0);
	//FragColor = texture(skybox, R);
};

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = normalize(-light.direction);

	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	//镜面光
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
	//合并结果
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoord).rgb;
	
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
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoord).rgb;
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
	vec3 ambient = light.ambient * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, texCoord).rgb;
	vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoord).rgb;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}