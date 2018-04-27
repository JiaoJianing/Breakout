#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texture1;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	//每个纹素大小
	vec2 tex_offset = 1.0 / textureSize(texture1, 0);
	vec3 result = texture(texture1, texCoord).rgb * weight[0];
	if (horizontal){
		for (int i=1; i<5; i++){
			result += texture(texture1, texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(texture1, texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else{
		for (int i=1; i<5; i++){
			result += texture(texture1, texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(texture1, texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}

    FragColor = vec4(result, 1.0);
};