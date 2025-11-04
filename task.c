#include <stdlib.h>
#include <pthread.h>
#include "utils.h"
#include "task.h"

// Makes a new task.
// Returns a pointer to the task in success, NULL if an error occurred.
TASK makeTask(int* vector, int start, int end){
  if (start < 0 || end < 0)
    return NULL;
  
  task_t* newTask = (task_t*)malloc(sizeof(task_t));
  if (!newTask)
    return NULL;
  
  newTask->vector = vector;
  newTask->start = start;
  newTask->end = end;
  newTask->i = start;
  newTask->j = start;

  // Already swapping the random pivot to the end of vector
  int pivotIdx = randInt(start, end);
  swapInts(&vector[pivotIdx], &vector[end]);

  // Initialization of locks & condition variables
  if (pthread_mutex_init(&newTask->controlLock, NULL) ||
      pthread_mutex_init(&newTask->domainLock, NULL)){
    free(newTask);
    return NULL;
  }

  return newTask;
}

// Destroys the task.
void destroyTask(TASK task){
  // Destroy locks & condition variables
  pthread_mutex_destroy(&task->controlLock);
  pthread_mutex_destroy(&task->domainLock);
  
  free(task);
}

// Checks if a given thread executing a task is the last one alive in it.
// WARNING: User must not modify this!
char isLastThreadInTask(TASK task){
  if (!task)
    return 0;
  char ret = 0;
  if (task->nFinishedWorkers == (task->nWorkers-1))
    ret = 1;
  return ret;
}

// Signals that a thread finished its execution in a task.
// WARNING: User must not modify this!
void finishTask(TASK task){
  if (!task)
    return;
  task->nFinishedWorkers++;   // Signals task that this thread finished its work
}
