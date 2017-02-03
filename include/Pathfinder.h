#ifndef PATHFINDER_H_
#define PATHFINDER_H_

#include "Pawn.h"
#include "Agent.h"
#include "Map.h"

#include "vector"
#include "list"

struct Node
{
  Node *parent;
  fvec2 position;
  float f, g, h;
};

struct PathCache
{
  int value;
  std::vector<fvec2> path;
};

//TODO: Do this class a singleton and save the most common paths
class PathfinderCache
{
public:

private:

};

//TODO: This class will be instantiated each pathfind request by threads and consult the pathfinder cache singleton
class Pathfinder
{
public:
  static Pathfinder* GetInstance();
  void Init(Map *mapRef);

  std::vector<fvec2> GetPath(const fvec2 *start, const fvec2 *end, bool door);

  Map *currentMap;

private:
  Pathfinder();
  ~Pathfinder();

  Node* GetBestNode();
  Node ProcessNode(const fvec2 *nodePosition, const fvec2 *end);
  void ProcessAroundNodes(const fvec2 *start, const fvec2 *end);
  bool InOpenList(const Node *node);
  bool InClosedList(const Node *node);
  bool PathFinished(const Node *node, const fvec2 *end);

  Node *parentNode = nullptr;
  bool finished = false;

  std::list<Node> openList;
  std::list<Node> closedList;

  std::vector<PathCache>pathsInCache;
};

#endif