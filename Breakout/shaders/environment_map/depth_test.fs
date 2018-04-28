#version 330 core
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 Position;
in vec3 Normal;

void main()
{
	//反射
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, Normal);
	
	//折射
	//折射率 空气==>玻璃 空气折射率/玻璃折射率
	float refractRatio = 1.0f / 1.52f;
	R = refract(I, Normal, refractRatio);

	//结果
	FragColor = texture(skybox, R);
};