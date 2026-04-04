#include "scheduler.h"
#include "scheduler_types.h"
#include "hal/systick.h"

#include "c2.h"
#include "telemetry.h"
#include "imu.h"
#include "i2c_servicer.h"

task_init_t task_init[] = {
  i2c_servicer_init,
  c2_init,
  imu_init,
  telemetry_init,
};

task_t schedule[] = {
  { .task_exec = imu_exec },
  { .task_exec = i2c_servicer_exec },
  { .task_exec = c2_exec },
  { .task_exec = i2c_servicer_exec },
  { .task_exec = imu_exec },
  { .task_exec = i2c_servicer_exec },
  { .task_exec = telemetry_exec },
};

bool scheduler_init()
{
  for (int i = 0; i < sizeof(task_init) / sizeof(task_init_t); ++i)
  {
    if (task_init[i])
    {
      task_init[i]();
    }
  }

  return true;
}

void scheduler_exec()
{
  int i = 0;
  const int NUM_TASKS = sizeof(schedule) / sizeof(task_t);

  while (true)
  {
    if (schedule[i].task_exec)
    {
      schedule[i].task_exec();
    }

    i++;
    i = i % NUM_TASKS;

    hal_delay_ms(20);
  }
}
