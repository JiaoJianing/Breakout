#version 330
layout (location=0) in vec4 vertex;

out vec2 texCoord;

void main()						
{
	gl_Position = vec4(vertex.xy, 0.0, 1.0);
	vec2 uv = vertex.zw;

	texCoord = uv;
};