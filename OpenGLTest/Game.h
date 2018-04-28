#pragma once
#include "SpriteRenderer.h"
#include "GameLevel.h"
#include "BallObject.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "PowerUp.h"
#include "Text.h"
#include <irrKlang.h>
#include <tuple>

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

#define Powerup_Speed "speed"
#define Powerup_Sticky "sticky"
#define Powerup_PassThrough "pass-through"
#define Powerup_PadIncrease "pad-increase"
#define Powerup_Confuse "confuse"
#define Powerup_Chaos "chaos"

typedef std::tuple<bool, Direction, glm::vec2> Collision;

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

	std::vector<PowerUp> PowerUps;

	void Init();

	void ProcessInput(float dt);
	void Update(float dt);
	void Render();

	void DoCollision();

	void ResetLevel();
	void ResetPlayer();

	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
	void ActivatePowerUp(PowerUp& powerup);

private:
	bool checkCollision(GameObject& one, GameObject& two);
	Collision checkCollision(BallObject& one, GameObject& two);
	Direction vectorDirection(glm::vec2 target);
	bool shouldSpawn(unsigned int chance);
	bool isOtherPowerUpActive(std::string type);

private:
	SpriteRenderer * m_SpriteRenderer;
	GameObject* m_Player;
	glm::vec2 m_PlayerSize;
	float m_PlayerVelocity;
	BallObject* m_Ball;
	glm::vec2 m_BallVelocity;
	float m_BallRadius;
	ParticleGenerator* m_Particles;
	PostProcessor* m_Effects;

	irrklang::ISoundEngine* m_SoundEngine;

	Text* m_Text;

	float m_ShakeTime;
};

