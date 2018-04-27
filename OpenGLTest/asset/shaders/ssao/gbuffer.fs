#version 330 core
layout (location = 0) out vec3 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec3 Normal;
in vec3 FragPos;

void main(){
	gPositionDepth.xyz = FragPos;
	gNormal = normalize(Normal);
	gAlbedo.rgb = vec3(0.95);
}