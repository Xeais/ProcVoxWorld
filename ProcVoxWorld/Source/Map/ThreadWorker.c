#include "ThreadWorker.h"

void ThreadWorkerCreate(Worker* worker, thrd_start_t func)
{
  cnd_init(&worker->data.condVar);
  mtx_init(&worker->data.stateMtx, mtx_plain);
  worker->data.state = WORKER_IDLE;
  worker->data.chunk = NULL;
  worker->data.generateTerrain = false;

  thrd_create(&worker->thread, func, &worker->data);
}

int32_t ThreadWorkerLoop(void* data)
{
  WorkerData* dataInt = (WorkerData*)data;

  while(true)
  {
    mtx_lock(&dataInt->stateMtx);
    while(dataInt->state != WORKER_BUSY && dataInt->state != WORKER_EXIT)
      cnd_wait(&dataInt->condVar, &dataInt->stateMtx);

    if(dataInt->state == WORKER_EXIT)
      break;

    mtx_unlock(&dataInt->stateMtx);

    if(dataInt->generateTerrain)
      ChunkGenerateTerrain(dataInt->chunk);
    ChunkGenerateMesh(dataInt->chunk);

    mtx_lock(&dataInt->stateMtx);
    if(dataInt->state == WORKER_EXIT)
      break;

    dataInt->state = WORKER_DONE;
    mtx_unlock(&dataInt->stateMtx);
  }

  thrd_exit(0);

#pragma warning(suppress: 4702) //This code segment is actually unreachable - never mind!	
  return 0; //Against "C4716": "function" must return a value.
}

void ThreadWorkerDestroy(Worker* worker)
{
  mtx_lock(&worker->data.stateMtx);
  worker->data.state = WORKER_EXIT;
  mtx_unlock(&worker->data.stateMtx);
  cnd_signal(&worker->data.condVar);

  thrd_join(worker->thread, NULL);
  mtx_destroy(&worker->data.stateMtx);
  cnd_destroy(&worker->data.condVar);
}