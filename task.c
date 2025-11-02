#include <stdlib.h>
#include <pthread.h>
#include "task.h"

// Makes a new task.
// Returns a pointer to the task in success, NULL if an error occurred.
TASK makeTask(int* vector, int startSeg, int endSeg){
  if (startSeg < 0 || endSeg < 0)
    return NULL;
  
  task_t* newTask = (task_t*)malloc(sizeof(task_t));
  if (!newTask)
    return NULL;
  
  newTask->vector = vector;
  newTask->startSeg = startSeg;
  newTask->endSeg = endSeg;
  // newTask->i = startSeg - 1;
  // newTask->j = startSeg;

  // Initialization of locks & condition variables
  if (pthread_mutex_init(&newTask->controlLock, NULL)
      /* ... */){
    free(newTask);
    return NULL;
  }

  return newTask;
}

// Destroys the task.
void destroyTask(TASK task){
  // Destroy locks & condition variables
  pthread_mutex_destroy(&task->controlLock);
  // ...
  
  free(task);
}

// Checks if a given thread executing a task is the last one alive in it.
char isLastThreadInTask(TASK task){
  if (!task)
    return 0;
  
  pthread_mutex_t* lockPtr = &task->controlLock;
  char ret = 0;
  
  pthread_mutex_lock(lockPtr);
  if (task->nFinishedWorkers == (task->nWorkers-1))
    ret = 1;
  pthread_mutex_unlock(lockPtr);

  return ret;
}

// Signals that a thread finished its execution in a task.
void finishTask(TASK task){
  if (!task)
    return;
  
  pthread_mutex_t* lockPtr = &task->controlLock;

  pthread_mutex_lock(lockPtr);
  task->nFinishedWorkers++;   // Signals task that this thread finished its work
  pthread_mutex_unlock(lockPtr);
}
