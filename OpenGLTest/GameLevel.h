#pragma once
#include "GameObject.h"
#include <vector>

class GameLevel
{
public:
	GameLevel();
	~GameLevel();

	//���ļ����عؿ�
	void Load(std::string path, unsigned int levelWidth, unsigned int levelHeight);

	//��Ⱦ�ؿ�
	void Draw(SpriteRenderer& renderer);

	//�ؿ��Ƿ�ͨ��
	bool IsCompleted();

	std::vector<GameObject> Bricks;

private:
	//��ש�����ݳ�ʼ���ؿ�
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

