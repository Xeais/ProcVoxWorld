#include "PlayerController.h"
#include "PlayerPhysics.h"

#include "../TimeMeasurement.h"
#include "../Window.h"

#include "../Map/Block.h"

#include <assert.h>

static void OnLeftMouseButton(PlayerController* pC)
{
  if(!pC->player->pointingAtBlock)
    return;

  Player* p = pC->player;

  int32_t x = p->blockPointedAt[0];
  int32_t y = p->blockPointedAt[1];
  int32_t z = p->blockPointedAt[2];

  MapSetBlock(x, y, z, AIR_BLOCK);

  p->pointingAtBlock = false;
}

static void FindBestSpotToPlaceBlock(Player* player, int32_t x, int32_t y, int32_t z, int32_t* bestX, int32_t* bestY, int32_t* bestZ, float* bestDist)
{
  float playerX = Blocked(player->pos[0]);
  float playerY = Blocked(player->pos[1]);
  float playerZ = Blocked(player->pos[2]);

  if(BlockRayIntersection(player->pos, player->front, x, y, z, AIR_BLOCK) && !BlockIsSolid(MapGetBlock(x, y, z)))
  {
    float dist = BlockPlayerDistSquared(x, y, z, playerX, playerY, playerZ);
    if(dist < *bestDist)
    {
      *bestDist = dist;
      *bestX = x;
      *bestY = y;
      *bestZ = z;
    }
  }
}

static void OnRightMouseButton(PlayerController* pC)
{
  Player* p = pC->player;

  if(!p->pointingAtBlock || p->buildBlock == PLAYER_HAND_BLOCK)
    return;

  int32_t x = p->blockPointedAt[0];
  int32_t y = p->blockPointedAt[1];
  int32_t z = p->blockPointedAt[2];

  int32_t bestX = 0, bestY = 0, bestZ = 0;
  float bestDist = BLOCK_BREAK_RADIUS_SQUARED * 2.0f;

  //Six potential spots around the active block:
  FindBestSpotToPlaceBlock(p, x - 1, y, z, &bestX, &bestY, &bestZ, &bestDist);
  FindBestSpotToPlaceBlock(p, x + 1, y, z, &bestX, &bestY, &bestZ, &bestDist);
  FindBestSpotToPlaceBlock(p, x, y - 1, z, &bestX, &bestY, &bestZ, &bestDist);
  FindBestSpotToPlaceBlock(p, x, y + 1, z, &bestX, &bestY, &bestZ, &bestDist);
  FindBestSpotToPlaceBlock(p, x, y, z - 1, &bestX, &bestY, &bestZ, &bestDist);
  FindBestSpotToPlaceBlock(p, x, y, z + 1, &bestX, &bestY, &bestZ, &bestDist);

  if((bestDist > BLOCK_BREAK_RADIUS_SQUARED) || bestY < 0 || bestY >= CHUNK_HEIGHT)
    return;

  vec3 blockHitbox[2];
  BlockGenAABB(bestX, bestY, bestZ, blockHitbox);
  if(AABBCollide(p->hitbox, blockHitbox))
    return;

  MapSetBlock(bestX, bestY, bestZ, p->buildBlock);
}

static void PlayerControllerOnMouseButtonCallback(void* thisObject, int32_t GLFWKeycode, int32_t GLFWActionCode)
{
  if(GLFWActionCode != GLFW_PRESS)
    return;

  PlayerController* pC = (PlayerController*)thisObject;

  if(!pC->isControlling)
  {
    pC->isControlling = true;
    pC->isFirstFrame = true;

    return;
  }

  if(GLFWKeycode == GLFW_MOUSE_BUTTON_LEFT)
    OnLeftMouseButton(pC);

  if(GLFWKeycode == GLFW_MOUSE_BUTTON_RIGHT)
    OnRightMouseButton(pC);
}

static void UpdateDir(PlayerController* pC)
{
  if(!pC->isControlling)
    return;

  double mouseX, mouseY;
  glfwGetCursorPos(WND->GLFW, &mouseX, &mouseY);

  if(pC->isFirstFrame)
  {
    pC->isFirstFrame = false;
    pC->lastMouseX = mouseX;
    pC->lastMouseY = mouseY;

    return;
  }

  float dX = (float)(mouseX - pC->lastMouseX); //The loss of precision (GLFW provides mouse data only as "double") should not be a problem. The same is true for time measurement (dt).
  float dY = (float)(mouseY - pC->lastMouseY);

  pC->lastMouseX = mouseX;
  pC->lastMouseY = mouseY;

  float newYaw = pC->player->yaw + dX * pC->mouseSens;
  float newPitch = pC->player->pitch - dY * pC->mouseSens;

  const float maxPitch = 89.99f;
  newPitch = glm_clamp(newPitch, -maxPitch, maxPitch);

  const float maxYaw = 360.0f;
  newYaw = LoopBetween(newYaw, 0.0f, maxYaw);

  PlayerSetViewDir(pC->player, newPitch, newYaw);
}

static void PlayerControllerOnMouseScrollCallback(void* thisObject, float xOffset, float yOffset)
{
  PlayerController* pC = (PlayerController*)thisObject;

  if(!pC->isControlling)
    return;

  if(yOffset > 0.0f)
    PlayerSetBuildBlock(pC->player, pC->player->buildBlock + 1);
  else
    PlayerSetBuildBlock(pC->player, pC->player->buildBlock - 1);

  xOffset; //A reference to resolve "C4100".
}

static void PlayerControllerOnKeyboardKeyCallback(void* thisObject, int32_t GLFWKeycode, int32_t GLFWActionCode)
{
  if(GLFWActionCode != GLFW_PRESS)
    return;

  PlayerController* pC = (PlayerController*)thisObject;

  switch(GLFWKeycode)
  {
    case GLFW_KEY_ESCAPE:
      pC->isControlling = false;
      break;
    case GLFW_KEY_TAB:
      pC->isFlyMode = !pC->isFlyMode;
      break;
  }
}

static void Decelerate(Player* p, vec3 frameAccel)
{
  vec3 accel;

  glm_vec3_copy(p->speed, accel);
  accel[1] = 0.0f;
  glm_vec3_normalize(accel);
  glm_vec3_scale(accel, -1.0f, accel);
  glm_vec3_scale(accel, DEC_HORIZONTAL, accel);

  glm_vec3_add(frameAccel, accel, frameAccel);
}

static bool PlayerControllerIsKeyPressed(PlayerController* pC, int32_t GLFWKeycode)
{
  return pC->isControlling && WindowIsKeyPressed(GLFWKeycode);
}

static void AccelerateWASD(PlayerController* pC, vec3 frameAccel)
{
  Player* p = pC->player;

  const bool keyW = PlayerControllerIsKeyPressed(pC, GLFW_KEY_W);
  const bool keyA = PlayerControllerIsKeyPressed(pC, GLFW_KEY_A);
  const bool keyS = PlayerControllerIsKeyPressed(pC, GLFW_KEY_S);
  const bool keyD = PlayerControllerIsKeyPressed(pC, GLFW_KEY_D);

  //Generate front as well as right vectors.
  vec3 front = {0}, right;
  front[0] = cosf(glm_rad(p->yaw));
  front[1] = 0.0f;
  front[2] = sinf(glm_rad(p->yaw));
  glm_vec3_normalize(front);
  glm_vec3_crossn(front, p->up, right);

  vec3 accel = {0.0f};

  if(keyW || keyS)
  {
    vec3 a = {front[0], 0.0f, front[2]};
    if(keyS) 
      glm_vec3_negate(a);

    glm_vec3_add(accel, a, accel);
  }

  if(keyA || keyD)
  {
    vec3 a = {right[0], 0.0f, right[2]};
    if(keyA) 
      glm_vec3_negate(a);

    glm_vec3_add(accel, a, accel);
  }

  float scale = ACC_HORIZONTAL;
  if(!p->onGround && !p->inWater)
    scale = ACC_HORIZONTAL / 6.0f;
  glm_vec3_scale(accel, scale, accel);

  glm_vec3_add(frameAccel, accel, frameAccel);
}

static void GenMotionVectorWalk(PlayerController* pC, double dt, vec3 res)
{
  Player* p = pC->player;

  const bool keyW = PlayerControllerIsKeyPressed(pC, GLFW_KEY_W);
  const bool keyA = PlayerControllerIsKeyPressed(pC, GLFW_KEY_A);
  const bool keyS = PlayerControllerIsKeyPressed(pC, GLFW_KEY_S);
  const bool keyD = PlayerControllerIsKeyPressed(pC, GLFW_KEY_D);

  const bool keySpace = PlayerControllerIsKeyPressed(pC, GLFW_KEY_SPACE);
  const bool keyShift = PlayerControllerIsKeyPressed(pC, GLFW_KEY_LEFT_SHIFT);
  const bool keyCtrl = PlayerControllerIsKeyPressed(pC, GLFW_KEY_LEFT_CONTROL);

  vec3 frameAccel = {0.0f};
  vec3 frameSpeed = {0.0f};
  static bool doneDeceleratingSneak = false;
  static bool doneDeceleratingRun = false;
  bool deceleratedNoKeys = false;

  p->isRunning = keyShift;
  p->isSneaking = keyCtrl;

  if(p->isSneaking && p->isRunning)
    p->isRunning = false;

  float xZSpeed = glm_vec2_norm((vec2) {p->speed[0], p->speed[2]});
  if(p->isSneaking && xZSpeed > MAX_SNEAK_SPEED && !doneDeceleratingSneak && p->onGround)
    Decelerate(p, frameAccel);
  else if(!p->isRunning && xZSpeed > MAX_MOVE_SPEED && !doneDeceleratingRun && p->onGround)
    Decelerate(p, frameAccel);
  else if(keyW || keyS || keyA || keyD)
    AccelerateWASD(pC, frameAccel);
  else if(p->onGround || p->inWater)
  {
    Decelerate(p, frameAccel);
    deceleratedNoKeys = true;
  }

  //Sneak as well as run logic:
  if(!doneDeceleratingSneak && xZSpeed <= MAX_SNEAK_SPEED)
    doneDeceleratingSneak = true;
  else if(doneDeceleratingSneak && !keyShift)
    doneDeceleratingSneak = false;

  if(!doneDeceleratingRun && xZSpeed <= MAX_MOVE_SPEED)
    doneDeceleratingRun = true;
  else if(doneDeceleratingRun && p->isRunning)
    doneDeceleratingRun = false;

  //Gravity plus jumps:
  if(p->inWater)
    frameAccel[1] -= GRAVITY_WATER;
  else
    frameAccel[1] -= GRAVITY;

  if(keySpace)
  {
    if(p->onGround)
      frameSpeed[1] = JUMP_POWER;
    else if(p->inWater)
      frameAccel[1] += ACC_WATER_EMERGE;
  }

  //Calculate frame speed and add it to camera's speed.
  vec3 speedFromAccel;
  glm_vec3_copy(frameAccel, speedFromAccel);
  glm_vec3_scale(speedFromAccel, (float)dt, speedFromAccel);
  glm_vec3_add(speedFromAccel, frameSpeed, frameSpeed);

  vec3 oldCamSpeed;
  glm_vec3_copy(p->speed, oldCamSpeed);
  glm_vec3_add(frameSpeed, p->speed, p->speed);

  //Stop the player completely if he was decelerating and a change of direction by almost 180 degrees has arisen.
  float angle = glm_vec3_angle((vec3){p->speed[0], 0.0f, p->speed[2]}, (vec3){oldCamSpeed[0], 0.0f, oldCamSpeed[2]});

  const float margin = GLM_PIf / 16.0f;
  if(angle > GLM_PIf - margin && angle < GLM_PIf + margin && deceleratedNoKeys)
    glm_vec3_zero(p->speed);

  //Clamp horizontal speed:
  float maxXZSpeed;
  if(p->inWater)
    maxXZSpeed = MAX_SWIM_SPEED;
  else if(p->isRunning || !doneDeceleratingRun)
    maxXZSpeed = MAX_RUN_SPEED;
  else if(p->isSneaking && doneDeceleratingSneak)
    maxXZSpeed = MAX_SNEAK_SPEED;
  else
    maxXZSpeed = MAX_MOVE_SPEED;

  xZSpeed = glm_vec2_norm((vec2){p->speed[0], p->speed[2]});
  if(xZSpeed > maxXZSpeed)
  {
    const float s = maxXZSpeed / xZSpeed;
    p->speed[0] *= s;
    p->speed[2] *= s;
  }

  //Clamp vertical speed:
  if(p->inWater)
  {
    if(p->speed[1] > MAX_EMERGE_SPEED)
      p->speed[1] = MAX_EMERGE_SPEED;
    else if(p->speed[1] < -MAX_DIVE_SPEED)
      p->speed[1] = -MAX_DIVE_SPEED;
  }
  else
  {
    if(p->speed[1] < -MAX_FALL_SPEED)
      p->speed[1] = -MAX_FALL_SPEED;
  }

  //Calculate frame motion and add it to camera's position.
  vec3 frameMotion;
  glm_vec3_copy(p->speed, frameMotion);
  glm_vec3_scale(frameMotion, (float)dt, frameMotion);
  glm_vec3_copy(frameMotion, res);
}

static void GenMotionVectorFly(PlayerController* pC, double dt, vec3 res)
{
  Player* p = pC->player;

  const bool keyW = PlayerControllerIsKeyPressed(pC, GLFW_KEY_W);
  const bool keyA = PlayerControllerIsKeyPressed(pC, GLFW_KEY_A);
  const bool keyS = PlayerControllerIsKeyPressed(pC, GLFW_KEY_S);
  const bool keyD = PlayerControllerIsKeyPressed(pC, GLFW_KEY_D);

  const bool keyShift = PlayerControllerIsKeyPressed(pC, GLFW_KEY_LEFT_SHIFT);
  const bool keyCtrl = PlayerControllerIsKeyPressed(pC, GLFW_KEY_LEFT_CONTROL);

  vec3 front, right, up;

  //Generate front, right and up vectors.
  glm_vec3_copy(p->front, front);
  glm_vec3_crossn(front, p->up, right);
  glm_vec3_copy(p->up, up);

  vec3 totalMove = {0.0f};

  if(keyW)
    glm_vec3_add(totalMove, front, totalMove);
  if(keyS)
    glm_vec3_sub(totalMove, front, totalMove);

  if(keyD)
    glm_vec3_add(totalMove, right, totalMove);
  if(keyA)
    glm_vec3_sub(totalMove, right, totalMove);

  if(keyShift)
    glm_vec3_add(totalMove, up, totalMove);
  if(keyCtrl)
    glm_vec3_sub(totalMove, up, totalMove);

  glm_vec3_copy(totalMove, p->speed);
  glm_vec3_copy(totalMove, res);

  glm_vec3_scale(res, (float)(dt * pC->flySpeed), res);
}

static void GenFrameMotion(PlayerController* pC, vec3 out)
{
  if(pC->isFlyMode)
    GenMotionVectorFly(pC, TimeMeasurementGet(), out);
  else
    GenMotionVectorWalk(pC, TimeMeasurementGet(), out);
}

PlayerController* PlayerControllerCreate(Player* p)
{
  assert(p);

  PlayerController* pC = (PlayerController*)OwnMalloc(sizeof(PlayerController), false);

  if(pC == NULL)
  {
    LogError("Variable \"pC\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  pC->player = p;
  pC->isFirstFrame = true;
  pC->isFlyMode = false;
  pC->isControlling = true;
  pC->lastMouseX = 0.0;
  pC->lastMouseY = 0.0;
  pC->mouseSens = 0.1f;
  pC->flySpeed = 20.0f * BLOCK_SIZE;

  RegisterMouseButtonKeyPressCallback(pC, PlayerControllerOnMouseButtonCallback);
  RegisterMouseScrollCallback(pC, PlayerControllerOnMouseScrollCallback);
  RegisterKeyboardKeyPressCallback(pC, PlayerControllerOnKeyboardKeyCallback);

  return pC;
}

void PlayerControllerDoControl(PlayerController* pC)
{
  UpdateDir(pC);

  vec3 frameMotion;
  GenFrameMotion(pC, frameMotion);

  if(pC->isFlyMode)
    glm_vec3_add(pC->player->pos, frameMotion, pC->player->pos);
  else
    PlayerPhysicsCollideWithMap(pC->player, frameMotion);

  //LogInfo("-> View Direction <-\nplayer->front[0]: %.3f, player->front[1]: %.3f, player->front[2]: %.3f", true, pC->player->front[0], pC->player->front[1], pC->player->front[2]);
}

void PlayerControllerDestroy(PlayerController* pC)
{
  free(pC);
}