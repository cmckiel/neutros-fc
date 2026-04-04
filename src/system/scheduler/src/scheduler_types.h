#ifndef _SCHEDULER_TYPES_H
#define _SCHEDULER_TYPES_H

typedef bool (*task_init_t)();
typedef bool (*task_exec_t)();

typedef struct {
  task_exec_t task_exec;
} task_t;

#endif /* _SCHEDULER_TYPES_H */
