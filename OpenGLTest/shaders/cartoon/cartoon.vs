#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;

out vec2 texCoord;
out vec3 vNormal;
out vec3 vLight;

void main()						
{
	mat4 viewModelMat = view * model;
	gl_Position = projection * viewModelMat * vec4(aPos, 1.0);
	mat3 normalMat = transpose(inverse(mat3(viewModelMat)));
	texCoord = aTexCoord;
	vNormal = normalize(normalMat * aNormal);
	vLight = (viewModelMat * vec4(lightPos, 1.0)).xyz;
};