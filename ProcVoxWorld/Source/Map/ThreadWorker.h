#pragma once

#include "TinyCThread/tinycthread.h" //-> "C11 threads on Windows": https://stackoverflow.com/a/54791888
/* From "https://tinycthread.github.io":
 * Need portable threads for your C app? Is C11 unavailable for your target compiler(s) (the following could be complemented) or it is available 
 * but you are on Windows and don't want to bother with the non-portable, non-standard "xthreads.h"?
 * 
 * Then you need TinyCThread! */

#include "Chunk.h"

typedef enum
{
  WORKER_IDLE,
  WORKER_BUSY,
  WORKER_DONE,
  WORKER_EXIT
} WorkerState;

typedef struct
{
  Chunk* chunk;
  bool generateTerrain;

  WorkerState state;
  mtx_t stateMtx;
  cnd_t condVar;
} WorkerData;

typedef struct
{
  thrd_t thread;
  WorkerData data;
} Worker;

void ThreadWorkerCreate(Worker* worker, thrd_start_t func);

int32_t ThreadWorkerLoop(void* data);

void ThreadWorkerDestroy(Worker* worker);