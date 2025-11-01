#include <stdlib.h>
#include <pthread.h>
#include "task.h"
#include "taskQueue.h"

// Element of queue (separate from task_t)
typedef struct element_s {
  TASK content;           // Task contained in the element       
  struct element_s* next; // Pointer to the next element of linked list
} element_t;

// Queue of tasks
typedef struct queue_s {
  // Plain queue variables
  element_t* first;     // Pointer to first element of the queue
  element_t* last;      // Pointer to last element of the queue
  
  // Thread-safety variables
  pthread_mutex_t lock; // Mutex lock to prevent data racing
} queue_t;

// Makes an empty task queue.
// Returns a pointer to the queue in success, NULL if an error occurred.
QUEUE makeQueue(){
  queue_t* newQueue = (queue_t*)malloc(sizeof(queue_t));
  if (!newQueue)
    return NULL;
  
  if (pthread_mutex_init(&newQueue->lock, NULL)){
    free(newQueue);
    return NULL;
  }

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
