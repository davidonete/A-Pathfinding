#ifndef PLAYER_H_
#define PLAYER_H_

#include "Pawn.h"

class Player : public Pawn
{
private:
  float maxAcceleration = 20.0f;

public:
  Player(){}
  Player(float x, float y, float mSpeed, const char* path);
  virtual ~Player(){}

  void Update(int dt);

  void MoveRight();
  void MoveLeft();
  void MoveUp();
  void MoveDown();

  void StopMoveLeft();
  void StopMoveRight();
  void StopMoveUp();
  void StopMoveDown();
};

#endif