#ifndef MAP_H_
#define MAP_H_

#include <ESAT/sprite.h>
#include "Pawn.h"

#include"vector"

struct Door
{
  fvec2 position;
  bool opened;
  std::vector<int> index;
};

struct Box
{
  fvec2 position;
  Pawn *parent;
  bool inUse;
  bool inCooldown;
  float cooldownSeconds;
};

class Map
{
public:
  Map(const char *spritePath, const char *costPath);
  ~Map();

  void Update(int dt);
  void Draw();

  int GetMapValue(fvec2 point, bool normalized);
  inline fvec2 GetNodeSize() { return nodeSize; }
  fvec2 WorldToCostCoords(fvec2 point);
  fvec2 CostToWorldCoords(fvec2 point);

  bool GetDoorStatus(int id);
  void SetDoorStatus(int id, bool value);

  inline fvec2 GetBoxPosition(int boxID) { return boxes[boxID].position; }
  int GetNearestBox(fvec2 pos);
  void AttachBox(Pawn *p, int boxID);
  void DetachBox(int boxID);
  void RestartBox(int boxID);

  const unsigned int wall = 0;
  const unsigned int closedDoor = 1;
  const unsigned int openedDoor = 2;
  const unsigned int outsideZone = 3;
  const unsigned int insideZone = 4;
  const unsigned int restZone = 5;
  const unsigned int workZone = 6;
  const unsigned int baseA = 7;
  const unsigned int baseB = 8;

private:
  ESAT::SpriteHandle mapSprite;
  ESAT::SpriteHandle costSprite;

  ESAT::SpriteHandle Door1Opened;
  ESAT::SpriteHandle Door1Closed;
  ESAT::SpriteHandle Door2Opened;
  ESAT::SpriteHandle Door2Closed;

  ESAT::SpriteHandle boxSprite;

  ivec2 mapSize;
  ivec2 costSize;
  fvec2 nodeSize;

  unsigned int* map;
  Door doors[2];
  Box boxes[20];
  const int num_boxes = 20;
};

#endif
