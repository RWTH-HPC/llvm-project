// RUN: %libomp-compile && env OMP_MAX_TASK_PRIORITY=5 \
// RUN:    %libomp-run | %sort-threads | FileCheck %s

// RUN: %libomp-compile && env OMP_MAX_TASK_PRIORITY=0 \
// RUN:    %libomp-run | %sort-threads | FileCheck %s --check-prefix NOPRIO

// REQUIRES: ompt
// UNSUPPORTED: gcc
#include "callback.h"
#include <omp.h>

int main() {
  int p_condition = 0, c_condition = 0;
#pragma omp parallel num_threads(2)
  {
#pragma omp master
    {
#pragma omp task shared(p_condition) priority(0)
      {
        OMPT_SIGNAL(p_condition);
        printf("%" PRIu64 ": Executing Task Priority 0\n",
               ompt_get_thread_data()->value);
      }
#pragma omp task shared(p_condition) priority(2)
      {
        OMPT_SIGNAL(p_condition);
        printf("%" PRIu64 ": Executing Task Priority 2\n",
               ompt_get_thread_data()->value);
      }
#pragma omp task shared(p_condition) priority(4)
      {
        OMPT_SIGNAL(p_condition);
        printf("%" PRIu64 ": Executing Task Priority 4\n",
               ompt_get_thread_data()->value);
      }
#pragma omp task shared(p_condition) priority(3)
      {
        OMPT_SIGNAL(p_condition);
        printf("%" PRIu64 ": Executing Task Priority 3\n",
               ompt_get_thread_data()->value);
      }
#pragma omp task shared(p_condition) priority(1)
      {
        OMPT_SIGNAL(p_condition);
        printf("%" PRIu64 ": Executing Task Priority 1\n",
               ompt_get_thread_data()->value);
      }

      OMPT_SIGNAL(c_condition);
      OMPT_WAIT(p_condition, 5);
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
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID2:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_priority: task_id=[[TASK_ID2]], priority=2
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID3:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_priority: task_id=[[TASK_ID3]], priority=4
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID4:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_priority: task_id=[[TASK_ID4]], priority=3
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: {{.*}}new_task_id=[[TASK_ID5:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_priority: task_id=[[TASK_ID5]], priority=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_masked_end:

  // CHECK: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_implicit_task_begin: parallel_id={{[0-9]+}}, task_id=[[IMPLICIT_TASK_ID:[0-9]+]]
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_wait_barrier_explicit_begin:
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID3]], prior_task_status=ompt_task_switch=7
  // CHECK: {{^}}[[THREAD_ID]]: Executing Task Priority 4
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[TASK_ID3]], second_task_id=[[IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID4]], prior_task_status=ompt_task_switch=7
  // CHECK: {{^}}[[THREAD_ID]]: Executing Task Priority 3
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[TASK_ID4]], second_task_id=[[IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID2]], prior_task_status=ompt_task_switch=7
  // CHECK: {{^}}[[THREAD_ID]]: Executing Task Priority 2
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[TASK_ID2]], second_task_id=[[IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID5]], prior_task_status=ompt_task_switch=7
  // CHECK: {{^}}[[THREAD_ID]]: Executing Task Priority 1
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[TASK_ID5]], second_task_id=[[IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[IMPLICIT_TASK_ID]], second_task_id=[[TASK_ID1]], prior_task_status=ompt_task_switch=7
  // CHECK: {{^}}[[THREAD_ID]]: Executing Task Priority 0
  // CHECK: {{^}}[[THREAD_ID]]: ompt_event_task_schedule: first_task_id=[[TASK_ID1]], second_task_id=[[IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1

  // NOPRIO-NOT: ompt_event_task_priority

  // clang-format on

  return 0;
}
