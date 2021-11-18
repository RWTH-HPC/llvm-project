// RUN: %sotoc-transform-compile
// RUN: %run-on-host | %filecheck %s

#include <stdio.h>

#pragma omp declare target
int numbers[10];
#pragma omp end declare target

int main() {
  int *pointers[10];

  #pragma omp target map(tofrom: pointers[:10])
  {
    for (int i = 0; i < 10; ++i) {
      numbers[i] = i;
      pointers[i] = &(numbers[i]);
    }
  }

  #pragma omp target map(tofrom: pointers[:10])
  {
    for (int i = 0; i < 10; ++i) {
      *(pointers[i]) *= 7;
    }
  }
  #pragma omp target update from(numbers[:10])

  for (int i = 0; i < 10; ++i) {
    printf("%d ", numbers[i]);
  }

  return 0;
}

// CHECK: 0 7 14 21 28 35 42 49 56 63