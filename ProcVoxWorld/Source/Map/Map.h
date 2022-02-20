#pragma once

#include "../LinkedList.h"
#include "../HashMap.h"

#include "Chunk.h"
#include "../Camera/Camera.h"

//Define data structures for chunks:
LINKED_LIST_DECLARATION(Chunk*, Chunks);
HASH_MAP_DECLARATION(Chunk*, Chunks);

void MapInit();

void MapUpdate(Camera* cam);

void MapRenderSky(Camera* cam);

void MapRenderSunMoon(Camera* cam);

void MapRenderChunks(Camera* cam, mat4 nearShadowMapMat, mat4 farShadowMapMat);

void MapRenderChunksRaw(vec4 frustumPlanes[6]);

void MapForceChunksNearPlayer(vec3 currPos);

void MapSetBlock(int32_t x, int32_t y, int32_t z, uint8_t block);

uint8_t MapGetBlock(int32_t x, int32_t y, int32_t z);

void MapSetSeed(int32_t newSeed);

int32_t MapGetSeed();

void MapSetTime(double newTime);

double MapGetTime();

float MapGetBlocksLight();

int32_t MapGetHighestBlock(int32_t x, int32_t z);

void MapGetLightDir(vec3 res);

void MapSave();

void MapFree();