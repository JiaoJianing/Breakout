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
	, m_Particles(0)
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
	if (m_Particles != 0) {
		delete m_Particles;
		m_Particles = 0;
	}
}

void Game::Init()
{
	ResourceManager::getInstance()->LoadShader("sprite", "shaders/breakout/sprite.vs", "shaders/breakout/sprite.fs");
	ResourceManager::getInstance()->LoadShader("particle", "shaders/breakout/particle.vs", "shaders/breakout/particle.fs");

	glm::mat4 projection = glm::ortho(0.0f, float(Width), float(Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::getInstance()->GetShader("sprite").use().setInt("sprite", 0);
	ResourceManager::getInstance()->GetShader("sprite").setMatrix4("projection", projection);
	ResourceManager::getInstance()->GetShader("particle").use().setInt("particle", 0);
	ResourceManager::getInstance()->GetShader("particle").setMatrix4("projection", projection);

	//加载纹理
	ResourceManager::getInstance()->LoadTexture("background", "resources/background.jpg");
	ResourceManager::getInstance()->LoadTexture("face", "resources/awesomeface.png");
	ResourceManager::getInstance()->LoadTexture("block", "resources/block.png");
	ResourceManager::getInstance()->LoadTexture("block_solid", "resources/block_solid.png");
	ResourceManager::getInstance()->LoadTexture("paddle", "resources/paddle.png");
	ResourceManager::getInstance()->LoadTexture("particle", "resources/particle.png");
	
	//加载关卡
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
	
	m_Particles = new ParticleGenerator(ResourceManager::getInstance()->GetShader("particle"),
		ResourceManager::getInstance()->GetTexture("particle"), 500);
}

void Game::ProcessInput(float dt)
{
	if (this->State == GameState::GAME_ACTIVE) {
		float velocity = m_PlayerVelocity * dt;
		//移动挡板
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

	//碰撞检测
	DoCollision();

	//球掉下底部则重置游戏
	if (m_Ball->Position.y >= this->Height) {
		this->ResetLevel();
		this->ResetPlayer();
	}

	//更新粒子
	m_Particles->Update(dt, *m_Ball, 2, glm::vec2(m_Ball->Radius / 2));
}

void Game::Render()
{
	if (this->State == GameState::GAME_ACTIVE) {
		//绘制背景
		m_SpriteRenderer->DrawSprite(ResourceManager::getInstance()->GetTexture("background"), 
			glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);

		//绘制关卡
		this->Levels[this->Level].Draw(*m_SpriteRenderer);

		//绘制底部挡板
		m_Player->Draw(*m_SpriteRenderer);

		//绘制粒子
		m_Particles->Draw();
		
		//绘制球
		m_Ball->Draw(*m_SpriteRenderer);
	}
}

void Game::DoCollision()
{
	//球和砖块的碰撞
	for (GameObject & box : this->Levels[this->Level].Bricks) {
		if (!box.Destroyed) {
			Collision collsion = checkCollision(*m_Ball, box);
			if (std::get<0>(collsion)) {//检测到碰撞
				//不是实心 销毁砖块
				if (!box.IsSolid) {
					box.Destroyed = true;
				}

				//碰撞处理
				Direction dir = std::get<1>(collsion);
				glm::vec2 diff_vector = std::get<2>(collsion);
				if (dir == Direction::LEFT || dir == Direction::RIGHT) {
					m_Ball->Velocity.x = -m_Ball->Velocity.x;
					//重定位
					float penetration = m_Ball->Radius - std::abs(diff_vector.x);
					if (dir == Direction::LEFT) {
						m_Ball->Position.x += penetration; //向右移动
					}
					else {
						m_Ball->Position.x -= penetration; //向左移动
					}
				}
				else {
					m_Ball->Velocity.y = -m_Ball->Velocity.y;
					//重定位
					float penetration = m_Ball->Radius - std::abs(diff_vector.y);
					if (dir == Direction::UP) {
						m_Ball->Position.y -= penetration; //向上移动
					}
					else {
						m_Ball->Position.y += penetration; //向下移动
					}
				}
			}
		}
	}

	//球和底部挡板的碰撞
	Collision result = checkCollision(*m_Ball, *m_Player);
	if (!m_Ball->Stuck && std::get<0>(result)) {
		//根据碰到挡板的位置判断球的速度改变
		float centerBoard = m_Player->Position.x + m_PlayerSize.x / 2;
		float distance = (m_Ball->Position.x + m_Ball->Radius) - centerBoard;
		float percentage = distance / (m_PlayerSize.x / 2);

		//根据结果移动
		float strength = 2.0f;
		glm::vec2 oldVelocity = m_Ball->Velocity;
		m_Ball->Velocity.x = m_BallVelocity.x * percentage * strength;
		//m_Ball->Velocity.y = -m_Ball->Velocity.y;
		m_Ball->Velocity.y = -1 * std::abs(m_Ball->Velocity.y);
		m_Ball->Velocity = glm::normalize(m_Ball->Velocity) * glm::length(oldVelocity);
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0) {
		this->Levels[0].Load("levels/one.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->Level == 1) {
		this->Levels[1].Load("levels/two.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->Level == 2) {
		this->Levels[2].Load("levels/three.lvl", this->Width, this->Height * 0.5f);
	}
	else if (this->Level == 3) {
		this->Levels[3].Load("levels/four.lvl", this->Width, this->Height * 0.5f);
	}
}

void Game::ResetPlayer()
{
	m_Player->Size = m_PlayerSize;
	m_Player->Position = glm::vec2(this->Width / 2 - m_PlayerSize.x / 2, this->Height - m_PlayerSize.y);
	m_Ball->Reset(m_Player->Position + glm::vec2(m_PlayerSize.x / 2 - m_BallRadius, -m_BallRadius * 2), m_BallVelocity);
}

bool Game::checkCollision(GameObject& one, GameObject& two)
{
	//使用AABB方形 检测碰撞
	//x轴方向是否碰撞
	bool collisionX = (one.Position.x + one.Size.x >= two.Position.x) && 
		(two.Position.x + two.Size.x >= one.Position.x);
	//y轴方向是否碰撞
	bool collisionY = (one.Position.y + one.Size.y >= two.Position.y) &&
		(two.Position.y + two.Size.y >= one.Position.y);

	return collisionX && collisionY;
}

Collision Game::checkCollision(BallObject& one, GameObject& two)
{
	//使用球形AABB 检测碰撞
	
	//获得圆心
	glm::vec2 center(one.Position + one.Radius);
	//计算two的AABB信息（中心，半边长）
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x,
		two.Position.y + aabb_half_extents.y);
	//获取两个中心的差矢量
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	//AABB上距离圆心最近点
	glm::vec2 closest = aabb_center + clamped;
	//求圆心和closest的距离
	difference = center - closest;

	if (glm::length(difference) <= one.Radius) {
		return std::make_tuple(true, vectorDirection(difference), difference);
	}
	else {
		return std::make_tuple(false, Direction::UP, glm::vec2(0.0f, 0.0f));
	}
}

Direction Game::vectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-1.0f, 0.0f),
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++) {
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max) {
			max = dot_product;
			best_match = i;
		}
	}

	return (Direction)best_match;
}
