#include "Map.h"
#include <iostream>
#include <fstream>
#include <vector>

Map::Map(const char *spritePath, const char *costPath)
{
  //Door initialize
  Door1Opened = ESAT::SpriteFromFile("../bin/gfx/maps/door_h_open.bmp");
  Door1Closed = ESAT::SpriteFromFile("../bin/gfx/maps/door_h_closed.bmp");
  Door2Opened = ESAT::SpriteFromFile("../bin/gfx/maps/door_v_open.bmp");
  Door2Closed = ESAT::SpriteFromFile("../bin/gfx/maps/door_v_closed.bmp");

  doors[0].position = fvec2(144.0f, 214.0f);
  doors[0].opened = false;

  doors[1].position = fvec2(816.0f, 256.0f);
  doors[1].opened = false;

  //Box initialize
  boxSprite = ESAT::SpriteFromFile("../bin/gfx/maps/box2.png");
  float boxSize = ESAT::SpriteWidth(boxSprite);

  for (int i = 0; i < num_boxes/2; i++)
  {
    ivec2 min = ivec2(363, 241);
    ivec2 max = ivec2(400, 313);

    float x = rand() % (max.x - min.x) + min.x;
    float y = rand() % (max.y - min.y) + min.y;

    boxes[i].parent = nullptr;
    boxes[i].position = fvec2(x, y);
    boxes[i].inUse = false;
    boxes[i].inCooldown = false;
    boxes[i].cooldownSeconds = 5.0f;
  }

  for (int i = num_boxes / 2; i < num_boxes; i++)
  {
    ivec2 min = ivec2(660, 241);
    ivec2 max = ivec2(696, 313);

    float x = rand() % (max.x - min.x) + min.x;
    float y = rand() % (max.y - min.y) + min.y;

    boxes[i].parent = nullptr;
    boxes[i].position = fvec2(x, y);
    boxes[i].inUse = true;
    boxes[i].inCooldown = false;
  }

  mapSprite = ESAT::SpriteFromFile(spritePath);
  costSprite = ESAT::SpriteFromFile(costPath);

  mapSize.x = ESAT::SpriteWidth(mapSprite);
  mapSize.y = ESAT::SpriteHeight(mapSprite);

  costSize.x = ESAT::SpriteWidth(costSprite);
  costSize.y = ESAT::SpriteHeight(costSprite);

  nodeSize.x = (float)mapSize.x / (float)costSize.x;
  nodeSize.y = (float)mapSize.y / (float)costSize.y;

  map = new unsigned int[costSize.x * costSize.y];

  for (int i = 0; i < costSize.y; i++)
  {
    for (int j = 0; j < costSize.x; j++)
    {
      unsigned char RGBA[4];
      unsigned int value = 0;
      ESAT::SpriteGetPixel(costSprite, j, i, RGBA);

      if (RGBA[0] == 34)
        value = insideZone;
      else if (RGBA[0] == 127)
        value = workZone;
      else if (RGBA[0] == 200)
        value = restZone;
      else if (RGBA[0] == 237 || RGBA[0] == 240)
      {
        if (RGBA[0] == 237)
          doors[0].index.push_back((costSize.x * i) + j);
        else if (RGBA[0] == 240)
          doors[1].index.push_back((costSize.x * i) + j);
        value = closedDoor;
      }
      else if (RGBA[0] == 255)
        value = outsideZone;
      
      if (RGBA[2] == 204)
        value = baseA;
      else if (RGBA[2] == 232)
        value = baseB;

      map[(costSize.x * i) + j] = value;
    }
  }

  //Debug map
  /*
  for (int i = 0; i < costSize.y; i++)
  {
    for (int j = 0; j < costSize.x; j++)
    {
      std::cout << map[(costSize.x * i) + j];
    }
    std::cout << std::endl;
  }
  */
}

Map::~Map()
{
  ESAT::SpriteRelease(mapSprite);
  ESAT::SpriteRelease(costSprite);
  delete map;
}

int Map::GetMapValue(fvec2 point, bool normalized)
{
  fvec2 nPoint = point;

  if (!normalized)
    nPoint = fvec2((int)(point.x / nodeSize.x), (int)(point.y / nodeSize.y));

  if (nPoint.x < 0 || nPoint.x > mapSize.x || nPoint.y < 0 || nPoint.y > mapSize.y)
    return -1;

  int costX = nPoint.x;
  int costY = nPoint.y;

  return map[costX + (costY * costSize.x)];
}

void Map::Update(int dt)
{
  float seconds = dt / 1000.0f;

  for (int i = 0; i < num_boxes; i++)
  {
    if (boxes[i].inCooldown)
    {
      boxes[i].cooldownSeconds -= seconds;
      if (boxes[i].cooldownSeconds <= 0.0f)
        RestartBox(i);
    }
    else
    {
      if (boxes[i].inUse)
      {
        if (boxes[i].parent != nullptr)
        {
          fvec2 pos = boxes[i].parent->GetPosition();
          boxes[i].position.x = pos.x + 10.0f;
          boxes[i].position.y = pos.y;
        }
      }
    }
  }
}

void Map::Draw()
{
  ESAT::DrawSprite(mapSprite, 0.0f, 0.0f);

  if (doors[0].opened)
    ESAT::DrawSprite(Door1Opened, doors[0].position.x, doors[0].position.y);
  else
    ESAT::DrawSprite(Door1Closed, doors[0].position.x, doors[0].position.y);
  
  if (doors[1].opened)
    ESAT::DrawSprite(Door2Opened, doors[1].position.x, doors[1].position.y);
  else
    ESAT::DrawSprite(Door2Closed, doors[1].position.x, doors[1].position.y);

  for (int i = 0; i < num_boxes; i++)
    ESAT::DrawSprite(boxSprite, boxes[i].position.x, boxes[i].position.y);     
}

fvec2 Map::WorldToCostCoords(fvec2 point)
{
  return fvec2((int)(point.x / nodeSize.x), (int)(point.y / nodeSize.y));
}

fvec2 Map::CostToWorldCoords(fvec2 point)
{
  return fvec2(point.x * nodeSize.x, point.y * nodeSize.y);
}

bool Map::GetDoorStatus(int id)
{
  return doors[id].opened;
}

void Map::SetDoorStatus(int id, bool value)
{
  for (int i = 0; i < doors[id].index.size(); i++)
  {
    if (value)
      map[doors[id].index[i]] = openedDoor;
    else
      map[doors[id].index[i]] = closedDoor;
  }
  doors[id].opened = value;
}

int Map::GetNearestBox(fvec2 pos)
{
  float bestDistance = 9999.0f;
  int bestBox = 0;
  for (int i = 0; i < num_boxes; i++)
  {
    if (!boxes[i].inUse)
    {
      float distance = math::Distance(pos, boxes[i].position);
      if (distance < bestDistance)
      {
        bestDistance = distance;
        bestBox = i;
      }
    }
  }

  boxes[bestBox].inUse = true;
  return bestBox;
}

void Map::AttachBox(Pawn *p, int boxID)
{
  boxes[boxID].parent = p;
  boxes[boxID].inUse = true;
}

void Map::DetachBox(int boxID)
{
  boxes[boxID].parent = nullptr;
  boxes[boxID].inUse = true;
  boxes[boxID].inCooldown = true;
}

void Map::RestartBox(int boxID)
{
  ivec2 min = ivec2(363, 241);
  ivec2 max = ivec2(400, 313);

  float x = rand() % (max.x - min.x) + min.x;
  float y = rand() % (max.y - min.y) + min.y;

  boxes[boxID].position = fvec2(x, y);
  boxes[boxID].parent = nullptr;
  boxes[boxID].inUse = false;
  boxes[boxID].inCooldown = false;
  boxes[boxID].cooldownSeconds = 5.0f;
}