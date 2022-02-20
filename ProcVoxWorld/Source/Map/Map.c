#include "Map.h"
#include "Block.h"
#include "ThreadWorker.h"

#include "../Database.h"
#include "../Window.h"

#include "../Shader.h"
#include "../Texture.h"

LINKED_LIST_IMPLEMENTATION(Chunk*, Chunks);
#pragma warning(suppress: 6001) //Using uninitialized memory "* list" (C6001) which I really could not find.
HASH_MAP_IMPLEMENTATION(Chunk*, Chunks, ChunkHashFunc);

//Macros that simplify the iteration over chunks.
#define MAP_FOREACH_ACTIVE_CHUNK_BEGIN(CHUNK_NAME)                    \
for(uint32_t i = 0; i < map->chunksActive->arraySize; ++i)            \
{                                                                     \
  LinkedListNodeMapChunks* node = map->chunksActive->array[i]->head;  \
  for(; node; node = node->ptrNext)                                   \
  {                                                                   \
    Chunk* CHUNK_NAME = node->data;

#define MAP_FOREACH_ACTIVE_CHUNK_END() }}

#define LIST_FOREACH_CHUNK_BEGIN(LIST, CHUNK_NAME)  \
{LinkedListNodeChunks* node = LIST->head;           \
for(; node; node = node->ptrNext)                   \
{                                                   \
  Chunk* c = node->data;

#define LIST_FOREACH_CHUNK_END() }}

typedef struct
{
  HashMapChunks* chunksActive;
  LinkedListChunks* chunksToRender;

  GLuint VAOSkybox;
  GLuint VBOSkybox;

  GLuint VAOSunMoon;
  GLuint VBOSunMoon;

  Worker* workers;
  int32_t numWorkers;

  int32_t seed;
} Map;

static Map* map; //Keep static object for simplicity.

//Returns "NULL" if chunk is not near.
static Chunk* MapGetChunk(int32_t chunkX, int32_t chunkZ)
{
  uint32_t index = ChunkHashFunc2(chunkX, chunkZ) % map->chunksActive->arraySize;
  LinkedListMapChunks* bucket = map->chunksActive->array[index];

  LinkedListNodeMapChunks* node = bucket->head;

  while(node && !(node->data->x == chunkX && node->data->z == chunkZ))
    node = node->ptrNext;

  return node ? node->data : NULL;
}

static void MapDeleteChunk(int32_t chunkX, int32_t chunkZ)
{
  Chunk* c = MapGetChunk(chunkX, chunkZ);
  if(c != NULL)
  {
    HashMapChunksRemove(map->chunksActive, c);
    ChunkDelete(c);

    //Chunks only keep separate copies of their neighbours.
  }
}

static void TryToDeleteFarChunks(vec3 currPos)
{
  int32_t playerCx = ChunkedCam(currPos[0]);
  int32_t playerCz = ChunkedCam(currPos[2]);

  LinkedListChunks* chunksToDelete = LinkedListChunksCreate();

  MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
  {
    //Worker thread could be processing this chunk.
    if(!c->isSafeToModify)
      continue;

    if(ChunkPlayerDistSquared(c->x, c->z, playerCx, playerCz) > CHUNK_UNLOAD_RADIUS_SQUARED)
      LinkedListChunksPushFront(chunksToDelete, c);
  }
  MAP_FOREACH_ACTIVE_CHUNK_END()

  LIST_FOREACH_CHUNK_BEGIN(chunksToDelete, c)
    MapDeleteChunk(c->x, c->z);
  LIST_FOREACH_CHUNK_END()

  LinkedListChunksDelete(chunksToDelete);
}

void MapInit()
{
  map = (Map*)OwnMalloc(sizeof(Map), false);

  if(map == NULL)
  {
    LogError("Variable \"map\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  map->chunksActive = HashMapChunksCreate((size_t)(CHUNK_RENDER_RADIUS_SQUARED * 1.2f));
  map->chunksToRender = LinkedListChunksCreate();

  map->VAOSkybox = OpenGLCreateVAO();
  map->VBOSkybox = OpenGLCreateVBOCube();
  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  //Newer OpenGL: OpenGL_VBOLayout(map->VAOSkybox, map->VBOSkybox, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, sizeof(float));

  map->VAOSunMoon = OpenGLCreateVAO();
  map->VBOSunMoon = OpenGLCreateVBOQuad();

  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  OpenGL_VBOLayout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

  /* Newer OpenGL:
   * OpenGL_VBOLayout(map->VAOSunMoon, map->VBOSunMoon, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, 5 * sizeof(float));
   * OpenGL_VBOLayout(map->VAOSunMoon, map->VBOSunMoon, 1, 0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 5 * sizeof(float)); */

  if(DatabaseHasMapInfo())
  {
    DatabaseLoadMapInfo();
    LogInfo("The existing map \"%s\" is used.", true, MAP_NAME);
  }
  else
  {
    if(MAP_SEED == -1)
      MapSetSeed(rand()); //"rand()" returns a pseudo-random integral number in the range between 0 and "RAND_MAX".
    else
      MapSetSeed(MAP_SEED);

    LogInfo("A new map \"%s\" is being created.", true, MAP_NAME);
  }

  //The following can be sensitive, so caution is advised!
  int32_t procCount = GetProcessorsCount();
  if(NUM_WORKERS > 0 && NUM_WORKERS <= procCount)
    map->numWorkers = NUM_WORKERS;
  else
    map->numWorkers = MAX(1, procCount); //Ensure there is at least one worker.

  LogInfo("%d worker%s used.", true, map->numWorkers, map->numWorkers > 0 ? "s are" : " is");

  map->workers = (Worker*)OwnMalloc(map->numWorkers * sizeof(Worker), false);

  if(map->workers == NULL)
  {
    LogError("Variable \"map->workers\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  for(int32_t i = 0; i < map->numWorkers; ++i)
    ThreadWorkerCreate(&map->workers[i], ThreadWorkerLoop);
}

//[0.0 - 1.0]
double MapGetTime()
{
  if(DISABLE_TIME_FLOW)
    return 0.1;
  else
    return 0.5 + remainder(glfwGetTime(), DAY_LENGTH) / (double)DAY_LENGTH;

  //Manual control for debugging (time lapse):
  /*double static time = 0.0;

  if(WindowIsKeyPressed(GLFW_KEY_T))
    time += 0.0001;
  else if(WindowIsKeyPressed(GLFW_KEY_L))
    time -= 0.0001;

  if(time < 0.0)
    time += 1.0;
  else if(time > 1.0)
    time -= 1.0;
  //LogInfo("Time: %8.3f", true, time);

  return time;*/
}

float MapGetBlocksLight()
{
  float time = (float)MapGetTime();

  if(time < EVE_TO_NIGHT_START)
    return glm_lerp(DAY_LIGHT, EVENING_LIGHT, glm_smoothstep(DAY_TO_EVE_START, EVE_TO_NIGHT_START, time));
  else if(time < NIGHT_TO_DAY_START)
    return glm_lerp(EVENING_LIGHT, NIGHT_LIGHT, glm_smoothstep(EVE_TO_NIGHT_START, NIGHT_START, time));
  else
    return glm_lerp(NIGHT_LIGHT, DAY_LIGHT, glm_smoothstep(NIGHT_TO_DAY_START, 1.0f, time));
}

static void MapGetFogColor(float* r, float* g, float* b)
{
  float time = (float)MapGetTime();

  //These colors were taken from the skybox, whereby the fog can blend in smoothly.
  static vec3 dayColor = {0.310f, 0.535f, 0.783f};
  static vec3 eveningColor = {0.812f, 0.532f, 0.240f};
  static vec3 nightColor = {0.0f, 0.0f, 0.0f};

  vec3 color;

  if(time < EVE_TO_NIGHT_START)
    glm_vec3_mix(dayColor, eveningColor, glm_smoothstep(DAY_TO_EVE_START, EVE_TO_NIGHT_START, time), color);
  else if(time < NIGHT_TO_DAY_START)
    glm_vec3_mix(eveningColor, nightColor, glm_smoothstep(EVE_TO_NIGHT_START, NIGHT_START, time), color);
  else
    glm_vec3_mix(nightColor, dayColor, glm_smoothstep(NIGHT_TO_DAY_START, 1.0, time), color);

  *r = color[0];
  *g = color[1];
  *b = color[2];
}

void MapRenderSky(Camera* cam)
{
  mat4 model;
  glm_mat4_identity(model);
  glm_translate(model, cam->pos);

  //Rotate the cube map slightly, as it looks better when stars and moon have different rotation axes.
  glm_rotate(model, (float)(GLM_PI / 4.0f), (vec3){0.0f, 1.0f, 0.0f});

  mat4 MVPMatrix;
  glm_mat4_mul(cam->viewProjMatrix, model, MVPMatrix);

  glUseProgram(SHADER_SKYBOX);
  ShaderSetMat4(SHADER_SKYBOX, "MVPMatrix", MVPMatrix);

  ShaderSetTextureSkybox(SHADER_SKYBOX, "textureDay", TEXTURE_SKYBOX_DAY, 0);
  ShaderSetTextureSkybox(SHADER_SKYBOX, "textureEvening", TEXTURE_SKYBOX_EVENING, 1);
  ShaderSetTextureSkybox(SHADER_SKYBOX, "textureNight", TEXTURE_SKYBOX_NIGHT, 2);

  ShaderSetFloat1(SHADER_SKYBOX, "time", (float)MapGetTime());
  ShaderSetFloat1(SHADER_SKYBOX, "dayToEveStart", DAY_TO_EVE_START);
  ShaderSetFloat1(SHADER_SKYBOX, "eveToNightStart", EVE_TO_NIGHT_START);
  ShaderSetFloat1(SHADER_SKYBOX, "nightStart", NIGHT_START);
  ShaderSetFloat1(SHADER_SKYBOX, "nightToDayStart", NIGHT_TO_DAY_START);

  vec3 fogCol = {0};
  MapGetFogColor(&fogCol[0], &fogCol[1], &fogCol[2]);
  ShaderSetFloat3(SHADER_SKYBOX, "fogColor", fogCol);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_ALWAYS);

  glBindVertexArray(map->VAOSkybox);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glDepthFunc(GL_LESS);
}

void MapRenderSunMoon(Camera* cam)
{
  float time = (float)MapGetTime();

  //Add offset to current time - synchronizing the light level with the sun in the sky.
  float offset = 0.1f;
  time += offset;
  if(time > 1.0f)
    time -= 1.0f;

  //Sun and moon are always on opposite sides of the sky.
  float angleSun = (float)(time * GLM_PI * 2.0f);
  float angleMoon = (float)(angleSun + GLM_PI);
  //LogInfo("Time: %8.3f\nSun angle: %8.3f", true, time, angleSun);

  //Distance to the quads sun and moon are rendered on.
  const float dist = 5.0f;

  mat4 modelSun, modelMoon;
  glm_mat4_identity(modelSun);
  glm_mat4_identity(modelMoon);

  //Move to player:
  glm_translate(modelSun, cam->pos);
  glm_translate(modelMoon, cam->pos);

  //Rotate axis:
  glm_rotate(modelSun, GLM_PIf / 6.0f, (vec3){0.0f, 1.0f, 0.0f});
  glm_rotate(modelSun, GLM_PIf / 4.0f, (vec3){0.0f, 0.0f, 1.0f});

  glm_rotate(modelMoon, GLM_PIf / 6.0f, (vec3){0.0f, 1.0f, 0.0f});
  glm_rotate(modelMoon, GLM_PIf / 4.0f, (vec3){0.0f, 0.0f, 1.0f});

  glm_translate(modelSun, (vec3){0.0f, 0.0f, dist * cosf(angleSun)});
  glm_translate(modelSun, (vec3){0.0f, dist * sinf(angleSun), 0.0f});

  glm_translate(modelMoon, (vec3){0.0f, 0.0f, dist * cosf(angleMoon)});
  glm_translate(modelMoon, (vec3){0.0f, dist * sinf(angleMoon), 0.0f});

  //Rotate quads so sun and moon are always looking at player.
  glm_rotate(modelSun, -angleSun, (vec3){1.0f, 0.0f, 0.0f});
  glm_rotate(modelSun, GLM_PIf / 4.0f, (vec3){0.0f, 0.0f, 1.0f});

  glm_rotate(modelMoon, -angleMoon, (vec3){1.0f, 0.0f, 0.0f });
  glm_rotate(modelMoon, GLM_PIf / 4.0f, (vec3){0.0f, 0.0f, 1.0f});

  glUseProgram(SHADER_SUN);
  glBindVertexArray(map->VAOSunMoon);

  glDepthFunc(GL_ALWAYS);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  mat4 MVPMatrix;
  glm_mat4_mul(cam->viewProjMatrix, modelSun, MVPMatrix);
  ShaderSetMat4(SHADER_SUN, "MVPMatrix", MVPMatrix);
  ShaderSetTexture2D(SHADER_SUN, "textureSampler", TEXTURE_SUN, 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glm_mat4_mul(cam->viewProjMatrix, modelMoon, MVPMatrix);
  ShaderSetMat4(SHADER_SUN, "MVPMatrix", MVPMatrix);
  ShaderSetTexture2D(SHADER_SUN, "textureSampler", TEXTURE_MOON, 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
}

static float GetShadowMultiplier()
{
  float time = (float)MapGetTime();

  const float start = 0.35f;
  const float end = 0.45f;
  const float dur = end - start;

  const float start2 = 0.85f;
  const float end2 = 0.95f;
  const float dur2 = end2 - start2;

  float res = 1.0f;

  if(time >= start && time <= start + dur / 2.0f)
    res = 1.0f - glm_smoothstep(start, start + dur / 2.0f, time);
  else if(time >= start + dur / 2.0f && time <= end)
    res = glm_smoothstep(start + dur / 2.0f, end, time);
  else if(time >= start2 && time <= start2 + dur2 / 2.0f)
    res = 1.0f - glm_smoothstep(start2, start2 + dur2 / 2.0f, time);
  else if(time >= start2 + dur2 / 2.0f && time <= end2)
    res = glm_smoothstep(start2 + dur2 / 2.0f, end2, time);

  return res;
}

void MapRenderChunks(Camera* cam, mat4 nearShadowMapMat, mat4 farShadowMapMat)
{
  glUseProgram(SHADER_BLOCK);

  ShaderSetMat4(SHADER_BLOCK, "MVPMatrix", cam->viewProjMatrix);
  ShaderSetFloat1(SHADER_BLOCK, "blockLight", MapGetBlocksLight());

  ShaderSetTextureArray(SHADER_BLOCK, "textureSampler", TEXTURE_BLOCKS, 0);

  ShaderSetFloat3(SHADER_BLOCK, "camPos", cam->pos);
  ShaderSetFloat1(SHADER_BLOCK, "fogDist", CHUNK_RENDER_RADIUS * CHUNK_SIZE * 0.95f);

  float r, g, b;
  MapGetFogColor(&r, &g, &b);
  ShaderSetFloat3(SHADER_BLOCK, "fogColor", (vec3){r, g, b});

  ShaderSetMat4(SHADER_BLOCK, "uNearShadowMapMat", nearShadowMapMat);
  ShaderSetMat4(SHADER_BLOCK, "uFarShadowMapMat", farShadowMapMat);

  ShaderSetTexture2D(SHADER_BLOCK, "uNearShadowMap", WND->fb->GBufShadowNearMap, 1);
  ShaderSetTexture2D(SHADER_BLOCK, "uFarShadowMap", WND->fb->GBufShadowFarMap, 2);

  vec3 lightDir;
  MapGetLightDir(lightDir);
  ShaderSetFloat3(SHADER_BLOCK, "uLightDir", lightDir);

  ShaderSetFloat1(SHADER_BLOCK, "uNearShadowDist", 2.0f);
  ShaderSetFloat1(SHADER_BLOCK, "uShadowBlendDist", 0.1f);

  ShaderSetFloat3(SHADER_BLOCK, "uPlayerPos", cam->pos);

  ShaderSetFloat1(SHADER_BLOCK, "uShadowMultiplier", GetShadowMultiplier());

  //Only water needs blending.
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);

  LIST_FOREACH_CHUNK_BEGIN(map->chunksToRender, c)
  {
    glBindVertexArray(c->VAOLand);
    glDrawArrays(GL_TRIANGLES, 0, (uint32_t)c->vertexLandCount);
  }
  LIST_FOREACH_CHUNK_END()

  //Water needs blending to be transparent. Face culling has to be disabled to see underwater.
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  LIST_FOREACH_CHUNK_BEGIN(map->chunksToRender, c)
  {
    glBindVertexArray(c->VAOWater);
    glDrawArrays(GL_TRIANGLES, 0, (uint32_t)c->vertexWaterCount);
  }
  LIST_FOREACH_CHUNK_END()

  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);

  LinkedListChunksClear(map->chunksToRender);
}

void MapRenderChunksRaw(vec4 frustumPlanes[6])
{
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
  MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
  {
    if(c->isGenerated && ChunkIsVisible(c->x, c->z, frustumPlanes))
    {
      glBindVertexArray(c->VAOLand);
      glDrawArrays(GL_TRIANGLES, 0, (uint32_t)c->vertexLandCount);
    }
  }
  MAP_FOREACH_ACTIVE_CHUNK_END()
}

static void SetBlockHelper(int32_t cX, int32_t cZ, int32_t bX, int32_t bY, int32_t bZ, int32_t block)
{
  DatabaseInsertBlock(cX, cZ, bX, bY, bZ, block);

  Chunk* c = MapGetChunk(cX, cZ);
  if(c != NULL)
  {
    c->blocks[XYZ(bX, bY, bZ)] = (uint8_t)block;
    c->isDirty = true;
  }
}

static void SetBlock(Chunk* c, int32_t bX, int32_t bY, int32_t bZ, int32_t block)
{
  SetBlockHelper(c->x, c->z, bX, bY, bZ, block);

  //Set also in neighbour chunks if needed.
  const int32_t first = -1;
  const int32_t last = CHUNK_WIDTH;

  if(bX == 0)
  {
    SetBlockHelper(c->x - 1, c->z, last, bY, bZ, block);

    if(bZ == 0)
    {
      SetBlockHelper(c->x - 1, c->z - 1, last, bY, last, block);
      SetBlockHelper(c->x, c->z - 1, 0, bY, last, block);
    }
    else if(bZ == CHUNK_WIDTH - 1)
    {
      SetBlockHelper(c->x - 1, c->z + 1, last, bY, first, block);
      SetBlockHelper(c->x, c->z + 1, 0, bY, first, block);
    }
  }
  else if(bX == CHUNK_WIDTH - 1)
  {
    SetBlockHelper(c->x + 1, c->z, first, bY, bZ, block);

    if(bZ == 0)
    {
      SetBlockHelper(c->x + 1, c->z - 1, first, bY, last, block);
      SetBlockHelper(c->x, c->z - 1, last - 1, bY, last, block);
    }
    else if(bZ == CHUNK_WIDTH - 1)
    {
      SetBlockHelper(c->x + 1, c->z + 1, first, bY, first, block);
      SetBlockHelper(c->x, c->z + 1, last - 1, bY, first, block);
    }
  }
  else
  {
    if(bZ == 0)
      SetBlockHelper(c->x, c->z - 1, bX, bY, last, block);
    else if(bZ == CHUNK_WIDTH - 1)
      SetBlockHelper(c->x, c->z + 1, bX, bY, first, block);
  }
}

void MapSetBlock(int32_t bX, int32_t bY, int32_t bZ, uint8_t block)
{
  int32_t cX = ChunkedBlock(bX);
  int32_t cZ = ChunkedBlock(bZ);

  Chunk* c = MapGetChunk(cX, cZ);

  if(c == NULL) 
  {
    LogError("Variable \"c\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }
  int32_t x = ToChunkCoord(bX);
  int32_t z = ToChunkCoord(bZ);

  SetBlock(c, x, bY, z, block);
}

uint8_t MapGetBlock(int32_t bX, int32_t bY, int32_t bZ)
{
  Chunk* c = MapGetChunk(ChunkedBlock(bX), ChunkedBlock(bZ));
  if(c == NULL || !c->isGenerated)
    return AIR_BLOCK;

  return c->blocks[XYZ(ToChunkCoord(bX), bY, ToChunkCoord(bZ))];
}

static bool FindChunkForWorker(Camera* cam, int32_t* bestX, int32_t* bestZ)
{
  int32_t playerCx = ChunkedCam(cam->pos[0]);
  int32_t playerCz = ChunkedCam(cam->pos[2]);

  int32_t bestScore = INT_MAX;
  int32_t bestCx = 0, bestCz = 0;
  bool found = false;

  for(int32_t x = playerCx - CHUNK_LOAD_RADIUS; x <= playerCx + CHUNK_LOAD_RADIUS; ++x)
  {
    for(int32_t z = playerCz - CHUNK_LOAD_RADIUS; z <= playerCz + CHUNK_LOAD_RADIUS; ++z)
    {
      if(ChunkPlayerDistSquared(x, z, playerCx, playerCz) > CHUNK_LOAD_RADIUS_SQUARED)
        continue;

      Chunk* c = MapGetChunk(x, z);

      bool notDirty = c ? !c->isDirty : true;
      if(c != NULL && notDirty)
        continue;

      bool notVisible = !ChunkIsVisible(x, z, cam->frustumPlanes);
      int32_t dist = ChunkPlayerDistSquared(x, z, playerCx, playerCz);

      //Visibility is more important than dirtiness and dirtiness, in turn, is more important than distance.
      int32_t currScore = ((notVisible << 24) | (notDirty << 16)) + dist;
      if(currScore <= bestScore)
      {
        bestCx = x;
        bestCz = z;
        bestScore = currScore;
        found = true;
      }
    }
  }

  if(found)
  {
    *bestX = bestCx;
    *bestZ = bestCz;

    return true;
  }

  return false;
}

static void HandleWorkers(Camera* cam)
{
  for(int32_t i = 0; i < map->numWorkers; ++i)
  {
    WorkerData* worker = &map->workers[i].data;
    mtx_lock(&worker->stateMtx);

    if(worker->state == WORKER_BUSY)
    {
      mtx_unlock(&worker->stateMtx);

      continue;
    }

    if(worker->state == WORKER_DONE)
    {
      worker->state = WORKER_IDLE;

      Chunk* c = worker->chunk;
      ChunkUploadMeshToGPU(c);

      c->isSafeToModify = true;
    }

    if(worker->state == WORKER_IDLE)
    {
      int32_t bestCx, bestCz;
      bool found = FindChunkForWorker(cam, &bestCx, &bestCz);
      if(!found)
      {
        mtx_unlock(&worker->stateMtx);

        continue;
      }

      Chunk* c = MapGetChunk(bestCx, bestCz);
      if(c != NULL)
      {
        c->isDirty = false;
        worker->generateTerrain = false;
      }
      else
      {
        c = ChunkInit(bestCx, bestCz);
        HashMapChunksInsert(map->chunksActive, c);
        worker->generateTerrain = true;
      }

      c->isSafeToModify = false;
      worker->chunk = c;
      worker->state = WORKER_BUSY;

      mtx_unlock(&worker->stateMtx);
      cnd_signal(&worker->condVar);

      continue;
    }

    mtx_unlock(&worker->stateMtx);
  }
}

static void LoadChunk(int32_t cX, int32_t cZ)
{
  Chunk* c = ChunkInit(cX, cZ);
  ChunkGenerateTerrain(c);
  ChunkGenerateMesh(c);
  ChunkUploadMeshToGPU(c);

  HashMapChunksInsert(map->chunksActive, c);
}

void MapForceChunksNearPlayer(vec3 currPos)
{
  const int32_t dist = 1;

  int32_t playerCx = ChunkedCam(currPos[0]);
  int32_t playerCz = ChunkedCam(currPos[2]);

  for(int32_t dX = -dist; dX <= dist; ++dX)
  {
    for(int32_t dZ = -dist; dZ <= dist; ++dZ)
    {
      const int32_t cX = playerCx + dX;
      const int32_t cZ = playerCz + dZ;

      if(MapGetChunk(cX, cZ) == NULL)
        LoadChunk(cX, cZ);
    }
  }
}

static void AddChunksToRenderList(Camera* cam)
{
  MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
  {
    if(c->isGenerated && ChunkIsVisible(c->x, c->z, cam->frustumPlanes))
      LinkedListChunksPushFront(map->chunksToRender, c);
  }
  MAP_FOREACH_ACTIVE_CHUNK_END()
}

void MapUpdate(Camera* cam)
{
  TryToDeleteFarChunks(cam->pos);
  HandleWorkers(cam);
  MapForceChunksNearPlayer(cam->pos);
  AddChunksToRenderList(cam);
}

void MapSetSeed(int32_t newSeed)
{
  map->seed = newSeed;

  LogInfo("%sSeed = %d%s (0 - %d)", true, LINE, newSeed, NOLINE, RAND_MAX);
}

int32_t MapGetSeed()
{
  return map->seed;
}

void MapSetTime(double newTime)
{
  glfwSetTime(DAY_LENGTH / 2.0 + newTime * DAY_LENGTH);
}

int32_t MapGetHighestBlock(int32_t bX, int32_t bZ)
{
  Chunk* c = MapGetChunk(ChunkedBlock(bX), ChunkedBlock(bZ));
  if(c == NULL) 
    return CHUNK_HEIGHT;

  int32_t x = ToChunkCoord(bX);
  int32_t z = ToChunkCoord(bZ);

  int32_t maxHeight = 0;
  for(int32_t y = 0; y < CHUNK_HEIGHT; ++y)
  {
    if(BlockIsSolid(c->blocks[XYZ(x, y, z)]))
      maxHeight = y;
  }

  return maxHeight;
}

void MapGetLightDir(vec3 res)
{
  float time = (float)MapGetTime();

  //Add offset to current time - synchronizing the light level with the sun in the sky.
  float offset = 0.1f;
  time += offset;
  if(time > 1.0f)
    time -= 1.0f;

  float angle = time * GLM_PIf * 2.0f;

  //Moonlight:
  if(time >= 0.5)
    angle += GLM_PIf;

  OwnGLMVec3Set(res, 0.0f, sinf(angle), cosf(angle));
  glm_vec3_rotate(res, GLM_PIf / 4.0f, (vec3){0.0f, 0.0f, 1.0f});
  glm_vec3_rotate(res, GLM_PIf / 6.0f, (vec3){0.0f, 1.0f, 0.0f});

  glm_vec3_negate(res);
}

void MapSave()
{
  DatabaseSaveMapInfo();
}

void MapFree()
{
  MapSave();

  //Workers:
  for(int32_t i = 0; i < map->numWorkers; ++i)
    ThreadWorkerDestroy(&map->workers[i]);
  free(map->workers);

  //Chunk hash maps and linked lists:
  LinkedListChunks* toDelete = LinkedListChunksCreate();
  MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
    LinkedListChunksPushBack(toDelete, c);
  MAP_FOREACH_ACTIVE_CHUNK_END()

  while(toDelete->size)
  {
    Chunk* c = LinkedListChunksPopFront(toDelete);
    ChunkDelete(c);
  }

  HashMapChunksDelete(map->chunksActive);
  LinkedListChunksDelete(map->chunksToRender);
  LinkedListChunksDelete(toDelete);

  free(map);
  map = NULL;
}