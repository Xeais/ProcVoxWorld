#include "PlayerPhysics.h"

#include "../Map/Block.h"
#include "../Map/Map.h"

#define FOREACH_SOLID_BLOCK_AROUND(PX, PY, PZ)         \
for(int32_t y = -3; y <= 3; ++y)                       \
{                                                      \
  int32_t bY = PY + y;                                 \
  if(bY < 0 || bY >= CHUNK_HEIGHT)                     \
    continue;                                          \
                                                       \
  for(int32_t x = -3; x <= 3; ++x)                     \
  {                                                    \
    for(int32_t z = -3; z <= 3; ++z)                   \
    {                                                  \
      int32_t bX = PX + x;                             \
      int32_t bZ = PZ + z;                             \
                                                       \
      uint8_t block = MapGetBlock(bX, bY, bZ);         \
      if(!BlockIsSolid(block) || BlockIsPlant(block))  \
        continue;                                      \
                                                       \
      vec3 blockHitbox[2];                             \
      BlockGenAABB(bX, bY, bZ, blockHitbox);

#define FOREACH_SOLID_BLOCK_AROUND_END() }}}

static void CollisionX(Player* p, vec3 blockHitbox[2])
{
  const bool movingToPlusX = (p->speed[0] >= 0);
  if(movingToPlusX)
    p->pos[0] -= (p->hitbox[1][0] - blockHitbox[0][0]) + 0.00001f;
  else
    p->pos[0] += (blockHitbox[1][0] - p->hitbox[0][0]) + 0.00001f;

  p->speed[0] = 0.0f;
}

static void CollisionY(Player* p, vec3 blockHitbox[2])
{
  const bool movingToPlusY = (p->speed[1] >= 0);
  if(movingToPlusY)
    p->pos[1] -= (p->hitbox[1][1] - blockHitbox[0][1]) + 0.00001f;
  else
  {
    p->pos[1] += (blockHitbox[1][1] - p->hitbox[0][1]) + 0.00001f;
    p->onGround = true;
  }

  p->speed[1] = 0.0f;
}

static void CollisionZ(Player* p, vec3 blockHitbox[2])
{
  const bool movingToPlusZ = (p->speed[2] >= 0);
  if(movingToPlusZ)
    p->pos[2] -= (p->hitbox[1][2] - blockHitbox[0][2]) + 0.00001f;
  else
    p->pos[2] += (blockHitbox[1][2] - p->hitbox[0][2]) + 0.00001f;

  p->speed[2] = 0.0f;
}

static bool CollideOneAxis(void (*CollisionHandler)(Player*, vec3 blockHitbox[2]), Player* p)
{
  int32_t playerX = (int32_t)Blocked(p->pos[0]);
  int32_t playerY = (int32_t)Blocked(p->pos[1]);
  int32_t playerZ = (int32_t)Blocked(p->pos[2]);

  PlayerUpdateHitbox(p);

  FOREACH_SOLID_BLOCK_AROUND(playerX, playerY, playerZ)
    if(AABBCollide(p->hitbox, blockHitbox))
    {
      CollisionHandler(p, blockHitbox);
      PlayerUpdateHitbox(p);

      return 1;
    }
  FOREACH_SOLID_BLOCK_AROUND_END()

  return 0;
}

static void CollideAllAxes(Player* p, vec3 motion, ivec3 doCollide)
{
  if(doCollide[1])
  {
    p->onGround = false; //Ground check comes first!
    p->pos[1] += motion[1];
    doCollide[1] = !CollideOneAxis(CollisionY, p);
  }

  if(doCollide[0])
  {
    p->pos[0] += motion[0];
    doCollide[0] = !CollideOneAxis(CollisionX, p);
  }

  if(doCollide[2])
  {
    p->pos[2] += motion[2];
    doCollide[2] = !CollideOneAxis(CollisionZ, p);
  }
}

void PlayerPhysicsCollideWithMap(Player* p, vec3 motion)
{
  const float maxStepSize = BLOCK_SIZE * 0.25f;

  float magnitude = glm_vec3_norm(motion);
  if(magnitude < 0.00001f)
    return;

  int32_t numSteps = 1 + (int32_t)(magnitude / maxStepSize);
  float stepSize = magnitude / numSteps;

  vec3 stepMotion;
  glm_vec3_copy(motion, stepMotion);
  glm_vec3_scale(stepMotion, (1.0f / magnitude) * stepSize, stepMotion);

  ivec3 doCollide;
  OwnGLMIvec3Set(doCollide, 1, 1, 1);

  for(int32_t i = 0; i < numSteps; ++i)
    CollideAllAxes(p, stepMotion, doCollide);
}