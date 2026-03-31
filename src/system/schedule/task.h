#ifndef _TASK_H
#define _TASK_H

typedef bool (*task_init_t)();
typedef bool (*task_exec_t)();

typedef struct {
  task_init_t task_init;
  task_exec_t task_exec;
} task_t;

#endif /* _TASK_H */
