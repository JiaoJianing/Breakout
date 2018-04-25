#version 330 core
out vec4 FragColor;

in vec2 texCoords;
in vec3 worldPos;
in vec3 normal;

uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughness;
uniform sampler2D texture_ao;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 viewPos;

const float PI = 3.14159265359;

const vec3 fogColor = vec3(0.5, 0.5, 0.5);
const float maxFogDist = 30;
const float minFogDist = 0.01;

vec3 getNormalFromTexture(){
	vec3 tangentNormal = texture(texture_normal, texCoords).xyz * 2.0 - 1.0;

	vec3 Q1 = dFdx(worldPos);
	vec3 Q2 = dFdy(worldPos);
	vec2 st1 = dFdx(texCoords);
	vec2 st2 = dFdy(texCoords);

	vec3 N = normalize(normal);
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

float distributionGGX(vec3 N, vec3 H, float roughness){
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.001);
}

float geometrySchlickGGX(float NdotV, float roughness){
	float r = (roughness = 1.0);
	float k = (r*r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 = (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main(){

	vec3 albedo = pow(texture(texture_albedo, texCoords).rgb, vec3(2.2));
	float metallic = texture(texture_metallic, texCoords).r;
	float roughness = texture(texture_roughness, texCoords).r;
	float ao = texture(texture_ao, texCoords).r;

	vec3 N = getNormalFromTexture();
	vec3 V = normalize(viewPos - worldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for (int i=0; i<4; i++){
		vec3 L = normalize(lightPositions[i] - worldPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - worldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		//BRDF
		float NDF = distributionGGX(N, H, roughness);
		float G = geometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.0) *  max(dot(N, L), 0.0);
		vec3 specular = nominator / max(denominator, 0.001);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(N, L), 0.0);

		Lo += (kD * albedo/ PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * albedo * ao;

	vec3 color = ambient + Lo;

	//色调映射
	color = color / (color + vec3(1.0));
	//gamma校正
	color = pow(color, vec3(1.0/2.2));

	//混合雾的颜色
	float dist = length(viewPos - worldPos);
	float fogFactor = (maxFogDist - dist) / (maxFogDist - minFogDist);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	color = mix(fogColor, color, fogFactor);

	FragColor = vec4(color, 1.0);
}