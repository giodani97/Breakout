//
// Created by giovanni on 08/06/22.
//

#ifndef BREAKOUT_PARTICLE_GENERATOR_H
#define BREAKOUT_PARTICLE_GENERATOR_H

#include <glm/glm.hpp>
#include <vector>
#include "game_object.h"
#include "texture.h"

struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

class ParticleGenerator{
public:
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);
    void Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
    void Draw();
private:
    std::vector<Particle> particles;
    unsigned int amount;
    Shader shader;
    Texture2D texture;
    unsigned int VAO;
    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};

#endif //BREAKOUT_PARTICLE_GENERATOR_H
