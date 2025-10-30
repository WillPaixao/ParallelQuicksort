#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

int* global_vec;

typedef struct {
    int start, end, arrow;
    int i;
    sem_t mutex;
} vec_t;

vec_t* create_queue(int start, int end){
    vec_t* vec;
    vec= (vec_t*) malloc(sizeof(vec_t));
    vec->start=start;
    vec->end=end;
    vec->arrow=start;
    vec->i=start;
    sem_init(&vec->mutex,0,1);
    return vec; 
}

void* quicksort(void* args){
    int local_arrow;
    vec_t* vec= (vec_t*) args;
    int temp;
    while(1){
        sem_wait(&vec->mutex);
        if(vec->arrow=vec->end){
            pthread_exit(NULL);
        }
        local_arrow=vec->arrow;
        vec->arrow++;
        sem_post(&vec->mutex);

        if(global_vec[local_arrow]<global_vec[vec->end]){
            sem_wait(&vec->mutex);
            // change i with arrow
            temp=global_vec[vec->i];
            global_vec[vec->i]=global_vec[vec->arrow];
            global_vec[vec->arrow]=temp;
            vec->i++;
            sem_post(&vec->mutex);
        }
    }
}