#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec2 texCoord;
in mat3 TBN;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform vec3 lightPos;
uniform bool blinn;

void main()
{
	vec3 normal = texture(texture_normal, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 texColor = texture(texture_diffuse, texCoord).rgb;
	//环境光
	vec3 ambient = texColor * 0.1;
	//漫反射
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 norNormal = normalize(normal);
	//vec3 norNormal = normalize(normal);
	float diff = max(dot(lightDir, norNormal), 0.0);
	vec3 diffuse = texColor * diff;
	//镜面光
	vec3 viewDir = normalize(viewPos - fragPos);
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, norNormal), 0.0), 32);
	}else{	
		vec3 reflectDir = reflect(-lightDir, norNormal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
	}
	vec3 specular = vec3(0.2) * spec;
	
	FragColor = vec4(ambient + diffuse + specular, 1.0);
};