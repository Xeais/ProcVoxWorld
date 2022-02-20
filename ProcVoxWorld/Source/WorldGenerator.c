#include "WorldGenerator.h"

#include "NoiseGenerator.h"

#include "Map/Block.h"

#include <assert.h>

static const int32_t waterLevel = 50;

//The following biomes are created:
typedef enum
{
  BIOME_PLAINS,
  BIOME_FOREST,
  BIOME_FLOWER_FOREST,
  BIOME_MOUNTAINS,
  BIOME_DESERT,
  BIOME_WATER
} Biome;
//Additionally, "WorldGenerator" creates trees procedurally.

/* Noise types of FastNoiseLite:
 * typedef enum
 * {
 *    FNL_NOISE_OPENSIMPLEX2,  //0
 *    FNL_NOISE_OPENSIMPLEX2S, //1
 *    FNL_NOISE_CELLULAR,      //2
 *    FNL_NOISE_PERLIN,        //3
 *    FNL_NOISE_VALUE_CUBIC,   //4
 *    FNL_NOISE_VALUE          //5
 * } fnl_noise_type; */

static Biome GetBiome(noiseState* noiseState, int32_t bX, int32_t bZ)
{
  //Voronoi diagram:
  noiseState->fnl.cellular_return_type = FNL_CELLULAR_RETURN_VALUE_CELLVALUE;
  NoiseGeneratorSetSettings(&noiseState->fnl, FNL_NOISE_CELLULAR, 0.005f, 1, 2.0f, 0.5f);

  float fX = (float)bX;
  float fZ = (float)bZ;

  fnlDomainWarp2D(&noiseState->fnl, &fX, &fZ);
  float height = NoiseGenerator2D(&noiseState->fnl, fX, fZ);

  if(height < 0.2f)
    return BIOME_WATER;
  else if(height < 0.45f)
    return BIOME_PLAINS;
  else if(height < 0.65f)
    return BIOME_FOREST;
  else if(height < 0.75f)
    return BIOME_FLOWER_FOREST;
  else if(height < 0.85f)
    return BIOME_MOUNTAINS;
  else
    return BIOME_DESERT;
}

/* Does not work the edges of a chunk, as some leaves could be in other chunks,
 * but blocks can only be placed in the current chunk. */
static void MakeTree(Chunk* c, int32_t x, int32_t y, int32_t z)
{
  assert(x >= 2 && x <= CHUNK_WIDTH - 3 && z >= 2 && z <= CHUNK_WIDTH - 3);

  for(uint32_t i = 1; i <= 5; ++i)
    c->blocks[XYZ(x, y + i, z)] = WOOD_BLOCK;

  c->blocks[XYZ(x, y + 7, z)] = LEAVES_BLOCK;

  for(int32_t dX = -2; dX <= 2; ++dX)
  {
    for(int32_t dZ = -1; dZ <= 1; ++dZ)
    {
      for(uint32_t dY = 4; dY <= 5; ++dY)
      {
        int32_t bX = x + dX;
        int32_t bY = y + dY;
        int32_t bZ = z + dZ;

        if(c->blocks[XYZ(bX, bY, bZ)] != AIR_BLOCK)
          continue;

        c->blocks[XYZ(bX, bY, bZ)] = LEAVES_BLOCK;
      }
    }
  }

  for(int32_t dX = -1; dX <= 1; ++dX)
  {
    for(int32_t dZ = -2; dZ <= 2; ++dZ)
    {
      for(uint32_t dY = 4; dY <= 5; ++dY)
      {
        int32_t bX = x + dX;
        int32_t bY = y + dY;
        int32_t bZ = z + dZ;

        if(c->blocks[XYZ(bX, bY, bZ)] != AIR_BLOCK)
          continue;

        c->blocks[XYZ(bX, bY, bZ)] = LEAVES_BLOCK;
      }
    }
  }

  for(int32_t dX = -1; dX <= 1; ++dX)
  {
    for(int32_t dZ = -1; dZ <= 1; ++dZ)
    {
      for(uint32_t dY = 3; dY <= 6; ++dY)
      {
        int32_t bX = x + dX;
        int32_t bY = y + dY;
        int32_t bZ = z + dZ;

        if(c->blocks[XYZ(bX, bY, bZ)] != AIR_BLOCK)
          continue;

        c->blocks[XYZ(bX, bY, bZ)] = LEAVES_BLOCK;
      }
    }
  }
}

static void GenPlains(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, int32_t h)
{
  for(int32_t y = 0; y < h; ++y)
    c->blocks[XYZ(x, y, z)] = DIRT_BLOCK;

  c->blocks[XYZ(x, h, z)] = GRASS_BLOCK;

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;

  //Only generate grass and flowers if there's no water.
  if(c->blocks[XYZ(x, h + 1, z)] == WATER_BLOCK)
    return;

  if(OwnRand(&noiseState->randValue) % 10 >= 7)
    c->blocks[XYZ(x, h + 1, z)] = GRASS_PLANT_BLOCK;
  else if(OwnRand(&noiseState->randValue) % 100 > 97)
  {
    if(OwnRand(&noiseState->randValue) % 2)
      c->blocks[XYZ(x, h + 1, z)] = FLOWER_DANDELION_BLOCK;
    else
      c->blocks[XYZ(x, h + 1, z)] = FLOWER_ROSE_BLOCK;
  }
}

static void GenForest(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, int32_t h)
{
  for(int32_t y = 0; y < h; ++y)
    c->blocks[XYZ(x, y, z)] = DIRT_BLOCK;

  c->blocks[XYZ(x, h, z)] = GRASS_BLOCK;

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;

  if(c->blocks[XYZ(x, h + 1, z)] == WATER_BLOCK)
    return;

  if(OwnRand(&noiseState->randValue) % 1000 > 975 && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
    MakeTree(c, x, h, z);
  else if(OwnRand(&noiseState->randValue) % 10 >= 9)
    c->blocks[XYZ(x, h + 1, z)] = GRASS_PLANT_BLOCK;
  else if(OwnRand(&noiseState->randValue) % 100 > 97)
  {
    if(OwnRand(&noiseState->randValue) % 2)
      c->blocks[XYZ(x, h + 1, z)] = FLOWER_DANDELION_BLOCK;
    else
      c->blocks[XYZ(x, h + 1, z)] = FLOWER_ROSE_BLOCK;
  }
}

static void GenFlowerForest(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, int32_t h)
{
  for(int32_t y = 0; y < h; ++y)
    c->blocks[XYZ(x, y, z)] = DIRT_BLOCK;

  c->blocks[XYZ(x, h, z)] = GRASS_BLOCK;

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;

  if(c->blocks[XYZ(x, h + 1, z)] == WATER_BLOCK)
    return;

  if(OwnRand(&noiseState->randValue) % 1000 > 975 && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
    MakeTree(c, x, h, z);
  else if(OwnRand(&noiseState->randValue) % 10 >= 7)
  {
    int32_t r = OwnRand(&noiseState->randValue) % 3;
    switch(r)
    {
      case 0:
        c->blocks[XYZ(x, h + 1, z)] = GRASS_PLANT_BLOCK;
        break;
      case 1:
        c->blocks[XYZ(x, h + 1, z)] = FLOWER_DANDELION_BLOCK;
        break;
      default:
        c->blocks[XYZ(x, h + 1, z)] = FLOWER_ROSE_BLOCK;
        break;
    }
  }
}

static void GenMountains(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, uint32_t h)
{
  for(uint32_t y = 0; y <= h; ++y)
  {
    if(y < 100 + OwnRand(&noiseState->randValue) % 10 - 5)
    {
      if(OwnRand(&noiseState->randValue) % 10 == 0)
        c->blocks[XYZ(x, y, z)] = GRAVEL_BLOCK;
      else
        c->blocks[XYZ(x, y, z)] = STONE_BLOCK;
    }
    else
      c->blocks[XYZ(x, y, z)] = SNOW_BLOCK;
  }

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;
}

static void GenDesert(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, int32_t h)
{
  for(int32_t y = 0; y < h; ++y)
    c->blocks[XYZ(x, y, z)] = SANDSTONE_BLOCK;

  c->blocks[XYZ(x, h, z)] = SAND_BLOCK;

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;

  if(c->blocks[XYZ(x, h + 1, z)] == WATER_BLOCK)
    return;

  if(OwnRand(&noiseState->randValue) % 1000 > 995)
  {
    int32_t cactusHeight = OwnRand(&noiseState->randValue) % 6;
    for(int32_t y = 0; y < cactusHeight; ++y)
      c->blocks[XYZ(x, h + 1 + y, z)] = CACTUS_BLOCK;
  }
  else if(OwnRand(&noiseState->randValue) % 1000 > 995)
    c->blocks[XYZ(x, h + 1, z)] = DEAD_PLANT_BLOCK;
}

static void GenWater(noiseState* noiseState, Chunk* c, int32_t x, int32_t z, int32_t h)
{
  for(int32_t y = 0; y <= h; ++y)
  {
    if(OwnRand(&noiseState->randValue) % 4 == 0)
      c->blocks[XYZ(x, y, z)] = GRAVEL_BLOCK;
    else
      c->blocks[XYZ(x, y, z)] = SAND_BLOCK;
  }

  for(int32_t y = h + 1; y <= waterLevel; ++y)
    c->blocks[XYZ(x, y, z)] = WATER_BLOCK;
}

static int32_t GetHeight(fnl_state* noiseState, Biome biome, int32_t bX, int32_t bZ)
{
  float v;
  switch(biome)
  {
    case BIOME_PLAINS:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.003f, 3, 2.5f, 0.1f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT;
      return 44 + (int32_t)(v / 8);
    case BIOME_FOREST:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT * 0.7f;
      return 38 + (int32_t)(v / 4);
    case BIOME_FLOWER_FOREST:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT * 0.7f;
      return 38 + (int32_t)(v / 4);
    case BIOME_MOUNTAINS:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.005f, 3, 2.0f, 1.0f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT;
      return 48 + (int32_t)(v / 3);
    case BIOME_DESERT:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.0006f, 5, 2.5f, 0.75f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT;
      return 48 + (int32_t)(v / 8);
    case BIOME_WATER:
      NoiseGeneratorSetSettings(noiseState, FNL_NOISE_OPENSIMPLEX2, 0.01f, 4, 2.0f, 0.5f);
      v = NoiseGenerator2D(noiseState, (float)bX, (float)bZ) * CHUNK_HEIGHT;
      return 32 + (int32_t)(v / 16);
    default:
      return CHUNK_HEIGHT - 1;
  }
}

//Bilinear interpolation:
static int32_t Blerp(int32_t h11, int32_t h12, int32_t h21, int32_t h22, float x, float y)
{
  return (int32_t)(h11 * (1 - x) * (1 - y) + h21 * x * (1 - y) + h12 * (1 - x) * y + h22 * x * y);
}

//Indexing into "biomes" and "heightmap" arrays:
#define XZ(x, z) ((((x) + 8) * ((CHUNK_WIDTH + 1) + 8 + 8)) + ((z) + 8))

void WorldGeneratorGenerateChunk(Chunk* c)
{
  noiseState* noiseState = NoiseGeneratorCreateState(c->x, c->z);

  /* Space for "CHUNK_WIDTH" - 1 normal chunk blocks
   * Two blocks are stored as neighbour data; 8 + 8 blocks are used for interpolation. */
  const int32_t sideLen = (CHUNK_WIDTH - 1) + 2 + 8 + 8;

  Biome* biomes = (Biome*)OwnMalloc((uintmax_t)sideLen * sideLen * sizeof(Biome), false);
  int32_t* heightmap = (int32_t*)OwnMalloc((uintmax_t)sideLen * sideLen * sizeof(int32_t), false);

  if(biomes == NULL || heightmap == NULL) 
  {
    LogError("Variables \"biomes\" and \"heightmap\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  int32_t cStartX = c->x * CHUNK_WIDTH;
  int32_t cStartZ = c->z * CHUNK_WIDTH;

  for(int32_t x = -8; x <= CHUNK_WIDTH + 8; ++x)
  {
    for(int32_t z = -8; z <= CHUNK_WIDTH + 8; ++z)
    {
      int32_t bX = cStartX + x;
      int32_t bZ = cStartZ + z;

      biomes[XZ(x, z)] = GetBiome(noiseState, bX, bZ);

      if(x % 8 == 0 && z % 8 == 0)
        heightmap[XZ(x, z)] = GetHeight(&noiseState->fnl, biomes[XZ(x, z)], bX, bZ);
    }
  }

  for(int32_t x = -1; x <= CHUNK_WIDTH; ++x)
  {
    for(int32_t z = -1; z <= CHUNK_WIDTH; ++z)
    {
      if(x % 8 || z % 8)
      {
        const int32_t xLeft = FloorEight(x);
        const int32_t zTop = FloorEight(z);

        heightmap[XZ(x, z)] = Blerp(heightmap[XZ(xLeft, zTop)], heightmap[XZ(xLeft, zTop + 8)], heightmap[XZ(xLeft + 8, zTop)],
                                    heightmap[XZ(xLeft + 8, zTop + 8)], (x - xLeft) / 8.0f, (z - zTop) / 8.0f);
      }

      switch(biomes[XZ(x, z)])
      {
        case BIOME_PLAINS:   
          GenPlains(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
        case BIOME_FOREST:
          GenForest(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
        case BIOME_FLOWER_FOREST: 
          GenFlowerForest(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
        case BIOME_MOUNTAINS:
          GenMountains(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
        case BIOME_DESERT:
          GenDesert(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
        case BIOME_WATER:   
          GenWater(noiseState, c, x, z, heightmap[XZ(x, z)]); 
          break;
      }
    }
  }

  free(biomes);
  free(heightmap);

  free(noiseState);
}