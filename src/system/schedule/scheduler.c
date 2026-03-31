#include "scheduler.h"
#include "config.h"
#include "hal/systick.h"

bool scheduler_init()
{
  for (int i = 0; i < sizeof(schedule.tasks) / sizeof(task_t); ++i)
  {
    if (schedule.tasks[i].task_init)
    {
      schedule.tasks[i].task_init();
    }
  }
}

bool scheduler_exec()
{
  int i = 0;
  const NUM_TASKS = sizeof(schedule.tasks) / sizeof(task_t);

  while (true)
  {
    if (schedule.tasks[i].task_exec)
    {
      schedule.tasks[i].task_exec();
    }

    i++;
    i = i % NUM_TASKS;

    hal_delay_ms(50);
  }
}
