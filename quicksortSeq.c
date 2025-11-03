#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "timer.h"

enum ORDER {
  INCREASING,
  DECREASING,
  RANDOM
};

int partition(int* vec, int start, int end){
  int i; // Index of last element to the left side of pivot
  int pivotIdx = randInt(start, end);  // Randomizing the pivot selection
  int pivot = vec[pivotIdx];

  // Putting the random pivot at the end of vector
  swapInts(&vec[pivotIdx], &vec[end]);

  i = start-1;
  for (int j = start; j < end; j++){  // Iterating through the vector
    if (vec[j] < pivot){  // If element belongs to left side of pivot...
      i++;              // ... make more space to it...
      swapInts(&vec[i], &vec[j]); // ... and put it into that place
    }
  }

  // The index after i has to be the pivot
  swapInts(&vec[i+1], &vec[end]);

  // Returning the pivot index
  return i+1;
}

void quicksortSeq(int* vec, int start, int end){
  if (start >= end) // Base case: one or less elements in vector
    return;
  
  int pivotIdx = partition(vec, start, end);
  quicksortSeq(vec, start, pivotIdx-1);
  quicksortSeq(vec, pivotIdx+1, end);
}

int main(int argc, char *argv[]) {
    int len, orderOption;
    char showVectors = 0;
    double startTime, endTime;
    int* orderedVec;
    int* originalVec;

    setRandomSeed();

    if (argc < 3) {
      printf("Use: %s <number of elements> <order option> <print? (OPTIONAL)>\n", argv[0]);
      printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
      return 1;
    }
    
    len = atoi(argv[1]);
    orderOption = atoi(argv[2]);
    if (argc >= 4)
      showVectors = atoi(argv[3]);

    orderedVec = makeOrderedVector(len);
    if (!orderedVec){
      printf("Couldn't allocate ordered vector!\n");
      return 1;
    }

    originalVec = makeCopyVector(orderedVec, len);
    if (!originalVec){
      printf("Couldn't allocate original vector!\n");
      return 1;
    }

    switch (orderOption){
      case INCREASING:
        break;
      case DECREASING:
        reverse(originalVec, len);
        break;
      case RANDOM:
        shuffle(originalVec, len);
        break;
      default:
        printf("Invalid order option %d!\n", orderOption);
        printf("Order options: (0) increasing, (1) decreasing, (2) random\n");
        return 1;
    }

    if (showVectors){
      printf("Original: ");
      printVector(originalVec, len);
      printf("\n");
    }

    GET_TIME(startTime);

    quicksortSeq(originalVec, 0, len-1);

    GET_TIME(endTime);

    if (showVectors){
      printf("Ordered: ");
      printVector(originalVec, len);
      printf("\n");
    }

    if (areEqualVectors(orderedVec, originalVec, len))
        printf("The original vector was sorted successfully! :)\n");
    else
        printf("The sorting of original vector has failed! :(\n");

    printf("Time to sort: %lf s\n", endTime - startTime);

    free(orderedVec);
    free(originalVec);

    return 0;
}