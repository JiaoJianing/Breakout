#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	//����ƬԪ�͹�Դ����
	float lightDistance = length(FragPos.xyz - lightPos);

	//ӳ�䵽[0,1]
	lightDistance = lightDistance / far_plane;

	//д�����
	gl_FragDepth = lightDistance;
};