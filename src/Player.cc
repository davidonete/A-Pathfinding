#include "Player.h"

Player::Player(float x, float y, float mSpeed, const char* path)
{
  position.x = x;
  position.y = y;

  velocity.x = 0.0f;
  velocity.y = 0.0f;
  acceleration.x = 0.0f;
  acceleration.y = 0.0f;

  maxSpeed = mSpeed;

  spriteHandler = ESAT::SpriteFromFile(path);
}

void Player::Update(int dt)
{
  Move(dt);
}

void Player::MoveRight() {
  SetVelocity(fvec2(maxSpeed, velocity.y));
}

void Player::MoveLeft() {
  SetVelocity(fvec2(-maxSpeed, velocity.y));
}

void Player::MoveUp() {
  SetVelocity(fvec2(velocity.x, -maxSpeed));
}

void Player::MoveDown() {
  SetVelocity(fvec2(velocity.x, maxSpeed));
}

void Player::StopMoveRight() {
  SetVelocity(fvec2(0.0f, velocity.y));
}

void Player::StopMoveLeft() {
  SetVelocity(fvec2(0.0f, velocity.y));
}

void Player::StopMoveUp() {
  SetVelocity(fvec2(velocity.x, 0.0f));
}

void Player::StopMoveDown() {
  SetVelocity(fvec2(velocity.x, 0.0f));
}