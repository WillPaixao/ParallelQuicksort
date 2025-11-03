#pragma once

void setRandomSeed();
int randInt(int min, int max);
void swapInts(int* a, int* b);
int* makeOrderedVector(int len);
int* makeCopyVector(int* src, int len);
void reverse(int* vec, int len);
void shuffle(int* vec, int len);
char areEqualVectors(int* vec1, int* vec2, int len);
void printVector(int* vec, int len);
