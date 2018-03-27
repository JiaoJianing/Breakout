#pragma once
#include <string>

//贴图信息
struct Texture {
	unsigned int id;
	std::string type;
	std::string path;//记录已加载的纹理路径 防止重复加载
};