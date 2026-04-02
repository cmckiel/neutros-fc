#ifndef _SCHEDULER_TYPES_H
#define _SCHEDULER_TYPES_H

#define MAX_NUM_OF_TASKS 2

typedef bool (*task_init_t)();
typedef bool (*task_exec_t)();

typedef struct {
  task_init_t task_init;
  task_exec_t task_exec;
} task_t;

typedef struct {
  task_t tasks[MAX_NUM_OF_TASKS];
} schedule_t;

#endif /* _SCHEDULER_TYPES_H */
