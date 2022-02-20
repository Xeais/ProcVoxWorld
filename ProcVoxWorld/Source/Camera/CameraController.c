#include "CameraController.h"

#include "../TimeMeasurement.h"
#include "../Window.h"

#include <assert.h>

static void OnKeyboardKey(void* thisObject, int32_t GLFWKeyCode, int32_t GLFWActionCode)
{
  CameraController* cC = (CameraController*)thisObject;

  if(GLFWKeyCode == GLFW_KEY_C)
  {
    if(GLFWActionCode == GLFW_PRESS && cC->camera->FOV == FOV)
      CameraSetFOV(cC->camera, FOV_ZOOM);
    else if(GLFWActionCode == GLFW_RELEASE && cC->camera->FOV == FOV_ZOOM)
      CameraSetFOV(cC->camera, FOV);
  }
}

CameraController* CameraControllerCreate(Camera* cam)
{
  CameraController* cC = (CameraController*)OwnMalloc(sizeof(CameraController), false);

  if(cam == NULL || cC == NULL) //If no camera was provided or the above "OwnMalloc()" could not deliver, cancel the function.
  {
    LogError("Variables \"cam\" and \"cC\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  cC->camera = cam;
  cC->isControlling = true;
  cC->flySpeed = 20.0f * BLOCK_SIZE;

  memset(&cC->trackedObjectInfo, 0, sizeof(ObjectLocationInfo));
  cC->isTracking = 0;

  cC->updateFunc = NULL;

  RegisterKeyboardKeyPressCallback(cC, OnKeyboardKey);

  return cC;
}

void CameraControllerSetCamera(CameraController* cC, Camera* cam)
{
  if(cam == NULL || cC == NULL)
  {
    LogError("Variables \"cam\" and \"cC\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  cC->camera = cam;
}

void CameraControllerSetTrackObject(CameraController* cC, ObjectLocationInfo* objectInfo)
{
  if(objectInfo != NULL)
    memcpy(&cC->trackedObjectInfo, objectInfo, sizeof(ObjectLocationInfo));
}

void CameraControllerSetTracking(CameraController* cC, bool isTracking)
{
  assert(isTracking && cC->trackedObjectInfo.front), "What am I gonna track?";

  cC->isTracking = isTracking;
}

void CameraControllerSetUpdateFunc(CameraController* cC, cameraUpdateFunc func)
{
  if(cC == NULL || func == NULL)
  {
    LogError("Variables \"cC\" and \"func\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  cC->updateFunc = func;
}

static void UpdateFlySpeed(CameraController* cC)
{
  bool const keyUp = WindowIsKeyPressed(GLFW_KEY_UP);
  bool const keyDown = WindowIsKeyPressed(GLFW_KEY_DOWN);
  float const dt = (float)TimeMeasurementGet();

  if(keyUp)
    cC->flySpeed *= (1.0f + dt);
  else if(keyDown)
    cC->flySpeed /= (1.0f + dt);
}

void CameraControllerDoControl(CameraController* cC)
{
  if(cC == NULL)
  {
    LogError("Variable \"cC\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  glm_vec3_copy(cC->camera->pos, cC->camera->prevPos);
  UpdateFlySpeed(cC);

  if(cC->updateFunc == NULL)
  {
    LogError("Variable \"cC->updateFunc\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  cC->updateFunc(cC);

  CameraUpdateMatrices(cC->camera);
}

void CameraControllerFirstPersonUpdate(void* cC)
{
  CameraController* controller = (CameraController*)cC;

  //Position:
  glm_vec3_copy(controller->trackedObjectInfo.pos, controller->camera->pos);

  //Rotation:
  glm_vec3_copy(controller->trackedObjectInfo.front, controller->camera->front);
  controller->camera->pitch = *controller->trackedObjectInfo.pitch;
  controller->camera->yaw = *controller->trackedObjectInfo.yaw;
}

void CameraControllerDestroy(CameraController* cC)
{
  free(cC);
}