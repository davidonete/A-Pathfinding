#ifndef AGENT_H_
#define AGENT_H_

#include "Pawn.h"
#include "Map.h"
#include "Pathfinder.h"

#include "vector"

enum AgentState
{
  kAgentStateRescue,
  kAgentStateRunAway,
  kAgentStateWork,
  kAgentStateRest,
  kAgentStateFree,
  kAgentStatePatrol,
  kAgentStateSuspect,
  kAgentStateAlert
};

/*
enum AgentDirectionPattern
{
  kAgentDirectionUp,
  kAgentDirectionDown,
  kAgentDirectionRight,
  kAgentDirectionLeft,
  kAgentDirectionUpRight,
  kAgentDirectionUpLeft,
  kAgentDirectionDownRight,
  kAgentDirectionDownLeft
};
*/
class FortAgent;

//////////////////////////////////////////////////////////

class AgentPawn : public Pawn
{
private:
  std::vector<fvec2> path;
  std::vector <std::vector<fvec2>> pathCache;
  int pathID;

  void Decelerate();

public:
  AgentPawn(){}
  AgentPawn(float x, float y, float mSpeed, const char* path);
  virtual ~AgentPawn(){}

  void Update(int dt);

  bool GoToPoint(fvec2 from, fvec2 point, bool door);
  void GoToDirection(fvec2 dir, float speed);
  void GoToRandomDirection(float speed);

  void Stop();

  void PrecomputePath(fvec2 *from, fvec2 *to);
};

//////////////////////////////////////////////////////////

class AgentBehavior
{
  struct AgentBlackBoard
  {
    AgentPawn *pawn;
    Pawn *target;
    FortAgent *fort;

    AgentBlackBoard(){};
    ~AgentBlackBoard(){};
  };

protected:
  AgentState state;
  AgentBlackBoard* blackboard;

public:
  AgentBehavior() { blackboard = new AgentBlackBoard(); };
  ~AgentBehavior(){}

  virtual void Init() {}
  virtual void Update(int dt) {}
  virtual void AlarmHeard(int AlarmTime) {}

  inline void SetState(AgentState state_) { state = state_; }
  inline void AttachPawn(AgentPawn *p) { blackboard->pawn = p; }
  inline void SetTarget(Pawn *target) { blackboard->target = target; }
  inline void SetFortReference(FortAgent *f) { blackboard->fort = f; }
};

//////////////////////////////////////////////////////////

class SoldierBehavior : public AgentBehavior
{
  enum SoldierMovement
  {
    kSoldierMovementBaseBToDoor1,
    kSoldierMovementBaseAToDoor2,
    kSoldierMovementOutDoor1ToDoor2,
    kSoldierMovementInDoor1ToDoor2,
    kSoldierMovementInDoor2ToDoor1,
    kSoldierMovementOutDoor1ToBaseB,
    kSoldierMovementOutDoor2ToBaseA
  };

public:
  SoldierBehavior(){}
  ~SoldierBehavior(){}

  void Init() override;
  void Update(int dt) override;
  void AlarmHeard(int AlarmTime) override;

private:
  void Rescue();
  void RunAway();

  fvec2 fromPoint, goalPoint;
  SoldierMovement movement;
  std::vector<fvec2> waypoints;

  float rescueVelocity = 30.0f;
  const float runAwayVelocity = 40.0f;
};

//////////////////////////////////////////////////////////

class PrisonerBehavior : public AgentBehavior
{
  enum PrisonerMovement
  {
    kPrisonerMovementInRestToWork,
    kPrisonerMovementGetBox,
    kPrisonerMovementUnloadBox,
    kPrisonerMovementInWorkToRest,
    kPrisonerMovementInDoor1ToDoor2,
    kPrisonerMovementInDoor2ToDoor1,
    kPrisonerMovementOutDoor1ToBaseB,
    kPrisonerMovementOutDoor2ToBaseA
  };

public:
  PrisonerBehavior(){}
  ~PrisonerBehavior(){}

  void Init() override;
  void Update(int dt) override;
  void AlarmHeard(int AlarmTime) override;

  int GetEnergy();
  void GoToWork(fvec2 from);
  void GoToRest(fvec2 from);

private:
  void Work();
  void Rest(int dt);
  void RunAway(int dt);
  void Free();

  const int maxEnergy = 5;
  int energy = 5;

  int boxID = -1;

  float minDistance;
  fvec2 fromPoint, goalPoint;
  std::vector<fvec2> waypoints;
  PrisonerMovement movement;

  bool was_working = false;

  float alarmCooldown = 0.0f;
  float restCooldown = 0.0f;
  const float restVelocity = 20.0f;
  const float workVelocity = 35.0f;
  const float runAwayVelocity = 45.0f;
};

//////////////////////////////////////////////////////////

class GuardBehavior : public AgentBehavior
{
  enum GuardMovement
  {
    kGuardMovementChase,
    kGuardMovementCloseDoor1,
    kGuardMovementCloseDoor2
  };

public:
  GuardBehavior() {}
  ~GuardBehavior() {}

  void Init() override;
  void Update(int dt) override;
  void AlarmHeard(int AlarmTime) override;

private:
  void Patrol(int dt);
  void Suspect(int dt);
  void Alert(int dt);

  bool alarmed = false;
  bool chasing_prisoner = false;
  float patrolCooldown = 0.0f;
  float alarmCooldown = 0.0f;

  float minDistance;
  fvec2 fromPoint, goalPoint;
  GuardMovement movement;
  std::vector<fvec2> waypoints;

  const float patrolVelocity = 30.0f;
  const float alarmVelocity = 45.0f;
  const float chaseVelocity = 50.0f;
};

//////////////////////////////////////////////////////////

class Agent
{
protected:
  AgentPawn *pawn;
  AgentBehavior *behavior;

public:
  Agent(){}
  ~Agent(){}

  virtual void Init() {}
  virtual void Init(float x, float y, float maxSpeed, const char *path) {}
  virtual void Init(float x, float y, float maxSpeed, const char *path, AgentState startState) {}
  
  virtual void Update(int dt) { behavior->Update(dt); pawn->Update(dt); }
  virtual void Draw() { pawn->Draw(); }
  
  void SetFortReference(FortAgent *a) { behavior->SetFortReference(a); }
  void SetAlarm(int Time) { behavior->AlarmHeard(Time); }

  inline fvec2 GetPosition() { return pawn->GetPosition(); }
  inline Pawn* GetPawn() { return pawn; }
};

//////////////////////////////////////////////////////////

class SoldierAgent : public Agent
{
public:
  SoldierAgent(){}
  ~SoldierAgent(){}
  void Init(float x, float y, float maxSpeed, const char *path) override;
};

//////////////////////////////////////////////////////////

class PrisonerAgent : public Agent
{
public:
  PrisonerAgent(){}
  ~PrisonerAgent(){}
  void Init(float x, float y, float maxSpeed, const char *path, AgentState startState) override;
  void CallOfDuty();
  int GetEnergy();
};

//////////////////////////////////////////////////////////

class GuardAgent : public Agent
{
public:
  GuardAgent(){}
  ~GuardAgent(){}

  void Init(float x, float y, float maxSpeed, const char *path) override;
};

//////////////////////////////////////////////////////////

class FortAgent
{
public:
  FortAgent(){}
  ~FortAgent(){}

  void Init();
  void AddAgent(SoldierAgent *a);
  void AddAgent(PrisonerAgent *a);
  void AddAgent(GuardAgent *a);
  void ActiveAlarm(int alarmTime);

  bool GetDoorStatus(int id);
  void SetDoorStatus(int id, bool value);

  fvec2 GetBoxPosition(int boxID);
  int GetNearestBox(fvec2 pos);
  void AttachBox(Pawn *p, int boxID);
  void DetachBox(int boxID);
  void RelieveWorker();
  void PrisonerFreed();

  Pawn* GetNearestSoldier(float minDistance, fvec2 pos);
  Pawn* GetNearestPrisoner(float minDistance, fvec2 pos);

  inline int GetPrisonersLeft() { return prisoners_left; }

private:
  int prisoners_left = 0;
  std::vector<SoldierAgent*> soldiers;
  std::vector<PrisonerAgent*> prisoners;
  std::vector<GuardAgent*> guards;
  Map *map;
};

//////////////////////////////////////////////////////////

#endif