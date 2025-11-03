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

int partition(int* vec, int start, int end){
  int i; // Index of last element to the left side of pivot
  int pivotIdx = randInt(start, end);  // Randomizing the pivot selection
  int pivot = vec[pivotIdx];

  // Putting the random pivot at the end of vector
  swapInts(&vec[pivotIdx], &vec[end]);

  i = start-1;
  for (int j = start; j < end; j++){  // Iterating through the vector
    if (vec[j] < pivot){  // If element belongs to left side of pivot...
      i++;              // ... make more space to it...
      swapInts(&vec[i], &vec[j]); // ... and put it into that place
    }
  }

  // The index after i has to be the pivot
  swapInts(&vec[i+1], &vec[end]);

  // Returning the pivot index
  return i+1;
}

void quicksort(TASK task, int taskTID, POOL pool){
  int start = task->start;
  int end = task->end;
  int* vec = task->vector;
  
  logMessage("Task (s=%d, d=%d): Received task\n", start, end);

  if (start >= end){ // Base case: one or less elements in vector
    if (isLastThreadInPool(pool))
      shutdownPool(pool);
    return;
  }
  
  logMessage("Task (s=%d, d=%d): Not base case, before partition\n", start, end);

  int pivotIdx = partition(vec, start, end);
  //char isBaseCase = 1;

  logMessage("Task (s=%d, d=%d): After partition\n", start, end);

  if (pivotIdx-1 > start){
    executeTask(makeTask(vec, start, pivotIdx-1), pool);
    //logMessage("Task (s=%d, d=%d): Calling \n", start, end);
  }
  if (pivotIdx+1 < end){
    executeTask(makeTask(vec, pivotIdx+1, end), pool);
    //logMessage("Task (s=%d, d=%d): After partition\n", start, end);
  }
  destroyTask(task);
}

int main(int argc, char* argv[]){
  int nThreads;
  //int maxWorkers;
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
  
  if (argc < 4){
    printf("ERROR: Arguments missing! Try %s <nº threads> <vector length> <order option> <print? (OPTIONAL)>\n", argv[0]);
    printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  //maxWorkers = atoi(argv[2]);
  vecLen = atoi(argv[2]);
  orderOption = atoi(argv[3]);
  if (argc >= 5)
    showVectors = atoi(argv[4]);

  if (nThreads <= 0 ||
      //maxWorkers <= 0 ||
      //maxWorkers > nThreads ||
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

  pool = makePool(nThreads, 1, quicksort);

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

// Second try of quicksort...
/*
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

  logMessage("Task (s=%d, d=%d): Thread %d started\n", start, end, taskTID);
  while (1){
    pthread_mutex_lock(lockPtr);
    jLocal = *jPtr; // Copying current j locally...
    (*jPtr)++; //... and incrementing for the next worker
    pthread_mutex_unlock(lockPtr);

    logMessage("Task (s=%d, d=%d): jLocal: %d\n", start, end, jLocal);

    // Current j is already out of bounds
    if (jLocal >= end){
      // Last thread in task is responsible for ending it
      if (isLastThreadInTask(task)){
        // Swaps pivot into its designated destination
        //pthread_mutex_lock(lockPtr);
        int pivotIdx = *iPtr;
        //pthread_mutex_unlock(lockPtr);
        
        logMessage("Task (s=%d, d=%d): pivotIdx: %d\n", start, end, pivotIdx);

        swapInts(&vec[pivotIdx], &vec[end]);

        char isBaseCase = 1; // Flag indicating if this thread falls into a base case

        // Makes the recursive calls according to pivot index
        // In making a recursive call, this worker is certainly not in a base case
        if (pivotIdx-1 > start){
          logMessage("Task (s=%d, d=%d): Calling (s=%d, d=%d)\n", start, end, start, pivotIdx-1);
          executeTask(makeTask(vec, start, pivotIdx-1), pool);
          isBaseCase = 0;
        }
        if (pivotIdx+1 < end){
          logMessage("Task (s=%d, d=%d): Calling (s=%d, d=%d)\n", start, end, pivotIdx+1, end);
          executeTask(makeTask(vec, pivotIdx+1, end), pool);
          isBaseCase = 0;
        }

        // If in base case and is the last thread active in the entire pool...
        if (isBaseCase && isLastThreadInPool(pool))
          shutdownPool(pool); // ... the work has been done

        destroyTask(task);
      }
      else 
        finishTask(task); // If this is not the last thread in task, just signal that finished it 
      
      return; // In both cases, return from this function
    }

    // If j is not out of bounds...
    if (vec[jLocal] < pivot){ // ... we need to check if a swap is needed
      if (*iPtr >= jLocal){
        pthread_mutex_lock(lockPtr);
        (*iPtr)++;
        pthread_mutex_unlock(lockPtr);
        continue;
      }
      
      pthread_mutex_lock(lockPtr);
      iLocal = *iPtr;
      (*iPtr)++;
      pthread_mutex_unlock(lockPtr);

      logMessage("Task (s=%d, d=%d): Swapping i=%d with j=%d\n", start, end, iLocal, jLocal);

      // Indexes are local
      swapInts(&vec[iLocal], &vec[jLocal]);
    }
  }
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