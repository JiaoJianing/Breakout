#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texture1;

const float offset = 1.0 / 300.0;

void main()
{
	vec4 texColor = texture(texture1, texCoord);
	vec4 resultColor = texColor;
	//resultColor = vec4(vec3(1) - texColor.rgb, 1.0);//反相

	//float average = (resultColor.r + resultColor.g + resultColor.b) / 3.0;
	//float average = 0.2126 * resultColor.r + 0.7152 * resultColor.g + 0.0722 * resultColor.b; //加权的平均方式，效果更好
	//resultColor = vec4(average, average, average, 1.0);//灰度

	//核 kernel(卷积矩阵)
	vec2 offset[9] = vec2[](
		vec2(-offset, offset),//左上
		vec2(0.0f, offset),//正上
		vec2(offset, offset),//右上
		vec2(-offset, 0.0f),//左
		vec2(0.0f, 0.0f),//中
		vec2(offset, 0.0f),//右
		vec2(-offset, -offset),//左下
		vec2(0.0f, -offset),//正下
		vec2(offset, -offset)//右下
	);

	//锐化核
	//float kernel[9] = float[](
		//-1, -1, -1,
		//-1, 9, -1,
		//-1, -1, -1
	//);

	//模糊核
	float kernel[9] = float[](
		1.0f / 16, 2.0 / 16, 1.0 / 16,
		2.0f / 16, 4.0 / 16, 2.0 / 16,
		1.0f / 16, 2.0 / 16, 1.0 / 16
	);

	//边缘检测核
	//float kernel[9] = float[](
		//1, 1, 1,
		//1, -8, 1,
		//1, 1, 1
	//);

	vec3 sampleTex[9];
	for (int i=0; i<9; i++){
		sampleTex[i] = vec3(texture(texture1, texCoord.st + offset[i]));
	}
	vec3 col = vec3(0.0);
	for (int i=0; i<9; i++){
		col += sampleTex[i] * kernel[i];
	}

	resultColor = vec4(col, 1);

	FragColor = resultColor;
};