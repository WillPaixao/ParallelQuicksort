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
