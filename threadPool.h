#pragma once
#include "task.h"

#ifdef DEBUG
  #define logMessage(...) printf(__VA_ARGS__)
#else
  #define logMessage(...)
#endif

struct pool_s;
typedef struct pool_s* POOL; // Opaque pointer to thread pool

POOL makePool(int nThreads, int maxWorkers, void (*taskFunc)(TASK, int, POOL));
void executeTask(TASK task, POOL pool);
void finishTask(TASK task);
char isLastThreadInPool(POOL pool);
void shutdownPool(POOL pool);
void waitPoolShutdown(POOL pool);
