#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include "utils.h"
#include "timer.h"

enum ORDER {
    INCREASING,
    DECREASING,
    RANDOM
};

int* global_vec;
sem_t global_mutex;

typedef struct {
    int start, end, arrow;
    int i;
    int thread_n,finished;
    sem_t mutex;
} vec_t;

vec_t* create_queue(int start, int end, int thread_n){
    vec_t* vec;
    vec= (vec_t*) malloc(sizeof(vec_t));
    vec->start=start;
    vec->end=end;
    vec->arrow=start;
    vec->i=start;
    vec->thread_n=thread_n;
    vec->finished=0;
    sem_init(&vec->mutex,0,1);
    return vec; 
}

void* quicksort(void* args);

void create_thread(int start,int i, int end, int nthreads){
    int nthreads1,nthreads2;
    nthreads1=nthreads/2;
    nthreads2=nthreads1+(nthreads%2);
    nthreads1+=1*(nthreads1==0);
    nthreads2+=1*(nthreads2==0);
    vec_t* queue1;
    if(i==end){
        queue1=create_queue(start,end-1,nthreads);

        //printf("Creation! Case 1\n");

        pthread_t *threads1 = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
        for (int i = 0; i < nthreads; i++) {
            pthread_create(&threads1[i], NULL, quicksort, (void*)queue1);
        }
        //printf("Creation finished! Case 1\n");

        for (int i = 0; i < nthreads; i++) {
            pthread_join(threads1[i], NULL);
        }
        //printf("Join finished!\n");

        free(threads1);
        free(queue1);
        return;
    }
    if(i==start){
        queue1=create_queue(start+1,end,nthreads);

        //printf("Creation! Case 1\n");

        pthread_t *threads1 = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
        for (int i = 0; i < nthreads; i++) {
            pthread_create(&threads1[i], NULL, quicksort, (void*)queue1);
        }
        //printf("Creation finished! Case 1\n");

        for (int i = 0; i < nthreads; i++) {
            pthread_join(threads1[i], NULL);
        }
        //printf("Join finished!\n");

        free(threads1);
        free(queue1);
        return;
    }

    vec_t* queue2;

    queue1=create_queue(start,i,nthreads1);
    queue2=create_queue(i+1,end,nthreads2);

    pthread_t *threads1 = (pthread_t *)malloc(nthreads1 * sizeof(pthread_t));
    for (int i = 0; i < nthreads1; i++) {
        pthread_create(&threads1[i], NULL, quicksort, (void*)queue1);
    }

    pthread_t *threads2 = (pthread_t *)malloc(nthreads2 * sizeof(pthread_t));
    for (int i = 0; i < nthreads2; i++) {
        pthread_create(&threads2[i], NULL, quicksort, (void*)queue2);
    }
    //printf("Creation finished! Case 2\n");

    for (int i = 0; i < nthreads1; i++) {
        pthread_join(threads1[i], NULL);
    }

    for (int i = 0; i < nthreads2; i++) {
        pthread_join(threads2[i], NULL);
    }
    //printf("Join finished!\n");

    free(threads1);
    free(threads2);
    free(queue1);
    free(queue2);

    return;
}

void* quicksort(void* args){
    int local_arrow, local_i;
    vec_t* vec= (vec_t*) args;
    int temp;
    //printf("Thread created: %i %i\n",vec->start,vec->end);

    sem_wait(&global_mutex);

    //printf("Thread started\n");

    while(1){
        sem_wait(&vec->mutex);
        if(vec->arrow==vec->end){
            //printf("Current i: %i, %i<%i<%i\n",vec->i,global_vec[vec->i],global_vec[vec->end],global_vec[vec->i+1]);
            vec->finished++;
            if(vec->finished==vec->thread_n){
                temp=global_vec[vec->i];
                global_vec[vec->i]=global_vec[vec->end];
                global_vec[vec->end]=temp;
                if(vec->end-vec->start>1){
                    sem_post(&global_mutex);
                    sem_post(&vec->mutex);
                    create_thread(vec->start,vec->i,vec->end,vec->thread_n);
                    pthread_exit(NULL);
                }
            }
            sem_post(&vec->mutex);
            sem_post(&global_mutex);
            pthread_exit(NULL);
        }
        local_arrow=vec->arrow;
        vec->arrow++;
        sem_post(&vec->mutex);

        if(global_vec[local_arrow]<global_vec[vec->end]){
            //printf("changing order, %i<%i\n",global_vec[local_arrow],global_vec[vec->end]);
            if(local_arrow==vec->i){
                sem_wait(&vec->mutex);
                vec->i++;
                sem_post(&vec->mutex);
                continue;
            }

            sem_wait(&vec->mutex);
            local_i=vec->i;
            vec->i++;
            sem_post(&vec->mutex);
             // change i with arrow
            temp=global_vec[local_i];
            global_vec[local_i]=global_vec[local_arrow];
            global_vec[local_arrow]=temp;
        }
        else{
        //printf("not changing order, %i>%i\n",global_vec[local_arrow],global_vec[vec->end]);
        }
    }
}

int main(int argc, char *argv[]) {
    int nthreads, n, orderOption;
    char showVectors = 0;
    double startTime, endTime;
    vec_t* queue;
    int* ordered_vec;

    setRandomSeed();

    if (argc < 4) {
        printf("Use: %s <number of threads> <number of elements> <order option> <print? (OPTIONAL)>\n", argv[0]);
        printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
        return 1;
    }
    
    nthreads = atoi(argv[1]);
    n = atoi(argv[2]);
    orderOption = atoi(argv[3]);
    if (argc >= 5)
        showVectors = atoi(argv[4]);
    
    sem_init(&global_mutex,0,nthreads);

    ordered_vec = makeOrderedVector(n);
    if (!ordered_vec){
        printf("Couldn't allocate ordered vector!\n");
        return 1;
    }

    global_vec = makeCopyVector(ordered_vec, n);
    if (!global_vec){
        printf("Couldn't allocate global vector!\n");
        return 1;
    }

    switch (orderOption){
        case INCREASING:
            break;
        case DECREASING:
            reverse(global_vec, n);
            break;
        case RANDOM:
            shuffle(global_vec, n);
            break;
        default:
            printf("Invalid order option %d!\n", orderOption);
            printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
            return 1;
    }

    if (showVectors){
        printf("Original: ");
        printVector(global_vec, n);
        printf("\n");
    }

    GET_TIME(startTime);

    queue=create_queue(0,n-1,nthreads);
    pthread_t *threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, quicksort, (void*)queue);
    }
    //printf("Creation finished!\n");

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    //printf("Join finished!\n");

    GET_TIME(endTime);

    if (showVectors){
        printf("Ordered: ");
        printVector(global_vec, n);
        printf("\n");
    }

    if (areEqualVectors(ordered_vec, global_vec, n))
        printf("The original vector was sorted successfully! :)\n");
    else
        printf("The sorting of original vector has failed! :(\n");

    printf("Time to sort: %lf s\n", endTime - startTime);

    free(ordered_vec);
    free(global_vec);
    free(threads);
    free(queue);

    return 0;
}