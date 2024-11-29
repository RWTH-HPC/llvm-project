#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <omp.h>

// Tool related code below
#include <omp-tools.h>

#include "callbacks.h"

#ifndef _TOOL_PREFIX
#define _TOOL_PREFIX ""
// If no _TOOL_PREFIX is set, we assume that we run as part of an OMPT test
#define _OMPT_TESTS
#endif

static ompt_get_thread_data_t ompt_get_thread_data;

static void format_task_type(int type, char *buffer) {
  char *progress = buffer;
  if (type & ompt_task_initial)
    progress += sprintf(progress, "ompt_task_initial");
  if (type & ompt_task_implicit)
    progress += sprintf(progress, "ompt_task_implicit");
  if (type & ompt_task_explicit)
    progress += sprintf(progress, "ompt_task_explicit");
  if (type & ompt_task_target)
    progress += sprintf(progress, "ompt_task_target");
  if (type & ompt_task_taskwait)
    progress += sprintf(progress, "ompt_task_taskwait");
  if (type & ompt_task_undeferred)
    progress += sprintf(progress, "|ompt_task_undeferred");
  if (type & ompt_task_untied)
    progress += sprintf(progress, "|ompt_task_untied");
  if (type & ompt_task_final)
    progress += sprintf(progress, "|ompt_task_final");
  if (type & ompt_task_mergeable)
    progress += sprintf(progress, "|ompt_task_mergeable");
  if (type & ompt_task_merged)
    progress += sprintf(progress, "|ompt_task_merged");
}

static const char *ompt_task_status_t_values[] = {
    "ompt_task_UNDEFINED",
    "ompt_task_complete", // 1
    "ompt_task_yield", // 2
    "ompt_task_cancel", // 3
    "ompt_task_detach", // 4
    "ompt_task_early_fulfill", // 5
    "ompt_task_late_fulfill", // 6
    "ompt_task_switch", // 7
    "ompt_taskwait_complete" // 8
};

static void
on_ompt_callback_task_create(
    ompt_data_t *encountering_task_data,
    const ompt_frame_t *encountering_task_frame,
    ompt_data_t* new_task_data,
    int type,
    int has_dependences,
    const void *codeptr_ra)
{
  if(new_task_data->ptr)
    printf("0: new_task_data initially not null\n");
  new_task_data->value = next_op_id++;
  char buffer[2048];

  format_task_type(type, buffer);

  printf(
      " Callback Task Create: parent_task_id=%lx" //PRIx64
      ", parent_task_frame.exit=%p, parent_task_frame.reenter=%p, "
      "new_task_id=%lx" //PRIx64
      ", codeptr_ra=%p, task_type=%s=%d, has_dependences=%s\n",
      encountering_task_data ? encountering_task_data->value : 0,
      encountering_task_frame ? encountering_task_frame->exit_frame.ptr : NULL,
      encountering_task_frame ? encountering_task_frame->enter_frame.ptr : NULL,
      new_task_data->value, codeptr_ra, buffer, type,
      has_dependences ? "yes" : "no");
}

static void
on_ompt_callback_task_schedule(
    ompt_data_t *first_task_data,
    ompt_task_status_t prior_task_status,
    ompt_data_t *second_task_data)
{
  printf("  Callback Task Schedule: first_task_id=%" PRIx64
         ", second_task_id=%" PRIx64 ", prior_task_status=%s=%d\n",
         first_task_data->value,
         (second_task_data ? second_task_data->value : -1),
         ompt_task_status_t_values[prior_task_status], prior_task_status);
  if (prior_task_status == ompt_task_complete ||
      prior_task_status == ompt_task_late_fulfill ||
      prior_task_status == ompt_taskwait_complete) {
    printf("  Callback Task Schedule End: task_id=%" PRIx64
           "\n", first_task_data->value);
  }
}