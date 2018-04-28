#version 330 core
out vec4 FragColor;

uniform sampler2D texture1;

in vec2 texCoord;

float near = 0.1;
float far = 100.0;

//��������zֵת��Ϊ���Ե�
float linearizeDepth(float depth){
	float z = depth * 2.0 - 1.0; //ת����-1~1
	return (2.0 * near * far) /(far + near - z * (far - near));
} 
void main()
{
	vec4 texColor = texture(texture1, texCoord);
	if (texColor.a < 0.01){
		discard;
	}
	FragColor = texColor;

	//�����Ȼ����е�zֵ(Ĭ�ϣ�������,���ھ�����ͶӰ�任�Ĵ���)
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

	//�������z
	//float depth = linearizeDepth(gl_FragCoord.z) / far;
	//FragColor = vec4(vec3(depth), 1.0);
};