#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
};

in vec3 fragPos;
in vec4 fragPosLightSpace;
in vec3 normal;
in vec2 texCoord;

uniform Material material;
uniform sampler2D depthTexture;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform bool blinn;

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir){
	//ִ��͸�ӳ���
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//�任��[0,1]
	projCoords = projCoords * 0.5 + 0.5;
	//ȡ�������ͼ��¼����������
	float closestDepth = texture(depthTexture, projCoords.xy).r;
	//ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
	float currentDepth = projCoords.z;
	//���ƬԪ�Ƿ�����Ӱ��
	float bias = 0.002;
	//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//if (projCoords.z > 1.0){
		//shadow = 0.0;
	//}

	return shadow;
}

float shadowCalculationPcf(vec4 fragPosLightSpace){
	//ִ��͸�ӳ���
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//�任��[0,1]
	projCoords = projCoords * 0.5 + 0.5;
	//ȡ�������ͼ��¼����������
	float closestDepth = texture(depthTexture, projCoords.xy).r;
	//ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
	float currentDepth = projCoords.z;
	//���ƬԪ�Ƿ�����Ӱ��
	float bias = 0.002;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
	for (int x = -1; x <= 1; ++x){
		for (int y = -1; y <= 1; ++y){
			float pcfDepth = texture(depthTexture, projCoords.xy + vec2(x, y)* texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return shadow;
}

void main()
{
	vec3 texColor = texture(material.texture_diffuse1, texCoord).rgb;
	//������
	vec3 ambient = texColor * 0.05;
	//������
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 norNormal = normalize(normal);
	float diff = max(dot(lightDir, norNormal), 0.0);
	vec3 diffuse = texColor * diff;
	//�����
	vec3 viewDir = normalize(viewPos - fragPos);
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, norNormal), 0.0), 16);
	}else{	
		vec3 reflectDir = reflect(-lightDir, norNormal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 16);
	}
	vec3 specular = vec3(0.3) * spec;

	//������Ӱ
	//float shadow = shadowCalculation(fragPosLightSpace, norNormal, lightDir);
	float shadow = shadowCalculationPcf(fragPosLightSpace);
	vec3 lighting = (ambient + (1-shadow) * (diffuse + specular)) * texColor;
	
	FragColor = vec4(lighting, 1.0);
};