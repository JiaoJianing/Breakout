#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm.hpp>
#include <map>
#include "Shader.h"

struct Character {
	unsigned int textureID;//��������ID
	glm::ivec2 size;//���δ�С
	glm::ivec2 bearing;//�ӻ�׼�ߵ�������/������ƫ��ֵ
	unsigned int advance;//ԭ�������һ������ԭ��ľ���
};

class Text
{
public:
	~Text();

	static Text* getInstance();
	static void deleteInstance();

	void Draw(Shader shader, const std::wstring& text, float x, float y, float scale, glm::vec3 color);

private:
	Text();
	Text(const Text& text);
	Text& operator=(const Text& text);

	void init();

	Character loadChar(wchar_t ch);

private:
	FT_Library m_Ft;
	FT_Face m_Face;
	std::map<wchar_t, Character> m_Characters;
	static Text* m_Instance;
	unsigned int m_VAO, m_VBO;
};

