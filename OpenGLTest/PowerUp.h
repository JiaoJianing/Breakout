#pragma once
#include "GameObject.h"

class PowerUp : public GameObject
{
public:
	PowerUp(std::string type, glm::vec3 color, float duration, glm::vec2 position, Texture texture);
	~PowerUp();

	//��������
	std::string Type;
	float Duration;
	bool Activated;
};

