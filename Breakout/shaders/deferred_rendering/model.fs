#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
};

in vec2 texCoord;
in vec3 worldNormal;

in vec4 lightSpacePos[3];
in float clipSpacePosZ;

uniform Material material;
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
	vec3 mixColor = texture(material.texture_diffuse1, texCoord).rgb;

	//shadow
	float ShadowFactor = 0.0;
    vec4 CascadeIndicator = vec4(0.0, 0.0, 0.0, 0.0);

    for (int i = 0 ; i < 3 ; i++) {
        if (clipSpacePosZ <= cascadeEndClipSpace[i]) {
            ShadowFactor = calcShadowFactor(i, lightSpacePos[i]);

            if (i == 0) 
                CascadeIndicator = vec4(0.1, 0.0, 0.0, 0.0);
            else if (i == 1)
                CascadeIndicator = vec4(0.0, 0.1, 0.0, 0.0);
            else if (i == 2)
                CascadeIndicator = vec4(0.0, 0.0, 0.1, 0.0);

            break;
        }
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