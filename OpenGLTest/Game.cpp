#include "Game.h"
#include <GLFW/glfw3.h>
#include <gtc/matrix_transform.hpp>
#include "ResourceManager.h"

Game::Game(unsigned int w, unsigned int h)
	: Width(w)
	, Height(h)
	, m_SpriteRenderer(0)
	,State(GameState::GAME_ACTIVE)
	, m_PlayerSize(100.0f, 20.0f)
	, m_PlayerVelocity(500.0f)
	, m_Player(0)
	, m_Ball(0)
	, m_BallVelocity(100.0f, -350.0f)
	, m_BallRadius(12.5f)
{
}


Game::~Game()
{
	if (m_SpriteRenderer != 0) {
		delete m_SpriteRenderer;
		m_SpriteRenderer = 0;
	}
	if (m_Player != 0) {
		delete m_Player;
		m_Player = 0;
	}
	if (m_Ball != 0) {
		delete m_Ball;
		m_Ball = 0;
	}
}

void Game::Init()
{
	ResourceManager::getInstance()->LoadShader("sprite", "shaders/breakout/sprite.vs", "shaders/breakout/sprite.fs");

	glm::mat4 projection = glm::ortho(0.0f, float(Width), float(Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::getInstance()->GetShader("sprite").use().setInt("sprite", 0);
	ResourceManager::getInstance()->GetShader("sprite").setMatrix4("projection", projection);

	//��������
	ResourceManager::getInstance()->LoadTexture("background", "resources/background.jpg");
	ResourceManager::getInstance()->LoadTexture("face", "resources/awesomeface.png");
	ResourceManager::getInstance()->LoadTexture("block", "resources/block.png");
	ResourceManager::getInstance()->LoadTexture("block_solid", "resources/block_solid.png");
	ResourceManager::getInstance()->LoadTexture("paddle", "resources/paddle.png");
	//���عؿ�
	GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height * 0.5);
	GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height * 0.5);
	GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height * 0.5);
	GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;

	m_SpriteRenderer = new SpriteRenderer(ResourceManager::getInstance()->GetShader("sprite"));
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - m_PlayerSize.x / 2, this->Height - m_PlayerSize.y);
	m_Player = new GameObject(playerPos, m_PlayerSize, ResourceManager::getInstance()->GetTexture("paddle"));
	glm::vec2 ballPos = playerPos + glm::vec2(m_PlayerSize.x / 2 - m_BallRadius, -m_BallRadius * 2);
	m_Ball = new BallObject(ballPos, m_BallRadius, m_BallVelocity, ResourceManager::getInstance()->GetTexture("face"));
}

void Game::ProcessInput(float dt)
{
	if (this->State == GameState::GAME_ACTIVE) {
		float velocity = m_PlayerVelocity * dt;
		//�ƶ�����
		if (this->Keys[GLFW_KEY_A] || this->Keys[GLFW_KEY_LEFT]) {
			if (m_Player->Position.x >= 0) {
				m_Player->Position.x -= velocity;
				if (m_Ball->Stuck) {
					m_Ball->Position.x -= velocity;
				}
			}
		}
		if (this->Keys[GLFW_KEY_D] || this->Keys[GLFW_KEY_RIGHT]) {
			if (m_Player->Position.x <= this->Width - m_PlayerSize.x) {
				m_Player->Position.x += velocity;
			}				
			if (m_Ball->Stuck) {
				m_Ball->Position.x += velocity;
			}
		}

		if (this->Keys[GLFW_KEY_SPACE]) {
			m_Ball->Stuck = false;
		}
	}

}

void Game::Update(float dt)
{
	m_Ball->Move(dt, this->Width);

	//��ײ���
	DoCollision();
}

void Game::Render()
{
	if (this->State == GameState::GAME_ACTIVE) {
		//���Ʊ���
		m_SpriteRenderer->DrawSprite(ResourceManager::getInstance()->GetTexture("background"), 
			glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);

		//���ƹؿ�
		this->Levels[this->Level].Draw(*m_SpriteRenderer);

		//���Ƶײ�����
		m_Player->Draw(*m_SpriteRenderer);
		
		//������
		m_Ball->Draw(*m_SpriteRenderer);
	}
}

void Game::DoCollision()
{
	for (GameObject & box : this->Levels[this->Level].Bricks) {
		if (!box.Destroyed) {
			if (checkCollision(*m_Ball, box)) {
				if (!box.IsSolid) {
					box.Destroyed = true;
				}
			}
		}
	}
}

bool Game::checkCollision(GameObject& one, GameObject& two)
{
	//ʹ��AABB���� �����ײ
	//x�᷽���Ƿ���ײ
	bool collisionX = (one.Position.x + one.Size.x >= two.Position.x) && 
		(two.Position.x + two.Size.x >= one.Position.x);
	//y�᷽���Ƿ���ײ
	bool collisionY = (one.Position.y + one.Size.y >= two.Position.y) &&
		(two.Position.y + two.Size.y >= one.Position.y);

	return collisionX && collisionY;
}

bool Game::checkCollision(BallObject& one, GameObject& two)
{
	//ʹ������AABB �����ײ
	
	//���Բ��
	glm::vec2 center(one.Position + one.Radius);
	//����two��AABB��Ϣ�����ģ���߳���
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x,
		two.Position.y + aabb_half_extents.y);
	//��ȡ�������ĵĲ�ʸ��
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	//AABB�Ͼ���Բ�������
	glm::vec2 closest = aabb_center + clamped;
	//��Բ�ĺ�closest�ľ���
	difference = center - closest;
	float distance = glm::length(difference);

	return distance < one.Radius;
}
