#include "Pathfinder.h"
#include <ESAT/draw.h>
#include "iostream"

Pathfinder::Pathfinder()
{
  
}

Pathfinder::~Pathfinder()
{
  openList.clear();
  closedList.clear();
}

Pathfinder* Pathfinder::GetInstance()
{
  static Pathfinder instance;
  return &instance;
}

void Pathfinder::Init(Map *mapRef)
{
  currentMap = mapRef;
}

std::vector<fvec2> Pathfinder::GetPath(const fvec2 *start, const fvec2 *end, bool door)
{
  std::cout << "Requested path from: " << start->x << ", " << start->y << " to: " << end->x << ", " << end->y << std::endl;
  std::vector<fvec2> result;

  fvec2 startPoint = currentMap->WorldToCostCoords(*start);
  fvec2 endPoint = currentMap->WorldToCostCoords(*end);

  if (door)
  {
    if (currentMap->GetMapValue(startPoint, true) < 1) { return result; }
    if (currentMap->GetMapValue(endPoint, true) < 1) { return result; }
  }
  else
  {
    if (currentMap->GetMapValue(startPoint, true) < 2) { return result; }
    if (currentMap->GetMapValue(endPoint, true) < 2) { return result; }
  }

  bool inCache = false;
  for (int i = 0; i < pathsInCache.size(); i++)
  {
    if (start->x == pathsInCache[i].path[0].x && start->y == pathsInCache[i].path[0].y)
    {
      if (end->x == pathsInCache[i].path[pathsInCache[i].path.size() - 1].x && end->y == pathsInCache[i].path[pathsInCache[i].path.size() - 1].y)
      {
        result = pathsInCache[i].path;
        pathsInCache[i].value += 100;
        inCache = true;
        std::cout << "Found path in cache!\n"<< std::endl;
        break;
      }
    }

    pathsInCache[i].value--;
    if (pathsInCache[i].value <= 0)
      pathsInCache.erase(pathsInCache.begin() + i);
  }
  
  if (!inCache)
  {
    finished = false;
    parentNode = nullptr;
    openList.clear();
    closedList.clear();

    Node startNode;
    startNode.position = startPoint;
    startNode.parent = parentNode;
    startNode.g = 0.0f;
    startNode.h = math::Distance(startPoint, endPoint);
    startNode.f = 0.0f;

    openList.push_back(startNode);

    while (!finished)
    {
      Node *q = GetBestNode();
      ProcessAroundNodes(&q->position, &endPoint);
    }

    Node *currentNode = &closedList.back();
    while (currentNode != nullptr)
    {
      fvec2 tmp = currentMap->CostToWorldCoords(currentNode->position);
      result.push_back(tmp);
      currentNode = currentNode->parent;
    }

    std::reverse(result.begin(), result.end());
    result[0] = *start;
    result.push_back(*end);

    PathCache path;
    path.value = 500;
    path.path = result;
    pathsInCache.push_back(path);

    std::cout << "Found path! (" << result.size() << " Nodes)\n" << std::endl;
  }
  return result;
}

void Pathfinder::ProcessAroundNodes(const fvec2 *start, const fvec2 *end)
{
  std::vector<fvec2> nodePos;
  nodePos.reserve(8);

  fvec2 nodePos1(start->x - 1.0f, start->y - 1.0f);
  nodePos.push_back(nodePos1);

  //Up node
  fvec2 nodePos2(start->x, start->y - 1.0f);
  nodePos.push_back(nodePos2);

  //Up right node
  fvec2 nodePos3(start->x + 1.0f, start->y - 1.0f);
  nodePos.push_back(nodePos3);

  //Left node
  fvec2 nodePos4(start->x - 1.0f, start->y);
  nodePos.push_back(nodePos4);

  //Down left node
  fvec2 nodePos6(start->x - 1.0f, start->y + 1.0f);
  nodePos.push_back(nodePos6);

  //Down node
  fvec2 nodePos7(start->x, start->y + 1.0f);
  nodePos.push_back(nodePos7);

  //Right node
  fvec2 nodePos5(start->x + 1.0f, start->y);
  nodePos.push_back(nodePos5);

  //Down right node
  fvec2 nodePos8(start->x + 1.0f, start->y + 1.0f);
  nodePos.push_back(nodePos8);

  int nodeValue = -1;
  for (int i = 0; i < 8; i++)
  {
    nodeValue = currentMap->GetMapValue(nodePos[i], true);
    if (nodeValue >= 2)
    {
      Node node = ProcessNode(&nodePos[i], end);
      if (!InOpenList(&node) && !InClosedList(&node))
        if (!PathFinished(&node, end))
          openList.push_back(node);
    }
  }
}

Node Pathfinder::ProcessNode(const fvec2 *nodePosition, const fvec2 *end)
{
  Node node;
  node.parent = parentNode;
  node.position = *nodePosition;

  node.g = node.parent->g + math::Distance(node.parent->position, *nodePosition);

  node.h = math::Distance(*nodePosition, *end);
  node.f = node.g + node.h;

  return node;
}

Node* Pathfinder::GetBestNode()
{
  std::list<Node>::iterator bestNode = openList.begin();
  for (std::list<Node>::iterator i = openList.begin(); i != openList.end(); i++)
  {
    if (i->f < bestNode->f)
      bestNode = i;
  }

  closedList.push_back(*bestNode);
  openList.erase(bestNode);
  parentNode = &closedList.back();
  return parentNode;
}

bool Pathfinder::InOpenList(const Node *node)
{
  for (std::list<Node>::iterator j = openList.begin(); j != openList.end(); j++)
  {
    if (j->position.x == node->position.x && j->position.y == node->position.y)
      if (j->f <= node->f) return true;
  }

  return false;
}

bool Pathfinder::InClosedList(const Node *node)
{
  for (std::list<Node>::iterator i = closedList.begin(); i != closedList.end(); i++)
  {
    if (i->position.x == node->position.x && i->position.y == node->position.y)
      if (i->f <= node->f) return true;
  }

  return false;
}

bool Pathfinder::PathFinished(const Node *node, const fvec2 *end)
{
  if (node->position.x == end->x && node->position.y == end->y)
  { 
    finished = true;
    closedList.push_back(*node);
    return true;
  }

  return false;
}