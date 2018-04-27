#version 330 core
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 Position;
in vec3 Normal;

void main()
{
	//����
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, Normal);
	
	//����
	//������ ����==>���� ����������/����������
	float refractRatio = 1.0f / 1.52f;
	R = refract(I, Normal, refractRatio);

	//���
	FragColor = texture(skybox, R);
};