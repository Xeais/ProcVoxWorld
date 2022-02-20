#pragma once //Could also be called "Voxel.h", but for fairly large voxels the term "block" is more appropriate.

#include "../Utils.h"

#include <stdint.h> 

//Block IDs:
#define AIR_BLOCK                0
#define PLAYER_HAND_BLOCK        1 
#define STONE_BLOCK              2 
#define DIRT_BLOCK               3 
#define GRASS_BLOCK              4 
#define WOODEN_PLANKS_BLOCK      5 
#define POLISHED_STONE_BLOCK     6 
#define BRICKS_BLOCK             7 
#define COBBLESTONE_BLOCK        8 
#define BEDROCK_BLOCK            9 
#define SAND_BLOCK               10
#define GRAVEL_BLOCK             11
#define WOOD_BLOCK               12
#define IRON_BLOCK               13
#define GOLD_BLOCK               14
#define DIAMOND_BLOCK            15
#define EMERALD_BLOCK            16
#define REDSTONE_BLOCK           17
#define MOSSY_COBBLESTONE_BLOCK  18
#define OBSIDIAN_BLOCK           19
#define STONE_BRICKS_BLOCK       20
#define SNOW_BLOCK               21
#define SNOW_GRASS_BLOCK         22
#define GLASS_BLOCK              23
#define WATER_BLOCK              24
#define LEAVES_BLOCK             25
#define GRASS_PLANT_BLOCK        26
#define FLOWER_ROSE_BLOCK        27
#define FLOWER_DANDELION_BLOCK   28
#define MUSHROOM_BROWN_BLOCK     29
#define MUSHROOM_RED_BLOCK       30
#define DEAD_PLANT_BLOCK         31
#define CACTUS_BLOCK             32
#define SANDSTONE_BLOCK          33
#define SANDSTONE_CHISELED_BLOCK 34
#define AMOUNT_BLOCKS            35

//Faces order:
#define LEFT_FACE_BLOCK   0
#define RIGHT_FACE_BLOCK  1
#define TOP_FACE_BLOCK    2
#define BOTTOM_FACE_BLOCK 3
#define BACK_FACE_BLOCK   4
#define FRONT_FACE_BLOCK  5

//Textures for each face of each block
extern uint8_t BLOCK_TEXTURES[][6];

void GenCubeVertices(Vertex* vertices, int32_t* currVertexCount, int32_t x, int32_t y, int32_t z,
                     int32_t blockType, float blockSize, int32_t isShort, int32_t faces[6], float AO[6][4]);

void GenPlantVertices(Vertex* vertices, int32_t* currVertexCount, int32_t x, int32_t y, int32_t z, int32_t blockType, float blockSize);

//Use "uint8_t" for a single a unsigned-byte/octet-value and "uint8_t*" for a sequence-of-unsigned-byte/octet-values.
bool BlockIsSolid(uint8_t block);

bool BlockIsTransparent(uint8_t block);

bool BlockIsPlant(uint8_t block);

void BlockGenAABB(int32_t x, int32_t y, int32_t z, vec3 AABB[2]);

//AABB hit detection: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
bool BlockRayIntersection(vec3 rayPos, vec3 rayDir, int32_t bX, int32_t bY, int32_t bZ, uint8_t bType);