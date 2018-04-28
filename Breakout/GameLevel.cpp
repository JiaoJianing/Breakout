#include "GameLevel.h"
#include <fstream>
#include <sstream>
#include "ResourceManager.h"

GameLevel::GameLevel()
{
}


GameLevel::~GameLevel()
{
}

void GameLevel::Load(std::string path, unsigned int levelWidth, unsigned int levelHeight)
{
	//��չ�������
	this->Bricks.clear();

	//���ļ�����
	unsigned int tileCode;
	std::string line;
	std::ifstream fstream(path);
	std::vector<std::vector<unsigned int>> tileData;
	if (fstream) {
		while (std::getline(fstream, line)) {//��ȡһ��
			std::istringstream sstream(line);
			std::vector<unsigned int> row;
			while (sstream >> tileCode) {//��ȡÿ�б��ո�ָ�������
				row.push_back(tileCode);
			}
			tileData.push_back(row);
		}
		if (tileData.size() > 0) {
			this->init(tileData, levelWidth, levelHeight);
		}
	}
}

void GameLevel::Draw(SpriteRenderer& renderer)
{
	for (GameObject & tile : this->Bricks) {
		if (!tile.Destroyed) {
			tile.Draw(renderer);
		}
	}
}

bool GameLevel::IsCompleted()
{
	for (GameObject& tile : this->Bricks) {
		if (!tile.IsSolid && !tile.Destroyed) {
			return false;
		}
	}

	return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
	unsigned int height = tileData.size();
	unsigned int width = tileData[0].size();
	float unit_width = levelWidth / width;
	float unit_height = levelHeight / height;

	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			//���ש������
			if (tileData[y][x] == 1) {
				glm::vec2 pos(unit_width * x, unit_height * y);
				glm::vec2 size(unit_width, unit_height);
				GameObject obj(pos, size, ResourceManager::getInstance()->GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
				obj.IsSolid = true;
				this->Bricks.push_back(obj);
			}
			else if (tileData[y][x] > 1) {
				glm::vec3 color = glm::vec3(1.0f);//Ĭ�ϰ�ɫ
				if (tileData[y][x] == 2) {
					color = glm::vec3(0.2f, 0.6f, 1.0f);
				}
				else if (tileData[y][x] == 3) {
					color = glm::vec3(0.0f, 0.7f, 0.0f);
				}
				else if (tileData[y][x] == 4) {
					color = glm::vec3(0.8f, 0.8f, 0.4f);
				}
				else if (tileData[y][x] == 5) {
					color = glm::vec3(1.0f, 0.5f, 0.0f);
				}

				glm::vec2 pos(unit_width * x, unit_height * y);
				glm::vec2 size(unit_width, unit_height);
				this->Bricks.push_back(GameObject(pos, size, ResourceManager::getInstance()->GetTexture("block"), color));
			}
		}
	}
}
