#pragma once
#include "SpriteRenderer.h"
#include "GameLevel.h"
#include "BallObject.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

class Game
{
public:
	Game(unsigned int w, unsigned int h);
	~Game();

	GameState State;
	bool Keys[1024];
	unsigned int Width, Height;
	std::vector<GameLevel> Levels;
	unsigned int Level;

	void Init();

	void ProcessInput(float dt);
	void Update(float dt);
	void Render();

	void DoCollision();

private:
	bool checkCollision(GameObject& one, GameObject& two);
	bool checkCollision(BallObject& one, GameObject& two);

private:
	SpriteRenderer * m_SpriteRenderer;
	GameObject* m_Player;
	glm::vec2 m_PlayerSize;
	float m_PlayerVelocity;
	BallObject* m_Ball;
	glm::vec2 m_BallVelocity;
	float m_BallRadius;
};

