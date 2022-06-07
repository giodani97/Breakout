#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include <glm/glm.hpp>
#include "game_level.h"
#include "ball_object.h"

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

GameObject* Player;
SpriteRenderer *Renderer;
BallObject* Ball;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    std::vector<GameLevel> Levels;
    unsigned int           Level;
}

Game::~Game() {
    delete Renderer;
};


void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("fragments/sprite.vert", "fragments/sprite.frag", nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Shader spriteShader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(spriteShader);
    // load textures
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");// load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    // load levels
    GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
    glm::vec2 playerPos = glm::vec2(
        this->Width / 2.0f - PLAYER_SIZE.x / 2.0f,
        this->Height - PLAYER_SIZE.y
    );
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
        -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
        ResourceManager::GetTexture("face"));
};

void Game::ProcessInput(float dt)
{
    float velocity = PLAYER_VELOCITY * dt;
    // move playerboard
    if (this->Keys[GLFW_KEY_A] or this->Keys[GLFW_KEY_LEFT])
    {
        if (Player->Position.x >= 0.0f)
        {
            Player->Position.x -= velocity;
            if (Ball->Stuck)
                Ball->Position.x -= velocity;
        }
    }
    if (this->Keys[GLFW_KEY_D] or this->Keys[GLFW_KEY_RIGHT])
    {
        if (Player->Position.x <= this->Width - Player->Size.x)
        {
            Player->Position.x += velocity;
            if (Ball->Stuck)
                Ball->Position.x += velocity;
        }
    }
    if (this->Keys[GLFW_KEY_SPACE])
        Ball->Stuck = false;
}

void Game::Update(float dt) {
    Ball->Move(dt, this->Width);
};

void Game::Render() {
    if (this->State == GAME_ACTIVE)
    {
        // draw background
        Texture2D sprite = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(sprite,
            glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
        );
        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Ball->Draw(*Renderer);
    }
};