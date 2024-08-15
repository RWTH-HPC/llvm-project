// RUN: %libomp-compile && \
// RUN:    %libomp-run | %sort-threads | FileCheck %s

// REQUIRES: ompt,clang-20
// UNSUPPORTED: gcc
#include "callback.h"
#include <omp.h>

int main() {
  int p_condition = 0, c_condition = 0;
#pragma omp parallel num_threads(2)
  {
#pragma omp master
    {
#pragma omp task shared(p_condition) ompx_name("Task 1")
      { OMPT_SIGNAL(p_condition); }
#pragma omp task shared(p_condition) ompx_name("Task 2")
      { OMPT_SIGNAL(p_condition); }
#pragma omp task shared(p_condition) ompx_name("Task 3")
      { OMPT_SIGNAL(p_condition); }

      OMPT_SIGNAL(c_condition);
      OMPT_WAIT(p_condition, 3);
    }
    OMPT_WAIT(c_condition, 1);
#pragma omp barrier
    print_ids(0);
  }

  // clang-format off
  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_create'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_schedule'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_priority'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_begin'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_end'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_mutex_acquire'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_mutex_acquired'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_mutex_released'

  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]

  // make sure initial data pointers are null
  // CHECK-NOT: 0: new_task_data initially not null

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_masked_begin:
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID1:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_name: task_id=[[TASK_ID1]], name=Task 1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID2:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_name: task_id=[[TASK_ID2]], name=Task 2
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID3:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_name: task_id=[[TASK_ID3]], name=Task 3
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_masked_end:

  // CHECK: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_implicit_task_begin: parallel_id={{[0-9]+}}, task_id=[[IMPLICIT_TASK_ID:[0-9]+]]
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_wait_barrier_explicit_begin:
  // CHECK-DAG: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID1]], prior_task_status=ompt_task_switch=7
  // CHECK-DAG: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID2]], prior_task_status=ompt_task_switch=7
  // CHECK-DAG: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID3]], prior_task_status=ompt_task_switch=7

  // clang-format on

  return 0;
}
