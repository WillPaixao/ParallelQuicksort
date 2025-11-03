// Defining this will allow debug log messages to appear in execution
//#define DEBUG // Comment this line if you do not want log messages

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"

// TODO: Testar com quicksort de fato

void incThread(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->start;
  int end = task->end;
  int nWorkers = task->nWorkers;
  int mid = start + (end - start)/2;

  logMessage("Task (s=%d, d=%d): Nº workers: %d\n", start, end, nWorkers);
  logMessage("Task (s=%d, d=%d): Task TID: %d\n", start, end, taskTID);
  for (int i = start+taskTID; i <= end; i += nWorkers)
    vec[i]++;
  
  logMessage("Task (s=%d, d=%d): Incremented vector!\n", start, end);

  if (isLastThreadInTask(task)){
    if (start < end){ // Recursive case
      executeTask(makeTask(vec, start, mid), pool);
      executeTask(makeTask(vec, mid+1, end), pool);
    }
    else if (isLastThreadInPool(pool)){ // Base case & is last thread awaken
      shutdownPool(pool);
    }

    logMessage("Task (s=%d, d=%d) is being destroyed!\n", start, end);
    destroyTask(task);
  }
  else {
    finishTask(task); // Signal that this thread is finished in this task
    logMessage("Task (s=%d, d=%d): Task TID %d finished task!\n", start, end, taskTID);
  }
}

int main(int argc, char* argv[]){
  int nThreads;
  int maxWorkers;
  int vecLen;
  int* vec;
  char showVectors = 0;

  double startTime;
  double endTime;

  POOL pool;
  TASK task;
  
  if (argc < 4){
    printf("ERROR: Arguments missing! Try %s <nº threads> <nº max workers per task> <vector length> <show vectors? (OPTIONAL)>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  maxWorkers = atoi(argv[2]);
  vecLen = atoi(argv[3]);
  if (argc >= 5)
    showVectors = atoi(argv[4]);

  if (nThreads <= 0 ||
      maxWorkers <= 0 ||
      maxWorkers > nThreads ||
      vecLen <= 0){
    printf("ERROR: Invalid arguments!\n");
    exit(EXIT_FAILURE);
  }

  vec = (int*)calloc(vecLen, sizeof(int));

  if (showVectors){
    printf("Initial vector: ");
    for (int i = 0; i < vecLen; i++)
      printf("%d ", vec[i]);
    printf("\n");
  }

  GET_TIME(startTime);

  pool = makePool(nThreads, maxWorkers, incThread);

  task = makeTask(vec, 0, vecLen-1);
  executeTask(task, pool);
  waitPoolShutdown(pool);

  GET_TIME(endTime);

  if (showVectors){
    printf("Final vector: ");
    for (int i = 0; i < vecLen; i++)
      printf("%d ", vec[i]);
    printf("\n");
  }

  printf("Time elapsed: %lf s\n", endTime - startTime);

  free(vec);

  return 0;
}

// First try of quicksort...
/*
void quicksort(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->start;
  int end = task->end;
  int pivot = vec[end]; // Random pivot is already at the end

  int* iPtr = &task->i; // Starts as start-1
  int* jPtr = &task->j; // Starts as start
  pthread_mutex_t* lockPtr = &task->domainLock;
  int iCopy;
  int jCopy;

  if (start >= end){ // Base case
    if (isLastThreadInPool(pool)){
      shutdownPool(pool);
      destroyTask(task);
    }
    finishTask(task);
    return;
  }

  while (1){
    pthread_mutex_lock(lockPtr);
    iCopy = *iPtr;
    jCopy = *jPtr;
    pthread_mutex_unlock(lockPtr);

    if (jCopy < end){ // If is still iterating
      if (vec[jCopy] < pivot){
        swapInts(&vec[iCopy+1], &vec[jCopy]);
        
        pthread_mutex_lock(lockPtr);
        (*iPtr)++;
        (*jPtr)++;
        pthread_mutex_unlock(lockPtr);
      }
      else {
        pthread_mutex_lock(lockPtr);
        (*jPtr)++;
        pthread_mutex_unlock(lockPtr);
      }
    }
    else if (isLastThreadInTask(task)) { // If it is the last iteration
      iCopy = *iPtr;
      swapInts(&vec[iCopy], &vec[jCopy]);

      executeTask(makeTask(vec, start, iCopy-1), pool);
      executeTask(makeTask(vec, iCopy+1, end), pool);
      break;
    }
  }

  finishTask(task);
}
*/

// Our initial test function incThread()
/*
void incThread(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->start;
  int end = task->end;
  int nWorkers = task->nWorkers;
  int mid = start + (end - start)/2;

  logMessage("Task (s=%d, d=%d): Nº workers: %d\n", start, end, nWorkers);
  logMessage("Task (s=%d, d=%d): Task TID: %d\n", start, end, taskTID);
  for (int i = start+taskTID; i <= end; i += nWorkers)
    vec[i]++;
  
  logMessage("Task (s=%d, d=%d): Incremented vector!\n", start, end);

  if (isLastThreadInTask(task)){
    if (start < end){ // Recursive case
      executeTask(makeTask(vec, start, mid), pool);
      executeTask(makeTask(vec, mid+1, end), pool);
    }
    else if (isLastThreadInPool(pool)){ // Base case & is last thread awaken
      shutdownPool(pool);
    }

    logMessage("Task (s=%d, d=%d) is being destroyed!\n", start, end);
    destroyTask(task);
  }
  else {
    finishTask(task); // Signal that this thread is finished in this task
    logMessage("Task (s=%d, d=%d): Task TID %d finished task!\n", start, end, taskTID);
  }
}
*/