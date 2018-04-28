#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;
layout (location=3) in vec3 aTangent;
layout (location=4) in vec3 aBiTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
out mat3 TBN;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	fragPos = vec3(model * vec4(aPos, 1.0));
	texCoord = aTexCoord;

	vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(model * vec4(aBiTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
	TBN = mat3(T, B, N);
};