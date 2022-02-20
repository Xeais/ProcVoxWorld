#pragma once

#include "cglm/cglm.h"

typedef struct
{
  vec3 pos;
  vec3 prevPos;

  vec3 front;
  vec3 up;

  double pitch;
  double yaw;

  int32_t FOV;
  float sens;

  float clipNear;
  float clipFar;
  float aspectRatio;

#pragma warning(suppress: 4324) //-> "‘structure was padded due to alignment specifier’ VS warning": https://stackoverflow.com/a/67458658
  vec4 frustumPlanes[6];
  mat4 viewMatrix;
  mat4 projMatrix;
  mat4 viewProjMatrix;
  mat4 prevViewMatrix;
} Camera;

Camera* CameraCreate(vec3 pos, float pitch, float yaw, vec3 front);

/* Not used at the moment:
 * void CameraUpdateViewDir(Camera* cam);
 *
 * void CameraUpdateParameters(Camera* cam, float dt); */

void CameraUpdateMatrices(Camera* cam);

void CameraSetAspectRatio(Camera* cam, float newRatio);

void CameraSetFOV(Camera* cam, int32_t newFOV);