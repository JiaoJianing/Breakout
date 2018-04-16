#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
};

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform Material material;
uniform samplerCube depthTexture;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform bool blinn;
uniform float far_plane;

float shadowCalculation(){
	vec3 fragToLight = fragPos - lightPos;
	float closetDepth = texture(depthTexture, fragToLight).r;
	closetDepth *= far_plane;
	float currentDepth = length(fragToLight);
	float bias = 0.05;
	float shadow = currentDepth - bias > closetDepth ? 1.0 : 0.0;

	return shadow;
}

void main()
{
	vec3 texColor = texture(material.texture_diffuse1, texCoord).rgb;
	vec3 norNormal = normalize(normal);
	vec3 lightColor = vec3(0.3);
	//环境光
	vec3 ambient = texColor * 0.3;
	//漫反射
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(lightDir, norNormal), 0.0);
	vec3 diffuse = lightColor * diff;
	//镜面光
	vec3 viewDir = normalize(viewPos - fragPos);
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, norNormal), 0.0), 64);
	}else{	
		vec3 reflectDir = reflect(-lightDir, norNormal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 64);
	}
	vec3 specular = lightColor * spec;

	//计算阴影
	float shadow = shadowCalculation();
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor;
	

	vec3 fragToLight = fragPos - lightPos;
	float closetDepth = texture(depthTexture, fragToLight).r;
	closetDepth *= far_plane;

	FragColor = vec4(lighting, 1.0);
};