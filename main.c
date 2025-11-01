#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task.h"
#include "taskQueue.h"
#include "threadPool.h"

// TODO:
// - Implementar caso de teste com incThread (incrementar números em um vetor)
// - Testar com quicksort de fato

// Há dois bugs fundamentais ocorrendo na versão atual:
// - Quando há mais threads que elementos no vetor, programa entra em loop infinito de criação de tasks
// - No caso contrário, programa entra em deadlock

void incThread(TASK task, int taskTID, POOL pool){
  int* vec = task->vector;
  int start = task->startSeg;
  int end = task->endSeg;
  int nWorkers = task->nWorkers;

  printf("Nº workers: %d\n", nWorkers);
  printf("Task TID: %d\n", taskTID);
  for (int i = start+taskTID; i <= end; i += nWorkers)
    vec[i]++;
  
  if (isLastThreadInTask(task)){
    printf("Incremented vector!\n");

    if (start < end){ // Recursive case
      makeTask(vec, start, end/2);
      makeTask(vec, (end/2)+1, end);

      executeTask(makeTask(vec, start, end/2), pool);
      executeTask(makeTask(vec, (end/2)+1, end), pool);
    }
    else if (isLastThreadInPool(pool)) // Base case & is last thread awaken
      shutdownPool(pool);

    destroyTask(task);
  }
  else {
    printf("Task TID %d finished task!\n", taskTID);
    finishTask(task);
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

  printf("\nInitial vector: ");
  for (int i = 0; i < VEC_LEN; i++)
    printf("%d ", vec[i]);
  printf("\n");

  pool = makePool(nThreads, maxWorkers, incThread);

  task = makeTask(vec, 0, VEC_LEN-1);
  executeTask(task, pool);
  waitPoolShutdown(pool);

  printf("\nFinal vector: ");
  for (int i = 0; i < VEC_LEN; i++)
    printf("%d ", vec[i]);
  printf("\n");

  return 0;
}