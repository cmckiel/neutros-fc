#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdbool.h>
#include "task.h"

#define MAX_NUM_OF_TASKS 1
typedef struct {
  task_t tasks[MAX_NUM_OF_TASKS];
} schedule_t;

bool scheduler_init();
bool scheduler_exec();

#endif /* _SCHEDULER_H */
