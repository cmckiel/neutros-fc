#include "scheduler.h"
#include "scheduler_types.h"
#include "task.h"
#include "hal/systick.h"

#include "c2.h"
#include "telemetry.h"

schedule_t schedule = {
  .tasks = {
    {
      .task_init = c2_init,
      .task_exec = c2_exec
    },
    {
      .task_init = telemetry_init,
      .task_exec = telemetry_exec
    }
  }
};

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
