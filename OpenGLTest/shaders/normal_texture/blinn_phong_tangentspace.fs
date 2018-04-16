#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform bool blinn;

void main()
{
	vec3 normal = texture(texture_normal, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 texColor = texture(texture_diffuse, texCoord).rgb;
	//������
	vec3 ambient = texColor * 0.1;
	//������
	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = texColor * diff;
	//�����
	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, normal), 0.0), 32);
	}else{	
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
	}
	vec3 specular = vec3(0.2) * spec;
	
	FragColor = vec4(ambient + diffuse + specular, 1.0);
};