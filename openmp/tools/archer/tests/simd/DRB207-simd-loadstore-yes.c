/*
 * DRB207-simd-loadstore-yes.c -- Archer testcase
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
!!! Copyright (c) 2017-20, Lawrence Livermore National Security, LLC
!!! and DataRaceBench project contributors. See the DataRaceBench/COPYRIGHT file for details.
!!!
!!! SPDX-License-Identifier: (BSD-3-Clause)
!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!
*/

/*
Data race in vectorizable code
Loop depencency with 64 element offset. Data race present.
Data Race Pairs, a[i + 64]@34:5:W vs. a[i]@34:17:R
*/

// RUN: %libarcher-compile -DTYPE=float && %libarcher-run-race | FileCheck --check-prefix=FLOAT %s
// RUN: %libarcher-compile -DTYPE=double && %libarcher-run-race | FileCheck --check-prefix=DOUBLE %s
// REQUIRES: tsan

#include <stdio.h>
#include <stdlib.h>

#ifndef TYPE
#define TYPE double
#endif /*TYPE*/

int main(int argc, char *argv[]) {
  int len = 20000;

  if (argc > 1)
    len = atoi(argv[1]);
  double a[len], b[len];

  for (int i = 0; i < len; i++) {
    a[i] = i;
    b[i] = i + 1;
  }

#pragma omp parallel for simd schedule(dynamic, 64)
  for (int i = 0; i < len - 64; i++)
    a[i + 64] = a[i] + b[i];

  printf("a[0]=%f, a[%i]=%f, a[%i]=%f\n", a[0], len / 2, a[len / 2], len - 1,
         a[len - 1]);
  fprintf(stderr, "DONE\n");
  return 0;
}

// FLOAT: WARNING: ThreadSanitizer: data race
// FLOAT-NEXT:   {{(Write|Read)}} of size {{(4|8)}}
// FLOAT-NEXT: #0 {{.*}}DRB207-simd-loadstore-yes.c
// FLOAT:   Previous {{(read|write)}} of size {{(4|8)}}
// FLOAT-NEXT: #0 {{.*}}DRB207-simd-loadstore-yes.c

// DOUBLE: WARNING: ThreadSanitizer: data race
// DOUBLE-NEXT:   {{(Write|Read)}} of size 8
// DOUBLE-NEXT: #0 {{.*}}DRB207-simd-loadstore-yes.c
// DOUBLE:   Previous {{(read|write)}} of size 8
// DOUBLE-NEXT: #0 {{.*}}DRB207-simd-loadstore-yes.c

// CHECK: DONE
// CHECK: ThreadSanitizer: reported {{[0-9]+}} warnings
