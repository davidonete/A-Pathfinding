#include <ESAT/time.h>
#include <ESAT/window.h>
#include <ESAT/draw.h>
#include <ESAT/input.h>

#include "iostream"
#include <time.h> 

#include "Agent.h"
#include "Player.h"
#include "Map.h"
#include "Pathfinder.h"

enum GameStatus {
	kState_Intro,
	kState_Menu,
	kState_Game
};

GameStatus eGameStatus;

FortAgent fort;

const int num_soldiers = 10;
SoldierAgent soldier[10];
const int num_prisoners = 10;
PrisonerAgent prisoner[10];
const int num_guards = 10;
GuardAgent guards[10];

Pathfinder *pathfinder;
Map *map;

bool bQuitGame = false;
int  iTimeStep = 16;

void Input() 
{  
  if (ESAT::MouseButtonPressed(0))
  {
    float x = ESAT::MousePositionX();
    float y = ESAT::MousePositionY();
    //std::cout << map->GetMapValue(fvec2(x, y), false) << std::endl;

    //fort.ActiveAlarm(30);
  }
  
  if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape))
    bQuitGame = true;
}

bool Init()
{
  srand(time(NULL));
  eGameStatus = kState_Game;
  ESAT::WindowInit(960, 704);
  ESAT::WindowSetMouseVisibility(true);
  /*
  player = new Player(400.0f, 300.0f, 100.0f, "../bin/gfx/agents/Player.png");
  for (int i = 0; i < num_agents; i++)
  {
    float x = rand() % 750 + 50;
    float y = rand() % 550 + 50;
    agent[i].Init(x, y, 50.0f, "../bin/gfx/agents/Enemy.png", "../bin/gfx/agents/Enemy_afraid.png", kAgentStateIdle);
    agent[i].SetTarget(player);
  }
  */

  map = new Map("../bin/gfx/maps/map_03_960x704_layout.bmp", "../bin/gfx/maps/map_03_120x88_bw2.bmp");
  pathfinder = Pathfinder::GetInstance();
  pathfinder->Init(map);
  fort.Init();
  
  for (int i = 0; i < num_soldiers; i++)
  {
    float x = rand() % 56 + 19;
    float y = rand() % 53 + 644;

    float vel = rand() % 30 + 10;

    soldier[i].Init(x, y, vel, "../bin/gfx/agents/Enemy2.png");
    fort.AddAgent(&soldier[i]);
  }

  for (int i = 0; i < num_prisoners; i++)
  {
    float x = rand() % 229 + 544;
    float y = rand() % 61 + 482;
    if (i > 4)
      prisoner[i].Init(x, y, 50.0f, "../bin/gfx/agents/Enemy_afraid2.png", kAgentStateRest);
    else
      prisoner[i].Init(x, y, 50.0f, "../bin/gfx/agents/Enemy_afraid2.png", kAgentStateWork);
    fort.AddAgent(&prisoner[i]);
  }

  std::vector<fvec2> startPoint;
  startPoint.push_back(fvec2(155.0f, 235.0f));
  startPoint.push_back(fvec2(805.0f, 275.0f));
  startPoint.push_back(fvec2(215.0f, 87.0f));
  startPoint.push_back(fvec2(729.0f, 96.0f));
  startPoint.push_back(fvec2(507.0f, 503.0f));
  startPoint.push_back(fvec2(664.0f, 513.0f));
  startPoint.push_back(fvec2(302.0f, 205.0f));
  startPoint.push_back(fvec2(312.0f, 351.0f));

  for (int i = 0; i < num_guards; i++)
  {
    int idx = rand() % startPoint.size();
    guards[i].Init(startPoint[idx].x, startPoint[idx].y, 50.0f, "../bin/gfx/agents/Player2.png");
    fort.AddAgent(&guards[i]);
  }
  
  return true;
}

void Update(int Time)
{
  switch (eGameStatus)
  {
  case kState_Intro:
    break;
  case kState_Menu:
    break;
  case kState_Game:
    map->Update(Time);
    for (int i = 0; i < num_soldiers; i++)
      soldier[i].Update(Time);

    for (int i = 0; i < num_prisoners; i++)
      prisoner[i].Update(Time);

    for (int i = 0; i < num_guards; i++)
      guards[i].Update(Time);
    break;
  }
}

void Render()
{
	ESAT::DrawBegin();
	ESAT::DrawClear(125, 125, 125, 255);
  
  map->Draw();

  for (int i = 0; i < num_soldiers; i++)
    soldier[i].Draw();

  for (int i = 0; i < num_prisoners; i++)
    prisoner[i].Draw();

  for (int i = 0; i < num_guards; i++)
    guards[i].Draw();

	ESAT::DrawEnd();
	ESAT::WindowFrame();
}

void Start() 
{
  int CurrentTime = ESAT::Time();

	while (!bQuitGame)
	{
		Input();
    int accTime = ESAT::Time() - CurrentTime;

		while (accTime >= iTimeStep) 
		{
			Update(iTimeStep);
			CurrentTime += iTimeStep;
      accTime = ESAT::Time() - CurrentTime;
		}
		Render();
	}
}

void Shutdown() 
{
  delete map;
}

int ESAT::main(int argc, char **argv) 
{
	Init();
	Start();
	Shutdown();
	return 0;
}
