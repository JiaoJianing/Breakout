#include "Game.h"
#include <GLFW/glfw3.h>
#include <algorithm>
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
	, m_Effects(0)
	, m_ShakeTime(0.0f)
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
	if (m_Effects != 0) {
		delete m_Effects;
		m_Effects = 0;
	}
}

void Game::Init()
{
	ResourceManager::getInstance()->LoadShader("sprite", "shaders/breakout/sprite.vs", "shaders/breakout/sprite.fs");
	ResourceManager::getInstance()->LoadShader("particle", "shaders/breakout/particle.vs", "shaders/breakout/particle.fs");
	ResourceManager::getInstance()->LoadShader("effects", "shaders/breakout/effects.vs", "shaders/breakout/effects.fs");

	glm::mat4 projection = glm::ortho(0.0f, float(Width), float(Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::getInstance()->GetShader("sprite").use().setInt("sprite", 0);
	ResourceManager::getInstance()->GetShader("sprite").setMatrix4("projection", projection);
	ResourceManager::getInstance()->GetShader("particle").use().setInt("particle", 0);
	ResourceManager::getInstance()->GetShader("particle").setMatrix4("projection", projection);

	//��������
	ResourceManager::getInstance()->LoadTexture("background", "resources/background.jpg");
	ResourceManager::getInstance()->LoadTexture("face", "resources/awesomeface.png");
	ResourceManager::getInstance()->LoadTexture("block", "resources/block.png");
	ResourceManager::getInstance()->LoadTexture("block_solid", "resources/block_solid.png");
	ResourceManager::getInstance()->LoadTexture("paddle", "resources/paddle.png");
	ResourceManager::getInstance()->LoadTexture("particle", "resources/particle.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_Speed, "resources/powerup_speed.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_Sticky, "resources/powerup_sticky.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_PassThrough, "resources/powerup_passthrough.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_PadIncrease, "resources/powerup_increase.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_Confuse, "resources/powerup_confuse.png");
	ResourceManager::getInstance()->LoadTexture(Powerup_Chaos, "resources/powerup_chaos.png");
	
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
	
	m_Particles = new ParticleGenerator(ResourceManager::getInstance()->GetShader("particle"),
		ResourceManager::getInstance()->GetTexture("particle"), 500);

	m_Effects = new PostProcessor(ResourceManager::getInstance()->GetShader("effects"), this->Width, this->Height);
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
			m_Ball->Stick = false;
			m_Player->Color = glm::vec3(1.0f);
		}
	}

}

void Game::Update(float dt)
{
	m_Ball->Move(dt, this->Width);

	//��ײ���
	DoCollision();

	//��������
	m_Particles->Update(dt, *m_Ball, 2, glm::vec2(m_Ball->Radius / 2));

	this->UpdatePowerUps(dt);

	if (m_ShakeTime > 0.0f) {
		m_ShakeTime -= dt;
		if (m_ShakeTime <= 0.0f) {
			m_Effects->Shake = false;
		}
	}

	//����µײ���������Ϸ
	if (m_Ball->Position.y >= this->Height) {
		this->ResetLevel();
		this->ResetPlayer();
	}
}

void Game::Render()
{
	if (this->State == GameState::GAME_ACTIVE) {
		
		m_Effects->BeginRender();

		//���Ʊ���
		m_SpriteRenderer->DrawSprite(ResourceManager::getInstance()->GetTexture("background"), 
			glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		//���ƹؿ�
		this->Levels[this->Level].Draw(*m_SpriteRenderer);
		//���Ƶײ�����
		m_Player->Draw(*m_SpriteRenderer);
		//��������
		m_Particles->Draw();
		//������
		m_Ball->Draw(*m_SpriteRenderer);
		//���Ƶ���
		for (PowerUp& powerup : this->PowerUps) {
			if (!powerup.Destroyed) {
				powerup.Draw(*m_SpriteRenderer);
			}
		}

		m_Effects->EndRender();

		m_Effects->Render(glfwGetTime());
	}
}

void Game::DoCollision()
{
	//���ש�����ײ
	for (GameObject & box : this->Levels[this->Level].Bricks) {
		if (!box.Destroyed) {
			Collision collsion = checkCollision(*m_Ball, box);
			if (std::get<0>(collsion)) {//��⵽��ײ
				//����ʵ�� ����ש��
				if (!box.IsSolid) {
					box.Destroyed = true;
					//������ɵ���
					this->SpawnPowerUps(box);
				}
				else {//��ײʵ��ש�飬����shakeЧ��
					m_ShakeTime = 0.05f;
					m_Effects->Shake = true;
				}

				//��ײ����
				Direction dir = std::get<1>(collsion);
				glm::vec2 diff_vector = std::get<2>(collsion);
				//��С��Ϊpass-through״̬ʱ���Դ���ʵ��ש��
				if (!(m_Ball->PassThrough && !box.IsSolid)) {

					if (dir == Direction::LEFT || dir == Direction::RIGHT) {
						m_Ball->Velocity.x = -m_Ball->Velocity.x;
						//�ض�λ
						float penetration = m_Ball->Radius - std::abs(diff_vector.x);
						if (dir == Direction::LEFT) {
							m_Ball->Position.x += penetration; //�����ƶ�
						}
						else {
							m_Ball->Position.x -= penetration; //�����ƶ�
						}
					}
					else {
						m_Ball->Velocity.y = -m_Ball->Velocity.y;
						//�ض�λ
						float penetration = m_Ball->Radius - std::abs(diff_vector.y);
						if (dir == Direction::UP) {
							m_Ball->Position.y -= penetration; //�����ƶ�
						}
						else {
							m_Ball->Position.y += penetration; //�����ƶ�
						}
					}
				}
			}
		}
	}

	//��͵ײ��������ײ
	Collision result = checkCollision(*m_Ball, *m_Player);
	if (!m_Ball->Stuck && std::get<0>(result)) {
		//�������������λ���ж�����ٶȸı�
		float centerBoard = m_Player->Position.x + m_PlayerSize.x / 2;
		float distance = (m_Ball->Position.x + m_Ball->Radius) - centerBoard;
		float percentage = distance / (m_PlayerSize.x / 2);

		//���ݽ���ƶ�
		float strength = 2.0f;
		glm::vec2 oldVelocity = m_Ball->Velocity;
		m_Ball->Velocity.x = m_BallVelocity.x * percentage * strength;
		//m_Ball->Velocity.y = -m_Ball->Velocity.y;
		m_Ball->Velocity = glm::normalize(m_Ball->Velocity) * glm::length(oldVelocity);
		m_Ball->Velocity.y = -1 * std::abs(m_Ball->Velocity.y);

		//����С��sticky״̬
		m_Ball->Stuck = m_Ball->Stick;
	}

	//�����ߺ͵ײ��򵲰����ײ
	for (PowerUp& powerUp : this->PowerUps) {
		if (!powerUp.Destroyed) {
			if (powerUp.Position.y >= this->Height) {
				powerUp.Destroyed = true;
			}
			if (checkCollision(*m_Player, powerUp)) {
				//���ߺ͵�����ײ���򼤻�
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
			}
		}
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

	//��յ���
	this->PowerUps.clear();
	m_Effects->Chaos = false;
	m_Effects->Shake = false;
	m_Effects->Confuse = false;
}

void Game::ResetPlayer()
{
	m_Player->Size = m_PlayerSize;
	m_Player->Position = glm::vec2(this->Width / 2 - m_PlayerSize.x / 2, this->Height - m_PlayerSize.y);
	m_Ball->Reset(m_Player->Position + glm::vec2(m_PlayerSize.x / 2 - m_BallRadius, -m_BallRadius * 2), m_BallVelocity);
}

void Game::SpawnPowerUps(GameObject& block)
{
	if (shouldSpawn(50)){// 1/50�ĸ���
		this->PowerUps.push_back(PowerUp(Powerup_Speed, glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_Speed)));
	}
	if (shouldSpawn(50)) {
		this->PowerUps.push_back(PowerUp(Powerup_Sticky, glm::vec3(1.0f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_Sticky)));
	}
	if (shouldSpawn(50)) {
		this->PowerUps.push_back(PowerUp(Powerup_PassThrough, glm::vec3(0.5f, 1.0f, 0.5f), 5.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_PassThrough)));
	}
	if (shouldSpawn(50)) {
		this->PowerUps.push_back(PowerUp(Powerup_PadIncrease, glm::vec3(1.0f, 0.6f, 0.4f), 5.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_PadIncrease)));
	}

	//������߱�Ƶ�����ɣ�������Ϸ�Ѷ�
	if (shouldSpawn(20)) {
		this->PowerUps.push_back(PowerUp(Powerup_Confuse, glm::vec3(1.0f, 0.3f, 0.3f), 5.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_Confuse)));
	}
	if (shouldSpawn(20)) {
		this->PowerUps.push_back(PowerUp(Powerup_Chaos, glm::vec3(0.9f, 0.25f, 0.25f), 5.0f, block.Position, ResourceManager::getInstance()->GetTexture(Powerup_Chaos)));
	}
}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerup : this->PowerUps) {
		powerup.Position += powerup.Velocity * dt;
		if (powerup.Activated) {
			powerup.Duration -= dt;
			//Ч��ʧЧ
			if (powerup.Duration <= 0.0f) {
				powerup.Activated = false;
				if (powerup.Type == Powerup_PassThrough) {
					if (!isOtherPowerUpActive(Powerup_PassThrough)) {
						m_Ball->PassThrough = false;
						m_Ball->Color = glm::vec3(1.0f);
					}
				}
				else if (powerup.Type == Powerup_PadIncrease) {
					if (!isOtherPowerUpActive(Powerup_PadIncrease)) {
						m_Player->Size = m_PlayerSize;
					}
				}
				else if (powerup.Type == Powerup_Confuse) {
					if (!isOtherPowerUpActive(Powerup_Confuse)) {
						m_Effects->Confuse = false;
					}
				}
				else if (powerup.Type == Powerup_Chaos) {
					if (!isOtherPowerUpActive(Powerup_Chaos)) {
						m_Effects->Chaos = false;
					}
				}
			}
		}
	}

	//�Ƴ�ʧЧ�ĵ��� lamda���ʽ
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(), 
		[](const PowerUp& powerup) {return powerup.Destroyed && !powerup.Activated; }), this->PowerUps.end());
}

void Game::ActivatePowerUp(PowerUp& powerup)
{
	if (powerup.Type == Powerup_Speed) {
		m_Ball->Velocity *= 1.2;
	}
	else if (powerup.Type == Powerup_Sticky) {
		m_Ball->Stick = true;
		m_Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerup.Type == Powerup_PassThrough) {
		m_Ball->PassThrough = true;
		m_Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerup.Type == Powerup_PadIncrease) {
		m_Player->Size.x = m_PlayerSize.x + 50;
	}
	else if (powerup.Type == Powerup_Confuse) {
		if (!m_Effects->Chaos) {
			m_Effects->Confuse = true; //chaos��confuse����ͬʱ����
		}
	}
	else if (powerup.Type == Powerup_Chaos) {
		if (!m_Effects->Confuse) {
			m_Effects->Chaos = true;
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

Collision Game::checkCollision(BallObject& one, GameObject& two)
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
	difference = closest - center;

	if (glm::length(difference) < one.Radius) {
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

bool Game::shouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}

bool Game::isOtherPowerUpActive(std::string type)
{
	for (const PowerUp& powerup : this->PowerUps) {
		if (powerup.Activated) {
			if (powerup.Type == type) {
				return true;
			}
		}
	}

	return false;
}
