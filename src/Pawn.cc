#include "Pawn.h"

#include <cmath>

fvec2 math::VectorDirection(fvec2 p1, fvec2 p2)
{
  fvec2 result;
  result.x = p2.x - p1.x;
  result.y = p2.y - p1.y;

  return result;
}

fvec2 math::NormalizeVector(fvec2 v)
{
  fvec2 normalized;
  float module = sqrt(pow(v.x, 2) + pow(v.y, 2));

  normalized.x = v.x / module;
  normalized.y = v.y / module;

  return normalized;
}

float math::Distance(fvec2 p1, fvec2 p2)
{
  fvec2 vector = math::VectorDirection(p1, p2);

  //Manhathan
  fvec2 abs(abs(vector.x), abs(vector.y));

  if (abs.x > abs.y)
    return 14.0f * abs.y + 10.0f * (abs.x - abs.y);

  return (14.0f * abs.x + 10.0f * (abs.y - abs.x));

  //Euclidean
  //float module = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
  //return module;
}

void Pawn::SetPosition(fvec2 p)
{
  position.x = p.x;
  position.y = p.y;
}

fvec2 Pawn::GetPosition()
{
  return position;
}

void Pawn::SetVelocity(fvec2 v)
{
  velocity.x = v.x;
  velocity.y = v.y;
}

fvec2 Pawn::GetVelocity()
{
  return velocity;
}

void Pawn::SetAcceleration(fvec2 a)
{
  acceleration.x = a.x;
  acceleration.y = a.y;
}

fvec2 Pawn::GetAcceleration()
{
  return acceleration;
}

void Pawn::Move(int dt)
{
  float time = dt * 0.001f;

  fvec2 desp;
  fvec2 finalVelocity;
  fvec2 initialVelocity = velocity;

  finalVelocity.x = initialVelocity.x + (acceleration.x * time);
  finalVelocity.y = initialVelocity.y + (acceleration.y * time);

  desp.x = initialVelocity.x * time + ((acceleration.x * 0.5f) * (time * time));
  desp.y = initialVelocity.y * time + ((acceleration.y * 0.5f) * (time * time));

  position.x = position.x + desp.x;
  position.y = position.y + desp.y;

  velocity = finalVelocity;
}

void Pawn::Draw()
{
  ESAT::DrawSprite(spriteHandler, position.x, position.y);
}