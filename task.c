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
  // ...
  return newTask;
}

// Destroys the task.
void destroyTask(TASK task){
  // Destroy locks & condition variables
  // ...
  
  free(task);
}
