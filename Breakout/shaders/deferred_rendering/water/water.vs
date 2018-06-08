#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aRefrCoord;
layout (location=2) in vec2 aNormCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 lightPos;

out vec4 viewCoords;
out vec4 refrCoords;
out vec4 normCoords;
out vec4 viewTangetSpace;
out vec4 lightTangetSpace;

void main()						
{
	// Because we have a flat plane for water we already know the vectors for tangent space
	vec3 tangent = vec3(1.0, 0.0, 0.0);
	vec3 normal = vec3(0.0, 1.0, 0.0);
	vec3 biTangent = vec3(0.0, 0.0, 1.0);
	
	vec3 viewDir = viewPos - aPos;
	viewTangetSpace.x = dot(viewDir, tangent);
	viewTangetSpace.y = dot(viewDir, biTangent);
	viewTangetSpace.z = dot(viewDir, normal);
	viewTangetSpace.w = 1.0;
	
	vec3 lightDir = lightPos - aPos;
	lightTangetSpace.x = dot(lightDir, tangent);
	lightTangetSpace.y = dot(lightDir, biTangent);
	lightTangetSpace.z = dot(lightDir, normal);
	lightTangetSpace.w = 1.0;
	
	viewCoords = projection * view * vec4(aPos, 1.0);
	refrCoords = vec4(aRefrCoord, 0.0, 0.0);
	normCoords = vec4(aNormCoord, 0.0, 0.0);
	
	gl_Position = viewCoords;
};