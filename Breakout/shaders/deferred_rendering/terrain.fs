#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 localNormal;
in vec3 worldNormal;

in vec4 lightSpacePos[3];
in float clipSpacePosZ;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snow;
uniform float heightThreshold;//阈值之上用snow 之下根据陡峭程度混合grass和rock

uniform sampler2D texture_shadow[3];
uniform float cascadeEndClipSpace[3];
uniform vec3 lightDirection;

float calcShadowFactor(int index, vec4 lSpacePos){
	vec3 projCoords = lSpacePos.xyz / lSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

	float bias = 0.001;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(texture_shadow[index], 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(texture_shadow[index], projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0){
		shadow = 0.0;
	}

	return shadow;
}

void main()
{
	vec3 grassColor = texture(texture_grass, texCoord * 16.0).rgb;
	vec3 rockColor = texture(texture_rock, texCoord * 32.0).rgb;
	vec3 snowColor = texture(texture_snow, texCoord * 32.0).rgb;

	vec3 normal = normalize(localNormal);
	float factor = dot(normal, vec3(0.0, 1.0, 0.0));
	vec3 mixColor = mix(grassColor, rockColor, 1 - factor);

	factor = fragPos.y / heightThreshold;
	factor = pow(factor, 4);
	mixColor = mix(snowColor, mixColor, 1 - factor);

	//shadow
	float ShadowFactor = 0.0;
	vec4 CascadeIndicator = vec4(0.0, 0.0, 0.0, 0.0);

	if (clipSpacePosZ <= cascadeEndClipSpace[0]){
		float ShadowFactor0 = calcShadowFactor(0, lightSpacePos[0]);
		float ShadowFactor1 = calcShadowFactor(1, lightSpacePos[1]);
		CascadeIndicator = vec4(0.1, 0.0, 0.0, 0.0);
		ShadowFactor = mix(ShadowFactor0, ShadowFactor1, clipSpacePosZ / cascadeEndClipSpace[0]);
	}else if (clipSpacePosZ <= cascadeEndClipSpace[1]){
		float ShadowFactor1 = calcShadowFactor(1, lightSpacePos[1]);
		float ShadowFactor2 = calcShadowFactor(2, lightSpacePos[2]);
		CascadeIndicator = vec4(0.0, 0.1, 0.0, 0.0);
		ShadowFactor = mix(ShadowFactor1, ShadowFactor2, (clipSpacePosZ-cascadeEndClipSpace[0]) / (cascadeEndClipSpace[1]-cascadeEndClipSpace[0]));
	}else if (clipSpacePosZ <= cascadeEndClipSpace[2]){
		float ShadowFactor2 = calcShadowFactor(2, lightSpacePos[2]);
        CascadeIndicator = vec4(0.0, 0.0, 0.1, 0.0);
		ShadowFactor = ShadowFactor2;
	}

	//光源方向
	vec3 lightDir = normalize(lightDirection);
	//环境光
	vec3 ambient = mixColor * 0.3;
	//漫反射
	float diff = max(dot(worldNormal, lightDir), 0.0);
	vec3 diffuse = mixColor * diff;
	
	vec3 result = ambient + (1-ShadowFactor) * diffuse;

	FragColor = vec4(result, 1.0);
};