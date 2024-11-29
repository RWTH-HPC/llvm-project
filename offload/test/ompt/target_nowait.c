// RUN: env LIBOMP_NUM_HIDDEN_HELPER_THREADS=1 %libomptarget-compile-run-and-check-generic
// REQUIRES: ompt
// REQUIRES: gpu

#include <omp.h>
#include <stdio.h>

#include "callbacks_tasking.h"
#include "register_emi_tasking.h"

int main() {
  printf("Devices: %i\n", omp_get_num_devices());
  int a[10] = {0};

#pragma omp target map(tofrom : a[ : 10]) depend(inout : a) nowait
  {
    a[0]++;
  }
#pragma omp target map(tofrom : a[ : 10]) depend(inout : a) nowait
  {
    a[1]++;
  }
#pragma omp target map(tofrom : a[ : 10]) depend(inout : a) nowait
  {
    a[2]++;
  }
  //}

#pragma omp barrier
  printf("%i, %i, %i, %i\n", a[0], a[1], a[2], a[3]);

  return 0;
}

// clang-format off

// TODO use {{.*}} or a new line with CHECK-SAME for readbility?

/// CHECK: Callback Init
/// CHECK-NOT: device_num=-1
/// CHECK: Devices: [[DEVICES:[0-9]+]]

// Save task ids from task creation, these will be the values of target task data later
/// CHECK: Callback Task Create
/// CHECK-SAME: new_task_id=[[TASK_ID1:[0-9a-fA-F]+]]
/// CHECK-SAME: task_type=ompt_task_target
/// CHECK: Callback Task Create
/// CHECK-SAME: new_task_id=[[TASK_ID2:[0-9a-fA-F]+]]
/// CHECK-SAME: task_type=ompt_task_target
/// CHECK: Callback Task Create
/// CHECK-SAME: new_task_id=[[TASK_ID3:[0-9a-fA-F]+]]
/// CHECK-SAME: task_type=ompt_task_target

/// CHECK-DAG: Callback Task Schedule{{.*}}second_task_id=[[TASK_ID1]]
/// CHECK-DAG: Callback Target EMI{{.*}}endpoint=1{{.*}}target_task_data={{.+}}[[TASK_ID1]]{{.+}}target_data=[[TARGET_DATA1:[0-9a-fA-F]+]]
/// CHECK: Callback Target EMI{{.*}}endpoint=2{{.*}}target_task_data={{.+}}[[TASK_ID1]]{{.+}}target_data=[[TARGET_DATA1:[0-9a-fA-F]+]]

/// CHECK-DAG: Callback Task Schedule{{.*}}second_task_id=[[TASK_ID2]]

/// CHECK-DAG: Callback Task Schedule{{.*}}second_task_id=[[TASK_ID3]]

