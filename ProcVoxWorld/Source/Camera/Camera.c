#include "Camera.h"

#include "../Configuration.h"

#include "../Window.h"
#include "../Map/Block.h"

#include <stdlib.h>
#include <math.h>

static void CameraFramebufferSizeChangeCallback(void* thisObject, int32_t newWidth, int32_t newHeight)
{
  Camera* cam = (Camera*)thisObject;
  CameraSetAspectRatio(cam, newWidth / (float)newHeight);
}

Camera* CameraCreate(vec3 pos, float pitch, float yaw, vec3 front)
{
  Camera* cam = (Camera*)OwnMalloc(sizeof(Camera), false);
  RegisterFramebufferSizeChangeCallback(cam, CameraFramebufferSizeChangeCallback);

  if(cam == NULL) //If the above "OwnMalloc()" failed - quite unlikely, abort.
  {
    LogError("Variable \"cam\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  glm_vec3_copy(pos, cam->pos);
  glm_vec3_copy(pos, cam->prevPos);
  //glm_vec3_fill(cam->speed, 0.0f);

  glm_vec3_copy(front, cam->front);
  OwnGLMVec3Set(cam->up, 0.0f, 1.0f, 0.0f);

  cam->pitch = pitch;
  cam->yaw = yaw;

  cam->FOV = FOV;
  cam->sens = MOUSE_SENS;

  cam->clipNear = BLOCK_SIZE / 10.0f;
  cam->clipFar = MAX((CHUNK_RENDER_RADIUS * 1.2f) * CHUNK_SIZE, 512 * BLOCK_SIZE);
  cam->aspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;

  glm_look(cam->pos, cam->front, cam->up, cam->viewMatrix);
  /* A "float" only has 24-bits of precision - an "int" on the other hand, possesses 32-bits. Therefore, a "float" can map integer values from zero to 16'777'215;
   * greater numbers than 16'777'215 do not necessarily have exact "float"-representations. More information: https://stackoverflow.com/a/23423240 
   * Since "FOV" (field of view) will never come close to this very high upper limit, this narrowing conversion (from "int32_t" to "float) poses absolutely no problem. */
  glm_perspective(glm_rad((float)cam->FOV), WINDOW_WIDTH / (float)WINDOW_HEIGHT, cam->clipNear, cam->clipFar, cam->projMatrix);
  glm_mat4_mul(cam->projMatrix, cam->viewMatrix, cam->viewProjMatrix);
  glm_mat4_copy(cam->viewMatrix, cam->prevViewMatrix);

  return cam;
}

void CameraUpdateMatrices(Camera* cam)
{
  glm_mat4_copy(cam->viewMatrix, cam->prevViewMatrix);
  glm_look(cam->pos, cam->front, cam->up, cam->viewMatrix);
  glm_mat4_mul(cam->projMatrix, cam->viewMatrix, cam->viewProjMatrix);

  glm_frustum_planes(cam->viewProjMatrix, cam->frustumPlanes);
}

void CameraSetAspectRatio(Camera* cam, float newRatio)
{
  cam->aspectRatio = newRatio;
  glm_perspective_resize(newRatio, cam->projMatrix);
  glm_frustum_planes(cam->viewProjMatrix, cam->frustumPlanes);
}

void CameraSetFOV(Camera* cam, int32_t newFOV)
{
  cam->FOV = newFOV;
  glm_perspective(glm_rad((float)newFOV), cam->aspectRatio, cam->clipNear, cam->clipFar, cam->projMatrix);
}