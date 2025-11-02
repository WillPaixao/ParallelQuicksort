#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"

#define min(a,b) (a) < (b) ? (a) : (b)

POOL populatePool(POOL pool);
void* threadFunc(void* args);
int recruitThreads(POOL pool, TASK task);

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

// Makes a thread pool.
// Returns a pointer to the pool in success, NULL if an error occurred.
// OBS.: The parameter taskFunc must be a pointer to a function that expects a task and a local task TID as arguments.
POOL makePool(int nThreads, int maxWorkers, void (*taskFunc)(TASK, int, POOL)){
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
      sem_init(&newPool->sleeping, 0, 0) ||
      pthread_mutex_init(&newPool->lock, NULL)){
    free(newPool);
    return NULL;
  }

  // Populates pool with new threads
  if (!populatePool(newPool)){
    sem_destroy(&newPool->sleeping);
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
  static int nAllocatedWorkers = 0;
  static int taskTIDGlobal;
  int taskTIDLocal;

  // Local references to synchronization variables
  pthread_mutex_t* lockPtr = &pool->lock;
  sem_t* sleepingPtr = &pool->sleeping;
  
  while (1){
    // Long critical section (acquiring and recruiting phases)
    pthread_mutex_lock(lockPtr);

    // While pool is still being fed and is empty and a recruitment is not taking place...
    if (!pool->shutdown && isEmptyQueue(pool->queue) && !isRecruiting){
      pthread_mutex_unlock(lockPtr);
      sem_wait(sleepingPtr); // ... sleep until something happens
      pthread_mutex_lock(lockPtr);
    }
    
    // If something happened and the queue is empty and is not being recruited...
    if (isEmptyQueue(pool->queue) && !isRecruiting){
      pthread_mutex_unlock(lockPtr);
      break; // ... then no more tasks will be assigned (pool shutdown)
    }

    pool->nAvailableThreads--;

    if (!isRecruiting){ // If not in recruiting phase, acquire a pending task and recruit
      isRecruiting = 1;
      currTaskGlobal = takeTask(pool->queue);
      taskTIDGlobal = 0;
      nAllocatedWorkers = recruitThreads(pool, currTaskGlobal);
      logMessage("Recruiting %d threads!\n", nAllocatedWorkers);
    }
    else { // Else, account for my recruitment
      taskTIDGlobal++;
    }

    // Assigning to local copies
    currTaskLocal = currTaskGlobal;
    taskTIDLocal = taskTIDGlobal;

    // If there is no one left to recruit...
    if (nAllocatedWorkers == taskTIDGlobal){
      logMessage("Ended recruitment of %d threads!\n", taskTIDGlobal);
      isRecruiting = 0;      // ... end the recruiting phase...
      taskTIDGlobal = 0;     // ... reset the global task TID...
      currTaskGlobal = NULL; // ... and reset the task at hand, for safety
    }

    pthread_mutex_unlock(lockPtr);

    pool->taskFunc(currTaskLocal, taskTIDLocal, pool); // Finally, execute the task (concurrently)

    pthread_mutex_lock(lockPtr);
    pool->nAvailableThreads++;  // Signals pool that this thread is available
    pthread_mutex_unlock(lockPtr);
  }

  pthread_exit(NULL);
}

// Schedules a task to be executed by a thread in the pool.
void executeTask(TASK task, POOL pool){
  if (!pool || !task)
    return;

  putTask(task, pool->queue);
  logMessage("Task (s=%d, d=%d) was put onto the queue!\n", task->startSeg, task->endSeg);

  //pthread_mutex_lock(&pool->lock);
  // pool->nAvailableThreads--;
  sem_post(&pool->sleeping);
  //pthread_mutex_unlock(&pool->lock);
}

// Recruits other threads in the pool to help in a task, signaling them.
// Assumes external mutual exclusion on access to the pool and task.
// Returns the number of allocated workers in success, 0 otherwise.
int recruitThreads(POOL pool, TASK task){
  if (!pool || !task)
    return 0;
  
  sem_t* sleepingPtr = &pool->sleeping;
  int nAvailableThreads = pool->nAvailableThreads;
  int maxWorkers = pool->maxWorkers - 1; // Maximum number of extra workers allocatable
  int nAllocatedWorkers = min(nAvailableThreads, maxWorkers);

  // pool->nAvailableThreads -= nAllocatedWorkers;
  for (int i = 0; i < nAllocatedWorkers; i++){
    logMessage("Thread %d recruited for task!\n", i+1);
    sem_post(sleepingPtr);
  }
  
  task->nWorkers = nAllocatedWorkers + 1; // Plus one accounting for the recruiter
  task->nFinishedWorkers = 0;

  return nAllocatedWorkers;
}

// Shutdown the pool, forcing threads to start ending activity.
void shutdownPool(POOL pool){
  if (!pool)
    return;

  pthread_mutex_t* lockPtr = &pool->lock;
  sem_t* sleepingPtr = &pool->sleeping;
  int nThreads = pool->nThreads; 

  pthread_mutex_lock(lockPtr);
  pool->shutdown = 1;
  pthread_mutex_unlock(lockPtr);

  for (int i = 0; i < nThreads; i++)
    sem_post(sleepingPtr); // Signaling all threads to prepare for leaving
}

// Waits the joining of threads and destroys the pool.
void waitPoolShutdown(POOL pool){
  if (!pool)
    return;

  int nThreads = pool->nThreads;
  pthread_t* tids = pool->tids;

  for (int i = 0; i < nThreads; i++){
    if (pthread_join(tids[i], NULL)){
      // TODO: think of a good error handling here
    }
    logMessage("Received join of thread %d!\n", i);
  }
  
  destroyQueue(pool->queue);
  free(pool->tids);
  sem_destroy(&pool->sleeping);
  pthread_mutex_destroy(&pool->lock);

  free(pool);
}

// Checks if a given thread is the only one awaken in the pool.
char isLastThreadInPool(POOL pool){
  if (!pool)
    return 0;
  
  pthread_mutex_t* lockPtr = &pool->lock;
  char ret = 0;

  pthread_mutex_lock(lockPtr);
  if (pool->nAvailableThreads == (pool->nThreads-1))
    ret = 1;
  pthread_mutex_unlock(lockPtr);

  return ret;
}
