#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	//计算片元和光源距离
	float lightDistance = length(FragPos.xyz - lightPos);

	//映射到[0,1]
	lightDistance = lightDistance / far_plane;

	//写入深度
	gl_FragDepth = lightDistance;
};