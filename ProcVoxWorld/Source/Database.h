#pragma once

#include "Map/Chunk.h"
#include "Player/Player.h"

void DatabaseInit(const char* dbPath);

void DatabaseInsertBlock(int32_t chunkX, int32_t chunkZ, int32_t x, int32_t y, int32_t z, int32_t block);

void DatabaseGetBlocksForChunk(Chunk* c);

void DatabaseSavePlayerInfo(Player* p);

void DatabaseLoadPlayerInfo(Player* p);

bool DatabaseHasPlayerInfo();

void DatabaseSaveMapInfo();

void DatabaseLoadMapInfo();

bool DatabaseHasMapInfo();

void DatabaseFree();