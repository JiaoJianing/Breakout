#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 localNormal;

in vec4 lightSpacePos[3];
in float clipSpacePosZ;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snow;
uniform float heightThreshold;//阈值之上用snow 之下根据陡峭程度混合grass和rock

uniform sampler2D texture_shadow[3];
uniform float cascadeEndClipSpace[3];

float CalcShadowFactor(int CascadeIndex, vec4 LightSpacePos)                                                  
{                                                                                           
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;                                  
    vec2 UVCoords;                                                                          
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;                                                  
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;                                                  
    float z = 0.5 * ProjCoords.z + 0.5;                                                     
    float Depth = texture(texture_shadow[CascadeIndex], UVCoords).x;                                          
    if (Depth < z + 0.00001)                                                                 
        return 0.5;                                                                         
    else                                                                                    
        return 1.0;                                                                         
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
	float ShadowFactor = CalcShadowFactor(0, lightSpacePos[0]);
    vec4 CascadeIndicator = vec4(0.0, 0.0, 0.0, 0.0);

    for (int i = 0 ; i < 3 ; i++) {
        if (clipSpacePosZ <= cascadeEndClipSpace[i]) {
            ShadowFactor = CalcShadowFactor(i, lightSpacePos[i]);

            if (i == 0) 
                CascadeIndicator = vec4(0.1, 0.0, 0.0, 0.0);
            else if (i == 1)
                CascadeIndicator = vec4(0.0, 0.1, 0.0, 0.0);
            else if (i == 2)
                CascadeIndicator = vec4(0.0, 0.0, 0.1, 0.0);

            break;
        }
   }

	FragColor = vec4(mixColor * ShadowFactor, 1.0);
};