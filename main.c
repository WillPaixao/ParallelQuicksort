// Defining this will allow debug log messages to appear in execution
//#define DEBUG // Comment this line if you do not want log messages

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"
#include "utils.h"
#include "timer.h"

enum ORDER {
  INCREASING,
  DECREASING,
  RANDOM
};

void quicksort(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->start;
  int end = task->end;
  int pivot = vec[end]; // Random pivot is already at the end
  int jLocal;
  int iLocal;

  int* iPtr = &task->i;
  int* jPtr = &task->j;
  pthread_mutex_t* lockPtr = &task->domainLock;

  while (1){
    pthread_mutex_lock(lockPtr);
    jLocal = (*jPtr)++; // Copying current j locally and incrementing for the next worker
    pthread_mutex_unlock(lockPtr);

    // Current j is already out of bounds
    if (jLocal >= end){
      // Last thread in task is responsible for ending it
      if (isLastThreadInTask(task)){
        // Swaps pivot into its designated destination
        int pivotIdx = *iPtr;
        swapInts(&vec[pivotIdx], &vec[end]);

        //char isBaseCase = 1; // Flag indicating if this thread falls into a base case

        // Makes the recursive calls according to pivot index
        // In making a recursive call, this worker is certainly not in a base case
        if (pivotIdx-1 > start){
          executeTask(makeTask(vec, start, pivotIdx-1), pool);
          //isBaseCase = 0;
        }
        if (pivotIdx+1 < end){
          executeTask(makeTask(vec, pivotIdx+1, end), pool);
          //isBaseCase = 0;
        }

        // If in base case and is the last thread active in the entire pool...
        if (/*isBaseCase &&*/ isLastThreadInPool(pool))
          shutdownPool(pool); // ... the work has been done

        destroyTask(task);
      }
      else
        finishTask(task); // If this is not the last thread in task, just signal that finished it 
      
      return; // In both cases, return from this function
    }

    // If j is not out of bounds...
    if (vec[jLocal] < pivot){ // ... we need to check if a swap is needed
      pthread_mutex_lock(lockPtr);
      iLocal = (*iPtr)++;
      pthread_mutex_unlock(lockPtr);
      
      // Indexes are local
      swapInts(&vec[iLocal], &vec[jLocal]);
    }
  }
}

int main(int argc, char* argv[]){
  int nThreads;
  int maxWorkers;
  int vecLen;
  int* orderedVec;
  int* vec;
  int orderOption;
  char showVectors = 0;

  double startTime;
  double endTime;

  POOL pool;
  TASK task;

  setRandomSeed();
  
  if (argc < 5){
    printf("ERROR: Arguments missing! Try %s <nº threads> <nº max workers per task> <vector length> <order option> <print? (OPTIONAL)>\n", argv[0]);
    printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  maxWorkers = atoi(argv[2]);
  vecLen = atoi(argv[3]);
  orderOption = atoi(argv[4]);
  if (argc >= 6)
    showVectors = atoi(argv[5]);

  if (nThreads <= 0 ||
      maxWorkers <= 0 ||
      maxWorkers > nThreads ||
      vecLen <= 0){
    printf("ERROR: Invalid arguments!\n");
    exit(EXIT_FAILURE);
  }

  orderedVec = makeOrderedVector(vecLen);
  if (!orderedVec){
    printf("Couldn't allocate ordered vector!\n");
    return 1;
  }

  vec = makeCopyVector(orderedVec, vecLen);
  if (!vec){
    printf("Couldn't allocate global vector!\n");
    return 1;
  }

  switch (orderOption){
    case INCREASING:
      break;
    case DECREASING:
      reverse(vec, vecLen);
      break;
    case RANDOM:
      shuffle(vec, vecLen);
      break;
    default:
      printf("Invalid order option %d!\n", orderOption);
      printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
      return 1;
  }

  if (showVectors){
    printf("Original: ");
    printVector(vec, vecLen);
    printf("\n");
  }

  GET_TIME(startTime);

  pool = makePool(nThreads, maxWorkers, quicksort);

  task = makeTask(vec, 0, vecLen-1);
  executeTask(task, pool);
  waitPoolShutdown(pool);

  GET_TIME(endTime);

  if (showVectors){
    printf("Ordered: ");
    printVector(vec, vecLen);
    printf("\n");
  }

  if (areEqualVectors(orderedVec, vec, vecLen))
    printf("The original vector was sorted successfully! :)\n");
  else
    printf("The sorting of original vector has failed! :(\n");

  printf("Time to sort: %lf s\n", endTime - startTime);

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