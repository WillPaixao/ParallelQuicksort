#pragma once
#include <stdlib.h>

// Representation of a recursive call (task)
typedef struct {
  int startIdx;
  int endIdx;
  // Locks & condition variables
} task_t;

typedef task_t* TASK; // Pointer to a task

TASK makeTask(int startIdx, int endIdx);
void destroyTask(TASK task);

struct queue_s;
typedef struct queue_s* QUEUE; // Opaque pointer to a task queue

QUEUE makeQueue();
TASK putTask(TASK task, QUEUE queue);
TASK takeTask(QUEUE queue);
void destroyQueue(QUEUE queue);

// DEBUG -- take this out in the final version
#define DEBUG
void printQueue(long int tid, QUEUE queue);
