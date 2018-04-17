#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_reflect1;
	sampler2D texture_normal1;
	sampler2D texture_parallax1;
};

in vec2 texCoord;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

uniform Material material;
uniform bool blinn;

float height_scale = 0.1f;

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir){
	//定义逐步逼近的层数
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	//每一层的深度
	float layerDepth = 1.0 / numLayers;
	//当前到达的深度
	float currentLayerDepth = 0.0;
	//纹理坐标每一层的偏移量
	vec2 p = viewDir.xy * height_scale;//沿着视线方向在xy平米投影逼近
	vec2 deltaTexCoords = p / numLayers;

	//开始
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(material.texture_parallax1, currentTexCoords).r;

	while (currentLayerDepth < currentDepthMapValue){
		//纹理坐标在p方向叠加
		currentTexCoords -= deltaTexCoords;
		//取得当前层的深度
		currentDepthMapValue = texture(material.texture_parallax1, currentTexCoords).r;
		//进入下一层
		currentLayerDepth += layerDepth;
	}

	//获取上一个纹理坐标
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(material.texture_parallax1, prevTexCoords).r - currentDepthMapValue + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

void main()
{
	//使用视差贴图计算偏移后的uv坐标
	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	vec2 texCoords = parallaxMapping(texCoord, viewDir);
	if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0){
		discard;
	}

	vec3 normal = texture(material.texture_normal1, texCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 texColor = texture(material.texture_diffuse1, texCoords).rgb;
	//环境光
	vec3 ambient = texColor * 0.1;
	//漫反射
	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = texColor * diff;
	//镜面光
	float spec = 0.0;
	if (blinn){
		vec3 halfWayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfWayDir, normal), 0.0), 32);
	}else{	
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
	}
	vec3 specular = texture(material.texture_specular1, texCoords).rgb * spec;
	
	FragColor = vec4(ambient + diffuse + specular, 1.0);
};