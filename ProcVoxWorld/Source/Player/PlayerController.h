#pragma once

#include "Player.h"

typedef struct
{
  Player* player;

  bool isFirstFrame;
  bool isFlyMode;
  bool isControlling;

  double lastMouseX;
  double lastMouseY;
  float mouseSens;
  float flySpeed;
} PlayerController;

PlayerController* PlayerControllerCreate(Player* p);

void PlayerControllerDoControl(PlayerController* pC);

void PlayerControllerDestroy(PlayerController* pC);