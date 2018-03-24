#pragma once
#include <glad/glad.h>
#include <string>

class Shader
{
public:
	//����ID
	unsigned int ID;

	//��������ȡ��������ɫ��
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	~Shader();

	//ʹ��/�������
	void use();

	//uniform ���ߺ���
	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int value) const;

	void setFloat(const std::string& name, float value) const;

	void setFloat4(const std::string& name, float x, float y, float z, float w);

	void setMatrix4fv(const std::string& name, float* value);
};

