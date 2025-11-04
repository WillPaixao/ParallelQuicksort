#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"
//#include "task.h"

#ifdef DEBUG
  #define logMessage(...) printf(__VA_ARGS__)
#else
  #define logMessage(...)
#endif

struct pool_s;
typedef struct pool_s* POOL;

// Thread pool
typedef struct pool_s {
  // User defined parameters
  int nThreads;                       // Number of threads in the pool
  int maxWorkers;                     // Maximum number of workers that can be allocated to a single task
  void (*taskFunc)(TASK, int, POOL);  // Function that operates on data of a TASK
  
  // Control variables
  QUEUE queue;                        // Internal task queue
  pthread_t* tids;                    // Array of thread IDs in the pool
  int nAvailableThreads;              // Number of current sleeping threads
  char shutdown;                      // Indicator of pool shutdown

  // Locks & condition variables
  sem_t sleeping;                     // Semaphore that implements on-demand task picking
  pthread_mutex_t lock;               // Mutex lock to inhibit data racing
} pool_t;

POOL makePool(int nThreads, int maxWorkers, void (*taskFunc)(TASK, int, POOL));
void executeTask(TASK task, POOL pool);
char isLastThreadInPool(POOL pool);
void shutdownPool(POOL pool);
void waitPoolShutdown(POOL pool);
