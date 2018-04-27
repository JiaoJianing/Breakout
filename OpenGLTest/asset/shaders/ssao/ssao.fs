#version 330 core

out float FragColor;

in vec2 texCoords;

uniform sampler2D texture_posDepth;
uniform sampler2D texture_normal;
uniform sampler2D texture_noise;

uniform vec3 samples[64];
uniform mat4 projection;

uniform float screenWidth;
uniform float screenHeight;

//屏幕的平铺噪声纹理会根据屏幕分辨率除以噪声大小值来决定
const int kernelSize = 64;
const float radius = 1.0;

void main(){
	vec2 noiseScale = vec2(screenWidth / 4.0, screenHeight / 4.0);

	vec3 fragPos = texture(texture_posDepth, texCoords).xyz;
	vec3 normal = texture(texture_normal, texCoords).xyz;
	vec3 randomVec = texture(texture_noise, texCoords * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i=0; i<kernelSize; i++){
		vec3 sample = TBN * samples[i];
		sample = fragPos + sample * radius;

		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;//view->projection
		offset.xyz /= offset.w;//透视划分
		offset.xyz = offset.xyz * 0.5 + 0.5; //to [0.0, 1.0]

		float sampleDepth = texture(texture_posDepth, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

		occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - occlusion / kernelSize;
	FragColor = occlusion;
}