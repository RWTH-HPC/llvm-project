// RUN: %sotoc-transform-compile

#include "stdio.h"
#include "stdlib.h"
#include "omp.h"

int main () {
#pragma omp target parallel
#pragma omp master
  printf("%d",omp_get_thread_num());

  return 0;
}
