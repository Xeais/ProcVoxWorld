#include "Chunk.h"
#include "Block.h"

#include "glad/glad.h" //If the successor, Glad 2, wasn't still in beta, I would have used it.

#include "../Database.h"

#include "../WorldGenerator.h"

Chunk* ChunkInit(int32_t cX, int32_t cZ)
{
  Chunk* c = (Chunk*)OwnMalloc(sizeof(Chunk), false);

  if(c == NULL)
  {
    LogError("Variable \"c\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return NULL;
  }

  c->blocks = NULL;
  c->x = cX;
  c->z = cZ;

  c->isDirty = false;
  c->isGenerated = false;
  c->isSafeToModify = true;

  c->VAOLand = 0;
  c->VBOLand = 0;
  c->VAOWater = 0;
  c->VBOWater = 0;
  c->vertexLandCount = 0;
  c->vertexWaterCount = 0;

  c->generatedMeshTerrain = NULL;
  c->generatedMeshWater = NULL;

  return c;
}

void ChunkGenerateTerrain(Chunk* c)
{
  c->blocks = (uint8_t*)OwnMalloc(BLOCKS_MEMORY_SIZE, false);

  if(c->blocks == NULL)
    LogError("Variable \"c->blocks\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

  WorldGeneratorGenerateChunk(c);
  DatabaseGetBlocksForChunk(c);
}

/* Array layout of neighbours (view towards -Y):
 *
 * ----> +X   Top layer   Middle layer   Bottom layer
 * |          18 21 24    9  12 15       0 3 6
 * v          19 22 25    10 13 16       1 4 7
 * +Z         20 23 26    11 14 17       2 5 8 */
static void BlockGetNeighbours(Chunk* c, int32_t x, int32_t y, int32_t z, uint8_t bNeighbs[27])
{
  int32_t index = 0;
  for(int32_t dY = -1; dY <= 1; ++dY)
  {
    for(int32_t dX = -1; dX <= 1; ++dX)
    {
      for(int32_t dZ = -1; dZ <= 1; ++dZ, ++index)
      {
        const int32_t bX = x + dX;
        const int32_t bY = y + dY;
        const int32_t bZ = z + dZ;

        bNeighbs[index] = c->blocks[XYZ(bX, bY, bZ)];
      }
    }
  }
}

static bool ShouldBeVisible(uint8_t block, Chunk* c, int32_t neighbX, int32_t neighbY, int32_t neighbZ)
{
  uint8_t neighb = c->blocks[XYZ(neighbX, neighbY, neighbZ)];

  return BlockIsTransparent(neighb) && block != neighb;
}

static int32_t BlockSetVisibleFaces(Chunk* c, int32_t x, int32_t y, int32_t z, int32_t faces[6])
{
  uint8_t block = c->blocks[XYZ(x, y, z)];

  faces[LEFT_FACE_BLOCK] = ShouldBeVisible(block, c, x - 1, y, z);
  faces[RIGHT_FACE_BLOCK] = ShouldBeVisible(block, c, x + 1, y, z);
  faces[TOP_FACE_BLOCK] = ShouldBeVisible(block, c, x, y + 1, z);
  faces[BOTTOM_FACE_BLOCK] = ShouldBeVisible(block, c, x, y - 1, z);
  faces[BACK_FACE_BLOCK] = ShouldBeVisible(block, c, x, y, z - 1);
  faces[FRONT_FACE_BLOCK] = ShouldBeVisible(block, c, x, y, z + 1);

  int32_t numNisible = 0;
  for(uint32_t i = 0; i < 6; ++i)
    numNisible += faces[i];

  return numNisible;
}

static void BlockSetAmbientOcclusion(uint8_t neighbs[27], float AO[6][4])
{
  //Indices of neighbours for each vertex of each face:
  static const uint8_t lookup[6][4][3] =
  {
    {{ 0,  1,  9}, { 2,  1, 11}, {18,  9, 19}, {20, 19, 11}}, //Left
    {{ 6, 15,  7}, { 8, 17,  7}, {24, 25, 15}, {26, 17, 25}}, //Right
    {{18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25}}, //Top
    {{ 0,  1,  3}, { 2,  1,  5}, { 6,  3,  7}, { 8,  5,  7}}, //Bottom
    {{ 0,  3,  9}, {18,  9, 21}, { 6,  3, 15}, {24, 15, 21}}, //Back
    {{ 2,  5, 11}, {20, 23, 11}, { 8,  5, 17}, {26, 17, 23}}, //Front
  };

  static const float curve[4] = {0.0f, 0.33f, 0.66f, 1.0f};

  for(uint32_t f = 0; f < 6; ++f)
  {
    for(uint32_t v = 0; v < 4; ++v)
    {
      bool corner = BlockIsTransparent(neighbs[lookup[f][v][0]]) ? false : true;
      bool side1 = BlockIsTransparent(neighbs[lookup[f][v][1]]) ? false : true;
      bool side2 = BlockIsTransparent(neighbs[lookup[f][v][2]]) ? false : true;

      AO[f][v] = side1 && side2 ? curve[3] : curve[corner + side1 + side2];
    }
  }
}

void ChunkGenerateMesh(Chunk* c)
{
  c->generatedMeshTerrain = (Vertex*)OwnMalloc((uintmax_t)CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex), false);
  c->generatedMeshWater = (Vertex*)OwnMalloc((uintmax_t)CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex), false);

  if(c->generatedMeshTerrain == NULL || c->generatedMeshWater == NULL)
  {
    LogError("RAM size is insufficient; decreasing the amount of worker threads should help!", true);

    exit(EXIT_FAILURE);
  }

  int32_t currVertexLandCount = 0;
  int32_t currVertexWaterCount = 0;

  for(int32_t x = 0; x < CHUNK_WIDTH; ++x)
  {
    for(int32_t y = 0; y < CHUNK_HEIGHT; ++y)
    {
      for(int32_t z = 0; z < CHUNK_WIDTH; ++z)
      {
        uint8_t block = c->blocks[XYZ(x, y, z)];
        if(block == AIR_BLOCK)
          continue;

        int32_t faces[6];
        int32_t numVisible = BlockSetVisibleFaces(c, x, y, z, faces);

        if(numVisible == 0)
          continue;

        uint8_t bNeighbs[27];
        BlockGetNeighbours(c, x, y, z, bNeighbs);

        float AO[6][4];
        BlockSetAmbientOcclusion(bNeighbs, AO);

        int32_t bX = x + (c->x * CHUNK_WIDTH);
        int32_t bY = y;
        int32_t bZ = z + (c->z * CHUNK_WIDTH);

        if(block == WATER_BLOCK)
        {
          uint8_t blockAbove = c->blocks[XYZ(x, y + 1, z)];
          int32_t makeShorter = (blockAbove == AIR_BLOCK);

          GenCubeVertices(c->generatedMeshWater, &currVertexWaterCount, bX, bY, bZ, block, BLOCK_SIZE, makeShorter, faces, AO);
        }
        else
        {
          if(BlockIsPlant(block))
            GenPlantVertices(c->generatedMeshTerrain, &currVertexLandCount, bX, bY, bZ, block, BLOCK_SIZE);
          else
            GenCubeVertices(c->generatedMeshTerrain, &currVertexLandCount, bX, bY, bZ, block, BLOCK_SIZE, 0, faces, AO);
        }
      }
    }
  }

  c->vertexLandCount = currVertexLandCount;
  c->vertexWaterCount = currVertexWaterCount;
}

void ChunkUploadMeshToGPU(Chunk* c)
{
  if(c->isGenerated)
  {
    glDeleteVertexArrays(2, (const GLuint[]) {c->VAOLand, c->VAOWater});
    glDeleteBuffers(2, (const GLuint[]) {c->VBOLand, c->VBOWater});
  }

  c->VAOLand = OpenGLCreateVAO();
  c->VBOLand = OpenGLCreateVBO(c->generatedMeshTerrain, c->vertexLandCount * sizeof(Vertex));
  free(c->generatedMeshTerrain);
  c->generatedMeshTerrain = NULL;

  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  OpenGL_VBOLayout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
  OpenGL_VBOLayout(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
  OpenGL_VBOLayout(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float));
  OpenGL_VBOLayout(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float) + 1);

  /* Newer OpenGL:
   * OpenGL_VBOLayout(c->VAOLand, c->VBOLand, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOLand, c->VBOLand, 1, 0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOLand, c->VBOLand, 2, 0, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOLand, c->VBOLand, 3, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOLand, c->VBOLand, 4, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float) + 1, sizeof(Vertex)); */

  c->VAOWater = OpenGLCreateVAO();
  c->VBOWater = OpenGLCreateVBO(c->generatedMeshWater, c->vertexWaterCount * sizeof(Vertex));
  free(c->generatedMeshWater);
  c->generatedMeshWater = NULL;

 OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
 OpenGL_VBOLayout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
 OpenGL_VBOLayout(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
 OpenGL_VBOLayout(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float));
 OpenGL_VBOLayout(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float) + 1);

  /* Newer OpenGL:
   * OpenGL_VBOLayout(c->VAOWater, c->VBOWater, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOWater, c->VBOWater, 1, 0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOWater, c->VBOWater, 2, 0, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOWater, c->VBOWater, 3, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float), sizeof(Vertex));
   * OpenGL_VBOLayout(c->VAOWater, c->VBOWater, 4, 0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(float) + 1, sizeof(Vertex)); */

  c->isGenerated = true;
}

bool ChunkIsVisible(int32_t cX, int32_t cZ, vec4 planes[6])
{
  //Construct chunk AABB:
  vec3 AABB[2] = {0};
  AABB[0][0] = cX * CHUNK_SIZE;
  AABB[0][1] = 0.0f;
  AABB[0][2] = cZ * CHUNK_SIZE;
  AABB[1][0] = AABB[0][0] + CHUNK_SIZE;
  AABB[1][1] = CHUNK_HEIGHT * BLOCK_SIZE;
  AABB[1][2] = AABB[0][2] + CHUNK_SIZE;

  return glm_aabb_frustum(AABB, planes);
}

void ChunkDelete(Chunk* c)
{
  if(c->isGenerated)
  {
    glDeleteVertexArrays(2, (const GLuint[]) {c->VAOLand, c->VAOWater});
    glDeleteBuffers(2, (const GLuint[]) {c->VBOLand, c->VBOWater});
    free(c->blocks);
  }

  if(c->generatedMeshTerrain)
  {
    free(c->generatedMeshTerrain);
    free(c->generatedMeshWater);
  }

  free(c);
}