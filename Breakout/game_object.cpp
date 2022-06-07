#include "game_object.h"

GameObject::GameObject() : Position(0.0, 0.0), Size(1.0, 1.0), Velocity(0.0), Color(1.0), Rotation(0.0), IsSolid(true), Destroyed(false), Sprite() {}

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color, glm::vec2 velocity) :
	Position(pos), Size(size), Sprite(sprite), Color(color), Velocity(velocity), IsSolid(true), Rotation(0.0), Destroyed(false) {}

void GameObject::Draw(SpriteRenderer &renderer) {
	renderer.DrawSprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
}