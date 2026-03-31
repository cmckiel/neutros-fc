#include "config.h"
#include "c2.h"
#include "hal/uart.h"

system_config_t system_config = {
  .c2_uart_channel = HAL_UART1,
};

schedule_t schedule = {
  .tasks = {
    {
      .task_init = c2_init,
      .task_exec = c2_exec
    },
  }
};
