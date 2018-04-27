#version 330 core					
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view ;
uniform mat4 projection;
uniform bool reverse_normal;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

void main()						
{								
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	fragPos = (model * vec4(aPos, 1.0)).xyz;
	texCoord = aTexCoord;
	mat3 normalMat = transpose(inverse(mat3(model)));
	if (reverse_normal){
		normal = normalMat * -aNormal;
	}else{
		normal = normalMat * aNormal;
	}
};