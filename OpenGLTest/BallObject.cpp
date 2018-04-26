#include "BallObject.h"



BallObject::BallObject()
	: GameObject(), Radius(12.5f), Stuck(true)
{
}


BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture sprite)
	:GameObject(pos, glm::vec2(radius*2, radius*2), sprite, glm::vec3(1.0f), velocity), Radius(radius), Stuck(true)
{

}

BallObject::~BallObject()
{
}

glm::vec2 BallObject::Move(float dt, unsigned int windowWidth)
{
	//���û�й̶�ס
	if (!this->Stuck) {
		//�ƶ���
		this->Position += this->Velocity * dt;

		//����Ƿ��ڴ��ڱ߽����⣬�ǵĻ�Ҫ��ת�ٶȲ��ָ�����ȷλ��
		if (this->Position.x <= 0.0f) {
			this->Velocity.x = -this->Velocity.x;
			this->Position.x = 0.0f;
		}
		else if (this->Position.x + this->Size.x >= windowWidth)
		{
			this->Velocity.x = -this->Velocity.x;
			this->Position.x = windowWidth - this->Size.x;
		}
		if (this->Position.y <= 0.0f) {
			this->Velocity.y = -this->Velocity.y;
			this->Position.y = 0.0f;
		}
	}

	return this->Position;
}

void BallObject::Reset(glm::vec2 position, glm::vec2 velocity)
{
	this->Position = position;
	this->Velocity = velocity;
	this->Stuck = true;
}
