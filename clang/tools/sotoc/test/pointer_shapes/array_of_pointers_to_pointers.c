// RUN: %sotoc-transform-compile
// RUN: %run-on-host | %filecheck %s

#include <stdio.h>

#pragma omp declare target
int numbers[10];
int *ptrs[10];
#pragma omp end declare target

int main() {
  int **pointers[10];

  #pragma omp target map(tofrom: pointers[:10])
  {
    for (int i = 0; i < 10; ++i) {
      numbers[i] = i;
      ptrs[i] = &(numbers[i]);
      pointers[i] = &(ptrs[i]);
    }
  }

  #pragma omp target map(tofrom: pointers[:10])
  {
    for (int i = 0; i < 10; ++i) {
      **(pointers[i]) *= 17;
    }
  }
  #pragma omp target update from(numbers[:10])

  for (int i = 0; i < 10; ++i) {
    printf("%d ", numbers[i]);
  }

  return 0;
}

// CHECK: 0 17 34 51 68 85 102 119 136 153