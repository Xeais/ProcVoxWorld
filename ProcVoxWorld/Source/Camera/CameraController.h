#pragma once

#include "Camera.h"

typedef struct
{
  float* pos;
  float* front;
  float* up;

  float* pitch;
  float* yaw;
} ObjectLocationInfo;

typedef void (*cameraUpdateFunc)(void* cC);

typedef struct
{
  Camera* camera;
  bool isControlling;
  float flySpeed;

  ObjectLocationInfo trackedObjectInfo;
  bool isTracking;

  cameraUpdateFunc updateFunc;
} CameraController;

CameraController* CameraControllerCreate(Camera* cam);

void CameraControllerSetCamera(CameraController* cC, Camera* cam);

void CameraControllerSetTrackObject(CameraController* cC, ObjectLocationInfo* objectInfo);

void CameraControllerSetTracking(CameraController* cC, bool isTracking);

void CameraControllerSetUpdateFunc(CameraController* cC, cameraUpdateFunc func);

void CameraControllerDoControl(CameraController* cC);

void CameraControllerFirstPersonUpdate(void* cC);

void CameraControllerDestroy(CameraController* cC);