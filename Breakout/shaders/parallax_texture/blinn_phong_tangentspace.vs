#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;
layout (location=3) in vec3 aTangent;
layout (location=4) in vec3 aBiTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec2 texCoord;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	vec3 fragPos = vec3(model * vec4(aPos, 1.0));
	texCoord = aTexCoord;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 B = normalize(normalMatrix * aBiTangent);
	vec3 N = normalize(normalMatrix * aNormal);

	mat3 TBN = transpose(mat3(T, B, N));
	tangentLightPos = TBN * lightPos;
	tangentViewPos = TBN * viewPos;
	tangentFragPos = TBN * fragPos;
};