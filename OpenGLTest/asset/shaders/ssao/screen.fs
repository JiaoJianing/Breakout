#version 330 core

out vec4 FragColor;

uniform sampler2D texture_posDepth;
uniform sampler2D texture_normal;
uniform sampler2D texture_albedo;
uniform sampler2D texture_blur;

in vec2 texCoords;

struct Light{
	vec3 Position;
	vec3 Color;

	float Linear;
	float Quadratic;
	float Radius;
};
uniform Light light;

void main(){
	//G������ȡ����
	vec3 fragPos = texture(texture_posDepth, texCoords).xyz;
	vec3 normal = texture(texture_normal, texCoords).xyz;
	vec3 diffuseColor = texture(texture_albedo, texCoords).rgb;
	float ambientOcclusion = texture(texture_blur, texCoords).r;

	//Blinn-Phong����(�۲�ռ���)
	vec3 ambient = vec3(0.3 * ambientOcclusion);//�����ڱ�����
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-fragPos);
	//������
	vec3 lightDir = normalize(light.Position - fragPos);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * diffuseColor * light.Color;
	//����
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
	vec3 specular = light.Color * spec;
	//˥��
	float dist = length(light.Position - fragPos);
	float attenuation = 1.0 / (1.0 + light.Linear * dist + light.Quadratic * dist * dist);

	diffuse *= attenuation;
	specular *= attenuation;
	lighting += diffuse + specular;

	FragColor = vec4(lighting, 1.0);
}