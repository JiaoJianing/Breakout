#pragma once
#include <glad/glad.h>
#include <string>

class Shader
{
public:
	//程序ID
	unsigned int ID;

	//构造器读取并构建着色器
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	~Shader();

	//使用/激活程序
	void use();

	//uniform 工具函数
	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int value) const;

	void setFloat(const std::string& name, float value) const;

	void setFloat4(const std::string& name, float x, float y, float z, float w);

	void setMatrix4fv(const std::string& name, float* value);
};

