#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
	sampler2D texture_normal1;
};

in vec2 texCoord;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

uniform Material material;
uniform bool blinn;

void main()
{
	vec3 normal = texture(material.texture_normal1, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 texColor = texture(material.texture_diffuse1, texCoord).rgb;
	//环境光
	vec3 ambient = texColor * 0.1;
	//漫反射
	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = texColor * diff;
	//镜面光
	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, normal), 0.0), 32);
	}else{	
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
	}
	vec3 specular = texture(material.texture_specular1, texCoord).rgb * spec;
	
	FragColor = vec4(ambient + diffuse + specular, 1.0);
};