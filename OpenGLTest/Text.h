#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm.hpp>
#include <map>
#include "Shader.h"

struct Character {
	unsigned int textureID;//字形纹理ID
	glm::ivec2 size;//字形大小
	glm::ivec2 bearing;//从基准线到字形左部/顶部的偏移值
	unsigned int advance;//原点距离下一个字形原点的距离
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

