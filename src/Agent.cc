#include "Agent.h"

#include <iostream>
#include <stdlib.h>

//////////////////////////////////////////////////////////

AgentPawn::AgentPawn(float x, float y, float mSpeed, const char* path)
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

void AgentPawn::Decelerate()
{
  if (velocity.x > -5.0f && velocity.x < 5.0f)
  {
    velocity.x = 0.0f;
    acceleration.x = 0.0f;
  }
  if (velocity.y > -5.0f && velocity.y < 5.0f)
  {
    velocity.y = 0.0f;
    acceleration.y = 0.0f;
  }
}

void AgentPawn::Update(int dt)
{
  Move(dt);
}

bool AgentPawn::GoToPoint(fvec2 from, fvec2 point, bool door)
{
  if (path.empty())
  {
    fvec2 abs_pos = fvec2(round(from.x), round(from.y));
    fvec2 nabs_pos = fvec2(round(point.x), round(point.y));
    Map *map = Pathfinder::GetInstance()->currentMap;

    int fromZone = map->GetMapValue(abs_pos, false);
    int goalZone = map->GetMapValue(nabs_pos, false);
    
    //If the zone of both points is the same and is the work zone or rest zone dont calculate pathfinding
    if (fromZone == goalZone && (fromZone == map->workZone || fromZone == map->restZone))
    {
      path.push_back(abs_pos);
      path.push_back(nabs_pos);
    }
    else
    {
      //If the distance is low dont calculate pathfinding
      float distance = math::Distance(from, point);
      if (distance < 100.0f)
      {
        path.push_back(abs_pos);
        path.push_back(point);
      }
      else
        path = Pathfinder::GetInstance()->GetPath(&abs_pos, &nabs_pos, door);
    }

    if (path.empty())
      std::cout << "BAD PATH REQUESTED!" << std::endl;
    else
    {
      pathID = 1;

      fvec2 direction = math::VectorDirection(position, path[pathID]);
      direction = math::NormalizeVector(direction);

      velocity.x = direction.x * maxSpeed;
      velocity.y = direction.y * maxSpeed;
    }
  }
  else
  {
    fvec2 abs_pos = fvec2(round(position.x), round(position.y));
    if (path[pathID].x == abs_pos.x && path[pathID].y == abs_pos.y)
    {
      //Path finished
      if (path.size() - 1 == pathID)
      {
        Stop();
        path.clear();
        return true;
      }
      else
      {
        pathID++;
        fvec2 direction = math::VectorDirection(position, path[pathID]);
        direction = math::NormalizeVector(direction);

        velocity.x = direction.x * maxSpeed;
        velocity.y = direction.y * maxSpeed;
      }
    }
  }

  return false;
}

void AgentPawn::GoToDirection(fvec2 dir, float speed)
{
  velocity.x = dir.x * speed;
  velocity.y = dir.y * speed;

  acceleration.x = (velocity.x / 2.5f) * -1;
  acceleration.y = (velocity.y / 2.5f) * -1;
}

void AgentPawn::GoToRandomDirection(float speed)
{
  float x = rand() % 800 + 1;
  float y = rand() % 600 + 1;

  fvec2 direction = math::NormalizeVector(math::VectorDirection(position, fvec2(x, y)));
  GoToDirection(direction, speed);
}

void AgentPawn::Stop()
{
  velocity.x = 0.0f;
  velocity.y = 0.0f;

  acceleration.x = 0.0f;
  acceleration.y = 0.0f;

  path.clear();
}

void AgentPawn::PrecomputePath(fvec2 *from, fvec2 *to)
{
  Pathfinder::GetInstance()->GetPath(from, to, false);
}

//////////////////////////////////////////////////////////

void SoldierBehavior::Init()
{
  std::cout << "Soldier:" << std::endl;
  //Base B
  waypoints.push_back(fvec2(45.0f, 670.0f));
  //Outside Door 1
  waypoints.push_back(fvec2(155.0f, 210.0f));
  //Outside Door 2 
  waypoints.push_back(fvec2(835.0f, 275.0f));
  //Inside Door 1
  waypoints.push_back(fvec2(155.0f, 235.0f));
  //Inside Door 2 
  waypoints.push_back(fvec2(805.0f, 275.0f));
  //Base A
  waypoints.push_back(fvec2(900.0f, 670.0f));

  //The soldier will go first to the Door 1 from Base B
  movement = kSoldierMovementBaseBToDoor1;
  goalPoint = waypoints[1];
  fromPoint = waypoints[0];

  rescueVelocity = blackboard->pawn->GetMaxSpeed();

  //Precompute paths
  //Base B to Door 1
  blackboard->pawn->PrecomputePath(&waypoints[0], &waypoints[1]);
  //Outside Door 1 to Outside Door 2
  blackboard->pawn->PrecomputePath(&waypoints[1], &waypoints[2]);
  //Inside Door 1 to Inside Door 2
  blackboard->pawn->PrecomputePath(&waypoints[3], &waypoints[4]);
  //Inside Door 2 to Inside Door 1
  blackboard->pawn->PrecomputePath(&waypoints[4], &waypoints[3]);
  //Outside Door 1 to Base B
  blackboard->pawn->PrecomputePath(&waypoints[1], &waypoints[0]);
  //Outside Door 2 to Base A
  blackboard->pawn->PrecomputePath(&waypoints[2], &waypoints[5]);
}

void SoldierBehavior::Update(int dt)
{
  switch (state)
  {
    case kAgentStateRescue:
      Rescue();
    break;
    case kAgentStateRunAway:
      RunAway();
    break;
    case kAgentStateFree:
    
    break;
    default: break;
  }
}

void SoldierBehavior::AlarmHeard(int AlarmTime)
{
  if (state != kAgentStateRunAway)
  {
    state = kAgentStateRunAway;
    blackboard->pawn->Stop();
    blackboard->pawn->SetMaxSpeed(runAwayVelocity);
    fvec2 pawnPos = blackboard->pawn->GetPosition();

    float distanceToDoor1 = math::Distance(pawnPos, waypoints[3]);
    float distanceToDoor2 = math::Distance(pawnPos, waypoints[4]);

    fromPoint = pawnPos;
    if (distanceToDoor1 < distanceToDoor2)
    {
      movement = kSoldierMovementInDoor2ToDoor1;
      goalPoint = waypoints[3];
    }
    else
    {
      movement = kSoldierMovementInDoor1ToDoor2;
      goalPoint = waypoints[4];
    }
  }
}

void SoldierBehavior::Rescue()
{
  float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
  if (goalDistance <= 100.0f)
  {
    blackboard->pawn->Stop();
    switch (movement)
    {
      case kSoldierMovementBaseBToDoor1:
        //If the Door 1 is opened
        if (blackboard->fort->GetDoorStatus(0))
        {
          movement = kSoldierMovementInDoor1ToDoor2;
          fromPoint = waypoints[3];
          goalPoint = waypoints[4];
        }
        else
        {
          movement = kSoldierMovementOutDoor1ToDoor2;
          fromPoint = waypoints[1];
          goalPoint = waypoints[2];
        }
      break;

      case kSoldierMovementBaseAToDoor2:
      case kSoldierMovementOutDoor1ToDoor2:
        blackboard->fort->SetDoorStatus(1, true);
        movement = kSoldierMovementInDoor2ToDoor1;
        fromPoint = waypoints[4];
        goalPoint = waypoints[3];
      break;

      case kSoldierMovementInDoor2ToDoor1:
        //If there is prisoners left
        if (blackboard->fort->GetPrisonersLeft() > 0)
        {
          blackboard->fort->SetDoorStatus(0, true);
          movement = kSoldierMovementInDoor1ToDoor2;
          fromPoint = waypoints[3];
          goalPoint = waypoints[4];
        }
        else
          AlarmHeard(0);
      break;

      case kSoldierMovementInDoor1ToDoor2:
        if (blackboard->fort->GetPrisonersLeft() > 0)
        {
          blackboard->fort->SetDoorStatus(1, true);
          movement = kSoldierMovementInDoor2ToDoor1;
          fromPoint = waypoints[4];
          goalPoint = waypoints[3];
        }
        else
          AlarmHeard(0);
      break;

      default: break;
    }
  }
  else
    blackboard->pawn->GoToPoint(fromPoint, goalPoint, true);
}

void SoldierBehavior::RunAway()
{
  float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
  if (goalDistance <= 100.0f)
  {
    blackboard->pawn->Stop();
    switch (movement)
    {
      case kSoldierMovementInDoor2ToDoor1:
        blackboard->fort->SetDoorStatus(0, true);
        movement = kSoldierMovementOutDoor1ToBaseB;
        fromPoint = waypoints[1];
        goalPoint = waypoints[0];
      break;

      case kSoldierMovementInDoor1ToDoor2:
        blackboard->fort->SetDoorStatus(1, true);
        movement = kSoldierMovementOutDoor2ToBaseA;
        fromPoint = waypoints[2];
        goalPoint = waypoints[5];
      break;

      case kSoldierMovementOutDoor2ToBaseA:
        if (blackboard->fort->GetPrisonersLeft() > 0)
        {
          state = kAgentStateRescue;
          movement = kSoldierMovementBaseAToDoor2;
          fromPoint = waypoints[5];
          goalPoint = waypoints[2];
        }
        else
          state = kAgentStateFree;
      break;

      case kSoldierMovementOutDoor1ToBaseB:
        if (blackboard->fort->GetPrisonersLeft() > 0)
        {
          state = kAgentStateRescue;
          movement = kSoldierMovementBaseBToDoor1;
          fromPoint = waypoints[0];
          goalPoint = waypoints[1];
        }
        else
          state = kAgentStateFree;
      break;

      default: break;
    }
  }
  else
    blackboard->pawn->GoToPoint(fromPoint, goalPoint, true);
}

//////////////////////////////////////////////////////////

void PrisonerBehavior::Init()
{
  std::cout << "Prisoner:" << std::endl;
  //Rest Area
  waypoints.push_back(fvec2(568.0f, 480.0f));
  //Work Area Load
  waypoints.push_back(fvec2(420.0f, 320.0f));
  //Work Area Unload
  waypoints.push_back(fvec2(660.0f, 320.0f));
  //Inside Door 1
  waypoints.push_back(fvec2(155.0f, 235.0f));
  //Inside Door 2 
  waypoints.push_back(fvec2(805.0f, 275.0f));
  //Base A
  waypoints.push_back(fvec2(900.0f, 670.0f));
  //Base B
  waypoints.push_back(fvec2(45.0f, 670.0f));
  //Outside Door 1
  waypoints.push_back(fvec2(155.0f, 210.0f));
  //Outside Door 2 
  waypoints.push_back(fvec2(835.0f, 275.0f));

  if (state == kAgentStateWork)
    GoToWork(fvec2(0.0f, 0.0f));

  else if (state == kAgentStateRest)
  {
    minDistance = 100.0f;
    fromPoint = blackboard->pawn->GetPosition();
    goalPoint = fromPoint;
  }

  //Precompute paths
  //Rest area to work area load
  blackboard->pawn->PrecomputePath(&waypoints[0], &waypoints[1]);
  //Work area unload to rest area
  blackboard->pawn->PrecomputePath(&waypoints[2], &waypoints[0]);
  //Rest area to Door 2
  blackboard->pawn->PrecomputePath(&waypoints[0], &waypoints[4]);
  //Door 2 to Door 1
  blackboard->pawn->PrecomputePath(&waypoints[4], &waypoints[3]);
  //Door 1 to Door 2
  blackboard->pawn->PrecomputePath(&waypoints[3], &waypoints[4]);
  //Door 2 to Base A
  //blackboard->pawn->PrecomputePath(&waypoints[8], &waypoints[5]);
  //Door 1 to Base B
  //blackboard->pawn->PrecomputePath(&waypoints[7], &waypoints[6]);
}

void PrisonerBehavior::Update(int dt)
{
  switch (state)
  {
  case kAgentStateWork:
    Work();
  break;
  case kAgentStateRest:
    Rest(dt);
  break;
  case kAgentStateRunAway:
    RunAway(dt);
  break;
  case kAgentStateFree:
  
  break;
  default: break;
  }
}

void PrisonerBehavior::AlarmHeard(int AlarmTime)
{
  if (state != kAgentStateRunAway)
  {
    if (state == kAgentStateWork)
    {
      if (boxID != -1)
      {
        blackboard->fort->DetachBox(boxID);
        boxID = -1;
      }

      fvec2 pawnPos = blackboard->pawn->GetPosition();
      Map *map = Pathfinder::GetInstance()->currentMap;

      if (map->GetMapValue(pawnPos, false) == map->workZone)
        fromPoint = waypoints[4];
      else
        fromPoint = pawnPos;

      float distanceToDoor1 = math::Distance(pawnPos, waypoints[3]);
      float distanceToDoor2 = math::Distance(pawnPos, waypoints[4]);
      if (distanceToDoor1 < distanceToDoor2)
      {
        movement = kPrisonerMovementInDoor2ToDoor1;
        goalPoint = waypoints[3];
      }
      else
      {
        movement = kPrisonerMovementInDoor1ToDoor2;
        goalPoint = waypoints[4];
      }
    }
    else if (state == kAgentStateRest)
    {
      was_working = false;
      movement = kPrisonerMovementInDoor1ToDoor2;
      fromPoint = waypoints[0];
      goalPoint = waypoints[4];
    }

    state = kAgentStateRunAway;
    blackboard->pawn->Stop();
    blackboard->pawn->SetMaxSpeed(runAwayVelocity);
    minDistance = 50.0f;
  }
  alarmCooldown = (float)AlarmTime;
}

int PrisonerBehavior::GetEnergy()
{
  if (state == kAgentStateRest)
    return energy;

  return 0;
}

void PrisonerBehavior::GoToWork(fvec2 from)
{
  was_working = true;
  state = kAgentStateWork;
  movement = kPrisonerMovementInRestToWork;
  minDistance = 100.0f;

  fromPoint = from;
  if (from.x == 0)
    fromPoint = waypoints[0];

  goalPoint = waypoints[1];
  blackboard->pawn->SetMaxSpeed(workVelocity);
}

void PrisonerBehavior::GoToRest(fvec2 from)
{
  was_working = false;
  state = kAgentStateWork;
  movement = kPrisonerMovementInWorkToRest;

  fromPoint = from;
  if (from.x == 0)
  {
    blackboard->fort->RelieveWorker();
    fromPoint = waypoints[2];
  }

  goalPoint = waypoints[0];
  minDistance = 100.0f;
  blackboard->pawn->SetMaxSpeed(restVelocity);
}

void PrisonerBehavior::Work()
{
  float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
  if (goalDistance <= minDistance)
  {
    blackboard->pawn->Stop();
    switch (movement)
    {
      case kPrisonerMovementInRestToWork:
        movement = kPrisonerMovementGetBox;
        boxID = blackboard->fort->GetNearestBox(blackboard->pawn->GetPosition());
        fromPoint = blackboard->pawn->GetPosition();
        goalPoint = blackboard->fort->GetBoxPosition(boxID);
        minDistance = 40.0f;
      break;

      case kPrisonerMovementGetBox:
        movement = kPrisonerMovementUnloadBox;
        blackboard->fort->AttachBox(blackboard->pawn, boxID);
        blackboard->pawn->SetMaxSpeed(workVelocity / 2.0f);
        fromPoint = goalPoint;
        goalPoint.x = rand() % 36 + (660);
        goalPoint.y = rand() % 72 + (241);
        minDistance = 100.0f;
      break;

      case kPrisonerMovementUnloadBox:
        movement = kPrisonerMovementInRestToWork;
        blackboard->fort->DetachBox(boxID);
        blackboard->pawn->SetMaxSpeed(workVelocity);
        boxID = -1;

        energy--;
        if (energy <= 0)
          GoToRest(fvec2(0.0f, 0.0f));
      break;

      case kPrisonerMovementInWorkToRest:
        state = kAgentStateRest;
      break;
    }
  }
  else
    blackboard->pawn->GoToPoint(fromPoint, goalPoint, false);
  
}

void PrisonerBehavior::Rest(int dt)
{
  if (restCooldown <= 0.0f)
  {
    float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
    if (goalDistance <= minDistance)
    {
      blackboard->pawn->Stop();
      blackboard->pawn->SetMaxSpeed(restVelocity);
      fromPoint = blackboard->pawn->GetPosition();
      goalPoint.x = rand() % 229 + 544;
      goalPoint.y = rand() % 61 + 482;
      restCooldown = 5.0f;

      if (energy < maxEnergy)
        energy++;
    }
    else
      blackboard->pawn->GoToPoint(fromPoint, goalPoint, false);
  }
  else
    restCooldown -= (dt / 1000.0f);
}

void PrisonerBehavior::RunAway(int dt)
{
  float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
  if (goalDistance <= minDistance)
  {
    blackboard->pawn->Stop();
    switch (movement)
    {
      case kPrisonerMovementInDoor2ToDoor1:
        if (blackboard->fort->GetDoorStatus(0))
        {
          movement = kPrisonerMovementOutDoor1ToBaseB;
          fromPoint = waypoints[7];
          goalPoint = waypoints[6];
          minDistance = 100.0f;
        }
        else
        {
          movement = kPrisonerMovementInDoor1ToDoor2;
          fromPoint = waypoints[3];
          goalPoint = waypoints[4];
          minDistance = 50.0f;
        }
      break;

      case kPrisonerMovementInDoor1ToDoor2:
        if (blackboard->fort->GetDoorStatus(1))
        {
          movement = kPrisonerMovementOutDoor2ToBaseA;
          fromPoint = waypoints[8];
          goalPoint = waypoints[5];
          minDistance = 100.0f;
        }
        else
        {
          movement = kPrisonerMovementInDoor2ToDoor1;
          fromPoint = waypoints[4];
          goalPoint = waypoints[3];
          minDistance = 50.0f;
        }
      break;

      case kPrisonerMovementOutDoor1ToBaseB:
      case kPrisonerMovementOutDoor2ToBaseA:
        Free();
        state = kAgentStateFree;
      break;

      default: break;
    }
  }
  else
    blackboard->pawn->GoToPoint(fromPoint, goalPoint, false);

  //Only if the prisoners are still inside the fort when the alarm is over
  if (movement != kPrisonerMovementOutDoor1ToBaseB && movement != kPrisonerMovementOutDoor2ToBaseA)
  {
    alarmCooldown -= (dt / 1000.0f);
    if (alarmCooldown <= 0.0f)
    {
      blackboard->pawn->Stop();
      if (was_working)
      {
        state = kAgentStateWork;
        GoToWork(blackboard->pawn->GetPosition());
      }
      else
      {
        state = kAgentStateRest;
        GoToRest(blackboard->pawn->GetPosition());
      }
      //Move to rest area or work area
    }
  }
}

void PrisonerBehavior::Free()
{
  blackboard->fort->PrisonerFreed();
}

//////////////////////////////////////////////////////////

void GuardBehavior::Init()
{
  std::cout << "Guard:" << std::endl;
  //Inside Door 1
  waypoints.push_back(fvec2(155.0f, 235.0f));
  //Inside Door 2 
  waypoints.push_back(fvec2(805.0f, 275.0f));
  //Patrol point up left
  waypoints.push_back(fvec2(215.0f, 87.0f));
  //Patrol point up right
  waypoints.push_back(fvec2(729.0f, 96.0f));
  //Patrol point down left
  waypoints.push_back(fvec2(507.0f, 503.0f));
  //Patrol point down right
  waypoints.push_back(fvec2(664.0f, 513.0f));
  //Patrol point
  waypoints.push_back(fvec2(302.0f, 205.0f));
  //Patrol point
  waypoints.push_back(fvec2(312.0f, 351.0f));

  for (int excludedID = 0; excludedID < waypoints.size(); excludedID++)
  {
    for (int i = 0; i < waypoints.size(); i++)
    {
      if (i != excludedID)
        blackboard->pawn->PrecomputePath(&waypoints[excludedID], &waypoints[i]);
    }
  }

  blackboard->target = nullptr;

  int nextPatrol = rand() % waypoints.size();
  fromPoint = blackboard->pawn->GetPosition();
  goalPoint = waypoints[nextPatrol];
  minDistance = 150.0f;
  blackboard->pawn->SetMaxSpeed(patrolVelocity);

  state = kAgentStatePatrol;
}

void GuardBehavior::Update(int dt)
{
  switch (state)
  {
    case kAgentStatePatrol:
      Patrol(dt);
    break;
    case kAgentStateSuspect:
      Suspect(dt);
    break;
    default: break;
  }
}

void GuardBehavior::AlarmHeard(int AlarmTime)
{
  if (!alarmed)
  {
    //state = kAgentStateAlert;
    alarmed = true;
    if (blackboard->target == nullptr)
    {
      patrolCooldown = 0.0f;
      //blackboard->pawn->Stop();
      blackboard->pawn->SetMaxSpeed(alarmVelocity);
    }
  }
  alarmCooldown = (float)AlarmTime;
}

void GuardBehavior::Patrol(int dt)
{
  Pawn* target = nullptr;
  target = blackboard->fort->GetNearestSoldier(100.0f, blackboard->pawn->GetPosition());
  if (alarmed)
  {
    target = blackboard->fort->GetNearestPrisoner(100.0f, blackboard->pawn->GetPosition());
    if (target != nullptr) chasing_prisoner = true;
  }
  if (target == nullptr)
  {
    if (patrolCooldown <= 0.0f)
    {
      float goalDistance = math::Distance(blackboard->pawn->GetPosition(), goalPoint);
      if (goalDistance <= minDistance)
      {
        if (goalPoint.x == waypoints[0].x && goalPoint.y == waypoints[0].y)
        {
          if (blackboard->fort->GetDoorStatus(0))
          {
            Map* map = Pathfinder::GetInstance()->currentMap;
            fvec2 pawnPos = blackboard->pawn->GetPosition();
            if (map->GetMapValue(pawnPos, false) > map->outsideZone)
            {
              blackboard->fort->SetDoorStatus(0, false);
              blackboard->fort->ActiveAlarm(30);
            }
          }
        }
        else if (goalPoint.x == waypoints[1].x && goalPoint.y == waypoints[1].y)
        {
          if (blackboard->fort->GetDoorStatus(1))
          {
            Map* map = Pathfinder::GetInstance()->currentMap;
            fvec2 pawnPos = blackboard->pawn->GetPosition();
            if (map->GetMapValue(pawnPos, false) > map->outsideZone)
            {
              blackboard->fort->SetDoorStatus(1, false);
              blackboard->fort->ActiveAlarm(30);
            }
          }
        }

        blackboard->pawn->Stop();
        int nextPatrol = rand() % waypoints.size();
        fromPoint = goalPoint;
        goalPoint = waypoints[nextPatrol];
        minDistance = 150.0f;
        if (!alarmed)
          patrolCooldown = 5.0f;
      }
      else
        blackboard->pawn->GoToPoint(fromPoint, goalPoint, true);
    }
    else
      patrolCooldown -= (dt / 1000.0f);
  }
  else
  {
    blackboard->pawn->Stop();
    blackboard->pawn->SetMaxSpeed(chaseVelocity);
    blackboard->target = target;
    state = kAgentStateSuspect;
  }

  if (alarmed)
  {
    alarmCooldown -= (dt / 1000.0f);
    if (alarmCooldown <= 0.0f)
    {
      alarmed = false;
      blackboard->target = nullptr;
      blackboard->pawn->SetMaxSpeed(patrolVelocity);
    }
  }
}

void GuardBehavior::Suspect(int dt)
{
  blackboard->pawn->Stop();
  Map* map = Pathfinder::GetInstance()->currentMap;

  fvec2 pawnPos = blackboard->pawn->GetPosition();
  fvec2 targetPos = blackboard->target->GetPosition();

  int fromZone = map->GetMapValue(pawnPos, false);
  int goalZone = map->GetMapValue(targetPos, false);

  blackboard->pawn->GoToPoint(pawnPos, targetPos, true);

  float door1Distance = math::Distance(pawnPos, waypoints[0]);
  float door2Distance = math::Distance(pawnPos, waypoints[1]);

  if (door1Distance < 100.0f)
  {
    if (blackboard->fort->GetDoorStatus(0))
    {
      if (fromZone > map->outsideZone)
      {
        blackboard->fort->SetDoorStatus(0, false);
        blackboard->fort->ActiveAlarm(30);
      }
    }
  }
  else if(door2Distance < 100.0f)
  {
    if (blackboard->fort->GetDoorStatus(1))
    {
      if (fromZone > map->outsideZone)
      {
        blackboard->fort->SetDoorStatus(1, false);
        blackboard->fort->ActiveAlarm(30);
      }
    }
  }

  if (goalZone == map->outsideZone)
  {
    blackboard->target = nullptr;
    blackboard->pawn->Stop();
    if (state != kAgentStateAlert)
    {
      blackboard->pawn->SetMaxSpeed(patrolVelocity);
      state = kAgentStatePatrol;
    }

    if (door1Distance < door2Distance)
      goalPoint = waypoints[0];
    else
      goalPoint = waypoints[1];

    fromPoint = pawnPos;
    minDistance = 150.0f;
    patrolCooldown = 0.0f;
  }

  if (alarmed)
  {
    alarmCooldown -= (dt / 1000.0f);
    if (alarmCooldown <= 0.0f)
    {
      alarmed = false;
      chasing_prisoner = false;
      blackboard->pawn->SetMaxSpeed(patrolVelocity);
      state = kAgentStatePatrol;
      blackboard->target = nullptr;
      blackboard->pawn->Stop();

      if (door1Distance < door2Distance)
        goalPoint = waypoints[0];
      else
        goalPoint = waypoints[1];

      fromPoint = pawnPos;
      minDistance = 150.0f;
      patrolCooldown = 0.0f;
    }
  }
}

//////////////////////////////////////////////////////////

void FortAgent::Init()
{
  map = Pathfinder::GetInstance()->currentMap;
}

void FortAgent::AddAgent(SoldierAgent *a)
{
  a->SetFortReference(this);
  soldiers.push_back(a);
}

void FortAgent::AddAgent(PrisonerAgent *a)
{
  a->SetFortReference(this);
  prisoners.push_back(a);
  prisoners_left++;
}

void FortAgent::AddAgent(GuardAgent *a)
{
  a->SetFortReference(this);
  guards.push_back(a);
}

bool FortAgent::GetDoorStatus(int id)
{
  return map->GetDoorStatus(id);
}

void FortAgent::SetDoorStatus(int id, bool value)
{
  map->SetDoorStatus(id, value);
}

void FortAgent::ActiveAlarm(int alarmTime)
{
  std::cout << "Alarm activated!" << std::endl;
  for (int i = 0; i < soldiers.size(); i++)
  {
    fvec2 position = soldiers[i]->GetPosition();
    int zone = map->GetMapValue(position, false);
    if (zone == map->insideZone || zone == map->restZone || zone == map->workZone)
      soldiers[i]->SetAlarm(alarmTime);
  }

  for (int i = 0; i < prisoners.size(); i++)
  {
    fvec2 position = prisoners[i]->GetPosition();
    int zone = map->GetMapValue(position, false);
    if (zone == map->insideZone || zone == map->restZone || zone == map->workZone)
      prisoners[i]->SetAlarm(alarmTime);
  }

  for (int i = 0; i < guards.size(); i++)
  {
    fvec2 position = guards[i]->GetPosition();
    int zone = map->GetMapValue(position, false);
    if (zone == map->insideZone || zone == map->restZone || zone == map->workZone)
      guards[i]->SetAlarm(alarmTime);
  }
}

fvec2 FortAgent::GetBoxPosition(int boxID)
{
  return map->GetBoxPosition(boxID);
}

int FortAgent::GetNearestBox(fvec2 pos)
{
  return map->GetNearestBox(pos);
}

void FortAgent::AttachBox(Pawn *p, int boxID)
{
  map->AttachBox(p, boxID);
}

void FortAgent::DetachBox(int boxID)
{
  map->DetachBox(boxID);
}

void FortAgent::RelieveWorker()
{
  for (int i = 0; i < prisoners.size(); i++)
  {
    if (prisoners[i]->GetEnergy() >= 5)
    {
      prisoners[i]->CallOfDuty();
      break;
    }
  }
}

void FortAgent::PrisonerFreed()
{
  if (prisoners_left > 0)
    prisoners_left--;
}

Pawn* FortAgent::GetNearestSoldier(float minDistance, fvec2 pos)
{
  for (int i = 0; i < soldiers.size(); i++)
  {
    fvec2 soldierPos = soldiers[i]->GetPosition();
    fvec2 abs_pos = fvec2(round(pos.x), round(pos.y));
    fvec2 sabs_pos = fvec2(round(soldierPos.x), round(soldierPos.y));

    int fromZone = map->GetMapValue(abs_pos, false);
    int goalZone = map->GetMapValue(sabs_pos, false);

    if (fromZone == goalZone && (fromZone >= map->insideZone))
      if (math::Distance(abs_pos, soldierPos) <= minDistance)
        return soldiers[i]->GetPawn();
  }

  return nullptr;
}

Pawn* FortAgent::GetNearestPrisoner(float minDistance, fvec2 pos)
{
  for (int i = 0; i < prisoners.size(); i++)
  {
    fvec2 soldierPos = prisoners[i]->GetPosition();
    fvec2 abs_pos = fvec2(round(pos.x), round(pos.y));
    fvec2 sabs_pos = fvec2(round(soldierPos.x), round(soldierPos.y));

    int fromZone = map->GetMapValue(abs_pos, false);
    int goalZone = map->GetMapValue(sabs_pos, false);

    if (fromZone == goalZone && (fromZone >= map->insideZone))
      if (math::Distance(abs_pos, soldierPos) <= minDistance)
        return prisoners[i]->GetPawn();
  }

  return nullptr;
}

//////////////////////////////////////////////////////////

void SoldierAgent::Init(float x, float y, float maxSpeed, const char *path)
{
  pawn = new AgentPawn(x, y, maxSpeed, path);
  behavior = new SoldierBehavior();
  behavior->AttachPawn(pawn);
  behavior->SetState(kAgentStateRescue);
  behavior->Init();
}

//////////////////////////////////////////////////////////

void PrisonerAgent::Init(float x, float y, float maxSpeed, const char *path, AgentState startState)
{
  pawn = new AgentPawn(x, y, maxSpeed, path);
  behavior = new PrisonerBehavior();
  behavior->AttachPawn(pawn);
  behavior->SetState(startState);
  behavior->Init();
}

int PrisonerAgent::GetEnergy()
{
  return dynamic_cast<PrisonerBehavior*>(behavior)->GetEnergy();
}

void PrisonerAgent::CallOfDuty()
{
  dynamic_cast<PrisonerBehavior*>(behavior)->GoToWork(fvec2(0.0f, 0.0f));
}

//////////////////////////////////////////////////////////

void GuardAgent::Init(float x, float y, float maxSpeed, const char *path)
{
  pawn = new AgentPawn(x, y, maxSpeed, path);
  behavior = new GuardBehavior();
  behavior->AttachPawn(pawn);
  behavior->Init();
}
