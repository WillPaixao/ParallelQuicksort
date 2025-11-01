#include <stdlib.h>
#include <pthread.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"

#define min(a,b) (a) < (b) ? (a) : (b)

POOL populatePool(POOL pool);
void* threadFunc(void* args);
int recruitThreads(POOL pool, TASK task);
void destroyPool(POOL pool);

// Thread pool
typedef struct pool_s {
  // User defined parameters
  int nThreads;            // Number of threads in the pool
  int maxWorkers;          // Maximum number of workers that can be allocated to a single task
  void (*taskFunc)(TASK);  // Function that operates on data of a TASK
  
  // Control variables
  QUEUE queue;             // Internal task queue
  pthread_t* tids;         // Array of thread IDs in the pool
  int nAvailableThreads;   // Number of current non-sleeping threads
  char shutdown;           // Indicator of pool shutdown

  // Locks & condition variables
  pthread_cond_t sleeping; // Condition variable that implements on-demand task picking
  pthread_mutex_t lock;    // Mutex lock to inhibit data racing
} pool_t;

// Makes a thread pool.
// Returns a pointer to the pool in success, NULL if an error occurred.
POOL makePool(int nThreads, int maxWorkers, void (*taskFunc)(TASK)){
  if (nThreads <= 0 ||
      maxWorkers <= 0 ||
      maxWorkers > nThreads ||
      !taskFunc)
    return NULL;
  
  pool_t* newPool = (pool_t*)malloc(sizeof(pool_t));
  if (!newPool)
    return NULL;
  
  newPool->nThreads = nThreads;
  newPool->maxWorkers = maxWorkers;
  newPool->taskFunc = taskFunc;

  newPool->nAvailableThreads = nThreads;
  newPool->shutdown = 0;
  newPool->queue = makeQueue();
  if (!newPool->queue || 
      pthread_cond_init(&newPool->sleeping, NULL) ||
      pthread_mutex_init(&newPool->lock, NULL)){
    free(newPool);
    return NULL;
  }

  // Populates pool with new threads
  if (!populatePool(newPool)){
    pthread_cond_destroy(&newPool->sleeping);
    pthread_mutex_destroy(&newPool->lock);
    return NULL;
  }
  
  return newPool;
}

// Populates pool with fresh threads.
// Returns a pointer to the pool in success, NULL otherwise.
POOL populatePool(POOL pool){
  if (!pool)
    return NULL;
  
  int nThreads = pool->nThreads;
  pthread_t* tids = (pthread_t*)calloc(nThreads, sizeof(pthread_t));
  if (!tids)
    return NULL;
  
  for (int i = 0; i < nThreads; i++){
    if (pthread_create(&tids[i], NULL, threadFunc, (void*)pool)){
      free(tids);
      return NULL;
    }
  }

  pool->tids = tids;
  return pool;
}

// Function that will be executed by the threads in the pool.
// It encapsulates all communication in the pool.
// Assumes that args is of type POOL.
void* threadFunc(void* args){
  // Global state
  POOL pool = (POOL)args;
  static TASK currTaskGlobal;
  TASK currTaskLocal;

  // Control variables for the recruiting phase
  static char isRecruiting = 0;
  static int nPendingWorkers;

  // Local references to synchronization variables
  pthread_mutex_t* lockPtr = &pool->lock;
  pthread_cond_t* sleepingPtr = &pool->sleeping;
  
  while (1){
    // Long critical section (acquiring and recruiting phases)
    pthread_mutex_lock(lockPtr);

    // While pool is still being fed and is empty...
    while (!pool->shutdown && isEmptyQueue(pool->queue))
      pthread_cond_wait(sleepingPtr, lockPtr); // ... sleep until something happens
    
    if (isEmptyQueue(pool->queue)){ // If something happened and the queue is empty...
      pthread_mutex_unlock(lockPtr);
      break; // ... then no more tasks will be assigned (pool shutdown)
    }

    if (!isRecruiting){ // If not in recruiting phase, acquire a pending task and recruit
      isRecruiting = 1;
      currTaskGlobal = takeTask(pool->queue);
      nPendingWorkers = recruitThreads(pool, currTaskGlobal);
    }
    else { // Else, account for my recruitment
      nPendingWorkers--;
    }

    currTaskLocal = currTaskGlobal; // Assigning to a local copy

    if (nPendingWorkers == 0){ // If there is no one left to recruit...
      isRecruiting = 0;        // ... end the recruiting phase...
      currTaskGlobal = NULL;   // ... and reset the task at hand, for safety
    }

    pthread_mutex_unlock(lockPtr);

    pool->taskFunc(currTaskLocal); // Finally, execute the task (concurrently)
  }

  pthread_exit(NULL);
}

// Schedules a task to be executed by a thread in the pool.
void executeTask(TASK task, POOL pool){
  if (!pool || !task)
    return;

  putTask(task, pool->queue);

  pthread_mutex_lock(&pool->lock);
  pthread_cond_signal(&pool->sleeping);
  pool->nAvailableThreads--;
  pthread_mutex_unlock(&pool->lock);
}

// Recruits other threads in the pool to help in a task, signaling them.
// Assumes external mutual exclusion on access to the pool and task.
// Returns the number of allocated workers in success, 0 otherwise.
int recruitThreads(POOL pool, TASK task){
  if (!pool || !task)
    return 0;
  
  pthread_cond_t* sleepingPtr = &pool->sleeping;
  int nAvailableThreads = pool->nAvailableThreads;
  int maxWorkers = pool->maxWorkers;
  int nAllocatedWorkers = min(nAvailableThreads, maxWorkers);

  for (int i = 0; i < nAllocatedWorkers; i++){
    pthread_cond_signal(sleepingPtr);
    nAvailableThreads--;
  }
  
  task->nWorkers = nAllocatedWorkers + 1; // Plus one accounting for the recruiter
  pool->nAvailableThreads = nAvailableThreads;

  return nAllocatedWorkers;
}

// Shutdown the pool, forcing threads to start ending activity.
// This also waits for the threads to return.
void shutdownPool(POOL pool){
  if (!pool)
    return;

  pthread_mutex_t* lockPtr = &pool->lock;
  pthread_cond_t* sleepingPtr = &pool->sleeping;
  int nThreads = pool->nThreads;
  pthread_t* tids = pool->tids;

  pthread_mutex_lock(lockPtr);
  pool->shutdown = 1;
  pthread_cond_broadcast(sleepingPtr); // Signaling all threads to prepare for leaving
  pthread_mutex_unlock(lockPtr);

  for (int i = 0; i < nThreads; i++){
    if (pthread_join(tids[i], NULL)){
      // TODO: think of a good error handling here
    }
  }

  destroyPool(pool);
}

// Destroys the pool.
// Assumes that no threads are alive in it.
void destroyPool(POOL pool){
  if (!pool)
    return;
  
  destroyQueue(pool->queue);
  free(pool->tids);
  pthread_cond_destroy(&pool->sleeping);
  pthread_mutex_destroy(&pool->lock);

  free(pool);
}
