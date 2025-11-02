#pragma once
#include <pthread.h>

// Representation of a recursive call (task)
typedef struct {
  // Domain variables
  int* vector;
  int startSeg;
  int endSeg;
  //int i;
  //int j;
  
  // Control variables (obligatory!)
  int nWorkers;
  int nFinishedWorkers;
  pthread_mutex_t controlLock;
        
  // Locks & condition variables
  // ...
} task_t;

typedef task_t* TASK; // Pointer to a task

TASK makeTask(int* vector, int startIdx, int endIdx);
void destroyTask(TASK task);
char isLastThreadInTask(TASK task);
void finishTask(TASK task);
