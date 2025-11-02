// Defining this will allow debug log messages to appear in execution
#define DEBUG // Comment this line if you do not want log messages

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"

// TODO: Testar com quicksort de fato

void incThread(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->startSeg;
  int end = task->endSeg;
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
  const int VEC_LEN = 10;
  int vec[] = {1,2,3,4,5,6,7,8,9,10};
  POOL pool;
  TASK task;
  
  if (argc < 3){
    printf("ERROR: Arguments missing! Try %s <nº threads> <nº max workers per task>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  maxWorkers = atoi(argv[2]);

  if (nThreads <= 0 ||
      maxWorkers <= 0 ||
      maxWorkers > nThreads){
    printf("ERROR: Invalid arguments!\n");
    exit(EXIT_FAILURE);
  }

  printf("Initial vector: ");
  for (int i = 0; i < VEC_LEN; i++)
    printf("%d ", vec[i]);
  printf("\n");

  pool = makePool(nThreads, maxWorkers, incThread);

  task = makeTask(vec, 0, VEC_LEN-1);
  executeTask(task, pool);
  waitPoolShutdown(pool);

  printf("Final vector: ");
  for (int i = 0; i < VEC_LEN; i++)
    printf("%d ", vec[i]);
  printf("\n");

  return 0;
}