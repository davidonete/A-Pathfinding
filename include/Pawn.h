#ifndef PAWN_H_
#define PAWN_H_

#include <ESAT/sprite.h>

struct fvec2
{
  float x, y;
  fvec2(){
    x = 0.0f;
    y = 0.0f;
  }

  fvec2(float _x, float _y)
  {
    x = _x;
    y = _y;
  }
};

struct ivec2
{
  int x, y;
  ivec2(){
    x = 0;
    y = 0;
  }

  ivec2(int _x, int _y)
  {
    x = _x;
    y = _y;
  }
};

struct math
{
  static fvec2 VectorDirection(fvec2 p1, fvec2 p2);
  static fvec2 NormalizeVector(fvec2 v);
  static float Distance(fvec2 p1, fvec2 p2);
};

class Pawn
{
protected:
  void Move(int dt);

  fvec2 velocity;
  fvec2 acceleration;
  fvec2 position;

  float maxSpeed;
  ESAT::SpriteHandle spriteHandler;

public:
  Pawn(){};
  virtual ~Pawn(){};

  virtual void Draw();

  void SetPosition(fvec2 p);
  fvec2 GetPosition();

  void SetVelocity(fvec2 v);
  fvec2 GetVelocity();

  void SetAcceleration(fvec2 a);
  fvec2 GetAcceleration();

  inline void SetMaxSpeed(float speed) { maxSpeed = speed; }
  inline float GetMaxSpeed() { return maxSpeed; }
};

#endif