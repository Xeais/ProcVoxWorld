#include "Database.h"

#include "SQLite/sqlite3.h"

#include "Map/Map.h"

#include <assert.h>

static sqlite3* db;
static bool sHasPlayerInfo;
static bool sHasMapInfo;

static sqlite3_stmt* DatabaseCompileStatement(const char* statement)
{
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, statement, -1, &stmt, NULL);
  if(stmt == NULL)
    LogError("SQLite (SQL database engine) reports an error during statement compiling.\nThe used statement was: %s.\nThe resulting error is: %s.", true, statement, sqlite3_errmsg(db));

  return stmt;
}

static void DatabaseCompileRunStatement(const char* statement)
{
  sqlite3_stmt* stmt = DatabaseCompileStatement(statement);

  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

static bool DatabaseIsTableEmpty(const char* table)
{
  char query[512];
  snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s", table);

  sqlite3_stmt* stmt = DatabaseCompileStatement(query);

  sqlite3_bind_text(stmt, 1, table, -1, SQLITE_STATIC);
  sqlite3_step(stmt);

  int32_t rowCount = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return rowCount == 0;
}

static void DatabaseCreateTables()
{
  DatabaseCompileRunStatement("CREATE TABLE IF NOT EXISTS Blocks(chunkX INTEGER NOT NULL, chunkZ INTEGER NOT NULL, x INTEGER NOT NULL, "
                              "y INTEGER NOT NULL, z INTEGER NOT NULL, type INTEGER NOT NULL, PRIMARY KEY(chunkX, chunkZ, x, y, z))");

  DatabaseCompileRunStatement("CREATE TABLE IF NOT EXISTS MapInfo(seed INTEGER NOT NULL, currTime REAL NOT NULL, chunkWidth INTEGER NOT NULL, chunkHeight INTEGER NOT NULL)");

  DatabaseCompileRunStatement("CREATE TABLE IF NOT EXISTS PlayerInfo(posX REAL NOT NULL, posY REAL NOT NULL, posZ REAL NOT NULL, pitch REAL NOT NULL, yaw REAL NOT NULL, buildBlock INTEGER NOT NULL)");

  if(!strcmp(sqlite3_errmsg(db), "not an error"))
    LogSuccess("Database tables \"Blocks\", \"MapInfo\" and \"PlayerInfo\" were successfully created.", true);

  if(!DatabaseIsTableEmpty("MapInfo"))
    sHasMapInfo = true;

  if(!DatabaseIsTableEmpty("PlayerInfo"))
    sHasPlayerInfo = true;
}

void DatabaseInit(const char* dbPath)
{
  int32_t result = sqlite3_open(dbPath, &db);
  if(result != SQLITE_OK)
  {
    LogError("There was a problem trying to open database file: %s.\nSignaled SQLite-error: %s", true, dbPath, sqlite3_errmsg(db));

    exit(EXIT_FAILURE);
  }

  if(!sqlite3_threadsafe())
  {
    LogError("An SQLite-connection has to be thread safe, but is not!", true);

    exit(EXIT_FAILURE);
  }

  DatabaseCreateTables();

  //Optimizations to significantly expedite the database:
  DatabaseCompileRunStatement("PRAGMA synchronous = off");
  DatabaseCompileRunStatement("PRAGMA temp_store = memory");
  DatabaseCompileRunStatement("PRAGMA locking_mode = exclusive");
}

void DatabaseInsertBlock(int32_t chunkX, int32_t chunkZ, int32_t x, int32_t y, int32_t z, int32_t block)
{
  static sqlite3_stmt* stmt = NULL;
  if(stmt == NULL)
    stmt = DatabaseCompileStatement("INSERT OR REPLACE INTO Blocks (chunkX, chunkZ, x, y, z, type) VALUES (?, ?, ?, ?, ?, ?)");

  sqlite3_reset(stmt);

  sqlite3_bind_int(stmt, 1, chunkX);
  sqlite3_bind_int(stmt, 2, chunkZ);
  sqlite3_bind_int(stmt, 3, x);
  sqlite3_bind_int(stmt, 4, y);
  sqlite3_bind_int(stmt, 5, z);
  sqlite3_bind_int(stmt, 6, block);

  sqlite3_step(stmt);
}

void DatabaseGetBlocksForChunk(Chunk* c)
{
  static sqlite3_stmt* stmt = NULL;
  if(stmt == NULL)
    stmt = DatabaseCompileStatement("SELECT x, y, z, type FROM Blocks WHERE chunkX = ? AND chunkZ = ?");

  sqlite3_reset(stmt);

  sqlite3_bind_int(stmt, 1, c->x);
  sqlite3_bind_int(stmt, 2, c->z);

  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    int32_t x = sqlite3_column_int(stmt, 0);
    int32_t y = sqlite3_column_int(stmt, 1);
    int32_t z = sqlite3_column_int(stmt, 2);
    int32_t block = sqlite3_column_int(stmt, 3);

    c->blocks[XYZ(x, y, z)] = (uint8_t)block; //"Chunk->blocks" is an "uint8_t*" as it is a data container (sequence of unsigned byte/octet values).
  }
}

void DatabaseSavePlayerInfo(Player* p)
{
  sqlite3_stmt* stmt = NULL;
  stmt = sHasPlayerInfo ? DatabaseCompileStatement("UPDATE PlayerInfo SET posX = ?, posY = ?, posZ = ?, pitch = ?, yaw = ?, buildBlock = ?")
                        : DatabaseCompileStatement("INSERT INTO PlayerInfo (posX, posY, posZ, pitch, yaw, buildBlock) VALUES (?, ?, ?, ?, ?, ?)");

  sqlite3_bind_double(stmt, 1, p->pos[0]);
  sqlite3_bind_double(stmt, 2, p->pos[1]);
  sqlite3_bind_double(stmt, 3, p->pos[2]);
  sqlite3_bind_double(stmt, 4, p->pitch);
  sqlite3_bind_double(stmt, 5, p->yaw);
  sqlite3_bind_int(stmt, 6, p->buildBlock);

  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  sHasPlayerInfo = true;
}

void DatabaseLoadPlayerInfo(Player* p)
{
  assert(sHasPlayerInfo);

  sqlite3_stmt* stmt = DatabaseCompileStatement("SELECT posX, posY, posZ, pitch, yaw, buildBlock FROM PlayerInfo");

  sqlite3_reset(stmt);
  sqlite3_step(stmt);

  p->pos[0] = (float)sqlite3_column_double(stmt, 0); //Strangely, there is no "sqlite3_column_float()".
  p->pos[1] = (float)sqlite3_column_double(stmt, 1);
  p->pos[2] = (float)sqlite3_column_double(stmt, 2);
  p->pitch = (float)sqlite3_column_double(stmt, 3);
  p->yaw = (float)sqlite3_column_double(stmt, 4);
  p->buildBlock = (uint8_t)sqlite3_column_int(stmt, 5); //"Player->buildBlock" is an "uint8_t" as it is a data container (a single unsigned byte/octet value).

  sqlite3_finalize(stmt);
}

bool DatabaseHasPlayerInfo()
{
  return sHasPlayerInfo;
}

void DatabaseSaveMapInfo()
{
  sqlite3_stmt* stmt = NULL;
  if(sHasMapInfo)
  {
    stmt = DatabaseCompileStatement("UPDATE MapInfo SET currTime = ?");
    sqlite3_bind_double(stmt, 1, MapGetTime());
  }
  else
  {
    stmt = DatabaseCompileStatement("INSERT INTO MapInfo (seed, currTime, chunkWidth, chunkHeight) VALUES (?, ?, ?, ?)");
    sqlite3_bind_int(stmt, 1, MapGetSeed());
    sqlite3_bind_double(stmt, 2, MapGetTime());
    sqlite3_bind_int(stmt, 3, CHUNK_WIDTH);
    sqlite3_bind_int(stmt, 4, CHUNK_HEIGHT);
  }

  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  sHasMapInfo = true;
}

void DatabaseLoadMapInfo()
{
  assert(sHasMapInfo);

  sqlite3_stmt* stmt = DatabaseCompileStatement("SELECT seed, currTime, chunkWidth, chunkHeight FROM MapInfo");

  sqlite3_reset(stmt);
  sqlite3_step(stmt);

  int32_t seed = sqlite3_column_int(stmt, 0);
  double currTime = sqlite3_column_double(stmt, 1);
  int32_t width = sqlite3_column_int(stmt, 2);
  int32_t height = sqlite3_column_int(stmt, 3);

  if(width != CHUNK_WIDTH || height != CHUNK_HEIGHT)
  {
    LogError("The map \"%s\" was created for use with \"CHUNK_WIDTH = %d\" and \"CHUNK_HEIGHT = %d\". "
             "Unfortunately, the current width and height (%d, %d) do not meet this requirement. "
             "To resolve this issue, please use another map or chunk dimensions!",
             true, MAP_NAME, width, height, CHUNK_WIDTH, CHUNK_HEIGHT);

    exit(EXIT_FAILURE);
  }

  MapSetSeed(seed);
  MapSetTime(currTime);
  sqlite3_finalize(stmt);
}

bool DatabaseHasMapInfo()
{
  return sHasMapInfo;
}

void DatabaseFree()
{
  sqlite3_close(db);
}