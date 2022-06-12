#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include <glm/glm.hpp>
#include "game_level.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "textRenderer.h"
#include <iostream>
#include <algorithm>
#include <irrKlang/irrKlang.h>
#include <sstream>

using namespace irrklang;

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
ParticleGenerator *Particles;
PostProcessor *Effects;
ISoundEngine *SoundEngine = createIrrKlangDevice();
ISound* backgroundMusic;
ISound* backgroundMusicRev;
ISoundEffectControl *bkgMusicFXControl;
TextRenderer *Text;

float ShakeTime = 0.0f;


Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    std::vector<GameLevel> Levels;
    unsigned int           Level;
    Lives = 3;
}

Game::~Game() {
    delete Renderer;
};


void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("fragments/sprite.vert", "fragments/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("fragments/particles.vert", "fragments/particles.frag", nullptr, "particle");
    ResourceManager::LoadShader("fragments/post_processing.vert", "fragments/post_processing.frag", nullptr, "effects");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
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
    ResourceManager::LoadTexture("textures/particle.png", true, "particle");
    ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "powerup_passthrough");
    ResourceManager::LoadTexture("textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("textures/powerup_dec_speed.png", true, "powerup_dec_speed");
    ResourceManager::LoadTexture("textures/powerup_slowmo.png", true, "powerup_slowmo");
    ResourceManager::LoadTexture("textures/powerup_death.png", true, "powerup_death");
    ResourceManager::LoadTexture("textures/powerup_ghost.png", true, "powerup_ghost");
    Particles = new ParticleGenerator(
            ResourceManager::GetShader("particle"),
            ResourceManager::GetTexture("particle"),
            500
    );
    Effects = new PostProcessor(ResourceManager::GetShader("effects"), this->Width, this->Height);
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
    backgroundMusic = SoundEngine->play2D("audio/breakout.mp3", true, false, true, ESM_AUTO_DETECT, true);
    backgroundMusicRev = SoundEngine->play2D("audio/breakout-reverse.mp3", true, true, true, ESM_AUTO_DETECT, false);
    bkgMusicFXControl = backgroundMusic->getSoundEffectControl();
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/ocraext.ttf", 24);
    this->State = GAME_MENU;
};

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_WIN)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;
            this->State = GAME_MENU;
        }
    }
    if (this->State == GAME_ACTIVE){
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

    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
        {
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_W] = true;
        }
        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
        {
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = 3;
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
    }
}

void Game::Update(float dt) {
    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = true;
        this->State = GAME_WIN;
    }
    if (this->State == GAME_ACTIVE || this->State == GAME_MENU){
        Ball->Move(dt, this->Width);
        this->DoCollisions();
        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
        if (Ball->Position.y >= this->Height)
        {
            --this->Lives;
            if (this->Lives == 0)
            {
                this->ResetLevel();
                this->State = GAME_MENU;
            }
            this->ResetPlayer();
        }
        UpdatePowerUps(dt);
        if (ShakeTime > 0.0f)
        {
            ShakeTime -= dt;
            if (ShakeTime <= 0.0f)
                Effects->Shake = false;
        }
    }

    if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
    }
};

void Game::Render() {
        Effects->BeginRender();
        // draw background
        Texture2D sprite = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(sprite,
            glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
        );
        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Particles->Draw();
        Ball->Draw(*Renderer);
        for (PowerUp &powerUp : this->PowerUps)
            if (!powerUp.Destroyed)
                powerUp.Draw(*Renderer);
        Effects->EndRender();
        Effects->Render(glfwGetTime());
        std::stringstream ss; ss << this->Lives;
        Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);


        if (this->State == GAME_WIN)
        {
            Text->RenderText(
                    "You WON!!!", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0)
            );
            Text->RenderText(
                    "Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0)
            );
        }

        if (this->State == GAME_MENU)
        {
            Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
            Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
        }

};


void Game::ResetLevel() {
    this->Lives = 3;
    switch (this->Level)
    {
    case 0:
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
        break;
    case 1:
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    case 2:
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
        break;
    case 3:
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);
    }
}

void Game::ResetPlayer() {
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two) // AABB - AABB collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x,
        two.Position.y + aabb_half_extents.y
    );
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}
void Game::SpawnPowerUps(GameObject &block)
{
    if (ShouldSpawn(75)) // 1 in 75 chance
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("dec_speed", glm::vec3(1.0f, 0.5f, 0.8), 0.0f, block.Position, ResourceManager::GetTexture("powerup_dec_speed")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("slowmo", glm::vec3(0.0f, 0.6f, 1.0), 20.0f, block.Position, ResourceManager::GetTexture("powerup_slowmo")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("ghost", glm::vec3(0.5f, 0.5f, 0.5), 20.0f, block.Position, ResourceManager::GetTexture("powerup_ghost")));
    else if (ShouldSpawn(25)) // Negative powerups should spawn more often
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
    else if (ShouldSpawn(25))
        this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
    else if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("death", glm::vec3(1.0f, 0.1f, 0.1f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_death")));
}

bool IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
    for (const PowerUp& powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}

void Game::ActivatePowerUp(PowerUp &powerUp)
{
    if (powerUp.Type == "speed")
    {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!Effects->Chaos) {
            Effects->Confuse = true; // only activate if chaos wasn't already active
            backgroundMusic->setIsPaused(true);
            backgroundMusicRev->setPlayPosition(backgroundMusic->getPlayLength() - backgroundMusic->getPlayPosition());
            backgroundMusicRev->setIsPaused(false);
        }
    }
    else if (powerUp.Type == "chaos")
    {
        if (!Effects->Confuse) {
            Effects->Chaos = true;
            if (bkgMusicFXControl)
                bkgMusicFXControl->enableDistortionSoundEffect();
            else
                std::cout << "This device or sound does not support sound effects.\n";
        }
    }
    else if (powerUp.Type == "dec_speed")
    {
        Ball->Velocity *= 0.8;
    }
    else if (powerUp.Type == "ghost")
    {
        Ball->Ghost = true;
    }
    else if (powerUp.Type == "slowmo")
    {
        if (!IsOtherPowerUpActive(this->PowerUps, "slowmo")) {
            Ball->oldVelocity = Ball->Velocity;
            Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * 0.3f;
            if (Ball->oldVelocity.y / abs(Ball->oldVelocity.y) != (Ball->Velocity.y / abs(Ball->Velocity.y)))
                Ball->Velocity.y *= -1;
            Ball->Color = glm::vec3(0.0f, 0.6f, 1.0f);
            backgroundMusic->setPlaybackSpeed(0.5f);
        }
    }
    else if (powerUp.Type == "death")
    {
        this->ResetLevel();
        this->ResetPlayer();
    }
}




void Game::UpdatePowerUps(float dt)
{
    for (PowerUp &powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated)
        {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f)
            {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // deactivate effects
                if (powerUp.Type == "sticky")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "ghost")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "ghost"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->Ghost = false;
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                        backgroundMusicRev->setIsPaused(true);
                        backgroundMusic->setPlayPosition(backgroundMusicRev->getPlayLength() - backgroundMusicRev->getPlayPosition());
                        backgroundMusic->setIsPaused(false);
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                        if (bkgMusicFXControl)
                            bkgMusicFXControl->disableDistortionSoundEffect();
                        else
                            std::cout << "This device or sound does not support sound effects.\n";
                    }
                }
                else if (powerUp.Type == "slowmo")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "slowmo"))
                    {	// only reset if no other PowerUp of type chaos is active
                        bool isNegative = false;
                        if (Ball->oldVelocity.y / abs(Ball->oldVelocity.y) != (Ball->Velocity.y / abs(Ball->Velocity.y)))
                            isNegative = true;
                        Ball->Velocity.y = Ball->oldVelocity.y;
                        Ball->Color = glm::vec3(1.0f);
                        if (isNegative)
                            Ball->Velocity.y *= -1;
                        backgroundMusic->setPlaybackSpeed(1.0f);
                    }
                }
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
                                        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}


void Game::DoCollisions() {
    for (GameObject& box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid) {
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                    SoundEngine->play2D("audio/bleep.mp3");
                }
                else if(!Ball->Ghost) {
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                    SoundEngine->play2D("audio/solid.wav");
                }
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (!(Ball->PassThrough && !box.IsSolid) && !(Ball->Ghost && box.IsSolid )){
                    if (dir == LEFT || dir == RIGHT) // horizontal collision
                    {
                        Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // move ball to right
                        else
                            Ball->Position.x -= penetration; // move ball to left;
                    }
                    else // vertical collision
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball back up
                        else
                            Ball->Position.y += penetration; // move ball back down
                    }
                }
            }
        }
    }
    for (PowerUp &powerUp : this->PowerUps)
    {
        if (!powerUp.Destroyed)
        {
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;
            if (CheckCollision(*Player, powerUp))
            {	// collided with player, now activate powerup
                ActivatePowerUp(powerUp);
                SoundEngine->play2D("audio/powerup.wav");
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
    }
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);

        // if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
        Ball->Stuck = Ball->Sticky;
        SoundEngine->play2D("audio/bleep.wav");
    }
}




Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}