#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include <time.h>

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
        if(vec->arrow==vec->end){
            printf("Current i: %i, %i<%i<%i",vec->i,global_vec[vec->i],global_vec[vec->end],global_vec[vec->i+1]);
            free(vec);
            pthread_exit(NULL);
        }
        local_arrow=vec->arrow;
        vec->arrow++;
        sem_post(&vec->mutex);

        if(global_vec[local_arrow]<global_vec[vec->end]){
            printf("changing order, %i<%i\n",global_vec[local_arrow],global_vec[vec->end]);

            sem_wait(&vec->mutex);
            // change i with arrow
            temp=global_vec[vec->i];
            global_vec[vec->i]=global_vec[vec->arrow];
            global_vec[vec->arrow]=temp;
            vec->i++;
            sem_post(&vec->mutex);
        }
        else{
        printf("not changing order, %i>%i\n",global_vec[local_arrow],global_vec[vec->end]);
        }
    }
}

int main(int argc, char *argv[]) {
    int nthreads,n,randomNumber,start,end,lenght;
    vec_t* queue;

    srand(time(NULL));

    if (argc != 3) {
        printf("Use: %s <number of threads> <number of elements>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);
    n = atoi(argv[2]);
    lenght=n/nthreads;

    global_vec=(int*) malloc(sizeof(int)*n);
    for (int i = 0; i < n; i++) {
        randomNumber = rand() % 1000;
        global_vec[i]=randomNumber;
    }
    queue=create_queue(0,n-1);
    pthread_t *threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, quicksort, (void*)queue);
    }
    printf("Creation finished!\n");

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Join finished!\n");

    for (int i = 0; i < n-1; i++) {

        if(global_vec[i]<global_vec[n-1] && global_vec[i+1]>global_vec[n-1]){
            printf("Maybe Correct exec, if once\n");
        }
    }

    free(global_vec);
    return 0;
}