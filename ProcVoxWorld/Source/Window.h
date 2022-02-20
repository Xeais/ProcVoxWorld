#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "Framebuffer.h"
#include "Player/Player.h"
#include "Map/Map.h"
#include "UI.h"

typedef struct
{
  GLFWwindow* GLFW;
  Framebuffers* fb;

  int32_t width;
  int32_t height;

  bool isFocused;
  bool showPip;
} Window;

typedef struct
{
  Player* player;
} GameObjectRefs;

extern Window* WND;

typedef void (*OnFramebufferSizeChange)(void* thisObject, int32_t newWidth, int32_t newHeight);
typedef void (*OnKeyboardKeyPress)(void* thisObject, int32_t GLFWKeycode, int32_t GLFWActionCode);
typedef void (*OnMouseButtonKeyPress)(void* thisObject, int32_t GLFWKeycode, int32_t GLFWActionCode);
typedef void (*OnMouseScroll)(void* thisObject, float xOffset, float yOffset);

void RegisterFramebufferSizeChangeCallback(void* thisObject, OnFramebufferSizeChange userCallback);
void RegisterKeyboardKeyPressCallback(void* thisObject, OnKeyboardKeyPress userCallback);
void RegisterMouseButtonKeyPressCallback(void* thisObject, OnMouseButtonKeyPress userCallback);
void RegisterMouseScrollCallback(void* thisObject, OnMouseScroll userCallback);

void WindowInit();

void WindowInitFb();

void WindowPollEvents();

bool WindowIsKeyPressed(int32_t GLFWKeyCode);

void WindowUpdateTitleFPS();

void WindowFree();