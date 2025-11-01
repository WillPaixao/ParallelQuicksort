#pragma once
#include "task.h"

struct queue_s;
typedef struct queue_s* QUEUE; // Opaque pointer to a task queue

QUEUE makeQueue();
char isEmptyQueue(QUEUE queue);
TASK putTask(TASK task, QUEUE queue);
TASK takeTask(QUEUE queue);
void destroyQueue(QUEUE queue);
