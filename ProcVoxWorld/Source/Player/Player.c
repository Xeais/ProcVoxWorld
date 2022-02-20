#include "Player.h"

#include "../Database.h"

#include "../Map/Block.h"
#include "../Map/Map.h"

//Update VAO (Vertex Array Object) as well as VBO (Vertex Buffer Object) to match the current build block.
static void RegenerateItemBuffer(Player* p)
{
  if(p->VAOItem)
  {
    glDeleteBuffers(1, &p->VBOItem);
    glDeleteVertexArrays(1, &p->VAOItem);
  }

  Vertex* vertices = (Vertex*)OwnMalloc(36 * sizeof(Vertex), false);

  if(vertices == NULL)
  {
    LogError("Variable \"vertices\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  int32_t faces[6] = {1, 1, 1, 1, 1, 1};
  float AO[6][4] = {0};
  int32_t currVertexCount = 0;

  if(BlockIsPlant(p->buildBlock))
    GenPlantVertices(vertices, &currVertexCount, 0, 0, 0, p->buildBlock, 1.0f);
  else
    GenCubeVertices(vertices, &currVertexCount, 0, 0, 0, p->buildBlock, 1.0f, 0, faces, AO);

  p->VAOItem = OpenGLCreateVAO();
  p->VBOItem = OpenGLCreateVBO(vertices, currVertexCount * sizeof(Vertex));

  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  OpenGL_VBOLayout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
  OpenGL_VBOLayout(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
  OpenGL_VBOLayout(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float));
  OpenGL_VBOLayout(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float) + 1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /* Newer OpenGL:
   * OpenGL_VBOLayout(p->VAOItem, p->VBOItem, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, sizeof(Vertex));
   * OpenGL_VBOLayout(p->VAOItem, p->VBOItem, 1, 0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(p->VAOItem, p->VBOItem, 2, 0, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(p->VAOItem, p->VBOItem, 3, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(p->VAOItem, p->VBOItem, 4, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float) + 1, sizeof(Vertex));
   *
   * glVertexArrayVertexBuffer(p->VAOItem, 0, p->VBOItem, 0, sizeof(Vertex)); */

  free(vertices);
}

static void RegenerateItemModelMatrix(Player* p)
{
  glm_mat4_identity(p->ModelMatItem);

  if(p->buildBlock == PLAYER_HAND_BLOCK)
  {
    glm_translate(p->ModelMatItem, (vec3){0.3f, -0.3f, 0.34f});
    glm_rotate(p->ModelMatItem, -1.0f, (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(p->ModelMatItem, 0.48f, (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(p->ModelMatItem, -0.18f, (vec3){0.0f, 0.0f, 1.0f});
    glm_scale(p->ModelMatItem, (vec3){0.125f, 0.3f, 0.1f});
  }
  else if(BlockIsPlant(p->buildBlock))
  {
    glm_translate(p->ModelMatItem, (vec3){0.11f, -0.08f, 0.77f});
    glm_rotate(p->ModelMatItem, 0.2f, (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(p->ModelMatItem, 0.5f, (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(p->ModelMatItem, 0.12f, (vec3){0.0f, 0.0f, 1.0f});
    glm_scale(p->ModelMatItem, (vec3){0.1f, 0.1f, 0.1f});
  }
  else
  {
    glm_translate(p->ModelMatItem, (vec3){0.14f, -0.13f, 0.73f});
    //glm_rotate(p->ModelMatItem, 0.0f, (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(p->ModelMatItem, 1.0f, (vec3) {0.0f, 1.0f, 0.0f});
    //glm_rotate(p->ModelMatItem, 0.0f, (vec3){0.0f, 0.0f, 1.0f});
    glm_scale(p->ModelMatItem, (vec3){0.1f, 0.1f, 0.1f});
  }

  /* Matrix multuplication order is reversed in code, so it's the first operation here.
   * At first, translate the current element bY (-0.5, -0.5, -0.5) so that its center point corresponds to the coordinate center point - this enables correct rotation. */
  glm_translate(p->ModelMatItem, (vec3){-0.5f, -0.5f, -0.5f});
}

static void RegenerateItem(Player* p)
{
  RegenerateItemBuffer(p);
  RegenerateItemModelMatrix(p);
}

static void PlayerPutOnGroundLevel(Player* p)
{
  int32_t bX = CHUNK_WIDTH / 2;
  int32_t bZ = CHUNK_WIDTH / 2;
  int32_t bY = MapGetHighestBlock(bX, bZ);

  p->pos[0] = bX * BLOCK_SIZE + BLOCK_SIZE / 2.0f;
  p->pos[1] = bY * BLOCK_SIZE + BLOCK_SIZE * 3.0f;
  p->pos[2] = bZ * BLOCK_SIZE + BLOCK_SIZE / 2.0f;

  PlayerUpdateHitbox(p);
}

Player* PlayerCreate()
{
  Player* p = (Player*)OwnMalloc(sizeof(Player), false);

  if(p == NULL)
  {
    LogError("Variable \"p\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  OwnGLMVec3Set(p->front, 1.0f, 0.0f, 1.0f);
  OwnGLMVec3Set(p->up, 0.0f, 1.0f, 0.0f);
  OwnGLMVec3Set(p->speed, 0.0f, 0.0f, 0.0f);

  p->yaw = 0.0f;
  p->pitch = 0.0f;

  p->buildBlock = PLAYER_HAND_BLOCK;
  p->pointingAtBlock = false;

  OwnGLMIvec3Set(p->blockPointedAt, 0, 0, 0);
  glm_vec3_fill(p->pos, 0.0f);

  bool isPlayerInDb = DatabaseHasPlayerInfo();
  if(isPlayerInDb)
    DatabaseLoadPlayerInfo(p);

  MapForceChunksNearPlayer(p->pos);

  if(!isPlayerInDb)
    PlayerPutOnGroundLevel(p);

  PlayerUpdateHitbox(p);

  p->onGround = false;
  p->inWater = false;
  p->isSneaking = false;
  p->isRunning = false;

  p->VAOItem = 0;
  p->VBOItem = 0;

  RegenerateItem(p);

  return p;
}

static void UpdateBlockPointingAt(Player* p)
{
  float camX = Blocked(p->pos[0]);
  float camY = Blocked(p->pos[1]);
  float camZ = Blocked(p->pos[2]);

  int32_t iCamX = (int32_t)camX;
  int32_t iCamY = (int32_t)camY;
  int32_t iCamZ = (int32_t)camZ;

  bool found = false;
  int32_t bestX = 0, bestY = 0, bestZ = 0;
  float bestDist = BLOCK_BREAK_RADIUS_SQUARED * 2.0f;

  //Iterate over each block around the player.
  for(int32_t y = iCamY - BLOCK_BREAK_RADIUS; y <= iCamY + BLOCK_BREAK_RADIUS; ++y)
  {
    if(y < 0 || y >= CHUNK_HEIGHT) 
      continue;

    for(int32_t x = iCamX - BLOCK_BREAK_RADIUS; x <= iCamX + BLOCK_BREAK_RADIUS; ++x)
    {
      for(int32_t z = iCamZ - BLOCK_BREAK_RADIUS; z <= iCamZ + BLOCK_BREAK_RADIUS; ++z)
      {
        if(BlockPlayerDistSquared(x, y, z, camX, camY, camZ) > BLOCK_BREAK_RADIUS_SQUARED)
          continue;

        uint8_t block = MapGetBlock(x, y, z);
        if(!BlockIsSolid(block))
          continue;

        if(BlockRayIntersection(p->pos, p->front, x, y, z, block))
        {
          float distance = BlockPlayerDistSquared(x, y, z, camX, camY, camZ);
          if(distance < bestDist)
          {
            bestDist = distance;
            bestX = x;
            bestY = y;
            bestZ = z;
            found = true;
          }
        }
      }
    }
  }

  if(!found)
  {
    p->pointingAtBlock = false;

    return;
  }

  p->pointingAtBlock = true;
  OwnGLMIvec3Set(p->blockPointedAt, bestX, bestY, bestZ);
}

void PlayerSetBuildBlock(Player* p, int32_t newBlock)
{
  //Skip air block:
  if(newBlock == AIR_BLOCK)
  {
    if(newBlock < p->buildBlock)
      --newBlock;
    else
      ++newBlock;
  }

  newBlock %= AMOUNT_BLOCKS;
  if(newBlock < 0)
    newBlock += AMOUNT_BLOCKS;

  if(newBlock == AIR_BLOCK)
    ++newBlock;

  p->buildBlock = (uint8_t)newBlock;

  RegenerateItem(p);
}

static void CheckIfInWater(Player* p)
{
  int32_t playerX = (int32_t)Blocked(p->pos[0]);
  int32_t playerY = (int32_t)Blocked(p->pos[1]);
  int32_t playerZ = (int32_t)Blocked(p->pos[2]);

  for(int32_t y = -1; y <= 1; ++y)
  {
    int32_t bY = playerY + y;

    if(bY < 0 || bY >= CHUNK_HEIGHT)
      continue;

    for(int32_t x = -1; x <= 1; ++x)
    {
      for(int32_t z = -1; z <= 1; ++z)
      {
        int32_t bX = playerX + x;
        int32_t bZ = playerZ + z;

        uint8_t block = MapGetBlock(bX, bY, bZ);
        if(block != WATER_BLOCK)
          continue;

        vec3 blockHitbox[2];
        BlockGenAABB(bX, bY, bZ, blockHitbox);

        if(AABBCollide(p->hitbox, blockHitbox))
        {
          p->inWater = true;

          return;
        }
      }
    }
  }

  p->inWater = false;
}

void PlayerUpdateHitbox(Player* p)
{
  p->hitbox[0][0] = p->pos[0] - BLOCK_SIZE * 0.3f;
  p->hitbox[0][1] = p->pos[1] - BLOCK_SIZE * 1.625f;
  p->hitbox[0][2] = p->pos[2] - BLOCK_SIZE * 0.3f;
  p->hitbox[1][0] = p->pos[0] + BLOCK_SIZE * 0.3f;
  p->hitbox[1][1] = p->pos[1] + BLOCK_SIZE * 0.175f;
  p->hitbox[1][2] = p->pos[2] + BLOCK_SIZE * 0.3f;
}

void PlayerSetViewDir(Player* p, float pitch, float yaw)
{
  p->pitch = pitch;
  p->yaw = yaw;

  p->front[0] = cosf(glm_rad(yaw)) * cosf(glm_rad(pitch));
  p->front[1] = sinf(glm_rad(pitch));
  p->front[2] = sinf(glm_rad(yaw)) * cosf(glm_rad(pitch));

  glm_vec3_normalize(p->front);
}

void PlayerUpdate(Player* p)
{
  CheckIfInWater(p);
  UpdateBlockPointingAt(p);
}

void PlayerRenderItem(Player* p)
{
  return; //This is not used at the moment.

  /* Render an item using an additional camera, which is created here.
   * The camera's position is (0, 0, -1) and it looks at (0, 0, 0). */

  /*mat4 view, projection;
  glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);
  glm_perspective(glm_rad(50.0f), p->cam->aspectRatio, 0.01f, 2.0f, projection);

  mat4 MVP;
  glm_mat4_mulN((mat4* []){&projection, &view, &p->modelMatItem}, 3, MVP);

  glUseProgram(SHADER_HAND_ITEM);
  ShaderSetMat4(SHADER_HAND_ITEM, "MVPMatrix", MVP);
  ShaderSetTextureArray(SHADER_HAND_ITEM, "textureSampler", TEXTURE_BLOCKS, 0);
  ShaderSetFloat1(SHADER_HAND_ITEM, "blockLight", MapGetBlocksLight());

  glDisable(GL_BLEND);
  glBindVertexArray(p->VAOItem);
  glDrawArrays(GL_TRIANGLES, 0, 36);*/

  p; //A reference to resolve "C4100".
}

void PlayerSave(Player* p)
{
  DatabaseSavePlayerInfo(p);
}

void PlayerDestroy(Player* p)
{
  PlayerSave(p);
  free(p);
}