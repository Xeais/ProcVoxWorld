#pragma once

#include "../Utils.h"

#define XYZ(x, y, z)   (((x) + 1) * CHUNK_WIDTH_REAL * CHUNK_HEIGHT_REAL) \
                     + (((y) + 1) * CHUNK_WIDTH_REAL)                     \
                     +  ((z) + 1)

typedef struct
{
  uint8_t* blocks;
  int32_t x, z;

  bool isDirty;
  bool isGenerated;
  bool isSafeToModify;

  GLuint VAOLand;
  GLuint VBOLand;
  GLuint VAOWater;
  GLuint VBOWater;
  size_t vertexLandCount;
  size_t vertexWaterCount;

  Vertex* generatedMeshTerrain;
  Vertex* generatedMeshWater;
} Chunk;

Chunk* ChunkInit(int32_t cX, int32_t cZ);

void ChunkGenerateTerrain(Chunk* c);

void ChunkGenerateMesh(Chunk* c);

void ChunkUploadMeshToGPU(Chunk* c);

bool ChunkIsVisible(int32_t cX, int32_t cZ, vec4 planes[6]);

void ChunkDelete(Chunk* c);

//----- Inline -----

//Hash functions which are used in chunk hash map:
static inline uint32_t ChunkHashFunc2(int32_t cx, int32_t cz)
{
  return (cx + cz) * (cx + cz + 1) / 2 + cz;
}

static inline uint32_t ChunkHashFunc(Chunk* c)
{
  return ChunkHashFunc2(c->x, c->z);
}