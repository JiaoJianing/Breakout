#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexCoord;

out vec2 texCoord;

float near_plane = 0.1f;
float far_plane = 100.0f;

float linearizeDepth(float depth){
	float z = depth * 2.0 - 1.0;
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()						
{								
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	texCoord = aTexCoord;
};