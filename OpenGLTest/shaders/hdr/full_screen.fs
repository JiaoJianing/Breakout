#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D hdrTexture;
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrTexture, texCoord).rgb;

    // �ع�ɫ��ӳ��
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // GammaУ�� 
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
};