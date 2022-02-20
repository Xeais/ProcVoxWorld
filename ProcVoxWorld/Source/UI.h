#pragma once

#include "Camera/Camera.h"
#include "Player/Player.h"

void UIInit(float aspectRatio);

void UIUpdateAspectRatio(float newRatio);

void UIRenderCrosshairs();

void UIRenderBlockWireframe(Player* p, Camera* cam);

void UIFree();