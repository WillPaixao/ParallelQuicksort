#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "taskQueue.h"
#include "threadPool.h"

// TODO:
// - Corrigir implementação de funções de TASK para diferenciar threads
// - Pensar em como verificar fim da recursão (shutdown)
// - Implementar caso de teste com incThread (incrementar números em um vetor)
// - Testar com quicksort de fato

void incThread(TASK args){

}

int main(int argc, char* argv[]){
  int nThreads;
  int maxWorkers;
  const int VEC_LEN = 10;
  int vec[VEC_LEN] = {1,2,3,4,5,6,7,8,9,10};
  POOL pool;
  TASK task;
  
  if (argc < 3){
    printf("ERROR: Arguments missing! Try %s <nº threads> <nº max workers per task>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  nThreads = atoi(argv[1]);
  maxWorkers = atoi(argv[2]);

  pool = makePool(nThreads, maxWorkers, incThread);

  task = makeTask(vec, 0, VEC_LEN-1);
  executeTask(task, pool);

  return 0;
}