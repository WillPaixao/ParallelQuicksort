#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

// Sets a random RNG seed in program.
void setRandomSeed(){
  srand(time(NULL));
}

// Yields a random integer from min to max.
int randInt(int min, int max){
  return (rand() % (max - min + 1)) + min;
}

// Swaps two integers in memory.
void swapInts(int* a, int* b){
  int temp;
  temp = *a;
  *a = *b;
  *b = temp;
}

// Makes an (increasing) ordered vector of integers.
// Returns a pointer to the vector in success, NULL otherwise.
int* makeOrderedVector(int len){
  if (len <= 0)
    return NULL;

  int* newVec = (int*)calloc(len, sizeof(int));
  if (!newVec)
    return NULL;
  
  for (int i = 0; i < len; i++)
    newVec[i] = i;
  
  return newVec;
}

// Makes a copy of a source integer vector.
// Returns a pointer to the new vector in success, NULL otherwise.
int* makeCopyVector(int* src, int len){
  int* copy = (int*)calloc(len, sizeof(int));
  if (!copy)
    return NULL;

  memcpy(copy, src, len * sizeof(int));

  return copy;
}

// Reverses the elements of an integer vector.
// Assumes that parameters are well behaved.
void reverse(int* vec, int len){
  for (int i = 0; i < len/2; i++)
    swapInts(&vec[i], &vec[len-i-1]);
}

// Shuffles an integer vector uniformly.
// Assumes that parameters are well behaved.
void shuffle(int* vec, int len){
  int randIdx;
  for (int i = 0; i < len; i++){
    randIdx = randInt(i, len-1);
    swapInts(&vec[i], &vec[randIdx]);
  }
}

// Checks if two integer vectors are equal element-wise.
// Assumes that neither vec's is NULL and that both of them have length len.
char areEqualVectors(int* vec1, int* vec2, int len){
  char ret = 1;
  for (int i = 0; i < len; i++){
    if (vec1[i] != vec2[i]){
      ret = 0;
      break;
    }
  }
  return ret;
}

// Prints the elements of an integer vector.
// Assumes that parameters are well behaved.
void printVector(int* vec, int len){
  for (int i = 0; i < len; i++)
    printf("%d ", vec[i]);
}
