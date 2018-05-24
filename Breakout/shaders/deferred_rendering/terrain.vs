#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
out vec3 fragPos;
out vec3 localNormal;

void main()						
{								
	gl_Position = projection * view * vec4(aPos, 1.0);
	texCoord = aTexCoord;
	fragPos = aPos;
	localNormal = aNormal;
};