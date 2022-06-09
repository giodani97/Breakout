#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tuple>
#include "game_level.h"
#include "powerup.h"

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

typedef std::tuple<bool, Direction, glm::vec2> Collision;

Direction VectorDirection(glm::vec2 target);

class Game {
public:
	GameState State;
	bool Keys[1024];
	unsigned int Width, Height;
    std::vector<PowerUp>  PowerUps;
	std::vector<GameLevel>  Levels;
	unsigned int Level;
	Game(unsigned int width, unsigned int height);
	~Game();
	void Init();
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();
	void DoCollisions();
	void ResetLevel();
	void ResetPlayer();
    void SpawnPowerUps(GameObject &block);
    void UpdatePowerUps(float dt);
};

#endif