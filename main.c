#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "taskQueue.h"

QUEUE queue;

void* testThread(void* args){
  long int i = (long int)args;
  TASK task = makeTask(i, i+1);
  putTask(task, queue);
  for (int j = 0; j < 1000000; j++);
  printQueue(i, queue);
  task = takeTask(queue);
  destroyTask(task);
  pthread_exit(NULL);
}

int main(int argc, char* argv[]){
  int nThreads;
  
  if (argc < 2){
    printf("ERROR: Arguments missing! Try %s <nº threads>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  if (nThreads < 1){
    printf("ERROR: Invalid number of threads!\n");
    exit(EXIT_FAILURE);
  }
  
  pthread_t tids[nThreads];
  queue = makeQueue();

  for (long int i = 0; i < nThreads; i++){
    if (pthread_create(&tids[i], NULL, testThread, (void*)i)){
      printf("ERROR: Cannot create thread %ld!\n", i);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < nThreads; i++){
    if (pthread_join(tids[i], NULL)){
      printf("ERROR: Cannot join thread %d!", i);
      exit(EXIT_FAILURE);
    }
  }

  destroyQueue(queue);

  return 0;
}