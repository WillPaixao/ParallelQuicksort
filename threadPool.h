#pragma once
#include "task.h"

struct pool_s;
typedef struct pool_s* POOL; // Opaque pointer to thread pool

POOL makePool(int nThreads, int maxWorkers, void (*taskFunc)(TASK));
void executeTask(TASK task, POOL pool);
void shutdownPool(POOL pool);
