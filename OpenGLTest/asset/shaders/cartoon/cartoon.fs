#version 330 core
out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
};

uniform Material material;

in vec2 texCoord;
in vec3 vNormal;
in vec3 vLight;

void main()
{
	float diffuse = dot(normalize(vLight), vNormal);
	if (diffuse > 0.8) {
        diffuse = 1.0;
    }
    else if (diffuse > 0.5) {
        diffuse = 0.6;
    }
    else if (diffuse > 0.2) {
        diffuse = 0.4;
    }
    else {
        diffuse = 0.2;
    }
	
	//����view����ϵ�£�����������Ļ������z���Ƿ�ƽ�У�ƽ�е�ƬԪ���
	float silhouette = length(vNormal * vec3(0.0, 0.0, 1.0));
	silhouette = step(0.5, silhouette);
	
	//vec3 texColor = texture(material.texture_diffuse1, texCoord).rgb;
	vec3 texColor = vec3(0.5, 0.5, 0.7);
	texColor *= diffuse * silhouette;
	
	FragColor = vec4(texColor, 1.0);
};