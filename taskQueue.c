#include <stdlib.h>
#include <pthread.h>
#include "taskQueue.h"

// Makes a new task.
// Returns a pointer to the task in success, NULL if an error occurred.
TASK makeTask(int startIdx, int endIdx){
  if (startIdx < 0 || endIdx < 0)
    return NULL;
  task_t* newTask = (task_t*)malloc(sizeof(task_t));
  if (!newTask)
    return NULL;
  newTask->startIdx = startIdx;
  newTask->endIdx = endIdx;
  // Initialization of locks & condition variables
  return newTask;
}

// Destroys the task.
void destroyTask(TASK task){
  // Destroy locks & condition variables
  free(task);
}

// Element of queue (separate from task_t)
typedef struct element_s {
  task_t* content;
  struct element_s* next;
} element_t;

// Queue of tasks
typedef struct queue_s {
  element_t* first;
  element_t* last;
  pthread_mutex_t lock; // Each queue has its own mutual exclusion lock
} queue_t;

// Makes an empty task queue.
// Returns a pointer to the queue in success, NULL if an error occurred.
QUEUE makeQueue(){
  queue_t* newQueue = (queue_t*)malloc(sizeof(queue_t));
  if (!newQueue)
    return NULL;
  if (pthread_mutex_init(&newQueue->lock, NULL))
    return NULL;
  newQueue->first = NULL;
  newQueue->last = NULL;
  return newQueue;
}

// Checks if a task queue is empty.
char isEmptyQueue(QUEUE queue){
  return queue->first == NULL;
}

// Puts a task at the end of the task queue.
// Returns a pointer to the task put in success, NULL otherwise.
TASK putTask(TASK task, QUEUE queue){
  if (!task || !queue)
    return NULL;
  element_t* newElement = (element_t*)malloc(sizeof(element_t));
  if (!newElement)
    return NULL;
  newElement->content = task;
  newElement->next = NULL;
  
  // Critical section (mutation of shared queue)
  pthread_mutex_lock(&queue->lock);
  if (isEmptyQueue(queue))
    queue->first = newElement;
  else
    queue->last->next = newElement;
  queue->last = newElement;
  pthread_mutex_unlock(&queue->lock);

  return task;
}

// Takes the task at the start of the task queue.
// Returns a pointer to the task taken in success, NULL otherwise.
TASK takeTask(QUEUE queue){
  if (!queue || isEmptyQueue(queue))
    return NULL;
  element_t* elementTaken;
  task_t* taskTaken;

  // Critical section (mutation of shared queue)
  pthread_mutex_lock(&queue->lock);
  elementTaken = queue->first;
  queue->first = elementTaken->next;
  if (isEmptyQueue(queue))
    queue->last = NULL;
  pthread_mutex_unlock(&queue->lock);

  taskTaken = elementTaken->content;
  free(elementTaken);
  return taskTaken;
}

// Destroys the task queue.
// Assumes that the queue is empty and no threads are accessing it.
void destroyQueue(QUEUE queue){
  pthread_mutex_destroy(&queue->lock);
  free(queue);
}

// DEBUG -- take this out in the final version
#ifdef DEBUG
  #include <stdio.h>

  void printQueue(long int tid, QUEUE queue){
    if (!queue)
      return;
    pthread_mutex_lock(&queue->lock);
    element_t* currElement = queue->first;
    printf("Thread %ld >> ", tid);
    while (currElement){
      printf("%d ", currElement->content->startIdx);
      currElement = currElement->next;
    }
    putchar('\n');
    pthread_mutex_unlock(&queue->lock);
  }
#else
  void printQueue(long int tid, QUEUE queue){}
#endif