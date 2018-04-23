#version 330 core
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 Normal;
in vec3 FragPos;

uniform float nearPlane;
uniform float farPlane;

float linearizeDepth(float depth){
	float z = depth * 2.0 - 1.0; // to NDC
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main(){
	gPositionDepth.xyz = FragPos;
	gPositionDepth.a = linearizeDepth(gl_FragCoord.z);
	gNormal = normalize(Normal);
	gAlbedoSpec.rgb = vec3(0.95);
}