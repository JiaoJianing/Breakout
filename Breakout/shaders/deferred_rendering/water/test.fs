#version 330 core
out vec4 FragColor;

uniform sampler2D scene;

in vec2 texCoord;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * 0.1 * 1000.0) / (1000.0 + 0.1 - z * (1000.0 - 0.1));    
}

void main()
{	
	float depth = LinearizeDepth(texture(scene, texCoord).r);
	FragColor = vec4(vec3(depth),1.0);

	FragColor = texture(scene, texCoord);
};