/*
 * DRB203-simd-broadcast-no.c -- Archer testcase
 */
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
//
// See tools/archer/LICENSE.txt for details.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

/*
!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!
!!! Copyright (c) 17-20, Lawrence Livermore National Security, LLC
!!! and DataRaceBench project contributors. See the DataRaceBench/COPYRIGHT file for details.
!!!
!!! SPDX-License-Identifier: (BSD-3-Clause)
!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!
*/

/*
No data race present.
*/

// RUN: %libarcher-compile -DTYPE=float && %libarcher-run | FileCheck %s
// RUN: %libarcher-compile -DTYPE=double && %libarcher-run | FileCheck %s
// REQUIRES: tsan

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef TYPE
#define TYPE double
#endif /*TYPE*/

int main(int argc, char *argv[]) {
  int len = 20000;
  if (argc > 1)
    len = atoi(argv[1]);
  double a[len];
  for (int i = 0; i < len; i++)
    a[i] = i;
  TYPE c = M_PI;

#pragma omp parallel for simd num_threads(2) schedule(dynamic, 64)
  for (int i = 0; i < len; i++)
    a[i] = a[i] + c;

  printf("a[0]=%f, a[%i]=%f, a[%i]=%f\n", a[0], len / 2, a[len / 2], len - 1,
         a[len - 1]);
  fprintf(stderr, "DONE\n");
  return 0;
}

// CHECK-NOT: ThreadSanitizer: data race
// CHECK-NOT: ThreadSanitizer: reported
// CHECK: DONE
