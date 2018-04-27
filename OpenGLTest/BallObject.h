#pragma once
#include "GameObject.h"

class BallObject : public GameObject
{
public:
	BallObject();
	BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture sprite);
	~BallObject();

	glm::vec2 Move(float dt, unsigned int windowWidth);
	void Reset(glm::vec2 position, glm::vec2 velocity);

	float Radius;
	bool Stuck;
	bool Stick, PassThrough;
};

