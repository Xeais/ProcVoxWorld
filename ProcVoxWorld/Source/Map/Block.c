#include "Block.h"

uint8_t BLOCK_TEXTURES[][6] =
{
  //lft  rgt  top  bot  bck  frt
  {  0,   0,   0,   0,   0,   0}, //0: AIR_BLOCK
  {239, 239, 239, 239, 239, 239}, //1: PLAYER_HAND_BLOCK
  {241, 241, 241, 241, 241, 241}, //2: STONE_BLOCK
  {242, 242, 242, 242, 242, 242}, //3: DIRT_BLOCK  
  {243, 243, 254, 242, 243, 243}, //4: GRASS_BLOCK 
  {244, 244, 244, 244, 244, 244}, //5: WOODEN_PLANKS_BLOCK
  {246, 246, 246, 246, 246, 246}, //6: POLISHED_STONE_BLOCK
  {247, 247, 247, 247, 247, 247}, //7: BRICKS_BLOCK
  {224, 224, 224, 224, 224, 224}, //8: OBBLESTONE_BLOCK
  {225, 225, 225, 225, 225, 225}, //9: BEDROCK_BLOCK  
  {226, 226, 226, 226, 226, 226}, //10: SAND_BLOCK  
  {227, 227, 227, 227, 227, 227}, //11: GRAVEL_BLOCK
  {228, 228, 229, 229, 228, 228}, //12: WOOD_BLOCK
  {230, 230, 230, 230, 230, 230}, //13: IRON_BLOCK
  {231, 231, 231, 231, 231, 231}, //14: GOLD_BLOCK
  {232, 232, 232, 232, 232, 232}, //15: DIAMOND_BLOCK
  {233, 233, 233, 233, 233, 233}, //16: EMERALD_BLOCK
  {234, 234, 234, 234, 234, 234}, //17: REDSTONE_BLOCK
  {212, 212, 212, 212, 212, 212}, //18: MOSSY_COBBLESTONE_BLOCK
  {213, 213, 213, 213, 213, 213}, //19: OBSIDIAN_BLOCK 
  {198, 198, 198, 198, 198, 198}, //20: STONE_BRICKS_BLOCK
  {178, 178, 178, 178, 178, 178}, //21: SNOW_BLOCK  
  {180, 180, 178, 242, 180, 180}, //22: SNOW_GRASS_BLOCK
  {193, 193, 193, 193, 193, 193}, //23: GLASS_BLOCK     
  { 61,  61,  61,  61,  61,  61}, //24: WATER_BLOCK
  {223, 223, 223, 223, 223, 223}, //25: LEAVES_BLOCK
  {215, 215, 215, 215, 215, 215}, //26: GRASS_PLANT_BLOCK
  {252, 252, 252, 252, 252, 252}, //27: FLOWER_ROSE_BLOCK
  {253, 253, 253, 253, 253, 253}, //28: FLOWER_DANDELION_BLOCK
  {237, 237, 237, 237, 237, 237}, //29: MUSHROOM_BROWN_BLOCK
  {236, 236, 236, 236, 236, 236}, //30: MUSHROOM_RED_BLOCK
  {199, 199, 199, 199, 199, 199}, //31: DEAD_PLANT_BLOCK 
  {182, 182, 181, 183, 182, 182}, //32: CACTUS_BLOCK
  { 48,  48,  64,  32,  48,  48}, //33: SANDSTONE_BLOCK
  { 21,  21,  64,  32,  21,  21}, //34: SANDSTONE_CHISELED_BLOCK
};

void GenCubeVertices(Vertex* vertices, int32_t* currVertexCount, int32_t x, int32_t y, int32_t z,
                     int32_t blockType, float blockSize, int32_t isShort, int32_t faces[6], float AO[6][4])
{
  //Six faces, each face has four points which form a square.
  static const float pos[6][4][3] =
  {
    {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}}, //Left
    {{1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}}, //Right
    {{0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}}, //Top
    {{0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1}}, //Bottom
    {{0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}}, //Back
    {{0, 0, 1}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1}}  //Front
  };

  //A cactus block is a bit smaller than others.
#define A 0.0625f
#define B (1.0f - A)
  static const float posCactus[6][4][3] =
  {
    {{A, 0, 0}, {A, 0, 1}, {A, 1, 0}, {A, 1, 1}}, //Left
    {{B, 0, 0}, {B, 0, 1}, {B, 1, 0}, {B, 1, 1}}, //Right
    {{0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}}, //Top
    {{0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1}}, //Bottom
    {{0, 0, A}, {0, 1, A}, {1, 0, A}, {1, 1, A}}, //Back
    {{0, 0, B}, {0, 1, B}, {1, 0, B}, {1, 1, B}}  //Front
  };
#undef A
#undef B

  static const int32_t indices[6][6] =
  {
    {0, 3, 2, 0, 1, 3},
    {0, 3, 1, 0, 2, 3},
    {0, 3, 2, 0, 1, 3},
    {0, 3, 1, 0, 2, 3},
    {0, 3, 2, 0, 1, 3},
    {0, 3, 1, 0, 2, 3}
  };

  static const int32_t indicesFlipped[6][6] =
  {
    {0, 1, 2, 1, 3, 2},
    {0, 2, 1, 2, 3, 1},
    {0, 1, 2, 1, 3, 2},
    {0, 2, 1, 2, 3, 1},
    {0, 1, 2, 1, 3, 2},
    {0, 2, 1, 2, 3, 1}
  };

  //-> UV mapping: https://en.wikipedia.org/wiki/UV_mapping
  static const float UVs[6][4][2] =
  {
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
    {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
    {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
    {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
    {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
    {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
  };

  for(uint32_t f = 0; f < 6; ++f)
  {
    if(!faces[f]) 
      continue;

    for(uint32_t v = 0; v < 6; ++v)
    {
      //Flip some quads to eliminate ambient occlusion unevenness: https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
      int32_t index = (AO[f][0] + AO[f][3] > AO[f][1] + AO[f][2]) ? indicesFlipped[f][v] : indices[f][v];

      int32_t i = (*currVertexCount)++;

      //A cactus block is a bit thinner than others.
      if(blockType == CACTUS_BLOCK)
      {
        vertices[i].pos[0] = (posCactus[f][index][0] + (float)x) * blockSize;
        vertices[i].pos[1] = (posCactus[f][index][1] + (float)y) * blockSize;
        vertices[i].pos[2] = (posCactus[f][index][2] + (float)z) * blockSize;
      }
      else
      {
        vertices[i].pos[0] = (pos[f][index][0] + (float)x) * blockSize;
        vertices[i].pos[1] = (pos[f][index][1] + (float)y) * blockSize;
        vertices[i].pos[2] = (pos[f][index][2] + (float)z) * blockSize;
      }

      //Only make the top of a block shorter.
      if(isShort && pos[f][index][1] > 0)
        vertices[i].pos[1] -= 0.125f * blockSize;

      vertices[i].texCoord[0] = UVs[f][index][0];
      vertices[i].texCoord[1] = UVs[f][index][1];
      vertices[i].AO = AO[f][index];
      vertices[i].tile = BLOCK_TEXTURES[blockType][f];
      vertices[i].normal = (uint8_t)f; //"Vertex.normal" is an "uint8_t" as it is a data container (a single unsigned byte/octet value).
    }
  }
}

void GenPlantVertices(Vertex* vertices, int32_t* currVertexCount, int32_t x, int32_t y, int32_t z, int32_t blockType, float blockSize)
{
  //A cross which is made up of two perpendicular quads.
  static const float pos[2][4][3] =
  {
    {{0.5f, 0.0f, 0.0f}, {0.5f, 0.0f, 1.0f}, {0.5f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.0f}},
    {{0.0f, 0.0f, 0.5f}, {1.0f, 0.0f, 0.5f}, {1.0f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f}}
  };

  static const int32_t indices[2][6] =
  {
    {0, 1, 2, 2, 3, 0},
    {0, 3, 2, 2, 1, 0}
  };

  static const float UVs[4][2] =
  {
    {0, 0}, {1, 0}, {1, 1}, {0, 1}
  };

  static const int32_t normals[4] = {0, 1, 5, 4};

  //A plant consists of twwo quads, but four are needed in order to fight face culling.
  for(uint32_t f = 0; f < 4; ++f)
  {
    for(uint32_t v = 0; v < 6; ++v)
    {
      int32_t index = indices[f % 2][v];
      int32_t i = (*currVertexCount)++;

      vertices[i].pos[0] = (pos[f / 2][index][0] + (float)x) * blockSize;
      vertices[i].pos[1] = (pos[f / 2][index][1] + (float)y) * blockSize;
      vertices[i].pos[2] = (pos[f / 2][index][2] + (float)z) * blockSize;

      vertices[i].texCoord[0] = UVs[index][0];
      vertices[i].texCoord[1] = UVs[index][1];
      vertices[i].AO = 0.0f;
      vertices[i].tile = BLOCK_TEXTURES[blockType][f];
      vertices[i].normal = (uint8_t)normals[f];
    }
  }
}

bool BlockIsSolid(uint8_t block)
{
  switch(block)
  {
    case AIR_BLOCK:
    case WATER_BLOCK:
      return false;
    default:
      return true;
  }
}

bool BlockIsTransparent(uint8_t block)
{
  switch(block)
  {
    case AIR_BLOCK:
    case GLASS_BLOCK:
    case WATER_BLOCK:
    case LEAVES_BLOCK:
    case GRASS_PLANT_BLOCK:
    case FLOWER_ROSE_BLOCK:
    case FLOWER_DANDELION_BLOCK:
    case MUSHROOM_BROWN_BLOCK:
    case MUSHROOM_RED_BLOCK:
    case DEAD_PLANT_BLOCK:
    case CACTUS_BLOCK:
      return true;
    default:
      return false;
  }
}

bool BlockIsPlant(uint8_t block)
{
  switch(block)
  {
    case GRASS_PLANT_BLOCK:
    case FLOWER_ROSE_BLOCK:
    case FLOWER_DANDELION_BLOCK:
    case MUSHROOM_BROWN_BLOCK:
    case MUSHROOM_RED_BLOCK:
    case DEAD_PLANT_BLOCK:
      return true;
    default:
      return false;
  }
}

void BlockGenAABB(int32_t x, int32_t y, int32_t z, vec3 AABB[2])
{
  AABB[0][0] = (float)x * BLOCK_SIZE;
  AABB[0][1] = (float)y * BLOCK_SIZE;
  AABB[0][2] = (float)z * BLOCK_SIZE;
  AABB[1][0] = AABB[0][0] + BLOCK_SIZE;
  AABB[1][1] = AABB[0][1] + BLOCK_SIZE;
  AABB[1][2] = AABB[0][2] + BLOCK_SIZE;
}

bool BlockRayIntersection(vec3 rayPos, vec3 rayDir, int32_t bX, int32_t bY, int32_t bZ, uint8_t bType)
{
  vec3 min = {bX * BLOCK_SIZE, bY * BLOCK_SIZE, bZ * BLOCK_SIZE};
  vec3 max = {min[0] + BLOCK_SIZE, min[1] + BLOCK_SIZE, min[2] + BLOCK_SIZE};

  //Diminish the hitbox:
  if(BlockIsPlant(bType))
  {
    float a = BLOCK_SIZE * 0.25f;
    min[0] += a;
    min[2] += a;
    max[0] -= a;
    max[1] -= 2 * a;
    max[2] -= a;
  }

  float tMin = 0.00001f;
  float tMax = 10000.0f;

  vec3 invD = {0}; //Initialize all elements to zero.
  for(uint32_t i = 0; i < 3; ++i)
    invD[i] = 1.0f / rayDir[i];

  vec3 t0s = {0};
  for(uint32_t i = 0; i < 3; ++i)
    t0s[i] = (min[i] - rayPos[i]) * invD[i];

  vec3 t1s = {0};
  for(uint32_t i = 0; i < 3; ++i)
    t1s[i] = (max[i] - rayPos[i]) * invD[i];

  vec3 tSmaller = {0};
  glm_vec3_minadd(t0s, t1s, tSmaller);

  vec3 tBigger = {0};
  glm_vec3_maxadd(t0s, t1s, tBigger);

  tMin = MAX(tMin, MAX(tSmaller[0], MAX(tSmaller[1], tSmaller[2])));
  tMax = MIN(tMax, MIN(tBigger[0], MIN(tBigger[1], tBigger[2])));

  return (tMin < tMax);
}