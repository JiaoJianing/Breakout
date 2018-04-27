#pragma once
#include "GameObject.h"
#include <vector>

class GameLevel
{
public:
	GameLevel();
	~GameLevel();

	//从文件加载关卡
	void Load(std::string path, unsigned int levelWidth, unsigned int levelHeight);

	//渲染关卡
	void Draw(SpriteRenderer& renderer);

	//关卡是否通关
	bool IsCompleted();

	std::vector<GameObject> Bricks;

private:
	//从砖块数据初始化关卡
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

