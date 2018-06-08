#version 330 core
out vec4 FragColor;

uniform sampler2D texture_reflect;
uniform sampler2D texture_normal;
uniform sampler2D texture_dudv;
uniform sampler2D texture_refract;
uniform sampler2D texture_depth;
uniform vec4 waterColor;

in vec4 viewCoords;
in vec4 refrCoords;
in vec4 normCoords;
in vec4 viewTangetSpace;
in vec4 lightTangetSpace;

void main()
{
	const float kShine = 128.0;
	const float kDistortion = 0.015;
	const float kRefraction = 0.009;
	
	vec4 distOffset = texture(texture_dudv, normCoords.xy) * kDistortion;
	vec4 dudvColor = texture(texture_dudv, vec2(refrCoords + distOffset));
	dudvColor = normalize(dudvColor * 2.0 - 1.0) * kRefraction;
	
	vec4 normalVector = texture(texture_normal, vec2(refrCoords + distOffset));
	normalVector = normalVector * 2.0 - 1.0;
	normalVector.a = 0.0;
	
	vec4 lightReflection = normalize( reflect(-lightTangetSpace, normalVector) );
	
	vec4 invertedFresnel = vec4( dot(normalVector, lightReflection ) );
	vec4 fresnelTerm = 1.0 - invertedFresnel;
	
	vec4 projCoord = viewCoords / viewCoords.w;
	projCoord = (projCoord + 1.0) * 0.5;
	projCoord += dudvColor;
	projCoord = clamp(projCoord, 0.001, 0.999);
	
	vec4 reflectionColor  = texture(texture_reflect, projCoord.xy);
	vec4 refractionColor  = texture(texture_refract, projCoord.xy);
	vec4 depthValue = texture2D(texture_depth, projCoord.xy);
	
	vec4 invDepth = 1.0 - depthValue;
	refractionColor *= invertedFresnel;
	refractionColor +=  waterColor * invertedFresnel;

	reflectionColor *= fresnelTerm;
	
	vec4 localView = normalize(viewTangetSpace);		
	float intensity = max(0.0, dot(lightReflection, localView) );
	vec4 specular = vec4(pow(intensity, kShine));

	FragColor = refractionColor + reflectionColor + specular;
};